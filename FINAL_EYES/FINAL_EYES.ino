#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <FluxGarage_RoboEyes.h>
roboEyes roboEyes;

enum Mode {
  IDLE,
  EMOTION
};

Mode currentMode = IDLE;
unsigned long modeStartTime = 0;

void setup() {
  Serial.begin(115200);

  if (!display.begin(0x3C, true)) {
    Serial.println(F("SH1106 allocation failed"));
    while (1);
  }

  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);

  roboEyes.setWidth(36, 36);
  roboEyes.setHeight(36, 36);
  roboEyes.setBorderradius(8, 8);
  roboEyes.setSpacebetween(10);

  modeStartTime = millis();
  currentMode = IDLE;
  Serial.println("Starting in IDLE mode");
}

void triggerRandomEmotion() {
  int r = random(0, 4);

  switch (r) {
    case 0:
      roboEyes.setMood(HAPPY);
      roboEyes.anim_laugh();
      roboEyes.setVFlicker(ON, 2);
      Serial.println("Mood: HAPPY");
      break;
    case 1:
      roboEyes.setMood(TIRED);
      roboEyes.blink();
      Serial.println("Mood: TIRED");
      break;
    case 2:
      roboEyes.setMood(ANGRY);
      roboEyes.setHFlicker(ON, 2);
      roboEyes.setVFlicker(ON, 2);
      Serial.println("Mood: ANGRY");

      break;
    case 3:
      roboEyes.setCuriosity(ON);
      Serial.println("Mood: CURIOUS");
      break;
  }
}

void resetToIdleMode() {
  roboEyes.setMood(DEFAULT);
  roboEyes.setCuriosity(OFF);
  roboEyes.setHFlicker(OFF);
  roboEyes.setVFlicker(OFF);
  Serial.println("Back to IDLE mode");
}

void loop() {
  roboEyes.update();
  unsigned long now = millis();

  if (currentMode == IDLE) {
    if (now - modeStartTime >= 30000) { // 60 seconds in IDLE
      triggerRandomEmotion();
      currentMode = EMOTION;
      modeStartTime = now;
    }
  }
  else if (currentMode == EMOTION) {
    if (now - modeStartTime >= 15000) { // 15 seconds in EMOTION
      resetToIdleMode();
      currentMode = IDLE;
      modeStartTime = now;
    }
  }
}
