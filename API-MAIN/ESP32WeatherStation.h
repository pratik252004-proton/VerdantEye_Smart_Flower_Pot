#ifndef ESP32WEATHERSTATION_H
#define ESP32WEATHERSTATION_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Arduino.h>

class ESP32WeatherStation {
public:
    ESP32WeatherStation(Adafruit_SH1106G* display);

    void setWiFi(const char* ssid, const char* password);
    void setLocation(const String& city, const String& countryCode);
    void setAPIKey(const String& key);

    void begin();
    void update();

private:
    Adafruit_SH1106G* display;
    const char* ssid;
    const char* password;
    String city, countryCode, apiKey;

    String cityName, description;
    float temp, windSpeed, pressure, humidity;
    int currentPage = 0;
    const int totalPages = 6;

    void connectWiFi();
    void getWeatherData();
    void drawHeader();
    void drawFooter(int currentPage);
    void showPage(const String& title, const String& data, const uint8_t* icon, int pageIndex);
    const uint8_t* getWeatherIcon(String desc);

    // Internal bitmaps
    static const unsigned char cityIcon[];
    static const unsigned char tempIcon[];
    static const unsigned char humidityIcon[];
    static const unsigned char pressureIcon[];
    static const unsigned char windIcon[];
    static const unsigned char clearSky[];
    static const unsigned char cloudRain[];
    static const unsigned char thunderStorm[];
    static const unsigned char fog[];
    static const unsigned char mist[];
    static const unsigned char cloudIcon[];
};

#endif
