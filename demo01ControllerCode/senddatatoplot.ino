/*************************************************************
  Sends data to the megunolink plot -- pretty self explanatory

  Walker Nelson
  2021.8.14
*************************************************************/
void senddatatoplot() {
  // Serial debugging
  //Serial.println("Executing Function:     senddatatoplot");

  // Send stuff
  MyPlot.SendData(F("Current Position"), currentPosition);
  MyPlot.SendData(F("Motor Speed"), moveSpeed);
  MyPlot.SendData(F("Target Position"), motorPosSetPoint);
}
