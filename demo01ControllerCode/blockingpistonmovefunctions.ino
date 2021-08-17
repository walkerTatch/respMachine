/***************************************************************************************************
  Functions are responsible for moving the piston to specific physical positions which are important
  for the setup

  All of these are blocking functions!

  Walker Nelson
  2021.8.16
***************************************************************************************************/
// Membrane zero
void movepistontomembranezero() {
  // Serial debugging
  Serial.println("Executing Function:     movepistontomembranezero");
  stepper_moveandwaitcommand((byte*)&membraneZeroPosition,(byte*)&jogSpeed,(byte*)&jogAccel);
}

// Fill position movement
void movepistontofillposition() {
  // Serial debugging
  Serial.println("Executing Function:     movepistontofillposition");
  stepper_moveandwaitcommand((byte*)&membraneFillPosition,(byte*)&jogSpeed,(byte*)&jogAccel);
}

// Vacuum pull position
void movepistontovacuumposition() {
  // Serial debugging
  Serial.println("Executing Function:     movepistontovacuumposition");
  stepper_moveandwaitcommand((byte*)&membraneVacuumPosition,(byte*)&jogSpeed,(byte*)&jogAccel);
}
