#ifndef displayST7565R_h
#define displayST7565R_h

#include <Arduino.h>

class displayST7565R {
    public:
    displayST7565R();
    void begin(uint8_t SI_Pin, uint8_t SC_Pin, uint8_t RS_Pin, uint8_t RST_Pin, uint8_t CS_Pin);
    void writeNumber(unsigned char *_lcd_string, uint8_t _inicio);
    void writeSymbol(unsigned char *_lcd_string, uint8_t _inicio, uint8_t _tamanho);
    void clear(uint8_t _inicio, uint8_t _tamanho);

    private:
    uint8_t SI_PIN = 0; // Serial data signal
    uint8_t SC_PIN = 0; // Serial clock signal
    uint8_t RS_PIN = 0; // Register select signal
    uint8_t RST_PIN = 0; // Reset signal
    uint8_t CS_PIN = 0; // Chip select signal

    void init();
    void commWrite(unsigned char _d);
    void dataWrite(unsigned char _d);
};

#endif