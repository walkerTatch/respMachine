/*************************************************************************************
  Second movement mode of the machine -- loads in a the signal laid out in a data file
  Motor position then tracks the signal

  Walker Nelson
  2021.8.14
*************************************************************************************/
void sdtrackfun() {
  // Serial debugging
  //Serial.println("Executing Function: sdtrackfun");

  // If the last state was not this one, do a little update
  if (prevState != state) {

    // Setup for the movement phase
    movephasesetup();

    // Open the file
    testFile = SD.open("dat.txt");
    if (testFile) {
      Serial.println("Tracking signal from dat.txt");
    } else {
      Serial.println("Error opening file");
    }
  }

  // Take the time;
  timeNow = micros();

  // If our sample update period has been hit, sample the SD card and the motor position
  if (timeNow - lastSampleTime > sampleUpdateIntUs) {
    
    // Try to read from SD card
    bool couldRead = readfloatfromsdcard();
    
    // If we could, update everything
    if (couldRead) {
      motorPosSetPoint += membraneZeroPosition;
      stepper_getposition();
      
    // Couldn't read? File is done -- stop movement
    } else {
      Serial.println("Done reading SD file");
      testFile.close();
      motorStopCommand = true;
    }
    
    // Reset the time
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
    // Stop the motor
    stepper_stop();
    // Move motor back to the membrane zero position
    movepistontomembranezero();
    // Set state to idle & reset button presses
    state = 1;
    resetstartbuttonpresses();
  }
}

// Helper function to read the next line from an SD card
bool readfloatfromsdcard() {
  bool success = false;
  if (testFile.available() >= sizeof(float)) {
    for (byte ii = 0; ii < sizeof(float); ii++) {
      motorPosSetPointPtr[ii] = testFile.read();
    }
    success = true;
  }
  return success;
}
