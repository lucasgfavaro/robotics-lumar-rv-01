# LUMAR RV-01

## Contexto para retomar — actualizado 12/10/2026

Proyecto activo: `E:\gitRepos\projects\robotics\LUMAR-RV-01`. Trabajar aquí, no en la copia de la carpeta del chat ni en el backup de Downloads. Este README registra el estado actual.

Arquitectura: Angular en la PC → WebSocket → ESP8266 NodeMCU → I²C → dos Arduino Pro Mini → dos drivers BTS7960 y dos motores por lateral. Son cuatro motores y cuatro encoders. La placa gateway es ESP8266, no ESP32.

Confirmado por el usuario: movimiento y giro funcionan; el problema de potencia de un motor quedó resuelto tras corregir el cableado; los cuatro encoders funcionan y sus signos quedaron corregidos. El encoder del motor 2 derecho no contaba por alimentación invertida y volvió a funcionar tras corregirla. No se identificó un driver defectuoso de manera concluyente.

Restricción de trabajo: el asistente no compila ni carga firmware; el usuario hace las cargas y verificaciones manuales. Las revisiones de archivos no acreditan una compilación ni una prueba automatizada. No publicar contraseñas de Wi-Fi.

## Placas y archivos para cargar

| Placa | Arduino IDE | Configuración |
|---|---|---|
| Mini izquierda | `arduino/mini_left/mini_left.ino` | ATmega328P, 3,3 V / 8 MHz, I²C 0x10 |
| Mini derecha | `arduino/mini_right/mini_right.ino` | ATmega328P, 3,3 V / 8 MHz, I²C 0x11 |
| Gateway | `arduino/esp8266/esp8266.ino` | ESP8266 NodeMCU |

Abrir cada sketch desde su carpeta completa, con sus headers. Las dos Minis comparten el código y pinout; las direcciones distintas están en `board_config.h`. El `#define ROVER_I2C_ADDRESS 0x10` dentro de `#ifndef` en los sketches es un fallback: el header ya define 0x10 o 0x11. Nunca cargar el mismo sketch de lateral en ambas placas.

Única estructura activa: `arduino/`. Editar directamente los sketches de cada placa. Mantener sincronizada la lógica de las Minis, conservando el `board_config.h` propio de cada lateral (dirección I2C y signos de encoder). Cada sketch tiene sus headers locales para Arduino IDE. No hay paso de regeneración.

Serial a 115200 y RESET permiten comprobar placa y versión. Las Minis actuales muestran `Pinout: UNIFICADO 2026-10-12` y `Encoders: avance positivo - 2026-10-12`.

Arduino IDE: instalar **WebSockets de Markus Sattler (Links2004), 2.7.2**, y **ArduinoJson de Benoit Blanchon, 6.21.5**, para la versión actual. `WebSocketsServer.h` pertenece a WebSockets, no a ArduinoWebsockets. `util/atomic.h` requiere seleccionar la placa AVR de la Mini. El proyecto se mantiene exclusivamente para Arduino IDE.

## Cableado definitivo — igual en ambos laterales

| Señal | Motor 1 | Motor 2 |
|---|---|---|
| L_EN | D6 | D13 |
| R_EN | D7 | D12 |
| LPWM | D5 | D11 |
| RPWM | D3 | D10 |
| Encoder A | D2 | D8 |
| Encoder B | D4 | D9 |

No existe un lateral legacy con otro pinout. D13 se usa como enable, no como LED de diagnóstico. Mini: SDA A4, SCL A5. ESP: SDA GPIO4/D2, SCL GPIO5/D1. GND común y señales compatibles con 3,3 V. Identificar las ruedas físicas como M1/M2 según esta tabla.

### Programación sin sacar las Minis del zócalo

Procedimiento probado por el usuario: el buck alimenta la Mini a 3,3 V mientras se conecta el loader USB-serie, con **VCC del loader desconectado**. Conectar GND a GND de la Mini, TX del loader a RX, RX a TX y DTR a la entrada de autorreset del conector de programación. Si el sexto cable es CTS, dejarlo desconectado; identificar etiquetas, no colores. Las señales del loader deben ser de 3,3 V.

GND del loader (USB de la PC) y GND del buck quedan unidos a través de la Mini. Mantener apagada la alimentación de potencia de motores durante la carga. Apagar el buck no lo aísla eléctricamente: alimentar desde VCC del loader requeriría aislar el circuito de alimentación para evitar alimentar otros componentes o la salida del buck.

