#define BLYNK_TEMPLATE_ID "TMPL3soEjxgc0"
#define BLYNK_TEMPLATE_NAME "Verdanteye"
#define BLYNK_AUTH_TOKEN "QJ1rYoVdZqmQBLYKKHRs1tHUHngfXdo4"

/*************************************************************
 * CLEANED CODE — ESP32 + BLYNK + DHT11 + SOIL (WITH SERIAL + PERCENTAGE)
 *************************************************************/
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <DHT.h>

// WiFi credentials
const char* ssid = "Hacked";
const char* password = "25june2004";

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

// Define dry and wet calibration values (adjust based on your sensor and soil)
const int SOIL_DRY = 2820;   // Analog value when soil is dry
const int SOIL_WET = 2440;   // Analog value when soil is wet

void setup() {
  Serial.begin(115200);
  dht.begin();
  for (int i = 0; i < numReadings; i++) readings[i] = 0;

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

unsigned long lastBlynkUpdate = 0;
const unsigned long blynkInterval = 30000;  // 30 seconds

void loop() {
  Blynk.run();
  unsigned long currentMillis = millis();

  // Soil averaging
  total -= readings[readIndex];
  readings[readIndex] = analogRead(soilPin);
  total += readings[readIndex];
  readIndex = (readIndex + 1) % numReadings;
  average = total / numReadings;

  // Convert to percentage (clamped between 0 and 100)
  int soilPercent = map(average, SOIL_DRY, SOIL_WET, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100);

  // Read DHT values
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();

  if (isnan(temp) || isnan(humid)) {
    Serial.println("DHT Sensor read failed");
    return;
  }

  // ==== New: Serial print every 5 seconds ====
  static unsigned long lastSerialPrint = 0;
  const unsigned long serialInterval = 1000;  // 5 seconds

  if (currentMillis - lastSerialPrint >= serialInterval) {
    lastSerialPrint = currentMillis;

    Serial.println("===============");
    Serial.print("Soil (raw)   : "); Serial.println(average);
    Serial.print("Soil (%)     : "); Serial.print(soilPercent); Serial.println(" %");
    Serial.print("Temperature  : "); Serial.print(temp); Serial.println(" °C");
    Serial.print("Humidity     : "); Serial.print(humid); Serial.println(" %");
  }

  // ==== Send to Blynk every 30 seconds ====
  if (currentMillis - lastBlynkUpdate >= blynkInterval) {
    lastBlynkUpdate = currentMillis;

    Blynk.virtualWrite(VPIN_SOIL, soilPercent);
    Blynk.virtualWrite(VPIN_TEMP, temp);
    Blynk.virtualWrite(VPIN_HUMID, humid);
    Serial.println("Data sent to Blynk!");
  }
}
