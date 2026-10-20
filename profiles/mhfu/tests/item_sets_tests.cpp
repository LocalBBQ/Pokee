// Item sets: loading a set through the box as the game's own deposit would,
// the sets file, the hunter's name, what the screens do with the buttons, the
// list of game calls a hook can make, and the hooks on stand-ins for the box
// menu and the game's windows, text and sounds. No game data.
#include "game/item_sets.hpp"

#include "game/guest_calls.hpp"
#include "game/guest_ram.hpp"
#include "game/item_sets_menu.hpp"
#include "psprecomp/runtime.hpp"
#include "settings/settings.hpp"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {
using namespace mhfu::game::item_sets;
using mhfu::game::BufferRam;
using mhfu::game::GuestRam;

int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

void set_environment(const char *name, const std::string &value) {
#if defined(_WIN32)
    _putenv_s(name, value.c_str());
#else
    setenv(name, value.c_str(), 1);
#endif
}

constexpr std::uint32_t kSys = 0x09000000u;
constexpr std::uint16_t kPotion = 7u;    // carries 10
constexpr std::uint16_t kPaintball = 64u; // carries 99
constexpr std::uint16_t kTicket = 96u;   // byte 3 is 255: stacks of 255
constexpr std::uint16_t kHerb = 20u;     // carries 10

void set_item(mhfu::game::Ram &ram, std::uint16_t item, std::uint8_t carry) {
    ram.store8(kItemTable + item * kItemRecord + kCarryByte, carry);
}

void set_box(mhfu::game::Ram &ram, std::uint32_t slot, std::uint16_t item, std::uint16_t count) {
    ram.store16(kSys + kBox + slot * 4u, item);
    ram.store16(kSys + kBox + slot * 4u + 2u, count);
}

void set_pouch(mhfu::game::Ram &ram, std::uint32_t slot, std::uint16_t item, std::uint16_t count) {
    ram.store16(kSys + kPouch + slot * 4u, item);
    ram.store16(kSys + kPouch + slot * 4u + 2u, count);
}

Slot box_at(const mhfu::game::Ram &ram, std::uint32_t slot) {
    return {ram.load16(kSys + kBox + slot * 4u), ram.load16(kSys + kBox + slot * 4u + 2u)};
}

// A GameSys with an item table, everything else empty.
BufferRam make_ram() {
    BufferRam ram(0x08800000u, 0x01800000u);
    ram.store32(kGameSysPointer, kSys);
    set_item(ram, kPotion, 10u);
    set_item(ram, kPaintball, 99u);
    set_item(ram, kTicket, 255u);
    set_item(ram, kHerb, 10u);
    return ram;
}

void test_reading() {
    BufferRam ram = make_ram();
    check(game_sys(ram) == kSys, "GameSys is the word at its pointer");
    BufferRam none(0x08800000u, 0x01800000u);
    check(game_sys(none) == 0u, "no GameSys before the game sets it");
    check(box_capacity(ram, kSys) == 800u, "eight pages of 100 to start with");
    ram.store32(kSys + kObtained + (829u >> 5u) * 4u, 1u << (829u & 31u));
    check(obtained(ram, kSys, 829u), "an obtained item's bit is read");
    check(box_capacity(ram, kSys) == 900u, "one guide adds a page");
    ram.store32(kSys + kObtained + (830u >> 5u) * 4u, (1u << (829u & 31u)) | (1u << (830u & 31u)));
    check(box_capacity(ram, kSys) == 1000u, "both add two");
    check(!obtained(ram, kSys, kFlagIds), "ids past the flags are never obtained");
    check(carry_limit(ram, kPotion) == 10u && stack_limit(ram, kPotion) == kStack, "a potion carries 10, stacks 99");
    check(stack_limit(ram, kTicket) == kLargeStack, "byte 3 of 255 stacks 255");

    const char16_t name[] = u"LOU";
    for (std::uint32_t i = 0; i < 3u; ++i) ram.store16(kSys + kName + i * 2u, name[i]);
    check(hunter_name(ram, kSys) == "LOU", "the name is read as UTF-16");
    for (std::uint32_t i = 0; i < kNameChars + 2u; ++i) ram.store16(kSys + kName + i * 2u, u'A');
    check(hunter_name(ram, kSys) == "AAAAAAAA", "at most eight characters");
    ram.store16(kSys + kName, 0x00E9u);
    ram.store16(kSys + kName + 2u, 0x30A2u);
    ram.store16(kSys + kName + 4u, 0u);
    check(hunter_name(ram, kSys) == "\xC3\xA9\xE3\x82\xA2", "two- and three-byte characters");
}

void test_load() {
    BufferRam ram = make_ram();
    // The box: potions in slots 0 and 5, paintballs in slot 1; the pouch has
    // 3 herbs and 2 paintballs.
    set_box(ram, 0, kPotion, 4u);
    set_box(ram, 1, kPaintball, 98u);
    set_box(ram, 5, kPotion, 9u);
    set_pouch(ram, 0, kHerb, 3u);
    set_pouch(ram, 3, kPaintball, 2u);
    ItemSet set{"Hunt", {}};
    set.items[2] = {kPotion, 10u};
    set.items[4] = {kPaintball, 99u};
    set.items[7] = {kHerb, 5u};

    const Loaded loaded = load(ram, kSys, set);
    check(loaded.done && !loaded.box_full, "the set is loaded");
    const Pouch pouch = read_pouch(ram, kSys);
    check(pouch[0] == Slot{} && pouch[3] == Slot{}, "the pouch's own slots are emptied");
    check(pouch[2] == (Slot{kPotion, 10u}), "the potions come out into the set's slot");
    check(pouch[4] == (Slot{kPaintball, 99u}), "the paintballs too, all of them");
    check(pouch[7] == (Slot{kHerb, 3u}), "only the 3 herbs the box has");
    // The box's screens put their copy back into the pouch (Put In Item's
    // Sort), so it gets the set too, or the old pouch would come back with its
    // items in the box as well.
    for (std::uint32_t i = 0; i < kPouchSlots; ++i)
        check(Slot{ram.load16(kSys + kBoxPouch + i * 4u), ram.load16(kSys + kBoxPouch + i * 4u + 2u)} == pouch[i],
            "the box screens' copy of the pouch is the loaded pouch");
    check(loaded.short_items.size() == 1u && loaded.short_items[0].item == kHerb &&
              loaded.short_items[0].wanted == 5u && loaded.short_items[0].taken == 3u,
        "the herbs are reported short");
    // Deposits top up the paintball stack first (98 + 1), then the next empty
    // slot gets the herbs (2) and the last paintball (3); taking comes from
    // the last stack back.
    check(box_at(ram, 0) == (Slot{kPotion, 3u}), "potions are taken from the last stack first");
    check(box_at(ram, 5) == Slot{}, "an emptied stack is an empty slot");
    check(box_at(ram, 2) == Slot{} && box_at(ram, 3) == Slot{}, "the herbs went back out of the box");
    check(box_count(ram, kSys, kPaintball) == 1u, "one of the 100 paintballs stays in the box");
    check(obtained(ram, kSys, kHerb) && obtained(ram, kSys, kPaintball), "deposited items are marked obtained");
}

