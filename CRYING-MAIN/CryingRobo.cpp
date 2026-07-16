#include "CryingRobo.h"
#include <math.h>

int eyeRadius = 14;
int pupilRadius = 5;
int leftEyeX = 36;
int rightEyeX = 92;
int eyeY = 26;

CryingRobo::CryingRobo(Adafruit_SH1106G* display) {
  this->display = display;
}

void CryingRobo::drawEyesWithTears(int pupilOffsetX, int pupilOffsetY, int tearY) {
  display->fillCircle(leftEyeX, eyeY, eyeRadius, SH110X_WHITE);
  display->fillCircle(rightEyeX, eyeY, eyeRadius, SH110X_WHITE);
  display->fillCircle(leftEyeX + pupilOffsetX, eyeY + pupilOffsetY, pupilRadius, SH110X_BLACK);
  display->fillCircle(rightEyeX + pupilOffsetX, eyeY + pupilOffsetY, pupilRadius, SH110X_BLACK);
  display->fillCircle(leftEyeX, tearY, 2, SH110X_WHITE);
  display->fillCircle(rightEyeX, tearY, 2, SH110X_WHITE);
}

void CryingRobo::drawSadMouth() {
  for (int angle = 30; angle <= 150; angle++) {
    float rad = angle * 3.1416 / 180.0;
    int x = 64 + 12 * cos(rad);
    int y = 50 - 4 * sin(rad);
    display->drawPixel(x, y, SH110X_WHITE);
  }
}

void CryingRobo::show() {
  int tearY;
  int pupilOffsetFrame = 0;

  for (int cycle = 0; cycle < 3; cycle++) {
    for (tearY = eyeY + 10; tearY <= 60; tearY += 3) {
      display->clearDisplay();
      int pupilOffsetX = (pupilOffsetFrame % 2 == 0) ? 0 : 1;
      int pupilOffsetY = (pupilOffsetFrame % 3 == 0) ? 1 : 0;
      pupilOffsetFrame++;

      drawEyesWithTears(pupilOffsetX, pupilOffsetY, tearY);
      drawSadMouth();
      display->display();
      delay(150);
    }
    delay(300);
  }

  display->clearDisplay();
  drawEyesWithTears(0, 0, 60);
  drawSadMouth();
  display->display();
  delay(2000);
}
