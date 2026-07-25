# ── Shell (Windows) ──────────────────────────────────────────────────────────────
set shell := ["cmd.exe", "/C"]

# ── Paths ───────────────────────────────────────────────────────────────────────
repo_root     := "."
build_dir     := repo_root / "build"
helio_ffi_dir  := repo_root / "_deps" / "helio-ffi"
helio_cpp_dir  := repo_root / "_deps" / "Helio-cpp"

# Detect Rust target
rust_target := if arch() == "x86_64" { "x86_64-pc-windows-msvc" } else { "aarch64-pc-windows-msvc" }

# ── Recipes ─────────────────────────────────────────────────────────────────────

default: run

# Clone Helio-cpp headers if not present
clone-helio-cpp:
    @if not exist "{{helio_cpp_dir}}" (
        git clone --depth 1 https://github.com/Far-Beyond-Pulsar/Helio-cpp.git "{{helio_cpp_dir}}"
    ) else (
        echo Helio-cpp already cloned
    )

# Build helio-ffi (vendored in _deps/helio-ffi)
build-ffi:
    cd /d "{{helio_ffi_dir}}" && cargo build --release --target {{rust_target}}
    @if not exist "{{build_dir}}\lib" mkdir "{{build_dir}}\lib"
    copy /Y "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.lib" "{{build_dir}}\lib\"
    copy /Y "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.dll" "{{build_dir}}\lib\"
    @if exist "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.pdb" copy /Y "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.pdb" "{{build_dir}}\lib\"

# CMake configure
configure: build-ffi clone-helio-cpp
    @if not exist "{{build_dir}}" mkdir "{{build_dir}}"
    cd /d "{{build_dir}}" && cmake .. -DHELIO_FFI_LIB="{{build_dir}}\lib\helio_ffi.lib" -DHELIO_CPP_INCLUDE="{{helio_cpp_dir}}\include"

# Build the C++ test app
build: configure
    cd /d "{{build_dir}}" && cmake --build . --config Release

# Run
run: build
    cd /d "{{build_dir}}\Release" && helio_imgui_test.exe

# Clean everything
clean:
    @if exist "{{build_dir}}" rmdir /S /Q "{{build_dir}}"
    @if exist "{{helio_ffi_dir}}\target" rmdir /S /Q "{{helio_ffi_dir}}\target"

# Full build from scratch
rebuild: clean build
