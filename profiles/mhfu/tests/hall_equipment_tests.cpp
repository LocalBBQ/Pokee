// Change Equipment in the Guild Hall: the button read, the refused choice, the
// label, the code check, the wrappers around a stand-in box, and the refresh
// of this console's member entry through a stand-in guest call. No game data.
#include "game/hall_equipment.hpp"

#include "game/guest_layout.hpp"
#include "install/release_addresses.hpp"
#include "psprecomp/runtime.hpp"
#include "settings/settings.hpp"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace {
using namespace mhfu::game::hall_equipment;

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
    // _putenv_s removes a variable set to "", so do the same here.
    if (value.empty())
        unsetenv(name);
    else
        setenv(name, value.c_str(), 1);
#endif
}

constexpr std::uint32_t kBox = 0x09A40000u;
constexpr std::uint32_t kGate = 0x09900000u;
constexpr std::uint32_t kSession = 0x09980000u;
constexpr std::uint32_t kReturn = 0x09A7B0B8u;
constexpr std::uint32_t kCallReturn = 0x0880000Cu; // where the stand-in guest call returns
constexpr std::uint32_t kMemory = 32u * 1024u * 1024u;

// What the stand-ins saw.
int box_calls{};
int draw_calls{};
int refresh_calls{};
std::uint8_t flag_at_choice{};
std::uint8_t flag_at_draw{};
std::uint32_t box_seen{};
std::uint32_t refresh_argument{};
// The screen the stand-in box leaves, or none; whether it commits a change.
int screen_after = -1;
bool commit_in_box{};
bool commit_unwound{};
// The stand-in guest call: the caller's return address and what follows.
std::uint32_t call_return{};
ThenFunction call_then{};

void write_signature(psprecomp::GuestMemory &memory) {
    for (const CodeWord &word : code_signature()) memory.store32(word.address, word.word);
    memory.store16(kHallLabel, kGreyLabel);
}

void set_buttons(psprecomp::GuestMemory &memory, std::uint16_t buttons) {
    memory.store32(kPadGate, kGate);
    memory.store8(kGate + kPadGateByte, 0u);
    memory.store32(kPadPressed, buttons);
    memory.store32(kPadRepeat, 0u);
}

// The Guild Hall's box on its top menu, the cursor on Change Equipment.
void open_hall_box(psprecomp::GuestMemory &memory) {
    memory.store8(kBox + kHallFlag, 1u);
    memory.store8(kBox + kScreen, 0u);
    memory.store8(kBox + kShown, 1u);
    memory.store8(kBox + kCursor, kChangeEquipment);
}

void game_draw(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    ++draw_calls;
    flag_at_draw = runtime.memory().load8(ctx.gpr[4] + kHallFlag);
    ctx.pc = ctx.gpr[31];
}

// The box's frame as far as the flag goes: the choice, then the menu drawn
// through the dispatch table, as 0x09ADD0E8 calls it.
void game_box(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    ++box_calls;
    const std::uint32_t box = ctx.gpr[4];
    const std::uint32_t back = ctx.gpr[31];
    box_seen = box;
    flag_at_choice = runtime.memory().load8(box + kHallFlag);
    psprecomp::AllegrexContext draw{};
    draw.gpr[4] = box;
    draw.gpr[31] = 0x09ADD0F0u;
    runtime.function_at(kMenuDraw)(runtime, draw);
    if (commit_unwound) {
        // The call returns before the commit runs: the commit's request is
        // reached from the dispatcher, after the wrapped call has returned.
        ctx.gpr[4] = runtime.memory().load32(kLoaderPointer);
        ctx.gpr[31] = back;
        ctx.pc = kRequestOwnReload;
        return;
    }
    if (commit_in_box) {
        // The equipment screen's commit asks for this hunter again (0x09AA16D0).
        psprecomp::AllegrexContext request{};
        request.gpr[4] = runtime.memory().load32(kLoaderPointer);
        request.gpr[31] = 0x09AA16D8u;
        runtime.function_at(kRequestOwnReload)(runtime, request);
    }
    if (screen_after >= 0) runtime.memory().store8(box + kScreen, static_cast<std::uint8_t>(screen_after));
    ctx.gpr[4] = 0xDEADBEEFu; // a0 is the callee's
    ctx.pc = back;
}

void game_refresh(psprecomp::Runtime &, psprecomp::AllegrexContext &ctx) {
    ++refresh_calls;
    refresh_argument = ctx.gpr[4];
    ctx.gpr[4] = 0u;
    ctx.pc = ctx.gpr[31];
}

// Kernel::call_guest as far as the call goes.
void guest_call(psprecomp::AllegrexContext &ctx, std::uint32_t function, std::uint32_t argument, ThenFunction then) {
    call_return = ctx.gpr[31];
    call_then = then;
    ctx.gpr[4] = argument;
    ctx.gpr[31] = kCallReturn;
    ctx.pc = function;
}

// The kernel's block allocation: the block it hands out, or none.
constexpr std::uint32_t kBlock = 0x09D80000u;
std::uint32_t block_answer = kBlock;
int allocations{};
std::uint32_t allocated_bytes{};

std::uint32_t allocate(std::uint32_t size) {
    ++allocations;
    allocated_bytes = size;
    return block_answer;
}

// One frame of the box, from the hall's call to its return, through the
// dispatch table and the guest call's return.
void run_box(psprecomp::Runtime &runtime) {
    psprecomp::AllegrexContext ctx{};
    ctx.gpr[4] = kBox;
    ctx.gpr[31] = kReturn;
    ctx.pc = kBoxUpdate;
    for (int step = 0; step < 8 && ctx.pc != kReturn; ++step) {
        if (ctx.pc == kCallReturn) {
            ctx.gpr[31] = call_return;
            ctx.pc = call_return;
            call_then(ctx);
            continue;
        }
        runtime.function_at(ctx.pc)(runtime, ctx);
    }
    check(ctx.pc == kReturn && ctx.gpr[31] == kReturn, "the box's frame returns to its caller");
}

