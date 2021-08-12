/**********************************************************************************************************************
  First try at getting a motor to track a signal using a PID loop
  Specify sine wave parameters -- virtually sample the sine wave -- track using PID

  Walker Nelson
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
// Used in a lot of stuff
int updateIntMs = 50;
// Sine wave gen and sampling
float sineVal;
float sineFreqHz = 0.25;
float sineAmp = 1;
// PID Control Loop
int controlLoopStartMs;
float Kp = 1, Ki = 0, Kd = 0;
float setPoint, input, output;
// Motor Control
double veryFar = 1.1;
double moveAmount = 0;
double moveSpeed = 0;
double moveAccel = 10;
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
// Threads
ThreadController doAllTasks = ThreadController();
Thread generateSineThread = Thread();
Thread sineWaveSamplingThread = Thread();
Thread getMotorPosThread = Thread();
Thread runPIDThread = Thread();
Thread plotThread = Thread();
// PID
QuickPID myQuickPID(&input, &output, &setPoint, Kp, Ki, Kd, QuickPID::REVERSE);
// Plotting
TimePlot MyPlot;

/******************
  Thread Functions:
******************/
// Generate the sine wave as fast as possible
void gensine() {
  //Serial.println("Generating Sine");
  sineVal = sineAmp * sin(2 * PI * sineFreqHz * millis() / 1000);
  //MyPlot.SendData(F("True Sine"),sineVal);
}

// Sample the sine wave a specific intervals
void samplesine() {
  //Serial.println("Sampling Sine");
  setPoint = sineVal;
}

// Do the PID calculations!
void querypidstate() {
  Serial.println("Running PID");
  input = (float)currentPosition;
  myQuickPID.Compute();
  moveSpeed = (double)output;
  if (moveSpeed >= 0) {
    targetPosition = veryFar;
  } else {
    targetPosition = -1 * veryFar;
  }
  sendpackettoslave();
}
// Send a bunch of data to a plot for debugging
void senddatatoplot() {
  MyPlot.SendData(F("Current Position"), currentPosition);
  MyPlot.SendData(F("Target Position"), targetPosition);
  MyPlot.SendData(F("Motor Speed"),moveSpeed);
  MyPlot.SendData(F("Output"),output);
  MyPlot.SendData(F("Input"),input);
  MyPlot.SendData(F("Set Point"),setPoint);
}

/*******************
  Arduino Functions:
*******************/
void setup() {
  // Serial communication for debugging
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Welcome to PID Tracking Code");

  // I2C communication -- setup to send stuff to motor controller
  Serial.println("Connecting To I2C");
  Wire.begin();
  Wire.setClock(400000);

  // Threads
  Serial.println("Starting Threads");
  generateSineThread.onRun(gensine);
  generateSineThread.setInterval(0);
  runPIDThread.onRun(querypidstate);
  runPIDThread.onRun(updateIntMs);
  getMotorPosThread.onRun(getpacketfromslave);
  getMotorPosThread.setInterval(updateIntMs);
  sineWaveSamplingThread.onRun(samplesine);
  sineWaveSamplingThread.setInterval(updateIntMs);
  plotThread.onRun(senddatatoplot);
  plotThread.setInterval(100);
  //doAllTasks.add(&runPIDThread);
  doAllTasks.add(&generateSineThread);
  doAllTasks.add(&getMotorPosThread);
  doAllTasks.add(&sineWaveSamplingThread);
  doAllTasks.add(&plotThread);

  // PID
  Serial.println("Turning on PID");
  myQuickPID.SetMode(QuickPID::AUTOMATIC);
}

void loop() {
  // Threads always run -- do everything in pseudo parallel
  doAllTasks.run();
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
    Serial.println("Sent Packet to Slave");
  }
}
