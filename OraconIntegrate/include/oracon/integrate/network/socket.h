#ifndef ORACON_INTEGRATE_NETWORK_SOCKET_H
#define ORACON_INTEGRATE_NETWORK_SOCKET_H

#include "oracon/core/types.h"
#include <string>
#include <memory>

namespace oracon {
namespace integrate {
namespace network {

using core::u16;
using core::i32;
using core::usize;
using core::String;

// Platform-specific socket handle
#ifdef _WIN32
    #include <winsock2.h>
    using SocketHandle = SOCKET;
    constexpr SocketHandle INVALID_SOCKET_HANDLE = INVALID_SOCKET;
#else
    using SocketHandle = int;
    constexpr SocketHandle INVALID_SOCKET_HANDLE = -1;
#endif

enum class SocketType {
    TCP,
    UDP
};

enum class SocketError {
    None,
    CreateFailed,
    BindFailed,
    ConnectFailed,
    SendFailed,
    ReceiveFailed,
    Timeout,
    Disconnected,
    Unknown
};

class Socket {
public:
    Socket(SocketType type = SocketType::TCP);
    ~Socket();

    // Disable copy, allow move
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;

    // Basic operations
    bool bind(const String& address, u16 port);
    bool listen(i32 backlog = 10);
    std::unique_ptr<Socket> accept();
    bool connect(const String& address, u16 port, i32 timeout_ms = 5000);

    // Data transfer
    i32 send(const void* data, usize size);
    i32 receive(void* buffer, usize size);
    i32 sendTo(const void* data, usize size, const String& address, u16 port);
    i32 receiveFrom(void* buffer, usize size, String& address, u16& port);

    // Socket options
    bool setBlocking(bool blocking);
    bool setReuseAddress(bool reuse);
    bool setKeepAlive(bool keepalive);
    bool setNoDelay(bool nodelay);
    bool setReceiveTimeout(i32 timeout_ms);
    bool setSendTimeout(i32 timeout_ms);

    // Status
    bool isValid() const { return m_handle != INVALID_SOCKET_HANDLE; }
    bool isConnected() const { return m_connected; }
    SocketHandle getHandle() const { return m_handle; }
    SocketError getLastError() const { return m_last_error; }
    String getLastErrorString() const;

    // Address info
    String getLocalAddress() const;
    u16 getLocalPort() const;
    String getRemoteAddress() const;
    u16 getRemotePort() const;

    void close();

private:
    Socket(SocketHandle handle, SocketType type);
    void setLastError(SocketError error);

    SocketHandle m_handle;
    SocketType m_type;
    bool m_connected;
    SocketError m_last_error;
};

// Socket initialization (for Windows)
class SocketInitializer {
public:
    SocketInitializer();
    ~SocketInitializer();
    bool isInitialized() const { return m_initialized; }

private:
    bool m_initialized;
    static i32 s_ref_count;
};

} // namespace network
} // namespace integrate
} // namespace oracon

#endif // ORACON_INTEGRATE_NETWORK_SOCKET_H
