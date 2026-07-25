# helio-imgui-test

Standalone C++ test application that exercises the [helio-ffi](https://github.com/Far-Beyond-Pulsar/helio-ffi) C API with an interactive ImGui window.

## Prerequisites

- **Windows** x64 (tested)
- **Rust** (stable) — [rustup.rs](https://rustup.rs/)
- **CMake** 3.20+
- **C++ compiler** — Visual Studio 2022 Build Tools or MSVC toolchain
- **just** command runner — `cargo install just` or [releases](https://github.com/casey/just/releases)

## Quick start

### 1. Clone with submodules

```sh
git clone --recursive https://github.com/Far-Beyond-Pulsar/helio-imgui-test.git
cd helio-imgui-test
```

If you already cloned without `--recursive`:

```sh
git submodule update --init --recursive
```

### 2. Set up the Helio workspace

helio-ffi depends on crates from the [Helio renderer](https://github.com/Far-Beyond-Pulsar/Helio). The repo includes Helio as a git submodule at `_deps/Helio`, but during active development you'll want to use a local Helio checkout with relaxed visibility.

Set `HELIO_ROOT` to point to your local Helio workspace:

```sh
# PowerShell
$env:HELIO_ROOT = "C:\path\to\Helio"

# Command Prompt
set HELIO_ROOT=C:\path\to\Helio
```

If `HELIO_ROOT` is not set, the justfile falls back to `C:/Users/redst/Documents/GitHub/Helio` (the developer default) or inits the git submodule.

### 3. Build and run

```sh
just run
```

This will:

1. Create a directory junction at `_deps/Helio` pointing to your local Helio workspace (or init the submodule)
2. Build `helio-ffi` with `cargo build --release` (produces `helio_ffi.dll` + `helio_ffi.lib`)
3. Run CMake configure + build (fetches GLFW and Dear ImGui via FetchContent)
4. Launch `helio_imgui_test.exe`

## Manual build (without just)

```powershell
# 1. Set up Helio workspace junction
New-Item -ItemType Junction -Path _deps\Helio -Target C:\path\to\Helio

# 2. Build helio-ffi
cd _deps\helio-ffi
cargo build --release
cd ..\..

# 3. CMake
mkdir build
cd build
cmake .. -DHELIO_FFI_LIB="..\_deps\helio-ffi\target\release\helio_ffi.lib"
cmake --build . --config Release

# 4. Run
.\Release\helio_imgui_test.exe
```

## Project structure

```
helio-imgui-test/
├── _deps/
│   ├── helio-ffi/        # Vendored Rust crate with C FFI (bootstrap + scene + renderer)
│   └── Helio/            # Git submodule pointing to the Helio renderer workspace
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

## Note on Helio API stability

The helio-ffi crate relies on certain `pub(crate)` Helio APIs that may not be publicly exported on the remote `main` branch. During development, use a local Helio checkout where these APIs are made `pub`. The Helio submodule is provided for reference but will likely not compile against helio-ffi until the visibility changes are upstreamed.
