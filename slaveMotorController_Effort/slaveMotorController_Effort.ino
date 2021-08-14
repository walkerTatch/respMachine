/*
   Function for running the stepper motor controller on one slave arduino
   This arduino will communicate with another arduino over an I2C connection
   Uses an effort signal to determine how much acceleration the motor should have
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
// Movement control bools
bool moveRequested = false;
bool moveDone = false;
bool eStop = false;
// Movement control params
const int MOTOR_STEP_PIN = 7;
const int MOTOR_DIRECTION_PIN = 8;
float veryFar = 100;
float targetPos;
float moveSpeed = 12;
float moveAccel;
float currentPos;
// Physical inputs
int eStopPin = 2;

// Wire transmission stuff
byte* currPosPtr = (byte*)&currentPos;
byte* targetPosPtr = (byte*)&targetPos;
byte* moveSpeedPtr = (byte*)&moveSpeed;
byte* moveAccelPtr = (byte*)&moveAccel;

/*
 * *********************
   Wire Receive Function
 * *********************
*/
// Function to run when sent a new value
void moverequested(int numBytes) {
  //Serial.println("Move Requested");
  for (byte i = 0; i < sizeof(float); i++) {
    Wire.read();
  }
  for (byte i = 0; i < sizeof(float); i++) {
    Wire.read();
  }
  for (byte i = 0; i < sizeof(float); i++) {
    moveAccelPtr[i] = Wire.read();
  }
  // Set new move parameters
  stepper.setAccelerationInRevolutionsPerSecondPerSecond(moveAccel);
  stepper.setSpeedInRevolutionsPerSecond(moveSpeed);
  stepper.setTargetPositionInRevolutions(targetPos);
  // Change the request flag to true
  if (moveAccel > 0) {
    targetPos = veryFar;
  } else {
    targetPos = -1*veryFar;
  }
  moveRequested = true;
  //Serial.println(targetPos);
  //Serial.println(moveSpeed);
  //Serial.println(moveAccel);
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
  Wire.onReceive(moverequested);
  Wire.onRequest(sendcurrentmotorpos);

  // Stepper stuff
  //Serial.println("Initializing Stepper");
  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
  stepper.setStepsPerRevolution(800);

  // eStop Interrupt
  pinMode(eStopPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(eStopPin), eStopRoutine, RISING);
}

void loop() {
  // If we have a move requested, do it!
  if (moveRequested) {
    // Now execute the move -- stop when done or if the button is pressed
    if (!stepper.motionComplete()) {
      moveDone = stepper.processMovement();
      currentPos = stepper.getCurrentPositionInRevolutions();
      // Stop if we are done
      if (moveDone || eStop) {
        // Reset all of our flags to zero
        moveRequested = false;
        moveDone = false;
        eStop = false;
      }
    }
  }
}

/*
 * ******************
   Interrupt Function
 * ******************
*/
void eStopRoutine() {
  eStop = true;
}
