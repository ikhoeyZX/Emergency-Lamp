/*********
//
// Lampu Emergency v1.3 REV 0, (c) 2023 - 2024 Undens Corp.
// DESIGNED FOR STM8 SMD board
//
**********/

#include <Serial.h>
#define digitalPinToInterrupt(p)  ((p)==(p)) // workaround-fix, empty macro is missing in header files

/*********
//
// PIN
//
**********/
// DIGITAL
const uint8_t BAT1_REY=2;   // Battery charger 1 mosfet
const uint8_t BAT2_REY=4;   // Battery charger 2 mosfet
const uint8_t BUZZER=5;     // Speaker buzzer type
const uint8_t V5_ON=7;      // Adapt detector
const uint8_t AC_DET=8;     // AC voltage detector
const uint8_t VIB_SEN_X=PA1;  // Vibrator sensor
const uint8_t VIB_SEN_Y=PA2;  // Vibrator sensor
const uint8_t MOSFETOUT=13; // PIN LED_OUT

// ANALOG
const uint8_t LDR=PC4;
const uint8_t BAT1=PD2;
const uint8_t BAT2=PD3;

// VARIABLE
volatile bool quakedet = LOW;
const uint16_t LDRtrig = 200;	// Set LDR detect as no light
const uint16_t limit = 2560; 	// Set limit count to reread battery since no need read battery every milis, max uint16_t is 65535

uint16_t bat1cal = 512;		 // Battery calibration, need DMM to calibrate it
uint16_t bat2cal = 512;		 // Battery calibration, need DMM to calibrate it

float bat1s=0,bat2s = 0;		// Set default battery condition
float battery_full = 4.18;  // battery full
float battery_low = 3.35;   // battery empty

uint16_t count = 0;         // counting to trigger "limit"
bool buzzcall = false;      // time loop buzzer ring

/*********
//
// Speaker
//
**********/
void buzzer(uint16_t duration){  // max 65535ms should be enough
  digitalWrite(BUZZER,HIGH);
  delay(duration);
  digitalWrite(BUZZER,LOW);
  delay(duration/2);
}
 

/*********
//
// Read battery
//
**********/
float baterai(uint8_t pinbat, uint16_t batcal, uint8_t relaypin, bool pilih){  // read battery
  float a=0; 
  for(uint8_t b=0; b<10; b++){    // Smoothing battery value
    a = a + ((analogRead(pinbat) * (3.3 / batcal)));
    delay(1);
  }
  a = a/10;
  
  if(!pilih){ // flipped since P-channel mosfet
    if(a >= battery_full){ // when battery full
      digitalWrite(relaypin, LOW);  
    }else if(a <= 1){  // when no battery
      digitalWrite(relaypin, LOW); 
    }else if(a <= (battery_full - 0.28) && a > 1){  // when battery low
      digitalWrite(relaypin, HIGH);
    }
  }else{ // when battery finished charger
    digitalWrite(relaypin, LOW);
  }

  return a;   // return battery value
}

void ledbuildin(){
  digitalWrite(LED_BUILTIN, LOW);     // turn the LED off (HIGH is the voltage level)
  delay(10);                          // wait for a second
  digitalWrite(LED_BUILTIN, HIGH);    // turn the LED on by making the voltage LOW
  delay(10);   
}

bool smoothlogic(uint16_t sensorin){  // smoothing digital AC detector
  uint8_t acc=0;

  for(uint8_t c=0; c<5; c++){
    acc = acc + digitalRead(sensorin);
    delay(1);
  }
  if(acc >3){   // when AC voltage detected high for 3x
    return HIGH;
  }

  return LOW;
}

void ISR(){ // Interrupt because quake sensor trigger (tilt sensor)
  quakedet = HIGH;
  digitalWrite(BUZZER,HIGH);
}

