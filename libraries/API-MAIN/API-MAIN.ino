#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "ESP32WeatherStation.h"

Adafruit_SH1106G display(128, 64, &Wire, -1);
ESP32WeatherStation weatherStation(&display);
const char* SSID = "Hacked";
const char* PASSWORD = "25june2004";

void setup() {
  Serial.begin(115200);
  weatherStation.setWiFi(SSID, PASSWORD);
  weatherStation.setLocation("Pune", "IN");
  weatherStation.setAPIKey("b12063ea5512f9cf0a836cf94fe512e6");
  weatherStation.begin();
}

void loop() {
  weatherStation.update();
}
