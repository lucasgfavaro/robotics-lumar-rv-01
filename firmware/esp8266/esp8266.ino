#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "config.h"
#include "../common/drive.h"
ESP8266WebServer http(80);
WebSocketsServer ws(81);
int owner=-1;
uint32_t lastDrive=0,lastBus=0,lastStatus=0,lastSeq=0;
bool hasSeq=false,active=false,leftPresent=false,rightPresent=false;
int16_t leftSpeed=0,rightSpeed=0;
bool sendSpeed(uint8_t address,int16_t speed) {
  uint8_t packet[4]; rover::encode(speed,packet);
  Wire.beginTransmission(address); Wire.write(packet,4);
  return Wire.endTransmission()==0;
}
void bus() {
  leftPresent=sendSpeed(0x10,leftSpeed);
  // Check the required side before allowing any motion on the optional side.
  if(!leftPresent) { active=false; leftSpeed=rightSpeed=0; }
  rightPresent=sendSpeed(0x11,rightSpeed);
  lastBus=millis();
}
void stop() { active=false; leftSpeed=rightSpeed=0; bus(); }
String status() {
  StaticJsonDocument<384> d;
  d["type"]="status"; d["active"]=active; d["watchdogMs"]=300;
  d["left"]=leftSpeed; d["right"]=rightSpeed;
  d["leftPresent"]=leftPresent; d["rightPresent"]=rightPresent;
  d["controller"]=owner; d["uptimeMs"]=millis();
  d["ip"]=WiFi.localIP().toString();
  String out; serializeJson(d,out); return out;
}
void event(uint8_t client,WStype_t type,uint8_t *payload,size_t length) {
  if(type==WStype_DISCONNECTED && owner==client) { stop(); owner=-1; hasSeq=false; }
  if(type!=WStype_TEXT) return;
  // ws.loop() may dispatch after the loop's initial deadline check.
  if(owner>=0 && (WiFi.status()!=WL_CONNECTED || rover::expired(millis(),lastDrive))) {
    stop(); owner=-1; hasSeq=false;
  }
  if(WiFi.status()!=WL_CONNECTED) return;
  if(length>256) return;
  StaticJsonDocument<256> d;
  if(deserializeJson(d,payload,length)) return;
  if(d["type"]!="drive") return;
  if(owner>=0 && owner!=client) { ws.sendTXT(client,"{\"type\":\"error\",\"reason\":\"busy\"}"); return; }
  if(!d["linear"].is<float>() || !d["angular"].is<float>() || !d["power"].is<int>() || !d["seq"].is<uint32_t>()) { if(owner==client) stop(); return; }
  float linear=d["linear"],angular=d["angular"]; int power=d["power"]; uint32_t seq=d["seq"];
  if(!isfinite(linear)||!isfinite(angular)||fabsf(linear)>1||fabsf(angular)>1||power<0||power>255) { if(owner==client) stop(); return; }
  if(hasSeq && (uint32_t(seq-lastSeq)==0 || uint32_t(seq-lastSeq)>=0x80000000UL)) return;
  owner=client; hasSeq=true; lastSeq=seq; lastDrive=millis();
  rover::mix(linear,angular,power,leftSpeed,rightSpeed);
  active=leftSpeed!=0 || rightSpeed!=0; bus();
}
void setup() {
  Serial.begin(115200); Wire.begin(ROVER_SDA,ROVER_SCL); Wire.setClock(100000);
  Wire.setClockStretchLimit(1500); stop();
  WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  ws.begin(); ws.onEvent(event);
  http.on("/health",HTTP_GET,[](){ http.send(200,"application/json",status()); });
  http.begin();
}
void loop() {
  uint32_t now=millis();
  if(owner>=0 && (WiFi.status()!=WL_CONNECTED || rover::expired(now,lastDrive))) { stop(); owner=-1; hasSeq=false; }
  ws.loop(); http.handleClient();
  if(millis()-lastBus>=100) bus();
  if(millis()-lastStatus>=200) { String s=status(); ws.broadcastTXT(s); lastStatus=millis(); }
  yield();
}
