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
  
  for(int j=0;j<NPix;j++) {

    for(int i=0;i<255;i++) {
      strip.setPixelColor(j, i, i, i);
      strip.show();
      delay(10);
    }
    
  }
  }

int w_spectrum( byte H, byte S, byte V ) {
    
    byte R;
    byte G;
    byte B;
    
    
    if ( S == 0 ) { //greyscale
        
        R = V;
        G = V;
        B = V;

        
    } else {
            if ( H == 255 ){
              H = 0;  
            } 
            
            region = H / 43;
            /* find remainder part, make it from 0-255 */
            fpart = (h - (region * 43)) * 6;
    
            /* calculate temp vars, doing integer multiplication */
            var_1 = (v * (255 - s)) >> 8;
            var_2 = (v * (255 - ((s * fpart) >> 8))) >> 8;
            var_3 = (v * (255 - ((s * (255 - fpart)) >> 8))) >> 8;
            
            if ( H >= 213 )                 { R = var_2 ; G = V     ; B = var_1 }
            else if ( H >= 170 && H < 213 ) { R = var_1 ; G = V     ; B = var_3;}
            else if ( H >= 128 && H < 170 ) { R = var_1 ; G = var_2 ; B = V;    }
            else if ( H >= 85  && H < 128 ) { R = var_3 ; G = var_1 ; B = V;    }
            else if ( H >= 43  && H < 85  ) { R = V     ; G = var_1 ; B = var_2;}
            else                            { R = V     ; G = var_3 ; B = var_1;}
 
    }
    return (color);
}
