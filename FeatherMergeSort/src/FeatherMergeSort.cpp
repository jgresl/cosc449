#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Feather M0 TFT display wiring configurations
#define STMPE_CS 6
#define TFT_CS 9
#define TFT_DC 10
#define SD_CS 5

// Create instance of TFT display
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

void setup() {
  // Begin TFT display
  tft.begin();

  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(1);
  tft.setTextSize(2);
  
  // Horizontal 
  tft.drawLine(10, 10, 50, 10, ILI9341_GREEN);
  tft.drawLine(10, 30, 50, 30, ILI9341_GREEN);
  tft.drawLine(10, 50, 50, 50, ILI9341_GREEN);
  tft.drawLine(10, 70, 50, 70, ILI9341_GREEN);
  tft.drawLine(10, 90, 50, 90, ILI9341_GREEN);
  tft.drawLine(10, 110, 50, 110, ILI9341_GREEN);
  tft.drawLine(10, 130, 50, 110, ILI9341_GREEN);
  tft.drawLine(10, 150, 50, 110, ILI9341_GREEN);
  tft.drawLine(10, 170, 50, 110, ILI9341_GREEN);
  tft.drawLine(10, 190, 50, 110, ILI9341_GREEN);
  tft.drawLine(10, 210, 50, 110, ILI9341_GREEN);
  tft.drawLine(10, 230, 50, 230, ILI9341_GREEN);

  
  // Vertical lines
  tft.drawLine(10, 10, 10, 230, ILI9341_GREEN);
  tft.drawLine(50, 10, 50, 230, ILI9341_GREEN);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  
}