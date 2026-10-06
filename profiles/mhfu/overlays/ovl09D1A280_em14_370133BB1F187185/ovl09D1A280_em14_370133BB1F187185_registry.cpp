#include "psprecomp/runtime.hpp"
#include <cstdint>

namespace psprecomp {
void register_ovl09D1A280_em14_370133BB1F187185_unit_0(Runtime &runtime);
void register_ovl09D1A280_em14_370133BB1F187185_unit_1(Runtime &runtime);
void register_ovl09D1A280_em14_370133BB1F187185_unit_2(Runtime &runtime);
void register_ovl09D1A280_em14_370133BB1F187185_unit_3(Runtime &runtime);

void register_ovl09D1A280_em14_370133BB1F187185_functions(Runtime &runtime) {
    register_ovl09D1A280_em14_370133BB1F187185_unit_0(runtime);
    register_ovl09D1A280_em14_370133BB1F187185_unit_1(runtime);
    register_ovl09D1A280_em14_370133BB1F187185_unit_2(runtime);
    register_ovl09D1A280_em14_370133BB1F187185_unit_3(runtime);
}
} // namespace psprecomp
