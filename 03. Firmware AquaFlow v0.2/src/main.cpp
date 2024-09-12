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

// LEITURA DE FLUXO E TEMPERATURA (MAX35103)

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

float registerTemp(uint16_t _TxInt, uint16_t _TxFrac) {
  uint32_t _registerInt = (uint32_t)_TxInt * 250;
  float _registerFrac = (float)_TxFrac * 3.814755;
  _registerFrac = _registerFrac / 1000.0;
  float _registerTemp = (float)_registerInt + _registerFrac;

  return _registerTemp;
}

float temperaturaPT1000(float _R, float _R0) {
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

// DISPLAY NHD-C12832A1Z

void dataWrite(unsigned char d) // Data Output Serial Interface
{
  digitalWrite(LCD_CS, LOW);
  digitalWrite(LCD_A0, HIGH);
  for (unsigned int n = 0; n < 8; n++)
  {
    if ((d & 0x80) == 0x80)
      digitalWrite(LCD_SI, HIGH);
    else
      digitalWrite(LCD_SI, LOW);
    while (0);
    d = (d << 1);
    digitalWrite(LCD_SCL, LOW);
    while (0);
    digitalWrite(LCD_SCL, HIGH);
    while (0);
    digitalWrite(LCD_SCL, LOW);
  }
  digitalWrite(LCD_CS, HIGH);
}

void commWrite(unsigned char d) // Command Output Serial Interface
{
  digitalWrite(LCD_CS, LOW);
  digitalWrite(LCD_A0, LOW);
  for (unsigned int n = 0; n < 8; n++)
  {
    if ((d & 0x80) == 0x80)
      digitalWrite(LCD_SI, HIGH);
    else
      digitalWrite(LCD_SI, LOW);
    while (0);
    d = (d << 1);
    digitalWrite(LCD_SCL, LOW);
    while (0);
    digitalWrite(LCD_SCL, HIGH);
    while (0);
    digitalWrite(LCD_SCL, LOW);
  }
  digitalWrite(LCD_CS, HIGH);
}

void DispPic(unsigned char *lcd_string) {
  unsigned char page = 0xB0;
  commWrite(0xAE); // Display OFF
  commWrite(0x40); // Display start address + 0x40
  for (unsigned int i = 0; i < 4; i++)
  {                   // 32pixel display / 8 pixels per page = 4 pages
    commWrite(page); // send page address
    commWrite(0x10); // column address upper 4 bits + 0x10
    commWrite(0x00); // column address lower 4 bits + 0x00
    for (unsigned int j = 0; j < 128; j++)
    {                          // 128 columns wide
      dataWrite(*lcd_string); // send picture data
      lcd_string++;
    }
    page++; // after 128 columns, go to next page
  }
  commWrite(0xAF);
}

void ClearLCD(unsigned char *lcd_string) {
  unsigned char page = 0xB0;
  commWrite(0xAE); // Display OFF
  commWrite(0x40); // Display start address + 0x40
  for (unsigned int i = 0; i < 4; i++)
  {                   // 32pixel display / 8 pixels per page = 4 pages
    commWrite(page); // send page address
    commWrite(0x10); // column address upper 4 bits + 0x10
    commWrite(0x00); // column address lower 4 bits + 0x00
    for (unsigned int j = 0; j < 128; j++)
    {                   // 128 columns wide
      dataWrite(0x00); // send picture data
      lcd_string++;
    }
    page++; // after 128 columns, go to next page
  }
  commWrite(0xAF);
}

void InitLCD() {
  commWrite(0xA0); // ADC select
  commWrite(0xAE); // Display OFF
  commWrite(0xC8); // COM direction scan
  commWrite(0xA2); // LCD bias set
  commWrite(0x2F); // Power Control set
  commWrite(0x21); // Resistor Ratio Set
  commWrite(0x81); // Electronic Volume Command (set contrast) Double Btye: 1 of 2
  commWrite(0x20); // Electronic Volume value (contrast value) Double Byte: 2 of 2
  commWrite(0xAF); // Display ON
}

// MOTOR

boolean abreValvula() {
  if(!digitalRead(SW1Pin)) {
    delay(20);
    if (!digitalRead(SW1Pin)) {
      return true;
    }
  }

  digitalWrite(motorIN1Pin, HIGH);
  digitalWrite(motorIN2Pin, LOW);

  uint32_t _cont = 0;
  boolean _aguardar = true;
  while(_aguardar) {
    if(_cont > 1500) {
      digitalWrite(motorIN1Pin, LOW);
      return false;
    }

    if(!digitalRead(SW1Pin)) {
      delay(20);
      if (!digitalRead(SW1Pin)) {
        digitalWrite(motorIN1Pin, LOW);
        _aguardar = false;
      }
    }

    delay(10);
    _cont++;
  }
  return true;
}

boolean fechaValvula() {
  if(!digitalRead(SW2Pin)) {
    delay(20);
    if (!digitalRead(SW2Pin)) {
      return true;
    }
  }

  digitalWrite(motorIN1Pin, LOW);
  digitalWrite(motorIN2Pin, HIGH);

  uint32_t _cont = 0;
  boolean _aguardar = true;
  while(_aguardar) {
    if(_cont > 1500) {
      digitalWrite(motorIN2Pin, LOW);
      return false;
    }

    if(!digitalRead(SW2Pin)) {
      delay(20);
      if (!digitalRead(SW2Pin)) {
        digitalWrite(motorIN2Pin, LOW);
        _aguardar = false;
      }
    }

    delay(10);
    _cont++;
  }
  return true;
}

boolean abreValvulaParcial(uint32_t _tempoMilis) {
  if (!fechaValvula()) {
    return false;
  }
  delay(250);

  digitalWrite(motorIN1Pin, HIGH);
  digitalWrite(motorIN2Pin, LOW);

  unsigned long _tempoInicial = millis();
  while (millis() < _tempoInicial + _tempoMilis) {

    if(!digitalRead(SW1Pin)) {
      delay(20);
      if (!digitalRead(SW1Pin)) {
        digitalWrite(motorIN1Pin, LOW);
        return false;
      }
    }

    delay(10);
  }
  
  digitalWrite(motorIN1Pin, LOW);
  return true;
}

// SETUP

void setup() {
  pinMode(btnPin, INPUT);
  //digitalWrite(btnPin, HIGH); // Pullup externo

  pinMode(MAX35103INT, INPUT);
  digitalWrite(MAX35103INT, HIGH);

  pinMode(MAX35103RST, OUTPUT);
  digitalWrite(MAX35103RST, HIGH);

  pinMode(MAX35103CE, OUTPUT);
  digitalWrite(MAX35103CE, HIGH);

  pinMode(SW1Pin, INPUT);
  //digitalWrite(SW1Pin, HIGH); // Pullup externo

  pinMode(SW2Pin, INPUT);
  //digitalWrite(SW2Pin, HIGH); // Pullup externo

  pinMode(motorSleepPin, OUTPUT);
  digitalWrite(motorSleepPin, LOW); // LOW = sleep

  pinMode(motorIN1Pin, OUTPUT);
  digitalWrite(motorIN1Pin, LOW);

  pinMode(motorIN2Pin, OUTPUT);
  digitalWrite(motorIN2Pin, LOW);

  pinMode(pressaoPin, INPUT);

  pinMode(LCD_SI, OUTPUT);
  pinMode(LCD_SCL, OUTPUT);
  pinMode(LCD_A0, OUTPUT);
  pinMode(LCD_RST, OUTPUT);
  pinMode(LCD_CS, OUTPUT);

  digitalWrite(LCD_RST, LOW);
  delay(100);
  digitalWrite(LCD_RST, HIGH);
  delay(100);
  InitLCD();

  SPI.begin();

  /////////////////////////////////////////////////
  Serial.begin(115200);
  delay(100);

  Serial.println("\n\nIniciando...");
  delay(100);
  /////////////////////////////////////////////////

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

  writeRegister16(MAX35103_EVT_TMN_W, 0x006B);
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