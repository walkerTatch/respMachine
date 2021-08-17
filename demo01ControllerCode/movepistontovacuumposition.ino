/**************************************************************************************************
  Function responsible for moving the piston to the vacuum position

  This is a blocking function!

  Walker Nelson
  2021.8.16
**************************************************************************************************/


void movepistontovacuumposition() {
  // Serial debugging
  Serial.println("Executing Function:     movepistontovacuumposition");

  // Store old values of move speed and accel
  float oldMoveSpeed = moveSpeed;
  float oldMoveAccel = moveAccel;

  // Assign new values of move speed and accel, then send move command to motor
  moveAccel = jogAccel;
  moveSpeed = jogSpeed;
  targetPosition = membraneVacuumPosition;
  stepper_movecommand();

  // Wait a hot sec
  delay(4000);

  // Restore old values for accel and move speed
  moveSpeed = oldMoveSpeed;
  moveAccel = oldMoveAccel;
}
