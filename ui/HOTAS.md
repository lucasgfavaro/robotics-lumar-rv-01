# HOTAS X52 — configuración actual al 12/10/2026

Solo cambia Angular: el firmware y el protocolo siguen iguales. El joystick principal controla avance/retroceso proporcional; el throttle controla el límite PWM de 20 a 255. La rotación del joystick (eje 5) controla giro proporcional, con la misma zona muerta: angular positivo gira a izquierda y negativo a derecha. Se puede combinar avance y giro.

## Configuración manual

1. Abrí la UI en `http://localhost:4200`, conectá el X52 por USB, hacé clic en la página y presioná un botón del joystick.
2. Abrí la pantalla **Joystick** desde la barra superior; el panel **Identificar ejes y botones** aparece desplegado. Cada dispositivo muestra su índice, nombre, ejes y botones. Si no aparece, verificá primero que Windows detecte el X52 mediante `joy.cpl`.
3. Mové solo el joystick adelante/atrás y anotá el número del eje que cambia. Luego mové el joystick sobre su eje de rotación y comprobá el eje 5 de giro; finalmente mové solo el throttle y anotá su eje. Seleccioná dispositivo(s), ejes y botón de habilitación. El throttle puede aparecer en el mismo dispositivo que el joystick o en otro.
4. Comprobá la vista previa SIN activar: adelante debe dar avance positivo, atrás negativo y centro cero; giro izquierdo debe dar angular positivo y derecho negativo; throttle mínimo debe mostrar 20 y máximo 255. Ajustá las inversiones si hace falta.
5. Volvé a **Conducción** y conectá el rover. Con ruedas levantadas, poné throttle al mínimo, centrá el joystick y pulsá **Activar HOTAS**. Soltá el botón elegido hasta ver **Listo**.
6. Mantené el botón elegido y mové el joystick. Soltar el botón o centrar la palanca detiene. Probá ambos sentidos y la detención antes de aumentar potencia.

Configuración inicial actual: avance/retroceso eje 1, giro por rotación eje 5, throttle eje 2 y habilitación con pinky, botón 5. Eje de giro y botón pinky fueron identificados por el usuario el 12/10/2026. Mantener el pinky para mover; soltarlo detiene. Verificar el sentido de giro con la vista previa y ajustar Invertir giro si corresponde. Configuración no persistida: volver a revisar al recargar.

## Detención

- Esc, DETENER, pérdida de foco, pestaña oculta, cambio de configuración o pérdida de conexión desactivan HOTAS. Requiere activación manual; no reanuda al reconectar.
- Mientras HOTAS está activo, teclado y botón de movimiento manual no controlan el rover.
- La zona muerta inicial es 0,12 y el resto del recorrido del joystick se reescala a -1..1.
- El servicio lee el mando cada 100 ms. Si pasa más de 250 ms entre lecturas, desactiva HOTAS. Los comandos HOTAS tienen además una habilitación de 250 ms que debe renovarse con nuevas lecturas; el heartbeat deja de mantener la última orden si no se renueva. Los watchdogs de ESP y Mini siguen en 300 ms.
- La API del navegador no permite garantizar que el driver refleje correctamente todos los controles. La verificación física de lectura, signo, centro, botón, desconexión y pérdida de foco sigue pendiente.

Durante la integración inicial no se compilaron ni ejecutaron pruebas. En la nueva UI del 12/10/2026, compilación Angular y pruebas de seguridad aprobadas; visor X52 revisado en navegador. Pendiente verificar físicamente giro por rotación, pinky y detenciones. No se necesita recargar firmware para esta etapa.

Referencia: https://developer.mozilla.org/en-US/docs/Web/API/Gamepad_API/Using_the_Gamepad_API



