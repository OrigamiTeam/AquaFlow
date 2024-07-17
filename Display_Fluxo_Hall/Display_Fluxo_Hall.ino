#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Arduino_ST7789_Fast.h>

#define TFT_DC 10
#define TFT_RST 14
#define SCR_WD 240
#define SCR_HT 240

#define sensorPin 2

Arduino_ST7789 tft = Arduino_ST7789(TFT_DC, TFT_RST);

const double pulsosPorLitro = 25.85;

uint32_t pulseCount = 0;
// uint32_t pulseCountTotal = 0;
uint32_t pulseCountVolume = 0;

double volume = 0.0;
double volumeAnterior = 0.0;

unsigned long ultimaAtualizacao = 0;
unsigned long ultimaAtualizacaoFluxo = 0;

String volumeString = "0000.0";
String fluxoString = "000.00";
//String contTotalString = "0";

void pulseCounter() {
  pulseCount++;
  pulseCountVolume++;
}

void setup() {
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  tft.init(SCR_WD, SCR_HT);
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);

  tft.setTextSize(2);
  tft.setCursor(20, 10);
  tft.println("Sensor Fluxo Hall");

  tft.setTextSize(6);
  tft.setCursor(20, 60);
  tft.println(volumeString);
  tft.setTextSize(2);
  tft.setCursor(220, 110);
  tft.println("l");

  /*tft.setTextSize(6);
  tft.setCursor(20, 60);
  tft.println(contTotalString);
  tft.setTextSize(2);
  tft.setCursor(20, 110);
  tft.println("pulsos");*/

  tft.setTextSize(6);
  tft.setCursor(20, 170);
  tft.println(fluxoString);
  tft.setTextSize(2);
  tft.setCursor(170, 220);
  tft.println("l/min");

  attachInterrupt(digitalPinToInterrupt(sensorPin), pulseCounter, FALLING);
}

void loop() {
  /*if (statusPin && !digitalRead(contadorPin)) {
    delay(20);
    if (!digitalRead(contadorPin)) {
      volume += 0.5;
      statusPin = false;
      digitalWrite(ledPin, HIGH);
    }
  }

  if (!statusPin && digitalRead(contadorPin)) {
    delay(20);
    if (digitalRead(contadorPin)) {
      statusPin = true;
      digitalWrite(ledPin, LOW);
    }
  }*/

  if (millis() > ultimaAtualizacao + 500) {

    volume = pulseCountVolume / pulsosPorLitro;

    if (volume != volumeAnterior) {
      tft.setCursor(20, 60);
      tft.setTextSize(6);
      tft.setTextColor(BLACK);
      tft.println(volumeString);

      volumeString = "";
      if (volume < 10.0) {
        volumeString = "000";
      }
      else if (volume < 100.0) {
        volumeString = "00";
      }
      else if (volume < 1000.0) {
        volumeString = "0";
      }
      volumeString += String(volume, 1);

      tft.setTextSize(6); 
      tft.setTextColor(WHITE);
      tft.setCursor(20, 60);
      tft.println(volumeString);

      volumeAnterior = volume;
    }

    ultimaAtualizacao = millis();
  }

  unsigned long _millis = millis();
  if (_millis > ultimaAtualizacaoFluxo + 10000) {

    /*pulseCountTotal += pulseCount;

    tft.setCursor(20, 60);
    tft.setTextSize(6);
    tft.setTextColor(BLACK);
    tft.println(contTotalString);

    contTotalString = (String)pulseCountTotal;

    tft.setTextSize(6);
    tft.setTextColor(WHITE);
    tft.setCursor(20, 60);
    tft.println(contTotalString);*/

    double _tempoMillis = (double)_millis - (double)ultimaAtualizacaoFluxo;
    double _litros = pulseCount / pulsosPorLitro; // litros contabilizados
    double _fluxoMilisegundo =  _litros / _tempoMillis; // fluxo em L / ms
    double _fluxo = _fluxoMilisegundo * 60000.0; // flluxo em L / min

    tft.setCursor(20, 170);
    tft.setTextSize(6);
    tft.setTextColor(BLACK);
    tft.println(fluxoString);

    fluxoString = "";
    if (_fluxo < 10.0) {
      fluxoString = "00";
    }
    else if (_fluxo < 100.0) {
      fluxoString = "0";
    }
    fluxoString += String(_fluxo, 2);

    tft.setTextSize(6); 
    tft.setTextColor(WHITE);
    tft.setCursor(20, 170);
    tft.println(fluxoString);

    pulseCount = 0;
    ultimaAtualizacaoFluxo = _millis;
  }

  delay(10);
}