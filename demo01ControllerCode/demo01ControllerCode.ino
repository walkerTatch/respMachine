/**********************************************************************************************************************
  Arduino code for first demo of resp machine

  State machine with a few different states:

  1) Homing / Initialization
  2) Idle (selecting files and configuring motion setup)
  3) Motion -- Sine tracking
  4) Motion -- SD signal tracking
  5) Motion -- Vacuum Pull
   

  Walker Nelson
  2021.8.11
**********************************************************************************************************************/

/******************
  Include Libraries:
******************/
#include <Wire.h>
//#include <SD.h>
//#include <SPI.h>
#include <QuickPID.h>
#include "MegunoLink.h"
#include "CommandHandler.h"

/***********************
  Parameter Definitions:
***********************/
// State control
uint8_t state = 0;                                                  //What's the state now
uint8_t prevState = 0;                                              //What was the state last time?

// Timing
uint32_t timeNow = 0;                                               //Tracker for looping controls
uint32_t lastSampleTime = 0;                                        //Tracker for sampling of I/O for PID loop
uint32_t lastPlotTime = 0;                                          //Tracker for plotting interval
uint32_t pidUpdateIntUs = 5000;                                     //Update rate of the calculations for the PID loop
uint32_t sampleUpdateIntUs = pidUpdateIntUs;                        //Update rate of I/O samplint for PID loop
uint32_t plotUpdateIntUs = 100000;                                  //Update rate of the plotting functions for both movement phases

// Sine wave generation parameters
uint32_t sineStartTimeUs;                                           // When did generation of the sine wave start?
float sineVal;                                                      // Current value of the sine wave
float sineFreqHz;                                                   // Frequency of the sine being generated
float sineAmp;                                                      // Amplitude of the sine being generated

// PID Control Loop
int controlLoopStartMs = 0;                                         //When was the control loop started in Ms
float Kp = 10, Ki = 0.1, Kd = 0.5;                                  //PID control parameters
float motorPosSetPoint;                                             //Target of motor position for PID tracking

// SD
//uint8_t chipSelect = 4;
//File testFile;

// Motor Control
float veryFar = 100;
float moveSpeed = 0;
float moveAccel = 25;
float currentPosition = 0;
float targetPosition = 0;

// Wire Transmission Stuff
byte* currPosPtr = (byte*)&currentPosition;
byte* targetPosPtr = (byte*)&targetPosition;
byte* moveSpeedPtr = (byte*)&moveSpeed;
byte* moveAccelPtr = (byte*)&moveAccel;

// Physical machine params
float motorSpeedMax = 12;
float membraneZeroPosition = 0;
float jogSpeed = 3;
float jogAccel = 10;

// Megunolink control params
boolean motorHomeCommand = false;
boolean motorStopCommand = false;
boolean motorJogMoveCommand = false;
boolean motorJogDirection = true;
boolean motorSineMoveCommand = false;


/*****************
  Library Objects:
*****************/
// Meguno commands
CommandHandler<> SerialCommandHandler;
// PID
QuickPID myPID(&currentPosition, &moveSpeed, &motorPosSetPoint, Kp, Ki, Kd, QuickPID::DIRECT);
// Plotting
TimePlot MyPlot;

/*******************
  Arduino Functions:
*******************/
void setup() {
  // Serial communication for debugging
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Welcome to The Resp Tester Machine...!");

  // I2C communication -- setup as master of the bus
  Serial.println("Connecting To I2C Bus");
  Wire.begin();
  Wire.setClock(400000);

  // Meguno buttons
  SerialCommandHandler.AddCommand(F("requestHome"),cmd_requesthome);
  SerialCommandHandler.AddCommand(F("stopMove"), cmd_stopmove);
  SerialCommandHandler.AddCommand(F("startMoveButton"), cmd_startmove);
  SerialCommandHandler.AddCommand(F("changeDir"), cmd_changedir);
  SerialCommandHandler.AddCommand(F("startSine"),cmd_startsine);

  // PID
  pidsetup();

  // SD
  sdsetup();

}

void loop() {
  SerialCommandHandler.Process();
  // State machine goes here
  switch(state){
    // State 0 corresponds to startup phase
    case 0:
      startupfun();
      prevState = 0;
    break;
    // State 1 corresponds to idle phase
    case 1:
      idlefun();
      prevState = 1;
    break;
    // State 2 corresponds to sine wave motion tracking
    case 2:
      sinetrackfun();
      prevState = 2;
    break;
    // State 3 corresponds to SD card motion tracking
    case 3:
      sdtrackfun();
      prevState = 3;
    break;
    // State 4 corresponds to vacuum pull
    case 4:
      vacuumpullfun();
      prevState = 4;
    break;
  }
}
