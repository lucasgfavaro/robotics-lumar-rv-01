#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include "config.h"
#include "drive.h"
#include "encoders.h"
ESP8266WebServer http(80);
WebSocketsServer ws(81);
int owner=-1;
uint32_t lastDrive=0,lastBus=0,lastStatus=0,lastSeq=0;
bool hasSeq=false,active=false,leftPresent=false,rightPresent=false;
int16_t leftSpeed=0,rightSpeed=0;
// Right side is mirrored; convert logical speed to its motor polarity.
constexpr int8_t LEFT_DIRECTION=1;
constexpr int8_t RIGHT_DIRECTION=-1;
constexpr int8_t FORWARD_DIRECTION=-1;
constexpr int8_t TURN_DIRECTION=-1;
struct EncoderState {
  bool valid=false,hasRate=false;
  uint32_t first=0,second=0,uptime=0,receivedAt=0;
  float firstPps=0,secondPps=0;
};
EncoderState leftEncoders,rightEncoders;
// Explicit prototypes keep Arduino's preprocessor from placing these before the struct.
void readEncoders(uint8_t address,EncoderState &state);
void encoderStatus(JsonObject out,const EncoderState &state);
uint32_t lastEncoderRead=0;
void readEncoders(uint8_t address,EncoderState &state){
  uint8_t packet[rover::ENCODER_PACKET_SIZE];uint8_t n=0;
  Wire.requestFrom(address,rover::ENCODER_PACKET_SIZE);
  while(Wire.available()){uint8_t value=Wire.read();if(n<sizeof(packet))packet[n]=value;++n;}
  uint32_t first,second,uptime;
  if(n!=sizeof(packet) || !rover::decodeEncoders(packet,first,second,uptime)){
    state.valid=false;state.hasRate=false;return;
  }
  uint32_t elapsed=uptime-state.uptime;
  if(state.valid && elapsed==0)return; // An unchanged snapshot must eventually expire.
  state.hasRate=state.valid && elapsed>0 && elapsed<=2000;
  if(state.hasRate){
    state.firstPps=float(rover::signedCount(first-state.first))*1000.0f/elapsed;
    state.secondPps=float(rover::signedCount(second-state.second))*1000.0f/elapsed;
  }
  state.first=first;state.second=second;state.uptime=uptime;
  state.receivedAt=millis();state.valid=true;
}
void encoderStatus(JsonObject out,const EncoderState &state){
  bool fresh=state.valid && uint32_t(millis()-state.receivedAt)<600;
  out["valid"]=fresh;
  if(fresh){
    out["ageMs"]=uint32_t(millis()-state.receivedAt);
    out["miniUptimeMs"]=state.uptime;
    out["m1Ticks"]=int32_t(rover::signedCount(state.first));
    out["m2Ticks"]=int32_t(rover::signedCount(state.second));
    if(state.hasRate){out["m1Pps"]=state.firstPps;out["m2Pps"]=state.secondPps;}
    else{out["m1Pps"]=nullptr;out["m2Pps"]=nullptr;}
  }
}
bool sendSpeed(uint8_t address,int16_t speed) {
  uint8_t packet[4]; rover::encode(speed,packet);
  Wire.beginTransmission(address); Wire.write(packet,4);
  return Wire.endTransmission()==0;
}
void bus() {
  leftPresent=sendSpeed(0x10,leftSpeed*LEFT_DIRECTION);
  // Check the required side before allowing any motion on the optional side.
  if(!leftPresent) { active=false; leftSpeed=rightSpeed=0; }
  rightPresent=sendSpeed(0x11,rightSpeed*RIGHT_DIRECTION);
  lastBus=millis();
}
void stop() { active=false; leftSpeed=rightSpeed=0; bus(); }
String status() {
  StaticJsonDocument<1536> d;
  d["type"]="status"; d["active"]=active; d["watchdogMs"]=300;
  d["left"]=leftSpeed; d["right"]=rightSpeed;
  d["leftPresent"]=leftPresent; d["rightPresent"]=rightPresent;
  d["controller"]=owner; d["uptimeMs"]=millis();
  d["ip"]=WiFi.localIP().toString();
  JsonObject encoders=d.createNestedObject("encoders");
  encoderStatus(encoders.createNestedObject("left"),leftEncoders);
  encoderStatus(encoders.createNestedObject("right"),rightEncoders);
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
  rover::mix(linear*FORWARD_DIRECTION,angular*TURN_DIRECTION,power,leftSpeed,rightSpeed);
  active=leftSpeed!=0 || rightSpeed!=0; bus();
}
void setup() {
  Serial.begin(115200);
  delay(500); // Allow the serial monitor to settle, as in the working sketch.
  Serial.println();
  Serial.println("ROVER - WebSocket - inicio"); Serial.flush();
  Serial.println("VERSION: giro progresivo - 2026-10-20");
  Serial.println("I2C: izquierda 0x10, derecha 0x11");
  Serial.print("Polaridad izquierda: "); Serial.println(int(LEFT_DIRECTION));
  Serial.print("Polaridad derecha: "); Serial.println(int(RIGHT_DIRECTION));
  Serial.flush();
  Serial.println("Inicializando I2C: SDA GPIO4, SCL GPIO5"); Serial.flush();
  Wire.begin(ROVER_SDA,ROVER_SCL); Wire.setClock(100000);
  Wire.setClockStretchLimit(1500);
  Serial.println("I2C iniciado; enviando STOP a Minis"); Serial.flush();
  stop();
  Serial.println("STOP inicial terminado; configurando WiFi"); Serial.flush();
  WiFi.mode(WIFI_STA);
  // Restore the fixed network configuration from the previously working sketch.
  const IPAddress localIP(192,168,100,50);
  const IPAddress gateway(192,168,100,1);
  const IPAddress subnet(255,255,255,0);
  const IPAddress dns(192,168,100,1);
  if(!WiFi.config(localIP,gateway,subnet,dns)) {
    Serial.println("ERROR configurando IP fija; motores detenidos");
    while(true) { delay(100); }
  }
  Serial.println("Rover: conectando a WiFi...");
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  Serial.println("Iniciando WebSocket puerto 81"); Serial.flush();
  ws.begin(); ws.onEvent(event);
  Serial.println("WebSocket iniciado"); Serial.flush();
  http.on("/health",HTTP_GET,[](){ http.send(200,"application/json",status()); });
  http.begin();
  Serial.println("HTTP iniciado puerto 80; setup terminado"); Serial.flush();
}
void loop() {
  // Report connection transitions without blocking the motor watchdogs.
  static bool wifiConnected=false;
  const bool connected=WiFi.status()==WL_CONNECTED;
  if(connected!=wifiConnected) {
    wifiConnected=connected;
    if(connected) {
      Serial.println("WiFi conectado");
      Serial.print("IP: "); Serial.println(WiFi.localIP());
      Serial.print("Health: http://"); Serial.print(WiFi.localIP()); Serial.println("/health");
      Serial.print("WebSocket: ws://"); Serial.print(WiFi.localIP()); Serial.println(":81/");
    } else {
      Serial.println("WiFi desconectado; esperando reconexion");
    }
  }
  uint32_t now=millis();
  if(owner>=0 && (WiFi.status()!=WL_CONNECTED || rover::expired(now,lastDrive))) { stop(); owner=-1; hasSeq=false; }
  ws.loop(); http.handleClient();
  if(millis()-lastBus>=100) bus();
  if(millis()-lastEncoderRead>=200){
    readEncoders(0x10,leftEncoders);readEncoders(0x11,rightEncoders);lastEncoderRead=millis();
    // Reads do not renew the motion command deadline.
    if(owner>=0 && rover::expired(millis(),lastDrive)){stop();owner=-1;hasSeq=false;}
  }
  if(millis()-lastStatus>=200) { String s=status(); ws.broadcastTXT(s); lastStatus=millis(); }
  yield();
}
