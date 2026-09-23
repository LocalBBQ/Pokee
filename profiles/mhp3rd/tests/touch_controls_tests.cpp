// The on-screen touch controls' layout, fingers and stick, without a screen.
#include "input/touch_controls.hpp"

#include <cmath>
#include <iostream>

namespace {
using namespace mhp3rd::input::touch;

int failures{};

void check(bool condition, const char *message) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << message << '\n';
    }
}

const Circle &at(const Layout &layout, Control control) { return layout.controls[static_cast<std::size_t>(control)]; }

void test_layout_fits(float width, float height, Insets insets, float size, const char *what) {
    const Layout layout = make_layout(width, height, insets, size);
    bool inside = true;
    for (const Circle &c : layout.controls)
        inside = inside && c.centre.x - c.radius >= insets.left - 0.5f &&
                 c.centre.x + c.radius <= width - insets.right + 0.5f && c.centre.y - c.radius >= insets.top - 0.5f &&
                 c.centre.y + c.radius <= height - insets.bottom + 0.5f;
    check(inside, what);
    bool apart = true;
    for (std::size_t i = 0; i < kControls; ++i)
        for (std::size_t j = i + 1; j < kControls; ++j) {
            const Circle &a = layout.controls[i];
            const Circle &b = layout.controls[j];
            apart = apart && std::hypot(a.centre.x - b.centre.x, a.centre.y - b.centre.y) >= a.radius + b.radius;
        }
    check(apart, "no two controls overlap");
    check(at(layout, Control::Cross).centre.x > layout.stick_split, "the face buttons are on the right");
    check(at(layout, Control::L).centre.x < layout.stick_split, "L is on the left");
    check(at(layout, Control::L).centre.y < height * 0.3f && at(layout, Control::R).centre.y < height * 0.3f,
          "the shoulders are at the top");
    check(at(layout, Control::Triangle).centre.y < at(layout, Control::Cross).centre.y &&
              at(layout, Control::Square).centre.x < at(layout, Control::Circle).centre.x,
          "the face buttons make the PSP's diamond");
}

void test_layouts() {
    test_layout_fits(1280.0f, 576.0f, {}, 1.0f, "20:9 fits");
    test_layout_fits(1920.0f, 1080.0f, {}, 1.0f, "16:9 fits");
    test_layout_fits(2400.0f, 1080.0f, {128.0f, 0.0f, 0.0f, 0.0f}, 1.0f, "a cutout on the left is kept clear");
    test_layout_fits(2400.0f, 1080.0f, {0.0f, 0.0f, 128.0f, 0.0f}, 1.5f, "larger controls still fit");
    test_layout_fits(1280.0f, 800.0f, {}, 0.6f, "16:10 at a small size fits");
}

void test_stick_maths() {
    check(stick_deflection({0.0f, 0.0f}, 100.0f).x == 0.0f, "no push at the centre");
    check(stick_deflection({10.0f, 0.0f}, 100.0f).x == 0.0f, "a twitch inside the dead zone is nothing");
    const Point full = stick_deflection({200.0f, 0.0f}, 100.0f);
    check(std::fabs(full.x - 1.0f) < 1e-5f && full.y == 0.0f, "beyond the reach is full deflection");
    const Point half = stick_deflection({0.0f, -56.0f}, 100.0f);
    check(half.y < -0.45f && half.y > -0.55f, "half way is about half");
    const Point diagonal = stick_deflection({100.0f, 100.0f}, 100.0f);
    check(std::fabs(std::hypot(diagonal.x, diagonal.y) - 1.0f) < 1e-4f, "a diagonal is not more than full");
}

void test_fingers() {
    Controls controls;
    const Layout layout = make_layout(1280.0f, 576.0f, {}, 1.0f);
    controls.set_layout(layout);

    // A thumb on the left moves; one on Cross presses; one on the free right
    // half turns the camera; all at once.
    controls.finger_down(1, {200.0f, 400.0f});
    controls.finger_move(1, {200.0f + layout.stick_radius, 400.0f});
    controls.finger_down(2, at(layout, Control::Cross).centre);
    controls.finger_down(3, {800.0f, 250.0f});
    controls.finger_move(3, {830.0f, 240.0f});
    check(std::fabs(controls.stick().x - 1.0f) < 1e-4f, "the stick pushes right");
    check(controls.buttons() == 0x4000u, "Cross is held");
    const Point drag = controls.take_camera_drag();
    check(drag.x == 30.0f && drag.y == -10.0f, "the camera drag is what the finger moved");
    check(controls.take_camera_drag().x == 0.0f, "a drag is taken once");

    // Sliding from Cross to Circle moves the press; off the buttons, nothing.
    controls.finger_move(2, at(layout, Control::Circle).centre);
    check(controls.buttons() == 0x2000u, "sliding onto Circle presses it instead");
    controls.finger_move(2, {640.0f, 300.0f});
    check(controls.buttons() == 0u, "sliding off the buttons lets go");

    // The stick follows a thumb dragged past its reach.
    controls.finger_move(1, {200.0f + layout.stick_radius * 3.0f, 400.0f});
    controls.finger_move(1, {200.0f + layout.stick_radius * 1.5f, 400.0f});
    check(controls.stick().x < 0.0f, "coming back from a long drag pushes the other way at once");

    controls.finger_up(1);
    check(controls.stick().x == 0.0f && !controls.stick_state().active, "lifting the thumb centres the stick");
    controls.finger_down(4, at(layout, Control::Menu).centre);
    check(controls.take_menu() && !controls.take_menu(), "the menu button is one tap");
    check(controls.buttons() == 0u, "the menu button presses no PSP button");
    controls.release_all();
    check(!controls.any_finger() && controls.buttons() == 0u, "release_all lets go of everything");
}

} // namespace

int main() {
    test_layouts();
    test_stick_maths();
    test_fingers();
    if (failures != 0) {
        std::cerr << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "touch controls tests passed\n";
    return 0;
}