void test_pressed() {
    psprecomp::GuestMemory memory(kMemory);
    set_buttons(memory, kConfirm);
    check(pressed(memory) == kConfirm, "the pressed word");
    memory.store32(kPadRepeat, 0x1234u);
    check(pressed(memory) == (kConfirm | 0x30u), "only the D-pad bits of the repeat word");
    memory.store8(kGate + kPadGateByte, 1u);
    check(pressed(memory) == 0u, "no buttons while the game holds input back");
    memory.store32(kPadGate, 0x7FFFFFF0u);
    check(pressed(memory) == 0u, "no buttons without the gate");
}

void test_refused_choice() {
    psprecomp::GuestMemory memory(kMemory);
    open_hall_box(memory);
    set_buttons(memory, kConfirm);
    check(refused_choice(memory, kBox), "confirm on Change Equipment in the hall");
    memory.store8(kBox + kCursor, 2u);
    check(!refused_choice(memory, kBox), "another entry is the game's");
    open_hall_box(memory);
    memory.store8(kBox + kHallFlag, 0u);
    check(!refused_choice(memory, kBox), "the house's box is the game's");
    open_hall_box(memory);
    memory.store8(kBox + kScreen, 1u);
    check(!refused_choice(memory, kBox), "not on the top menu");
    check(in_equipment_screen(memory, kBox), "but in the equipment screen");
    open_hall_box(memory);
    check(!in_equipment_screen(memory, kBox), "the top menu is not the equipment screen");
    memory.store8(kBox + kShown, 0u);
    check(!refused_choice(memory, kBox), "the menu put away takes no choice");
    open_hall_box(memory);
    set_buttons(memory, 0u);
    check(!refused_choice(memory, kBox), "no confirm");
    set_buttons(memory, kConfirm | kStart);
    check(!refused_choice(memory, kBox), "confirm with Start");
    set_buttons(memory, kConfirm | kCancel);
    check(!refused_choice(memory, kBox), "confirm with cancel");
    check(!refused_choice(memory, 0x7FFFFFF0u), "a box outside memory");
    check(!in_equipment_screen(memory, 0x7FFFFFF0u), "nor its equipment screen");
}

void test_session() {
    psprecomp::GuestMemory memory(kMemory);
    check(session(memory) == 0u, "no session before ad hoc play");
    memory.store32(kSessionPointer, kSession);
    check(session(memory) == kSession, "the session object");
    memory.store32(kSessionPointer, 0x7FFFFFF0u);
    check(session(memory) == 0u, "a pointer outside memory is none");
}

void test_code_check() {
    psprecomp::GuestMemory memory(kMemory);
    check(!code_matches(memory), "empty memory is not lobby_task");
    write_signature(memory);
    check(code_matches(memory), "the traced words are lobby_task");
    memory.store32(code_signature()[18].address, 0x24020004u);
    check(!code_matches(memory), "one different word is other code");
}

constexpr std::uint32_t kLoader = 0x09B30000u;
constexpr std::uint32_t kArchive = 0x09000000u;
constexpr std::uint32_t kFiles = 0x09A50000u;

// The loader's head as the stand-in frame leaves it, and slot 1 as it saw it.
std::uint32_t head_after{};
SlotRows slot1_seen{};
int loader_calls{};

// The loader's frame as far as the slots go: it reads slot 1 and, during this
// hunter's reload, loads a new piece there.
void game_loader(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    ++loader_calls;
    psprecomp::GuestMemory &memory = runtime.memory();
    check(read_slot(memory, kStagingSlot, slot1_seen), "the stand-in loader reads slot 1");
    const std::uint32_t head = memory.load32(ctx.gpr[4]);
    if (head >= kOwnReloadFirst && head <= kOwnReloadLast) {
        SlotRows rows = slot1_seen;
        rows.file[3] = 0x0777u;
        rows.state[3] = 3u;
        write_slot(memory, kStagingSlot, rows);
    }
    memory.store32(ctx.gpr[4], head_after);
    ctx.pc = ctx.gpr[31];
}

// 0x09A7D390 as far as the head goes.
void game_request(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    runtime.memory().store32(ctx.gpr[4], kOwnReloadFirst);
    ctx.pc = ctx.gpr[31];
}

// 0x088DB534 as far as the rows go: slot a1's parts are emptied.
void game_slot_reset(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    psprecomp::GuestMemory &memory = runtime.memory();
    const std::uint32_t slot = ctx.gpr[5] & 0xFFu;
    SlotRows rows;
    if (read_slot(memory, slot, rows)) {
        rows.state.fill(0u);
        rows.file.fill(0xFFFFu);
        write_slot(memory, slot, rows);
    }
    ctx.pc = ctx.gpr[31];
}

void reset_slot(psprecomp::Runtime &runtime, std::uint32_t slot, std::uint32_t back) {
    psprecomp::AllegrexContext ctx{};
    ctx.gpr[4] = kFiles;
    ctx.gpr[5] = slot;
    ctx.gpr[31] = back;
    runtime.function_at(kSlotReset)(runtime, ctx);
    check(ctx.pc == back, "the slot reset returns to its caller");
}

void run_loader(psprecomp::Runtime &runtime, std::uint32_t after) {
    head_after = after;
    psprecomp::AllegrexContext ctx{};
    ctx.gpr[4] = kLoader;
    ctx.gpr[31] = 0x09A6C2A0u;
    runtime.function_at(kLoaderFrame)(runtime, ctx);
    check(ctx.pc == 0x09A6C2A0u, "the loader's frame returns to its caller");
}

