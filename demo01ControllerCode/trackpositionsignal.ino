/************************************************************************************
  Function responsible for generating motor commands for PID position signal tracking
  Also sends tracked signals to the motor control slave arduino over I2C
  
  Walker Nelson
  2021.8.14
************************************************************************************/
void trackpositionsignal(){
  // Serial debugging
  //Serial.println("Executing function:     trackpositionsignal");
  
  // PID function has internal update timer & only does calculations at a specified interval
  // Always run the function, but only send commands to motor if the calculations were completed
  if (myPID.Compute()) {
    // Current PID scheme outputs motor velocity, but the motor works with speed and direction -- translate
    if (moveSpeed >= 0) {
      targetPosition = veryFar;
    } else {
      moveSpeed = -1*moveSpeed;
      targetPosition = -1 * veryFar;
    }
    // Send command to motor
    sendpackettoslave();
  }
}
