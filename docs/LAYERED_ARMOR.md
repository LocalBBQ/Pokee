# Layered armor: how the hunter's look is kept apart from its armor

Layered armor draws the hunter played on this machine in other armor pieces
than it wears, part by part, while the game keeps the real pieces for
everything else. The player's side is in the profile README
([Layered armor](../profiles/mhfu/README.md#layered-armor)); this page says
what the game does, how that was found, and why the rest of the game cannot
see the difference.

Everything here is for USA Freedom Unite, `ULUS-10391`. It was read from
that executable: the model lookup, the armor tables and the load step below.
The name tables and the equipment box are still the Portable 3rd addresses in
[EQUIPMENT_MODS.md](EQUIPMENT_MODS.md), so the choice lists show piece ids.
No playthrough was run.

The code is `profiles/mhfu/host/game/layered_armor.{hpp,cpp}` (pure
functions over a `Ram`, tested on made-up tables in
`tests/equipment_models_tests.cpp`) and `layered_armor_hook.cpp` (what is put
in the game's place), and the page is `host/ui/layered_armor_screen.cpp`.

## Where the game decides what a hunter looks like

### The hunter records

The block the game reaches through the pointer at `0x089CC558` holds, from
`+0x30`, a 140-byte record per hunter. Record 0 is the hunter played here.
The address is `block + index * 140 + 0x30`, from `sll` by 3, `subu` of the
index, `sll` by 2, `addu`, `sll` by 2 (`0x0885187C`).

| Offset in a record | What |
| --- | --- |
| `+0x10` (u8) | 0 male, otherwise female (`lbu` at `0x088518C0`) |
| `+0x3E`, `+0x4A`, `+0x56`, `+0x62`, `+0x6E` (u16) | The armor ids. The lookup's part is 0 chest, 2 arms, 3 waist, 4 legs, 5 head. Part 1 is not an armor model |

The armor tables are 40 bytes per id, male model then female model as u16,
and the byte at `+4` says who can wear it. Chest `0x08973954`, arms
`0x08963374`, waist `0x08967794`, legs `0x0896B934`, head `0x0896F96C`. The
lookup indexes one with `sll` by 2, `addu` of the id, `sll` by 3.

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
`0x088DD18C` pass that to the lookup. Which of these runs in the village and
which on a quest was not watched.

Other code reads the armor tables by id. Layered armor does not wrap that.
`0x088516A4` is a second lookup, by sex, part and id rather than by record;
the lobby overlay calls it. Layered armor leaves it alone, because the
record lookup is what the load step and the part loop call.

### What layered armor changes

While it is on, `0x0885187C` is replaced (the runtime's function table, at the
flip, as the analog camera and the HUD toggle do). For index 0 and an armor
part with a chosen piece it returns that piece's model for the record's sex,
exactly what the game's function returns for that piece; anything else runs
the game's own code. The file and the inner wear of *Nothing* then follow as
for a piece really worn. Head models with special cases were not located.

It writes nothing: not the records, not the character, not the equipment
box. Turned off, the game's own function goes back in the table. Off from the
start, nothing is registered at all. A different executable is left alone:
the replacement is only installed when the code at the traced addresses is
the code described here.

## When a new look shows

`0x088C0E58` loads the hunter's models. `0x088BCBFC` calls it with the load
object and waits until it returns non-zero. `+0x998` is the state: the
function returns done only when that word is 5 (`xori` by 5, then `sltiu` by
1, at `0x088C1154`). The game's own request to load again stores 0 at
`+0x99C` and `+0x998` (`0x088BCBB4`). What makes the game call the step — an
area, a quest, the item box — was not watched.

The model lookup alone changes nothing on screen until the hunter loads its
models again. So layered armor also wraps the step: when the game steps that
load with the state already 5, and the look differs from the one it was last
loaded with, it stores 0 at `+0x99C` and `+0x998` as the game's request does,
and the game's step goes on from there. A change thus waits for the next
load the game makes by itself. Nothing is started outside those calls.

Not measured in the game: no scripted run and no playthrough. The claim that
a change shows on the next load is the load step's own state, not a screen
that was watched.

## Multiplayer: what other players see

Other players see the real armor. The replacement applies to record 0 only
and writes no guest memory, so the equipment the game sends is the equipment
it wears. How that equipment is filled into records 1 to 3 was not read, and
no two-instance session was run.

## Not verified

- Any of this on screen: a village, a quest, a male hunter, the item box.
- The game's names and the equipment box, which are still the Portable 3rd addresses.
- What `0x088516A4` draws in the lobby. It keeps the real pieces.
- Pieces whose head model the game treats specially.
- An ad hoc session with layered armor on.
