# Registro de las Minis

| Placa física | Sketch para Arduino IDE | Dirección I2C | Procesador indicado por el usuario |
|---|---|---|---|
| Lateral izquierdo | `mini_left/mini_left.ino` | `0x10` | ATmega328P, 3,3 V / 8 MHz |
| Lateral derecho | `mini_right/mini_right.ino` | `0x11` | ATmega328P, 3,3 V / 8 MHz |

Etiquetar las placas físicas como **LEFT 0x10** y **RIGHT 0x11**. El puerto COM identifica al adaptador USB-serie, no necesariamente a la Mini conectada.

Los sketches activos son `mini_left/mini_left.ino` y `mini_right/mini_right.ino`. Editarlos directamente y mantener sincronizada la lógica común. Cada carpeta contiene sus headers locales. Conservar el `board_config.h` de cada lateral: define su dirección I2C y signos de encoder. No hay copias generadas ni un paso de regeneración.

Para cargar, abrir el sketch del lateral correcto y seleccionar placa, procesador y puerto. Al arrancar, Serial a 115200 muestra lateral, dirección I2C y revisión del pinout. Pulsar RESET con el monitor abierto para ver el mensaje.

La carpeta antigua mini/ y la estructura duplicada se archivaron fuera del proyecto activo.

## Registro de cargas manuales

| Placa | Fecha de carga | Revisión | Verificación |
|---|---|---|---|
| Izquierda | 2026-10-12, según confirmación del usuario | Pinout unificado y signos corregidos | Encoders funcionando; lectura de Serial no registrada |
| Derecha | 2026-10-12, según confirmación del usuario | Pinout unificado y signos corregidos | Encoders funcionando; lectura de Serial no registrada |

Actualizar esta tabla después de cada carga efectiva. La generación de carpetas no acredita una carga ni una prueba física.

## Cableado unificado — revisión 2026-10-12

| Señal | Motor 1 (ambos laterales) | Motor 2 (ambos laterales) |
|---|---|---|
| L_EN | D6 | D13 |
| R_EN | D7 | D12 |
| LPWM | D5 | D11 |
| RPWM | D3 | D10 |
| Encoder A | D2 | D8 |
| Encoder B | D4 | D9 |

A y B siguen el orden de los dos cables de encoder en la lista del usuario. Los contadores M1/M2 de la UI corresponden a esta tabla. Izquierda 0x10 y derecha 0x11 usan el mismo cableado. No se modificaron los sentidos en la ESP.
Signo de encoders calibrado por software: izquierda M1 -1, M2 -1; derecha M1 -1, M2 +1. Avance incrementa el conteo. No cambia la dirección de los motores ni el pinout.
