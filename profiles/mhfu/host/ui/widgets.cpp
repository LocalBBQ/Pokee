#include "ui/widgets.hpp"

#include "ui/layer.hpp"

#include "gpu/vulkan_renderer.hpp"

#include "imgui_internal.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>

namespace mhfu::ui {
namespace {

bool focus_next = false;

// Moves the gamepad and keyboard focus to the item just submitted when
// focus_next_row() asked for it. SetKeyboardFocusHere() would not do: it
// tabs to the next text field and starts editing it.
void take_focus() {
    if (!focus_next) return;
    focus_next = false;
    ImGui::FocusItem();
    ImGui::SetScrollHereY(0.3f);
}

float font() {
    return Layer::get().font_size();
}

// End of the visible part of a label: "##" starts the part that only makes
// the id unique, as everywhere in ImGui.
const char *shown_end(const char *label) {
    const char *hidden = std::strstr(label, "##");
    return hidden != nullptr ? hidden : label + std::strlen(label);
}
float px(float value) {
    return std::round(value * Layer::get().scale());
}

// Up or down on the keyboard, the D-pad or the left stick, with repeat.
int vertical_press() {
    int delta = 0;
    for (ImGuiKey key : {ImGuiKey_UpArrow, ImGuiKey_GamepadDpadUp, ImGuiKey_GamepadLStickUp})
        if (ImGui::IsKeyPressed(key, true)) delta = -1;
    for (ImGuiKey key : {ImGuiKey_DownArrow, ImGuiKey_GamepadDpadDown, ImGuiKey_GamepadLStickDown})
        if (ImGui::IsKeyPressed(key, true)) delta = 1;
    return delta;
}

// Left or right on the keyboard, the D-pad or the left stick, with repeat.
int horizontal_press() {
    int delta = 0;
    for (ImGuiKey key : {ImGuiKey_LeftArrow, ImGuiKey_GamepadDpadLeft, ImGuiKey_GamepadLStickLeft})
        if (ImGui::IsKeyPressed(key, true)) delta = -1;
    for (ImGuiKey key : {ImGuiKey_RightArrow, ImGuiKey_GamepadDpadRight, ImGuiKey_GamepadLStickRight})
        if (ImGui::IsKeyPressed(key, true)) delta = 1;
    return delta;
}

void draw_triangle(ImDrawList *draw, ImVec2 center, float size, bool right, ImU32 color) {
    const float h = size * 0.5f;
    if (right)
        draw->AddTriangleFilled({center.x - h * 0.8f, center.y - h}, {center.x - h * 0.8f, center.y + h},
            {center.x + h * 0.8f, center.y}, color);
    else
        draw->AddTriangleFilled({center.x + h * 0.8f, center.y - h}, {center.x + h * 0.8f, center.y + h},
            {center.x - h * 0.8f, center.y}, color);
}

// One pixel of the frame's pixel art, at the current text size.
float unit() {
    return std::max(2.0f, std::round(font() * 0.14f));
}

// The diamond lattice of the game's menu backgrounds: rows of dark diamonds
// that touch at their points, each with a fainter diamond inside, and a faint
// band along every row.
void draw_lattice(ImDrawList *draw, ImVec2 min, ImVec2 max) {
    const float cell = std::round(font() * 0.55f);
    const float h = cell * 0.5f;
    const float inner = h * 0.55f;
    const float band = std::max(1.0f, std::round(h * 0.12f));
    for (float y = min.y; y < max.y + h; y += cell) {
        draw->AddRectFilled({min.x, y - band}, {max.x, y + band}, colors::kLatticeLight);
        for (float x = min.x; x < max.x + h; x += cell) {
            draw->AddQuadFilled({x, y - h}, {x + h, y}, {x, y + h}, {x - h, y}, colors::kLatticeDark);
            draw->AddQuad({x, y - inner}, {x + inner, y}, {x, y + inner}, {x - inner, y}, colors::kLatticeLight,
                px(1.5f));
        }
    }
}

// The game's window frame: a square knob at each corner, a crenellated band
// along the top and bottom (dark teal outside, red teeth rising into teal,
// dark red inside), and a zig-zag chain of black blocks down the sides.
void draw_frame(ImDrawList *draw, ImVec2 min, ImVec2 max) {
    const float u = unit();
    const ImU32 ink = colors::kFrameInk;
    const ImU32 knob = colors::kFrameKnob;
    const float left = min.x + 6.0f * u;
    const float right = max.x - 6.0f * u;
    // Teeth two units wide with two-unit gaps, centred between the knobs.
    const int teeth = std::max(0, static_cast<int>((right - left) / (4.0f * u)));
    const float first = std::round(left + (right - left - (4.0f * teeth - 2.0f) * u) * 0.5f);
    for (const bool top : {true, false}) {
        // Row r of the band's six, counted from the outside edge.
        auto row = [&](int r, int rows, float x0, float x1, ImU32 color) {
            const float y0 = top ? min.y + r * u : max.y - (r + rows) * u;
            draw->AddRectFilled({x0, y0}, {x1, y0 + rows * u}, color);
        };
        row(0, 1, left, right, colors::kBandTealDark);
        row(1, 3, left, right, colors::kBandTeal);
        row(4, 1, left, right, colors::kBandRed);
        row(5, 1, left, right, colors::kBandRedDark);
        for (int i = 0; i < teeth; ++i) {
            const float x = first + i * 4.0f * u;
            row(2, 2, x, x + 2.0f * u, colors::kBandRed);
        }
    }
    for (const bool left_side : {true, false}) {
        int i = 0;
        for (float y = min.y + 5.0f * u; y < max.y - 5.0f * u; y += 2.0f * u, ++i) {
            const float in = i % 2 == 0 ? 1.0f : 2.0f; // units in from the edge
            const float x = left_side ? min.x + in * u : max.x - (in + 2.0f) * u;
            draw->AddRectFilled({x, y}, {x + 2.0f * u, std::min(y + 2.0f * u, max.y - u)}, ink);
        }
    }
    for (const ImVec2 corner : {min, ImVec2{max.x - 6.0f * u, min.y}, ImVec2{min.x, max.y - 6.0f * u},
             ImVec2{max.x - 6.0f * u, max.y - 6.0f * u}}) {
        draw->AddRectFilled(corner, {corner.x + 6.0f * u, corner.y + 6.0f * u}, ink);
        draw->AddRectFilled({corner.x + u, corner.y + u}, {corner.x + 5.0f * u, corner.y + 5.0f * u}, knob);
        draw->AddRectFilled({corner.x + 2.0f * u, corner.y + 2.0f * u}, {corner.x + 4.0f * u, corner.y + 4.0f * u}, ink);
    }
}

// A plain edge for small windows such as an open list: black outside, orange
// inside.
void draw_edge(ImDrawList *draw, ImVec2 min, ImVec2 max) {
    const float u = unit();
    draw->AddRect(min, max, colors::kFrameInk, 0.0f, 0, u * 2.0f);
    draw->AddRect({min.x + u, min.y + u}, {max.x - u, max.y - u}, colors::kEmber, 0.0f, 0, u);
}

struct Row {
    ImVec2 min;
    ImVec2 max;
    bool pressed{};
    bool focused{};
    bool hovered{};
};

// The part every row shares: a full-width focusable strip with its label.
Row row(const char *label, const RowOptions &options, ImU32 color = colors::kText, float height_lines = 1.0f) {
    const float height = std::round(font() * (0.9f + height_lines));
    Row result;
    result.min = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    result.max = {result.min.x + width, result.min.y + height};
    result.pressed = ImGui::InvisibleButton(label, {width, height}, ImGuiButtonFlags_EnableNav);
    take_focus();
    result.focused = ImGui::IsItemFocused();
    result.hovered = ImGui::IsItemHovered();

    ImDrawList *draw = ImGui::GetWindowDrawList();
    if (result.focused)
        draw_selection(draw, result.min, result.max, 235);
    else if (result.hovered)
        draw_selection(draw, result.min, result.max, 80);
    const float text_y = result.min.y + (height - font()) * 0.5f;
    const ImU32 ink = options.disabled ? colors::kTextDisabled : result.focused ? colors::kAccentBright : color;
    draw->AddText({result.min.x + px(16.0f), text_y}, ink, label, shown_end(label));
    if (result.focused || (result.hovered && Layer::get().description().empty())) {
        std::string description = options.description;
        if (!options.note.empty()) description += (description.empty() ? "" : "\n") + options.note;
        Layer::get().set_description(description);
    }
    return result;
}

// Draws `value` right-aligned in the row, with the note to its left.
float draw_value(const Row &r, const std::string &value, const RowOptions &options, float right_inset, ImU32 color) {
    ImDrawList *draw = ImGui::GetWindowDrawList();
    const float text_y = r.min.y + (r.max.y - r.min.y - font()) * 0.5f;
    const float value_width = ImGui::CalcTextSize(value.c_str()).x;
    const float x = r.max.x - right_inset - value_width;
    draw->AddText({x, text_y}, options.disabled ? colors::kTextDisabled : color, value.c_str());
    if (!options.note.empty() && options.disabled) {
        ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * 0.78f);
        const float note_width = ImGui::CalcTextSize(options.note.c_str()).x;
        draw->AddText({x - px(18.0f) - note_width, text_y + font() * 0.15f}, colors::kTextDim, options.note.c_str());
        ImGui::PopFont();
    }
    return x;
}

// A rounded label such as a shoulder button or a key.
float draw_cap(ImDrawList *draw, ImVec2 at, const char *text, bool filled) {
    const float height = font() * 1.15f;
    const ImVec2 text_size = ImGui::CalcTextSize(text);
    const float width = std::max(height, text_size.x + font() * 0.7f);
    const ImVec2 min{at.x, at.y + (font() - height) * 0.5f};
    const ImVec2 max{min.x + width, min.y + height};
    if (filled)
        draw->AddRectFilled(min, max, colors::kAccent, height * 0.3f);
    else
        draw->AddRect(min, max, colors::kTextDim, height * 0.3f, 0, px(1.5f));
    draw->AddText({min.x + (width - text_size.x) * 0.5f, min.y + (height - text_size.y) * 0.5f},
        filled ? colors::kPanel : colors::kText, text);
    return width;
}

// A face button as the pad in use labels it: a PlayStation shape or a letter.
float draw_face(ImDrawList *draw, ImVec2 at, SDL_GamepadButton button) {
    const float radius = font() * 0.62f;
    const ImVec2 center{at.x + radius, at.y + font() * 0.5f};
    draw->AddCircleFilled(center, radius, colors::kAccent);
    SDL_Gamepad *pad = Layer::get().attached() ? Layer::get().renderer().gamepad() : nullptr;
    SDL_GamepadButtonLabel label =
        pad != nullptr ? SDL_GetGamepadButtonLabel(pad, button) : SDL_GAMEPAD_BUTTON_LABEL_UNKNOWN;
    if (label == SDL_GAMEPAD_BUTTON_LABEL_UNKNOWN) {
        // Positional names of the common Xbox-style layout, as on a Steam Deck.
        switch (button) {
        case SDL_GAMEPAD_BUTTON_SOUTH:
            label = SDL_GAMEPAD_BUTTON_LABEL_A;
            break;
        case SDL_GAMEPAD_BUTTON_EAST:
            label = SDL_GAMEPAD_BUTTON_LABEL_B;
            break;
        case SDL_GAMEPAD_BUTTON_WEST:
            label = SDL_GAMEPAD_BUTTON_LABEL_X;
            break;
        default:
            label = SDL_GAMEPAD_BUTTON_LABEL_Y;
            break;
        }
    }
    const ImU32 ink = colors::kPanel;
    const float s = radius * 0.45f;
    const float thickness = std::max(1.5f, radius * 0.16f);
    switch (label) {
    case SDL_GAMEPAD_BUTTON_LABEL_CROSS:
        draw->AddLine({center.x - s, center.y - s}, {center.x + s, center.y + s}, ink, thickness);
        draw->AddLine({center.x - s, center.y + s}, {center.x + s, center.y - s}, ink, thickness);
        break;
    case SDL_GAMEPAD_BUTTON_LABEL_CIRCLE:
        draw->AddCircle(center, s * 1.1f, ink, 0, thickness);
        break;
    case SDL_GAMEPAD_BUTTON_LABEL_SQUARE:
        draw->AddRect({center.x - s, center.y - s}, {center.x + s, center.y + s}, ink, 0.0f, 0, thickness);
        break;
    case SDL_GAMEPAD_BUTTON_LABEL_TRIANGLE:
        draw->AddTriangle({center.x, center.y - s * 1.1f}, {center.x + s * 1.1f, center.y + s * 0.8f},
            {center.x - s * 1.1f, center.y + s * 0.8f}, ink, thickness);
        break;
    default: {
        const char *letter = label == SDL_GAMEPAD_BUTTON_LABEL_A ? "A"
            : label == SDL_GAMEPAD_BUTTON_LABEL_B                ? "B"
            : label == SDL_GAMEPAD_BUTTON_LABEL_X                ? "X"
                                                                 : "Y";
        const ImVec2 size = ImGui::CalcTextSize(letter);
        draw->AddText({center.x - size.x * 0.5f, center.y - size.y * 0.5f}, ink, letter);
        break;
    }
    }
    return radius * 2.0f;
}

const char *shoulder_name(bool right) {
    SDL_Gamepad *pad = Layer::get().attached() ? Layer::get().renderer().gamepad() : nullptr;
    const SDL_GamepadType type = pad != nullptr ? SDL_GetGamepadType(pad) : SDL_GAMEPAD_TYPE_UNKNOWN;
    const char *name = pad != nullptr ? SDL_GetGamepadName(pad) : nullptr;
    const bool playstation =
        type == SDL_GAMEPAD_TYPE_PS3 || type == SDL_GAMEPAD_TYPE_PS4 || type == SDL_GAMEPAD_TYPE_PS5;
    // The Steam Deck prints L1 and R1 on its bumpers.
    const bool deck = name != nullptr && std::strstr(name, "Steam Deck") != nullptr;
    if (playstation || deck) return right ? "R1" : "L1";
    if (type == SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO || type == SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR)
        return right ? "R" : "L";
    return right ? "RB" : "LB";
}

// The button left of the pad's centre, as the pad in use names it.
const char *select_name() {
    SDL_Gamepad *pad = Layer::get().attached() ? Layer::get().renderer().gamepad() : nullptr;
    switch (pad != nullptr ? SDL_GetGamepadType(pad) : SDL_GAMEPAD_TYPE_UNKNOWN) {
    case SDL_GAMEPAD_TYPE_PS3:
        return "Select";
    case SDL_GAMEPAD_TYPE_PS4:
        return "Share";
    case SDL_GAMEPAD_TYPE_PS5:
        return "Create";
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
        return "−";
    default:
        return "View";
    }
}

} // namespace

