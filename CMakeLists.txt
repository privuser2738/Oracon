cmake_minimum_required(VERSION 3.15)
project(Oracon VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler options
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Add subdirectories
add_subdirectory(OraconCore)
add_subdirectory(OraconMath)
add_subdirectory(OraconLang)
add_subdirectory(OraconIntegrate)
add_subdirectory(OraconGfx)
add_subdirectory(OraconAuto)
add_subdirectory(OraconEngine)

# Installation
install(DIRECTORY docs/ DESTINATION share/doc/oracon)
