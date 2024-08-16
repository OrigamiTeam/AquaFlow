#include <Arduino.h>
#include <SPI.h>

#define SPI_CLOCK 4000000

#define MAX35103_TOF_Up 0x00
#define MAX35103_TOF_Down 0x01
#define MAX35103_TOF_Diff 0x02
#define MAX35103_Temperature 0x03
#define MAX35103_Reset 0x04
#define MAX35103_Initialize 0x05
#define MAX35103_ToFlash 0x06

#define MAX35103_Flash_Read 0x90
#define MAX35103_Flash_Write 0x10
#define MAX35103_Flash_Erase 0x13
#define MAX35103_LDO_Timed 0x0B
#define MAX35103_LDO_ON 0x0C
#define MAX35103_LDO_OFF 0x0D

#define MAX35103_TOF1_W 0x38 // R: 0xB8
#define MAX35103_TOF2_W 0x39 // R: 0xB9
#define MAX35103_TOF3_W 0x3A // R: 0xBA
#define MAX35103_TOF4_W 0x3B // R: 0xBB
#define MAX35103_TOF5_W 0x3C // R: 0xBC
#define MAX35103_TOF6_W 0x3D // R: 0xBD
#define MAX35103_TOF7_W 0x3E // R: 0xBE
#define MAX35103_EVT_TMN_W 0x40 // R: 0xC0
#define MAX35103_TOF_MES_W 0x41 // R: 0xC1
#define MAX35103_CLB_CTR_W 0x42 // R: 0xC2

#define MAX35103_INT_STATUS_R 0xFE

#define MAX35103INT 3
#define MAX35103CE 10

#define ledPin 6
#define btnPin 7

unsigned long leituraAnterior = 0;

uint16_t readRegister16(uint8_t _address) {
  digitalWrite(MAX35103CE, LOW);
  delay(10);

  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE1));

  SPI.transfer(_address);
  uint8_t _v1 = SPI.transfer(0x00);
  uint8_t _v2 = SPI.transfer(0x00);
  uint16_t _value = 0;
  _value |=  ((uint16_t) _v1) << 8;
  _value |=  ((uint16_t) _v2);

  SPI.endTransaction();

  delay(10);
  digitalWrite(MAX35103CE, HIGH);
  return _value;
}

void writeRegister16(uint8_t _address, uint16_t _value) {
  uint8_t _v1 = (_value >> 8) & 0xFF;
  uint8_t _v2 = _value & 0xFF;
  digitalWrite(MAX35103CE, LOW);
  delay(10);

  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE1));

  SPI.transfer(_address);
  SPI.transfer(_v1);
  SPI.transfer(_v2);

  SPI.endTransaction();

  delay(10);
  digitalWrite(MAX35103CE, HIGH);
}

void opcodeCommand(uint8_t _command) {
  digitalWrite(MAX35103CE, LOW);
  delay(10);

  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE1));

  SPI.transfer(_command);

  SPI.endTransaction();

  delay(10);
  digitalWrite(MAX35103CE, HIGH);
}

boolean interruptStatus(uint8_t _bit) {
  uint16_t _status = readRegister16(MAX35103_INT_STATUS_R);

  /*Serial.print("_status: ");
  Serial.print(_status, BIN);
  Serial.print(" | ");
  Serial.println(_status);*/

  return (_status & (1 << _bit));
}

uint16_t readFlash16(uint16_t _address) {
  opcodeCommand(MAX35103_LDO_Timed);
  delay(100);

  while (interruptStatus(10) == 0) {
    delay(100);
  }
  Serial.println("LDO ON e Estavel!");

  uint8_t _addr1 = (_address >> 8) & 0xFF;
  uint8_t _addr2 = _address & 0xFF;

  digitalWrite(MAX35103CE, LOW);
  delay(10);

  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE1));

  SPI.transfer(MAX35103_Flash_Read);
  SPI.transfer(_addr1);
  SPI.transfer(_addr2);
  uint8_t _v1 = SPI.transfer(0x00);
  uint8_t _v2 = SPI.transfer(0x00);
  uint16_t _value = 0;
  _value |=  ((uint16_t) _v1) << 8;
  _value |=  ((uint16_t) _v2);

  SPI.endTransaction();

  delay(10);
  digitalWrite(MAX35103CE, HIGH);
  return _value;
}

