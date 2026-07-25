# Helio-cpp

C API header and C++ RAII wrappers for the [Helio](https://github.com/Far-Beyond-Pulsar/Helio) renderer engine.

This is a **header-only** C++ library that wraps the C FFI exported by the [`helio-ffi`](https://github.com/Far-Beyond-Pulsar/helio-ffi) Rust crate.

## Prerequisites

- A C++17 compiler
- CMake 3.20+ (optional, for the test targets)
- The [`helio-ffi`](https://github.com/Far-Beyond-Pulsar/helio-ffi) static library built somewhere on your system

## Quick start (without CMake)

```cpp
#include <helio/helio_capi.h>   // C API (pure C, no stdlib dependency)
#include <helio/helio.hpp>       // C++ RAII wrappers (optional)

// Link against libhelio_ffi.a / helio_ffi.lib
// Build with:  g++ -std=c++17 -I/path/to/Helio-cpp/include main.cpp -lhelio_ffi
```

## Using CMake

```cmake
# In your CMakeLists.txt:
add_subdirectory(path/to/Helio-cpp)
target_link_libraries(your_app PRIVATE helio)

# Point CMake to the helio_ffi library for test linking:
cmake -B build -DHELIO_FFI_LIB=/path/to/libhelio_ffi.a
```

## File layout

```
include/helio/
├── helio_capi.h     # Pure C FFI declarations (structs + extern "C" functions)
└── helio.hpp        # C++ RAII wrappers (Scene, Renderer, Editor, Picker, etc.)
tests/
├── test_capi.cpp    # Compilation/layout smoke test for the C header
└── test_cpp.cpp     # Compilation smoke test for the C++ header
```

## C API (`helio_capi.h`)

Declares `extern "C"` functions matching every symbol exported by `helio-ffi`. All types are `#[repr(C)]`-compatible with the Rust side. See the `helio-ffi` README for the full function listing.

## C++ RAII wrappers (`helio.hpp`)

| Class | Wraps | Key methods |
|-------|-------|-------------|
| `helio::Handle` | `HelioHandle` | `is_null()`, equality, `null()` |
| `helio::MeshUpload` | `HelioMeshUploadPtr` | `build()`, move-only |
| `helio::Scene` | `HelioScenePtr` | Scene mutation: meshes, materials, lights, objects, decals, water, voxels, virtual geo |
| `helio::Renderer` | `HelioRendererPtr` | `render()`, config, debug draw |
| `helio::Editor` | `HelioEditorPtr` | Gizmo selection/dragging |
| `helio::Picker` | `HelioPickerPtr` | `register_mesh()`, `cast_ray()` |

All classes are **move-only** (no copy) — ownership of the underlying opaque pointer is transferred on move.

### Example

```cpp
#include <helio/helio.hpp>
#include <vector>

int main() {
    // Create wgpu device/queue (not shown)
    HelioDevicePtr device = ...;
    HelioQueuePtr  queue  = ...;

    // Scene
    helio::Scene scene(device, queue);

    // Upload a mesh
    std::vector<HelioPackedVertex> verts = {
        { { -1,-1,0 }, 0, {0,0}, {0,0}, 0, 0 },
        { {  1,-1,0 }, 0, {1,0}, {0,0}, 0, 0 },
        { {  0, 1,0 }, 0, {0,1}, {0,0}, 0, 0 },
    };
    std::vector<uint32_t> idx = { 0, 1, 2 };
    helio::MeshUpload upload;
    upload.build(verts, idx);
    helio::MeshId mesh = scene.insert_mesh(*(upload.get()));

    // Material
    HelioGpuMaterial mat = {};
    mat.base_color[0] = 1.0f;  // red
    helio::MaterialId material = scene.insert_material(mat);

    // Object
    HelioObjectDescriptor obj = {};
    obj.mesh     = mesh.get();
    obj.material = material.get();
    obj.transform = helio::mat4_identity();
    scene.insert_object(obj);

    // Renderer (consumes the scene)
    HelioRendererConfig cfg = {};
    cfg.width  = 1920;
    cfg.height = 1080;
    helio::Renderer renderer(device, queue, 0, cfg, scene, ...);

    // Frame loop
    for (;;) {
        HelioCameraDesc cam = helio::camera_perspective_look_at(...);
        renderer.render(cam, texture_view);
    }
}
```

## Integration with `helio-ffi`

1. Build `helio-ffi`: `cargo build --release` → produces `libhelio_ffi.a`
2. Point CMake at it: `cmake -B build -DHELIO_FFI_LIB=/path/to/libhelio_ffi.a`
3. Build and run tests: `cmake --build build && ctest --test-dir build`

## License

MIT OR Apache-2.0