void test_carry_limit() {
    BufferRam ram = make_ram();
    set_box(ram, 0, kPotion, 50u);
    ItemSet set{"Too many", {}};
    set.items[0] = {kPotion, 30u};
    const Loaded loaded = load(ram, kSys, set);
    check(read_pouch(ram, kSys)[0] == (Slot{kPotion, 10u}), "no more than the pouch carries");
    check(loaded.short_items.size() == 1u && loaded.short_items[0].taken == 10u, "the rest is short");
    check(box_count(ram, kSys, kPotion) == 40u, "and stays in the box");
}

void test_box_full() {
    BufferRam ram = make_ram();
    for (std::uint32_t i = 0; i < 800u; ++i) set_box(ram, i, kPaintball, 99u);
    set_pouch(ram, 0, kHerb, 3u);
    ItemSet set{"Any", {}};
    set.items[0] = {kPaintball, 10u};
    const Loaded loaded = load(ram, kSys, set);
    check(!loaded.done && loaded.box_full, "a box with no room refuses");
    check(read_pouch(ram, kSys)[0] == (Slot{kHerb, 3u}), "and nothing moves");
    check(ram.load16(kSys + kBoxPouch) == 0u, "not even the box screens' copy");
    check(box_at(ram, 0) == (Slot{kPaintball, 99u}), "the box is as it was");
    // A stack with room takes the deposit even when no slot is empty.
    set_box(ram, 10, kHerb, 1u);
    const Loaded fits = load(ram, kSys, set);
    check(fits.done, "a stack of the same item with room takes it");
    check(box_at(ram, 10) == (Slot{kHerb, 4u}), "topped up to 4");
    // Slots past the box's pages are not used.
    BufferRam small = make_ram();
    for (std::uint32_t i = 0; i < 800u; ++i) set_box(small, i, kPaintball, 99u);
    set_pouch(small, 0, kHerb, 3u);
    check(load(small, kSys, ItemSet{}).box_full, "slot 800 is not there without a guide");
}

void test_large_stacks() {
    BufferRam ram = make_ram();
    set_box(ram, 0, kTicket, 250u);
    set_pouch(ram, 0, kTicket, 10u);
    check(load(ram, kSys, ItemSet{}).done, "an empty set only empties the pouch");
    check(box_at(ram, 0) == (Slot{kTicket, 255u}) && box_at(ram, 1) == (Slot{kTicket, 5u}),
        "a 255 stack fills to 255, then a new stack");
    check(ItemSet{}.empty(), "a set with nothing in it is empty");
}

void test_store() {
    const std::filesystem::path file = std::filesystem::temp_directory_path() / "pokee_item_sets_test.txt";
    std::error_code ec;
    std::filesystem::remove(file, ec);
    {
        Store store(file);
        const Sets fresh = store.sets("LOU");
        check(fresh[0].name == "Item Set 1" && fresh[7].name == "Item Set 8" && fresh[3].empty(),
            "a new hunter has eight empty sets");
        ItemSet set{"Rath\nalos", {}};
        set.items[0] = {kPotion, 10u};
        set.items[23] = {kPaintball, 5u};
        store.set("LOU", 2, set);
        store.set("Test", 0, ItemSet{"", {}});
        store.set("LOU", kSets, set);
        check(store.save(), "the file is written");
    }
    {
        Store store(file);
        const Sets lou = store.sets("LOU");
        check(lou[2].name == "Rath alos", "a name is one line");
        check(lou[2].items[0] == (Slot{kPotion, 10u}) && lou[2].items[23] == (Slot{kPaintball, 5u}),
            "the slots come back in place");
        check(lou[0].empty() && lou[0].name == "Item Set 1", "the others stay empty");
        check(store.sets("Test")[0].name == "Item Set 1", "an empty name is the default");
        check(store.sets("Nobody")[5].name == "Item Set 6", "an unknown hunter gets new sets");
    }
    {
        std::ofstream out(file, std::ios::trunc);
        out << "hunter LOU\r\nset 25 Out of range\nitems 7:10\nset 1\nitems 7:10 bad 99999:1 1:70000 64:3\nset x\n"
               "set 24 Last\nitems 64:2\n";
    }
    Store damaged(file);
    const Sets sets = damaged.sets("LOU");
    check(sets[0].name == "Item Set 1", "a set without a name gets the default");
    check(sets[0].items[0] == (Slot{kPotion, 10u}) && sets[0].items[4] == (Slot{kPaintball, 3u}),
        "readable slots are kept");
    check(sets[0].items[2] == Slot{} && sets[0].items[3] == Slot{}, "an id or count out of range is dropped");
    for (std::size_t i = 1; i + 1u < kSets; ++i) check(sets[i].empty(), "a set past the last is ignored");
    check(sets[kSets - 1u].name == "Last" && sets[kSets - 1u].items[0] == (Slot{kPaintball, 2u}),
        "the 24th set is read");
    std::filesystem::remove(file, ec);
    check(Store(std::filesystem::temp_directory_path() / "pokee_no_such_dir" / "sets.txt").sets("A")[0].name ==
              "Item Set 1",
        "a missing file is no sets");
}

// The hooks ------------------------------------------------------------------

int text_calls{};
int cursor_calls{};
std::uint32_t cursor_rows{};
int update_calls{};

