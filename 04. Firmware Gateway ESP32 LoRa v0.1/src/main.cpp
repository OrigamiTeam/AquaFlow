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

#define qtdPayload 10
String payloadMQTT[qtdPayload] = {"", "", "", "", "", "", "", "", "", ""};
boolean payloadSalvo = true;

void enviaLora(String _pacote) {
  LoRa.beginPacket();
  LoRa.print(_pacote);
  LoRa.endPacket();
}

void enviaPayloadLoRa() {
  uint8_t _size = 0;
  for (uint8_t _i = 0; _i < qtdPayload; _i++) {
    if (payloadMQTT[_i].compareTo("") != 0) {
      _size = _i + 1;
    }
    else {
      break;
    }
  }

  if (_size) {
    for (uint8_t _i = 0; _i < _size; _i++) {
      enviaLora(payloadMQTT[_i]);
      #if DEBUG
      Serial.print(F("Enviado payload: "));
      Serial.println(payloadMQTT[_i]);
      #endif
      payloadMQTT[_i] = "";
      delay(500);
    }
    #if DEBUG
    Serial.println(F("Todos os payloads foram enviados!"));
    #endif
  }
  else {
    enviaLora(F("{\"o\":\"a\", \"t\":\"0x1FFF\"}"));
    #if DEBUG
    Serial.println(F("Sem payloads para enviar!"));
    #endif
  }
}

boolean salvaPayload(String _payload) {
  for (uint8_t _i = 0; _i < qtdPayload; _i++) {
    if (payloadMQTT[_i].compareTo("") == 0) {
      payloadMQTT[_i] = _payload;
      payloadSalvo = true;
      return true;
    }
  }

  return false;
}

void callbackMQTT(String &_topic, String &_payload) {
  #if DEBUG
  Serial.print("_topic: ");
  Serial.println(_topic);
  Serial.print("_payload: ");
  Serial.println(_payload);
  #endif
  if (!salvaPayload(_payload)) {
    mqtt.publish(IOT_TOPIC_PUB, F("{\"o\":\"a\", \"t\":\"0x1FFE\"}"), false, 0);
    
    #if DEBUG
    Serial.println(F("Erro ao salvar payload!"));
    #endif
  }

  /*if (!_topic.compareTo(IOT_TOPIC_SUB)) {
    enviaLora(_payload);
    Serial.println("Enviado LoRa!");
  }*/
}

boolean verificaPacoteLoRa(String _pacote) {
  uint8_t _tamanho = _pacote.length() + 1;
  char _data[_tamanho];
  _pacote.toCharArray(_data, _tamanho);

  if (_data[0] == '{' && _data[_tamanho - 2] == '}') {
    return true;
  }
  return false;
}

long leValorInt(String _pacote, uint8_t _indiceComando) {
  uint8_t _tamanho = _pacote.length() + 1;
  char _data[_tamanho];
  _pacote.toCharArray(_data, _tamanho);

  uint8_t _indiceSeparador = 0;
  for (uint8_t _i = _indiceComando + 1; _i < _tamanho - 2; _i++) {
    if (_data[_i] == ':') {
      _indiceSeparador = _i;
      break;
    }
  }

  uint8_t _indiceValor = 0;
  for (uint8_t _i = _indiceSeparador + 1; _i < _tamanho - 2; _i++) {
    if (_data[_i] - '0' >= 0 && _data[_i] - '0' <= 9) {
      _indiceValor = _i;
      break;
    }
  }

  String _valorString = "";
  for (uint8_t _i = _indiceValor; _i < _tamanho - 2; _i++) {
    if (_data[_i] - '0' >= 0 && _data[_i] - '0' <= 9) {
      _valorString.concat(_data[_i] - '0');
    }
    else {
      break;
    }
  }

  return _valorString.toInt();
}

void recebeLora() {
  #if DEBUG
  Serial.print(F("Recebido LoRa: "));
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

  if (verificaPacoteLoRa(_LoRaData)) {
    int _indiceComando = _LoRaData.indexOf("\"p\":");
    if (_indiceComando != -1) {

      uint8_t _valorInt = leValorInt(_LoRaData, _indiceComando);
      if (_valorInt == 1) {
        enviaPayloadLoRa();
      }
      else {
        // #####################################################################
        #if DEBUG
        Serial.println(F("Falha! Comando LoRa desconhecido!"));
        #endif
      }
    }
    else {
      // Se não reconhecer comando "p", encaminha pacote para MQTT
      mqtt.publish(IOT_TOPIC_PUB, _LoRaData, false, 0);

      #if DEBUG
      Serial.println(F("Pacote LoRa encaminhado ao MQTT!"));
      #endif
    }
  }
  else {
    // #####################################################################
    #if DEBUG
    Serial.println(F("Erro no pacote LoRa recebido!"));
    #endif
  }
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  //WiFi.begin("QLwifi", "-bMZdXyT6d");
  WiFi.begin("MEO-0BE960", "2bead7a59c");
  //WiFi.begin("Felipe", "00000000");

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
    recebeLora();
  }

  if (millis() > ultimoStatus + 500) {
    ledStatus = !ledStatus;
    digitalWrite(ledPin, ledStatus);
    ultimoStatus = millis();
  }

  delay(10);
}