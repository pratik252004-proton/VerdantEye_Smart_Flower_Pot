#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <math.h>

// I2C OLED setup for ESP32
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire);

int leftEyeX = 40;
int rightEyeX = 88;
int eyeY = 20;

void drawArcSimulated(int x, int y, int r) {
  for (int i = -r; i <= r; i++) {
    int j = round(sqrt(r * r - i * i));
    display.drawPixel(x + i, y + j, SH110X_WHITE);
  }
}

void drawSleepyFace(bool openMouth) {
  drawArcSimulated(leftEyeX, eyeY, 10);
  drawArcSimulated(rightEyeX, eyeY, 10);

  if (openMouth) {
    display.fillCircle(64, 50, 8, SH110X_WHITE);      // Mouth outer
    display.fillCircle(64, 50, 4, SH110X_BLACK);      // Inner dark
  } else {
    display.drawLine(58, 50, 70, 50, SH110X_WHITE);   // Closed mouth
  }
}

void drawZs(int stage) {
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setFont(); // Use default font

  if (stage >= 1) display.setCursor(100, 44), display.print("Z");
  if (stage >= 2) display.setCursor(108, 36), display.print("Z");
  if (stage >= 3) display.setCursor(116, 28), display.print("Z");
  if (stage >= 4) display.setCursor(122, 20), display.print("Z");
}

void showSleepingRobo() {
  for (int i = 1; i <= 4; i++) {
    display.clearDisplay();
    drawSleepyFace(true);
    drawZs(i);
    display.display();
    delay(500);
  }

  display.clearDisplay();
  drawSleepyFace(false);
  drawZs(4);
  display.display();
  delay(2000);
}

void setup() {
  display.begin(0x3C, true);  // Address 0x3C for most SH1106 displays
  display.clearDisplay();
}

void loop() {
  showSleepingRobo();
  delay(1000);
}
