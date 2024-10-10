#include "MAX35103.h"

MAX35103::MAX35103() {}

void MAX35103::begin(uint8_t intPin, uint8_t rstPin, uint8_t cePin) {
  _intPin = intPin;
  _rstPin = rstPin;
  _cePin = cePin;

  pinMode(_intPin, INPUT);
  digitalWrite(_intPin, HIGH);
  pinMode(_rstPin, OUTPUT);
  digitalWrite(_rstPin, HIGH);
  pinMode(_cePin, OUTPUT);
  digitalWrite(_cePin, HIGH);
}

void MAX35103::writeRegister16(uint8_t _address, uint16_t _value) {
  uint8_t _v1 = (_value >> 8) & 0xFF;
  uint8_t _v2 = _value & 0xFF;
  digitalWrite(_cePin, LOW);
  delay(10);

  SPI.beginTransaction(SPISettings(MAX35103_SPI_CLOCK, MSBFIRST, SPI_MODE1));

  SPI.transfer(_address);
  SPI.transfer(_v1);
  SPI.transfer(_v2);

  SPI.endTransaction();

  delay(10);
  digitalWrite(_cePin, HIGH);
}

void MAX35103::config() {
  writeRegister16(MAX35103_RTC_SEC_W, 0x000);
  delay(100);
  writeRegister16(MAX35103_RTC_HRMIN_W, 0x0000);
  delay(100);
  writeRegister16(MAX35103_RTC_D_W, 0x0000);
  delay(100);
  writeRegister16(MAX35103_RTC_MY_W, 0x0000);
  delay(100);
  writeRegister16(MAX35103_WDT_W, 0x0000);
  delay(100);
  writeRegister16(MAX35103_ALARM_W, 0x0000);
  delay(100);

  writeRegister16(0x36, 0x0000);
  delay(100);
  writeRegister16(0x37, 0x0000);
  delay(100);

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

  writeRegister16(MAX35103_EVT_TMN1_W, 0x0000);
  delay(100);
  writeRegister16(MAX35103_EVT_TMN2_W, 0x006B);
  delay(100);
  
  writeRegister16(MAX35103_TOF_MES_W, 0x00E9);
  delay(100);
  //writeRegister16(MAX35103_CLB_CTR_W, 0x0240); // Event Timing de ciclo unico
  writeRegister16(MAX35103_CLB_CTR_W, 0x0340); // Event Timing em ciclo continuo
  delay(100);

  writeRegister16(0x43, 0x0080);
  delay(100);
}

uint16_t MAX35103::readRegister16(uint8_t _address) {
  digitalWrite(_cePin, LOW);
  delay(10);

  SPI.beginTransaction(SPISettings(MAX35103_SPI_CLOCK, MSBFIRST, SPI_MODE1));

  SPI.transfer(_address);
  uint8_t _v1 = SPI.transfer(0x00);
  uint8_t _v2 = SPI.transfer(0x00);
  uint16_t _value = 0;
  _value |=  ((uint16_t) _v1) << 8;
  _value |=  ((uint16_t) _v2);

  SPI.endTransaction();

  delay(10);
  digitalWrite(_cePin, HIGH);
  return _value;
}

boolean MAX35103::interruptStatus(uint8_t _bit) {
  uint16_t _status = readRegister16(MAX35103_INT_STATUS_R);

  /*Serial.print("_status: ");
  Serial.print(_status, BIN);
  Serial.print(" | ");
  Serial.println(_status);*/

  return (_status & (1 << _bit));
}

void MAX35103::opcodeCommand(uint8_t _command) {
  digitalWrite(_cePin, LOW);
  delay(10);

  SPI.beginTransaction(SPISettings(MAX35103_SPI_CLOCK, MSBFIRST, SPI_MODE1));

  SPI.transfer(_command);

  SPI.endTransaction();

  delay(10);
  digitalWrite(_cePin, HIGH);
}

boolean MAX35103::reset() {
  opcodeCommand(MAX35103_Reset);
  delay(100);

  unsigned long _millisInicio = millis();
  while (interruptStatus(2) == 0) {
    delay(100);

    if (millis() > _millisInicio + _timeout) {
      return false;
    }
  }
  return true;
}

boolean MAX35103::toFlash() {
  opcodeCommand(MAX35103_ToFlash);
  delay(100);

  unsigned long _millisInicio = millis();
  while (interruptStatus(7) == 0) {
    delay(100);
    
    if (millis() > _millisInicio + _timeout) {
      return false;
    }
  }
  return true;
}

