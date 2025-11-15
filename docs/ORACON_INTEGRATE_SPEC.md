# OraconIntegrate Specification

Version 0.1.0

## Overview

OraconIntegrate is a cross-platform remote desktop, file sharing, and system integration tool that provides seamless connectivity between Linux, Windows, and macOS systems.

## Core Features

### 1. Network Communication
- **Dual Mode**: Acts as both server and client simultaneously
- **Auto-Discovery**: Automatic discovery of OraconIntegrate instances on local network
- **Protocols**: TCP for reliable data transfer, UDP for low-latency streaming
- **Encryption**: TLS 1.3 for all communications
- **Port Configuration**: Default port 7788 (configurable)

### 2. File Transfer System
- **Bidirectional Transfer**: Send and receive files between any connected systems
- **Large File Support**: Chunked transfer with resume capability
- **Compression**: Optional zlib/zstd compression for faster transfers
- **Progress Tracking**: Real-time transfer progress with speed metrics
- **Drag & Drop**: GUI support for easy file sharing

### 3. Remote Terminal Access
- **Shell Access**: Execute commands on remote systems
- **Interactive Terminal**: Full PTY support for interactive sessions
- **Cross-Platform**: Access Linux shell from Windows, CMD/PowerShell from Linux, etc.
- **Multiple Sessions**: Support for multiple concurrent terminal sessions
- **Security**: Authenticated and encrypted terminal access

### 4. Desktop Streaming
- **Real-Time Streaming**: Low-latency desktop capture and streaming
- **GPU Acceleration**: Hardware-accelerated encoding/decoding
  - NVIDIA (NVENC/NVDEC)
  - AMD (VCE/UVD)
  - Intel Quick Sync
  - ROCm for AMD on Linux
- **Adaptive Quality**: Dynamic bitrate and resolution adjustment
- **Input Forwarding**: Mouse and keyboard control of remote desktop
- **Multi-Monitor**: Support for multiple displays

### 5. System Integration
- **Service Mode**: Run as background service/daemon
- **Auto-Start**: Optional startup on system boot
- **System Tray**: Minimize to system tray
- **Notifications**: Desktop notifications for events
- **Clipboard Sync**: Shared clipboard across systems

## Architecture

### Components

```
OraconIntegrate
├── Core
│   ├── Network Layer
│   ├── Protocol Handler
│   └── Connection Manager
├── Features
│   ├── File Transfer
│   ├── Terminal Access
│   ├── Desktop Streaming
│   └── Clipboard Sync
├── Platform Abstraction
│   ├── Linux (X11, Wayland)
│   ├── Windows (Win32 API)
│   └── macOS (Cocoa)
└── UI
    ├── GUI (Qt6)
    └── CLI Interface
```

### Protocol Specification

#### Message Format
```
[Header: 16 bytes]
├── Magic: 4 bytes (0x4F524143) "ORAC"
├── Version: 2 bytes
├── Message Type: 2 bytes
├── Payload Length: 4 bytes
├── Flags: 2 bytes
└── Reserved: 2 bytes

[Payload: variable length]
```

#### Message Types
- `0x0001`: HELLO (connection establishment)
- `0x0002`: AUTH (authentication)
- `0x0010`: FILE_TRANSFER_START
- `0x0011`: FILE_TRANSFER_DATA
- `0x0012`: FILE_TRANSFER_END
- `0x0020`: TERMINAL_OPEN
- `0x0021`: TERMINAL_DATA
- `0x0022`: TERMINAL_RESIZE
- `0x0023`: TERMINAL_CLOSE
- `0x0030`: DESKTOP_STREAM_START
- `0x0031`: DESKTOP_STREAM_FRAME
- `0x0032`: DESKTOP_INPUT
- `0x0040`: CLIPBOARD_SYNC
- `0x00FF`: ERROR

### GPU Acceleration

#### Encoding Pipeline
1. **Capture**: Platform-specific screen capture (DirectX, X11, CoreGraphics)
2. **Encode**: GPU-accelerated H.264/H.265 encoding
3. **Transmit**: Network transmission with FEC
4. **Decode**: GPU-accelerated decoding on client
5. **Display**: Render to screen

#### Supported Codecs
- **H.264/AVC**: Universal support, good compatibility
- **H.265/HEVC**: Better compression, higher quality
- **VP9**: Open codec, good for WebRTC integration
- **AV1**: Future support for best compression

### Security

