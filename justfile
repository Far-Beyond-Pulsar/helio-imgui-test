set shell := ["cmd.exe", "/C"]

export PATH := env_var_or_default("PATH", "") +
    ";C:\\Program Files\\CMake\\bin;C:\\Users\\redst\\.cargo\\bin"

build_dir := "."
cmake := env_var_or_default("CMAKE", "cmake")

default: run

configure:
    @if not exist "{{build_dir}}\build" mkdir "{{build_dir}}\build"
    cd /d "{{build_dir}}\build" && {{cmake}} ..

build: configure
    cd /d "{{build_dir}}\build" && {{cmake}} --build . --config Release

run: build
    cd /d "{{build_dir}}\build\Release" && helio_imgui_test.exe

clean:
    @if exist "{{build_dir}}\build" rmdir /S /Q "{{build_dir}}\build"

rebuild: clean build
