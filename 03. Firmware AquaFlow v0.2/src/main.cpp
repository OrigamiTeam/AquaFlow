#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>
#include <avr/pgmspace.h>

#include "MAX35103.h"
#include "valvulaMotorDC.h"
#include "displayST7565R.h"

#define DEBUG true

// Pino para botao com acesso externo
#define btnPin 2

// Pinos para interface com MAX35103
#define MAX35103INT 3
#define MAX35103RST 4
#define MAX35103CE 10

// Pinos para acionamento do driver de motor DC e sensores fim de curso
#define SW1Pin 5
#define SW2Pin 6
#define motorSleepPin 7
#define motorIN1Pin 8
#define motorIN2Pin 9

// Pino para leitura analogica do sensor de pressao
#define pressaoPin 14

// Pinos para interface com display NHD-C12832A1Z-FSW-FBW-3V3
#define LCD_SI 15
#define LCD_SCL 16
#define LCD_A0 17
#define LCD_LED 23
#define LCD_RST 25
#define LCD_CS 26

// Pinos para interface externa com modulos de comunicacao (SPI compartilhado com SS dedicado, RS-232 dedicado e I2C dedicado)
#define RS232_RX_EXT 0
#define RX232_TX_EXT 1
#define I2C_SDA_EXT 18
#define I2C_SCL_EXT 19
#define SPI_SS_EXT 24

// ### MODULO LoRa ###

#define LoRaSS I2C_SDA_EXT
#define LoRaRST I2C_SCL_EXT
#define LoRaDIO0 SPI_SS_EXT

#define tempoInfoLCD 10000

const unsigned char numeros [10][56] PROGMEM = {
  {0xF8, 0xF8, 0xF8, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0xF8, 0xF8, 0xF8, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x0F, 0x0F, 0x0F, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F},
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xF8, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F},
  {0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0xF8, 0xF8, 0xF8, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0F, 0x0F, 0x0F, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E},
  {0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0xF8, 0xF8, 0xF8, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF, 0xFF, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F},
  {0xF8, 0xF8, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xF8, 0xF8, 0xFF, 0xFF, 0xFF, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF, 0xFF, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F},
  {0xF8, 0xF8, 0xF8, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0xFF, 0xFF, 0xFF, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF, 0xFF, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F},
  {0xF8, 0xF8, 0xF8, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF, 0xFF, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF, 0xFF, 0x0F, 0x0F, 0x0F, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F},
  {0xF8, 0xF8, 0xF8, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0xF8, 0xF8, 0xF8, 0x3F, 0x3F, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F},
  {0xF8, 0xF8, 0xF8, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0xF8, 0xF8, 0xF8, 0xFF, 0xFF, 0xFF, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF, 0xFF, 0x0F, 0x0F, 0x0F, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F},
  {0xF8, 0xF8, 0xF8, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0xF8, 0xF8, 0xF8, 0xFF, 0xFF, 0xFF, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF, 0xFF, 0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F}
};

const unsigned char simboloPonto [8] PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x0E};
const unsigned char simboloM3 [60] PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x60, 0x60, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0x66, 0x66, 0x7F, 0x7F, 0xFE, 0xFE, 0x06, 0x06, 0x06, 0x7E, 0x7E, 0x06, 0x06, 0x06, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00};
const unsigned char simboloLMin [168] PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0xFE, 0x1E, 0x00, 0x00, 0xFE, 0xFE, 0x06, 0x06, 0x06, 0x7E, 0x7E, 0x06, 0x06, 0x06, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0xF6, 0xF6, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0x06, 0x06, 0x06, 0xFE, 0xFE, 0x0F, 0x0F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x0F, 0x0F, 0x01, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x0F};
const unsigned char simboloCelsius [48] PROGMEM = {0x38, 0x38, 0x38, 0x00, 0x00, 0xF8, 0xF8, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x3F, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const unsigned char simboloBar [108] PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0x66, 0x66, 0x66, 0xFE, 0xFC, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0x66, 0x66, 0x66, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0xE6, 0xE6, 0xE6, 0x7E, 0x7E, 0x0F, 0x0F, 0x0C, 0x0C, 0x0C, 0x0F, 0x07, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x01, 0x03, 0x0F, 0x0E};

