//Direction#1#Speed#50#Target#0#Active#1#
#include <EEPROM.h>

//variables
int active = 0;
float speedVal = 50; //float value between 0-100 for speed between 2 set limits
signed int dir; //Value of 0 or 1 for clockwise and anti-clockwise respectively
float targetAngle; //float value of angle to move to
float currentAngle;
float testAngle;
int angleAddr = 0; //EEPROM address to read and write the current angle to/from
int minWaitAddr = sizeof(float);
int maxWaitAddr = sizeof(float) + sizeof(int);

int minWait; //Sets the maximum speed
int maxWait;
int waitTime;

bool data = false; //flag to tell the program if it's reading new data or the values that data maps to
String dataType;

int pulsePin = 7;
int dirPin = 2;
int pulsesPerStep = 5; //Set by the DIP switches on the stepper driver

unsigned long currentTime;
unsigned long startTime;
unsigned long elapsedTime;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  pinMode(pulsePin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  //checks if this is the first time the code is being run, if so sets current angle to 0.
  EEPROM.get(angleAddr, currentAngle);
  EEPROM.get(minWaitAddr, minWait);
  EEPROM.get(maxWaitAddr, maxWait);
  if (minWait < 100){
    minWait = 100;
  }
  if (maxWait < minWait){
    maxWait = minWait * 2;
  }
  Serial.print("Current Angle: ");
  Serial.println(currentAngle);
  Serial.print("Min Wait: ");
  Serial.println(minWait);
  Serial.print("Max Wait: ");
  Serial.println(maxWait);
}

// the loop routine runs over and over again forever:
void loop() {

  //Trap in loop until there's something to read
    while ((Serial.available() == 0) && (currentAngle >= -10) && (currentAngle <= 370) && (active == 1)){
      digitalWrite(pulsePin, HIGH);
      delayMicroseconds(waitTime);
      digitalWrite(pulsePin, LOW);
      startTime = micros();

      currentAngle += dir * (0.02 / pulsesPerStep); //Incriments the current angle based on the direction and current step size

      //Serial.println(currentAngle); //Return the current angle to labview so it can display the current value (useful for confidence in the setup working properly)
      //Tests if the target angle has been reached (within 1 step) or if the target angle has been overstepped (depending on direction)
      if (abs(targetAngle - currentAngle) < (0.02 / pulsesPerStep)){
        EEPROM.put(angleAddr, currentAngle);
        active = 0;
        Serial.print("Target Reached: ");
        Serial.println(currentAngle, 10);
      }
      elapsedTime = micros() - startTime;
      //Serial.println(elapsedTime);
      delayMicroseconds(waitTime - elapsedTime);
    }

    if (EEPROM.get(angleAddr, testAngle) != currentAngle){
      EEPROM.put(angleAddr, currentAngle);
    }
    
    //If there's something to read, keep reading
    //Strings are separated by # sign
    while (Serial.available() != 0){
      //Serial.println();
      if (!data){
        dataType = Serial.readStringUntil('#');
        data = true;
      } else{
        String dataString = Serial.readStringUntil('#');
        if (dataType == "Direction"){
          dir = dataString.toInt();
          if ((dir == 1) || (dir == -1)){
          } else {
            dir = 1;
          }
        } else if (dataType == "Speed"){
          speedVal = dataString.toFloat();
          if ((speedVal > 100) || (speedVal < 1)){
            speedVal = 50;
          }
        } else if (dataType == "Target"){
          targetAngle = dataString.toFloat();
          if (targetAngle > 370){
            targetAngle = 370;
          } else if (targetAngle < -10){
            targetAngle = -10;
          }
        } else if (dataType == "Active"){
          active = dataString.toInt();
          if ((active == 1) || (active == 0)){
          } else {
            active = 0;
          }
        } else if (dataType == "Calibrate"){
          currentAngle = dataString.toFloat();
          if (currentAngle > 370){
            currentAngle = 370;
          } else if (currentAngle < -10){
            currentAngle = -10;
          }
          EEPROM.put(angleAddr, currentAngle);
        } else if (dataType == "MinWait"){
          minWait = dataString.toInt();
          if (minWait < 100){
            minWait = 100;
          }
          EEPROM.put(minWaitAddr, minWait);
        } else if (dataType == "MaxWait"){
          maxWait = dataString.toInt();
          EEPROM.put(maxWaitAddr, maxWait);
        } else if (dataType == "PulsesPerStep"){
          pulsesPerStep = dataString.toInt();
        } else if (dataType == "PulsePin"){
          pulsePin = dataString.toInt();
        } else if (dataType == "DirectionPin"){
          dirPin = dataString.toInt();
        }
        data = false;
      }
    }
    //Performs commands which only need to be done once after recieving new values (to reduce clutter in loop()
    if (dir == 1){
      digitalWrite(dirPin, LOW);
    } else if (dir == -1){
      digitalWrite(dirPin, HIGH);
    }
    waitTime = map(speedVal, 1, 100, maxWait, minWait);
}