void draw_selection(ImDrawList *draw, ImVec2 min, ImVec2 max, int alpha) {
    draw->AddRectFilled(min, max, colors::with_alpha(colors::kSelect, alpha));
}

ImGuiStyle make_style(float scale, float font_size) {
    ImGuiStyle style;
    ImGui::StyleColorsDark(&style);
    // Square, as the game's own windows are.
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.WindowBorderSize = 1.5f;
    style.PopupBorderSize = 1.5f;
    style.WindowPadding = {28.0f, 26.0f};
    style.FramePadding = {12.0f, 7.0f};
    style.ItemSpacing = {10.0f, 4.0f};
    style.ScrollbarSize = 10.0f;
    style.ScrollbarRounding = 0.0f;
    style.ScaleAllSizes(scale);
    style.FontSizeBase = font_size;

    ImVec4 *c = style.Colors;
    const auto rgba = [](ImU32 color) { return ImGui::ColorConvertU32ToFloat4(color); };
    c[ImGuiCol_Text] = rgba(colors::kText);
    c[ImGuiCol_TextDisabled] = rgba(colors::kTextDisabled);
    c[ImGuiCol_WindowBg] = rgba(colors::kPanel);
    c[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_PopupBg] = rgba(colors::kPopup);
    c[ImGuiCol_ModalWindowDimBg] = rgba(colors::kShade);
    c[ImGuiCol_Border] = rgba(colors::kPanelEdge);
    c[ImGuiCol_FrameBg] = rgba(colors::with_alpha(colors::kText, 18));
    c[ImGuiCol_FrameBgHovered] = rgba(colors::kRowHover);
    c[ImGuiCol_FrameBgActive] = rgba(colors::kRowFocus);
    c[ImGuiCol_TextSelectedBg] = rgba(colors::with_alpha(colors::kAccent, 110));
    c[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_ScrollbarGrab] = rgba(colors::with_alpha(colors::kPanelEdge, 160));
    c[ImGuiCol_ScrollbarGrabHovered] = rgba(colors::with_alpha(colors::kAccent, 200));
    c[ImGuiCol_ScrollbarGrabActive] = rgba(colors::kAccent);
    c[ImGuiCol_Separator] = rgba(colors::with_alpha(colors::kPanelEdge, 120));
    // Rows draw their own focus; ImGui's rectangle would double it.
    c[ImGuiCol_NavCursor] = ImVec4(0, 0, 0, 0);
    return style;
}