const unsigned char simboloAviso [2][100] PROGMEM = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xE0, 0x70, 0x18, 0x18, 0x18, 0x70, 0xE0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xE0, 0x78, 0x1E, 0x07, 0x01, 0x00, 0xFC, 0xFC, 0xFC, 0x00, 0x01, 0x07, 0x1E, 0x78, 0xE0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0xE0, 0x78, 0x1E, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE7, 0xE7, 0xE7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x1E, 0x78, 0xE0, 0x80, 0x03, 0x07, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0E, 0x07, 0x03},
                                                     {0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xE0, 0xF0, 0xF0, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF0, 0xF0, 0xE0, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0xF0, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xF9, 0xF1, 0xE3, 0xC7, 0x8F, 0x1F, 0x3F, 0x1F, 0x8F, 0xC7, 0xE3, 0xF1, 0xF9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xF0, 0x07, 0x1F, 0x7F, 0xFF, 0xFF, 0xFF, 0xCF, 0xC7, 0xE3, 0xF1, 0xF8, 0xFC, 0xFE, 0xFC, 0xF8, 0xF1, 0xE3, 0xC7, 0xCF, 0xFF, 0xFF, 0xFF, 0x7F, 0x1F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x07, 0x07, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x07, 0x07, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00}};

unsigned long leituraAnterior = 0;

MAX35103 MAX;
valvulaMotorDC valvula;
displayST7565R LCD;

JsonDocument avisos; ////////////////// ALTERAR PARA uint16_t avisos[30] registrando apenas o código

void logAviso(uint16_t _codigo) {
  avisos["o"].add("e"); // Operacao erro
  avisos["ts"].add(millis()); // timestamp
  avisos["c"].add(_codigo); // codigo
}

// ### INICIO MAX35103 ###

/*uint16_t _dado = readFlash16(0x0000);
  Serial.print("_dado: ");
  Serial.println(_dado, HEX);
  delay(500);

  eraseFlash(0x0000);
  delay(100);

  while (interruptStatus(7) == 0) {
    delay(100);
  }
  Serial.println("Erase OK!");

  delay(500);

  _dado = readFlash16(0x0000);
  Serial.print("_dado: ");
  Serial.println(_dado, HEX);
  delay(500);

  writeFlash16(0x0000, 0x1234);
  delay(100);

  while (interruptStatus(7) == 0) {
    delay(100);
  }
  Serial.println("Write OK!");

  delay(500);

  _dado = readFlash16(0x0000);
  Serial.print("_dado: ");
  Serial.println(_dado, HEX);

  delay(500);*/

#if DEBUG
void printConfigMAX() {
  uint16_t _reg = MAX.readRegister16(0xB8);
  Serial.print(F("TOF1 0xB8: 0x"));
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xB9);
  Serial.print(F("TOF2 0xB9: 0x"));
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xBA);
  Serial.print(F("TOF3 0xBA: 0x"));
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xBB);
  Serial.print(F("TOF4 0xBB: 0x"));
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xBC);
  Serial.print(F("TOF5 0xBC: 0x"));
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xBD);
  Serial.print(F("TOF5 0xBD: 0x"));
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xBE);
  Serial.print(F("TOF5 0xBE: 0x"));
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xC0);
  Serial.print(F("_reg 0xC0: 0x"));
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xC1);
  Serial.print(F("_reg 0xC1: 0x"));
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xC2);
  Serial.print(F("_reg 0xC2: 0x"));
  Serial.println(_reg, HEX);
}
#endif

// ### FINAL MAX35103 ###

// ### INICIO DISPLAY ###

