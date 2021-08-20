/************************************************************************************
  Function responsible for generating motor commands for PID position signal tracking
  Also sends tracked signals to the motor control slave arduino over I2C

  Walker Nelson
  2021.8.14
************************************************************************************/
void trackpositionsignal() {
  // Serial debugging
  //Serial.println("Executing function:     trackpositionsignal");

  // PID function has internal update timer & only does calculations at a specified interval
  // Always run the function, but only send commands to motor if the calculations were completed
  if (myPID.Compute()) {
    // Smooth the value a bit
    moveSpeed = movingaverage(moveSpeed);
    // Check if we are far enough away from our target position
    if (abs(motorPosSetPoint - currentPosition) > minPositionErrToMove) {
      // Current PID scheme outputs motor velocity, but the motor works with speed and direction -- translate
      if (moveSpeed >= 0) {
        targetPosition = veryFar;
      } else {
        moveSpeed = -1 * moveSpeed;
        targetPosition = -1 * veryFar;
      }
    } else {
      moveSpeed = 0;
    }
    // Send command to motor
    stepper_movecommand();
  }
}

// Function for doing a moving average on a value which comes out of the PID controller -- hopefully smoothes things out a bit
float movingaverage(float newVal) {
  // Store the new value
  movAvgBuff[movAvgIndex] = newVal;
  // Increment the counter like a circular buffer
  movAvgIndex++;
  if (movAvgIndex == numSamplesToAvg) {
    movAvgIndex = 0;
  }

  // Take the average value
  float posSum = 0;
  for (int ii = 0; ii < numSamplesToAvg; ii++) {
    posSum += movAvgBuff[ii];
  }
  posSum = posSum / numSamplesToAvg;
  return posSum;
}
