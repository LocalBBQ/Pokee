#include "game/item_sets.hpp"

#include "game/game_data.hpp"
#include "game/guest_calls.hpp"
#include "game/guest_layout.hpp"
#include "game/guest_ram.hpp"
#include "game/hooks.hpp"
#include "game/item_sets_menu.hpp"
#include "install/release_addresses.hpp"
#include "install/user_data.hpp"
#include "psprecomp/runtime.hpp"
#include "settings/settings.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <utility>

namespace mhfu::game::item_sets {
namespace {

// Item Sets' addresses are the executable's, lobby_task's and the host's
// scratch memory's, all named as ULUS-10391 has them.
std::uint32_t ported(std::uint32_t traced) {
    return install::port_from(traced, install::Image::lobby_task);
}

constexpr std::array<CodeWord, 13> kCodeSignature = {{
    {kCursor, 0x27BDFFE0u},      // addiu sp, sp, -32
    {0x0881C2ECu, 0x90A40000u},  // lbu a0, 0(a1): the row
    {0x0882F880u, 0x0E69EE24u},  // jal 0x09A7B890: a row's text
    {kWindowBuild, 0x7C081E20u}, // seh v1, t0
    {kWindowDraw, 0x27BDFFC0u},  // addiu sp, sp, -64
    {kPager, 0x27BDFFC0u},       // addiu sp, sp, -64
    {kFontSize, 0xA085012Cu},    // sb a1, 300(a0)
    {kFontColour + 4u, 0xA085012Eu}, // sb a1, 302(a0)
    {kItemWindow, 0x27BDFF80u},  // addiu sp, sp, -128
    {kMessageDraw, 0x27BDFFF0u}, // addiu sp, sp, -16
    {kPrint, 0x27BDFCA0u},       // addiu sp, sp, -864
    {kSound, 0x27BDFFC0u},       // addiu sp, sp, -64
    {kPrintFormat, 0x00007325u}, // "%s"
}};

// lobby_task's code. The menu's data words are checked apart, since the
// hooks change them.
constexpr std::array<CodeWord, 26> kOverlaySignature = {{
    {kText, 0x00A03021u},       // addu a2, a1, zero
    {0x09A7B894u, 0x0A69EE08u}, // j 0x09A7B820
    {0x09A7B898u, 0x00002821u}, // addu a1, zero, zero: the first table
    {0x09ADCE08u, 0x26450036u}, // addiu a1, s2, 54: the row
    {0x09ADCE0Cu, 0x0E2070B9u}, // jal 0x0881C2E4
    {0x09ADCE10u, 0x24070006u}, // addiu a3, zero, 6: the rows
    {0x09ADCEB4u, 0xA2440038u}, // sb a0, 56(s2): the phase
    {0x09ADCF64u, 0x92420036u}, // lbu v0, 54(s2)
    {0x09ADCF68u, 0x2C410006u}, // sltiu at, v0, 6: rows 0 to 5 start
    {kUpdate, 0x27BDFFF0u},     // addiu sp, sp, -16
    {kBoxFrame, 0x27BDFFD0u},   // addiu sp, sp, -48
    {0x09ADD008u, 0x30422000u}, // andi v0, v0, 0x2000: back, for every row
    {0x09ADD0E8u, 0x0E6B8E3Eu}, // jal 0x09AE38F8: the update
    {0x09ADD100u, 0x24020001u}, // addiu v0, zero, 1
    {0x09AE39F0u, 0x92040038u}, // lbu a0, 56(s0): the phase
    {0x09AE3A20u, 0x2C610006u}, // sltiu at, v1, 6: rows 0 to 5 run
    {0x09AE6720u, 0x24A5B0A8u}, // addiu a1, a1, -0x4F58: the menu record
    {0x09AE675Cu, 0x24425168u}, // addiu v0, v0, 0x5168: the Small Item Box's rows
    {kBoxMenu, 0x27BDFFC0u},    // addiu sp, sp, -64
    {0x09AE6768u, 0x92070036u}, // lbu a3, 54(s0): the row to highlight
    {0x09AE677Cu, 0x0E20BD60u}, // jal 0x0882F580, returning to kMenuDrawReturn
    {0x09AE3958u, 0x8CC305E8u}, // lw v1, 1512(a2): the update's message flag
    {0x09ADCE14u, 0x32024000u}, // andi v0, s0, 0x4000: the menu's choose
    {0x09ADCF5Cu, 0x34420004u}, // ori v0, v0, 4: set once a row is chosen
    {kMessageRecord, 0x00C800D8u},      // Put In Item's message window: x 216, y 200
    {kMessageRecord + 4u, 0x00400100u}, // 256 by 64
}};

// The menu record's row count and list, and the two lists, as the game has
// them and as the hooks leave them.
// The record's rows word is u16 row height, then u16 rows. Only the count is
// the hooks': ULUS-10391's rows are 0x14 high, Freedom Unite Complete's 0x12.
constexpr std::uint32_t kRowCountShift = 16u;
constexpr std::uint32_t row_count(std::uint32_t rows) {
    return rows >> kRowCountShift;
}
constexpr std::uint32_t with_rows(std::uint32_t rows, std::uint32_t count) {
    return (rows & 0xFFFFu) | (count << kRowCountShift);
}
// Each list is eight halfwords: six ids and two of padding, the first ending
// it. Item Sets goes after Combine Items and the rest move down into the
// padding.
using List = std::array<std::uint16_t, 8>;
constexpr List kBoxRows = {294u, 295u, 296u, 297u, 298u, 299u, 0u, 0u};
constexpr List kPatchedBoxRows = {294u, 295u, 296u, kTextId, 297u, 298u, 299u, 0u};
constexpr List kSmallBoxRows = {300u, 301u, 302u, 303u, 304u, 305u, 0u, 0u};
constexpr List kPatchedSmallBoxRows = {300u, 301u, 302u, kTextId, 303u, 304u, 305u, 0u};
// The record's place, u16 x then u16 y. Seven rows reach the hints under the
// window (Rotate, Menu OFF), so the window is raised by 12.
constexpr std::uint32_t kRecordPlace = 0x00180010u;
constexpr std::uint32_t kPatchedPlace = 0x000C0010u;

// Where the screens sit: the menu and the list beside the box's menu, the
// pouch's window where Put In Item has it, and the hint and the Yes/No in
// Put In Item's message window. The colour is the box menu's.
constexpr std::uint16_t kChooseX = 176u;
constexpr std::uint16_t kChooseY = 56u;
constexpr std::uint16_t kListX = 176u;
constexpr std::uint16_t kListY = 40u;
constexpr std::uint32_t kYesNoX = 320u;
constexpr std::uint32_t kYesNoY = 240u;
constexpr std::uint32_t kHintX = 224u;
constexpr std::uint32_t kHintY = 241u;
constexpr std::uint32_t kWindowColour = 0x8000C040u;
// The list: 13 characters of 20 make it 152 wide (0x0882FA24), up to the
// pouch's window at 328. Its title goes between the left arrow and the page
// number ("1/3", ending 21 in from the right), 7 pixels a character.
constexpr std::uint8_t kListChars = 13u;
constexpr std::uint32_t kListWidth = ((kListChars * 20u / 2u + 23u) & 0x7FF8u);
constexpr std::uint32_t kCharWidth = 7u;
constexpr std::uint32_t kTitleLeft = kListX + 14u;
constexpr std::uint32_t kTitleRight = kListX + kListWidth - 21u - 3u * kCharWidth - 4u;
constexpr std::uint32_t kTitleColour = 5u;

std::uint32_t title_x(const std::string &title) {
    const std::uint32_t width = static_cast<std::uint32_t>(title.size()) * kCharWidth;
    const std::uint32_t middle = (kTitleLeft + kTitleRight) / 2u;
    return width / 2u >= middle - kTitleLeft ? kTitleLeft : middle - width / 2u;
}

std::uint16_t clamp16(std::uint32_t value) {
    return static_cast<std::uint16_t>(std::min<std::uint32_t>(value, 0xFFFFu));
}

bool trace_enabled() {
    static const bool trace = [] {
        const char *text = std::getenv("MHFU_TRACE_ITEM_SETS");
        return text != nullptr && *text != '\0' && std::string(text) != "0";
    }();
    return trace;
}

std::string utf16_to_utf8(const std::vector<std::uint16_t> &units) {
    std::string out;
    for (std::uint16_t unit : units) {
        const std::uint32_t c = unit;
        if (c < 0x80u) {
            out += static_cast<char>(c);
        } else if (c < 0x800u) {
            out += static_cast<char>(0xC0u | (c >> 6u));
            out += static_cast<char>(0x80u | (c & 0x3Fu));
        } else {
            out += static_cast<char>(0xE0u | (c >> 12u));
            out += static_cast<char>(0x80u | ((c >> 6u) & 0x3Fu));
            out += static_cast<char>(0x80u | (c & 0x3Fu));
        }
    }
    return out;
}

// One box stack's place, as loaded into a copy.
std::vector<Slot> read_box(const Ram &ram, std::uint32_t sys) {
    std::vector<Slot> box(box_capacity(ram, sys));
    for (std::uint32_t i = 0; i < box.size(); ++i)
        box[i] = {ram.load16(sys + kBox + i * 4u), ram.load16(sys + kBox + i * 4u + 2u)};
    return box;
}

// Puts `count` of `item` in the copy as 0x08853ABC does; what did not fit.
std::uint32_t deposit(std::vector<Slot> &box, std::uint16_t item, std::uint32_t count, std::uint16_t limit) {
    for (Slot &slot : box) {
        if (count == 0u) return 0u;
        if (slot.item != item || slot.count >= limit) continue;
        const std::uint32_t room = limit - slot.count;
        const std::uint32_t put = std::min(room, count);
        slot.count = static_cast<std::uint16_t>(slot.count + put);
        count -= put;
    }
    for (Slot &slot : box) {
        if (count == 0u) return 0u;
        if (slot.item != 0u) continue;
        const std::uint32_t put = std::min<std::uint32_t>(limit, count);
        slot = {item, static_cast<std::uint16_t>(put)};
        count -= put;
    }
    return count;
}

// Takes up to `count` of `item` out of the copy, from the last stack back, so
// the earlier stacks stay full. What it took.
std::uint32_t withdraw(std::vector<Slot> &box, std::uint16_t item, std::uint32_t count) {
    std::uint32_t taken = 0u;
    for (auto slot = box.rbegin(); slot != box.rend() && taken < count; ++slot) {
        if (slot->item != item) continue;
        const std::uint32_t take = std::min<std::uint32_t>(slot->count, count - taken);
        slot->count = static_cast<std::uint16_t>(slot->count - take);
        taken += take;
        if (slot->count == 0u) slot->item = 0u;
    }
    return taken;
}

void mark_obtained(Ram &ram, std::uint32_t sys, std::uint16_t item) {
    if (item == 0u || item >= kFlagIds) return;
    const std::uint32_t word = sys + kObtained + (item >> 5u) * 4u;
    ram.store32(word, ram.load32(word) | (1u << (item & 31u)));
}

} // namespace

std::span<const CodeWord> code_signature() {
    return kCodeSignature;
}

std::span<const CodeWord> overlay_signature() {
    return kOverlaySignature;
}

bool matches(const Ram &ram, std::span<const CodeWord> words) {
    for (const CodeWord &expected : words)
        if (!install::code_matches(ram, expected.address, expected.word, install::Image::lobby_task) &&
            !layout::box_button_test(ram, expected.address, expected.word))
            return false;
    return true;
}

std::uint8_t shown_row(std::uint8_t row) {
    if (row == kRow) return kShownRow;
    return row >= kShownRow && row < kRow ? static_cast<std::uint8_t>(row + 1u) : row;
}

std::uint8_t game_row(std::uint8_t shown) {
    if (shown == kShownRow) return kRow;
    return shown > kShownRow && shown < kRows ? static_cast<std::uint8_t>(shown - 1u) : shown;
}

std::uint8_t move_shown(std::uint8_t row, std::uint16_t buttons) {
    std::uint8_t shown = shown_row(row);
    if ((buttons & 0x10u) != 0u)
        shown = shown == 0u ? static_cast<std::uint8_t>(kRows - 1u) : static_cast<std::uint8_t>(shown - 1u);
    else if ((buttons & 0x40u) != 0u)
        shown = static_cast<std::uint8_t>((shown + 1u) % kRows);
    return game_row(shown);
}

// Box and pouch --------------------------------------------------------------

std::uint32_t game_sys(const Ram &ram) {
    if (!ram.contains(ported(kGameSysPointer), 4u)) return 0u;
    const std::uint32_t sys = ram.load32(ported(kGameSysPointer));
    if (!layout::in_ram(sys) || !ram.contains(sys, kPouch + kPouchSlots * 4u)) return 0u;
    return sys;
}

bool obtained(const Ram &ram, std::uint32_t sys, std::uint16_t item) {
    if (item >= kFlagIds) return false;
    return ((ram.load32(sys + kObtained + (item >> 5u) * 4u) >> (item & 31u)) & 1u) != 0u;
}

std::uint32_t box_capacity(const Ram &ram, std::uint32_t sys) {
    std::uint32_t pages = kBasePages;
    for (std::uint16_t item : kPageItems)
        if (obtained(ram, sys, item)) ++pages;
    return std::min(pages * kPageSlots, kBoxSlots);
}

std::uint16_t carry_limit(const Ram &ram, std::uint16_t item) {
    const std::uint32_t at = ported(kItemTable) + static_cast<std::uint32_t>(item) * kItemRecord + kCarryByte;
    return ram.contains(at, 1u) ? ram.load8(at) : std::uint16_t{0u};
}

std::uint16_t stack_limit(const Ram &ram, std::uint16_t item) {
    return carry_limit(ram, item) == kLargeStack ? kLargeStack : kStack;
}

Pouch read_pouch(const Ram &ram, std::uint32_t sys) {
    Pouch pouch{};
    for (std::uint32_t i = 0; i < kPouchSlots; ++i)
        pouch[i] = {ram.load16(sys + kPouch + i * 4u), ram.load16(sys + kPouch + i * 4u + 2u)};
    return pouch;
}

std::string hunter_name(const Ram &ram, std::uint32_t sys) {
    std::vector<std::uint16_t> units;
    for (std::uint32_t i = 0; i < kNameChars; ++i) {
        const std::uint16_t unit = ram.load16(sys + kName + i * 2u);
        if (unit == 0u) break;
        units.push_back(unit);
    }
    return utf16_to_utf8(units);
}

std::uint32_t box_count(const Ram &ram, std::uint32_t sys, std::uint16_t item) {
    std::uint32_t count = 0u;
    for (const Slot &slot : read_box(ram, sys))
        if (slot.item == item) count += slot.count;
    return count;
}

// A set ------------------------------------------------------------------------

bool ItemSet::empty() const {
    return std::all_of(items.begin(), items.end(), [](const Slot &s) { return s.item == 0u || s.count == 0u; });
}

Loaded load(Ram &ram, std::uint32_t sys, const ItemSet &set) {
    Loaded result;
    std::vector<Slot> box = read_box(ram, sys);
    Pouch pouch = read_pouch(ram, sys);
    std::vector<std::uint16_t> deposited;
    for (Slot &slot : pouch) {
        if (slot.item == 0u || slot.count == 0u) {
            slot = {};
            continue;
        }
        if (deposit(box, slot.item, slot.count, stack_limit(ram, slot.item)) != 0u) {
            result.box_full = true;
            return result;
        }
        deposited.push_back(slot.item);
        slot = {};
    }
    for (std::uint32_t i = 0; i < kPouchSlots; ++i) {
        const Slot &wanted = set.items[i];
        if (wanted.item == 0u || wanted.count == 0u || wanted.item >= kFlagIds) continue;
        const std::uint16_t carry = carry_limit(ram, wanted.item);
        const std::uint32_t taken = withdraw(box, wanted.item, std::min<std::uint32_t>(wanted.count, carry));
        if (taken > 0u) pouch[i] = {wanted.item, static_cast<std::uint16_t>(taken)};
        if (taken < wanted.count) result.short_items.push_back({wanted.item, wanted.count, clamp16(taken)});
    }
    for (std::uint32_t i = 0; i < box.size(); ++i) {
        ram.store16(sys + kBox + i * 4u, box[i].item);
        ram.store16(sys + kBox + i * 4u + 2u, box[i].count);
    }
    for (std::uint32_t pouch_at : {sys + kPouch, sys + kBoxPouch})
        for (std::uint32_t i = 0; i < kPouchSlots; ++i) {
            ram.store16(pouch_at + i * 4u, pouch[i].item);
            ram.store16(pouch_at + i * 4u + 2u, pouch[i].count);
        }
    for (std::uint16_t item : deposited) mark_obtained(ram, sys, item);
    result.done = true;
    return result;
}

std::string default_name(std::size_t index) {
    return "Item Set " + std::to_string(index + 1u);
}

// The file: a line `hunter NAME` starts a hunter's sets, then one line per
// set, `set N NAME`, and its slots, `items ID:COUNT ...`, 24 of them.
void Store::load_file() {
    loaded_ = true;
    hunters_.clear();
    std::ifstream in(path_);
    std::string line;
    Sets *sets = nullptr;
    ItemSet *current = nullptr;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.rfind("hunter ", 0) == 0) {
            hunters_.emplace_back(line.substr(7), Sets{});
            sets = &hunters_.back().second;
            for (std::size_t i = 0; i < kSets; ++i) (*sets)[i].name = default_name(i);
            current = nullptr;
        } else if (sets != nullptr && line.rfind("set ", 0) == 0) {
            std::istringstream fields(line.substr(4));
            std::size_t number = 0u;
            if (!(fields >> number) || number < 1u || number > kSets) {
                current = nullptr;
                continue;
            }
            current = &(*sets)[number - 1u];
            std::string name;
            std::getline(fields >> std::ws, name);
            current->name = name.empty() ? default_name(number - 1u) : name;
        } else if (current != nullptr && line.rfind("items", 0) == 0) {
            std::istringstream fields(line.substr(5));
            std::string pair;
            for (std::uint32_t i = 0; i < kPouchSlots && fields >> pair; ++i) {
                const std::size_t colon = pair.find(':');
                if (colon == std::string::npos) continue;
                const unsigned long item = std::strtoul(pair.substr(0, colon).c_str(), nullptr, 10);
                const unsigned long count = std::strtoul(pair.substr(colon + 1u).c_str(), nullptr, 10);
                if (item >= kFlagIds || count > 0xFFFFu) continue;
                current->items[i] = {static_cast<std::uint16_t>(item), static_cast<std::uint16_t>(count)};
            }
        }
    }
}

