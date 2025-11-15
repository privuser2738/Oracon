cmake_minimum_required(VERSION 3.15)
project(OraconIntegrate VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Platform detection
if(WIN32)
    set(ORACON_PLATFORM "Windows")
    add_definitions(-DORACON_PLATFORM_WINDOWS)
elseif(APPLE)
    set(ORACON_PLATFORM "macOS")
    add_definitions(-DORACON_PLATFORM_MACOS)
elseif(UNIX)
    set(ORACON_PLATFORM "Linux")
    add_definitions(-DORACON_PLATFORM_LINUX)
endif()

message(STATUS "Building OraconIntegrate for ${ORACON_PLATFORM}")

# Find dependencies
find_package(Qt6 COMPONENTS Core Widgets Network QUIET)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Optional dependencies
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(ZSTD libzstd)

    # FFmpeg for video encoding
    pkg_check_modules(LIBAVCODEC libavcodec)
    pkg_check_modules(LIBAVUTIL libavutil)
    pkg_check_modules(LIBSWSCALE libswscale)

    if(UNIX AND NOT APPLE)
        pkg_check_modules(X11 x11)
        pkg_check_modules(XRANDR xrandr)

        # VAAPI for hardware video encoding on Linux
        pkg_check_modules(LIBVA libva)
        pkg_check_modules(LIBVA_DRM libva-drm)
    endif()
endif()

# Source files - Core
set(INTEGRATE_CORE_SOURCES
    src/network/socket.cpp
    src/network/server.cpp
    src/network/client.cpp
    src/network/connection.cpp
    src/protocol/message.cpp
    src/protocol/handler.cpp
    src/filesystem/transfer.cpp
    src/terminal/pty.cpp
    src/terminal/shell.cpp
    src/desktop/capture.cpp
    src/desktop/encoder.cpp
    src/desktop/stream.cpp
)

# Platform-specific sources
if(WIN32)
    list(APPEND INTEGRATE_CORE_SOURCES
        platform/windows/capture_windows.cpp
        platform/windows/service_windows.cpp
        platform/windows/pty_windows.cpp
    )
elseif(APPLE)
    list(APPEND INTEGRATE_CORE_SOURCES
        platform/macos/capture_macos.mm
        platform/macos/service_macos.cpp
        platform/macos/pty_macos.cpp
        platform/macos/encoder_videotoolbox.cpp
    )
else()
    list(APPEND INTEGRATE_CORE_SOURCES
        platform/linux/capture_linux.cpp
        platform/linux/service_linux.cpp
        platform/linux/pty_linux.cpp
        platform/linux/encoder_vaapi.cpp
        platform/linux/encoder_stubs.cpp
    )
endif()

# Headers
set(INTEGRATE_HEADERS
    include/oracon/integrate/network/socket.h
    include/oracon/integrate/network/server.h
    include/oracon/integrate/network/client.h
    include/oracon/integrate/network/connection.h
    include/oracon/integrate/protocol/message.h
    include/oracon/integrate/protocol/handler.h
    include/oracon/integrate/filesystem/transfer.h
    include/oracon/integrate/terminal/pty.h
    include/oracon/integrate/terminal/shell.h
    include/oracon/integrate/desktop/capture.h
    include/oracon/integrate/desktop/encoder.h
    include/oracon/integrate/desktop/stream.h
    include/oracon/integrate/config.h
)

# Create core library
add_library(OraconIntegrateCore ${INTEGRATE_CORE_SOURCES} ${INTEGRATE_HEADERS})

target_include_directories(OraconIntegrateCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(OraconIntegrateCore
    PUBLIC
        OraconCore
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
)

# Add optional dependencies
if(ZSTD_FOUND)
    target_link_libraries(OraconIntegrateCore PRIVATE ${ZSTD_LIBRARIES})
    target_include_directories(OraconIntegrateCore PRIVATE ${ZSTD_INCLUDE_DIRS})
    target_compile_definitions(OraconIntegrateCore PRIVATE ORACON_HAS_ZSTD)
endif()

# FFmpeg support
if(LIBAVCODEC_FOUND AND LIBAVUTIL_FOUND AND LIBSWSCALE_FOUND)
    target_link_libraries(OraconIntegrateCore PRIVATE
        ${LIBAVCODEC_LIBRARIES}
        ${LIBAVUTIL_LIBRARIES}
        ${LIBSWSCALE_LIBRARIES}
    )
    target_include_directories(OraconIntegrateCore PRIVATE
        ${LIBAVCODEC_INCLUDE_DIRS}
        ${LIBAVUTIL_INCLUDE_DIRS}
        ${LIBSWSCALE_INCLUDE_DIRS}
    )
    target_compile_definitions(OraconIntegrateCore PRIVATE ORACON_HAVE_FFMPEG)
    message(STATUS "FFmpeg found - Software video encoding enabled")

    # VAAPI support (Linux only)
    if(LIBVA_FOUND AND LIBVA_DRM_FOUND)
        target_link_libraries(OraconIntegrateCore PRIVATE
            ${LIBVA_LIBRARIES}
            ${LIBVA_DRM_LIBRARIES}
        )
        target_include_directories(OraconIntegrateCore PRIVATE
            ${LIBVA_INCLUDE_DIRS}
            ${LIBVA_DRM_INCLUDE_DIRS}
        )
        message(STATUS "VAAPI found - Hardware video encoding enabled")
    endif()
else()
    message(WARNING "FFmpeg not found - Video encoding will be disabled")
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(OraconIntegrateCore PRIVATE ws2_32 iphlpapi)
elseif(APPLE)
    target_link_libraries(OraconIntegrateCore PRIVATE
        "-framework Foundation"
        "-framework CoreGraphics"
        "-framework IOKit"
    )
else()
    if(X11_FOUND)
        target_link_libraries(OraconIntegrateCore PRIVATE ${X11_LIBRARIES})
        target_include_directories(OraconIntegrateCore PRIVATE ${X11_INCLUDE_DIRS})
    endif()
    if(XRANDR_FOUND)
        target_link_libraries(OraconIntegrateCore PRIVATE ${XRANDR_LIBRARIES})
    endif()
    target_link_libraries(OraconIntegrateCore PRIVATE util) # For PTY support
endif()

# CLI executable (always build)
add_executable(oracon-integrate-cli src/main_cli.cpp)
target_link_libraries(oracon-integrate-cli PRIVATE OraconIntegrateCore)
set_target_properties(oracon-integrate-cli PROPERTIES OUTPUT_NAME oracon-integrate)

# GUI executable (if Qt6 is available)
if(Qt6_FOUND)
    set(INTEGRATE_GUI_SOURCES
        src/gui/mainwindow.cpp
        src/gui/connection_widget.cpp
        src/gui/file_browser.cpp
        src/gui/terminal_widget.cpp
        src/gui/desktop_viewer.cpp
        src/gui/settings_dialog.cpp
        src/main_gui.cpp
    )

    add_executable(oracon-integrate-gui ${INTEGRATE_GUI_SOURCES})
    target_link_libraries(oracon-integrate-gui PRIVATE
        OraconIntegrateCore
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
    )

    set_target_properties(oracon-integrate-gui PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
    )

    message(STATUS "Qt6 found - Building GUI version")
else()
    message(STATUS "Qt6 not found - Building CLI version only")
endif()

# Service executable
add_executable(oracon-integrate-service src/main_service.cpp)
target_link_libraries(oracon-integrate-service PRIVATE OraconIntegrateCore)

# Installation
install(TARGETS OraconIntegrateCore oracon-integrate-cli oracon-integrate-service
    EXPORT OraconIntegrateTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

if(Qt6_FOUND)
    install(TARGETS oracon-integrate-gui
        RUNTIME DESTINATION bin
        BUNDLE DESTINATION Applications
    )
endif()

install(DIRECTORY include/oracon DESTINATION include)

# Configuration files
install(FILES resources/oracon-integrate.conf.example
    DESTINATION etc/oracon
    RENAME oracon-integrate.conf
)

# Systemd service (Linux only)
if(UNIX AND NOT APPLE)
    install(FILES platform/linux/oracon-integrate.service
        DESTINATION /lib/systemd/system
        OPTIONAL
    )
endif()
