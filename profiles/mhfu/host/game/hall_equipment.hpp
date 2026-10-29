#pragma once

// Change Equipment in the Guild Hall: the box there offers Change Equipment as
// the house box does. On by default (Mods page, `game.hall_equipment`;
// `MHFU_HALL_EQUIPMENT=0` turns it off). Off, the entry stays greyed out and
// refused, as the game has it. The switch is read at each confirm.
//
// lobby_task (ULUS-10391, at 0x09A5F300) opens one box screen for both rooms
// through 0x09ADCA98(box, hall), which stores `hall` in the box's byte at
// +0x34: 0x09AADF34 passes 0 for the house and 0x09AADF58 passes 1 for the
// Guild Hall, online or not. The box's frame, 0x09ADCCD0(box), reads the
// pressed buttons through 0x0884FB6C and, on the top menu (+0x38 is 0), takes
// 0x4000 as the choice of the entry under the cursor (+0x36). With +0x34 set
// and the cursor on entry 3, Change Equipment, it plays the refusal sound and
// stays (0x09ADCE20). The same byte keeps the house's camera and its
// Rotate / Menu OFF panel out of the hall, changes Arrange Items and Sell Item,
// and has the menu read its labels from the table at 0x09B05168, whose entry 3
// is the greyed Change Equipment (text 0x12F; the house's table at 0x09B05158
// has 0x129). Traced with MHFU_TRACE_CAPTURE on the box in the house and in
// the online Guild Hall.
//
// So the byte stays set, and only the confirm on Change Equipment is let
// through: the wrapper clears +0x34 for that one call of 0x09ADCCD0 and sets
// it again before the box draws its menu (0x09AE38F8, called from 0x09ADD0E8)
// and after the call. The equipment screen it opens (0x09ADF750) does not read
// +0x34. The hall's label table gets the house's entry, so the entry is not
// drawn greyed. A hunter readied for a quest gets the game's box back: the
// label greyed and the choice refused, so the equipment does not change after
// the others have seen it, nor during the departure.
//
// The other consoles do not read the hunter: they get a copy. In ad hoc play
// each console keeps one member entry per player in the session object
// (load32(0x09A0B684); entries of 0xC8 bytes from +0x57C, the address at +0,
// the hunter at +0x20), and 0x0889D6EC(session) refreshes its own entry from
// the hunter through 0x088B0C8C and sets +0x12D0 to 60 so it is sent. The
// hall calls it after each change to the entry's status byte (+0xAC), as at
// 0x09AAFB34; the equipment screen does not, and with two instances a hunter
// who changed in the hall reached the other console in the old equipment,
// even one that entered the hall after the change. So once the equipment
// screen goes back to the box's top menu in the hall, the box's next frame
// first runs 0x0889D6EC, as a guest call in the game's thread, and then the
// frame itself. Offline, 0x0889D6EC does nothing.
//
// That reaches the others' member entries but not their screens: a console
// loads a hunter's models when the hunter appears, and with two instances
// one already in the hall kept the old equipment. 0x088994AC, which finds
// who appeared, keeps a member's cached slot only while the status byte the
// member sends (entry + 0xBF, from the sender's session + 0x12D5) is 2 to 5;
// any other status clears it, and the member counts as appearing once it is
// back in range. 0x08898980 sends this console's status and entry each time
// the counter at session + 0x12D0 goes below 0. So after the refresh this
// console's status is 1 for kAbsentFrames frames and then 2 again, each sent
// at once, and the others load this hunter again. Only from status 2, the
// status of a hunter standing in the hall: 3 and 4 were not traced, and may
// be what the quest counter reads, so a change made then reaches the others
// when they next load this hunter.
//
// They load it from their copy of this console's member entry, and with two
// instances that copy kept the old equipment: 0x0889D6EC only hands the entry
// on (0x088B0C8C copies it out), and the equipment screen does not write it.
// The entry's hunter (+0x20) is the hunter record's bytes from +0x10: the
// loader copies a member's +0x20 + i to the request's +8 + i (0x09A7CAB8)
// and that to the record's +0x10 + i (0x09A7D6E0), for i below 0x19 and from
// 0x1C to 0x79 (the weapon at +0x20, the five armor pieces from +0x2C). So
// before the status goes to 1 those bytes of this console's record 0 are
// copied into its own entry (the entry with the address at 0x09A289BC,
// 0x0889C518), if they matched it when Change Equipment was chosen.
//
// The new pieces' models. The equipment screen's commit, 0x09AA1538, copies
// the chosen pieces into the worn ones and asks the hall's model loader
// (load32(0x09B23790), 8 requests of 0x84 bytes, the first word the state)
// for this console's hunter again through 0x09A7D390, which gives the first
// free request state 3. The loader's frame, 0x09A7C9C8, runs the request at
// the head: 3 (0x09A7DA00), 4 (0x09A7E2F8) and 5 (0x09A7E598) are that
// reload, for hunter 0. They load the pieces into archive slot 1, a constant
// in every call (0x088DB4C8, 0x088DB534, 0x088DB5C0, 0x088DB5CC, 0x08866F6C)
// and in fixed offsets (+0x803E6, +0x80424: slot 1's part 6), compare them
// with slot 0's, and build the hunter from slot 1. The archive
// (load32(0x08A62D7C)) keeps 6 slots of 8 parts, each part with a row in five
// tables (0x088673CC), and the file object (load32(0x09A4F204)) a word per
// slot at +0x24. At home slot 1 is free. In the hall it is the second
// hunter's: traced with two instances, the reload reset slot 1's parts while
// the other hunter's model was in them, and the next draw of that hunter
// (0x088D3F68) read a pointer out of the new file and faulted, or hung.
//
// At home slot 1 starts empty. The reload loads only the pieces that changed
// into it, and the hunter is built from those and from slot 0 for the rest:
// the memory it draws from is never written. A copy of slot 0's rows in slot 1
// did keep the other hunter's slot whole, but then the changed piece was read
// over the one being drawn, and the next draw (0x088D3F68) faulted on it.
//
// So a reload the hall's equipment screen asked for runs with slot 1 an empty
// slot: a hall of fewer than four keeps slots 2 and 3 with their memory set
// aside and nothing in it. As it starts, slot 1's rows are kept and a free
// slot's rows put in their place; the new pieces go into the free slot's
// memory. When the loader's head is no longer state 3 to 5, slot 0 takes each
// part slot 1 loaded, the free slot takes slot 0's old memory for that part
// in exchange, and slot 1 gets its own rows back.
//
// A full hall has no such slot: the archive's pool (base at +0x80230, used at
// +0x8022C) is exactly four slots of 0x29000 bytes. Then the free slot is a
// reserve: memory of this console's own, laid out as slot 0's parts and with
// their rows as 0x088675BC leaves an empty part (state 0, file 0xFFFF; the
// kind, room, address and size kept). The reserve's first memory is a block
// taken once from the kernel's free memory, from the top as the thread stacks
// are, and kept: the game's image and its own arena end below it (UserSbrk
// sits at the image's end). After a reload slot 0 draws from the reserve's
// memory and the reserve holds slot 0's old memory, as a free slot would. The
// reserve is used only while none of its memory is in any slot's rows, so
// the memory slot 0 left is not taken while the game has given it out again;
// otherwise the block itself, if no slot uses it. Change Equipment is offered
// only while there is a free slot or a reserve.
//
// While the reload runs, other code may still reset slot 1 for the hunter
// whose slot it is: the hall's handling of a member who left (0x09A7CC38,
// when the loader is idle) and area changes call 0x088DB534(files, slot)
// directly. Such a call gets slot 1's own rows for its length, so it resets
// that hunter's parts and not the ones being loaded; the reload's own call
// returns to 0x09A7E1A4. 0x088DB534 only writes rows (0x088675BC), so the call
// runs to its end inside the wrapper.

