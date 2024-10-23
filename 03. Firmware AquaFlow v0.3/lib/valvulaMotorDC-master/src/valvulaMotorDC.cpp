#include "valvulaMotorDC.h"

valvulaMotorDC::valvulaMotorDC() {}

void valvulaMotorDC::begin(uint8_t _SW1, uint8_t _SW2, uint8_t _sleep, uint8_t _IN1, uint8_t _IN2) {
  _SW1Pin = _SW1;
  _SW2Pin = _SW2;
  _motorSleepPin = _sleep;
  _motorIN1Pin = _IN1;
  _motorIN2Pin = _IN2;

  pinMode(_SW1Pin, INPUT);
  pinMode(_SW2Pin, INPUT);

  pinMode(_motorSleepPin, OUTPUT);
  digitalWrite(_motorSleepPin, LOW); // LOW = sleep

  pinMode(_motorIN1Pin, OUTPUT);
  digitalWrite(_motorIN1Pin, LOW);

  pinMode(_motorIN2Pin, OUTPUT);
  digitalWrite(_motorIN2Pin, LOW);
}

boolean valvulaMotorDC::abre() {
  if(!digitalRead(_SW1Pin)) {
    delay(20);
    if (!digitalRead(_SW1Pin)) {
      return true;
    }
  }
  digitalWrite(_motorSleepPin, HIGH);
  delay(10);
  digitalWrite(_motorIN1Pin, LOW);
  digitalWrite(_motorIN2Pin, HIGH);

  uint32_t _cont = 0;
  boolean _aguardar = true;
  while(_aguardar) {
    if(_cont > 1500) {
      digitalWrite(_motorIN2Pin, LOW);
      digitalWrite(_motorSleepPin, LOW);
      return false;
    }

    if(!digitalRead(_SW1Pin)) {
      delay(20);
      if (!digitalRead(_SW1Pin)) {
        digitalWrite(_motorIN1Pin, HIGH);
        delay(500);

        digitalWrite(_motorIN1Pin, LOW);
        digitalWrite(_motorIN2Pin, LOW);
        digitalWrite(_motorSleepPin, LOW);
        _aguardar = false;
      }
    }

    delay(10);
    _cont++;
  }
  return true;
}

boolean valvulaMotorDC::fecha() {
  if(!digitalRead(_SW2Pin)) {
    delay(20);
    if (!digitalRead(_SW2Pin)) {
      return true;
    }
  }

  digitalWrite(_motorSleepPin, HIGH);
  delay(10);
  digitalWrite(_motorIN1Pin, HIGH);
  digitalWrite(_motorIN2Pin, LOW);

  uint32_t _cont = 0;
  boolean _aguardar = true;
  while(_aguardar) {
    if(_cont > 1500) {
      digitalWrite(_motorIN1Pin, LOW);
      digitalWrite(_motorSleepPin, LOW);
      return false;
    }

    if(!digitalRead(_SW2Pin)) {
      delay(20);
      if (!digitalRead(_SW2Pin)) {
        digitalWrite(_motorIN2Pin, HIGH);
        delay(500);

        digitalWrite(_motorIN1Pin, LOW);
        digitalWrite(_motorIN2Pin, LOW);
        digitalWrite(_motorSleepPin, LOW);
        _aguardar = false;
      }
    }

    delay(10);
    _cont++;
  }
  return true;
}

boolean valvulaMotorDC::abreParcial(unsigned long _tempoMilis) {
  if (!fecha()) {
    return false;
  }
  delay(1000);

  digitalWrite(_motorSleepPin, HIGH);
  delay(10);
  digitalWrite(_motorIN1Pin, LOW);
  digitalWrite(_motorIN2Pin, HIGH);

  unsigned long _tempoInicial = millis();
  while (millis() < _tempoInicial + _tempoMilis) {

    if(!digitalRead(_SW1Pin)) {
      delay(20);
      if (!digitalRead(_SW1Pin)) {
        digitalWrite(_motorIN1Pin, HIGH);
        delay(500);

        digitalWrite(_motorIN1Pin, LOW);
        digitalWrite(_motorIN2Pin, LOW);
        digitalWrite(_motorSleepPin, LOW);
        return false;
      }
    }

    delay(10);
  }

  digitalWrite(_motorIN1Pin, HIGH);
  delay(500);
  digitalWrite(_motorIN1Pin, LOW);
  digitalWrite(_motorIN2Pin, LOW);
  digitalWrite(_motorSleepPin, LOW);
  return true;
}