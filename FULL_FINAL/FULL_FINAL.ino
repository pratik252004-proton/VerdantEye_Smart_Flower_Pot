#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ESP32WeatherStation.h>
#include "esp_sleep.h" // For deep sleep

#include <VerdantEye.h>
#include <RGBController.h>

// ---------------- CONFIG ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define LED_PIN 19

#define SLEEP_TIME_MINUTES 50   // Deep sleep duration
#define ACTIVE_TIME_MINUTES 30  // Max runtime before sleep

// Weather API
const char* CITY    = "Pune";
const char* COUNTRY = "IN";
const char* API_KEY = "b12063ea5512f9cf0a836cf94fe512e6";

// ---------------- GLOBALS ----------------
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ESP32WeatherStation weatherStation(&display);
Preferences preferences;
WebServer server(80);

#include <FluxGarage_RoboEyes.h>
roboEyes roboEyes;
VerdantEye* plantMonitor;  // Will be created later
RGBController* rgb;        // Will be created later

String ssid;
String pass;

enum Mode { IDLE, EMOTION, WEATHER };
Mode currentMode = IDLE;

unsigned long modeStartTime = 0;
unsigned long activeStartTime = 0;

// HTML page for Wi-Fi setup
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>VerdantEye - WiFi Setup</title>
<style>
  body{font-family:Arial;background:#00110a;color:#8fb3a6;display:flex;justify-content:center;align-items:center;height:100vh;}
  .card{background:#0f1a18;padding:20px;border-radius:10px;box-shadow:0 0 15px rgba(0,0,0,0.5);}
  input,button{width:100%;padding:10px;margin:5px 0;border-radius:5px;border:none;}
  button{background:#00ff88;color:#00110a;font-weight:bold;}
</style>
</head>
<body>
<div class="card">
<h2>VerdantEye Wi-Fi Setup</h2>
<form action="/save" method="POST">
  <input type="text" name="ssid" placeholder="Wi-Fi SSID" required maxlength="32"/>
  <input type="password" name="password" placeholder="Wi-Fi Password" required maxlength="64"/>
  <button type="submit">Save & Connect</button>
</form>
<p>Clear stored credentials: <a href="/clear">/clear</a></p>
</div>
</body>
</html>
)rawliteral";

// ---------------- FUNCTIONS ----------------
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleSave() {
  if (!server.hasArg("ssid") || !server.hasArg("password")) {
    server.send(400, "text/plain", "Missing fields");
    return;
  }
  ssid = server.arg("ssid");
  pass = server.arg("password");

  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", pass);
  preferences.end();

  server.send(200, "text/plain", "Saved! Rebooting...");
  delay(1000);
  ESP.restart();
}

void handleClear() {
  preferences.begin("wifi", false);
  preferences.clear();
  preferences.end();
  server.send(200, "text/plain", "Cleared! Rebooting...");
  delay(500);
  ESP.restart();
}

void startAPMode() {
  Serial.println("Starting AP mode...");
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_AP);
  delay(50);
  WiFi.softAP("VerdantEye_Setup", "12345678");
  Serial.println("AP started: SSID=VerdantEye_Setup PASS=12345678");

  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP: "); Serial.println(ip);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/clear", HTTP_GET, handleClear);
  server.begin();
}

bool tryConnectSavedWiFi(unsigned long timeoutMs = 10000) {
  preferences.begin("wifi", true);
  ssid = preferences.getString("ssid", "");
  pass = preferences.getString("pass", "");
  preferences.end();

  if (ssid.isEmpty()) {
    Serial.println("No stored Wi-Fi credentials");
    return false;
  }

  Serial.printf("Connecting to SSID: %s\n", ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  return false;
}

void initRoboEyesAndWeather() {
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);
  roboEyes.setWidth(36, 36);
  roboEyes.setHeight(36, 36);
  roboEyes.setBorderradius(8, 8);
  roboEyes.setSpacebetween(10);

  weatherStation.setWiFi(ssid.c_str(), pass.c_str());
  weatherStation.setLocation(CITY, COUNTRY);
  weatherStation.setAPIKey(API_KEY);
  weatherStation.begin();
}

void resetToIdleMode() {
  roboEyes.setMood(DEFAULT);
  roboEyes.setCuriosity(OFF);
  roboEyes.setHFlicker(OFF);
  roboEyes.setVFlicker(OFF);
}

void goToDeepSleep() {
  Serial.printf("Sleeping for %d minutes...\n", SLEEP_TIME_MINUTES);
  digitalWrite(LED_PIN, LOW);
  esp_sleep_enable_timer_wakeup((uint64_t)SLEEP_TIME_MINUTES * 60ULL * 1000000ULL);
  delay(100);
  esp_deep_sleep_start();
}

// ---------------- MAIN ----------------
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  display.begin(0x3C, true);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("VerdantEye Booting...");
  display.display();

  if (tryConnectSavedWiFi()) {
    // Create objects AFTER Wi-Fi credentials are loaded
    plantMonitor = new VerdantEye(ssid.c_str(), pass.c_str(), BLYNK_AUTH_TOKEN);
    rgb = new RGBController(ssid.c_str(), pass.c_str(), BLYNK_AUTH_TOKEN);
    plantMonitor->begin();
    rgb->begin();
    initRoboEyesAndWeather();
    currentMode = IDLE;
    modeStartTime = millis();
    activeStartTime = millis();
    //digitalWrite(LED_PIN, LOW);
  } else {
    startAPMode();
  }
}

void loop() {
  if (WiFi.getMode() == WIFI_AP) {
    server.handleClient();
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 500) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      lastBlink = millis();
    }
    return;
  }

  unsigned long now = millis();

  if (currentMode == IDLE) {
    roboEyes.update();
    if (now - modeStartTime >= 30000) {
      int r = random(0, 5);
      switch (r) {
        case 0: currentMode = WEATHER; break;
        case 1: roboEyes.setMood(TIRED); currentMode = EMOTION; break;
        case 2: roboEyes.setMood(ANGRY); currentMode = EMOTION; break;
        case 3: roboEyes.setCuriosity(ON); currentMode = EMOTION; break;
        case 4: roboEyes.setMood(HAPPY); currentMode = EMOTION; break;
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
    weatherStation.update();
    if (now - modeStartTime >= 60000) {
      display.clearDisplay();
      display.display();
      currentMode = IDLE;
      modeStartTime = now;
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    startAPMode();
  }

  if ((millis() - activeStartTime) >= (ACTIVE_TIME_MINUTES * 60UL * 1000UL)) {
    goToDeepSleep();
  }

  if (plantMonitor) plantMonitor->run();
  if (rgb) rgb->run();
}