Sets Store::sets(const std::string &hunter) {
    if (!loaded_) load_file();
    for (const auto &[name, sets] : hunters_)
        if (name == hunter) return sets;
    Sets fresh;
    for (std::size_t i = 0; i < kSets; ++i) fresh[i].name = default_name(i);
    return fresh;
}

void Store::set(const std::string &hunter, std::size_t index, const ItemSet &set) {
    if (index >= kSets) return;
    if (!loaded_) load_file();
    auto found = std::find_if(hunters_.begin(), hunters_.end(), [&](const auto &h) { return h.first == hunter; });
    if (found == hunters_.end()) {
        hunters_.emplace_back(hunter, sets(hunter));
        found = std::prev(hunters_.end());
    }
    found->second[index] = set;
    // A name is one line of the file.
    std::string &name = found->second[index].name;
    std::replace(name.begin(), name.end(), '\n', ' ');
    std::replace(name.begin(), name.end(), '\r', ' ');
    if (name.empty()) name = default_name(index);
}

bool Store::save() {
    if (!loaded_) load_file();
    std::error_code ec;
    if (path_.has_parent_path()) std::filesystem::create_directories(path_.parent_path(), ec);
    const std::filesystem::path temporary = path_.string() + ".tmp";
    {
        std::ofstream out(temporary, std::ios::trunc);
        if (!out) return false;
        out << "# Item sets, written by Pokee: one block per hunter, by name.\n";
        for (const auto &[hunter, sets] : hunters_) {
            out << "hunter " << hunter << "\n";
            for (std::size_t i = 0; i < kSets; ++i) {
                out << "set " << (i + 1u) << " " << sets[i].name << "\nitems";
                for (const Slot &slot : sets[i].items) out << " " << slot.item << ":" << slot.count;
                out << "\n";
            }
        }
        if (!out.flush()) return false;
    }
    std::filesystem::rename(temporary, path_, ec);
    if (ec) {
        std::filesystem::remove(path_, ec);
        std::filesystem::rename(temporary, path_, ec);
    }
    return !ec;
}

