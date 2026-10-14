# Layered armor: how the hunter's look is kept apart from its armor

Layered armor draws the hunter played on this machine in other armor pieces
than it wears, part by part, while the game keeps the real pieces for
everything else. The player's side is in the profile README
([Layered armor](../profiles/mhfu/README.md#layered-armor)); this page says
what the game does, how that was found, and why the rest of the game cannot
see the difference.

Everything here is for USA Freedom Unite, `ULUS-10391`. It was read from
that executable: the model lookup, the armor tables, the load step below, and
the name tables. The text block is at `0x089CC560`. `0x0885148C` reads table
`index` as the word at `(index + 2) * 4`, an offset from that block. Head,
chest, arms, waist and legs are tables 7, 9, 11, 13 and 15 (`0x08851598` and
the direct thunks after it). The equipment box is the 1000 records of 12
bytes at `load32(0x089CC558) + 0x548` (`0x08901788` walks them; `0x08850e58`
makes the count 1000). A layered piece has to be one the hunter wears or
keeps there. A record's byte 1 is the kind, and the box does not number the
parts the way the port's menu does: legs 0, head 1, chest 2, arms 3, waist 4,
blademaster weapons 5 and gunner weapons 6. `0x0885C83C` picks the armor table
by that byte through the jump table at `0x089315AC` (0 legs `0x08973954`, 1
head, 2 chest, 3 arms, 4 waist, then the two weapon tables), and `0x08901788`
turns its equipment classes into the same byte through the bytes at
`0x089B9260`. `0x08850720` writes a record with that kind (below 7). The worn
pieces are kept in the same order, legs first (`0x08850D04`, from
`block + 0x4A0 + 0x2C`). An earlier version compared the byte with the menu's
part number, so it listed another part's pieces under this part's names and
missed the pieces really kept, such as a new hunter's starting armor. Watched
on a loaded hunter, before that was found: the block pointer was
`0x09999DA0`, so the box began at `0x0999A2E8`; slot 128 was a used kind 1
record with id `0x0168`, and slot 137 a kind 4 record with id `0x0166`. Read
with the kinds above, those are a head and a waist; they were not checked
against the screen.

The code is `profiles/mhfu/host/game/layered_armor.{hpp,cpp}` (pure
functions over a `Ram`, tested on made-up tables in
`tests/equipment_models_tests.cpp`) and `layered_armor_hook.cpp` (what is put
in the game's place), and the page is `host/ui/layered_armor_screen.cpp`.

## Where the game decides what a hunter looks like

### The hunter records

The block the game reaches through the pointer at `0x089CC558` holds, from
`+0x30`, a 140-byte record per hunter. A quest does not keep the hunter
played here in record 0. The address is `block + index * 140 + 0x30`, from
`sll` by 3, `subu` of the index, `sll` by 2, `addu`, `sll` by 2
(`0x0885187C`).

| Offset in a record | What |
| --- | --- |
| `+0x10` (u8) | 0 male, otherwise female (`lbu` at `0x088518C0`) |
| `+0x56`, `+0x62`, `+0x6E`, `+0x3E`, `+0x4A` (u16) | Chest, arms, waist, legs and head. The lookup's part is 0 legs, 2 head, 3 chest, 4 arms, 5 waist. Part 1 is not an armor model |

The armor tables are 40 bytes per id, male model then female model as u16,
and the byte at `+4` says who can wear it. Chest `0x08967794`, arms
`0x0896B934`, waist `0x0896F96C`, legs `0x08973954`, head `0x08963374`. The
lookup indexes one with `sll` by 2, `addu` of the id, `sll` by 3. Which slot
a part is comes from the file base at `0x089B662C` (`u16` by sex and part):
both sexes store pac 020 of that slot, part 0 the legs, part 2 the head, part
3 the chest, part 4 the arms and part 5 the waist. `0x08866E10` adds the
model to that base only when `0x08851C3C` says the part has armor. An empty
real head takes the other base, `0x089B6644`, which is the hair.

### The model lookup, 0x0885187C

`0x0885187C` takes the block (a0), a record index (a1) and a part (a2, below
6) and returns a model number: the male or female half, by the record's byte
at `+0x10`, of the armor table's entry at the record's id. It only reads
memory. The executable calls it from `0x08852004`, `0x088C1104`,
`0x088CE47C`, `0x088DA2F0` (once per part, parts 0 to 5), `0x088DBC64` and
`0x088DD18C`:

What those calls draw was read from the instructions:

| Caller | What the instructions do with the model |
| --- | --- |
| `0x088DA2F0` | Asks for parts 0 to 5 and stores each model |
| `0x088C1104` | Asks for part 2 while the load step (`0x088C0E58`) is advancing |
| `0x088DBC64` | Asks inside the file load at `0x088DBBBC`, which then calls `0x088DB4C8` |

A hunter object keeps its record index as a u16 at `+0x1E6`; `0x088CE47C` and
`0x088DD18C` pass that to the lookup. Watched with `MHFU_TRACE_LAYERED_ARMOR=1`
on one arrival in the village and one departure onto a quest. While
`cont_task` is loading the village, the lookup runs from `0x088DBC64` (the
file load), from `0x088DD18C` and from `0x088C1104` (inside the load step).
After `lobby_task` replaces `cont_task`, and again after `game_task` replaces
that on the quest, the lookup runs from `0x088DBC64`, `0x088DA2F0` and
`0x088CE47C`. A walk from the village to the Guild Hall was not watched.

Other code reads the armor tables by id. `0x088516A4` is a second lookup, by
sex, part and id rather than by record; the lobby overlay calls it. Parts 0,
3, 4 and 5 return that id's model and do not read t0. Part 2 returns the
model when t0 is 1 and the id otherwise. Part 1 returns the id. While layered
armor is on, an armor part returns the chosen piece's model for that sex.
The equipment change is the call that returns to `0x09A7E360`: it sets t0
from a halfword at the object `+0x38` (0 for an ordinary piece) and passes
the result to `0x088DB4C8` as the model to read. That read resolves the file
in `0x088D6388` (a1 the part, a2 the model; a nonzero model is returned as
the model to load). The call that returns to `0x09A7E4AC` passes the newly
worn piece's model. While layered armor is on, that lobby call returns the
chosen piece's model instead, and it does not ask the equipment box: that
box is being rewritten by the same change, so a box check there answers
with the piece just put on. `0x088516A4` (a2 the part, a1 the sex, a3 the
armor id) returns the chosen piece's model for that sex when the caller is
the equipment change (return `0x09A7E360`). The other callers keep the
game's answer. `0x088DB4C8` jumps to `0x08866E10` (returns `0x08866EA8` and
`0x08866EFC`). The equipment change comes back to `0x09A7E548`: a3 is the
part, a2 the sex and t0 the model, and while the switch is on t0 is the
chosen model. The function then runs as the game wrote it. The character
load comes back to `0x088DBCCC` and already has the model from `0x0885187C`;
replacing t0 there faulted the reader at `0x24AB8540`. Returning a chosen
model from `0x088D6388` on that read faulted the reader at `0x249D5510`,
including when the part files were left alone. The first character load
keeps the game's return from that read as well: replacing every call
faulted at `0x257A07D0`. Turning the switch off or on releases the part files once, at the display
flip, after the hunter's load has finished. The game's file load then reads
every part, and it keeps the model `0x0885187C` returned. Replacing t0 on
that load faulted at `0x24AB8540`. The equipment change is a second read:
with the switch on, `0x088516A4` returns the chosen model and `0x08866E10`
is entered with that model in t0; with the switch off, those calls run the
game's own function. The equipment calls stay installed after the switch is
turned off.
Part 1 is
still the game's own answer. The record lookup still refuses a piece this
hunter does not own.

### What layered armor changes

While it is on, `0x0885187C` is replaced (the runtime's function table, at the
flip, as the analog camera and the HUD toggle do). For a hunter record and an
armor part with a chosen piece it returns that piece's model for the record's
sex, exactly what the game's function returns for that piece; anything else
runs the game's own code. A quest does not keep the hunter played here in
record 0, so the choice applies to every record this machine draws.
`0x08851C3C` is replaced the same way: *Nothing* reports no armor, and a
chosen piece this sex can wear reports armor, so a helm is read from the
helm files even when the real head is empty. A real part is the game's own
answer. The file and the inner wear of *Nothing* then follow as for a piece
really worn.

A chosen id is used only when that hunter wears it or a used slot in the
equipment box holds it. Anything else is the game's own answer, including a
value written straight into `settings.ini`. It does not write the records,
the character or the equipment box. Turned off, the record lookup is the
game's own function again. The equipment-box lookups stay installed and run
the game's own function until the switch is on, so the next box change can
read the chosen pieces without a restart. Off from the start,
nothing is registered at all. A different executable is left alone: the
replacement is only installed when the code at the traced addresses is the
code described here.

## When a new look shows

A change shows the next time a piece is changed in the equipment box. That
change reads one part's file. With the switch on, the file is the chosen
piece. With the switch off, the file is the piece just equipped. Turning
the switch off and on in the same session changes which of those the next
box change reads. Arriving in an area, or loading into a quest, keeps the
meshes already loaded. Standing still does not rebuild the hunter.

`0x088C0E58` loads the hunter's models. `0x088BCBFC` calls it once and, when
the low byte of the result is 0, returns without finishing the hunter
(`0x088BD734`). `+0x998` is the state: the function returns done only on the
call that makes that word 5 (`xori` by 5, then `sltiu` by 1, at
`0x088C1154`). Entering with the state already 5 does not report done.

Watched on the arrival in the village, before `lobby_task` is installed.
`cont_task` calls `0x088BC7F0` (returns `0x09A5F53C`, then `0x09A5F640`; the
hunter object was `0x09A06BD0`). That dispatches `+12` through the table at
`0x08932378`, and 2 is the model load. The load step then runs from
`0x088BCC04` on that same object, and it calls the file load `0x088DBBBC`
from `0x088C1080` (return `0x088C1088`). The file load's first argument is
not the hunter (`0x09A4F1BC` on that run); its second is the part. The lookup
inside the file load returns to `0x088DBC6C`. After `lobby_task` replaces
`cont_task`, the hunter task and the load step stop. Standing in the village
still asks the lookup, from `0x088DA2F0` and `0x088CE47C`.

Watched on a departure from that village onto a quest. `game_task` is
installed and the stage changes (`stage021` on that run). `0x088BC7F0` and
`0x088C0E58` are not called. The lookup is, from inside the file load
(`0x088DBC64`), from `0x088DA2F0` and from `0x088CE47C`, on the same block
and record 0. The file load's own entry is not entered through the function
table on that path (no call was logged at `0x088DBBBC`), so a wrapper there
does not see the quest. The lookup does, and that is the model the quest
reads. A walk from the village to the Guild Hall was not watched, so whether
`lobby_task` stays installed across that walk is not known.

A part file is read by `0x088DBBBC`. Its progress for one part is the word
at `file_object + 0x24 + part * 4`, and the object is the pointer at
`0x09A4F204` (`0x088DAF18` stores it). Once that word is 8 or more, the
function returns without reading. That is why the first load shows a chosen
piece and a later area or quest keeps the old meshes: `lobby_task` and
`game_task` do call the file load (`0x088DBEE0` steps it), and the call
finds the files already finished.

Setting the hunter's `+0x998` back to 0 does not read those files. State 0
of `0x088C0E58` calls `0x088DDBD8`, and that call drops the face. The port
does not do it. Marking the six part words is not enough on its own: the
quest skips `0x088DBEE0` while bit 2 of the halfword at the model object's
`+0x54` is set (`load32(0x09A4F210)`), and `0x088DBEE0` itself returns
without reading once the word at `+0x3C` is 4 or more. `lobby_task` only
calls that pump when the byte at `load32(0x08A62D7C) + 0x800A4` is 0.
Turning the switch off or on releases the part files once: the six part
words, the pump word, bit 2 of the model object's `+0x54`, and the byte at
`load32(0x08A62D7C) + 0x800A4`. The game's file load then reads every part
and keeps the model `0x0885187C` returned. The equipment change also loads
its own file. `0x08851A68` (a1 the part) returns the model, and the call
that returns to `0x09A7DF24` stores it and passes it to `0x08866E10` from
`0x09A7DF70` as t0. While the switch is on, that lookup returns the chosen
model. While it is off, it runs the game's own function. Returning a
different model from `0x088D6388` on that read faulted the reader. Watched
three times, as layered armor off, change equipment, layered armor on,
change equipment again. Returning the chosen model from the resident read
faulted at `0x249D5510` (`lwc1` at `0x08860060`, from `0x08867980`),
including once when the part files were left alone. Passing the chosen id
into `0x088516A4` and leaving t0 alone did not fault, and the real armor
stayed on screen. Replacing t0 on the character load's entry faulted at
`0x24AB8540`. The port does
not call
`0x088C0E58`, `0x088BC7F0`, `0x088DBBBC`, `0x088DAF18`, `0x0890DEBC` or
`0x0890DF34`.

`0x0890C41C` walks a task list, but the village reaches it by a jump inside
the same compiled unit, so a wrapper on that address never runs.
`0x0890DEBC` and `0x0890DF34` do enter from another unit. Stepping
`0x088C0E58` from the first of those walks, after the switch was turned off,
cleared the pointer at `0x09C190D0` (`0x09C20000 - 28464`). Embarking then
called `0x09B62B80` with that pointer null and read offset `0x502C`. Turning
the switch off does not step that load. It releases the part files, and the
game's own next load draws the real armor. Turning the switch on again
releases them once more, and that load draws the chosen pieces. The
hunter's task word and `+0x998` stay as they are.

The display flip does run in both places. Stepping `0x088C0E58` from that
flip, and calling `0x088BC7F0` itself, both replace the model root
(`0x088DAF18` zeros every slot) and the quest then reads offset `0x648` from
a null part. Calling `0x088DBBBC` and `0x088DB608` from the flip, which store
one new part at `root+12+part*4`, froze the picture while the game kept
running.

Not measured in the game: no scripted run, and the pieces on that departure
were the real armor, so a chosen piece was not watched on screen. Replacing
the model root from the display flip was watched as a crash at guest
`0x08816C54`. Replacing one slot from that flip was watched as a frozen
picture; fast forward and the menu still ran. Turning layered armor off and
embarking was watched as a crash at guest `0x09B62B80` while the task-list
step was in place. That step is no longer taken. The departure watched here,
with layered armor left on, installed `game_task` and did not stop.

## Multiplayer: what other players see

Other players receive the real armor. The replacement does not write the
armor records, so the equipment the game sends is the equipment it wears. On this machine
every hunter record is drawn with the chosen look, because a quest does not
keep the local hunter in record 0. No two-instance session was run.

## Not verified

- A quest started after turning layered armor off, now that the task list no longer steps the load. The crash with that step in place was watched. A departure with the switch left on was watched and did not stop.
- A chosen piece on screen. The village arrival and the quest departure above were traced with the real armor, so the lookup ran and the meshes of a different piece were not watched.
- A walk from the village to the Guild Hall while `lobby_task` stays installed.
- The equipment box on the Equipment screen. The slots above were read from memory on a loaded hunter. Names are read from this executable and were not checked on screen.
- A character load with layered armor on, now that `0x08866E10` leaves t0 alone when the caller returns to `0x088DBCCC`. Replacing t0 on that entry was watched as the crash at `0x24AB8540` while `cont_task` was loading the hunter.
- The redraw after the switch is turned off and on, now that the part files are released once and the character load's t0 is left alone. Replacing t0 on that load was watched as the crash at `0x24AB8540`. Leaving the files in place was watched as no crash and the real armor staying on screen.
- What the equipment change draws after the switch is turned off and on in one session. With the switch on, `0x088516A4` returns the chosen model and `0x08866E10` is entered with that model in t0. With the switch off, those calls run the game's own function. The part files are not released from the display flip. Returning a chosen model from the resident read was watched as the crash at `0x249D5510`, including with the part files left alone. Leaving t0 as the worn model was watched as no crash and the real armor staying on screen. The equipment change was watched once, with the switch left on, and the chosen meshes showed. A boot that replaced every call of `0x088D6388` faulted at `0x257A07D0`. Off, then a box change, then on, then a box change, was not watched on this build.
- A head on screen: an empty real head now reports armor so the helm files are used. No helm was watched.
- An ad hoc session with layered armor on.
