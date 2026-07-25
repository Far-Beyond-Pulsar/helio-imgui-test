cmake_minimum_required(VERSION 3.20)
project(helio-imgui-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# GLFW
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
FetchContent_MakeAvailable(glfw)

# Dear ImGui (docking branch)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
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
        "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
    )
    target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}" "${imgui_SOURCE_DIR}/backends")
    target_link_libraries(imgui PRIVATE glfw)
endif()

# Helio FFI (Rust)
if(DEFINED HELIO_FFI_LIB)
    add_library(helio_ffi STATIC IMPORTED GLOBAL)
    set_target_properties(helio_ffi PROPERTIES IMPORTED_LOCATION "${HELIO_FFI_LIB}")
endif()

add_executable(helio_imgui_test src/main.cpp)
target_include_directories(helio_imgui_test PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_include_directories(helio_imgui_test PRIVATE "${CMAKE_SOURCE_DIR}/_deps/Helio-cpp/include")
target_link_libraries(helio_imgui_test PRIVATE glfw imgui opengl32)
if(TARGET helio_ffi)
    target_link_libraries(helio_imgui_test PRIVATE helio_ffi)
endif()
if(WIN32)
    target_link_libraries(helio_imgui_test PRIVATE ws2_32 userenv ntdll bcrypt advapi32 shlwapi)
endif()