void PGMReadNumeros(unsigned char *_vetor, uint8_t _posicao) {
  for (uint8_t _i = 0; _i < 56; _i++) {
    _vetor[_i] = pgm_read_byte(&(numeros[_posicao][_i]));
  }
}

void PGMreadSimboloPonto(unsigned char *_vetor) {
  for (uint8_t _i = 0; _i < 8; _i++) {
    _vetor[_i] = pgm_read_byte(&(simboloPonto[_i]));
  }
}

void PGMreadSimboloM3(unsigned char *_vetor) {
  for (uint8_t _i = 0; _i < 60; _i++) {
    _vetor[_i] = pgm_read_byte(&(simboloM3[_i]));
  }
}

void PGMreadSimboloLMin(unsigned char *_vetor) {
  for (uint8_t _i = 0; _i < 168; _i++) {
    _vetor[_i] = pgm_read_byte(&(simboloLMin[_i]));
  }
}

void PGMreadSimboloCelsius(unsigned char *_vetor) {
  for (uint8_t _i = 0; _i < 48; _i++) {
    _vetor[_i] = pgm_read_byte(&(simboloCelsius[_i]));
  }
}

void PGMreadSimboloBar(unsigned char *_vetor) {
  for (uint8_t _i = 0; _i < 108; _i++) {
    _vetor[_i] = pgm_read_byte(&(simboloBar[_i]));
  }
}

void PGMreadSimboloAviso(unsigned char *_vetor, boolean _erro) {
  for (uint8_t _i = 0; _i < 100; _i++) {
    _vetor[_i] = pgm_read_byte(&(simboloAviso[_erro][_i]));
  }
}

// _volumeInt deve ser um numero uint32_t formado pelo volume em float multiplicado por 10, ou seja, a unidade sera exibida como casa decimal
void volumeLCD(uint32_t _volumeInt) {
  String _string = "";

  if (_volumeInt < 100000) {
    _string.concat("0");
  }
  if (_volumeInt < 10000) {
    _string.concat("0");
  }
  if (_volumeInt < 1000) {
    _string.concat("0");
  }
  if (_volumeInt < 100) {
    _string.concat("0");
  }
  if (_volumeInt < 10) {
    _string.concat("0");
  }
  
  _string.concat(_volumeInt);
  uint32_t _length = _string.length();

  unsigned char _char [60] = {};

  PGMReadNumeros(_char, _string[_length - 6] - '0');
  LCD.writeNumber(_char, 0);

  PGMReadNumeros(_char, _string[_length - 5] - '0');
  LCD.writeNumber(_char, 18);
  
  PGMReadNumeros(_char, _string[_length - 4] - '0');
  LCD.writeNumber(_char, 36);
  
  PGMReadNumeros(_char, _string[_length - 3] - '0');
  LCD.writeNumber(_char, 54);
  
  PGMReadNumeros(_char, _string[_length - 2] - '0');
  LCD.writeNumber(_char, 72);

  // simbolo de . (ponto decimal)
  PGMreadSimboloPonto(_char);
  LCD.writeSymbol(_char, 89, 2); // 2 colunas no display
  
  PGMReadNumeros(_char, _string[_length - 1] - '0');
  LCD.writeNumber(_char, 94);
  
  // simbolo de m3 (metro cubico)
  PGMreadSimboloM3(_char);
  LCD.writeSymbol(_char, 113, 15); // 15 colunas no display
}

