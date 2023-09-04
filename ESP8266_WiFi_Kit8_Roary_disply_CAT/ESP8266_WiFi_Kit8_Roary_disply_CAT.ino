// 28-Aug-2023 | Using the Rotary Encoder E38S6_600 
// addred a 20 homs resiator from 5V to reset pin, to avoid atou reset the Arduino when The CAT is enabled (DTR  -> low)
// need to use the Heltec ESP8266 Series Dev-boards - > Wifi Kit 8 to be able to run the heltec.h to disply the data correctly in the 0.91″ OLED display. (https://www.arduinoecia.com.br/como-usar-o-modulo-esp8266-com-display-oled-embutido/)

#include <Arduino.h>
#include <ESP8266WiFi.h>
//#include <U8g2lib.h>
#include <heltec.h>
#include <EEPROM.h>

//U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(16);
 
//ICACHE_RAM_ATTR void updateFrequency();
//ICACHE_RAM_ATTR void askForFrequency();
ICACHE_RAM_ATTR void ai0(); //Encoder Pin A //interupt rotine
ICACHE_RAM_ATTR void ai1(); //Encoder Pin B //interupt rotine
ICACHE_RAM_ATTR void Ask_Switch_Check(); // Ask frequence switch interupt rotine
 
const int P1 = 12; // D6 - GPIO12 -> Rotary Encoder A
const int P2 = 13; // D7 - GPIO13 -> Rotary Encoder B
const int Ask_Switch = 2; // GPIO02 -> Ask frequence switch
const int ADC_pin = A0; // Squelch  potentiometer
const int BAUDRATE = 57600;
const int DEFAULT_FREQ = 7100000;
const int DEFAULT_VFO_STEP = 1000;
const int ROTARY_STEP = 100;
const int SERIAL_TIMEOUT = 2000;

volatile int Analog_Reading = 0;
volatile int Filtered_Analog_Reading = 0;
volatile float average_Analog_Reading = 0;

volatile int frequency = DEFAULT_FREQ;
volatile int TempAnalog_Value, Analog_Value = 0;  //These variable will hold the Analog Squelch values;
volatile long TempCounter, counter, readFrequency = 0; //These variable will increase or decrease depending on the rotation of encoder
volatile bool asked = false;
volatile bool LockEncoder = false;

long lastUpdateMillis = 0;
char RotaryNumber[16];
char Received_Freq[16];
char *Hz = " Hz";
char Freq_Hz[15];

void initWifi();
void initComm();
void initGpio();
void sendSquech(int);
void Ask_Switch_Check();
void askForFrequency();
void serialTxFlush(String);
void ai0();
void Display_Encoder();
void sendFrequency();
void Display_Recived();
void Format_frequency();

void setup() 
{
  initWifi();
  initComm();
  initGpio();
  //u8x8.begin();
  Heltec.begin(true, false);
  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_24);
  Heltec.display->drawString(0,5,"! Connected");
  Heltec.display->display(); //Mostra as informacoes no display
}
void initWifi()
{
  WiFi.softAPdisconnect(true); // Disable default Wifi Access Point (something like 'FaryLink_xxxxxx')
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
}
void initComm()
{
  Serial.setDebugOutput(false);
  Serial.setTimeout(SERIAL_TIMEOUT);
  Serial.clearWriteError();
  Serial.begin(BAUDRATE, SERIAL_8N1);
  while (!Serial)
  {
    yield();
    delay(10); // wait for serial port to connect. Needed for native USB port only
  }
}
void initGpio()
{
  digitalWrite(P1, HIGH); //turn pullup resistor on
  digitalWrite(P2, HIGH); //turn pullup resistor on
  pinMode(P1, INPUT_PULLUP);
  pinMode(P2, INPUT_PULLUP);
  pinMode(Ask_Switch, INPUT_PULLUP);
  attachInterrupt(P1, ai0, RISING);   //A rising pulse from encoder activate ai0(). AttachInterrupt 0 is DigitalPin nr 12.
  //attachInterrupt(P2, ai1, RISING); // (Not in use, to reduce number of counts - "Resolution") B rising pulse from encoder activate ai1(). AttachInterrupt 1 is DigitalPin nr 13.
  attachInterrupt(digitalPinToInterrupt(Ask_Switch), Ask_Switch_Check, FALLING);
}

