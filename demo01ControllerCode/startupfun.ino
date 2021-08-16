/************************************************************************************
  Mechanical setup for the machine -- takes care of homing, filling the membrane etc.
  
  Walker Nelson
  2021.8.14
************************************************************************************/

void startupfun(){
  // Temp message before everything is set up
  Serial.println("Startup function not written yet. There's no rules");
  delay(2000);
  state = 1;

  // Reeset the slave arduino
  Serial.println("Resetting motor control");
  stepper_reset();
  delay(1000);

  // Home the stepper motor
  //Serial.println("Homing motor");
  stepper_home();
  delay(5000);
  // Move back to membrane zero
  //Serial.println("Moving to piston zero");
  //movepistontomembranezero();
  //delay(5000);

  // Actual function flow is as follows:

  // Ask user if membrane valve is open -- wait until confirmation

  // Home piston position

  // Tell user to close valve -- wait for confirmation

  // Move to starting piston position

  // Re-zero motor position
  
}
