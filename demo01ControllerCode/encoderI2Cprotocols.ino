/*******************************************************************************************
  Functions for dealing with I2C commands back and forth from the slave encoder read arduino

  Walker Nelson
  2021.8.20
*******************************************************************************************/

// First is the function to read from the encoder arduino

// Get the encoder position (in mm)
void encoder_getposition() {
  // Serial debugging
  //Serial.println("Executing Function:     encoder_getposition");
  Wire.requestFrom(8, 4);
  int ii = 0;
  while (Wire.available() > 0) {
    membPosPtr[ii] = Wire.read();
    ii++;
  }
}

// Then commands which can be sent to the slave arduino are defined

// Zero command
void encoder_zero() {
  // Serial debugging
  //Serial.println("Executing Function:     encoder_zero");
  // Just have to send the correct command byte, no params necessary
  Wire.beginTransmission(8);
  byte cmdByte = 1;
  Wire.write(cmdByte);
  Wire.endTransmission(8);
}