void loop() 
{
  if (readFrequency == 0){
    asked = true;
    LockEncoder = true;
    askForFrequency();
  }
  if (!asked){
    Analog_Reading = analogRead(ADC_pin);
    average_Analog_Reading += 0.001 * (Analog_Reading - average_Analog_Reading); // one pole digital filter, about 20Hz cutoff
    Filtered_Analog_Reading = (average_Analog_Reading - 200) / 5 * 1.2; 
    if (Filtered_Analog_Reading != TempAnalog_Value){
      sendSquech(Filtered_Analog_Reading);
      //Serial.println(Filtered_Analog_Reading);
      TempAnalog_Value = Filtered_Analog_Reading;
    }
  } 
  else if (asked) {  
    askForFrequency(); 
  } 
  if( counter != TempCounter ){
    //Serial.println (counter);
    Display_Encoder();
    sendFrequency();
    TempCounter = counter;
  }    
}
void sendSquech(int value){
  Analog_Value = map(value, -20, 170, 0, 255);
  Display_Squelch();
  char result[3]; 
  sprintf(result, "%03d", Analog_Value);
  serialTxFlush("SQ0" + String(result) + ";;");
}

void Ask_Switch_Check() {
  asked = true;
}

void askForFrequency(){
  Serial.flush(); // wait until TX buffer is empty
  delay(20);
  Serial.println("FA;");
  delay(20);
  asked = true;
  LockEncoder = true;
  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_24);
  Heltec.display->drawString(0,5,"! Connected");
  Heltec.display->display(); //Mostra as informacoes no display
  if(Serial.available()){
    String rxresponse = Serial.readStringUntil(';');
    if (rxresponse.startsWith("FA")){
      readFrequency = rxresponse.substring(2, 13).toInt();
      //EEPROM.write(0, readFrequency);
      Display_Recived();
      counter = 0,
      asked = false;
      LockEncoder = false;
    }
  }
}

void serialTxFlush(String command)
{
  Serial.flush(); // wait until TX buffer is empty
  delay(5);
  Serial.println(command);
  delay(5);
}

void ai0() {
  if (!LockEncoder){
      if(digitalRead(13)==LOW){
        counter++;
      }
      else{
        counter--;
      }
  }
}

void Display_Encoder()
{
  itoa(counter, RotaryNumber, 10);
  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
  Heltec.display->drawString(128,0,Freq_Hz);
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->drawString(0,17,"Encoder:");
  Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
  Heltec.display->drawString(128,17, String(counter));
  Heltec.display->display(); //Mostra as informacoes no display
}

void Display_Squelch(){
  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
  Heltec.display->drawString(128,0,Freq_Hz);
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->drawString(0,17,"Squelch:");
  Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
  Heltec.display->drawString(128,17, String(Filtered_Analog_Reading));
  Heltec.display->display(); //Mostra as informacoes no display

}

void sendFrequency(){
  volatile long temp = readFrequency + (counter * 10);
  if (temp < 1000){
    readFrequency = 0;
    temp = 1000;
    counter = 0;
  }  
  char result[11]; 
  sprintf(result, "%011d", temp);
  serialTxFlush("FA" + String(result) + ";;");
  delay(20);
}

void Display_Recived()
{
  itoa(readFrequency, Received_Freq, 10);
  Heltec.display->clear();
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
  strcpy(Freq_Hz,ltos(readFrequency, Received_Freq, 10));
  strcat(Freq_Hz,Hz);
  Heltec.display->drawString(128,0,Freq_Hz);
  Heltec.display->display(); //Mostra as informacoes no display
}

char *ultos_recursive(unsigned long val, char *s, unsigned radix, int pos)
{
  int c;
  if (val >= radix)
    s = ultos_recursive(val / radix, s, radix, pos+1);
  c = val % radix;
  c += (c < 10 ? '0' : 'a' - 10);
  *s++ = c;
  if (pos % 3 == 0) *s++ = '.';
  return s;
}
char *ltos(long val, char *s, int radix)
{
  if (radix < 2 || radix > 36) {
    s[0] = 0;
  } else {
    char *p = s;
    if (radix == 10 && val < 0) {
      val = -val;
      *p++ = '-';
    }
    p = ultos_recursive(val, p, radix, 0) - 1;
    *p = 0;
  }
  return s;
}