// Hooks ----------------------------------------------------------------------

namespace {

struct State {
    psprecomp::Runtime::RecompiledFunction text{};
    psprecomp::Runtime::RecompiledFunction cursor{};
    psprecomp::Runtime::RecompiledFunction box_frame{};
    psprecomp::Runtime::RecompiledFunction window{};
    bool reported{};
    bool open{};
    bool first{}; // the frame Item Sets was chosen: its button is still down
    menu::State screen;
};

State &state() {
    static State s;
    return s;
}

bool enabled() {
    return settings::current().item_sets;
}

bool lobby_ready(const Ram &ram) {
    return layout::overlay_loaded(ram, layout::task_slot(), layout::kVillageOverlay) && matches(ram, kOverlaySignature);
}

List read_list(const Ram &ram, std::uint32_t at) {
    List list{};
    for (std::uint32_t i = 0; i < list.size(); ++i) list[i] = ram.load16(at + i * 2u);
    return list;
}

void write_list(Ram &ram, std::uint32_t at, const List &list) {
    for (std::uint32_t i = 0; i < list.size(); ++i) ram.store16(at + i * 2u, list[i]);
}

// Change Equipment in the Guild Hall (game/hall_equipment.hpp) gives the
// Small Item Box's Change Equipment the house's text, 0x129, in place of the
// greyed 303. The list is still the game's, and keeps that text when Item
// Sets' row is put in or taken out.
constexpr std::uint16_t kGreyChangeEquipment = 303u;
constexpr std::uint16_t kOfferedChangeEquipment = 0x129u;

List as_game(List list) {
    std::replace(list.begin(), list.end(), kOfferedChangeEquipment, kGreyChangeEquipment);
    return list;
}

// The menu's data as the game left it, or as the hooks did.
bool menu_data_known(const Ram &ram) {
    const std::uint32_t rows = ram.load32(ported(kMenuRecord) + 8u);
    const std::uint32_t place = ram.load32(ported(kMenuRecord));
    const auto list_is = [&](std::uint32_t at, const List &a, const List &b) {
        const List now = at == ported(kSmallBoxList) ? as_game(read_list(ram, at)) : read_list(ram, at);
        return now == a || now == b;
    };
    return (place == kRecordPlace || place == kPatchedPlace) && ram.load32(ported(kMenuRecord) + 12u) == ported(kBoxList) &&
        (row_count(rows) == kGameRows || row_count(rows) == kRows) && list_is(ported(kBoxList), kBoxRows, kPatchedBoxRows) &&
        list_is(ported(kSmallBoxList), kSmallBoxRows, kPatchedSmallBoxRows);
}

void set_menu(Ram &ram, bool seven) {
    ram.store32(ported(kMenuRecord), seven ? kPatchedPlace : kRecordPlace);
    ram.store32(ported(kMenuRecord) + 8u, with_rows(ram.load32(ported(kMenuRecord) + 8u), seven ? kRows : kGameRows));
    write_list(ram, ported(kBoxList), seven ? kPatchedBoxRows : kBoxRows);
    const List hall = read_list(ram, ported(kSmallBoxList));
    List small = seven ? kPatchedSmallBoxRows : kSmallBoxRows;
    if (std::find(hall.begin(), hall.end(), kOfferedChangeEquipment) != hall.end())
        std::replace(small.begin(), small.end(), kGreyChangeEquipment, kOfferedChangeEquipment);
    write_list(ram, ported(kSmallBoxList), small);
}

// Text the game prints, cut to fit and ended by a 0.
void write_text(Ram &ram, std::uint32_t at, std::uint32_t size, const std::string &text) {
    const std::size_t length = std::min<std::size_t>(text.size(), size - 1u);
    for (std::size_t i = 0; i < length; ++i)
        ram.store8(at + static_cast<std::uint32_t>(i), static_cast<std::uint8_t>(text[i]));
    ram.store8(at + static_cast<std::uint32_t>(length), 0u);
}

std::uint32_t row_text(std::uint16_t id) {
    return kRowText + static_cast<std::uint32_t>(id - kChooseId) * kRowTextSize;
}

// A window template, as 0x0882FA24 reads it.
void write_template(Ram &ram, std::uint32_t at, std::uint16_t x, std::uint16_t y, std::uint8_t chars,
    std::uint16_t rows, std::uint32_t ids) {
    ram.store16(at, x);
    ram.store16(at + 2u, y);
    ram.store8(at + 4u, chars);
    ram.store8(at + 5u, 14u); // line height 16, as the box menu's
    ram.store8(at + 6u, 0u);
    ram.store8(at + 7u, 3u); // the box menu's frame
    ram.store16(at + 8u, 20u);
    ram.store16(at + 10u, rows);
    ram.store32(at + 12u, ids);
    ram.store32(at + 16u, kWindowColour);
}

void write_ids(Ram &ram, std::uint32_t at, std::uint16_t first, std::uint16_t count) {
    for (std::uint16_t i = 0; i < count; ++i) ram.store16(at + i * 2u, static_cast<std::uint16_t>(first + i));
    ram.store16(at + count * 2u, 0u);
}

// Everything the screens show that lives in guest memory, for this frame.
void write_screen(Ram &ram, const menu::State &screen, const Sets &sets) {
    write_text(ram, kLabel, 16u, kLabelText);
    write_template(ram, kChooseTemplate, kChooseX, kChooseY, 14u, menu::kChooseRows, kChooseIds);
    write_template(ram, kListTemplate, kListX, kListY, kListChars, static_cast<std::uint16_t>(kSetsPerPage), kListIds);
    write_ids(ram, kChooseIds, kChooseId, menu::kChooseRows);
    write_ids(ram, kListIds, kSetId, static_cast<std::uint16_t>(kSetsPerPage));
    for (std::uint8_t row = 0; row < menu::kChooseRows; ++row)
        write_text(ram, row_text(static_cast<std::uint16_t>(kChooseId + row)), kRowTextSize, menu::choose_row(row));
    const std::size_t first = menu::set_page(screen) * kSetsPerPage;
    for (std::uint8_t row = 0; row < kSetsPerPage; ++row)
        write_text(ram, row_text(static_cast<std::uint16_t>(kSetId + row)), kRowTextSize,
            menu::set_row(static_cast<std::uint8_t>(first + row)));
    write_text(ram, kTitleText, kRowTextSize, "");
    write_text(ram, kListTitle, kRowTextSize, menu::list_title(screen.mode));
    write_text(ram, kMessageText, kMessageTextSize, menu::message(screen));
    write_text(ram, kHintText, kHintTextSize, menu::hint(screen));
    write_text(ram, kYesNoText, kRowTextSize, screen.yes ? "~C02[Yes]~C00 / No " : "~C00 Yes / ~C02[No]");
    const ItemSet &set = sets[screen.set];
    for (std::uint32_t i = 0; i < kPouchSlots; ++i) {
        const Slot slot = set.items[i].count == 0u ? Slot{} : set.items[i];
        ram.store16(kSetSlots + i * 4u, slot.item);
        ram.store16(kSetSlots + i * 4u + 2u, slot.count);
    }
}

guest_calls::Call call(std::uint32_t function, std::initializer_list<std::uint32_t> args, std::uint32_t stack = 0u) {
    guest_calls::Call c;
    c.function = ported(function);
    std::size_t i = 0;
    for (std::uint32_t arg : args) c.args[i++] = arg;
    c.stack = stack;
    return c;
}

std::uint16_t buttons(const Ram &ram) {
    const std::uint32_t owner = ram.load32(ported(kInputOwner));
    if (layout::in_ram(owner) && ram.load8(owner + kInputBlocked) != 0u) return 0u;
    const auto pressed =
        static_cast<std::uint16_t>((ram.load32(ported(kHeldDpad)) & 0xF0u) | ram.load32(ported(kButtons)));
    return layout::as_traced_box_buttons(ram, pressed);
}

std::vector<guest_calls::Call> draw_calls(const Ram &ram, std::uint32_t box, const menu::State &screen,
    menu::Sound sound) {
    const std::uint32_t ui = ram.load32(ported(kUiPointer));
    const std::uint32_t font = ram.load32(ported(kFontPointer));
    std::vector<guest_calls::Call> calls;
    // The box as its update leaves it, then its menu dimmed, as Put In Item has it.
    calls.push_back(call(kUpdate, {box}));
    calls.push_back(call(kBoxMenu, {box, 1u}));
    if (screen.page == menu::Page::Choose) {
        calls.push_back(call(kWindowBuild, {kWindowRecord, kChooseTemplate, 0u, 0u, 0u}));
        calls.push_back(call(kWindowDraw, {ui, kWindowRecord, 0u, screen.row, 255u, 1u}));
    } else {
        calls.push_back(call(kWindowBuild, {kWindowRecord, kListTemplate, 1u, 0u, 0u}));
        calls.push_back(
            call(kWindowDraw, {ui, kWindowRecord, kTitleText, menu::set_row_on_page(screen), 255u, 1u}));
        calls.push_back(call(kPager, {ui, kWindowRecord, menu::set_page(screen), kSetPages, 0u, 0u}));
        calls.push_back(call(kFontSize, {font, 14u, 14u}));
        calls.push_back(call(kFontColour, {font, kTitleColour}));
        calls.push_back(call(kPrint, {font, title_x(menu::list_title(screen.mode)), kListY + 7u, kListTitle}));
        calls.push_back(call(kItemWindow, {ui, kSetSlots, screen.contents * 8u, 0u, 0x10u, 0u}));
        calls.push_back(call(kMessageDraw, {ui, ported(kMessageRecord), kMessageText, 255u}));
        if (screen.page == menu::Page::Confirm) {
            const std::uint32_t game = text_entry_address(ram, install::port(kTextBlock), 0, screen.yes ? kYesText : kNoText);
            calls.push_back(call(kPrint, {font, kYesNoX, kYesNoY, ported(kPrintFormat), game != 0u ? game : kYesNoText}));
        } else {
            calls.push_back(call(kPrint, {font, kHintX, kHintY, kHintText}));
        }
    }
    if (sound != menu::Sound::None)
        calls.push_back(call(kSound, {ram.load32(ported(kSoundPointer)), menu::sound_bank(sound), menu::sound_id(sound)}));
    return calls;
}

std::string short_text(const Loaded &loaded, const Ram &ram) {
    std::string text = "Loaded. The box was short of\n";
    for (std::size_t i = 0; i < loaded.short_items.size() && i < 2u; ++i) {
        const Short &item = loaded.short_items[i];
        if (i > 0u) text += ", ";
        text += text_entry(ram, install::port(kTextBlock), 1, item.item) + " " + std::to_string(item.taken) + "/" +
            std::to_string(item.wanted);
    }
    if (loaded.short_items.size() > 2u) text += "...";
    return text;
}

// Does what the screen asked for, and says how it went.
// Does what the screen asked for, says how it went, and gives the sound for
// it: items moving for a load, the equipment set's for a registration.
menu::Sound act(Ram &ram, std::uint32_t sys, const menu::Step &step, const std::string &hunter, const Sets &sets) {
    menu::State &screen = state().screen;
    const std::string name = menu::set_row(static_cast<std::uint8_t>(step.set));
    switch (step.act) {
    case menu::Act::Load: {
        const Loaded loaded = load(ram, sys, sets[step.set]);
        if (trace_enabled())
            std::cout << "[item sets] load " << name << (loaded.box_full ? ": box full" : "") << ", "
                      << loaded.short_items.size() << " short" << std::endl;
        if (loaded.box_full) {
            menu::finished(screen, "~C02The Item Box is full.", true);
            return menu::Sound::Refuse;
        }
        if (!loaded.short_items.empty())
            menu::finished(screen, short_text(loaded, ram), true);
        else
            menu::finished(screen, name + " loaded.", false);
        return menu::Sound::Moved;
    }
    case menu::Act::Register: {
        store().set(hunter, step.set, ItemSet{sets[step.set].name, read_pouch(ram, sys)});
        if (trace_enabled()) std::cout << "[item sets] register " << name << std::endl;
        if (!store().save()) {
            menu::finished(screen, "~C02The set could not be saved.", true);
            return menu::Sound::Refuse;
        }
        menu::finished(screen, "The pouch is registered\nas " + name + ".", false);
        return menu::Sound::Registered;
    }
    case menu::Act::Delete: {
        store().set(hunter, step.set, ItemSet{default_name(step.set), {}});
        if (trace_enabled()) std::cout << "[item sets] delete " << name << std::endl;
        if (!store().save()) {
            menu::finished(screen, "~C02The set could not be saved.", true);
            return menu::Sound::Refuse;
        }
        menu::finished(screen, name + " is deleted.", false);
        return menu::Sound::Confirm;
    }
    case menu::Act::None:
    case menu::Act::Close: break;
    }
    return step.sound;
}

void text(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    const std::uint16_t id = static_cast<std::uint16_t>(ctx.gpr[5] & 0xFFFFu);
    if (id == kTextId || (id >= kChooseId && id <= kLastId)) {
        GuestRam ram(runtime.memory());
        if (id == kTextId) write_text(ram, kLabel, 16u, kLabelText);
        ctx.set_gpr(2, id == kTextId ? kLabel : row_text(id));
        ctx.pc = ctx.gpr[31];
        return;
    }
    ctx.pc = ported(kText);
    s.text(runtime, ctx);
}

bool seven_rows(psprecomp::Runtime &runtime) {
    return enabled() && row_count(runtime.memory().load32(ported(kMenuRecord) + 8u)) == kRows;
}

// The box's cursor, over the rows as they are shown: 0x0881C2E4's up and
// down with its wrap and sound, the row kept as the game numbers it.
void cursor(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t row_at = ctx.gpr[5];
    if (install::traced_code(ctx.gpr[31], install::Image::lobby_task) != kCursorReturn || ctx.gpr[7] != kGameRows ||
        !seven_rows(runtime) ||
        !memory.contains(row_at, 1u)) {
        ctx.pc = ported(kCursor);
        s.cursor(runtime, ctx);
        return;
    }
    const std::uint8_t row = memory.load8(row_at);
    const std::uint8_t moved = move_shown(row, static_cast<std::uint16_t>(ctx.gpr[6]));
    if (moved == row) {
        ctx.set_gpr(2, 0u);
        ctx.pc = ctx.gpr[31];
        return;
    }
    memory.store8(row_at, moved);
    GuestRam ram(memory);
    guest_calls::run(runtime, ctx,
        {call(kSound, {ram.load32(ported(kSoundPointer)), menu::sound_bank(menu::Sound::Cursor), menu::sound_id(menu::Sound::Cursor)})}, 1u);
}

// The box menu's highlight on the row as it is shown.
void window(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    if (install::traced_code(ctx.gpr[31], install::Image::lobby_task) == kMenuDrawReturn && seven_rows(runtime) &&
        ctx.gpr[7] < kRows)
        ctx.set_gpr(7, shown_row(static_cast<std::uint8_t>(ctx.gpr[7])));
    ctx.pc = ported(kWindowDraw);
    s.window(runtime, ctx);
}

bool on_item_sets(const psprecomp::GuestMemory &memory, std::uint32_t box) {
    return memory.contains(box, kPhase + 1u) && memory.load8(box + kPhase) == 1u &&
        memory.load8(box + kChosenRow) == kRow;
}

// The box's menu, its rows shown, choosing Item Sets this frame: the
// buttons the menu reads (0x09ADCCD0: none while it is hidden) with choose
// and nothing the menu would act on first.
bool chooses_item_sets(psprecomp::Runtime &runtime, const Ram &ram, std::uint32_t box) {
    if (!seven_rows(runtime) || !ram.contains(box, kMenuShown + 1u) || ram.load8(box + kPhase) != 0u ||
        ram.load8(box + kChosenRow) != kRow || ram.load8(box + kMenuShown) == 0u)
        return false;
    const std::uint16_t pressed = buttons(ram);
    return (pressed & menu::kConfirm) != 0u &&
        (pressed & (menu::kCancel | menu::kUp | menu::kDown | kStart)) == 0u;
}

// The box's frame on Item Sets: the screens take its buttons and draw after
// its update. Its own frame would take back to its menu at once (0x09ADD008).
void box_frame(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t box = ctx.gpr[4];
    GuestRam ram(memory);
    // Choosing Item Sets: started here, so the box's own frame never shows
    // a frame of Put In Item's message without its menu.
    bool waiting = on_item_sets(memory, box);
    menu::Sound opening = menu::Sound::None;
    if (!waiting && chooses_item_sets(runtime, ram, box)) {
        memory.store8(box + kPhase, 1u);
        waiting = true;
        opening = menu::Sound::Confirm;
    }
    const std::uint32_t sys = waiting ? game_sys(ram) : 0u;
    if (!waiting || sys == 0u) {
        s.open = false;
        if (waiting) memory.store8(box + kPhase, 0u);
        ctx.pc = ported(kBoxFrame);
        s.box_frame(runtime, ctx);
        return;
    }
    if (!s.open) {
        s.open = true;
        s.first = true;
        s.screen = {};
        if (trace_enabled()) std::cout << "[item sets] chosen at box " << std::hex << box << std::dec << std::endl;
    }
    // The update shows the message of the box's own rows while this is set.
    const std::uint32_t ui = ram.load32(ported(kUiPointer));
    if (layout::in_ram(ui)) ram.store32(ui + kUiFlags, ram.load32(ui + kUiFlags) & ~kUiMessage);
    const std::string hunter = hunter_name(ram, sys);
    Sets sets = store().sets(hunter);
    menu::Step step{menu::Act::None, 0u, opening};
    if (!std::exchange(s.first, false))
        step = menu::step(s.screen, buttons(ram), sets, ItemSet{{}, read_pouch(ram, sys)}.empty());
    if (step.act == menu::Act::Close) {
        // Back to the box's menu, on Item Sets, as the box's own back does.
        s.open = false;
        memory.store8(box + kPhase, 0u);
        memory.store32(box + kBoxFlags, memory.load32(box + kBoxFlags) & ~7u);
        if (trace_enabled()) std::cout << "[item sets] back to the box menu" << std::endl;
        guest_calls::run(runtime, ctx,
            {call(kUpdate, {box}), call(kSound, {ram.load32(ported(kSoundPointer)), menu::sound_bank(step.sound), menu::sound_id(step.sound)})},
            1u);
        return;
    }
    if (step.act != menu::Act::None) {
        step.sound = act(ram, sys, step, hunter, sets);
        sets = store().sets(hunter);
    }
    write_screen(ram, s.screen, sets);
    guest_calls::run(runtime, ctx, draw_calls(ram, box, s.screen, step.sound), 1u);
}

} // namespace

