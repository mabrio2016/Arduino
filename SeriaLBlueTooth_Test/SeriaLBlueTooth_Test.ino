//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"
#include <TFT_eSPI.h>
#include <SPI.h>

//#define USE_PIN // Uncomment this to use PIN during pairing. The pin is specified on the line below
const char *pin = "1234"; // Change this to more secure PIN.

String device_name = "ESP32-BT-Slave";
// Coordinates of the cursor
int x=0;
int y=0;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;
TFT_eSPI tft = TFT_eSPI(135, 240);

void setup() {
  Serial.begin(115200);
  SerialBT.begin(device_name); //Bluetooth device name
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
  //Serial.printf("The device with name \"%s\" and MAC address %s is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str(), SerialBT.getMacString()); // Use this after the MAC method is implemented
  #ifdef USE_PIN
    SerialBT.setPin(pin);
    Serial.println("Using PIN");
  #endif
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(0, 0);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(3);
    tft.drawString("Test", tft.width() / 2, tft.height() / 2 - 12);
    delay(1000);
    tftClearTerminal();
    tft.setTextSize(1);
}

void loop() {
  if (Serial.available()) {
      SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
      //Serial.write(SerialBT.read());
      char c = SerialBT.read();
      tftPutchar(c);
      //tft.print((char)SerialBT.read());
      Serial.write(c);
  }
  delay(20);
}

void tftPutchar(char c)
{
  // Create an string for displaying the char
  char string[2];
  string[0]=c;
  string[1]=0; 

  // New line ?
  if (c=='\n')
  {
    x=0;
    increase_Y();
  }
  else
  {
    // Display char
    tft.drawString(string, 5 + x*8, 125-y*10, 2);
    increase_X();
  }  
}
void increase_Y()
{
  // Increase Y and check if last line
  if (++y>17)
    {
      // Clear terminal (new page)
      tftClearTerminal();
      x=0;
      y=0;      
    }
}

// Increase Y (next char)
// If cursor is a end of line, 
// jump to a new line
void increase_X()
{
  // Increase X and check if end of line
  if (++x>29) 
  {
    // New line
    x=0;
    //y = y + 1;
    increase_Y();
  }
}
void tftClearTerminal()
{    
  // Display a black rectangle in the terminal body 
  tft.fillScreen(TFT_BLACK);
}
