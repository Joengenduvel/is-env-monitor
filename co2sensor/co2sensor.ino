#define debug false

#include "credentials.h"
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
const uint32_t connectTimeoutMs = 5000;

#include <Wire.h>
#include <SparkFunCCS811.h>

#define CCS811_ADDR 0x5B //Default I2C Address
CCS811 myCCS811(CCS811_ADDR);

bool ccs811Present = false;
bool sd30Present = false;

void setup() {

  Serial.begin(115200);
  Serial.println("\nInfo Support Environment Monitor 1.0");

  WiFi.persistent(false);
  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);

  // Register multi WiFi networks
  wifiMulti.addAP(SSID1, PWD1);
  wifiMulti.addAP(SSID2, PWD2);

  Wire.begin(); //Inialize I2C Hardware

  if (myCCS811.begin())
  {
    ccs811Present = true;
  } else {
    Serial.print("CCS811 error. Please check wiring.");
  }

  if (!ccs811Present && !sd30Present) {
    Serial.print("No Sensors found, going to sleep");
    gotoSleep();
  }
}

void loop() {
  if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
    logConnectionStatus(true);
    int co2 = readCo2Ccs811();
    Serial.println(co2);

  } else {
    logConnectionStatus(false);
  }
  delay(1000);
}

int readCo2Ccs811() {
  if (ccs811Present) {
    if (myCCS811.dataAvailable())
    {
      myCCS811.readAlgorithmResults();
      return myCCS811.getCO2();
    } else if (myCCS811.checkForStatusError())
    {
      Serial.println("Failed to read ccs811 sensor!");
      gotoSleep();
    }
  }
}

void logConnectionStatus(bool connected) {
  if (debug) {
    if (connected) {
      Serial.print("WiFi connected: ");
      Serial.print(WiFi.SSID());
      Serial.print(" ");
      Serial.println(WiFi.localIP());
    }
    else {
      Serial.println("WiFi not connected!");
    }
  }
}

void gotoSleep() {
  //TODO: really sleep
  while (1);
}
