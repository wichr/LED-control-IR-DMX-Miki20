/*
*  Control remoto de LED RGB 
*  Ejemplo de como hacer variar el color y el brillo con un led RGB  
*  con un mando a distancia.
*  Se utiliza un receptor de infrarrojos del tipo TSOP1738 
*  Autor: Jose Daniel Herrera
*  Fecha: 28/08/2012
*  http://arduino-guay.blogspot.com.es
*  http://playground.arduino.cc/Code/PwmFrequency
*  *  Pilot 44p     
      
0xFF3AC5  0xFFBA45  0xFF827D  0xFF02FD
0xFF1AE5  0xFF9A65  0xFFA25D  0xFF22DD
0xFF2AD5  0xFFAA55  0xFF926D  0xFF12ED
0xFF0AF5  0xFF8A75  0xFFB24D  0xFF32CD
0xFF38C7  0xFFB847  0xFF7887  0xFFF807
0xFF18E7  0xFF9867  0xFF58A7  0xFFD827
0xFF28D7  0xFFA857  0xFF6897  0xFFE817
0xFF08F7  0xFF8877  0xFF48B7  0xFFC837
0xFF30CF  0xFFB04F  0xFF708F  0xFFF00F
0xFF10EF  0xFF906F  0xFF50AF  0xFFD02F
0xFF20DF  0xFFA05F  0xFF609F  0xFFE01F
      
      
pilot dmx     
0xFFA25D  0xFF629D . 0xFFE21D . 
0xFF22DD  0xFF02FD  0xFFC23D  .
0xFFE01F  0xFFA857  0xFF906F  
0xFF6897  0xFF9867  0xFFB04F  
0xFF30CF  0xFF18E7  0xFF7A85  .
0xFF10EF  0xFF38C7  0xFF5AA5 . 
0xFF42BD.  0xFF4AB5.  0xFF52AD . 
      
Pilot 24      
0xF700FF  0xF7807F  0xF740BF  0xF7C03F
0xF720DF  0xF7A05F  0xF7609F  0xF7E01F
0xF710EF  0xF7906F  0xF750AF  0xF7D02F
0xF730CF  0xF7B04F  0xF7708F  0xF7F00F
0xF708F7  0xF78877  0xF748B7  0xF7C837
0xF728D7  0xF7A857  0xF76897  0xF7E817
*/
#include <Timers.h>
#include <IRremote.h>
#include <EEPROM.h>
#include <DMXSerial.h>


long FADE_IN_TIME = SECS(10.25);
long STROBO_IN_TIME = SECS(2.25);
long FADE_OUT_TIME = SECS(10.8);
long CYCLE_DELAY_TIME = SECS(5.75);
Timer timer;
Timer diy;
enum class BlinkState : byte {
  DELAY,
  FADE_IN,
  FADE_OUT
} state;

int RECV_PIN = 8;
int R_PIN = 6;
int G_PIN = 10;
int B_PIN = 9;
int W_PIN = 5;
byte fade = 57;
  byte brightness = 255;
#define ON                0xF7C03F
#define PAUSE             0xFF827D
#define OFF               0xFF02FD
#define BRIGHTNESS_UP     0xFF3AC5
#define BRIGHTNESS_DOWN   0xFFBA45
#define FLASH             0xFFD02F
#define AUTO              0xFFF00F
#define FADE3             0xFF609F
#define FADE7             0xFFE01F
#define JUMP3             0xFF20DF
#define JUMP7             0xFFA05F
#define QUICK             0xFFE817
#define SLOW              0xFFC837

#define RED_UP            0xFF28D7
#define RED_DOWN          0xFF08F7
#define GREEN_UP          0xFFA857 
#define GREEN_DOWN        0xFF8877  
#define BLUE_UP           0xFF6897 
#define BLUE_DOWN         0xFF48B7

#define DIY1              0xFF30CF
#define DIY2              0xFF10EF
#define DIY3              0xFFB04F   
#define DIY4              0xFF906F
#define DIY5              0xFF708F
#define DIY6              0xFF50AF

#define RED               0xFF1AE5
#define GREEN             0xFF9A65
#define BLUE              0xFFA25D
#define WHITE             0xFF22DD

