#include "oracon/integrate/network/socket.h"
#include <cstring>

#ifdef _WIN32
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
#endif

namespace oracon {
namespace integrate {
namespace network {

// SocketInitializer
i32 SocketInitializer::s_ref_count = 0;

SocketInitializer::SocketInitializer() : m_initialized(false) {
#ifdef _WIN32
    if (s_ref_count == 0) {
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0) {
            m_initialized = true;
        }
    } else {
        m_initialized = true;
    }
    s_ref_count++;
#else
    m_initialized = true;
#endif
}

SocketInitializer::~SocketInitializer() {
#ifdef _WIN32
    s_ref_count--;
    if (s_ref_count == 0) {
        WSACleanup();
    }
#endif
}

// Socket implementation
Socket::Socket(SocketType type)
    : m_handle(INVALID_SOCKET_HANDLE)
    , m_type(type)
    , m_connected(false)
    , m_last_error(SocketError::None)
{
    int sock_type = (type == SocketType::TCP) ? SOCK_STREAM : SOCK_DGRAM;
    int protocol = (type == SocketType::TCP) ? IPPROTO_TCP : IPPROTO_UDP;

    m_handle = socket(AF_INET, sock_type, protocol);
    if (m_handle == INVALID_SOCKET_HANDLE) {
        setLastError(SocketError::CreateFailed);
    }
}

Socket::Socket(SocketHandle handle, SocketType type)
    : m_handle(handle)
    , m_type(type)
    , m_connected(true)
    , m_last_error(SocketError::None)
{}

Socket::~Socket() {
    close();
}

Socket::Socket(Socket&& other) noexcept
    : m_handle(other.m_handle)
    , m_type(other.m_type)
    , m_connected(other.m_connected)
    , m_last_error(other.m_last_error)
{
    other.m_handle = INVALID_SOCKET_HANDLE;
    other.m_connected = false;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        close();
        m_handle = other.m_handle;
        m_type = other.m_type;
        m_connected = other.m_connected;
        m_last_error = other.m_last_error;
        other.m_handle = INVALID_SOCKET_HANDLE;
        other.m_connected = false;
    }
    return *this;
}

bool Socket::bind(const String& address, u16 port) {
    if (!isValid()) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (address.empty() || address == "0.0.0.0") {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, address.c_str(), &addr.sin_addr);
    }

