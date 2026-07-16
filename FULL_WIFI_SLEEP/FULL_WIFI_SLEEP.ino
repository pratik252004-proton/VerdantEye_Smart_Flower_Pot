/* 
  VerdantEye - Full sketch with custom AP Wi-Fi setup page + Deep Sleep
  Modified for:
    - Deep sleep for 50 minutes
    - Max active time of 30 minutes
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ESP32WeatherStation.h>
#include <VerdantEye.h>
#include <RGBController.h>
#include "esp_sleep.h" // For deep sleep

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define LED_PIN 19

#define SLEEP_TIME_MINUTES 50 // Deep sleep duration in minutes
#define ACTIVE_TIME_MINUTES 30 // Active runtime before sleep

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ESP32WeatherStation weatherStation(&display);

String ssid;
String pass;
VerdantEye plantMonitor(ssid.c_str(), pass.c_str(), BLYNK_AUTH_TOKEN);
RGBController rgb(ssid.c_str(), pass.c_str(), BLYNK_AUTH_TOKEN);

#include <FluxGarage_RoboEyes.h>
roboEyes roboEyes;


Preferences preferences;
WebServer server(80);

// Weather API settings
const char* CITY    = "Pune";
const char* COUNTRY = "IN";
const char* API_KEY = "b12063ea5512f9cf0a836cf94fe512e6";

// App runtime modes
enum Mode { IDLE, EMOTION, WEATHER };
Mode currentMode = IDLE;
unsigned long modeStartTime = 0;
unsigned long activeStartTime = 0; // track total active time

// HTML page
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
    <footer>VerdantEye • Plant-friendly robot eyes 🌱</footer>
  </div>
</body>
</html>
)rawliteral";

// ---------------- Helper Functions ----------------
void handleRoot() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(200, "text/html", htmlPage);
}

void handleSave() {
  if (!server.hasArg("ssid") || !server.hasArg("password")) {
    server.send(400, "text/plain", "Missing fields");
    return;
  }
  String ssid = server.arg("ssid");
  String pass = server.arg("password");

  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("pass", pass);
  preferences.end();

  String reply = "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'/><title>Saved</title></head><body style='font-family:Arial;text-align:center;color:#004d2b;background:#00110a;padding:40px;'>";
  reply += "<h2 style='color:#00ff88;'>✅ Saved!</h2>";
  reply += "<p>Attempting to connect to <strong>" + ssid + "</strong>. The device will restart now.</p>";
  reply += "</body></html>";

  server.send(200, "text/html", reply);
  delay(1200);
  ESP.restart();
}

void handleClear() {
  preferences.begin("wifi", false);
  preferences.clear();
  preferences.end();
  server.send(200, "text/plain", "Credentials cleared. Restarting...");
  delay(600);
  ESP.restart();
}

void startAPMode() {
  Serial.println("Starting AP mode...");
  WiFi.disconnect(true, true);
  delay(200);
  WiFi.mode(WIFI_AP);
  delay(50);

  const char* apName = "VerdantEye_Setup";
  const char* apPass = "12345678";
  if (!WiFi.softAP(apName, apPass)) {
    Serial.println("Failed to start softAP!");
    return;
  }

  IPAddress ip = WiFi.softAPIP();
  Serial.printf("AP started: SSID=%s  PASS=%s  IP=%s\n", apName, apPass, ip.toString().c_str());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/clear", HTTP_GET, handleClear);
  server.begin();
}

bool tryConnectSavedWiFi(unsigned long timeoutMs = 12000) {
  preferences.begin("wifi", true);
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");
  preferences.end();

  if (ssid.length() == 0) {
    Serial.println("No stored credentials.");
    return false;
  }

  Serial.printf("Found stored credentials: SSID='%s'\n", ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);
  WiFi.begin(ssid.c_str(), pass.c_str());

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected. IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }
  Serial.println("Failed to connect with stored credentials.");
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

  preferences.begin("wifi", true);
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");
  preferences.end();

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
  Serial.println("Back to IDLE mode");
}

// Deep sleep function
void goToDeepSleep() {
  Serial.printf("Going into deep sleep for %d minutes...\n", SLEEP_TIME_MINUTES);

  digitalWrite(LED_PIN, LOW);

  esp_sleep_enable_timer_wakeup((uint64_t)SLEEP_TIME_MINUTES * 60ULL * 1000000ULL);

  delay(100);
  esp_deep_sleep_start();
}

// ---------------- Main ----------------
void setup() {
  Serial.begin(115200);
  delay(50);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  if (!display.begin(0x3C, true)) {
    Serial.println(F("SH1106 allocation failed"));
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(0, 0);
    display.print("VerdantEye Booting...");
    display.display();
  }

  bool connected = tryConnectSavedWiFi();
  if (!connected) {
    startAPMode();
    Serial.println("AP ready. Please open http://192.168.4.1/ in your browser.");
    if (display.width() > 0) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("AP: VerdantEye_Setup");
      display.display();
    }
    
  } else {
    initRoboEyesAndWeather();
    currentMode = IDLE;
    modeStartTime = millis();
    activeStartTime = millis(); // start tracking active period
    digitalWrite(LED_PIN, LOW);
    Serial.println("Ready and running in normal mode.");
  }
   plantMonitor.begin();
   rgb.begin();
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
      Serial.println("Returning to IDLE from WEATHER");
      //goToDeepSleep(); // Sleep immediately after weather mode
    }
  }

  // If Wi-Fi lost, go back to AP mode
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected — starting AP mode for reconfiguration.");
    delay(200);
    startAPMode();
  }

  // Check total active time
  if ((millis() - activeStartTime) >= (ACTIVE_TIME_MINUTES * 60UL * 1000UL)) {
    Serial.println("30 minutes active time reached — going to deep sleep.");
    goToDeepSleep();
  }
  plantMonitor.run();
  rgb.run();
}
