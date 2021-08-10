/*
   Function for running the stepper motor controller on one slave arduino.
   This arduino (Nano) is responsible for nothing except executing the motor controls & reporting the motor position
   This arduino will communicate with another arduino over a wired serial connection
*/

/*
 * *****************
   Include Libraries
 * *****************
*/
#include <FlexyStepper.h>
#include <Wire.h>
#include "MegunoLink.h"
#include "CommandHandler.h"

/*
 * **********************
   Define Library Objects
 * **********************
*/
FlexyStepper stepper;
CommandHandler<> SerialCommandHandler;

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
const int MOTOR_STEP_PIN = 3;
const int MOTOR_DIRECTION_PIN = 4;
double targetPos;
double moveSpeed;
double moveAccel;
double currentPos;
// Physical inputs

/*
 * *************************
   Command Handler Functions
 * *************************
*/
// Function to request a move
void Cmd_RequestMove(CommandParameter &Parameters) {
  Serial.println("Move Requested");
  targetPos = Parameters.NextParameterAsDouble();
  moveSpeed = Parameters.NextParameterAsDouble();
  moveAccel = Parameters.NextParameterAsDouble();
  moveRequested = true;
}

/*
 * ******************************
   Arduino Architecture Functions
 * ******************************
*/
void setup() {
  // Serial communication
  Serial.begin(57600);
  // Wait for Serial communication to start
  while (!Serial) {
    ;
  }
  Serial.println("Hello...");

  // Stepper stuff
  Serial.println("Initializing Stepper");
  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
  stepper.setStepsPerRevolution(800);

  // Command handler stuff
  Serial.println("Initializing Command Handler");
  SerialCommandHandler.AddCommand(F("moveRequested"), Cmd_RequestMove);
}

void loop() {
  // Process commands from the other arduino
  Serial.println("Reading Messages");
  SerialCommandHandler.Process();
  // If we have a move requested, do it!
  if (moveRequested) {
    // Set up the move first
    stepper.setAccelerationInRevolutionsPerSecondPerSecond(moveAccel);
    stepper.setSpeedInRevolutionsPerSecond(moveSpeed);
    stepper.setTargetPositionInRevolutions(targetPos);
    // Now execute the move -- stop when done or if the button is pressed
    while (!stepper.motionComplete()) {
      moveDone = stepper.processMovement();
      currentPos = stepper.getCurrentPositionInRevolutions();
      // Stop if we are done
      if (moveDone) {
        moveexitfun();
      }
    }
  }
  delay(1000);
}

/*
 * ***********************************
   Definition of Helper Functions etc.
 * ***********************************
*/
// Function for reporting motor position
void reportmotorposition() {
  Serial.println((String)"{TIMEPLOT|DATA|StepperPos|T|" + stepper.getCurrentPositionInRevolutions() + "}");
}

// Function called when a move is finished
void moveexitfun() {
  // Reset all of our flags to zero
  moveRequested = false;
  moveDone = false;
  eStop = false;
}
