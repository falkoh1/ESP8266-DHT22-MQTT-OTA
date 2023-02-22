/* Falko Höltzer
Date created: 12.01.23
Version: V1.2

needed libraries 
DHT sensor library by Adafuit 1.4.4
PubSubClient by Nick ‘O Leary V2.8.0
ArduinoOTA by Juraj Andrassy V1.0.7  
Wire by Ivan Grokhotkov V1.0
Ticker by Ivan Grokhtokov V1.0

board wirering:
DHT22-AM2302  WeMos D1 Mini 
+           3.3V
-           GND
out           D1 / GPIO5
Bridge RST with GPIO16 (D0) - After the sketch has been uploaded to the ESP8266
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Ticker.h>
#include "DHT.h"

#define wifi_ssid "Star-Trek"
#define wifi_password "81167554326413327186"

#define mqtt_server "192.168.1.46"
#define mqtt_user ""
#define mqtt_password ""
#define mqtt_port 1883

#define humidity_topic "esp8266/dht/humidity/keller"
#define temperature_topic "esp8266/dht/temperature/keller"
#define status_topic "esp8266/dht/status/keller"

#define durationSleep 600  // in Sec -> 10min.
#define DHTPIN 14
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

float temp = 0.0;
float hum = 0.0;
float hi = 0.0;
bool status;

void setup() {
  Serial.begin(115200);
  dht.begin();
  if (isnan(hum) || isnan(temp)) {
    Serial.println("DHT22 Sensor not found, check wirering");
    while (1)
      ;
  }

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  hi = dht.computeHeatIndex(temp, hum, false);

  Serial.print(F("Temperature = "));
  Serial.print(String(temp).c_str());
  Serial.println(" °C");
  client.publish(temperature_topic, String(temp).c_str(), true);

  Serial.print(F("Humidity = "));
  Serial.print(String(hum).c_str());
  Serial.println(" %");
  client.publish(humidity_topic, String(hum).c_str(), true);

  Serial.println("Gefühlte Temperatur: " + String(hi) + " °C");

  Serial.print(F("Status = "));
  client.publish(status_topic, "Sen-Keller deep-sleep");
  delay(1000);  //Zeit zum übertrage von den MQTT Daten
  Serial.println("Sensor in deep sleep");
  ESP.deepSleep(durationSleep * 1e6);
}

void loop() {
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  int _try = 0;
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(2000);
    _try++;  // Wenn nach 10 Versuchen nicht mit WiFi verbunden werden kann, deep-sleep
    if (_try >= 10) {
      Serial.println("Kann nicht mit WiFi verbunden werden, gehe in deep-sleep");
      ESP.deepSleep(durationSleep * 1e6);
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Sen-Keller-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(status_topic, "Sen-Keller alive");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}