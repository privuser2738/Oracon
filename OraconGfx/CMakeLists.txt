cmake_minimum_required(VERSION 3.15)
project(OraconGfx)

# Find SDL2
find_package(SDL2 QUIET)
if(NOT SDL2_FOUND)
    # Try pkg-config as fallback
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SDL2 IMPORTED_TARGET sdl2)
    endif()
endif()

# OraconGfx 2D Graphics Library
add_library(OraconGfx
    src/renderer/renderer.cpp
)

target_include_directories(OraconGfx PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(OraconGfx PUBLIC
    OraconCore
    OraconMath
)

# Optional SDL2 support
if(SDL2_FOUND)
    if(TARGET SDL2::SDL2)
        target_link_libraries(OraconGfx PUBLIC SDL2::SDL2)
    elseif(TARGET PkgConfig::SDL2)
        target_link_libraries(OraconGfx PUBLIC PkgConfig::SDL2)
    endif()
    target_compile_definitions(OraconGfx PUBLIC ORACON_HAS_SDL2)
    message(STATUS "OraconGfx: SDL2 support enabled")
else()
    message(STATUS "OraconGfx: SDL2 not found, window support disabled")
endif()

# Require C++17
target_compile_features(OraconGfx PUBLIC cxx_std_17)

# Add examples
add_subdirectory(examples)
