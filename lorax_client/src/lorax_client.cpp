#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <rtcZero.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Queue.h"

/* Feather M0 wiring configurations */
#define RF95_CHIP_SELECT_PIN        8
#define RF95_INTERRUPT_PIN          3
#define RF95_RESET_PIN              4
#define VBATPIN                     A7        // Voltage pin to determine battery life

/* LoRa radio configurations */
#define RF95_FREQUENCY              915.0     // Between 137.0 and 1020.0   (Default = 915 Mhz)
#define RF95_TRANSMISSION_POWER     20        // Between 5 and 23           (Default = 13 Db)
#define RF95_CAD_TIMEOUT            10000     // Greater or equal to 0      (Default = 0 ms)
#define RF95_SPREADING_FACTOR       12        // Between 6 and 12           (Default = ?)             --> Overwritten by setModemConfig()
#define RF95_BANDWIDTH              125000    // Between 7800 and 500000    (Default = 125000 Hz)     --> Overwritten by setModemConfig()
#define RF95_PREAMBLE_LENGTH        8         //                            (Default = 8)
#define RF95_SYNC_WORD              0x39      //                            (Default = 0x39)
#define RF95_CODING_RATE            5         // Between 5 and 8            (Default = 5-bit)         --> Overwritten by setModemConfig()
#define RF95_RESTRANSMIT_RETRIES    3         //                            (Default = 3)
#define RF95_RESTRANSMIT_TIMEOUT    500       //                            (Default = 200 ms)
#define RF95_GATEWAY_ID             100       // Assign unique ID to gateway

/* Declare functions */
void initialize_node();
void clearSerial();
void sync_with_gateway();
void log_sensor_readings();
void print_payload_queue();
void log_payload(char *);
void transmit_payloads();
void substring(char *, char *, int, int);
void set_date_time(char *);
const char *get_date_time(char *);
const char *two_digits(int, char *);
void set_new_alarm();

/* Create an instance of the real time clock */
RTCZero rtc;

/* Create instance of the radio driver */
RH_RF95 rf95(RF95_CHIP_SELECT_PIN, RF95_INTERRUPT_PIN);

/* Create instance of the packet manager */
RHReliableDatagram manager(rf95);

/* Create instance of BME280 sensor */
Adafruit_BME280 bme;

/* Gloabal variables */
int node_id = 0;
int celsius = 0;
int16_t lastRssi = -1;
char date_time_on_sync[19], sampling_rate_temp[3], transmission_delay_temp[3], temperature_reading[150];
Queue *pointerToQueue = createQueue(150, 50);
float batteryVoltage;
int batteryPercent;

void setup() {
    /* Initialize the real time clock */
    rtc.begin();

    /* Initialize LoRa radio with defined configurations */
    initialize_node();
    
    /* Begin BME280 sensor */
    bme.begin(0x76);

    /* Setup interrupt for taking samples */
    rtc.setAlarmSeconds(14);
    rtc.enableAlarm(rtc.MATCH_SS);
    rtc.attachInterrupt(log_sensor_readings);

    /* Sync node with gateway */
    while (node_id == 0) {
        sync_with_gateway();
    }

    delay(500);
}

void loop() {
    /* Get total seconds since device startup */
    uint8_t totalSeconds = rtc.getHours() * 3600 + rtc.getMinutes() * 60 + rtc.getSeconds();

    /* Transmit payload queue every 20 seconds */
    if (node_id > 0 && totalSeconds % 20 == 0) {
        transmit_payloads();
    }

    /* Delay to prevent loop from running more than once per a second */
    delay(1000);
}

void initialize_node() {
    /* Manual reset on radio module */
    pinMode(RF95_RESET_PIN, OUTPUT);
    digitalWrite(RF95_RESET_PIN, LOW);
    delay(10);
    digitalWrite(RF95_RESET_PIN, HIGH);
    delay(10);

    /* Initialize LoRa radio */
    Serial.printf("Radio:       ");
    while (!manager.init()) {
        Serial.printf("Failed\n");
        while (1);
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

void substring(char *source_string, char *sub_string, int sub_string_start, int sub_string_length) {
    int char_index = 0;

    while (char_index < sub_string_length) {
        sub_string[char_index] = source_string[sub_string_start + char_index - 1];
        char_index++;
    }
    sub_string[char_index] = '\0';
}

const char *get_date_time(char *date_time) {
    char *number_string = (char *)malloc(sizeof(char) * 19);
    sprintf(date_time, "%s%s-%s-%s %s:%s:%s", "20", two_digits(rtc.getYear(), number_string), two_digits(rtc.getMonth(), number_string + 3), two_digits(rtc.getDay(), number_string + 6), two_digits(rtc.getHours(), number_string + 9), two_digits(rtc.getMinutes(), number_string + 12), two_digits(rtc.getSeconds(), number_string + 15));
    free(number_string);
    return date_time;
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

            /* Create interrupt to periodically log sensor_readings */
            rtc.setAlarmSeconds(14);
            rtc.enableAlarm(rtc.MATCH_SS);
            rtc.attachInterrupt(log_sensor_readings);
        }
    }
}

void printElement(void *element) {
    Serial.printf("%s\n", (char *) element);
}

void log_sensor_readings() {
    /* Read values from sensors */
    Serial.print("\nTaking sensor readings...");
    celsius = (int) bme.readTemperature();
    
    /* Prepare payloads */
    Serial.print("\nPreparing payloads...");
    char *date_time = (char *)malloc(sizeof(char) * 19);
    sprintf(temperature_reading, "{\"node_id\":\"%d\", \"sensor_type\":\"%c\", \"date_time\":\"%s\", \"value\":\"%d\"}", node_id, 'T', get_date_time(date_time), celsius);
    free(date_time);

    /* Log payloads */
    Serial.println("\nLogging payloads...");
    enqueue(pointerToQueue, temperature_reading);

    set_new_alarm();
}

void transmit_payloads()
{
    Serial.println("Transmitting payload queue...");
    void *memoryLocationForQueueElement = malloc(pointerToQueue->sizeOfDataElement);
    while (pointerToQueue->size > 0)
    {
        //Store the first element (the one being tranmitted) in the Queue and print it
        dequeue(pointerToQueue, memoryLocationForQueueElement);
        //If transmitted successfully
        if (manager.sendtoWait((uint8_t *)memoryLocationForQueueElement, pointerToQueue->sizeOfDataElement, RF95_GATEWAY_ID))
        {
            lastRssi = rf95.lastRssi();
            Serial.printf("\nTransmission acknowledged", lastRssi);
            Serial.printf("             Last RSSI: %d\n\n", rf95.lastRssi());  
        }
        else
        //Store the element back at the end of the queue
        {
            enqueue(pointerToQueue, memoryLocationForQueueElement);
        }
    }
    free(memoryLocationForQueueElement);
}

const char *two_digits(int number, char *number_string) {
    if (number < 10) {
        sprintf(number_string, "0%d", number);
    } else {
        sprintf(number_string, "%d", number);
    }
    return number_string;
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