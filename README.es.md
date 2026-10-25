<p align="center"><a href="README.md">English</a> · <a href="README.ru.md">Русский</a> · <b>Español</b></p>

<p align="center">
  <a href="https://github.com/LocalBBQ/Pokee"><img src="https://img.shields.io/badge/source-LocalBBQ%2FPokee-b98335?style=flat-square&amp;labelColor=2a1a22" alt="Source: LocalBBQ/Pokee"></a>
  <a href="https://github.com/LocalBBQ/Pokee/actions/workflows/tests.yml?query=branch%3Amain"><img src="https://img.shields.io/github/actions/workflow/status/LocalBBQ/Pokee/tests.yml?branch=main&amp;event=push&amp;style=flat-square&amp;label=tests&amp;labelColor=2a1a22" alt="Unit tests on main"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-b98335?style=flat-square&amp;labelColor=2a1a22" alt="License: MIT"></a>
  <a href="docs/BUILDING.md"><img src="https://img.shields.io/badge/platforms-Windows%20%C2%B7%20Linux%20%C2%B7%20macOS%20%C2%B7%20Android-b98335?style=flat-square&amp;labelColor=2a1a22" alt="Build targets: Windows, Linux, macOS and Android"></a>
  <a href="https://github.com/LocalBBQ/Pokee/stargazers"><img src="https://img.shields.io/github/stars/LocalBBQ/Pokee?style=flat-square&amp;label=stars&amp;labelColor=2a1a22&amp;color=b98335" alt="GitHub stars"></a>
</p>

<p align="center"><img src="docs/images/emblem.svg" width="120" alt="Emblema de Pokee"></p>

<h1 align="center">Pokee</h1>

<p align="center"><b>Monster Hunter Freedom Unite, nativo en tu PC.</b><br>
Pantalla panorámica, alta resolución, fotogramas fluidos, texturas HD, ratón y mando de verdad, y caza con amigos.</p>

<p align="center">
  <a href="https://github.com/LocalBBQ/Pokee/releases/latest"><b>Descargar para Windows</b></a> ·
  <a href="docs/LINUX.md">Linux y Steam Deck</a> ·
  <a href="docs/MACOS.md">macOS</a> ·
  <a href="docs/BUILDING.md">Compilar desde el código</a>
</p>

<p align="center"><img src="docs/images/readme/village.jpg" alt="La aldea Pokke en panorámico y alta resolución"></p>

