#include <Arduino.h>

#include <SPI.h>
#include <LoRa.h>

#define ss 23
#define rst 24
#define dio0 7

#define ledPin 6

//unsigned long ultimoStatus = 0;
//boolean ledStatus = false;

unsigned long ultimoEnvio = 0;
int counter = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n\nLoRa Sender");

  LoRa.setPins(ss, rst, dio0);
  
  uint8_t _timeout = 0;
  while (!LoRa.begin(868E6) && _timeout < 15) {
    _timeout++;
    delay(1000);
  }

  #if DEBUG
  if (_timeout >= 15) {
    Serial.println("Falha ao iniciar modulo LoRa!");
  }
  else {
    Serial.println("Modulo LoRa!");
  }
  #endif

  LoRa.setTxPower(20);

  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
}

void loop() {
  if (ultimoEnvio == 0 || millis() > ultimoEnvio + 2000) {
    Serial.print("Sending packet: ");
    Serial.println(counter);

    LoRa.beginPacket();
    LoRa.print("AquaFlow ");
    LoRa.print(counter);
    LoRa.endPacket();

    ultimoEnvio = millis();
    counter++;

    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
  }
  

  /*if (millis() > ultimoStatus + 500) {
    ledStatus = !ledStatus;
    digitalWrite(ledPin, ledStatus);
    ultimoStatus = millis();
  }*/

  delay(10);
}