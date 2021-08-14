/********************************************************************************
  Sends a packet to the motor control slave arduino
  Specifically, it sends the desired move parameters as floats:
  - Target position
  - Move speed
  - Move accel

  Walker Nelson
  2021.8.14
********************************************************************************/
void sendpackettoslave() {
  // Serial debugging
  //Serial.println("Executing Function:     sendpackettoslave");
  Wire.beginTransmission(9);
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
