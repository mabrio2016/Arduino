#include <AccelStepper.h>  
  
#define HALFSTEP 8
#define ANALOG_IN_Tunner A0
#define ANALOG_IN_Rotor A1

const byte interruptPin = 2;

int Tunner_Position = 0;
float average_tunner = 0;
int analog_Tunner = 0;
boolean flag_Tunner = 0;

long StartTime = 0;
int Rotor_Position = 0;
int latest_Rotor_Position = 0;
int latest_Rotor = 0;
float average_rotor = 0;
int analog_rotor = 0;
bool Rotor_isRunning = 0;
boolean flag_Home = 0;
boolean flag_Limit = 0;
boolean Enable_state;
int counter;

// NOTE: The sequence 1-3-2-4 is required for proper sequencing of 28BYJ-48 
AccelStepper stepper_tunner(HALFSTEP, 8, 10, 9, 11); 
AccelStepper stepper_rotor(HALFSTEP, 3, 5, 4, 6);

int endPoint = -4000;                 // Move this many steps; approx 90 degrees angle turn  
volatile boolean stopNow = false;    // flag for Interrupt Service routine  
boolean homed = false;               // flag to indicate when motor is homed  
  
void setup()  
{  
    pinMode(7, OUTPUT);                       // Configure the digital pin 7 to enable or disble the Step Mottor Driver and reduce the current drain
    digitalWrite(7, HIGH);                    // Step Mottor Driver enabled
    pinMode(interruptPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interruptPin), intService, CHANGE);
    attachInterrupt(0, intService, CHANGE);  // Enable interrupt 0 (Pin2), switch pulled low  
    Serial.begin(38400);    
  
    stepper_rotor.setMaxSpeed(1500);   
    stepper_rotor.setAcceleration(800);  
    stepper_rotor.setSpeed(700);  
    stepper_rotor.moveTo(endPoint);

    stepper_tunner.setMaxSpeed(1500);
    stepper_tunner.setAcceleration(800);  
    stepper_tunner.setSpeed(500);
    Tunner_Position = analogRead (ANALOG_IN_Tunner);
    Rotor_Position = analogRead (ANALOG_IN_Rotor);
    
    StartTime = millis(); //start the timer
    counter = 0;
}
  
void loop()
{ 
  Homming(); // Check if rotor is hommed   
  if(homed)  
   {     
      Tunner_Position = analogRead (ANALOG_IN_Tunner);
      average_tunner += 0.0005 * (Tunner_Position - average_tunner); // one pole digital filter, about 20Hz cutoff
      analog_Tunner = (average_tunner - 100) * 2.2;
      flag_Tunner = Move_tunner(analog_Tunner);      
      
      Rotor_Position = analogRead (ANALOG_IN_Rotor);
      average_rotor += 0.001 * (Rotor_Position - average_rotor); // one pole digital filter, about 20Hz cutoff
      analog_rotor = (average_rotor - 200) / 5; // * 1.2; 
      Rotor_isRunning = Move_rotor(analog_rotor);
      if((millis()-StartTime) >= 10000) { //The larger this number the better. Multiple serial.println interferes with the stepper motor
        StartTime = millis();
         Serial.print("distanceToGo = ");
         Serial.println(stepper_rotor.distanceToGo());
            if (stepper_rotor.distanceToGo() == 0) {
            digitalWrite(7, LOW);
          }
        //Serial.print("Rotor_isRunning = ");
        //Serial.println(Rotor_isRunning);   
      }
   }  
}

void Homming()
{
  if (stopNow || counter > 5) {   // Runs whne the home limitswitch i terrupt is detecetd.
      //Serial.print("Counter position = ");
      //Serial.println(counter);
      //detachInterrupt(0);                          // Interrupt not needed again (for the moment)  
      //Serial.print("Interrupted at ");  
      //Serial.println(stepper_rotor.currentPosition());  
      stepper_rotor.stop(); 
      stepper_rotor.move(-200); 
      stepper_rotor.setCurrentPosition(0);  
      //Serial.print("position established at home...");  
      //Serial.println(stepper_rotor.currentPosition());  
      stopNow = false;                            // Prevents repeated execution of the above code  
      counter = 0;
      homed = true;  
   }  
  else if (stepper_rotor.distanceToGo() == 0 && !homed) {   //executed repeatedly until we hit the limitswitch  
     if((millis()-StartTime) >= 400) {   //The larger this number the better. Multiple serial.println interferes with the stepper motor
        Serial.println(stepper_rotor.currentPosition());  
        stepper_rotor.setCurrentPosition(0);  
        stepper_rotor.moveTo(endPoint);  
        Serial.print("cycle count = ");  
        Serial.println(counter);  
        Serial.println(stepper_rotor.currentPosition());
     }      
     counter++; 
  }  
  if(!homed)
  {
      stepper_rotor.run(); //step the motor (this will step the motor by 1 step at each loop)
  }
}

int Move_tunner(int TunnerPosition){
  stepper_tunner.setSpeed(600);
  stepper_tunner.moveTo(TunnerPosition * 2);
  stepper_tunner.runSpeedToPosition();
  return (stepper_tunner.isRunning());
}

bool Move_rotor(int RotorPosition){
  if (stepper_rotor.distanceToGo() != 0) {
    digitalWrite(7, HIGH);  
  }
  stepper_rotor.setSpeed(800);
  stepper_rotor.moveTo(RotorPosition * 13);
  stepper_rotor.runSpeedToPosition();  
  latest_Rotor_Position = RotorPosition;
  return (stepper_rotor.isRunning());
}
//  
// Interrupt service routine for INT 0  
//  
void intService()  //Interrupt Services
{  
  stopNow = true; // Set flag to show Interrupt recognised and then stop the motor
  if(homed)
  {
     stepper_rotor.stop();  
     stepper_rotor.setCurrentPosition(0);
     stepper_rotor.move(-200);
//     Serial.print("!!!Interrupted!!!");
//     flag_Limit = 1;
  }   
}
