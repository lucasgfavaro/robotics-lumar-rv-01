import { Injectable, signal } from '@angular/core';
export interface DriveCommand { type: 'drive'; linear: number; angular: number; power: number; seq: number; }
export interface EncoderSide { valid:boolean; ageMs?:number; miniUptimeMs?:number; m1Ticks?:number; m2Ticks?:number; m1Pps?:number|null; m2Pps?:number|null; }
export interface RoverStatus { type: 'status'; active: boolean; left: number; right: number; leftPresent: boolean; rightPresent: boolean; encoders?:{left:EncoderSide;right:EncoderSide}; }
@Injectable({providedIn:'root'})
export class DriveService {
  readonly state=signal('Desconectado');
  readonly telemetry=signal<RoverStatus|null>(null);
  private socket?: WebSocket;
  private timer?: ReturnType<typeof setInterval>;
  private seq=0;
  private held=false;
  private inputDeadline?:number;
  private lastStatus=0;
  private pendingSince=0;
  private hasStatus=false;
  private command={linear:0,angular:0,power:80};
  constructor() {
    window.addEventListener('blur',()=>this.stop());
    window.addEventListener('pagehide',()=>this.disconnect());
    document.addEventListener('visibilitychange',()=>{if(document.hidden)this.stop();});
  }
  connect(url:string) {
    this.disconnect(); this.state.set('Conectando');
    try { this.socket=new WebSocket(url); } catch {this.state.set('URL inválida');return;}
    const socket=this.socket;
    socket.onopen=()=>{
      if(this.socket!==socket)return;
      this.seq=0;this.held=false;this.hasStatus=false;this.lastStatus=Date.now();this.state.set('Esperando estado');this.send();
      this.timer=setInterval(()=>{
        if(this.inputDeadline!==undefined && Date.now()>=this.inputDeadline)this.stop();
        if(this.checkPending())return;
        if(Date.now()-this.lastStatus>600){this.disconnect();this.state.set('Sin respuesta');return;}
        if(this.held)this.send();
      },100);
    };
    socket.onmessage=e=>{
      if(this.socket!==socket)return;
      try {
        const d=JSON.parse(e.data);
        if(d?.type==='status'){
          if(!this.validStatus(d)){this.disconnect();this.state.set('Respuesta inválida');return;}
          this.telemetry.set(d);this.lastStatus=Date.now();this.hasStatus=true;this.state.set('Conectado');
        }
        if(d?.type==='error'){this.disconnect();this.state.set(d.reason==='busy'?'Otro controlador está activo':'Comando rechazado');}
      }catch {this.disconnect();this.state.set('Respuesta inválida');}
    };
    socket.onerror=()=>{if(this.socket===socket){this.disconnect();this.state.set('Error de conexión');}};
    socket.onclose=e=>{
      if(this.socket!==socket)return;
      this.disconnect();
      this.state.set(`Conexión cerrada (${e?.code ?? 'sin código'})`);
    };
  }
  setCommand(linear:number,angular:number,power:number) {
    if(![linear,angular,power].every(Number.isFinite))return;
    this.command={linear:Math.max(-1,Math.min(1,linear)),angular:Math.max(-1,Math.min(1,angular)),power:Math.round(Math.max(20,Math.min(255,power)))};
    if(this.held)this.send();
  }
  hold(inputLeaseMs?:number) {
    if(this.socket?.readyState!==WebSocket.OPEN || !this.hasStatus)return;
    if(Date.now()-this.lastStatus>600){this.disconnect();this.state.set('Sin respuesta');return;}
    const wasHeld=this.held;
    this.inputDeadline=inputLeaseMs===undefined?undefined:Date.now()+inputLeaseMs;
    this.held=true;if(!wasHeld)this.send();
  }
  stop() {
    const wasHeld=this.held;
    this.inputDeadline=undefined;
    this.held=false;
    // pointerup, lostpointercapture and blur can report the same release.
    if(wasHeld)this.send();
  }
  disconnect() {
    this.stop();if(this.timer)clearInterval(this.timer);this.timer=undefined;
    const socket=this.socket;this.socket=undefined;socket?.close();
    this.hasStatus=false;this.telemetry.set(null);this.state.set('Desconectado');
  }
  private validStatus(d:unknown):d is RoverStatus {
    if(!d || typeof d!=='object')return false;
    const s=d as RoverStatus;
    return s.type==='status' && typeof s.active==='boolean' &&
      typeof s.leftPresent==='boolean' && typeof s.rightPresent==='boolean' &&
      Number.isInteger(s.left) && Math.abs(s.left)<=255 &&
      Number.isInteger(s.right) && Math.abs(s.right)<=255 &&
      (s.encoders===undefined || (!!s.encoders && this.validEncoders(s.encoders.left) && this.validEncoders(s.encoders.right)));
  }
  private validEncoders(s:EncoderSide):boolean {
    if(!s || typeof s.valid!=='boolean')return false;
    if(!s.valid)return true;
    return Number.isInteger(s.m1Ticks) && Number.isInteger(s.m2Ticks) &&
      typeof s.ageMs==='number' && Number.isFinite(s.ageMs) && s.ageMs>=0 && s.ageMs<600 &&
      [s.m1Pps,s.m2Pps].every(value=>value===null || Number.isFinite(value));
  }
  private send() {
    const socket=this.socket;if(socket?.readyState!==WebSocket.OPEN)return;
    if(this.checkPending())return;
    // Brief buffering is normal. Skip motion updates until it drains,
    // but allow one STOP behind the outstanding command when released.
    if(socket.bufferedAmount>0 && this.held)return;
    if(socket.bufferedAmount===0)this.pendingSince=Date.now();
    const c=this.held?this.command:{...this.command,linear:0,angular:0};
    const packet:DriveCommand={type:'drive',...c,seq:this.seq++>>>0};
    socket.send(JSON.stringify(packet));
  }
  private checkPending():boolean {
    const socket=this.socket;
    if(!socket || socket.bufferedAmount===0)return false;
    if(socket.bufferedAmount<=4096 && Date.now()-this.pendingSince<200)return false;
    this.held=false;this.disconnect();
    this.state.set('Envío trabado: conexión cerrada');return true;
  }
}
