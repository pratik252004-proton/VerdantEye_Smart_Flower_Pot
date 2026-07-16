#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ESP32WeatherStation.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define led 19

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ESP32WeatherStation weatherStation(&display);

#include <FluxGarage_RoboEyes.h>
roboEyes roboEyes;

// Wi-Fi credentials
const char* SSID = "Hacked";
const char* PASSWORD = "25june2004";

// Weather API
const char* CITY = "Pune";
const char* COUNTRY = "IN";
const char* API_KEY = "b12063ea5512f9cf0a836cf94fe512e6";

enum Mode {
  IDLE,
  EMOTION,
  WEATHER
};

Mode currentMode = IDLE;
unsigned long modeStartTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH); 
  if (!display.begin(0x3C, true)) {
    Serial.println(F("SH1106 allocation failed"));
    while (1);
  }

  // Setup RoboEyes
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);
  roboEyes.setWidth(36, 36);
  roboEyes.setHeight(36, 36);
  roboEyes.setBorderradius(8, 8);
  roboEyes.setSpacebetween(10);

  // Setup Weather Station
  weatherStation.setWiFi(SSID, PASSWORD);
  weatherStation.setLocation(CITY, COUNTRY);
  weatherStation.setAPIKey(API_KEY);
  weatherStation.begin();

  currentMode = IDLE;
  modeStartTime = millis();
  Serial.println("Starting in IDLE mode");
}

void resetToIdleMode() {
  roboEyes.setMood(DEFAULT);
  roboEyes.setCuriosity(OFF);
  roboEyes.setHFlicker(OFF);
  roboEyes.setVFlicker(OFF);
  Serial.println("Back to IDLE mode");
}

void loop() {
  unsigned long now = millis();

  if (currentMode == IDLE) {
    roboEyes.update();

    if (now - modeStartTime >= 30000) {
      int r = random(0, 5); // 0–3 = emotions, 4 = weather

      switch (r) {
        case 0:
          Serial.println("Weather Mode Triggered");
          currentMode = WEATHER;
          break;

        case 1:
          roboEyes.setMood(TIRED);
          roboEyes.blink();
          Serial.println("Mood: TIRED");
          currentMode = EMOTION;
          break;

        case 2:
          roboEyes.setMood(ANGRY);
          roboEyes.setHFlicker(ON, 2);
          roboEyes.setVFlicker(ON, 2);
          Serial.println("Mood: ANGRY");
          currentMode = EMOTION;
          break;

        case 3:
          roboEyes.setCuriosity(ON);
          Serial.println("Mood: CURIOUS");
          currentMode = EMOTION;
          break;

        case 4:
          roboEyes.setMood(HAPPY);
          roboEyes.anim_laugh();
          roboEyes.setVFlicker(ON, 2);
          Serial.println("Mood: HAPPY");
          currentMode = EMOTION;
          break;
      }

      modeStartTime = now;
    }
  }

  else if (currentMode == EMOTION) {
    roboEyes.update();

    if (now - modeStartTime >= 15000) {
      resetToIdleMode();
      currentMode = IDLE;
      modeStartTime = now;
    }
  }

  else if (currentMode == WEATHER) {
    // Keep updating the weather UI repeatedly
    weatherStation.update();  // ← keep calling this in every loop

    if (now - modeStartTime >= 60000) {
      display.clearDisplay();
      display.display();
      currentMode = IDLE;
      modeStartTime = now;
      Serial.println("Returning to IDLE from WEATHER");
    }
  }
}
