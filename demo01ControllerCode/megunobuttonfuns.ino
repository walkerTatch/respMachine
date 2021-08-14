/****************************************************************************************************************
  This is a set of functions which are called when 

  Walker Nelson
  2021.8.14
****************************************************************************************************************/
// Home button
void cmd_requesthome(CommandParameter &Parameters){
  // Serial debugging
  //Serial.println("Homing request received");
  motorHomeCommand = true;
}

// Stop move button
void cmd_stopmove(CommandParameter &Parameters) {
  // Serial debugging
  //Serial.println("Motor stop command received");
  motorStopCommand = true;
}
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

// Start sine move button
void cmd_startsine(CommandParameter &Parameters) {
  // Serial debugging
  //Serial.println("Motor sine move command received");
  sineAmp = Parameters.NextParameterAsDouble();
  sineFreqHz = Parameters.NextParameterAsDouble();
  motorSineMoveCommand = true;
}
