#include <Arduino.h>

#include <Ynvisible_Driver_4.2-1.0.0.h>

int i2c_address = 43;         //The i2c address of driver board 4.1
int number_of_segments = 15;  //Number of segments on display (1-15)

YNV_ECD ECD(i2c_address, number_of_segments); //ECD Object

uint8_t pinos[7] = {7, 6, 8, 9, 10, 12, 11};

uint8_t contA = 0;
uint8_t contB = 0;

byte displaySeteSeg[10][7] = { 
 { 1,1,1,1,1,1,0 },  //DIGITO 0
 { 0,1,1,0,0,0,0 },  //DIGITO 1
 { 1,1,0,1,1,0,1 },  //DIGITO 2
 { 1,1,1,1,0,0,1 },  //DIGITO 3
 { 0,1,1,0,0,1,1 },  //DIGITO 4
 { 1,0,1,1,0,1,1 },  //DIGITO 5
 { 1,0,1,1,1,1,1 },  //DIGITO 6
 { 1,1,1,0,0,0,0 },  //DIGITO 7
 { 1,1,1,1,1,1,1 },  //DIGITO 8
 { 1,1,1,1,0,1,1 }   //DIGITO 9
};

void ligaSegmentosDisplay(uint8_t digito){  
  for (uint8_t _i = 0; _i < 7; _i++) { 
    digitalWrite(pinos[_i], displaySeteSeg[digito][_i]);
  }
}

void setup() {
  for (uint8_t _i = 0; _i < 7; _i++) {
    pinMode(pinos[_i], OUTPUT);
    digitalWrite(pinos[_i], LOW);
  }
}

void loop() {
  /*for(int i = 00; i < 100; i++) {
    ECD.setNumber2x7(i);
    delay(2000);
    ECD.refresh();
    delay(3000);
  }*/

  ligaSegmentosDisplay(contA);
  ECD.setNumber2x7(contB);
  delay(2000);
  
  ligaSegmentosDisplay(contA);
  ECD.refresh();
  delay(2000);

  contA++;
  contB++;

  if (contA > 9) {
    contA = 0;
  }

  if (contB > 99) {
    contB = 0;
  }
}