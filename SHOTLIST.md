# README capture checklist

Captures for the front-page README. Save them in `docs/images/readme/` under the file names given here, and the README can reference them as they land.

## Before you start

- [ ] **One look for every capture.** Pick these once and keep them for the whole set, so the page looks like one game:
  - Window 1920×1080 (or fullscreen at 1080p), *Aspect ratio* Fill, *Resolution* ×4 or higher.
  - HD texture pack: either on for every shot or off for every shot.
  - *Frame rate* 60 (GIFs look smoother when the source is smooth).
- [ ] **A clean screen.** Turn off the performance overlay and anything you use only for debugging. Close the Pokee menu before each capture, unless the menu is the subject.
- [ ] **Names.** Use a hunter name you're happy to have on the front page. In multiplayer shots, ask the other players first, or have every hunter be one of your own instances.
- [ ] **One gear set.** Wear one good-looking armor set and keep it on in every shot, unless the shot is about changing equipment.
- [ ] **Tools.**
  - Stills: Pokee's *Take a screenshot* (System section) or its screenshot key. Files go to the screenshots folder.
  - GIFs: ShareX → *Screen recording (GIF)*, region set to the game window. Keep each one 3–8 seconds, about 960 px wide, 15–20 fps, and under ~8 MB.
  - Longer clips (frame rate, multiplayer): record MP4 and attach it to the README on GitHub instead of committing it.
- [ ] **Before/after pairs.** For a feature with an off switch, capture the same spot twice, once with it off and once with it on, without moving the camera. Name the files `<name>-off` and `<name>-on`.

## Requested features

### Item Sets (Mods → Item Sets)
- [ ] **GIF** `item-sets.gif`: at the village item box, open *Item Sets* → *Load Item Set*. Page through a set's items with left/right, choose it, answer *Yes*, then show the pouch filling.
- [ ] **Still** `item-sets-list.png`: the set list with a well-stocked set highlighted (potions, mega potions, whetstones, traps, bombs) so the pouch window beside it is full.
- [ ] Tip: register a few named-looking sets first (a hunting set, a capture set, a gathering set) so the list isn't just Set 1.

### Damage Numbers (Game → Damage numbers)
- [ ] **GIF** `damage-numbers.gif`: a large monster (Tigrex, Rathalos, Nargacuga) hit repeatedly on a weak spot. Use *Mine*, *Damage breakdown* Split, and an element weapon so the coloured element glyph shows. Get at least one orange (weak-spot) number and one critical sparkle in frame.
- [ ] **GIF** `damage-numbers-gun.gif` (optional): bow or bowgun shots landing on different parts, with the numbers stacking where each shot struck.
- [ ] **Still or GIF** `damage-numbers-online.gif` (optional): two hunters hitting the same monster with *Everyone*, showing the other player's numbers.
- [ ] Tip: get close enough that the numbers read clearly at 960 px wide. Try *Damage number size* 120–150 for the capture.

### Eating in the Guild Hall (Mods → Eat in the Guild Hall)
- [ ] **Still** `hall-meal-prompt.png`: seated at a hall table, with the *:Stand Up* / *:Order Food* rows at the top left in frame.
- [ ] **GIF** `hall-meal.gif`: sit, press △, the Felyne Kitchen menu opens, pick a meal, the skill message shows.

### Change Equipment in the Guild Hall, online (Mods → Change Equipment in the Guild Hall)
- [ ] **GIF** `hall-equipment.gif`, the key one: two instances in the same online hall. Hunter A opens the hall item box → *Change Equipment* → swaps to a very different-looking armor set. Then show Hunter B's window with A appearing in the new armor.
- [ ] **Still** `hall-equipment-menu.png`: the hall item box with *Change Equipment* and its submenu open.
- [ ] Tip: record both windows side by side (each 960×540) so the other player's update is in the same clip.

### UI Scaling (Video → UI Scaling)
- [ ] **Before/after stills** `ui-scaling-off.png` / `ui-scaling-on.png`: the same moment in a quest, at 100% everywhere and then with a layout you'd actually play with (for example Map 120, Status 85, Items 110, Names 80).
- [ ] **GIF** `ui-scaling.gif` (optional): the menu open over a quest, dragging the Map or Status slider and the HUD resizing live behind it. *Pause the game when the menu opens* has to be off so the HUD stays visible.
- [ ] Tip: a busy hunt HUD shows this best: a monster found (yellow eye), a status icon, a long sword spirit gauge or bowgun ammo, and the big map.

### Modern Sharpness Gauge (Game → Modern sharpness gauge)
- [ ] **Before/after stills** `sharpness-off.png` / `sharpness-on.png`: a crop of the top-left status corner, with the gauge partly used up.
- [ ] **GIF** `sharpness.gif`: a long sword (it also has the bar under the blade). Hit a monster until the colour visibly shortens and drops to the next colour, then sharpen and watch it refill.
- [ ] Tip: crop to the status corner and scale it up, since the gauge is tiny at full-frame size. Capture orange or yellow too; they get a colour only with the gauge on.

### Fast item selection: Item left/right and Empty item slot
I couldn't find a feature called "fast item selection" in the code or history. These two are the closest; correct me if you meant something else.
- [ ] **GIF** `item-bar.gif`: scroll the item bar with one button (mouse side buttons or a back paddle bound to *Item left/right*) and stop on *Nothing Selected* past the last item. Show the input on screen if you can (a pad overlay, or a caption).
- [ ] **Still** `empty-item-slot.png`: the item bar at the start of a quest, reading *Nothing Selected*.

## Other features of yours worth a shot

### Gather icon and Hold to gather (Game → Gather icon, Hold to gather)
- [ ] **Still** `gather-icon.png`: a gather point showing the item picture in the disc. Best is two marks side by side, such as a bone pile next to a carvable body.
- [ ] **GIF** `hold-to-gather.gif`: hold ○ on a point and the hunter gathers repeatedly until it runs out. Alternatively, carve a monster with one hold.

### True Attack Value (Game → True Attack Value)
- [ ] **Before/after stills** `true-attack-off.png` / `true-attack-on.png`: the same great sword or hammer on the equipment screen, for example 672 → 140, with element shown too.

### Key quests (Mods → Show Key Quests)
- [ ] **Still** `key-quests.png`: a village or guild quest list with two or three *[Key]* entries visible.

### Field of view (Video → Field of view)
- [ ] **Before/after stills** `fov-50.png` / `fov-75.png`: the same spot in a wide hunting area (Snowy Mountains peak, Great Forest) at the game's 50° and at a wider setting.

### Lock-on and the analog camera (Game → Lock-on; Controls → Analog camera)
- [ ] **GIF** `lock-on.gif`: lock onto a large monster with the red ring marker. The camera follows it while you move around it, then a tap recenters.
- [ ] **GIF** `bowgun-aim.gif` (optional): aiming a bowgun with the right stick.

### Pokee's menu in the game's style
- [ ] **Still** `menu.png`: the menu open over an attractive in-game moment rather than the title screen, on the Game section (it shows several of your switches at once).

### Hide HUD (Hide HUD key)
- [ ] **Still** `photo-mode.png`: a scenic shot with the HUD hidden. This makes a strong hero image for the top of the page.

### Multiplayer
- [ ] **Still** `hall-party.png`: a full Guild Hall with the party list.
- [ ] **Still** `hunt-party.png`: a party fighting one monster together, ideally with damage numbers on.

## When you're done
- [ ] Put the files in `docs/images/readme/` and tell me which ones made it; I'll rewire the README around them.
- [ ] Nothing else to remove: the earlier picks are already out of the branch.