SlotRows rows_of(std::uint32_t base, std::uint16_t file) {
    SlotRows rows;
    for (std::uint32_t p = 0; p < kSlotParts; ++p) {
        rows.kind[p] = 1u;
        rows.room[p] = 0x2000u + p * 0x800u;
        rows.address[p] = base + p * 0x8000u;
        rows.size[p] = rows.room[p];
        rows.state[p] = 3u;
        rows.file[p] = static_cast<std::uint16_t>(file + p);
    }
    rows.progress = 0u;
    return rows;
}

// An empty slot with its own memory, as the hall sets slots 2 and 3 aside.
SlotRows empty_rows(std::uint32_t base) {
    SlotRows rows = rows_of(base, 0u);
    rows.state.fill(0u);
    rows.file.fill(0xFFFFu);
    return rows;
}

void set_slots(psprecomp::GuestMemory &memory) {
    memory.store32(kArchivePointer, kArchive);
    memory.store32(kFilesPointer, kFiles);
    memory.store32(kLoaderPointer, kLoader);
    memory.store32(kFiles + kHunters, 0x090B3580u);
    memory.store32(kFiles + kHunters + 4u, 0x090D94C0u);
    memory.store32(kFiles + kHunters + 8u, 0u);
    memory.store32(kFiles + kHunters + 12u, 0u);
    write_slot(memory, kOwnSlot, rows_of(0x091BF4A0u, 0x0100u));
    write_slot(memory, kStagingSlot, rows_of(0x091E84A0u, 0x0200u));
    write_slot(memory, 2u, empty_rows(0x092114A0u));
    write_slot(memory, 3u, empty_rows(0x0923A4A0u));
    for (std::uint32_t k = 0; k < kLoaderRequests; ++k) memory.store32(kLoader + kLoaderRequestBytes * k, 0u);
}

void install(psprecomp::Runtime &runtime) {
    runtime.register_function(kBoxUpdate, &game_box, "game_box");
    runtime.register_function(kMenuDraw, &game_draw, "game_draw");
    runtime.register_function(kRefreshOwnEntry, &game_refresh, "game_refresh");
    runtime.register_function(kLoaderFrame, &game_loader, "game_loader");
    runtime.register_function(kRequestOwnReload, &game_request, "game_request");
    if (runtime.function_at(kSlotReset) == nullptr)
        runtime.register_function(kSlotReset, &game_slot_reset, "game_slot_reset");
    frame(runtime, &guest_call, &allocate);
}

void test_slot_rows() {
    psprecomp::GuestMemory memory(kMemory);
    SlotRows rows;
    check(!read_slot(memory, 0u, rows), "no archive, no rows");
    set_slots(memory);
    check(read_slot(memory, kOwnSlot, rows) && rows == rows_of(0x091BF4A0u, 0x0100u), "slot 0's rows");
    check(read_slot(memory, kStagingSlot, rows) && rows == rows_of(0x091E84A0u, 0x0200u), "slot 1's rows");
    check(write_slot(memory, 3u, rows) && read_slot(memory, 3u, rows) && rows == rows_of(0x091E84A0u, 0x0200u),
        "rows written to another slot read back");
    SlotRows other;
    check(read_slot(memory, kOwnSlot, other) && other == rows_of(0x091BF4A0u, 0x0100u), "without touching slot 0");
    check(!read_slot(memory, 6u, rows) && !write_slot(memory, 6u, rows), "six slots");
}

void test_staging() {
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    auto &settings = mhfu::settings::current();
    settings.hall_equipment = true;
    write_signature(memory);
    set_slots(memory);
    install(runtime);
    const SlotRows own = rows_of(0x091BF4A0u, 0x0100u);
    const SlotRows other = rows_of(0x091E84A0u, 0x0200u);
    SlotRows rows;

    // The hall's equipment screen commits a change.
    open_hall_box(memory);
    memory.store8(kBox + kScreen, 1u);
    set_buttons(memory, 0u);
    commit_in_box = true;
    run_box(runtime);
    commit_in_box = false;
    check(memory.load32(kLoader) == kOwnReloadFirst, "the commit asks for this hunter again");
    const SlotRows spare = empty_rows(0x0923A4A0u);
    run_loader(runtime, 4u);
    check(slot1_seen == spare, "the reload sees the free slot 3 in slot 1");
    check(read_slot(memory, kStagingSlot, rows) && rows.address == spare.address, "and loads into its memory");
    check(read_slot(memory, kOwnSlot, rows) && rows == own, "the memory this hunter is drawn from is untouched");
    run_loader(runtime, 0u);
    check(read_slot(memory, kStagingSlot, rows) && rows == other, "slot 1 is the other hunter's again");
    check(read_slot(memory, kOwnSlot, rows) && rows.file[3] == 0x0777u && rows.address[3] == spare.address[3] &&
            rows.state[3] == 3u,
        "slot 0 has the new piece, in the free slot's memory");
    check(rows.address[2] == own.address[2] && rows.file[2] == own.file[2], "and its other pieces as they were");
    check(read_slot(memory, 3u, rows) && rows.address[3] == own.address[3] && rows.state[3] == 0u &&
            rows.address[2] == spare.address[2],
        "slot 3 takes the old piece's memory, empty");
    run_loader(runtime, 0u);
    check(slot1_seen == other, "the next frame of the loader sees the other hunter's slot");

    // The commit reached after the wrapped call has returned still counts.
    set_slots(memory);
    open_hall_box(memory);
    memory.store8(kBox + kScreen, 1u);
    commit_unwound = true;
    run_box(runtime);
    commit_unwound = false;
    run_loader(runtime, 0u);
    check(slot1_seen == spare, "a commit run from the dispatcher reloads into the free slot");
    check(read_slot(memory, kStagingSlot, rows) && rows == other, "and slot 1 is given back");

    // A reload the hall's box did not ask for is the game's.
    set_slots(memory);
    memory.store8(kBox + kScreen, 0u);
    psprecomp::AllegrexContext request{};
    request.gpr[4] = kLoader;
    request.gpr[31] = 0x09A5FEFCu;
    runtime.function_at(kRequestOwnReload)(runtime, request);
    run_loader(runtime, 0u);
    check(slot1_seen == other, "a reload asked for elsewhere uses slot 1 as the game does");

    // Off, the box's commit is the game's too.
    settings.hall_equipment = false;
    set_slots(memory);
    commit_in_box = true;
    run_box(runtime);
    commit_in_box = false;
    run_loader(runtime, 0u);
    check(slot1_seen == other, "off, slot 1 is not touched");
    settings.hall_equipment = true;

    // Leaving the hall during the reload gives slot 1 back.
    set_slots(memory);
    open_hall_box(memory);
    memory.store8(kBox + kScreen, 1u);
    commit_in_box = true;
    run_box(runtime);
    commit_in_box = false;
    run_loader(runtime, 4u);
    check(slot1_seen == spare, "the reload has started in the free slot");
    memory.store32(code_signature()[0].address, 0u);
    frame(runtime, &guest_call, &allocate);
    check(read_slot(memory, kStagingSlot, rows) && rows == other, "without lobby_task slot 1 is given back");
    check(read_slot(memory, kOwnSlot, rows) && rows == own, "and slot 0 is as it was");
    write_signature(memory);
}

