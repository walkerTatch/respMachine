/********************************************************************************
  Sets up the PID control loop w/ necessary parameters and prepares it to autorun

  - motorSpeedMax: float indicating max speed (revolutions / sec) of stepper
  - updateIntPIDUs: uint32_t indicating the PID refresh period in microseconds
  
  Walker Nelson
  2021.8.14
********************************************************************************/
void pidsetup() {
  // Serial debugging
  //Serial.println("Executing Function:     pidsetup");
  Serial.println("Turning on PID");
  myPID.SetOutputLimits(motorSpeedMax*-1, motorSpeedMax);
  myPID.SetSampleTimeUs(pidUpdateIntUs);
  myPID.SetMode(QuickPID::AUTOMATIC);
}
