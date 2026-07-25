# helio-imgui-test

C++ test application that exercises the [helio-ffi](https://github.com/Far-Beyond-Pulsar/helio-ffi) C API.

## Prerequisites

- [Rust](https://rustup.rs/) (stable)
- [CMake](https://cmake.org/) 3.20+
- [just](https://github.com/casey/just) command runner

## Build & Run

```sh
just run
```

This will:

1. Clone `helio-ffi` from GitHub into `_deps/`
2. Build `helio-ffi` with `cargo build --release` (produces `.lib` + `.dll`)
3. Run CMake configure + build
4. Launch `helio_imgui_test.exe`

## What it tests

- wgpu device/queue/surface bootstrap via `bootstrap_init/create_surface`
- Scene creation: meshes, materials, objects, lights
- Renderer creation and rendering
- Swapchain presentation
- Editor and picker smoke tests
- MeshUpload helper

## Architecture

The bootstrap functions (`bootstrap_init`, `bootstrap_create_surface`, etc.) are
compiled directly into `helio_ffi.dll`/`helio_ffi.lib` — no separate Rust crate
is needed. All wgpu types are kept within a single Rust library, avoiding
duplicate symbol issues at link time.
