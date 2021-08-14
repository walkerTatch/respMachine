/************************************************************************************
  Mechanical setup for the machine -- takes care of homing, filling the membrane etc.
  
  Walker Nelson
  2021.8.14
************************************************************************************/

void startupfun(){
  // Temp message before everything is set up
  Serial.println("Startup function not written yet. There's no rules!!!!!!");
  delay(2000);
  state = 1;

  // Actual function flow is as follows:

  // Ask user if membrane valve is open -- wait until confirmation

  // Home piston position

  // Tell user to close valve -- wait for confirmation

  // Move to starting piston position

  // Re-zero motor position
  
}
