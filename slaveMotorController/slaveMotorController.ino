/*
   Function for running the stepper motor controller on one slave arduino.
   This arduino (Nano) is responsible for nothing except executing the motor controls & reporting the motor position
   This arduino will communicate with another arduino over an I2C connection
*/

/*
 * *****************
   Include Libraries
 * *****************
*/
#include <FlexyStepper.h>
#include <Wire.h>

/*
 * **********************
   Define Library Objects
 * **********************
*/
FlexyStepper stepper;

/*
 * ****************
   Setup Parameters
 * ****************
*/
// Wire Comm Variables
byte commandByte;
// Movement control bools
bool moveRequested = false;
bool moveDone = false;
bool stopRequested = false;

// Movement control params
const int MOTOR_STEP_PIN = 7;
const int MOTOR_DIRECTION_PIN = 8;
byte homeDir = -1;
float homeSpeed = 2.5;
long maxHomeDistance = 40;
float targetPos;
float moveSpeed = 1;
float moveAccel = 1;
float currentPos;

// Physical inputs
int limitSwitchPin = 9;

// Wire transmission stuff
byte* currPosPtr = (byte*)&currentPos;
byte* targetPosPtr = (byte*)&targetPos;
byte* moveSpeedPtr = (byte*)&moveSpeed;
byte* moveAccelPtr = (byte*)&moveAccel;

/*
 * *************
   I2C Functions
 * *************
*/
// Read one byte, then decide what to do
void parsecommand(){
  commandByte = Wire.read();
  switch (commandByte){
    // Reset the arduino
    case 0:
       resetcommand();
    break;
    // Set position to zero
    case 1:
      zerocommand();
    break;
    // Stop moving
    case 2:
      stopcommand();
    break;
    // Move command
    case 3:
      movecommand();
    break;
    case 4:
      homecommand();
    break;
  }
}

// Function to run when a value is requested
void sendcurrentmotorpos() {
  //Serial.println("Data Requested");
  for (byte i = 0; i < sizeof(float); i++) {
    Wire.write(currPosPtr[i]);
  }
  //Serial.println(currentPos);
}

/*
 * ******************************
   Arduino Architecture Functions
 * ******************************
*/
void setup() {
  // Serial debugging
  //Serial.begin(9600);

  // I2C communication
  //Serial.println("Starting Wire Communication");
  Wire.begin(9);
  Wire.setClock(400000);
  Wire.onReceive(parsecommand);
  Wire.onRequest(sendcurrentmotorpos);

  // Stepper stuff
  //Serial.println("Initializing Stepper");
  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
  stepper.setStepsPerRevolution(800);
  stepper.setAccelerationInRevolutionsPerSecondPerSecond(moveAccel);
  stepper.setSpeedInRevolutionsPerSecond(moveSpeed);
}

void loop() {
  // If we have a move requested, do it!
  if (moveRequested) {
    // Now execute the move -- stop when done or if the button is pressed
    if (!stepper.motionComplete()) {
      moveDone = stepper.processMovement();
      currentPos = stepper.getCurrentPositionInRevolutions();
      // Stop if we are done
      if (moveDone || stopRequested) {
        // Reset all of our flags to zero
        moveRequested = false;
        moveDone = false;
        stopRequested = false;
      }
    }
  }
}

/*
 * *****************
   Helper Functions:
 * *****************
*/
// Function resets the arduino
void resetcommand() {
  // Serial debugging
  Serial.println("Reset Command Received");
  stopcommand();
  zerocommand();
}

// Function zeros out the motor position
void zerocommand() {
  // Serial debugging
  Serial.println("Zero Command Received");
  stepper.setCurrentPositionInRevolutions(0);
}

// Function stops the motor
void stopcommand() {
  // Serial debugging
  Serial.println("Stop Command Received");
  stopRequested = true;
}

// Function reads in data for a move command and initiates the move
void movecommand() {
  // Serial debugging
  Serial.println("Move Command Received");
  // Read in the three values & store
  for (byte i = 0; i < sizeof(float); i++) {
    targetPosPtr[i] = Wire.read();
  }
  for (byte i = 0; i < sizeof(float); i++) {
    moveSpeedPtr[i] = Wire.read();
  }
  for (byte i = 0; i < sizeof(float); i++) {
    moveAccelPtr[i] = Wire.read();
  }
  // Set new move parameters
  stepper.setAccelerationInRevolutionsPerSecondPerSecond(moveAccel);
  stepper.setSpeedInRevolutionsPerSecond(moveSpeed);
  stepper.setTargetPositionInRevolutions(targetPos);
  // Change the request flag to true if a speed > 0 was requested
  if (moveSpeed != 0) {
    moveRequested = true;
  } else {
    moveRequested = false;
  }
  //Serial.println(targetPos);
  //Serial.println(moveSpeed);
  //Serial.println(moveAccel);
}

// Function does a homing operation
void homecommand() {
  // Serial debugging
  Serial.println("Homing Command Received");
  stepper.moveToHomeInRevolutions(homeDir,homeSpeed,maxHomeDistance,limitSwitchPin);
}
