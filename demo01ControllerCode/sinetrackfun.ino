/*************************************************************************************
  First movement mode of the machine -- generates a sine wave w/ user specified params
  Motor position then tracks the sine wave

  Walker Nelson
  2021.8.14
*************************************************************************************/

void sinetrackfun() {
  // Serial debugging
  //Serial.println("Executing Function:     sinetrackfun");

  // If the last state was not this one, do a little update
  if (prevState != state) { 
    delay(2000);
        
    // Make sure we're at the correct starting position here
    getpacketfromslave();
    if (currentPosition != membraneZeroPosition) {
      movepistontomembranezero();
    }

    // Clear plot
    MyPlot.Clear();
    
    // Reset timers
    lastPlotTime = 0;
    lastSampleTime = 0;

    // Reset "zero time" for sine wave (do this last)
    sineStartTimeUs = micros();
  }

  // Take the time;
  timeNow = micros();
  uint32_t sineTime = timeNow-sineStartTimeUs;
  Serial.println(sineTime);

  // Generate the sine wave every loop
  sineVal = sineAmp * sin(2 * PI * sineFreqHz * (timeNow - sineStartTimeUs) / 1000000);

  // If our sample update period has been hit, sample the sine wave and the motor position
  if (timeNow - lastSampleTime > sampleUpdateIntUs) {
    motorPosSetPoint = sineVal + membraneZeroPosition;
    getpacketfromslave();
    lastSampleTime = timeNow;
  }

  // Run the PID loop
  trackpositionsignal();

  // If our plot update period has been hit, send data to the plot
  if (timeNow - lastPlotTime > plotUpdateIntUs) {
    senddatatoplot();
  }

  // If a stop was requested -- do the "movement phase ended" events
  if (motorStopCommand) {
    // Reset button state
    motorStopCommand = false;
    // Move motor back to the membrane zero position
    movepistontomembranezero();
    // Set state to idle
    state = 1;
  }
}