void game_text(psprecomp::Runtime &, psprecomp::AllegrexContext &ctx) {
    ++text_calls;
    ctx.gpr[2] = 0x1234u;
    ctx.pc = ctx.gpr[31];
}
void game_cursor(psprecomp::Runtime &, psprecomp::AllegrexContext &ctx) {
    ++cursor_calls;
    cursor_rows = ctx.gpr[7];
    ctx.pc = ctx.gpr[31];
}
void game_update(psprecomp::Runtime &, psprecomp::AllegrexContext &ctx) {
    ++update_calls;
    ctx.pc = ctx.gpr[31];
}

void write_words(psprecomp::GuestMemory &memory, std::span<const CodeWord> words) {
    for (const CodeWord &word : words) memory.store32(word.address, word.word);
}

void install_lobby(psprecomp::GuestMemory &memory) {
    constexpr std::uint32_t kSlot = 0x09A5F300u;
    memory.store32(kSlot, 0x336F574Du);
    memory.store32(kSlot + 8u, kSlot);
    const char name[] = "lobby_task.ovl";
    for (std::uint32_t i = 0; i < sizeof name; ++i) memory.store8(kSlot + 32u + i, static_cast<std::uint8_t>(name[i]));
    write_words(memory, overlay_signature());
    memory.store32(kMenuRecord, 0x00180010u);
    memory.store32(kMenuRecord + 8u, 0x00060014u);
    memory.store32(kMenuRecord + 12u, kBoxList);
    const std::uint32_t box[] = {0x01270126u, 0x01290128u, 0x012B012Au, 0u};
    const std::uint32_t small[] = {0x012D012Cu, 0x012F012Eu, 0x01310130u, 0u};
    for (std::uint32_t i = 0; i < 4u; ++i) {
        memory.store32(kBoxList + i * 4u, box[i]);
        memory.store32(kSmallBoxList + i * 4u, small[i]);
    }
}

std::uint32_t call(psprecomp::Runtime &runtime, std::uint32_t function, std::uint32_t a0, std::uint32_t a1,
    std::uint32_t a3, std::uint32_t ra) {
    psprecomp::AllegrexContext ctx{};
    ctx.gpr[4] = a0;
    ctx.gpr[5] = a1;
    ctx.gpr[7] = a3;
    ctx.gpr[31] = ra;
    runtime.function_at(function)(runtime, ctx);
    check(ctx.pc == ra, "the call returns to its caller");
    return ctx.gpr[2];
}

void test_signatures() {
    psprecomp::GuestMemory memory(32u * 1024u * 1024u);
    const GuestRam ram(memory);
    check(!matches(ram, code_signature()) && !matches(ram, overlay_signature()), "empty memory is neither");
    write_words(memory, code_signature());
    write_words(memory, overlay_signature());
    check(matches(ram, code_signature()) && matches(ram, overlay_signature()), "the traced words match");
    memory.store32(overlay_signature()[5].address, 0u);
    check(!matches(ram, overlay_signature()), "one different word is another overlay");
}

void test_hooks() {
    psprecomp::Runtime runtime(32u * 1024u * 1024u);
    psprecomp::GuestMemory &memory = runtime.memory();
    mhfu::settings::current().item_sets = true;
    runtime.register_function(kCursor, &game_cursor, "game_cursor");
    frame(runtime);
    check(runtime.function_at(kCursor) == &game_cursor, "other code is not wrapped");

    write_words(memory, code_signature());
    frame(runtime);
    check(runtime.function_at(kCursor) != &game_cursor, "the cursor call is wrapped");
    check(runtime.function_at(kText) == nullptr, "nothing in lobby_task before it is installed");

    install_lobby(memory);
    runtime.register_function(kText, &game_text, "game_text");
    runtime.register_function(kBoxFrame, &game_update, "game_box_frame");
    frame(runtime);
    check(memory.load32(kMenuRecord + 8u) == 0x00070014u, "the menu has seven rows");
    check(memory.load32(kMenuRecord) == 0x000C0010u, "and is raised clear of the hints");
    check(memory.load16(kBoxList + 4u) == 296u && memory.load16(kBoxList + 6u) == kTextId &&
              memory.load16(kBoxList + 8u) == 297u && memory.load16(kBoxList + 12u) == 299u &&
              memory.load16(kBoxList + 14u) == 0u,
        "the box's list has Item Sets after Combine Items");
    check(memory.load16(kSmallBoxList + 6u) == kTextId && memory.load16(kSmallBoxList + 12u) == 305u &&
              memory.load16(kSmallBoxList + 14u) == 0u && memory.load16(kSmallBoxList + 16u) == 0u,
        "so does the Small Item Box's, still ended before what follows it");
    check(runtime.function_at(kText) != &game_text && runtime.function_at(kBoxFrame) != &game_update,
        "the text and the box's frame are wrapped");
    frame(runtime);
    check(memory.load32(kMenuRecord + 8u) == 0x00070014u, "doing it again changes nothing");

    // The text: Item Sets is answered, everything else is the game's.
    text_calls = 0;
    const std::uint32_t label = call(runtime, kText, 0x09900000u, kTextId, 0u, 0x0882F884u);
    check(label == kLabel && text_calls == 0, "Item Sets' id is answered with the label");
    std::string text;
    for (std::uint32_t at = kLabel; memory.load8(at) != 0u; ++at) text += static_cast<char>(memory.load8(at));
    check(text == "Item Sets", "the label holds the words");
    check(call(runtime, kText, 0x09900000u, 294u, 0u, 0x0882F884u) == 0x1234u && text_calls == 1,
        "Put In Item is the game's");

    // The cursor: the box's moves over the rows as shown (test_screens);
    // another menu's is the game's.
    call(runtime, kCursor, 0u, 0x09B1A97Eu, kGameRows, 0x09A00000u);
    check(cursor_calls == 1 && cursor_rows == kGameRows, "another menu's cursor is the game's");

    // The box's frame on its menu and on the game's own rows: the game's.
    constexpr std::uint32_t kBoxRecord = 0x09B1A948u;
    update_calls = 0;
    call(runtime, kBoxFrame, kBoxRecord, 0u, 0u, 0x09A00010u);
    check(update_calls == 1 && !screen_open(), "the menu's frame is the game's");
    memory.store8(kBoxRecord + kPhase, 1u);
    memory.store8(kBoxRecord + kChosenRow, 2u);
    call(runtime, kBoxFrame, kBoxRecord, 0u, 0u, 0x09A00010u);
    check(update_calls == 2 && !screen_open(), "the game's own rows run as before");
    memory.store8(kBoxRecord + kPhase, 0u);

    // Off: the game's six rows, and the cursor keeps them.
    mhfu::settings::current().item_sets = false;
    frame(runtime);
    check(memory.load32(kMenuRecord + 8u) == 0x00060014u && memory.load32(kMenuRecord) == 0x00180010u,
        "off puts the six rows back");
    check(memory.load16(kBoxList + 6u) == 297u && memory.load32(kBoxList + 12u) == 0u &&
              memory.load16(kSmallBoxList + 6u) == 303u && memory.load32(kSmallBoxList + 12u) == 0u,
        "and the lists");
    call(runtime, kCursor, 0u, 0x09B1A97Eu, kGameRows, kCursorReturn);
    check(cursor_calls == 2 && cursor_rows == kGameRows, "the box's cursor is the game's again");
    mhfu::settings::current().item_sets = true;

    // Another overlay in the slot: nothing is changed.
    memory.store8(0x09A5F300u + 32u, 'g');
    memory.store32(kMenuRecord + 8u, 0x00060014u);
    frame(runtime);
    check(memory.load32(kMenuRecord + 8u) == 0x00060014u, "another overlay is left alone");
}

