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
#include <SPI.h>
#include <HighPowerStepperDriver.h>

/*
 * **********************
   Define Library Objects
 * **********************
*/
FlexyStepper stepper;
HighPowerStepperDriver myDriver;

/*
 * ****************
   Setup Parameters
 * ****************
*/
// State control variables
byte state = 0;           // 0 --> flex move, 1 --> blocking move, 2 --> homing

// Wire Comm Variables
byte commandByte;

// SPI Variables
const uint8_t CSPin = 10;

// Movement control bools
bool moveRequested = false;
bool moveDone = false;
bool stopRequested = false;
bool blockingMoveDone = false;

// Movement control params
const int MOTOR_STEP_PIN = 7;
const int MOTOR_DIRECTION_PIN = 8;
byte homeDir = -1;
float homeSpeed = 2.5;
long maxHomeDistance = 40;
float targetPos;
float moveSpeed = 10;
float moveAccel = 10;
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
void parsecommand(int numBytes) {
  // Get the command byte
  commandByte = Wire.read();
  // Do a different command depending on what was sent
  switch (commandByte) {
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
    // Flex move command
    case 3:
      flexmovecommand();
      break;
    // Blocking move command
    case 4:
      blockingmovecommand();
      break;
    // Home command
    case 5:
      homecommand();
      break;
  }
}

// Function finishes reading the wire buffer if there is a move command
void readmoveparams() {
  // First the target position, then the speed, then the accel
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
  if (moveSpeed != 0) {
    moveRequested = true;
  } else {
    moveRequested = false;
  }
}

// Function to run when a value is requested
void requesthandler(){
  // If we're in a flexy move, always send the motor position
  if (state == 0) {
    sendcurrentmotorpos();
  // If we're in a blocking move, send the state of the blocking move
  } else {
    Wire.write(blockingMoveDone);
  }
}

// Sends back the motor position
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
  Serial.begin(115200);

  // Stepper stuff
  Serial.println("Initializing Stepper");
  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
  stepper.setStepsPerRevolution(800);
  stepper.setAccelerationInRevolutionsPerSecondPerSecond(moveAccel);
  stepper.setSpeedInRevolutionsPerSecond(moveSpeed);

  // Stepper driver
  // Start SPI
  Serial.println("Setting up Stepper Driver");
  SPI.begin();
  myDriver.setChipSelectPin(CSPin);
  // Reset driver
  myDriver.resetSettings();
  myDriver.clearStatus();
  // Set parameters (taken from Pololu example -- can go up to 3000mA for motor)
  myDriver.setDecayMode(HPSDDecayMode::AutoMixed);
  myDriver.setCurrentMilliamps36v4(2000);
  myDriver.setStepMode(HPSDStepMode::MicroStep4);
  // Enable driver
  Serial.println("Enabling Stepper Driver");
  myDriver.enableDriver();
  
  // I2C communication
  Serial.println("Starting Wire Communication");
  Wire.begin(9);
  Wire.setClock(400000);
  Wire.onReceive(parsecommand);
  Wire.onRequest(requesthandler);
}

void loop() {
  // Check the state
  switch (state) {
    // Flex move state
    case 0:
      flexmovefun();
      break;
    // Blocking move state
    case 1:
      blockingmovefun();
      break;
    // Homing State
    case 2:
      homefun();
      break;
  }
}

/***********************
  Functions for Movement:
 **********************/
// Function for going home
void homefun() {
  // Home command
  stepper.moveToHomeInRevolutions(1, 3, 100, 9);
  // End by going into the normal movement state
  stepper.setCurrentPositionInRevolutions(35.25);
  blockingMoveDone = true;
  delay(100);
  state = 0;
}

// Function checks if we need to move, then does it if necessary -- keeps the main looping function open so that the state and move parameters can be updated mid move
void flexmovefun() {
  if (moveRequested && moveSpeed != 0) {
    // Execute the move -- keep exiting this function and coming back
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

// Function checks if we need to move, then does it -- BLOCKING! Waits until the move is done, then sends a confirmation byte to the master arduino
void blockingmovefun() {
  if (moveRequested && moveSpeed != 0) {
    // Do the move, but don't break out of this function
    while (!stepper.motionComplete() && !stopRequested) {
      stepper.processMovement();
      currentPos = stepper.getCurrentPositionInRevolutions();
    }
    // Reset flags to zero
    moveRequested = false;
    moveDone = false;
    stopRequested = false;
  }
  // Change the move state to true
  blockingMoveDone = true;
  delay(100);
  state = 0;
}

/*
 * **********************
   Wire Helper Functions:
 * **********************
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

// Function starts a non-blocking move
void flexmovecommand() {
  // Serial debugging
  Serial.println("Flex Move Command Received");
  readmoveparams();
  state = 0;
}

// Function starts a blocking move
void blockingmovecommand() {
  // Serial debugging
  Serial.println("Blocking Move Command Received");
  readmoveparams();
  state = 1;
  blockingMoveDone = false;
}

// Function starts a homing move
void homecommand() {
  // Serial debugging
  Serial.println("Home Move Command Received");
  state = 2;
  blockingMoveDone = false;
}