// The hall's equipment screen commits a change and the loader runs the
// reload to its end; slot 1 as the reload saw it.
SlotRows commit_and_reload(psprecomp::Runtime &runtime) {
    open_hall_box(runtime.memory());
    runtime.memory().store8(kBox + kScreen, 1u);
    set_buttons(runtime.memory(), 0u);
    commit_in_box = true;
    run_box(runtime);
    commit_in_box = false;
    run_loader(runtime, 4u);
    const SlotRows seen = slot1_seen;
    run_loader(runtime, 0u);
    return seen;
}

void test_full_hall() {
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    auto &settings = mhfu::settings::current();
    settings.hall_equipment = true;
    write_signature(memory);
    set_slots(memory);
    install(runtime);
    // Four hunters: slots 2 and 3 are taken and loaded.
    memory.store32(kFiles + kHunters + 8u, 0x09100000u);
    memory.store32(kFiles + kHunters + 12u, 0x09110000u);
    write_slot(memory, 2u, rows_of(0x092114A0u, 0x0300u));
    write_slot(memory, 3u, rows_of(0x0923A4A0u, 0x0400u));
    check(spare_slot(memory) == -1, "a full hall has no free slot");
    const SlotRows own = rows_of(0x091BF4A0u, 0x0100u);
    const SlotRows other = rows_of(0x091E84A0u, 0x0200u);
    const SlotRows block = reserve_layout(own, kBlock);
    check(reserve_bytes(own) == 0x1E000u, "the reserve is slot 0's rooms, each rounded to 64 bytes");
    check(block.address[3] == kBlock + 0x7800u && block.state[3] == 0u && block.file[3] == kNoFile &&
            block.room == own.room && block.size == own.room,
        "laid out as an empty slot");
    SlotRows pooled = own;
    pooled.room[5] = kPooledRoom;
    check(reserve_bytes(pooled) == 0u, "a part in the bump pool has no room for a reserve");

    allocations = 0;
    open_hall_box(memory);
    set_buttons(memory, kConfirm);
    run_box(runtime);
    check(flag_at_choice == 0u, "a full hall offers Change Equipment");
    check(allocations == 1 && allocated_bytes == 0x1E000u, "with a block for the reserve");

    SlotRows seen = commit_and_reload(runtime);
    SlotRows rows;
    check(seen == block, "the reload loads into the block");
    check(read_slot(memory, kStagingSlot, rows) && rows == other, "slot 1 is the other hunter's again");
    check(read_slot(memory, kOwnSlot, rows) && rows.address[3] == block.address[3] && rows.file[3] == 0x0777u,
        "slot 0 draws the new piece from the block");
    check(rows.address[2] == own.address[2], "and its other pieces from where they were");
    for (std::uint32_t slot = 2u; slot <= 3u; ++slot)
        check(read_slot(memory, slot, rows) && rows.state[0] == 3u, "the other hunters' slots are untouched");

    // The game laid slot 0 out again: the memory the reserve held is slot
    // 0's, and the block is free.
    write_slot(memory, kOwnSlot, own);
    seen = commit_and_reload(runtime);
    check(seen == block, "a reserve in a slot's rows gives way to the block");

    // The next change reloads into slot 0's old memory, without a new block.
    open_hall_box(memory);
    set_buttons(memory, kConfirm);
    run_box(runtime);
    check(flag_at_choice == 0u && allocations == 1, "the next change needs no new block");
    seen = commit_and_reload(runtime);
    check(seen.address[3] == own.address[3] && seen.address[2] == block.address[2] && seen.file[3] == kNoFile,
        "it loads into the memory slot 0 left");
    check(read_slot(memory, kOwnSlot, rows) && rows.address[3] == own.address[3], "and slot 0 takes it back");

    // Neither is free: the reserve is the block again, and slot 0 draws from it.
    SlotRows in_block = block;
    in_block.state.fill(3u);
    write_slot(memory, kOwnSlot, in_block);
    open_hall_box(memory);
    set_buttons(memory, kConfirm);
    run_box(runtime);
    check(flag_at_choice == 1u && allocations == 1, "without free memory the game's refusal stands");
}