#include "psprecomp/guest_memory.hpp"

#include <array>
#include <cstdint>
#include <span>

namespace psprecomp {
class Runtime;
struct AllegrexContext;
} // namespace psprecomp

namespace mhfu::game::hall_equipment {

inline constexpr std::uint32_t kBoxUpdate = 0x09ADCCD0u;
inline constexpr std::uint32_t kMenuDraw = 0x09AE38F8u;
inline constexpr std::uint32_t kHallFlag = 0x34u; // u8: 1 for the Guild Hall's box
inline constexpr std::uint32_t kCursor = 0x36u;   // u8: the top menu's entry
inline constexpr std::uint32_t kScreen = 0x38u;   // u8: 0 on the top menu
inline constexpr std::uint32_t kShown = 0x60u;    // u8: 0 while the menu is put away
inline constexpr std::uint8_t kChangeEquipment = 3u;
inline constexpr std::uint16_t kConfirm = 0x4000u;
inline constexpr std::uint16_t kCancel = 0x2000u;
inline constexpr std::uint16_t kStart = 0x0008u; // puts the house's menu away
// The hall's label for Change Equipment, and the two texts it can hold.
inline constexpr std::uint32_t kHallLabel = 0x09B05168u + 2u * kChangeEquipment;
// The hall's labels, where Change Equipment's is entry 3, or entry 4 once
// Item Sets (game/item_sets.hpp) has put its row before it. The label is
// found by its text, which no other row of the hall's box has.
inline constexpr std::uint32_t kHallLabels = 0x09B05168u;
inline constexpr std::uint32_t kHallLabelRows = 8u;
inline constexpr std::uint16_t kGreyLabel = 0x012Fu;
inline constexpr std::uint16_t kLabel = 0x0129u;
// The Guild Hall's call to its box: its first word, and the one that passes
// the hall flag (addiu a1, zr, 1) in the call's delay slot. FUComplete's
// Guildhall Full Chest (Offline) option, set in its CONFIG.BIN, puts a jump
// to its own code in the first word, which passes the flag itself (0 in the
// hall offline, where the box is then the house's), and clears the second.
inline constexpr std::uint32_t kHallBoxCall = 0x09AADF50u;
inline constexpr std::uint32_t kHallFlagArgument = 0x09AADF5Cu;
// The buttons as 0x0884FB6C returns them: none while the byte at
// load32(kPadGate) + 0x74 is set, otherwise the word at kPadPressed with the
// D-pad bits of the word at kPadRepeat.
inline constexpr std::uint32_t kPadGate = 0x08A62D80u;
inline constexpr std::uint32_t kPadGateByte = 0x74u;
inline constexpr std::uint32_t kPadPressed = 0x08A62D8Cu;
inline constexpr std::uint32_t kPadRepeat = 0x08A62D94u;
// The refresh of this console's member entry, and the session it takes.
inline constexpr std::uint32_t kRefreshOwnEntry = 0x0889D6ECu;
inline constexpr std::uint32_t kSessionPointer = 0x09A0B684u;
inline constexpr std::uint32_t kSessionBytes = 0x12D8u; // up to the status bytes the refresh reads
inline constexpr std::uint32_t kSendCounter = 0x12D0u;  // s32: sent when it goes below 0
inline constexpr std::uint32_t kStatus = 0x12D5u;       // u8: this console's status
inline constexpr std::uint8_t kAbsentStatus = 1u; // outside the 2 to 5 the others keep
inline constexpr std::uint8_t kHallStatus = 2u;   // standing in the hall
// Readied for a quest and on the way to it: with three instances one went 2,
// 3, 4 for the wait, 5 as it departed, and 2 again in the hall.
inline constexpr std::uint8_t kReadyFirst = 3u;
inline constexpr std::uint8_t kReadyLast = 5u;
inline constexpr std::uint32_t kAbsentFrames = 45u;
// The reset of a slot's parts, and where the reload's own call returns.
inline constexpr std::uint32_t kSlotReset = 0x088DB534u;
inline constexpr std::uint32_t kReloadResetReturn = 0x09A7E1A4u;
// The members' entries, this console's address, and its hunter record.
inline constexpr std::uint32_t kMembers = 0x57Cu;
inline constexpr std::uint32_t kMemberBytes = 0xC8u;
inline constexpr std::uint32_t kMemberCount = 0xEE3u; // s8
inline constexpr std::uint32_t kMemberHunter = 0x20u;
inline constexpr std::uint32_t kOwnAddress = 0x09A289BCu; // 6 bytes
inline constexpr std::uint32_t kGameSysPointer = 0x089CC558u;
inline constexpr std::uint32_t kOwnRecord = 0x30u;
inline constexpr std::uint32_t kRecordMirror = 0x10u; // entry hunter + i is record + 0x10 + i
inline constexpr std::uint32_t kMirrorGapFirst = 0x19u; // three bytes the loader skips
inline constexpr std::uint32_t kMirrorGapEnd = 0x1Cu;
inline constexpr std::uint32_t kMirrorEnd = 0x79u;
// The model loader, its request for this hunter, and the states that run it.
inline constexpr std::uint32_t kLoaderFrame = 0x09A7C9C8u;
inline constexpr std::uint32_t kRequestOwnReload = 0x09A7D390u;
inline constexpr std::uint32_t kLoaderPointer = 0x09B23790u;
inline constexpr std::uint32_t kOwnReloadFirst = 3u;
inline constexpr std::uint32_t kOwnReloadLast = 5u;
// The archive's rows, per part e = slot * 8 + part.
inline constexpr std::uint32_t kArchivePointer = 0x08A62D7Cu;
inline constexpr std::uint32_t kFilesPointer = 0x09A4F204u;
inline constexpr std::uint32_t kSlotParts = 8u;
inline constexpr std::uint32_t kRowKind = 0x800A8u;    // 8 bytes: kind, and the part's room at +4
inline constexpr std::uint32_t kRowAddress = 0x80258u; // 4 bytes
inline constexpr std::uint32_t kRowSize = 0x80318u;    // 4 bytes
inline constexpr std::uint32_t kRowState = 0x803D8u;   // 1 byte
inline constexpr std::uint32_t kRowFile = 0x80408u;    // 2 bytes
inline constexpr std::uint32_t kArchiveBytes = kRowFile + 2u * 6u * kSlotParts;
inline constexpr std::uint32_t kSlotProgress = 0x24u; // word per slot in the file object
inline constexpr std::uint32_t kStagingSlot = 1u;
inline constexpr std::uint32_t kOwnSlot = 0u;
inline constexpr std::uint32_t kFirstSpareSlot = 2u;
inline constexpr std::uint32_t kLastSpareSlot = 3u;
inline constexpr std::uint32_t kHunters = 0xCu; // hunter pointers in the file object, one per slot
inline constexpr std::uint32_t kLoaderRequests = 8u;
inline constexpr std::uint32_t kLoaderRequestBytes = 0x84u;
inline constexpr std::uint32_t kSlots = 6u;
inline constexpr std::uint32_t kPooledRoom = 0xFFFFFFFFu; // a part in the bump pool, not memory of its own
inline constexpr std::uint16_t kNoFile = 0xFFFFu;
inline constexpr std::uint32_t kReserveAlignment = 0x40u;

// One slot's rows.
struct SlotRows {
    std::array<std::uint32_t, kSlotParts> kind{};
    std::array<std::uint32_t, kSlotParts> room{};
    std::array<std::uint32_t, kSlotParts> address{};
    std::array<std::uint32_t, kSlotParts> size{};
    std::array<std::uint8_t, kSlotParts> state{};
    std::array<std::uint16_t, kSlotParts> file{};
    std::uint32_t progress{};
    bool operator==(const SlotRows &) const = default;
};

// A slot's rows, and putting them back; false when the archive or the file
// object is not in memory.
[[nodiscard]] bool read_slot(const psprecomp::GuestMemory &memory, std::uint32_t slot, SlotRows &rows);
bool write_slot(psprecomp::GuestMemory &memory, std::uint32_t slot, const SlotRows &rows);

// A slot the reload can load into: no hunter, nothing loaded or loading, no
// request for it in the loader, and its parts in memory of their own as large
// as slot 0's. The last such slot of 2 and 3, or none (-1).
[[nodiscard]] int spare_slot(const psprecomp::GuestMemory &memory);

// After the reload: slot 0 takes the parts the staging rows loaded, and the
// spare slot slot 0's memory for those parts, empty.
void take_loaded(const SlotRows &staged, SlotRows &own, SlotRows &spare);

// The reserve: the bytes a block needs for slot 0's parts (0 when a part has
// no room of its own), and the rows of an empty slot in a block at `base`.
[[nodiscard]] std::uint32_t reserve_bytes(const SlotRows &own);
[[nodiscard]] SlotRows reserve_layout(const SlotRows &own, std::uint32_t base);
// Whether rows can take slot 0's parts: each part in memory, as large as slot
// 0's, and none of it in any slot's rows.
[[nodiscard]] bool reserve_fits(const psprecomp::GuestMemory &memory, const SlotRows &rows);

struct CodeWord {
    std::uint32_t address;
    std::uint32_t word;
};

// What the addresses above were read off. A mismatch means another overlay
// or executable, and nothing is wrapped or written.
[[nodiscard]] std::span<const CodeWord> code_signature();
[[nodiscard]] bool code_matches(const psprecomp::GuestMemory &memory);

// The buttons the box's frame sees this frame.
[[nodiscard]] std::uint16_t pressed(const psprecomp::GuestMemory &memory);

// Whether this frame of the box is the Guild Hall's confirm on Change
// Equipment, the one the game refuses.
[[nodiscard]] bool refused_choice(const psprecomp::GuestMemory &memory, std::uint32_t box);

// Whether the box is in the Guild Hall's equipment screen.
[[nodiscard]] bool in_equipment_screen(const psprecomp::GuestMemory &memory, std::uint32_t box);

// The session the refresh takes; nothing outside ad hoc memory.
[[nodiscard]] std::uint32_t session(const psprecomp::GuestMemory &memory);

// Whether this console is readied for a quest (status 3 to 5). Then Change
// Equipment stays the game's: greyed out and refused. Never offline.
[[nodiscard]] bool readied(const psprecomp::GuestMemory &memory);

// This console's member entry; 0 when there is none.
[[nodiscard]] std::uint32_t own_entry(const psprecomp::GuestMemory &memory, std::uint32_t net);
// How many of the mirrored bytes of the entry differ from record 0, and
// copying them over; -1 when the entry or the record is not in memory.
[[nodiscard]] int entry_differences(const psprecomp::GuestMemory &memory, std::uint32_t entry);
bool copy_record_to_entry(psprecomp::GuestMemory &memory, std::uint32_t entry);

// The announcement after a change: this console's status for each flip
// while it runs. `start` begins it; it returns false once it is over.
struct Announcement {
    std::uint8_t status{};
    std::uint32_t frames{};
    bool running{};
};
// Starts it on the session (status 1, sent at once) when the status is 2;
// false otherwise.
bool start_announcement(psprecomp::GuestMemory &memory, std::uint32_t net, Announcement &announcement);
// One flip: after kAbsentFrames the status comes back, sent at once.
void step_announcement(psprecomp::GuestMemory &memory, std::uint32_t net, Announcement &announcement);

// Runs a guest function in the calling thread, with `argument` in a0, and
// then `then` with ctx back at the caller's return address, the way
// Kernel::call_guest does. The host passes the kernel's.
using ThenFunction = void (*)(psprecomp::AllegrexContext &ctx);
using GuestCaller = void (*)(
    psprecomp::AllegrexContext &ctx, std::uint32_t function, std::uint32_t argument, ThenFunction then);

// Takes `size` bytes of guest memory for good; 0 when there are none. The
// host passes the kernel's.
using BlockAllocator = std::uint32_t (*)(std::uint32_t size);

// At the flip: puts the wrappers in place each time lobby_task is installed
// and sets the hall's label for the switch. Runs every frame.
void frame(psprecomp::Runtime &runtime, GuestCaller caller, BlockAllocator allocate);

} // namespace mhfu::game::hall_equipment
