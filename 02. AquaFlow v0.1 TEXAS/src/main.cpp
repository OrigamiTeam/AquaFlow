#include <Arduino.h>
#include <SPI.h>

#define CALIBRATION2_PERIODS 10
#define TDC7200_AVG_CYCLES 2.0 // media de 4 medidas

#define TDC1000_CONFIG_0 0x00
#define TDC1000_CONFIG_1 0x01
#define TDC1000_CONFIG_2 0x02
#define TDC1000_CONFIG_3 0x03
#define TDC1000_TOF_1 0x05
#define TDC1000_TOF_0 0x06

#define TDC7200_CONFIG1 0x00
#define TDC7200_CONFIG2 0x01

#define TDC7200_TIME1 0x10
#define TDC7200_TIME2 0x12
#define TDC7200_TIME3 0x14
#define TDC7200_TIME4 0x16
#define TDC7200_TIME5 0x18
#define TDC7200_TIME6 0x1A

#define TDC7200_CLOCK_COUNT1 0x11
#define TDC7200_CLOCK_COUNT2 0x13
#define TDC7200_CLOCK_COUNT3 0x15
#define TDC7200_CLOCK_COUNT4 0x17
#define TDC7200_CLOCK_COUNT5 0x19

#define TDC7200_CALIBRATION1 0x1B
#define TDC7200_CALIBRATION2 0x1C

#define TDC7200INT 2

#define ledPin 6
#define btnPin 7

#define TDC1000CSB 8
#define TDC7200CSB 9
#define TDC1000EN 14
#define TDC1000CHSEL 15
#define TDC7200EN 18

unsigned long ultimaMedida = 0;

float _tofBA[3] = {0.0, 0.0, 0.0};
float _tofAB[3] = {0.0, 0.0, 0.0};

uint32_t readRegister24(uint8_t _csb, uint8_t _registerAddress) {
  digitalWrite(_csb, LOW);
  delay(10);
  
  uint8_t _regAddrAux = 0x00;
  _regAddrAux += _registerAddress & 0x3F;
  SPI.transfer(_regAddrAux);

  uint8_t _time[3] = {0, 0, 0};

  _time[0] = SPI.transfer(0x00);
  _time[1] = SPI.transfer(0x00);
  _time[2] = SPI.transfer(0x00);
  
  uint32_t _value = 0;
  _value |=  ((uint32_t) _time[0]) << 16;
  _value |=  ((uint32_t) _time[1]) << 8;
  _value |=  ((uint32_t) _time[2]);

  delay(10);
  digitalWrite(_csb, HIGH);

  return _value;
}

float calculaToF(uint32_t _timeA, uint32_t _timeB, uint32_t _clock, uint32_t _calibration1, uint32_t _calibration2) {
  float _clockPeriod = 125.0; // em ns

  float _calCountAux1 = (float)_calibration2 - (float)_calibration1;
  float _calCountAux2 = (float)CALIBRATION2_PERIODS - 1.0;
  float _calCount =  _calCountAux1 / _calCountAux2;
  float _normLSB = _clockPeriod / _calCount;
  float _deltaTime = (float)_timeA - (float)_timeB;
  float _tofAux1 = _normLSB * _deltaTime;
  float _tofAux2 = (float)_clock * _clockPeriod;
  float _tof = _tofAux1 + _tofAux2;

  /*Serial.print("_calCount: ");
  Serial.println(_calCount, 6);
  Serial.print("_normLSB: ");
  Serial.println(_normLSB, 6);*/
  
  return _tof;
}

