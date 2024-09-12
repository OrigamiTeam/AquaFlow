#include <Arduino.h>

#include <SPI.h>
#include <LoRa.h>

#define ss 5
#define rst 22
#define dio0 21

#define ledPin 2

unsigned long ultimoStatus = 0;
boolean ledStatus = false;

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n\nLoRa Receiver");

  LoRa.setPins(ss, rst, dio0);
  
  while (!LoRa.begin(868E6)) {
    Serial.println(".");
    delay(500);
  }

  //LoRa.setGain(3);
  
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  if (LoRa.parsePacket()) {
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
  }

  delay(10);
}