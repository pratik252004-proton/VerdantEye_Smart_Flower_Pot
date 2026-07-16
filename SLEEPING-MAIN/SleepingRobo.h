#ifndef SLEEPING_ROBO_H
#define SLEEPING_ROBO_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

class SleepingRobo {
public:
  SleepingRobo(Adafruit_SH1106G* display); // Constructor
  void show();                             // Public function to call in loop

private:
  Adafruit_SH1106G* display;

  void drawArcSimulated(int x, int y, int r);
  void drawSleepyFace(bool openMouth);
  void drawZs(int stage);
};

#endif