// Item Sets shown fourth, numbered 6.
void test_rows() {
    const std::uint8_t shown[] = {0u, 1u, 2u, 4u, 5u, 6u, 3u};
    for (std::uint8_t row = 0; row < kRows; ++row) {
        check(shown_row(row) == shown[row], "each row is shown in its place");
        check(game_row(shown_row(row)) == row, "and its place gives it back");
    }
    check(move_shown(2u, 0x40u) == kRow, "down from Combine Items is Item Sets");
    check(move_shown(kRow, 0x40u) == 3u, "and then Change Equipment");
    check(move_shown(3u, 0x10u) == kRow, "up from Change Equipment is Item Sets");
    check(move_shown(5u, 0x40u) == 0u && move_shown(0u, 0x10u) == 5u, "Sell Item is still last, with the wrap");
    check(move_shown(4u, 0u) == 4u, "no button, no move");
}

// The screens' buttons -------------------------------------------------------

void test_menu() {
    namespace m = mhfu::game::item_sets::menu;
    Sets sets;
    for (std::size_t i = 0; i < kSets; ++i) sets[i].name = default_name(i);
    sets[2].items[0] = {kPotion, 10u};
    m::State s;
    check(m::step(s, 0u, sets, false).sound == m::Sound::None, "no button, nothing happens");
    check(m::step(s, m::kUp, sets, false).sound == m::Sound::Cursor && s.row == 1u, "up wraps to the last row");
    check(m::step(s, m::kDown, sets, false).sound == m::Sound::Cursor && s.row == 0u, "down wraps to the first");
    check(m::message(s).empty() && m::choose_row(0) == "Load Item Set" && m::choose_row(1) == "Register Pouch",
        "the menu's rows, and no message");
    check(m::step(s, m::kCancel, sets, false).act == m::Act::Close, "back from the menu leaves");

    // Loading: an empty set is refused, a set asks first, Yes loads it.
    check(m::step(s, m::kConfirm, sets, false).sound == m::Sound::Confirm && s.page == m::Page::List &&
              s.mode == m::Mode::Load,
        "the first row lists the sets to load");
    check(m::message(s) == "Please select the set to load." && m::list_title(s.mode) == "Load Item Set",
        "with the prompt and the title");
    check(m::step(s, m::kConfirm, sets, false).sound == m::Sound::Refuse && s.result_warning &&
              m::message(s) == "~C02No set is registered here.",
        "an empty set cannot be loaded");
    m::step(s, m::kDown, sets, false);
    check(s.set == 1u && s.result.empty(), "moving clears what the last press said");
    m::step(s, m::kUp, sets, false);
    m::step(s, m::kUp, sets, false);
    check(s.set == 7u, "the sets wrap too, on their page");
    check(m::step(s, m::kR, sets, false).sound == m::Sound::Page && s.set == 15u && m::set_page(s) == 1u &&
              m::set_row_on_page(s) == 7u,
        "R is the next page of sets, on the same row");
    m::step(s, m::kDown, sets, false);
    check(s.set == 8u, "down wraps within the page");
    m::step(s, m::kR, sets, false);
    m::step(s, m::kR, sets, false);
    check(s.set == 0u, "past the last page is the first");
    m::step(s, m::kL, sets, false);
    check(s.set == 16u && m::set_page(s) == 2u, "L before the first is the last");
    check(m::message(s) == "Please select the set to load.", "the prompt stays");
    s.set = 2u;
    check(m::step(s, m::kRight, sets, false).sound == m::Sound::Page && s.contents == 1u, "right turns the page");
    m::step(s, m::kLeft, sets, false);
    m::step(s, m::kLeft, sets, false);
    check(s.contents == m::kPages - 1u, "left wraps to the last page");
    check(m::step(s, m::kConfirm, sets, false).act == m::Act::None && s.page == m::Page::Confirm && s.yes,
        "a set asks first, on Yes");
    check(m::message(s) == "Load Item Set 3?\nThe pouch goes into the box.", "and says what happens");
    m::step(s, m::kRight, sets, false);
    check(!s.yes, "left and right choose");
    check(m::step(s, m::kConfirm, sets, false).act == m::Act::None && s.page == m::Page::List, "No does nothing");
    m::step(s, m::kConfirm, sets, false);
    m::Step loaded = m::step(s, m::kConfirm, sets, false);
    check(loaded.act == m::Act::Load && loaded.set == 2u && loaded.sound == m::Sound::Confirm, "Yes loads it");
    m::finished(s, "Item Set 3 loaded.", false);
    check(m::message(s) == "Item Set 3 loaded." && !s.result_warning, "and the list says so");
    m::step(s, m::kConfirm, sets, false);
    check(m::step(s, m::kCancel, sets, false).sound == m::Sound::Cancel && s.page == m::Page::List,
        "back from Yes/No is the list");

    // Deleting: △ asks, on No.
    check(m::step(s, m::kDelete, sets, false).sound == m::Sound::Confirm && s.page == m::Page::Confirm &&
              s.ask == m::Ask::Delete && !s.yes && m::message(s) == "Delete this Set?",
        "triangle asks to delete, on No");
    m::step(s, m::kLeft, sets, false);
    check(m::step(s, m::kConfirm, sets, false).act == m::Act::Delete, "Yes deletes");
    s.set = 0u;
    check(m::step(s, m::kDelete, sets, false).sound == m::Sound::Refuse && s.page == m::Page::List,
        "an empty set has nothing to delete");

    // Registering: an empty pouch is refused, an empty set takes it at once,
    // a set asks before it is replaced.
    check(m::step(s, m::kCancel, sets, false).sound == m::Sound::Cancel && s.page == m::Page::Choose,
        "back from the list is the menu");
    m::step(s, m::kDown, sets, false);
    m::step(s, m::kConfirm, sets, false);
    check(s.mode == m::Mode::Register && m::message(s) == "Please select where to\nregister the pouch.",
        "the second row lists where to register");
    check(m::step(s, m::kConfirm, sets, true).sound == m::Sound::Refuse && m::message(s) == "~C02The pouch is empty.",
        "an empty pouch is not registered");
    check(m::step(s, m::kConfirm, sets, false).act == m::Act::Register, "an empty set takes the pouch at once");
    s.set = 2u;
    check(m::step(s, m::kConfirm, sets, false).act == m::Act::None && s.ask == m::Ask::Register && !s.yes,
        "a set asks before it is replaced, on No");
    check(m::message(s) == "Replace Item Set 3\nwith the pouch?", "with its name");
    m::step(s, m::kRight, sets, false);
    check(m::step(s, m::kConfirm, sets, false).act == m::Act::Register, "Yes replaces it");
    check(m::set_row(7) == "Item Set 8" && m::list_title(m::Mode::Register) == "Register Set", "names");
    check(m::hint(s) == "~C00~B01Register Set  ~B02Delete Set", "registering's hints");
    s.mode = m::Mode::Load;
    check(m::hint(s) == "~C00~B01Load Set  ~B02Delete Set", "loading's hints");
    check(m::sound_id(m::Sound::Page) == 17u && m::sound_id(m::Sound::Confirm) == 19u &&
              m::sound_id(m::Sound::Cancel) == 20u && m::sound_id(m::Sound::Refuse) == 21u &&
              m::sound_id(m::Sound::Cursor) == 23u && m::sound_id(m::Sound::Moved) == 136u &&
              m::sound_id(m::Sound::Registered) == 137u && m::sound_id(m::Sound::None) == 0u,
        "the game's sound numbers");
    check(m::sound_bank(m::Sound::Moved) == 1u && m::sound_bank(m::Sound::Registered) == 1u &&
              m::sound_bank(m::Sound::Confirm) == 0u && m::sound_bank(m::Sound::Cursor) == 0u,
        "the box's item sounds are in the second bank, the menus' in the first");
}