// _fluxoInt deve ser um numero uint32_t formado pelo fluxo em float multiplicado por 10, ou seja, a unidade sera exibida como casa decimal
void fluxoLCD(uint32_t _fluxoInt, uint8_t _inicio) {
  String _string = "";

  if (_fluxoInt < 100) {
    _string.concat("0");
  }
  if (_fluxoInt < 10) {
    _string.concat("0");
  }
  
  _string.concat(_fluxoInt);
  uint32_t _length = _string.length();

  unsigned char _char [168] = {};
  
  PGMReadNumeros(_char, _string[_length - 3] - '0');
  LCD.writeNumber(_char, _inicio);
  
  PGMReadNumeros(_char, _string[_length - 2] - '0');
  LCD.writeNumber(_char, _inicio + 18);

  // simbolo de . (ponto decimal)
  PGMreadSimboloPonto(_char);
  LCD.writeSymbol(_char, _inicio + 35, 2); // 2 colunas no display
  
  PGMReadNumeros(_char, _string[_length - 1] - '0');
  LCD.writeNumber(_char, _inicio + 40);
  
  // simbolo de L/MIN (litro por minuto)
  PGMreadSimboloLMin(_char);
  LCD.writeSymbol(_char, _inicio + 59, 42); // 42 colunas no display
}

// _tempInt deve ser um numero uint32_t formado pela temperatura em float multiplicado por 10, ou seja, a unidade sera exibida como casa decimal
void temperaturaLCD(uint32_t _tempInt, uint8_t _inicio) {
  String _string = "";

  if (_tempInt < 100) {
    _string.concat("0");
  }
  if (_tempInt < 10) {
    _string.concat("0");
  }
  
  _string.concat(_tempInt);
  uint32_t _length = _string.length();

  unsigned char _char [56] = {};
  
  PGMReadNumeros(_char, _string[_length - 3] - '0');
  LCD.writeNumber(_char, _inicio);
  
  PGMReadNumeros(_char, _string[_length - 2] - '0');
  LCD.writeNumber(_char, _inicio + 18);

  // simbolo de . (ponto decimal)
  PGMreadSimboloPonto(_char);
  LCD.writeSymbol(_char, _inicio + 35, 2); // 2 colunas no display
  
  PGMReadNumeros(_char, _string[_length - 1] - '0');
  LCD.writeNumber(_char, _inicio + 40);
  
  // simbolo de ºC (celsius)
  PGMreadSimboloCelsius(_char);
  LCD.writeSymbol(_char, _inicio + 59, 12); // 12 colunas no display
}

// _presaoInt deve ser um numero uint32_t formado pela temperatura em float multiplicado por 10, ou seja, a unidade sera exibida como casa decimal
void pressaoLCD(uint32_t _presaoInt, uint8_t _inicio) {
  String _string = "";

  if (_presaoInt < 100) {
    _string.concat("0");
  }
  if (_presaoInt < 10) {
    _string.concat("0");
  }
  
  _string.concat(_presaoInt);
  uint32_t _length = _string.length();

  unsigned char _char [108] = {};
  
  PGMReadNumeros(_char, _string[_length - 3] - '0');
  LCD.writeNumber(_char, _inicio);
  
  PGMReadNumeros(_char, _string[_length - 2] - '0');
  LCD.writeNumber(_char, _inicio + 18);

  // simbolo de . (ponto decimal)
  PGMreadSimboloPonto(_char);
  LCD.writeSymbol(_char, _inicio + 35, 2); // 2 colunas no display
  
  PGMReadNumeros(_char, _string[_length - 1] - '0');
  LCD.writeNumber(_char, _inicio + 40);
  
  // simbolo de bar
  PGMreadSimboloBar(_char);
  LCD.writeSymbol(_char, _inicio + 59, 27); // 27 colunas no display
}

