#include "oracon/integrate/network/connection.h"
#include "oracon/core/logger.h"
#include <vector>

namespace oracon {
namespace integrate {
namespace network {

std::atomic<u32> Connection::s_next_id{1};

Connection::Connection(std::unique_ptr<Socket> socket)
    : m_socket(std::move(socket))
    , m_id(s_next_id++)
    , m_state(ConnectionState::Disconnected)
    , m_running(false)
{
    m_info.remote_address = m_socket->getRemoteAddress();
    m_info.remote_port = m_socket->getRemotePort();
    m_info.state = ConnectionState::Disconnected;
}

Connection::~Connection() {
    stop();
}

bool Connection::start() {
    if (m_running) {
        ORACON_LOG_WARNING("Connection already started");
        return false;
    }

    setState(ConnectionState::Connecting);

    // Wait for HELLO message
    std::vector<core::u8> header_buffer(16);
    if (m_socket->receive(header_buffer.data(), 16) != 16) {
        ORACON_LOG_ERROR("Failed to receive HELLO header");
        setState(ConnectionState::Error);
        return false;
    }

    auto header = protocol::MessageHeader::deserialize(header_buffer.data(), 16);
    if (!header.isValid() || header.type != protocol::MessageType::HELLO) {
        ORACON_LOG_ERROR("Invalid HELLO message");
        setState(ConnectionState::Error);
        return false;
    }

    // Receive payload
    std::vector<core::u8> payload_buffer(header.payload_length);
    core::u32 received = 0;
    while (received < header.payload_length) {
        core::i32 n = m_socket->receive(payload_buffer.data() + received,
                                        header.payload_length - received);
        if (n <= 0) {
            ORACON_LOG_ERROR("Failed to receive HELLO payload");
            setState(ConnectionState::Error);
            return false;
        }
        received += n;
    }

    auto hello_msg = protocol::HelloMessage::deserialize(payload_buffer.data(),
                                                          payload_buffer.size());
    if (!hello_msg) {
        ORACON_LOG_ERROR("Failed to parse HELLO message");
        setState(ConnectionState::Error);
        return false;
    }

    m_info.hostname = hello_msg->getHostname();
    m_info.os_type = hello_msg->getOSType();

    ORACON_LOG_INFO("Connection ", m_id, " established from ",
                    m_info.hostname, " (", m_info.os_type, ")");

    setState(ConnectionState::Connected);

    // Start receive thread
    m_running = true;
    m_receive_thread = std::thread(&Connection::receiveLoop, this);

    return true;
}

void Connection::stop() {
    if (!m_running) {
        return;
    }

    m_running = false;
    setState(ConnectionState::Closing);

    // Close socket to unblock receive
    if (m_socket) {
        m_socket->close();
    }

    // Wait for thread to finish
    if (m_receive_thread.joinable()) {
        m_receive_thread.join();
    }

    setState(ConnectionState::Disconnected);
    ORACON_LOG_INFO("Connection ", m_id, " stopped");
}

bool Connection::sendMessage(const protocol::Message& message) {
    if (!m_socket || !m_socket->isConnected()) {
        return false;
    }

    auto data = message.serialize();
    return m_socket->send(data.data(), data.size()) > 0;
}

void Connection::setMessageCallback(MessageCallback callback) {
    std::lock_guard<std::mutex> lock(m_callback_mutex);
    m_message_callback = callback;
}

void Connection::receiveLoop() {
    ORACON_LOG_DEBUG("Receive loop started for connection ", m_id);

    while (m_running) {
        // Receive message header
        std::vector<core::u8> header_buffer(16);
        core::i32 n = m_socket->receive(header_buffer.data(), 16);

        if (n <= 0) {
            if (m_running) {
                ORACON_LOG_WARNING("Connection ", m_id, " lost");
                setState(ConnectionState::Error);
            }
            break;
        }

        if (n != 16) {
            ORACON_LOG_ERROR("Incomplete header received");
            continue;
        }

        auto header = protocol::MessageHeader::deserialize(header_buffer.data(), 16);
        if (!header.isValid()) {
            ORACON_LOG_ERROR("Invalid message header");
            continue;
        }

        // Receive payload
        std::vector<core::u8> payload_buffer(header.payload_length);
        core::u32 received = 0;

        while (received < header.payload_length && m_running) {
            n = m_socket->receive(payload_buffer.data() + received,
                                 header.payload_length - received);
            if (n <= 0) {
                ORACON_LOG_ERROR("Failed to receive payload");
                break;
            }
            received += n;
        }

        if (received != header.payload_length) {
            ORACON_LOG_ERROR("Incomplete payload received");
            continue;
        }

        // Parse message
        auto message = protocol::MessageParser::parse(
            header_buffer.data(),
            header_buffer.size() + payload_buffer.size()
        );

        if (message) {
            processMessage(std::move(message));
        }
    }

    ORACON_LOG_DEBUG("Receive loop ended for connection ", m_id);
}

void Connection::processMessage(std::unique_ptr<protocol::Message> message) {
    std::lock_guard<std::mutex> lock(m_callback_mutex);

    if (m_message_callback) {
        m_message_callback(std::move(message));
    } else {
        ORACON_LOG_DEBUG("Received message but no callback set");
    }
}

void Connection::setState(ConnectionState state) {
    m_state = state;
    m_info.state = state;
}

} // namespace network
} // namespace integrate
} // namespace oracon
