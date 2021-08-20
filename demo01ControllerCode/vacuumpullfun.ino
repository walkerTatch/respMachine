/*****************************************************************************************
  Function for vacuum pulling
  Moves motor to the max extension of the piston (or whatever the target is)
  This will create a vacuum inside the membrane and pull it over some block resting inside

  Motor is returned to prior position at a jog speed afterwards

  Other than that, doesn't do much...
  
  Walker Nelson
  2021.8.14
*****************************************************************************************/
void vacuumpullfun(){
  // Serial debugging
  //Serial.println("Executing Function:     vacuumpullfun");

  // If this the first cycle of this state, move to the vacuum position
  if (prevState != state) {
      movepistontovacuumposition();
  }

  delay(50);
  // If the user stops the pull, go back to the normal position, return to the idle state, and reset the button presses
  if (vacuumPullStopCommand){
    vacuumPullStopCommand = false;
    movepistontomembranezero();
    state = 1;
    resetstartbuttonpresses();
  }
  
}
