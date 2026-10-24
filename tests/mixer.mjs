import {readFileSync} from 'node:fs';
import {strict as assert} from 'node:assert';
const source=readFileSync(new URL('../arduino/esp8266/drive.h',import.meta.url),'utf8');
let body=source.split('int16_t &right) {')[1].split('// Version,')[0].trim().replace(/}$/, '');
body=body.replace(/\b(?:float|bool) /g,'let ').replace(/(\d+(?:\.\d+)?)f\b/g,'$1').replace(/fmaxf/g,'Math.max').replace(/fabsf/g,'Math.abs').replace(/lroundf/g,'Math.round').replace(/int16_t\(/g,'Number(');
const mix=new Function('linear','angular','power','let left,right;'+body+';return [left||0,right||0];');
assert.deepEqual(mix(1,0,100),[100,100]);
assert.deepEqual(mix(1,1,100),[0,100]);
assert.deepEqual(mix(0,1,100),[-100,100]);
assert.deepEqual(mix(-1,1,100),[-100,0]);
assert.equal(mix(1,.2,100)[0],51);
for(let l=-10;l<=10;l++)for(let a=-10;a<=10;a++)for(const p of [0,20,80,255]){
 const out=mix(l/10,a/10,p);
 assert.ok(out.every(x=>Number.isInteger(x)&&Math.abs(x)<=p));
 const mirror=mix(l/10,-a/10,p);
 assert.ok(out[0]===mirror[1]&&out[1]===mirror[0]);
}
console.log('PASS: actual firmware mixer bounds, yaw, pivot, reverse and stronger steering');
