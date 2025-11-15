cmake_minimum_required(VERSION 3.15)
project(OraconAuto)

# Find libcurl
find_package(CURL REQUIRED)

# OraconAuto - AI/LLM automation library
add_library(OraconAuto
    src/llm_client_impl.cpp
    src/http_client_curl.cpp
    src/anthropic_client.cpp
    src/tools_impl.cpp
)

target_include_directories(OraconAuto PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link to OraconCore for types and logging, and CURL for HTTP
target_link_libraries(OraconAuto PUBLIC
    OraconCore
    CURL::libcurl
)

# Require C++17
target_compile_features(OraconAuto PUBLIC cxx_std_17)

# Add examples
add_subdirectory(examples)
