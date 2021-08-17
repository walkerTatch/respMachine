/**************************************************************************************************
  Function responsible for moving the piston back to the fill position

  Only really used in startup function

  Walker Nelson
  2021.8.16
**************************************************************************************************/


void movepistontofillposition() {
  // Serial debugging
  Serial.println("Executing Function:     movepistontofillposition");

  // Store old values of move speed and accel
  float oldMoveSpeed = moveSpeed;
  float oldMoveAccel = moveAccel;

  // Assign new values of move speed and accel, then send move command to motor
  moveAccel = jogAccel;
  moveSpeed = jogSpeed;
  targetPosition = membraneFillPosition;
  stepper_movecommand();

  // Wait a hot sec
  delay(2000);

  // Restore old values for accel and move speed
  moveSpeed = oldMoveSpeed;
  moveAccel = oldMoveAccel;
}
