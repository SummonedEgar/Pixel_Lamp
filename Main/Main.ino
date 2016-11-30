#include <FastLED.h>
#include <Wire.h>
#include <SparkFun_VL6180X.h>
//VL6180
#define PIN_0 2
#define N_Sensor 5

#define VL6180 0x29

//WS2182
#define NUM_LEDS    30
#define DATA_PIN    9 //led strip
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define FPS         100

//Global variables
//VL6180
VL6180x sensor[N_Sensor] = {
  VL6180x(VL6180),
  VL6180x(VL6180),
  VL6180x(VL6180),
  VL6180x(VL6180),
  VL6180x(VL6180)  
};

struct Data {

  uint8_t lux[N_Sensor] = {0,0,0,0,0};
  uint8_t mm[N_Sensor] = {0,0,0,0,0};
  uint8_t Hue[NUM_LEDS]= {0};  
  uint8_t Sat[NUM_LEDS]= {0};  
  uint8_t Val[NUM_LEDS]= {0};  

};

typedef struct Data data;

data Main; 

const uint8_t New_VL6180[ ]={0x2A,0x2B,0x2C,0x2D,0x2F};

//WS2182
CRGB leds[NUM_LEDS];

//Functions
void update_strip ( int led_num) {
    
  leds[led_num].setHSV(Main.Hue[led_num],Main.Sat[led_num],Main.Val[led_num]); 
  FastLED.show();
    
}

void calculate_val ( int n_sensor) { //n_sensor from 0 to 4

  double luminosity=(Main.lux[n_sensor])/255; 
  
  for (int i=0; i<(NUM_LEDS/N_Sensor); i++) {

    Main.Val[(i+1)*(n_sensor+1)]=luminosity;
      
  }
   
}

void setup() { 

  //VL6180 Initialization
  
  Serial.begin(115200); 
  Wire.begin(); 
    
  for(int i=0;i<N_Sensor;i++) { //Turn off all sensors
  
    pinMode((PIN_0 + i), OUTPUT);
    digitalWrite((PIN_0+i),LOW);
  
  }
  
   delay(500);
   
  for(int i=0;i<N_Sensor;i++) { //Setting the sensor and changing the address
    
    digitalWrite((PIN_0+i),HIGH);
    sensor[i].VL6180xDefautSettings();
    delay(1000);
    sensor[i].changeAddress(VL6180,New_VL6180[i]);
  }
  
  //WS2182 Initialization
  
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
      
}

void loop() {

  short H=0;
  
  for(int i=0 ; i<N_Sensor; i++) {
    
    Main.lux[i]=sensor[i].getAmbientLight(GAIN_1);
    Main.mm[i]=sensor[i].getDistance();
    
    calculate_val(i);
       
  }

  for(int i=0; i<NUM_LEDS; i++) {
  
    Main.Hue[i]=H;
    
    H=H+3;
    
    update_strip(i);
    
  }
  
}