void writeFlash16(uint16_t _address, uint16_t _value) {
  opcodeCommand(MAX35103_LDO_Timed);
  delay(100);

  while (interruptStatus(10) == 0) {
    delay(100);
  }
  Serial.println("LDO ON e Estavel!");

  uint8_t _addr1 = (_address >> 8) & 0xFF;
  uint8_t _addr2 = _address & 0xFF;
  uint8_t _v1 = (_value >> 8) & 0xFF;
  uint8_t _v2 = _value & 0xFF;

  digitalWrite(MAX35103CE, LOW);
  delay(10);

  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE1));

  SPI.transfer(MAX35103_Flash_Write);
  SPI.transfer(_addr1);
  SPI.transfer(_addr2);
  SPI.transfer(_v1);
  SPI.transfer(_v2);

  SPI.endTransaction();

  delay(10);
  digitalWrite(MAX35103CE, HIGH);
}

void eraseFlash(uint16_t _address) {
  opcodeCommand(MAX35103_LDO_Timed);
  delay(100);

  while (interruptStatus(10) == 0) {
    delay(100);
  }
  Serial.println("LDO ON e Estavel!");

  uint8_t _addr1 = (_address >> 8) & 0xFF;
  uint8_t _addr2 = _address & 0xFF;

  digitalWrite(MAX35103CE, LOW);
  delay(10);

  SPI.beginTransaction(SPISettings(SPI_CLOCK, MSBFIRST, SPI_MODE1));

  SPI.transfer(MAX35103_Flash_Erase);
  SPI.transfer(_addr1);
  SPI.transfer(_addr2);

  SPI.endTransaction();

  delay(10);
  digitalWrite(MAX35103CE, HIGH);
}

