// Item sets: loading a set through the box as the game's own deposit would,
// the sets file, the hunter's name, and the hooks on stand-ins for the box
// menu (its record, its lists, the text, cursor and update calls). No game
// data.
#include "game/item_sets.hpp"

#include "game/guest_ram.hpp"
#include "psprecomp/runtime.hpp"
#include "settings/settings.hpp"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

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
        out << "hunter LOU\r\nset 9 Out of range\nitems 7:10\nset 1\nitems 7:10 bad 99999:1 1:70000 64:3\nset x\n";
    }
    Store damaged(file);
    const Sets sets = damaged.sets("LOU");
    check(sets[0].name == "Item Set 1", "a set without a name gets the default");
    check(sets[0].items[0] == (Slot{kPotion, 10u}) && sets[0].items[4] == (Slot{kPaintball, 3u}),
        "readable slots are kept");
    check(sets[0].items[2] == Slot{} && sets[0].items[3] == Slot{}, "an id or count out of range is dropped");
    check(sets[1].empty(), "a set past eight is ignored");
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
    memory.store32(kBoxList, 0x01270126u);
    memory.store32(kSmallBoxList, 0x012D012Cu);
    memory.store32(0x09B05164u, 0u);
    memory.store32(0x09B05174u, 0u);
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
    runtime.register_function(kUpdate, &game_update, "game_update");
    frame(runtime);
    check(memory.load32(kMenuRecord + 8u) == 0x00070014u, "the menu has seven rows");
    check(memory.load32(kMenuRecord) == 0x000C0010u, "and is raised clear of the hints");
    check(memory.load16(0x09B05164u) == kTextId && memory.load16(0x09B05166u) == 0u, "the box's list ends in Item Sets");
    check(memory.load16(0x09B05174u) == kTextId && memory.load16(0x09B05176u) == 0u, "so does the Small Item Box's");
    check(runtime.function_at(kText) != &game_text && runtime.function_at(kUpdate) != &game_update,
        "the text and the update are wrapped");
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

    // The cursor: seven rows for the box only.
    call(runtime, kCursor, 0u, 0x09B1A97Eu, kGameRows, kCursorReturn);
    check(cursor_calls == 1 && cursor_rows == kRows, "the box's cursor gets seven rows");
    call(runtime, kCursor, 0u, 0x09B1A97Eu, kGameRows, 0x09A00000u);
    check(cursor_rows == kGameRows, "another menu keeps its count");

    // The update: waiting on Item Sets asks for the screen and holds the box.
    constexpr std::uint32_t kBoxRecord = 0x09B1A948u;
    update_calls = 0;
    call(runtime, kUpdate, kBoxRecord, 0u, 0u, 0x09ADD0F0u);
    check(update_calls == 1, "the menu's frame runs the game's update");
    check(!take_screen_request(), "nothing is asked for");
    memory.store8(kBoxRecord + kPhase, 1u);
    memory.store8(kBoxRecord + kChosenRow, kRow);
    call(runtime, kUpdate, kBoxRecord, 0u, 0u, 0x09ADD0F0u);
    call(runtime, kUpdate, kBoxRecord, 0u, 0u, 0x09ADD0F0u);
    check(update_calls == 1, "the box waits while the screen is open");
    check(screen_open() && take_screen_request() && !take_screen_request(), "the screen is asked for once");
    screen_closed();
    call(runtime, kUpdate, kBoxRecord, 0u, 0u, 0x09ADD0F0u);
    check(memory.load8(kBoxRecord + kPhase) == 0u && update_calls == 2, "closing goes back to the menu");
    check(!screen_open(), "and the screen is closed");
    memory.store8(kBoxRecord + kPhase, 1u);
    memory.store8(kBoxRecord + kChosenRow, 2u);
    call(runtime, kUpdate, kBoxRecord, 0u, 0u, 0x09ADD0F0u);
    check(update_calls == 3 && !take_screen_request(), "the game's own rows run as before");

    // Off: the game's six rows, and the cursor keeps them.
    mhfu::settings::current().item_sets = false;
    frame(runtime);
    check(memory.load32(kMenuRecord + 8u) == 0x00060014u && memory.load32(kMenuRecord) == 0x00180010u,
        "off puts the six rows back");
    check(memory.load32(0x09B05164u) == 0u && memory.load32(0x09B05174u) == 0u, "and the lists");
    call(runtime, kCursor, 0u, 0x09B1A97Eu, kGameRows, kCursorReturn);
    check(cursor_rows == kGameRows, "the cursor keeps six");
    mhfu::settings::current().item_sets = true;

    // Another overlay in the slot: nothing is changed.
    memory.store8(0x09A5F300u + 32u, 'g');
    memory.store32(kMenuRecord + 8u, 0x00060014u);
    frame(runtime);
    check(memory.load32(kMenuRecord + 8u) == 0x00060014u, "another overlay is left alone");
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
    std::error_code ec;
    std::filesystem::remove_all(data, ec);
    if (failures == 0) std::cout << "item sets tests passed\n";
    return failures == 0 ? 0 : 1;
}
