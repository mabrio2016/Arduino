// 10 Feb 2024
// Arduino Nano -- Processor -> Atmeha328P (OldBootloader) 
//This version uses a servor in the Rotor, instead of a step motor.

#include <AccelStepper.h>
#include <Servo.h>

  
#define HALFSTEP 8
#define ANALOG_IN_Tunner A0
#define ANALOG_IN_Rotor A1

//const byte interruptPin = 3;

int Tunner_Position = 0;
float average_tunner = 0;
int analog_Tunner = 0;
bool flag_Tunner = 0;

long StartTime = 0;
int Rotor_Position = 0;
int last_Rotor_Position = 0;
float average_rotor = 0;
int Analog_Reading = 0;

// NOTE: The sequence 1-3-2-4 is required for proper sequencing of 28BYJ-48 
AccelStepper stepper_tunner(HALFSTEP, 11, 9, 10, 8); // (HALFSTEP, 8, 10, 9, 11) - Inverted the tuner movement
Servo myservo;  // create servo object to control a servo

//int endPoint = -4000;                 // Move this many steps; approx 90 degrees angle turn  
//volatile boolean stopNow = false;    // flag for Interrupt Service routine  
//boolean homed = false;               // flag to indicate when motor is homed  
  
void setup()  
{  
    pinMode(7, OUTPUT);                       // Configure the digital pin 7 to enable or disble the Step Mottor Driver and reduce the current drain
    digitalWrite(7, HIGH);                    // Step Mottor Driver enabled
    //pinMode(interruptPin, INPUT_PULLUP);
    //attachInterrupt(digitalPinToInterrupt(interruptPin), intService, CHANGE);
    //attachInterrupt(0, intService, CHANGE);  // Enable interrupt 0 (Pin2), switch pulled low  
    Serial.begin(57600);    
  
    stepper_tunner.setMaxSpeed(1500);
    stepper_tunner.setAcceleration(800);  
    stepper_tunner.setSpeed(500);
    Tunner_Position = analogRead (ANALOG_IN_Tunner);
    
    StartTime = millis(); //start the timer
    //Analog_Reading = 0;
    //myservo.write(0);
}
  
void loop()
{ 
  Tunner_Position = analogRead (ANALOG_IN_Tunner);
  average_tunner += 0.05 * (Tunner_Position - average_tunner); // one pole digital filter, about 20Hz cutoff
  analog_Tunner = (average_tunner);
  flag_Tunner = Move_tunner(analog_Tunner);
  Serial.println(flag_Tunner); 
  if (flag_Tunner != true){
    Rotor_Position = analogRead (ANALOG_IN_Rotor);
    int average = 0;
    for (int i=0; i < 20; i++) {
      average = average + analogRead(ANALOG_IN_Rotor);
    }
    average = average/10;
    Analog_Reading += 0.12 * (Rotor_Position - Analog_Reading); // This Smoothg out the servo moviment
    Rotor_Position = map(Analog_Reading, 0, 1023 , 0, 190);
    if (last_Rotor_Position != Rotor_Position){
      //Serial.print("Rotor Position  = ");
      //Serial.println(Rotor_Position);
      digitalWrite(7, HIGH);
      StartTime = millis();
      myservo.write(Rotor_Position); 
      myservo.attach(2);  // attaches the servo on pin D2 to the servo object
      last_Rotor_Position = Rotor_Position;
      delay(20);
    }
  }

  if((millis()-StartTime) >= 3000) { //The larger this number the better. Multiple serial.println interferes with the stepper motor
    digitalWrite(7, LOW);
    //Serial.println(myservo.read());
    StartTime = millis();
  }  
}

bool Move_tunner(int TunnerPosition){
  stepper_tunner.setSpeed(600);
  stepper_tunner.moveTo(TunnerPosition * 2);
  stepper_tunner.runSpeedToPosition();
  return (stepper_tunner.distanceToGo());
}

//  
// Interrupt service routine for INT 0  
//  
//void intService()  //Interrupt Services
//{  
  //stopNow = true; // Set flag to show Interrupt recognised and then stop the motor
  //if(homed)
  //{
     //stepper_rotor.stop();  
     //stepper_rotor.setCurrentPosition(0);
     //stepper_rotor.move(-200);
//     Serial.print("!!!Interrupted!!!");
//     flag_Limit = 1;
  //}   
//}


//----------------------




// #include <Servo.h>

// Servo myservo;  // create servo object to control a servo

// int ANALOG_IN_Rotor = A1;  // analog pin used to connect the potentiometer
// int Read_Val = 0;    // variable to read the value from the analog pin
// int val = 0;    // variable to read the value from the analog pin

// void setup() {
//   Serial.begin(57600); 
//   myservo.attach(2);  // attaches the servo on pin 9 to the servo object
//   pinMode(7, OUTPUT);                       // Configure the digital pin 7 to enable or disble the Step Mottor Driver and reduce the current drain
//   digitalWrite(7, HIGH); 
// }

// void loop() {
//   Read_Val = analogRead(ANALOG_IN_Rotor);            // reads the value of the potentiometer (value between 0 and 1023)
//   val = map(Read_Val, 0, 1023, 0, 180);     // scale it to use it with the servo (value between 0 and 180)
//   //Serial.print("Rotor Position  = ");
//   //Serial.println(Read_Val);
//   myservo.write(val);                  // sets the servo position according to the scaled value
//   //myservo.read();
//   delay(15);                           // waits for the servo to get there
// }
