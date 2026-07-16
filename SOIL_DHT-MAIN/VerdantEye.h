#define BLYNK_TEMPLATE_ID "TMPL3-HpMpwye"
#define BLYNK_TEMPLATE_NAME "VerdantEye"
#define BLYNK_AUTH_TOKEN "uIRoxlp2JGRSVHu5pGt8eB5WnWvxG3L2"


#ifndef VERDANTEYE_H
#define VERDANTEYE_H

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <DHT.h>

class VerdantEye {
public:
    VerdantEye(const char* ssid, const char* pass, const char* authToken);
    void begin();
    void run();

private:
    const char* _ssid;
    const char* _password;
    const char* _authToken;

    DHT dht;
    const int soilPin = 34;
    static const int numReadings = 100;
    int readings[numReadings];
    int readIndex;
    int total;
    int average;

    const int SOIL_DRY = 3279;
    const int SOIL_WET = 2745;

    unsigned long lastBlynkUpdate;
    const unsigned long blynkInterval = 30000;
    unsigned long lastSerialPrint;
    const unsigned long serialInterval = 1000;

    void readSensors(int &soilPercent, float &temp, float &humid);
};

#endif
