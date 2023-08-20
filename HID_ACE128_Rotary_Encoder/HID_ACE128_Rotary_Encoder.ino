
#define ACE_ADDR 0x20 //I2C address
#include <Mouse.h>

#include <ACE128.h>  // Include the ACE128.h from the library folder
#include <ACE128map87654321.h> // mapping for pin order 87654321

ACE128 myACE(ACE_ADDR, (uint8_t*)encoderMap_87654321); // I2C without using EEPROM
uint8_t oldPos = 255;
uint8_t upos;
int8_t pos;
int16_t mpos;

int invertMouse = 1;        //Invert mouse orientation

int vertValue, horzValue;  // Stores current analog output of each axis
const int sensitivity = 1;  // Higher sensitivity value = slower mouse, should be <= about 500
int mouseClickFlag = 0;

void setup() {
  Serial.begin(9600);
  myACE.begin();    // this is required for each instance, initializes the pins
  Mouse.begin(); //Init mouse emulation
}


void loop() {
  pos = myACE.pos();                 // get logical position - signed -64 to +63
  upos = myACE.upos();               // get logical position - unsigned 0 to +127
  mpos = myACE.mpos();               // get multiturn position - signed -32768 to +32767

  if (upos != oldPos) {            // did we move?
    if (upos < oldPos ){
      invertMouse = -1;
    } 
    else{
      invertMouse = 1;
    }
    oldPos = upos;                 // remember where we are
    //Serial.println(pos, DEC);
    //Serial.println(upos, DEC);
    Serial.println(mpos, DEC);    
    Mouse.move(0, (invertMouse *(mpos / sensitivity)), 0); // move mouse on y axis
  }
}