void begin_panel(const char *id, const std::string &title, const std::string &subtitle, bool dim_game) {
    const ImGuiIO &io = ImGui::GetIO();
    if (dim_game) ImGui::GetBackgroundDrawList()->AddRectFilled({0, 0}, io.DisplaySize, colors::kBackdrop);
    const float margin = std::round(std::min(io.DisplaySize.x, io.DisplaySize.y) * 0.03f);
    const ImVec2 size{std::min(io.DisplaySize.x - 2.0f * margin, font() * 46.0f),
        std::min(io.DisplaySize.y - 2.0f * margin, font() * 31.0f)};
    ImGui::SetNextWindowPos({io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f}, ImGuiCond_Always, {0.5f, 0.5f});
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);
    // The frame below replaces ImGui's border, and the background is drawn
    // inside it so that nothing shows past the frame.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::Begin(id, nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImDrawList *draw = ImGui::GetWindowDrawList();
    const ImVec2 window_min = ImGui::GetWindowPos();
    const ImVec2 window_max{window_min.x + ImGui::GetWindowSize().x, window_min.y + ImGui::GetWindowSize().y};
    const float inset = unit() * 3.0f;
    const ImVec2 inside_min{window_min.x + inset, window_min.y + inset};
    const ImVec2 inside_max{window_max.x - inset, window_max.y - inset};
    draw->PushClipRect(window_min, window_max);
    draw->AddRectFilled(inside_min, inside_max, colors::kPanel);
    draw->PushClipRect(inside_min, inside_max, true);
    draw_lattice(draw, inside_min, inside_max);
    draw->PopClipRect();
    draw_frame(draw, window_min, window_max);
    draw->PopClipRect();

    ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * 1.45f);
    const ImVec2 title_at = ImGui::GetCursorScreenPos();
    ImGui::PushStyleColor(ImGuiCol_Text, colors::kText);
    ImGui::TextUnformatted(title.c_str());
    ImGui::PopStyleColor();
    const float title_height = ImGui::GetItemRectSize().y;
    ImGui::PopFont();
    if (!subtitle.empty()) {
        const float width = ImGui::CalcTextSize(subtitle.c_str()).x;
        const float right = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x;
        draw->AddText({right - width, title_at.y + title_height - font() * 1.15f}, colors::kTextDim, subtitle.c_str());
    }
    const ImVec2 line = ImGui::GetCursorScreenPos();
    const float line_width = ImGui::GetContentRegionAvail().x;
    const ImU32 fade = colors::with_alpha(colors::kEmber, 0);
    draw->AddRectFilledMultiColor(line, {line.x + line_width, line.y + px(2.0f)}, colors::kEmber, fade, fade,
        colors::kEmber);
    ImGui::Dummy({0.0f, px(10.0f)});
}