/*void readToF() {
  uint32_t _time1 = readRegister24(TDC7200CSB, TDC7200_TIME1);
  uint32_t _time2 = readRegister24(TDC7200CSB, TDC7200_TIME2);
  uint32_t _time3 = readRegister24(TDC7200CSB, TDC7200_TIME3);
  uint32_t _time4 = readRegister24(TDC7200CSB, TDC7200_TIME4);
  uint32_t _time5 = readRegister24(TDC7200CSB, TDC7200_TIME5);
  uint32_t _time6 = readRegister24(TDC7200CSB, TDC7200_TIME6);

  uint32_t _clock1 = readRegister24(TDC7200CSB, TDC7200_CLOCK_COUNT1);
  uint32_t _clock2 = readRegister24(TDC7200CSB, TDC7200_CLOCK_COUNT2);
  uint32_t _clock3 = readRegister24(TDC7200CSB, TDC7200_CLOCK_COUNT3);
  uint32_t _clock4 = readRegister24(TDC7200CSB, TDC7200_CLOCK_COUNT4);
  uint32_t _clock5 = readRegister24(TDC7200CSB, TDC7200_CLOCK_COUNT5);

  uint32_t _calibration1 = readRegister24(TDC7200CSB, TDC7200_CALIBRATION1);
  uint32_t _calibration2 = readRegister24(TDC7200CSB, TDC7200_CALIBRATION2);

  Serial.print("_time1: ");
  Serial.println(_time1);
  Serial.print("_time2: ");
  Serial.println(_time2);
  Serial.print("_time3: ");
  Serial.println(_time3);
  Serial.print("_time4: ");
  Serial.println(_time4);
  Serial.print("_time5: ");
  Serial.println(_time5);
  Serial.print("_time6: ");
  Serial.println(_time6);

  Serial.print("_clock1: ");
  Serial.println(_clock1);
  Serial.print("_clock2: ");
  Serial.println(_clock2);
  Serial.print("_clock3: ");
  Serial.println(_clock3);
  Serial.print("_clock4: ");
  Serial.println(_clock4);
  Serial.print("_clock5: ");
  Serial.println(_clock5);

  Serial.print("_calibration1: ");
  Serial.println(_calibration1);
  Serial.print("_calibration2: ");
  Serial.println(_calibration2);

  Serial.println("");

  if (_clock1) {
    float _tof1 = calculaToF(_time1, _time2, _clock1, _calibration1, _calibration2);
    _tof1 = _tof1 / TDC7200_AVG_CYCLES;
    Serial.print("_tof1: ");
    Serial.println(_tof1, 6);
  }

  if (_clock2) {
    float _tof2 = calculaToF(_time2, _time3, _clock2, _calibration1, _calibration2);
    _tof2 = _tof2 / TDC7200_AVG_CYCLES;
    Serial.print("_tof2: ");
    Serial.println(_tof2, 6);
  }

  if (_clock3) {
    float _tof3 = calculaToF(_time3, _time4, _clock3, _calibration1, _calibration2);
    _tof3 = _tof3 / TDC7200_AVG_CYCLES;
    Serial.print("_tof3: ");
    Serial.println(_tof3, 6);
  }

  if (_clock4) {
    float _tof4 = calculaToF(_time4, _time5, _clock4, _calibration1, _calibration2);
    _tof4 = _tof4 / TDC7200_AVG_CYCLES;
    Serial.print("_tof4: ");
    Serial.println(_tof4, 6);
  }

  if (_clock5) {
    float _tof5 = calculaToF(_time5, _time6, _clock5, _calibration1, _calibration2);
    _tof5 = _tof5 / TDC7200_AVG_CYCLES;
    Serial.print("_tof5: ");
    Serial.println(_tof5, 6);
  }
}*/

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

    /*Serial.print("_timeA: ");
    Serial.println(_timeA);
    Serial.print("_timeB: ");
    Serial.println(_timeB);

    Serial.print("_clock: ");
    Serial.println(_clock);

    Serial.print("_calibration1: ");
    Serial.println(_calibration1);
    Serial.print("_calibration2: ");
    Serial.println(_calibration2);

    Serial.println("");*/

    if (_clock) {
      float _tof = calculaToF(_timeA, _timeB, _clock, _calibration1, _calibration2);
      _tof = _tof / TDC7200_AVG_CYCLES;
      
      /*Serial.print("_tof");
      Serial.print(_tofNumber);
      Serial.print(": ");
      Serial.println(_tof, 6);*/

      return _tof;
    }
    
    return 0.0;

  }
  return 0.0;
}

