# UI de LUMAR RV-01 — estado actual al 12/10/2026

Carpeta de trabajo: `E:\gitRepos\projects\robotics\LUMAR-RV-01\ui`. Este README reúne el diseño acordado, lo implementado y los próximos pasos. El [README general](../README.md) conserva hardware, firmware, red y operación; [HOTAS.md](HOTAS.md) detalla el mando.

## Iniciar y verificar

Usar Node compatible con `package.json`, por ejemplo 22.22.3. Desde esta carpeta:

```powershell
npm.cmd ci
npm.cmd exec --yes --package=node@22.22.3 -- node node_modules/@angular/cli/bin/ng.js serve
```

Abrir `http://localhost:4200`. WebSocket inicial: `ws://192.168.100.50:81/`, editable en experto. No hace falta reinstalar dependencias por cambios de HTML/CSS.

Verificaciones de UI:

```powershell
npm.cmd test
npm.cmd exec --yes --package=node@22.22.3 -- node node_modules/@angular/cli/bin/ng.js build
```

Las pruebas están en `tests/safety.mjs`; no se configura Vitest ni `ng test`. Comprueban seguridad de conexión/movimiento, pinout unificado del 12/10/2026 y detención al cambiar pantalla/modo. No compilan firmware.

## Diseño acordado e implementado

Referencia: [Diseño de referencia de la interfaz](https://app.diagrams.net/#G1yWCdTdSM0WgTZXTuvAuD5SM5uYTYHD_o), página `03018318-947c-dd8e-b7a3-06fadd420f32`.

Uso principal: PC widescreen, monitor del usuario de **3440 × 1440**. La pantalla Conducción usa todo el ancho y se ajusta a la altura disponible del navegador. Columna izquierda proporcional hasta 680 px, con controles y rover ampliados en pantallas grandes. La cámara ocupa la altura restante; el modo experto permite scroll interno de la columna izquierda si excede el espacio. En pantallas pequeñas los paneles se apilan. Joystick conserva scroll normal.

Barra superior compartida: Conducción / Joystick, estado de conexión, Conectar, Desconectar, selector Minimalista / Experto y PARAR (Esc).

| Funcionalidad | Estado actual |
|---|---|
| Minimalista | Modo inicial; dirección gráfica e intensidad, potencia, movimiento manual y HOTAS solo con activación/desactivación y estado |
| Experto | Misma distribución; añade edición de WebSocket, valores de avance/giro, conteos, pulsos/s, antigüedad de encoders, PWM y presencia por lateral |
| Conducción | Arriba a la izquierda; teclado, selección de dirección y mantener para mover |
| Rover gráfico | Debajo de conducción; cuatro ruedas con colores y flechas grandes animadas |
| Cámara | Panel principal a la derecha; espacio visual implementado, transmisión pendiente |
| Logo de empresa | Espacio reservado abajo del panel de cámara; asset pendiente |
| Joystick | Pantalla independiente de configuración, vista previa e identificación de ejes y botones |
| LiDAR | Postergado por el usuario; sin panel en esta primera UI |
| RPM y distancia | Pendientes de calibración física |

El color de las ruedas representa la orden PWM informada por lateral (verde a rojo), no potencia medida por motor. Las flechas junto a cada rueda indican el signo de pulsos/s: avance, retroceso, reposo o sin lectura. No se muestra el banner de dirección: el usuario prefirió solo las flechas de ruedas. Animaciones respetan la preferencia de movimiento reducido. Cero no distingue rueda quieta de encoder desconectado. M1/M2 son etiquetas del cableado; su posición delantera/trasera en el dibujo no acredita la posición física.

## Controles y seguridad

- La UI espera estado válido del rover antes de habilitar movimiento.
- Flechas mantenidas: avance, retroceso y giro, con combinaciones. Esc y PARAR detienen.
- Botones de dirección seleccionan valores; no inician movimiento. Mantener el botón de movimiento para conducir; soltarlo detiene.
- Potencia 20..255, sin equivalencia calibrada a m/s. En HOTAS la potencia viene del throttle.
- Los campos editables no activan movimiento con flechas.
- Soltar, perder foco, ocultar la pestaña o perder conexión detiene; HOTAS requiere reactivación manual.
- Cambiar pantalla o modo detiene y desactiva HOTAS.
- Joystick permite identificar/configurar sin conectar el rover y no permite conducir. Para activar HOTAS, volver a Conducción.
- Datos ausentes o inválidos se muestran como sin lectura, no como cero.

## HOTAS actual

X52: avance/retroceso eje **1**, giro por **rotación eje 5**, throttle eje **2**, habilitación con **pinky botón 5**. Giro y pinky identificados por el usuario el 12/10/2026. Mantener pinky para mover; soltarlo detiene. Zona muerta inicial 0,12. Dispositivos, asignaciones e inversiones siguen editables en Joystick. Verificar sentido de giro en vista previa y ajustar inversión si corresponde.

Configuración y modo no se persisten al recargar. El visor original de ejes y botones se conserva y aparece desplegado en Joystick.

## Validación y pendientes

Compilación Angular y pruebas de seguridad aprobadas. Se revisaron ambas vistas, visor X52 y dimensiones de página a 3440 × 1440 en navegador, sin desbordamiento de página en esa resolución. Existe advertencia de presupuesto de estilos: 10,83 kB frente a 10 kB; la compilación finaliza correctamente.

Pendiente validación física del usuario de conducción con la nueva UI, giro por rotación y pinky. No se compiló ni cargó firmware durante estos cambios.

Próximos pasos:

- Cámara: definir hardware, equipo transmisor, protocolo, resolución y latencia para conectar el panel.
- Incorporar logo real.
- Decidir persistencia de preferencias y configuración HOTAS.
- LiDAR, más adelante: definir modelo, procesamiento y representación antes de integrarlo.
- Calibrar encoders antes de añadir RPM/distancia; control independiente, PID, navegación autónoma y frenado por obstáculos no están implementados.

Los cambios recuperados del 7 y 11 de octubre no se compilaron ni probaron en esas recuperaciones; las verificaciones anteriores corresponden a esta nueva UI del 12/10. Ver [recuperación histórica](../RECUPERACION.md).
