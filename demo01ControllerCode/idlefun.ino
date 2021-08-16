/****************************************************************************************************************
  Idle function of the machine -- basically does nothing but wait and allow the state to change to something else
  This is where the user can set parameters like sine tracking stuff, SD file to read etc...

  Walker Nelson
  2021.8.14
****************************************************************************************************************/
void idlefun() {
  // Serial debugging
  Serial.println("No idle function yet");

  // If sine button was pressed, go there
  if (motorSineMoveCommand) {
    state = 2;
    motorSineMoveCommand = false;
  }

  // If SSD button was pressed, go there
  if (motorSDMoveCommand){
    state = 3;
    motorSDMoveCommand = false;
  }

  delay(500);
}
