#include "credentials.h"
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti wifiMulti;

const uint32_t connectTimeoutMs = 5000;

void setup() {
  WiFi.persistent(false);

  Serial.begin(115200);
  Serial.println("\nESP8266 Multi WiFi example");

  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);

  // Register multi WiFi networks
  wifiMulti.addAP(SSID1, PWD1);
  wifiMulti.addAP(SSID2, PWD2);

}

void loop() {
  if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
    Serial.print("WiFi connected: ");
    Serial.print(WiFi.SSID());
    Serial.print(" ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi not connected!");
  }

  delay(1000);
}