// _codigo deve ser um numero uint32_t que indica o numero da mensagem e o parametro _erro indica se o simbolo sera de aviso ou erro
void avisoLCD(uint32_t _codigo, boolean _erro, uint8_t _inicio) {
  String _string = "";

  if (_codigo < 1000) {
    _string.concat("0");
  }
  if (_codigo < 100) {
    _string.concat("0");
  }
  if (_codigo < 10) {
    _string.concat("0");
  }
  
  _string.concat(_codigo);
  uint32_t _length = _string.length();

  unsigned char _char [100] = {};

  // simbolo de aviso ou erro
  PGMreadSimboloAviso(_char, _erro);
  LCD.writeSymbol(_char, _inicio, 25); // 25 colunas no display
  
  PGMReadNumeros(_char, _string[_length - 4] - '0');
  LCD.writeNumber(_char, _inicio + 29);
  
  PGMReadNumeros(_char, _string[_length - 3] - '0');
  LCD.writeNumber(_char, _inicio + 47);
  
  PGMReadNumeros(_char, _string[_length - 2] - '0');
  LCD.writeNumber(_char, _inicio + 65);

  PGMReadNumeros(_char, _string[_length - 1] - '0');
  LCD.writeNumber(_char, _inicio + 83);
}

// ### FINAL DISPLAY ###

// ### INICIO LORA

void enviaLora(String _pacote) {
  LoRa.beginPacket();
  LoRa.print(_pacote);
  LoRa.endPacket();
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
  Serial.println(_LoRaData);
  #endif

  JsonDocument _lora;
  DeserializationError _erro = deserializeJson(_lora, _LoRaData);

  if (!_erro) {
    if (!_lora["c"].isNull()) {
      if (_lora["c"] == 1) {
        if(valvula.fecha()) {

          _lora.clear();
          _lora["c"] = 1;
          _lora["p"] = "ok";
          String _pacote = "";
          serializeJson(_lora, _pacote);
          enviaLora(_pacote);

          #if DEBUG
          Serial.println(F("Valvula Fechada!"));
          #endif
        }
        else {
          logAviso(0x1009);

          #if DEBUG
          Serial.println(F("Falha ao fechar valvula!"));
          #endif
        }
      }
      else if (_lora["c"] == 2) {
        if(valvula.abre()) {

          _lora.clear();
          _lora["c"] = 2;
          _lora["p"] = "ok";
          String _pacote = "";
          serializeJson(_lora, Serial);
          serializeJson(_lora, _pacote);
          enviaLora(_pacote);

          #if DEBUG
          Serial.println(F("Valvula Aberta!"));
          #endif
        }
        else {
          logAviso(0x100A);

          #if DEBUG
          Serial.println(F("Falha ao abrir valvula!"));
          #endif
        }
      }
      else if (_lora["c"] == 3) {
        if (!_lora["p"].isNull()) {
          String _tempoString = _lora["p"];
          if(valvula.abreParcial(_tempoString.toInt())) {

            _lora.clear();
            _lora["c"] = 3;
            _lora["p"] = "ok";
            String _pacote = "";
            serializeJson(_lora, _pacote);
            enviaLora(_pacote);

            #if DEBUG
            Serial.println(F("Valvula com Abertura Parcial!"));
            #endif
          }
          else {
            logAviso(0x100C);

            #if DEBUG
            Serial.println(F("Falha ao realizar abertura parcial da valvula!"));
            #endif
          }
        }
        else {
          logAviso(0x100B);

          #if DEBUG
          Serial.println(F("Falha! Tempo de abertura parcial ausente!"));
          #endif
        }
      }
      else {
        logAviso(0x1008);

        #if DEBUG
        Serial.println(F("Falha! Comando LoRa desconhecido!"));
        #endif
      }
    }
    else {
      logAviso(0x1007);

      #if DEBUG
      Serial.println(F("Falha! Pacote LoRa desconhecido!"));
      #endif
    }
  }
  else {
    logAviso(0x1006);

    #if DEBUG
    Serial.print(F("Erro no deserializeJson(): "));
    Serial.println(_erro.f_str());
    #endif
  }
}

// ### FINAL LORA ###

