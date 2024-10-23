/*********
//
// Lampu Emergency v1.2 REV 5, (c) 2023 - 2024 Undens
// DESIGNED FOR STM8
//
**********/

// #include <sduino.h>
#include <Serial.h>
#define digitalPinToInterrupt(p)  ((p)==(p)) // fix interrupt

/*********
//
// PIN
//
**********/
// DIGITAL
const uint8_t BAT1_REY=2;	// BATTERY TRIPPER
const uint8_t BAT2_REY=4;	// BATTERY TRIPPER
const uint8_t BUZZER=5;
const uint8_t V5_ON=7;		// ADAPTOR DETECT
const uint8_t AC_DET=8;		// ZERO CROSS
const uint8_t VIB_SEN=PA2;	// TILT SENSOR
const uint8_t MOSFETOUT=13; // PIN LED_OUT

// ANALOG
const uint8_t LDR=PC4;		// LIGHT SENSOR
const uint8_t BAT1=PD2;		// READ BATTERY
const uint8_t BAT2=PD3;		// READ BATTERY

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
void Buzzer(uint16_t duration){  // max 65535ms should be enough
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
float baterai(uint8_t pinbat, uint16_t batcal, uint8_t relaypin, bool pilih){  //baca baterai
  float a = 0;
  for(uint8_t b = 0; b < 10; b++){
    a = a + ((analogRead(pinbat) * (3.3 / batcal)));
    delay(2);
  }
  a = a/10;
  
  if(pilih == LOW){
    if(a >= battery_full){
      digitalWrite(relaypin, HIGH);
    }else if(a <= 1){ 
      digitalWrite(relaypin, HIGH);
    }else if(a <= (battery_full - 0.28) && a > 1){ 
      digitalWrite(relaypin, LOW);
    }
  }else{
    digitalWrite(relaypin, HIGH);
  }

  return a;
}

/*********
//
// Led Buildin
//
**********/
void ledbuildin(){
  digitalWrite(LED_BUILTIN, LOW);   
  delay(20);                       
  digitalWrite(LED_BUILTIN, HIGH);    
  delay(20);   
}

/*********
//
// Digital Smoothing signal
//
**********/
bool smoothlogic(uint16_t sensorin){
  uint8_t acc = 0;

  for(uint8_t c=0; c<5; c++){
    acc = acc + digitalRead(sensorin);
    delay(2);
  }
  if(acc >3){
    return HIGH;
  }
  
  return LOW;
}

/*********
//
// Interrupt
//
**********/
void ISR(){
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
  pinMode(VIB_SEN, INPUT_PULLUP);
  pinMode(MOSFETOUT, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // ANALOG INPUT PIN: BAT1, BAT2, LDR
  digitalWrite(BAT1_REY,HIGH);
  digitalWrite(BAT2_REY,HIGH);
  digitalWrite(MOSFETOUT,HIGH);

//  Serial_begin(9600); // enable it if you need calibration

  ledbuildin();
  attachInterrupt(digitalPinToInterrupt(VIB_SEN), ISR, FALLING);
  Buzzer(10);
  Buzzer(20);
  Buzzer(10);
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

  if(count > limit){
	 bat1s = baterai(BAT1, bat1cal, BAT1_REY, v5_on);
	 bat2s = baterai(BAT2, bat2cal, BAT2_REY, v5_on);
	 ledbuildin();
	 Serial_print_s("\nBat1: ");
	 Serial_println_f(bat1s);
	 Serial_print_s("Bat2: ");
	 Serial_println_f(bat2s);
	 count = 0; //reset counter
   buzzcall = false;
  }else{
	 count++;
  }

//disable it if you no need system detect AC ON but adaptor not ON  
  if(buzzcall){

  }else if(v5_on == HIGH && ac == LOW && !buzzcall){ // ring buzzer if adaptor not yet on but AC voltage exist
    Buzzer(50);
    Buzzer(5);
    Buzzer(20);
    Buzzer(10);

    buzzcall = true;
  }
 //

/* Remove this comment for calibration

  Serial_print_s("D5v: ");
  Serial_println_i(v5_on);
  Serial_print_s("DAC: ");
  Serial_println_i(ac);
  Serial_print_s("SGMP: ");
  Serial_println_i(quakedet);
  Serial_print_s("SLDR: ");
  Serial_println_u(bacaldr);
  
Remove this comment for calibration */

  if(quakedet == HIGH && bacaldr < LDRtrig ){	// Turn on MOSFET
    digitalWrite(MOSFETOUT, HIGH);
    ledbuildin();
    delay(500);
    quakedet = LOW;
  }else if(quakedet == HIGH){					// Turn off MOSFET
    digitalWrite(MOSFETOUT, LOW);
    ledbuildin();
    quakedet = LOW;
  }


  if(!buzzcall){

  }else if(bat1s <= battery_low && bat2s <= battery_low && ac == HIGH){			// VERY LOW BATTERY
    digitalWrite(MOSFETOUT, LOW);
    delay(10000);
  }else if(bat1s < (battery_low + 0.2) && bat2s <= (battery_low + 0.2) && ac == HIGH && !buzzcall){ 	// LOW BATTERY
    digitalWrite(MOSFETOUT, HIGH);
    Buzzer(110);
    Buzzer(80);
    Buzzer(110);
    buzzcall = true;
  }else if(ac == HIGH && bacaldr < LDRtrig){				// LDR TRIGGER
    digitalWrite(MOSFETOUT, HIGH);
  }else if(bat1s < 1 && bat2s < 1){ 						// NO BATTERY EXIST!
    digitalWrite(MOSFETOUT, LOW);
    Buzzer(20);
    Buzzer(60);
    Buzzer(20);
    delay(1000);
  }else{
    digitalWrite(MOSFETOUT, LOW);
  }
}