void frame(psprecomp::Runtime &runtime) {
    State &s = state();
    GuestRam ram(runtime.memory());
    if (runtime.function_at(ported(kCursor)) != &cursor && runtime.function_at(ported(kCursor)) != nullptr &&
        matches(ram, code_signature())) {
        s.cursor = runtime.function_at(ported(kCursor));
        runtime.register_function(ported(kCursor), &cursor, "mhfu_item_sets_cursor");
    }
    if (runtime.function_at(ported(kWindowDraw)) != &window && runtime.function_at(ported(kWindowDraw)) != nullptr &&
        matches(ram, code_signature())) {
        s.window = runtime.function_at(ported(kWindowDraw));
        runtime.register_function(ported(kWindowDraw), &window, "mhfu_item_sets_window");
    }
    if (s.cursor == nullptr || !lobby_ready(ram) || !menu_data_known(ram)) {
        // The box went with lobby_task.
        if (!layout::overlay_loaded(ram, layout::task_slot(), layout::kVillageOverlay)) s.open = false;
        return;
    }
    guest_calls::install(runtime);
    set_menu(ram, enabled());
    // Change Equipment in the Guild Hall wraps the box's frame too, and Eat in
    // the Guild Hall the text call (game/hooks.hpp).
    (void)hooks::wrap(runtime, ported(kText), &text, s.text, "mhfu_item_sets_text");
    (void)hooks::wrap(runtime, ported(kBoxFrame), &box_frame, s.box_frame, "mhfu_item_sets_box");
    if (!s.reported) {
        s.reported = true;
        std::cout << "[item sets] box menu entry in place" << std::endl;
    }
}

bool screen_open() {
    return state().open;
}

Store &store() {
    static Store sets(install::user_data_directory() / "item_sets.txt");
    return sets;
}

} // namespace mhfu::game::item_sets
