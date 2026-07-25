set shell := ["cmd.exe", "/C"]

export PATH := env_var_or_default("PATH", "") + ";C:\\Program Files\\CMake\\bin;C:\\Users\\redst\\.cargo\\bin"

repo_root    := "."
build_dir    := repo_root / "build"
helio_ffi_dir := repo_root / "_deps" / "helio-ffi"
cmake := env_var_or_default("CMAKE", "cmake")
cargo := env_var_or_default("CARGO", "cargo")
rust_target := if arch() == "x86_64" { "x86_64-pc-windows-msvc" } else { "aarch64-pc-windows-msvc" }

default: run

build-ffi:
    cd /d "{{helio_ffi_dir}}" && {{cargo}} build --release --target {{rust_target}}
    @if not exist "{{build_dir}}\lib" mkdir "{{build_dir}}\lib"
    copy /Y "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.lib" "{{build_dir}}\lib\"
    copy /Y "{{helio_ffi_dir}}\target\{{rust_target}}\release\helio_ffi.dll" "{{build_dir}}\lib\"

configure: build-ffi
    @if not exist "{{build_dir}}" mkdir "{{build_dir}}"
    cd /d "{{build_dir}}" && {{cmake}} .. -DHELIO_FFI_LIB=".\lib\helio_ffi.lib"

build: configure
    cd /d "{{build_dir}}" && {{cmake}} --build . --config Release

run: build
    cd /d "{{build_dir}}\Release" && helio_imgui_test.exe

clean:
    @if exist "{{build_dir}}" rmdir /S /Q "{{build_dir}}"

rebuild: clean build
