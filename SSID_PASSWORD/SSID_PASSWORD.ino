#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>

#define EEPROM_SIZE 96

WebServer server(80);

String ssid = "";
String password = "";

void handleRoot() {
  String page = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>VerdantEye WiFi Setup</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body {
          font-family: Arial, sans-serif;
          background: linear-gradient(to right, #e0f7f1, #f0fff9);
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
          margin: 0;
        }
        .card {
          background: #ffffff;
          border-radius: 12px;
          box-shadow: 0 4px 12px rgba(0, 100, 80, 0.2);
          padding: 30px;
          width: 90%%;
          max-width: 380px;
          text-align: center;
        }
        h2 {
          color: #00796b;
          margin-top: 10px;
        }
        input[type=text], input[type=password] {
          width: 100%%;
          padding: 12px;
          margin: 8px 0;
          border: 1px solid #ccc;
          border-radius: 6px;
        }
        input[type=submit] {
          background-color: #009688;
          color: white;
          padding: 12px;
          width: 100%%;
          border: none;
          border-radius: 6px;
          font-size: 16px;
          cursor: pointer;
        }
        input[type=submit]:hover {
          background-color: #00796b;
        }
        .plant-icon {
          width: 80px;
          height: 80px;
          margin: 0 auto 15px;
        }
      </style>
    </head>
    <body>
      <div class="card">
        <h2>VerdantEye Setup</h2>
        <form action="/save" method="POST">
          <input type="text" name="ssid" placeholder="WiFi SSID" required>
          <input type="password" name="password" placeholder="WiFi Password" required>
          <input type="submit" value="Save & Connect">
        </form>
      </div>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", page);
}

void handleSave() {
  ssid = server.arg("ssid");
  password = server.arg("password");

  Serial.println("Saving credentials...");
  Serial.println("SSID: " + ssid);
  Serial.println("Password: " + password);

  // Save to EEPROM
  for (int i = 0; i < ssid.length(); ++i) {
    EEPROM.write(i, ssid[i]);
  }
  EEPROM.write(ssid.length(), '\0');

  for (int i = 0; i < password.length(); ++i) {
    EEPROM.write(32 + i, password[i]);
  }
  EEPROM.write(32 + password.length(), '\0');

  EEPROM.commit();

  server.send(200, "text/html", "<h2>Saved! Rebooting...</h2>");
  delay(2000);
  ESP.restart();
}

void startAPMode() {
  Serial.println("Starting AP mode...");
  WiFi.softAP("VerdantEye-Setup");

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);

  // Captive portal: redirect all unknown pages to root
  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
  Serial.println("Web server started.");
}

bool connectToSavedWiFi() {
  EEPROM.begin(EEPROM_SIZE);
  char ssidBuf[32], passBuf[32];

  for (int i = 0; i < 32; ++i) {
    ssidBuf[i] = EEPROM.read(i);
    passBuf[i] = EEPROM.read(32 + i);
  }

  ssid = String(ssidBuf);
  password = String(passBuf);

  Serial.println("Connecting to saved WiFi:");
  Serial.println("SSID: " + ssid);

  if (ssid.length() < 1) return false;

  WiFi.begin(ssid.c_str(), password.c_str());

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    Serial.print(".");
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println("\nFailed to connect.");
  return false;
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  if (!connectToSavedWiFi()) {
    startAPMode();
  }
}

void loop() {
  server.handleClient();

  // Optional: your main logic after WiFi connection
}
