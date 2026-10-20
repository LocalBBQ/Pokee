#include "camera/game_camera.hpp"

#include "camera/camera_input.hpp"
#include "camera/lock_on.hpp"
#include "game/guest_layout.hpp"
#include "game/layered_armor.hpp"
#include "input/bindings.hpp"
#include "platform/utf8_path.hpp"
#include "settings/settings.hpp"
#include "psprecomp/runtime.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <utility>

namespace mhfu::camera {

namespace {

// Set each game flip from the Focus camera bind (input/bindings.hpp). Tests
// set it directly. With the bind empty, the focus turn is L.
bool focus_bind_held_now{};

bool focus_camera_is_bound() {
    const input::Layout &controls = settings::current().controls;
    const auto slot = static_cast<std::size_t>(input::Action::FocusCamera);
    return input::count(controls.keys[slot]) != 0u || input::count(controls.pad[slot]) != 0u;
}
// The mouse and a drag on a touch screen move the camera the same way,
// aiming included: a pointer's motion rather than a held rate.
Turn combine(const Turn &a, const Turn &b) {
    return {a.yaw_degrees + b.yaw_degrees, a.pitch_degrees + b.pitch_degrees, a.yaw_held || b.yaw_held,
        a.pitch_held || b.pitch_held};
}
Turn take_pointer() {
    const Turn mouse = take(Source::Mouse);
    return combine(mouse, take(Source::Touch));
}
Turn peek_pointer() {
    return combine(peek(Source::Mouse), peek(Source::Touch));
}
} // namespace
namespace {

// ULUS-10391: the camera update is the function at 0x08886E50. Its argument is
// the parent object; the camera structure is that plus 0xA0 (s4 in the
// function). It turns by ±1150 units when bits 0xA0 of the halfword at +0x84
// are set, eases the filtered yaw at +0x82 a quarter of the way to the target
// at +0x80, and reads the preset pointer at +0x70 for the eye offset (height
// at +4, distance at +8) before collision. A non-negative byte at +0x91 takes
// the behind-the-hunter placement instead. These facts come from this
// project's own reading of the executable; each one the driver relies on is
// listed in kSignature.
constexpr std::uint32_t kCameraUpdate = 0x08886E50u;
constexpr std::uint32_t kCameraFromParent = 0xA0u;
constexpr float kRadians = 3.14159265358979323846f / 180.0f;
constexpr float kAngleUnits = 65536.0f / 360.0f;
// The lowest tilt, looking up from below. The game keeps the eye no more than
// 30 degrees below the point it looks at: with the tilt lower, it raised that
// point instead, measured at -30.0 degrees in 58 of 61 updates, and let go at
// once when the tilt came back or a wall moved the eye, which flipped the
// view. MHFU_CAMERA_LOW_TILT=1 allows -60 degrees, as before.
float lowest_tilt() {
    static const float lowest = std::getenv("MHFU_CAMERA_LOW_TILT") != nullptr ? -60.0f : -30.0f;
    return lowest;
}

// Camera structure, relative to the parent object's camera (a0 + 0xA0).
constexpr std::uint32_t kEyeY = 0x04u;
constexpr std::uint32_t kPreset = 0x70u;
constexpr std::uint32_t kPresetHeight = 0x04u;
constexpr std::uint32_t kPresetDistance = 0x08u;
constexpr std::uint32_t kYawTarget = 0x80u;
constexpr std::uint32_t kYawCurrent = 0x82u;
constexpr std::uint32_t kButtons = 0x84u;
constexpr std::uint32_t kVerticalButtons = 0x86u;
// 0x0888E494 returns non-zero, and the recenter at 0x08887508 runs, when this
// bit is set on the halfword at camera +0x86 (0x0888E54C) and on the hunter's
// button word at +1152 (0x0888E55C). It also returns 1 when 0x09A6EE48 does,
// without reading the bit (0x0888E534): that helper returns 1 when the word
// at hunter +1160 has L's bit, 0x100. The camera update copies the halfword
// from the hunter list at +0xA72 onto the camera first (0x0888D054, stored at
// 0x0888D05C). The recenter loads the hunter's facing at 0x088874F4 and stores
// that plus half a turn. While Focus owns that L press, the port eases the
// camera itself and puts the game's snap back, so L does not move the camera
// and then get overridden. A non-negative gate at +0x91 steps toward the
// facing (0x0888755C); that gate is held negative for the same update only.
// 0x0200 is R in the pad sample. The game's recenter check reads it, and R is
// also guard, aim and draw. Focus must not treat that press as its turn
// unless Focus camera is bound to it. When that bind is R, the facing stands
// in for the monster for the update and is put back afterwards.
constexpr std::uint16_t kRecenterBit = 0x0200u;
// L in the pad sample. With Focus camera unbound, a tap of this button is
// the focus turn. A bind replaces it.
constexpr std::uint16_t kFocusButton = 0x0100u;
constexpr std::uint32_t kSnap = 0x8Eu;
// L's recenter writes 20 here (0x08887510) and the same update counts it
// down while it is not zero (0x08887F50). The eye is placed for that snap
// for as long as it lasts.
constexpr std::uint32_t kSnapCount = 0x8Du;
// Non-negative, this byte selects the behind-the-hunter placement (0x08887530)
// and the follow driver leaves that update to the game. At exactly 0 it is the
// game's soft follow instead (0x0888756C): 0x0888E634 steps the yaw toward
// behind the hunter only while it is between 22.5 and about 146 degrees off
// that, and the gate can stay 0 for many seconds, after a bowgun's aim among
// others. While the stick drives the camera, that gate is held negative for
// the update, so the stick keeps the camera and is not pulled back behind the
// hunter. MHFU_CAMERA_GAME_FOLLOW=1 leaves it to the game, as before.
constexpr std::uint32_t kAim = 0x91u;
// Unless this option byte is 1: the update then skips the placement however
// the gate is set (0x08887540 to 0x08887554). The byte is at the word stored
// at 0x089CC558, plus 0x6ADE0. The gate can stay set for many seconds, after
// a bowgun's aim among others, and leaving those updates to the game left
// the stick doing nothing.
constexpr std::uint32_t kGameOptions = 0x089CC558u;
constexpr std::uint32_t kPlacementOff = 0x6ADE0u;
// The followed hunter is not an argument of the camera update. 0x08886E80
// loads the camera object and 0x08886EF8 loads the hunter from it
// (game::layout::followed_hunter). The same object is s3 where the aim camera
// reads the facing and the pitch.
// Bow and bowgun aim is the action at halfword +804 equal to 1001. The aim
// function at 0x09AA4DD0 (game_task) returns unless that action is current,
// then steps the aim heading at halfword +800 by 128, 215 or 400 as the left
// stick's magnitude (halfword +1182) crosses 40, 90 and 123, or by 576 from
// the D-pad. It adds 1, 2 or 4 (5 or 10 from the D-pad) to the pitch byte at
// +1662 through 0x088D5EF0, which stops at ±100. The aim camera at 0x08888448
// reads that byte and turns it into an angle with (pitch + 100) * 34 - 2816,
// so one pitch unit is 34 of the yaw's angle units. The body facing is the
// low 16 bits of the word at +500 (0x0888755C); a write that moves the aim
// heading moves those bits too while they still match it.
constexpr std::uint32_t kAimAction = 804u;
constexpr std::uint16_t kAimingAction = 1001u;
// 1001 is not the gun's alone: with a sword and shield, MHFU_TRACE_CAMERA_STATE
// shows it for two frames between the attack actions 1004 and 1404, and for
// up to seconds at a time, and the aim function never ran in any of them.
// So 1001 is the aim only while that function has run for this hunter in it,
// within this many frames.
constexpr std::uint64_t kAimFunctionGrace = 2u;
constexpr std::uint32_t kAimHeading = 800u;
constexpr std::uint32_t kFacing = 500u;
constexpr std::uint32_t kStickMagnitude = 1182u;
constexpr std::uint32_t kHunterButtons = 1152u;
// The stick reader at 0x08868F50 packs a direction into the high half of the
// button word, from the 16-entry table at 0x089A619C (one entry per 22.5°).
// Index 0 is right (0x20) and the angle runs counterclockwise: up 0x10, left
// 0x80, down 0x40, the same bits as the D-pad. Each aim function below picks
// one of two branches every call. Read from 0x09AA4E2C on: while the byte at
// hunter +1380 is non-zero, or bit 0x1000 of the word at +1556 is set, it
// takes the stick branch (0x09AA5008), which reads those bits shifted up by
// 16 and the magnitude, and the D-pad bits too when 0x088D27CC returns 0.
// Otherwise it takes the D-pad branch, which steps the heading by 576 from
// the D-pad's left (0x80) and right (0x20) and the pitch from up (0x10) and
// down (0x40). +1380 is a timer the function itself sets to 5 or 10 after a
// step, not a weapon flag. A push is shown on both, so whichever branch the
// game takes sees it, and the movement stick is unchanged.
constexpr std::uint32_t kStickDirectionMask = 0x00F00000u;
constexpr std::uint32_t kStickUp = 0x00100000u;
constexpr std::uint32_t kStickRight = 0x00200000u;
constexpr std::uint32_t kStickDown = 0x00400000u;
constexpr std::uint32_t kStickLeft = 0x00800000u;
constexpr std::uint32_t kDpadMask = 0x00F0u;
constexpr std::uint32_t kDpadUp = 0x0010u;
constexpr std::uint32_t kDpadRight = 0x0020u;
constexpr std::uint32_t kDpadDown = 0x0040u;
constexpr std::uint32_t kDpadLeft = 0x0080u;
constexpr int kStickDeadzone = 40;
constexpr int kStickFull = 127;
// The aim functions, in game_task. Each prologue is checked before its wrapper
// is installed, because these addresses belong to whichever overlay is loaded.
// 0x09AA4DD0 aims in action 1001. 0x09AAA158 is the same code for an action
// its caller passes in a1 (8 from 0x09AA3D4C, 0x09AA3D5C and 0x09AA3D70). It
// is not wrapped: once it had run, every hunter in that action counted as
// aiming, which took the right stick off the camera during sword swings.
constexpr std::uint32_t kAimFunction = 0x09AA4DD0u;
constexpr std::array<CodeWord, 4> kAimSignature{{
    {0x09AA4DD0u, 0x27BDFFE0u, "addiu sp,sp,-32"},
    {0x09AA4DD4u, 0xAFBF001Cu, "sw ra,28(sp)"},
    {0x09AA4DD8u, 0xAFB30018u, "sw s3,24(sp)"},
    {0x09AA4DDCu, 0xAFB20014u, "sw s2,20(sp)"},
}};
// The bow aims in its own actions, 1400 and up (traced with MHFU_TRACE_AIM:
// 1401 while the arrow's line is up, 1405 and 1408 as it draws). The bow's
// handler 0x09BB5440 calls 0x09BB6D98 from 0x09BB57D4, and that call is a
// jump inside one generated unit, so no host wrapper sees it. 0x09BB6D98
// aims from the D-pad alone: while R (0x200) is down in the button word, bit
// 0x8 of the word at +1040 is set (0x09BB6E2C) and bit 0x8 of the word at
// +4380 is clear (0x09BB6E68), it steps the heading by 576 from D-pad left and
// right and the pitch from up and down. So while those hold, the right stick
// is pressed on the D-pad in the pad sample itself, and drive_aim resizes each
// step from the stick, as it does for the left stick.
// Those action numbers are not the bow's alone: a sword and shield attacks in
// 1404 too, and with R held and the same bits set it counted as the bow's aim.
// The weapon is the byte at hunter +488 (4 for a sword and shield in every
// MHFU_TRACE_WEAPON dump). 0x09A8AEB0 switches on it below 11 through the
// table at 0x09C0C830; the case at 0x09A8AF98 is `jal 0x09BB4F18`, which
// leads to the bow's handler 0x09BB5440. There are fewer cases than weapons,
// so the bow's number is read from that table rather than assumed.
constexpr std::uint16_t kBowActions = 1400u;
constexpr std::uint16_t kBowActionsEnd = 1500u;
constexpr std::uint32_t kWeaponType = 488u;
constexpr std::uint32_t kWeaponTable = 0x09C0C830u;
constexpr std::uint32_t kWeaponTypes = 11u;
constexpr std::uint32_t kBowCase = 0x09A8AF98u;
constexpr std::uint32_t kBowCaseCall = 0x0E6ED3C6u; // jal 0x09BB4F18
constexpr std::uint32_t kBowAimReady = 1040u;
constexpr std::uint32_t kBowAimBlocked = 4380u;
constexpr std::uint32_t kBowAimBit = 0x8u;
// How far an axis of the right stick is pushed (of 127) before it presses
// that way on the D-pad.
constexpr int kBowDpadThreshold = 24;
// The largest step the aim function adds in one call, with the D-pad's 576
// and the stick's 400 both able to land. Anything larger is the game setting
// the heading for another reason.
constexpr int kLargestYawStep = 1024;

// The one vertical aim. The byte steps by at most 10 and stops at 100. One
// unit is 34 angle units in the aim camera, which is kAngleUnits / 34 degrees.
struct PitchField {
    std::uint32_t offset;
    bool halfword;
    int limit;
    int largest_step;
    float units_per_degree;
};
constexpr float kAimPitchUnitsPerDegree = kAngleUnits / 34.0f;
constexpr std::array<PitchField, 1> kPitchFields{{
    {1662u, false, 100, 16, kAimPitchUnitsPerDegree},
}};
constexpr std::uint32_t kHunterExtent = 1664u;
// Left and right live in the button halfword; up and down live in the next one.
constexpr std::uint16_t kTurnBits = 0x00A0u;
constexpr std::uint16_t kVerticalBits = 0x0050u;
// The camera update copies its two button halfwords from the object at
// 0x089CBD04: +0xA70 onto camera +0x84 (0x0888D04C) and +0xA72 onto +0x86.
constexpr std::uint32_t kCameraObject = 0x089CBD04u;
constexpr std::uint32_t kObjectTurnButtons = 0xA70u;
constexpr std::uint32_t kObjectVerticalButtons = 0xA72u;

// The mouse while aiming. Its degrees wait for a step of the game to size
// (the game may step an update after the stick showed the direction); up to
// this much, and for this many updates without a step, after which they are
// dropped, as a push of the stick is while the game makes no step.
constexpr float kMouseAimCarry = 45.0f;
constexpr unsigned kMouseAimWaits = 3u;
// Less than this, on an axis, is not presented to the game as a push on it.
constexpr float kMouseAimThreshold = 0.05f;
// Degrees of mouse yaw in one frame that switch on the game's own turn.
constexpr float kMouseStockTurn = 0.5f;

// Game code the driver depends on, read from ULUS-10391. If any word differs,
// the executable is not the one these facts were read from, and the driver
// stays out rather than write to places that may mean something else.
// When a wall blocks the eye, the follow update sweeps two ways toward the eye
// it wants: from where the eye was, which stops it on the near side of the
// wall, and along the view line from camera +0x50, which pulls it in. This
// function (called at 0x08887CCC, its argument the camera's parent) picks the
// pull-in only during a snap (camera +0x8D, parent +301) or while the target
// yaw is within a quarter turn of the hunter's facing: in front of the hunter.
// Behind the hunter, with its back to a wall, the eye was held where the wall
// stopped it while the view turned, then snapped once the wanted eye came
// clear. While the analog camera drives, the wrapper picks the pull-in from
// any side. MHFU_CAMERA_WALL_SLIDE=1 leaves the choice to the game.
constexpr std::uint32_t kPullInChoice = 0x0888FCE4u;

constexpr std::array<CodeWord, 46> kSignature{{
    {0x08886E50u, 0x27BDFE30u, "addiu sp,sp,-464: the camera update"},
    {0x08886EC8u, 0x26B400A0u, "addiu s4,s5,160: the camera is the parent object plus 0xA0"},
    {0x08887470u, 0x96850084u, "lhu a1,0x84(s4): the turn buttons"},
    {0x08887474u, 0x30A200A0u, "andi v0,a1,0xa0: left and right gate the turn"},
    {0x0888749Cu, 0x2403FB82u, "addiu v1,zero,-1150: one sign of the yaw step"},
    {0x088874A0u, 0x2403047Eu, "addiu v1,zero,1150: and the other"},
    {0x088874B0u, 0x86820080u, "lh v0,0x80(s4): the target yaw"},
    {0x088874B8u, 0xA6820080u, "sh v0,0x80(s4): written back by that step"},
    {0x08887614u, 0x86820080u, "lh v0,0x80(s4): the filter reads the target"},
    {0x08887618u, 0x86830082u, "lh v1,0x82(s4): and the filtered yaw"},
    {0x08887624u, 0x00021083u, "sra v0,v0,2: a quarter of the difference"},
    {0x08887630u, 0xA6820082u, "sh v0,0x82(s4): the filtered yaw"},
    {0x088874F4u, 0x8E6401F4u, "lw a0,500(s3): L's recenter reads the hunter's facing"},
    {0x0888750Cu, 0xA283008Eu, "sb v1,0x8E(s4): and sets the snap byte"},
    {0x08887650u, 0x2C621000u, "sltiu v0,v1,4096: within 22.5 degrees of the target"},
    {0x08887658u, 0xA280008Eu, "sb zero,0x8E(s4): the snap is dropped at once"},
    {0x08887530u, 0x82860091u, "lb a2,0x91(s4): the behind-the-hunter gate"},
    {0x08887534u, 0x04C00029u, "bltz a2: a negative gate skips that placement"},
    {0x08887814u, 0xC4400004u, "lwc1 f0,4(v0): the preset's eye-offset height"},
    {0x0888781Cu, 0xC4400008u, "lwc1 f0,8(v0): and its distance"},
    {0x08886E80u, 0x3C03089Du, "lui v1,0x89d: the base of the hunter list"},
    {0x08886E84u, 0x8C70BD04u, "lw s0,-17148(v1): the list at 0x089CBD04"},
    {0x08886EF8u, 0x8E130A84u, "lw s3,2692(s0): the hunter the camera follows"},
    {0x0888755Cu, 0x8E6301F4u, "lw v1,500(s3): the hunter's facing"},
    {0x088D5EF0u, 0x8083067Eu, "lb v1,1662(a0): the aim pitch"},
    {0x088D5EF8u, 0xA083067Eu, "sb v1,1662(a0): the aim pitch, written back"},
    {0x088D5F00u, 0x28610065u, "slti at,v1,101: the aim pitch stops at 100"},
    {0x08888448u, 0x8265067Eu, "lb a1,1662(s3): the aim camera reads that pitch"},
    {0x08888460u, 0x00052840u, "sll a1,a1,1: one pitch unit becomes 34 angle units"},
    {0x08888464u, 0x24A5F500u, "addiu a1,a1,-2816: the aim camera's pitch angle"},
    {0x08887D0Cu, 0xC6820004u, "lwc1 f2,4(s4): the eye height the ease reads"},
    {0x08887D10u, 0x3C023E00u, "lui v0,0x3e00: one eighth, the eye-height ease"},
    {0x08887D24u, 0xE6800004u, "swc1 f0,4(s4): the eased eye height"},
    // The camera update copies this halfword onto camera +0x86 before it tests
    // L. Clearing the camera's copy is undone by that copy.
    {0x08886F14u, 0x0E223401u, "jal 0x0888D004: copies the button halfwords onto the camera"},
    {0x0888D054u, 0x96230A72u, "lhu v1,0xA72(s1): the recenter bit, on the hunter list"},
    {0x0888D05Cu, 0xA6030086u, "sh v1,0x86(s0): written to the camera in the branch delay"},
    {0x0888D04Cu, 0x96230A70u, "lhu v1,0xA70(s1): the turn buttons, on the hunter list"},
    {0x0888D050u, 0xA6030084u, "sh v1,0x84(s0): written to the camera"},
    {0x08887540u, 0x8C44C558u, "lw a0,-15016(v0): the options at 0x089CC558"},
    {0x08887548u, 0x3C020007u, "lui v0,0x7: plus 0x70000"},
    {0x08887550u, 0x8042ADE0u, "lb v0,-21024(v0): the byte that turns the placement off at 1"},
    {0x08887CCCu, 0x0E223F39u, "jal 0x0888FCE4: whether a blocked eye is pulled in along the view line"},
    {0x0888FCE4u, 0x3C02089Du, "lui v0,0x89d: that choice"},
    {0x0888FCECu, 0x9083012Du, "lbu v1,301(a0): it pulls in during a snap"},
    {0x088874E4u, 0x0E223925u, "jal 0x0888E494: the recenter check"},
    {0x0888E494u, 0x27BDFFF0u, "addiu sp,sp,-16: the recenter check"},
}};

struct State {
    bool prepared{};
    bool hooked{};
    RotationFunction original{};
    RotationFunction pull_in_original{};
    std::uint64_t frame{};
    std::uint64_t last_update{};
    std::uint32_t address{};
    bool available{};
    float yaw_remainder{};
    bool pitch_owned{};
    // The pitch above is lock-on's (lock_on.hpp), not the player's.
    bool pitch_by_lock{};
    float pitch{};
    // The shared preset's height and distance, rewritten for this update only
    // and put back after the game has read them.
    bool preset_patched{};
    std::uint32_t preset{};
    float saved_height{};
    float saved_distance{};
    unsigned updates{};
    // L's recenter, which the game took and dropped in the same update.
    bool recenter_dropped{};
    // L was down on the previous update. A hold is one tap; a second tap is a
    // press after it has been released.
    bool focus_held{};
    // R is down, or it started the snap that is still set. That snap is not a
    // focus press and not a camera command.
    bool r_latch{};
    // This hold already recentered behind the hunter. It does not aim at the
    // monster again until L has been released.
    bool yielded_this_hold{};
    // The hunter's facing, replaced for this update when the focus bind is R,
    // so the game's recenter looks at the monster, and put back afterwards.
    bool facing_substituted{};
    std::uint32_t saved_facing{};
    // Focus owns this L press. The game's recenter still runs, and is put
    // back: the port's ease is the only move. Not a second tap.
    bool quiet_l{};
    // The gate is the game's soft follow and the stick drives the camera: it
    // is held negative for this update.
    bool quiet_follow{};
    // The eye's distance from the point looked at, eased back out after a
    // wall pulled it in (ease_wall). The game's own eye is kept so it can be
    // put back before its next update.
    bool wall_valid{};
    float wall_distance{};
    bool wall_eased{};
    std::array<std::uint32_t, 3> wall_game_eye{};
    std::array<std::uint32_t, 3> wall_shown_eye{};
    bool aiming{};
    // The bow is aiming (bow_aiming), as of the last flip.
    bool bow_aiming{};
    // MHFU_TRACE_AIM: calls of the aim wrapper, and those that showed it a
    // push, since the last [aim] line.
    std::uint64_t aim_calls{};
    std::uint64_t aim_pushes{};
    // The generated aim function, and the centred deflection to show it.
    RotationFunction aim_original{};
    // The wrapper is in place, and the last frame it ran for a hunter in 1001.
    bool aim_wrapped{};
    std::uint64_t aim_ran_frame{};
    std::uint32_t aim_ran_hunter{};
    int aim_x{};
    int aim_y{};
    // How far the stick is pushed on each axis, -1 to 1. Zero: the follow
    // camera sizes the step (the mouse). Set with aim_x/aim_y.
    float aim_scale_x{};
    float aim_scale_y{};
    // The frame whose aim step was already sized from the stick, so the
    // follow camera does not size it again. 0: never.
    unsigned sized_yaw_frame{};
    unsigned sized_pitch_frame{};
    // The followed hunter's action is the aim, even when the aim camera has
    // replaced the follow camera and that update is not running.
    bool hunter_aiming{};
    std::uint32_t aim_hunter{};
    std::uint16_t aim_heading{};
    std::array<int, 3> aim_pitch{};
    float aim_yaw_remainder{};
    float aim_pitch_remainder{};
    // The mouse's aim not yet spent on a step, in degrees, and the frame
    // after the one its direction was last shown to the game (0: never).
    float mouse_yaw{};
    float mouse_pitch{};
    unsigned mouse_yaw_waits{};
    unsigned mouse_pitch_waits{};
    std::uint64_t mouse_shown{};
    // The direction last shown, each axis -1, 0 or +1.
    int shown_x{};
    int shown_y{};
    // The game's own steps for a shown direction, learned from the steps it
    // made in this aim: size (straight and diagonal) and which way it went
    // for a positive push (+1, -1; 0: not seen yet). Scope and aim step by
    // different amounts, so a change between them starts over.
    bool learned_scoped{};
    std::array<int, 2> yaw_step{};
    int yaw_sign{};
    std::array<int, 2> pitch_step{};
    int pitch_sign{};
    std::size_t pitch_field{};
    // The game's next step, taken off the aim in advance at the flip (see
    // game_camera_anticipate_aim), and the values that left in the hunter.
    int anticipated_yaw{};
    std::uint16_t anticipated_heading{};
    int anticipated_pitch{};
    int anticipated_pitch_value{};
};
State state;
// Kept apart from State, which prepare_game_camera() starts again.
bool pose_wanted = false;

bool option_on() {
    const auto &s = settings::current();
    // Right stick describes a physical stick. On Android a finger drag turns
    // the camera whatever it says, so there Analog camera alone decides.
    return s.analog_camera &&
        (s.right_stick == settings::RightStick::Camera || settings::kPlatform == settings::Platform::Android);
}

bool driving_allowed() {
    return state.hooked && option_on();
}

bool lock_on_allowed() {
    return settings::current().lock_on;
}

float load_float(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    return std::bit_cast<float>(memory.load32(address));
}

void store_float(psprecomp::GuestMemory &memory, std::uint32_t address, float value) {
    memory.store32(address, std::bit_cast<std::uint32_t>(value));
}

// While the analog camera drives the follow camera and D-pad camera is off,
// the D-pad does not also turn it or step its height levels: its bits are
// cleared from the halfwords the update copies, for that update only, and put
// back after it for everything else that reads the pad.
struct DpadMute {
    std::uint32_t object{};
    std::uint16_t turn{};
    std::uint16_t vertical{};
};

std::optional<DpadMute> mute_dpad(psprecomp::GuestMemory &memory, std::uint32_t address) {
    if (settings::current().dpad_camera || !driving_allowed()) return std::nullopt;
    DpadMute mute{memory.load32(kCameraObject)};
    if (mute.object == 0u || memory.raw_pointer(mute.object + kObjectTurnButtons, 4u) == nullptr) return std::nullopt;
    mute.turn = memory.load16(mute.object + kObjectTurnButtons);
    mute.vertical = memory.load16(mute.object + kObjectVerticalButtons);
    memory.store16(mute.object + kObjectTurnButtons, mute.turn & static_cast<std::uint16_t>(~kTurnBits));
    memory.store16(mute.object + kObjectVerticalButtons, mute.vertical & static_cast<std::uint16_t>(~kVerticalBits));
    // The camera's own copies, from the last update, are what the driver reads.
    memory.store16(address + kButtons, memory.load16(address + kButtons) & static_cast<std::uint16_t>(~kTurnBits));
    memory.store16(address + kVerticalButtons,
        memory.load16(address + kVerticalButtons) & static_cast<std::uint16_t>(~kVerticalBits));
    return mute;
}

void unmute_dpad(psprecomp::GuestMemory &memory, const DpadMute &mute) {
    memory.store16(mute.object + kObjectTurnButtons, mute.turn);
    memory.store16(mute.object + kObjectVerticalButtons, mute.vertical);
}

void release() {
    state.available = false;
    state.pitch_owned = false;
    state.preset_patched = false;
    state.yaw_remainder = 0.0f;
    state.aiming = false;
    state.aim_yaw_remainder = 0.0f;
    state.aim_pitch_remainder = 0.0f;
    state.mouse_yaw = 0.0f;
    state.mouse_pitch = 0.0f;
    state.mouse_yaw_waits = 0u;
    state.mouse_pitch_waits = 0u;
}

bool placement_off(const psprecomp::GuestMemory &memory);

// MHFU_TRACE_CAMERA_STATE: one line per follow update, and one starting with
// F per game frame.
std::ofstream *state_trace() {
    static const std::filesystem::path path = environment_path("MHFU_TRACE_CAMERA_STATE");
    if (path.empty()) return nullptr;
    static std::ofstream out(path);
    return &out;
}

void trace(const psprecomp::GuestMemory &memory, std::uint32_t address, const Turn &turn, bool active, bool yielded,
    bool vertical_command, float height, float distance) {
    std::ofstream *stream = state_trace();
    if (stream == nullptr) return;
    std::ofstream &out = *stream;
    const float eye_dx = load_float(memory, address) - load_float(memory, address + 0x10u);
    const float eye_dy = load_float(memory, address + 4u) - load_float(memory, address + 0x14u);
    const float eye_dz = load_float(memory, address + 8u) - load_float(memory, address + 0x18u);
    // After the eye's pitch: why an update was left to the game, and whether
    // the stick stays the driver's: snap byte and count, behind gate, the
    // button halfwords as the driver read them, R's latch, the stick kept, the
    // aim, and whether the game's option turns the placement off.
    out << state.frame << ',' << std::hex << address << std::dec << ',' << active << ',' << turn.yaw_degrees << ','
        << turn.pitch_degrees << ',' << memory.load16(address + kYawTarget) << ','
        << memory.load16(address + kYawCurrent) << ',' << height << ',' << distance << ',' << state.pitch_owned << ','
        << state.pitch << ',' << yielded << ',' << vertical_command << ','
        << std::atan2(eye_dy, std::hypot(eye_dx, eye_dz)) / kRadians << ',' << int{memory.load8(address + kSnap)} << ','
        << int{memory.load8(address + kSnapCount)} << ',' << int{static_cast<std::int8_t>(memory.load8(address + kAim))}
        << ',' << std::hex << memory.load16(address + kButtons) << ',' << memory.load16(address + kVerticalButtons)
        << std::dec << ',' << state.r_latch << ',' << state.available << ',' << state.hunter_aiming << ','
        << placement_off(memory) << '\n';
    if (state.updates % 30u == 0u) out.flush();
}

std::uint32_t followed_hunter(const psprecomp::GuestMemory &memory);

bool placement_off(const psprecomp::GuestMemory &memory) {
    const auto options = memory.load32(kGameOptions);
    if (options == 0u || memory.raw_pointer(options + kPlacementOff, 1u) == nullptr) return false;
    return static_cast<std::int8_t>(memory.load8(options + kPlacementOff)) == 1;
}

// After the follow update, the game's wall collision has set the eye's
// distance from the point it looks at. A wall pulls the eye in at once, and
// the game copies the eye's sideways axes at once, so a collision that comes
// and goes from one update to the next snaps the view in and out; with the
// back against a wall that kept on until the view had turned round to the
// other side of the hunter. The distance shown follows the game's straight in,
// which keeps the eye clear of the wall, and goes back out a part of the way
// each update. The eye stays on the line the game put it on, nearer the point
// looked at, which the collision has already cleared. The game's own eye is
// put back before its next update, so its ease and collision work from their
// own values. MHFU_CAMERA_WALL_SNAP=1 leaves the eye as the game put it.
constexpr float kWallEaseOut = 0.2f;

void unease_wall(psprecomp::GuestMemory &memory, std::uint32_t address) {
    if (!std::exchange(state.wall_eased, false) || state.address != address) return;
    for (std::uint32_t i = 0; i < 3u; ++i)
        if (memory.load32(address + i * 4u) != state.wall_shown_eye[i]) return; // the game moved it since
    for (std::uint32_t i = 0; i < 3u; ++i) memory.store32(address + i * 4u, state.wall_game_eye[i]);
}

void ease_wall(psprecomp::GuestMemory &memory, std::uint32_t address, bool fresh) {
    static const bool snap = std::getenv("MHFU_CAMERA_WALL_SNAP") != nullptr;
    if (snap || !driving_allowed()) {
        state.wall_valid = false;
        return;
    }
    std::array<float, 3> eye{};
    std::array<float, 3> offset{};
    for (std::uint32_t i = 0; i < 3u; ++i) {
        eye[i] = load_float(memory, address + i * 4u);
        offset[i] = eye[i] - load_float(memory, address + 0x10u + i * 4u);
    }
    const float distance = std::sqrt(offset[0] * offset[0] + offset[1] * offset[1] + offset[2] * offset[2]);
    if (!std::isfinite(distance) || distance < 1.0f) {
        state.wall_valid = false;
        return;
    }
    if (fresh || !state.wall_valid || distance <= state.wall_distance)
        state.wall_distance = distance;
    else
        state.wall_distance += (distance - state.wall_distance) * kWallEaseOut;
    state.wall_valid = true;
    if (state.wall_distance >= distance - 0.5f) return;
    const float scale = state.wall_distance / distance;
    for (std::uint32_t i = 0; i < 3u; ++i) {
        state.wall_game_eye[i] = memory.load32(address + i * 4u);
        const float shown = eye[i] - offset[i] * (1.0f - scale);
        store_float(memory, address + i * 4u, shown);
        state.wall_shown_eye[i] = memory.load32(address + i * 4u);
    }
    state.wall_eased = true;
}

void restore_preset(psprecomp::GuestMemory &memory) {
    if (!state.preset_patched) return;
    state.preset_patched = false;
    if (memory.raw_pointer(state.preset, 0x0Cu) == nullptr) return;
    store_float(memory, state.preset + kPresetHeight, state.saved_height);
    store_float(memory, state.preset + kPresetDistance, state.saved_distance);
}

// The quest follow camera. Yaw is written to both of the game's yaw fields
// before the update, so its quarter-step filter has nothing left to close.
// Pitch rewrites the preset offset this update is about to read, including
// for its collision, and the caller puts those two floats back afterwards.
// 0x08887D10 then eases the eye's height an eighth of the way to that offset
// and copies the other two axes at once. The player's change in height is
// added to the eye first, so the ease is left for terrain and collision.
void drive_follow(psprecomp::GuestMemory &memory, std::uint32_t address) {
    if (state.address != address || state.frame - state.last_update > 2u) {
        state.pitch_owned = false;
        state.yaw_remainder = 0.0f;
    }
    state.address = address;
    state.last_update = state.frame;
    state.aiming = false;
    ++state.updates;

    const bool active = driving_allowed();
    // Taken even while this update is left to the game, so nothing built up
    // is spent the moment the port drives again. The stick's held rate is
    // noted first: in Follow it pauses tracking without dropping the monster.
    const Rate stick_rate = rate(Source::Stick);
    const Turn turn = take();
    const auto buttons = memory.load16(address + kButtons);
    const auto vertical = memory.load16(address + kVerticalButtons);
    const bool snap = memory.load8(address + kSnap) != 0u;
    const bool dropped = std::exchange(state.recenter_dropped, false);
    // A quick second L is the game's recenter behind the hunter. Leave its
    // snap alone until that recenter finishes, then a later tap can focus.
    if (lock_on_yielding_recenter() && !snap && memory.load8(address + kSnapCount) == 0u) lock_on_finish_yield();
    // Focus camera unbound means L. A bind is that button instead, and L
    // recenters behind the hunter again. While Focus owns an L press, the
    // game's recenter is put back after the update and the port eases on its
    // own. A second tap, after the button was released, is the game's recenter
    // behind the hunter. With nothing selected, L is that recenter too.
    // Follow never takes the press. When the focus bind is R, the facing
    // stands in for the monster so the game's own snap aims there.
    const auto hunter = followed_hunter(memory);
    const auto hunter_buttons = hunter != 0u ? memory.load32(hunter + kHunterButtons) : 0u;
    const bool l_down = (hunter_buttons & kFocusButton) != 0u;
    const bool r_down = (hunter_buttons & kRecenterBit) != 0u;
    const bool bound = focus_camera_is_bound();
    // LT (the default) is Lock on and Focus/Follow Camera. The release is the
    // tap. Holding the trigger must not also count as the focus button, or
    // the press and the release would both act, and L would no longer be the
    // camera on its own.
    const bool shared = lock_on_trigger_initiates_camera();
    const bool focus_down = shared ? false : (bound ? focus_bind_held_now : l_down);
    bool focus_press = focus_down && !state.focus_held;
    if (shared && lock_on_take_focus_press()) focus_press = true;
    state.focus_held = focus_down;
    // Follow uses this same bind as a toggle. A hold is one press. With the
    // bind empty, L stays the game's recenter and does not toggle.
    if (focus_press && bound && lock_on_allowed() && settings::current().lock_on_style == settings::LockOnStyle::Follow)
        lock_on_toggle_follow();
    // R shares the bit the game's recenter check reads. A snap it starts has
    // to be ignored after the button is up as well, until that snap is gone,
    // or the next update treats it as the focus button. The focus bind itself
    // is that button even when it is R.
    if (r_down && !focus_down) state.r_latch = true;
    if (focus_press) state.r_latch = false;
    if (state.r_latch && !r_down && !snap && memory.load8(address + kSnapCount) == 0u) state.r_latch = false;
    if (!focus_down) state.yielded_this_hold = false;
    const bool focus_selected = lock_on_allowed() &&
        settings::current().lock_on_style == settings::LockOnStyle::Focus && lock_on_status().locked;
    const bool second_tap = focus_press && lock_on_focus_double_tap();
    // Accept before the aim is chosen. A hold of that second tap must not
    // start aiming at the monster once the behind-the-hunter snap has finished.
    if (second_tap) {
        lock_on_accept_double_tap();
        state.yielded_this_hold = true;
    }
    // R beside L, with nothing bound to Focus camera, is not the turn.
    const bool steal_recenter = focus_down && !(r_down && !bound) && focus_selected && !lock_on_yielding_recenter() &&
        !second_tap && !state.yielded_this_hold;
    // L is the game's recenter. While this press is the focus turn, that
    // recenter is put back and the ease below is what the frame shows. R, when
    // it is the focus bind, still recenters through the game, aimed below.
    state.quiet_l = steal_recenter && l_down;
    const bool game_will_recenter = r_down && !state.quiet_l;
    if (!state.quiet_l && steal_recenter && game_will_recenter && memory.raw_pointer(hunter + kFacing, 4u) != nullptr &&
        memory.raw_pointer(address + 0x18u, 4u) != nullptr) {
        const Vec3 look_at{load_float(memory, address + 0x10u), load_float(memory, address + 0x14u),
            load_float(memory, address + 0x18u)};
        if (const std::optional<std::uint16_t> yaw = lock_on_focus_aim_yaw(memory, look_at)) {
            const auto word = memory.load32(hunter + kFacing);
            state.saved_facing = word;
            state.facing_substituted = true;
            // 0x08887500 adds half a turn to this word and stores the low 16
            // bits as the target yaw. The value that survives that add is the
            // direction of the monster.
            memory.store32(hunter + kFacing, (word & 0xFFFF0000u) | static_cast<std::uint16_t>(*yaw - 0x8000u));
        }
    }
    // The snap byte is cleared so a recenter already marked does not also
    // place the camera behind the hunter. With a bind, a snap from L is the
    // game's recenter, not this turn.
    const bool snap_is_ours = snap && (bound ? focus_down : true);
    const bool focus_snap = !lock_on_yielding_recenter() && !state.r_latch &&
        (snap_is_ours || dropped || (focus_press && steal_recenter)) && lock_on_allowed() &&
        settings::current().lock_on_style == settings::LockOnStyle::Focus && lock_on_status().locked;
    // A snap byte left set by this same hold is the turn, not a second tap.
    // A snap that appears while the focus button is up, inside the window, is
    // the second tap the tests and the game mark with that byte.
    if (focus_snap && lock_on_focus_double_tap() && !focus_down)
        lock_on_accept_double_tap();
    else if (focus_snap)
        memory.store8(address + kSnap, 0u);
    // One press starts the reticle's short life. The frames of a hold are the
    // same press, and a snap byte while the button is up is the press the tests use.
    if (focus_selected && (focus_press || (focus_snap && !focus_down))) lock_on_focus_reticle();
    // A non-negative gate is the behind-the-hunter placement. That update,
    // a snap, and the D-pad's own turn and tilt stay the game's. While the
    // facing stands in for the monster, the game's placement is the turn.
    // The gate is ignored for an L press Focus owns: it is held negative for
    // the update, and the ease has to be written before that.
    static const bool game_follow = std::getenv("MHFU_CAMERA_GAME_FOLLOW") != nullptr;
    const auto gate = static_cast<std::int8_t>(memory.load8(address + kAim));
    state.quiet_follow = active && gate == 0 && !game_follow;
    const bool behind = !state.quiet_l && !state.quiet_follow && gate >= 0 && !placement_off(memory);
    const bool turn_command = (buttons & kTurnBits) != 0u;
    const bool vertical_command = (vertical & kVerticalBits) != 0u;
    const bool game_snap = snap && !focus_snap && !state.r_latch;
    const bool yielded = behind || game_snap || turn_command || vertical_command || state.facing_substituted;

    const auto preset = memory.load32(address + kPreset);
    const bool preset_ok = memory.raw_pointer(preset, 0x20u) != nullptr;
    const float height = preset_ok ? load_float(memory, preset + kPresetHeight) : 0.0f;
    const float distance = preset_ok ? load_float(memory, preset + kPresetDistance) : 1.0f;
    const float radius = std::hypot(height, distance);
    const bool radius_ok = std::isfinite(radius) && radius >= 1.0f && radius <= 10000.0f;
    const float stock_pitch = std::atan2(height, std::fabs(distance)) / kRadians;

    std::optional<LockOnAim> lock;
    if (lock_on_allowed() && lock_on_wanted()) {
        LockOnCamera camera;
        camera.address = address;
        camera.yaw = memory.load16(address + kYawCurrent);
        camera.look_at = {load_float(memory, address + 0x10u), load_float(memory, address + 0x14u),
            load_float(memory, address + 0x18u)};
        camera.manual_yaw = !yielded && turn.yaw_held ? turn.yaw_degrees : 0.0f;
        camera.stick = !yielded && (stick_rate.yaw != 0.0f || stick_rate.pitch != 0.0f);
        camera.command = game_snap || turn_command || vertical_command;
        camera.snap = focus_snap && !lock_on_yielding_recenter();
        camera.pitch = state.pitch_owned ? state.pitch : stock_pitch;
        camera.player_pitch = (active && turn.pitch_held && !yielded) || (state.pitch_owned && !state.pitch_by_lock);
        lock = lock_on_update(memory, camera);
    }
    const bool lock_pitch = lock && lock->pitch;
    if ((!active && !lock_pitch) || behind || game_snap || vertical_command) state.pitch_owned = false;

    if (yielded) {
        state.yaw_remainder = 0.0f;
    } else if (lock) {
        state.yaw_remainder = 0.0f;
        memory.store16(address + kYawTarget, lock->yaw);
        memory.store16(address + kYawCurrent, lock->yaw);
    } else if (active && turn.yaw_held) {
        state.yaw_remainder -= turn.yaw_degrees * kAngleUnits;
        const int step = static_cast<int>(state.yaw_remainder);
        state.yaw_remainder -= static_cast<float>(step);
        const auto yaw = static_cast<std::uint16_t>(memory.load16(address + kYawCurrent) + step);
        memory.store16(address + kYawTarget, yaw);
        memory.store16(address + kYawCurrent, yaw);
    } else {
        state.yaw_remainder = 0.0f;
    }

    if (!yielded && preset_ok && radius_ok && (lock_pitch || active)) {
        const float previous_pitch = state.pitch_owned ? state.pitch : stock_pitch;
        if (lock_pitch) {
            state.pitch = *lock->pitch;
            state.pitch_owned = true;
            state.pitch_by_lock = true;
        } else if (turn.pitch_held) {
            if (!state.pitch_owned) state.pitch = stock_pitch;
            state.pitch_owned = true;
            state.pitch_by_lock = false;
            state.pitch = std::clamp(state.pitch + turn.pitch_degrees, lowest_tilt(), 70.0f);
        }
        if (state.pitch_owned) {
            state.preset = preset;
            state.saved_height = height;
            state.saved_distance = distance;
            const float angle = state.pitch * kRadians;
            const float sign = distance < 0.0f ? -1.0f : 1.0f;
            store_float(memory, preset + kPresetHeight, radius * std::sin(angle));
            store_float(memory, preset + kPresetDistance, std::copysign(radius * std::cos(angle), sign));
            state.preset_patched = true;
            // The ease at 0x08887D10 closes an eighth of (desired height - eye
            // height). Moving the eye by the player's own change leaves it
            // nothing of that change to close, in either direction.
            const float delta = radius * (std::sin(angle) - std::sin(previous_pitch * kRadians));
            if (std::isfinite(delta) && delta != 0.0f)
                store_float(memory, address + kEyeY, load_float(memory, address + kEyeY) + delta);
        }
    }
    state.available = !yielded && (active || lock.has_value());
    trace(memory, address, turn, active, yielded, vertical_command, height, distance);
}

// MHFU_TRACE_CAMERA_MODES=path.csv: every call of the camera update, with the
// stick and the camera structure. For finding what another camera keeps where.
std::ofstream *modes_trace() {
    static const std::filesystem::path path = environment_path("MHFU_TRACE_CAMERA_MODES");
    if (path.empty()) return nullptr;
    static std::ofstream out(path);
    return &out;
}

void dump_camera(std::ostream &out, const psprecomp::GuestMemory &memory, std::uint32_t address) {
    out << ",yaw=" << memory.load16(address + kYawTarget) << ':' << memory.load16(address + kYawCurrent)
        << ",buttons=" << std::hex << memory.load16(address + kButtons) << std::dec;
    out << ",words=" << std::hex;
    for (std::uint32_t offset = 0u; offset < 0x190u; offset += 4u)
        out << (offset ? ":" : "") << memory.load32(address + offset);
    out << std::dec;
}

// Once a flip as well, from the last camera the ordinary update named: an
// aiming camera may never call the helper at all.
void trace_flip(const psprecomp::GuestMemory &memory) {
    std::ofstream *out = modes_trace();
    if (out == nullptr || state.address == 0u || memory.raw_pointer(state.address, 0x190u) == nullptr) return;
    const Rate stick = rate(Source::Stick);
    *out << state.frame << ",flip,s1=" << std::hex << state.address << std::dec << ",stick=" << stick.yaw << ':'
         << stick.pitch << ",aim=" << static_cast<int>(static_cast<std::int8_t>(memory.load8(state.address + kAim)));
    dump_camera(*out, memory, state.address);
    *out << std::endl;
}

void trace_modes(const psprecomp::GuestMemory &memory, const psprecomp::AllegrexContext &ctx) {
    std::ofstream *trace_out = modes_trace();
    if (trace_out == nullptr) return;
    std::ofstream &out = *trace_out;
    const auto parent = ctx.gpr[4];
    const auto address = parent + kCameraFromParent;
    const bool camera = parent >= 0x08800000u && memory.raw_pointer(address, 0x190u) != nullptr;
    const Rate stick = rate(Source::Stick);
    out << state.frame << ",a0=" << std::hex << parent << ",camera=" << address << std::dec;
    out << ",stick=" << stick.yaw << ':' << stick.pitch;
    if (camera) dump_camera(out, memory, address);
    out << '\n';
    out.flush();
}

// The followed hunter, or 0 when the camera object is not in guest RAM yet.
std::uint32_t followed_hunter(const psprecomp::GuestMemory &memory) {
    return game::layout::followed_hunter(memory, kHunterExtent);
}

// The hunter carries a bow: the game's weapon switch sends its weapon to the
// bow's case. Another overlay at that address, or another weapon, is not.
bool carries_bow(const psprecomp::GuestMemory &memory, std::uint32_t hunter) {
    if (memory.raw_pointer(kWeaponTable, kWeaponTypes * 4u) == nullptr || memory.raw_pointer(kBowCase, 4u) == nullptr ||
        memory.load32(kBowCase) != kBowCaseCall)
        return false;
    const std::uint32_t weapon = memory.load8(hunter + kWeaponType);
    return weapon < kWeaponTypes && memory.load32(kWeaponTable + weapon * 4u) == kBowCase;
}

// The bow's aim, by the conditions 0x09BB6D98 tests before it steps.
bool bow_aiming(const psprecomp::GuestMemory &memory, std::uint32_t hunter) {
    if (hunter == 0u || memory.raw_pointer(hunter, kBowAimBlocked + 4u) == nullptr) return false;
    const auto action = memory.load16(hunter + kAimAction);
    return action >= kBowActions && action < kBowActionsEnd && carries_bow(memory, hunter) &&
        (memory.load32(hunter + kHunterButtons) & kRecenterBit) != 0u &&
        (memory.load32(hunter + kBowAimReady) & kBowAimBit) != 0u &&
        (memory.load32(hunter + kBowAimBlocked) & kBowAimBit) == 0u;
}

bool hunter_aiming(const psprecomp::GuestMemory &memory, std::uint32_t hunter) {
    if (hunter == 0u) return false;
    const auto action = memory.load16(hunter + kAimAction);
    if (action == kAimingAction) {
        // The hunter stays in 1001, and the aim function keeps running, after
        // R is released: the game has dropped the reticle and is gliding the
        // follow camera back by then. Aiming is R held, as for the bow, so the
        // stick goes back to the camera with it. MHFU_CAMERA_AIM_AFTER_R=1
        // counts 1001 as aiming whatever R does, as before.
        static const bool after_r = std::getenv("MHFU_CAMERA_AIM_AFTER_R") != nullptr;
        if (!after_r && (memory.load32(hunter + kHunterButtons) & kRecenterBit) == 0u) return false;
        // Without the wrapper nothing sees the aim function run.
        if (!state.aim_wrapped) return true;
        return state.aim_ran_hunter == hunter && state.aim_ran_frame != 0u &&
            state.frame - state.aim_ran_frame <= kAimFunctionGrace;
    }
    return bow_aiming(memory, hunter);
}

// MHFU_TRACE_AIM=path.csv: one line per aiming camera update with what the
// game stepped and what the driver made of it, and one per direction the
// mouse showed the game. For telling the game's steps from the driver's.
std::ofstream *aim_trace() {
    static const std::filesystem::path path = environment_path("MHFU_TRACE_AIM");
    if (path.empty()) return nullptr;
    static std::ofstream out(path);
    return &out;
}

int load_pitch(const psprecomp::GuestMemory &memory, std::uint32_t hunter, const PitchField &field) {
    return field.halfword ? static_cast<std::int16_t>(memory.load16(hunter + field.offset))
                          : static_cast<std::int8_t>(memory.load8(hunter + field.offset));
}

void store_pitch(psprecomp::GuestMemory &memory, std::uint32_t hunter, const PitchField &field, int value) {
    if (field.halfword)
        memory.store16(hunter + field.offset, static_cast<std::uint16_t>(static_cast<std::int16_t>(value)));
    else
        memory.store8(hunter + field.offset, static_cast<std::uint8_t>(static_cast<std::int8_t>(value)));
}

void remember_aim(const psprecomp::GuestMemory &memory, std::uint32_t hunter) {
    state.aim_hunter = hunter;
    state.aim_heading = memory.load16(hunter + kAimHeading);
    for (std::size_t i = 0; i < kPitchFields.size(); ++i)
        state.aim_pitch[i] = load_pitch(memory, hunter, kPitchFields[i]);
}

// One aim step from a number of degrees, keeping the fraction for the next.
int take_yaw_units(float degrees) {
    state.aim_yaw_remainder += degrees * kAngleUnits;
    const int step = static_cast<int>(state.aim_yaw_remainder);
    state.aim_yaw_remainder -= static_cast<float>(step);
    return step;
}

int take_pitch_units(float degrees, const PitchField &field) {
    const float wanted = state.aim_pitch_remainder + degrees;
    const int step = static_cast<int>(wanted * field.units_per_degree);
    state.aim_pitch_remainder = wanted - static_cast<float>(step) / field.units_per_degree;
    return step;
}

// Writes the aim heading. The body facing's low 16 bits move with it while
// they are still on that heading (or within one aim step of it); a facing the
// game has put somewhere else is left there.
void store_heading(psprecomp::GuestMemory &memory, std::uint32_t hunter, std::uint16_t heading) {
    const auto previous = memory.load16(hunter + kAimHeading);
    memory.store16(hunter + kAimHeading, heading);
    const auto facing = memory.load32(hunter + kFacing);
    const auto body = static_cast<std::uint16_t>(facing);
    const int apart = static_cast<std::int16_t>(static_cast<std::uint16_t>(body - previous));
    if (std::abs(apart) <= kLargestYawStep) memory.store32(hunter + kFacing, (facing & 0xFFFF0000u) | heading);
}

int sign_of(float value) {
    return value > 0.0f ? 1 : (value < 0.0f ? -1 : 0);
}

// The direction the mouse shows the game this frame, each axis -1, 0 or +1:
// every axis with degrees waiting is pushed all the way, so the game steps it
// and the driver sizes the step. A push the game sees as a clear on or off per
// axis is also one whose step can be told in advance.
std::optional<std::pair<int, int>> mouse_direction() {
    if (!game_camera_aim_boost()) return std::nullopt;
    const Turn pending = peek_pointer();
    const float yaw = state.mouse_yaw + pending.yaw_degrees;
    const float pitch = state.mouse_pitch + pending.pitch_degrees;
    const int x = std::fabs(yaw) >= kMouseAimThreshold ? sign_of(yaw) : 0;
    const int y = std::fabs(pitch) >= kMouseAimThreshold ? sign_of(pitch) : 0;
    if (x == 0 && y == 0) return std::nullopt;
    return std::pair{x, y};
}

// Puts back what game_camera_anticipate_aim took off, if it is still there:
// the game has since added its own step to it, or has not run its aim code at
// all. A value the game has set outright (a roll, the aim ending) is left be.
void settle_anticipation(psprecomp::GuestMemory &memory) {
    const std::uint32_t hunter = state.aim_hunter;
    const bool valid = memory.raw_pointer(hunter, kHunterExtent) != nullptr;
    if (valid && state.anticipated_yaw != 0) {
        const auto heading = memory.load16(hunter + kAimHeading);
        const int game = static_cast<std::int16_t>(static_cast<std::uint16_t>(heading - state.anticipated_heading));
        if (std::abs(game) <= kLargestYawStep)
            store_heading(memory, hunter, static_cast<std::uint16_t>(heading + state.anticipated_yaw));
    }
    if (valid && state.anticipated_pitch != 0) {
        const PitchField &field = kPitchFields[state.pitch_field];
        const int value = load_pitch(memory, hunter, field);
        if (std::abs(value - state.anticipated_pitch_value) <= field.largest_step)
            store_pitch(memory, hunter, field, std::clamp(value + state.anticipated_pitch, -field.limit, field.limit));
    }
    state.anticipated_yaw = 0;
    state.anticipated_pitch = 0;
}

void forget_steps(bool scope) {
    state.learned_scoped = scope;
    state.yaw_step = {};
    state.yaw_sign = 0;
    state.pitch_step = {};
    state.pitch_sign = 0;
}

// Learns how the game steps for the direction it was shown: how far, and
// which way for a positive push.
void learn_steps(bool scope, int game_yaw, int game_pitch, std::size_t pitch_field) {
    if (scope != state.learned_scoped) forget_steps(scope);
    const std::size_t diagonal = state.shown_x != 0 && state.shown_y != 0 ? 1u : 0u;
    if (state.shown_x != 0 && game_yaw != 0) {
        state.yaw_step[diagonal] = std::abs(game_yaw);
        state.yaw_sign = (game_yaw > 0 ? 1 : -1) * state.shown_x;
    }
    if (state.shown_y != 0 && game_pitch != 0) {
        state.pitch_field = pitch_field;
        state.pitch_step[diagonal] = std::abs(game_pitch);
        state.pitch_sign = (game_pitch > 0 ? 1 : -1) * state.shown_y;
    }
}

// A bow or a bowgun in the aim action. The right stick, or the mouse, is
// shown to the aim function for that call (aim_step). A stick step is sized
// there, from how far the stick is pushed, before the aim camera reads it;
// this update leaves that step as the wrapper wrote it. The movement stick
// is not that push. With the right stick idle the left stick is what the
// game read, and its three speeds are replaced here by one that follows its
// magnitude. No step from the game -- rolling, firing, a state that locks an
// axis, the D-pad with the stick centred -- means the port adds none of its
// own. A step made for the mouse is sized by the mouse's degrees, and none
// left means the step is taken back.
void drive_aim(psprecomp::GuestMemory &memory, std::uint32_t hunter, std::uint32_t address) {
    state.address = address;
    const int anticipated_yaw = state.anticipated_yaw;
    const int anticipated_pitch = state.anticipated_pitch;
    settle_anticipation(memory);
    if (!driving_allowed() || memory.raw_pointer(hunter, kHunterExtent) == nullptr) {
        release();
        return;
    }
    // The camera and the stick are the game's while aiming.
    state.available = false;
    state.pitch_owned = false;
    state.yaw_remainder = 0.0f;
    state.last_update = state.frame;
    ++state.updates;
    const Turn mouse = take_pointer();
    const Turn turn = take();
    const bool scope = false;
    const int magnitude = static_cast<std::int16_t>(memory.load16(hunter + kStickMagnitude));
    const bool left_held = magnitude >= kStickDeadzone;
    if (!state.aiming || state.aim_hunter != hunter) {
        // The first update of an aim only learns where the aim starts. A
        // step the aim function already sized this frame keeps its fraction.
        const bool sized = state.sized_yaw_frame == state.frame || state.sized_pitch_frame == state.frame;
        const float yaw_remainder = state.aim_yaw_remainder;
        const float pitch_remainder = state.aim_pitch_remainder;
        state.aiming = true;
        state.aim_yaw_remainder = sized ? yaw_remainder : 0.0f;
        state.aim_pitch_remainder = sized ? pitch_remainder : 0.0f;
        state.mouse_yaw = 0.0f;
        state.mouse_pitch = 0.0f;
        state.mouse_yaw_waits = 0u;
        state.mouse_pitch_waits = 0u;
        forget_steps(scope);
        remember_aim(memory, hunter);
        return;
    }
    state.mouse_yaw = std::clamp(state.mouse_yaw + mouse.yaw_degrees, -kMouseAimCarry, kMouseAimCarry);
    state.mouse_pitch = std::clamp(state.mouse_pitch + mouse.pitch_degrees, -kMouseAimCarry, kMouseAimCarry);
    // The game's step answers the mouse only if the stick was idle and the
    // mouse's direction was on the second stick this frame or the last.
    const bool mouse_shown = state.mouse_shown != 0u && state.frame + 1u - state.mouse_shown <= 1u;
    const bool shown_now = state.mouse_shown == state.frame + 1u;
    bool yaw_spent = false;
    bool pitch_spent = false;

    const float carried_yaw = state.mouse_yaw;
    const auto heading = memory.load16(hunter + kAimHeading);
    const int game_yaw = static_cast<std::int16_t>(static_cast<std::uint16_t>(heading - state.aim_heading));
    const bool yaw_sized = state.sized_yaw_frame == state.frame;
    if (!yaw_sized && game_yaw != 0 && std::abs(game_yaw) <= kLargestYawStep &&
        (turn.yaw_held || mouse_shown || left_held)) {
        float degrees = std::fabs(turn.yaw_degrees);
        if (!turn.yaw_held && mouse_shown) {
            degrees = std::fabs(state.mouse_yaw);
            state.mouse_yaw = 0.0f;
            yaw_spent = true;
        } else if (!turn.yaw_held) {
            degrees = static_cast<float>(magnitude) / static_cast<float>(kStickFull) * settings::current().aim_speed *
                advance_seconds();
        }
        const int step = take_yaw_units(degrees);
        store_heading(memory, hunter, static_cast<std::uint16_t>(state.aim_heading + (game_yaw > 0 ? step : -step)));
    } else if (!yaw_sized && game_yaw == 0) {
        state.aim_yaw_remainder = 0.0f;
    }

    int game_pitch_seen = 0;
    std::size_t pitch_field_seen = 0u;
    bool pitch_stepped = false;
    for (std::size_t i = 0; i < kPitchFields.size(); ++i) {
        const PitchField &field = kPitchFields[i];
        const int current = load_pitch(memory, hunter, field);
        const int game_pitch = current - state.aim_pitch[i];
        if (game_pitch == 0 || std::abs(game_pitch) > field.largest_step) continue;
        if (game_pitch_seen == 0) {
            game_pitch_seen = game_pitch;
            pitch_field_seen = i;
        }
        if (state.sized_pitch_frame == state.frame) continue;
        if (!(turn.pitch_held || mouse_shown || left_held)) continue;
        pitch_stepped = true;
        float degrees = std::fabs(turn.pitch_degrees);
        if (!turn.pitch_held && mouse_shown) {
            degrees = std::fabs(state.mouse_pitch);
            pitch_spent = true;
        } else if (!turn.pitch_held) {
            degrees = static_cast<float>(magnitude) / static_cast<float>(kStickFull) * settings::current().aim_speed *
                advance_seconds();
        }
        const int step = take_pitch_units(degrees, field);
        const int next = std::clamp(state.aim_pitch[i] + (game_pitch > 0 ? step : -step), -field.limit, field.limit);
        if (next == field.limit || next == -field.limit) state.aim_pitch_remainder = 0.0f;
        store_pitch(memory, hunter, field, next);
    }
    if (!pitch_stepped && state.sized_pitch_frame != state.frame) state.aim_pitch_remainder = 0.0f;
    // Only a step that answered this frame's direction says how the game
    // answers a direction.
    if (shown_now && !turn.yaw_held && !turn.pitch_held)
        learn_steps(scope, std::abs(game_yaw) <= kLargestYawStep ? game_yaw : 0, game_pitch_seen, pitch_field_seen);
    if (std::ofstream *out = aim_trace()) {
        const int applied = static_cast<std::int16_t>(
            static_cast<std::uint16_t>(memory.load16(hunter + kAimHeading) - state.aim_heading));
        *out << state.frame << ",update,scope=" << scope << ",stick=" << turn.yaw_degrees << ':' << turn.pitch_degrees
             << ",mouse=" << mouse.yaw_degrees << ':' << mouse.pitch_degrees << ",carried=" << carried_yaw
             << ",shown=" << mouse_shown << ",anticipated=" << anticipated_yaw << ':' << anticipated_pitch
             << ",game=" << game_yaw << ':' << game_pitch_seen << ",yaw=" << applied
             << ",pitch=" << load_pitch(memory, hunter, kPitchFields[0]) - state.aim_pitch[0] << '\n';
    }
    if (pitch_spent) state.mouse_pitch = 0.0f;
    // A stick in use owns the aim; otherwise mouse degrees the game has not
    // stepped for in a few updates are dropped, each axis on its own, so an
    // axis the game keeps still does not save up degrees for a jump later.
    if (turn.yaw_held || turn.pitch_held) {
        state.mouse_yaw = 0.0f;
        state.mouse_pitch = 0.0f;
    }
    const auto wait = [](float &degrees, unsigned &waits, bool spent) {
        if (spent || degrees == 0.0f) {
            waits = 0u;
        } else if (++waits > kMouseAimWaits) {
            degrees = 0.0f;
            waits = 0u;
        }
    };
    wait(state.mouse_yaw, state.mouse_yaw_waits, yaw_spent);
    wait(state.mouse_pitch, state.mouse_pitch_waits, pitch_spent);
    remember_aim(memory, hunter);
}

// The aim camera reads the heading after this function returns and before the
// follow camera, which may not run at all while aiming. A stick step is
// replaced here, from the deflection present_aim stored, so that read sees
// Aim speed times how far the stick is pushed. A scale of zero (the mouse)
// is left for the follow camera. No step from the game is left as it is.
void resize_shown_aim(psprecomp::GuestMemory &memory, std::uint32_t hunter, std::uint16_t heading_before,
    const std::array<int, kPitchFields.size()> &pitch_before) {
    const float seconds = advance_seconds();
    const float speed = settings::current().aim_speed;
    if (state.aim_scale_x != 0.0f) {
        const auto heading = memory.load16(hunter + kAimHeading);
        const int game_yaw = static_cast<std::int16_t>(static_cast<std::uint16_t>(heading - heading_before));
        if (game_yaw != 0 && std::abs(game_yaw) <= kLargestYawStep) {
            const float degrees = std::fabs(state.aim_scale_x) * speed * seconds;
            const int step = take_yaw_units(degrees);
            store_heading(memory, hunter, static_cast<std::uint16_t>(heading_before + (game_yaw > 0 ? step : -step)));
            state.sized_yaw_frame = state.frame;
        }
    }
    if (state.aim_scale_y != 0.0f) {
        for (std::size_t i = 0; i < kPitchFields.size(); ++i) {
            const PitchField &field = kPitchFields[i];
            const int current = load_pitch(memory, hunter, field);
            const int game_pitch = current - pitch_before[i];
            if (game_pitch == 0 || std::abs(game_pitch) > field.largest_step) continue;
            const float degrees = std::fabs(state.aim_scale_y) * speed * seconds;
            const int step = take_pitch_units(degrees, field);
            const int next = std::clamp(pitch_before[i] + (game_pitch > 0 ? step : -step), -field.limit, field.limit);
            if (next == field.limit || next == -field.limit) state.aim_pitch_remainder = 0.0f;
            store_pitch(memory, hunter, field, next);
            state.sized_pitch_frame = state.frame;
        }
    }
}

// Shows aim_x/aim_y to an aim function, then puts the hunter's stick back,
// so walking still follows the stick the player is holding.
void show_aim(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx, RotationFunction original) {
    auto &memory = runtime.memory();
    const auto hunter = ctx.gpr[4];
    // Only while R, the aim button, is held as the game reads it. The game
    // stays in the aim for as long as the aim function sees a direction, so a
    // push shown after R was released kept the hunter aiming, and the stick
    // off the camera, for as long as the stick moved.
    // MHFU_CAMERA_AIM_AFTER_R=1 shows it whatever R does, as before.
    static const bool after_r = std::getenv("MHFU_CAMERA_AIM_AFTER_R") != nullptr;
    const bool show = (state.aim_x != 0 || state.aim_y != 0) && memory.raw_pointer(hunter, kHunterExtent) != nullptr &&
        (after_r || (memory.load32(hunter + kHunterButtons) & kRecenterBit) != 0u);
    std::uint32_t saved_buttons = 0u;
    std::uint16_t saved_magnitude = 0u;
    std::uint16_t heading_before = 0u;
    std::array<int, kPitchFields.size()> pitch_before{};
    if (show) {
        ++state.aim_pushes;
        saved_buttons = memory.load32(hunter + kHunterButtons);
        saved_magnitude = memory.load16(hunter + kStickMagnitude);
        heading_before = memory.load16(hunter + kAimHeading);
        for (std::size_t i = 0; i < kPitchFields.size(); ++i)
            pitch_before[i] = load_pitch(memory, hunter, kPitchFields[i]);
        // On the stick half and on the D-pad bits: the function picks its
        // branch as it runs (see kStickDirectionMask), and either one then
        // sees this push.
        std::uint32_t direction = 0u;
        if (state.aim_x > 0)
            direction |= kStickRight | kDpadRight;
        else if (state.aim_x < 0)
            direction |= kStickLeft | kDpadLeft;
        if (state.aim_y < 0)
            direction |= kStickUp | kDpadUp;
        else if (state.aim_y > 0)
            direction |= kStickDown | kDpadDown;
        const auto clear = kStickDirectionMask | kDpadMask;
        memory.store32(hunter + kHunterButtons, (saved_buttons & ~clear) | direction);
        memory.store16(hunter + kStickMagnitude, static_cast<std::uint16_t>(kStickFull));
    }
    if (original != nullptr) original(runtime, ctx);
    if (show && memory.raw_pointer(hunter, kHunterExtent) != nullptr) {
        memory.store32(hunter + kHunterButtons, saved_buttons);
        memory.store16(hunter + kStickMagnitude, saved_magnitude);
        resize_shown_aim(memory, hunter, heading_before, pitch_before);
    }
}

void aim_step(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    ++state.aim_calls;
    const auto hunter = ctx.gpr[4];
    const auto &memory = runtime.memory();
    if (memory.raw_pointer(hunter, kHunterExtent) != nullptr && memory.load16(hunter + kAimAction) == kAimingAction) {
        state.aim_ran_frame = state.frame;
        state.aim_ran_hunter = hunter;
    }
    show_aim(runtime, ctx, state.aim_original);
}

// The aim function lives in an overlay, so its wrapper is put back whenever
// that overlay is installed again. A different image at the same address is
// left alone.
template <std::size_t N>
void install_one(psprecomp::Runtime &runtime, std::uint32_t address, const std::array<CodeWord, N> &signature,
    RotationFunction wrapper, RotationFunction &original, const char *name) {
    const RotationFunction current = runtime.function_at(address);
    if (current == nullptr || current == wrapper) return;
    const auto &memory = runtime.memory();
    for (const CodeWord &expected : signature)
        if (memory.load32(expected.address) != expected.word) return;
    original = current;
    runtime.register_function(address, wrapper, name);
    if (aim_trace() != nullptr)
        std::cout << "[aim] wrapper installed at 0x" << std::hex << address << std::dec << std::endl;
}

void install_aim_hook(psprecomp::Runtime &runtime) {
    if (option_on()) install_one(runtime, kAimFunction, kAimSignature, &aim_step, state.aim_original, "mhfu_aim_step");
    state.aim_wrapped = runtime.function_at(kAimFunction) == &aim_step;
}

// MHFU_TRACE_WEAPON=1: on each press of R (guard, aim, draw), the followed
// hunter and the played hunter's record as hex, to find where the weapon
// class and the drawn state are kept. R is 0x0200 in the hunter's button word.
void trace_weapon(const psprecomp::GuestMemory &memory) {
    static const bool enabled = std::getenv("MHFU_TRACE_WEAPON") != nullptr;
    if (!enabled) return;
    static bool r_was_down = false;
    const std::uint32_t hunter = followed_hunter(memory);
    const bool r_down = hunter != 0u && (memory.load32(hunter + kHunterButtons) & kRecenterBit) != 0u;
    const bool pressed = r_down && !r_was_down;
    r_was_down = r_down;
    if (!pressed) return;
    const auto hex = [&](std::uint32_t from, std::uint32_t bytes) {
        std::ostringstream out;
        out << std::hex << std::setfill('0');
        for (std::uint32_t i = 0; i < bytes; ++i) out << std::setw(2) << static_cast<unsigned>(memory.load8(from + i));
        return out.str();
    };
    std::cout << "[weapon] frame=" << state.frame << " hunter=0x" << std::hex << hunter << std::dec
              << " entity=" << hex(hunter, kHunterExtent);
    namespace layered = game::layered;
    const std::uint32_t block =
        memory.raw_pointer(layered::kGameBlockPointer, 4u) != nullptr ? memory.load32(layered::kGameBlockPointer) : 0u;
    const std::uint32_t record = block + layered::kHunterRecords;
    if (block != 0u && memory.raw_pointer(record, layered::kHunterRecordBytes) != nullptr)
        std::cout << " record=" << hex(record, layered::kHunterRecordBytes);
    std::cout << std::endl;
}

// MHFU_TRACE_AIM: one line when an aim starts or ends, and one a second while
// it lasts, with each condition the right stick's aim depends on. The stick
// reaches the aim function only while `boost` is 1 and `driving` is 0.
void trace_aim_state(psprecomp::Runtime &runtime, bool aiming) {
    if (aim_trace() == nullptr) return;
    // The hunter's action whenever R is held and it changes, aiming or not:
    // which actions a weapon aims in.
    static std::uint16_t last_action = 0xFFFFu;
    const std::uint32_t hunter = followed_hunter(runtime.memory());
    if (hunter != 0u && (runtime.memory().load32(hunter + kHunterButtons) & kRecenterBit) != 0u) {
        const auto action = runtime.memory().load16(hunter + kAimAction);
        if (action != last_action)
            std::cout << "[aim] frame=" << state.frame << " R held, action " << action << std::endl;
        last_action = action;
    } else {
        last_action = 0xFFFFu;
    }
    static bool was_aiming = false;
    static std::uint64_t last_line = 0u;
    const bool changed = aiming != was_aiming;
    was_aiming = aiming;
    if (!changed && !(aiming && state.frame - last_line >= 30u)) return;
    last_line = state.frame;
    std::cout << "[aim] frame=" << state.frame << (changed ? (aiming ? " start" : " end") : " held")
              << " wrapper=" << (runtime.function_at(kAimFunction) == &aim_step ? 1 : 0)
              << " bow=" << (state.bow_aiming ? 1 : 0) << " option=" << (option_on() ? 1 : 0)
              << " hooked=" << (state.hooked ? 1 : 0) << " available=" << (state.available ? 1 : 0)
              << " lock_on_drives=" << (lock_on_drives_camera() ? 1 : 0)
              << " driving=" << (game_camera_driving() ? 1 : 0) << " boost=" << (game_camera_aim_boost() ? 1 : 0)
              << " update_age=" << (state.frame - state.last_update) << " calls=" << state.aim_calls
              << " pushes=" << state.aim_pushes << std::endl;
    state.aim_calls = 0u;
    state.aim_pushes = 0u;
}

// L's recenter writes the hunter's facing plus half a turn to the target yaw
// and sets the snap byte (0x08887508). Within 22.5 degrees the same update
// clears that byte (0x08887650). Farther away the filter steps the current
// yaw a quarter of the way (0x08887624) and the eye is placed from it before
// this update returns. That is one frame of the camera jumping behind the
// hunter: when the hunter is off at the side of the picture, the eye lands
// inside the hunter or a wall and the frame is black. Focus puts the yaw,
// the eye and the snap's countdown back before the update returns. A target
// already holding that yaw is moved by one unit first, so the write shows,
// and put back when none came.
struct RecenterWatch {
    std::uint32_t address{};
    std::uint16_t behind{};
    std::uint16_t before{};
    std::uint16_t current{};
    float eye_x{};
    float eye_y{};
    float eye_z{};
    std::uint8_t snap_count{};
    bool nudged{};
};

std::optional<RecenterWatch> watch_recenter(
    psprecomp::GuestMemory &memory, std::uint32_t address, std::uint32_t hunter) {
    if (hunter == 0u || !lock_on_allowed() || settings::current().lock_on_style != settings::LockOnStyle::Focus ||
        !lock_on_status().locked)
        return std::nullopt;
    if (memory.load8(address + kSnap) != 0u || static_cast<std::int8_t>(memory.load8(address + kAim)) >= 0 ||
        (memory.load16(address + kButtons) & kTurnBits) != 0u)
        return std::nullopt;
    RecenterWatch watch{address, static_cast<std::uint16_t>(memory.load32(hunter + kFacing) + 0x8000u),
        memory.load16(address + kYawTarget), memory.load16(address + kYawCurrent), load_float(memory, address),
        load_float(memory, address + kEyeY), load_float(memory, address + 8u), memory.load8(address + kSnapCount),
        false};
    watch.nudged = watch.before == watch.behind;
    if (watch.nudged) memory.store16(address + kYawTarget, static_cast<std::uint16_t>(watch.before + 1u));
    return watch;
}

// The game has already placed the eye from the snapped yaw. Put the pose
// back so the rest of the frame draws the camera where it was.
void undo_recenter(psprecomp::GuestMemory &memory, const RecenterWatch &watch) {
    memory.store16(watch.address + kYawTarget, watch.before);
    memory.store16(watch.address + kYawCurrent, watch.current);
    store_float(memory, watch.address, watch.eye_x);
    store_float(memory, watch.address + kEyeY, watch.eye_y);
    store_float(memory, watch.address + 8u, watch.eye_z);
    memory.store8(watch.address + kSnap, 0u);
    memory.store8(watch.address + kSnapCount, watch.snap_count);
}

void check_recenter(psprecomp::GuestMemory &memory, const RecenterWatch &watch) {
    const auto after = memory.load16(watch.address + kYawTarget);
    if (after != watch.behind) {
        if (watch.nudged && after == static_cast<std::uint16_t>(watch.before + 1u))
            memory.store16(watch.address + kYawTarget, watch.before);
        return;
    }
    // The press that just started the turn also opens the double-tap window,
    // so that window is not this snap. Put the game's pose back. A held L,
    // after the turn has arrived, would start another one if it were remembered.
    if (lock_on_drives_camera() || state.quiet_l) {
        undo_recenter(memory, watch);
        return;
    }
    // A second L within a third of a second of the focus tap. The game has
    // placed the camera behind the hunter; leave that pose.
    if (lock_on_focus_double_tap()) {
        lock_on_accept_double_tap();
        return;
    }
    if (lock_on_yielding_recenter()) return;
    // Put the pose back so this frame never shows the jump behind the hunter.
    // The next update eases toward the monster from the camera where it was,
    // including when the monster is already on screen.
    undo_recenter(memory, watch);
    state.recenter_dropped = true;
}

// P lines of MHFU_TRACE_CAMERA_STATE, after a follow update and its
// collision: the eye and the point looked at, how far apart they are and how
// far the preset wanted them, and the yaw. A wall that flips the camera in and
// out shows as that distance jumping between updates.
void trace_after(const psprecomp::GuestMemory &memory, std::uint32_t address) {
    std::ofstream *out = state_trace();
    if (out == nullptr) return;
    float eye[3];
    float at[3];
    for (std::uint32_t i = 0; i < 3u; ++i) {
        eye[i] = load_float(memory, address + i * 4u);
        at[i] = load_float(memory, address + 0x10u + i * 4u);
    }
    const auto preset = memory.load32(address + kPreset);
    const float wanted = memory.raw_pointer(preset, 0x0Cu) != nullptr
        ? std::hypot(load_float(memory, preset + kPresetHeight), load_float(memory, preset + kPresetDistance))
        : 0.0f;
    const float dx = eye[0] - at[0];
    const float dy = eye[1] - at[1];
    const float dz = eye[2] - at[2];
    *out << "P," << state.frame << ',' << eye[0] << ',' << eye[1] << ',' << eye[2] << ',' << at[0] << ',' << at[1]
         << ',' << at[2] << ',' << std::sqrt(dx * dx + dy * dy + dz * dz) << ',' << std::hypot(dx, dz) << ',' << wanted
         << ',' << memory.load16(address + kYawCurrent) << '\n';
}

void pull_in_choice(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    static const bool slide = std::getenv("MHFU_CAMERA_WALL_SLIDE") != nullptr;
    if (!slide && driving_allowed()) {
        ctx.gpr[2] = 1u;
        ctx.pc = ctx.gpr[31];
        return;
    }
    state.pull_in_original(runtime, ctx);
}

void camera_rotation(psprecomp::Runtime &runtime, psprecomp::AllegrexContext &ctx) {
    state.facing_substituted = false;
    state.quiet_l = false;
    state.quiet_follow = false;
    trace_modes(runtime.memory(), ctx);
    auto &memory = runtime.memory();
    const auto parent = ctx.gpr[4];
    const auto address = parent + kCameraFromParent;
    const bool camera = parent >= 0x08800000u && memory.raw_pointer(address, 0x98u) != nullptr;
    const auto hunter = followed_hunter(memory);
    std::optional<RecenterWatch> watch;
    std::optional<DpadMute> dpad;
    std::uint8_t saved_gate = 0u;
    bool restore_gate = false;
    bool follow = false;
    bool fresh_wall = false;
    if (hunter_aiming(memory, hunter)) {
        drive_aim(memory, hunter, camera ? address : 0u);
    } else if (camera) {
        unease_wall(memory, address);
        // A camera that was not updated for a while starts its distance anew.
        fresh_wall = state.address != address || state.frame - state.last_update > 2u;
        dpad = mute_dpad(memory, address);
        drive_follow(memory, address);
        follow = true;
        // The facing stands in for the monster, so the game's recenter is the
        // turn. Watching it would put that turn back. With Focus camera bound
        // to something else, L is the game's recenter behind the hunter and
        // must be left there. While Focus owns L, a non-negative gate would
        // also step toward the hunter; hold it negative for this update only.
        if ((state.quiet_l || state.quiet_follow) && static_cast<std::int8_t>(memory.load8(address + kAim)) >= 0) {
            saved_gate = memory.load8(address + kAim);
            memory.store8(address + kAim, 0xFFu);
            restore_gate = true;
        }
        // A shared trigger is not "focus held": L's recenter is left where the
        // game put it. Holding LT must not put that recenter back.
        const bool focus_held = focus_bind_held_now && !lock_on_trigger_initiates_camera();
        const bool focus_elsewhere = focus_camera_is_bound() && !focus_held;
        if (!state.facing_substituted && !focus_elsewhere) watch = watch_recenter(memory, address, hunter);
    } else if (state.aiming) {
        release();
    }
    // Continue the original update with the same CPU context and return PC.
    // Do not invoke an isolated guest call: normal scheduling must be retained.
    state.original(runtime, ctx);
    if (dpad) unmute_dpad(memory, *dpad);
    if (restore_gate) memory.store8(address + kAim, saved_gate);
    if (watch) check_recenter(memory, *watch);
    restore_preset(memory);
    if (follow) ease_wall(memory, address, fresh_wall);
    if (follow) trace_after(memory, address);
    if (state.facing_substituted) {
        state.facing_substituted = false;
        if (hunter != 0u && memory.raw_pointer(hunter + kFacing, 4u) != nullptr)
            memory.store32(hunter + kFacing, state.saved_facing);
    }
}

} // namespace

void set_focus_camera_held(bool held) {
    focus_bind_held_now = held;
}

std::span<const CodeWord> game_code_signature() {
    return kSignature;
}

std::span<const CodeWord> aim_code_signature() {
    return kAimSignature;
}

bool game_camera_hunter_aiming() {
    return state.hunter_aiming;
}

std::uint32_t game_camera_aim_dpad(int x, int y) {
    if (!driving_allowed() || !state.bow_aiming) return 0u;
    std::uint32_t dpad = 0u;
    if (x >= kBowDpadThreshold)
        dpad |= kDpadRight;
    else if (x <= -kBowDpadThreshold)
        dpad |= kDpadLeft;
    if (y <= -kBowDpadThreshold)
        dpad |= kDpadUp;
    else if (y >= kBowDpadThreshold)
        dpad |= kDpadDown;
    return dpad;
}

bool prepare_game_camera(psprecomp::Runtime &runtime, RotationFunction original) {
    state = State{};
    reset();
    lock_on_reset();
    if (!original || !runtime.has_function(kCameraUpdate)) {
        std::cerr << "[camera] the camera update is not in the generated code; analog camera unavailable\n";
        return false;
    }
    for (const CodeWord &expected : kSignature) {
        const std::uint32_t found = runtime.memory().load32(expected.address);
        if (found == expected.word) continue;
        std::cerr << "[camera] game code differs at 0x" << std::hex << std::uppercase << std::setfill('0')
                  << std::setw(8) << expected.address << ": 0x" << std::setw(8) << found << ", expected 0x"
                  << std::setw(8) << expected.word << std::dec << std::nouppercase << std::setfill(' ') << " ("
                  << expected.what << "); analog camera unavailable\n";
        return false;
    }
    state.original = original;
    state.prepared = true;
    return true;
}

void game_camera_frame(psprecomp::Runtime &runtime) {
    ++state.frame;
    trace_flip(runtime.memory());
    // A step taken off in advance that no aim update has put back: the aim
    // code and the camera did not run, so it goes back now.
    settle_anticipation(runtime.memory());
    // A lock that is on, or a tap waiting, needs the hook as well; a player
    // who never taps pays nothing for lock-on either.
    if (!lock_on_allowed()) lock_on_release("");
    lock_on_frame(runtime.memory());
    if (state.prepared && !state.hooked && (option_on() || pose_wanted || (lock_on_allowed() && lock_on_wanted()))) {
        // Only ever at the game's flip, from an import: no generated frame is
        // live on the host stack, so the dispatch tables can change here.
        // A host registration disables this unit's direct-call shortcut, so the
        // generated cross-unit camera call reaches the wrapper without
        // regeneration. It stays until exit; with the option off again the
        // wrapper only passes through.
        runtime.register_function(kCameraUpdate, &camera_rotation, "mhfu_camera_update");
        if (const RotationFunction choice = runtime.function_at(kPullInChoice); choice != nullptr) {
            state.pull_in_original = choice;
            runtime.register_function(kPullInChoice, &pull_in_choice, "mhfu_camera_pull_in");
        }
        state.hooked = true;
    }
    install_aim_hook(runtime);
    // The aim camera replaces the follow camera, so this update can stop
    // while the hunter is still aiming. Releasing then would drop the aim
    // step's fraction every frame.
    const bool still_aiming = hunter_aiming(runtime.memory(), followed_hunter(runtime.memory()));
    if (state.frame - state.last_update > 1u && !still_aiming) release();
    state.hunter_aiming = still_aiming;
    state.bow_aiming = bow_aiming(runtime.memory(), followed_hunter(runtime.memory()));
    trace_aim_state(runtime, still_aiming);
    trace_weapon(runtime.memory());
    if (!driving_allowed() && !(state.pitch_by_lock && lock_on_drives_camera())) {
        state.pitch_owned = false;
        state.yaw_remainder = 0.0f;
    }
    // Nothing takes the input while the camera update is not running.
    if (!game_camera_driving()) discard();
    // F, frame, the stick's rate, the stick kept, the hunter aiming, the last
    // follow update: for telling a stick at rest from input that is dropped.
    if (std::ofstream *out = state_trace()) {
        const Rate stick = rate(Source::Stick);
        *out << "F," << state.frame << ',' << stick.yaw << ',' << stick.pitch << ',' << game_camera_driving() << ','
             << state.hunter_aiming << ',' << state.aiming << ',' << state.last_update;
        // The hunter's R as the game reads it, and the action: an aim that
        // outlasts R shows here.
        const std::uint32_t hunter = followed_hunter(runtime.memory());
        if (hunter != 0u && runtime.memory().raw_pointer(hunter, kHunterExtent) != nullptr)
            *out << ',' << ((runtime.memory().load32(hunter + kHunterButtons) & kRecenterBit) != 0u) << ','
                 << runtime.memory().load16(hunter + kAimAction);
        *out << '\n';
    }
}

std::optional<CameraPose> game_camera_pose(const psprecomp::GuestMemory &memory) {
    if (!state.hooked || state.address == 0u || state.frame - state.last_update > 2u ||
        memory.raw_pointer(state.address, 0x1Cu) == nullptr)
        return std::nullopt;
    CameraPose pose;
    for (std::uint32_t axis = 0; axis < 3u; ++axis) {
        pose.eye[axis] = load_float(memory, state.address + axis * 4u);
        pose.look_at[axis] = load_float(memory, state.address + 0x10u + axis * 4u);
        if (!std::isfinite(pose.eye[axis]) || !std::isfinite(pose.look_at[axis])) return std::nullopt;
    }
    return pose;
}

void game_camera_want_pose(bool wanted) {
    pose_wanted = wanted;
}

bool game_camera_driving() {
    return (driving_allowed() || (state.hooked && lock_on_drives_camera())) && state.available;
}

bool game_camera_aim_boost() {
    return driving_allowed() && state.hunter_aiming;
}

void game_camera_present_aim(int x, int y, float scale_x, float scale_y) {
    state.aim_x = x;
    state.aim_y = y;
    state.aim_scale_x = std::clamp(scale_x, -1.0f, 1.0f);
    state.aim_scale_y = std::clamp(scale_y, -1.0f, 1.0f);
}

std::optional<StickDirection> game_camera_mouse_aim() {
    const auto direction = mouse_direction();
    if (!direction) return std::nullopt;
    const auto [x, y] = *direction;
    if (std::ofstream *out = aim_trace(); out != nullptr && state.mouse_shown != state.frame + 1u)
        *out << state.frame << ",show," << x << ':' << y << '\n';
    state.mouse_shown = state.frame + 1u;
    state.shown_x = x;
    state.shown_y = y;
    const float length = std::hypot(static_cast<float>(x), static_cast<float>(y));
    return StickDirection{static_cast<float>(x) / length, static_cast<float>(y) / length};
}

void game_camera_anticipate_aim(psprecomp::Runtime &runtime) {
    auto &memory = runtime.memory();
    settle_anticipation(memory);
    const auto direction = mouse_direction();
    const std::uint32_t hunter = state.aim_hunter;
    if (!direction || memory.raw_pointer(hunter, kHunterExtent) == nullptr) return;
    // A held stick is shown to the game instead of the mouse.
    for (const Source source : {Source::Stick, Source::Keys, Source::Touch}) {
        const Rate held = rate(source);
        if (held.yaw != 0.0f || held.pitch != 0.0f) return;
    }
    const auto [x, y] = *direction;
    const std::size_t diagonal = x != 0 && y != 0 ? 1u : 0u;
    // A step size not seen yet in this aim is taken from the other one: the
    // game's diagonal steps are its straight ones times cos 45 degrees.
    const auto step_for = [diagonal](const std::array<int, 2> &steps) {
        if (steps[diagonal] != 0) return steps[diagonal];
        constexpr float kDiagonal = 0.70710678f;
        const float other = static_cast<float>(steps[1u - diagonal]);
        return static_cast<int>(std::lround(diagonal != 0u ? other * kDiagonal : other / kDiagonal));
    };
    const auto heading = memory.load16(hunter + kAimHeading);
    const int yaw_step = step_for(state.yaw_step);
    if (x != 0 && state.yaw_sign != 0 && yaw_step != 0 && heading == state.aim_heading) {
        state.anticipated_yaw = state.yaw_sign * x * yaw_step;
        state.anticipated_heading = static_cast<std::uint16_t>(heading - state.anticipated_yaw);
        store_heading(memory, hunter, state.anticipated_heading);
    }
    const PitchField &field = kPitchFields[state.pitch_field];
    const int pitch = load_pitch(memory, hunter, field);
    const int pitch_step = step_for(state.pitch_step);
    if (y != 0 && state.pitch_sign != 0 && pitch_step != 0 && pitch == state.aim_pitch[state.pitch_field]) {
        const int ahead = std::clamp(pitch - state.pitch_sign * y * pitch_step, -field.limit, field.limit);
        state.anticipated_pitch = pitch - ahead;
        state.anticipated_pitch_value = ahead;
        if (state.anticipated_pitch != 0) store_pitch(memory, hunter, field, ahead);
    }
}

int game_camera_mouse_stock_turn() {
    if (game_camera_driving() || game_camera_aim_boost()) return 0;
    const float yaw = peek_pointer().yaw_degrees;
    if (!(std::fabs(yaw) >= kMouseStockTurn)) return 0;
    return yaw > 0.0f ? 1 : -1;
}

float game_camera_degrees_per_second() {
    const auto &s = settings::current();
    return state.aiming ? s.aim_speed : s.camera_speed;
}

} // namespace mhfu::camera
