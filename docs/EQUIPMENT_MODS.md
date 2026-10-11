# Equipment mods: which file a piece of equipment uses

An equipment mod (`Type="EquipSET"`, `EquipHEAD`, `EquipGS`, ...) is a model
without a fixed target: the player types the file id of the `DATA.BIN` entry
it replaces, four hex digits such as `0601`, and the pieces that use that file
then look like the mod. The Mods page stores that id with the mod. It does
not read the hunter's worn armor or the weapon being carried.

The lookup that filled those ids from the loaded hunter (the worn piece, the
empty-slot models, and the carried weapon) was traced on Monster Hunter
Portable 3rd and is not in this build. File ids stay typed by hand.
`MHFU_TRACE_DATA_BIN=1`, with any mod on, logs every file the game reads:
change the equipment on screen and note the new ids.

The hunter shows the mod once the game loads those models again: at the item
box, change to other armor and back, or restart. When a mod's file is larger
than the file it replaces, the Mods page says that a restart is needed and
offers *Restart now*, as for any other mod.

## Armor model records

Layered armor reads the model number of an armor piece from this executable
(ULUS-10391). Each part has a 40-byte record per armor id. The first two u16
are the model number for a male and for a female hunter. The byte at `+4`
says who can wear the piece (bit 0 male, bit 1 female, bit 2 blademaster,
bit 3 gunner).

| Part (kind) | Table |
| --- | --- |
| Chest (0) | `0x08967794` |
| Arms (1) | `0x0896B934` |
| Waist (2) | `0x0896F96C` |
| Legs (3) | `0x08973954` |
| Head (4) | `0x08963374` |

The code is `profiles/mhfu/host/game/equipment_models.{hpp,cpp}`.
`tests/equipment_models_tests.cpp` checks the records layered armor uses.
Those records do not name a `DATA.BIN` file. The file bases and the inner-wear
lists that turned a model number into a file id were the Portable 3rd trace
and are not here.
