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
#define MOS_PIN 19

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
// HTML page for Wi-Fi setup (Enhanced UI)
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>VerdantEye - WiFi Setup</title>
<style>
  :root{--bg:#071012;--panel:#0f1a18;--accent:#00ff88;--muted:#8fb3a6;}
  body{margin:0;font-family:Segoe UI,Roboto,Arial;background:radial-gradient(circle at 10% 10%, #06221b 0%, var(--bg) 40%);color:var(--muted);display:flex;min-height:100vh;align-items:center;justify-content:center}
  .card{width:360px;max-width:94vw;background:linear-gradient(180deg, rgba(255,255,255,0.02), rgba(0,0,0,0.03));border-radius:16px;padding:18px;box-shadow:0 8px 30px rgba(0,0,0,0.6);border:1px solid rgba(0,255,136,0.06)}
  .logo{display:flex;align-items:center;gap:12px;margin-bottom:8px}
  .logo .eye{width:48px;height:48px;border-radius:50%;background:linear-gradient(145deg,#002b22,#003833);display:flex;align-items:center;justify-content:center;box-shadow:inset 0 -6px 16px rgba(0,0,0,0.5)}
  .logo .eye:before{content:'';width:18px;height:18px;background:var(--accent);border-radius:50%;box-shadow:0 0 12px rgba(0,255,136,0.9)}
  h1{margin:0;color:var(--accent);font-size:20px}
  p.lead{margin:6px 0 16px;color:#9bdab6;font-size:13px}
  form{display:flex;flex-direction:column;gap:10px}
  input[type=text],input[type=password]{padding:12px;border-radius:8px;border:1px solid rgba(255,255,255,0.04);background:transparent;color:var(--muted);outline:none}
  input::placeholder{color:#6f8f7f}
  button{padding:12px;border-radius:10px;border:none;background:var(--accent);color:#00110a;font-weight:700;cursor:pointer}
  .small{font-size:12px;color:#8fb3a6;margin-top:12px;text-align:center}
  .link{color:#8fdab0;text-decoration:underline;cursor:pointer}
  footer{margin-top:12px;text-align:center;font-size:11px;color:#7eaea0}
</style>
</head>
<body>
  <div class="card">
    <div class="logo">
      <div class="eye" aria-hidden="true"></div>
      <div>
        <h1>VerdantEye Setup</h1>
        <div class="small">Enter your Wi-Fi credentials to connect</div>
      </div>
    </div>
    <p class="lead">The device will attempt to connect after you press <strong>Save & Connect</strong>.</p>
    <form action="/save" method="POST" autocomplete="off">
      <input type="text" name="ssid" placeholder="Wi-Fi SSID" required maxlength="32"/>
      <input type="password" name="password" placeholder="Wi-Fi Password" required maxlength="64"/>
      <button type="submit">Save &amp; Connect</button>
    </form>
    <div class="small">If you want to erase stored credentials, visit <a href="/clear" class="link">/clear</a></div>
    <footer>VerdantEye • Plant-friendly robot-Plant 🌱</footer>
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

  String reply = "<!doctype html><html><head>"
                 "<meta charset='utf-8'>"
                 "<meta name='viewport' content='width=device-width,initial-scale=1'/>"
                 "<title>Saved</title>"
                 "<style>"
                 "body{font-family:Arial;text-align:center;color:#004d2b;background:#00110a;padding:40px;}"
                 "h2{color:#00ff88;}"
                 "p{color:#8fb3a6;}"
                 "</style>"
                 "</head><body>";
  reply += "<h2>✅ Saved!</h2>";
  reply += "<p>Attempting to connect to <strong>" + ssid + "</strong>.<br>The device will restart now.</p>";
  reply += "</body></html>";

  server.send(200, "text/html", reply);
  delay(2000);
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
  WiFi.softAP("VerdantEye_Setup", "#rakshabandan25");
  Serial.println("AP started: SSID=VerdantEye_Setup PASS=#rakshabandan25");

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
  digitalWrite(MOS_PIN, LOW);
  esp_sleep_enable_timer_wakeup((uint64_t)SLEEP_TIME_MINUTES * 60ULL * 1000000ULL);
  delay(100);
  esp_deep_sleep_start();
}

// ---------------- MAIN ----------------
void setup() {
  Serial.begin(115200);
  pinMode(MOS_PIN, OUTPUT);
  digitalWrite(MOS_PIN, HIGH);

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
    //digitalWrite(MOS_PIN, LOW);
  } else {
    startAPMode();
  }
}

void loop() {
  if (WiFi.getMode() == WIFI_AP) {
    server.handleClient();
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 500) {
      digitalWrite(MOS_PIN, !digitalRead(MOS_PIN));
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
