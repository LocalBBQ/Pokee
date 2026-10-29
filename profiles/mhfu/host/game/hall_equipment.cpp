#include "game/hall_equipment.hpp"

#include "game/guest_layout.hpp"
#include "game/hooks.hpp"
#include "install/release_addresses.hpp"
#include "psprecomp/runtime.hpp"
#include "settings/settings.hpp"

#include <array>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

namespace mhfu::game::hall_equipment {
namespace {

// The addresses are the executable's and lobby_task's, named as ULUS-10391
// has them.
std::uint32_t ported(std::uint32_t traced) {
    return install::port_from(traced, install::Image::lobby_task);
}

constexpr std::array<CodeWord, 70> kSignature = {{
    {kSlotReset, 0x27BDFFF0u},  // addiu sp, sp, -16
    {0x088DB560u, 0x0E219C02u}, // jal 0x08867008: a part's row reset
    {0x09A7D338u, 0x0E236D4Du}, // jal 0x088DB534: a member who left
    {0x0889C518u, 0x3C0209A3u}, // lui v0, 0x09A3: this console's address
    {0x0889C520u, 0x244289BCu}, // addiu v0, v0, -0x7644
    {0x09A7CAE8u, 0x90C50000u}, // lbu a1, 0(a2): a member's hunter into a request
    {0x09A7CB3Cu, 0xC4C0001Cu}, // lwc1 f0, 0x1C(a2)
    {0x09A7CB7Cu, 0x24CA002Cu}, // addiu t2, a2, 0x2C: the armor
    {0x09A7D764u, 0x92030008u}, // lbu v1, 8(s0): the request into the record
    {0x09A7D768u, 0x24820040u}, // addiu v0, a0, 0x40
    {0x09A7D81Cu, 0x92030070u}, // lbu v1, 0x70(s0)
    {0x088989ACu, 0x922212D5u}, // lbu v0, 0x12D5(s1): the status 0x08898980 sends
    {0x088989CCu, 0x8E2212D0u}, // lw v0, 0x12D0(s1): the counter
    {0x088989E0u, 0x0E2273CDu}, // jal 0x0889CF34: the status into the entry
    {0x088989E8u, 0x0E2275BBu}, // jal 0x0889D6EC: the entry refreshed
    {0x088994CCu, 0x908412D5u}, // lbu a0, 0x12D5(a0): 0x088994AC
    {0x088995B4u, 0x90A300BFu}, // lbu v1, 0xBF(a1): the member's status
    {kLoaderFrame, 0x27BDFFF0u},      // addiu sp, sp, -16
    {0x09A7C9D4u, 0x8C830000u},       // lw v1, 0(a0): the head's state
    {0x09A7CA6Cu, 0x0E69F680u},       // jal 0x09A7DA00: state 3
    {0x09A7CA7Cu, 0x0E69F8BEu},       // jal 0x09A7E2F8: state 4
    {0x09A7CA8Cu, 0x0E69F966u},       // jal 0x09A7E598: state 5
    {kRequestOwnReload, 0x00003021u}, // addu a2, zr, zr
    {0x09A7D3B0u, 0x24050003u},       // addiu a1, zr, 3
    {0x09A7D3BCu, 0xAC650000u},       // sw a1, 0(v1)
    {0x09AA16D0u, 0x0E69F4E4u},       // jal 0x09A7D390: the commit's request
    {0x09A7DF64u, 0x24050001u},       // addiu a1, zr, 1: slot 1
    {0x09A7DF68u, 0x0E236D32u},       // jal 0x088DB4C8: a piece's file
    {0x09A7E19Cu, 0x0E236D4Du},       // jal 0x088DB534: slot 1's parts reset
    {0x09A7E1A0u, 0x24050001u},       // addiu a1, zr, 1
    {0x09A7E600u, 0x904203E6u},       // lbu v0, 0x3E6(v0): slot 1's part 6
    {0x09A7E614u, 0x94520424u},       // lhu s2, 0x424(v0)
    {0x08867414u, 0x342100A8u},       // ori at, at, 0xA8: the kind rows
    {0x08867424u, 0x342103D8u},       // ori at, at, 0x3D8: the state rows
    {0x088674CCu, 0xAC620258u},       // sw v0, 0x258(v1): the address rows
    {0x088674FCu, 0xAC530318u},       // sw s3, 0x318(v0): the size rows
    {0x0886758Cu, 0xA4530408u},       // sh s3, 0x408(v0): the file rows
    {0x088DB5A4u, 0xAC600024u},       // sw zr, 0x24(v1): the slot's progress
    {kRefreshOwnEntry, 0x27BDFFF0u}, // addiu sp, sp, -16
    {0x0889D714u, 0x8C63A57Cu},      // lw v1, -0x5A84(v1): only in ad hoc play
    {0x0889D738u, 0x0E227146u},      // jal 0x0889C518: this console's address
    {0x0889D7A8u, 0x0E22C323u},      // jal 0x088B0C8C: the hunter into the entry
    {0x0889D7ACu, 0x2444059Cu},      // addiu a0, v0, 0x59C: entry + 0x20
    {0x0889D7C8u, 0x2403003Cu},      // addiu v1, zr, 60
    {0x0889D7CCu, 0xAE4312D0u},      // sw v1, 0x12D0(s2): send it
    {0x09AAFB30u, 0x3C0209A1u},      // lui v0, 0x09A1: the hall's own refresh
    {0x09AAFB34u, 0x0E2275BBu},      // jal 0x0889D6EC
    {0x09AAFB38u, 0x8C44B684u},      // lw a0, -0x497C(v0): the session
    {0x09ADD040u, 0xA2400038u},      // sb zr, 0x38(s2): back to the top menu
    {kBoxUpdate, 0x27BDFFD0u},  // addiu sp, sp, -48
    {0x09ADCCECu, 0x0E213EDBu}, // jal 0x0884FB6C: the buttons
    {0x09ADCCF8u, 0x92420034u}, // lbu v0, 0x34(s2): Start is dropped in the hall
    {0x09ADCDC0u, 0x92430038u}, // lbu v1, 0x38(s2): the top menu
    {0x09ADCE14u, 0x32024000u}, // andi v0, s0, 0x4000: the choice
    {0x09ADCE20u, 0x92420034u}, // lbu v0, 0x34(s2)
    {0x09ADCE2Cu, 0x92430036u}, // lbu v1, 0x36(s2): the cursor
    {0x09ADCE30u, 0x24020003u}, // addiu v0, zr, 3: Change Equipment
    {0x09ADD0E8u, 0x0E6B8E3Eu}, // jal 0x09AE38F8: the menu is drawn
    {0x09ADCADCu, 0xA2300034u}, // sb s0, 0x34(s1): the box is opened
    {0x09AADF58u, 0x0E6B72A6u}, // jal 0x09ADCA98: the Guild Hall's box
    {0x09AADF5Cu, 0x24050001u}, // addiu a1, zr, 1
    {kMenuDraw, 0x27BDFFF0u},   // addiu sp, sp, -16
    {0x09AE6758u, 0x3C0209B0u}, // lui v0, 0x09B0
    {0x09AE675Cu, 0x24425168u}, // addiu v0, v0, 0x5168: the hall's labels
    {0x0884FB6Cu, 0x3C0308A6u}, // lui v1, 0x08A6
    {0x0884FB70u, 0x8C632D80u}, // lw v1, 0x2D80(v1)
    {0x0884FB74u, 0x90630074u}, // lbu v1, 0x74(v1)
    {0x0884FB84u, 0x8C432D94u}, // lw v1, 0x2D94(v0)
    {0x0884FB8Cu, 0x8C422D8Cu}, // lw v0, 0x2D8C(v0)
    {0x0884FB90u, 0x306300F0u}, // andi v1, v1, 0xF0
}};

struct State {
    psprecomp::Runtime::RecompiledFunction box_update{};
    psprecomp::Runtime::RecompiledFunction menu_draw{};
    GuestCaller caller{};
    // The box whose flag is cleared for the call, and the value to put back.
    std::uint32_t lifted{};
    std::uint8_t flag{};
    // The box the refresh of this console's member entry returns to.
    std::uint32_t resume{};
    psprecomp::Runtime::RecompiledFunction loader_frame{};
    psprecomp::Runtime::RecompiledFunction request_reload{};
    psprecomp::Runtime::RecompiledFunction slot_reset{};
    // The Guild Hall's box last updated, and the one in its equipment screen
    // at the start of its last frame. A call into the game can return before
    // the game's code is done (the rest runs from the dispatcher once it
    // reaches code that cannot be chained), so nothing is decided by what
    // runs inside a wrapped call, only by memory.
    std::uint32_t hall_box{};
    std::uint32_t equipping{};
    // A reload the hall's box asked for that has not started; one running
    // with slot 1 this hunter's, and slot 1's own rows.
    bool reload_asked{};
    bool staging{};
    SlotRows kept{};
    std::uint32_t spare{};
    // The reserve a full hall reloads into, and the block it started from.
    BlockAllocator allocate{};
    bool from_reserve{};
    bool reserve_set{};
    SlotRows reserve{};
    std::uint32_t block{};
    std::uint32_t block_bytes{};
    // The others are told to load this hunter again once the refresh ran.
    bool announce{};
    Announcement announcement{};
    // This console's entry matched its record when Change Equipment was chosen.
    bool entry_matched{};
    bool reported{};
    bool trace{};
    bool trace_models{}; // MHFU_TRACE_HALL_EQUIPMENT=2: the models and slots, each frame they change
};

State &state() {
    static State s;
    return s;
}

// The reserve's rows, if they can take slot 0's parts now: the reserve as the
// last reload left it, or else the block, taken the first time it is needed.
bool usable_reserve(psprecomp::GuestMemory &memory, SlotRows &rows) {
    State &s = state();
    SlotRows own;
    if (!read_slot(memory, kOwnSlot, own)) return false;
    if (s.reserve_set && reserve_fits(memory, s.reserve)) {
        rows = s.reserve;
        return true;
    }
    const std::uint32_t bytes = reserve_bytes(own);
    if (bytes == 0u) return false;
    if (s.block == 0u && s.allocate != nullptr) {
        s.block = s.allocate(bytes);
        s.block_bytes = s.block != 0u ? bytes : 0u;
        if (s.trace)
            std::cerr << "[hall equipment] reserve of " << std::hex << bytes << " bytes at " << s.block << std::dec
                      << std::endl;
    }
    if (s.block == 0u || bytes > s.block_bytes) return false;
    const SlotRows fresh = reserve_layout(own, s.block);
    if (!reserve_fits(memory, fresh)) return false;
    s.reserve = fresh;
    s.reserve_set = true;
    rows = fresh;
    return true;
}

void restore(psprecomp::GuestMemory &memory) {
    State &s = state();
    if (s.lifted == 0u) return;
    memory.store8(s.lifted + kHallFlag, s.flag);
    s.lifted = 0u;
}

// After the refresh: the box's frame, with the box in a0 and the frame's own
// return address, which the guest call has put back.
void resume_box(psprecomp::AllegrexContext &ctx) {
    ctx.gpr[4] = state().resume;
    ctx.pc = ported(kBoxUpdate);
}

void box_update(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t box = ctx.gpr[4];
    restore(memory);
    const bool on = settings::current().hall_equipment;
    const bool hall = on && memory.contains(box, kShown + 1u) && memory.load8(box + kHallFlag) != 0u;
    const bool equipping = on && in_equipment_screen(memory, box);
    // Back on the top menu from the equipment screen since the last frame. At
    // the frame's entry only a0 and ra are live, so the refresh runs first as
    // an ordinary guest call, and then this frame.
    if (s.equipping == box && !equipping) {
        s.equipping = 0u;
        const std::uint32_t net = session(memory);
        if (hall && memory.load8(box + kScreen) == 0u && s.caller != nullptr && net != 0u) {
            if (s.trace) std::cout << "[hall equipment] refreshing this hunter's entry for the hall" << std::endl;
            s.announce = true;
            s.resume = box;
            s.caller(ctx, ported(kRefreshOwnEntry), net, &resume_box);
            return;
        }
    }
    s.equipping = equipping ? box : 0u;
    s.hall_box = hall ? box : 0u;
    // Readied for a quest, or without a free slot or the reserve for the new
    // pieces, the game's refusal stands.
    SlotRows reserve;
    if (on && refused_choice(memory, box) && !readied(memory) &&
        (spare_slot(memory) >= 0 || usable_reserve(memory, reserve))) {
        s.lifted = box;
        s.flag = memory.load8(box + kHallFlag);
        memory.store8(box + kHallFlag, 0u);
        const int differences = entry_differences(memory, own_entry(memory, session(memory)));
        s.entry_matched = differences == 0;
        if (s.trace)
            std::cout << "[hall equipment] Change Equipment chosen in box " << std::hex << box << std::dec
                      << "; this console's entry differs from its record in " << differences << " bytes"
                      << std::endl;
    }
    // The flag comes back when the frame draws its menu (menu_draw), after the
    // choice; and at the next frame or flip if the frame never got there.
    ctx.pc = ported(kBoxUpdate);
    s.box_update(runtime, ctx);
}

void request_reload(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    // The equipment screen's commit, while the hall's box is in it.
    if (settings::current().hall_equipment && s.hall_box != 0u && in_equipment_screen(runtime.memory(), s.hall_box)) {
        s.reload_asked = true;
        if (s.trace) std::cerr << "[hall equipment] the equipment screen asks for this hunter's models" << std::endl;
    }
    ctx.pc = ported(kRequestOwnReload);
    s.request_reload(runtime, ctx);
}

bool own_reload_at_head(const psprecomp::GuestMemory &memory, std::uint32_t loader) {
    if (!memory.contains(loader, 4u)) return false;
    const std::uint32_t head = memory.load32(loader);
    return head >= kOwnReloadFirst && head <= kOwnReloadLast;
}

// Slot 1 becomes this hunter's for the reload the hall's box asked for, and
// is given back once that reload has left the head of the loader.
void stage(psprecomp::GuestMemory &memory, std::uint32_t loader) {
    State &s = state();
    const bool running = own_reload_at_head(memory, loader);
    if (!s.staging && running && s.reload_asked) {
        s.reload_asked = false;
        const int spare = spare_slot(memory);
        SlotRows rows;
        const bool slot = spare >= 0 && read_slot(memory, static_cast<std::uint32_t>(spare), rows);
        if (!read_slot(memory, kStagingSlot, s.kept) || (!slot && !usable_reserve(memory, rows))) {
            if (s.trace) std::cerr << "[hall equipment] no free slot or reserve for the reload" << std::endl;
            return;
        }
        s.from_reserve = !slot;
        s.spare = slot ? static_cast<std::uint32_t>(spare) : 0u;
        write_slot(memory, kStagingSlot, rows);
        s.staging = true;
        if (s.trace) {
            if (slot)
                std::cerr << "[hall equipment] reload into free slot " << spare << std::endl;
            else
                std::cerr << "[hall equipment] reload into the reserve at " << std::hex << rows.address[0] << std::dec
                          << std::endl;
        }
    } else if (s.staging && !running) {
        SlotRows staged, own, spare = s.reserve;
        if (read_slot(memory, kStagingSlot, staged) && read_slot(memory, kOwnSlot, own) &&
            (s.from_reserve || read_slot(memory, s.spare, spare))) {
            take_loaded(staged, own, spare);
            write_slot(memory, kOwnSlot, own);
            if (s.from_reserve)
                s.reserve = spare;
            else
                write_slot(memory, s.spare, spare);
        }
        write_slot(memory, kStagingSlot, s.kept);
        s.staging = false;
        if (s.trace) std::cerr << "[hall equipment] new pieces in slot 0, slot 1 given back" << std::endl;
    }
}

// A reset of slot 1 from anything but the reload resets slot 1's own rows.
void slot_reset(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t back = ctx.gpr[31];
    SlotRows staged;
    const bool redirect = s.staging && (ctx.gpr[5] & 0xFFu) == kStagingSlot &&
                          install::traced_code(back, install::Image::lobby_task) != kReloadResetReturn &&
                          read_slot(memory, kStagingSlot, staged);
    if (redirect) write_slot(memory, kStagingSlot, s.kept);
    ctx.pc = ported(kSlotReset);
    s.slot_reset(runtime, ctx);
    if (!redirect) return;
    if (ctx.pc != back && s.trace)
        std::cerr << "[hall equipment] slot 1's reset did not return in one call" << std::endl;
    (void)read_slot(memory, kStagingSlot, s.kept);
    write_slot(memory, kStagingSlot, staged);
    if (s.trace)
        std::cerr << "[hall equipment] slot 1 reset from " << std::hex << back << std::dec
                  << " during the reload went to its own rows" << std::endl;
}

void loader_frame(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    State &s = state();
    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t loader = ctx.gpr[4];
    stage(memory, loader);
    ctx.pc = ported(kLoaderFrame);
    s.loader_frame(runtime, ctx);
    stage(memory, loader);
}

void menu_draw(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    restore(runtime.memory());
    ctx.pc = ported(kMenuDraw);
    state().menu_draw(runtime, ctx);
}

void set_label(psprecomp::GuestMemory &memory, bool on) {
    for (std::uint32_t row = 0; row < kHallLabelRows; ++row) {
        const std::uint32_t at = ported(kHallLabels) + 2u * row;
        const std::uint16_t label = memory.load16(at);
        if (label != kGreyLabel && label != kLabel) continue;
        const std::uint16_t wanted = on ? kLabel : kGreyLabel;
        if (label != wanted) memory.store16(at, wanted);
        return;
    }
}

// The hall's call to its box as FUComplete's Full Chest option leaves it, on
// Freedom Unite Complete: a jump out, and no word passing the flag.
bool full_chest_call(const psprecomp::GuestMemory &memory, const CodeWord &expected) {
    if (expected.address != kHallFlagArgument || install::running_map() == nullptr) return false;
    const std::uint32_t call = ported(kHallBoxCall), argument = ported(kHallFlagArgument);
    return memory.contains(call, 4u) && memory.contains(argument, 4u) && memory.load32(call) >> 26 == 2u &&
           memory.load32(argument) == 0u;
}

} // namespace

std::span<const CodeWord> code_signature() {
    return kSignature;
}

bool code_matches(const psprecomp::GuestMemory &memory) {
    for (const CodeWord &expected : kSignature)
        if (!install::code_matches(memory, expected.address, expected.word, install::Image::lobby_task) &&
            !layout::box_button_test(memory, expected.address, expected.word) && !full_chest_call(memory, expected))
            return false;
    return memory.contains(ported(kHallLabel), 2u);
}

std::uint16_t pressed(const psprecomp::GuestMemory &memory) {
    const std::uint32_t gate_at = ported(kPadGate), pressed_at = ported(kPadPressed), repeat_at = ported(kPadRepeat);
    if (!memory.contains(gate_at, 4u) || !memory.contains(pressed_at, 4u) || !memory.contains(repeat_at, 4u))
        return 0u;
    const std::uint32_t gate = memory.load32(gate_at);
    if (!memory.contains(gate + kPadGateByte, 1u) || memory.load8(gate + kPadGateByte) != 0u) return 0u;
    // As ULUS-10391's box reads them, where choose and back trade places.
    return layout::as_traced_box_buttons(
        memory, static_cast<std::uint16_t>((memory.load32(repeat_at) & 0xF0u) | memory.load32(pressed_at)));
}

bool refused_choice(const psprecomp::GuestMemory &memory, std::uint32_t box) {
    if (!memory.contains(box, kShown + 1u)) return false;
    if (memory.load8(box + kHallFlag) == 0u || memory.load8(box + kScreen) != 0u || memory.load8(box + kShown) == 0u ||
        memory.load8(box + kCursor) != kChangeEquipment)
        return false;
    const std::uint16_t buttons = pressed(memory);
    // Start would put the menu away in this frame, and cancel closes the box first.
    return (buttons & kConfirm) != 0u && (buttons & (kStart | kCancel)) == 0u;
}

// The hunters' models and the loads that replace them, as one line whenever
// any of it changes while lobby_task runs: this console's index (byte +0x28
// of load32(0x089CC558)); for each of the first four hunters
// (load32(load32(0x09A4F204) + 0xC + 4 * i)) its address and the model
// pointers the draw at 0x088D3F68 reads (+0x190, +0xF60, +0x1090); the hall's
// model loader requests (load32(0x09B23790), 8 of 0x84 bytes: the state word
// and the hunter it is for); and the file load's part progress (+0x24 to
// +0x3C of load32(0x09A4F204)).
void trace_models(const psprecomp::GuestMemory &memory) {
    static std::string last;
    const auto word = [&](std::uint32_t address) {
        return memory.contains(address, 4u) ? memory.load32(address) : 0u;
    };
    std::ostringstream line;
    line << std::hex;
    const std::uint32_t game = word(ported(0x089CC558u));
    line << "own " << (memory.contains(game + 0x28u, 1u) ? unsigned(memory.load8(game + 0x28u)) : 0xFFu);
    const std::uint32_t files = word(ported(0x09A4F204u));
    for (std::uint32_t i = 0; i < 4u; ++i) {
        const std::uint32_t hunter = files != 0u ? word(files + 0xCu + 4u * i) : 0u;
        line << " | h" << i << " " << hunter;
        if (hunter != 0u)
            line << " " << word(hunter + 0x190u) << " " << word(hunter + 0xF60u) << " " << word(hunter + 0x1090u);
    }
    const std::uint32_t loader = word(ported(0x09B23790u));
    line << " | load";
    for (std::uint32_t k = 0; k < 8u && loader != 0u; ++k)
        line << " " << word(loader + 0x84u * k) << ":" << word(loader + 0x84u * k + 4u);
    line << " | parts";
    for (std::uint32_t p = 0; p < 7u && files != 0u; ++p) line << " " << word(files + 0x24u + 4u * p);
    // The session: this console's status and counter, and each member's
    // address (last byte) and the status it sent (entry + 0xBF).
    const std::uint32_t net = session(memory);
    if (net != 0u) {
        const auto count = static_cast<std::int8_t>(memory.load8(net + 0xEE3u));
        line << " | net " << unsigned(memory.load8(net + kStatus)) << " " << unsigned(memory.load8(net + 0x12D4u))
             << " n" << int(count);
        for (int m = 0; m < count && m < 4; ++m) {
            const std::uint32_t entry = net + 0x57Cu + 0xC8u * static_cast<std::uint32_t>(m);
            line << " " << unsigned(memory.load8(entry + 5u)) << ":" << unsigned(memory.load8(entry + 0xBFu));
        }
    }
    // The archive's files (load32(0x08A62D7C)), as 0x088673CC keeps them: for
    // entry e (hunter * 8 + part) the kind byte and preset word at
    // +0x800A8 + 8e, the state byte at +0x803D8 + e, the address at +0x80258 + 4e
    // and the size at +0x80318 + 4e; and its two bump pools (0x08866B5C:
    // base +0x80230, used +0x8022C; 0x08866BD4: base +0x80248, used +0x80244,
    // size +0x80240).
    const std::uint32_t archive = word(ported(0x08A62D7Cu));
    if (archive != 0u && memory.contains(archive + 0x80400u, 4u)) {
        line << " | pools " << word(archive + 0x80230u) << "+" << word(archive + 0x8022Cu) << " "
             << word(archive + 0x80248u) << "+" << word(archive + 0x80244u) << "/" << word(archive + 0x80240u);
        for (std::uint32_t e = 0; e < 4u * kSlotParts; ++e) {
            line << (e % 8u == 0u ? " | e" : " ") << e << "=" << unsigned(memory.load8(archive + 0x800A8u + 8u * e)) << ","
                 << word(archive + 0x800ACu + 8u * e) << "," << unsigned(memory.load8(archive + 0x803D8u + e)) << ","
                 << word(archive + 0x80258u + 4u * e) << "," << word(archive + 0x80318u + 4u * e);
        }
    }
    if (line.str() == last) return;
    last = line.str();
    // stderr, with the [io] and [kernel] lines, in the order they happened.
    std::cerr << "[hall equipment] " << last << std::endl;
}

bool in_equipment_screen(const psprecomp::GuestMemory &memory, std::uint32_t box) {
    if (!memory.contains(box, kShown + 1u)) return false;
    return memory.load8(box + kHallFlag) != 0u && memory.load8(box + kScreen) != 0u &&
           memory.load8(box + kCursor) == kChangeEquipment;
}

std::uint32_t session(const psprecomp::GuestMemory &memory) {
    const std::uint32_t pointer = ported(kSessionPointer);
    if (!memory.contains(pointer, 4u)) return 0u;
    const std::uint32_t net = memory.load32(pointer);
    return net != 0u && memory.contains(net, kSessionBytes) ? net : 0u;
}

bool readied(const psprecomp::GuestMemory &memory) {
    const std::uint32_t net = session(memory);
    if (net == 0u) return false;
    const std::uint8_t status = memory.load8(net + kStatus);
    return status >= kReadyFirst && status <= kReadyLast;
}

namespace {

bool slot_tables(const psprecomp::GuestMemory &memory, std::uint32_t &archive, std::uint32_t &files) {
    const std::uint32_t archive_at = ported(kArchivePointer), files_at = ported(kFilesPointer);
    if (!memory.contains(archive_at, 4u) || !memory.contains(files_at, 4u)) return false;
    archive = memory.load32(archive_at);
    files = memory.load32(files_at);
    return archive != 0u && files != 0u && memory.contains(archive, kArchiveBytes) &&
           memory.contains(files, kSlotProgress + 4u * 6u);
}

} // namespace

bool read_slot(const psprecomp::GuestMemory &memory, std::uint32_t slot, SlotRows &rows) {
    std::uint32_t archive{}, files{};
    if (slot >= 6u || !slot_tables(memory, archive, files)) return false;
    for (std::uint32_t p = 0; p < kSlotParts; ++p) {
        const std::uint32_t e = slot * kSlotParts + p;
        rows.kind[p] = memory.load32(archive + kRowKind + 8u * e);
        rows.room[p] = memory.load32(archive + kRowKind + 8u * e + 4u);
        rows.address[p] = memory.load32(archive + kRowAddress + 4u * e);
        rows.size[p] = memory.load32(archive + kRowSize + 4u * e);
        rows.state[p] = memory.load8(archive + kRowState + e);
        rows.file[p] = memory.load16(archive + kRowFile + 2u * e);
    }
    rows.progress = memory.load32(files + kSlotProgress + 4u * slot);
    return true;
}

bool write_slot(psprecomp::GuestMemory &memory, std::uint32_t slot, const SlotRows &rows) {
    std::uint32_t archive{}, files{};
    if (slot >= 6u || !slot_tables(memory, archive, files)) return false;
    for (std::uint32_t p = 0; p < kSlotParts; ++p) {
        const std::uint32_t e = slot * kSlotParts + p;
        memory.store32(archive + kRowKind + 8u * e, rows.kind[p]);
        memory.store32(archive + kRowKind + 8u * e + 4u, rows.room[p]);
        memory.store32(archive + kRowAddress + 4u * e, rows.address[p]);
        memory.store32(archive + kRowSize + 4u * e, rows.size[p]);
        memory.store8(archive + kRowState + e, rows.state[p]);
        memory.store16(archive + kRowFile + 2u * e, rows.file[p]);
    }
    memory.store32(files + kSlotProgress + 4u * slot, rows.progress);
    return true;
}

std::uint32_t own_entry(const psprecomp::GuestMemory &memory, std::uint32_t net) {
    const std::uint32_t own = ported(kOwnAddress);
    if (net == 0u || !memory.contains(net, kSessionBytes) || !memory.contains(own, 6u)) return 0u;
    const auto count = static_cast<std::int8_t>(memory.load8(net + kMemberCount));
    for (int m = 0; m < count && m < 16; ++m) {
        const std::uint32_t entry = net + kMembers + kMemberBytes * static_cast<std::uint32_t>(m);
        bool same = true;
        for (std::uint32_t b = 0; b < 6u && same; ++b) same = memory.load8(entry + b) == memory.load8(own + b);
        if (same) return entry;
    }
    return 0u;
}

namespace {

bool mirrored(std::uint32_t i) {
    return i < kMirrorGapFirst || (i >= kMirrorGapEnd && i < kMirrorEnd);
}

std::uint32_t own_record(const psprecomp::GuestMemory &memory) {
    const std::uint32_t pointer = ported(kGameSysPointer);
    if (!memory.contains(pointer, 4u)) return 0u;
    const std::uint32_t record = memory.load32(pointer) + kOwnRecord;
    return memory.contains(record + kRecordMirror, kMirrorEnd) ? record : 0u;
}

} // namespace

int entry_differences(const psprecomp::GuestMemory &memory, std::uint32_t entry) {
    const std::uint32_t record = own_record(memory);
    if (entry == 0u || record == 0u || !memory.contains(entry + kMemberHunter, kMirrorEnd)) return -1;
    int differences = 0;
    for (std::uint32_t i = 0; i < kMirrorEnd; ++i)
        if (mirrored(i) && memory.load8(entry + kMemberHunter + i) != memory.load8(record + kRecordMirror + i))
            ++differences;
    return differences;
}

bool copy_record_to_entry(psprecomp::GuestMemory &memory, std::uint32_t entry) {
    const std::uint32_t record = own_record(memory);
    if (entry == 0u || record == 0u || !memory.contains(entry + kMemberHunter, kMirrorEnd)) return false;
    for (std::uint32_t i = 0; i < kMirrorEnd; ++i)
        if (mirrored(i)) memory.store8(entry + kMemberHunter + i, memory.load8(record + kRecordMirror + i));
    return true;
}

bool start_announcement(psprecomp::GuestMemory &memory, std::uint32_t net, Announcement &announcement) {
    if (net == 0u || !memory.contains(net, kSessionBytes)) return false;
    const std::uint8_t status = memory.load8(net + kStatus);
    if (status != kHallStatus) return false;
    announcement.status = status;
    announcement.frames = 0u;
    announcement.running = true;
    memory.store8(net + kStatus, kAbsentStatus);
    memory.store32(net + kSendCounter, 0xFFFFFFFFu);
    return true;
}

void step_announcement(psprecomp::GuestMemory &memory, std::uint32_t net, Announcement &announcement) {
    if (!announcement.running) return;
    if (net == 0u || !memory.contains(net, kSessionBytes)) {
        announcement.running = false;
        return;
    }
    if (++announcement.frames < kAbsentFrames) return;
    announcement.running = false;
    // A status the game set in the meantime is the game's.
    if (memory.load8(net + kStatus) != kAbsentStatus) return;
    memory.store8(net + kStatus, announcement.status);
    memory.store32(net + kSendCounter, 0xFFFFFFFFu);
}

int spare_slot(const psprecomp::GuestMemory &memory) {
    std::uint32_t archive{}, files{};
    SlotRows own, rows;
    if (!slot_tables(memory, archive, files) || !read_slot(memory, kOwnSlot, own)) return -1;
    const std::uint32_t loader_at = ported(kLoaderPointer);
    const std::uint32_t loader = memory.contains(loader_at, 4u) ? memory.load32(loader_at) : 0u;
    for (std::uint32_t slot = kLastSpareSlot + 1u; slot-- > kFirstSpareSlot;) {
        if (memory.load32(files + kHunters + 4u * slot) != 0u || !read_slot(memory, slot, rows)) continue;
        bool free = true;
        for (std::uint32_t p = 0; p < kSlotParts && free; ++p)
            free = rows.state[p] == 0u && rows.address[p] != 0u && rows.room[p] != 0xFFFFFFFFu &&
                   rows.room[p] >= own.room[p];
        for (std::uint32_t k = 0; k < kLoaderRequests && free && loader != 0u; ++k) {
            const std::uint32_t request = loader + kLoaderRequestBytes * k;
            free = !memory.contains(request, 8u) || memory.load32(request) == 0u || memory.load32(request + 4u) != slot;
        }
        if (free) return static_cast<int>(slot);
    }
    return -1;
}

void take_loaded(const SlotRows &staged, SlotRows &own, SlotRows &spare) {
    for (std::uint32_t p = 0; p < kSlotParts; ++p) {
        if (staged.state[p] == 0u) continue;
        const std::uint32_t old = own.address[p];
        const std::uint32_t old_room = own.room[p];
        own.kind[p] = staged.kind[p];
        own.room[p] = staged.room[p];
        own.address[p] = staged.address[p];
        own.size[p] = staged.size[p];
        own.state[p] = staged.state[p];
        own.file[p] = staged.file[p];
        spare.address[p] = old;
        spare.room[p] = old_room;
        spare.state[p] = 0u;
        spare.file[p] = kNoFile;
    }
}

std::uint32_t reserve_bytes(const SlotRows &own) {
    std::uint32_t bytes = 0u;
    for (std::uint32_t p = 0; p < kSlotParts; ++p) {
        if (own.room[p] == kPooledRoom || own.room[p] == 0u) return 0u;
        bytes += (own.room[p] + kReserveAlignment - 1u) & ~(kReserveAlignment - 1u);
    }
    return bytes;
}

SlotRows reserve_layout(const SlotRows &own, std::uint32_t base) {
    SlotRows rows;
    std::uint32_t offset = 0u;
    for (std::uint32_t p = 0; p < kSlotParts; ++p) {
        rows.kind[p] = own.kind[p];
        rows.room[p] = own.room[p];
        rows.address[p] = base + offset;
        rows.size[p] = own.room[p];
        rows.state[p] = 0u;
        rows.file[p] = kNoFile;
        offset += (own.room[p] + kReserveAlignment - 1u) & ~(kReserveAlignment - 1u);
    }
    rows.progress = 0u;
    return rows;
}

bool reserve_fits(const psprecomp::GuestMemory &memory, const SlotRows &rows) {
    SlotRows own;
    if (!read_slot(memory, kOwnSlot, own)) return false;
    for (std::uint32_t p = 0; p < kSlotParts; ++p)
        if (rows.address[p] == 0u || own.room[p] == kPooledRoom || rows.room[p] < own.room[p] ||
            !memory.contains(rows.address[p], rows.room[p]))
            return false;
    SlotRows slot;
    for (std::uint32_t i = 0; i < kSlots; ++i) {
        if (!read_slot(memory, i, slot)) return false;
        for (std::uint32_t q = 0; q < kSlotParts; ++q) {
            const std::uint32_t length = slot.room[q] == kPooledRoom ? slot.size[q] : slot.room[q];
            if (slot.address[q] == 0u || length == 0u) continue;
            for (std::uint32_t p = 0; p < kSlotParts; ++p)
                if (rows.address[p] < slot.address[q] + length && slot.address[q] < rows.address[p] + rows.room[p])
                    return false;
        }
    }
    return true;
}

void frame(psprecomp::Runtime &runtime, GuestCaller caller, BlockAllocator allocate) {
    State &s = state();
    s.caller = caller;
    s.allocate = allocate;
    static const bool trace = [] {
        const char *text = std::getenv("MHFU_TRACE_HALL_EQUIPMENT");
        return text != nullptr && *text != '\0' && std::string(text) != "0";
    }();
    s.trace = trace;
    psprecomp::GuestMemory &memory = runtime.memory();
    if (s.lifted != 0u && memory.contains(s.lifted, kShown + 1u)) restore(memory);
    // Every overlay in lobby_task's window is checked by these words.
    if (!code_matches(memory)) {
        // Left the hall during a reload: slot 1 goes back as it was.
        if (s.staging) {
            write_slot(memory, kStagingSlot, s.kept);
            s.staging = false;
        }
        s.reload_asked = false;
        if (s.announcement.running) {
            s.announcement.frames = kAbsentFrames;
            step_announcement(memory, session(memory), s.announcement);
        }
        s.announce = false;
        return;
    }
    if (s.trace) trace_models(memory);
    set_label(memory, settings::current().hall_equipment && !readied(memory));
    if (s.announcement.running) {
        step_announcement(memory, session(memory), s.announcement);
        if (s.trace && !s.announcement.running)
            std::cerr << "[hall equipment] status " << unsigned(s.announcement.status) << " again" << std::endl;
    } else if (s.announce) {
        s.announce = false;
        const std::uint32_t entry = own_entry(memory, session(memory));
        const int differences = entry_differences(memory, entry);
        const bool copied = s.entry_matched && copy_record_to_entry(memory, entry);
        if (s.trace)
            std::cerr << "[hall equipment] the new equipment is " << differences << " bytes of this console's entry; "
                      << (copied ? "copied" : "not copied (the entry did not match the record before)")
                      << std::endl;
        const bool started = start_announcement(memory, session(memory), s.announcement);
        if (s.trace)
            std::cerr << "[hall equipment] "
                      << (started ? "status 1 for the others to load this hunter again"
                                  : "status not one the others keep; not announced")
                      << std::endl;
    }
    const std::uint32_t box_at = ported(kBoxUpdate), draw_at = ported(kMenuDraw), loader_at = ported(kLoaderFrame),
                        request_at = ported(kRequestOwnReload), reset_at = ported(kSlotReset);
    const psprecomp::Runtime::RecompiledFunction box = runtime.function_at(box_at);
    const psprecomp::Runtime::RecompiledFunction draw = runtime.function_at(draw_at);
    const psprecomp::Runtime::RecompiledFunction loader = runtime.function_at(loader_at);
    const psprecomp::Runtime::RecompiledFunction request = runtime.function_at(request_at);
    // The slot reset is the executable's, so it is wrapped once and stays so;
    // outside a staged reload the wrapper only calls it.
    const psprecomp::Runtime::RecompiledFunction reset = runtime.function_at(reset_at);
    if (reset != nullptr && reset != &slot_reset) {
        s.slot_reset = reset;
        runtime.register_function(reset_at, &slot_reset, "mhfu_hall_equipment_slot_reset");
    }
    if (box == nullptr || draw == nullptr || loader == nullptr || request == nullptr) return;
    if (loader != &loader_frame) {
        s.loader_frame = loader;
        runtime.register_function(loader_at, &loader_frame, "mhfu_hall_equipment_loader");
    }
    if (request != &request_reload) {
        s.request_reload = request;
        runtime.register_function(request_at, &request_reload, "mhfu_hall_equipment_request");
    }
    // lobby_task registers its functions again each time it is installed.
    // Item Sets wraps the box's frame too (game/hooks.hpp).
    (void)hooks::wrap(runtime, box_at, &box_update, s.box_update, "mhfu_hall_equipment");
    if (draw != &menu_draw) {
        s.menu_draw = draw;
        runtime.register_function(draw_at, &menu_draw, "mhfu_hall_equipment_draw");
    }
    if (!s.reported) {
        s.reported = true;
        std::cout << "[hall equipment] Guild Hall box wrapped" << std::endl;
    }
}

} // namespace mhfu::game::hall_equipment
