// sceUtility dialogs: the on-screen keyboard and the message dialog. The
// save-data dialog is in hle_savedata.cpp. None of them draws anything yet;
// each answers the way a player confirming it would.
#include "hle_common.hpp"
#include "utility_dialog.hpp"

#if defined(MHP3RD_HAS_RENDERER)
#include "gpu/vulkan_renderer.hpp"
#endif

#include "psprecomp/common.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace mhp3rd {
namespace {

// Dialog status values shared by every sceUtility dialog.
constexpr std::uint32_t kStatusNone = 0u;
constexpr std::uint32_t kStatusInit = 1u;
constexpr std::uint32_t kStatusVisible = 2u;
constexpr std::uint32_t kStatusQuit = 3u;

// SceUtilityOskParams: 48-byte common header, then field count and field array.
constexpr std::uint32_t kOskFieldCountOffset = 48u;
constexpr std::uint32_t kOskFieldsOffset = 52u;
constexpr std::uint32_t kOskStateOffset = 56u;
// SceUtilityOskData offsets.
constexpr std::uint32_t kOskInitialTextOffset = 32u;
constexpr std::uint32_t kOskOutputLengthOffset = 36u;
constexpr std::uint32_t kOskOutputTextOffset = 40u;
constexpr std::uint32_t kOskResultOffset = 44u;
constexpr std::uint32_t kOskOutputLimitOffset = 48u;

constexpr std::uint32_t kOskResultCancelled = 1u;
constexpr std::uint32_t kOskResultChanged = 2u;

struct OskState {
    std::uint32_t params{};
    std::uint32_t status{kStatusNone};
    bool text_taken{};
};

OskState &osk() {
    static OskState state;
    return state;
}

std::string read_utf16(const psprecomp::GuestMemory &memory, std::uint32_t address, std::size_t max_units = 64u) {
    std::string text;
    if (address == 0u) return text;
    for (std::size_t i = 0; i < max_units; ++i) {
        const std::uint16_t unit = memory.load16(address + static_cast<std::uint32_t>(i) * 2u);
        if (unit == 0u) break;
        // The names this dialog collects are ASCII in practice.
        if (unit < 0x80u) text.push_back(static_cast<char>(unit));
    }
    return text;
}

void write_utf16(psprecomp::GuestMemory &memory, std::uint32_t address, const std::string &text,
                 std::uint32_t capacity_units) {
    if (address == 0u || capacity_units == 0u) return;
    const std::size_t count = std::min<std::size_t>(text.size(), capacity_units - 1u);
    for (std::size_t i = 0; i < count; ++i)
        memory.store16(address + static_cast<std::uint32_t>(i) * 2u, static_cast<std::uint16_t>(text[i]));
    memory.store16(address + static_cast<std::uint32_t>(count) * 2u, 0u);
}

// Host typing is opt-in; see the note in sceUtilityOskInitStart.
bool interactive_osk() {
    static const bool enabled = std::getenv("MHP3RD_OSK_INTERACTIVE") != nullptr;
    return enabled;
}

// Name the keyboard answers with when the host is not typing one.
std::string default_name() {
    const char *configured = std::getenv("MHP3RD_OSK_TEXT");
    return configured != nullptr ? configured : "Hunter";
}

std::uint32_t field_address(const psprecomp::GuestMemory &memory, std::uint32_t params) {
    if (params == 0u || memory.load32(params + kOskFieldCountOffset) == 0u) return 0u;
    return memory.load32(params + kOskFieldsOffset);
}

// Copies the entered text into the guest field and marks the dialog finished.
void finish_osk(psprecomp::GuestMemory &memory, const std::string &text, bool cancelled) {
    const std::uint32_t field = field_address(memory, osk().params);
    if (field != 0u) {
        const std::uint32_t capacity = memory.load32(field + kOskOutputLengthOffset);
        const std::uint32_t limit = memory.load32(field + kOskOutputLimitOffset);
        const std::uint32_t units = limit != 0u ? std::min(capacity, limit + 1u) : capacity;
        if (!cancelled) write_utf16(memory, memory.load32(field + kOskOutputTextOffset), text, units);
        memory.store32(field + kOskResultOffset, cancelled ? kOskResultCancelled : kOskResultChanged);
    }
    if (osk().params != 0u) memory.store32(osk().params + kOskStateOffset, kStatusQuit);
    osk().status = kStatusQuit;
    std::cout << "[osk] " << (cancelled ? "cancelled" : "entered \"" + text + "\"") << "\n";
}

void register_osk(HleRegistrar &hle) {
    hle.add("sceUtility", "sceUtilityOskInitStart", [](Runtime &rt, AllegrexContext &ctx) {
        osk().params = arg(ctx, 0);
        // The guest only polls a dialog it believes is on screen, so report it
        // visible from the start rather than waiting for its first update.
        osk().status = kStatusVisible;
        osk().text_taken = false;
        const std::uint32_t field = field_address(rt.memory(), osk().params);
        const std::string initial =
            field != 0u ? read_utf16(rt.memory(), rt.memory().load32(field + kOskInitialTextOffset)) : std::string{};
#if defined(MHP3RD_HAS_RENDERER)
        // Typing in the window means several frames with the guest drawing
        // nothing, because the real keyboard is drawn by system software we do
        // not have. Answering immediately keeps the screen alive, so host typing
        // is opt-in.
        if (interactive_osk()) {
            if (gpu::VulkanRenderer *renderer = active_renderer(); renderer != nullptr) {
                renderer->begin_text_input(initial);
                kernel().finish(ctx, 0u);
                return;
            }
        }
#endif
        finish_osk(rt.memory(), initial.empty() ? default_name() : initial, false);
        kernel().finish(ctx, 0u);
    });

    hle.add("sceUtility", "sceUtilityOskUpdate", [](Runtime &rt, AllegrexContext &ctx) {
        if (osk().status == kStatusInit) osk().status = kStatusVisible;
#if defined(MHP3RD_HAS_RENDERER)
        if (gpu::VulkanRenderer *renderer = active_renderer();
            renderer != nullptr && osk().status == kStatusVisible && !osk().text_taken) {
            if (renderer->text_input_confirmed() || renderer->text_input_cancelled()) {
                const bool cancelled = renderer->text_input_cancelled();
                const std::string text = renderer->text_input();
                renderer->end_text_input();
                osk().text_taken = true;
                finish_osk(rt.memory(), text, cancelled);
            }
        }
#endif
        kernel().finish(ctx, 0u);
    });

    hle.add("sceUtility", "sceUtilityOskGetStatus", [](Runtime &, AllegrexContext &ctx) {
        kernel().finish(ctx, osk().status);
    });

    hle.add("sceUtility", "sceUtilityOskShutdownStart", [](Runtime &rt, AllegrexContext &ctx) {
        if (osk().params != 0u) rt.memory().store32(osk().params + kOskStateOffset, kStatusNone);
        osk().status = kStatusNone;
        osk().params = 0u;
        kernel().finish(ctx, 0u);
    });
}

// SceUtilityMsgDialogParams, after the common dialog header.
namespace msg {
constexpr std::uint32_t kMode = 0x34u;         // 0: error code, 1: text
constexpr std::uint32_t kErrorValue = 0x38u;
constexpr std::uint32_t kMessage = 0x3Cu;      // char[512], UTF-8
constexpr std::uint32_t kOptions = 0x23Cu;
constexpr std::uint32_t kButtonPressed = 0x240u;
constexpr std::uint32_t kMinimumSize = 0x244u;

constexpr std::uint32_t kModeError = 0u;
constexpr std::uint32_t kOptionYesNo = 0x10u;
constexpr std::uint32_t kOptionDefaultNo = 0x100u;
constexpr std::uint32_t kPressedYes = 1u;
} // namespace msg

DialogLifecycle &msg_dialog() {
    static DialogLifecycle dialog;
    return dialog;
}

// With no dialog UI, every message is answered at once as if the player
// pressed confirm: "OK" for a notice, "Yes" for a question. The text is
// logged so the conversation can be followed.
void register_msg_dialog(HleRegistrar &hle) {
    hle.add("sceUtility", "sceUtilityMsgDialogInitStart", [](Runtime &rt, AllegrexContext &ctx) {
        auto &memory = rt.memory();
        const std::uint32_t params = arg(ctx, 0);
        const std::uint32_t size = memory.load32(params + dialog_common::kSizeOffset);
        const std::uint32_t mode = memory.load32(params + msg::kMode);
        const std::uint32_t options = size >= msg::kMinimumSize ? memory.load32(params + msg::kOptions) : 0u;
        if (mode == msg::kModeError) {
            std::cerr << "[msgdialog] error " << psprecomp::hex32(memory.load32(params + msg::kErrorValue)) << "\n";
        } else {
            std::cerr << "[msgdialog] \"" << read_cstring(memory, params + msg::kMessage, 512u) << "\""
                      << ((options & msg::kOptionYesNo) != 0u ? " [yes/no]" : "")
                      << ((options & msg::kOptionDefaultNo) != 0u ? " [default no]" : "") << " -> "
                      << ((options & msg::kOptionYesNo) != 0u ? "yes" : "ok") << "\n";
        }
        if (size >= msg::kMinimumSize) memory.store32(params + msg::kButtonPressed, msg::kPressedYes);
        memory.store32(params + dialog_common::kResultOffset, 0u);
        msg_dialog().start();
        kernel().finish(ctx, 0u);
    });

    hle.add("sceUtility", "sceUtilityMsgDialogUpdate", [](Runtime &, AllegrexContext &ctx) {
        (void)msg_dialog().poll();
        kernel().finish(ctx, 0u);
    });

    hle.add("sceUtility", "sceUtilityMsgDialogGetStatus", [](Runtime &, AllegrexContext &ctx) {
        kernel().finish(ctx, msg_dialog().poll());
    });

    hle.add("sceUtility", "sceUtilityMsgDialogShutdownStart", [](Runtime &, AllegrexContext &ctx) {
        if (!msg_dialog().active()) {
            kernel().finish(ctx, kErrorUtilityInvalidStatus);
            return;
        }
        if (!msg_dialog().shutdown()) std::cerr << "[msgdialog] ShutdownStart before the dialog finished\n";
        kernel().finish(ctx, 0u);
    });
}

} // namespace

void register_utility(HleRegistrar &hle, const std::filesystem::path &memory_stick) {
    register_osk(hle);
    register_msg_dialog(hle);
    register_savedata(hle, memory_stick);
}

} // namespace mhp3rd
