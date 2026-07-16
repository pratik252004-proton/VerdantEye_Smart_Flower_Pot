#include "RGBController.h"

// Pin definitions
const int RGBController::RED_PIN   = 25;
const int RGBController::GREEN_PIN = 26;
const int RGBController::BLUE_PIN  = 27;
const int RGBController::LED1_PIN  = 32;
const int RGBController::LED2_PIN  = 14;

// PWM channels
const int RGBController::RED_CHANNEL   = 0;
const int RGBController::GREEN_CHANNEL = 1;
const int RGBController::BLUE_CHANNEL  = 2;

// Blynk handlers mapping to library functions
BLYNK_WRITE(V3) { RGBController::setRed(param); }
BLYNK_WRITE(V4) { RGBController::setGreen(param); }
BLYNK_WRITE(V5) { RGBController::setBlue(param); }
BLYNK_WRITE(V6) { RGBController::setLED1(param); }
BLYNK_WRITE(V7) { RGBController::setLED2(param); }

RGBController::RGBController(const char* wifiSsid, const char* wifiPass, const char* blynkToken) {
  _ssid = wifiSsid;
  _pass = wifiPass;
  _auth = blynkToken;
}

void RGBController::begin() {
  Serial.begin(115200);

  // RGB LED setup
  ledcSetup(RED_CHANNEL, 5000, 8);
  ledcAttachPin(RED_PIN, RED_CHANNEL);

  ledcSetup(GREEN_CHANNEL, 5000, 8);
  ledcAttachPin(GREEN_PIN, GREEN_CHANNEL);

  ledcSetup(BLUE_CHANNEL, 5000, 8);
  ledcAttachPin(BLUE_PIN, BLUE_CHANNEL);

  // Extra LEDs setup
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);

  // Start Blynk
  Blynk.begin(_auth, _ssid, _pass);
}

void RGBController::run() {
  Blynk.run();
}

void RGBController::setRed(BlynkParam param) {
  ledcWrite(RED_CHANNEL, param.asInt());
}

void RGBController::setGreen(BlynkParam param) {
  ledcWrite(GREEN_CHANNEL, param.asInt());
}

void RGBController::setBlue(BlynkParam param) {
  ledcWrite(BLUE_CHANNEL, param.asInt());
}

void RGBController::setLED1(BlynkParam param) {
  digitalWrite(LED1_PIN, param.asInt());
}

void RGBController::setLED2(BlynkParam param) {
  digitalWrite(LED2_PIN, param.asInt());
}