void test_slot_reset() {
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    auto &settings = mhfu::settings::current();
    settings.hall_equipment = true;
    write_signature(memory);
    set_slots(memory);
    install(runtime);
    check(runtime.function_at(kSlotReset) != &game_slot_reset, "the slot reset is wrapped");
    install(runtime);
    check(runtime.function_at(kSlotReset) != &game_slot_reset, "and stays wrapped once lobby_task is installed again");
    const SlotRows other = rows_of(0x091E84A0u, 0x0200u);
    SlotRows rows;

    // Outside a reload a reset of slot 1 is the game's.
    reset_slot(runtime, kStagingSlot, 0x09A7D340u);
    check(read_slot(memory, kStagingSlot, rows) && rows.state[0] == 0u && rows.address == other.address,
        "outside a reload slot 1 is reset");

    // A member who leaves during the reload: its own rows are reset, not the
    // ones being loaded.
    set_slots(memory);
    open_hall_box(memory);
    memory.store8(kBox + kScreen, 1u);
    commit_in_box = true;
    run_box(runtime);
    commit_in_box = false;
    run_loader(runtime, 4u);
    const SlotRows loading = slot1_seen;
    SlotRows before;
    check(read_slot(memory, kStagingSlot, before) && before.file[3] == 0x0777u, "the reload is loading a piece");
    reset_slot(runtime, kStagingSlot, 0x09A7D340u);
    check(read_slot(memory, kStagingSlot, rows) && rows == before, "the pieces being loaded are untouched");
    reset_slot(runtime, 2u, 0x09A7D340u);
    check(read_slot(memory, 2u, rows) && rows.address == empty_rows(0x092114A0u).address, "another slot is the game's");

    // The reload's own reset is of the slot it loads into.
    reset_slot(runtime, kStagingSlot, kReloadResetReturn);
    check(read_slot(memory, kStagingSlot, rows) && rows.file[3] == 0xFFFFu && rows.address == loading.address,
        "the reload's own reset is of the free slot");

    run_loader(runtime, 0u);
    check(read_slot(memory, kStagingSlot, rows) && rows.address == other.address && rows.state[0] == 0u &&
            rows.file[0] == 0xFFFFu,
        "slot 1 is given back as the member's reset left it");
}

void test_announcement() {
    psprecomp::GuestMemory memory(kMemory);
    Announcement a;
    check(!start_announcement(memory, 0u, a), "no session, nothing to announce");
    memory.store8(kSession + kStatus, 2u);
    memory.store32(kSession + kSendCounter, 40u);
    check(start_announcement(memory, kSession, a) && a.running, "a status the others keep is announced");
    check(memory.load8(kSession + kStatus) == kAbsentStatus, "as status 1");
    check(memory.load32(kSession + kSendCounter) == 0xFFFFFFFFu, "sent at once");
    memory.store32(kSession + kSendCounter, 60u);
    for (std::uint32_t f = 1; f < kAbsentFrames; ++f) step_announcement(memory, kSession, a);
    check(a.running && memory.load8(kSession + kStatus) == kAbsentStatus, "for kAbsentFrames frames");
    step_announcement(memory, kSession, a);
    check(!a.running && memory.load8(kSession + kStatus) == 2u, "then the status it had");
    check(memory.load32(kSession + kSendCounter) == 0xFFFFFFFFu, "sent at once too");
    step_announcement(memory, kSession, a);
    check(memory.load8(kSession + kStatus) == 2u, "and nothing after");

    memory.store8(kSession + kStatus, 5u);
    check(!start_announcement(memory, kSession, a) && memory.load8(kSession + kStatus) == 5u,
        "a status 0x08898980 does not send is left alone");
    memory.store8(kSession + kStatus, 1u);
    check(!start_announcement(memory, kSession, a), "and one the others do not keep");

    memory.store8(kSession + kStatus, 3u);
    check(!start_announcement(memory, kSession, a) && memory.load8(kSession + kStatus) == 3u,
        "nor 3 or 4, which were not traced");

    memory.store8(kSession + kStatus, 2u);
    check(start_announcement(memory, kSession, a), "status 2 again");
    memory.store8(kSession + kStatus, 4u);
    for (std::uint32_t f = 0; f < kAbsentFrames; ++f) step_announcement(memory, kSession, a);
    check(!a.running && memory.load8(kSession + kStatus) == 4u, "a status the game set meanwhile stays");
}

constexpr std::uint32_t kGameSys = 0x09200000u;

// A hall of two: the other hunter's entry first, then this console's, whose
// hunter mirrors record 0.
std::uint32_t set_members(psprecomp::GuestMemory &memory) {
    memory.store32(kSessionPointer, kSession);
    memory.store8(kSession + kMemberCount, 2u);
    const std::uint8_t own[6] = {0x3Au, 0xECu, 0xC7u, 0xBFu, 0x2Eu, 0x34u};
    for (std::uint32_t b = 0; b < 6u; ++b) {
        memory.store8(kOwnAddress + b, own[b]);
        memory.store8(kSession + kMembers + b, b == 5u ? 0x35u : own[b]);
        memory.store8(kSession + kMembers + kMemberBytes + b, own[b]);
    }
    const std::uint32_t entry = kSession + kMembers + kMemberBytes;
    memory.store32(kGameSysPointer, kGameSys);
    for (std::uint32_t i = 0; i < kMirrorEnd; ++i) {
        const auto value = static_cast<std::uint8_t>(i * 7u + 3u);
        memory.store8(kGameSys + kOwnRecord + kRecordMirror + i, value);
        memory.store8(entry + kMemberHunter + i, value);
    }
    return entry;
}

void test_entry() {
    psprecomp::GuestMemory memory(kMemory);
    check(own_entry(memory, kSession) == 0u, "no members, no entry");
    const std::uint32_t entry = set_members(memory);
    check(own_entry(memory, kSession) == entry, "this console's entry is found by its address");
    check(own_entry(memory, 0u) == 0u, "not without a session");
    check(entry_differences(memory, entry) == 0, "the entry mirrors record 0");
    check(entry_differences(memory, 0u) == -1, "no entry, no comparison");
    // The weapon's first byte and an armor piece's, and a byte the loader skips.
    memory.store8(kGameSys + kOwnRecord + kRecordMirror + 0x20u, 0xEEu);
    memory.store8(kGameSys + kOwnRecord + kRecordMirror + 0x40u, 0xEEu);
    memory.store8(kGameSys + kOwnRecord + kRecordMirror + 0x1Au, 0xEEu);
    check(entry_differences(memory, entry) == 2, "the skipped bytes do not count");
    check(copy_record_to_entry(memory, entry), "the record is copied");
    check(entry_differences(memory, entry) == 0, "into the mirrored bytes");
    check(memory.load8(entry + kMemberHunter + 0x1Au) != 0xEEu, "and not into the skipped ones");
    check(!copy_record_to_entry(memory, 0u), "no entry, nothing copied");
}

