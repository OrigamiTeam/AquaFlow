#include "valvulaMotorDC.h"

valvulaMotorDC::valvulaMotorDC() {}

void valvulaMotorDC::begin(uint8_t SW1, uint8_t SW2, uint8_t sleep, uint8_t IN1, uint8_t IN2) {
  _SW1Pin = SW1;
  _SW2Pin = SW2;
  _motorSleepPin = sleep;
  _motorIN1Pin = IN1;
  _motorIN2Pin = IN2;

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

    if(!digitalRead(_SW1Pin)) {
      delay(20);
      if (!digitalRead(_SW1Pin)) {
        digitalWrite(_motorIN1Pin, LOW);
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

    if(!digitalRead(_SW2Pin)) {
      delay(20);
      if (!digitalRead(_SW2Pin)) {
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

boolean valvulaMotorDC::abreParcial(uint32_t _tempoMilis) {
  if (!fecha()) {
    return false;
  }
  delay(250);

  digitalWrite(_motorSleepPin, HIGH);
  delay(10);
  digitalWrite(_motorIN1Pin, HIGH);
  digitalWrite(_motorIN2Pin, LOW);

  unsigned long _tempoInicial = millis();
  while (millis() < _tempoInicial + _tempoMilis) {

    if(!digitalRead(_SW1Pin)) {
      delay(20);
      if (!digitalRead(_SW1Pin)) {
        digitalWrite(_motorIN1Pin, LOW);
        digitalWrite(_motorSleepPin, LOW);
        return false;
      }
    }

    delay(10);
  }
  
  digitalWrite(_motorIN1Pin, LOW);
  digitalWrite(_motorSleepPin, LOW);
  return true;
}