    if (::bind(m_handle, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        setLastError(SocketError::BindFailed);
        return false;
    }

    return true;
}

bool Socket::listen(i32 backlog) {
    if (!isValid() || m_type != SocketType::TCP) return false;

    if (::listen(m_handle, backlog) < 0) {
        return false;
    }

    return true;
}

std::unique_ptr<Socket> Socket::accept() {
    if (!isValid() || m_type != SocketType::TCP) return nullptr;

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    SocketHandle client_handle = ::accept(m_handle, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    if (client_handle == INVALID_SOCKET_HANDLE) {
        return nullptr;
    }

    return std::unique_ptr<Socket>(new Socket(client_handle, SocketType::TCP));
}

bool Socket::connect(const String& address, u16 port, i32 timeout_ms) {
    if (!isValid()) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

    // TODO: Implement timeout
    (void)timeout_ms;

    if (::connect(m_handle, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        setLastError(SocketError::ConnectFailed);
        return false;
    }

    m_connected = true;
    return true;
}

i32 Socket::send(const void* data, usize size) {
    if (!isValid() || !m_connected) return -1;

    i32 result = ::send(m_handle, static_cast<const char*>(data), static_cast<int>(size), 0);
    if (result < 0) {
        setLastError(SocketError::SendFailed);
    }
    return result;
}

i32 Socket::receive(void* buffer, usize size) {
    if (!isValid()) return -1;

    i32 result = ::recv(m_handle, static_cast<char*>(buffer), static_cast<int>(size), 0);
    if (result < 0) {
        setLastError(SocketError::ReceiveFailed);
    } else if (result == 0) {
        m_connected = false;
        setLastError(SocketError::Disconnected);
    }
    return result;
}

i32 Socket::sendTo(const void* data, usize size, const String& address, u16 port) {
    if (!isValid()) return -1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

    return ::sendto(m_handle, static_cast<const char*>(data), static_cast<int>(size), 0,
                    reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
}

i32 Socket::receiveFrom(void* buffer, usize size, String& address, u16& port) {
    if (!isValid()) return -1;

    sockaddr_in addr{};
    socklen_t addr_len = sizeof(addr);

    i32 result = ::recvfrom(m_handle, static_cast<char*>(buffer), static_cast<int>(size), 0,
                            reinterpret_cast<sockaddr*>(&addr), &addr_len);

    if (result >= 0) {
        char addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr.sin_addr, addr_str, INET_ADDRSTRLEN);
        address = addr_str;
        port = ntohs(addr.sin_port);
    }

    return result;
}

bool Socket::setBlocking(bool blocking) {
    if (!isValid()) return false;

#ifdef _WIN32
    u_long mode = blocking ? 0 : 1;
    return ioctlsocket(m_handle, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(m_handle, F_GETFL, 0);
    if (flags < 0) return false;

    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return fcntl(m_handle, F_SETFL, flags) == 0;
#endif
}

bool Socket::setReuseAddress(bool reuse) {
    if (!isValid()) return false;

    int opt = reuse ? 1 : 0;
    return setsockopt(m_handle, SOL_SOCKET, SO_REUSEADDR,
                      reinterpret_cast<const char*>(&opt), sizeof(opt)) == 0;
}

bool Socket::setKeepAlive(bool keepalive) {
    if (!isValid()) return false;

    int opt = keepalive ? 1 : 0;
    return setsockopt(m_handle, SOL_SOCKET, SO_KEEPALIVE,
                      reinterpret_cast<const char*>(&opt), sizeof(opt)) == 0;
}

bool Socket::setNoDelay(bool nodelay) {
    if (!isValid() || m_type != SocketType::TCP) return false;

    int opt = nodelay ? 1 : 0;
    return setsockopt(m_handle, IPPROTO_TCP, TCP_NODELAY,
                      reinterpret_cast<const char*>(&opt), sizeof(opt)) == 0;
}

bool Socket::setReceiveTimeout(i32 timeout_ms) {
    if (!isValid()) return false;

#ifdef _WIN32
    DWORD timeout = timeout_ms;
    return setsockopt(m_handle, SOL_SOCKET, SO_RCVTIMEO,
                      reinterpret_cast<const char*>(&timeout), sizeof(timeout)) == 0;
#else
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    return setsockopt(m_handle, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == 0;
#endif
}

bool Socket::setSendTimeout(i32 timeout_ms) {
    if (!isValid()) return false;

#ifdef _WIN32
    DWORD timeout = timeout_ms;
    return setsockopt(m_handle, SOL_SOCKET, SO_SNDTIMEO,
                      reinterpret_cast<const char*>(&timeout), sizeof(timeout)) == 0;
#else
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    return setsockopt(m_handle, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == 0;
#endif
}

String Socket::getLastErrorString() const {
    // TODO: Implement proper error string conversion
    return "Socket error";
}

String Socket::getLocalAddress() const {
    if (!isValid()) return "";

    sockaddr_in addr{};
    socklen_t addr_len = sizeof(addr);
    if (getsockname(m_handle, reinterpret_cast<sockaddr*>(&addr), &addr_len) < 0) {
        return "";
    }

    char addr_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, addr_str, INET_ADDRSTRLEN);
    return addr_str;
}

u16 Socket::getLocalPort() const {
    if (!isValid()) return 0;

    sockaddr_in addr{};
    socklen_t addr_len = sizeof(addr);
    if (getsockname(m_handle, reinterpret_cast<sockaddr*>(&addr), &addr_len) < 0) {
        return 0;
    }

    return ntohs(addr.sin_port);
}

String Socket::getRemoteAddress() const {
    if (!isValid() || !m_connected) return "";

    sockaddr_in addr{};
    socklen_t addr_len = sizeof(addr);
    if (getpeername(m_handle, reinterpret_cast<sockaddr*>(&addr), &addr_len) < 0) {
        return "";
    }

    char addr_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, addr_str, INET_ADDRSTRLEN);
    return addr_str;
}

u16 Socket::getRemotePort() const {
    if (!isValid() || !m_connected) return 0;

    sockaddr_in addr{};
    socklen_t addr_len = sizeof(addr);
    if (getpeername(m_handle, reinterpret_cast<sockaddr*>(&addr), &addr_len) < 0) {
        return 0;
    }

    return ntohs(addr.sin_port);
}

void Socket::close() {
    if (m_handle != INVALID_SOCKET_HANDLE) {
#ifdef _WIN32
        closesocket(m_handle);
#else
        ::close(m_handle);
#endif
        m_handle = INVALID_SOCKET_HANDLE;
        m_connected = false;
    }
}

void Socket::setLastError(SocketError error) {
    m_last_error = error;
}

} // namespace network
} // namespace integrate
} // namespace oracon