void leave_equipment_screen(psprecomp::Runtime &runtime);

void test_entry_flow() {
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    auto &settings = mhfu::settings::current();
    settings.hall_equipment = true;
    write_signature(memory);
    set_slots(memory);
    install(runtime);
    const std::uint32_t entry = set_members(memory);
    memory.store8(kSession + kStatus, 2u);

    open_hall_box(memory);
    set_buttons(memory, kConfirm);
    run_box(runtime);
    check(flag_at_choice == 0u, "Change Equipment is chosen");
    // The equipment screen changes the weapon in the record.
    memory.store8(kGameSys + kOwnRecord + kRecordMirror + 0x20u, 0x55u);
    leave_equipment_screen(runtime);
    run_box(runtime);
    frame(runtime, &guest_call, &allocate);
    check(entry_differences(memory, entry) == 0, "the new weapon is in this console's entry");
    check(memory.load8(kSession + kStatus) == kAbsentStatus, "and the others are told to load it again");
    for (std::uint32_t f = 0; f < kAbsentFrames; ++f) frame(runtime, &guest_call, &allocate);

    // An entry that did not mirror the record is left as it was.
    memory.store8(entry + kMemberHunter + 0x30u, 0x77u);
    open_hall_box(memory);
    set_buttons(memory, kConfirm);
    run_box(runtime);
    memory.store8(kGameSys + kOwnRecord + kRecordMirror + 0x20u, 0x66u);
    leave_equipment_screen(runtime);
    run_box(runtime);
    frame(runtime, &guest_call, &allocate);
    check(memory.load8(entry + kMemberHunter + 0x20u) == 0x55u, "an entry that did not match is not written");
    for (std::uint32_t f = 0; f < kAbsentFrames; ++f) frame(runtime, &guest_call, &allocate);
}

void test_spare_slot() {
    psprecomp::GuestMemory memory(kMemory);
    check(spare_slot(memory) == -1, "no archive, no free slot");
    set_slots(memory);
    check(spare_slot(memory) == 3, "the last free slot");
    memory.store32(kFiles + kHunters + 12u, 0x09100000u);
    check(spare_slot(memory) == 2, "slot 3 has a hunter");
    memory.store32(kFiles + kHunters + 12u, 0u);
    memory.store32(kLoader + kLoaderRequestBytes * 2u, 1u);
    memory.store32(kLoader + kLoaderRequestBytes * 2u + 4u, 3u);
    check(spare_slot(memory) == 2, "slot 3 is about to be loaded");
    memory.store32(kLoader + kLoaderRequestBytes * 2u, 0u);
    SlotRows rows = empty_rows(0x0923A4A0u);
    rows.state[5] = 1u;
    write_slot(memory, 3u, rows);
    check(spare_slot(memory) == 2, "slot 3 is loading");
    rows = empty_rows(0x0923A4A0u);
    rows.room[0] = 0x1000u;
    write_slot(memory, 3u, rows);
    check(spare_slot(memory) == 2, "slot 3's part is smaller than this hunter's");
    rows = empty_rows(0x0923A4A0u);
    rows.address[4] = 0u;
    write_slot(memory, 3u, rows);
    memory.store32(kFiles + kHunters + 8u, 0x09100000u);
    check(spare_slot(memory) == -1, "a full hall has none");
}

void test_take_loaded() {
    const SlotRows own = rows_of(0x091BF4A0u, 0x0100u);
    SlotRows staged = empty_rows(0x0923A4A0u);
    staged.state[6] = 3u;
    staged.file[6] = 0x0999u;
    SlotRows mine = own, spare = empty_rows(0x0923A4A0u);
    take_loaded(staged, mine, spare);
    check(mine.address[6] == 0x0923A4A0u + 6u * 0x8000u && mine.file[6] == 0x0999u, "the loaded part is this hunter's");
    check(spare.address[6] == own.address[6] && spare.state[6] == 0u, "the free slot gets its old memory");
    for (std::uint32_t p = 0; p < kSlotParts; ++p)
        if (p != 6u)
            check(mine.address[p] == own.address[p] && spare.address[p] == 0x0923A4A0u + p * 0x8000u,
                "the parts not loaded stay where they were");
}

