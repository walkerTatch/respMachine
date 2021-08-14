/**********************************************************************************************************************
  Script to do autotuning on motor PID control so it doesn't track like crap!

  Brett Beauregard & Walker Nelson
  2021.8.11
**********************************************************************************************************************/

/******************
  Include Libraries:
******************/
#include <Thread.h>
#include <ThreadController.h>
#include <Wire.h>
#include "QuickPID.h"
#include "MegunoLink.h"

/***********************
  Parameter Definitions:
***********************/
// Set the clock frequency
int updateIntMs = 50;
// Autotune stuff
byte ATuneModeRemember = 2;
float setpoint = 10;
float kp = 01, ki = 0.1, kd = 0.1;

float outputStart = 5;
float aTuneStep = 5, aTuneNoise = 1, aTuneStartValue = 10;
unsigned int aTuneLookBack = 20;

boolean pidLoop = false;
boolean tuning = false;
unsigned long serialTime;

// Motor Control
float veryFar = 30;
float moveAmount = 0;
float moveSpeed = 0;
float moveAccel = 100;
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
// PID Stuff
QuickPID myPID(&currentPosition, &moveSpeed, &setpoint, kp, ki, kd, QuickPID::DIRECT);
PID_ATune aTune(&currentPosition, &moveSpeed);
// Meguno Plotting
TimePlot MyPlot;
// Threads
ThreadController doAllTasks = ThreadController();
Thread getMotorPosThread = Thread();
Thread runPIDThread = Thread();
Thread tunePIDThread = Thread();
Thread plotThread = Thread();

/******************
  Thread Functions:
******************/
// Do the PID calculations!
void querypidstate() {
  //Serial.println("Running PID");
  if (myPID.Compute()) {
    if (moveSpeed >= 0) {
      targetPosition = veryFar;
    } else {
      targetPosition = -1 * veryFar;
    }
    sendpackettoslave();
  }
}

// Tune the PID
void pidtunemain() {
  if (tuning) {
    Serial.println("Tuning");
    byte val = (aTune.Runtime());
    if (val != 0) {
      tuning = false;
    }
    // Set the tuning parameters if done
    if (!tuning) {
      kp = aTune.GetKp();
      ki = aTune.GetKi();
      kd = aTune.GetKd();
      myPID.SetTunings(kp, ki, kd);
      AutoTuneHelper(false);
    }
  }
}

// Send a bunch of data to a plot for debugging
void senddatatoplot() {
  MyPlot.SendData(F("Current Position"), currentPosition);
  //MyPlot.SendData(F("Target Position"), targetPosition);
  MyPlot.SendData(F("Motor Speed"), moveSpeed);
  MyPlot.SendData(F("Set Point"), setpoint);
  //MyPlot.SendData(F("Output"), output);
  //MyPlot.SendData(F("Input"), input);
  //MyPlot.SendData(F("Set Point"), setPoint);
}

/*******************
  Arduino Functions:
*******************/
void setup() {
  // Serial communication for debugging
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Welcome to PID Tuning Code");

  // I2C communication for slave arduino
  Serial.println("Connecting To I2C");
  Wire.begin();
  Wire.setClock(400000);

  // PID
  Serial.println("Turning on PID");
  myPID.SetOutputLimits(-12, 12);
  myPID.SetSampleTime(updateIntMs);
  myPID.SetMode(AUTOMATIC);

  // PID autotuner
  myPID.AutoTune(tuningMethod::ZIEGLER_NICHOLS_PID);
  myPID.autoTune->autoTuneConfig(outputStep, hysteresis, setpoint, output, QuickPID::DIRECT, printOrPlotter, sampleTimeUs);

  // Threads
  //Motor position
  getMotorPosThread.onRun(getpacketfromslave);
  getMotorPosThread.setInterval(updateIntMs);
  //PID
  runPIDThread.onRun(querypidstate);
  runPIDThread.setInterval(0);
  //PID Tuning
  tunePIDThread.onRun(pidtunemain);
  tunePIDThread.setInterval(0);
  //Plotting
  plotThread.onRun(senddatatoplot);
  plotThread.setInterval(100);
  //Add to main thread
  doAllTasks.add(&getMotorPosThread);
  doAllTasks.add(&runPIDThread);
  doAllTasks.add(&tunePIDThread);
  doAllTasks.add(&plotThread);
}

void loop() {
  // Run all threads
  doAllTasks.run();
}

/********************
  PID Tune Functions:
********************/
void changeAutoTune() {
  if (!tuning)  {
    //Set the output to the desired starting frequency.
    moveSpeed = aTuneStartValue;
    aTune.SetNoiseBand(aTuneNoise);
    aTune.SetOutputStep(aTuneStep);
    aTune.SetLookbackSec((int)aTuneLookBack);
    AutoTuneHelper(true);
    tuning = true;
  }
  else { //cancel autotune
    aTune.Cancel();
    tuning = false;
    AutoTuneHelper(false);
  }
}

void AutoTuneHelper(boolean start) {
  if (start)
    ATuneModeRemember = myPID.GetMode();
  else
    myPID.SetMode(ATuneModeRemember);
}

float avg(int inputVal) {
  static int arrDat[16];
  static int pos;
  static long sum;
  pos++;
  if (pos >= 16) pos = 0;
  sum = sum - arrDat[pos] + inputVal;
  arrDat[pos] = inputVal;
  return (float)sum / 16.0;
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
  for (byte i = 0; i < sizeof(float); i++) {
    Wire.write(targetPosPtr[i]);
  }
  for (byte i = 0; i < sizeof(float); i++) {
    Wire.write(moveSpeedPtr[i]);
  }
  for (byte i = 0; i < sizeof(float); i++) {
    Wire.write(moveAccelPtr[i]);
  }
  byte stat = Wire.endTransmission(9);
  if (stat == 0) {
    //Serial.println("Sent Packet to Slave");
  }
}
