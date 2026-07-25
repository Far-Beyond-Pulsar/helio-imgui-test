# helio-imgui-test

C++ test application that exercises the [helio-ffi](https://github.com/Far-Beyond-Pulsar/helio-ffi) C API with an interactive ImGui window.

## Prerequisites

- **Windows** x64 (tested)
- **Rust** (stable) — [rustup.rs](https://rustup.rs/)
- **CMake** 3.20+
- **C++ compiler** — Visual Studio 2022 Build Tools or MSVC toolchain
- **just** command runner — `cargo install just` or [releases](https://github.com/casey/just/releases)

## Build & Run

```sh
git clone https://github.com/Far-Beyond-Pulsar/helio-imgui-test.git
cd helio-imgui-test
just run
```

This will:

1. Clone `Helio-cpp` C++ headers from GitHub into `_deps/`
2. Build `helio-ffi` with `cargo build --release` (fetches Helio crates from the `feat/401-hlfs-phase1` branch)
3. Run CMake configure + build (fetches GLFW and Dear ImGui via FetchContent)
4. Launch `helio_imgui_test.exe`

## Manual build (without just)

```powershell
cd _deps\helio-ffi
cargo build --release
cd ..\..

mkdir build
cd build
cmake .. -DHELIO_FFI_LIB="..\_deps\helio-ffi\target\release\helio_ffi.lib" -DHELIO_CPP_INCLUDE="..\_deps\Helio-cpp\include"
cmake --build . --config Release
.\Release\helio_imgui_test.exe
```

## Project structure

```
helio-imgui-test/
├── _deps/
│   ├── helio-ffi/        # Vendored Rust crate with C FFI (bootstrap + scene + renderer)
│   └── Helio-cpp/        # Vendored C++ headers for the helio_ffi C API
├── include/
│   └── bootstrap.h       # C header for wgpu bootstrapping functions
├── src/
│   └── main.cpp          # C++ test app with ImGui + GLFW
├── CMakeLists.txt         # CMake build (GLFW + ImGui via FetchContent)
├── justfile               # just recipes for the full build pipeline
└── README.md
```

## What it tests

- wgpu device/queue/surface bootstrap via `bootstrap_init` / `bootstrap_create_surface`
- Scene creation: meshes, materials, objects, lights
- Renderer creation and rendering
- Swapchain presentation
- Editor and picker smoke tests

## Notes

- helio-ffi depends on the `feat/401-hlfs-phase1` branch of Helio (visibility fixes for FFI). This will be merged to `main` once stable.
