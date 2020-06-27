#include <pigpio.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
// #include <json-c/json.h>
// #include <mysql.h>

//Function Definitions
void sig_handler(int sig);

/* Raspberry Pi wiring configurations */
#define RF95_CHIP_SELECT_PIN 8
#define RF95_INTERRUPT_PIN 25
#define RF95_LED 4

/* LoRa radio configurations */
#define RF95_FREQUENCY 915.0          // Between 137.0 and 1020.0   (Default = 915 Mhz)
#define RF95_TRANSMISSION_POWER 20    // Between 5 and 23           (Default = 13 Db)
#define RF95_CAD_TIMEOUT 10000        // Greater or equal to 0      (Default = 0 ms)
#define RF95_SPREADING_FACTOR 12      // Between 6 and 12           (Default = ?)             --> Overwritten by setMOdemConfig()
#define RF95_BANDWIDTH 125000         // Between 7800 and 500000    (Default = 125000 Hz)     --> Overwritten by setMOdemConfig()
#define RF95_PREAMBLE_LENGTH 0x39     //                            (Default = ?)
#define RF95_SYNC_WORD 0x39           //                            (Default = 0x39)
#define RF95_CODING_RATE 5            // Between 5 and 8            (Default = 5-bit)         --> Overwritten by setMOdemConfig()
#define RF95_RETRANSMIT_TIMEOUT 2000  //                            (Default = 200ms)
#define RF95_GATEWAY_ID 100           // Assign unique ID to gateway

/* Create instance of the radio driver */
RH_RF95 rf95(RF95_CHIP_SELECT_PIN, RF95_INTERRUPT_PIN);

/* Create instance of the packet manager */
RHReliableDatagram manager(rf95, RF95_GATEWAY_ID);

//Flag for Ctrl-C
int flag = 0;

//Main Function
int main (int argc, const char* argv[] )
{
  if (gpioInitialise()<0)
  {
    printf( "\n\nRPI rf95_reliable_datagram_server startup Failed.\n" );
    return 1;
  }

  gpioSetSignalFunc(2, sig_handler); //2 is SIGINT. Ctrl+C will cause signal.

  printf( "\nRPI rf95_reliable_datagram_server startup OK.\n" );
  printf( "\nRPI GPIO settings:\n" );
  printf("CS-> GPIO %d\n", (uint8_t) RFM95_CS_PIN);
  printf("IRQ-> GPIO %d\n", (uint8_t) RFM95_IRQ_PIN);
#ifdef RFM95_LED
  gpioSetMode(RFM95_LED, PI_OUTPUT);
  printf("\nINFO: LED on GPIO %d\n", (uint8_t) RFM95_LED);
  gpioWrite(RFM95_LED, PI_ON);
  gpioDelay(500000);
  gpioWrite(RFM95_LED, PI_OFF);
#endif

  if (!rf95.init())
  {
    printf( "\n\nRF95 Driver Failed to initialize.\n\n" );
    return 1;
  }

  /* Begin Manager/Driver settings code */
  printf("\nRFM 95 Settings:\n");
  printf("Frequency= %d MHz\n", (uint16_t) RFM95_FREQUENCY);
  printf("Power= %d\n", (uint8_t) RFM95_TXPOWER);
  printf("Client Address= %d\n", CLIENT_ADDRESS);
  printf("Server(This) Address= %d\n", SERVER_ADDRESS);
  rf95.setTxPower(RFM95_TXPOWER, false);
  rf95.setFrequency(RFM95_FREQUENCY);
  rf95.setThisAddress(SERVER_ADDRESS);
  rf95.setHeaderFrom(SERVER_ADDRESS);
  /* End Manager/Driver settings code */

  /* Begin Reliable Datagram Server Code */
  uint8_t data[] = "And hello back to you";
  // Dont put this on the stack:
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

  while(!flag)
  {
    if (manager.available())
    {
      // Wait for a message addressed to us from the client
      uint8_t len = sizeof(buf);
      uint8_t from;
      if (manager.recvfromAck(buf, &len, &from))
      {
#ifdef RFM95_LED
        gpioWrite(RFM95_LED, PI_ON);
#endif
        Serial.print("got request from : 0x");
        Serial.print(from, HEX);
        Serial.print(": ");
        Serial.println((char*)buf);

        // Send a reply back to the originator client
        if (!manager.sendtoWait(data, sizeof(data), from))
          Serial.println("sendtoWait failed");
#ifdef RFM95_LED
        gpioWrite(RFM95_LED, PI_OFF);
#endif
      }
    }
  }
  printf( "\nrf95_reliable_datagram_server Tester Ending\n" );
  gpioTerminate();
  return 0;
}

void sig_handler(int sig)
{
  flag=1;
}

