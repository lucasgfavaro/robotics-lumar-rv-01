# Recuperación del proyecto Rover — 7 de octubre de 2026

## Ubicaciones

- Backup inspeccionado: `C:\Users\lucas\Downloads\g-p-6a0b40a0a39481918db84577161ebda7`.
- Proyecto de trabajo: `C:\Users\lucas\.codex\.chatgpt-projects\g-p-6a0b40a0a39481918db84577161ebda7\rover`.
- Resguardo anterior a los cambios: `..\recovery\original-2026-10-07`. Incluye el código, la configuración local y el sketch original; no incluye dependencias ni salidas generadas. La configuración local puede contener credenciales: mantener el resguardo privado.

El backup de Downloads no se modificó. Tampoco se modificaron `sources/`, AGENTS.md ni el sketch original.

## Qué se encontró

La carpeta actual ya contenía una copia idéntica del código del backup, comprobada mediante hashes por archivo. No faltaban archivos de código del backup en el proyecto actual. No fue necesario regenerar Angular ni sobrescribir el proyecto.

- Angular: controles lineal/angular/potencia, transporte WebSocket, envío cada 100 ms mientras se mantiene el botón, detención al soltar y perder foco, estado del gateway y pruebas de seguridad.
- ESP8266: WebSocket en puerto 81, diagnóstico `/health` en 80, mixer diferencial normalizado, dueño único del control, secuencias, watchdog de 300 ms y envío I²C periódico.
- Pro Mini: pinout original, velocidad signed de -255 a 255 para ambos motores del lateral, validación del paquete y watchdog de 300 ms. Encoders por polling conservados.
- PlatformIO: ESP8266, Mini izquierda 0x10 y Mini derecha 0x11; bibliotecas y placas declaradas.
- Documentación, configuración de ejemplo y configuración local presentes. En el backup también había dependencias de Angular y artefactos de PlatformIO; su presencia no acredita una compilación correcta.

La planilla de notas se consultó como referencia, sin modificarla. El alcance sigue siendo control manual con un lateral obligatorio y otro opcional. ROS, PID y telemetría de encoders siguen pendientes de una etapa posterior.

## Cambios de esta recuperación

1. La UI espera un estado válido antes de habilitar movimiento. Valida tipos y rango de velocidades del estado; una respuesta inválida cierra la conexión.
2. Al apretar el control, la UI verifica que el estado tenga como máximo 600 ms, aunque el temporizador todavía no haya ejecutado su revisión.
3. El gateway vuelve a comprobar el watchdog dentro del callback WebSocket: si el mensaje llega después del chequeo inicial del loop, libera al controlador vencido antes de procesarlo.
4. Si la izquierda no responde por I²C, el gateway envía directamente cero a la derecha, evitando un envío transitorio de movimiento antes de detenerla.
5. Las pruebas existentes se actualizaron para el primer estado, estado inválido y estado vencido al apretar.

## Validación pendiente a cargo del usuario

No se ejecutaron compilaciones, instalaciones ni pruebas en esta recuperación, por pedido del usuario. Las correcciones se revisaron por lectura de código; todavía deben compilarse y probarse.

Los comandos de compilación, ejecución de pruebas y carga están en README.md. Confirmar el modelo y frecuencia de las placas antes de cargar. Primero probar con ruedas levantadas y potencia baja: avance, retroceso, giro, soltar botón, pérdida de foco, corte de WebSocket/Wi-Fi y desconexión de I²C. Verificar la detención de ambas etapas y el funcionamiento con la derecha ausente.