void test_wrappers() {
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    auto &settings = mhfu::settings::current();
    settings.hall_equipment = true;
    write_signature(memory);
    set_slots(memory);
    frame(runtime, &guest_call, &allocate);
    check(runtime.function_at(kBoxUpdate) == nullptr, "nothing is put in place before lobby_task is installed");
    check(memory.load16(kHallLabel) == kLabel, "on, the hall's label is the house's");

    install(runtime);
    check(runtime.function_at(kBoxUpdate) != &game_box, "the box's frame is wrapped");
    check(runtime.function_at(kMenuDraw) != &game_draw, "the menu's draw is wrapped");
    frame(runtime, &guest_call, &allocate);

    open_hall_box(memory);
    set_buttons(memory, kConfirm);
    box_calls = draw_calls = 0;
    run_box(runtime);
    check(box_calls == 1 && draw_calls == 1, "the game's frame and draw run once");
    check(flag_at_choice == 0u, "the choice is not refused");
    check(flag_at_draw == 1u, "the menu is drawn as the hall's");
    check(memory.load8(kBox + kHallFlag) == 1u, "the box stays the hall's");

    memory.store8(kBox + kCursor, 1u);
    run_box(runtime);
    check(flag_at_choice == 1u, "another entry keeps the hall's box");

    memory.store8(kBox + kCursor, kChangeEquipment);
    memory.store32(kFiles + kHunters + 8u, 0x09100000u);
    memory.store32(kFiles + kHunters + 12u, 0x09110000u);
    block_answer = 0u;
    run_box(runtime);
    block_answer = kBlock;
    check(flag_at_choice == 1u, "a full hall without memory for a reserve keeps the game's refusal");
    memory.store32(kFiles + kHunters + 8u, 0u);
    memory.store32(kFiles + kHunters + 12u, 0u);

    settings.hall_equipment = false;
    memory.store8(kBox + kCursor, kChangeEquipment);
    run_box(runtime);
    check(flag_at_choice == 1u, "off, the game refuses it");
    frame(runtime, &guest_call, &allocate);
    check(memory.load16(kHallLabel) == kGreyLabel, "off, the label is greyed again");

    memory.store16(kHallLabel, 0x0200u);
    settings.hall_equipment = true;
    frame(runtime, &guest_call, &allocate);
    check(memory.load16(kHallLabel) == 0x0200u, "a label that is neither text is left alone");

    // Item Sets' row before it moves Change Equipment's label to entry 4.
    const std::uint16_t with_sets[] = {300u, 301u, 302u, 0x7FF0u, kGreyLabel, 304u, 305u, 0u};
    for (std::uint32_t row = 0; row < kHallLabelRows; ++row) memory.store16(kHallLabels + 2u * row, with_sets[row]);
    frame(runtime, &guest_call, &allocate);
    check(memory.load16(kHallLabels + 8u) == kLabel && memory.load16(kHallLabels + 6u) == 0x7FF0u,
        "after Item Sets' row the label is found in entry 4, and Item Sets' row is left alone");
    settings.hall_equipment = false;
    frame(runtime, &guest_call, &allocate);
    check(memory.load16(kHallLabels + 8u) == kGreyLabel, "and greyed there again when off");
    settings.hall_equipment = true;
    memory.store16(kHallLabels + 8u, 304u);
    memory.store16(kHallLabel, 0x0200u);

    // lobby_task installed again registers its own functions.
    install(runtime);
    check(runtime.function_at(kBoxUpdate) != &game_box && runtime.function_at(kMenuDraw) != &game_draw,
        "wrapped again after lobby_task is installed again");
    memory.store16(kHallLabel, kGreyLabel);
    open_hall_box(memory);
    run_box(runtime);
    check(flag_at_choice == 0u && memory.load8(kBox + kHallFlag) == 1u, "and the choice goes through");
}

void test_readied() {
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    auto &settings = mhfu::settings::current();
    settings.hall_equipment = true;
    write_signature(memory);
    set_slots(memory);
    install(runtime);
    check(!readied(memory), "offline is never readied");
    memory.store32(kSessionPointer, kSession);
    memory.store8(kSession + kStatus, kHallStatus);
    check(!readied(memory), "standing in the hall is not readied");

    for (std::uint8_t status = kReadyFirst; status <= kReadyLast; ++status) {
        memory.store8(kSession + kStatus, status);
        check(readied(memory), "statuses 3 to 5 are readied");
        frame(runtime, &guest_call, &allocate);
        check(memory.load16(kHallLabel) == kGreyLabel, "readied, the label is greyed");
        open_hall_box(memory);
        set_buttons(memory, kConfirm);
        run_box(runtime);
        check(flag_at_choice == 1u, "readied, the game refuses Change Equipment");
    }

    memory.store8(kSession + kStatus, kHallStatus);
    frame(runtime, &guest_call, &allocate);
    check(memory.load16(kHallLabel) == kLabel, "back in the hall, the label is the house's");
    open_hall_box(memory);
    set_buttons(memory, kConfirm);
    run_box(runtime);
    check(flag_at_choice == 0u, "and Change Equipment goes through again");
    memory.store32(kSessionPointer, 0u);
}

// Leaves the hall's equipment screen for the top menu in one frame.
void leave_equipment_screen(psprecomp::Runtime &runtime) {
    open_hall_box(runtime.memory());
    runtime.memory().store8(kBox + kScreen, 1u);
    set_buttons(runtime.memory(), 0u);
    screen_after = 0;
    run_box(runtime);
    screen_after = -1;
}

void test_refresh() {
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    auto &settings = mhfu::settings::current();
    settings.hall_equipment = true;
    write_signature(memory);
    memory.store32(kSessionPointer, kSession);
    install(runtime);

    refresh_calls = box_calls = 0;
    memory.store8(kSession + kStatus, 2u);
    leave_equipment_screen(runtime);
    check(refresh_calls == 0 && box_calls == 1, "the frame that leaves the equipment screen is the game's");
    run_box(runtime);
    check(refresh_calls == 1, "the next frame refreshes this hunter's entry");
    check(refresh_argument == kSession, "with the session");
    check(box_calls == 2 && box_seen == kBox, "and then runs the box's frame for the same box");
    frame(runtime, &guest_call, &allocate);
    check(memory.load8(kSession + kStatus) == kAbsentStatus, "the flip after it starts the announcement");
    for (std::uint32_t f = 0; f < kAbsentFrames; ++f) frame(runtime, &guest_call, &allocate);
    check(memory.load8(kSession + kStatus) == 2u, "which ends with the status it had");
    run_box(runtime);
    check(refresh_calls == 1 && box_calls == 3, "once");

    memory.store8(kBox + kScreen, 1u);
    screen_after = 1;
    run_box(runtime);
    screen_after = -1;
    run_box(runtime);
    check(refresh_calls == 1, "staying in the equipment screen sends nothing");

    settings.hall_equipment = false;
    leave_equipment_screen(runtime);
    run_box(runtime);
    check(refresh_calls == 1, "off, nothing is sent");

    settings.hall_equipment = true;
    leave_equipment_screen(runtime);
    settings.hall_equipment = false;
    run_box(runtime);
    check(refresh_calls == 1, "turned off before the next frame, nothing is sent");
    settings.hall_equipment = true;
    run_box(runtime);
    check(refresh_calls == 1, "and the refresh is not kept for later");

    memory.store32(kSessionPointer, 0u);
    leave_equipment_screen(runtime);
    box_calls = 0;
    run_box(runtime);
    check(refresh_calls == 1 && box_calls == 1, "without a session the frame runs alone");
    memory.store32(kSessionPointer, kSession);

    leave_equipment_screen(runtime);
    memory.store8(kBox + kHallFlag, 0u);
    run_box(runtime);
    check(refresh_calls == 1, "a box that is no longer the hall's sends nothing");
}

