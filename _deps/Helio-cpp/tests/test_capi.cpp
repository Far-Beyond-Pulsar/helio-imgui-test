/// Minimal smoke test — verifies that the C API header compiles and all
/// symbol declarations are correct.  Actual linking requires the built
/// helio_ffi static library.
#include <helio/helio_capi.h>
#include <cstdio>
#include <cassert>

int main() {
    // Struct sizes (repr(C) must match Rust)
    assert(sizeof(HelioHandle) == 8);
    assert(sizeof(HelioVec3) == 12);
    assert(sizeof(HelioMat4) == 64);
    assert(sizeof(HelioGpuLight) % 16 == 0);
    assert(sizeof(HelioGpuMaterial) % 16 == 0);
    assert(sizeof(HelioGpuDecal) % 16 == 0);

    // Handle null helpers
    HelioHandle h = helio_handle_null();
    assert(helio_handle_is_null(h));

    // Camera helpers can be called at runtime
    printf("sizeof(HelioCameraDesc) = %zu\n", sizeof(HelioCameraDesc));
    printf("sizeof(HelioRendererConfig) = %zu\n", sizeof(HelioRendererConfig));
    printf("Helio-cpp C API header compiles OK.\n");
    return 0;
}
