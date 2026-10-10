# Informe de validación

## Realizado
- Inspección directa del código de OBS 31.0.3, commit
  fcd1910bf5116b69404a6ecdda6efedd1d00ebdf.
- Contraste de IDs de NVENC, captura de ventana, captura WASAPI por proceso,
  parámetros preset/multipass/rate_control y firmas públicas de libobs.

Una ejecución de CI anterior generó los artefactos Windows con el empaquetador
de dependencias de OBS. Eso confirma el paquete de esa revisión, no estos cambios
de plataformas, interfaz y selección DXGI del monitor. Esta revisión debe pasar
una nueva compilación CI y una prueba de ejecución en Windows.

## Automatización preparada, ejecución completa aún pendiente
- Workflow Windows 2022/MSVC: descarga dependencias, compila, instala,
  empaqueta dependencias de Qt con OBS, comprueba archivos esenciales, genera
  paquetes y guarda logs. La compilación Linux de este entorno no puede verificar
  código C++/Qt de Windows, NVENC, captura ni estos cambios.
- El inventario de archivos NO prueba resolución transitiva de DLL.
- No hay todavía tests unitarios automatizados ni transmisión a un servidor de prueba.
- Los destinos Kick, Twitch y RTMP personalizado usan `rtmp_custom` con servidor y
  clave separados. TikTok solo es configurable si la cuenta facilita sus datos RTMP;
  no se han probado conexiones reales.
- La captura de pantalla principal selecciona el `monitor_id` de Windows y fuerza
  DXGI. Los cambios deben validarse en una máquina Windows real; no se ha confirmado
  todavía si el indicador amarillo del equipo venía del sistema o del método de captura.

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
8. Kick y Twitch: solo desde el PC y tras pulsar INICIAR STREAM; comprobar
   1080p60, H.264 Main, AAC, bitrate y keyframes en recepción. TikTok requiere
   acceso LIVE y datos RTMP facilitados por la propia cuenta.
9. Benchmarks repetidos: misma escena y ajustes, sin streaming / P4 / P5 / P6.
   Usar PresentMon para frametimes y contadores Windows para CPU/RAM/GPU.
   Registrar versiones, duración, FPS medios, p95/p99 frametime y frames perdidos.
   No extrapolar resultados de un juego al resto.
10. Confirmar que monitor sigue a su resolución y frecuencia originales.

Resultado actual de todas las pruebas anteriores: **NO EJECUTADAS**.
