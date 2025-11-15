cmake_minimum_required(VERSION 3.15)
project(OraconEngine)

# OraconEngine - 2D Game Engine
add_library(OraconEngine
    src/core/game_loop.cpp
    src/core/time.cpp
    src/ecs/entity.cpp
    src/ecs/world.cpp
    src/physics/collision.cpp
    src/input/input.cpp
    src/scene/scene.cpp
    src/scene/camera.cpp
    src/script/script.cpp
)

target_include_directories(OraconEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(OraconEngine PUBLIC
    OraconCore
    OraconMath
    OraconGfx
    OraconLang
    OraconAuto
)

# Require C++17
target_compile_features(OraconEngine PUBLIC cxx_std_17)

# Add examples
add_subdirectory(examples)
