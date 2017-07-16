// BME280
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>

/* includes:
    WIFI_NAME
    WIFI_PASSWORD
    API_KEY

    See "credentials-exmaple.h"
*/
#include "credentials.h";

#define API_HOST "api.thingspeak.com"

#define SEALEVELPRESSURE_HPA (1013.25)

#define FALLBACK_TIMEOUT 7000 // 5 sec
unsigned long lastMs;

#define SLEEP_TIME_SEC 300 // 5 min

// Common
Adafruit_BME280 bme;
WiFiClient client;

void setup() {
  Serial.begin(9600);

  Serial.print("Connecting to ");
  Serial.println(WIFI_NAME);

  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

  lastMs = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");

    unsigned long ms = millis();
    if (ms >= lastMs + FALLBACK_TIMEOUT) {
      lastMs = ms;
      Serial.println("Fallbak: cannot connect");
      goToSleep();
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  bool status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    goToSleep();
  }

  delay(100);

  float temp = bme.readTemperature();
  float humid = bme.readHumidity();
  float pres = bme.readPressure() / 100.0F;
  float alt = bme.readAltitude(SEALEVELPRESSURE_HPA);
  int signalStrength = WiFi.RSSI();

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.println("C");

  Serial.print("Hum: ");
  Serial.print(humid);
  Serial.println("%");

  Serial.print("Pres: ");
  Serial.print(pres);
  Serial.println("hPa");

  Serial.print("Alt: ");
  Serial.print(alt);
  Serial.println("m");

  Serial.print("Sig: ");
  Serial.print(signalStrength);
  Serial.println("dBm");

  if (client.connect(API_HOST, 80)) { //   "184.106.153.149" or api.thingspeak.com
    String postStr = "";
    postStr += "field1=";
    postStr += String(temp);
    postStr += "&field2=";
    postStr += String(humid);
    postStr += "&field3=";
    postStr += String(pres);
    postStr += "&field4=";
    postStr += String(alt);
    postStr += "&field5=";
    postStr += String(signalStrength);
    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + String(API_KEY) + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }

  Serial.println();
  Serial.println("closing connection");
  client.stop();

  // Sleep
  goToSleep();
}

void goToSleep() {
  Serial.println("ESP8266 in sleep mode");
  ESP.deepSleep(SLEEP_TIME_SEC * 1000000);
}


void loop() {
  // keep it empty
}
