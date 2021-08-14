/******************************************************************************
   AutoTune Filter DIRECT Example
   Circuit: https://github.com/Dlloydev/QuickPID/wiki/AutoTune_RC_Filter
 ******************************************************************************/

#include "QuickPID.h"
#include <Wire.h>

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

const uint32_t sampleTimeUs = 50000; // 50ms
uint32_t lastTime = 0;
uint32_t timeNow;
const int outputMax = 12;
const int outputMin = -12;

bool printOrPlotter = 1;  // on(1) monitor, off(0) plotter
float POn = 1.0;          // proportional on Error to Measurement ratio (0.0-1.0), default = 1.0
float DOn = 0.0;          // derivative on Error to Measurement ratio (0.0-1.0), default = 0.0

byte outputStep = 1;
byte hysteresis = 1;
int setpoint = 20;       // 1/3 of range for symetrical waveform
int output = 1;          // 1/3 of range for symetrical waveform

float Input, Output, Setpoint;
float Kp = 0, Ki = 0, Kd = 0;
bool pidLoop = false;

QuickPID _myPID = QuickPID(&currentPosition, &moveSpeed, &Setpoint, Kp, Ki, Kd, POn, DOn, QuickPID::DIRECT);

void setup() {
  Serial.begin(115200);
  Serial.println();
  if (constrain(output, outputMin, outputMax - outputStep - 5) < output) {
    Serial.println(F("AutoTune test exceeds outMax limit. Check output, hysteresis and outputStep values"));
    while (1);
  }

  // I2C communication for slave arduino
  Serial.println("Connecting To I2C");
  Wire.begin();
  Wire.setClock(400000);

  // Select one, reference: https://github.com/Dlloydev/QuickPID/wiki
  //_myPID.AutoTune(tuningMethod::ZIEGLER_NICHOLS_PI);
  _myPID.AutoTune(tuningMethod::ZIEGLER_NICHOLS_PID);
  //_myPID.AutoTune(tuningMethod::TYREUS_LUYBEN_PI);
  //_myPID.AutoTune(tuningMethod::TYREUS_LUYBEN_PID);
  //_myPID.AutoTune(tuningMethod::CIANCONE_MARLIN_PI);
  //_myPID.AutoTune(tuningMethod::CIANCONE_MARLIN_PID);
  //_myPID.AutoTune(tuningMethod::AMIGOF_PID);
  //_myPID.AutoTune(tuningMethod::PESSEN_INTEGRAL_PID);
  //_myPID.AutoTune(tuningMethod::SOME_OVERSHOOT_PID);
  //_myPID.AutoTune(tuningMethod::NO_OVERSHOOT_PID);

  _myPID.autoTune->autoTuneConfig(outputStep, hysteresis, setpoint, output, QuickPID::DIRECT, printOrPlotter, sampleTimeUs);
}

void loop() {
  timeNow = micros();
    if (timeNow - lastTime >= sampleTimeUs) {
      getpacketfromslave();
      lastTime = timeNow;
    }

  if (_myPID.autoTune) // Avoid dereferencing nullptr after _myPID.clearAutoTune()
  {
    switch (_myPID.autoTune->autoTuneLoop()) {
      case _myPID.autoTune->AUTOTUNE:
        sendpackettoslave();
        break;

      case _myPID.autoTune->TUNINGS:
        _myPID.autoTune->setAutoTuneConstants(&Kp, &Ki, &Kd); // set new tunings
        _myPID.SetMode(QuickPID::AUTOMATIC); // setup PID
        _myPID.SetSampleTimeUs(sampleTimeUs);
        _myPID.SetTunings(Kp, Ki, Kd, POn, DOn); // apply new tunings to PID
        Setpoint = 500;
        break;

      case _myPID.autoTune->CLR:
        if (!pidLoop) {
          _myPID.clearAutoTune(); // releases memory used by AutoTune object
          pidLoop = true;
        }
        break;
    }
  }
  if (pidLoop) {
    if (printOrPlotter == 0) { // plotter
      Serial.print("Setpoint:");  Serial.print(Setpoint);  Serial.print(",");
      Serial.print("Input:");     Serial.print(currentPosition);     Serial.print(",");
      Serial.print("Output:");    Serial.print(moveSpeed);    Serial.println(",");
    }
    if (_myPID.Compute()) {
      sendpackettoslave();
    }
  }
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
  Serial.println("Requesting Packet From Slave");
  Wire.requestFrom(9, 4);
  int ii = 0;
  while (Wire.available() > 0) {
    currPosPtr[ii] = Wire.read();
    ii++;
  }
}

// Send all of the necessary parameters as bytes
void sendpackettoslave() {
  if (moveSpeed >= 0) {
    targetPosition = veryFar;
  } else {
    targetPosition = -1 * veryFar;
  }
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
    Serial.print("Sent Packet to Slave");
    Serial.print(targetPosition);
    Serial.print(moveSpeed);
    Serial.println(moveAccel);
  }
}
