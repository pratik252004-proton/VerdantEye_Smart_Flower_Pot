#include "RGBController.h"

#define WIFI_SSID "Hacked"
#define WIFI_PASS "25june2004"

RGBController rgb(WIFI_SSID, WIFI_PASS, BLYNK_AUTH_TOKEN);

void setup() {
  rgb.begin();
}

void loop() {
  rgb.run();
}
