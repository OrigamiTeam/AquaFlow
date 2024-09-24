#ifndef valvulaMotorDC_h
#define valvulaMotorDC_h

#include <Arduino.h>

class valvulaMotorDC {
    public:
    valvulaMotorDC();
    void begin(uint8_t SW1, uint8_t SW2, uint8_t sleep, uint8_t IN1, uint8_t IN2);  
    boolean abre();
    boolean fecha();
    boolean abreParcial(unsigned long _tempoMilis);

    private:
    uint8_t _SW1Pin = 0;
    uint8_t _SW2Pin = 0;
    uint8_t _motorSleepPin = 0;
    uint8_t _motorIN1Pin = 0;
    uint8_t _motorIN2Pin = 0;
};

#endif