// The game calls ---------------------------------------------------------------

struct Logged {
    std::uint32_t function{};
    std::array<std::uint32_t, 8> args{};
    std::uint32_t stack{};
};
std::vector<Logged> logged;

// Every game function the screens call, as a stand-in: noted and returned.
void game_function(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    Logged call{ctx.pc, {}, runtime.memory().load32(ctx.gpr[29])};
    for (std::uint32_t i = 0; i < 4u; ++i) call.args[i] = ctx.gpr[4u + i];
    for (std::uint32_t i = 0; i < 4u; ++i) call.args[4u + i] = ctx.gpr[8u + i];
    logged.push_back(call);
    ctx.gpr[2] = 0xBADu;
    ctx.pc = ctx.gpr[31];
}

void begin_two(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    namespace calls = mhfu::game::guest_calls;
    calls::Call first{0x09000100u, {1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u}, 9u};
    calls::Call second{0x09000200u, {10u}, 0u};
    calls::run(runtime, ctx, {first, second}, 42u);
}

// Runs guest code the way the runtime's loop does, from ctx.pc until it
// returns to `until`.
bool dispatch(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx, std::uint32_t until) {
    for (int i = 0; i < 64 && ctx.pc != until; ++i) {
        const psprecomp::Runtime::RecompiledFunction function = runtime.function_at(ctx.pc);
        if (function == nullptr || runtime.stopped()) return false;
        function(runtime, ctx);
    }
    return ctx.pc == until;
}

void test_guest_calls() {
    namespace calls = mhfu::game::guest_calls;
    psprecomp::Runtime runtime(32u * 1024u * 1024u);
    runtime.register_function(0x09000000u, &begin_two, "begin_two");
    runtime.register_function(0x09000100u, &game_function, "first");
    runtime.register_function(0x09000200u, &game_function, "second");
    logged.clear();
    psprecomp::AllegrexContext ctx{};
    ctx.gpr[29] = 0x09F00000u;
    ctx.gpr[31] = 0x09ADD0F0u;
    ctx.gpr[16] = 0x5555u;
    ctx.pc = 0x09000000u;
    check(dispatch(runtime, ctx, 0x09ADD0F0u), "the list returns to the caller");
    check(logged.size() == 2u && logged[0].function == 0x09000100u && logged[1].function == 0x09000200u,
        "the functions run in order");
    check(logged[0].args == (std::array<std::uint32_t, 8>{1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u}) && logged[0].stack == 9u,
        "with eight arguments in registers and the ninth at 0(sp)");
    check(logged[1].args[0] == 10u && logged[1].stack == 0u, "each with its own");
    check(ctx.gpr[2] == 42u && ctx.gpr[29] == 0x09F00000u && ctx.gpr[31] == 0x09ADD0F0u && ctx.gpr[16] == 0x5555u,
        "the caller gets the chosen value, its stack and its registers back");
    check(!calls::running(), "nothing runs after");

    // An empty list returns at once.
    psprecomp::AllegrexContext empty{};
    empty.gpr[31] = 0x09ADD0F0u;
    calls::run(runtime, empty, {}, 7u);
    check(empty.pc == 0x09ADD0F0u && empty.gpr[2] == 7u, "an empty list just returns");

    // A list started while another runs returns at once.
    psprecomp::AllegrexContext outer{};
    outer.gpr[29] = 0x09F00000u;
    outer.gpr[31] = 0x09ADD0F0u;
    calls::run(runtime, outer, {calls::Call{0x09000100u}}, 0u);
    check(calls::running(), "the first list is running");
    psprecomp::AllegrexContext inner{};
    inner.gpr[31] = 0x09ABCDEFu;
    calls::run(runtime, inner, {calls::Call{0x09000200u}}, 3u);
    check(inner.pc == 0x09ABCDEFu && inner.gpr[2] == 3u, "a second one does not start");
    check(dispatch(runtime, outer, 0x09ADD0F0u) && !calls::running(), "the first one still finishes");

    // The stub reached with nothing running stops the game rather than jump anywhere.
    psprecomp::AllegrexContext stray{};
    stray.pc = calls::kReturnStub;
    runtime.function_at(calls::kReturnStub)(runtime, stray);
    check(runtime.stopped(), "a stray return stops the game");
}

