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
// Unified wiring confirmed in README.md on 2026-10-12.
const minis=['left','right'].map(side=>readFileSync(new URL('arduino/mini_'+side+'/mini_'+side+'.ino',root),'utf8'));
const pinout={PIN_M1_RPWM:3,PIN_M1_LPWM:5,PIN_M1_REN:7,PIN_M1_LEN:6,PIN_M1_ENCODER_A:2,PIN_M1_ENCODER_B:4,PIN_M2_RPWM:10,PIN_M2_LPWM:11,PIN_M2_REN:12,PIN_M2_LEN:13,PIN_M2_ENCODER_A:8,PIN_M2_ENCODER_B:9};
for(const mini of minis)for(const [pin,value] of Object.entries(pinout))assert.match(mini,new RegExp(`#define ${pin} ${value}(?:\\s|$)`));
console.log('PASS: drive safety and unified pinout');

let appSource=readFileSync(new URL('../src/app/app.ts',import.meta.url),'utf8');
appSource=appSource.replace(/^import .*;\r?\n/gm,'');
let stops=0,disarms=0,holds=0;
const fakeDrive={telemetry:()=>null,state:()=> 'Conectado',stop:()=>stops++,hold:()=>holds++,setCommand:()=>{}};
const fakeHotas={enabled:()=>false,disarm:()=>disarms++};
const fakeSignal=value=>{const result=()=>value;result.set=next=>value=next;result.update=fn=>value=fn(value);return result;};
const appContext={exports:{},Component:()=>c=>c,FormsModule:{},DriveService:fakeDrive,HotasService:fakeHotas,inject:value=>value,signal:fakeSignal,computed:fn=>fn,effect:()=>{},window:{addEventListener:()=>{}},document:{addEventListener:()=>{}},HTMLElement:class{}};
vm.runInNewContext(transpileModule(appSource,{compilerOptions:{module:ModuleKind.CommonJS,experimentalDecorators:true}}).outputText,appContext);
const app=new appContext.exports.App();
const key={key:'ArrowUp',target:null,repeat:false,preventDefault:()=>{}};
app.keyDown(key);assert.equal(holds,1);
app.changeView('joystick');assert.equal(stops,1);assert.equal(disarms,1);assert.equal(app.arrows().length,0);
app.keyDown(key);assert.equal(holds,1,'Joystick identification must not drive');
app.changeView('drive');app.keyDown(key);assert.equal(holds,2);
app.changeMode();assert.equal(app.arrows().length,0);assert.equal(stops,3);assert.equal(app.expert(),true);
console.log('PASS: screen and mode changes stop motion; joystick identification ignores driving keys');
