/**************************************************************************************************
  Function responsible for moving the piston back to the membrane zero after/before other movements

  Really all tracking movement functions should end here anyway, but it doesn't hurt to be save

  This is a blocking function!

  Walker Nelson
  2021.8.14
**************************************************************************************************/


void movepistontomembranezero() {
  // Serial debugging
  Serial.println("Executing Function:     movepistontomembranezero");

  // Store old values of move speed and accel
  float oldMoveSpeed = moveSpeed;
  float oldMoveAccel = moveAccel;

  // Assign new values of move speed and accel, then send move command to motor
  moveAccel = jogAccel;
  moveSpeed = jogSpeed;
  targetPosition = membraneZeroPosition;
  stepper_movecommand();

  // Wait a hot sec
  delay(2000);

  // Restore old values for accel and move speed
  moveSpeed = oldMoveSpeed;
  moveAccel = oldMoveAccel;
}
