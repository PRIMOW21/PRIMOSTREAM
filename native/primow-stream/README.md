# PRIMOW STREAM — desarrollo nativo

Aplicación Windows x64 en C++20, Qt 6 Widgets y libobs. No es una web,
no usa Electron y no necesita Replit ni OBS Studio instalado para ejecutarse
una vez correctamente empaquetada. **Todavía no compilada ni verificada en Windows.**

## Compilar

1. Copiar el repositorio a GitHub, conservando `.github/workflows/windows-build.yml`.
2. Abrir Actions → PRIMOW STREAM Windows build → Run workflow.
3. Si la compilación termina, descargar el artefacto Windows-development.
   Un ZIP generado no certifica captura, NVENC, transmisión ni instalación.
4. Ante fallos, descargar windows-build-logs. No incluir claves en el informe.

Localmente: Windows x64, Visual Studio 2022 con Desktop development with C++ y
SDK 10.0.22621, Git, CMake >=3.30, PowerShell 7 e Inno Setup 6.
Ejecutar desde la raíz:

```powershell
./native/primow-stream/scripts/build-windows.ps1
```

La primera compilación descarga OBS 31.0.3 y sus dependencias fijadas por
buildspec.json (incluye hashes); usa su grafo CMake para evitar mezclar ABIs.
El script añade PRIMOW a la copia de OBS descargada, no modifica el repositorio
original. OBS UI, navegador, scripting y otras funciones no necesarias se desactivan.
La versión 31.0.3 es una base fijada e inspeccionada, no una afirmación de ser la más reciente.
MSVC y herramientas del runner aún no son herméticas: revisar actualizaciones del runner.

## Implementación presente, pendiente de validación Windows

- Enumeración de ventanas visibles de cualquier ejecutable, sin lista cerrada de juegos.
- Selección explícita de WGC, captura de juego estándar de OBS o pantalla principal.
- Cambio de fuente en la misma escena, conservando salida y codificadores.
- Seguimiento opcional del primer plano, solo de ejecutables autorizados durante la sesión.
- Micrófono WASAPI independiente, volumen y silencios; audio del sistema o de aplicación.
- NVENC H.264, 1080p60, CBR, 6000 kbps (máximo configurable 8000), Main,
  keyframes 2 s, single-pass, AAC 48 kHz estéreo 160 kbps.
- Perfiles P4/P5/P6 sin lógica específica de CS2.
- RTMP(S), estados desde señales reales, reconexión limitada a cinco intentos.
- Clave en Windows Credential Manager, nunca enviada a Replit.
- No se modifican monitor, ratón, juego, Steam ni anticheat.
- Métricas reales de bytes/FPS/frames perdidos; ningún dato de rendimiento inventado.

## Límites actuales / pendientes

No es todavía una versión final distribuible. Quedan: compilación y depuración
Windows; previsualización nativa y suspensión de su renderizado; medidores y prueba
de audio; selector de dispositivos y de monitores; asistente inicial; clasificación
heurística de juegos (actualmente se enumeran ventanas, no se afirma que sean juegos);
autorefresco de lista; seguimiento con validación robusta del ejecutable; diagnóstico
de cuadros negros por muestreo (dimensiones no bastan); confirmación del fallback
desde la alerta; detección de dispositivos desconectados; contador de tiempo;
reconexión configurable; diagnóstico GPU/NVENC detallado; benchmarks; logs seguros;
persistencia completa de fuentes/audio y auditoría de distribución/licencias.

La aplicación advierte sobre fuentes sin dimensiones, pero no puede distinguir aún
pantallas negras válidas de bloqueo de captura. No cambiará silenciosamente a pantalla
completa porque puede exponer notificaciones. El audio del sistema incluye otras apps;
la captura por aplicación depende de la versión de Windows y del soporte WASAPI.
No existe garantía de compatibilidad con todos los juegos o anticheats.

Preferencias no secretas: QSettings en AppData. Stream Key: Windows Credential Manager.
Para borrar la clave: Administrador de credenciales → Credenciales de Windows →
PRIMOW-STREAM/Kick. No introducir claves en GitHub Actions.

## Pruebas

Ver `docs/VALIDATION.md`. No se han realizado pruebas Windows, Kick ni benchmarks.
El entorno Linux no dispone de Windows/MSVC, NVENC ni dispositivos del PC del usuario.
El workflow está preparado pero no se ha ejecutado desde este entorno.
