#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <string.h>
#include <rtcZero.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_BMP280.h>
#include "Queue.h"

/* Feather M0 TFT display wiring configurations */
#define STMPE_CS                    6
#define TFT_CS                      9
#define TFT_DC                      10
#define SD_CS                       5

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
void print_setup();
void clearTFT();
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
void printSensorValues();

/* Create instance of TFT display */
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

/* Create an instance of the real time clock */
RTCZero rtc;

/* Create instance of the radio driver */
RH_RF95 rf95(RF95_CHIP_SELECT_PIN, RF95_INTERRUPT_PIN);

/* Create instance of the packet manager */
RHReliableDatagram manager(rf95);

/* Create instance of BMP280 sensor */
Adafruit_BMP280 bmp;

/* Gloabal variables */
int node_id = 0;
int celsius = 0;
int sampling_rate = 15, transmission_delay = 20;
int16_t lastRssi = -1;
char date_time_on_sync[19], sampling_rate_temp[3], transmission_delay_temp[3], temperature_reading[150];
Queue *pointerToQueue = createQueue(150, 50);
float batteryVoltage;
int batteryPercent;

void setup()
{
    /* Initialize the real time clock */
    rtc.begin();

    /* Begin TFT display */
    tft.begin();

    /* Initialize LoRa radio with defined configurations */
    initialize_node();
    
    /* Begin BMP280 sensor */
    bmp.begin(0x76);

    rtc.setAlarmSeconds(14);
    rtc.enableAlarm(rtc.MATCH_SS);
    rtc.attachInterrupt(log_sensor_readings);

    /* Sync node with gateway 
    while (node_id == 0) {
    sync_with_gateway();
    } */

    print_setup();

    delay(500);
    printSensorValues();
}

void loop()
{
    uint8_t totalSeconds = rtc.getHours() * 3600 + rtc.getMinutes() * 60 + rtc.getSeconds();

    /* Transmit payload queue as often as the transmission delay in seconds */
    if (totalSeconds % transmission_delay == 0)
    {
        transmit_payloads();
    }

    /* Delay to prevent loop from running more than once per a second */
    delay(1000);
}

void initialize_node()
{
    /* Manual reset on radio module */
    pinMode(RF95_RESET_PIN, OUTPUT);
    digitalWrite(RF95_RESET_PIN, LOW);
    delay(10);
    digitalWrite(RF95_RESET_PIN, HIGH);
    delay(10);
    void print_setup();
}

void substring(char *source_string, char *sub_string, int sub_string_start, int sub_string_length)
{
    int char_index = 0;

    while (char_index < sub_string_length)
    {
        sub_string[char_index] = source_string[sub_string_start + char_index - 1];
        char_index++;
    }
    sub_string[char_index] = '\0';
}

const char *get_date_time(char *date_time)
{
    char *number_string = (char *)malloc(sizeof(char) * 6 * 3 + 1);
    sprintf(date_time, "%s%s-%s-%s %s:%s:%s", "20", two_digits(rtc.getYear(), number_string), two_digits(rtc.getMonth(), number_string + 3), two_digits(rtc.getDay(), number_string + 6), two_digits(rtc.getHours(), number_string + 9), two_digits(rtc.getMinutes(), number_string + 12), two_digits(rtc.getSeconds(), number_string + 15));
    free(number_string);
    return date_time;
}

/*Syncs with raspberry pi*/
void sync_with_gateway()
{
    /* Request node_id and date_time from gateway */
    const uint32_t *serial_number_words[4] = {(uint32_t *)0x0080A00C, (uint32_t *)0x0080A040, (uint32_t *)0x0080A044, (uint32_t *)0x0080A048};
    char serial_number[130];
    sprintf((char *)serial_number, "%lu %lu %lu %lu", *serial_number_words[0], *serial_number_words[1], *serial_number_words[2], *serial_number_words[3]);

    Serial.print("\nSending to gateway:\t");
    Serial.println((char *)serial_number);
    if (manager.sendtoWait((uint8_t *)serial_number, sizeof(serial_number), RF95_GATEWAY_ID))
    {
        /* Receive response from gateway */
        uint8_t node_id_response[RH_RF95_MAX_MESSAGE_LEN];
        uint8_t response_length = sizeof(node_id_response);
        uint8_t gateway_id;
        if (manager.recvfromAckTimeout(node_id_response, &response_length, 2000, &gateway_id))
        {
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

            /* Update sampling_rate */
            substring((char *)node_id_response, sampling_rate_temp, 69, 2);
            sscanf(sampling_rate_temp, "%d", &sampling_rate);
            Serial.printf("\t\t\tsampling_rate set to %d seconds\n", sampling_rate);

            /* Update transmission_delay */
            substring((char *)node_id_response, transmission_delay_temp, 96, 2);
            sscanf(transmission_delay_temp, "%d", &transmission_delay);
            Serial.printf("\t\t\ttransmission_delay set to %d seconds\n\n", transmission_delay);

            /* Create interrupt to periodically log sensor_readings */
            rtc.setAlarmSeconds(14);
            rtc.enableAlarm(rtc.MATCH_SS);
            rtc.attachInterrupt(log_sensor_readings);
        }
    }
}