> **Pokee no incluye el juego.** Necesitas la imagen de disco de tu propia copia de *Monster Hunter Freedom Unite* (EE. UU., `ULUS-10391`, versión 1.01). Consulta el [aviso legal](#aviso-legal).

> Esto es una traducción. Si difiere del [README en inglés](README.md), prevalece el inglés. La documentación detallada está en inglés.

## Ponerlo en marcha en Windows

1. **Descarga** el zip más reciente de la [página de versiones](https://github.com/LocalBBQ/Pokee/releases/latest).
   - `pokee-…-windows-x86_64.zip` guarda la configuración y las partidas en `%APPDATA%\Pokee\MHFU`.
   - `pokee-…-windows-x86_64-portable.zip` lo guarda todo en una carpeta `data` junto a `Pokee.exe`, así que puede ir en una memoria USB.
2. **Extrae la carpeta entera** en un lugar donde se pueda escribir, como `C:\Games\Pokee`. No en `Program Files`.
3. **Añade esa carpeta a las exclusiones de Windows Defender.** Pokee carga sus 290 bibliotecas de código recompilado (`overlays\*.dll`) cada vez que arranca, y Defender analiza cada una antes de dejarla cargar. Sin la exclusión, el arranque puede tardar minutos en vez de segundos.
   - **Seguridad de Windows** → **Protección antivirus y contra amenazas** → **Administrar la configuración** → **Exclusiones: Agregar o quitar exclusiones** → **Agregar una exclusión** → **Carpeta** → elige la carpeta de Pokee.
   - O bien, en PowerShell ejecutado como administrador:

     ```powershell
     Add-MpPreference -ExclusionPath "C:\Games\Pokee"
     ```

   Excluye solo una carpeta que hayas extraído de la página de versiones de este repositorio.
4. **Ejecuta `Pokee.exe`** y elige tu imagen de disco (`.iso`) cuando la pida. Pokee la comprueba y después la copia (unos 1,3 GB, para que luego puedas mover el original) o la usa donde está. Un mando basta para toda la configuración.
5. **Juega.** Los siguientes arranques van directos al juego. Pulsa **Esc**, o **L3 + R3** en el mando, para abrir el menú de Pokee.

Necesitas Windows de 64 bits y un controlador gráfico compatible con Vulkan. SDL3, FFmpeg y el runtime de Visual C++ vienen en el zip.

¿Ya tienes una partida en una PSP? [Impórtala](profiles/mhfu/README.md#importing-a-save-from-a-psp) y sigue donde lo dejaste.

## Más nítido, más ancho, más fluido

<table>
  <tr>
    <td><img src="docs/images/readme/forest.jpg" alt="Vista de las montañas entre los árboles en panorámico"></td>
    <td><img src="docs/images/readme/coast.jpg" alt="Un Hermitaur en la costa en panorámico"></td>
  </tr>
</table>

Renderiza hasta seis veces la resolución de la PSP, llena una pantalla panorámica, amplía el campo de visión y deja que la interpolación de fotogramas lleve los 30 fps del juego hasta la frecuencia de tu monitor. Los [paquetes de texturas HD](profiles/mhfu/README.md#hd-texture-packs) compatibles con PPSSPP se instalan tal cual; las capturas de esta página usan uno.

## Cazad juntos

<table>
  <tr>
    <td><img src="docs/images/readme/guild-hall.jpg" alt="Cuatro cazadores en el salón del gremio"></td>
    <td><img src="docs/images/readme/hunt-together.jpg" alt="Tres cazadores en una misión en el desierto"></td>
  </tr>
</table>

El salón del gremio funciona como en la PSP: juego ad hoc entre jugadores de Pokee en la misma red, o por internet a través de un servidor ad hoc. Crea o únete a una sesión desde la sección Red del menú. [Cómo configurarlo](profiles/mhfu/README.md#multiplayer-ad-hoc)

## Menos menús, más caza

<table>
  <tr>
    <td><img src="docs/images/readme/item-sets.jpg" alt="Conjuntos de objetos en el cofre"></td>
    <td><img src="docs/images/readme/hall-equipment.jpg" alt="Cambiar el equipo desde el cofre del salón del gremio"></td>
  </tr>
</table>

Añadidos opcionales que respetan el aspecto del propio juego:

- **[Conjuntos de objetos](profiles/mhfu/README.md#item-sets)**: guarda tu bolsa y vuelve a cargarla con una pulsación.
- **Cambiar el equipo en el salón del gremio**, y **[pedir comida desde el asiento](profiles/mhfu/README.md#eating-in-the-guild-hall)** allí mismo.
- **[Números de daño](profiles/mhfu/README.md#damage-numbers)** donde cae cada golpe, compartidos con los demás jugadores de Pokee de tu cacería.
- **Valor de ataque real** de las armas, su elemento y su estado en las pantallas de equipo.
- **[Iconos de recolección](profiles/mhfu/README.md#gather-icon)** que muestran lo que da cada punto, y **recolectar manteniendo pulsado**.
- **[Misiones clave](profiles/mhfu/README.md#key-quests)** marcadas en las listas de misiones, y un **[indicador de filo moderno](profiles/mhfu/README.md#modern-sharpness-gauge)**.
- **[Armadura por capas](profiles/mhfu/README.md#layered-armor)**: el aspecto de un conjunto con las estadísticas de otro.

## Un menú para todo

<table>
  <tr>
    <td><img src="docs/images/readme/menu-video.jpg" alt="Menú de Pokee, sección Vídeo"></td>
    <td><img src="docs/images/readme/menu-mods.jpg" alt="Menú de Pokee, sección Mods"></td>
  </tr>
</table>

Vídeo, audio, controles, opciones de juego, multijugador y mods están en un solo menú sobre el juego, y cada cambio se aplica al momento. Funciona con mando, teclado o ratón. [Todos los ajustes](profiles/mhfu/README.md#in-game-menu)

**Controles.** En el mando, el stick derecho gira la cámara y apunta, y los botones coinciden con las indicaciones del juego. Elige un preajuste (Default, Modern con los ataques en los gatillos, Left-handed, Classic keyboard) o reasigna lo que quieras, incluidas combinaciones como L1 + ○. El teclado y el ratón están totalmente soportados: WASD para moverse, el ratón para la cámara y sus botones para atacar. [Tablas de controles](profiles/mhfu/README.md#running)

## Estado

El juego se puede jugar desde un personaje nuevo o una partida importada de PSP: la aldea, el salón del gremio y las cacerías, con música, vídeos y partidas en el formato de la PSP.

| Funciona | Pendiente |
| --- | --- |
| Todo el juego: aldea, salón del gremio, cacerías, música y vídeos | Los diálogos de partida guardada y de mensajes no dibujan nada y responden como si se hubieran confirmado ([upstream #33](https://github.com/TeamGDB/Yakumo/issues/33)) |
| Partidas en formato PSP y misiones descargadas; importación, exportación y copias de seguridad | Modo de descarga (`sceHttp`, `sceNetInet`) |
| Juego ad hoc entre instancias de Pokee, en LAN o mediante un servidor ad hoc | Jugar con PPSSPP y en servidores públicos |
| [Mods](profiles/mhfu/README.md#mods) de reemplazo de archivos y paquetes de texturas HD | Mods de código |

Las versiones para Windows están en la página de versiones. Linux, Steam Deck, macOS y Android se compilan desde el código; consulta [LINUX.md](docs/LINUX.md), [MACOS.md](docs/MACOS.md) y [BUILDING.md](docs/BUILDING.md). ¿Has encontrado un problema? [Abre una incidencia](https://github.com/LocalBBQ/Pokee/issues) con tu plataforma y lo que estabas haciendo.

## Compilar desde el código

Necesitas tu propia copia del juego, CMake 3.20+, Ninja, un compilador de C++20, Python 3, SDL3, Vulkan y `glslangValidator`, además de unos cuantos gigabytes de memoria libre. En resumen:

1. Compila `Pokee` y ejecuta `Pokee --install /ruta/a/la/imagen.iso` para preparar el ejecutable del juego a partir de tu imagen.
2. Genera el código recompilado con `profiles/mhfu/scripts/generate.sh` y compila `Pokee`.
3. Recompila los overlays de código con `profiles/mhfu/scripts/build_overlays.sh` (unos 40 minutos la primera vez; se puede reanudar).
4. Ejecuta `out/mhfu/bin/Pokee`.

[docs/BUILDING.md](docs/BUILDING.md) cubre todas las plataformas y cómo evitar recompilaciones completas. [CONTRIBUTING.md](CONTRIBUTING.md) explica cómo contribuir.

## Cómo funciona

Pokee no emula el procesador de la PSP mientras juegas. El código MIPS del juego se traduce a C++ por adelantado y se compila dentro del programa, junto con los 290 overlays de código que el juego carga mientras se ejecuta. Un intérprete cubre lo que el código recompilado no alcanza. Alrededor de ese código hay una reimplementación del sistema de la PSP: núcleo, lectura del disco, un renderizador Vulkan para el motor gráfico, mezcla de audio, entrada, partidas guardadas y red ad hoc.

[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) describe el modelo de ejecución. Pokee se basa en [PSPRecomp](https://github.com/jessicanataliagta/PSPRecomp), un framework de recompilación estática independiente del juego; [docs/PROFILE_GUIDE.md](docs/PROFILE_GUIDE.md) explica cómo adaptar otro título.

## Autores

[Pokee](https://github.com/LocalBBQ/Pokee) parte de [Yakumo](https://github.com/TeamGDB/Yakumo), el port de Portable 3rd. Ese proyecto y quienes contribuyeron a él conservan el crédito del host del que sale este árbol:

- [@MHunterG](https://github.com/MHunterG)
- [@mojitosunrise](https://github.com/mojitosunrise)

## Créditos

- [Yakumo](https://github.com/TeamGDB/Yakumo): el port original, de sus colaboradores
- [PSPRecomp](https://github.com/jessicanataliagta/PSPRecomp): el framework de recompilación sobre el que se construye el proyecto
- [SDL3](https://www.libsdl.org/): ventana, entrada y salida de audio
- [FFmpeg](https://ffmpeg.org/): decodificación de la música y el vídeo
- [Vulkan](https://www.vulkan.org/) y [MoltenVK](https://github.com/KhronosGroup/MoltenVK): renderizado
- [Dear ImGui](https://github.com/ocornut/imgui): menús y pantallas de configuración inicial de Pokee
- [tiny-AES-c](https://github.com/kokke/tiny-AES-c): instalador y compatibilidad con las partidas guardadas de PSP
- [stb_truetype](https://github.com/nothings/stb): rasterización de fuentes
- [svanheulen/mhef](https://github.com/svanheulen/mhef): documentación de la comunidad sobre el formato de archivo del juego
- [Cinzel](https://github.com/NDISCOVER/Cinzel) y [Shippori Mincho](https://github.com/fontdasu/ShipporiMincho): tipografías del logotipo, bajo la SIL Open Font License

## Aviso legal

**Pokee** es un proyecto independiente y de código abierto, y no está afiliado, autorizado, patrocinado ni respaldado por CAPCOM, Sony ni ninguna de sus filiales.

Monster Hunter, Monster Hunter Freedom Unite, CAPCOM, PlayStation, PSP y todas las marcas comerciales, recursos del juego, ilustraciones, audio, personajes y demás propiedad intelectual relacionados pertenecen a sus respectivos propietarios.

**Pokee** no incluye ningún recurso del juego ni archivos originales del juego: ni la imagen de disco, ni una copia del ejecutable o de los datos del juego, ni texturas, modelos, audio o vídeo del juego. Para instalar o compilar **Pokee** debes aportar los archivos de tu propia copia legal de Monster Hunter Freedom Unite (EE. UU.); el instalador comprueba esa copia y solo acepta `ULUS-10391` versión 1.01.

Para usar **Pokee**, los usuarios deben aportar los archivos necesarios a partir de su propia copia adquirida legalmente de Monster Hunter Freedom Unite (EE. UU.) para PSP.

Los usuarios son los únicos responsables de obtener, volcar, extraer y usar su copia del juego conforme a las leyes aplicables en su jurisdicción.

**Pokee** no admite, proporciona, enlaza ni fomenta el uso de copias no autorizadas o piratas del juego.

Cualquier referencia al juego original o a sus marcas comerciales se hace únicamente con fines de identificación, compatibilidad e interoperabilidad.

Las capturas de pantalla y otras representaciones del juego original solo pueden usarse para documentar o mostrar el funcionamiento de **Pokee**. Todo el contenido de terceros que aparezca sigue siendo propiedad de sus respectivos titulares.

La licencia de **Pokee** se aplica únicamente al código y los materiales originales del propio proyecto y no concede ningún derecho sobre la propiedad intelectual de terceros.

**Pokee** proporciona el software, no el juego. Debes aportar tu propia copia adquirida legalmente.

## Licencia

El repositorio se distribuye bajo la licencia MIT; consulta [`LICENSE`](LICENSE). Los archivos de terceros conservan sus licencias junto a ellos; [`docs/SOURCE_PROVENANCE.md`](docs/SOURCE_PROVENANCE.md) enumera sus orígenes y licencias.