#define ORANGE            0xFF2AD5
#define YELLOW_DARK       0xFF0AF5
#define YELLOW_MEDIUM     0xFF38C7
#define YELLOW_LIGHT      0xFF18E7

#define GREEN_LIGHT       0xFFAA55
#define GREEN_BLUE1       0xFF8A75
#define GREEN_BLUE2       0xFFB847
#define GREEN_BLUE3       0xFF9867

#define BLUE_RED          0xFF926D
#define PURPLE_DARK       0xFFB24D
#define PURPLE_LIGHT      0xFF7887
#define PINK              0xFF58A7

#define PINK_LIGHT        0xFF12ED
#define PINK_MED          0xFF32CD
#define BLUE_LIGHT        0xFFF807
#define BLUE_MED          0xFFD827

#define INCREMENTO 10

unsigned long rgb = 0;
byte r,g,b,w;

IRrecv irrecv(RECV_PIN);

decode_results results;

void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x07; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

unsigned long timeToEnd;  //= timer.time();
byte prev_fade;
byte dmx_fade;
byte full_program = 1;
byte dimmer = 255 ;
byte button=0;
void setup()
{
    DMXSerial.init(DMXReceiver);
    //Serial.begin(9600);
  irrecv.enableIRIn(); // Inicializamos el receptor
  pinMode(R_PIN, OUTPUT);   
  pinMode(G_PIN, OUTPUT);   
  pinMode(B_PIN, OUTPUT);  
  pinMode(W_PIN, OUTPUT); 
 // RGB(0x00FFFFFF);
  timer.begin(CYCLE_DELAY_TIME);
  diy.begin(SECS(2));
 setPwmFrequency(R_PIN, 8);
 setPwmFrequency(G_PIN, 8);
  setPwmFrequency(B_PIN, 8);
    setPwmFrequency(W_PIN, 8);
    
}

byte jumpUpdate(byte var) {
  brightness = 255;
    if (timer.available()) {
      timer.begin(FADE_IN_TIME);
      fade=fade+var;//fade--;
    }
return brightness;
}
byte blinkUpdate(byte var) {
 // byte brightness;
  
  switch (state) {
    case BlinkState::DELAY:
    brightness = 0;
    if (timer.available()) {
      state = BlinkState::FADE_IN;
      timer.begin(FADE_IN_TIME);
    fade=fade+var;//  fade--;
    }
    break;

    case BlinkState::FADE_IN:
    if (timer.available()) {
      state = BlinkState::FADE_OUT;
      timer.begin(FADE_OUT_TIME);
    }
    brightness = map(timer.time(), FADE_IN_TIME, 0, 0, 255);
    break;

    case BlinkState::FADE_OUT:
    if (timer.available()) {
      state = BlinkState::DELAY;
      timer.begin(CYCLE_DELAY_TIME);
   
    
    }
    brightness = map(timer.time(), FADE_OUT_TIME, 0, 255, 0);
    break;
  }
//Serial.println(brightness);
return brightness;
}

byte stroboUpdate(byte var) {

    if (timer.available()) 
  {
   if (brightness == 255) brightness = 0;
   else brightness = 255;
      timer.begin(FADE_IN_TIME/8); //  /50 STROBO_IN_TIME = SECS(2.25);
    //timer.restart();
   fade=fade+var; //fade--; 
  }
  return brightness;
}
 byte EppromUpdate (){// (byte adr,byte val) {
       if (diy.available()) 
  {    
    if (button>4)return 2;       // g=255;                 ///3// else if (button>2) b=255;            ///2
    else return 1;               //r=255;  
    diy.restart();
    } else return 0;
}
  void EppromColor (){
EEPROM.write(31, r); EEPROM.write(32, g); EEPROM.write(33, b); EEPROM.write(34 , w);EEPROM.write(35 , dimmer);
//r=g=b=w=0;
}
 
void variar1 (byte* color, char valor) {
        if( *color >= 0){
               if (valor > 0) {
        if ( *color + valor <= 255) {
            *color += valor;
        } else {
            *color = 255;
        }
    } else { 
        if (*color + valor >= 1) {
            *color += valor;
        } else {
            *color = 1;
        }
  }
        }
 
}
void variar (byte* color, char valor) {
        
               if (valor > 0) {
        if ( *color + valor <= 255) {
            *color += valor;
        } else {
            *color = 255;
        }
    } else { 
        if (*color + valor >= 0) {
            *color += valor;
        } else {
            *color = 0;
        }
  }
        
 
}