void displayInfo() {
  uint8_t _size = avisos["o"].size();
  uint16_t _avisos[_size];
  if (_size) {
    for (uint8_t _i = 0; _i < _size; _i++) {
      uint16_t _cod = avisos["c"][_i];

      boolean _erro = false;
      uint8_t _tipo = _cod >> 12;
      if (_tipo >= 0x2) {
        _erro = true;
      }

      _avisos[_i] = _cod & 0xFFF;

      #if DEBUG
      Serial.print(F("_erro: "));
      Serial.print(_erro);
      Serial.print(F(" | _cod: 0x"));
      Serial.println(_avisos[_i], HEX);
      #endif

      if(_avisos[_i] != 0x0000) {
        LCD.clear(0, 128);
        delay(1);
        avisoLCD(_avisos[_i], _erro, 15);

        unsigned long _millis = millis();
        boolean _aguardar = true;
        while (_aguardar) {
          if (millis() > _millis + tempoInfoLCD) {
            LCD.clear(0, 128);
            delay(1);
            return;
          }

          if (!digitalRead(btnPin)) {
            delay(20);
            if (!digitalRead(btnPin)) {
              while (!digitalRead(btnPin));
              _aguardar = false;
            }
          }

          delay(10);
        }
      }
    }
  }

  for (uint8_t _i = 0; _i < 4; _i++) {
    LCD.clear(0, 128);
    delay(1);

    switch (_i) {
      case 0:
        volumeLCD(234567);
        break;

      case 1:
        fluxoLCD(712, 14); // inicio em 14
        break;

      case 2:
        temperaturaLCD(293, 29); // inicio em 29
        break;

      case 3:
        pressaoLCD(53, 21); // inicio em 21
        break;
    }

    unsigned long _millis = millis();
    boolean _aguardar = true;
    while (_aguardar) {
      if(millis() > _millis + tempoInfoLCD) {
        LCD.clear(0, 128);
        delay(1);
        return;
      }

      if (!digitalRead(btnPin)) {
        delay(20);
        if (!digitalRead(btnPin)) {
          while (!digitalRead(btnPin));
          _aguardar = false;
        }
      }

      delay(10);
    }
  }

  LCD.clear(0, 128);
  delay(1);
}