// The screens, end to end on stand-ins ------------------------------------------

constexpr std::uint32_t kScreenBox = 0x09B1A948u;
constexpr std::uint32_t kBoxReturn = 0x09A00010u;
constexpr std::uint32_t kUi = 0x09A04CE0u;

std::vector<Logged> press(psprecomp::Runtime &runtime, std::uint16_t buttons) {
    runtime.memory().store32(kButtons, buttons);
    logged.clear();
    psprecomp::AllegrexContext ctx{};
    ctx.gpr[4] = kScreenBox;
    ctx.gpr[29] = 0x09F00000u;
    ctx.gpr[31] = kBoxReturn;
    ctx.pc = kBoxFrame;
    check(dispatch(runtime, ctx, kBoxReturn), "each frame returns to the box");
    check(ctx.gpr[29] == 0x09F00000u, "with its stack");
    check(ctx.gpr[2] == 1u || logged.size() == 1u, "and the frame's 1, as the box's own frame returns");
    runtime.memory().store32(kButtons, 0u);
    return logged;
}

const Logged *find(const std::vector<Logged> &calls, std::uint32_t function) {
    for (const Logged &call : calls)
        if (call.function == function) return &call;
    return nullptr;
}

std::string guest_text(const psprecomp::GuestMemory &memory, std::uint32_t at) {
    std::string text;
    for (; memory.load8(at) != 0u; ++at) text += static_cast<char>(memory.load8(at));
    return text;
}

// The text printed at `x`, or of the words at `words`.
const Logged *print_at(const std::vector<Logged> &calls, std::uint32_t x, std::uint32_t words = 0u) {
    for (const Logged &call : calls)
        if (call.function == kPrint && (call.args[1] == x || (words != 0u && call.args[3] == words))) return &call;
    return nullptr;
}

std::uint32_t sound_of(const std::vector<Logged> &calls) {
    const Logged *sound = find(calls, kSound);
    return sound == nullptr ? 0u : sound->args[2];
}

