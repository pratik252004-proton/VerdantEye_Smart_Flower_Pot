#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "HeartEyesRobo.h"

Adafruit_SH1106G display(128, 64, &Wire);
HeartEyesRobo robo(&display);

void setup() {
  display.begin(0x3C, true);
  display.clearDisplay();
}

void loop() {
  robo.show(); // One function call to animate
  delay(500);
}