void begin_content() {
    const float footer = std::round(font() * 4.3f);
    const float content = std::max(font() * 3.0f, ImGui::GetContentRegionAvail().y - footer);
    ImGui::BeginChild("content", {0.0f, content}, ImGuiChildFlags_NavFlattened);
}

void touch_scroll() {
    ImGuiIO &io = ImGui::GetIO();
    if (io.MouseSource != ImGuiMouseSource_TouchScreen) return;
    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        return;
    if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left, font() * 0.4f)) return;
    ImGui::SetScrollY(ImGui::GetScrollY() - io.MouseDelta.y);
    // A drag is a scroll, not a press: the row the finger started on must not
    // act when it lifts.
    ImGui::ClearActiveID();
}

void begin_footer() {
    touch_scroll();
    ImGui::EndChild();
    ImDrawList *draw = ImGui::GetWindowDrawList();
    const ImVec2 line = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    draw->AddLine({line.x, line.y + px(4.0f)}, {line.x + width, line.y + px(4.0f)},
        colors::with_alpha(colors::kPanelEdge, 110), px(1.0f));
    ImGui::Dummy({0.0f, px(10.0f)});
    // Two lines for the description, whatever it holds, so the hints stay put.
    const float description_height = font() * 2.4f;
    const ImVec2 at = ImGui::GetCursorScreenPos();
    // A description that would wrap past its two lines (a narrow or a very
    // wide window) is set smaller rather than run into the hints below.
    const std::string &description = Layer::get().description();
    float size = ImGui::GetStyle().FontSizeBase * 0.88f;
    const float smallest = ImGui::GetStyle().FontSizeBase * 0.66f;
    while (size > smallest &&
        ImGui::GetFont()->CalcTextSizeA(size, FLT_MAX, width, description.c_str()).y > description_height)
        size -= 1.0f;
    ImGui::PushFont(nullptr, size);
    ImGui::PushStyleColor(ImGuiCol_Text, colors::kTextDim);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + width);
    ImGui::TextUnformatted(description.c_str());
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::SetCursorScreenPos({at.x, at.y + description_height});
}

void end_panel() {
    ImGui::End();
}

bool tab_bar(const char *const *labels, int count, int &selected) {
    const int before = selected;
    const bool typing = ImGui::GetIO().WantTextInput;
    if (ImGui::IsKeyPressed(ImGuiKey_GamepadL1, false) || (!typing && ImGui::IsKeyPressed(ImGuiKey_Q, false)))
        selected = (selected + count - 1) % count;
    if (ImGui::IsKeyPressed(ImGuiKey_GamepadR1, false) || (!typing && ImGui::IsKeyPressed(ImGuiKey_W, false)))
        selected = (selected + 1) % count;

    ImDrawList *draw = ImGui::GetWindowDrawList();
    const bool pad = Layer::get().input_device() == InputDevice::Gamepad;
    const float height = font() * 1.9f;
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    // The switch buttons at both ends.
    const float cap_left =
        draw_cap(draw, {start.x, start.y + (height - font()) * 0.5f}, pad ? shoulder_name(false) : "Q", false);
    const float cap_right_width = ImGui::CalcTextSize(pad ? shoulder_name(true) : "W").x + font() * 0.7f;
    draw_cap(draw, {start.x + width - std::max(font() * 1.15f, cap_right_width), start.y + (height - font()) * 0.5f},
        pad ? shoulder_name(true) : "W", false);

    const float inner_left = start.x + cap_left + px(14.0f);
    const float inner_width = width - cap_left - std::max(font() * 1.15f, cap_right_width) - px(28.0f);
    const float tab_width = inner_width / static_cast<float>(count);
    ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
    for (int i = 0; i < count; ++i) {
        const ImVec2 min{inner_left + tab_width * static_cast<float>(i), start.y};
        const ImVec2 max{min.x + tab_width, start.y + height};
        ImGui::SetCursorScreenPos(min);
        ImGui::PushID(i);
        if (ImGui::InvisibleButton("tab", {tab_width, height})) selected = i;
        const bool hovered = ImGui::IsItemHovered();
        ImGui::PopID();
        const bool active = i == selected;
        if (active)
            draw_selection(draw, min, max, 235);
        else if (hovered)
            draw_selection(draw, min, max, 80);
        const ImVec2 size = ImGui::CalcTextSize(labels[i]);
        draw->AddText({min.x + (tab_width - size.x) * 0.5f, min.y + (height - size.y) * 0.5f},
            active ? colors::kAccentBright : colors::kTextDim, labels[i]);
        if (active)
            draw->AddRectFilled({min.x, max.y - unit()}, {max.x, max.y}, colors::kEmber);
    }
    ImGui::PopItemFlag();
    ImGui::SetCursorScreenPos({start.x, start.y + height + px(8.0f)});
    ImGui::Dummy({0.0f, 0.0f});
    return selected != before;
}

void focus_next_row() {
    focus_next = true;
}

namespace {

bool dropdown_swallowed = false;

struct Dropdown {
    bool open{};
    bool just_opened{};
    int highlight{};
};

std::map<std::string, Dropdown> &dropdowns() {
    static std::map<std::string, Dropdown> states;
    return states;
}

} // namespace

void dropdown_new_frame() {
    dropdown_swallowed = false;
}

void dismiss_dropdowns() {
    for (auto &entry : dropdowns()) entry.second.open = entry.second.just_opened = false;
}

bool dropdown_swallowed_back() {
    return dropdown_swallowed;
}

