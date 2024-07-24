#include <Arduino.h>
#include <SPI.h>

#define MAX35103_TOF_Up 0x00
#define MAX35103_TOF_Down 0x01
#define MAX35103_TOF_Diff 0x02
#define MAX35103_Temperature 0x03
#define MAX35103_Reset 0x04
#define MAX35103_Initialize 0x05
#define MAX35103_ToFlash 0x06

#define MAX35103_TOF1_W 0x38
#define MAX35103_CLBT_CTRL_W 0x42

#define MAX35103_INT_STATUS_R 0xFE

#define MAX35103INT 3

#define ledPin 6
#define btnPin 7

#define MAX35103CE 10

uint16_t readRegister16(uint8_t _address) {
  digitalWrite(MAX35103CE, LOW);
  delay(100);

  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE1));
  
  SPI.transfer(_address);
  uint8_t _v1 = SPI.transfer(0x00);
  uint8_t _v2 = SPI.transfer(0x00);
  uint16_t _value = 0;
  _value |=  ((uint16_t) _v1) << 8;
  _value |=  ((uint16_t) _v2);

  SPI.endTransaction();

  delay(100);
  digitalWrite(MAX35103CE, HIGH);
  return _value;
}

void writeRegister16(uint8_t _address, uint16_t _value) {
  uint8_t _v1 = (_value >> 8) & 0xFF;
  uint8_t _v2 = _value & 0xFF;
  digitalWrite(MAX35103CE, LOW);
  delay(100);

  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE1));

  SPI.transfer(_address);
  SPI.transfer(_v1);
  SPI.transfer(_v2);

  SPI.endTransaction();

  delay(100);
  digitalWrite(MAX35103CE, HIGH);
}

void opcodeCommand(uint8_t _command) {
  digitalWrite(MAX35103CE, LOW);
  delay(100);

  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE1));

  SPI.transfer(_command);

  SPI.endTransaction();

  delay(100);
  digitalWrite(MAX35103CE, HIGH);
}

boolean interruptStatus(uint8_t _bit) {
  uint16_t _status = readRegister16(MAX35103_INT_STATUS_R);

  Serial.print("_status: ");
  Serial.print(_status, BIN);
  Serial.print(" | ");
  Serial.println(_status);

  return ((_status & (1 << _bit)) != false);
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

  Serial.println("Iniciando...");

  delay(5000);
  
  writeRegister16(MAX35103_TOF1_W, 0x510); // configura PLD para 0b0001 = 1MHz
  delay(100);
  writeRegister16(MAX35103_CLBT_CTRL_W, 0x200); // configura INT_EN
  delay(100);
  opcodeCommand(MAX35103_ToFlash);
  delay(100);
  opcodeCommand(MAX35103_Initialize);
  delay(100);

  uint16_t _reg = readRegister16(0xB8);

  Serial.print("_reg 0xB8: ");
  Serial.print(_reg, BIN);
  Serial.print(" | ");
  Serial.println(_reg);

  _reg = readRegister16(0xC2);

  Serial.print("_reg 0xC2: ");
  Serial.print(_reg, BIN);
  Serial.print(" | ");
  Serial.println(_reg);

  digitalWrite(ledPin, HIGH);
  delay(50);
  digitalWrite(ledPin, LOW);
  Serial.println("Loop!");
}

void loop() {
  Serial.println("ToF");
  opcodeCommand(MAX35103_TOF_Diff);

  /*while (digitalRead(MAX35103INT)) {
    delayMicroseconds(10);
  }*/

  delay(500);

  interruptStatus(0);

  /*Serial.println("startMeasurement()");

  startMeasurement();

  while (digitalRead(MAX35103INT)) {
    delayMicroseconds(10);
  }

  float _tof1BA = readToF(1);
  float _tof2BA = readToF(2);
  float _tof3BA = readToF(3);

  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);

  Serial.println("startMeasurement() 2");

  startMeasurement();
  //delay(50);

  //Serial.println("Aquardando INT 2");

  while (digitalRead(TDC7200INT)) {
    delayMicroseconds(10);
  }

  float _tof1AB = readToF(1);
  float _tof2AB = readToF(2);
  float _tof3AB = readToF(3);

  digitalWrite(TDC1000EN, LOW);
  Serial.println("TDC1000EN = LOW");
  delay(50);

  Serial.println("\n--------------------\n");
  Serial.println("BA: ");

  Serial.print("tof1: ");
  Serial.println(_tof1BA);
  Serial.print("tof2: ");
  Serial.println(_tof2BA);
  Serial.print("tof3: ");
  Serial.println(_tof3BA);

  Serial.println("AB: ");

  Serial.print("tof1: ");
  Serial.println(_tof1AB);
  Serial.print("tof2: ");
  Serial.println(_tof2AB);
  Serial.print("tof3: ");
  Serial.println(_tof3AB);

  Serial.println("\nDelta ToF: ");

  float _deltaTof1 = _tof1BA - _tof1AB;
  float _deltaTof2 = _tof2BA - _tof2AB;
  float _deltaTof3 = _tof3BA - _tof3AB;

  Serial.print("1: ");
  Serial.println(_deltaTof1, 6);
  Serial.print("2: ");
  Serial.println(_deltaTof2, 6);
  Serial.print("3: ");
  Serial.println(_deltaTof3, 6);

  Serial.println("");

  float _f1 = fluxoAgua(_tof1BA, _tof1AB);
  Serial.print("f1: ");
  Serial.println(_f1, 2);

  Serial.println("");

  float _f2 = fluxoAgua(_tof2BA, _tof2AB);
  Serial.print("f2: ");
  Serial.println(_f2, 2);

  Serial.println("");

  float _f3 = fluxoAgua(_tof3BA, _tof3AB);
  Serial.print("f3: ");
  Serial.println(_f3, 2);*/

  delay(1000);
}