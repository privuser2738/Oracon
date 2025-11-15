#include "oracon/integrate/protocol/message.h"
#include <cstring>
#include <algorithm>

namespace oracon {
namespace integrate {
namespace protocol {

// MessageHeader implementation
MessageHeader::MessageHeader()
    : magic(PROTOCOL_MAGIC)
    , version(PROTOCOL_VERSION)
    , type(MessageType::HELLO)
    , payload_length(0)
    , flags(MessageFlags::NONE)
    , reserved(0)
{}

MessageHeader::MessageHeader(MessageType msg_type, u32 length, u16 msg_flags)
    : magic(PROTOCOL_MAGIC)
    , version(PROTOCOL_VERSION)
    , type(msg_type)
    , payload_length(length)
    , flags(msg_flags)
    , reserved(0)
{}

bool MessageHeader::isValid() const {
    return magic == PROTOCOL_MAGIC && version == PROTOCOL_VERSION;
}

std::vector<u8> MessageHeader::serialize() const {
    std::vector<u8> buffer(16);
    u8* ptr = buffer.data();

    // Magic (4 bytes, big-endian)
    *ptr++ = (magic >> 24) & 0xFF;
    *ptr++ = (magic >> 16) & 0xFF;
    *ptr++ = (magic >> 8) & 0xFF;
    *ptr++ = magic & 0xFF;

    // Version (2 bytes, big-endian)
    *ptr++ = (version >> 8) & 0xFF;
    *ptr++ = version & 0xFF;

    // Type (2 bytes, big-endian)
    u16 type_val = static_cast<u16>(type);
    *ptr++ = (type_val >> 8) & 0xFF;
    *ptr++ = type_val & 0xFF;

    // Payload length (4 bytes, big-endian)
    *ptr++ = (payload_length >> 24) & 0xFF;
    *ptr++ = (payload_length >> 16) & 0xFF;
    *ptr++ = (payload_length >> 8) & 0xFF;
    *ptr++ = payload_length & 0xFF;

    // Flags (2 bytes, big-endian)
    *ptr++ = (flags >> 8) & 0xFF;
    *ptr++ = flags & 0xFF;

    // Reserved (2 bytes)
    *ptr++ = 0;
    *ptr++ = 0;

    return buffer;
}

MessageHeader MessageHeader::deserialize(const u8* data, usize size) {
    MessageHeader header;

    if (size < 16) {
        header.magic = 0; // Invalid
        return header;
    }

    const u8* ptr = data;

    // Magic
    header.magic = (static_cast<u32>(ptr[0]) << 24) |
                   (static_cast<u32>(ptr[1]) << 16) |
                   (static_cast<u32>(ptr[2]) << 8) |
                   static_cast<u32>(ptr[3]);
    ptr += 4;

    // Version
    header.version = (static_cast<u16>(ptr[0]) << 8) | static_cast<u16>(ptr[1]);
    ptr += 2;

    // Type
    u16 type_val = (static_cast<u16>(ptr[0]) << 8) | static_cast<u16>(ptr[1]);
    header.type = static_cast<MessageType>(type_val);
    ptr += 2;

    // Payload length
    header.payload_length = (static_cast<u32>(ptr[0]) << 24) |
                            (static_cast<u32>(ptr[1]) << 16) |
                            (static_cast<u32>(ptr[2]) << 8) |
                            static_cast<u32>(ptr[3]);
    ptr += 4;

    // Flags
    header.flags = (static_cast<u16>(ptr[0]) << 8) | static_cast<u16>(ptr[1]);
    ptr += 2;

    // Reserved (skip 2 bytes)

    return header;
}

// HelloMessage implementation
HelloMessage::HelloMessage(const String& hostname, const String& os_type)
    : Message(MessageType::HELLO)
    , m_hostname(hostname)
    , m_os_type(os_type)
{}

std::vector<u8> HelloMessage::serialize() const {
    auto payload = getPayload();
    MessageHeader header(m_type, static_cast<u32>(payload.size()), m_flags);
    auto header_bytes = header.serialize();

    header_bytes.insert(header_bytes.end(), payload.begin(), payload.end());
    return header_bytes;
}

std::vector<u8> HelloMessage::getPayload() const {
    std::vector<u8> payload;

    // Hostname length (2 bytes) + hostname
    u16 hostname_len = static_cast<u16>(m_hostname.length());
    payload.push_back((hostname_len >> 8) & 0xFF);
    payload.push_back(hostname_len & 0xFF);
    payload.insert(payload.end(), m_hostname.begin(), m_hostname.end());

    // OS type length (2 bytes) + os_type
    u16 os_len = static_cast<u16>(m_os_type.length());
    payload.push_back((os_len >> 8) & 0xFF);
    payload.push_back(os_len & 0xFF);
    payload.insert(payload.end(), m_os_type.begin(), m_os_type.end());

    return payload;
}

std::unique_ptr<HelloMessage> HelloMessage::deserialize(const u8* data, usize size) {
    if (size < 4) return nullptr;

    const u8* ptr = data;

    // Hostname length
    u16 hostname_len = (static_cast<u16>(ptr[0]) << 8) | static_cast<u16>(ptr[1]);
    ptr += 2;

    if (size < static_cast<usize>(4 + hostname_len)) return nullptr;

    // Hostname
    String hostname(reinterpret_cast<const char*>(ptr), hostname_len);
    ptr += hostname_len;

    // OS type length
    u16 os_len = (static_cast<u16>(ptr[0]) << 8) | static_cast<u16>(ptr[1]);
    ptr += 2;

    if (size < static_cast<usize>(4 + hostname_len + os_len)) return nullptr;

    // OS type
    String os_type(reinterpret_cast<const char*>(ptr), os_len);

    return std::make_unique<HelloMessage>(hostname, os_type);
}

// Placeholder implementations for other message types
AuthMessage::AuthMessage(const std::vector<u8>& auth_data)
    : Message(MessageType::AUTH), m_auth_data(auth_data) {}

std::vector<u8> AuthMessage::serialize() const {
    auto payload = getPayload();
    MessageHeader header(m_type, static_cast<u32>(payload.size()), m_flags);
    auto header_bytes = header.serialize();
    header_bytes.insert(header_bytes.end(), payload.begin(), payload.end());
    return header_bytes;
}

std::vector<u8> AuthMessage::getPayload() const {
    return m_auth_data;
}

std::unique_ptr<AuthMessage> AuthMessage::deserialize(const u8* data, usize size) {
    std::vector<u8> auth_data(data, data + size);
    return std::make_unique<AuthMessage>(auth_data);
}

// FileTransferStartMessage
FileTransferStartMessage::FileTransferStartMessage(const String& filename, u64 file_size)
    : Message(MessageType::FILE_TRANSFER_START), m_filename(filename), m_file_size(file_size) {}

std::vector<u8> FileTransferStartMessage::serialize() const {
    auto payload = getPayload();
    MessageHeader header(m_type, static_cast<u32>(payload.size()), m_flags);
    auto header_bytes = header.serialize();
    header_bytes.insert(header_bytes.end(), payload.begin(), payload.end());
    return header_bytes;
}

std::vector<u8> FileTransferStartMessage::getPayload() const {
    std::vector<u8> payload;

    // File size (8 bytes, big-endian)
    for (int i = 7; i >= 0; i--) {
        payload.push_back((m_file_size >> (i * 8)) & 0xFF);
    }

    // Filename length (2 bytes) + filename
    u16 filename_len = static_cast<u16>(m_filename.length());
    payload.push_back((filename_len >> 8) & 0xFF);
    payload.push_back(filename_len & 0xFF);
    payload.insert(payload.end(), m_filename.begin(), m_filename.end());

    return payload;
}

std::unique_ptr<FileTransferStartMessage> FileTransferStartMessage::deserialize(const u8* data, usize size) {
    if (size < 10) return nullptr;

    const u8* ptr = data;

    // File size
    u64 file_size = 0;
    for (int i = 0; i < 8; i++) {
        file_size = (file_size << 8) | ptr[i];
    }
    ptr += 8;

    // Filename length
    u16 filename_len = (static_cast<u16>(ptr[0]) << 8) | static_cast<u16>(ptr[1]);
    ptr += 2;

    if (size < static_cast<usize>(10 + filename_len)) return nullptr;

    // Filename
    String filename(reinterpret_cast<const char*>(ptr), filename_len);

    return std::make_unique<FileTransferStartMessage>(filename, file_size);
}

// FileTransferDataMessage
FileTransferDataMessage::FileTransferDataMessage(u64 offset, const std::vector<u8>& data)
    : Message(MessageType::FILE_TRANSFER_DATA), m_offset(offset), m_data(data) {}

std::vector<u8> FileTransferDataMessage::serialize() const {
    auto payload = getPayload();
    MessageHeader header(m_type, static_cast<u32>(payload.size()), m_flags);
    auto header_bytes = header.serialize();
    header_bytes.insert(header_bytes.end(), payload.begin(), payload.end());
    return header_bytes;
}

std::vector<u8> FileTransferDataMessage::getPayload() const {
    std::vector<u8> payload;

    // Offset (8 bytes, big-endian)
    for (int i = 7; i >= 0; i--) {
        payload.push_back((m_offset >> (i * 8)) & 0xFF);
    }

    // Data
    payload.insert(payload.end(), m_data.begin(), m_data.end());

    return payload;
}

std::unique_ptr<FileTransferDataMessage> FileTransferDataMessage::deserialize(const u8* data, usize size) {
    if (size < 8) return nullptr;

    const u8* ptr = data;

    // Offset
    u64 offset = 0;
    for (int i = 0; i < 8; i++) {
        offset = (offset << 8) | ptr[i];
    }
    ptr += 8;

    // Data
    std::vector<u8> file_data(ptr, data + size);

    return std::make_unique<FileTransferDataMessage>(offset, file_data);
}

// TerminalDataMessage
TerminalDataMessage::TerminalDataMessage(u32 terminal_id, const std::vector<u8>& data)
    : Message(MessageType::TERMINAL_DATA), m_terminal_id(terminal_id), m_data(data) {}

std::vector<u8> TerminalDataMessage::serialize() const {
    auto payload = getPayload();
    MessageHeader header(m_type, static_cast<u32>(payload.size()), m_flags);
    auto header_bytes = header.serialize();
    header_bytes.insert(header_bytes.end(), payload.begin(), payload.end());
    return header_bytes;
}

std::vector<u8> TerminalDataMessage::getPayload() const {
    std::vector<u8> payload;

    // Terminal ID (4 bytes, big-endian)
    payload.push_back((m_terminal_id >> 24) & 0xFF);
    payload.push_back((m_terminal_id >> 16) & 0xFF);
    payload.push_back((m_terminal_id >> 8) & 0xFF);
    payload.push_back(m_terminal_id & 0xFF);

    // Data
    payload.insert(payload.end(), m_data.begin(), m_data.end());

    return payload;
}

std::unique_ptr<TerminalDataMessage> TerminalDataMessage::deserialize(const u8* data, usize size) {
    if (size < 4) return nullptr;

    const u8* ptr = data;

    // Terminal ID
    u32 terminal_id = (static_cast<u32>(ptr[0]) << 24) |
                      (static_cast<u32>(ptr[1]) << 16) |
                      (static_cast<u32>(ptr[2]) << 8) |
                      static_cast<u32>(ptr[3]);
    ptr += 4;

    // Data
    std::vector<u8> terminal_data(ptr, data + size);

    return std::make_unique<TerminalDataMessage>(terminal_id, terminal_data);
}

// MessageParser
std::unique_ptr<Message> MessageParser::parse(const u8* data, usize size) {
    if (size < 16) return nullptr;

    MessageHeader header = parseHeader(data, size);
    if (!header.isValid()) return nullptr;

    const u8* payload = data + 16;
    usize payload_size = header.payload_length;

    if (size < 16 + payload_size) return nullptr;

    switch (header.type) {
        case MessageType::HELLO:
            return HelloMessage::deserialize(payload, payload_size);
        case MessageType::AUTH:
            return AuthMessage::deserialize(payload, payload_size);
        case MessageType::FILE_TRANSFER_START:
            return FileTransferStartMessage::deserialize(payload, payload_size);
        case MessageType::FILE_TRANSFER_DATA:
            return FileTransferDataMessage::deserialize(payload, payload_size);
        case MessageType::TERMINAL_DATA:
            return TerminalDataMessage::deserialize(payload, payload_size);
        default:
            return nullptr;
    }
}

MessageHeader MessageParser::parseHeader(const u8* data, usize size) {
    return MessageHeader::deserialize(data, size);
}

} // namespace protocol
} // namespace integrate
} // namespace oracon
