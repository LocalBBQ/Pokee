<p align="center"><img src="docs/images/logo.svg" alt="Yakumo" width="720"></p>

<p align="center"><a href="README.md">English</a> · <a href="README.ru.md">Русский</a> · <b>Español</b></p>

<p align="center">
  <a href="https://github.com/TeamGDB/Yakumo/releases/latest"><img src="https://img.shields.io/github/v/release/TeamGDB/Yakumo?style=flat-square&amp;label=release&amp;labelColor=2a1a22&amp;color=b98335" alt="Latest stable release"></a>
  <a href="https://github.com/TeamGDB/Yakumo/actions/workflows/tests.yml?query=branch%3Amain"><img src="https://img.shields.io/github/actions/workflow/status/TeamGDB/Yakumo/tests.yml?branch=main&amp;event=push&amp;style=flat-square&amp;label=tests&amp;labelColor=2a1a22" alt="Unit tests on main"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-b98335?style=flat-square&amp;labelColor=2a1a22" alt="License: MIT"></a>
  <a href="docs/COMPATIBILITY.md"><img src="https://img.shields.io/badge/platforms-Windows%20%C2%B7%20Linux%20%C2%B7%20macOS%20%C2%B7%20Android-b98335?style=flat-square&amp;labelColor=2a1a22" alt="Platforms: Windows, Linux, macOS and Android"></a>
  <a href="https://github.com/TeamGDB/Yakumo/stargazers"><img src="https://img.shields.io/github/stars/TeamGDB/Yakumo?style=flat-square&amp;label=stars&amp;labelColor=2a1a22&amp;color=b98335" alt="GitHub stars"></a>
  <a href="https://github.com/TeamGDB/Yakumo/releases"><img src="https://img.shields.io/github/downloads/TeamGDB/Yakumo/total?style=flat-square&amp;label=downloads&amp;labelColor=2a1a22&amp;color=b98335" alt="GitHub release asset downloads across all releases"></a>
</p>

<p align="center">
  <a href="https://discord.gg/XbQSE3b4m"><img src="https://img.shields.io/badge/Discord-Join%20community-b98335?style=flat-square&amp;logo=discord&amp;logoColor=white&amp;labelColor=2a1a22" alt="Join the TeamGDB Discord community"></a>
</p>

# Yakumo

Una versión nativa de **Monster Hunter Portable 3rd HD Ver.** hecha mediante recompilación estática: el código de PSP del juego se traduce de antemano a C++ y se compila para tu equipo, y después se ejecuta sobre una reimplementación del software de sistema de la PSP. No es un emulador —no hay un intérprete ni un JIT en su núcleo— ni tampoco una decompilación.

> **Este proyecto no incluye ningún recurso del juego.** Para instalar o compilar Yakumo debes aportar los archivos de tu propia copia legal de Monster Hunter Portable 3rd HD Ver. (`NPJB-40001`).

> Esto es una traducción. Si difiere del [README en inglés](README.md), prevalece el inglés. La documentación detallada está en inglés.

## Aviso legal

**Yakumo** es un proyecto independiente y de código abierto, y no está afiliado, autorizado, patrocinado ni respaldado por CAPCOM, Sony ni ninguna de sus filiales.

Monster Hunter, Monster Hunter Portable 3rd HD Ver., CAPCOM, PlayStation, PSP y todas las marcas comerciales, recursos del juego, ilustraciones, audio, personajes y demás propiedad intelectual relacionados pertenecen a sus respectivos propietarios.

**Yakumo** no incluye ningún recurso del juego ni archivos originales del juego: ni la imagen de disco, ni una copia del ejecutable o de los datos del juego, ni texturas, modelos, audio o vídeo del juego. Para instalar o compilar **Yakumo** debes aportar los archivos de tu propia copia legal de Monster Hunter Portable 3rd HD Ver.; el instalador comprueba esa copia y solo acepta la edición original.

Para usar **Yakumo**, los usuarios deben aportar los archivos necesarios a partir de su propia copia adquirida legalmente de Monster Hunter Portable 3rd HD Ver. para PlayStation 3.

Los usuarios son los únicos responsables de obtener, volcar, extraer y usar su copia del juego conforme a las leyes aplicables en su jurisdicción.

**Yakumo** no admite, proporciona, enlaza ni fomenta el uso de copias no autorizadas o piratas del juego.

