#include <Wire.h>
#include <util/atomic.h>
#include "drive.h"
#ifndef ROVER_I2C_ADDRESS
#define ROVER_I2C_ADDRESS 0x11
#endif
// Pinout supplied by the user on 2026-10-11.
#define PIN_M1_RPWM 10
#define PIN_M1_LPWM 11
#define PIN_M1_REN 12
#define PIN_M1_LEN 13
#define PIN_M2_RPWM 3
#define PIN_M2_LPWM 5
#define PIN_M2_REN 6
#define PIN_M2_LEN 7
#define PIN_M1_ENCODER_A 8
#define PIN_M1_ENCODER_B 9
#define PIN_M2_ENCODER_A 2
#define PIN_M2_ENCODER_B 4
volatile int16_t requested=0;
volatile uint32_t receivedAt=0;
volatile bool received=false;
int16_t applied=0;
long encoderM1Count=0, encoderM2Count=0;
uint8_t prev1=0, prev2=0;
const int8_t encoderTable[16]={0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
void receiveCommand(int count) {
  uint8_t p[4]; int n=0;
  while(Wire.available()) { uint8_t b=Wire.read(); if(n<4) p[n]=b; ++n; }
  int16_t speed;
  if(count!=4 || n!=4 || !rover::decode(p,speed)) { requested=0; received=false; return; }
  requested=speed; receivedAt=millis(); received=true;
}
void motor(uint8_t rpwm,uint8_t lpwm,uint8_t ren,uint8_t len,int16_t speed) {
  // Clear both directions before applying the new direction.
  analogWrite(rpwm,0); analogWrite(lpwm,0);
  digitalWrite(ren,speed ? HIGH:LOW); digitalWrite(len,speed ? HIGH:LOW);
  if(speed>0) analogWrite(rpwm,speed);
  if(speed<0) analogWrite(lpwm,-speed);
}
void setSpeed(int16_t speed) {
  motor(PIN_M1_RPWM,PIN_M1_LPWM,PIN_M1_REN,PIN_M1_LEN,speed);
  motor(PIN_M2_RPWM,PIN_M2_LPWM,PIN_M2_REN,PIN_M2_LEN,speed);
  applied=speed;
}
uint8_t encoder(uint8_t a,uint8_t b) { return (digitalRead(a)<<1)|digitalRead(b); }
void setup() {
  const uint8_t outputs[]={PIN_M1_RPWM,PIN_M1_LPWM,PIN_M1_REN,PIN_M1_LEN,PIN_M2_RPWM,PIN_M2_LPWM,PIN_M2_REN,PIN_M2_LEN};
  for(uint8_t p:outputs) { digitalWrite(p,LOW); pinMode(p,OUTPUT); }
  const uint8_t inputs[]={PIN_M1_ENCODER_A,PIN_M1_ENCODER_B,PIN_M2_ENCODER_A,PIN_M2_ENCODER_B};
  for(uint8_t p:inputs) pinMode(p,INPUT_PULLUP);
  prev1=encoder(PIN_M1_ENCODER_A,PIN_M1_ENCODER_B); prev2=encoder(PIN_M2_ENCODER_A,PIN_M2_ENCODER_B);
  setSpeed(0); Wire.begin(ROVER_I2C_ADDRESS); Wire.onReceive(receiveCommand);
}
void loop() {
  uint8_t a=encoder(PIN_M1_ENCODER_A,PIN_M1_ENCODER_B),b=encoder(PIN_M2_ENCODER_A,PIN_M2_ENCODER_B);
  encoderM1Count+=encoderTable[(prev1<<2)|a]; prev1=a;
  encoderM2Count+=encoderTable[(prev2<<2)|b]; prev2=b;
  int16_t speed; uint32_t last; bool valid;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { speed=requested; last=receivedAt; valid=received; }
  if(!valid || rover::expired(millis(),last)) speed=0;
  if(speed!=applied) setSpeed(speed);
}
