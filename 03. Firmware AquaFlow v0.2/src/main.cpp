#include <Arduino.h>

#include "MAX35103.h"
#include "valvulaMotorDC.h"
#include "displayST7565R.h"

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

unsigned long leituraAnterior = 0;

MAX35103 MAX;
valvulaMotorDC valvula;
displayST7565R LCD;

// DISPLAY
void volumeLCD() {
  LCD.write(LCD.numeros[0], 0, 14);
  delay(1);
  LCD.write(LCD.numeros[1], 18, 14);
  delay(1);
  LCD.write(LCD.numeros[2], 36, 14);
  delay(1);
  LCD.write(LCD.numeros[3], 54, 14);
  delay(1);
  LCD.write(LCD.numeros[4], 72, 14);
  delay(1);
  LCD.write(LCD.simbolos[0], 89, 2);
  delay(1);
  LCD.write(LCD.numeros[5], 94, 14);
  delay(1);
  LCD.write(LCD.simbolos[1], 113, 15);

  delay(5000);
  LCD.clear(0, 128);
  delay(1);
}

void setup() {
  pinMode(btnPin, INPUT);
  //digitalWrite(btnPin, HIGH); // Pullup externo

  pinMode(pressaoPin, INPUT);

  MAX.begin(MAX35103INT, MAX35103RST, MAX35103CE);

  valvula.begin(SW1Pin, SW2Pin, motorSleepPin, motorIN1Pin, motorIN2Pin);
  valvula.abre();

  LCD.begin(LCD_SI, LCD_SCL, LCD_A0, LCD_RST, LCD_CS);
  delay(10);
  LCD.clear(0, 128);

  delay(100);
  volumeLCD();

  /////////////////////////////////////////////////
  Serial.begin(115200);
  delay(100);

  Serial.println("\n\nIniciando...");
  delay(100);
  /////////////////////////////////////////////////

  if (MAX.reset()) {
    Serial.println("MAX35103 Reset OK!");
  }
  else {
    Serial.println("ERRO no Reset do MAX35103");
  }

  delay(500);

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
  
  MAX.config();

  uint16_t _reg = MAX.readRegister16(0xB8);
  Serial.print("TOF1 0xB8: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xB9);
  Serial.print("TOF2 0xB9: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xBA);
  Serial.print("TOF3 0xBA: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xBB);
  Serial.print("TOF4 0xBB: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xBC);
  Serial.print("TOF5 0xBC: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xBD);
  Serial.print("TOF5 0xBD: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xBE);
  Serial.print("TOF5 0xBE: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xC0);
  Serial.print("_reg 0xC0: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xC1);
  Serial.print("_reg 0xC1: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = MAX.readRegister16(0xC2);
  Serial.print("_reg 0xC2: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  delay(500);

  if (MAX.toFlash()) {
    Serial.println("MAX35103 toFlash OK!");
  }
  else {
    Serial.println("ERRO no toFlash do MAX35103");
  }
  
  delay(500);

  if (MAX.initialize()) {
    Serial.println("Initialize OK!");
  }
  else {
    Serial.println("ERRO no initialize do MAX35103");
  }
}

void loop() {
  if (millis() > leituraAnterior + 1000) {
    leituraAnterior = millis();

    /*Serial.print("ToF Diff: ");
    opcodeCommand(MAX35103_TOF_Diff);

    while (digitalRead(MAX35103INT)) {
      delay(50);
    }

    // interruptStatus(15) indica timeout!!!!!!!!
    if (interruptStatus(12)) {
      Serial.println("OK!");
    }

    uint16_t _TOF_DIFFInt = readRegister16(0xE2);
    uint16_t _TOF_DIFFFrac = readRegister16(0xE3);
    float _TOF_DIFF = TOF_DIF(_TOF_DIFFInt, _TOF_DIFFFrac);
    float _fluxoAgua = fluxoAgua(_TOF_DIFF);

    Serial.print("TOF_DIFF: ");
    Serial.println(_TOF_DIFF);

    Serial.print("_fluxoAgua: ");
    Serial.println(_fluxoAgua);*/

    Serial.print("Temp: ");
    opcodeCommand(MAX35103_Temperature);

    while (digitalRead(MAX35103INT)) {
      delay(50);
    }

    // interruptStatus(15) indica timeout!!!!!!!!
    if (interruptStatus(11)) {
      Serial.println("OK!");
    }

    uint16_t _TxInt = readRegister16(0xE7);
    uint16_t _TxFrac = readRegister16(0xE8);
    float _timeT1 = registerTemp(_TxInt, _TxFrac);

    /*_TxInt = readRegister16(0xE9);
    _TxFrac = readRegister16(0xEA);
    float _timeT2 = registerTemp(_TxInt, _TxFrac);*/

    _TxInt = readRegister16(0xEB);
    _TxFrac = readRegister16(0xEC);
    float _timeT3 = registerTemp(_TxInt, _TxFrac);

    /*_TxInt = readRegister16(0xED);
    _TxFrac = readRegister16(0xEF);
    float _timeT4 = registerTemp(_TxInt, _TxFrac);*/

    float _temperatura1 = temperaturaPT1000(_timeT1, _timeT3);
    //float _temperatura2 = temperaturaPT1000(_timeT2, _timeT4);

    Serial.print("_temperatura1: ");
    Serial.println(_temperatura1, 1);

    /*Serial.print("_temperatura2: ");
    Serial.println(_temperatura2, 1);*/

    uint32_t _pressaoAnalog = analogRead(pressaoPin);
    uint32_t _pressaoBarInt = map(_pressaoAnalog, 95, 870, 0, 690);
    float _pressaoBar = (float)_pressaoBarInt / 100.0;

    Serial.print("_pressao: ");
    Serial.print(_pressaoAnalog);
    Serial.print(" | ");
    Serial.print(_pressaoBar);
    Serial.println(" bar");

    if (_pressaoAnalog < 100) {
      Serial.println("### Ausencia de agua! ###");
    }

    /*
    0 psi = 0,5V
    100 psi = 4,5V com Vcc de 5,0V
    100 psi = 2,8V com Vcc de 3,3V

    1 psi = 0,068947 bar

    leitura analogica 0 = 0V
    leitura analogica 1023 = 3,3V

    0,0 bar = 95
    6,9 bar = 870
    */
  }

  delay(10);
}