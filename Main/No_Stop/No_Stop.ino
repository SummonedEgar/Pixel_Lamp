#include <SparkFun_VL6180X.h>
#include <FastLED.h>
#include <Wire.h>
#include <math.h>

#define L0 60
#define E 150 //min diff actuation
#define H_Delta 4
#define T_OFF 300
#define T_Blink 400
#define C0 6

//VL6180
#define PIN_0 2
#define N_Sensor 5

#define VL6180 0x29

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
  
  byte state[N_Sensor];
  byte skip[N_Sensor];
  byte face[N_Sensor];
    
  uint8_t Hue[NUM_LEDS] ; 
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

uint8_t New_VL6180[N_Sensor]={0x2B,0x2D,0x19,0x13,0x25};

data Main;
data Temp;
unsigned long t_running=0;
uint8_t state[N_Sensor]={0};
byte cycle=0;

//WS2182
CRGB leds[NUM_LEDS];

//Functions
void update_led ( int led_num) {
    
  leds[led_num].setHSV(Main.Hue[led_num],Main.Sat[led_num],Main.Val[led_num]); 
  FastLED.show();

}

void update_Temp(int i) {
   
  Temp.lux[i]=Main.lux[i];
  Temp.mm[i]=Main.mm[i];

}

void get_data (int i) {

//  Main.lux[i]=sensor[i].getAmbientLight(GAIN_1);
  Main.mm[i]=sensor[i].getDistance();

}

int determine_state (int i) { 
  
    get_data(i);  
    
    if(Main.mm[i]<E) {
      Main.state[i]=1;
    } else {
      Main.state[i]=0;
    }
  
}

int stable_distance(int i) {//checks for persistency 

///////////DO NOT CALL t_running=millis() BEFORE THIS FUNCTION//////////

  if(millis()-t_running>T_OFF/4) {      
            
    get_data(i);
          
    if(Main.mm[i]>E) { //not close anymore
        
      return(0);
        
    } else if (millis()-t_running>T_OFF) {
      return(2);  
    }          
  }
}

void blink_led (int k, int a) {

  for(int i=0;i<k;i++) {
    for(int j=0;j<LPF;j++) {//OFF
              
      Main.Val[a*LPF+j]=0;
      update_led(a*LPF+j);
            
    }
    delay(T_Blink);
    for(int j=0;j<LPF;j++) {//ON
              
      Main.Hue[a*LPF+j]=0;
      Main.Sat[a*LPF+j]=0;
      Main.Val[a*LPF+j]=255;
      update_led(a*LPF+j);
            
    }
    delay(T_Blink);
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
    
    delay(300);
    
    sensor[i].VL6180xInit(); 
    sensor[i].VL6180xDefautSettings();
    delay(100);
    
    sensor[i].changeAddress(VL6180,New_VL6180[i]);
    
    get_data(i);
    update_Temp(i);
    Main.state[i]=0;
    
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

  byte check=0;
  byte check_2=0
  byte i,j,count=0;
  
  if(check!=1) {
  check=0;
  }
  switch(Main.state[cycle]) {
    
    case 0: //Sensor measures 255
  
    Main.state[cycle]=determine_state(cycle);
    t_running=millis();
    break;
    
    case 1: //Something went over the sensor
    
    Main.state[cycle]=stable_distance(cycle);
    t_running=millis();
    i=cycle;
    break;
    
    }
    
    case 2: //Hand is over the sensor

    if(check!=1) {//Blink until ready
    blink_led(2,cycle);
    }
    if(millis()-t_running>1500) { //Wait 1500 before capturing data
    
    j=(i+1)%N_Sensor; //Cycle number of other sensors
    check=1; //Stop blinking 
    if(i-cycle<N_Sensor-1) { //Cycle through all other sensors
      
      Main.state[j]=determine_state(j); //Check state
      i++; //Move to other sensor
    }      
    
    if(Main.state[j]==1) { //Hand over sensor over 1500 ms
        count += determine_state(j); //Start checking it's there
        if(count>C0) { //It's there
          Main.state[j]=2; //Hand stable
          count=0; //Reset for other sensors
        }
    } else {
      no_data++; //sum of no input sensors
      Main.state[j]=0;
            
    }
    if (no_data==4) { //no hand over any other sensors
      no_data=0;//reset
      if (check_2==0){ //start counting time
        prev_t=millis();
        check_2=1;
      }
      if(Main.Val[cycle*LPF]==0) {
        blink_led(2,cycle); //signal it's turning on
                      
        if(millis()-prev_t>2*T_BLINK) {
          Main.face[cycle]=1; //face on
          check_2=0; //reset
          break;
        }
      } else {
         blink_led(1,cycle); //signal it's turning off
                      
         if(millis()-prev_t>T_BLINK) {
         Main.face[cycle]=0; //face off
         check_2=0; //reset
         break;
        }
      }
      Main.state[cycle]==0;
      }   
  
      if(Main.state[j]==2) { 
    
        get_data();
        for(int j=0;j<LPF;j++) {
              
          Main.Hue[cycle*LPF+j]=Main.mm[cycle];
          update_led(cycle*LPF+j);
            
        }
       delay(50);
                     
       Main.state[j]=determine_state(j);
       if(Main.state[j]==0) {
       get_data();
       break;
          } 
     break;
        ;
        } 
      }      
    }
    }
  }
     
  
  for(int i=0;i<N_Sensor;i++) {

    for(int j=0;j<LPF;j++) {
      
      Main.Hue[(i*LPF+j)]=Main.Hue[(i*LPF+j)]+H_Delta;
      update_led(i*LPF+j);
      delay(1000/FPS);
      
    }
  }
  
  for(int i=0;i<N_Sensor;i++){
    get_data(i); 
  }
  
}


