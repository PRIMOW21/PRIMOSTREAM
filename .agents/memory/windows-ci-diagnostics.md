---
name: Diagnóstico de CI Windows
description: Limitación observada al conservar salida de compiladores nativos mediante PowerShell.
---

No asumir que Start-Transcript conserva la salida de procesos nativos en el runner de Windows.

**Why:** Los artefactos descargados contenían un transcript de solo 918 bytes con «cmake failed (1)», aunque la consola de GitHub mostraba cientos de líneas del compilador. El registro de configuración de CMake tampoco contiene los errores de compilación posteriores.

**How to apply:** Para diagnosticar un fallo, solicitar el archivo completo de registros de la ejecución de GitHub, no solo el artefacto de diagnóstico. Si se mejora la persistencia de logs, capturar explícitamente stdout y stderr de los comandos nativos y conservar sus códigos de salida.
