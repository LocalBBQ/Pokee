#include "game/hooks.hpp"

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace mhfu::game::hooks {
namespace {

using Function = psprecomp::Runtime::RecompiledFunction;

// Per address, the wrappers registered over the function found there,
// innermost first.
std::unordered_map<std::uint32_t, std::vector<Function>> &chains() {
    static std::unordered_map<std::uint32_t, std::vector<Function>> chains;
    return chains;
}

} // namespace

bool wrap(psprecomp::Runtime &runtime, std::uint32_t address, Function wrapper, Function &next, const std::string &name,
    bool chained) {
    const Function current = runtime.function_at(address);
    if (current == nullptr) return false;
    std::vector<Function> &chain = chains()[address];
    if (std::find(chain.begin(), chain.end(), current) == chain.end()) chain.clear();
    if (std::find(chain.begin(), chain.end(), wrapper) != chain.end()) return true;
    next = current;
    if (chained) register_chained(runtime, address, wrapper, name);
    else runtime.register_function(address, wrapper, name);
    chain.push_back(wrapper);
    return true;
}

} // namespace mhfu::game::hooks
