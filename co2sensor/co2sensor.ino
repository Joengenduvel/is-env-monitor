#define debug true

#include "credentials.h"
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
ESP8266WiFiMulti wifiMulti;
HTTPClient http;
WiFiClient client;
const uint32_t connectTimeoutMs = 5000;

#include <Wire.h>
#include <SparkFunCCS811.h>
#include <Adafruit_SCD30.h>

#define CCS811_ADDR 0x5B //Default I2C Address
CCS811 myCCS811(CCS811_ADDR);
Adafruit_SCD30  scd30;

bool ccs811Present = false;
bool scd30Present = false;

String serverName = "http://ptsv2.com/";

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
    Serial.print("CCS811 not found. Please check wiring.");
  }

  if (scd30.begin()){
    scd30Present = true;
  } else {
    Serial.print("scd30 not found. Please check wiring.");
  }

  if (!ccs811Present && !scd30Present) {
    Serial.print("No Sensors found, going to sleep");
    gotoSleep();
  }
}

void loop() {
  if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
    logConnectionStatus(true);
    sendCvs811Data();
    sendScd30Data();
  } else {
    logConnectionStatus(false);
  }
  delay(5000);
}

void sendCvs811Data() {
  if (ccs811Present) {
    if (myCCS811.dataAvailable()) {
      myCCS811.readAlgorithmResults();
      logCcs811Information();
      
      // SEND DATA HERE
      
    } else if (myCCS811.checkForStatusError()) {
      Serial.println("Failed to read ccs811 sensor!");
      gotoSleep();
    }
  }
}

void sendScd30Data() {
  if (scd30Present && scd30.dataReady()) {
    if (scd30.read()) {
      logScd30Information();
      // SEND DATA HERE

      http.begin(client, "http://ptsv2.com/t/9n61g-1639231375/post");
      http.addHeader("Content-Type", "application/json");

      String requestData= "{"
        "\"temprature\":" + String(scd30.temperature) + ","
        "\"humidity\":" + String(scd30.relative_humidity) + ","
        "\"co2\":" + String(scd30.CO2) +
        "}";
      
      // int httpResponseCode = http.POST(requestData);
      // Serial.println(httpResponseCode);
    } else {
      Serial.println("Failed to read scd30 sensor!");
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

void logScd30Information() {
  if (debug){
    Serial.println("==== SCD30 ====");
    Serial.print("Temperature: ");
    Serial.print(scd30.temperature);
    Serial.println(" degrees C");
    
    Serial.print("Relative Humidity: ");
    Serial.print(scd30.relative_humidity);
    Serial.println(" %");
    
    Serial.print("CO2: ");
    Serial.print(scd30.CO2, 3);
    Serial.println(" ppm");
    Serial.println("===============");
  }
}

void logCcs811Information(){
  if (debug) {
    Serial.println("==== CCS811 ====");
    Serial.print("CCS811 CO2: ");
    Serial.println(myCCS811.getCO2());
    Serial.println("================");
  }
}

void gotoSleep() {
  //TODO: really sleep
  while (1);
}
