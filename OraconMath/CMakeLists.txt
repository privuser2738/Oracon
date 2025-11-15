cmake_minimum_required(VERSION 3.15)
project(OraconMath)

# OraconMath is a header-only library
add_library(OraconMath INTERFACE)

target_include_directories(OraconMath INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link to OraconCore for types
target_link_libraries(OraconMath INTERFACE OraconCore)

# Require C++17
target_compile_features(OraconMath INTERFACE cxx_std_17)

# Add test executable
add_executable(test_math tests/test_math.cpp)
target_link_libraries(test_math OraconMath)
