#ifndef ORACON_INTEGRATE_CONFIG_H
#define ORACON_INTEGRATE_CONFIG_H

#include "oracon/core/types.h"
#include <string>

namespace oracon {
namespace integrate {

using core::u16;
using core::u32;
using core::String;

// Version information
constexpr u32 INTEGRATE_VERSION_MAJOR = 0;
constexpr u32 INTEGRATE_VERSION_MINOR = 1;
constexpr u32 INTEGRATE_VERSION_PATCH = 0;

// Protocol constants
constexpr u32 PROTOCOL_MAGIC = 0x4F524143; // "ORAC"
constexpr u16 PROTOCOL_VERSION = 1;

// Default configuration
constexpr u16 DEFAULT_PORT = 7788;
constexpr u32 DEFAULT_BUFFER_SIZE = 65536; // 64 KB
constexpr u32 MAX_CONNECTIONS = 10;
constexpr u32 CONNECTION_TIMEOUT = 30; // seconds

// Desktop streaming
constexpr u32 DEFAULT_FPS = 30;
constexpr u32 DEFAULT_BITRATE = 5000000; // 5 Mbps

// File transfer
constexpr u32 FILE_CHUNK_SIZE = 1048576; // 1 MB

struct IntegrateConfig {
    // Network
    u16 port = DEFAULT_PORT;
    bool enable_discovery = true;
    u32 max_connections = MAX_CONNECTIONS;

    // Security
    bool require_auth = true;
    String key_file;
    String cert_file;

    // Desktop
    enum class Quality { Low, Medium, High, Ultra };
    Quality quality = Quality::Medium;
    u32 fps = DEFAULT_FPS;
    bool gpu_acceleration = true;

    // Service
    bool auto_start = false;
    bool run_as_service = false;

    // Features
    bool file_transfer = true;
    bool terminal_access = true;
    bool desktop_streaming = true;
    bool clipboard_sync = true;

    // Load from file
    static IntegrateConfig load(const String& filename);
    void save(const String& filename) const;
};

} // namespace integrate
} // namespace oracon

#endif // ORACON_INTEGRATE_CONFIG_H