bool dropdown_row(const char *label, const char *const *items, int count, int &selected, const RowOptions &options,
    bool back) {
    const Row r = row(label, options);
    selected = count > 0 ? std::clamp(selected, 0, count - 1) : 0;
    const char *value = count > 0 ? items[selected] : "";
    const bool live = (r.focused || r.hovered) && !options.disabled;
    const float chevron = font() * 0.55f;
    draw_value(r, value, options, px(16.0f) + chevron + font() * 0.35f, live ? colors::kAccentBright : colors::kText);
    ImDrawList *draw = ImGui::GetWindowDrawList();
    if (!options.disabled) {
        const float mid_y = (r.min.y + r.max.y) * 0.5f;
        const float x = r.max.x - px(16.0f) - chevron * 0.35f;
        const ImU32 ink = live ? colors::kAccentBright : colors::kTextDim;
        draw->AddTriangleFilled({x - chevron * 0.45f, mid_y - chevron * 0.2f},
            {x + chevron * 0.45f, mid_y - chevron * 0.2f}, {x, mid_y + chevron * 0.4f}, ink);
    }

    Dropdown &state = dropdowns()[label];
    const bool was_open = state.open;
    if (options.disabled) state.open = false;
    // A click on the row opens the list, and another click closes it. Confirm
    // on a keyboard or a gamepad chooses the highlighted entry instead: the
    // pad's own movement never reaches the list through the page behind it.
    const bool mouse_released = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
    bool changed = false;
    if (state.open && back) {
        state.open = false;
        dropdown_swallowed = true;
    } else if (!options.disabled && r.pressed) {
        if (!state.open) {
            state.open = true;
            state.just_opened = true;
            state.highlight = selected;
        } else if (mouse_released) {
            state.open = false;
        } else if (!state.just_opened) {
            changed = state.highlight != selected;
            selected = state.highlight;
            state.open = false;
        }
    }
    if (state.open && count > 0) {
        const int delta = vertical_press();
        if (delta != 0) state.highlight = (state.highlight + delta + count) % count;
    }
    if (!state.open || count <= 0) {
        state.just_opened = false;
        if (was_open) ImGui::NavMoveRequestCancel();
        return changed;
    }

    float widest = 0.0f;
    for (int i = 0; i < count; ++i) widest = std::max(widest, ImGui::CalcTextSize(items[i]).x);
    const float item_height = std::round(font() * 1.7f);
    const float pad = px(12.0f);
    // Room inside the edge on every side.
    const float edge = unit() * 3.0f;
    const float popup_width = widest + pad * 2.0f + edge * 2.0f;
    const float popup_height = item_height * static_cast<float>(count) + edge * 2.0f;
    const ImVec2 popup_max{r.max.x - px(8.0f), r.max.y + px(4.0f) + popup_height};
    ImVec2 popup_min{popup_max.x - popup_width, r.max.y + px(4.0f)};
    const float screen_bottom = ImGui::GetIO().DisplaySize.y - px(8.0f);
    if (popup_max.y > screen_bottom) {
        const float lift = popup_max.y - screen_bottom;
        popup_min.y -= lift;
    }

    const std::string window_id = std::string("##dropdown/") + label;
    ImGui::SetNextWindowPos(popup_min);
    ImGui::SetNextWindowSize({popup_width, popup_height});
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::ColorConvertU32ToFloat4(colors::kPopup));
    ImGui::PushStyleColor(ImGuiCol_Border, ImGui::ColorConvertU32ToFloat4(colors::kPanelEdge));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {edge, edge});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    // The list must not take focus. A gamepad move would otherwise land on
    // the first entry, or on the page behind the list, instead of the highlight.
    const bool begun = ImGui::Begin(window_id.c_str(), nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing);
    if (begun) {
        ImDrawList *frame_draw = ImGui::GetWindowDrawList();
        const ImVec2 window_min = ImGui::GetWindowPos();
        const ImVec2 window_max{window_min.x + popup_width, window_min.y + popup_height};
        frame_draw->PushClipRect(window_min, window_max);
        draw_lattice(frame_draw, window_min, window_max);
        draw_edge(frame_draw, window_min, window_max);
        frame_draw->PopClipRect();
        // The click that opened the list is still on the row, so it must not
        // choose an entry or dismiss the list on that same frame.
        if (!state.just_opened && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsWindowHovered())
            state.open = false;
        for (int i = 0; i < count && state.open; ++i) {
            const ImVec2 item_size{popup_width - edge * 2.0f, item_height};
            ImGui::PushID(i);
            const bool pressed = ImGui::InvisibleButton(items[i], item_size);
            // The nav cursor reports the focused row as hovered. Only a real
            // mouse position should move the highlight.
            const bool item_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_NoNavOverride);
            ImGui::PopID();
            if (item_hovered) state.highlight = i;
            const ImVec2 item_min = ImGui::GetItemRectMin();
            const ImVec2 item_max = ImGui::GetItemRectMax();
            ImDrawList *popup_draw = ImGui::GetWindowDrawList();
            if (i == state.highlight)
                draw_selection(popup_draw, item_min, item_max, 235);
            else if (item_hovered)
                draw_selection(popup_draw, item_min, item_max, 80);
            popup_draw->AddText({item_min.x + pad, item_min.y + (item_height - font()) * 0.5f},
                i == state.highlight ? colors::kAccentBright : colors::kText, items[i]);
            if (pressed && !state.just_opened) {
                const int choice = mouse_released ? i : state.highlight;
                changed = choice != selected;
                selected = choice;
                state.open = false;
            }
        }
    }
    ImGui::End();
    // The page behind the list must not take the pad's up and down.
    if (state.open) ImGui::NavMoveRequestCancel();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
    state.just_opened = false;
    return changed;
}

bool disclosure_row(const char *label, bool &open, const RowOptions &options) {
    const Row r = row(label, options);
    const bool toggled = !options.disabled && r.pressed;
    if (toggled) open = !open;
    if (options.disabled) return false;
    const bool live = r.focused || r.hovered;
    const float chevron = font() * 0.55f;
    const float mid_y = (r.min.y + r.max.y) * 0.5f;
    const float x = r.max.x - px(16.0f) - chevron * 0.35f;
    const ImU32 ink = live ? colors::kAccentBright : colors::kTextDim;
    ImDrawList *draw = ImGui::GetWindowDrawList();
    if (open) {
        draw->AddTriangleFilled({x - chevron * 0.45f, mid_y - chevron * 0.2f},
            {x + chevron * 0.45f, mid_y - chevron * 0.2f}, {x, mid_y + chevron * 0.4f}, ink);
    } else {
        draw->AddTriangleFilled({x - chevron * 0.2f, mid_y - chevron * 0.45f},
            {x - chevron * 0.2f, mid_y + chevron * 0.45f}, {x + chevron * 0.4f, mid_y}, ink);
    }
    return toggled;
}

