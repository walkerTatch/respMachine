/**********************************************************************************************************************
  Script to do autotuning on motor PID control so it doesn't track like crap!

  Brett Beaureard & Walker Nelson
  2021.8.11
**********************************************************************************************************************/

/******************
  Include Libraries:
******************/
#include <Thread.h>
#include <ThreadController.h>
#include <Wire.h>
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
#include "MegunoLink.h"

/***********************
  Parameter Definitions:
***********************/
// Set the clock frequency
int updateIntMs = 50;
// Autotune stuff
byte ATuneModeRemember = 2;
double setpoint = 10;
double kp = 5, ki = 0.5, kd = 2;

double outputStart = 5;
double aTuneStep = 5, aTuneNoise = 1, aTuneStartValue = 10;
unsigned int aTuneLookBack = 20;

boolean tuning = false;
unsigned long serialTime;

// Motor Control
double veryFar = 30;
double moveAmount = 0;
double moveSpeed = 0;
double moveAccel = 100;
double currentPosition = 0;
double targetPosition = 0;
// Wire Transmission Stuff
byte* currPosPtr = (byte*)&currentPosition;
byte* targetPosPtr = (byte*)&targetPosition;
byte* moveSpeedPtr = (byte*)&moveSpeed;
byte* moveAccelPtr = (byte*)&moveAccel;

/*****************
  Library Objects:
*****************/
// PID Stuff
PID myPID(&currentPosition, &moveSpeed, &setpoint, kp, ki, kd, DIRECT);
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
void pidtunemain(){
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
  MyPlot.SendData(F("Set Point"),setpoint);
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
  myPID.SetOutputLimits(-12,12);
  myPID.SetSampleTime(updateIntMs);
  myPID.SetMode(AUTOMATIC);

  // PID autotuner
  if (tuning){
    tuning = false;
    changeAutoTune();
    tuning = true;
  }

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

void loop(){
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
// Send out data
void SerialSend() {
  if (tuning) {
    Serial.println("tuning mode");
  } else {
    Serial.print("kp: "); Serial.print(myPID.GetKp()); Serial.print(" ");
    Serial.print("ki: "); Serial.print(myPID.GetKi()); Serial.print(" ");
    Serial.print("kd: "); Serial.print(myPID.GetKd()); Serial.println();
  }
}

// Get some stuff from serial to change the tuning state
void SerialReceive() {
  if (Serial.available())
  {
    char b = Serial.read();
    Serial.flush();
    if ((b == '1' && !tuning) || (b != '1' && tuning))changeAutoTune();
  }
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
