#define debug true

#include "credentials.h"
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
ESP8266WiFiMulti wifiMulti;
const uint32_t connectTimeoutMs = 5000;

#include <Wire.h>
#include <SparkFunCCS811.h>

#define CCS811_ADDR 0x5B //Default I2C Address
CCS811 myCCS811(CCS811_ADDR);

ESP8266WebServer http_server(9090);
#include <ESP8266WebServer.h>

bool ccs811Present = false;
bool sd30Present = false;

String newHostname = "Livingroom";


// HTTP metrics endpoint
#define HTTP_METRICS_ENDPOINT "/metrics"
// Prometheus namespace, aka metric prefix
#define PROM_NAMESPACE "office"

int co2 = 0;

void setup() {

  Serial.begin(115200);
  Serial.println("\nInfo Support Environment Monitor 1.0");

  WiFi.persistent(false);
  WiFi.hostname(newHostname.c_str());
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

  setup_http_server();
}

void loop() {
  if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
    logConnectionStatus(true);
    Serial.printf("Default hostname: %s\n", WiFi.hostname().c_str());

  //Set new hostname
  WiFi.hostname(newHostname.c_str());

  //Get Current Hostname
  Serial.printf("New hostname: %s\n", WiFi.hostname().c_str());
    co2 = readCo2Ccs811();
    Serial.println(co2);

    http_server.handleClient();

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

void setup_http_server() {
    char message[128];
    log("Setting up HTTP server");
    http_server.on("/", HTTPMethod::HTTP_GET, handle_http_root);
    http_server.on(HTTP_METRICS_ENDPOINT, HTTPMethod::HTTP_GET, handle_http_metrics);
    http_server.onNotFound(handle_http_not_found);
    http_server.begin();
}

void handle_http_root() {
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
        PROM_NAMESPACE "_info{version=\"%s\",board=\"%s\",sensor=\"%s\"} 1\n"
        "# HELP " PROM_NAMESPACE "_air_co2 CO2 value.\n"
        "# TYPE " PROM_NAMESPACE "_air_co2 gauge\n"
        "# UNIT " PROM_NAMESPACE "_air_co2 co2\n"
        PROM_NAMESPACE "_air_co2 %d\n";
        

    char response[BUFSIZE];
    snprintf(response, BUFSIZE, response_template, "1", "ESP8266", "CO2", co2);
    http_server.send(200, "text/plain; charset=utf-8", response);
    log(response);
}

void handle_http_not_found() {
    http_server.send(404, "text/plain; charset=utf-8", "Not found.");
}


void log(String message){
  Serial.println(message);
}
