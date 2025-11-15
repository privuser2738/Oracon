#ifndef ORACON_INTEGRATE_NETWORK_CONNECTION_H
#define ORACON_INTEGRATE_NETWORK_CONNECTION_H

#include "oracon/integrate/network/socket.h"
#include "oracon/integrate/protocol/message.h"
#include "oracon/core/types.h"
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <queue>
#include <mutex>

namespace oracon {
namespace integrate {
namespace network {

using core::u32;
using core::String;

// Connection state
enum class ConnectionState {
    Disconnected,
    Connecting,
    Authenticating,
    Connected,
    Closing,
    Error
};

// Message callback
using MessageCallback = std::function<void(std::unique_ptr<protocol::Message>)>;

// Connection info
struct ConnectionInfo {
    String remote_address;
    u16 remote_port;
    String hostname;
    String os_type;
    ConnectionState state;
};

class Connection {
public:
    explicit Connection(std::unique_ptr<Socket> socket);
    ~Connection();

    // Disable copy, allow move
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) noexcept = default;
    Connection& operator=(Connection&&) noexcept = default;

    // Start the connection (spawns receive thread)
    bool start();

    // Stop the connection
    void stop();

    // Send a message
    bool sendMessage(const protocol::Message& message);

    // Set callback for received messages
    void setMessageCallback(MessageCallback callback);

    // Status
    ConnectionState getState() const { return m_state; }
    const ConnectionInfo& getInfo() const { return m_info; }
    u32 getId() const { return m_id; }
    bool isConnected() const { return m_state == ConnectionState::Connected; }

    // Get socket (for direct operations like file transfer)
    Socket* getSocket() { return m_socket.get(); }

private:
    void receiveLoop();
    void processMessage(std::unique_ptr<protocol::Message> message);
    void setState(ConnectionState state);

    std::unique_ptr<Socket> m_socket;
    u32 m_id;
    ConnectionInfo m_info;
    std::atomic<ConnectionState> m_state;

    std::thread m_receive_thread;
    std::atomic<bool> m_running;

    MessageCallback m_message_callback;
    std::mutex m_callback_mutex;

    static std::atomic<u32> s_next_id;
};

} // namespace network
} // namespace integrate
} // namespace oracon

#endif // ORACON_INTEGRATE_NETWORK_CONNECTION_H
