cmake_minimum_required(VERSION 3.20)
project(helio-cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Header-only library ─────────────────────────────────────────────────────────
add_library(helio INTERFACE)
target_include_directories(helio INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# ── Tests ───────────────────────────────────────────────────────────────────────
option(HELIO_BUILD_TESTS "Build Helio-cpp tests" ON)
if(HELIO_BUILD_TESTS)
    enable_testing()

    find_package(helio-ffi QUIET)
    if(NOT helio-ffi_FOUND)
        message(STATUS "helio-ffi not found — linking tests to the static lib directly")
        # Expect HELIO_FFI_LIB to point to the built helio_ffi static library
        if(DEFINED HELIO_FFI_LIB)
            add_library(helio-ffi STATIC IMPORTED GLOBAL)
            set_target_properties(helio-ffi PROPERTIES IMPORTED_LOCATION "${HELIO_FFI_LIB}")
        else()
            message(WARNING "HELIO_FFI_LIB not set — tests will not link. "
                            "Set it to the path of libhelio_ffi.a / helio_ffi.lib")
        endif()
    endif()

    add_executable(helio_test_capi tests/test_capi.cpp)
    target_link_libraries(helio_test_capi PRIVATE helio)
    if(TARGET helio-ffi)
        target_link_libraries(helio_test_capi PRIVATE helio-ffi)
    endif()

    add_executable(helio_test_cpp tests/test_cpp.cpp)
    target_link_libraries(helio_test_cpp PRIVATE helio)
    if(TARGET helio-ffi)
        target_link_libraries(helio_test_cpp PRIVATE helio-ffi)
    endif()

    add_test(NAME capi COMMAND helio_test_capi)
    add_test(NAME cpp  COMMAND helio_test_cpp)
endif()
