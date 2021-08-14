/********************************************************************************
  Requests a packet from the motor control slave arduino
  Specifically, it requests the current motor position as a floating point number

  Walker Nelson
  2021.8.14
********************************************************************************/
void getpacketfromslave() {
  // Serial debugging
  //Serial.println("Executing Function:     getpacketfromslave");
  Wire.requestFrom(9, 4);
  int ii = 0;
  while (Wire.available() > 0) {
    currPosPtr[ii] = Wire.read();
    ii++;
  }
}
