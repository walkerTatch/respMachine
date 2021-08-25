/*******************************************************************************************************************
  Manual movement mode of the machine -- user can move the piston as they please, home the motor, and set zeros etc.
  Motor position then tracks the sine wave

  Walker Nelson
  2021.8.14
*******************************************************************************************************************/
void jogfun(){
  // Serial debugging
  //Serial.println("Executing Function:     jogfun");

  // Do the startup routine for this state if we weren't in it last loop
  if (prevState != state) {
    // Turn on the indicator
    MyPanel.SetIndicator(F("motorOnIndicator"),true);
    // Reset button commands
    resetstartbuttonpresses();
    // Reset the time
    lastSampleTime = micros();
    
  }

  // Get the time
  timeNow = micros();
  
  // Read data and update readouts on a timer
  if (timeNow - lastSampleTime > plotUpdateIntUs){
    // Get positions
    stepper_getposition();
    encoder_getposition();
    lastSampleTime = timeNow;
  }
  // If there is a stop, do that
  if (motorStopCommand){
    stepper_stop();
    motorStopCommand = false;
  }
  
  // If we have a move, do it!
  if (motorJogMoveCommand) {
    // Have to do some juggling because the non-blocking move command isn't great and is linked to constant variables
    motorJogMoveCommand = false;
    float oldAccel = moveAccel;
    moveAccel = jogAccelManual;
    moveSpeed = jogSpeedManual;
    targetPosition = currentPosition + jogDistManual;
    stepper_movecommand();
    moveAccel = oldAccel;
  }

  // If we have a zero, do it!
  // Motor
  if (motorZeroCommand){
    // Make the user do the startup procedure again
    startupComplete = false;
    // Zero the stepper
    motorZeroCommand = false;
    stepper_zero();
  }
  // Encoder
  if (encoderZeroCommand){
    encoderZeroCommand = false;
    encoder_zero();
  }  

  // If we have a home, do it!
  if (motorHomeCommand){
    motorHomeCommand = false;
    stepper_home();
  }

  // If the user turned off the jog functionality -- exit the state and turn off the indicator
  if (toggleJogOnOff){
    // Turn off the indicator
    MyPanel.SetIndicator(F("motorOnIndicator"),false);
    // Return to idle
    state = 1;
    // Reset button presses
    resetstartbuttonpresses(); 
  }  
}