void printElement(void *element)
{
    Serial.printf("%s\n", (char *) element);
}

void log_sensor_readings()
{
    /* Read values from sensors */
    Serial.print("Taking sensor readings...   ");
    celsius++; 

    /* Prepare payloads */
    Serial.print("Preparing payloads...   ");
    char *date_time = (char *)malloc(sizeof(char) * 19);
    sprintf(temperature_reading, "{\"node_id\":\"%d\", \"sensor_type\":\"%c\", \"date_time\":\"%s\", \"value\":\"%d\"}", node_id, 'T', get_date_time(date_time), celsius);
    free(date_time);

    /* Log payloads */
    Serial.println("Logging payloads...");
    enqueue(pointerToQueue, temperature_reading);

    Serial.println();
    for_each(pointerToQueue, printElement);
    set_new_alarm();
}

void transmit_payloads()
{
    Serial.println("Transmitting payload queue...");
    void *memoryLocationForQueueElement = malloc(pointerToQueue->sizeOfDataElement);
    while (pointerToQueue->size > 0)
    {
        int y = tft.getCursorY();
        if (y > 200)
        {
        printSensorValues();
        }

        //Store the first element (the one being tranmitted) in the Queue and print it
        dequeue(pointerToQueue, memoryLocationForQueueElement);

        tft.setTextColor(ILI9341_YELLOW);
        tft.setTextSize(2);
        tft.printf("Sending to gateway:\n");
        tft.setTextColor(ILI9341_WHITE);
        tft.setTextSize(1);
        tft.printf("%s\n", (char *) memoryLocationForQueueElement);

        //If transmitted successfully
        if (manager.sendtoWait((uint8_t *)memoryLocationForQueueElement, pointerToQueue->sizeOfDataElement, RF95_GATEWAY_ID))
        {
            tft.setTextColor(ILI9341_WHITE);
            lastRssi = rf95.lastRssi();
            tft.printf("\nTransmission acknowledged", lastRssi);
            tft.printf("             Last RSSI: %d\n\n", rf95.lastRssi());  
        }
        else
        //Store the element back at the end of the queue
        {
            tft.setTextColor(ILI9341_RED);
            tft.printf("\nNo acknowledgment received\n\n");
            enqueue(pointerToQueue, memoryLocationForQueueElement);
        }
    }
    free(memoryLocationForQueueElement);
}

const char *two_digits(int number, char *number_string)
{
    if (number < 10)
    {
        sprintf(number_string, "0%d", number);
    }
    else
    {
        sprintf(number_string, "%d", number);
    }
    return number_string;
}

