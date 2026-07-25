# ── Paths ───────────────────────────────────────────────────────────────────────
repo_root    := "."
build_dir    := repo_root / "build"
deps_dir     := repo_root / "_deps"
helio_ffi_dir := deps_dir / "helio-ffi"

# Detect Rust target
rust_target := if arch() == "x86_64" { "x86_64-pc-windows-msvc" } else { "aarch64-pc-windows-msvc" }

# ── Recipes ─────────────────────────────────────────────────────────────────────

default: run

# Ensure deps directory exists
deps-dir:
    mkdir {{deps_dir}}

# Clone helio-ffi if not already present
clone-ffi: deps-dir
    @if not exist "{{helio_ffi_dir}}" (
        git clone --depth 1 https://github.com/Far-Beyond-Pulsar/helio-ffi.git "{{helio_ffi_dir}}"
    ) else (
        echo Already cloned: helio-ffi
    )

# Build helio-ffi (includes bootstrap, scene, renderer, etc.)
build-ffi: clone-ffi
    cd "{{helio_ffi_dir}}" && cargo build --release --target {{rust_target}}
    if not exist "{{build_dir}}\lib" mkdir "{{build_dir}}\lib"
    copy /Y "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.lib" "{{build_dir}}\lib\"
    if exist "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.dll" (
        copy /Y "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.dll" "{{build_dir}}\lib\"
    )
    if exist "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.pdb" (
        copy /Y "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.pdb" "{{build_dir}}\lib\"
    )

# CMake configure
configure: build-ffi
    if not exist "{{build_dir}}" mkdir "{{build_dir}}"
    cd "{{build_dir}}" && cmake .. -DHELIO_FFI_LIB="{{build_dir}}\lib\helio_ffi.lib"

# Build the C++ test app
build: configure
    cd "{{build_dir}}" && cmake --build . --config Release

# Run
run: build
    cd "{{build_dir}}\Release" && helio_imgui_test.exe

# Clean everything
clean:
    if exist "{{build_dir}}" rmdir /S /Q "{{build_dir}}"
    if exist "{{deps_dir}}" rmdir /S /Q "{{deps_dir}}"

# Full build from scratch
rebuild: clean build