/*float calculaToF(uint32_t _timeA, uint32_t _timeB, uint32_t _clock, uint32_t _calibration1, uint32_t _calibration2) {
  float _clockPeriod = 125.0; // em ns

  float _calCountAux1 = (float)_calibration2 - (float)_calibration1;
  float _calCountAux2 = (float)CALIBRATION2_PERIODS - 1.0;
  float _calCount =  _calCountAux1 / _calCountAux2;
  float _normLSB = _clockPeriod / _calCount;
  float _deltaTime = (float)_timeA - (float)_timeB;
  float _tofAux1 = _normLSB * _deltaTime;
  float _tofAux2 = (float)_clock * _clockPeriod;
  float _tof = _tofAux1 + _tofAux2;

  Serial.print("_calCount: ");
  Serial.println(_calCount, 6);
  Serial.print("_normLSB: ");
  Serial.println(_normLSB, 6);
  
  return _tof;
}

float readToF(uint8_t _tofNumber) {
  if (_tofNumber >= 1 && _tofNumber <= 5) {
    uint32_t _timeA = 0;
    uint32_t _timeB = 0;
    uint32_t _clock = 0;

    switch (_tofNumber) {
      case 1:
        _timeA = readRegister24(TDC7200CSB, TDC7200_TIME1);
        _timeB = readRegister24(TDC7200CSB, TDC7200_TIME2);
        _clock = readRegister24(TDC7200CSB, TDC7200_CLOCK_COUNT1);
        break;
      case 2:
        _timeA = readRegister24(TDC7200CSB, TDC7200_TIME2);
        _timeB = readRegister24(TDC7200CSB, TDC7200_TIME3);
        _clock = readRegister24(TDC7200CSB, TDC7200_CLOCK_COUNT2);
        break;
      case 3:
        _timeA = readRegister24(TDC7200CSB, TDC7200_TIME3);
        _timeB = readRegister24(TDC7200CSB, TDC7200_TIME4);
        _clock = readRegister24(TDC7200CSB, TDC7200_CLOCK_COUNT3);
        break;
      case 4:
        _timeA = readRegister24(TDC7200CSB, TDC7200_TIME4);
        _timeB = readRegister24(TDC7200CSB, TDC7200_TIME5);
        _clock = readRegister24(TDC7200CSB, TDC7200_CLOCK_COUNT4);
        break;
      case 5:
        _timeA = readRegister24(TDC7200CSB, TDC7200_TIME5);
        _timeB = readRegister24(TDC7200CSB, TDC7200_TIME6);
        _clock = readRegister24(TDC7200CSB, TDC7200_CLOCK_COUNT5);
        break;
    
    default:
      return 0.0;
      break;
    }

    uint32_t _calibration1 = readRegister24(TDC7200CSB, TDC7200_CALIBRATION1);
    uint32_t _calibration2 = readRegister24(TDC7200CSB, TDC7200_CALIBRATION2);

    Serial.print("_timeA: ");
    Serial.println(_timeA);
    Serial.print("_timeB: ");
    Serial.println(_timeB);

    Serial.print("_clock: ");
    Serial.println(_clock);

    Serial.print("_calibration1: ");
    Serial.println(_calibration1);
    Serial.print("_calibration2: ");
    Serial.println(_calibration2);

    Serial.println("");

    if (_clock) {
      float _tof = calculaToF(_timeA, _timeB, _clock, _calibration1, _calibration2);
      _tof = _tof / TDC7200_AVG_CYCLES;
      
      Serial.print("_tof");
      Serial.print(_tofNumber);
      Serial.print(": ");
      Serial.println(_tof, 6);

      return _tof;
    }
    
    return 0.0;

  }
  return 0.0;
}

float fluxoAgua(float _tUp, float _tDown) {

  float _deltaToF = _tUp - _tDown;
  float _velocidadeAuxA = _deltaToF / 1000.0;  // _deltaToF em us
  float _velocidadeAuxB = _velocidadeAuxA * 2.196324; // em mm^2/us

  float _velocidadeAuxC = _velocidadeAuxB / 124.0;  // em mm/us
  float _velocidade = _velocidadeAuxC * 1000.0;  // em m/s

  Serial.print("_deltaToF: ");
  Serial.println(_deltaToF, 6);

  Serial.print("_velocidadeAuxA: ");
  Serial.println(_velocidadeAuxA, 6);

  Serial.print("_velocidadeAuxB: ");
  Serial.println(_velocidadeAuxB, 6);

  Serial.print("_velocidadeAuxC: ");
  Serial.println(_velocidadeAuxC, 6);

  Serial.print("_velocidade: ");
  Serial.println(_velocidade, 6);

  float _fluxo = _velocidade * 6.785840; // em l/m

  return _fluxo;
}*/

float TOF_DIF(uint16_t _TOF_DIFFInt, uint16_t _TOF_DIFFFrac) {
  // Verificar MSB do _TOF_DIFFInt e sendo 1:
  // A - Inverter todos os demais bits do _TOF_DIFFInt
  // B - Inverter todos os bits de _TOF_DIFFFrac
  // C - Somar 1 ao _TOF_DIFFFrac. Ou seja, quando _TOF_DIFFFrac for 0xFFFF, ele representa -1

  uint16_t _TOF_DIFFIntFinal = _TOF_DIFFInt;
  uint16_t _TOF_DIFFFracFinal = _TOF_DIFFFrac;
  boolean _negativo = false;

  if (_TOF_DIFFInt & (1 << 15)) {
    _TOF_DIFFIntFinal = _TOF_DIFFInt ^ 65535;
    _TOF_DIFFFracFinal = _TOF_DIFFFrac ^ 65535;
    
    if (_TOF_DIFFFracFinal < 65535) {
      _TOF_DIFFFracFinal++;
    }
    
    _negativo = true;
  }

  uint32_t _tempoIntNanoSec = (uint32_t)_TOF_DIFFIntFinal * 250;
  float _tempoFracNanoSec = (float)_TOF_DIFFFracFinal * 3.814755;
  _tempoFracNanoSec = _tempoFracNanoSec / 1000.0;
  float _tempoNanoSec = (float)_tempoIntNanoSec + _tempoFracNanoSec;

  if (_negativo) {
    _tempoNanoSec = _tempoNanoSec * -1.0;
  }

  return _tempoNanoSec;
}

