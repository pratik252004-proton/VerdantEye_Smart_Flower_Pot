#include "VerdantEye.h"
#define WIFI_SSID "Hacked"
#define WIFI_PASS "25june2004"
#define led 19
VerdantEye plantMonitor(WIFI_SSID, WIFI_PASS, BLYNK_AUTH_TOKEN);

void setup() {
  plantMonitor.begin();
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
}

void loop() {
  plantMonitor.run();
}
