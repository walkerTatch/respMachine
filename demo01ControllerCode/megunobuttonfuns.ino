/****************************************************************************************************************
  This is a set of functions which are called when

  Walker Nelson
  2021.8.14
****************************************************************************************************************/

/******
  Global:
******/
// Helper function resets the start commands -- call when exiting a state so new commands don't start immediately
void resetstartbuttonpresses() {
  // Jog
  toggleJogOnOff = false;
  motorHomeCommand = false;
  motorZeroCommand = false;
  motorJogMoveCommand = false;
  // Startup
  beginStartupCommand = false;
  // Sine
  motorSineMoveCommand = false;
  // SD
  motorSDMoveCommand = false;
  // Vacuum
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
void cmd_beginstartup() {
  // Serial debugging
  //Serial.println("Begin startup procedure command received";
  beginStartupCommand = true;
}

// Startup procedure begin button
void cmd_valveopenconfirmation() {
  // Serial debugging
  //Serial.println("Valve open confirmation command received";
  valveOpenedConfirmation = true;
}

// Startup procedure begin button
void cmd_valveclosedconfirmation() {
  // Serial debugging
  //Serial.println("Begin startup procedure command received";
  valveClosedConfirmation = true;
}

/*********
  Jog Panel:
*********/
// Manual control on/off toggle
void cmd_jogonoff(CommandParameter &Parameters) {
  // Serial debugging
  Serial.println("Manual control on/off switch toggled");
  toggleJogOnOff = true;
}

// Start jog move button
void cmd_startjog(CommandParameter &Parameters) {
  // Serial debugging
  Serial.println("Motor jog move command received");
  jogDistManual = Parameters.NextParameterAsDouble();
  jogSpeedManual = Parameters.NextParameterAsDouble();
  jogAccelManual = Parameters.NextParameterAsDouble();
  motorJogMoveCommand = true;
}

// Motor zero button
void cmd_requestmotorzero(CommandParameter &Parameters) {
  // Serial debugging
  Serial.println("Motor zero request received");
 motorZeroCommand = true;   
}

// Encoder zero button
void cmd_requestencoderzero(CommandParameter &Parameters) {
  // Serial debugging
  Serial.println("Motor zero request received");
 encoderZeroCommand = true;   
}

// Home button
void cmd_requesthome(CommandParameter &Parameters) {
  // Serial debugging
  Serial.println("Homing request received");
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
void cmd_startvacuumpull(CommandParameter &Parameters) {
  // Serial debuging
  //Serial.println("Vacuum pull start command received");
  vacuumPullStartCommand = true;
}

// Stop vac pull button
void cmd_stopvacuumpull(CommandParameter &Parameters) {
  // Serial debuging
  //Serial.println("Vacuum pull start command received");
  vacuumPullStopCommand = true;
}