void test_screens() {
    psprecomp::Runtime runtime(32u * 1024u * 1024u);
    psprecomp::GuestMemory &memory = runtime.memory();
    GuestRam ram(memory);
    mhfu::settings::current().item_sets = true;
    for (std::uint32_t function :
        {kCursor, kText, kBoxFrame, kUpdate, kBoxMenu, kWindowBuild, kWindowDraw, kPager, kFontSize, kFontColour, kItemWindow, kMessageDraw, kPrint,
            kSound})
        runtime.register_function(function, &game_function, "game");
    write_words(memory, code_signature());
    install_lobby(memory);
    frame(runtime);
    memory.store32(kUiPointer, kUi);
    memory.store32(kFontPointer, 0x09971400u);
    memory.store32(kSoundPointer, 0x09980000u);
    memory.store32(kGameSysPointer, kSys);
    const char16_t name[] = u"SETS";
    for (std::uint32_t i = 0; i < 4u; ++i) memory.store16(kSys + kName + i * 2u, name[i]);
    set_item(ram, kPotion, 10u);
    set_item(ram, kHerb, 10u);
    set_pouch(ram, 0, kPotion, 10u);

    // The box's cursor: over the rows as shown, with the game's sound.
    constexpr std::uint32_t kRowByte = kScreenBox + kChosenRow;
    const auto move = [&](std::uint16_t buttons) {
        logged.clear();
        psprecomp::AllegrexContext ctx{};
        ctx.gpr[4] = kUi;
        ctx.gpr[5] = kRowByte;
        ctx.gpr[6] = buttons;
        ctx.gpr[7] = kGameRows;
        ctx.gpr[29] = 0x09F00000u;
        ctx.gpr[31] = kCursorReturn;
        ctx.pc = kCursor;
        check(dispatch(runtime, ctx, kCursorReturn), "the cursor returns to the box");
        return ctx.gpr[2];
    };
    memory.store8(kRowByte, 2u);
    check(move(menu::kDown) == 1u && memory.load8(kRowByte) == kRow && sound_of(logged) == 23u,
        "down from Combine Items is Item Sets, with the cursor's sound");
    check(move(menu::kDown) == 1u && memory.load8(kRowByte) == 3u, "then Change Equipment");
    check(move(0u) == 0u && logged.empty(), "no button: no move and no sound");
    check(find(logged, kCursor) == nullptr, "the game's own cursor is not run for the box");
    // Its highlight where the row is shown; other windows as they ask.
    const auto highlight = [&](std::uint32_t row, std::uint32_t ra) {
        logged.clear();
        psprecomp::AllegrexContext ctx{};
        ctx.gpr[7] = row;
        ctx.gpr[29] = 0x09F00000u;
        ctx.gpr[31] = ra;
        ctx.pc = kWindowDraw;
        runtime.function_at(kWindowDraw)(runtime, ctx);
        return logged.empty() ? 0xFFu : logged[0].args[3];
    };
    check(highlight(kRow, kMenuDrawReturn) == 3u && highlight(3u, kMenuDrawReturn) == 4u &&
              highlight(1u, kMenuDrawReturn) == 1u,
        "the box menu highlights the row where it is shown");
    check(highlight(kRow, 0x09AE4D58u) == kRow, "another window's row is its own");

    // The box on Item Sets: its update, its menu dimmed, then the screen's menu.
    // Choosing Item Sets on the box's menu: the screens start in that same
    // frame, with the box's sound, and the box's own frame does not run (it
    // would show Put In Item's message and no menu for a frame).
    memory.store8(kScreenBox + kPhase, 0u);
    memory.store8(kScreenBox + kChosenRow, 2u);
    memory.store8(kScreenBox + kMenuShown, 1u);
    check(press(runtime, menu::kConfirm).size() == 1u && logged[0].function == kBoxFrame && !screen_open(),
        "choosing another row is the box's own");
    memory.store8(kScreenBox + kChosenRow, kRow);
    check(press(runtime, menu::kConfirm | menu::kDown)[0].function == kBoxFrame && !screen_open(),
        "nor when the cursor moves in the same press");
    memory.store8(kScreenBox + kMenuShown, 0u);
    check(press(runtime, menu::kConfirm)[0].function == kBoxFrame && !screen_open(), "nor with the menu hidden");
    memory.store8(kScreenBox + kMenuShown, 1u);
    memory.store32(kUi + kUiFlags, 0xFFu);
    std::vector<Logged> calls = press(runtime, menu::kConfirm);
    check(screen_open() && memory.load8(kScreenBox + kPhase) == 1u, "choosing Item Sets opens the screens at once");
    check(find(calls, kBoxFrame) == nullptr && sound_of(calls) == 19u, "in place of the box's frame, with its sound");
    check(calls.size() == 5u && calls[1].function == kBoxMenu && calls[2].function == kWindowBuild &&
              calls[2].args[1] == kChooseTemplate,
        "and the box's menu and the screen's menu drawn in that frame");
    check(memory.load32(kUi + kUiFlags) == 0xFBu, "the update's message for the box's rows is turned off, nothing else");
    calls = press(runtime, 0u);
    check(calls.size() == 4u && find(calls, kBoxFrame) == nullptr, "the next frame is the screens' too");
    check(calls[0].function == kUpdate && calls[0].args[0] == kScreenBox, "the box's update first");
    check(calls[1].function == kBoxMenu && calls[1].args[0] == kScreenBox && calls[1].args[1] == 1u,
        "then its menu, dimmed");
    check(calls[2].function == kWindowBuild && calls[2].args[0] == kWindowRecord && calls[2].args[1] == kChooseTemplate &&
              calls[2].args[2] == 0u,
        "the menu's window is built from its template");
    check(calls[3].function == kWindowDraw && calls[3].args[0] == kUi && calls[3].args[1] == kWindowRecord &&
              calls[3].args[3] == 0u && calls[3].args[5] == 1u,
        "and drawn on its first row with the lobby's text");

    check(memory.load16(kChooseTemplate) == 176u && memory.load16(kChooseTemplate + 10u) == 2u &&
              memory.load32(kChooseTemplate + 12u) == kChooseIds && memory.load16(kChooseIds) == kChooseId &&
              memory.load16(kChooseIds + 4u) == 0u,
        "two rows, their ids ended by 0");
    check(call(runtime, kText, 0u, kChooseId, 0u, 0x0882F884u) == kRowText &&
              guest_text(memory, kRowText) == "Load Item Set" &&
              guest_text(memory, call(runtime, kText, 0u, kChooseId + 1u, 0u, 0x0882F884u)) == "Register Pouch",
        "the rows' words are answered");
    check(guest_text(memory, call(runtime, kText, 0u, kTextId, 0u, 0x0882F884u)) == "Item Sets", "and the box's row");

    // Register Pouch into the first set.
    check(sound_of(press(runtime, menu::kDown)) == 23u, "the cursor sounds as the game's");
    calls = press(runtime, menu::kConfirm);
    check(sound_of(calls) == 19u, "choosing sounds");
    const Logged *build = find(calls, kWindowBuild);
    check(build != nullptr && build->args[1] == kListTemplate && build->args[2] == 1u, "the list has a title");
    const Logged *draw = find(calls, kWindowDraw);
    check(draw != nullptr && draw->args[2] == kTitleText && draw->args[3] == 0u, "and the cursor on the first set");
    check(guest_text(memory, kTitleText).empty() && guest_text(memory, kListTitle) == "Register Set",
        "titled with what it does");
    const Logged *title = nullptr;
    for (const Logged &c : calls)
        if (c.function == kPrint && c.args[3] == kListTitle) title = &c;
    check(title != nullptr && title->args[1] == 194u && title->args[2] == 47u,
        "the title is printed centred between the arrow and the page number");
    check(find(calls, kFontColour) != nullptr && find(calls, kFontColour)->args[1] == 5u, "in the titles' yellow");
    check(memory.load16(kListTemplate + 10u) == kSetsPerPage && memory.load16(kListIds + 7u * 2u) == kLastId &&
              memory.load16(kListIds + 8u * 2u) == 0u,
        "eight sets");
    check(guest_text(memory, call(runtime, kText, 0u, kSetId + 7u, 0u, 0x0882F884u)) == "Item Set 8", "named");
    const Logged *pager = find(calls, kPager);
    check(pager != nullptr && pager->args[1] == kWindowRecord && pager->args[2] == 0u && pager->args[3] == kSetPages,
        "with the page's arrows and number in its title");
    // L and R turn the page of sets.
    calls = press(runtime, menu::kR);
    check(sound_of(calls) == 17u && find(calls, kPager)->args[2] == 1u && find(calls, kWindowDraw)->args[3] == 0u,
        "R shows the second page, on the same row");
    check(guest_text(memory, call(runtime, kText, 0u, kSetId, 0u, 0x0882F884u)) == "Item Set 9" &&
              guest_text(memory, call(runtime, kText, 0u, kSetId + 7u, 0u, 0x0882F884u)) == "Item Set 16",
        "with its sets' names");
    calls = press(runtime, menu::kL);
    check(find(calls, kPager)->args[2] == 0u &&
              guest_text(memory, call(runtime, kText, 0u, kSetId, 0u, 0x0882F884u)) == "Item Set 1",
        "L goes back");
    const Logged *items = find(calls, kItemWindow);
    check(items != nullptr && items->args[1] == kSetSlots && items->args[2] == 0u && items->args[3] == 0u &&
              items->args[4] == 0x10u,
        "the set's items in the pouch's window, with its pages and no cursor");
    const Logged *message = find(calls, kMessageDraw);
    check(message != nullptr && message->args[1] == kMessageRecord && message->args[2] == kMessageText &&
              guest_text(memory, kMessageText) == "Please select where to\nregister the pouch.",
        "the prompt in Put In Item's message window");
    const Logged *hint = print_at(calls, 0xFFFFu, kHintText);
    check(hint != nullptr && hint->args[1] == 224u && hint->args[2] == 241u && hint->args[3] == kHintText &&
              guest_text(memory, kHintText) == "~C00~B01Register Set  ~B02Delete Set",
        "with the choose button's and the triangle's hints, in the game's markup");
    calls = press(runtime, menu::kConfirm);
    check(sound_of(calls) == 137u && guest_text(memory, kMessageText) == "The pouch is registered\nas Item Set 1.",
        "an empty set takes the pouch, with the equipment set's sound");
    check(store().sets("SETS")[0].items[0] == (Slot{kPotion, 10u}), "and it is kept");
    check(memory.load16(kSetSlots) == kPotion && memory.load16(kSetSlots + 2u) == 10u, "the window shows it");

    // Load it back after the pouch changed.
    set_pouch(ram, 0, kHerb, 3u);
    set_box(ram, 0, kPotion, 20u);
    press(runtime, menu::kCancel);
    press(runtime, menu::kUp);
    press(runtime, menu::kConfirm);
    calls = press(runtime, menu::kConfirm);
    check(guest_text(memory, kMessageText) == "Load Item Set 1?\nThe pouch goes into the box.", "loading asks");
    const Logged *yes_no = print_at(calls, 320u);
    check(yes_no != nullptr && yes_no->args[1] == 320u && yes_no->args[2] == 240u && yes_no->args[4] == kYesNoText &&
              guest_text(memory, kYesNoText).find("[Yes]") != std::string::npos,
        "with Yes and No where Put In Item has them, on Yes");
    check(print_at(calls, 0xFFFFu, kHintText) == nullptr, "and no hint");
    calls = press(runtime, menu::kConfirm);
    check(guest_text(memory, kMessageText) == "Item Set 1 loaded." && sound_of(calls) == 136u &&
              find(calls, kSound)->args[1] == 1u,
        "Yes loads it, with Take Out Item's sound");
    check(guest_text(memory, kHintText) == "~C00~B01Load Set  ~B02Delete Set", "the list's hint says load");
    check(read_pouch(ram, kSys)[0] == (Slot{kPotion, 10u}), "the potions are back in the pouch");
    check(box_count(ram, kSys, kHerb) == 3u && box_count(ram, kSys, kPotion) == 10u, "the herbs went into the box");

    // Delete it.
    press(runtime, menu::kDelete);
    press(runtime, menu::kLeft);
    calls = press(runtime, menu::kConfirm);
    check(guest_text(memory, kMessageText) == "Item Set 1 is deleted." && store().sets("SETS")[0].empty() &&
              sound_of(calls) == 19u,
        "triangle and Yes delete the set");

    // Back out: from the list to the menu, then the box's menu, on Item Sets.
    calls = press(runtime, menu::kCancel);
    check(screen_open() && find(calls, kWindowBuild) != nullptr && find(calls, kWindowBuild)->args[1] == kChooseTemplate &&
              sound_of(calls) == 20u,
        "back from the list is the menu again");
    calls = press(runtime, menu::kDown);
    check(find(calls, kWindowDraw)->args[3] == 1u, "and it moves");
    memory.store32(kScreenBox + kBoxFlags, 0xFFu);
    calls = press(runtime, menu::kCancel);
    check(!screen_open() && memory.load8(kScreenBox + kPhase) == 0u, "back from the menu is the box's menu");
    check(memory.load32(kScreenBox + kBoxFlags) == 0xF8u, "with the box's screen flags cleared as its own back does");
    check(calls.size() == 2u && calls[0].function == kUpdate && sound_of(calls) == 20u, "after the box's update");
    calls = press(runtime, 0u);
    check(calls.size() == 1u && calls[0].function == kBoxFrame, "and the box runs as before");

    // The input being someone else's: no buttons.
    memory.store32(kInputOwner, 0x09990000u);
    memory.store8(0x09990000u + kInputBlocked, 1u);
    memory.store8(kScreenBox + kPhase, 1u);
    press(runtime, 0u);
    check(sound_of(press(runtime, menu::kCancel)) == 0u && screen_open(), "blocked input does nothing");
    memory.store8(0x09990000u + kInputBlocked, 0u);
    press(runtime, menu::kCancel);
    check(!screen_open(), "and the screens close once it is back");

    // No GameSys: the box goes back to its menu.
    memory.store32(kGameSysPointer, 0u);
    memory.store8(kScreenBox + kPhase, 1u);
    calls = press(runtime, 0u);
    check(memory.load8(kScreenBox + kPhase) == 0u && calls.size() == 1u && calls[0].function == kBoxFrame &&
              !screen_open(),
        "without the hunter's data there is nothing to show");
}

// Before anything reads the settings: MHFU_ITEM_SETS=0 wins over the default.
void test_setting() {
    check(mhfu::settings::defaults().item_sets, "item sets are on by default");
    set_environment("MHFU_ITEM_SETS", "0");
    check(!mhfu::settings::current().item_sets, "MHFU_ITEM_SETS=0 turns them off");
    set_environment("MHFU_ITEM_SETS", "");
}

} // namespace

int main() {
    const std::filesystem::path data = std::filesystem::temp_directory_path() / "pokee_item_sets_tests";
    // A run that failed may have left its sets behind.
    std::error_code stale;
    std::filesystem::remove_all(data, stale);
    std::filesystem::create_directories(data);
    set_environment("MHFU_DATA_DIR", data.string());
    set_environment("MHFU_TRACE_ITEM_SETS", "1");
    test_setting();
    test_reading();
    test_load();
    test_carry_limit();
    test_box_full();
    test_large_stacks();
    test_store();
    test_signatures();
    test_hooks();
    test_rows();
    test_menu();
    test_guest_calls();
    test_screens();
    std::error_code ec;
    std::filesystem::remove_all(data, ec);
    if (failures == 0) std::cout << "item sets tests passed\n";
    return failures == 0 ? 0 : 1;
}
