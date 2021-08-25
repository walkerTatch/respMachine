/****************************************************************************************************************
  Idle function of the machine -- basically does nothing but wait and allow the state to change to something else
  This is where the user can set parameters like sine tracking stuff, SD file to read etc...

  Walker Nelson
  2021.8.14
****************************************************************************************************************/
void idlefun() {
  // Serial debugging
  Serial.println("Idle....");

  // Check on the membrane and motor position
  stepper_getposition();
  encoder_getposition();
  

  // If the startup procedure was initialized, go there
  if (beginStartupCommand) {
    state = 0;
    beginStartupCommand = false;
  }

  // If sine button was pressed, go there
  if (motorSineMoveCommand) {
    if (startupComplete) {
      state = 2;
    } else {
      Serial.println("Please complete startup procedure");
    }
    motorSineMoveCommand = false;
  }

  // If SD button was pressed, go there
  if (motorSDMoveCommand) {
    if (startupComplete) {
      state = 3;
    } else {
      Serial.println("Please complete startup procedure");
    }
    motorSDMoveCommand = false;
  }

  // If vacuum pull button was pressed, go there
  if (vacuumPullStartCommand) {
    if (startupComplete) {
      state = 4;
    } else {
      Serial.println("Please complete startup procedure");
    }
    vacuumPullStartCommand = false;
  }

  // If jog panel was activated, go there
  if (toggleJogOnOff){
    state = 5;
    toggleJogOnOff = false;
  }

  // Delay so we don't spam the message log too much
  delay(500);
}
