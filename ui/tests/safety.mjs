import { readFileSync } from 'node:fs';
import { strict as assert } from 'node:assert';
import { transpileModule, ModuleKind } from 'typescript';
import vm from 'node:vm';
const events={}, docEvents={}, intervals=new Set();let now=1000;
class Socket {
 static OPEN=1;readyState=1;bufferedAmount=0;sent=[];
 constructor(){Socket.last=this;}
 send(s){this.sent.push(JSON.parse(s));}
 close(){this.readyState=3;this.onclose?.();}
}
let source=readFileSync(new URL('../src/app/drive.service.ts',import.meta.url),'utf8');
source=source.replace(/import .*from '@angular\/core';/, 'const Injectable = () => (c: any) => c; const signal = (v: any) => { const s: any = () => v; s.set = (n: any) => v=n; return s; };');
const code=transpileModule(source,{compilerOptions:{module:ModuleKind.CommonJS,experimentalDecorators:true}}).outputText;
const context={exports:{},WebSocket:Socket,window:{addEventListener:(k,f)=>events[k]=f},document:{hidden:false,addEventListener:(k,f)=>docEvents[k]=f},Date:{now:()=>now},setInterval:f=>{intervals.add(f);return f;},clearInterval:f=>intervals.delete(f)};
vm.runInNewContext(code,context);const service=new context.exports.DriveService();
const status=()=>Socket.last.onmessage({data:JSON.stringify({type:'status',active:false,left:0,right:0,leftPresent:true,rightPresent:false})});
const open=()=>{Socket.last.onopen();status();};
service.connect('ws://test');let socket=Socket.last;socket.onopen();
assert.equal(socket.sent.at(-1).linear,0);
service.setCommand(.8,.2,120);service.hold();assert.equal(socket.sent.at(-1).linear,0);
assert.equal(service.state(),'Esperando estado');status();
service.setCommand(.8,.2,120);service.hold();assert.equal(socket.sent.at(-1).angular,.2);
now+=100;for(const tick of intervals)tick();assert.equal(socket.sent.at(-1).linear,.8);
events.blur();assert.equal(socket.sent.at(-1).linear,0);assert.equal(socket.sent.at(-1).angular,0);
const afterRelease=socket.sent.length;
socket.bufferedAmount=1;service.stop();service.stop();
assert.equal(socket.sent.length,afterRelease);assert.equal(socket.readyState,1);
socket.bufferedAmount=0;
const count=socket.sent.length;for(const tick of intervals)tick();assert.equal(socket.sent.length,count);
service.hold();context.document.hidden=true;docEvents.visibilitychange();assert.equal(socket.sent.at(-1).linear,0);
service.hold();now+=700;for(const tick of intervals)tick();assert.equal(socket.readyState,3);assert.equal(service.state(),'Sin respuesta');
service.connect('ws://test');socket=Socket.last;open();assert.equal(socket.sent.at(-1).linear,0);
service.hold();socket.bufferedAmount=1;
const pendingCount=socket.sent.length;
for(const tick of intervals)tick();assert.equal(socket.readyState,1);assert.equal(socket.sent.length,pendingCount);
service.stop();assert.equal(socket.sent.at(-1).linear,0);assert.equal(socket.readyState,1);
now+=100;socket.bufferedAmount=0;for(const tick of intervals)tick();assert.equal(socket.readyState,1);
service.hold();socket.bufferedAmount=1;now+=200;status();
for(const tick of intervals)tick();assert.equal(socket.readyState,3);assert.equal(service.state(),'Envío trabado: conexión cerrada');
service.connect('ws://test');socket=Socket.last;open();socket.onmessage({data:'{"type":"error","reason":"busy"}'});assert.equal(service.state(),'Otro controlador está activo');
service.connect('ws://test');socket=Socket.last;open();
socket.onmessage({data:'{"type":"status","active":false,"left":999,"right":0,"leftPresent":true,"rightPresent":false}'});
assert.equal(socket.readyState,3);assert.equal(service.state(),'Respuesta inválida');
service.connect('ws://test');socket=Socket.last;open();now+=601;service.hold();
assert.equal(socket.readyState,3);assert.equal(service.state(),'Sin respuesta');
const root=new URL('../../',import.meta.url);
const original=readFileSync(new URL('./fixtures/controlador_arduino_mini_prueba.ino',import.meta.url),'utf8');
const mini=readFileSync(new URL('firmware/mini/mini.ino',root),'utf8');
// Wiring revised by the user on 2026-10-11; retain the historical fixture.
const currentPinout=original.replace('#define PIN_M1_RPWM 11','#define PIN_M1_RPWM 10')
 .replace('#define PIN_M1_LPWM 10','#define PIN_M1_LPWM 11')
 .replace(/#define PIN_M2_REN\s+4/, '#define PIN_M2_REN 6')
 .replace(/#define PIN_M2_LEN\s+6/, '#define PIN_M2_LEN 7')
 .replace(/#define PIN_M2_ENCODER_B\s+7/, '#define PIN_M2_ENCODER_B 4');
for(const match of currentPinout.matchAll(/#define (PIN_\w+)\s+(\d+)/g))assert.match(mini,new RegExp(`#define ${match[1]} ${match[2]}(?:\\s|$)`));
console.log('PASS: heartbeat, release, blur, hidden tab, stale telemetry, initial status, invalid status, stale hold, reconnect, backpressure, busy controller, exact pinout');