void setup() {
  #if DEBUG
  Serial.begin(115200);
  delay(10);
  Serial.println(F("\n\nIniciando..."));
  #endif

  LCD.begin(LCD_SI, LCD_SCL, LCD_A0, LCD_RST, LCD_CS);
  delay(10);
  LCD.clear(0, 128);

  pinMode(LCD_LED, OUTPUT);
  digitalWrite(LCD_LED, LOW);

  /*////////////////////////////////////////////////
  //digitalWrite(LCD_LED, HIGH);
  //delay(100);

  while (true) {
    avisoLCD(404, false, 15);
    delay(3000);
    LCD.clear(0, 128);
    delay(1);

    avisoLCD(500, true, 15);
    delay(3000);
    LCD.clear(0, 128);
    delay(1);

    volumeLCD(234567);
    delay(3000);
    LCD.clear(0, 128);
    delay(1);

    fluxoLCD(712, 14); // inicio em 14
    delay(3000);
    LCD.clear(0, 128);
    delay(1);

    temperaturaLCD(293, 29); // inicio em 29
    delay(3000);
    LCD.clear(0, 128);
    delay(1);

    pressaoLCD(53, 21); // inicio em 21
    delay(3000);
    LCD.clear(0, 128);
    delay(1);
  }
  
  digitalWrite(LCD_LED, LOW);
  */////////////////////////////////////////////////

  pinMode(btnPin, INPUT);
  //digitalWrite(btnPin, HIGH); // Pullup externo

  pinMode(pressaoPin, INPUT);

  logAviso(0x0000); // Inicio do programa

  SPI.begin();

  delay(100);
  MAX.begin(MAX35103INT, MAX35103RST, MAX35103CE);

  delay(100);
  if (!MAX.reset()) {
    logAviso(0x2001);
    
    #if DEBUG
    Serial.println(F("ERRO no Reset do MAX35103"));
    #endif
  }
  #if DEBUG
  else {
    Serial.println(F("MAX35103 Reset OK!"));
  }
  #endif

  delay(100);
  MAX.config();

  #if DEBUG
  delay(100);
  printConfigMAX();
  #endif

  delay(100);
  if (!MAX.toFlash()) {
    logAviso(0x2002);

    #if DEBUG
    Serial.println(F("ERRO no toFlash do MAX35103"));
    #endif
  }
  #if DEBUG
  else {
    Serial.println(F("MAX35103 toFlash OK!"));
  }
  #endif
  
  delay(100);
  if (!MAX.initialize()) {
    logAviso(0x2003);

    #if DEBUG
    Serial.println(F("ERRO no initialize do MAX35103"));
    #endif
  }
  #if DEBUG
  else {
    Serial.println(F("Initialize OK!"));
  }
  #endif

  valvula.begin(SW1Pin, SW2Pin, motorSleepPin, motorIN1Pin, motorIN2Pin);
  
  if (!valvula.abre()) {
    logAviso(0x1004);
    
    #if DEBUG
    Serial.println(F("Falha ao abrir valvula!"));
    #endif
  }
  #if DEBUG
  else {
    Serial.println(F("Valvula aberta!"));
  }
  #endif

  LoRa.setPins(LoRaSS, LoRaRST, LoRaDIO0);

  uint8_t _timeout = 0;
  while (!LoRa.begin(868E6) && _timeout < 5) {
    _timeout++;
    delay(1000);
  }

  if (_timeout >= 5) {
    logAviso(0x1005);

    #if DEBUG
    Serial.println(F("Falha ao iniciar modulo LoRa!"));
    #endif
  }
  #if DEBUG
  else {
    Serial.println(F("Modulo LoRa iniciado!"));
  }
  #endif

  LoRa.setTxPower(20);

  // Change sync word (0xAF) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xAF);

  #if DEBUG
  Serial.println(F("Setup OK!"));
  #endif

  ///////////////////////////////////////////////////////////////////////////////////////////
  serializeJson(avisos, Serial);
  Serial.println("");
  ///////////////////////////////////////////////////////////////////////////////////////////
}

void loop() {
  if (millis() > leituraAnterior + 3000) {
    
    /*float _fluxo = 0.0;
    if (MAX.fluxoToFDIff(&_fluxo)) {

      JsonDocument doc;

      // Add values in the document
      doc["f"] = _fluxo;

      //JsonArray data = doc["data"].to<JsonArray>();
      //data.add(48.756080);
      //data.add(2.302038);

      char output[255];
      serializeJson(doc, output);

      enviaLora(String(output));

      #if DEBUG
      Serial.println(String(output));
      #endif
    }
    else {
      enviaLora("f: ERRO");

      #if DEBUG
      Serial.println(F("Erro ao ler fluxo!"));
      #endif
    }*/

    /*float _temperatura1 = 0.0;
    if (MAX.temperatura(1, &_temperatura1)) {
      Serial.print(F("Temp1: "));
      Serial.println(_temperatura1, 1);
    }
    else {
      Serial.println(F("Erro ao ler temperatura!"));
    }*/

    /*uint32_t _pressaoAnalog = analogRead(pressaoPin);
    uint32_t _pressaoBarInt = map(_pressaoAnalog, 95, 870, 0, 690);
    float _pressaoBar = (float)_pressaoBarInt / 100.0;

    Serial.print("_pressao: ");
    Serial.print(_pressaoAnalog);
    Serial.print(" | ");
    Serial.print(_pressaoBar);
    Serial.println(" bar");

    if (_pressaoAnalog < 100) {
      Serial.println(F("### Ausencia de agua! ###"));
    }*/

    leituraAnterior = millis();
  }

  if (LoRa.parsePacket()) {
    recebeLora();
  }

  if (!digitalRead(btnPin)) {
    delay(20);
    if (!digitalRead(btnPin)) {
      while (!digitalRead(btnPin));
      displayInfo();
    }
  }

  delay(10);
}