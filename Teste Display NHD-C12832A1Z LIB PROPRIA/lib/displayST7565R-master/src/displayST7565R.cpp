#include "displayST7565R.h"

displayST7565R::displayST7565R() {}

// Command Output Serial Interface
void displayST7565R::commWrite(unsigned char _d) {
  digitalWrite(CS_PIN, LOW);
  digitalWrite(RS_PIN, LOW);
  for (unsigned int n = 0; n < 8; n++) {
    if ((_d & 0x80) == 0x80) {
      digitalWrite(SI_PIN, HIGH);
    }
    else {
      digitalWrite(SI_PIN, LOW);
    }
    while (0);
    _d = (_d << 1);
    digitalWrite(SC_PIN, LOW);
    while (0);
    digitalWrite(SC_PIN, HIGH);
    while (0);
    digitalWrite(SC_PIN, LOW);
  }
  digitalWrite(CS_PIN, HIGH);
}

void displayST7565R::init() {
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

void displayST7565R::begin(uint8_t SI_Pin, uint8_t SC_Pin, uint8_t RS_Pin, uint8_t RST_Pin, uint8_t CS_Pin) {
  SI_PIN = SI_Pin;
  SC_PIN = SC_Pin;
  RS_PIN = RS_Pin;
  RST_PIN = RST_Pin;
  CS_PIN = CS_Pin;

  pinMode(SI_PIN, OUTPUT);
  pinMode(SC_PIN, OUTPUT);
  pinMode(RS_PIN, OUTPUT);
  pinMode(RST_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);

  digitalWrite(RST_PIN, LOW);
  delay(100);
  digitalWrite(RST_PIN, HIGH);
  delay(100);
  init();
}

// Data Output Serial Interface
void displayST7565R::dataWrite(unsigned char _d) {
  digitalWrite(CS_PIN, LOW);
  digitalWrite(RS_PIN, HIGH);
  for (unsigned int n = 0; n < 8; n++) {
    if ((_d & 0x80) == 0x80) {
      digitalWrite(SI_PIN, HIGH);
    }
    else {
      digitalWrite(SI_PIN, LOW);
    }
    while (0);
    _d = (_d << 1);
    digitalWrite(SC_PIN, LOW);
    while (0);
    digitalWrite(SC_PIN, HIGH);
    while (0);
    digitalWrite(SC_PIN, LOW);
  }
  digitalWrite(CS_PIN, HIGH);
}

void displayST7565R::write (unsigned char *_lcd_string, uint8_t _inicio, uint8_t _tamanho) {
  unsigned char page = 0xB0;
  commWrite(0xAE); // Display OFF
  commWrite(0x40); // Display start address + 0x40
  // 32pixel display / 8 pixels per page = 4 pages
  for (unsigned int i = 0; i < 4; i++) {
    commWrite(page); // send page address
    commWrite(0x10 + (_inicio >> 4)); // column address upper 4 bits + 0x10
    commWrite(0x00 + (_inicio & 0x0F)); // column address lower 4 bits + 0x00
    for (unsigned int j = 0; j < _tamanho; j++) {
      dataWrite(*_lcd_string); // send picture data
      _lcd_string++;
    }
    page++; // after the columns, go to next page
  }
  commWrite(0xAF);
}

void displayST7565R::clear (uint8_t _inicio, uint8_t _tamanho) {
  unsigned char page = 0xB0;
  commWrite(0xAE); // Display OFF
  commWrite(0x40); // Display start address + 0x40
  // 32pixel display / 8 pixels per page = 4 pages
  for (unsigned int i = 0; i < 4; i++) {
    commWrite(page); // send page address
    commWrite(0x10 + (_inicio >> 4)); // column address upper 4 bits + 0x10
    commWrite(0x00 + (_inicio & 0x0F)); // column address lower 4 bits + 0x00
    for (unsigned int j = 0; j < _tamanho; j++) {
      dataWrite(0x00); // send data
    }
    page++; // after 128 columns, go to next page
  }
  commWrite(0xAF);
}