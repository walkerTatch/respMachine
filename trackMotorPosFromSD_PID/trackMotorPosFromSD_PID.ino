/**********************************************************************************************************************
  First try at getting a motor to track a signal using a PID loop
  Pulls datapoints from a file on an SD card -- then uses these as the tracking signal of a PID loop
  Communicates with the slave motor control arduino over Wire to send move commands and receive current position data

  Walker Nelson
  2021.8.11
**********************************************************************************************************************/

/******************
  Include Libraries:
******************/
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "QuickPID.h"
#include <Thread.h>
#include <ThreadController.h>

/*****************
  Library Objects:
*****************/
// Threads
ThreadController doAllTasks = ThreadController();
Thread getMotorPosThread = Thread();
Thread updateTargetPosThread = Thread();

/***********************
  Parameter Definitions:
***********************/
// SD card
const int CS = 11;
File myCurrentFile;
const byte numChars = 32;
// PID Control Loop
int controlLoopStartMs;
int updateIntMs = 50;
// Motor Control
double moveAmount = 0;
double moveSpeed = 0;
double moveAccel = 0;
double currentPosition = 0;
double targetPosition = 0;
float targetPosFloat = 0;
// Wire Transmission Stuff
byte* currPosPtr = (byte*)&currentPosition;
byte* targetPosPtr = (byte*)&targetPosition;
byte* moveSpeedPtr = (byte*)&moveSpeed;
byte* moveAccelPtr = (byte*)&moveAccel;

/******************
  Thread Functions:
******************/

/*******************
  Arduino Functions:
*******************/
void setup() {
  // Serial communication for debugging
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Welcome to PID Tracking Code");

  // I2C communication -- setup to send stuff to motor controller
  Wire.begin();
  Wire.setClock(400000);

  // SD Card -- attempt to initialize the card
  if (!SD.begin(CS)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
  myCurrentFile = SD.open("MYDATA~1.TXT",FILE_READ);
  // PID Stuff
  
  // Threads
  getMotorPosThread.onRun(getpacketfromslave);
  getMotorPosThread.setInterval(updateIntMs);
  updateTargetPosThread.onRun(readnextlinefromsdcard);
  updateTargetPosThread.setInterval(updateIntMs);
  doAllTasks.add(&getMotorPosThread);
  doAllTasks.add(&updateTargetPosThread);
  

}

void loop() {
  doAllTasks.run();
  // put your main code here, to run repeatedly:
  readnextlinefromsdcard();
  // Reporting
  Serial.println(targetPosition);
  Serial.println(currentPosition);
  delay(1000);
  moveSpeed = 10;
  moveAccel = 100;
  targetPosition = millis()/1000;
  sendpackettoslave();
}

/******************
  Helper Functions:
******************/
// Read the newest line from the SD card -- the most inefficient way possible lol...
void readnextlinefromsdcard(){
  if (myCurrentFile.available() > 0){
    String firstVal = myCurrentFile.readStringUntil(',');
    String secondVal = myCurrentFile.readStringUntil('\n');
    targetPosFloat = secondVal.toFloat();
    targetPosition = (double)targetPosFloat;
  }
}
/****************
  Wire Functions:
****************/
// Request a number from the slave arduino
void getpacketfromslave() {
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
  if (stat == 0){Serial.println("Sent Packet to Slave");}
}