boolean MAX35103::initialize() {
  opcodeCommand(MAX35103_Initialize);
  delay(100);

  unsigned long _millisInicio = millis();
  while (interruptStatus(3) == 0) {
    delay(100);

    if (millis() > _millisInicio + _timeout) {
      return false;
    }
  }
  return true;
}

/*uint16_t MAX35103::readFlash16(uint16_t _address) {
  opcodeCommand(MAX35103_LDO_Timed);
  delay(100);

  while (interruptStatus(10) == 0) {
    delay(100);
  }
  //Serial.println("LDO ON e Estavel!");

  uint8_t _addr1 = (_address >> 8) & 0xFF;
  uint8_t _addr2 = _address & 0xFF;

  digitalWrite(_cePin, LOW);
  delay(10);

  SPI.beginTransaction(SPISettings(MAX35103_SPI_CLOCK, MSBFIRST, SPI_MODE1));

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
  digitalWrite(_cePin, HIGH);
  return _value;
}

void MAX35103::writeFlash16(uint16_t _address, uint16_t _value) {
  opcodeCommand(MAX35103_LDO_Timed);
  delay(100);

  while (interruptStatus(10) == 0) {
    delay(100);
  }
  //Serial.println("LDO ON e Estavel!");

  uint8_t _addr1 = (_address >> 8) & 0xFF;
  uint8_t _addr2 = _address & 0xFF;
  uint8_t _v1 = (_value >> 8) & 0xFF;
  uint8_t _v2 = _value & 0xFF;

  digitalWrite(_cePin, LOW);
  delay(10);

  SPI.beginTransaction(SPISettings(MAX35103_SPI_CLOCK, MSBFIRST, SPI_MODE1));

  SPI.transfer(MAX35103_Flash_Write);
  SPI.transfer(_addr1);
  SPI.transfer(_addr2);
  SPI.transfer(_v1);
  SPI.transfer(_v2);

  SPI.endTransaction();

  delay(10);
  digitalWrite(_cePin, HIGH);
}

void MAX35103::eraseFlash(uint16_t _address) {
  opcodeCommand(MAX35103_LDO_Timed);
  delay(100);

  while (interruptStatus(10) == 0) {
    delay(100);
  }
  //Serial.println("LDO ON e Estavel!");

  uint8_t _addr1 = (_address >> 8) & 0xFF;
  uint8_t _addr2 = _address & 0xFF;

  digitalWrite(_cePin, LOW);
  delay(10);

  SPI.beginTransaction(SPISettings(MAX35103_SPI_CLOCK, MSBFIRST, SPI_MODE1));

  SPI.transfer(MAX35103_Flash_Erase);
  SPI.transfer(_addr1);
  SPI.transfer(_addr2);

  SPI.endTransaction();

  delay(10);
  digitalWrite(_cePin, HIGH);
}*/