int choice_row(const char *label, const std::string &value, const RowOptions &options) {
    const Row r = row(label, options);
    const float mid_y = (r.min.y + r.max.y) * 0.5f;
    const float right_arrow_x = r.max.x - px(16.0f) - font() * 0.45f;
    const float value_x = r.max.x - px(16.0f) - font() * 1.1f - ImGui::CalcTextSize(value.c_str()).x;
    const float left_arrow_x = value_x - font() * 0.65f;
    int delta = r.focused ? horizontal_press() : 0;
    if (r.pressed) {
        // A click on the left arrow steps back; anything else steps forward.
        const float mouse_x = ImGui::GetIO().MousePos.x;
        const bool on_left_arrow = ImGui::GetIO().MouseReleased[0] && mouse_x > left_arrow_x - font() &&
            mouse_x < left_arrow_x + font() * 0.6f;
        delta = on_left_arrow ? -1 : 1;
    }
    const bool live = r.focused || r.hovered;
    ImDrawList *draw = ImGui::GetWindowDrawList();
    draw_value(r, value, options, px(16.0f) + font() * 1.1f, live ? colors::kAccentBright : colors::kText);
    // A locked row shows its value without the arrows that would change it.
    if (options.disabled) return 0;
    const ImU32 arrow = live ? colors::kAccentBright : colors::kTextDim;
    draw_triangle(draw, {right_arrow_x, mid_y}, font() * 0.55f, true, arrow);
    draw_triangle(draw, {left_arrow_x, mid_y}, font() * 0.55f, false, arrow);
    return delta;
}

bool toggle_row(const char *label, bool value, const RowOptions &options) {
    const Row r = row(label, options);
    const int delta = r.focused ? horizontal_press() : 0;
    const bool toggled = !options.disabled && (r.pressed || (delta > 0 && !value) || (delta < 0 && value));

    ImDrawList *draw = ImGui::GetWindowDrawList();
    const float height = font() * 0.95f;
    const float width = height * 1.9f;
    const float mid_y = (r.min.y + r.max.y) * 0.5f;
    const ImVec2 max{r.max.x - px(16.0f), mid_y + height * 0.5f};
    const ImVec2 min{max.x - width, mid_y - height * 0.5f};
    const bool on = toggled ? !value : value;
    const ImU32 track = options.disabled ? colors::kTrack : on ? colors::kAccent : colors::kTrack;
    // Square and outlined in black, so it reads on the green selection too.
    draw->AddRectFilled(min, max, colors::with_alpha(colors::kFrameInk, 150));
    draw->AddRectFilled(min, max, track);
    draw->AddRect(min, max, colors::kFrameInk, 0.0f, 0, unit());
    const float knob = height * 0.5f - unit() * 1.5f;
    const ImVec2 knob_center{on ? max.x - height * 0.5f : min.x + height * 0.5f, mid_y};
    draw->AddRectFilled({knob_center.x - knob, knob_center.y - knob}, {knob_center.x + knob, knob_center.y + knob},
        options.disabled ? colors::kTextDisabled : colors::kText);
    draw_value(
        r, on ? "On" : "Off", options, px(16.0f) + width + px(12.0f), on ? colors::kAccentBright : colors::kTextDim);
    return toggled;
}

bool slider_row(const char *label, int &value, int minimum, int maximum, int step, const char *format,
    const RowOptions &options, std::optional<int> mark) {
    const Row r = row(label, options);
    const int before = value;
    ImDrawList *draw = ImGui::GetWindowDrawList();
    const float mid_y = (r.min.y + r.max.y) * 0.5f;
    const float bar_width = std::min(font() * 10.0f, (r.max.x - r.min.x) * 0.38f);
    const ImVec2 bar_max{r.max.x - px(16.0f), mid_y + px(3.0f)};
    const ImVec2 bar_min{bar_max.x - bar_width, mid_y - px(3.0f)};

    if (!options.disabled) {
        if (r.focused) value += horizontal_press() * step;
        // Dragging along the bar sets the value directly.
        static ImGuiID dragging = 0;
        const ImGuiID id = ImGui::GetItemID();
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        if (ImGui::IsItemActivated() && mouse.x >= bar_min.x - font() && mouse.x <= bar_max.x + font()) dragging = id;
        if (dragging == id) {
            if (ImGui::IsMouseDown(0)) {
                const float t = std::clamp((mouse.x - bar_min.x) / bar_width, 0.0f, 1.0f);
                const float raw = static_cast<float>(minimum) + t * static_cast<float>(maximum - minimum);
                value = minimum + static_cast<int>(std::round((raw - static_cast<float>(minimum)) / step)) * step;
            } else {
                dragging = 0;
            }
        }
        value = std::clamp(value, minimum, maximum);
    }

    const float t = static_cast<float>(value - minimum) / static_cast<float>(std::max(1, maximum - minimum));
    const bool live = (r.focused || r.hovered) && !options.disabled;
    draw->AddRectFilled(bar_min, bar_max, colors::with_alpha(colors::kFrameInk, 150));
    draw->AddRectFilled(bar_min, {bar_min.x + bar_width * t, bar_max.y},
        options.disabled ? colors::kTextDisabled : colors::kAccent);
    draw->AddRect(bar_min, bar_max, colors::kFrameInk, 0.0f, 0, unit() * 0.75f);
    // The mark stands above and below the bar, outlined in black so it reads
    // on the filled part too, and its ends still show past the knob.
    if (mark && *mark >= minimum && *mark <= maximum) {
        const float mark_t = static_cast<float>(*mark - minimum) / static_cast<float>(std::max(1, maximum - minimum));
        const float x = std::round(bar_min.x + bar_width * mark_t);
        const float half = std::round(unit() * 0.5f);
        const float reach = font() * 0.5f;
        const float ink = std::round(unit() * 0.5f);
        draw->AddRectFilled(
            {x - half - ink, mid_y - reach - ink}, {x + half + ink, mid_y + reach + ink}, colors::kFrameInk);
        draw->AddRectFilled({x - half, mid_y - reach}, {x + half, mid_y + reach},
            options.disabled ? colors::kTextDisabled : colors::kTextDim);
    }
    draw->AddCircleFilled({bar_min.x + bar_width * t, mid_y}, font() * (live ? 0.42f : 0.34f),
        options.disabled ? colors::kTextDisabled
            : live       ? colors::kAccentBright
                         : colors::kText);
    char text[32];
    std::snprintf(text, sizeof(text), format, value);
    draw_value(r, text, options, px(16.0f) + bar_width + font() * 1.0f, live ? colors::kAccentBright : colors::kText);
    return value != before;
}

