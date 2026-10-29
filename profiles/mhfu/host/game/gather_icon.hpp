#pragma once

// A gather mark drawn by the port over a node the game would gather (ULUS-10391).
// Off, nothing here runs. On, the mark sits above the node while the hunter is
// inside the radius the game's own search uses, and it is left out of the
// game's frames.
//
// The search is game_task 0x09A6F490, and it is read only while that overlay
// is loaded (the "MWo3" header and the name game_task.ovl at 0x09A5F300).
// It asks 0x08869570 for the loaded area's list, then walks 24-byte records
// until x is -1:
//   +0 x, +4 y, +8 z, +12 radius, all floats
//   +16 the gather id it returns, +20 the kind the action switches on
// The hunter is inside when node.y - 200 <= hunter.y < node.y + 100 and the
// horizontal distance is within the radius. The first such record is the one
// ○ gathers. The list does not carry uses left, so a spent node still matches
// while it remains on the list.
//
// 0x08869570 reads the object at 0x08A62D9C. Its word at +96 is the area
// table and the word at +80 is added to that area's pointer. The area index
// is 0x088C722C: the byte at GameSys+0x6AF10 selects an 8-byte entry of the
// table at 0x09CE4808 (a count, then halfword stage ids), and the stage id is
// the halfword at the followed hunter +0x29A. The followed hunter is the word
// at (the pointer at 0x089CBD04) + 2692, the same object the follow camera
// reads. The view a world draw uploads has no translation, so the mark is
// placed from the follow camera's eye and look-at.

#include "game/gather_art.hpp"
#include "psprecomp/guest_memory.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace psprecomp {
class Runtime;
}

