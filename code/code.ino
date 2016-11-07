#include <Adafruit_NeoPixel.h>

#define PIN 6
#define NPix 40

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NPix, PIN, NEO_GRB + NEO_KHZ800);


void setup() {
  // Inizializzazione
  
  strip.begin();
  strip.show();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int j=0;j<NPix;j++) {

    for(int i=0;i<255;i++) {
      strip.setPixelColor(j, i, i, i);
      strip.show();
      delay(10);
    }
  }
  }
  