bool button_row(const char *label, const RowOptions &options, ImU32 color) {
    const Row r = row(label, options, color);
    ImDrawList *draw = ImGui::GetWindowDrawList();
    const float mid_y = (r.min.y + r.max.y) * 0.5f;
    const ImU32 chevron = options.disabled ? colors::kTextDisabled
        : (r.focused || r.hovered)         ? colors::kAccentBright
                                           : colors::kTextDim;
    const float x = r.max.x - px(16.0f) - font() * 0.3f;
    const float s = font() * 0.28f;
    draw->AddLine({x - s, mid_y - s * 1.6f}, {x + s * 0.6f, mid_y}, chevron, px(2.0f));
    draw->AddLine({x + s * 0.6f, mid_y}, {x - s, mid_y + s * 1.6f}, chevron, px(2.0f));
    if (options.disabled && !options.note.empty()) draw_value(r, "", options, px(16.0f) + font() * 1.2f, 0);
    return r.pressed && !options.disabled;
}

bool value_row(const char *label, const std::string &value, const RowOptions &options) {
    const Row r = row(label, options);
    ImDrawList *draw = ImGui::GetWindowDrawList();
    const float mid_y = (r.min.y + r.max.y) * 0.5f;
    const bool live = (r.focused || r.hovered) && !options.disabled;
    const ImU32 chevron = options.disabled ? colors::kTextDisabled : live ? colors::kAccentBright : colors::kTextDim;
    const float x = r.max.x - px(16.0f) - font() * 0.3f;
    const float s = font() * 0.28f;
    draw->AddLine({x - s, mid_y - s * 1.6f}, {x + s * 0.6f, mid_y}, chevron, px(2.0f));
    draw->AddLine({x + s * 0.6f, mid_y}, {x - s, mid_y + s * 1.6f}, chevron, px(2.0f));
    draw_value(r, value, options, px(16.0f) + font() * 1.1f,
        options.warning ? colors::kDanger
            : live      ? colors::kAccentBright
                        : colors::kText);
    return r.pressed && !options.disabled;
}

bool list_row(const char *id, const std::string &name, const std::string &detail, ListIcon icon, bool highlight) {
    const Row r = row(id, {});
    ImDrawList *draw = ImGui::GetWindowDrawList();
    const float mid_y = (r.min.y + r.max.y) * 0.5f;
    const float icon_x = r.min.x + px(18.0f);
    const float unit = font() * 0.5f;
    const ImU32 ink = highlight ? colors::kAccent : colors::kTextDim;
    switch (icon) {
    case ListIcon::Folder:
    case ListIcon::ParentFolder: {
        const ImVec2 min{icon_x, mid_y - unit * 0.7f};
        const ImVec2 max{icon_x + unit * 2.0f, mid_y + unit * 0.8f};
        draw->AddRectFilled({min.x, min.y - unit * 0.3f}, {min.x + unit * 0.9f, min.y + unit * 0.2f}, ink, px(2.0f));
        draw->AddRectFilled(min, max, ink, px(2.0f));
        if (icon == ListIcon::ParentFolder)
            draw->AddTriangleFilled({min.x + unit, min.y + unit * 0.25f}, {min.x + unit * 0.55f, min.y + unit * 0.9f},
                {min.x + unit * 1.45f, min.y + unit * 0.9f}, colors::kPanel);
        break;
    }
    case ListIcon::Disc:
        draw->AddCircle({icon_x + unit, mid_y}, unit * 0.85f, ink, 0, px(2.0f));
        draw->AddCircleFilled({icon_x + unit, mid_y}, unit * 0.25f, ink);
        break;
    case ListIcon::Drive:
        draw->AddRect(
            {icon_x, mid_y - unit * 0.55f}, {icon_x + unit * 2.0f, mid_y + unit * 0.55f}, ink, px(2.0f), 0, px(2.0f));
        draw->AddCircleFilled({icon_x + unit * 1.55f, mid_y}, unit * 0.15f, ink);
        break;
    case ListIcon::File:
        draw->AddRect({icon_x + unit * 0.3f, mid_y - unit * 0.8f}, {icon_x + unit * 1.7f, mid_y + unit * 0.8f}, ink,
            px(2.0f), 0, px(1.5f));
        break;
    case ListIcon::None:
        break;
    }
    const float text_x = icon == ListIcon::None ? r.min.x + px(16.0f) : icon_x + unit * 2.0f + px(14.0f);
    const float text_y = mid_y - font() * 0.5f;
    const float detail_width = ImGui::CalcTextSize(detail.c_str()).x;
    const float detail_x = r.max.x - px(16.0f) - detail_width;
    ImGui::PushClipRect({text_x, r.min.y}, {detail_x - px(12.0f), r.max.y}, true);
    draw->AddText({text_x, text_y}, highlight ? colors::kAccentBright : colors::kText, name.c_str());
    ImGui::PopClipRect();
    draw->AddText({detail_x, text_y}, colors::kTextDim, detail.c_str());
    return r.pressed;
}

void info_row(const char *label, const std::string &value) {
    // Label above, value below and wrapped: paths can be long.
    const float width = ImGui::GetContentRegionAvail().x - px(32.0f);
    const float value_height = ImGui::CalcTextSize(value.c_str(), nullptr, false, width).y;
    const float height = std::round(font() * 1.5f + value_height);
    const ImVec2 min = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton(label, {ImGui::GetContentRegionAvail().x, height}, ImGuiButtonFlags_EnableNav);
    const bool focused = ImGui::IsItemFocused();
    ImDrawList *draw = ImGui::GetWindowDrawList();
    const ImVec2 max{min.x + width + px(32.0f), min.y + height};
    if (focused) draw_selection(draw, min, max, 235);
    ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * 0.8f);
    draw->AddText({min.x + px(16.0f), min.y + px(5.0f)}, focused ? colors::kText : colors::kTextDim, label,
        shown_end(label));
    ImGui::PopFont();
    draw->AddText(nullptr, 0.0f, {min.x + px(16.0f), min.y + font() * 0.95f},
        focused ? colors::kAccentBright : colors::kText, value.c_str(), nullptr, width);
}

// A section title in orange, smaller than the rows, so the rows below read as
// one group without a rule across the panel.
void section(const char *title) {
    ImGui::Dummy({0.0f, px(10.0f)});
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + px(16.0f));
    ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * 0.85f);
    ImGui::PushStyleColor(ImGuiCol_Text, colors::kEmber);
    ImGui::TextUnformatted(title);
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::Dummy({0.0f, px(4.0f)});
}

