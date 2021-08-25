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
    // Setup for the movement phase
    movephasesetupprotocol();

    // Reset "zero time" for sine wave (do this last)
    sineStartTimeUs = micros();
  }

  // Take the time;
  timeNow = micros();
  uint32_t sineTime = timeNow-sineStartTimeUs;

  // Generate the sine wave every loop
  sineVal = sineAmp * sin(2 * PI * sineFreqHz * (timeNow - sineStartTimeUs) / 1000000);

  // If our sample update period has been hit, sample the sine wave and the motor position
  if (timeNow - lastSampleTime > sampleUpdateIntUs) {
    motorPosSetPoint = sineVal + membraneZeroPosition;
    stepper_getposition();
    encoder_getposition();
    lastSampleTime = timeNow;
  }

  // Run the PID loop
  trackpositionsignal();

  // If our plot update period has been hit, send data to the plot
  if (timeNow - lastPlotTime > plotUpdateIntUs) {
    senddatatoplot();
    lastPlotTime = timeNow;
  }

  // If a stop was requested -- do the "movement phase ended" events
  if (motorStopCommand) {
    // Reset button state
    motorStopCommand = false;
    // Trigger the tracking move phase exit protocol
    movephaseexitprotocol();
  }
}
