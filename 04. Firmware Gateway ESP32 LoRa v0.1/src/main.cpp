#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FS.h>
#include <SPIFFS.h>
#include <MQTT.h>

#define MQTT_BUFFER_SIZE 4096
#define IOT_ENDPOINT "a2ed3xy6iprkkz-ats.iot.eu-central-1.amazonaws.com"
#define IOT_PORTA 8883
#define DEVICE_NAME "AF001"
#define IOT_TOPIC_SUB "af/AF001/d"
#define IOT_TOPIC_PUB "af/AF001/s"

#define DEBUG true

#define SPI_MISO 12
#define SPI_MOSI 13
#define SPI_SCK 14

#define LoRaSS 32 // 32 - 25
#define LoRaRST 33 // 33 - 2
#define LoRaDIO0 36 // 36 - 35

#define ledPin 21

unsigned long ultimoStatus = 0;
boolean ledStatus = false; // logica invertida: false = ligado

WiFiClientSecure wifiClient = WiFiClientSecure();
MQTTClient mqtt = MQTTClient(MQTT_BUFFER_SIZE);

unsigned long millisReconectWiFi = 0;
unsigned long millisReconectAWS = 0;

void enviaLora(String _pacote) {
  LoRa.beginPacket();
  LoRa.print(_pacote);
  LoRa.endPacket();
}

void callbackMQTT(String &_topic, String &_payload) {
  #if DEBUG
  Serial.print("_topic: ");
  Serial.println(_topic);
  Serial.print("_payload: ");
  Serial.println(_payload);
  #endif
  if (!_topic.compareTo(IOT_TOPIC_SUB)) {
    enviaLora(_payload);
    Serial.println("Enviado LoRa!");
  }
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  //WiFi.begin("QLwifi", "-bMZdXyT6d");
  WiFi.begin("MEO-0BE960", "2bead7a59c");

  if(!SPIFFS.begin(true)) {
    Serial.println("Erro ao montar SPIFFS!");
    return;
  }

  File _ca1 = SPIFFS.open("/AmazonRootCA1.pem");
  if (!_ca1) {
    Serial.println("Erro ao abrir CACert!");
    return;
  }

  File _crt = SPIFFS.open("/certificate.pem.crt");
  if (!_ca1) {
    Serial.println("Erro ao abrir Certificate!");
    return;
  }

  File _key = SPIFFS.open("/private.pem.key");
  if (!_ca1) {
    Serial.println("Erro ao abrir PrivateKey!");
    return;
  }

  wifiClient.loadCACert(_ca1, _ca1.size());
  wifiClient.loadCertificate(_crt, _crt.size());
  wifiClient.loadPrivateKey(_key, _key.size());

  mqtt.begin(IOT_ENDPOINT, IOT_PORTA, wifiClient);
  mqtt.onMessage(callbackMQTT);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, ledStatus);

  #if DEBUG
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\nLoRa Gateway WiFi");
  #endif

  setupWiFi();

  #if DEBUG
  Serial.println("wifiClient e MQTT configurados!");
  #endif

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  LoRa.setPins(LoRaSS, LoRaRST, LoRaDIO0);
  
  while (!LoRa.begin(868E6)) {
    #if DEBUG
    Serial.println("Falha ao iniciar modulo LoRa!");
    #endif
    delay(1000);
  }

  //LoRa.setGain(3);
  
  // Change sync word (0xAF) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xAF);

  #if DEBUG
  Serial.println("Pronto!");
  #endif
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

  if (LoRa.parsePacket()) {
    #if DEBUG
    Serial.print("Received packet: ");
    #endif

    String _LoRaData = "";
    while (LoRa.available()) {
      _LoRaData.concat(LoRa.readString());
    }

    #if DEBUG
    Serial.print(_LoRaData); 
    Serial.print(" | RSSI: ");
    Serial.println(LoRa.packetRssi());
    #endif

    mqtt.publish(IOT_TOPIC_PUB, _LoRaData, false, 0);
  }

  if (millis() > ultimoStatus + 500) {
    ledStatus = !ledStatus;
    digitalWrite(ledPin, ledStatus);
    ultimoStatus = millis();
  }

  delay(10);
}