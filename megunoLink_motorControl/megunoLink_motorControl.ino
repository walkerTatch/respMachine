
// First try at getting some Megunolink panel controls working with a stepper motor :D
// Walker Nelson
// 2021.7.26

/*
**************************************************************************************************************
  Include Libraries
**************************************************************************************************************
*/
#include <SpeedyStepper.h>
#include "MegunoLink.h"
#include "CommandHandler.h"
#include <Thread.h>
#include <ThreadController.h>

/*
**************************************************************************************************************
  Library Object Definitions
**************************************************************************************************************
*/
// Meguno Stuff
CommandHandler<> SerialCommandHandler;
InterfacePanel MyPanel;
TimePlot myPlot;
// Stepper
SpeedyStepper stepper;
// Thread controller
ThreadController motorControlThread = ThreadController();
Thread posReportThread =  Thread();
Thread stateMachineThread =  Thread();

/*
**************************************************************************************************************
  Parameter Definitions
**************************************************************************************************************
*/
// Pin Definitions
const int MOTOR_STEP_PIN = 3;
const int MOTOR_DIRECTION_PIN = 4;
// Button press events
bool fwdRevToggle = false;
bool onOffToggle = false;
bool moveRequested = false;
bool stopRequested = false;
// State machine stuff
char *stateNames[] = {"off", "standby", "moving"};
int *stateNums[] = {0, 1, 2};
int state = 0;
bool movedone = true;
// Motion Stuff
double moveAmount = 0;
double moveSpeed = 0;
double moveAccel = 0;
double targetPosition = 0;
bool eStop = false;
bool forward = true;
// Misc
int stdbyWait = 0;
int timeNow = 0;

/*
**************************************************************************************************************
  Meguno Button Functions
**************************************************************************************************************
*/
// Start move button
void Cmd_StartMove(CommandParameter &Parameters) {
  Serial.println("Move Requested");
  moveAmount = Parameters.NextParameterAsDouble();
  moveSpeed = Parameters.NextParameterAsDouble();
  moveAccel = Parameters.NextParameterAsDouble();
  moveRequested = true;
}
// Stop move button
void Cmd_StopMove(CommandParameter &Parameters) {
  Serial.println("Estop Command Received");
  eStop = true;
}
// On/off button
void Cmd_ToggleOnOff(CommandParameter &Parameters) {
  Serial.println("On/Off Toggle");
  onOffToggle = true;
}
// Fwd/rev button
void Cmd_ChangeDir(CommandParameter &Parameters) {
  // flip the boolean for motor direction
  Serial.println("Direction Toggle");
  fwdRevToggle = true;
}

/*
**************************************************************************************************************
  Thread Functions
**************************************************************************************************************
*/
// Function for state machine thread
void runstatemachine() {
  // Read serial commands
  SerialCommandHandler.Process();
  // Switch Case for State Machine
  switch (state) {
    // Forward case
    case 0:
      offFun();
      break;
    // Reverse case
    case 1:
      stdbyFun();
      break;
    // Off case
    case 2:
      movFun();
      break;
  }
}
// Function for reporting motor position
void reportmotorposition() {
  // Send current motor position
  if (state == 2) {
    Serial.println((String)"{TIMEPLOT|DATA|StepperPos|T|" + stepper.getCurrentPositionInRevolutions() + "}");
  }
}

/*
**************************************************************************************************************
  Main Arduino Functions
**************************************************************************************************************
*/
// Setup function
void setup() {
  // Establish Serial Connection
  Serial.begin(9600);
  Serial.println("Initializing Motor Control....\n");

  // Setup Thread Controller
  // Configure Individual Threads
  stateMachineThread.onRun(runstatemachine);
  stateMachineThread.setInterval(1);
  posReportThread.onRun(reportmotorposition);
  posReportThread.setInterval(100);
  // Add Individual Threads to Thread Controller
  motorControlThread.add(&stateMachineThread);
  motorControlThread.add(&posReportThread);

  // Setup Meguno Commands
  SerialCommandHandler.AddCommand(F("startMoveButton"), Cmd_StartMove);
  SerialCommandHandler.AddCommand(F("stopMove"), Cmd_StopMove);
  SerialCommandHandler.AddCommand(F("changeDir"), Cmd_ChangeDir);
  SerialCommandHandler.AddCommand(F("toggleOnOff"), Cmd_ToggleOnOff);

  // Setup Stepper Motor
  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
  stepper.setStepsPerRevolution(800);
  stepper.setSpeedInRevolutionsPerSecond(1.0);
  stepper.setAccelerationInRevolutionsPerSecondPerSecond(1.0);

  // Set Meguno Indicators to Default State
  MyPanel.SetIndicator(F("forwardIndicator"), forward); // indicate whatever the state of "forward" is
  MyPanel.SetIndicator(F("motorOnIndicator"), false);  // indicate 'off'
}

// Looping function
void loop() {
  motorControlThread.run();
}
/*
**************************************************************************************************************
  State machine functions
**************************************************************************************************************
*/
// Function for the off mode -- do nothing except read commands
void offFun() {
  // If the on/off button is pressed, go to standby state
  if (onOffToggle) {
    state = 1;
    onOffToggle = !onOffToggle;
    MyPanel.SetIndicator(F("motorOnIndicator"), true);  // indicate 'on'
  }

  // Reset button presses besides on/off switch
  moveRequested = false;
  fwdRevToggle = false;
  eStop = false;
}

// Function for the standby mode
void stdbyFun() {
  // If the on/off button is pressed, go to off state
  if (onOffToggle) {
    state = 0;
    onOffToggle = !onOffToggle;
    MyPanel.SetIndicator(F("motorOnIndicator"), false);  // indicate 'off'
    return;
  }

  // Not turning off
  // Check on the fwd/back button and update UI
  if (fwdRevToggle) {
    forward = !forward;
    MyPanel.SetIndicator(F("forwardIndicator"), forward); // indicate whatever the state of "forward" is
    fwdRevToggle = false;
  }

  // Check if a move has been requested
  if (moveRequested) {
    // Setup the move then change state
    if (!forward) {
      moveAmount = moveAmount * -1;
    }
    targetPosition = stepper.getCurrentPositionInRevolutions() + moveAmount;
    stepper.setAccelerationInRevolutionsPerSecondPerSecond(moveAccel);
    stepper.setSpeedInRevolutionsPerSecond(moveSpeed);
    stepper.setupMoveInRevolutions(targetPosition);
    state = 2;
    myPlot.Clear("stepperPos");
  }

  // Standby timer -- send a command every once and a while to let the user know the machine is working
  timeNow = millis();
  if ((timeNow - stdbyWait) / 1000 > 5) {
    Serial.println("Standby.....");
    stdbyWait = millis();
  }
}

// Function when moving
void movFun() {
  // Keep moving if we are not done
  if (eStop){
    state = 1;
    movExitFun();
  }
  if (!stepper.motionComplete()) {
    movedone = stepper.processMovement();
    // Stop if we are done
    if (movedone) {
     movExitFun();
    }
  }
}

void movExitFun(){
   // Change state and process commands which were sent during the move
      state = 1;
      SerialCommandHandler.Process();
      moveRequested = false;
      fwdRevToggle = false;
      eStop = false;
}