void RGB(unsigned long valor) {
   r = valor >> 16; 
   g = (valor >> 8) & 0xFF; 
   b = valor & 0xFF; 
   w = 0;
}

void loop() {
  if (irrecv.decode(&results)) {
   // Serial.print("0x");
    //  Serial.println(results.value, HEX);
    if ( results.value != 0xFFFFFFFF) {
      switch (results.value) {
           case BRIGHTNESS_UP : 
               variar1 (&dimmer, INCREMENTO);
               break; 
           case BRIGHTNESS_DOWN : 
               variar1 (&dimmer, -INCREMENTO);
               break; 
           case PAUSE         : if (fade !=20) {timeToEnd=timer.time(); timer.time(STOP); prev_fade=fade;  fade=20;}else {
                                timer.begin(timeToEnd); fade=prev_fade;}  break; 
           case OFF           : r=g=b=w=0;       fade=0;EppromColor();break; ///w=0 timer.time(STOP);
           case RED           : r=255;g=b=w=0;   fade=0;EppromColor();break; ///w=0   // EEPROM.write(31, r); EEPROM.write(32, g); EEPROM.write(33, b); EEPROM.write(34 , w);
           case GREEN         : RGB(0x0000FF00); fade=0;EppromColor();break;
           case BLUE          : RGB(0x000000FF); fade=0;EppromColor();break;
           case WHITE         : r=g=b=w=255;     fade=0;EppromColor();break;  //RGB(0x00FFFFFF); break;
           case ORANGE        : RGB(0x00FF7F00); fade=0;EppromColor();break;
           case YELLOW_DARK   : RGB(0x00FFAA00); fade=0;EppromColor();break;
           case YELLOW_MEDIUM : RGB(0x00FFD400); fade=0;EppromColor();break;
           case YELLOW_LIGHT  : RGB(0x00FFFF00); fade=0;EppromColor();break;
           case GREEN_LIGHT   : RGB(0x0000FFAA); fade=0;EppromColor();break;
           case GREEN_BLUE1   : RGB(0x0000FFFF); fade=0;EppromColor();break;
           case GREEN_BLUE2   : RGB(0x0000AAFF); fade=0;EppromColor();break;
           case GREEN_BLUE3   : RGB(0x000055FF); fade=0;EppromColor();break;
           case BLUE_RED      : RGB(0x00000080); fade=0;EppromColor();break;
           case PURPLE_DARK   : RGB(0x003F0080); fade=0;EppromColor();break;
           case PURPLE_LIGHT  : RGB(0x007A00BF); fade=0;EppromColor();break;
           case PINK          : RGB(0x00FF00FF); fade=0;EppromColor();break;
           case PINK_LIGHT    : RGB(0x00AA00FF); fade=0;EppromColor();break;
           case PINK_MED      : RGB(0x00FF00AA); fade=0;EppromColor();break;
           case BLUE_LIGHT    : r=50;g=50;b=255;w=0; fade=0;EppromColor();break; ///w=0
           case BLUE_MED      : r=20;g=20;b=200;w=0; fade=0;EppromColor();break; ///w=0
           case RED_UP        : variar (&r, INCREMENTO); fade=0;break;
           case RED_DOWN      : variar (&r, -INCREMENTO); fade=0;break;
           case GREEN_UP      : variar (&g, INCREMENTO);  fade=0;break;
           case GREEN_DOWN    : variar (&g, -INCREMENTO); fade=0;break;
           case BLUE_UP       : variar (&b,  INCREMENTO);  fade=0;break;
           case BLUE_DOWN     : variar (&b, -INCREMENTO); fade=0;break;
           case DIY1          :  fade=51; button = 1 ; diy.begin(SECS(4));  break;// EppromUpdate();   EEPROM.write(1 , r); EEPROM.write(2 , g); EEPROM.write(3 , b);  break;
           case DIY2          :  fade=52; button = 1 ; diy.begin(SECS(4));  break;// EEPROM.write(4 , r); EEPROM.write(6 , g); EEPROM.write(6 , b);  break;
           case DIY3          :  fade=53; button = 1 ; diy.begin(SECS(4));  break;//EEPROM.write(7 , r); EEPROM.write(8 , g); EEPROM.write(9 , b);  break;
           case DIY4          :  fade=54; button = 1 ; diy.begin(SECS(4));  break;//EEPROM.write(10, r); EEPROM.write(11, g); EEPROM.write(12, b);  break;
           case DIY5          :  fade=55; button = 1 ; diy.begin(SECS(4));  break;//EEPROM.write(13, r); EEPROM.write(14, g); EEPROM.write(15, b);  break;
           case DIY6          :  fade=56; button = 1 ; diy.begin(SECS(4));  break;//EEPROM.write(16, r); EEPROM.write(17, g); EEPROM.write(18, b);  break;


     
           case FADE3          :  r= 0;g= 0;b= 0;w=0; fade=4 ; timer.begin(CYCLE_DELAY_TIME);  full_program=0; break; ///w=0
           case FADE7          :  r= 0;g= 0;b= 0;w=0; fade=12 ; timer.begin(CYCLE_DELAY_TIME);  full_program=0; break; ///w=0
           case JUMP3          :  r= 0;g= 0;b= 0;w=0; fade=34 ; timer.begin(CYCLE_DELAY_TIME);  full_program=0; break; ///w=0
           case JUMP7          :  r= 0;g= 0;b= 0;w=0; fade=42 ; timer.begin(CYCLE_DELAY_TIME);  full_program=0; break; ///w=0
           case FLASH          :  r= 0;g= 0;b= 0;w=0; fade=23;  timer.begin(FADE_IN_TIME/8);   full_program=0; break; ///w=0  
           case AUTO           :  r= 0;g= 0;b= 0;w=0; fade=44;  timer.begin(FADE_IN_TIME/8);  full_program=20;  break; 
           case SLOW          :  if(FADE_IN_TIME < SECS(20)){  FADE_IN_TIME=FADE_IN_TIME*111/100; FADE_IN_TIME=FADE_IN_TIME*111/100; CYCLE_DELAY_TIME=CYCLE_DELAY_TIME*111/100; } timer.begin(CYCLE_DELAY_TIME); break;                         
           case QUICK           :  if(FADE_IN_TIME > SECS(0.5)){  FADE_IN_TIME=FADE_IN_TIME/111*100; FADE_IN_TIME=FADE_IN_TIME/111*100; CYCLE_DELAY_TIME=CYCLE_DELAY_TIME/111*100;} timer.begin(CYCLE_DELAY_TIME); break;    
      }

      
    } else if (fade > 50 and fade < 60 ) { button++;} //0xFFFFFFFF
    irrecv.resume(); // Receive the next value
  }//decode

    unsigned long lastPacket = DMXSerial.noDataSince();
 //   unsigned long lastPacket = 10000;
  
  if (lastPacket < 500) { //5000
   // read recent DMX values and set pwm levels  
     /*  analogWrite(R_PIN,   DMXSerial.read(1));
    analogWrite(G_PIN, DMXSerial.read(2));
    analogWrite(B_PIN,  DMXSerial.read(3));
       analogWrite(W_PIN,  DMXSerial.read(4));
     */
         r = DMXSerial.read(4);
            g = DMXSerial.read(3);
            b = DMXSerial.read(2);
             w = DMXSerial.read(5);
       if (DMXSerial.read(1)>10) dimmer = DMXSerial.read(1);      
        dmx_fade = DMXSerial.read(6);///5;
        if (dmx_fade != prev_fade and dmx_fade >0){
          prev_fade= dmx_fade;
          fade=dmx_fade/6;
        }

    }
    
  ///fade
        if(fade <= 0){
            
    //  analogWrite(R_PIN,map(r, 255, 0, dimmer, 0));
     // analogWrite(G_PIN,map(g, 255, 0, dimmer, 0));
     // analogWrite(B_PIN,map(b, 255, 0, dimmer, 0));
      }
 else if (fade == 1){
   if(full_program>0){fade=44; full_program=20 ;}else   fade=4;
 }
       else   if (fade == 2){
     r=blinkUpdate(-1); b=0; g=0; w=0;
  }else   if (fade == 3){   //r=0;
    g=blinkUpdate(-1);  b=0; r=0; w=0;
  }else   if (fade == 4){    
     b=blinkUpdate(-1);  g=0; r=0; w=0;
  }
    else    if (fade == 5){    
   if(full_program>0)fade=4 ;else    fade=12;
  } 
   else    if (fade == 6){    
       r=g=b=blinkUpdate(-1);w=0;
       
    }else   if (fade == 7){    
       r=g=blinkUpdate(-1);w=0;
       
       b=0; 
    }else   if (fade == 8){    
       g=blinkUpdate(-1);
       b=0;
       r=0;w=0; 
    }else   if (fade == 9){    
       g=b=blinkUpdate(-1);
      w=0;
       r=0;  
    }else   if (fade == 10){    
       b=blinkUpdate(-1);
       r=0; 
       g=0; w=0;
    }else   if (fade == 11){    
       r=b=blinkUpdate(-1);
     
       g=0; w=0; 
    }else   if (fade == 12){    
       r=blinkUpdate(-1);
       b=0;
       g=0; w=0;
  }   
 else if (fade == 13){     
   // if(full_program>0) ;else {   }
       r=g=b=blinkUpdate(0);w=0;//fade++;
       
 
    }else   if (fade == 14){ 
       r=g=blinkUpdate(0);w=0;//fade++;
      
       b=0;
    }else   if (fade == 15){  
       g=blinkUpdate(0);w=0;//fade++;
       b=0;
       r=0; 
    }else   if (fade == 16){  
       g=b=blinkUpdate(0);w=0;//fade++;
       
       r=0;  
    }else   if (fade == 17){   
       b=blinkUpdate(0);w=0; //fade++;
       r=0; 
       g=0; 
    }else   if (fade == 18){  
       r=b=blinkUpdate(0);w=0;//fade++;
       
       g=0;  
    }else   if (fade == 19){    
       r=blinkUpdate(0);//fade++;
       b=0;w=0;
       g=0; 
  }   
    else   if (fade == 20){    
      fade=19;
  } 
     else   if (fade == 21){    
      fade=19;
  } 
    else    if (fade == 22){    
    fade=23;
  } 
   else if (fade == 23){   
       r=g=b=w=stroboUpdate(-1);
      
    }else   if (fade == 24){ 
       r=g=stroboUpdate(0);w=0;
      
       b=0;//fade++;
    }else   if (fade == 25){  
       g=stroboUpdate(0);w=0;
       b=0;
       r=0;// fade++;
    }else   if (fade == 26){  
       g=b=stroboUpdate(0);w=0;
       
       r=0; // fade++;
    }else   if (fade == 27){   
       b=stroboUpdate(0);
       r=0; 
       g=0; //fade++;
    }else   if (fade == 28){ 
       r=b=stroboUpdate(0);w=0;
    
       g=0; // fade++;
    }else   if (fade == 29){  
       r=stroboUpdate(0);w=0;
       b=0;
       g=0; //fade++;
  }   
  
   else if (fade == 30){
  fade=29;
 }
  
  //fade//

  //jump
  else if (fade == 31){
   if(full_program>0)fade=12;else   fade=34;
 }
     else     if (fade == 32){
     r=jumpUpdate(-1); b=0; g=0; w=0;
  }else   if (fade == 33){   //r=0;
    g=jumpUpdate(-1);  b=0; r=0; w=0;
  }else   if (fade == 34){    
     b=jumpUpdate(-1);  g=0; r=0; w=0;
  }
     else   if (fade == 35){    
   if(full_program>0)fade=34 ; else    fade=42;
  } 
     else  if (fade == 36){    
       r=g=b=jumpUpdate(-1);w=0;
       
    }else   if (fade == 37){    
       r=g=jumpUpdate(-1);w=0;
       
       b=0;
    }else   if (fade == 38){    
       g=jumpUpdate(-1);w=0;
       b=0;
       r=0; 
    }else   if (fade == 39){    
       b=g=jumpUpdate(-1);w=0;
      
       r=0;  
    }else   if (fade == 40){    
       b=jumpUpdate(-1);w=0;
       r=0; 
       g=0; 
    }else   if (fade == 41){    
       b=r=jumpUpdate(-1);w=0;
      
       g=0;  
    }else   if (fade == 42){    
       r=jumpUpdate(-1);w=0;
       b=0;
       g=0; 
  }    //jump//
 
   else if (fade == 43){  
     
      if( full_program <= 1 )fade = 42;  else {full_program--; fade = 44; }
      
    }
  
 else if (fade == 44){   
     
       r=b=g=w=stroboUpdate(-1);
       
       // if( full_program==1 ) fade--;
  }    
    else   if (fade == 45){    
       r=b=g=w=stroboUpdate(0);//fade++;
  } 
      else   if (fade == 46){    
       r=b=g=w=stroboUpdate(0);//fade++;
  } 
      else   if (fade == 47){    
       r=b=g=w=stroboUpdate(0);//fade++;
  } 
      else   if (fade == 48){    
       r=b=g=w=stroboUpdate(0);//fade++;
  } 
      else   if (fade == 49){    
       r=b=g=w=stroboUpdate(0);//fade++;
  } 
      else   if (fade == 50){    
       r=b=g=w=stroboUpdate(0);
  } 
  
  
  //jump//
 else if (fade == 51){   
   if ( EppromUpdate() == 1)     { r = EEPROM.read(1); g = EEPROM.read(2); b = EEPROM.read(3); w = EEPROM.read(4); }
   else if (EppromUpdate() == 2) { EEPROM.write(1 , r); EEPROM.write(2 , g); EEPROM.write(3 , b);  EEPROM.write(4 , w); }
  } 
   else if (fade == 52){   
   if ( EppromUpdate() == 1)     { r = EEPROM.read(6); g = EEPROM.read(7); b = EEPROM.read(8); w = EEPROM.read(9); }
   else if (EppromUpdate() == 2) {  EEPROM.write(6 , r); EEPROM.write(7 , g); EEPROM.write(9 , b); EEPROM.write(9 , w);  }
  } 
     else if (fade == 53){   
   if ( EppromUpdate() == 1)     { r = EEPROM.read(11); g = EEPROM.read(12); b = EEPROM.read(13);w = EEPROM.read(14);  }
   else if (EppromUpdate() == 2) { EEPROM.write(11 , r); EEPROM.write(12 , g); EEPROM.write(13 , b); EEPROM.write(14 , w);  }
  } 
     else if (fade == 54){   
   if ( EppromUpdate() == 1)     { r = EEPROM.read(16); g = EEPROM.read(17); b = EEPROM.read(18); w = EEPROM.read(19); }
   else if (EppromUpdate() == 2) {   EEPROM.write(16, r); EEPROM.write(17, g); EEPROM.write(18, b);  EEPROM.write(19 , w); }
  } 
     else if (fade == 55){   
   if ( EppromUpdate() == 1)     { r = EEPROM.read(21); g = EEPROM.read(22); b = EEPROM.read(23);  w = EEPROM.read(24); }
   else if (EppromUpdate() == 2) { EEPROM.write(21, r); EEPROM.write(22, g); EEPROM.write(23, b); EEPROM.write(24 , w);}
  } 
     else if (fade == 56){   
   if ( EppromUpdate() == 1)     { r = EEPROM.read(26); g = EEPROM.read(27); b = EEPROM.read(28);  w = EEPROM.read(29); }
   else if (EppromUpdate() == 2) {  EEPROM.write(26, r); EEPROM.write(27, g); EEPROM.write(28, b); EEPROM.write(29 , w); }
  } 
   else if (fade == 57){   
   r = EEPROM.read(31); g = EEPROM.read(32); b = EEPROM.read(33);  w = EEPROM.read(34);// dimmer= EEPROM.read(35);//fade=0;
  } 
       else if (fade > 57){   
  fade = 0;
  } 

     // r=b=g=w=255;
        analogWrite(R_PIN,map(r, 255, 0, dimmer, 0));
      analogWrite(G_PIN,map(g, 255, 0, dimmer, 0));
      analogWrite(B_PIN,map(b, 255, 0, dimmer, 0));
       analogWrite(W_PIN,map(w, 255, 0, dimmer, 0));
 
}