## Red y diagnóstico de conexión

Firmware con IP fija **192.168.100.50**, máscara 255.255.255.0 y gateway/DNS 192.168.100.1. SSID actual WHYFI; contraseña en config.h, no en documentación. Mantener la configuración local de las copias de ESP correspondiente al método de carga.

- Estado HTTP: `http://192.168.100.50/health`.
- Control WebSocket: `ws://192.168.100.50:81/`.
- Serial informa conexión Wi-Fi e IP efectiva. Iniciar servidores no demuestra que Wi-Fi esté conectado.

Si falla: comprobar primero Serial, después /health y desde PowerShell `Test-NetConnection 192.168.100.50 -Port 81`. La PC puede estar en otra subred si hay una ruta entre routers; no asumir fallo solo por las direcciones. En este montaje hubo un falso contacto de alimentación y también una contraseña equivocada. Revisar alimentación y conectividad antes de modificar la UI.

`leftPresent/rightPresent` indican ACK I²C, no movimiento ni funcionamiento del encoder. Ambas deben estar presentes en el montaje completo. Si ambas Minis tienen la misma dirección, corregir la carga por lateral. HTTP accesible no garantiza puerto 81 accesible.

## UI Angular y HOTAS

Trabajar en `ui`. Angular 22 requiere Node compatible con `ui/package.json` (por ejemplo 22.22.3). Inicio manual desde esa carpeta:

```powershell
npm.cmd ci
npm.cmd exec --yes --package=node@22.22.3 -- node node_modules/@angular/cli/bin/ng.js serve
```

Abrir `http://localhost:4200` y conectar al WebSocket. No hace falta reinstalar dependencias por cada cambio de HTML/CSS. La UI sigue el boceto de draw.io, con modos minimalista/experto y pantalla Joystick independiente. Barra superior: conexión, Conectar/Desconectar, selector de modo y PARAR. Controles y rover a la izquierda; cámara a la derecha, pendiente de transmisión. Adaptada a 3440 × 1440: usa todo el ancho y la altura del navegador, con columna izquierda ampliada y scroll interno cuando hace falta. Las ruedas tienen flechas grandes animadas según encoders y color según PWM informado por lateral; no hay banner de movimiento. Experto agrega conteos, pulsos/s, antigüedad y estado por lateral. LiDAR queda para más adelante.

Flechas: arriba avanza, abajo retrocede, izquierda/derecha giran; permite combinar avance y giro. Esc detiene. El diagrama ilumina las flechas. Los presets seleccionan dirección; el botón de movimiento debe mantenerse presionado. Soltar o perder foco detiene. Los campos editables no deben activar movimiento con flechas.

Potencia seleccionable 20..255: por debajo de 20 el usuario indicó que no avanza. Es un límite de potencia; no garantiza que cada salida mezclada tenga al menos 20. No son m/s.

HOTAS X52: joystick controla avance y giro, throttle potencia. Dispositivos/ejes/botón configurables; valores iniciales avance Y=1, giro por rotación=5, throttle=2, habilitación pinky=5; giro y pinky identificados por el usuario. Después de activar, centrar joystick y soltar botón; luego mantener el botón elegido para mover. Zona muerta inicial 0,12. Configuración no persistida. La pantalla Joystick permite configurar e identificar ejes y botones sin conducir ni conectar el rover; HOTAS se activa desde Conducción. Cambiar pantalla o modo detiene y desactiva HOTAS. Ver `ui/HOTAS.md`.

## Movimiento, telemetría y seguridad

Comando WebSocket: `{"type":"drive","linear":0.8,"angular":0.2,"power":200,"seq":152}`. Linear y angular -1..1; angular positivo significa izquierda. Mixer progresivo: durante traslacion reduce el lateral interior hasta detenerlo con giro maximo; sin traslacion conserva el pivote. Ver arduino/TRACTION.md. La ESP aplica ajustes físicos del montaje: LEFT_DIRECTION=1, RIGHT_DIRECTION=-1, FORWARD_DIRECTION=-1, TURN_DIRECTION=-1. Conservarlos: no corregir nuevamente la dirección de motores para cambiar el signo de encoders.

