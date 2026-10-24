#pragma once
#include <stdint.h>
namespace traction {
// Provisional values, NOT measured calibration. PWM at command magnitude 40.
// Index 0: negative electrical command; index 1: positive electrical command.
struct Calibration { uint8_t minimum[2]; float fullScalePps[2]; };
inline int16_t clamp(int16_t x,int16_t lo,int16_t hi){return x<lo?lo:(x>hi?hi:x);}
struct Wheel {
  int16_t output=0, correction=0;
  int8_t direction=0;
  uint32_t lastMotion=0;
  void reset(){output=correction=0;direction=0;}
  int16_t update(int16_t command,int32_t ticks,uint32_t elapsed,uint32_t now,const Calibration &cal){
    if(!command){reset();return 0;}
    int8_t sign=command>0?1:-1;
    if(direction!=sign){reset();direction=sign;lastMotion=now;return 0;}
    int16_t magnitude=command*sign;
    uint8_t index=sign>0?1:0;
    // Continuous near zero, full scale stays 255; zero is never lifted.
    int16_t floor=cal.minimum[index];
    int16_t base=magnitude<40 ? (int32_t(magnitude)*floor/40)
      : floor+int32_t(magnitude-40)*(255-floor)/215;
    int16_t desired=0;
    if(ticks)lastMotion=now;
    if(cal.fullScalePps[index]>0 && elapsed>0 && elapsed<=250){
      float measured=(ticks<0 ? -float(ticks) : float(ticks))*1000.0f/elapsed;
      float target=cal.fullScalePps[index]*magnitude/255.0f;
      // Proportional only: no integral windup, no reference to other wheels.
      float error=(target-measured)*0.02f;
      desired=error>20 ? 20 : (error< -20 ? -20 : int16_t(error));
      if(uint32_t(now-lastMotion)>=600 && desired>0)desired=0;
    }
    correction+=clamp(desired-correction,-2,2);
    if(uint32_t(now-lastMotion)>=600 && correction>0)correction=0;
    int16_t target=clamp(base+correction,0,255);
    // Bound increases; decreases and STOP take effect immediately.
    output=target<output ? target : clamp(target,0,output+6);
    return sign*output;
  }
};
}
