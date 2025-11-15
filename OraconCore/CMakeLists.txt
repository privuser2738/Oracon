cmake_minimum_required(VERSION 3.15)
project(OraconCore)

# Source files
set(CORE_SOURCES
    src/common.cpp
    src/memory.cpp
    src/logger.cpp
)

# Header files
set(CORE_HEADERS
    include/oracon/core/common.h
    include/oracon/core/memory.h
    include/oracon/core/logger.h
    include/oracon/core/types.h
)

# Create library
add_library(OraconCore ${CORE_SOURCES} ${CORE_HEADERS})

# Include directories
target_include_directories(OraconCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Installation
install(TARGETS OraconCore
    EXPORT OraconCoreTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/oracon DESTINATION include)
