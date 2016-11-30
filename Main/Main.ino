#include <SparkFun_VL6180X.h>
#include <FastLED.h>
#include <Wire.h>
#include <math.h>

#define L0 60
#define E 50 //min diff actuation
#define H_Delta 4
//VL6180
#define PIN_0 2
#define N_Sensor 5

#define VL6180 0x29

#define VL6180_0 0x2B
#define VL6180_1 0x2D
#define VL6180_2 0x13
#define VL6180_3 0x19
#define VL6180_4 0x25

//WS2182
#define NUM_LEDS    30
#define DATA_PIN    9 //led strip
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define LPF NUM_LEDS/N_Sensor
#define FPS         100

//Typedef
struct Data {

  uint8_t lux[N_Sensor] = {0,0,0,0,0};
  uint8_t mm[N_Sensor] = {0,0,0,0,0};
  uint8_t Hue[NUM_LEDS]= {0};  
  uint8_t Sat[NUM_LEDS] ;  
  uint8_t Val[NUM_LEDS] ;  

};

typedef struct Data data;

//Global variables
//VL6180
VL6180x sensor[N_Sensor] = {
  VL6180x(VL6180),
  VL6180x(VL6180),
  VL6180x(VL6180),
  VL6180x(VL6180),
  VL6180x(VL6180)  
};

uint8_t New_VL6180[N_Sensor]={VL6180_0,VL6180_1,VL6180_2,VL6180_3,VL6180_4};

data Main;
data Temp; 
unsigned long t_running=0;
uint8_t state[N_Sensor]={0};
uint8_t skip_check=0;

//WS2182
CRGB leds[NUM_LEDS];

//Functions
void update_led ( int led_num) {
    
  leds[led_num].setHSV(Main.Hue[led_num],Main.Sat[led_num],Main.Val[led_num]); 
  FastLED.show();
    
}

void calculate_val ( int n_sensor) { //n_sensor from 0 to 4
  
  uint8_t luminosity;
  
  luminosity = (Main.lux[n_sensor]/10000)*(255-L0)+L0;
  
  for (int i=0; i<(NUM_LEDS/N_Sensor); i++) {

    Main.Val[(i+1)*(n_sensor+1)]=luminosity;
      
  }
   
}

void update_Temp(int i) {
   
  Temp.lux[i]=Main.lux[i];
  Temp.mm[i]=Main.mm[i];

}

void get_data (int i) {

  Main.lux[i]=sensor[i].getAmbientLight(GAIN_1);
  Main.mm[i]=sensor[i].getDistance();

}

int mm_diff(int i) {

  return(Temp.mm[i]-Main.mm[i]);
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
    
    delay(100);
    
    sensor[i].VL6180xInit(); 
     
    sensor[i].VL6180xDefautSettings();
    delay(100);
    sensor[i].changeAddress(VL6180,New_VL6180[i]);
    get_data(i);
    update_Temp(i);
  }

  //WS2182 Initialization
  
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  for(int i=0;i<NUM_LEDS;i++) {
    Main.Hue[i]=0;
    Main.Sat[i]=255;
    Main.Val[i]=255;
    update_led(i);
  }
  //Initialization
  t_running=millis();    //t=0
  

   
}

void loop() {

  byte check;
  
  skip_check=1;
  
/////////////////IO BEGIN//////////////
  if(millis()-t_running>300) { //Scan every 300 ms
    
    t_running=millis();
    
    for(int i=0 ; i<N_Sensor; i++) { //Get data /// determine state

     update_Temp(i);
     get_data(i);  
/////////////////IO END///////////////    
    
     if(mm_diff(i)>E) {
      Serial.println("Ho una differenza");
      state[i]=1;
    } else {
      state[i]=0;
    }
    }
  }   
    
    if(skip_check==1){
        Serial.println("Sto Facendo il ciclo");
    for(int i=0 ; i<N_Sensor; i++) {
      if (state[i]==1) { 
        Serial.println("Ho controllato la variazione");
        update_Temp(i);
        check=1;
        t_running=millis();
        
        while(check==1){
          Serial.println("Sono nel while");
          get_data(i);
          if(mm_diff(i)>E) {
            Serial.println("Troppa Variazione");
            state[i]=0;
          } else if (millis()-t_running>1000) {
            Serial.println("Ciclo Riuscito");
            check=2;
          }
        }
          delay(250);
          if(state[i]==0) {
            Serial.println("Sto per uscire dal ciclo");
            check=0;
          } 
        
        if(check==2){
          Serial.println("Spengo i led");
        for(int j=0 ; j<LPF; j++){
          Main.Val[(i*LPF+j)]=0;
          update_led(i*LPF+j);        
        }
      }
    }
    }
  

  }
  for(int i=0;i<N_Sensor;i++){
    get_data(i); 
  }
  
  for(int i=0;i<N_Sensor;i++) {

    for(int j=0;j<LPF;j++) {
      
      Main.Hue[(i*LPF+j)]=Main.Hue[(i*LPF+j)]+H_Delta;
      update_led(i*LPF+j);
      delay(1000/FPS);
      
    }
  }
  
}


