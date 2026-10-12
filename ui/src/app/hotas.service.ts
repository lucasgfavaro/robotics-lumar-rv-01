import { DestroyRef, effect, inject, Injectable, signal } from '@angular/core';
import { DriveService } from './drive.service';

interface PadView { index:number; id:string; axes:number[]; buttons:boolean[]; }
@Injectable({providedIn:'root'})
export class HotasService {
  private readonly drive=inject(DriveService);
  readonly devices=signal<PadView[]>([]);
  readonly enabled=signal(false);
  readonly moving=signal(false);
  readonly message=signal('Conectá el X52 y presioná un botón para detectarlo.');
  readonly linear=signal(0);
  readonly angular=signal(0);
  readonly power=signal(20);
  joystick=0; throttle=0; forwardAxis=1; turnAxis=0; throttleAxis=2; enableButton=0;
  invertForward=true; invertTurn=true; invertThrottle=true; deadzone=0.12;
  private identities:string[]=[];
  private ready=false;
  private lastPoll=0;
  constructor(){
    const timer=setInterval(()=>this.poll(),100);
    const blur=()=>this.disarm();
    const hidden=()=>{if(document.hidden)this.disarm();};
    const lost=(e:GamepadEvent)=>{
      if(e.gamepad.index===this.joystick || e.gamepad.index===this.throttle)this.disarm();
    };
    window.addEventListener('blur',blur);
    window.addEventListener('gamepaddisconnected',lost);
    document.addEventListener('visibilitychange',hidden);
    effect(()=>{if(this.drive.state()!=='Conectado' && this.enabled())this.disarm();});
    inject(DestroyRef).onDestroy(()=>{
      clearInterval(timer);this.disarm();
      window.removeEventListener('blur',blur);
      window.removeEventListener('gamepaddisconnected',lost);
      document.removeEventListener('visibilitychange',hidden);
    });
  }
  arm(){
    if(this.drive.state()!=='Conectado')return;
    const pads=this.devices();
    const stick=pads.find(p=>p.index===this.joystick), throttle=pads.find(p=>p.index===this.throttle);
    if(!stick || !throttle){this.message.set('Seleccioná dispositivos detectados.');return;}
    this.drive.stop();this.identities=[stick.id,throttle.id];this.ready=false;
    this.enabled.set(true);this.message.set('Centrar ambos ejes del joystick y soltar el botón de habilitación.');
  }
  disarm(){
    const owned=this.enabled();this.enabled.set(false);this.moving.set(false);this.ready=false;
    if(owned)this.drive.stop();
    this.message.set('HOTAS desactivado. Activá para volver a usarlo.');
  }
  private poll(){
    const now=Date.now();
    if(this.enabled() && this.lastPoll && now-this.lastPoll>250)this.disarm();
    this.lastPoll=now;
    let pads:(Gamepad|null)[];
    try{
      if(!navigator.getGamepads){this.disarm();this.message.set('Este navegador no ofrece acceso a gamepads.');return;}
      pads=Array.from(navigator.getGamepads());
    }catch{
      this.disarm();this.message.set('El navegador bloqueó el acceso al HOTAS. Abrí la UI en localhost.');return;
    }
    this.devices.set(pads.filter((p):p is Gamepad=>!!p && p.connected).map(p=>({index:p.index,id:p.id,axes:Array.from(p.axes),buttons:p.buttons.map(b=>b.pressed)})));
    const stick=pads[this.joystick], throttle=pads[this.throttle];
    const axis=stick?.axes[this.forwardAxis], powerAxis=throttle?.axes[this.throttleAxis];
    const turnAxis=stick?.axes[this.turnAxis];
    const button=stick?.buttons[this.enableButton];
    if(!stick?.connected || !throttle?.connected || !button ||
      !Number.isFinite(axis) || !Number.isFinite(turnAxis) || !Number.isFinite(powerAxis) ||
      !Number.isFinite(this.deadzone) || this.deadzone<0 || this.deadzone>=1 ||
      this.forwardAxis===this.turnAxis ||
      (this.joystick===this.throttle && (this.forwardAxis===this.throttleAxis || this.turnAxis===this.throttleAxis))){
      this.linear.set(0);this.angular.set(0);this.power.set(20);
      if(this.enabled())this.disarm();
      this.message.set('Verificá dispositivos, ejes y botón. Si no aparece el X52, presioná un botón.');return;
    }
    const raw=Math.max(-1,Math.min(1,axis!))*(this.invertForward?-1:1);
    const linear=Math.abs(raw)<=this.deadzone?0:Math.sign(raw)*(Math.abs(raw)-this.deadzone)/(1-this.deadzone);
    const turnRaw=Math.max(-1,Math.min(1,turnAxis!))*(this.invertTurn?-1:1);
    const angular=Math.abs(turnRaw)<=this.deadzone?0:Math.sign(turnRaw)*(Math.abs(turnRaw)-this.deadzone)/(1-this.deadzone);
    const throttleValue=Math.max(-1,Math.min(1,powerAxis!))*(this.invertThrottle?-1:1);
    const power=Math.round(20+(throttleValue+1)/2*235);
    this.linear.set(linear);this.angular.set(angular);this.power.set(power);
    if(!this.enabled())return;
    if(document.hidden || !document.hasFocus() || this.drive.state()!=='Conectado' ||
      stick.id!==this.identities[0] || throttle.id!==this.identities[1]){this.disarm();return;}
    if(!this.ready){
      if(!button.pressed && linear===0 && angular===0){this.ready=true;this.message.set('Listo. Mantené el botón elegido para mover.');}
      return;
    }
    if(!button.pressed){
      this.drive.stop();this.moving.set(false);this.message.set('Detenido · botón de habilitación suelto');return;
    }
    this.drive.setCommand(linear,angular,power);
    this.drive.hold(250);this.moving.set(linear!==0 || angular!==0);
    this.message.set(linear===0 && angular===0?'Detenido · joystick centrado':'Control HOTAS activo');
  }
}
