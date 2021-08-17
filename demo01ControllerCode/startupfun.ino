/************************************************************************************
  Mechanical setup for the machine -- takes care of homing, filling the membrane etc.

  Walker Nelson
  2021.8.14
************************************************************************************/

void startupfun() {
  // Serial debugging
  //Serial.println("Executing Function:     startupfun);

  // If this the first cycle of this state, reset the procedure
  if (prevState != state) {
    // Make controls invisible
    resetstartuppanelindicators();

    // Reset variables
    valveOpenedConfirmation = false;
    valveJustOpened = true;
    valveClosedConfirmation = false;
    motorHomed = false;
    motorJustHomed = true;

    // Make the valve open message and buttons appear
    delay(1000);
    MyPanel.ShowControl(F("valveOpenWarning"));
    delay(1000);
    MyPanel.ShowControl(F("valveOpen"));
  }

  // User pushed the first button -- display the next steps and home
  if (valveOpenedConfirmation) {
    // Valve has been opened -- wait a bit and display the homing message if it was just opened
    if (valveJustOpened) {
      delay(1000);
      MyPanel.ShowControl(F("homingMessage"));
      valveJustOpened = false;
      delay(1000);
      motorHomeTime = micros();
      stepper_home();
    }
  }

  // For now, just wait a bit until the motor is homed
  timeNow = micros();
  if (valveOpenedConfirmation && (timeNow - motorHomeTime >= 5000000)) {
    motorHomed = true;
  }

  // Once the motor is homed, display the next buttons/warnings
  if (motorHomed && motorJustHomed) {
    // Move the motor to the fill position
    MyPanel.HideControl(F("fillPosMessage"));
    movepistontofillposition();
    // Ask the user to close the valve
    MyPanel.ShowControl(F("valveCloseWarning"));
    MyPanel.ShowControl(F("valveClosed"));
    motorJustHomed = false;
  }

  // Finally, once they press the last button, wait a bit and then display the done message
  if (motorHomed && valveClosedConfirmation) {
    // Wait a second and then move the piston to the correct neutral position
    delay(1000);
    movepistontomembranezero();
    // Then show that we are done
    MyPanel.ShowControl(F("startupFinishedMsg"));
    // Clear the errant "start" button presses
    motorSineMoveCommand = false;
    motorSDMoveCommand = false;
    // Flag the startup as complete, move back to the idle state, and reset button presses
    startupComplete = true;
    state = 1;
    resetstartbuttonpresses();
  }

  delay(500);
}

// Function resets all of the startup panel indicators which need to be blank
void resetstartuppanelindicators(){
 MyPanel.HideControl(F("valveOpenWarning"));
    MyPanel.HideControl(F("valveOpen"));
    MyPanel.HideControl(F("homingMessage"));
    MyPanel.HideControl(F("fillPosMessage"));
    MyPanel.HideControl(F("valveCloseWarning"));
    MyPanel.HideControl(F("valveClosed"));
    MyPanel.HideControl(F("startupFinishedMsg"));  
}
