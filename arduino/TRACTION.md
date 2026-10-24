# Ajuste de traccion — 2026-10-20

Cargar las dos Minis y la ESP. No se ha cargado ningun controlador automaticamente.
Protocolo I2C y polaridades conservados; no requiere actualizar la UI.

ESP: durante avance o retroceso, direccion progresiva reduce el lateral interior
hasta cero con direccion maxima; conserva pivote al no pedir avance.
Ejemplo normalizado: avance 1, giro 0.2 -> 0.512 / 1; giro 1 -> 0 / 1.

Minis: salidas por motor independientes, rampa ascendente de 6 PWM cada 50 ms,
reduccion inmediata y pausa de un ciclo de control al invertir.
STOP o watchdog de 300 ms anulan ambos motores inmediatamente en el loop.
Minimo provisional: 48 PWM a comando 40, interpolado continuamente desde cero
y hasta 255. No es calibracion medida. Ajustar WHEEL_CAL en board_config.h por
rueda y sentido ELECTRICO (negativo/positivo); no cambiar signos de encoders.

Feedback implementado pero DESACTIVADO: fullScalePps=0 en las cuatro ruedas.
Para activarlo medir pulsos/s por rueda y sentido a PWM 255 con lectura fiable,
validar tambien la curva a baja velocidad y configurar esos valores. El modelo
inicial de velocidad objetivo es proporcional al comando: requiere validacion.
Usa ventana local de 50 ms, proporcional sin integral, correccion maxima +/-20
PWM y variacion maxima 2 PWM por ventana. Sin pulsos durante 600 ms retira
refuerzo positivo; no diagnostica bloqueo ni corta el PWM base.
No utiliza velocidades de otras ruedas como referencia: una rueda en el aire
no aumenta la consigna del resto. Los encoders solos no prueban adherencia.
El polling debe verificarse por perdida de transiciones antes de activar feedback.

Prueba inicial: ruedas levantadas, potencia baja, verificar cuatro sentidos,
STOP y desconexion. Luego en suelo comparar avance recto, curva parcial y giro
maximo. Registrar por rueda el menor PWM que arranca y que mantiene movimiento.
La UI sigue mostrando comandos por lateral, no PWM real individual aplicado.

Dependencias ESP: WebSockets 2.6.1 y ArduinoJson 6.21.5. Instaladas tambien en el sketchbook configurado por Arduino IDE (arduino/libraries, excluido de Git).
