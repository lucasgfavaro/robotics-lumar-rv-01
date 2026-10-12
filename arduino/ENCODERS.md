# Telemetría de encoders — 2026-10-12

## Cargas necesarias

1. Mini izquierda: `mini_left/mini_left.ino`, I2C 0x10.
2. Mini derecha: `mini_right/mini_right.ino`, I2C 0x11.
3. ESP: `esp8266/esp8266.ino`.
4. Recargar Angular desde la carpeta `ui` del proyecto de E:.

Minis: ATmega328P, 3,3 V / 8 MHz según el montaje informado. No se compilaron ni ejecutaron pruebas. Los archivos generados están preparados para carga manual; la implementación debe verificarse físicamente.

## Lectura y protocolo

Cada Mini conserva dos contadores unsigned de 32 bits con wrap definido. Lee los encoders por polling con el pinout unificado registrado en MINIS.md: ambos laterales M1 D2/D4, M2 D8/D9. Prepara una instantánea coherente cada 10 ms; `Wire.onRequest` solo devuelve esa instantánea, sin calcular velocidades ni escribir por Serial dentro de la interrupción.

Paquete I2C de 14 bytes: versión 1, contador motor 1 (4 bytes), contador motor 2 (4 bytes), uptime de Mini en ms (4 bytes), XOR de los 13 bytes anteriores. Campos de 32 bits little endian. Los comandos de movimiento mantienen su formato anterior de cuatro bytes.

ESP consulta ambas Minis cada 200 ms. Valida longitud, versión y checksum. Calcula pulsos/s con diferencia modular de contadores y diferencia del uptime de la Mini. La primera muestra, o una discontinuidad de tiempo, no produce velocidad. Lecturas fallidas o antiguas se marcan inválidas, sin convertirlas en una velocidad cero aparente. La consulta no renueva el watchdog de movimiento.

`/health` y WebSocket incluyen `encoders.left` y `encoders.right` con `valid`. Si es válido: `m1Ticks`, `m2Ticks`, `m1Pps`, `m2Pps`, `ageMs`, `miniUptimeMs`. Tasas nulas mientras falta una segunda muestra válida. Los signos están normalizados al avance físico: izquierda M1/M2 multiplican por -1; derecha M1 por -1 y M2 por +1, según la medición del usuario. Avanzar incrementa el conteo y retroceder lo reduce; los pulsos/s indican el sentido actual.

El polling puede perder transiciones a alta velocidad; esta primera etapa sirve para comprobar cableado y dirección, no certifica exactitud para control cerrado. No hay RPM, PID ni detección automática de encoder desconectado.

## Comprobación manual

Con motores sin alimentación de potencia, pero Minis y ESP encendidas, girar una rueda a mano y comprobar qué contador cambia. Repetir con las cuatro ruedas y ambos sentidos. Confirmar que ningún otro contador cambie con esa rueda. Un A/B intercambiado invierte el signo.

Después probar movimiento brevemente, con ruedas levantadas y potencia baja; comprobar detención al soltar. Desconectar una Mini con todo apagado y volver a encender: su panel debe indicar falta de lectura. Reiniciar una Mini: su contador vuelve a cero y la velocidad requiere nueva muestra. Verificar `/health` y ambos sentidos conservados antes de usar en el suelo.
