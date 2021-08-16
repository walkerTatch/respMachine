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
  Wire.requestFrom(9, 4);
  int ii = 0;
  while (Wire.available() > 0) {
    currPosPtr[ii] = Wire.read();
    ii++;
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
  for (byte i = 0; i < sizeof(double); i++) {
    Wire.write(targetPosPtr[i]);
  }
  for (byte i = 0; i < sizeof(double); i++) {
    Wire.write(moveSpeedPtr[i]);
  }
  for (byte i = 0; i < sizeof(double); i++) {
    Wire.write(moveAccelPtr[i]);
  }
  byte stat = Wire.endTransmission(9);
  if (stat != 0) {
    Serial.println("Error Sending Packet to Slave");
  }
}

// Reset command
void stepper_reset(){
  // Serial debugging
  //Serial.println("Executing Function:     stepper_reset");
  // Just have to send the correct command byte, no params necessary
  Wire.beginTransmission(9);
  byte cmdByte = 0;
  Wire.write(cmdByte);
  Wire.endTransmission(9);  
}

// Reset command
void stepper_zero(){
  // Serial debugging
  //Serial.println("Executing Function:     stepper_zero");
  // Just have to send the correct command byte, no params necessary
  Wire.beginTransmission(9);
  byte cmdByte = 1;
  Wire.write(cmdByte);
  Wire.endTransmission(9);  
}

// Reset command
void stepper_stop(){
  // Serial debugging
  //Serial.println("Executing Function:     stepper_stop");
  // Just have to send the correct command byte, no params necessary
  Wire.beginTransmission(9);
  byte cmdByte = 2;
  Wire.write(cmdByte);
  Wire.endTransmission(9);  
}

// Reset command
void stepper_home(){
  // Serial debugging
  //Serial.println("Executing Function:     stepper_home");
  // Just have to send the correct command byte, no params necessary
  Wire.beginTransmission(9);
  byte cmdByte = 4;
  Wire.write(cmdByte);
  Wire.endTransmission(9);  
}
