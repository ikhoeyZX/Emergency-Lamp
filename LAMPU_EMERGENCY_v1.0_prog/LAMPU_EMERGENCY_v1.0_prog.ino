/*********
//
// Lampu Emergency v1.0, (c) 2023 Undens
//
**********/
#include <ESP8266WiFi.h>


/*********
//
// PIN
//
**********/
const uint8_t BAT1_REY=2;
const uint8_t BAT2_REY=15;
const uint8_t RELAY_PWR=4;
//const uint8_t SHORT_DET=15;
const uint8_t V5_ON=5;
const uint8_t VIB_SEN=14;
const uint8_t PLEX_A=0;
const uint8_t PLEX_B=12;
const uint8_t DIGITAL=13;
const uint8_t MOSFETOUT=3; // PIN LED_OUT
const uint8_t LED_MANUAL=16;
volatile bool adagempa = LOW;
uint8_t cek5v=0;
bool baterai1penuh = LOW; 
bool baterai2penuh = LOW; 

/*********
//
// Vibration sensor
//
**********/
void IRAM_ATTR GEMPA() {
  digitalWrite(MOSFETOUT, HIGH);
  adagempa = HIGH;
}

/*********
//
// Speaker
//
**********/
void Putarbuzzer(uint durasi){
    digitalWrite(PLEX_B, LOW);
    pinMode(DIGITAL, OUTPUT);
    delay(10); 
    digitalWrite(DIGITAL,HIGH);
    delay(durasi);
    digitalWrite(DIGITAL,LOW);
    delay(durasi/2);
    pinMode(DIGITAL, INPUT_PULLUP);
    delay(10);
    digitalWrite(PLEX_B, HIGH);
} 

/*********
//
// Read battery
//
**********/
float baterai(bool plex){
  float a=0;
  float b;
  digitalWrite(PLEX_A, plex);
  delay(10);
  for(b=0;b<3;b++){
    a = a + (analogRead(A0) * (3.3 / 330));
    delay(50);
  }
  a = a/3;
  return a;
}

void bateraistatus(float pinbat, uint8_t relaypin, bool batid, bool psu){
  bool vin;
  
  if(psu == LOW){
    cek5v=0;
    digitalWrite(relaypin, HIGH);
    batid = LOW;
  }else{
    if(pinbat >= 4.1){
      digitalWrite(relaypin, HIGH);
      batid = HIGH;
    }else if(pinbat <= 4.09 &&pinbat >= 3.8 && batid == HIGH){ 
      digitalWrite(relaypin, HIGH);
    }else if(pinbat <= 4.09 &&pinbat >= 3.8 && batid == LOW){
      
      digitalWrite(relaypin, LOW);
    }else if(pinbat <= 3.79 &&pinbat >= 2){
      batid = LOW;
      vin = cekpsu(V5_ON);
      if(vin == LOW && cek5v < 5){
        digitalWrite(relaypin, LOW);
        digitalWrite(RELAY_PWR, LOW);
        cek5v = cek5v+1;
      }else if(vin == HIGH){
        cek5v=0;
      }else{
        Putarbuzzer(5);
        Putarbuzzer(10);
        Putarbuzzer(3);
        digitalWrite(RELAY_PWR, HIGH);
        digitalWrite(relaypin, HIGH);
        delay(1000);
      }
    }else if(pinbat <= 2){
      digitalWrite(relaypin, HIGH);
    }else{

    }
  }
}

/*********
//
// AC VOLTAGE / Adaptor (ON/OFF)
// 
// pindigital=
//    DIGITAL = 220VAC 
//    V5_ON = ADAPTOR
//
**********/
bool cekpsu(uint8_t pindigital){
  bool a;
  digitalWrite(PLEX_B, HIGH);
  pinMode(DIGITAL, INPUT_PULLUP);
  delay(10);
  a = digitalRead(pindigital);
  if(a == LOW){
    return HIGH;
  }else{
    return LOW;
  }
}

/*********
//
// Setup
//
**********/
void setup() {
//  Serial.begin(74880);
  WiFi.mode(WIFI_OFF);

  pinMode(BAT1_REY, OUTPUT);
  pinMode(BAT2_REY, OUTPUT);
  pinMode(RELAY_PWR, OUTPUT);
  pinMode(V5_ON, INPUT_PULLUP);
  pinMode(PLEX_A, OUTPUT);
  pinMode(PLEX_B, OUTPUT);
  pinMode(MOSFETOUT, OUTPUT);
  pinMode(LED_MANUAL, INPUT);
  digitalWrite(PLEX_B,HIGH);
  digitalWrite(BAT1_REY,LOW);
  digitalWrite(BAT2_REY,HIGH); 
  digitalWrite(MOSFETOUT,LOW);
  Putarbuzzer(10);
  Putarbuzzer(20);
  Putarbuzzer(10);
  attachInterrupt(digitalPinToInterrupt(VIB_SEN), GEMPA, FALLING);
  digitalWrite(BAT1_REY,HIGH);
}


/*********
//
// Main
//
**********/
void loop() {
  float a,b;
  bool ac,V5OFF;

  ac = cekpsu(DIGITAL);

  bool tombol = digitalRead(LED_MANUAL);
  if(adagempa == HIGH){
    Putarbuzzer(150);
    Putarbuzzer(10);
    Putarbuzzer(120);
    Putarbuzzer(60);
    adagempa = LOW;
  }else{  }

  a = baterai(HIGH);
  bateraistatus(a, BAT1_REY, baterai1penuh, ac);
  b = baterai(LOW);
  bateraistatus(b, BAT2_REY, baterai2penuh, ac);
  // Serial.print(F("Bat 1: "));
  // Serial.println(a);
  // Serial.print(F("Bat 2: "));
  // Serial.println(b);

  if(a >= 4.19 && b >= 4.19){
    digitalWrite(RELAY_PWR, HIGH);
  }else if(a <15&& b >= 4.19){
   digitalWrite(RELAY_PWR, HIGH);
  }else if(b <1 && a >= 4.19){
    digitalWrite(RELAY_PWR, HIGH);
  }else{ }

  if(a <= 3.41 && b <= 3.41 && ac == LOW){
    digitalWrite(MOSFETOUT, LOW);
    digitalWrite(RELAY_PWR, HIGH);
    delay(6000);
  }else if(a <= 3.42 && b <= 3.42 && ac == LOW){ 
    digitalWrite(MOSFETOUT, LOW);
    digitalWrite(RELAY_PWR, HIGH);
    Putarbuzzer(110);
    Putarbuzzer(80);
    Putarbuzzer(110);
    delay(10000);
  }else{
    if(tombol == LOW){
      digitalWrite(MOSFETOUT, HIGH);
      delay(2000);
    }else{ 
      if(ac == LOW){
        digitalWrite(MOSFETOUT, HIGH);
        digitalWrite(RELAY_PWR, HIGH);
      }else{
        digitalWrite(MOSFETOUT, LOW);
      }
      delay(1500);
    }
  }
}
