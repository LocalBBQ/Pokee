// Include the vendor implementation to exercise its private debug formatter.
#include "../third_party/imgui/imgui.cpp"

#include <cstring>
#include <iostream>

int main() {
    char buffer[64]{};
    FormatTextureRefForDebugDisplay(buffer, sizeof(buffer), ImTextureRef(ImTextureID(0x123456789ABCDEF0ULL)));
    if (std::strcmp(buffer, "0x123456789ABCDEF0") != 0) {
        std::cerr << "64-bit texture ID was truncated: " << buffer << '\n';
        return 1;
    }
    FormatTextureRefForDebugDisplay(buffer, sizeof(buffer), ImTextureRef(ImTextureID(0x1234ULL)));
    if (std::strcmp(buffer, "0x1234") != 0) return 1;
    char small[5]{};
    FormatTextureRefForDebugDisplay(small, sizeof(small), ImTextureRef(ImTextureID(0x123456789ABCDEF0ULL)));
    if (small[4] != '\0') return 1;
    std::cout << "ImGui debug texture format tests passed\n";
    return 0;
}
