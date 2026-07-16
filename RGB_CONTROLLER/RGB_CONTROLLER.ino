/*************************************************************
 * RGB LED Controller with Blynk + 2 Extra LEDs
 * Hardware: ESP32, RGB LED + 2 Single LEDs
 *************************************************************/
#define BLYNK_TEMPLATE_ID "TMPL320nKLAs9"
#define BLYNK_TEMPLATE_NAME "RGB Controller"
#define BLYNK_AUTH_TOKEN "KGxre5lVzSQXFOigI3EvuhqCoJf_2t5A"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// WiFi credentials
char ssid[] = "Hacked";
char pass[] = "25june2004";

// RGB LED Pins (PWM-capable pins)
const int RED_PIN   = 25;
const int GREEN_PIN = 26;
const int BLUE_PIN  = 27;

// Extra LEDs
const int LED1_PIN = 14; // Change if needed
const int LED2_PIN = 12; // Change if needed

// PWM Channels
#define RED_CHANNEL   0
#define GREEN_CHANNEL 1
#define BLUE_CHANNEL  2

// Blynk handlers for RGB sliders
BLYNK_WRITE(V3) {
  int redVal = param.asInt();
  ledcWrite(RED_CHANNEL, redVal);
}

BLYNK_WRITE(V4) {
  int greenVal = param.asInt();
  ledcWrite(GREEN_CHANNEL, greenVal);
}

BLYNK_WRITE(V5) {
  int blueVal = param.asInt();
  ledcWrite(BLUE_CHANNEL, blueVal);
}

// Button for LED1
BLYNK_WRITE(V6) {
  int state = param.asInt();
  digitalWrite(LED1_PIN, state);
}

// Button for LED2
BLYNK_WRITE(V7) {
  int state = param.asInt();
  digitalWrite(LED2_PIN, state);
}

void setup() {
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
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
  Blynk.run();
}