I²C movimiento: cuatro bytes [versión 1, velocidad signed int16 little endian, XOR]. Cada Mini recibe el comando de su lateral y aplica compensacion y rampa independientes por motor. Feedback proporcional implementado pero desactivado hasta calibrar; ver arduino/TRACTION.md.

Encoders: contadores de 32 bits por polling, snapshot cada 10 ms; respuesta I²C de 14 bytes con versión 1, dos contadores, uptime y XOR. La ESP consulta cada 200 ms y calcula pulsos/s. /health y WebSocket incluyen `encoders.left/right` con valid, m1Ticks, m2Ticks, m1Pps, m2Pps, ageMs y miniUptimeMs cuando la lectura es válida. Una primera muestra puede tener velocidad nula; fallo o lectura antigua se presenta como no disponible, no como cero.

Signos corregidos según prueba física del usuario: izquierda M1=-1, M2=-1; derecha M1=-1, M2=+1. Avanzar aumenta los contadores, retroceder los reduce. El signo de pulsos/s indica el sentido actual; el conteo acumulado puede seguir negativo aunque la rueda esté avanzando. Se reinicia al reiniciar la Mini. Cero no distingue reposo de encoder desconectado. No hay RPM ni distancia calibradas; el polling puede perder transiciones a alta velocidad.

Watchdogs de movimiento: ESP y Minis 300 ms. UI manda órdenes activas cada 100 ms y corta ante falta de estado por 600 ms. HOTAS renueva habilitación de 250 ms. No hay reanudación automática al reconectar. Un controlador a la vez. Falta de izquierda detiene ambos laterales; derecha ausente todavía permite operar izquierda. Detener elimina potencia, no garantiza frenado mecánico instantáneo. Sin autenticación/TLS; uso en red local de pruebas.

## Confirmaciones y pendientes

Confirmaciones físicas del usuario al 12/10/2026: cableado unificado; problema de fuerza resuelto; cuatro encoders leyendo; signos corregidos; programación en zócalo con buck y VCC del loader desconectado funcionando. El asistente no ejecutó compilaciones ni pruebas automatizadas durante estos cambios. Estas confirmaciones no equivalen a una validación exhaustiva de watchdogs ni de precisión de encoders.

Ideas propuestas, **no implementadas**: diagnóstico de potencia aplicada/watchdog/versión por I²C, reset remoto de contadores, captura de encoders por interrupciones. Después: calibración de pulsos por vuelta/reducción, RPM y distancia; evaluar PID solo con lectura confiable. No confundir corrección de signo ya realizada con calibración física pendiente.

Diagnóstico futuro de encoder sin conteo: medir primero VCC-GND cerca del encoder y comparar con uno sano; revisar polaridad, continuidad y señales A/B. Intercambiar A/B invierte el signo, no debería eliminar el conteo. Si un motor funciona bien en otro driver, investigar control/alimentación del driver original antes de condenar el motor.

Validación de la nueva UI al 12/10/2026: compilación Angular y pruebas de seguridad aprobadas; revisión en navegador de ambos modos, visor X52 y ajuste a 3440 × 1440. La compilación informa una advertencia de presupuesto de estilos (10,83 kB frente a 10 kB), sin impedir generar la UI. Pendiente prueba física de conducción con la nueva UI y asignación HOTAS. No se compiló ni cargó firmware durante esta etapa.

Para retomar en otro chat: indicar la ruta del proyecto y pedir leer este README, `ui/README.md`, `ui/HOTAS.md`, `arduino/MINIS.md`, `arduino/ENCODERS.md` y el código relevante. Mantener la restricción de no compilar ni cargar firmware sin una nueva instrucción del usuario.

Documentación adicional: [Minis](arduino/MINIS.md), [encoders](arduino/ENCODERS.md), [UI](ui/README.md), [HOTAS](ui/HOTAS.md). Notas externas del proyecto: [planilla](https://docs.google.com/spreadsheets/d/1-8gbIZ-9NJ74Lot5L4k9WtnZ3bQboOgyeRztklJkotk/edit).
Planificación de interfaz: decisiones, inventario de funcionalidades y preguntas abiertas sobre modos minimalista/experto, cámara y LiDAR en [UI](ui/README.md#diseño-y-evolución-de-la-interfaz--12102026). El README de la UI distingue lo implementado de lo pendiente.



Ajustes de traccion del 20/10/2026 (pendientes de carga y prueba fisica): [detalle y calibracion](arduino/TRACTION.md).
