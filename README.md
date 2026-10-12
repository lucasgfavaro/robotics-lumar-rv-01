# Robot Rover

Implementación independiente: Angular en el navegador de la PC → WebSocket ESP8266 → I2C → Pro Mini y dos BTS7960 por lateral. El sketch original se conserva intacto. No requiere copiar código desde el chat.

## Puesta en marcha

1. En `firmware/esp8266/config.h`, completá `WIFI_SSID` y `WIFI_PASSWORD`. Si falta el archivo, copiá `config.example.h`. No publiques credenciales.
2. Desde esta carpeta, con PlatformIO instalado, compilá con `pio run`. Cargá cada placa con `pio run -e mini_left -t upload --upload-port COMx` y `pio run -e esp8266 -t upload --upload-port COMy`. Reemplazá los puertos por los reales. `mini_right` configura la futura placa en 0x11.
3. La configuración de Mini es ATmega328P **5 V / 16 MHz**. Para una Mini 3,3 V / 8 MHz cambiá el board a `pro8MHzatmega328`. La ESP usa `nodemcuv2`; ajustalo si tu placa es otra.
4. La ESP obtiene dirección por DHCP; buscala en el router. Podés reservar `192.168.100.50` allí. No se fuerza esa IP en el firmware. Consultá `http://IP/health`.
5. En `ui`, con Node 22.22.3 o una versión compatible con Angular 22: `npm ci`, `npm start`. Abrí `http://localhost:4200`, ingresá `ws://IP:81/` y conectá.
6. Ajustá avance, giro y potencia. Los botones Adelante/Atrás/Izquierda/Derecha eligen valores: **solo se mueve al mantener apretado MANTENER PARA MOVER**. Soltar o DETENER envía cero. La conexión no reanuda movimiento automáticamente.

Para compilar la UI con el Node actual de esta PC sin cambiarlo globalmente:

```powershell
npm exec --yes --package=node@22.22.3 -- node node_modules/@angular/cli/bin/ng.js build
npm exec --yes --package=node@22.22.3 -- node node_modules/@angular/cli/bin/ng.js serve
```

## Cableado actualizado según lista del usuario (2026-10-11)

| Función | Motor 1 | Motor 2 |
|---|---:|---:|
| RPWM | D10 | D3 |
| LPWM | D11 | D5 |
| R_EN | D12 | D6 |
| L_EN | D13 | D7 |
| Encoder A | D8 | D2 |
| Encoder B | D9 | D4 |

D13 es L_EN: no se usa como LED. I2C en Mini: A4 SDA, A5 SCL. ESP: GPIO4/D2 SDA, GPIO5/D1 SCL, configurables en config.h. Masa común. Para Mini de 5 V, usá adaptación de nivel I2C entre 3,3 V y 5 V; no conectes pull-ups de 5 V directamente a la ESP.

## Protocolo v1

WebSocket TCP 81, texto JSON, comando cada 100 ms mientras el control está apretado:

```json
{"type":"drive","linear":0.8,"angular":0.2,"power":200,"seq":152}
```

`linear` y `angular`: -1..1; `power`: entero 0..255; `seq`: uint32 creciente por conexión. `linear > 0` es avance; `angular > 0` es giro izquierdo. Cero en ambos detiene. Los valores son **esfuerzo normalizado**, todavía no velocidad física ni `/cmd_vel` en m/s y rad/s.

Mixer: `left=linear-angular`, `right=linear+angular`; ambos se dividen por `max(1,abs(left),abs(right))`, se multiplican por power y se redondean. Ejemplos con power 200: (1,0) → (200,200); (-1,0) → (-200,-200); (0,1) → (-200,200); (1,1) → (0,200).

I2C: izquierda 0x10 obligatoria, derecha 0x11 opcional. Paquete de cuatro bytes `[version=1, speedLo, speedHi, xor]`, signed int16 little endian, rango -255..255; XOR de los tres primeros bytes. Cada Mini aplica el mismo signed speed a sus dos motores: positivo usa RPWM, negativo LPWM, cero apaga PWM y habilitaciones. Un paquete inválido detiene y no refresca el watchdog.

La ESP envía estado `type=status` cada 200 ms: velocidades PWM, presencia por ACK I2C, actividad, controlador e IP. `/health` devuelve el mismo estado y uptime; HTTP 200 significa que el gateway responde, **no** que todos los motores están disponibles. Presencia por ACK no comprueba que una rueda se mueva.

## Detención y límites

- Watchdog de comandos válidos de 300 ms en ESP. Ping/pong no prolonga movimiento. Desconexión del dueño o pérdida de Wi-Fi envía cero; los comandos duplicados/viejos no renuevan el timeout.
- Watchdog I2C independiente de 300 ms en Mini. El gateway refresca el estado I2C cada 100 ms; al vencer el control de red refresca únicamente cero. Si falla el gateway, la Mini detiene por su propio timeout.
- Un controlador activo a la vez; un segundo recibe `busy`. La propiedad vence al dejar de enviar drive, incluida una UI quieta. Los mensajes de otros clientes no alteran al dueño.
- La UI detiene al perder foco, ocultarse, cancelar el puntero o soltar; corta la conexión si no recibe estado en 600 ms o detecta envío pendiente. No reconecta automáticamente.
- La UI espera un primer estado válido antes de habilitar movimiento y comprueba su antigüedad al apretar. Un estado mal formado corta la conexión.
- Si falta izquierda se detiene también derecha; si falta derecha, izquierda puede operar para esta etapa de montaje.
- La detención elimina la potencia: no garantiza frenado mecánico inmediato. Los 300 ms son el timeout de software; la ejecución del loop y las transacciones añaden latencia.
- Encoders conservados con lectura por polling como el original; aún no hay control de velocidad cerrado ni telemetría de encoders. A alta velocidad el polling puede perder pulsos.
- No hay autenticación/TLS; esta versión está destinada a la red local de pruebas. La arquitectura separa UI/transporte/mixer/actuadores para poder añadir un adaptador ROS 2 después. ROS 2 aún no está implementado.

## Verificación

UI: `npm run build`; pruebas de seguridad: `node tests/safety.mjs` desde ui. Firmwares: `pio run -e esp8266 -e mini_left -e mini_right`.

Recuperación del 7 de octubre de 2026: se revisó el código sin compilar ni ejecutar pruebas, por indicación del usuario. Ver `RECUPERACION.md` para el inventario y los cambios.

Prueba física pendiente: con ruedas levantadas y potencia baja, verificar avance/reversa de ambos motores y sentido de giro; luego cortar WebSocket, Wi-Fi y gateway por separado, y comprobar detención. Desconectar I2C y comprobar el watchdog de Mini. El montaje especular puede requerir invertir los cables de un motor para que el signo positivo represente avance físico. Confirmá ambas direcciones antes de apoyar el rover.

Referencias de implementación: [compatibilidad Angular](https://angular.dev/reference/versions), [arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets). Notas del proyecto: [planilla](https://docs.google.com/spreadsheets/d/1-8gbIZ-9NJ74Lot5L4k9WtnZ3bQboOgyeRztklJkotk/edit).
