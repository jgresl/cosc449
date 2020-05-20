#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <Wire.h>
#include <rtcZero.h>
#include <stdio.h>
#include <stdlib.h>

#include "lorax_buffer.h"
#include "string_helper.h"

/* Feather M0 wiring configurations */
#define RF95_CHIP_SELECT_PIN 8
#define RF95_INTERRUPT_PIN 3
#define RF95_RESET_PIN 4
#define VBATPIN A7  // Voltage pin to determine battery life

/* LoRa radio configurations */
#define RF95_FREQUENCY 915.0          // Between 137.0 and 1020.0   (Default = 915 Mhz)
#define RF95_TRANSMISSION_POWER 20    // Between 5 and 23           (Default = 13 Db)
#define RF95_CAD_TIMEOUT 10000        // Greater or equal to 0      (Default = 0 ms)
#define RF95_SPREADING_FACTOR 12      // Between 6 and 12           (Default = ?)             --> Overwritten by setModemConfig()
#define RF95_BANDWIDTH 125000         // Between 7800 and 500000    (Default = 125000 Hz)     --> Overwritten by setModemConfig()
#define RF95_PREAMBLE_LENGTH 8        //                            (Default = 8)
#define RF95_SYNC_WORD 0x39           //                            (Default = 0x39)
#define RF95_CODING_RATE 5            // Between 5 and 8            (Default = 5-bit)         --> Overwritten by setModemConfig()
#define RF95_RESTRANSMIT_RETRIES 3    //                            (Default = 3)
#define RF95_RESTRANSMIT_TIMEOUT 500  //                            (Default = 200 ms)
#define RF95_GATEWAY_ID 100           // Assign unique ID to gateway

/* Declare functions */
void initialize_node();
void sync_with_gateway();
void log_samples();
void transmit_samples();
void set_date_time(char *);
const char *get_date_time(char *);
void set_new_alarm();

/* Create an instance of the real time clock */
RTCZero rtc;

/* Create instance of the radio driver */
RH_RF95 rf95(RF95_CHIP_SELECT_PIN, RF95_INTERRUPT_PIN);

/* Create instance of the packet manager */
RHReliableDatagram manager(rf95);

/* Create instance of BME280 sensor */
Adafruit_BME280 bme;

/* Create a queue to log the sensor readings (uint8_t[100] : max = 200 elements) */
lorax_buffer *all_samples = create_buffer(150, 100);

/* Gloabal variables */
int node_id = 0;
int sample_number = 1;
char date_time_on_sync[19];

void setup() {
    /* Initialize the real time clock */
    rtc.begin();

    /* Begin BME280 sensor */
    bme.begin(0x76);

    /* Initialize LoRa radio with defined configurations */
    initialize_node();

    /* Sync node with gateway */
    while (node_id == 0) {
        sync_with_gateway();
    }

    /* Setup interrupt for taking samples */
    rtc.setAlarmSeconds(59);
    rtc.enableAlarm(rtc.MATCH_SS);
    rtc.attachInterrupt(log_samples);

    delay(1000);
}

void loop() {
    /* Get total seconds since device startup */
    uint8_t totalSeconds = rtc.getHours() * 3600 + rtc.getMinutes() * 60 + rtc.getSeconds();

    /* Transmit samples every 20 seconds */
    if (node_id > 0 && totalSeconds % 65 == 0) {
        transmit_samples();
    }

    /* Delay to prevent loop from running more than once per a second */
    delay(1000);
}