Cualquier referencia al juego original o a sus marcas comerciales se hace únicamente con fines de identificación, compatibilidad e interoperabilidad.

Las capturas de pantalla y otras representaciones del juego original solo pueden usarse para documentar o mostrar el funcionamiento de **Yakumo**. Todo el contenido de terceros que aparezca sigue siendo propiedad de sus respectivos titulares.

La licencia de **Yakumo** se aplica únicamente al código y los materiales originales del propio proyecto y no concede ningún derecho sobre la propiedad intelectual de terceros.

**Yakumo** proporciona el software, no el juego. Debes aportar tu propia copia adquirida legalmente.

## Estado: jugable

Puedes cargar una partida copiada de una PSP o empezar una nueva, cazar con otros jugadores y guardar tu progreso, con música, cinemáticas e iluminación. La simulación del juego sigue funcionando a los 30 fotogramas por segundo de la PSP; la interpolación opcional puede presentarla a 45, 60, 90, 120 fotogramas o a la frecuencia de la pantalla sin cambiar la velocidad del juego. Las cargas son más cortas que en una PSP: mientras el juego carga en silencio, avanza más deprisa que el tiempo real (*Fast loading*, activado por defecto).

| Funciona | Falta o tiene problemas |
| --- | --- |
| Arranque, menús, creación de personaje, la aldea y las zonas de caza | |
| Partidas guardadas en el formato de la propia PSP, incluidas partidas y misiones descargadas copiadas de una PSP; importar, exportar y hacer copias de seguridad desde el menú | Superficies curvas (#10); los diálogos de guardado todavía no dibujan nada (#33) |
| Gráficos Vulkan: modelos, animación, texturas, transparencias, iluminación y niebla; resolución interna ajustable, cualquier forma de ventana e interpolación de fotogramas | |
| Paquetes de texturas HD compatibles con PPSSPP, instalados desde el menú o copiados al directorio de datos | |
| Mods en el formato de la comunidad (mhp3reload): reemplazos y parches de archivos, gestionados desde el menú (ver el [README del perfil](profiles/mhp3rd/README.md#mods)) | Mods de código (#81) |
| Efectos de sonido, música en streaming y cinemáticas | |
| Controles de teclado y ratón totalmente reasignables; mandos con cámara y apuntado analógicos en el stick derecho, preajustes de control y asignación de combinaciones de botones | |
| El menú de Yakumo dentro del juego, la configuración inicial, el explorador de archivos y el teclado en pantalla, todos utilizables con mando, teclado o ratón | |
| Los 355 overlays de código, recompilados | |
| Multijugador: a través de los servidores ad hoc que usan los jugadores de PSP, o con anfitrión desde el propio juego en una red local o VPN | |

Se ha probado en macOS (Apple Silicon, Vulkan a través de MoltenVK), en una Steam Deck en el modo de juego, con Vulkan nativo y los controles integrados, y en Windows 11 con MSVC.

El estado de cada parte del juego en cada plataforma está en [`docs/COMPATIBILITY.md`](docs/COMPATIBILITY.md).

## Jugar

### ⬇️ [Descargar la última versión estable](https://github.com/TeamGDB/Yakumo/releases/latest)

Una versión publicada solo necesita tu imagen de disco. Elige el archivo para tu dispositivo en los **Assets** de la versión, inícialo e indica tu imagen en la configuración inicial: comprueba la imagen, prepara el juego a partir de ella y lo guarda todo en un directorio del usuario. Actualizar conserva tus ajustes y partidas guardadas.

| Dispositivo | Archivo | Instalación |
| --- | --- | --- |
| **Windows** (x86-64) | `yakumo-…-windows-x86_64.zip` | Descomprímelo donde quieras y ejecuta `Yakumo.exe`. Sus datos van a `%APPDATA%`. Desde la v0.7.0 también hay un `…-portable.zip` que lo guarda todo (ajustes, partidas guardadas, el juego preparado) en una carpeta `data` junto a `Yakumo.exe`, por ejemplo en una memoria USB; más detalles en [Portable copy](profiles/mhp3rd/README.md#portable-copy). |
| **Steam Deck y Linux** | `yakumo-…-linux-x86_64.flatpak` (o el archivo portátil `.tar.gz`) | Instala el Flatpak, inicia Yakumo y añádelo a Steam para el modo de juego. [`docs/LINUX.md`](docs/LINUX.md) (en inglés) explica la instalación, el primer inicio, el modo de juego, dónde están las partidas guardadas, cómo actualizar y cómo desinstalar. |
| **Android** (64 bits, Android 10+, Vulkan 1.1) | `yakumo-…-android-arm64.apk` | Abre el APK en el teléfono y permite instalarlo. En el primer inicio toma tu `.iso` mediante el selector de archivos de Android y lo copia dentro de la aplicación (unos 1,3 GB además de los 0,8 GB de la aplicación). Los controles táctiles se dibujan sobre el juego; los mandos también funcionan. |
| **macOS** (Apple Silicon, macOS 13+) | `yakumo-…-macos-arm64.dmg` | Abre la imagen de disco y arrastra Yakumo a Aplicaciones. Apple no la ha notarizado, así que macOS pide permitirla una vez. [`docs/MACOS.md`](docs/MACOS.md) (en inglés) explica la instalación, el primer inicio, dónde se guardan las partidas, cómo actualizar y cómo desinstalar. |

**Versiones de prueba.** La [página de versiones](https://github.com/TeamGDB/Yakumo/releases) también muestra versiones de prueba, marcadas *(test)* (las anteriores, *(unstable)*) y publicadas como pre-release. Traen los cambios más nuevos y pueden tener errores nuevos: instálalas solo si quieres ayudar a probar. Si solo quieres jugar, usa la última versión estable de arriba.

## Requisitos

Compilar desde el código fuente es una forma plenamente compatible de jugar. Se necesita:

- Tu propia copia del juego (ver arriba)
- CMake 3.20 o posterior, Ninja y un compilador de C++20
- Python 3
- SDL3, Vulkan y `glslangValidator`
- `make` y un compilador de C en macOS y Linux: la compilación hace su propio FFmpeg para la música y las cinemáticas. En Windows no hay que instalar nada para ello
- Varios gigabytes de memoria libre para compilar: el código recompilado es grande

## Primeros pasos

En resumen:

1. Prepara el ejecutable del juego a partir de tu imagen de disco: compila `Yakumo` una vez sin código recompilado y ejecútalo con `--install /ruta/a/la/imagen.iso`. No hace falta ninguna herramienta de descifrado externa. Después enlaza la imagen y ese ejecutable con el perfil mediante `profiles/mhp3rd/scripts/prepare_game.sh`.
2. Configura, genera el código recompilado con `profiles/mhp3rd/scripts/generate.sh` y compila `Yakumo`.
3. Recompila los overlays de código con `profiles/mhp3rd/scripts/build_overlays.sh` (unos 40 minutos la primera vez; si se interrumpe, continúa donde se quedó).
4. Ejecuta `out/mhp3rd/bin/Yakumo`.

Las instrucciones completas, con todos los ajustes, están en [`profiles/mhp3rd/README.md`](profiles/mhp3rd/README.md). Todas las plataformas, Windows incluido, cuánto dura cada etapa y cómo trabajar en el código sin recompilarlo todo: [`docs/BUILDING.md`](docs/BUILDING.md).

## Controles

En un mando, los botones están donde esperas: en un mando de PlayStation el círculo confirma y la equis vuelve atrás, como indican los mensajes del juego, y el stick derecho mueve la cámara y el apuntado. Los preajustes de control (Default, Modern con los ataques en los gatillos, Left-handed, Classic keyboard y los tuyos propios) abarcan todas las acciones, y cualquiera puede asignarse a un solo botón o a una combinación como L1 + ○. El control con teclado y ratón es completo; de forma predeterminada, WASD mueve al personaje, el ratón controla la cámara y sus botones atacan. Las tablas completas están en el [README del perfil](profiles/mhp3rd/README.md#running).

Esc, o los dos sticks pulsados a la vez (L3+R3), abre el menú propio de Yakumo. Reúne los ajustes de vídeo, audio, controles, red y partidas, incluidos la relación de aspecto, la frecuencia de fotogramas, la resolución interna y la importación de texturas HD. El primer arranque prepara el juego a partir de tu imagen de disco en la misma ventana, y basta con un mando para hacerlo.

## Hoja de ruta

- Android en teléfonos reales: pruebas en dispositivos, controladores gráficos y rendimiento ([#127](https://github.com/TeamGDB/Yakumo/issues/127), [#17](https://github.com/TeamGDB/Yakumo/issues/17))
- Un estilo visual coherente para las pantallas de configuración, los menús y los overlays de Yakumo ([#33](https://github.com/TeamGDB/Yakumo/issues/33))
- Controles táctiles para el juego y los menús, además de la cámara y el apuntado

## Cómo funciona

El ejecutable se analiza y cada instrucción de su código se convierte en C++, que se compila dentro del programa. Además, el juego carga durante la ejecución 355 overlays de código en unas pocas regiones de memoria compartidas; cada uno se recompila en su propia biblioteca, y cuando el juego carga uno, la biblioteca correspondiente se instala entre fotogramas. Un intérprete ejecuta cualquier código que el conjunto recompilado no cubra, así que el juego nunca se detiene; solo va más lento en esas partes.

Alrededor de ese código hay una reimplementación del sistema de la PSP: un núcleo con hilos, semáforos, flags de eventos y temporizadores; lectura del disco directamente desde la imagen; un renderizador en Vulkan para el motor gráfico de la PSP; mezcla de voces por software para el audio; y entrada mediante SDL3.

[`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) describe el modelo de ejecución y [`docs/DATA_BIN.md`](docs/DATA_BIN.md) el formato de archivo del juego. [`docs/TESTING.md`](docs/TESTING.md) contiene la prueba de humo y cómo informar de los resultados.

## Estructura del repositorio

```text
include/psprecomp/   Framework: interfaces del runtime, la memoria y el estado Allegrex
src/                 Framework: carga de ELF, decodificador, runtime, intérprete
tools/               Framework: analizador y generador de código C++
tests/               Pruebas de regresión del framework
configs/             Datos de NID de PSP y ejemplos genéricos
profiles/mhp3rd/     Todo lo específico de este juego: host, núcleo, renderizador,
                     audio, entrada, configuración y scripts de compilación
docs/                Arquitectura, formato de archivo, guía de perfiles, normas
```

El código recompilado se genera localmente a partir de tu copia del juego y nunca se sube al repositorio.

## Basado en PSPRecomp

**Yakumo** está construido sobre [PSPRecomp](https://github.com/jessicanataliagta/PSPRecomp), un framework de recompilación estática para software de PSP. El framework no depende de ningún juego y se puede compilar por separado:

```bash
cmake -S . -B out/framework -DPSPRECOMP_PROFILE=""
cmake --build out/framework --config Release
ctest --test-dir out/framework -C Release --output-on-failure
```

Para dar soporte a otro juego, consulta [`docs/PROFILE_GUIDE.md`](docs/PROFILE_GUIDE.md). [`docs/SOURCE_PROVENANCE.md`](docs/SOURCE_PROVENANCE.md) recoge las normas sobre código escrito de forma independiente y código de terceros.

## Autores

- [@MHunterG](https://github.com/MHunterG)
- [@mojitosunrise](https://github.com/mojitosunrise)

## Créditos

- [PSPRecomp](https://github.com/jessicanataliagta/PSPRecomp): el framework de recompilación sobre el que se construye el proyecto
- [SDL3](https://www.libsdl.org/): ventana, entrada y salida de audio
- [FFmpeg](https://ffmpeg.org/): decodificación de la música y el vídeo
- [Vulkan](https://www.vulkan.org/) y [MoltenVK](https://github.com/KhronosGroup/MoltenVK): renderizado
- [Dear ImGui](https://github.com/ocornut/imgui): menús y pantallas de configuración inicial de Yakumo
- [tiny-AES-c](https://github.com/kokke/tiny-AES-c): instalador y compatibilidad con las partidas guardadas de PSP
- [stb_truetype](https://github.com/nothings/stb): rasterización de fuentes
- [svanheulen/mhef](https://github.com/svanheulen/mhef): documentación de la comunidad sobre el formato de archivo del juego
- [Cinzel](https://github.com/NDISCOVER/Cinzel) y [Shippori Mincho](https://github.com/fontdasu/ShipporiMincho): tipografías del logotipo, bajo la SIL Open Font License

## Licencia

El repositorio se distribuye bajo la licencia MIT; consulta [`LICENSE`](LICENSE). Los archivos de terceros conservan sus licencias junto a ellos; [`docs/SOURCE_PROVENANCE.md`](docs/SOURCE_PROVENANCE.md) enumera sus orígenes y licencias.
