# Informe de validación

## Realizado
- Inspección directa del código de OBS 31.0.3, commit
  fcd1910bf5116b69404a6ecdda6efedd1d00ebdf.
- Contraste de IDs de NVENC, captura de ventana, captura WASAPI por proceso,
  parámetros preset/multipass/rate_control y firmas públicas de libobs.

El CI Windows/MSVC llegó a compilar e instalar `primow-stream.exe` en el
directorio de preparación. El empaquetado falló después por buscar
`windeployqt`, que las dependencias Qt de OBS no incluyen. Se cambió al
empaquetador de dependencias de OBS; ese cambio aún debe verificarse en CI.
Esto no equivale a una prueba de ejecución ni a un instalador validado.

## Automatización preparada, ejecución completa aún pendiente
- Workflow Windows 2022/MSVC: descarga dependencias, compila, instala,
  despliega Qt, comprueba archivos esenciales, genera paquetes y guarda logs.
- El inventario de archivos NO prueba resolución transitiva de DLL.
- No hay todavía tests unitarios automatizados ni transmisión a un servidor de prueba.

## Pruebas manuales necesarias antes de declarar una versión funcional

1. Windows limpio, sin OBS, Visual Studio ni Qt: instalar, abrir, desinstalar.
2. RTX 4070 Super: inicializar NVENC realmente y verificar salida H.264.
3. CS2, Valorant, PUBG, Ragnarok Online y otra aplicación: probar ventana,
   juego donde esté permitido y pantalla con consentimiento. Registrar modo,
   versión Windows/driver/juego, pantalla negra y tiempo de cambio.
4. Cambiar fuentes durante transmisión a servidor RTMPS de prueba: comprobar
   continuidad y ausencia de reinicio del codificador.
5. Seguimiento desactivado no cambia fuente; activado no captura apps no autorizadas.
6. Micrófono/sistema/app: verificar niveles, silencio, sincronía y ausencia de duplicados;
   desconectar dispositivos y registrar comportamiento.
7. Interrumpir red y recuperar: observar reconexión limitada, doble clic de inicio,
   detención durante conexión y cierre seguro.
8. Kick: solo desde el PC y tras pulsar INICIAR STREAM; comprobar 1080p60,
   H.264 Main, AAC, bitrate y keyframes en recepción.
9. Benchmarks repetidos: misma escena y ajustes, sin streaming / P4 / P5 / P6.
   Usar PresentMon para frametimes y contadores Windows para CPU/RAM/GPU.
   Registrar versiones, duración, FPS medios, p95/p99 frametime y frames perdidos.
   No extrapolar resultados de un juego al resto.
10. Confirmar que monitor sigue a su resolución y frecuencia originales.

Resultado actual de todas las pruebas anteriores: **NO EJECUTADAS**.