void initialize_node() { /* Manual reset on radio module */
    pinMode(RF95_RESET_PIN, OUTPUT);
    digitalWrite(RF95_RESET_PIN, LOW);
    delay(10);
    digitalWrite(RF95_RESET_PIN, HIGH);
    delay(10);

    /* Initialize LoRa radio */
    Serial.printf("Radio:       ");
    while (!manager.init()) {
        Serial.printf("Failed\n");
        while (1)
            ;
    }
    Serial.printf("Initialized\n");

    /* Set radio address */
    Serial.printf("Address:     ");
    Serial.printf("%d\n", manager.thisAddress());

    /* Set frequency (ISM band) */
    Serial.printf("Frequency:   ");
    rf95.setFrequency(RF95_FREQUENCY);
    Serial.print(RF95_FREQUENCY);
    Serial.printf(" Mhz\n");

    /* Set transmission power (5 to 23 dBm) */
    Serial.printf("Power:       ");
    rf95.setTxPower(RF95_TRANSMISSION_POWER, false);
    Serial.printf("%d dBm\n", RF95_TRANSMISSION_POWER);

    /* Set channel activity detection timeout 
    Serial.printf("CAD timeout:\t\t\t");
    rf95.setCADTimeout(RF95_CAD_TIMEOUT);
    Serial.printf("%d ms\n", RF95_CAD_TIMEOUT); */

    /* Set signal bandwith
    Serial.printf("Bandwidth:\t\t\t");
    rf95.setSignalBandwidth(RF95_BANDWIDTH);
    Serial.printf("%d Hz\n", RF95_BANDWIDTH); */

    /* Set coding rate 
    Serial.printf("Coding rate:\t\t\t");
    rf95.setCodingRate4(5);
    Serial.printf("%d-bit\n", RF95_CODING_RATE); */

    /* Set spreading factor
    Serial.printf("Spreading factor:\t\t");
    rf95.setSpreadingFactor(RF95_SPREADING_FACTOR);
    Serial.printf("SF%d\n", RF95_SPREADING_FACTOR); */

    /* Set preamble length 
    Serial.printf("Preamble length:\t\t");
    rf95.setSignalBandwidth(RF95_PREAMBLE_LENGTH);
    Serial.printf("%x\n", RF95_PREAMBLE_LENGTH); */

    /* Set sync word 
    Serial.printf("Sync word:\t\t\t");
    rf95.spiWrite(RH_RF95_REG_39_SYNC_WORD, RF95_SYNC_WORD);
    Serial.printf("%x\n", RF95_SYNC_WORD); */

    /* Set modem configuration */
    if (!rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128)) {
        Serial.print("Invalid setModemConfig() option\n");
    } else {
        Serial.printf("Bandwidth:   ");
        Serial.printf("%d Hz\n", 125000);

        Serial.printf("Coding rate: ");
        Serial.printf("%s\n", "4/5");

        Serial.printf("Spreading:   ");
        Serial.printf("%d c/s\n", 128);
    }

    /* Set amount of transmit retries */
    Serial.printf("Retries:     ");
    manager.setRetries(RF95_RESTRANSMIT_RETRIES);
    Serial.printf("%d\n", RF95_RESTRANSMIT_RETRIES);

    /* Set minimum re-transmit timeout */
    Serial.printf("Retry time:  ");
    manager.setTimeout(RF95_RESTRANSMIT_TIMEOUT);
    Serial.printf("%d ms\n", RF95_RESTRANSMIT_TIMEOUT);
}