float MAX35103::ToF_Diff(uint16_t _TOF_DIFFInt, uint16_t _TOF_DIFFFrac) {
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

float MAX35103::fluxoAgua(float _deltaToF) {
  float _velocidadeAux = _deltaToF * 2.202256; // 2.202256 em mm²/us²
  //float _velocidade = _velocidadeAux / 124.0; // em mm/10⁶ns = mm/ms
  float _velocidade = _velocidadeAux / 110.0; // em mm/10⁶ns = mm/ms

  /*Serial.print("_deltaToF: ");
  Serial.println(_deltaToF, 6);

  Serial.print("_velocidadeAux: ");
  Serial.println(_velocidadeAux, 6);

  Serial.print("_velocidade: ");
  Serial.println(_velocidade, 6);*/

  float _fluxo = _velocidade * 6.785840; // em l/m

  return _fluxo;
}

boolean MAX35103::fluxoToFDIff(float *_fluxo) {
  opcodeCommand(MAX35103_TOF_Diff);

  unsigned long _millisInicio = millis();
  while (digitalRead(_intPin)) {
    delay(50);

    if (millis() > _millisInicio + _timeout) {
      return false;
    }
  }

  // interruptStatus(15) indica timeout!
  if (!interruptStatus(12)) {
    return false;
  }

  uint16_t _ToF_DiffInt = readRegister16(0xE2);
  uint16_t _ToF_DiffFrac = readRegister16(0xE3);
  float _ToF_Diff = ToF_Diff(_ToF_DiffInt, _ToF_DiffFrac);
  float _fluxoAgua = fluxoAgua(_ToF_Diff);

  *_fluxo = _fluxoAgua;
  return true;
}

boolean MAX35103::fluxoToFDIff(float *_fluxo, float *_ToFDiff) {
  opcodeCommand(MAX35103_TOF_Diff);

  unsigned long _millisInicio = millis();
  while (digitalRead(_intPin)) {
    delay(50);

    if (millis() > _millisInicio + _timeout) {
      return false;
    }
  }

  // interruptStatus(15) indica timeout!
  if (!interruptStatus(12)) {
    return false;
  }

  uint16_t _ToF_DiffInt = readRegister16(0xE2);
  uint16_t _ToF_DiffFrac = readRegister16(0xE3);
  float _ToF_Diff = ToF_Diff(_ToF_DiffInt, _ToF_DiffFrac);
  float _fluxoAgua = fluxoAgua(_ToF_Diff);

  *_ToFDiff = _ToF_Diff;
  *_fluxo = _fluxoAgua;
  return true;
}

float MAX35103::registerTemp(uint16_t _TxInt, uint16_t _TxFrac) {
  uint32_t _registerInt = (uint32_t)_TxInt * 250;
  float _registerFrac = (float)_TxFrac * 3.814755;
  _registerFrac = _registerFrac / 1000.0;
  float _registerTemp = (float)_registerInt + _registerFrac;

  return _registerTemp;
}

float MAX35103::temperaturaPT1000(float _R, float _R0) {
  float _A = 3.9083E-3;
  float _B = -5.775E-7;
  
  float _ratio = _R / _R0;

  //T = (0.0-A + sqrt((A*A) - 4.0 * B * (1.0 - Ratio))) / 2.0 * B; 
  float _T = 0.0 - _A; 
  _T += sqrt((_A * _A) - 4.0 * _B * (1.0 - _ratio)); 
  _T /= (2.0 * _B);

  if(_T > 0 && _T < 200) { 
    return _T; 
  } 
  
  //T=  (0.0-A - sqrt((A*A) - 4.0 * B * (1.0 - Ratio))) / 2.0 * B; 
  _T = 0.0 - _A; 
  _T -= sqrt((_A * _A) - 4.0 * _B * (1.0 - _ratio)); 
  _T /= (2.0 * _B); 
  return _T; 
}

boolean MAX35103::temperatura(uint8_t _sensor, float *_temperatura) {
  if (_sensor == 1) {
    opcodeCommand(MAX35103_Temperature);

    unsigned long _millisInicio = millis();
    while (digitalRead(_intPin)) {
      delay(50);

      if (millis() > _millisInicio + _timeout) {
        return false;
      }
    }

    // interruptStatus(15) indica timeout!
    if (!interruptStatus(11)) {
      return false;
    }

    uint16_t _TxInt = readRegister16(0xE7);
    uint16_t _TxFrac = readRegister16(0xE8);
    float _timeT1 = registerTemp(_TxInt, _TxFrac);

    if (_TxInt == 0xFFFF && _TxFrac == 0xFFFF) {
      return false;
    }

    _TxInt = readRegister16(0xEB);
    _TxFrac = readRegister16(0xEC);
    float _timeT3 = registerTemp(_TxInt, _TxFrac);

    if (_TxInt == 0xFFFF && _TxFrac == 0xFFFF) {
      return false;
    }

    float _temp = temperaturaPT1000(_timeT1, _timeT3);

    *_temperatura = _temp;
    return true;
  }
  /*else if (_sensor == 2) {
    opcodeCommand(MAX35103_Temperature);

    unsigned long _millisInicio = millis();
    while (digitalRead(_intPin)) {
      delay(50);

      if (millis() > _millisInicio + _timeout) {
        return false;
      }
    }

    // interruptStatus(15) indica timeout!
    if (!interruptStatus(11)) {
      return false;
    }

    uint16_t _TxInt = readRegister16(0xE9);
    uint16_t _TxFrac = readRegister16(0xEA);
    float _timeT2 = registerTemp(_TxInt, _TxFrac);

    _TxInt = readRegister16(0xED);
    _TxFrac = readRegister16(0xEE);
    float _timeT4 = registerTemp(_TxInt, _TxFrac);

    float _temperatura2 = temperaturaPT1000(_timeT2, _timeT4);

    Serial.print("_temperatura2: ");
    Serial.println(_temperatura2, 1);
  }*/
  return false;
}