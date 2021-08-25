/********************************************************************************************
  Functions for dealing with I2C commands back and forth from the slave motor control arduino

  Walker Nelson
  2021.8.14
********************************************************************************************/

// First is the function to read from the motor control arduino

// Get the motor position
void stepper_getposition() {
  // Serial debugging
  //Serial.println("Executing Function:     stepper_getposition");
  // Ask for the position (float) and the "moveDone" byte
  Wire.requestFrom(9, 5);
  // Get the float
  for (int ii = 0; ii < 4; ii++) {
    currPosPtr[ii] = Wire.read();
  }
  // Get the moveDone byte
  blockingMoveDone = Wire.read();
  // Send to PC
  if (updateJogPanelCoordinates) {
    MyPanel.SetText(F("motorPos"), currentPosition);
  }
}

// Then commands which can be sent to the slave arduino are defined

// Move command
void stepper_movecommand() {
  // Serial debugging
  //Serial.println("Executing Function:     stepper_movecommand");
  Wire.beginTransmission(9);
  // Send the correct command identifier
  byte cmdByte = 3;
  Wire.write(cmdByte);
  // Send the data
  for (byte i = 0; i < sizeof(float); i++) {
    Wire.write(targetPosPtr[i]);
  }
  for (byte i = 0; i < sizeof(float); i++) {
    Wire.write(moveSpeedPtr[i]);
  }
  for (byte i = 0; i < sizeof(float); i++) {
    Wire.write(moveAccelPtr[i]);
  }
  byte stat = Wire.endTransmission(9);
  if (stat != 0) {
    Serial.println("Error Sending Packet to Slave in Flex Move");
  }
}

// Blocking Move Command
void stepper_moveandwaitcommand(byte* targetPos, byte* movSpd, byte* movAcc) {
  Wire.beginTransmission(9);
  // Send the correct command identifier
  byte cmdByte = 3;
  Wire.write(cmdByte);
  // Send the data
  for (byte i = 0; i < sizeof(float); i++) {
    Wire.write(targetPos[i]);
  }
  for (byte i = 0; i < sizeof(float); i++) {
    Wire.write(movSpd[i]);
  }
  for (byte i = 0; i < sizeof(float); i++) {
    Wire.write(movAcc[i]);
  }
  byte stat = Wire.endTransmission(9);
  if (stat != 0) {
    Serial.println("Error Sending Packet to Slave in Blocking Move");
  }
  // Wait for the return byte from the motor control arduino
  if (!(waitforunblockingbyte())) {
    Serial.println("Motor controller timeout on home command");
  }
}

// Reset command
void stepper_reset() {
  // Serial debugging
  //Serial.println("Executing Function:     stepper_reset");
  // Just have to send the correct command byte, no params necessary
  Wire.beginTransmission(9);
  byte cmdByte = 0;
  Wire.write(cmdByte);
  Wire.endTransmission(9);
}

// Zero command
void stepper_zero() {
  // Serial debugging
  //Serial.println("Executing Function:     stepper_zero");
  // Just have to send the correct command byte, no params necessary
  Wire.beginTransmission(9);
  byte cmdByte = 1;
  Wire.write(cmdByte);
  Wire.endTransmission(9);
}

// Reset command
void stepper_stop() {
  // Serial debugging
  //Serial.println("Executing Function:     stepper_stop");
  // Just have to send the correct command byte, no params necessary
  Wire.beginTransmission(9);
  byte cmdByte = 2;
  Wire.write(cmdByte);
  Wire.endTransmission(9);
}

// Home command
void stepper_home() {
  // Serial debugging
  //Serial.println("Executing Function:     stepper_home");
  // Just have to send the correct command byte, no params necessary
  Wire.beginTransmission(9);
  byte cmdByte = 4;
  Wire.write(cmdByte);
  Wire.endTransmission(9);
  // Wait for the return byte from the motor control arduino
  if (!waitforunblockingbyte()) {
    Serial.println("Motor controller timeout on home command");
  }
}

// Helper function to wait for a return byte from a blocking command on the motor control slave
bool waitforunblockingbyte() {
  // Define some timeout params
  uint32_t startMs = millis();
  bool timeout = false;
  bool moveDone = false;
  // Reset blockingMoveDone
  blockingMoveDone = 0;
  // Continuously poll
  while (!moveDone && !timeout) {
    // Small delay
    delay(50);
    // Read position and move status
    stepper_getposition();
    // If the move status is done, exit the loop
    if (blockingMoveDone == 1) {
      moveDone = true;
    }
    // If it has taken too long, exit the loop
    if ((millis() - startMs) > blockingFunctionTimeoutMs) {
      timeout = true;
    }
  }
  return moveDone;
}
