/**********************************************************************************************************************
  First try at getting a motor to track a signal using a PID loop
  Specify sine wave parameters -- virtually sample the sine wave -- track using PID

  Walker Nelson
  2021.8.11
**********************************************************************************************************************/

/******************
  Include Libraries:
******************/
//#include <Thread.h>
#include <ThreadController.h>
#include <Wire.h>
#include <QuickPID.h>
//#include <PID_AutoTune_v0.h>
#include "MegunoLink.h"

/***********************
  Parameter Definitions:
***********************/
// Timing
uint32_t timeNow = 0;
uint32_t lastTime = 0;
// Used in a lot of stuff
uint32_t updateIntUs = 10000;
// Sine wave gen and sampling
float sineVal;
float sineFreqHz = .167;
float sineAmp = 5;
// PID Control Loop
int controlLoopStartMs;
float Kp = 20, Ki = 3, Kd = 8;
float setPoint;
// Motor Control
float veryFar = 100;
float moveAmount = 0;
float moveSpeed = 0;
float moveAccel = 150;
float currentPosition = 0;
float targetPosition = 0;
// Wire Transmission Stuff
byte* currPosPtr = (byte*)&currentPosition;
byte* targetPosPtr = (byte*)&targetPosition;
byte* moveSpeedPtr = (byte*)&moveSpeed;
byte* moveAccelPtr = (byte*)&moveAccel;

/*****************
  Library Objects:
*****************/
// Threads
//ThreadController doAllTasks = ThreadController();
//Thread generateSineThread = Thread();
//Thread sineWaveSamplingThread = Thread();
//Thread getMotorPosThread = Thread();
//Thread runPIDThread = Thread();
//Thread plotThread = Thread();
// PID
QuickPID myPID(&currentPosition, &moveSpeed, &setPoint, Kp, Ki, Kd, QuickPID::DIRECT);
// Plotting
TimePlot MyPlot;

/******************
  Thread Functions:
******************/
// Generate the sine wave as fast as possible
void gensine() {
  //Serial.println("Generating Sine");
  sineVal = sineAmp * sin(2 * PI * sineFreqHz * micros() / 1000000);
  sineVal = sineVal + 0.2*sin(2*PI*6*sineFreqHz*micros()/1000000);
  //MyPlot.SendData(F("True Sine"),sineVal);
}

// Sample the sine wave a specific intervals
void samplesine() {
  //Serial.println("Sampling Sine");
  setPoint = sineVal;
}

// Do the PID calculations!
void querypidstate() {
  //Serial.println("Running PID");
  if (myPID.Compute()) {
    if (moveSpeed >= 0) {
      targetPosition = veryFar;
    } else {
      moveSpeed = -1*moveSpeed;
      targetPosition = -1 * veryFar;
    }
    sendpackettoslave();
  }
}
// Send a bunch of data to a plot for debugging
void senddatatoplot() {
  MyPlot.SendData(F("Current Position"), currentPosition);
  MyPlot.SendData(F("Motor Speed"), moveSpeed);
  MyPlot.SendData(F("Set Point"),setPoint);
}

/*******************
  Arduino Functions:
*******************/
void setup() {
  // Serial communication for debugging
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Welcome to PID Tracking Code");

  // I2C communication -- setup to send stuff to motor controller
  Serial.println("Connecting To I2C");
  Wire.begin();
  Wire.setClock(400000);

  // PID
  Serial.println("Turning on PID");
  myPID.SetOutputLimits(-12.5,12.5);
  myPID.SetSampleTimeUs(updateIntUs);
  myPID.SetMode(QuickPID::AUTOMATIC);

  // Plotting
  MyPlot.Clear();
}

void loop() {
  // Update the time
  timeNow = micros();
  
  // Always generate the sine wave
  gensine();
  // Sample and send stuff to plot if necessary
  if (timeNow - lastTime > updateIntUs){
    samplesine();
    getpacketfromslave();
    senddatatoplot();
    lastTime = timeNow;
  }

  // Always run PID loop
  querypidstate();
}

/****************
  Wire Functions:
****************/
// Request a number from the slave arduino
void getpacketfromslave() {
  //Serial.println("Requesting Packet From Slave");
  Wire.requestFrom(9, 4);
  int ii = 0;
  while (Wire.available() > 0) {
    currPosPtr[ii] = Wire.read();
    ii++;
  }
}

// Send all of the necessary parameters as bytes
void sendpackettoslave() {
  Wire.beginTransmission(9);
  for (byte i = 0; i < sizeof(double); i++) {
    Wire.write(targetPosPtr[i]);
  }
  for (byte i = 0; i < sizeof(double); i++) {
    Wire.write(moveSpeedPtr[i]);
  }
  for (byte i = 0; i < sizeof(double); i++) {
    Wire.write(moveAccelPtr[i]);
  }
  byte stat = Wire.endTransmission(9);
  if (stat == 0) {
    //Serial.println("Sent Packet to Slave");
  }
}
