#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

/* Feather M0 TFT display wiring configurations */
#define STMPE_CS                    6
#define TFT_CS                      9
#define TFT_DC                      10
#define SD_CS                       5

/* Feather M0 wiring configurations */
#define RF95_CHIP_SELECT_PIN        8
#define RF95_INTERRUPT_PIN          3
#define RF95_RESET_PIN              4

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
void initialize_gateway();
void print_setup();
void clearTFT();
void recieve_data();

/* Create instance of TFT display */
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

/* Create instance of the radio driver */
RH_RF95 rf95(RF95_CHIP_SELECT_PIN, RF95_INTERRUPT_PIN);

/* Create instance of the packet manager */
RHReliableDatagram manager(rf95);

/* Gloabal variables */
int node_id;

void setup()
{
  /* Begin TFT display */
  tft.begin();

  /* Initialize LoRa radio with defined configurations */
  initialize_gateway();

  delay(5000);
  clearTFT();
}

void loop()
{
  recieve_data();
  /* Delay to prevent loop from running more than once per a second */
  delay(1000);
}

void initialize_gateway()
{
  /* Manual reset on radio module */
  pinMode(RF95_RESET_PIN, OUTPUT);
  digitalWrite(RF95_RESET_PIN, LOW);
  delay(10);
  digitalWrite(RF95_RESET_PIN, HIGH);
  delay(10);
  print_setup();
}

void recieve_data()
{
  uint8_t node_id, incoming_message[150];
  uint8_t incoming_message_length = sizeof(incoming_message);

  if (manager.available()) {
    /* Wait for a message addressed to us from a node*/
    if (manager.recvfromAck(incoming_message, &incoming_message_length, &node_id)) {
      int y = tft.getCursorY();
      if (y > 200)
      {
        clearTFT();
      }   
      tft.setTextColor(ILI9341_YELLOW);
      tft.setTextSize(2);
      tft.printf("Received from node %d:\n", node_id);
      tft.setTextColor(ILI9341_GREEN);
      tft.setTextSize(1);
      tft.print((char*) incoming_message);
      tft.setTextColor(ILI9341_WHITE);
      tft.printf("        Last RSSI: %d\n\n", rf95.lastRssi());  
    }
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
  manager.setThisAddress(100);
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

void clearTFT() {
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  tft.printf("Feather M0 LoRa Gateway\n");
  tft.printf("--------------------------\n");
}