namespace mhfu::game {

struct GatherNode {
    std::uint32_t address{};
    float x{};
    float y{};
    float z{};
    std::uint16_t id{};
    // The halfword at +20. The action switches on 0 (○: plants and the like),
    // 3 (□: mining) and 4 (□: a bug net).
    std::uint16_t kind{};
};

// True for the kinds the game gathers with □: mining (3) and a bug net (4).
// gather_glyph names the same kinds.
[[nodiscard]] inline bool gather_uses_square(std::uint16_t kind) noexcept {
    return kind == 3u || kind == 4u;
}

// Above the node's origin, where the mark floats.
inline constexpr float kGatherLift = 100.0f;
// The usual mark, in the picture's pixels (the 272-tall frame): the disc's
// radius and the ? drawn in it. Size is a percent of these
// (gather_mark_shape).
inline constexpr float kGatherDisc = 8.0f;
inline constexpr float kGatherGlyph = 9.0f;

// The mark as drawn, in those same pixels. `ring` is the outline's width.
struct GatherShape {
    float radius{};
    float ring{};
    float glyph{};
};

// `size_percent` is clamped to the settings range. The mark is a one-pixel
// ring and the font's own ?, scaled with the size.
[[nodiscard]] GatherShape gather_mark_shape(std::uint32_t size_percent) noexcept;

struct GatherWord {
    std::uint32_t address{};
    std::uint32_t instruction{};
};

// The instructions that establish the search, the list and the hunter.
[[nodiscard]] std::span<const GatherWord> gather_checked_words();

// True when every one of those words is mapped and matches. An unmapped word
// (the quest overlay is not loaded) is a quiet false. A mapped word that
// differs is logged once.
[[nodiscard]] bool gather_code_matches(const psprecomp::GuestMemory &memory);

// The first node on `list` the hunter at (x, y, z) would gather. Nothing at
// the sentinel, past 64 records, or when none is in range.
[[nodiscard]] std::optional<GatherNode> gather_from_list(const psprecomp::GuestMemory &memory, std::uint32_t list,
    float hunter_x, float hunter_y, float hunter_z);

// The area list for this stage id, or nothing when the pointers are not there.
[[nodiscard]] std::optional<std::uint32_t> gather_list_for(const psprecomp::GuestMemory &memory, std::uint16_t stage);

// The node ○ would gather now, when the instructions match. Nothing in the
// village (see farm_point), with no hunter, or with no node in range.
[[nodiscard]] std::optional<GatherNode> gather_node(const psprecomp::GuestMemory &memory);

// The projection and viewport of one perspective draw. The view matrix is not
// kept: the uploaded view has no translation.
struct GatherPicture {
    std::array<float, 16> projection{};
    float x_scale{};
    float y_scale{};
    float x_offset{};
    float y_offset{};
    float origin_x{};
    float origin_y{};
};

struct GatherPose {
    float eye_x{};
    float eye_y{};
    float eye_z{};
    float look_x{};
    float look_y{};
    float look_z{};
};

// The perspective draw with the most vertices since the mark last asked.
void gather_note_draw(const GatherPicture &picture, std::uint32_t vertices);

// What the mark stands for: the node's kind, a carve, or a fishing spot.
enum class GatherGlyph : std::uint8_t { Plant, Mining, BugNet, Carve, Fishing };

// 3 is mining and 4 a bug net; every other kind is gathered with ○ and shown
// as a plant.
[[nodiscard]] GatherGlyph gather_glyph(std::uint16_t kind) noexcept;

// A thing ○, □ or a bait would gather, carve or fish at now (gather_targets).
// `address` is its record, so a new target is told from the same one.
struct GatherTarget {
    std::uint32_t address{};
    float x{};
    float y{};
    float z{};
    GatherGlyph glyph{};
    // What the mark shows (game/gather_art.hpp). A carve leaves it empty; the
    // caller knows the monsters.
    GatherArt art{};
    // The PSP button that does it now (gather_button): ○ (0x2000), □
    // (0x8000), or 0 when neither would.
    std::uint32_t button{};
    // At a mining or bug point whose mark is the tool from the pouch, how many
    // of that tool the pouch holds (gather_tool_count); 0 otherwise.
    std::uint16_t carried{};
};

// Everything that can be done where the hunter stands, at most one of each, in
// this order: the node (gather_node), the body ○ would carve (carve_body), and
// the fishing spot a bait would fish at (fishing_spot). The circle handler
// (game_task 0x09AA6E70) asks the gather search once and switches on the
// kind. A plant (kind 0) starts its gather there, and the carve search is
// never reached. Mining and a bug net start only from □, with the pickaxe or
// the net selected; on ○ the handler goes on to the carve search (0x09AA73AC),
// as it does with no node and with every other kind. So a body over a plant is
// no target, and one over a mining point is. □ asks the circle handler and then the item handler
// (0x09AA5C04), and the bait's spot search does not read the gather list, so
// a fishing spot is a target beside either.
[[nodiscard]] std::vector<GatherTarget> gather_targets(const psprecomp::GuestMemory &memory);

// The mark as it moves, one step per game frame. A new target fades the mark
// in, and losing it fades the mark out at the place it was. Moving from one
// target to another near it glides there in world space; a far one is jumped
// to. The screen place of the last step is kept, so a present between two
// game frames can blend the mark as it blends the world.
class GatherMotion {
public:
    // Opacity gained or lost per game frame: four frames, about 130 ms.
    static constexpr float kFadeStep = 0.25f;
    // The part of the way to a new target covered per game frame.
    static constexpr float kGlide = 0.5f;
    // Closer than this, in world units, a glide ends on the target.
    static constexpr float kArrive = 1.0f;
    // A target farther than this from the mark is jumped to.
    static constexpr float kJump = 600.0f;

    struct Shown {
        float x{};
        float y{};
        float z{};
        float alpha{};
        GatherGlyph glyph{};
        GatherArt art{};
        std::uint32_t button{};
        std::uint16_t carried{};
    };

