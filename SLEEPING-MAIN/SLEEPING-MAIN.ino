#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "SleepingRobo.h"

Adafruit_SH1106G display(128, 64, &Wire);
SleepingRobo robo(&display); // Pass display object

void setup() {
  display.begin(0x3C, true);
  display.clearDisplay();
}

void loop() {
  robo.show(); // Only one function call
  delay(1000);
}
