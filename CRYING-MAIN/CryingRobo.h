#ifndef CRYING_ROBO_H
#define CRYING_ROBO_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

class CryingRobo {
public:
  CryingRobo(Adafruit_SH1106G* display); // Pass pointer to display object
  void show(); // This is the only function needed in main sketch

private:
  Adafruit_SH1106G* display;

  void drawEyesWithTears(int pupilOffsetX, int pupilOffsetY, int tearY);
  void drawSadMouth();
};

#endif
