#ifndef ORACON_INTEGRATE_PROTOCOL_MESSAGE_H
#define ORACON_INTEGRATE_PROTOCOL_MESSAGE_H

#include "oracon/integrate/config.h"
#include "oracon/core/types.h"
#include <vector>
#include <memory>

namespace oracon {
namespace integrate {
namespace protocol {

using core::u8;
using core::u16;
using core::u32;
using core::u64;
using core::usize;
using core::String;

// Message types
enum class MessageType : u16 {
    HELLO = 0x0001,
    AUTH = 0x0002,
    AUTH_RESPONSE = 0x0003,
    PING = 0x0004,
    PONG = 0x0005,

    FILE_TRANSFER_START = 0x0010,
    FILE_TRANSFER_DATA = 0x0011,
    FILE_TRANSFER_END = 0x0012,
    FILE_TRANSFER_ACK = 0x0013,
    FILE_LIST_REQUEST = 0x0014,
    FILE_LIST_RESPONSE = 0x0015,

    TERMINAL_OPEN = 0x0020,
    TERMINAL_DATA = 0x0021,
    TERMINAL_RESIZE = 0x0022,
    TERMINAL_CLOSE = 0x0023,

    DESKTOP_STREAM_START = 0x0030,
    DESKTOP_STREAM_FRAME = 0x0031,
    DESKTOP_STREAM_STOP = 0x0032,
    DESKTOP_INPUT = 0x0033,

    CLIPBOARD_SYNC = 0x0040,

    ERROR = 0x00FF
};

// Message flags
enum MessageFlags : u16 {
    NONE = 0x0000,
    COMPRESSED = 0x0001,
    ENCRYPTED = 0x0002,
    FRAGMENTED = 0x0004,
    LAST_FRAGMENT = 0x0008
};

// Message header (16 bytes)
struct MessageHeader {
    u32 magic;           // 0x4F524143 "ORAC"
    u16 version;         // Protocol version
    MessageType type;    // Message type
    u32 payload_length;  // Length of payload
    u16 flags;           // Message flags
    u16 reserved;        // Reserved for future use

    MessageHeader();
    MessageHeader(MessageType msg_type, u32 length, u16 msg_flags = MessageFlags::NONE);

    bool isValid() const;
    std::vector<u8> serialize() const;
    static MessageHeader deserialize(const u8* data, usize size);
};

// Base message class
class Message {
public:
    virtual ~Message() = default;

    MessageType getType() const { return m_type; }
    u16 getFlags() const { return m_flags; }

    void setFlags(u16 flags) { m_flags = flags; }
    void addFlag(MessageFlags flag) { m_flags |= flag; }
    bool hasFlag(MessageFlags flag) const { return (m_flags & flag) != 0; }

    // Serialize message to bytes
    virtual std::vector<u8> serialize() const = 0;

    // Get payload (without header)
    virtual std::vector<u8> getPayload() const = 0;

protected:
    Message(MessageType type, u16 flags = MessageFlags::NONE)
        : m_type(type), m_flags(flags) {}

    MessageType m_type;
    u16 m_flags;
};

// HELLO message
class HelloMessage : public Message {
public:
    HelloMessage(const String& hostname, const String& os_type);

    const String& getHostname() const { return m_hostname; }
    const String& getOSType() const { return m_os_type; }

    std::vector<u8> serialize() const override;
    std::vector<u8> getPayload() const override;

    static std::unique_ptr<HelloMessage> deserialize(const u8* data, usize size);

private:
    String m_hostname;
    String m_os_type;
};

// AUTH message
class AuthMessage : public Message {
public:
    AuthMessage(const std::vector<u8>& auth_data);

    const std::vector<u8>& getAuthData() const { return m_auth_data; }

    std::vector<u8> serialize() const override;
    std::vector<u8> getPayload() const override;

    static std::unique_ptr<AuthMessage> deserialize(const u8* data, usize size);

private:
    std::vector<u8> m_auth_data;
};

// FILE_TRANSFER_START message
class FileTransferStartMessage : public Message {
public:
    FileTransferStartMessage(const String& filename, u64 file_size);

    const String& getFilename() const { return m_filename; }
    u64 getFileSize() const { return m_file_size; }

    std::vector<u8> serialize() const override;
    std::vector<u8> getPayload() const override;

    static std::unique_ptr<FileTransferStartMessage> deserialize(const u8* data, usize size);

private:
    String m_filename;
    u64 m_file_size;
};

// FILE_TRANSFER_DATA message
class FileTransferDataMessage : public Message {
public:
    FileTransferDataMessage(u64 offset, const std::vector<u8>& data);

    u64 getOffset() const { return m_offset; }
    const std::vector<u8>& getData() const { return m_data; }

    std::vector<u8> serialize() const override;
    std::vector<u8> getPayload() const override;

    static std::unique_ptr<FileTransferDataMessage> deserialize(const u8* data, usize size);

private:
    u64 m_offset;
    std::vector<u8> m_data;
};

// TERMINAL_DATA message
class TerminalDataMessage : public Message {
public:
    TerminalDataMessage(u32 terminal_id, const std::vector<u8>& data);

    u32 getTerminalId() const { return m_terminal_id; }
    const std::vector<u8>& getData() const { return m_data; }

    std::vector<u8> serialize() const override;
    std::vector<u8> getPayload() const override;

    static std::unique_ptr<TerminalDataMessage> deserialize(const u8* data, usize size);

private:
    u32 m_terminal_id;
    std::vector<u8> m_data;
};

// Message parser
class MessageParser {
public:
    // Parse complete message from buffer
    static std::unique_ptr<Message> parse(const u8* data, usize size);

    // Parse just the header
    static MessageHeader parseHeader(const u8* data, usize size);
};

} // namespace protocol
} // namespace integrate
} // namespace oracon

#endif // ORACON_INTEGRATE_PROTOCOL_MESSAGE_H
