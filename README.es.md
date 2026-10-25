<p align="center"><a href="README.md">English</a> · <a href="README.ru.md">Русский</a> · <b>Español</b></p>

<p align="center"><img src="docs/images/emblem.svg" width="120" alt="Emblema de Pokee"></p>

<h1 align="center">Pokee</h1>

<p align="center"><b>Monster Hunter Freedom Unite, nativo en tu PC.</b></p>

<p align="center">
  <a href="https://github.com/LocalBBQ/Pokee/releases/latest"><b>Descargar para Windows</b></a> ·
  <a href="docs/LINUX.md">Linux / Steam Deck</a> ·
  <a href="docs/MACOS.md">macOS</a> ·
  <a href="docs/BUILDING.md">Compilar desde el código</a>
</p>

> Pokee no incluye el juego. Necesitas tu propia imagen de disco de *Monster Hunter Freedom Unite* (EE. UU., `ULUS-10391`, v1.01).

> Esto es una traducción. Si difiere del [README en inglés](README.md), prevalece el inglés.

## Instalar en Windows

1. Descarga el zip de [Versiones](https://github.com/LocalBBQ/Pokee/releases/latest) y extráelo, por ejemplo en `C:\Games\Pokee` (no en `Program Files`).
2. Añade esa carpeta a las exclusiones de Windows Defender. Defender analiza las 290 DLL de Pokee en cada arranque, y el arranque puede tardar minutos. En PowerShell como administrador:
   ```powershell
   Add-MpPreference -ExclusionPath "C:\Games\Pokee"
   ```
3. Ejecuta `Pokee.exe` y elige tu `.iso`.

**Esc** o **L3 + R3** abren el menú de Pokee.

## Funciones

| | |
| --- | --- |
| **[Conjuntos de objetos](profiles/mhfu/README.md#item-sets)** | Guarda tu bolsa y cárgala desde el cofre |
| **[Números de daño](profiles/mhfu/README.md#damage-numbers)** | El daño de cada golpe donde cae, también en línea |
| **[Comer en el salón del gremio](profiles/mhfu/README.md#eating-in-the-guild-hall)** | Pide comida en cualquier mesa del salón |
| **Cambiar equipo en línea** | Cambia de equipo desde el cofre del salón del gremio |
| **[Escala de la interfaz](profiles/mhfu/README.md#ui-scaling)** | Tamaño propio para mapa, estado, objetos y nombres |
| **[Indicador de filo moderno](profiles/mhfu/README.md#modern-sharpness-gauge)** | Un filo que se ve gastarse |
| **[Selección rápida de objetos](profiles/mhfu/README.md#item-left-and-right)** | Recorre los objetos con un botón, y una [casilla vacía](profiles/mhfu/README.md#empty-item-slot) |
| **[Iconos de recolección](profiles/mhfu/README.md#gather-icon)** | Lo que da cada punto; mantén pulsado para seguir recolectando |
| **Valor de ataque real** | Los valores reales del arma en las pantallas de equipo |
| **[Misiones clave](profiles/mhfu/README.md#key-quests)** | Marcadas en las listas de misiones |
| **Gráficos** | Panorámico, hasta 6× de resolución, más de 60 fps, campo de visión, [texturas HD](profiles/mhfu/README.md#hd-texture-packs) |
| **[Multijugador](profiles/mhfu/README.md#multiplayer-ad-hoc)** | Ad hoc por LAN o por internet |

Todos los ajustes y controles: [README del perfil](profiles/mhfu/README.md). Cómo funciona: [ARCHITECTURE.md](docs/ARCHITECTURE.md). Problemas: [incidencias](https://github.com/LocalBBQ/Pokee/issues).

<details>
<summary><b>Créditos</b></summary>

Basado en [Yakumo](https://github.com/TeamGDB/Yakumo), el port de Portable 3rd de [@MHunterG](https://github.com/MHunterG), [@mojitosunrise](https://github.com/mojitosunrise) y colaboradores, sobre el framework [PSPRecomp](https://github.com/jessicanataliagta/PSPRecomp).

Usa [SDL3](https://www.libsdl.org/), [FFmpeg](https://ffmpeg.org/), [Vulkan](https://www.vulkan.org/), [MoltenVK](https://github.com/KhronosGroup/MoltenVK), [Dear ImGui](https://github.com/ocornut/imgui), [tiny-AES-c](https://github.com/kokke/tiny-AES-c) y [stb_truetype](https://github.com/nothings/stb); documentación del formato de archivo de [svanheulen/mhef](https://github.com/svanheulen/mhef); tipografías del logotipo [Cinzel](https://github.com/NDISCOVER/Cinzel) y [Shippori Mincho](https://github.com/fontdasu/ShipporiMincho) (SIL Open Font License).

</details>

<details>
<summary><b>Aviso legal</b></summary>


**Pokee** es un proyecto independiente y de código abierto, y no está afiliado, autorizado, patrocinado ni respaldado por CAPCOM, Sony ni ninguna de sus filiales.

Monster Hunter, Monster Hunter Freedom Unite, CAPCOM, PlayStation, PSP y todas las marcas comerciales, recursos del juego, ilustraciones, audio, personajes y demás propiedad intelectual relacionados pertenecen a sus respectivos propietarios.

**Pokee** no incluye ningún recurso del juego ni archivos originales del juego: ni la imagen de disco, ni una copia del ejecutable o de los datos del juego, ni texturas, modelos, audio o vídeo del juego. Para instalar o compilar **Pokee** debes aportar los archivos de tu propia copia legal de Monster Hunter Freedom Unite (EE. UU.); el instalador comprueba esa copia y solo acepta `ULUS-10391` versión 1.01.

Los usuarios son los únicos responsables de obtener, volcar, extraer y usar su copia del juego conforme a las leyes aplicables en su jurisdicción.

**Pokee** no admite, proporciona, enlaza ni fomenta el uso de copias no autorizadas o piratas del juego.

Cualquier referencia al juego original o a sus marcas comerciales se hace únicamente con fines de identificación, compatibilidad e interoperabilidad.

Las capturas de pantalla y otras representaciones del juego original solo pueden usarse para documentar o mostrar el funcionamiento de **Pokee**. Todo el contenido de terceros que aparezca sigue siendo propiedad de sus respectivos titulares.

La licencia de **Pokee** se aplica únicamente al código y los materiales originales del propio proyecto y no concede ningún derecho sobre la propiedad intelectual de terceros.


</details>

Licencia MIT; consulta [LICENSE](LICENSE) y [SOURCE_PROVENANCE.md](docs/SOURCE_PROVENANCE.md) para los avisos de terceros.
