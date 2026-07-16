#ifndef HEART_EYES_ROBO_H
#define HEART_EYES_ROBO_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

class HeartEyesRobo {
public:
  HeartEyesRobo(Adafruit_SH1106G* display); // Constructor
  void show(); // Call this to animate heart eyes

private:
  Adafruit_SH1106G* display;

  void drawScaledHeart(int cx, int cy, int scale);
  void drawSmileMouth();
};

#endif
