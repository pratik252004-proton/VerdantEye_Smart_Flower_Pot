#define BLYNK_TEMPLATE_ID "TMPL3soEjxgc0"
#define BLYNK_TEMPLATE_NAME "Verdanteye"
#define BLYNK_AUTH_TOKEN "QJ1rYoVdZqmQBLYKKHRs1tHUHngfXdo4"

/*************************************************************
 * FULL WORKING CODE — ESP32 + BLYNK + DHT11 + SOIL + OLED
 *************************************************************/
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <DHT.h>

// WiFi credentials
const char* ssid = "Hacked";
const char* password = "25june2004";

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// DHT sensor setup
#define DHTPIN 18
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Soil moisture sensor
const int soilPin = 34;

// Virtual Pins
#define VPIN_SOIL V0
#define VPIN_TEMP V1
#define VPIN_HUMID V2

// Soil smoothing
const int numReadings = 100;
int readings[numReadings];
int readIndex = 0;
int total = 0;
int average = 0;

void setup() {
  Serial.begin(115200);

  // Initialize sensors
  dht.begin();
  for (int i = 0; i < numReadings; i++) readings[i] = 0;

  // OLED start
  display.begin(0x3C, true);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting to WiFi...");
  display.display();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected.");
  display.display();
  delay(1000);

  // Start Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

unsigned long lastDisplayUpdate = 0;
unsigned long lastBlynkUpdate = 0;
const unsigned long displayInterval = 100;       // 100 ms
const unsigned long blynkInterval = 30000;       // 30 seconds

void loop() {
  Blynk.run();

  unsigned long currentMillis = millis();

  // Read sensors every loop (or optimize this too)
  total -= readings[readIndex];
  readings[readIndex] = analogRead(soilPin);
  total += readings[readIndex];
  readIndex = (readIndex + 1) % numReadings;
  average = total / numReadings;

  float temp = dht.readTemperature();
  float humid = dht.readHumidity();

  if (isnan(temp) || isnan(humid)) {
    Serial.println("DHT Read Failed!");
    return;
  }

  // 🟢 Blynk update every 60 seconds
  if (currentMillis - lastBlynkUpdate >= blynkInterval) {
    lastBlynkUpdate = currentMillis;

    Blynk.virtualWrite(VPIN_SOIL, average);
    Blynk.virtualWrite(VPIN_TEMP, temp);
    Blynk.virtualWrite(VPIN_HUMID, humid);

    Serial.print("[BLYNK] Soil: ");
    Serial.print(average);
    Serial.print(" | Temp: ");
    Serial.print(temp);
    Serial.print("°C | Humid: ");
    Serial.println(humid);
  }

  // 🟢 OLED display update every 100 ms
  if (currentMillis - lastDisplayUpdate >= displayInterval) {
    lastDisplayUpdate = currentMillis;

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Soil:");
    display.setTextSize(2);
    display.setCursor(40, 0);
    display.print(average);

    display.setTextSize(1);
    display.setCursor(0, 24);
    display.print("Temp: ");
    display.print(temp, 1);
    display.print(" C");

    display.setCursor(0, 40);
    display.print("Humidity: ");
    display.print(humid, 1);
    display.print(" %");

    display.display();
  }
}