#### Authentication
- **Key-Based**: Ed25519 public/private key pairs
- **Password**: Optional password authentication
- **Certificates**: X.509 certificates for enterprise deployments

#### Encryption
- **Transport**: TLS 1.3 with perfect forward secrecy
- **Data**: AES-256-GCM for payload encryption
- **Keys**: Automatic key rotation every 24 hours

## System Requirements

### Minimum Requirements
- **OS**: Linux (kernel 4.0+), Windows 10+, macOS 10.14+
- **CPU**: Dual-core 2.0 GHz
- **RAM**: 512 MB
- **Network**: 100 Mbps for basic features

### Recommended Requirements
- **CPU**: Quad-core 3.0 GHz
- **RAM**: 2 GB
- **GPU**: Any GPU with hardware video encoding
- **Network**: 1 Gbps for desktop streaming

### GPU Support
- **NVIDIA**: GTX 600 series or newer (Kepler+)
- **AMD**: Radeon HD 7000 series or newer (GCN+)
- **Intel**: HD Graphics 4000 or newer (Ivy Bridge+)

## Build System

### Dependencies
- **CMake**: 3.15+
- **Qt6**: For GUI (Core, Widgets, Network)
- **OpenSSL**: 1.1.1+
- **FFmpeg**: 4.0+ (with hardware acceleration support)
- **zstd**: For compression
- **Platform-Specific**:
  - Linux: X11/Wayland libraries
  - Windows: Windows SDK
  - macOS: Xcode Command Line Tools

### Build Targets
- `oracon-integrate`: Main executable
- `oracon-integrate-service`: Service/daemon mode
- `libOraconIntegrate`: Shared library for integration

## Configuration

### Config File: `oracon-integrate.conf`
```ini
[network]
port = 7788
enable_discovery = true
max_connections = 10

[security]
require_auth = true
key_file = ~/.oracon/keys/id_ed25519
cert_file = ~/.oracon/certs/integrate.crt

[desktop]
quality = medium
fps = 30
gpu_acceleration = auto

[service]
auto_start = false
run_as_service = false

[features]
file_transfer = true
terminal_access = true
desktop_streaming = true
clipboard_sync = true
```

## Command Line Interface

```bash
# Start GUI
oracon-integrate

# Start in server-only mode
oracon-integrate --server

# Connect to remote host
oracon-integrate --connect 192.168.1.100:7788

# Install as service
oracon-integrate --install-service

# List connected devices
oracon-integrate --list

# Send file
oracon-integrate --send file.txt --to 192.168.1.100

# Remote shell
oracon-integrate --shell 192.168.1.100
```

## GUI Features

### Main Window
- **Connection List**: Active and discovered connections
- **File Browser**: Integrated file manager for transfers
- **Terminal Tabs**: Multiple terminal sessions
- **Desktop Viewer**: Remote desktop display
- **Settings**: Configuration panel

### System Tray
- Quick connect menu
- Transfer progress
- Notifications
- Quick settings

## Platform-Specific Features

### Linux
- **X11 Support**: Full compatibility with X11
- **Wayland Support**: Native Wayland screen capture
- **Systemd Service**: Integration with systemd
- **VAAPI**: Hardware acceleration via VAAPI

### Windows
- **DirectX Capture**: Desktop Duplication API
- **Service Integration**: Windows Service Manager
- **NVENC**: NVIDIA hardware encoding
- **Quick Sync**: Intel hardware encoding

### macOS
- **CoreGraphics**: Native screen capture
- **Launchd**: macOS service integration
- **VideoToolbox**: Apple hardware encoding
- **Keychain**: Secure credential storage

## Performance Targets

### File Transfer
- **Local Network**: 100+ MB/s (gigabit)
- **Internet**: Limited by connection speed
- **Compression**: 2-5x for text files, 1.2-1.5x for media

### Desktop Streaming
- **Latency**: < 50ms on local network
- **Quality**: 1080p @ 30fps (medium quality)
- **Bandwidth**: 5-10 Mbps (H.264, medium quality)

### Resource Usage
- **Idle**: < 50 MB RAM, < 1% CPU
- **Streaming**: < 200 MB RAM, 10-20% CPU (with GPU)
- **Without GPU**: 30-50% CPU for encoding

## Future Enhancements

- WebRTC support for browser-based access
- Mobile app integration (Android/iOS)
- Audio streaming
- Webcam forwarding
- USB device forwarding
- Wake-on-LAN support
- Cloud relay for internet connections
