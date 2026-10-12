import { Component, effect, inject, signal } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { DriveService } from './drive.service';
import { HotasService } from './hotas.service';
@Component({selector:'app-root',imports:[FormsModule],templateUrl:'./app.html',styleUrl:'./app.css'})
export class App {
  readonly drive=inject(DriveService);
  readonly hotas=inject(HotasService);
  url='ws://192.168.100.50:81/';
  linear=1; angular=0; power=80;
  readonly pressing=signal(false);
  readonly arrows=signal<string[]>([]);
  private pointer:number|null=null;
  constructor(){
    window.addEventListener('blur',()=>this.release());
    window.addEventListener('pointerup',()=>this.releasePointer());
    window.addEventListener('keydown',e=>this.keyDown(e));
    window.addEventListener('keyup',e=>this.keyUp(e));
    document.addEventListener('focusin',e=>{if(this.editing(e.target))this.release();});
    document.addEventListener('visibilitychange',()=>{if(document.hidden)this.release();});
    effect(()=>{if(this.drive.state()!=='Conectado')this.release();});
  }
  update(){
    if(this.hotas.enabled())return;
    const keys=this.arrows();
    if(keys.length){
      this.drive.setCommand(Number(keys.includes('ArrowUp'))-Number(keys.includes('ArrowDown')),
        Number(keys.includes('ArrowLeft'))-Number(keys.includes('ArrowRight')),this.power);
    }else this.drive.setCommand(this.linear,this.angular,this.power);
  }
  private editing(target:EventTarget|null):boolean {
    return target instanceof HTMLElement &&
      !!target.closest('input, textarea, select, [contenteditable]:not([contenteditable="false"])');
  }
  private keyDown(e:KeyboardEvent){
    if(e.key==='Escape'){e.preventDefault();this.release();return;}
    if(!['ArrowUp','ArrowDown','ArrowLeft','ArrowRight'].includes(e.key) || this.editing(e.target))return;
    e.preventDefault();
    if(e.repeat || this.hotas.enabled() || this.drive.state()!=='Conectado' || this.pointer!==null || this.arrows().includes(e.key))return;
    this.arrows.update(keys=>[...keys,e.key]);this.update();this.drive.hold();
  }
  private keyUp(e:KeyboardEvent){
    if(!this.arrows().includes(e.key))return;
    e.preventDefault();this.arrows.update(keys=>keys.filter(key=>key!==e.key));
    if(this.arrows().length)this.update();else this.drive.stop();
  }
  releasePointer(){if(this.pointer!==null)this.release();}
  press(e:PointerEvent){
    if(this.hotas.enabled() || this.pointer!==null || this.arrows().length || e.button!==0)return;
    e.preventDefault();this.pointer=e.pointerId;
    (e.currentTarget as HTMLElement).setPointerCapture(e.pointerId);
    this.update();this.pressing.set(true);this.drive.hold();
  }
  release(){
    this.hotas.disarm();
    this.pointer=null;this.pressing.set(false);this.arrows.set([]);this.drive.stop();
  }
  armHotas(){this.release();this.hotas.arm();}
  preset(linear:number,angular:number){this.linear=linear;this.angular=angular;this.update();}
}
