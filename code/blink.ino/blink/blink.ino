#include "FastLED.h"

#define NUM_LEDS 60
#define DATA_PIN 6

CRGB leds[NUM_LEDS];

void setup() { 

      FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
      
}


  uint8_t Hue = 0;
  uint8_t HueDelta = 3;

void loop() {

  byte i;
  for(i=0;i<NUM_LEDS;i++) {

    Hue += HueDelta;
    leds[i].setHue(Hue);
    FastLED.show();
  }
  
 
}

