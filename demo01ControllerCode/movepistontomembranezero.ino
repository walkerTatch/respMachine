/**************************************************************************************************
  Function responsible for moving the piston back to the membrane zero after/before other movements

  Really all tracking movement functions should end here anyway, but it doesn't hurt to be save
  
  Walker Nelson
  2021.8.14
**************************************************************************************************/


void movepistontomembranezero(){
  // Serial debugging
  //Serial.println("Executing Function:     movepistontomembranezero");

  // Store old values of move speed and accel
  float oldMoveSpeed = moveSpeed;
  float oldMoveAccel = moveAccel;
  
  // Assign new values of move speed and accel, then send move command to motor 
  moveAccel = jogAccel;
  moveSpeed = jogSpeed;
  targetPosition = membraneZeroPosition;
  sendpackettoslave();

  // Restore old values for accel and move speed
  moveSpeed = oldMoveSpeed;
  moveAccel = oldMoveAccel;
}
