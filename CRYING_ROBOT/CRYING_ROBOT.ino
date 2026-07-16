
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <math.h>

// Initialize SH1106 OLED
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire);

// Eye positions and sizes
int eyeRadius = 14;
int pupilRadius = 5;
int leftEyeX = 36;
int rightEyeX = 92;
int eyeY = 26;

// Draw eyes, pupils, and tears
void drawEyesWithTears(int pupilOffsetX, int pupilOffsetY, int tearY) {
  // Eye whites
  display.fillCircle(leftEyeX, eyeY, eyeRadius, SH110X_WHITE);
  display.fillCircle(rightEyeX, eyeY, eyeRadius, SH110X_WHITE);

  // Pupils (black, animated)
  display.fillCircle(leftEyeX + pupilOffsetX, eyeY + pupilOffsetY, pupilRadius, SH110X_BLACK);
  display.fillCircle(rightEyeX + pupilOffsetX, eyeY + pupilOffsetY, pupilRadius, SH110X_BLACK);

  // Tears (falling)
  display.fillCircle(leftEyeX, tearY, 2, SH110X_WHITE);
  display.fillCircle(rightEyeX, tearY, 2, SH110X_WHITE);
}

// Draw sad arc mouth using pixels
void drawSadMouth() {
  for (int angle = 30; angle <= 150; angle++) {
    float rad = angle * 3.1416 / 180.0;
    int x = 64 + 12 * cos(rad);
    int y = 50 - 4 * sin(rad);
    display.drawPixel(x, y, SH110X_WHITE);
  }
}

// Show crying robot with animated eyes and tears
void showHungryCryingRobo() {
  int tearY;
  int pupilOffsetFrame = 0;

  for (int cycle = 0; cycle < 3; cycle++) {
    for (tearY = eyeY + 10; tearY <= 60; tearY += 3) {
      display.clearDisplay();

      // Slight animation in pupil position
      int pupilOffsetX = (pupilOffsetFrame % 2 == 0) ? 0 : 1;
      int pupilOffsetY = (pupilOffsetFrame % 3 == 0) ? 1 : 0;
      pupilOffsetFrame++;

      drawEyesWithTears(pupilOffsetX, pupilOffsetY, tearY);
      drawSadMouth();

      // Optional message
      // display.setTextSize(1);
      // display.setCursor(30, 10);
      // display.print("I'm Hungry...");

      display.display();
      delay(150);
    }

    delay(300);
  }

  // Final frame (tears at bottom)
  display.clearDisplay();
  drawEyesWithTears(0, 0, 60);
  drawSadMouth();

  // Final message
  // display.setTextSize(1);
  // display.setCursor(24, 10);
  // display.print("Feed me please!");

  display.display();
  delay(2000);
}

void setup() {
  display.begin(0x3C, true); // 0x3C is standard SH1106 I2C address
  display.clearDisplay();
}

void loop() {
  showHungryCryingRobo();
  delay(1000);
}
