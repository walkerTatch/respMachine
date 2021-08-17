/****************************************************************************************************************
  This is a set of functions which are called when 

  Walker Nelson
  2021.8.14
****************************************************************************************************************/

/******
Global:
******/
// Helper function resets the start commands -- call when exiting a state so new commands don't start immediately
void resetstartbuttonpresses(){
beginStartupCommand = false;
motorSineMoveCommand = false;
motorSDMoveCommand = false;
vacuumPullStartCommand = false;
}

// Stop move button
void cmd_stopmove(CommandParameter &Parameters) {
  // Serial debugging
  //Serial.println("Motor stop command received");
  motorStopCommand = true;
}

/***********************
Startup Procedure Panel:
***********************/
// Startup procedure begin button
void cmd_beginstartup(){
  // Serial debugging
  //Serial.println("Begin startup procedure command received";
  beginStartupCommand = true;
}

// Startup procedure begin button
void cmd_valveopenconfirmation(){
  // Serial debugging
  //Serial.println("Valve open confirmation command received";
  valveOpenedConfirmation = true;
}

// Startup procedure begin button
void cmd_valveclosedconfirmation(){
  // Serial debugging
  //Serial.println("Begin startup procedure command received";
  valveClosedConfirmation = true;
}

/*********
Jog Panel:
*********/

// Start jog move button
void cmd_startmove(CommandParameter &Parameters) {
  // Serial debugging
  //Serial.println("Motor jog move command received");
  moveSpeed = Parameters.NextParameterAsDouble();
  moveAccel = Parameters.NextParameterAsDouble();
  motorJogMoveCommand = true;
}

// Fwd/rev button
void cmd_changedir(CommandParameter &Parameters) {
  // Serial debugging
  //Serial.println("Motor direction flip command received");
  motorJogDirection = !motorJogDirection;
}

// Home button
void cmd_requesthome(CommandParameter &Parameters){
  // Serial debugging
  //Serial.println("Homing request received");
  motorHomeCommand = true;
}

/***************
Sine Wave Panel:
***************/
// Start sine move button
void cmd_startsine(CommandParameter &Parameters) {
  // Serial debugging
  //Serial.println("Motor sine move command received");
  sineAmp = Parameters.NextParameterAsDouble();
  sineFreqHz = Parameters.NextParameterAsDouble();
  motorSineMoveCommand = true;
}

/**************
SD Track Panel:
**************/
// Start sd move button
void cmd_startsd(CommandParameter &Parameters) {
  // Serial debugging
  //Serial.println("Motor SD move command received");
  motorSDMoveCommand = true;
}

/**************
Vac Pull Panel:
**************/
// Start vac pull button
void cmd_startvacuumpull(CommandParameter &Parameters){
  // Serial debuging
  //Serial.println("Vacuum pull start command received");
  vacuumPullStartCommand = true;
}

// Stop vac pull button
void cmd_stopvacuumpull(CommandParameter &Parameters){
  // Serial debuging
  //Serial.println("Vacuum pull start command received");
  vacuumPullStopCommand = true;
}
