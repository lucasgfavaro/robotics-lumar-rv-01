# UI del Rover

Desde esta carpeta, con una versión de Node compatible con engines de package.json:

```powershell
npm ci
npm test
npm run build
npm start
```

Las pruebas disponibles son las de seguridad en tests/safety.mjs; este proyecto no configura Vitest ni ng test.

Abrí http://localhost:4200 y conectá a ws://IP_DE_LA_ESP:81/.
La UI espera estado válido antes de habilitar MANTENER PARA MOVER.
Los botones de dirección seleccionan valores; el movimiento ocurre mientras mantenés el control apretado.

También podés controlar con las flechas del teclado y con HOTAS X52 (avance, giro lateral y throttle). Ver HOTAS.md para identificar ejes, configurar el botón de habilitación y probar el mando.

Carpeta de trabajo actual: `E:\gitRepos\arduinoIDE\iot\iot-projects\rover-billy\ui`.
Los cambios de UI de la carpeta del chat se incorporaron el 11 de octubre de 2026. No se ejecutaron compilaciones ni pruebas durante esta incorporación. La prueba de pinout usa una referencia local en `tests/fixtures`, sin depender de la carpeta del chat.

Ver ../README.md y ../RECUPERACION.md para firmware, cableado, configuración y validación física.
No se ejecutaron compilaciones ni pruebas durante la recuperación del 7 de octubre de 2026.
