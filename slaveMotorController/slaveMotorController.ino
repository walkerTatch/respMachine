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
byte state = 0;               // 0 --> idle, 1 --> moving, 2 --> homing
byte prevState = 0;

// Movement control flags
bool moveRequested = false;
bool homeRequested = false;
bool moveDone = false;

// Stepper params
const long idleCurrent = 500;
const long fullCurrent = 2000;

// Movement control params
// Homing phase
const byte homeDir = -1;
const float homeSpeed = 2.5;
const long maxHomeDistance = 40;
const float homeSwitchPositionRev = 35.25;
// Movement phase
float targetPos;
float moveSpeed = 10;
float moveAccel = 10;
float currentPos;
// Idle phase
bool powerSave = false;
const uint32_t powerSaveTimeoutMillis = 100000;
uint32_t idleStateStart = 0;

// Pin declarations
const int MOTOR_STEP_PIN = 7;
const int MOTOR_DIRECTION_PIN = 8;
const int limitSwitchPin = 2;
const uint8_t CSPin = 10;

// Wire transmission stuff
byte commandByte;
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
  //Serial.println(commandByte);
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
      movecommand();
      break;
    // Home command
    case 4:
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
}

// Function to run when a value is requested
// Always sends back the current motor position followed by a byte representing the "moveDone" flag
void requesthandler() {
  // Get the motor position & send it back
  currentPos = stepper.getCurrentPositionInRevolutions();
  for (byte i = 0; i < sizeof(float); i++) {
    Wire.write(currPosPtr[i]);
  }
  // Send back the "moveDone" flag as a byte
  Wire.write((byte)moveDone);
}

/*
 * ******************************
   Arduino Architecture Functions
 * ******************************
*/
void setup() {
  // Serial debugging
  //Serial.begin(115200);

  // Stepper stuff
  //Serial.println("Initializing Stepper");
  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
  stepper.setStepsPerRevolution(800);
  stepper.setAccelerationInRevolutionsPerSecondPerSecond(moveAccel);
  stepper.setSpeedInRevolutionsPerSecond(moveSpeed);

  // Stepper driver
  // Start SPI
  //Serial.println("Setting up Stepper Driver");
  SPI.begin();
  myDriver.setChipSelectPin(CSPin);
  // Reset driver
  myDriver.resetSettings();
  myDriver.clearStatus();
  // Set parameters (taken from Pololu example -- can go up to 3000mA for motor)
  myDriver.setDecayMode(HPSDDecayMode::AutoMixed);
  myDriver.setCurrentMilliamps36v4(idleCurrent);
  myDriver.setStepMode(HPSDStepMode::MicroStep4);
  // Enable driver
  //Serial.println("Enabling Stepper Driver");
  myDriver.enableDriver();

  // I2C communication
  //Serial.println("Starting Wire Communication");
  Wire.begin(9);
  Wire.setClock(400000);
  Wire.onReceive(parsecommand);
  Wire.onRequest(requesthandler);
}

void loop() {

  //Serial.print(digitalRead(limitSwitchPin));
  // Check the state
  switch (state) {
    // Idle state
    case 0:
      idlefun();
      prevState = 0;
      break;
    // Moving state
    case 1:
      movefun();
      prevState = 1;
      break;
    // Homing state
    case 2:
      homefun();
      prevState = 2;
      break;
  }
}

/***********************
  Functions for Movement:
 **********************/
// Function for idling the motor at a lower current so it doesn't remain at full power when not moving
void idlefun() {
  // If this is the first time in the state, start a timer for killing motor power
  if (prevState != state) {
    idleStateStart = millis();
    //Serial.println("Idle");
  }

  // Check the timer and see if we need to kill power
  if ((millis() - idleStateStart > powerSaveTimeoutMillis) && !powerSave) {
    //Serial.println("Entering power save mode");
    powerSave = true;
    myDriver.setCurrentMilliamps36v4(idleCurrent);
  }

  // If there is a move operation requested do that
  if (moveRequested) {
    moveRequested = false;
    powerSave = false;
    //Serial.println("Upping Motor Current");
    myDriver.setCurrentMilliamps36v4(fullCurrent);
    state = 1;
  }
  
  // If there is a home operation requested do that
  if (homeRequested) {
    homeRequested = false;
    powerSave = false;
    //Serial.println("Upping Motor Current");
    myDriver.setCurrentMilliamps36v4(fullCurrent);
    state = 2;
  }
}

// Function checks if we need to move, continues to move until it finishes or the stop command is called
void movefun() {
  if (moveSpeed != 0) {
    // Execute the move -- keep exiting this function and coming back
    if (!stepper.motionComplete()) {
      moveDone = stepper.processMovement();
    }
    // Stop if we are done
    if (moveDone) {
      // Go into the idle state
      state = 0;
    }
  }
}

// Function homes the motor
void homefun() {
  // Home command
  stepper.moveToHomeInRevolutions(1, 3, 100, limitSwitchPin);
  // Set to the correct position
  stepper.setCurrentPositionInRevolutions(homeSwitchPositionRev);
  // Send a flag, wait, and go straight to idle
  moveDone = true;
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
  //Serial.println("Reset Command Received");
  stopcommand();
  zerocommand();
}

// Function zeros out the motor position
void zerocommand() {
  // Serial debugging
  //Serial.println("Zero Command Received");
  stepper.setCurrentPositionInRevolutions(0);
}

// Function stops the motor
void stopcommand() {
  // Serial debugging
  //Serial.println("Stop Command Received");
  state = 0;
  moveDone = true;
  moveRequested = false;
}

// Function starts a move
void movecommand() {
  // Serial debugging
  //Serial.println("Move Command Received");
  moveRequested = true;
  readmoveparams();
  moveDone = false;
}

// Function starts a homing move
void homecommand() {
  // Serial debugging
  //Serial.println("Home Move Command Received");
  homeRequested = true;
  moveDone = false;  
}
