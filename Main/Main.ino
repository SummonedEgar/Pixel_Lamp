#include <FastLED.h>
#include <Wire.h>
#include <SparkFun_VL6180X.h>

//VL6180
#define VL6180X_ADDRESS 0x29
//WS2182
#define NUM_LEDS    60
#define DATA_PIN    6
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define FPS         100

VL6180x sensor(VL6180X_ADDRESS);
CRGB leds[NUM_LEDS];

uint8_t Hue = 0;
uint8_t HueDelta = 3;
  
void setup() { 

  //VL6180 Initialization
  
  Serial.begin(115200); //Start Serial at 115200bps
  Wire.begin(); //Start I2C library
  delay(100); // delay .1s

  sensor.VL6180xDefautSettings();
  
  //WS2182 Initialization
 
  delay( 3000 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
      
}

void loop() {

  byte i;
  
  for(i=0;i<NUM_LEDS;i++) {

    Hue += HueDelta;
    leds[i].setHue(Hue);
    FastLED.show();
    FastLED.delay(1000 / FPS);
  }
  
 
}

