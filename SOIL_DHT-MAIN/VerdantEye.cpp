#include "VerdantEye.h"

#define DHTPIN 18
#define DHTTYPE DHT11

#define VPIN_SOIL V0
#define VPIN_TEMP V1
#define VPIN_HUMID V2

VerdantEye::VerdantEye(const char* ssid, const char* pass, const char* authToken)
: _ssid(ssid), _password(pass), _authToken(authToken), 
  dht(DHTPIN, DHTTYPE), readIndex(0), total(0), average(0),
  lastBlynkUpdate(0), lastSerialPrint(0)
{
    for (int i = 0; i < numReadings; i++) readings[i] = 0;
}

void VerdantEye::begin() {
    Serial.begin(115200);
    dht.begin();

    WiFi.begin(_ssid, _password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");

    Blynk.begin(_authToken, _ssid, _password);
}

void VerdantEye::readSensors(int &soilPercent, float &temp, float &humid) {
    total -= readings[readIndex];
    readings[readIndex] = analogRead(soilPin);
    total += readings[readIndex];
    readIndex = (readIndex + 1) % numReadings;
    average = total / numReadings;

    soilPercent = map(average, SOIL_DRY, SOIL_WET, 0, 100);
    soilPercent = constrain(soilPercent, 0, 100);

    temp = dht.readTemperature();
    humid = dht.readHumidity();
}

void VerdantEye::run() {
    Blynk.run();
    unsigned long currentMillis = millis();

    int soilPercent;
    float temp, humid;
    readSensors(soilPercent, temp, humid);

    if (isnan(temp) || isnan(humid)) {
        Serial.println("DHT Sensor read failed");
        return;
    }

    if (currentMillis - lastSerialPrint >= serialInterval) {
        lastSerialPrint = currentMillis;
        Serial.println("===============");
        Serial.print("Soil (raw)   : "); Serial.println(average);
        Serial.print("Soil (%)     : "); Serial.print(soilPercent); Serial.println(" %");
        Serial.print("Temperature  : "); Serial.print(temp); Serial.println(" °C");
        Serial.print("Humidity     : "); Serial.print(humid); Serial.println(" %");
    }

    if (currentMillis - lastBlynkUpdate >= blynkInterval) {
        lastBlynkUpdate = currentMillis;
        Blynk.virtualWrite(VPIN_SOIL, soilPercent);
        Blynk.virtualWrite(VPIN_TEMP, temp);
        Blynk.virtualWrite(VPIN_HUMID, humid);
        Serial.println("Data sent to Blynk!");
    }
}
