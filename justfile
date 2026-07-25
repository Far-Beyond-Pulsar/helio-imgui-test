# ── Shell (Windows) ──────────────────────────────────────────────────────────────
set shell := ["cmd.exe", "/C"]

# ── Paths ───────────────────────────────────────────────────────────────────────
repo_root    := "."
build_dir    := repo_root / "build"
deps_dir     := repo_root / "_deps"
helio_ffi_dir := deps_dir / "helio-ffi"
helio_dir    := deps_dir / "Helio"

# Local Helio workspace path (override via HELIO_ROOT env var)
helio_root := env_var_or_default("HELIO_ROOT", "C:/Users/redst/Documents/GitHub/Helio")

# Detect Rust target
rust_target := if arch() == "x86_64" { "x86_64-pc-windows-msvc" } else { "aarch64-pc-windows-msvc" }

# ── Recipes ─────────────────────────────────────────────────────────────────────

default: run

# Ensure deps directory exists
deps-dir:
    mkdir {{deps_dir}} 2>nul || ver>nul

# Set up Helio workspace: junction to local Helio, or init submodule
setup-helio: deps-dir
    @if not exist "{{helio_dir}}" (if exist "{{helio_root}}" (mklink /J "{{helio_dir}}" "{{helio_root}}" >nul) else (git submodule update --init "{{helio_dir}}")) else (echo Helio workspace ready: {{helio_dir}})

# Build helio-ffi (vendored in _deps/helio-ffi)
build-ffi: setup-helio
    cd /d "{{helio_ffi_dir}}" && cargo build --release --target {{rust_target}}
    @if not exist "{{build_dir}}\lib" mkdir "{{build_dir}}\lib"
    copy /Y "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.lib" "{{build_dir}}\lib\"
    copy /Y "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.dll" "{{build_dir}}\lib\"
    @if exist "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.pdb" copy /Y "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.pdb" "{{build_dir}}\lib\"

# CMake configure
configure: build-ffi
    @if not exist "{{build_dir}}" mkdir "{{build_dir}}"
    cd /d "{{build_dir}}" && cmake .. -DHELIO_FFI_LIB="{{build_dir}}\lib\helio_ffi.lib"

# Build the C++ test app
build: configure
    cd /d "{{build_dir}}" && cmake --build . --config Release

# Run
run: build
    cd /d "{{build_dir}}\Release" && helio_imgui_test.exe

# Clean everything
clean:
    @if exist "{{build_dir}}" rmdir /S /Q "{{build_dir}}"
    @if exist "{{helio_dir}}" (rmdir "{{helio_dir}}" >nul 2>nul & if exist "{{helio_dir}}" rmdir /S /Q "{{helio_dir}}")
    @if exist "{{helio_ffi_dir}}\target" rmdir /S /Q "{{helio_ffi_dir}}\target"

# Full build from scratch
rebuild: clean build
