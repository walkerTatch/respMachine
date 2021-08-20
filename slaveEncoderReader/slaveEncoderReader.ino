/*
   Function for running the encoder sensor on one slave arduino.
   This arduino (Nano) is responsible for nothing except executing reading the encoder and then reporting this to the master arduino
   Essentially it transforms the encoder into an I2C device
   This arduino will communicate with another arduino over an I2C connection
*/

/*
 * *****************
   Include Libraries
 * *****************
*/
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include <Wire.h>

/*
 * **********************
   Define Library Objects
 * **********************
*/
// Make sure the two pins have interrupt capabilities! -- 2 & 3 for nano
Encoder myEnc(2, 3);
/*
 * ****************
   Setup Parameters
 * ****************
*/
// Numbers & such
long currentPositionTicks = 0;
float currentPositionLinear = 0;
const float mmPerIndex = PI*0.5*25.4/4096;
// Wire transmission things
byte* currPosLinPtr = (byte*)&currentPositionLinear;
byte commandByte;


/*
 * *************
   I2C Functions
 * *************
*/
// Function to run when a value is requested
void requesthandler(){
  currentPositionLinear = currentPositionTicks*mmPerIndex;
  for (byte i = 0; i < sizeof(float); i++) {
    Wire.write(currPosLinPtr[i]);
  }
}

// Function is run when a value is sent
void parsecommand(int numBytes) {
  // Get the command byte
  commandByte = Wire.read();
  // Do a different command depending on what was sent
  switch (commandByte) {
    // Reset the arduino
    case 0:
      resetcommand();
      break;
    // Set position to zero
    case 1:
      zerocommand();
      break;
  }
}

/*
 * ******************************
   Arduino Architecture Functions
 * ******************************
*/
void setup() {
  // Serial debugging
  Serial.begin(115200);
  Serial.println("Welcome to the encoder script!");

  // Connect to I2C
  Serial.println("Connecting to I2C");
  Wire.begin(8);
  Wire.setClock(400000);
  Wire.onReceive(parsecommand);
  Wire.onRequest(requesthandler);
}

// Looping function just reads the encoder as fast as it can
void loop() {
  currentPositionTicks = myEnc.read();
  currentPositionLinear = currentPositionTicks*mmPerIndex;
  Serial.println(currentPositionLinear);
}

/*
 * **********************
   Wire Helper Functions:
 * **********************
*/
// Function resets the arduino --> sometime in the future ;D
void resetcommand() {
  // Serial debugging
  Serial.println("Reset Command Received");
  zerocommand();
}


// Function zeros out the encoder position
void zerocommand() {
  // Serial debugging
  Serial.println("Zero Command Received");
  currentPositionTicks = 0;
}