void test_other_code() {
    psprecomp::Runtime runtime(kMemory);
    runtime.register_function(kBoxUpdate, &game_box, "other_code");
    runtime.register_function(kMenuDraw, &game_draw, "other_draw");
    runtime.memory().store16(kHallLabel, kGreyLabel);
    mhfu::settings::current().hall_equipment = true;
    frame(runtime, &guest_call, &allocate);
    check(runtime.function_at(kBoxUpdate) == &game_box, "code that does not match is not wrapped");
    check(runtime.memory().load16(kHallLabel) == kGreyLabel, "and its data is not written");
}

// On Freedom Unite Complete each word is at its counterpart, the box chooses
// with ○, and the box's frame is wrapped where Complete has it.
void test_complete() {
    using namespace mhfu::install;
    namespace layout = mhfu::game::layout;
    const auto at = [](std::uint32_t traced) { return port_from(traced, Image::lobby_task); };
    set_running_release(&kFreedomUniteComplete);
    psprecomp::Runtime runtime(kMemory);
    psprecomp::GuestMemory &memory = runtime.memory();
    check(!code_matches(memory), "empty memory is not Complete's lobby_task");
    for (const CodeWord &word : code_signature()) {
        check(at(word.address) != 0u && at(word.address) != word.address, "each word moves to Complete");
        memory.store32(at(word.address), word.address == layout::kBoxChoose ? 0x32022000u : word.word);
    }
    memory.store16(at(kHallLabel), kGreyLabel);
    check(code_matches(memory), "Complete's words, choosing with ○, are its lobby_task");
    memory.store32(at(code_signature()[18].address), 0x24020004u);
    check(!code_matches(memory), "one different word is other code there too");
    memory.store32(at(code_signature()[18].address), code_signature()[18].word);
    // FUComplete's Full Chest option: a jump out of the hall's call to its
    // box, and the word that passed the hall flag cleared.
    const std::uint32_t call = at(kHallBoxCall), argument = at(kHallFlagArgument);
    const std::uint32_t call_word = memory.load32(call), argument_word = memory.load32(argument);
    memory.store32(argument, 0u);
    check(!code_matches(memory), "the flag's word cleared without the jump is other code");
    memory.store32(call, 0x0A28D64Au); // j 0x08A35928
    check(code_matches(memory), "with FUComplete's jump the hall's box is still Complete's");
    memory.store32(argument, 0x24050002u);
    check(!code_matches(memory), "another flag is other code");
    memory.store32(call, call_word);
    memory.store32(argument, argument_word);

    memory.store32(at(kPadGate), kGate);
    memory.store8(kGate + kPadGateByte, 0u);
    memory.store32(at(kPadRepeat), 0u);
    memory.store32(at(kPadPressed), layout::kCircle);
    check(pressed(memory) == kConfirm, "Complete's ○ chooses");
    memory.store32(at(kPadPressed), layout::kCross);
    check(pressed(memory) == kCancel, "and its ✕ goes back");
    memory.store32(at(kSessionPointer), kSession);
    check(session(memory) == kSession, "the session from Complete's pointer");

    runtime.register_function(at(kBoxUpdate), &game_box, "complete_box");
    runtime.register_function(at(kMenuDraw), &game_draw, "complete_draw");
    runtime.register_function(at(kLoaderFrame), &game_loader, "complete_loader");
    runtime.register_function(at(kRequestOwnReload), &game_request, "complete_request");
    runtime.register_function(at(kSlotReset), &game_slot_reset, "complete_reset");
    mhfu::settings::current().hall_equipment = true;
    frame(runtime, &guest_call, &allocate);
    check(runtime.function_at(at(kBoxUpdate)) != &game_box, "Complete's box frame is wrapped");
    check(runtime.function_at(at(kMenuDraw)) != &game_draw, "and its menu's draw");
    check(runtime.function_at(at(kSlotReset)) != &game_slot_reset, "and its slot reset");
    check(runtime.function_at(kBoxUpdate) == nullptr, "nothing at the traced address");
    check(memory.load16(at(kHallLabel)) == kLabel, "Complete's label is the house's");
    set_running_release(nullptr);
}

} // namespace

int main() {
    const std::filesystem::path data = std::filesystem::temp_directory_path() / "pokee_hall_equipment_tests";
    std::filesystem::create_directories(data);
    set_environment("MHFU_DATA_DIR", data.string());
    set_environment("MHFU_HALL_EQUIPMENT", "");
    set_environment("MHFU_TRACE_HALL_EQUIPMENT", "");
    check(mhfu::settings::current().hall_equipment, "Change Equipment in the Guild Hall is on by default");
    test_pressed();
    test_refused_choice();
    test_session();
    test_code_check();
    test_other_code();
    test_wrappers();
    test_refresh();
    test_readied();
    test_slot_rows();
    test_announcement();
    test_entry();
    test_entry_flow();
    test_spare_slot();
    test_take_loaded();
    test_staging();
    test_slot_reset();
    test_full_hall();
    test_complete();
    std::error_code ec;
    std::filesystem::remove_all(data, ec);
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "hall equipment tests passed\n";
    return 0;
}
