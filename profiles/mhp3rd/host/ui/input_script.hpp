#pragma once

// MHP3RD_INPUT_SCRIPT: scripted input for testing the interface without a
// person at the controls, the way MHP3RD_AUTO_CONFIRM walks through the game.
//
// The script is a list of `frame:action` steps separated by semicolons. The
// frame counts window-event pumps: one per game frame while the game runs, one
// per interface frame while a screen is up. Actions:
//
//   key NAME          press and release a key (SDL key names: Escape, Down, Return, Q)
//   pad BUTTON[+...]  press and release buttons of a virtual gamepad (SDL names:
//                     a, b, x, y, start, leftstick, rightstick, leftshoulder,
//                     dpup, dpdown, dpleft, dpright, ...)
//   axis NAME VALUE   hold an axis of the virtual gamepad at VALUE, -1 to 1
//                     (leftx, lefty, rightx, righty, lefttrigger, righttrigger)
//   text STRING       type text
//   drop PATH         drop a file onto the window
//   shot NAME         write the window image to MHP3RD_SCREENSHOT_DIR/NAME.bmp
//   quit              close the window
//
// For example: MHP3RD_INPUT_SCRIPT="300:key Escape;330:shot menu;360:pad leftstick+rightstick"
//
// MHP3RD_INPUT_LIVE names a file read while the game runs: each line appended
// to it is one step, and its frame counts from when the line is read, so
// `echo "0:shot now" >> file` captures the window within a few frames. It
// always connects the virtual pad. Used to drive two instances side by side,
// for example in ad hoc tests.
namespace mhp3rd::ui::script {

// Reads the script and, if it presses gamepad buttons, connects the virtual pad.
void attach();
// Runs the steps due at this frame. Called before each pump.
void tick();

} // namespace mhp3rd::ui::script
