/********************************************************************************
  Sets up the system for a new tracking move

  - Clears plot
  - Moves to membrane zero
  - Resets loop timers

  Walker Nelson
  2021.8.14
********************************************************************************/
void movephasesetup() {
  // Serial debugging
  Serial.println("Executing Function:     movephasesetup");

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