void sync_with_gateway() {
    /* Request node_id and date_time from gateway */
    const uint32_t *serial_number_words[4] = {(uint32_t *)0x0080A00C, (uint32_t *)0x0080A040, (uint32_t *)0x0080A044, (uint32_t *)0x0080A048};
    char serial_number[130];
    sprintf((char *)serial_number, "%lu %lu %lu %lu", *serial_number_words[0], *serial_number_words[1], *serial_number_words[2], *serial_number_words[3]);

    Serial.print("\nSending to gateway:\t");
    Serial.println((char *)serial_number);
    if (manager.sendtoWait((uint8_t *)serial_number, sizeof(serial_number), RF95_GATEWAY_ID)) {
        /* Receive response from gateway */
        uint8_t node_id_response[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t response_length = sizeof(node_id_response);
        uint8_t gateway_id;
        if (manager.recvfromAckTimeout(node_id_response, &response_length, 2000, &gateway_id)) {
            /* Print response from gateway */
            Serial.print("Received from gateway:\t");
            Serial.println((char *)node_id_response);

            /* Store node_id and change address of RHReliable datagram driver */
            node_id = (char)node_id_response[12] - 48;
            manager.setThisAddress(node_id);
            Serial.printf("\n\t\t\tnode_id set to %d\n", node_id);

            /* Store date_time_on_sync and millis_elapsed_on_sync */
            substring((char *)node_id_response, date_time_on_sync, 30, 19);
            set_date_time(date_time_on_sync);
            Serial.printf("\t\t\tdate_time_on_sync set to %s\n", date_time_on_sync);
        }
    }
}

void log_samples() {
    /* Read values from sensors */
    Serial.print("\nTaking sensor samples...\n");
    int celsius = bme.readTemperature();
    int humidity = bme.readHumidity();
    int pressure = bme.readPressure();

    /* Read battery voltage */
    float battery = analogRead(VBATPIN);
    battery *= 2;     // we divided by 2, so multiply back
    battery *= 3.3;   // Multiply by 3.3V, our reference voltage
    battery /= 1024;  // convert to voltage
    int battery100 = battery * 100;

    /* Prepare payloads */
    Serial.print("\nPreparing JSON samples...\n");
    /* Current Time */
    char *date_time = (char *)malloc(sizeof(char) * 19);
    get_date_time(date_time);
    /* Temperature */
    char temperature_sample[150];
    sprintf(temperature_sample, "{\"node_id\":\"%d\", \"sensor_type\":\"%c\", \"date_time\":\"%s\", \"value\":\"%d\", \"sample\":\"%d\"}", node_id, 'T', date_time, celsius, sample_number);
    sample_number++;
    Serial.println(temperature_sample);
    /* Humidity */
    char humidity_sample[150];
    sprintf(humidity_sample, "{\"node_id\":\"%d\", \"sensor_type\":\"%c\", \"date_time\":\"%s\", \"value\":\"%d\", \"sample\":\"%d\"}", node_id, 'H', date_time, humidity, sample_number);
    sample_number++;
    Serial.println(humidity_sample);
    /* Air Pressure */
    char pressure_sample[150];
    sprintf(pressure_sample, "{\"node_id\":\"%d\", \"sensor_type\":\"%c\", \"date_time\":\"%s\", \"value\":\"%d\", \"sample\":\"%d\"}", node_id, 'P', date_time, pressure, sample_number);
    sample_number++;
    Serial.println(pressure_sample);
    /* Battery */
    char battery_sample[150];
    sprintf(battery_sample, "{\"node_id\":\"%d\", \"sensor_type\":\"%c\", \"date_time\":\"%s\", \"value\":\"%d\", \"sample\":\"%d\"}", node_id, 'B', date_time, battery100, sample_number);
    sample_number++;
    Serial.println(battery_sample);

    free(date_time);

    /* Store payloads */
    Serial.println("\nLogging JSON samples...");
    store_sample(all_samples, (uint8_t *)temperature_sample);
    store_sample(all_samples, (uint8_t *)humidity_sample);
    store_sample(all_samples, (uint8_t *)pressure_sample);
    store_sample(all_samples, (uint8_t *)battery_sample);
    print_buffer(all_samples);

    /* Set new interupt timing (used when sampling every 15 seconds) */
    //set_new_alarm();
}

void transmit_samples() {
    Serial.println("\nTransmitting samples...");
    void *memoryLocationForQueueElement = malloc(all_samples->__element_size);
    uint16_t size = all_samples->__size;
    
    for (uint16_t i = 0; i < size; i++) {   
        // Store the first element (the one being tranmitted) in the Queue and print it
        peek_sample(all_samples, (uint8_t *)memoryLocationForQueueElement);
        // If transmitted successfully
        if (manager.sendtoWait((uint8_t *)memoryLocationForQueueElement, all_samples->__element_size, RF95_GATEWAY_ID)) {
            Serial.printf("Transmission acknowledged");
            Serial.printf("   Last RSSI: %d\n", rf95.lastRssi());
            remove_sample(all_samples, (uint8_t *)memoryLocationForQueueElement);
        } else {
            // Store the element back at the end of the queue
            Serial.printf("Transmitting element %d: <no acknowledgment>\n", i);
        }
    }
    free(memoryLocationForQueueElement);
}

const char *get_date_time(char *date_time) {
    char *number_string = (char *)malloc(sizeof(char) * 19);
    sprintf(
        date_time, "%s%s-%s-%s %s:%s:%s", "20",
        two_digits(rtc.getYear(), number_string),
        two_digits(rtc.getMonth(), number_string + 3),
        two_digits(rtc.getDay(), number_string + 6),
        two_digits(rtc.getHours(), number_string + 9),
        two_digits(rtc.getMinutes(), number_string + 12),
        two_digits(rtc.getSeconds(), number_string + 15));
    free(number_string);
    return date_time;
}

void set_date_time(char *date_time_on_sync) {
    char date_time_temp[5];
    int date_time_temp_int;
    /* Year */
    substring(date_time_on_sync, date_time_temp, 1, 4);
    sscanf(date_time_temp, "%d", &date_time_temp_int);
    rtc.setYear(date_time_temp_int - 2000);
    /* Month */
    substring(date_time_on_sync, date_time_temp, 6, 2);
    sscanf(date_time_temp, "%d", &date_time_temp_int);
    rtc.setMonth(date_time_temp_int);
    /* Day */
    substring(date_time_on_sync, date_time_temp, 9, 2);
    sscanf(date_time_temp, "%d", &date_time_temp_int);
    rtc.setDay(date_time_temp_int);
    /* Hours */
    substring(date_time_on_sync, date_time_temp, 12, 2);
    sscanf(date_time_temp, "%d", &date_time_temp_int);
    rtc.setHours(date_time_temp_int);
    /* Minutes */
    substring(date_time_on_sync, date_time_temp, 15, 2);
    sscanf(date_time_temp, "%d", &date_time_temp_int);
    rtc.setMinutes(date_time_temp_int);
    /* Seconds */
    substring(date_time_on_sync, date_time_temp, 18, 2);
    sscanf(date_time_temp, "%d", &date_time_temp_int);
    rtc.setSeconds(date_time_temp_int);
}

void set_new_alarm() {
    switch (rtc.getAlarmSeconds()) {
        case 14:
            rtc.setAlarmSeconds(29);
            break;
        case 29:
            rtc.setAlarmSeconds(44);
            break;
        case 44:
            rtc.setAlarmSeconds(59);
            break;
        case 59:
            rtc.setAlarmSeconds(14);
            break;
    }
}