    // One game frame with this target, or none.
    void step(const std::optional<GatherTarget> &target);
    // Where the mark is drawn after the last step; nothing once it has faded.
    [[nodiscard]] std::optional<Shown> shown() const;
    // The screen place drawn for the last step, 0..1, or nothing.
    [[nodiscard]] const std::optional<std::array<float, 2>> &last_at() const { return last_at_; }
    void set_last_at(std::optional<std::array<float, 2>> at) { last_at_ = at; }
    void reset() { *this = GatherMotion{}; }

private:
    bool live_{};
    float x_{};
    float y_{};
    float z_{};
    float alpha_{};
    GatherGlyph glyph_{};
    GatherArt art_{};
    std::uint32_t button_{};
    std::uint16_t carried_{};
    std::optional<std::array<float, 2>> last_at_;
};

// What a mark stands for. Each has its own GatherMotion, so a node and the body
// beside it fade and glide apart.
enum class GatherRole : std::uint8_t { Node, Carve, Fishing };
inline constexpr std::size_t kGatherRoles = 3u;
[[nodiscard]] GatherRole gather_role(GatherGlyph glyph) noexcept;
using GatherMotions = std::array<GatherMotion, kGatherRoles>;

// The mark for one target and one game frame.
struct GatherMark {
    std::array<float, 2> at{}; // on the picture, 0..1 from the top left
    // Where the previous game frame drew it, when it was drawn; an
    // interpolated present between the two blends from there.
    std::optional<std::array<float, 2>> older;
    float alpha{1.0f};
    GatherGlyph glyph{};
    GatherArt art{};
    // The button that does it now, shown beside the disc; 0 for none.
    std::uint32_t button{};
    // How many of the tool the mark shows the pouch holds, shown as xn at the
    // disc's upper right; 0 for none.
    std::uint16_t carried{};
    // The world point it stands over, before the lift: the node, or the body
    // being carved.
    std::array<float, 3> world{};
    // The item's icon on the atlas and its tint, when `art` is an item that has
    // one. Filled in by the caller, which reads the game's tables.
    std::optional<ItemArt> icon;
    // The game's red ? (kUnknownIcon), drawn when there is no picture: an item
    // without an icon, a monster the sheet has no picture for, a cut tail.
    // Filled in by the caller; without it the interface draws its own ?.
    std::optional<ItemArt> unknown;
};

// Where the marks go on the picture while Gather icon is on: one over each of
// gather_targets, in that order: the node ○ or □ would gather, the monster
// body or cut tail ○ would carve, and the fishing spot a bait would fish at.
// None when it is off, the code differs, there is nothing to gather, carve or
// fish, and none for a point behind the camera or off the picture. Takes the
// draw noted since the last call. Without `motions` each mark is where its
// target is, at full opacity. With them, each role's motion steps once and
// its mark is where it shows; a frame without a world draw, or the setting
// off, resets them all. Marks over the same place are not moved apart here
// (spread_gather_marks).
[[nodiscard]] std::vector<GatherMark> gather_marks(const psprecomp::GuestMemory &memory,
    const std::optional<GatherPose> &pose, GatherMotions *motions = nullptr);

// The gap between two marks set side by side, in the picture's pixels (the
// 272-tall frame).
inline constexpr float kGatherSpacing = 3.0f;

// Marks whose discs would overlap, set side by side. `apart` is the distance
// between the centres of two marks that just touch, plus the spacing, as a
// part of the picture's width and of its height. Each group of marks closer
// than that becomes a row at the group's middle, in the order of their x.
// `older` moves with its mark, so a mark set aside still blends with the
// world. Marks apart already stay where they are.
void spread_gather_marks(std::span<GatherMark> marks, std::array<float, 2> apart);

// The hunter's own copy of the pad, which its actions read. Each frame EBOOT
// 0x088D61CC, called at 0x088DB474 for the local hunter (the hunter list's
// entry for the byte at GameSys +40), copies the pad words at 0x08A62D88
// into it: the buttons held at +1152, the ones new this frame at +1156 and
// the ones let go at +1160, each with the stick's direction in bits
// 0x00F00000, then the stick's angle and magnitude at +1180 and +1182 (from
// 0x08A62D98, written by the stick reader at 0x08869404). While the hunter
// may not act, when 0x08830F74 returns 1 or the word at +1556 has bit 1 or
// 0x200, the copy clears the buttons itself. The quest's ○ (game_task
// 0x09AA5424) tests bit 0x2000 of +1156 and calls the circle handler, and
// the Pokke Farm starts a point on □ new at +1156 (lobby_task 0x09AADA40).
// Menus, the map and the item bar read the pad, not these words, so a press
// written here reaches the hunter's actions alone.
struct HunterButtons {
    std::uint32_t held{};
    std::uint32_t fresh{};
    std::uint32_t released{};
    std::uint16_t magnitude{};
};
inline constexpr std::uint32_t kHunterHeld = 1152u;
inline constexpr std::uint32_t kHunterFresh = 1156u;
inline constexpr std::uint32_t kHunterReleased = 1160u;
inline constexpr std::uint32_t kHunterMagnitude = 1182u;
// The stick's direction in the button words; the buttons are the low half.
inline constexpr std::uint32_t kStickDirections = 0x00F00000u;
inline constexpr std::uint32_t kPadButtons = 0xFFFFu;

// A hold presses the gather button again every this many frames: the rate of
// the pulse the hold used to send on the pad (three reads down, three up).
inline constexpr unsigned kGatherPressFrames = 6u;

// △, × and R. With ○ or □ they make the weapons' chords (△+○ is an attack
// for most of them), which the game reads on the same press. Any of them down
// is never a gather: the buttons pass through and a running hold ends.
inline constexpr std::uint32_t kGatherChord = 0x1000u | 0x4000u | 0x0200u;

// Holding ○ or □ on a gather point, or ○ on a monster that can be carved.
// Plants and carving use ○ (B on an Xbox pad); mining and a bug net use □
// (X on an Xbox pad). The game starts one when that button is new in the
// hunter's +1156, so a button that stays down is one gather or one carve.
// The hold marks it new again every kGatherPressFrames frames, the way a
// player mashes it: the game starts the next gather on the first press after
// the hunter is free and ignores the ones during the animation. The hunter's
// action is not read. Following it broke whenever the gather went through an
// action the hold did not expect, such as stopping out of a run, and then the
// hold never pressed again.
// A hold begins only from a press the hunter's words showed: a button held
// through a menu, which the copy clears while it is open, is not a gather
// after it closes until it is pressed again. One pressed and held before the
// point came into reach presses at once. While the hold is going the hunter
// is shown the gather button alone, with the stick centred, so movement and
// other buttons cannot walk it off or start another action. A chord button
// (kGatherChord) is the exception: with one down the press is an attack, so
// the words pass through and the hold ends. Another button can take the
// gather button out of the resolved pad for a few frames; the hold stays
// until nothing is held. Away from a node and from a body, or with the
// setting off, the words pass through. L+○ or L+□ before a hold has started
// is the item bar.
class GatherRepeat {
public:
    // One frame of the hunter's words. `ready` is the gather search, or the
    // carve search, accepting `button`, ○ or □. True when the hold owns this
    // frame: `buttons` is then the gather button held, new on a press frame,
    // nothing let go, and the stick centred.
    bool apply(HunterButtons &buttons, bool ready, std::uint32_t button);
    void reset() { *this = GatherRepeat{}; }
    // The hold owns the words, even on a frame where the gather button is missing.
    [[nodiscard]] bool underway() const { return holding_ && latched_; }
    // The ○ or □ bit this hold presses. Meaningful while underway.
    [[nodiscard]] std::uint32_t button() const { return button_; }

private:
    bool holding_{};
    // The gather button was down for this hold. Stays set while some other
    // button is held, so a resolved pad that dropped it does not read as a release.
    bool latched_{};
    // The hunter saw the gather button new since it went down.
    bool pressed_{};
    std::uint32_t button_{0x2000u}; // ○ until a hold picks □
    unsigned step_{};
};

// A monster body the game would carve with ○. 0x0886CEA8 walks the table whose
// pointer is the word at 0x09A4F1A0: records start 4 bytes in, 32 bytes each,
// at most 20. +0 x, +4 y, +8 z and +12 radius are floats. The halfword at +16
// is 0xFFFF for an empty slot. The halfword at +24 is the stage and must
// match the hunter at +0x29A. The height window is the gather one: at or
// above the body minus 200, and below it plus 100. The horizontal distance
// must be inside the radius. 0x0886CF78 returns the halfword at +20 of the
// first such body; 0xFFFF is not a carve, and the circle handler
// (game_task 0x09AA73A4) does not look at a later body. It also skips the
// carve when bit 8 of the word at hunter +1164 is set. Nothing in the
// village, with no hunter, or with no body in reach.
[[nodiscard]] bool carve_in_reach(const psprecomp::GuestMemory &memory);

// The body carve_in_reach finds: its record and the x, y and z at +0, +4 and
// +8. A cut tail is carved through the same search. Nothing when there is no
// carve.
struct CarveBody {
    std::uint32_t address{};
    float x{};
    float y{};
    float z{};
};
[[nodiscard]] std::optional<CarveBody> carve_body(const psprecomp::GuestMemory &memory);

// A fishing spot. Fishing is the item action, not the circle handler: with a
// bait selected (items 136 to 143), the item handler (game_task 0x09AA7F98)
// asks the spot search 0x09AA6B18 and fishes when the type it returns is 23.
// The search reads the object at 0x09A44DB0, the word at 0x09A4F1A0 points to
// it. Its first word is the loaded stage object, taken from the table at
// 0x089B505C by 0x088C6A3C. The search calls that object's method +40 for the
// area's spot table. In each of the 266 stage overlays (0x09D63100) that
// method is lui v0, jr ra, addiu v0, so it returns a fixed address. The
// address is decoded from those three words here, and nothing is called. In
// that table the word at +16 is the spot list and the signed byte at +62 the
// count. Spots are 24 bytes:
//   +0 a halfword, +2 the type the search returns, +4 x, +8 y, +12 z,
//   +16 radius, all floats, +20 a halfword the fishing action keeps
// The hunter is inside when spot.y - 50 <= hunter.y < spot.y + 50 and the
// horizontal distance is within the radius. The first such spot is the one
// returned, so another spot in reach before it (a camp's bed or box) is no
// fishing. Bait is not read: the mark shows where a bait would fish.
struct FishingSpot {
    std::uint32_t address{};
    float x{};
    float y{};
    float z{};
};
inline constexpr std::uint16_t kFishingSpotType = 23u;

// The instructions that establish the spot search and the bait case.
[[nodiscard]] std::span<const GatherWord> fishing_checked_words();
// The area's spot table, decoded from the stage's method +40, or nothing when
// the pointers are not there or that method is not the three-word getter.
[[nodiscard]] std::optional<std::uint32_t> spot_table(const psprecomp::GuestMemory &memory);
// The first spot in `table` the hunter at (x, y, z) is inside, of any type.
// Nothing with no list, a count of 0 or less, or no spot in reach.
struct GatherSpot {
    std::uint32_t address{};
    std::uint16_t type{};
    float x{};
    float y{};
    float z{};
};
[[nodiscard]] std::optional<GatherSpot> spot_from_table(const psprecomp::GuestMemory &memory, std::uint32_t table,
    float hunter_x, float hunter_y, float hunter_z);
// The spot a bait would fish at now: the first spot in reach, when its type is
// 23. Nothing outside a quest, when the instructions differ, with no hunter,
// or with another spot first.
[[nodiscard]] std::optional<FishingSpot> fishing_spot(const psprecomp::GuestMemory &memory);

// A Pokke Farm spot that □ would mine or catch bugs at now (lobby_task.ovl).
// The village keeps no gather list. Each frame 0x09AAF290 walks the stage's
// 24-byte interaction points and stores the one in reach at village hunter
// +1364, or 0 (0x09AAF3AC). The handler at 0x09AAD350 starts every point but
// a person (type 1) when bit 0x8000, □, is new in hunter +1156, and switches
// on the halfword at point +2 through the table at 0x09B0F300. Type 15 is a
// mining outcrop and type 16 a bug thicket: they start sub-actions 0 and 1
// of the hunter's action group 8, in the order of the quest's mining (81) and
// bug net (82). The halfword at point +22 is the spot's index. Before
// starting, the handler asks game_sub.ovl whether the spot has uses left:
// 0x09C295F8 for type 15 and 0x09C29648 for type 16. Both read the farm at
// GameSys +0x6A238: the spot count is the byte at +2 (mining) or +3 (bugs)
// and the uses left are the halfword at +8 or +48, 8 bytes a spot. An index
// past the count, or no uses, is no gather. The handler acts only while the
// village state object (the word at 0x09B22E70) has 0 at +0 and +26, for the
// local hunter (+486 equal to the byte at GameSys +40). The hunter is the one
// the camera follows, as on a quest.
struct FarmPoint {
    std::uint32_t address{};
    std::uint16_t type{}; // 15 mining, 16 bugs
    std::uint16_t index{};
    std::uint16_t uses{};
};
inline constexpr std::uint16_t kFarmMining = 15u;
inline constexpr std::uint16_t kFarmBugs = 16u;

// The instructions that establish the farm search, the handler and the checks.
[[nodiscard]] std::span<const GatherWord> farm_checked_words();
// As gather_code_matches, for the village: lobby_task.ovl and game_sub.ovl
// must be loaded and every farm word must match.
[[nodiscard]] bool farm_code_matches(const psprecomp::GuestMemory &memory);
// The mining outcrop or bug thicket □ would gather at now, with uses left.
// Nothing outside the village, in a menu, or with no such spot in reach.
[[nodiscard]] std::optional<FarmPoint> farm_point(const psprecomp::GuestMemory &memory);

// Tools. The circle handler's mining path (game_task 0x09AA6FB0) reads the
// item in the selected pouch slot and goes on only for these: pickaxes for
// mining (kind 3) and bug nets for a bug point (kind 4).
inline constexpr std::array<std::uint16_t, 4> kPickaxes{144u, 145u, 146u, 1225u};
inline constexpr std::array<std::uint16_t, 4> kBugNets{148u, 149u, 150u, 1226u};

// The pouch slot (0 to 23) holding a tool for this node kind, with a count
// above 0: the selected slot when it holds one, else the first that does.
// Nothing for other kinds, with no local hunter, or with no such tool. The
// selection is the halfword at hunter +1608; the pouch is 24 slots of an id
// and a count at GameSys + 0x6AEA6 (game/empty_item_slot.hpp).
[[nodiscard]] std::optional<std::uint16_t> gather_tool_slot(
    const psprecomp::GuestMemory &memory, std::uint16_t kind);
// The item in that slot: the pickaxe or the net a gather there would use, so
// the mark shows that one in its own tint (gather_targets).
[[nodiscard]] std::optional<std::uint16_t> gather_tool_item(
    const psprecomp::GuestMemory &memory, std::uint16_t kind);
// How many of `item` the pouch holds, over every slot it is in.
[[nodiscard]] std::uint16_t gather_tool_count(const psprecomp::GuestMemory &memory, std::uint16_t item);
// True when the selected slot holds a tool for this kind.
[[nodiscard]] bool gather_tool_selected(const psprecomp::GuestMemory &memory, std::uint16_t kind);

// The button that gathers, carves or fishes at a target now: ○ for a plant
// and a carve, □ for a fishing spot (a bait), and for mining or a bug point
// ○ while Gather with ○ would take it (gather_circle), □ when the tool is
// selected, and 0 when neither would do anything. `carve` is a body ○ would
// carve in reach, which keeps ○ a carve.
[[nodiscard]] std::uint32_t gather_button(
    const psprecomp::GuestMemory &memory, GatherGlyph glyph, std::uint16_t kind, bool carve);

// ○ for every gather the pouch has the tool for (`game.gather_circle`, on by
// default). The game mines and catches bugs only on □ with a pickaxe or a
// bug net selected; ○ on such a point goes on to the carve search
// (gather_targets). With the setting on, ○ new on a mining point or a bug
// point with that tool in the pouch (gather_tool_slot) is □ in the hunter's
// words until ○ is let go, and the circle handler is given the tool's slot in
// place of the selection (gather_slot_for). The selection itself is never
// changed, so the item bar and the HUD go on showing the item the player
// chose. A body ○ would carve keeps ○ a carve. At a Pokke Farm outcrop or
// thicket (farm_point), which takes no tool, ○ is □. Without the tool ○ is
// left as it is, so it never uses another item. L+○ is the item bar, and a
// chord (kGatherChord) is an attack: both are left alone.
//
// The circle handler reads the selected slot (hunter +1608) only to pass it
// to 0x08856884, which returns the slot's address in the pouch (GameSys +
// 0x6AEA6 + 4 * slot): at 0x09AA6FB8 for mining's tool check, 0x09AA72B0
// for the net's, and 0x09AA7234 for the id it keeps at hunter +1684 (stored
// at 0x09AA7248). Every game_task call that takes the tool from the pouch
// afterwards uses that id.

// Both settings on one frame of the local hunter's words, as the copy has just
// written them: the hold (GatherRepeat) on the point, body or farm spot in
// reach, then Gather with ○. `hunter` is the one the copy wrote; any other
// than the one the camera follows is left alone. A press turned into □ for a
// tool also readies gather_slot_for until the next frame.
void gather_input(HunterButtons &buttons, std::uint32_t hunter, psprecomp::GuestMemory &memory);

// The slot 0x08856884 is asked for in place of the selection: the tool's, on
// the frame ○ became □ for it, when `return_address` is one of the circle
// handler's three calls and `hunter` (its s2) is the one the press was for.
// Nothing otherwise, and the game's own slot is used.
[[nodiscard]] std::optional<std::uint16_t> gather_slot_for(std::uint32_t return_address, std::uint32_t hunter);

// The instructions behind the copy and its call, behind 0x08856884, and
// behind the circle handler's reads of the selection (checked as ○ is pressed,
// since game_task is not always loaded).
[[nodiscard]] std::span<const GatherWord> gather_input_words();
[[nodiscard]] std::span<const GatherWord> gather_slot_words();
[[nodiscard]] std::span<const GatherWord> gather_tool_words();

// Puts the wrappers in place: over the copy (0x088D61CC) once Hold to gather
// or Gather with ○ is first on, and over 0x08856884 once Gather with ○ is,
// each after a check of its code; at the flip, when no generated code is
// running. Both are chained (hooks::wrap), so the game's calls run them inline.
void gather_frame(psprecomp::Runtime &runtime);

} // namespace mhfu::game
