#pragma once
#include <stdint.h>
#include <math.h>
namespace rover {
constexpr uint32_t TIMEOUT_MS = 300;
constexpr uint8_t PACKET_VERSION = 1;
inline bool expired(uint32_t now, uint32_t last) { return uint32_t(now-last) >= TIMEOUT_MS; }
inline void mix(float linear, float angular, uint8_t power, int16_t &left, int16_t &right) {
  // Preserve pivot turns; while moving, full steering stops the inner side.
  float turn=fabsf(angular);
  float shaped=turn*(3.0f-3.0f*turn+turn*turn);
  float travel=fabsf(linear);
  float outer=fmaxf(travel,turn);
  float inner=travel*(1.0f-shaped);
  float l,r;
  if(travel<0.001f){l=-angular;r=angular;}
  else {
    float direction=linear>0 ? 1.0f : -1.0f;
    // Positive angular always means positive yaw, including in reverse.
    bool leftInner=(angular*linear)>=0;
    l=direction*(leftInner ? inner : outer);
    r=direction*(leftInner ? outer : inner);
  }
  float scale=fmaxf(1.0f, fmaxf(fabsf(l), fabsf(r)));
  left=int16_t(lroundf(l/scale*power)); right=int16_t(lroundf(r/scale*power));
}
// Version, signed int16 little endian, XOR checksum.
inline void encode(int16_t speed, uint8_t *p) {
  p[0]=PACKET_VERSION; p[1]=uint16_t(speed)&255; p[2]=uint16_t(speed)>>8; p[3]=p[0]^p[1]^p[2];
}
inline bool decode(const uint8_t *p, int16_t &speed) {
  if(p[0]!=PACKET_VERSION || p[3]!=(p[0]^p[1]^p[2])) return false;
  uint16_t raw=uint16_t(p[1]) | (uint16_t(p[2])<<8);
  int32_t value=raw>=32768 ? int32_t(raw)-65536 : raw;
  if(value < -255 || value > 255) return false;
  speed=int16_t(value); return true;
}
}
