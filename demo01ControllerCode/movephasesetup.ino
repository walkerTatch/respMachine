/********************************************************************************
  Functions prepare the machine to enter and exit a tracking move

  Walker Nelson
  2021.8.14
********************************************************************************/
// First function is the setup for a tracking move
void movephasesetupprotocol() {
  // Serial debugging
  Serial.println("Executing Function:     movephasesetup");

  // Stop updating on the jog panel since there will be a plot
  updateJogPanelCoordinates = true;

  // Wait a short time
  delay(500);

  // Make sure we're at the correct starting position
  stepper_getposition();
  encoder_getposition();
  if (currentPosition != membraneZeroPosition) {
    movepistontomembranezero();
  }

  // Clear plot
  MyPlot.Clear();

  // Reset timers
  lastPlotTime = 0;
  lastSampleTime = 0;
}

// Second function is the exit protocol for a tracking move
void movephaseexitprotocol() {
  // Stop the motor
  stepper_stop();
  // Move motor back to the membrane zero position
  movepistontomembranezero();
  // Reset button presses
  resetstartbuttonpresses();
  // Start updating on the jog panel again
  updateJogPanelCoordinates = true;
  // Set state to idle
  state = 1;
}
