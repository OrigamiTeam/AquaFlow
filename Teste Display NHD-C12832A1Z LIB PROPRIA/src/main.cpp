#include <Arduino.h>
#include "displayST7565R.h"

#define SI 15 // Serial data signal
#define SC 16 // Serial clock signal
#define RS 17 // Register select signal
#define RES 18 // Reset signal
#define CS 19  // Chip select signal

displayST7565R LCD;

void setup() {
  LCD.begin(SI, SC, RS, RES, CS);
  delay(100);
  LCD.clear(0, 128);
  delay(100);
}

void loop() {
  LCD.write(LCD.numeros[0], 0, 14);
  delay(1);
  LCD.write(LCD.numeros[1], 18, 14);
  delay(1);
  LCD.write(LCD.numeros[2], 36, 14);
  delay(1);
  LCD.write(LCD.numeros[3], 54, 14);
  delay(1);
  LCD.write(LCD.numeros[4], 72, 14);
  delay(1);
  LCD.write(LCD.simbolos[0], 89, 2);
  delay(1);
  LCD.write(LCD.numeros[5], 94, 14);
  delay(1);
  LCD.write(LCD.simbolos[1], 113, 15);

  delay(5000);
  LCD.clear(0, 128);
  delay(1);

  LCD.write(LCD.numeros[4], 0, 14);
  delay(1);
  LCD.write(LCD.numeros[5], 18, 14);
  delay(1);
  LCD.write(LCD.numeros[6], 36, 14);
  delay(1);
  LCD.write(LCD.numeros[7], 54, 14);
  delay(1);
  LCD.write(LCD.numeros[8], 72, 14);
  delay(1);
  LCD.write(LCD.simbolos[0], 89, 2);
  delay(1);
  LCD.write(LCD.numeros[9], 94, 14);
  delay(1);
  LCD.write(LCD.simbolos[1], 113, 15);

  delay(5000);
  LCD.clear(0, 128);
  delay(1);
}