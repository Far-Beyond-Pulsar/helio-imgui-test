# helio-ffi

C-compatible FFI bindings for the [Helio](https://github.com/Far-Beyond-Pulsar/Helio) renderer engine.

This crate compiles to a **static library** (`libhelio_ffi.a` / `helio_ffi.lib`) and a **dynamic library** (`helio_ffi.dll` / `libhelio_ffi.so`) exposing a pure C API. Use it from C, C++, or any language with C FFI support.

## Building

```sh
git clone https://github.com/Far-Beyond-Pulsar/helio-ffi.git
cd helio-ffi
cargo build --release
```

The output lives in `target/release/`:
- Windows: `helio_ffi.lib` + `helio_ffi.dll`
- macOS/Linux: `libhelio_ffi.a` + `libhelio_ffi.so`

## Usage from C/C++

Include the function declarations from [Helio-cpp](https://github.com/Far-Beyond-Pulsar/Helio-cpp) (`include/helio/helio_capi.h`) or write your own extern declarations matching the symbols below.

Link against the built library and initialize a wgpu device/queue yourself:

```c
#include <helio/helio_capi.h>

// Create wgpu device/queue however you like, then:
HelioDevicePtr device = ...;
HelioQueuePtr  queue  = ...;

// 1. Create a scene
HelioScenePtr scene = helio_scene_new(device, queue);

// 2. Build a renderer (transfers ownership of the scene)
HelioRendererConfig cfg = HelioRendererConfig_default();
HelioBufferPtr debug_cam = ...;  // wgpu::Buffer
HelioBufferPtr cull_stats = ...; // wgpu::Buffer
HelioRendererPtr renderer = helio_renderer_new(device, queue, surface_format,
                                                &cfg, scene, debug_cam, cull_stats);

// 3. Mutate the scene through the renderer
HelioScenePtr s = helio_renderer_scene(renderer);
HelioCameraDesc cam = helio_camera_perspective_look_at(
    0,0,0, 0,0,-1, 1.57f, 16/9.f, 0.1f, 1000.f);
helio_scene_update_camera(s, &cam);

// 4. Insert geometry, lights, materials, etc.
HelioGpuMaterial mat = { ... };
HelioHandle material = helio_scene_insert_material(s, &mat);

// 5. Render each frame
helio_renderer_render(renderer, &cam, target_texture_view);

// 6. Cleanup
helio_renderer_destroy(renderer);
```

## API Overview

| Module | Functions | Purpose |
|--------|-----------|---------|
| **Scene** | `helio_scene_new/destroy/flush/advance_frame/set_render_size/clear` | Scene lifecycle |
| **Camera** | `helio_camera_from_matrices`, `helio_camera_perspective_look_at` | Camera construction |
| **Groups** | `helio_scene_hide/show/is_group_hidden` | Per-group visibility |
| **Meshes** | `helio_scene_insert/remove/dynamic_mesh` | Static & dynamic mesh resources |
| **Materials** | `helio_scene_insert/remove/update_material`, `set_material_class`, `update_material_class_params` | Material resources |
| **Textures** | `helio_scene_insert/remove_texture` | Texture resources |
| **Lights** | `helio_scene_insert/remove/update_light`, `insert_light_with_tag` | Light resources |
| **Objects** | `helio_scene_insert/remove_object`, `update_object_transform`, `get_object_transform`, `update_object_material` | Placed mesh instances |
| **Decals** | `helio_scene_insert/remove/update_decal` | Decal overlays |
| **Water** | `helio_scene_insert/remove_water_volume`, `insert/remove_water_hitbox` | Water volumes & hitboxes |
| **Reflections** | `helio_scene_insert/remove_reflection_capture` | Reflection probes |
| **Voxels** | `helio_scene_insert/remove_voxel_volume` | Voxel volumes |
| **Virtual Geo** | `helio_scene_insert_virtual_mesh/object` | GPU-driven virtual geometry |
| **Renderer** | `helio_renderer_new/destroy/render/scene` | Main render loop |
| **Renderer config** | `set_clear_color/ambient/debug_mode/editor_mode/shadow_quality` | Runtime config |
| **Debug draw** | `helio_renderer_debug_line/sphere` | Immediate-mode debug visualization |
| **Editor** | `helio_editor_new/destroy/select/deselect/ray/update_hover/drag/gizmos` | Gizmo-based scene editing |
| **Picker** | `helio_picker_new/destroy/register_mesh/cast_ray` | Ray-intersection picking |
| **Mesh helpers** | `helio_mesh_upload_create/free` | Build opaque mesh upload objects |

## Dependencies

- [Helio](https://github.com/Far-Beyond-Pulsar/Helio) — the renderer engine (fetched via git)
- [wgpu](https://github.com/gfx-rs/wgpu) 30.x — GPU abstraction
- [glam](https://github.com/bitshifter/glam-rs) 0.33 — math types

## License

MIT OR Apache-2.0
