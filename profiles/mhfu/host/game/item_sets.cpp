#include "game/item_sets.hpp"

#include "game/guest_layout.hpp"
#include "game/guest_ram.hpp"
#include "install/user_data.hpp"
#include "psprecomp/runtime.hpp"
#include "settings/settings.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

namespace mhfu::game::item_sets {
namespace {

constexpr std::array<CodeWord, 3> kCodeSignature = {{
    {kCursor, 0x27BDFFE0u},     // addiu sp, sp, -32
    {0x0881C2ECu, 0x90A40000u}, // lbu a0, 0(a1): the row
    {0x0882F880u, 0x0E69EE24u}, // jal 0x09A7B890: a row's text
}};

// lobby_task's code. The menu's data words are checked apart, since the
// hooks change them.
constexpr std::array<CodeWord, 14> kOverlaySignature = {{
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
    {0x09AE39F0u, 0x92040038u}, // lbu a0, 56(s0): the phase
    {0x09AE3A20u, 0x2C610006u}, // sltiu at, v1, 6: rows 0 to 5 run
    {0x09AE6720u, 0x24A5B0A8u}, // addiu a1, a1, -0x4F58: the menu record
    {0x09AE675Cu, 0x24425168u}, // addiu v0, v0, 0x5168: the Small Item Box's rows
}};

// The menu record's row count and list, and the two lists, as the game has
// them and as the hooks leave them.
constexpr std::uint32_t kRecordRows = 0x00060014u; // u16 14 (row height), u16 6
constexpr std::uint32_t kPatchedRows = 0x00070014u;
constexpr std::uint32_t kBoxTail = 0x09B05164u;      // 298, 299 at +8; padding at +12
constexpr std::uint32_t kSmallBoxTail = 0x09B05174u; // the same after 304, 305
constexpr std::uint32_t kPatchedTail = kTextId;      // kTextId, then the 0 that ends the list
// The record's place, u16 x then u16 y. Seven rows reach the hints under the
// window (Rotate, Menu OFF), so the window is raised by 12.
constexpr std::uint32_t kRecordPlace = 0x00180010u;
constexpr std::uint32_t kPatchedPlace = 0x000C0010u;

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
        if (!ram.contains(expected.address, 4u) || ram.load32(expected.address) != expected.word) return false;
    return true;
}

// Box and pouch --------------------------------------------------------------

std::uint32_t game_sys(const Ram &ram) {
    if (!ram.contains(kGameSysPointer, 4u)) return 0u;
    const std::uint32_t sys = ram.load32(kGameSysPointer);
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
    const std::uint32_t at = kItemTable + static_cast<std::uint32_t>(item) * kItemRecord + kCarryByte;
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
    for (std::uint32_t i = 0; i < kPouchSlots; ++i) {
        ram.store16(sys + kPouch + i * 4u, pouch[i].item);
        ram.store16(sys + kPouch + i * 4u + 2u, pouch[i].count);
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
    psprecomp::Runtime::RecompiledFunction update{};
    bool reported{};
    bool request{};
    bool open{};
    bool closed{};
    std::uint32_t box{}; // the box record while the screen is open
};

State &state() {
    static State s;
    return s;
}

bool enabled() {
    return settings::current().item_sets;
}

bool lobby_ready(const Ram &ram) {
    return layout::overlay_loaded(ram, layout::kTaskSlot, layout::kVillageOverlay) && matches(ram, kOverlaySignature);
}

// The menu's data as the game left it, or as the hooks did.
bool menu_data_known(const Ram &ram) {
    const std::uint32_t rows = ram.load32(kMenuRecord + 8u);
    const std::uint32_t place = ram.load32(kMenuRecord);
    return (place == kRecordPlace || place == kPatchedPlace) && ram.load32(kMenuRecord + 12u) == kBoxList &&
        (rows == kRecordRows || rows == kPatchedRows) && ram.load32(kBoxList) == 0x01270126u &&
        ram.load32(kSmallBoxList) == 0x012D012Cu &&
        (ram.load32(kBoxTail) == 0u || ram.load32(kBoxTail) == kPatchedTail) &&
        (ram.load32(kSmallBoxTail) == 0u || ram.load32(kSmallBoxTail) == kPatchedTail);
}

void set_menu(Ram &ram, bool seven) {
    ram.store32(kMenuRecord, seven ? kPatchedPlace : kRecordPlace);
    ram.store32(kMenuRecord + 8u, seven ? kPatchedRows : kRecordRows);
    ram.store32(kBoxTail, seven ? kPatchedTail : 0u);
    ram.store32(kSmallBoxTail, seven ? kPatchedTail : 0u);
}

void write_label(Ram &ram) {
    for (std::size_t i = 0; i < sizeof kLabelText; ++i)
        ram.store8(kLabel + static_cast<std::uint32_t>(i), static_cast<std::uint8_t>(kLabelText[i]));
}

void text(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    if ((ctx.gpr[5] & 0xFFFFu) == kTextId) {
        GuestRam ram(runtime.memory());
        write_label(ram);
        ctx.set_gpr(2, kLabel);
        ctx.pc = ctx.gpr[31];
        return;
    }
    ctx.pc = kText;
    s.text(runtime, ctx);
}

void cursor(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    if (ctx.gpr[31] == kCursorReturn && ctx.gpr[7] == kGameRows && enabled() &&
        runtime.memory().load32(kMenuRecord + 8u) == kPatchedRows)
        ctx.set_gpr(7, kRows);
    ctx.pc = kCursor;
    s.cursor(runtime, ctx);
}

void update(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t box = ctx.gpr[4];
    if (memory.contains(box, kPhase + 1u) && memory.load8(box + kPhase) == 1u && memory.load8(box + kChosenRow) == kRow) {
        if (s.closed) {
            // Back to the menu, on Item Sets.
            s.closed = false;
            s.open = false;
            memory.store8(box + kPhase, 0u);
            if (trace_enabled()) std::cout << "[item sets] back to the box menu" << std::endl;
        } else {
            if (!s.open) {
                s.open = true;
                s.request = true;
                s.box = box;
                if (trace_enabled())
                    std::cout << "[item sets] chosen at box " << std::hex << box << std::dec << std::endl;
            }
            ctx.set_gpr(2, 0u);
            ctx.pc = ctx.gpr[31];
            return;
        }
    }
    ctx.pc = kUpdate;
    s.update(runtime, ctx);
}

} // namespace

void frame(psprecomp::Runtime &runtime) {
    State &s = state();
    GuestRam ram(runtime.memory());
    if (runtime.function_at(kCursor) != &cursor && runtime.function_at(kCursor) != nullptr &&
        matches(ram, code_signature())) {
        s.cursor = runtime.function_at(kCursor);
        runtime.register_function(kCursor, &cursor, "mhfu_item_sets_cursor");
    }
    if (s.cursor == nullptr || !lobby_ready(ram) || !menu_data_known(ram)) {
        if (!layout::overlay_loaded(ram, layout::kTaskSlot, layout::kVillageOverlay)) {
            // The box went with lobby_task; a screen still open stays until it closes.
            if (!s.open) s.closed = false;
        }
        return;
    }
    set_menu(ram, enabled());
    if (runtime.function_at(kText) != &text && runtime.function_at(kText) != nullptr) {
        s.text = runtime.function_at(kText);
        runtime.register_function(kText, &text, "mhfu_item_sets_text");
    }
    if (runtime.function_at(kUpdate) != &update && runtime.function_at(kUpdate) != nullptr) {
        s.update = runtime.function_at(kUpdate);
        runtime.register_function(kUpdate, &update, "mhfu_item_sets_update");
    }
    if (!s.reported) {
        s.reported = true;
        std::cout << "[item sets] box menu entry in place" << std::endl;
    }
}

bool take_screen_request() {
    return std::exchange(state().request, false);
}

void screen_closed() {
    State &s = state();
    if (s.open) s.closed = true;
}

bool screen_open() {
    return state().open;
}

Store &store() {
    static Store sets(install::user_data_directory() / "item_sets.txt");
    return sets;
}

} // namespace mhfu::game::item_sets
