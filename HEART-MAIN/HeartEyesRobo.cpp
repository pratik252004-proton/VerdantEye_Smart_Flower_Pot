#include "HeartEyesRobo.h"
#include <math.h>

int leftEyeX = 44;
int rightEyeX = 84;
int eyeY = 30;

HeartEyesRobo::HeartEyesRobo(Adafruit_SH1106G* display) {
  this->display = display;
}

void HeartEyesRobo::drawScaledHeart(int cx, int cy, int scale) {
  const uint8_t heart[8] = {
    0b01100110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00000000
  };

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (heart[y] & (1 << (7 - x))) {
        int x0 = cx + (x - 4) * scale;
        int y0 = cy + (y - 4) * scale;
        for (int dy = 0; dy < scale; dy++) {
          for (int dx = 0; dx < scale; dx++) {
            display->drawPixel(x0 + dx, y0 + dy, SH110X_WHITE);
          }
        }
      }
    }
  }
}

void HeartEyesRobo::drawSmileMouth() {
  for (int angle = 45; angle <= 135; angle++) {
    float rad = angle * 3.1416 / 180.0;
    int x = 64 + 12 * cos(rad);
    int y = 50 + 4 * sin(rad);
    display->drawPixel(x, y, SH110X_WHITE);
  }
}

void HeartEyesRobo::show() {
  for (int frame = 0; frame < 12; frame++) {
    display->clearDisplay();

    int scale = (frame % 6 < 3) ? (frame % 3 + 1) : (3 - frame % 3);

    drawScaledHeart(leftEyeX, eyeY, scale);
    drawScaledHeart(rightEyeX, eyeY, scale);
    drawSmileMouth();

    display->display();
    delay(100);
  }

  display->clearDisplay();
  drawScaledHeart(leftEyeX, eyeY, 2);
  drawScaledHeart(rightEyeX, eyeY, 2);
  drawSmileMouth();
  display->display();
  delay(1000);
}
