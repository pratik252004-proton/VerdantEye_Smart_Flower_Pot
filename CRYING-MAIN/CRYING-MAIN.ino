#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "CryingRobo.h"

Adafruit_SH1106G display(128, 64, &Wire);
CryingRobo cryingRobo(&display); // Pass pointer to display object

void setup() {
  display.begin(0x3C, true);
  display.clearDisplay();
}

void loop() {
  cryingRobo.show(); // Only one function call needed
  delay(1000);
}
