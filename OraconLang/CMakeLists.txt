cmake_minimum_required(VERSION 3.15)
project(OraconLang)

# Create include directory structure
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/oracon/lang)

# Source files
set(LANG_SOURCES
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/ast/ast.cpp
    src/interpreter/value.cpp
    src/interpreter/environment.cpp
    src/interpreter/interpreter.cpp
    src/runtime/builtins.cpp
)

# Header files
set(LANG_HEADERS
    include/oracon/lang/lexer/token.h
    include/oracon/lang/lexer/lexer.h
    include/oracon/lang/parser/parser.h
    include/oracon/lang/ast/ast.h
    include/oracon/lang/interpreter/value.h
    include/oracon/lang/interpreter/environment.h
    include/oracon/lang/interpreter/interpreter.h
    include/oracon/lang/runtime/builtins.h
)

# Create library
add_library(OraconLang ${LANG_SOURCES} ${LANG_HEADERS})

# Include directories
target_include_directories(OraconLang
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Link with OraconCore
target_link_libraries(OraconLang PUBLIC OraconCore)

# Create executable
add_executable(oracon src/main.cpp)
target_link_libraries(oracon PRIVATE OraconLang)

# Installation
install(TARGETS OraconLang oracon
    EXPORT OraconLangTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/oracon DESTINATION include)

# Examples
if(BUILD_EXAMPLES AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/CMakeLists.txt)
    add_subdirectory(examples)
endif()

# Tests
if(BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
    add_subdirectory(tests)
endif()
