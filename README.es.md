<p align="center"><img src="docs/images/logo.svg" alt="Yakumo" width="720"></p>

<p align="center"><a href="README.md">English</a> · <a href="README.ru.md">Русский</a> · <b>Español</b></p>

# Yakumo

Una versión nativa de **Monster Hunter Portable 3rd HD Ver.** hecha mediante recompilación estática: el código de PSP del juego se traduce de antemano a C++ y se compila para tu equipo, y después se ejecuta sobre una reimplementación del software de sistema de la PSP. No es un emulador —no hay un intérprete ni un JIT en su núcleo— ni tampoco una decompilación.

> **Este proyecto no incluye ninguna parte del juego.** Necesitas tu propia copia de Monster Hunter Portable 3rd HD Ver. (`NPJB-40001`). La compilación recompila el juego a partir de esa copia en tu equipo, y el resultado no se puede redistribuir.

> Esto es una traducción. Si difiere del [README en inglés](README.md), prevalece el inglés. La documentación detallada está en inglés.

## Aviso legal

**Yakumo** es un proyecto independiente y de código abierto, y no está afiliado, autorizado, patrocinado ni respaldado por CAPCOM, Sony ni ninguna de sus filiales.

Monster Hunter, Monster Hunter Portable 3rd HD Ver., CAPCOM, PlayStation, PSP y todas las marcas comerciales, recursos del juego, ilustraciones, audio, personajes y demás propiedad intelectual relacionados pertenecen a sus respectivos propietarios.

**Yakumo** no incluye ni distribuye el juego original, la imagen de disco, los archivos ejecutables, los datos del juego, las texturas, los modelos, el audio, el vídeo ni otros recursos protegidos por derechos de autor.

Para usar **Yakumo**, los usuarios deben aportar los archivos necesarios a partir de su propia copia adquirida legalmente de Monster Hunter Portable 3rd HD Ver. para PlayStation 3.

Los usuarios son los únicos responsables de obtener, volcar, extraer y usar su copia del juego conforme a las leyes aplicables en su jurisdicción.

**Yakumo** no admite, proporciona, enlaza ni fomenta el uso de copias no autorizadas o piratas del juego.

Cualquier referencia al juego original o a sus marcas comerciales se hace únicamente con fines de identificación, compatibilidad e interoperabilidad.

Las capturas de pantalla y otras representaciones del juego original solo pueden usarse para documentar o mostrar el funcionamiento de **Yakumo**. Todo el contenido de terceros que aparezca sigue siendo propiedad de sus respectivos titulares.

La licencia de **Yakumo** se aplica únicamente al código y los materiales originales del propio proyecto y no concede ningún derecho sobre la propiedad intelectual de terceros.

**Yakumo** proporciona el software, no el juego. Debes aportar tu propia copia adquirida legalmente.

## Estado: casi jugable

Puedes crear un personaje, recorrer la aldea y salir de caza, con sonido, teclado y mando. Todavía no se puede jugar de principio a fin sin asperezas:

| Funciona | Falta o tiene problemas |
| --- | --- |
| Arranque, menús, creación de personaje, partidas guardadas en el formato de la propia PSP | **Iluminación y niebla**: las escenas se ven más planas de lo debido y los marcadores de color sobre los NPC salen blancos |
| La aldea y las zonas de caza | **Ritmo de fotogramas**: el juego aún no está sincronizado con el tiempo real, así que el sonido se adelanta a la imagen |
| Modelos 3D, animación, texturas, transparencias | Superficies curvas, red |
| Efectos de sonido, música y cinemáticas | La aldea va más lenta que otras zonas |
| Teclado, y mandos con cámara real en el stick derecho | |
| Los 355 overlays de código, recompilados | |

Se ha probado en macOS (Apple Silicon, Vulkan a través de MoltenVK) y en una Steam Deck hasta la aldea, donde funciona con Vulkan nativo y los controles integrados. Windows es una plataforma objetivo, pero todavía no se ha verificado.

El estado de cada parte del juego en cada plataforma está en [`docs/COMPATIBILITY.md`](docs/COMPATIBILITY.md).

## Requisitos

- Tu propia copia del juego (ver arriba)
- CMake 3.20 o posterior, Ninja y un compilador de C++20
- Python 3
- SDL3, Vulkan y `glslangValidator`
- Opcional: FFmpeg (`libavcodec`, `libavutil`) para la música y las cinemáticas; sin él, no hay música y las cinemáticas se saltan
- Varios gigabytes de memoria libre para compilar: el código recompilado es grande

## Primeros pasos

En resumen:

1. Enlaza tu imagen de disco y el ejecutable descifrado con el perfil mediante `profiles/mhp3rd/scripts/prepare_game.sh`. El ejecutable debe coincidir con el SHA-256 indicado en el README del perfil.
2. Configura, genera el código recompilado con `profiles/mhp3rd/scripts/generate.sh` y compila `MHP3rdNative`.
3. Recompila los overlays de código con `profiles/mhp3rd/scripts/build_overlays.sh` (unos 40 minutos la primera vez; si se interrumpe, continúa donde se quedó).
4. Ejecuta `out/mhp3rd/bin/MHP3rdNative`.

Las instrucciones completas, con todos los ajustes, están en [`profiles/mhp3rd/README.md`](profiles/mhp3rd/README.md).

## Controles

En un mando, los botones están donde esperas: en un mando de PlayStation el círculo confirma y la equis vuelve atrás, como indican los mensajes del juego, y el stick derecho mueve la cámara. En el teclado, las flechas son la cruceta, I/J/K/L el stick analógico, X y Z son ○ y ✕, A y S son □ y △, Q y W son L y R, y Enter es START. Ambas tablas están en el [README del perfil](profiles/mhp3rd/README.md#running).

Esc, o los dos sticks pulsados a la vez (L3+R3), abre el menú propio de Yakumo: pausa el juego y reúne los ajustes de imagen, sonido y controles, que se guardan para la próxima vez. El primer arranque prepara el juego a partir de tu imagen de disco en la misma ventana, y basta con un mando para hacerlo.

## Hoja de ruta

- Iluminación y niebla
- Ritmo de fotogramas, que también corrige el adelanto del sonido
- Compilaciones verificadas en Steam Deck y Windows

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
- [stb_truetype](https://github.com/nothings/stb): rasterización de fuentes
- [svanheulen/mhef](https://github.com/svanheulen/mhef): documentación de la comunidad sobre el formato de archivo del juego
- [Cinzel](https://github.com/NDISCOVER/Cinzel) y [Shippori Mincho](https://github.com/fontdasu/ShipporiMincho): tipografías del logotipo, bajo la SIL Open Font License

## Licencia

El repositorio se distribuye bajo la licencia MIT; consulta [`LICENSE`](LICENSE). Los archivos de terceros conservan sus propias licencias junto a ellos; actualmente, `profiles/mhp3rd/third_party/stb_truetype.h`, bajo MIT o de dominio público.
