#ifndef MAX35103_h
#define MAX35103_h

#include <Arduino.h>
#include <SPI.h>

#define MAX35103_SPI_CLOCK 4000000

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

class MAX35103 {
    public:
    MAX35103();
    void begin(uint8_t intPin, uint8_t rstPin, uint8_t cePin);
    void config();
    uint16_t readRegister16(uint8_t _address);
    boolean reset();
    boolean toFlash();
    boolean initialize();
    boolean temperatura(uint8_t _sensor, float *_temperatura);
    boolean fluxoToFDIff(float *_fluxo);
    boolean fluxoToFDIff(float *_fluxo, float *_ToFDiff);

    private:
    uint8_t _intPin = 0;
    uint8_t _rstPin = 0;
    uint8_t _cePin = 0;

    uint16_t _timeout = 2000; // em ms
    
    void writeRegister16(uint8_t _address, uint16_t _value);
    void opcodeCommand(uint8_t _command);
    boolean interruptStatus(uint8_t _bit);
    float ToF_Diff(uint16_t _TOF_DIFFInt, uint16_t _TOF_DIFFFrac);
    float fluxoAgua(float _deltaToF);
    float registerTemp(uint16_t _TxInt, uint16_t _TxFrac);
    float temperaturaPT1000(float _R, float _R0);
    //uint16_t readFlash16(uint16_t _address);
    //void writeFlash16(uint16_t _address, uint16_t _value);
    //void eraseFlash(uint16_t _address);
};

#endif