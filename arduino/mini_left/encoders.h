#pragma once
#include <stdint.h>
namespace rover {
constexpr uint8_t ENCODER_PACKET_SIZE=14;
inline void put32(uint8_t *p,uint32_t value){for(uint8_t i=0;i<4;++i)p[i]=uint8_t(value>>(8*i));}
inline uint32_t get32(const uint8_t *p){uint32_t value=0;for(uint8_t i=0;i<4;++i)value|=uint32_t(p[i])<<(8*i);return value;}
inline void encodeEncoders(uint8_t *p,uint32_t first,uint32_t second,uint32_t uptime){
  p[0]=1;put32(p+1,first);put32(p+5,second);put32(p+9,uptime);
  p[13]=0;for(uint8_t i=0;i<13;++i)p[13]^=p[i];
}
inline bool decodeEncoders(const uint8_t *p,uint32_t &first,uint32_t &second,uint32_t &uptime){
  uint8_t sum=0;for(uint8_t i=0;i<13;++i)sum^=p[i];
  if(p[0]!=1 || sum!=p[13])return false;
  first=get32(p+1);second=get32(p+5);uptime=get32(p+9);return true;
}
inline int64_t signedCount(uint32_t value){return value<=0x7fffffffUL?int64_t(value):int64_t(value)-0x100000000LL;}
}
