// Picks the installer's front end.

#include "install/installer.hpp"

#include <iostream>

namespace mhp3rd::install {

std::unique_ptr<InstallerUi> make_installer_ui() { return make_dialog_ui(); }

bool report_problem(const std::string &title, const std::string &message, bool ask_setup) {
    std::cerr << title << ": " << message << "\n";
    return report_problem_in_dialog(title, message, ask_setup);
}

} // namespace mhp3rd::install