bool big_button(const char *label, float width, bool primary, bool disabled) {
    const float height = std::round(font() * 2.2f);
    const ImVec2 min = ImGui::GetCursorScreenPos();
    const ImVec2 max{min.x + width, min.y + height};
    const bool pressed = ImGui::InvisibleButton(label, {width, height}, ImGuiButtonFlags_EnableNav);
    take_focus();
    const bool focused = ImGui::IsItemFocused();
    const bool hovered = ImGui::IsItemHovered();
    ImDrawList *draw = ImGui::GetWindowDrawList();
    const float rounding = 0.0f;
    ImU32 text = colors::kText;
    if (disabled) {
        draw->AddRect(min, max, colors::kTextDisabled, rounding, 0, px(1.5f));
        text = colors::kTextDisabled;
    } else if (primary) {
        draw->AddRectFilled(min, max, focused || hovered ? colors::kAccentBright : colors::kAccent, rounding);
        text = colors::kPanel;
    } else {
        draw->AddRectFilled(min, max,
            focused       ? colors::kRowFocus
                : hovered ? colors::kRowHover
                          : colors::kRow,
            rounding);
        // A neutral edge when unfocused, so the gold focus edge stands out.
        if (focused) {
            draw->AddRect(min, max, colors::kAccentBright, rounding, 0, px(2.0f));
            text = colors::kAccentBright;
        } else {
            draw->AddRect(min, max, colors::kTextDisabled, rounding, 0, px(1.5f));
        }
    }
    if (focused && !disabled)
        draw->AddRect({min.x - px(4.0f), min.y - px(4.0f)}, {max.x + px(4.0f), max.y + px(4.0f)}, colors::kAccentBright,
            rounding, 0, px(3.0f));
    const char *end = shown_end(label);
    const ImVec2 size = ImGui::CalcTextSize(label, end);
    draw->AddText({min.x + (width - size.x) * 0.5f, min.y + (height - size.y) * 0.5f}, text, label, end);
    return pressed && !disabled;
}

void progress_bar(float fraction, const std::string &overlay) {
    const float height = std::round(font() * 1.6f);
    const ImVec2 min = ImGui::GetCursorScreenPos();
    const float width = ImGui::GetContentRegionAvail().x;
    ImGui::Dummy({width, height});
    ImDrawList *draw = ImGui::GetWindowDrawList();
    const ImVec2 max{min.x + width, min.y + height};
    draw->AddRectFilled(min, max, colors::kTrack, height * 0.5f);
    const float filled = width * std::clamp(fraction, 0.0f, 1.0f);
    if (filled > 1.0f)
        draw->AddRectFilled(min, {min.x + std::max(filled, height), max.y}, colors::kAccent, height * 0.5f);
    const ImVec2 size = ImGui::CalcTextSize(overlay.c_str());
    draw->AddText({min.x + (width - size.x) * 0.5f, min.y + (height - size.y) * 0.5f}, colors::kText, overlay.c_str());
}

void paragraph(const std::string &text, ImU32 color) {
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(text.c_str());
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
}

void heading(const std::string &text) {
    ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * 1.15f);
    ImGui::PushStyleColor(ImGuiCol_Text, colors::kAccentBright);
    ImGui::TextUnformatted(text.c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::Dummy({0.0f, px(4.0f)});
}

void hints(std::initializer_list<Hint> list) {
    ImDrawList *draw = ImGui::GetWindowDrawList();
    const bool pad = Layer::get().input_device() == InputDevice::Gamepad;
    const bool south = Layer::get().confirm_south();
    const SDL_GamepadButton confirm = south ? SDL_GAMEPAD_BUTTON_SOUTH : SDL_GAMEPAD_BUTTON_EAST;
    const SDL_GamepadButton back = south ? SDL_GAMEPAD_BUTTON_EAST : SDL_GAMEPAD_BUTTON_SOUTH;
    const ImVec2 start = ImGui::GetCursorScreenPos();
    ImVec2 at = start;
    const float gap = px(6.0f);
    for (const Hint &hint : list) {
        // The browser's file filter switch has a button on the pad only.
        const bool pad_only = hint.control == Control::Toggle || hint.control == Control::Shift ||
            hint.control == Control::Space || hint.control == Control::Symbols || hint.control == Control::Reset;
        if (pad_only && !pad) continue;
        float x = at.x;
        const auto cap = [&](const char *text) { x += draw_cap(draw, {x, at.y}, text, false) + gap; };
        const auto arrows = [&] {
            const float size = font() * 1.15f;
            for (int i = 0; i < 2; ++i) {
                const ImVec2 min{x, at.y + (font() - size) * 0.5f};
                draw->AddRect(min, {min.x + size, min.y + size}, colors::kTextDim, size * 0.3f, 0, px(1.5f));
                draw_triangle(draw, {min.x + size * 0.5f, min.y + size * 0.5f}, size * 0.4f, i == 1, colors::kText);
                x += size + gap;
            }
        };
        switch (hint.control) {
        case Control::Confirm:
            if (pad)
                x += draw_face(draw, {x, at.y}, confirm) + gap;
            else
                cap("Enter");
            break;
        case Control::Back:
            if (pad)
                x += draw_face(draw, {x, at.y}, back) + gap;
            else
                cap("Esc");
            break;
        case Control::Tabs:
            cap(pad ? shoulder_name(false) : "Q");
            cap(pad ? shoulder_name(true) : "W");
            break;
        case Control::Change:
            arrows();
            break;
        case Control::Menu:
            if (pad) {
                cap("L3");
                cap("R3");
            } else {
                cap("Esc");
            }
            break;
        case Control::Start:
            cap(pad ? "Start" : "Enter");
            break;
        case Control::Toggle:
        case Control::Space:
            x += draw_face(draw, {x, at.y}, SDL_GAMEPAD_BUTTON_NORTH) + gap;
            break;
        case Control::Delete:
            if (pad)
                x += draw_face(draw, {x, at.y}, back) + gap;
            else
                cap("Backspace");
            break;
        case Control::Shift:
            x += draw_face(draw, {x, at.y}, SDL_GAMEPAD_BUTTON_WEST) + gap;
            break;
        case Control::Reset:
            cap(select_name());
            break;
        case Control::Clear:
            if (pad)
                x += draw_face(draw, {x, at.y}, SDL_GAMEPAD_BUTTON_NORTH) + gap;
            else
                cap("Del");
            break;
        case Control::Symbols:
            cap(select_name());
            break;
        case Control::Cursor:
            if (pad) {
                cap(shoulder_name(false));
                cap(shoulder_name(true));
            } else {
                arrows();
            }
            break;
        }
        draw->AddText({x + px(2.0f), at.y}, colors::kTextDim, hint.text);
        at.x = x + ImGui::CalcTextSize(hint.text).x + px(26.0f);
    }
    // Room for what was drawn, so a window around the hints can fit them.
    ImGui::Dummy({std::max(0.0f, at.x - start.x - px(24.0f)), font()});
}

} // namespace mhfu::ui
