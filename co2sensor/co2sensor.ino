#define debug true

#include "credentials.h"
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
ESP8266WiFiMulti wifiMulti;
const uint32_t connectTimeoutMs = 5000;

#include <Wire.h>
#include <SparkFunCCS811.h>
#include <Adafruit_SCD30.h>

#define CCS811_ADDR 0x5B //Default I2C Address
CCS811 myCCS811(CCS811_ADDR);
Adafruit_SCD30  scd30;

const int port = 80; //9090
ESP8266WebServer http_server(port);
#include <ESP8266WebServer.h>

bool ccs811Present = false;
bool scd30Present = false;

String room = "Livingroom";


// HTTP metrics endpoint
#define HTTP_METRICS_ENDPOINT "/metrics"
// Prometheus namespace, aka metric prefix
#define PROM_NAMESPACE "office"

int co2 = 0;
bool connected = false;

typedef struct EnvironmentData {
  String sensorName;
  bool hasTemperature;
  bool hasHumidity;
  bool hasCO2;
  int temperature;
  int humidity;
  int CO2;
};

void setup() {

  Serial.begin(115200);
  Serial.println("\nInfo Support Environment Monitor 1.0");

  WiFi.persistent(false);
  WiFi.hostname(room.c_str());
  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);

  // Register multi WiFi networks
  wifiMulti.addAP(SSID1, PWD1);
  wifiMulti.addAP(SSID2, PWD2);

  Wire.begin(); //Inialize I2C Hardware

  if (myCCS811.begin())
  {
    ccs811Present = true;
    Serial.println("CCS811 connected.");
  } else {
    Serial.println("CCS811 error. Please check wiring.");
  }

  if (scd30.begin()) {
    scd30Present = true;
    Serial.println("scd30 connected.");
  } else {
    Serial.println("scd30 not found. Please check wiring.");
  }

  if (!ccs811Present && !scd30Present) {
    Serial.println("No Sensors found, going to sleep");
    gotoSleep();
  }
}

void loop() {
  ensureConnection();
  http_server.handleClient();
  if (debug && micros() % (1000 * 1000 * 10) < 50) {
    readSensors();
  }
  if (!debug && micros() % (1000 * 1000) < 50) {
    EnvironmentData data = readSensors();
    Serial.println(data.CO2);
  }
}

void ensureConnection() {
  if (!connected) {
    if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
      connected = true;
      WiFi.hostname(room.c_str());
      logConnectionStatus();

      setup_http_server();
    } else {
      connected = false;
      http_server.close();
      http_server.stop();
      delay(1000);
    }
  }
}

EnvironmentData readSensors() {
  EnvironmentData returnData;
  if (ccs811Present) {
    EnvironmentData data =  readCo2Ccs811();
    logSensorData(data);
    returnData = data;
  }
  if (scd30Present) {
    EnvironmentData data =  readScd30();
    logSensorData(data);
    returnData = data;
  }
  return returnData;
}

EnvironmentData readCo2Ccs811() {
  EnvironmentData data;
  data.sensorName = "CCS811";

  if (myCCS811.dataAvailable())
  {
    myCCS811.readAlgorithmResults();
    data.hasCO2 = true;
    data.CO2 = myCCS811.getCO2();
    return data;
  } else if (myCCS811.checkForStatusError())
  {
    Serial.println("Failed to read ccs811 sensor!");
  }
}

EnvironmentData readScd30() {
  EnvironmentData data = {};
  data.sensorName = "SCD30";

  if (scd30Present && scd30.dataReady()) {
    if (scd30.read()) {
      data.hasTemperature = true;
      data.temperature = scd30.temperature;

      data.hasHumidity = true;
      data.humidity = scd30.relative_humidity;

      data.hasCO2 = true;
      data.CO2 = scd30.CO2;
      return data;
    } else {
      Serial.println("Failed to read scd30 sensor!");
    }
  }
}

void logConnectionStatus() {
  if (debug) {
    if (connected) {
      Serial.printf("WiFi connected: %s @ %s as %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.hostname().c_str());
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

void setup_http_server() {
  char message[128];
  log("Setting up HTTP server @ " + port);
  http_server.on("/", HTTPMethod::HTTP_GET, handle_http_root);
  http_server.on(HTTP_METRICS_ENDPOINT, HTTPMethod::HTTP_GET, handle_http_metrics);
  http_server.onNotFound(handle_http_not_found);
  http_server.begin(port);
}

void handle_http_root() {
  Serial.println("root");
  static size_t const BUFSIZE = 256;
  static char const *response_template =
    "Prometheus ESP8266 DHT Exporter by HON95.\n"
    "\n"
    "Project: https://github.com/HON95/prometheus-esp8266-dht-exporter\n"
    "\n"
    "Usage: %s\n";
  char response[BUFSIZE];
  snprintf(response, BUFSIZE, response_template, HTTP_METRICS_ENDPOINT);
  http_server.send(200, "text/plain; charset=utf-8", response);
}


void handle_http_metrics() {
  log("sending metrics");
  static size_t const BUFSIZE = 1024;
  static char const *response_template =
    "# HELP " PROM_NAMESPACE "_info Metadata about the device.\n"
    "# TYPE " PROM_NAMESPACE "_info gauge\n"
    "# UNIT " PROM_NAMESPACE "_info \n"
    PROM_NAMESPACE "_info{room=\"%s\"} 1\n"
    "# HELP " PROM_NAMESPACE "_air_co2 CO2 value.\n"
    "# TYPE " PROM_NAMESPACE "_air_co2 gauge\n"
    "# UNIT " PROM_NAMESPACE "_air_co2 ppm\n"
    PROM_NAMESPACE "_air_co2 %d\n";


  char response[BUFSIZE];
  EnvironmentData data = readSensors();
  snprintf(response, BUFSIZE, response_template, room, data.CO2);
  http_server.send(200, "text/plain; charset=utf-8", response);
  log(response);
}

void handle_http_not_found() {
  http_server.send(404, "text/plain; charset=utf-8", "Not found.");
}

void logSensorData(EnvironmentData data) {
  if (debug) {
    Serial.print("==== ");
    Serial.print(data.sensorName);
    Serial.println(" ====");

    if (data.hasTemperature) {
      Serial.print("Temperature: ");
      Serial.print(scd30.temperature);
      Serial.println(" degrees C");
    }

    if (data.hasHumidity) {
      Serial.print("Relative Humidity: ");
      Serial.print(scd30.relative_humidity);
      Serial.println(" %");
    }

    if (data.hasCO2) {
      Serial.print("CO2: ");
      Serial.print(scd30.CO2, 3);
      Serial.println(" ppm");
    }
    Serial.println("===============");
  }
}

void log(String message) {
  Serial.println(message);
}