/*float fluxoAgua(float _tUp, float _tDown) {

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

float ajustaFluxo(float _fluxo) {
  float _kAuxA = 94.9 * _fluxo;
  float _kAuxB = _kAuxA / 1000.0;
  float _k = _kAuxB - 0.356;

  float _fluxoAjustado = _fluxo * _k;
  return _fluxoAjustado;
}

void writeRegister(uint8_t _csb,  uint8_t _registerAddress, uint8_t _value) {
  digitalWrite(_csb, LOW);
  delay(10);
  uint8_t _regAddrAux = 0x40;
  _regAddrAux += _registerAddress & 0x3F;
  SPI.transfer(_regAddrAux);
  SPI.transfer(_value);
  delay(10);
  digitalWrite(_csb, HIGH);
}

void startMeasurement() {
  writeRegister(TDC7200CSB, TDC7200_CONFIG1, 0x03);
}

float mediano3(float _vetor[]) {
  for(uint8_t _i = 0; _i < 2; _i++) {
    for(uint8_t _j = 0; _j < 2-_i; _j++) {
      if(_vetor[_j] > _vetor[_j+1]) {
        float _temp = _vetor[_j];
        _vetor[_j] = _vetor[_j+1];
        _vetor[_j+1] = _temp;
      }
    }
  }

  return _vetor[1];
}

void setup() {
  Serial.begin(115200);

  pinMode(TDC7200INT, INPUT);
  digitalWrite(TDC7200INT, HIGH);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  pinMode(btnPin, INPUT);
  digitalWrite(btnPin, HIGH);

  pinMode(TDC1000CSB, OUTPUT);
  digitalWrite(TDC1000CSB, HIGH);

  pinMode(TDC7200CSB, OUTPUT);
  digitalWrite(TDC7200CSB, HIGH);

  pinMode(TDC1000EN, OUTPUT);
  digitalWrite(TDC1000EN, LOW);

  pinMode(TDC1000CHSEL, OUTPUT);
  digitalWrite(TDC1000CHSEL, LOW);

  pinMode(TDC7200EN, OUTPUT);
  digitalWrite(TDC7200EN, LOW);

  SPI.begin();

  Serial.println("Config...");
  
  delay(1000);
  
  digitalWrite(TDC1000EN, HIGH);
  Serial.println("TDC1000EN = HIGH");
  delay(500);

  writeRegister(TDC1000CSB, TDC1000_CONFIG_0, 0x45);
  delay(100);
  writeRegister(TDC1000CSB, TDC1000_CONFIG_1, 0x48); // media de 2 medidas com pulsos de stop ilimitados
  //writeRegister(TDC1000CSB, TDC1000_CONFIG_1, 0x50); // media de 4 medidas com pulsos de stop ilimitados
  //writeRegister(TDC1000CSB, TDC1000_CONFIG_1, 0x54); // media de 4 medidas com 4 pulsos de stop
  //writeRegister(TDC1000CSB, TDC1000_CONFIG_1, 0x40); // medida unica
  delay(100);
  //writeRegister(TDC1000CSB, TDC1000_CONFIG_2, 0x12); // TOF mode 2 sem Damping
  //writeRegister(TDC1000CSB, TDC1000_CONFIG_2, 0x32); // TOF mode 2 com Damping
  writeRegister(TDC1000CSB, TDC1000_CONFIG_2, 0x2A); // TOF mode 2 com Damping e CHSEL externo
  delay(100);
  writeRegister(TDC1000CSB, TDC1000_CONFIG_3, 0x05);
  delay(100);
  writeRegister(TDC1000CSB, TDC1000_TOF_1, 0x00);
  delay(100);
  writeRegister(TDC1000CSB, TDC1000_TOF_0, 0x3F);
  delay(100);

  digitalWrite(TDC1000EN, LOW);
  Serial.println("TDC1000EN = LOW");

  digitalWrite(TDC7200EN, HIGH);
  Serial.println("TDC7200EN = HIGH");
  delay(500);

  //writeRegister(TDC7200CSB, TDC7200_CONFIG1, 0x02);
  //delay(100);
  
  writeRegister(TDC7200CSB, TDC7200_CONFIG2, 0x48); // media de 2 medidas
  //writeRegister(TDC7200CSB, TDC7200_CONFIG2, 0x50); // media de 4 medidas
  //writeRegister(TDC7200CSB, TDC7200_CONFIG2, 0x40); // medida unica
  delay(100);

  digitalWrite(ledPin, HIGH);
  delay(50);
  digitalWrite(ledPin, LOW);
  Serial.println("Loop!");
}

void loop() {
  if (millis() >= ultimaMedida + 1000) {
    ultimaMedida = millis();

    _tofBA[0] = 0.0;
    _tofBA[1] = 0.0;
    _tofBA[2] = 0.0;
    _tofAB[0] = 0.0;
    _tofAB[1] = 0.0;
    _tofAB[2] = 0.0;

    digitalWrite(TDC1000EN, HIGH);
    delay(10);
    
    for (uint8_t _i = 0; _i < 3; _i++) {
      digitalWrite(TDC1000CHSEL, LOW);
      delay(10);

      startMeasurement();

      while (digitalRead(TDC7200INT)) {
        delayMicroseconds(10);
      }

      _tofBA[_i] = readToF(1);

      digitalWrite(ledPin, HIGH);
      delay(10);
      digitalWrite(ledPin, LOW);

      digitalWrite(TDC1000CHSEL, HIGH);
      delay(10);

      startMeasurement();

      while (digitalRead(TDC7200INT)) {
        delayMicroseconds(10);
      }

      _tofAB[_i] = readToF(1);
    }

    /*digitalWrite(TDC1000EN, LOW);
    Serial.println("TDC1000EN = LOW");
    delay(50);*/

    digitalWrite(TDC1000CHSEL, LOW);
    delay(10);

    /*Serial.print("tofBA: ");
    Serial.print(_tofBA[0], 2);
    Serial.print(" , ");
    Serial.print(_tofBA[1], 2);
    Serial.print(" , ");
    Serial.println(_tofBA[2], 2);*/

    /*Serial.print("tofAB: ");
    Serial.print(_tofAB[0], 2);
    Serial.print(" , ");
    Serial.print(_tofAB[1], 2);
    Serial.print(" , ");
    Serial.println(_tofAB[2], 2);*/

    /*Serial.println("\n--------------------\n");
    Serial.println("BA: ");

    Serial.print("tof1: ");
    Serial.println(_tof1BA);*/
    /*Serial.print("tof2: ");
    Serial.println(_tof2BA);
    Serial.print("tof3: ");
    Serial.println(_tof3BA);*/

    /*Serial.println("AB: ");

    Serial.print("tof1: ");
    Serial.println(_tof1AB);*/
    /*Serial.print("tof2: ");
    Serial.println(_tof2AB);
    Serial.print("tof3: ");
    Serial.println(_tof3AB);*/

    //Serial.println("\nDelta ToF: ");

    /*float _deltaTof1 = _tof1BA - _tof1AB;
    float _deltaTof2 = _tof2BA - _tof2AB;
    float _deltaTof3 = _tof3BA - _tof3AB;*/

    /*Serial.print("1: ");
    Serial.println(_deltaTof1, 6);*/
    /*Serial.print("2: ");
    Serial.println(_deltaTof2, 6);
    Serial.print("3: ");
    Serial.println(_deltaTof3, 6);*/

    //Serial.println("");

    float _deltaTof[3] = {0.0, 0.0, 0.0};

    for (uint8_t _i = 0; _i < 3; _i++) {
      _deltaTof[_i] = _tofBA[_i] - _tofAB[_i];
    }

    float _deltTofMed3 = mediano3(_deltaTof);
    /*Serial.print("_deltTofMed3: ");
    Serial.println(_deltTofMed3, 2);*/

    float _fluxo1 = fluxoAgua(_deltTofMed3);
    float _fluxoAjustado = ajustaFluxo(_fluxo1);
    
    Serial.print("f: ");
    Serial.println(_fluxoAjustado, 2);
  }

  delay(10);
}