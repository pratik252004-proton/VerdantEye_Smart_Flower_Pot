#define BLYNK_TEMPLATE_ID "TMPL3-HpMpwye"
#define BLYNK_TEMPLATE_NAME "VerdantEye"
#define BLYNK_AUTH_TOKEN "uIRoxlp2JGRSVHu5pGt8eB5WnWvxG3L2"

#ifndef RGBController_h
#define RGBController_h

#include <Arduino.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

class RGBController {
  public:
    RGBController(const char* wifiSsid, const char* wifiPass, const char* blynkToken);
    void begin();
    void run();

    // Blynk handlers must be public so macros can call them
    static void setRed(BlynkParam param);
    static void setGreen(BlynkParam param);
    static void setBlue(BlynkParam param);
    static void setLED1(BlynkParam param);
    static void setLED2(BlynkParam param);

  private:
    // Credentials
    const char* _ssid;
    const char* _pass;
    const char* _auth;

    // Internal pins
    static const int RED_PIN;
    static const int GREEN_PIN;
    static const int BLUE_PIN;
    static const int LED1_PIN;
    static const int LED2_PIN;

    // Internal channels
    static const int RED_CHANNEL;
    static const int GREEN_CHANNEL;
    static const int BLUE_CHANNEL;
};

#endif