float fluxoAgua(float _deltaToF) {
  float _velocidadeAux = _deltaToF * 2.202256; // 2.202256 em mm²/us²
  float _velocidade = _velocidadeAux / 124.0; // em mm/10⁶ns = mm/ms

  /*Serial.print("_deltaToF: ");
  Serial.println(_deltaToF, 6);

  Serial.print("_velocidadeAux: ");
  Serial.println(_velocidadeAux, 6);

  Serial.print("_velocidade: ");
  Serial.println(_velocidade, 6);*/

  float _fluxo = _velocidade * 6.785840; // em l/m

  return _fluxo;
}

void setup() {
  Serial.begin(115200);

  pinMode(MAX35103INT, INPUT);
  digitalWrite(MAX35103INT, HIGH);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  pinMode(btnPin, INPUT);
  digitalWrite(btnPin, HIGH);

  pinMode(MAX35103CE, OUTPUT);
  digitalWrite(MAX35103CE, HIGH);

  SPI.begin();

  Serial.println("\n\nIniciando...");
  delay(100);

  opcodeCommand(MAX35103_Reset);
  delay(100);

  while (interruptStatus(2) == 0) {
    delay(100);
  }
  Serial.println("Reset OK!");

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
  
  writeRegister16(MAX35103_TOF1_W, 0x1311);
  delay(100);
  writeRegister16(MAX35103_TOF2_W, 0xA4F2);
  delay(100);
  writeRegister16(MAX35103_TOF3_W, 0x0A0B);
  delay(100);
  writeRegister16(MAX35103_TOF4_W, 0x0C0D);
  delay(100);
  writeRegister16(MAX35103_TOF5_W, 0x0E0F);
  delay(100);
  writeRegister16(MAX35103_TOF6_W, 0x0048);
  delay(100);
  writeRegister16(MAX35103_TOF7_W, 0x0048);
  delay(100);

  writeRegister16(MAX35103_EVT_TMN_W, 0x0008);
  delay(100);
  
  writeRegister16(MAX35103_TOF_MES_W, 0x00E9);
  delay(100);
  writeRegister16(MAX35103_CLB_CTR_W, 0x0240);
  delay(100);

  uint16_t _reg = readRegister16(0xB8);
  Serial.print("TOF1 0xB8: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = readRegister16(0xB9);
  Serial.print("TOF2 0xB9: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = readRegister16(0xBA);
  Serial.print("TOF3 0xBA: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = readRegister16(0xBB);
  Serial.print("TOF4 0xBB: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = readRegister16(0xBC);
  Serial.print("TOF5 0xBC: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = readRegister16(0xBD);
  Serial.print("TOF5 0xBD: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = readRegister16(0xBE);
  Serial.print("TOF5 0xBE: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = readRegister16(0xC0);
  Serial.print("_reg 0xC0: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = readRegister16(0xC1);
  Serial.print("_reg 0xC1: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  _reg = readRegister16(0xC2);
  Serial.print("_reg 0xC2: ");
  Serial.print(_reg, BIN);
  Serial.print(" | 0x");
  Serial.println(_reg, HEX);

  delay(500);

  Serial.println("MAX35103_ToFlash");

  opcodeCommand(MAX35103_ToFlash);
  delay(100);

  while (interruptStatus(7) == 0) {
    delay(100);
  }
  Serial.println("Flash Pronta!");
  
  delay(500);

  Serial.println("Iniciando...");

  opcodeCommand(MAX35103_Initialize);
  delay(100);

  while (interruptStatus(3) == 0) {
    delay(100);
  }
  Serial.println("Initialize OK!");
}

void loop() {
  if (millis() > leituraAnterior + 1000) {
    Serial.print("ToF Diff: ");
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
    Serial.println(_fluxoAgua);

    /*Serial.print("Temp: ");
    opcodeCommand(MAX35103_Temperature);

    while (digitalRead(MAX35103INT)) {
      delay(50);
    }

    // interruptStatus(15) indica timeout!!!!!!!!
    if (interruptStatus(11)) {
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
  }
  

  delay(10);
}