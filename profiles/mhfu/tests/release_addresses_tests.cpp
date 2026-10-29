// Moving ULUS-10391's addresses to another release's (install/release_addresses.hpp).
// No game data: the ranges are made up, apart from a few of
// complete_addresses.inc's.
#include "game/guest_layout.hpp"
#include "install/game_identity.hpp"
#include "install/release_addresses.hpp"

#include <cstdint>
#include <iostream>
#include <map>

namespace {
int failures = 0;

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

using mhfu::install::AddressMap;
using mhfu::install::AddressRange;
using mhfu::install::Image;

// Words by address; anything else is not there.
struct Words {
    std::map<std::uint32_t, std::uint32_t> words;
    [[nodiscard]] bool contains(std::uint32_t address, std::uint32_t bytes) const {
        for (std::uint32_t at = address & ~3u; at < address + bytes; at += 4u)
            if (!words.contains(at)) return false;
        return true;
    }
    [[nodiscard]] std::uint32_t load32(std::uint32_t address) const {
        const auto found = words.find(address);
        return found != words.end() ? found->second : 0u;
    }
};

constexpr AddressRange kRanges[] = {
    {0x08810000u, 0x08810100u, 0x08810040u, Image::executable},
    {0x08810100u, 0x08810200u, 0x08811000u, Image::executable},
    {0x09A60000u, 0x09A60100u, 0x09A5B000u, Image::game_task},
    {0x09A60000u, 0x09A60080u, 0x09A5C000u, Image::lobby_task},
};

void moves_ranges() {
    const AddressMap map{kRanges};
    check(map.port(0x08810000u, Image::executable) == 0x08810040u, "a range's start moves");
    check(map.port(0x088100FCu, Image::executable) == 0x0881013Cu, "its last word moves by the same amount");
    check(map.port(0x08810100u, Image::executable) == 0x08811000u, "the next range moves by its own amount");
    check(map.port(0x08810200u, Image::executable) == 0u, "past the last range there is nothing");
    check(map.port(0x0880FFFCu, Image::executable) == 0u, "before the first there is nothing");
    check(map.port(0x08800010u, Image::executable) == 0x08800010u, "the host's memory below the game stays put");
    check(map.port(0x09A60040u, Image::game_task) == 0x09A5B040u, "game_task's ranges are its own");
    check(map.port(0x09A60040u, Image::lobby_task) == 0x09A5C040u, "lobby_task's are its own");
    check(map.port(0x09A60080u, Image::lobby_task) == 0u, "lobby_task's range ends where it ends");
    check(map.port(0x09A60040u, Image::game_sub) == 0u, "an image without ranges has nothing");
    check(map.port(0x08810000u, Image::game_task) == 0u, "the executable's ranges are not game_task's");
}

void moves_back() {
    const AddressMap map{kRanges};
    check(map.traced(0x08810040u, Image::executable) == 0x08810000u, "a moved start moves back");
    check(map.traced(0x0881013Cu, Image::executable) == 0x088100FCu, "and its last word");
    check(map.traced(0x08810140u, Image::executable) == 0u, "the gap after the first range is nothing traced");
    check(map.traced(0x08811004u, Image::executable) == 0x08810104u, "the second range moves back");
    check(map.traced(0x08810000u, Image::executable) == 0u, "below the first moved range is nothing");
    check(map.traced(0x09A5C010u, Image::lobby_task) == 0x09A60010u, "lobby_task moves back by its own amount");
    check(map.traced(0x09A5C010u, Image::game_task) == 0u, "and not by game_task's");
    check(map.traced(0x08801234u, Image::executable) == 0x08801234u, "the host's memory stays put");
}

void the_images_of_traced_addresses() {
    using mhfu::install::image_at;
    check(image_at(0x08886E50u, Image::game_task) == Image::executable, "below the slots is the executable");
    check(image_at(0x09AA4DD0u, Image::game_task) == Image::game_task, "the task slot is game_task from it");
    check(image_at(0x09AA4DD0u, Image::lobby_task) == Image::lobby_task, "and lobby_task from that");
    check(image_at(0x09AA4DD0u, Image::executable) == Image::game_task, "and game_task from the executable");
    check(image_at(0x09C3ABA8u, Image::executable) == Image::game_sub, "the sub slot is game_sub");
    check(image_at(0x09A289BCu, Image::lobby_task) == Image::executable,
        "below the task slot is the executable's, whatever reaches it");
}

// complete_addresses.inc is in order and its ranges do not overlap.
void the_table_is_sound() {
    const auto ranges = mhfu::install::complete_ranges();
    check(ranges.size() > 100u, "the table has ranges");
    bool sound = true;
    for (std::size_t i = 0; i < ranges.size(); ++i) {
        if (ranges[i].end <= ranges[i].start) sound = false;
        if (i > 0 && ranges[i].image == ranges[i - 1].image && ranges[i].start < ranges[i - 1].end) sound = false;
    }
    check(sound, "each range is not empty and starts after the one before it in its image");
}

void follows_the_running_release() {
    using namespace mhfu::install;
    set_running_release(nullptr);
    check(port(0x08886E50u) == 0x08886E50u, "with no release yet the traced address stays");
    check(traced_address(0x08886E50u) == 0x08886E50u, "and moves back to itself");
    check(running_map() == nullptr, "and there is no map");
    set_running_release(&kFreedomUniteUsa);
    check(port(0x08886E50u) == 0x08886E50u, "on ULUS-10391 the traced address stays");
    check(features_ported(), "and the features run");
    set_running_release(&kFreedomUniteComplete);
    check(running_map() != nullptr, "Freedom Unite Complete has a map");
    check(features_ported(), "so the features run there too");
    check(!traced_executable(), "though it is not the traced executable");
    // From complete_addresses.inc.
    check(port(0x08886E50u) == 0x08886B54u, "the camera update moves to Complete's");
    check(traced_address(0x08886B54u) == 0x08886E50u, "and back");
    check(port(0x09AA4DD0u, Image::game_task) == 0x09A9FFA0u, "the aim function moves within game_task");
    check(port(0x09C3ABA8u, Image::game_sub) == 0x09C35A98u, "the collision sweep moves within game_sub");
    check(port(0x089CBD04u) == 0x089C6CB4u, "the camera object's pointer moves with the data");
    namespace layout = mhfu::game::layout;
    check(layout::task_slot() == 0x09A5A580u, "the task slot is Complete's");
    check(layout::sub_slot() == 0x09C14280u, "the sub slot is Complete's");
    check(layout::em_overlay() == 0x09D15100u, "the em slot is Complete's");
    check(layout::em_overlay_end() == 0x09D5DF80u, "and ends at Complete's stage slot");
    set_running_release(nullptr);
}

// Words that differ only where an address is kept.
void compares_instructions() {
    using namespace mhfu::install;
    // jal 0x08815384 from 0x08812D98 (camera/field_of_view.cpp), and lui v0,0x089D.
    constexpr std::uint32_t kCall = 0x08812D98u;
    constexpr std::uint32_t kCallWord = 0x0E2054E1u;
    Words memory;
    set_running_release(&kFreedomUniteUsa);
    memory.words[kCall] = kCallWord;
    check(code_matches(memory, kCall, kCallWord), "on ULUS-10391 the same word matches");
    check(!same_instruction(memory, kCall, kCallWord + 1u, kCall, kCallWord, Image::executable),
        "on ULUS-10391 any other word does not");

    set_running_release(&kFreedomUniteComplete);
    const std::uint32_t at = port(kCall);
    const std::uint32_t target = port(0x08815384u);
    const std::uint32_t moved_call = 0x0C000000u | ((target >> 2) & 0x03FFFFFFu);
    check(same_instruction(memory, at, moved_call, kCall, kCallWord, Image::executable),
        "a call to the moved target matches");
    check(!same_instruction(memory, at, moved_call + 1u, kCall, kCallWord, Image::executable),
        "a call elsewhere does not");
    check(!same_instruction(memory, at, 0x08000000u | (moved_call & 0x03FFFFFFu), kCall, kCallWord, Image::executable),
        "a jump to it is not a call");
    memory.words[at] = moved_call;
    check(code_matches(memory, kCall, kCallWord), "code_matches reads the moved word");
    check(!code_matches(memory, 0x0880FFF0u, 0u), "a word outside memory does not match");

    constexpr std::uint32_t kHigh = 0x3C02089Du; // lui v0,0x089D
    check(same_instruction(memory, 0x08900000u, 0x3C02089Cu, 0x08900000u, kHigh, Image::executable),
        "an address's high half may differ");
    check(!same_instruction(memory, 0x08900000u, 0x3C0241F0u, 0x08900000u, kHigh, Image::executable),
        "but stays an address");
    check(!same_instruction(memory, 0x08900000u, 0x3C0241F1u, 0x08900000u, 0x3C0241F0u, Image::executable),
        "a constant's high half may not differ");
    check(
        !same_instruction(memory, 0x08900000u, 0x3C03089Cu, 0x08900000u, kHigh, Image::executable), "nor its register");

    constexpr std::uint32_t kLow = 0x8C50BD04u; // lw s0,-0x42FC(v0)
    Words near;
    near.words[0x08900000u] = kHigh;
    near.words[0x08900004u] = 0u;
    check(same_instruction(near, 0x08900008u, 0x8C506CB4u, 0x08900008u, kLow, Image::executable),
        "a low half may differ after a lui of its base");
    check(!same_instruction(near, 0x08900008u, 0x8C516CB4u, 0x08900008u, kLow, Image::executable),
        "but not its other register");
    Words far;
    far.words[0x08900000u] = 0x3C03089Du; // lui v1, another base
    far.words[0x08900004u] = 0u;
    check(!same_instruction(far, 0x08900008u, 0x8C506CB4u, 0x08900008u, kLow, Image::executable),
        "nor without a lui of its base");
    check(!same_instruction(near, 0x08900008u, 0x00021081u, 0x08900008u, 0x00021083u, Image::executable),
        "an instruction without an address must be the same");
    set_running_release(nullptr);
}

// Code the game hands back, and addresses a feature reaches from a task overlay.
void moves_code_back() {
    using namespace mhfu::install;
    set_running_release(&kFreedomUniteComplete);
    check(traced_code(0x08886B54u) == 0x08886E50u, "the executable's code moves back");
    check(traced_code(0x09A9FFA0u) == 0x09AA4DD0u, "the task slot's is game_task's");
    check(traced_code(0x09C35A98u) == 0x09C3ABA8u, "the sub slot's is game_sub's");
    check(traced_code(port(0x09ADCCD0u, Image::lobby_task), Image::lobby_task) == 0x09ADCCD0u,
        "and lobby_task's when asked");
    check(port_from(0x09C3ABA8u, Image::lobby_task) == 0x09C35A98u, "from lobby_task the sub slot is game_sub");
    check(port_from(0x09ADCCD0u, Image::lobby_task) == port(0x09ADCCD0u, Image::lobby_task),
        "and the task slot lobby_task");
    check(port_from(0x08886E50u, Image::lobby_task) == 0x08886B54u, "and the executable itself");
    check(port_from(0x09A289BCu, Image::lobby_task) == 0x09A245B8u, "and what it keeps below the task slot");
    set_running_release(&kFreedomUniteUsa);
    check(traced_code(0x09A9FFA0u) == 0x09A9FFA0u, "on ULUS-10391 code stays where it is");
    set_running_release(nullptr);
}

void moves_pointers_and_overlay_calls() {
    using namespace mhfu::install;
    set_running_release(&kFreedomUniteComplete);
    Words memory;
    // A pointer to the aim function, as a table of game_task would keep it.
    const std::uint32_t aim = port(0x09AA4DD0u, Image::game_task);
    check(same_instruction(memory, 0x09C10000u, aim, 0x09C10000u, 0x09AA4DD0u, Image::game_task),
        "a pointer is the moved pointer");
    check(!same_instruction(memory, 0x09C10000u, aim + 4u, 0x09C10000u, 0x09AA4DD0u, Image::game_task),
        "and not another");
    // From the executable, the task slot may hold lobby_task: jal 0x09A7B890 (game/hall_meal.hpp's kText).
    const std::uint32_t text = port(0x09A7B890u, Image::lobby_task);
    check(text != 0u && moves_to(0x09A7B890u, Image::executable, text), "a call into lobby_task moves with it");
    const std::uint32_t call = 0x0C000000u | ((text >> 2) & 0x03FFFFFFu);
    check(same_instruction(memory, 0x08836C4Cu, call, 0x08836C4Cu, 0x0E69EE24u, Image::executable),
        "so the executable's call to it matches");
    check(!moves_to(0x09A7B890u, Image::game_sub, text), "but not from game_sub");
    // code_matches finds a task-slot address in game_task.
    memory.words[aim] = 0x27BDFFE0u;
    check(code_matches(memory, 0x09AA4DD0u, 0x27BDFFE0u), "a task-slot address is game_task's by default");
    set_running_release(nullptr);
}

// The item box's choose and back buttons (game/guest_layout.hpp).
void swaps_the_box_buttons() {
    using namespace mhfu::install;
    namespace layout = mhfu::game::layout;
    check(layout::swap_circle_and_cross(layout::kCircle) == layout::kCross, "○ becomes ✕");
    check(layout::swap_circle_and_cross(layout::kCross | 0x10u) == (layout::kCircle | 0x10u),
        "✕ becomes ○ and the rest stays");
    check(layout::swap_circle_and_cross(layout::kCircle | layout::kCross) == (layout::kCircle | layout::kCross),
        "both stay both");

    constexpr std::uint32_t kChooseWord = 0x32024000u; // andi v0, s0, 0x4000
    Words memory;
    set_running_release(&kFreedomUniteUsa);
    memory.words[layout::kBoxChoose] = kChooseWord;
    check(!layout::box_chooses_with_circle(memory), "ULUS-10391 chooses with ✕");
    check(layout::as_traced_box_buttons(memory, layout::kCircle) == layout::kCircle, "so its buttons stay");

    set_running_release(&kFreedomUniteComplete);
    const std::uint32_t choose = port(layout::kBoxChoose, Image::lobby_task);
    memory.words[choose] = 0x32022000u; // andi v0, s0, 0x2000
    check(layout::box_chooses_with_circle(memory), "Complete chooses with ○");
    check(layout::as_traced_box_buttons(memory, layout::kCircle) == layout::kCross, "so its ○ is the traced ✕");
    check(layout::box_button_test(memory, layout::kBoxChoose, kChooseWord), "its test of choose matches");
    memory.words[choose] = 0x32021000u;
    check(!layout::box_button_test(memory, layout::kBoxChoose, kChooseWord), "a test of another bit does not");
    memory.words[choose] = 0x32032000u;
    check(!layout::box_button_test(memory, layout::kBoxChoose, kChooseWord), "nor of another register");
    check(!layout::box_button_test(memory, 0x09ADCE18u, kChooseWord), "nor a word that is no button test");
    set_running_release(nullptr);
}

} // namespace

int main() {
    moves_ranges();
    moves_back();
    the_images_of_traced_addresses();
    the_table_is_sound();
    follows_the_running_release();
    compares_instructions();
    moves_code_back();
    moves_pointers_and_overlay_calls();
    swaps_the_box_buttons();
    if (failures != 0) return 1;
    std::cout << "release address tests passed\n";
    return 0;
}
