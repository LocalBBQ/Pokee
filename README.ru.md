<p align="center"><a href="README.md">English</a> · <b>Русский</b> · <a href="README.es.md">Español</a></p>

<p align="center"><img src="docs/images/emblem.svg" width="120" alt="Эмблема Pokee"></p>

<h1 align="center">Pokee</h1>

<p align="center"><b>Monster Hunter Freedom Unite, нативно на вашем ПК.</b></p>

<p align="center">
  <a href="https://github.com/LocalBBQ/Pokee/releases/latest"><b>Скачать для Windows</b></a> ·
  <a href="docs/LINUX.md">Linux / Steam Deck</a> ·
  <a href="docs/MACOS.md">macOS</a> ·
  <a href="docs/BUILDING.md">Сборка из исходников</a>
</p>

> Pokee не содержит игру. Нужен собственный образ диска *Monster Hunter Freedom Unite* (США, `ULUS-10391`, v1.01).

> Это перевод. Если он расходится с [английским README](README.md), правильным считается английский.

## Установка на Windows

1. Скачайте zip со страницы [релизов](https://github.com/LocalBBQ/Pokee/releases/latest) и распакуйте его, например в `C:\Games\Pokee` (не в `Program Files`).
2. Добавьте эту папку в исключения Защитника Windows. Защитник проверяет все 290 DLL Pokee при каждом запуске, и запуск может занимать минуты. В PowerShell от имени администратора:
   ```powershell
   Add-MpPreference -ExclusionPath "C:\Games\Pokee"
   ```
3. Запустите `Pokee.exe` и выберите свой `.iso`.

**Esc** или **L3 + R3** открывают меню Pokee.

## Возможности

| | |
| --- | --- |
| **[Наборы предметов](profiles/mhfu/README.md#item-sets)** | Сохраняйте подсумок и загружайте его из сундука |
| **[Числа урона](profiles/mhfu/README.md#damage-numbers)** | Урон каждого удара там, куда он пришёлся, и в сетевой игре |
| **[Еда в зале гильдии](profiles/mhfu/README.md#eating-in-the-guild-hall)** | Заказ еды за любым столом зала |
| **Смена снаряжения онлайн** | Меняйте снаряжение из сундука в зале гильдии |
| **[Масштаб интерфейса](profiles/mhfu/README.md#ui-scaling)** | Отдельный размер карты, статуса, предметов и имён |
| **[Современная шкала остроты](profiles/mhfu/README.md#modern-sharpness-gauge)** | Острота, которая заметно убывает |
| **[Быстрый выбор предметов](profiles/mhfu/README.md#item-left-and-right)** | Прокрутка предметов одной кнопкой и [пустой слот](profiles/mhfu/README.md#empty-item-slot) |
| **[Значки сбора](profiles/mhfu/README.md#gather-icon)** | Видно, что даёт точка; удерживайте, чтобы собирать дальше |
| **Истинная атака** | Настоящие значения оружия на экранах снаряжения |
| **[Ключевые задания](profiles/mhfu/README.md#key-quests)** | Отмечены в списках заданий |
| **Графика** | Широкий экран, разрешение до ×6, 60+ кадров/с, угол обзора, [HD-текстуры](profiles/mhfu/README.md#hd-texture-packs) |
| **[Сетевая игра](profiles/mhfu/README.md#multiplayer-ad-hoc)** | Ad hoc по локальной сети или через интернет |

Все настройки и управление: [README профиля](profiles/mhfu/README.md). Как это работает: [ARCHITECTURE.md](docs/ARCHITECTURE.md). Проблемы: [issues](https://github.com/LocalBBQ/Pokee/issues).

<details>
<summary><b>Благодарности</b></summary>

Основан на [Yakumo](https://github.com/TeamGDB/Yakumo), порте Portable 3rd от [@MHunterG](https://github.com/MHunterG), [@mojitosunrise](https://github.com/mojitosunrise) и участников, на фреймворке [PSPRecomp](https://github.com/jessicanataliagta/PSPRecomp).

Использует [SDL3](https://www.libsdl.org/), [FFmpeg](https://ffmpeg.org/), [Vulkan](https://www.vulkan.org/), [MoltenVK](https://github.com/KhronosGroup/MoltenVK), [Dear ImGui](https://github.com/ocornut/imgui), [tiny-AES-c](https://github.com/kokke/tiny-AES-c) и [stb_truetype](https://github.com/nothings/stb); документация формата архива — [svanheulen/mhef](https://github.com/svanheulen/mhef); шрифты логотипа — [Cinzel](https://github.com/NDISCOVER/Cinzel) и [Shippori Mincho](https://github.com/fontdasu/ShipporiMincho) (SIL Open Font License).

</details>

<details>
<summary><b>Правовая оговорка</b></summary>


**Pokee** — независимый проект с открытым исходным кодом. Он не связан с CAPCOM, Sony или их аффилированными лицами, не уполномочен, не спонсируется и не одобрен ими.

Monster Hunter, Monster Hunter Freedom Unite, CAPCOM, PlayStation, PSP и все связанные с ними товарные знаки, игровые ресурсы, изображения, звук, персонажи и иная интеллектуальная собственность принадлежат их законным владельцам.

**Pokee** не содержит никаких ресурсов игры и оригинальных файлов игры: ни образа диска, ни копии исполняемого файла или данных игры, ни текстур, моделей, звука и видео из неё. Чтобы установить или собрать **Pokee**, нужны файлы из вашей собственной законно полученной копии Monster Hunter Freedom Unite (США); установщик проверяет эту копию и принимает только `ULUS-10391` версии 1.01.

Пользователи сами несут ответственность за то, чтобы получать, снимать дамп, извлекать и использовать свою копию игры в соответствии с законами, действующими в их юрисдикции.

**Pokee** не поддерживает, не предоставляет, не ссылается на нелицензионные или пиратские копии игры и не поощряет их использование.

Любые упоминания оригинальной игры и её товарных знаков служат исключительно для идентификации, совместимости и взаимодействия.

Снимки экрана и иные изображения оригинальной игры могут использоваться только для документирования или демонстрации возможностей **Pokee**. Весь изображённый сторонний игровой контент остаётся собственностью его правообладателей.

Лицензия **Pokee** распространяется только на собственный оригинальный код и материалы проекта и не даёт никаких прав на интеллектуальную собственность третьих лиц.


</details>

Лицензия MIT; см. [LICENSE](LICENSE) и [SOURCE_PROVENANCE.md](docs/SOURCE_PROVENANCE.md) об уведомлениях третьих сторон.
