#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FS.h>
#include <SPIFFS.h>
#include <MQTT.h>

#define MQTT_BUFFER_SIZE 4096
#define IOT_ENDPOINT "endpoint"
#define IOT_PORTA 8883
#define DEVICE_NAME "O mesmo que esta no IoT Core"
#define IOT_TOPIC_SUB "sub"
#define IOT_TOPIC_PUB "pub"

#define DEBUG true

#define ss 5
#define rst 22
#define dio0 21

#define ledPin 2

unsigned long ultimoStatus = 0;
boolean ledStatus = false;

WiFiClientSecure wifiClient = WiFiClientSecure();
MQTTClient mqtt = MQTTClient(MQTT_BUFFER_SIZE);

unsigned long millisReconectWiFi = 0;
unsigned long millisReconectAWS = 0;

void callbackMQTT(String &_topic, String &_payload) {
  Serial.print("_topic: ");
  Serial.println(_topic);
  Serial.print("_payload: ");
  Serial.println(_payload);
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin("QLwifi", "-bMZdXyT6d");

  File _ca1 = SPIFFS.open("/AmazonRootCA1.pem");
  File _crt = SPIFFS.open("/certificate.pem.crt");
  File _key = SPIFFS.open("/private.pem.key");

  wifiClient.loadCACert(_ca1, _ca1.size());
  wifiClient.loadCertificate(_crt, _crt.size());
  wifiClient.loadPrivateKey(_key, _key.size());

  mqtt.begin(IOT_ENDPOINT, IOT_PORTA, wifiClient);
  mqtt.onMessage(callbackMQTT);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  #if DEBUG
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\nLoRa Gateway WiFi");
  #endif

  setupWiFi();

  /*LoRa.setPins(ss, rst, dio0);
  
  while (!LoRa.begin(868E6)) {
    Serial.println(".");
    delay(500);
  }

  //LoRa.setGain(3);
  
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);*/

  Serial.println("Pronto!");
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    if (millis() > millisReconectWiFi + 4000) {
      if (WiFi.reconnect() == ESP_OK) {
        #if DEBUG
        Serial.println("Conectado ao WiFi!");
        #endif
      }
      millisReconectWiFi = millis();
    }
  }
  else {
    mqtt.loop();
    delay(10);
    if (!mqtt.connected()) {
      if (millis() > millisReconectAWS + 5000) {
        if (mqtt.connect(DEVICE_NAME, "public", "public", false)) {
          #if DEBUG
          Serial.println("Conectado ao AWS IoT Core!");
          #endif
          mqtt.subscribe(IOT_TOPIC_SUB);
        }
        millisReconectAWS = millis();
      }
    }
  }

  //mqtt.publish(IOT_TOPIC_PUB, "mensagem", false, 0);

  /*if (LoRa.parsePacket()) {
    Serial.print("Received packet '");

    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData); 
    }

    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }

  if (millis() > ultimoStatus + 500) {
    ledStatus = !ledStatus;
    digitalWrite(ledPin, ledStatus);
    ultimoStatus = millis();
  }*/

  delay(10);
}