//Libraries
#include <SparkFun_VL6180X.h>
#include <FastLED.h>
#include <Wire.h>
#include <math.h>
#include <EEPROM.h>

//Costants
#define L0 60
#define E 254 //min diff actuation
#define T_OFF 300
#define T_Blink 400
#define C0 6
#define conversion 180/3.1452
#define ratio 180/8
#define symmetry 180/8
#define dt 360/LPF
#define FPS         100

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

//Typedef
struct Data {

  uint8_t lux[N_Sensor] = {0,0,0,0,0};
  uint8_t mm[N_Sensor] = {0,0,0,0,0};
  
  byte state[N_Sensor];
  byte facet[N_Sensor];
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

//Time variables
unsigned long t_running=0;
unsigned long timer_t=0;
unsigned long wait=0;

uint8_t cycle=0;
uint8_t led=0;
uint8_t lednum=0;
uint8_t no_data=0;
uint8_t prev_facet=0;
uint8_t tmp=0;
uint8_t j,count;
uint8_t now_face=0;

uint16_t t=0;

//WS2182
CRGB leds[NUM_LEDS];

//Functions
void update_led ( int led_num) {
    
  leds[led_num].setHSV(Main.Hue[led_num],Main.Sat[led_num],Main.Val[led_num]); 
  FastLED.show();

}

void get_data (int i) {

//  Main.lux[i]=sensor[i].getAmbientLight(GAIN_1);
  Main.mm[i]=sensor[i].getDistance();

}

int determine_state (int i) { 
  
    get_data(i);  
    
    if(Main.mm[i]<E) {
      return 1;
    } else {
      return 0;
    }
  
}

int stable_distance(int i) {//checks for persistency 

///////////DO NOT CALL t_running=millis() BEFORE THIS FUNCTION//////////
///////////USE ONCE FOR EACH STATE///////////

  if(millis()-t_running>T_OFF/4) {      
            
    get_data(i);
          
    if(Main.mm[i]>E) { //not close anymore
        
      return 0;
        
    } 
    else if (millis()-t_running>T_OFF) {
      t_running=millis();
      return 2;  
    } 
  }
  return 1;
}


int pulse( int angle) {
  
  if (angle<180-symmetry && angle >180-symmetry-ratio) {
    return 1;
  } else if (angle>180+symmetry && angle <180+symmetry+ratio) {
    return 1;
  } else {
    return 0;
  }

}

int absolute (int a) {
  if(a>0) {
    return (a);
  } else {
    return (-a);
  }
}

void cpy(int New, int Old) {
  
  for(int i=0; i<LPF; i++) {
    Main.Hue[Old*LPF+i]=Main.Hue[New*LPF+i];
    Main.Sat[Old*LPF+i]=Main.Sat[New*LPF+i];
    Main.Val[Old*LPF+i]=Main.Val[New*LPF+i];
  }
  Main.face[Old]=Main.face[New];
  Main.facet[Old]=Main.facet[New];
  
}

/////////SETUP///////////
void setup() { 
  
  //VL6180 Initialization
  
  Serial.begin(115200); 
  Wire.begin(); 
    
  for(int i=0;i<N_Sensor;i++) { //Turn off all sensors
  
    pinMode((PIN_0 + i), OUTPUT);
    digitalWrite((PIN_0+i),LOW);
  
  }
  
   delay(500);
  
  //WS2182 Initialization
  
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
     
  for(int i=0;i<N_Sensor;i++) { //Setting the sensor and changing the address
    
    digitalWrite((PIN_0+i),HIGH);
    
    delay(300);
    
    sensor[i].VL6180xInit(); 
    sensor[i].VL6180xDefautSettings();
    delay(100);
    
    sensor[i].changeAddress(VL6180,New_VL6180[i]);
    
    get_data(i);
    Main.state[i]=0;
    Main.facet[i]=EEPROM.read(i);
    Main.face[i]=EEPROM.read(i+N_Sensor);
    
    for(int k=0;k<LPF;k++) {
      Main.Hue[i*LPF+k]=EEPROM.read(i+2*N_Sensor);
      Main.Sat[i*LPF+k]=EEPROM.read(i+3*N_Sensor);
      Main.Val[i*LPF+k]=EEPROM.read(i+4*N_Sensor);
      update_led(i);
    }
  }


  
  //Initialization
  t_running=millis();    //t=0
  cycle=0;
  Serial.println("Inizializzazione Finita");
 
}

//////////MAIN///////////
void loop() {
  
  switch(Main.state[cycle]) {
    
    case 0: //Sensor measures 255
    
    Main.state[cycle]=determine_state(cycle);
    if(Main.state[cycle]==0) {
      cycle = (cycle+1)%N_Sensor;
    } 
    else {
      t_running=millis();
    }
    break;
    
    case 1: //Something went over the sensor
    
    Main.state[cycle]=stable_distance(cycle);
    if(Main.state[cycle]==2) {
      j=(cycle+1)%N_Sensor;
      prev_facet=Main.facet[cycle];
      Main.facet[cycle]=2; //Blink
      t_running=millis();
      wait=t_running;
      no_data=0;
    }
    break;
        
    case 2: //Hand is over the sensor
      
    if(millis()-wait>1500) { //Wait 1500 before capturing data
      
      Main.facet[cycle]=prev_facet; //Stop blinking
            
      switch(Main.state[j]) { //Change state with other sensors
        
      case 0:
        
        Main.state[cycle]=determine_state(cycle);
        if(Main.state[cycle]==0) {
        
          Main.state[cycle]=5;
          t_running=millis();
          break;
        } 
        else {
          Main.state[cycle]=2;
        }
        
        Main.state[j]=determine_state(j); //Check state
        if(Main.state[j]==0) {  
          no_data++; //sum of no input sensors
          if(no_data==4) {
            Main.face[cycle]=(Main.face[cycle]+1)%2; //turn on off and viceversa
            Main.state[cycle]=0;
            cycle=(cycle+1)%N_Sensor;
            break;
          }
          j=(j+1)%N_Sensor;
          if(j==cycle) {
            j=(cycle+1)%N_Sensor;
          }
          
        } 
        else { //Hand over sensor
          count=0;
          t_running=millis();
        }
        break;

        case 1: //Hand over sensor after 1500 ms
        
        if(millis()-t_running>200) {
            
          tmp=Main.mm[j];
          t_running=millis();
          
          if(absolute(tmp-Main.mm[j])<10) {    
            count++;
            
            if(count>10) {
              Main.state[cycle]=3;
              prev_facet=Main.facet[cycle];
              Main.facet[cycle]=2;
              count=0;
              t_running=millis();
              wait=t_running;
              break;
            }
          } 
          else {
            count=0;
          }
          }
          get_data(j);
          
          for(int i=0;i<LPF;i++) { //Change led color based on distance
            Main.Hue[cycle*LPF+i]=Main.mm[j];
            update_led(cycle*LPF+i);
          }
       
    }
    }
    break;
    
    case 3: //Change luminosity
    if(millis()-wait>1000) {
      
      Main.facet[cycle]=prev_facet;
      
      if(millis()-t_running>200) {
            
          tmp=Main.mm[j];
          t_running=millis();
          
          if(absolute(tmp-Main.mm[j])<10) {    
            count++;
            
            if(count>10) {
              Main.state[cycle]=4;
              prev_facet=Main.facet[cycle];
              Main.facet[cycle]=2;
              count=0;
              t_running=millis();
              wait=t_running;
              break;
            }
          } 
          else {
            count=0;
          }
          }
    get_data(j);
       
          
    for(int i=0;i<LPF;i++) { //Change led color based on distance
      Main.Val[cycle*LPF+i]=Main.mm[j];
      update_led(cycle*LPF+i);
    }
    }
    break; 
    
    case 4: //Change mode
    
    if(millis()-wait>1000) {

    
    if(millis()-t_running>200) {
            
          tmp=Main.mm[j];
          t_running=millis();
          
          if(absolute(tmp-Main.mm[j])<10) {    
            count++;
            
            if(count>10) {
              Main.state[cycle]=0;
              cycle=(cycle+1)%N_Sensor;
              count=0;
              t_running=millis();
              wait=t_running;
              break;
            }
          } 
          else {
            count=0;
          }
          }
    
    get_data(cycle);

    Main.facet[cycle]=(int)(Main.mm[cycle]/63);
    
    } 
    else {
      Main.facet[cycle]=2;
    }
    break;
    
    case 5: //Copy face

      cpy(j,cycle);
      Main.state[cycle]=0;
      cycle = (cycle+1)%N_Sensor;
      break;
  }

  switch(Main.face[now_face]) {

    case 0: //Face Off
    
      led=0;
      now_face= (now_face+1)%N_Sensor;
      break;

    case 1: //Face On
        
        switch(Main.facet[now_face]) {
        
        case 0: //Solid color
          update_led(lednum);
          break;
          
        case 1: //Breathing
          leds[lednum]= Main.Val[lednum]*absolute((sin(t*conversion)));
          break;

        case 2: //Pulse 
          leds[lednum]= Main.Val[lednum]*pulse(t);
          break;
        
        case 3: //Snake
          leds[lednum]= Main.Val[lednum]*absolute((sin((t+dt)*conversion)));
          break;
        

    }
  }

  if(millis()-timer_t>1000/FPS) {
    
    timer_t=millis();
    t = (t+1)%360;
    led=(led+1)%NUM_LEDS;
    
    if(led==0){
    now_face= (now_face+1)%N_Sensor;
    }
    
    lednum=now_face*LPF+led;
    
      EEPROM.update(cycle,Main.facet[cycle]);  
      EEPROM.update(cycle+N_Sensor,Main.face[cycle]);
      EEPROM.update(cycle+2*N_Sensor,Main.Hue[cycle*LPF]);
      EEPROM.update(cycle+3*N_Sensor,Main.Sat[cycle*LPF]);
      EEPROM.update(cycle+4*N_Sensor,Main.Val[cycle*LPF]);  
    }
  
  
  FastLED.show();
}