/*********
//
// Setup
//
**********/
void setup() {
  // DIGITAL PIN
  pinMode(BAT1_REY, OUTPUT);
  pinMode(BAT2_REY, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(V5_ON, INPUT_PULLUP);
  pinMode(AC_DET, INPUT_PULLUP);
  pinMode(VIB_SEN_X, INPUT_PULLUP);
  pinMode(VIB_SEN_Y, INPUT_PULLUP);
  pinMode(MOSFETOUT, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // ANALOG INPUT PIN: BAT1, BAT2, LDR
  digitalWrite(BAT1_REY,HIGH);
  digitalWrite(BAT2_REY,HIGH);
  digitalWrite(MOSFETOUT,HIGH);

///////////////////////////////////////////////////
// uncomment Serial_begin bellow to calibrate bat1 and bat2 after that you can disable this again
///////////////////////////////////////////////////

  // Serial_begin(9600); 

///////////////////////////////////////////////////

  ledbuildin();
  attachInterrupt(digitalPinToInterrupt(VIB_SEN_X), ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(VIB_SEN_Y), ISR, CHANGE);
  buzzer(10);
  buzzer(20);
  buzzer(10);

  // Read battery now!
  bool v5_on = digitalRead(V5_ON);
  bat1s = baterai(BAT1, bat1cal, BAT1_REY, v5_on);
  bat2s = baterai(BAT2, bat2cal, BAT2_REY, v5_on);
}


/*********
//
// Main
//
**********/
void loop() {
  bool v5_on = smoothlogic(V5_ON);
  bool ac = smoothlogic(AC_DET);
  uint16_t bacaldr = analogRead(LDR);

  if(buzzcall){

  }else if(v5_on == HIGH && ac == LOW && !buzzcall){ // ring buzzer if adaptor not yet on but AC voltage exist
    buzzer(50);
    buzzer(5);
    buzzer(20);
    buzzer(10);

    buzzcall = true;
  }

  if(count > limit){
    bat1s = baterai(BAT1, bat1cal, BAT1_REY, v5_on);  // Read battery 1
    bat2s = baterai(BAT2, bat2cal, BAT2_REY, v5_on);  // Read battery 2
    ledbuildin();  // blink led

///////////////////////////////////////////////////
// uncomment bellow this to calibrate battery1 and battery2
///////////////////////////////////////////////////

    // Serial_print_s("\nBat1: ");
    // Serial_println_f(bat1s);
    // Serial_print_s("Bat2: ");
    // Serial_println_f(bat2s);

///////////////////////////////////////////////////
    
    count = 0; //reset counter
    buzzcall = false;
 }else{
    count++;
 }

///////////////////////////////////////////////////
// uncomment bellow this to debug sensors pin output
///////////////////////////////////////////////////

    // Serial_print_s("D5v: ");
    // Serial_println_i(v5_on);
    // Serial_print_s("DAC: ");
    // Serial_println_i(ac);
    // Serial_print_s("SGMP: ");
    // Serial_println_i(quakedet);
    // Serial_print_s("SLDR: ");
    // Serial_println_u(bacaldr);

///////////////////////////////////////////////////


  if(quakedet == HIGH && bacaldr < LDRtrig ){   // Quake detected and it happen at night
    digitalWrite(MOSFETOUT, HIGH);
    ledbuildin();
//    Serial_println_s("GMP_MLM");
    delay(500);
    quakedet= LOW;
  }else if(quakedet== HIGH){  // Quake detected and it happen at morning or afternoon
    digitalWrite(MOSFETOUT, LOW);
    ledbuildin();
//    Serial_println_s("GMP_SNG");
    quakedet = LOW;
  }
  digitalWrite(BUZZER,LOW); // Turn off buzzer after detect quake


  if(!buzzcall){

  }else if(bat1s <= battery_low && bat2s <= battery_low && ac == HIGH){ // Turn off mosfet output because low battery
    digitalWrite(MOSFETOUT, LOW);
//    Serial_println_s("LWBAT");
    delay(10000);
  }else if(bat1s < (battery_low + 0.2) && bat2s <= (battery_low + 0.2) && ac == HIGH && !buzzcall){ // Turn off mosfet output and ring speaker because low battery
    digitalWrite(MOSFETOUT, HIGH);
//    Serial_println_s("LWBAT_WRN");
    buzzer(110);
    buzzer(80);
    buzzer(110);
    buzzcall = true;
  }else if(ac == HIGH && bacaldr < LDRtrig){  // Turn on mosfet because no AC voltage detected
    digitalWrite(MOSFETOUT, HIGH);
//    Serial_println_s("NOACMLM"); 
  }else if(bat1s < 1 && bat2s < 1){   // Turn off mosfet output and ring speaker because battery not detected
    digitalWrite(MOSFETOUT, LOW);
    buzzer(20);
    buzzer(60);
    buzzer(20);
//    Serial_println_s("NOBAT");
    delay(2000);
  }else{      // Turn off mosfet output since no quake and ac voltage detected
//    Serial_println_s("MOSOFF");
    digitalWrite(MOSFETOUT, LOW);
  }
}
