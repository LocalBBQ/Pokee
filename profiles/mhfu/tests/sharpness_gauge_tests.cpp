// Sharpness gauge: the colour and how full the game's bar still is, the
// weapon's colour ends through the game's tables, and the cut that shortens
// the bar. No game data. Guest memory is a buffer laid out like the traced
// great sword (id 182).
#include "game/guest_ram.hpp"
#include "game/sharpness_gauge.hpp"

#include <cmath>
#include <iostream>

namespace {
using namespace mhfu::game;
using namespace mhfu::game::sharpness;

int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

bool near(float a, float b) {
    return std::fabs(a - b) < 1e-4f;
}

// The traced great sword: its row in the class table, and the most without
// Handicraft.
constexpr Ends kTraced{50, 80, 210, 330, 390, 430, 450};
constexpr int kTracedMost = 250;

void test_colour_index() {
    check(colour_index(kTraced, 0) == 0 && colour_index(kTraced, 50) == 0, "up to the first end is red");
    check(colour_index(kTraced, 51) == 1 && colour_index(kTraced, 80) == 1, "past it is orange, to its end");
    check(colour_index(kTraced, 210) == 2 && colour_index(kTraced, 211) == 3, "210 is yellow and 211 green");
    check(colour_index(kTraced, 249) == 3, "the traced 249 points are green, as the game's byte said");
    check(colour_index(kTraced, 450) == 6 && colour_index(kTraced, 999) == 6, "past the last end it stays purple");
    constexpr Ends no_red{0, 0, 100, 200, 200, 200, 200};
    check(colour_index(no_red, 1) == 2, "a colour of no length is skipped");
}

void test_gauge_reading() {
    const std::optional<Reading> full = gauge_reading(kTraced, 250, kTracedMost);
    check(full && full->colour == Sharpness::Green && full->low == 210 && full->high == 250 && near(full->fill, 1.0f),
        "a fresh blade fills green up to the most, not to the table's 330");
    const std::optional<Reading> traced = gauge_reading(kTraced, 249, kTracedMost);
    check(traced && near(traced->fill, 39.0f / 40.0f), "one hit takes a fortieth of that green");
    const std::optional<Reading> yellow = gauge_reading(kTraced, 210, kTracedMost);
    check(yellow && yellow->colour == Sharpness::Yellow && yellow->low == 80 && yellow->high == 210 &&
            near(yellow->fill, 1.0f),
        "the first point of yellow is a full yellow bar");
    const std::optional<Reading> half = gauge_reading(kTraced, 145, kTracedMost);
    check(half && near(half->fill, 0.5f), "halfway through yellow is half a bar");
    const std::optional<Reading> blunt = gauge_reading(kTraced, 0, kTracedMost);
    check(blunt && blunt->colour == Sharpness::Red && near(blunt->fill, 0.0f), "no sharpness is an empty red bar");
    const std::optional<Reading> handicraft = gauge_reading(kTraced, 300, 300);
    check(handicraft && handicraft->colour == Sharpness::Green && handicraft->high == 300,
        "Handicraft's longer most lengthens the colour it reaches");
    check(gauge_reading(kTraced, 400, kTracedMost)->points == kTracedMost, "points above the most are clamped");
    check(!gauge_reading(kTraced, 10, 0), "no most is no gauge");
    check(!gauge_reading(Ends{50, 40, 60, 70, 80, 90, 100}, 10, 100), "a row that falls is not a weapon's");
    check(!gauge_reading(Ends{-1, 40, 60, 70, 80, 90, 100}, 10, 100), "a negative end is not a weapon's");
}

constexpr std::uint32_t kBase = 0x08800000u;
constexpr std::uint32_t kHunter = 0x08A00000u;
constexpr std::uint32_t kHunters = 0x08A10000u;
constexpr std::uint32_t kTable = 0x08979280u;
constexpr std::uint16_t kId = 182u;
constexpr std::uint16_t kModel = 77u;
constexpr std::uint8_t kClass = 4u;
constexpr std::uint8_t kRow = 1u;

BufferRam traced_memory() {
    BufferRam ram(kBase, 0x300000u);
    ram.store32(kHunterList, kHunters);
    ram.store32(kHunters + kFollowedHunter, kHunter);
    ram.store8(kHunter + kWeaponSlot, 1u);
    ram.store8(kHunter + kWeaponSlot + 1u, kWeaponKind);
    ram.store16(kHunter + kWeaponSlot + 2u, kId);
    ram.store16(kHunter + kPoints, 249u);
    ram.store16(kHunter + kMostPoints, kTracedMost);
    ram.store8(kHunter + kColourByte, 3u);
    const std::uint32_t record = kWeaponRecords + kId * kWeaponRecordBytes;
    ram.store16(record, kModel);
    ram.store8(record + kWeaponRow, kRow);
    ram.store8(kModelClasses + kModel, kClass);
    ram.store32(kClassTables + kClass * 4u, kTable);
    for (std::size_t i = 0; i < kColours; ++i)
        ram.store16(
            kTable + kRow * kRowBytes + static_cast<std::uint32_t>(i) * 2u, static_cast<std::uint16_t>(kTraced[i]));
    return ram;
}

void test_tables() {
    BufferRam ram = traced_memory();
    check(followed_hunter(ram) == kHunter, "the followed hunter is read from the hunter list");
    const std::optional<Ends> ends = weapon_ends(ram, kHunter);
    check(ends && *ends == kTraced, "the weapon's row is found through its record, model class and table");
    const std::optional<Reading> reading = read_hunter(ram, kHunter);
    check(reading && reading->colour == Sharpness::Green && reading->points == 249 && reading->game_colour == 3,
        "the hunter's points read as the colour the game stored");

    ram.store16(kHunter + kPoints, 210u);
    check(read_hunter(ram, kHunter)->colour == Sharpness::Yellow, "and follow the points as they fall");

    BufferRam no_weapon = traced_memory();
    no_weapon.store8(kHunter + kWeaponSlot + 1u, 0u);
    check(!read_hunter(no_weapon, kHunter), "a slot that holds no weapon gives no gauge");
    BufferRam no_table = traced_memory();
    no_table.store32(kClassTables + kClass * 4u, 0u);
    check(!weapon_ends(no_table, kHunter), "a class with no table gives no ends");
    BufferRam outside = traced_memory();
    outside.store32(kClassTables + kClass * 4u, 0x0A000000u);
    check(!weapon_ends(outside, kHunter), "a table outside memory gives no ends");
    BufferRam bad_class = traced_memory();
    bad_class.store8(kModelClasses + kModel, 12u);
    check(!weapon_ends(bad_class, kHunter), "a class past the table's eleven gives no ends");
    check(!read_hunter(ram, 0u), "no hunter gives no gauge");
    BufferRam empty(kBase, 0x300000u);
    check(followed_hunter(empty) == 0u, "an empty hunter list is no hunter");
}

void test_cut() {
    const Edge full = cut_right(kBarLeft, kBarLeft + kBarWidth, 224.0f, 240.0f, 1.0f);
    check(near(full.x, kBarLeft + kBarWidth) && near(full.u, 240.0f) && near(cap_shift(kBarWidth, 1.0f), 0.0f),
        "a fresh blade keeps the bar and the cap where the game drew them");
    const float one = 249.0f / static_cast<float>(kTracedMost);
    const Edge spent = cut_right(kBarLeft, kBarLeft + kBarWidth, 224.0f, 240.0f, one);
    check(near(spent.x, kBarLeft + kBarWidth * one) && near(spent.u, 224.0f + 16.0f * one),
        "one point takes that fraction off the bar, and the texel moves with the cut");
    check(near(cap_shift(kBarWidth, one), kBarWidth * (one - 1.0f)), "the cap follows the end back by the same amount");
    const Edge yellow =
        cut_right(kBarLeft, kBarLeft + kBarWidth, 0.0f, 16.0f, 210.0f / static_cast<float>(kTracedMost));
    check(near(yellow.x, kBarLeft + kBarWidth * 210.0f / static_cast<float>(kTracedMost)),
        "the end of yellow is that far along the bar the game drew");
    const Edge empty = cut_right(kBarLeft, kBarLeft + kBarWidth, 224.0f, 240.0f, 0.0f);
    check(near(empty.x, kBarLeft) && near(empty.u, 224.0f), "no sharpness leaves the bar's left edge and its texel");
    check(near(cap_shift(48.0f, 0.5f), -24.0f), "a bar the HUD has shrunk still loses half of its own width");
}

void test_remaining() {
    reset_for_tests();
    const Reading green = *gauge_reading(kTraced, 249, kTracedMost);
    check(bar_remaining() < 0.0f && colour_remaining() < 0.0f, "the drawing is left whole before a reading");

    set_reading_for_tests(green, false);
    check(bar_remaining() < 0.0f && colour_remaining() < 0.0f,
        "with the setting off the game's drawing stays whole");

    set_reading_for_tests(green, true);
    check(near(bar_remaining(), 249.0f / static_cast<float>(kTracedMost)),
        "with the setting on the bar keeps the fraction of sharpness that is left");
    check(near(colour_remaining(), 39.0f / 40.0f), "one hit takes a fortieth of the colour on the knife");
    set_reading_for_tests(*gauge_reading(kTraced, kTracedMost, kTracedMost), true);
    check(near(bar_remaining(), 1.0f) && near(colour_remaining(), 1.0f),
        "a fresh blade keeps the whole bar and the whole colour");
    set_reading_for_tests(*gauge_reading(kTraced, 0, kTracedMost), true);
    check(near(bar_remaining(), 0.0f) && near(colour_remaining(), 0.0f),
        "no sharpness cuts the bar and the colour away");

    set_reading_for_tests(std::nullopt, true);
    check(bar_remaining() < 0.0f && colour_remaining() < 0.0f, "a gun has no bar and the game's HUD is left alone");

    set_reading_for_tests(green, true);
    gauge_frame(true, nullptr);
    check(bar_remaining() < 0.0f && colour_remaining() < 0.0f, "a frame with no memory forgets the reading");
    reset_for_tests();
}

} // namespace

int main() {
    test_colour_index();
    test_gauge_reading();
    test_tables();
    test_cut();
    test_remaining();
    if (failures != 0) {
        std::cerr << failures << " sharpness gauge test(s) failed\n";
        return 1;
    }
    std::cout << "sharpness gauge tests passed\n";
    return 0;
}
