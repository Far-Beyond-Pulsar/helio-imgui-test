cmake_minimum_required(VERSION 3.20)
project(helio-imgui-test LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Dependencies via FetchContent ───────────────────────────────────────────────
include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8
    GIT_SHALLOW TRUE
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    add_library(imgui STATIC
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_demo.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
    )
    target_include_directories(imgui PUBLIC
        "${imgui_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}/backends"
    )
    target_link_libraries(imgui PRIVATE glfw)
endif()

# ── Helio FFI (Rust library with built-in bootstrap) ────────────────────────────
# Built by the justfile before CMake is invoked.
# The justfile passes -DHELIO_FFI_LIB to point to the built .lib.
#
# helio_ffi now includes the bootstrap functions (bootstrap_init, etc.)
# so no separate bootstrap library is needed.

if(DEFINED HELIO_FFI_LIB)
    add_library(helio_ffi STATIC IMPORTED GLOBAL)
    set_target_properties(helio_ffi PROPERTIES IMPORTED_LOCATION "${HELIO_FFI_LIB}")
else()
    message(WARNING "HELIO_FFI_LIB not set — FFI target won't link")
endif()

# ── Helio C++ headers (Helio-cpp repo) ──────────────────────────────────────────
if(DEFINED HELIO_CPP_INCLUDE)
    # path to Helio-cpp repo's include/
else()
    message(WARNING "HELIO_CPP_INCLUDE not set — C++ headers won't be found")
endif()

# ── Test executable ─────────────────────────────────────────────────────────────
add_executable(helio_imgui_test src/main.cpp)
target_include_directories(helio_imgui_test PRIVATE "${CMAKE_SOURCE_DIR}/include")
if(DEFINED HELIO_CPP_INCLUDE)
    target_include_directories(helio_imgui_test PRIVATE "${HELIO_CPP_INCLUDE}")
endif()
target_link_libraries(helio_imgui_test PRIVATE glfw imgui)
if(TARGET helio_ffi)
    target_link_libraries(helio_imgui_test PRIVATE helio_ffi)
endif()

# Windows system libs
if(WIN32)
    target_link_libraries(helio_imgui_test PRIVATE
        user32 gdi32 shell32 ole32
        ws2_32 userenv ntdll bcrypt advapi32 shlwapi
    )
    # If duplicate wgpu symbols from linking separate Rust staticlibs,
    # uncomment the next line:
    # target_link_options(helio_imgui_test PRIVATE /FORCE:MULTIPLE)
endif()