void set_date_time(char *date_time_on_sync)
{
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

void set_new_alarm()
{
    switch (rtc.getAlarmSeconds())
    {
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

void print_setup()
{  
  clearTFT();

  /* Initialize LoRa radio */
  tft.setTextColor(ILI9341_RED);
  tft.printf("Radio:       ");
  tft.setTextColor(ILI9341_YELLOW);
  while (!manager.init())
  {
    tft.printf("Failed\n");
    while (1)
      ;
  }
  tft.printf("Initialized\n");

  /* Set radio address */
  tft.setTextColor(ILI9341_RED);
  tft.printf("Address:     ");
  tft.setTextColor(ILI9341_YELLOW);
  tft.printf("%d\n", manager.thisAddress());

  /* Set frequency (ISM band) */
  tft.setTextColor(ILI9341_RED);
  tft.printf("Frequency:   ");
  rf95.setFrequency(RF95_FREQUENCY);
  tft.setTextColor(ILI9341_YELLOW);
  tft.print(RF95_FREQUENCY);
  tft.printf(" Mhz\n");

  /* Set transmission power (5 to 23 dBm) */
  tft.setTextColor(ILI9341_RED);
  tft.printf("Power:       ");
  rf95.setTxPower(RF95_TRANSMISSION_POWER, false);
  tft.setTextColor(ILI9341_YELLOW);
  tft.printf("%d dBm\n", RF95_TRANSMISSION_POWER);

  /* Set channel activity detection timeout 
  tft.printf("CAD timeout:\t\t\t");
  rf95.setCADTimeout(RF95_CAD_TIMEOUT);
  tft.printf("%d ms\n", RF95_CAD_TIMEOUT); */

  /* Set signal bandwith
  tft.printf("Bandwidth:\t\t\t");
  rf95.setSignalBandwidth(RF95_BANDWIDTH);
  tft.printf("%d Hz\n", RF95_BANDWIDTH); */

  /* Set coding rate 
  tft.printf("Coding rate:\t\t\t");
  rf95.setCodingRate4(5);
  tft.printf("%d-bit\n", RF95_CODING_RATE); */

  /* Set spreading factor
  tft.printf("Spreading factor:\t\t");
  rf95.setSpreadingFactor(RF95_SPREADING_FACTOR);
  tft.printf("SF%d\n", RF95_SPREADING_FACTOR); */

  /* Set preamble length 
  tft.printf("Preamble length:\t\t");
  rf95.setSignalBandwidth(RF95_PREAMBLE_LENGTH);
  tft.printf("%x\n", RF95_PREAMBLE_LENGTH); */

  /* Set sync word 
  tft.printf("Sync word:\t\t\t");
  rf95.spiWrite(RH_RF95_REG_39_SYNC_WORD, RF95_SYNC_WORD);
  tft.printf("%x\n", RF95_SYNC_WORD); */

  /* Set modem configuration */
  if (!rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128))
  {
    tft.print("Invalid setModemConfig() option\n");
  } else {
  
  tft.setTextColor(ILI9341_RED);
  tft.printf("Bandwidth:   ");
  tft.setTextColor(ILI9341_YELLOW);
  tft.printf("%d Hz\n", 125000);

  tft.setTextColor(ILI9341_RED);
  tft.printf("Coding rate: ");
  rf95.setCodingRate4(5);
  tft.setTextColor(ILI9341_YELLOW);
  tft.printf("%s\n", "4/5");

  tft.setTextColor(ILI9341_RED);
  tft.printf("Spreading:   ");
  tft.setTextColor(ILI9341_YELLOW);
  tft.printf("%d c/s\n", 128);
  }
  
  /* Set amount of transmit retries */
  tft.setTextColor(ILI9341_RED);
  tft.printf("Retries:     ");
  manager.setRetries(RF95_RESTRANSMIT_RETRIES);
  tft.setTextColor(ILI9341_YELLOW);
  tft.printf("%d\n", RF95_RESTRANSMIT_RETRIES);

  /* Set minimum re-transmit timeout */
  tft.setTextColor(ILI9341_RED);
  tft.printf("Retry time:  ");
  manager.setTimeout(RF95_RESTRANSMIT_TIMEOUT);
  tft.setTextColor(ILI9341_YELLOW);
  tft.printf("%d ms\n", RF95_RESTRANSMIT_TIMEOUT);
}

void clearTFT()
{
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.printf("Feather M0 LoRa Node   ");
  tft.setTextColor(ILI9341_YELLOW);
  //batteryVoltage = analogRead(A7);
  //batteryVoltage *= 2;    // Adafruit divided by 2, so multiply back
  //batteryVoltage *= 3.3;  // Multiply by 3.3V, the reference voltage
  //batteryVoltage /= 1024; // Convert to voltage
  //float batteryPercent = constrain(map(batteryVoltage * 100, 310, 420, 0, 100), 0 , 99);
  tft.printf("%d%s\n", 89, "%");
  tft.setTextColor(ILI9341_WHITE);
  tft.printf("--------------------------\n");
}

void printSensorValues() {
    tft.setRotation(1);
    tft.fillScreen(ILI9341_BLACK);
    tft.drawLine(0, 60, 320, 60, ILI9341_GREEN);
    tft.drawLine(80, 0, 80, 60, ILI9341_GREEN);
    tft.drawLine(200, 0, 200, 60, ILI9341_GREEN);
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_GREEN);
    tft.print(" Temp   ");
    tft.print("Pressure  ");
    tft.println("Altitude\n");
    tft.setTextColor(ILI9341_WHITE);
    tft.print(" ");
    tft.print((int) bmp.readTemperature());
    tft.print((char)247);
    tft.print("C   ");
    tft.print((int) bmp.readPressure());
    tft.print(" Pa  ");
    tft.print((int) bmp.readAltitude(1013.25)); /* Adjusted to local forecast! */
    tft.print(" m\n\n\n");
}