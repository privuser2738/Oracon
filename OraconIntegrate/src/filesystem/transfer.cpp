#include "oracon/integrate/filesystem/transfer.h"
#include "oracon/integrate/protocol/message.h"
#include "oracon/core/logger.h"
#include <fstream>
#include <chrono>
#include <algorithm>
#include <thread>

#ifdef ORACON_HAS_ZSTD
#include <zstd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <libgen.h>
#endif

namespace oracon {
namespace integrate {
namespace filesystem {

FileTransfer::FileTransfer(const TransferConfig& config)
    : m_config(config)
    , m_status(TransferStatus::Idle)
    , m_bytes_transferred(0)
    , m_total_bytes(0)
    , m_speed_bps(0)
    , m_should_cancel(false)
    , m_last_bytes_transferred(0)
{}

FileTransfer::~FileTransfer() {
    cancel();
}

String FileTransfer::getFilename(const String& path) {
#ifdef _WIN32
    size_t pos = path.find_last_of("\\/");
#else
    size_t pos = path.find_last_of('/');
#endif
    if (pos != String::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

u64 FileTransfer::getFileSize(const String& path) {
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &fileInfo)) {
        LARGE_INTEGER size;
        size.HighPart = fileInfo.nFileSizeHigh;
        size.LowPart = fileInfo.nFileSizeLow;
        return static_cast<u64>(size.QuadPart);
    }
#else
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return static_cast<u64>(st.st_size);
    }
#endif
    return 0;
}

bool FileTransfer::sendFile(network::Socket& socket, const String& filepath,
                            ProgressCallback callback) {
    return sendFileImpl(socket, filepath, 0, callback);
}

bool FileTransfer::resumeSend(network::Socket& socket, const String& filepath,
                              u64 offset, ProgressCallback callback) {
    return sendFileImpl(socket, filepath, offset, callback);
}

bool FileTransfer::sendFileImpl(network::Socket& socket, const String& filepath,
                                u64 start_offset, ProgressCallback callback) {
    // Open file
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        setError("Failed to open file: " + filepath);
        return false;
    }

    // Get file size
    m_total_bytes = getFileSize(filepath);
    if (m_total_bytes == 0) {
        setError("File is empty or size cannot be determined");
        return false;
    }

    // Get filename
    String filename = getFilename(filepath);

    // Send FILE_TRANSFER_START message
    protocol::FileTransferStartMessage start_msg(filename, m_total_bytes);
    auto start_bytes = start_msg.serialize();

    if (socket.send(start_bytes.data(), start_bytes.size()) < 0) {
        setError("Failed to send start message");
        return false;
    }

    ORACON_LOG_INFO("Starting file transfer: ", filename, " (", m_total_bytes, " bytes)");

    // Seek to start offset if resuming
    if (start_offset > 0) {
        file.seekg(start_offset);
        m_bytes_transferred = start_offset;
    } else {
        m_bytes_transferred = 0;
    }

    m_status = TransferStatus::InProgress;
    m_should_cancel = false;
    m_start_time = std::chrono::steady_clock::now();
    m_last_update_time = m_start_time;
    m_last_bytes_transferred = m_bytes_transferred;

    // Transfer loop
    std::vector<u8> buffer(m_config.chunk_size);

    while (m_bytes_transferred < m_total_bytes && !m_should_cancel) {
        // Check for pause
        while (m_status == TransferStatus::Paused && !m_should_cancel) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (m_should_cancel) break;

        // Read chunk
        u64 remaining = m_total_bytes - m_bytes_transferred;
        u32 to_read = static_cast<u32>(std::min(remaining, static_cast<u64>(m_config.chunk_size)));

        file.read(reinterpret_cast<char*>(buffer.data()), to_read);
        u32 bytes_read = static_cast<u32>(file.gcount());

        if (bytes_read == 0) {
            setError("Failed to read from file");
            return false;
        }

        // Optionally compress
        std::vector<u8> data_to_send(buffer.begin(), buffer.begin() + bytes_read);
        if (m_config.use_compression) {
            data_to_send = compressData(data_to_send);
        }

        // Send FILE_TRANSFER_DATA message
        protocol::FileTransferDataMessage data_msg(m_bytes_transferred, data_to_send);
        auto data_bytes = data_msg.serialize();

        if (socket.send(data_bytes.data(), data_bytes.size()) < 0) {
            setError("Failed to send data chunk");
            return false;
        }

        m_bytes_transferred += bytes_read;

        // Update progress
        updateProgress(bytes_read, callback);

        // Wait for ACK (optional, for reliability)
        // TODO: Implement ACK mechanism
    }

    file.close();

    if (m_should_cancel) {
        m_status = TransferStatus::Cancelled;
        ORACON_LOG_INFO("File transfer cancelled");
        return false;
    }

    // Send FILE_TRANSFER_END message
    protocol::MessageHeader end_header(protocol::MessageType::FILE_TRANSFER_END, 0);
    auto end_bytes = end_header.serialize();
    socket.send(end_bytes.data(), end_bytes.size());

    m_status = TransferStatus::Completed;
    ORACON_LOG_INFO("File transfer completed: ", m_bytes_transferred, " bytes sent");

    return true;
}

bool FileTransfer::receiveFile(network::Socket& socket, const String& save_path,
                               ProgressCallback callback) {
    return receiveFileImpl(socket, save_path, 0, callback);
}

bool FileTransfer::resumeReceive(network::Socket& socket, const String& save_path,
                                 u64 offset, ProgressCallback callback) {
    return receiveFileImpl(socket, save_path, offset, callback);
}

bool FileTransfer::receiveFileImpl(network::Socket& socket, const String& save_path,
                                   u64 start_offset, ProgressCallback callback) {
    // Receive FILE_TRANSFER_START message
    std::vector<u8> header_buffer(16);
    if (socket.receive(header_buffer.data(), 16) != 16) {
        setError("Failed to receive start message header");
        return false;
    }

    auto header = protocol::MessageHeader::deserialize(header_buffer.data(), 16);
    if (!header.isValid() || header.type != protocol::MessageType::FILE_TRANSFER_START) {
        setError("Invalid start message");
        return false;
    }

    // Receive payload
    std::vector<u8> payload_buffer(header.payload_length);
    u32 received = 0;
    while (received < header.payload_length) {
        core::i32 n = socket.receive(payload_buffer.data() + received, header.payload_length - received);
        if (n <= 0) {
            setError("Failed to receive start message payload");
            return false;
        }
        received += n;
    }

    auto start_msg = protocol::FileTransferStartMessage::deserialize(
        payload_buffer.data(), payload_buffer.size());

    if (!start_msg) {
        setError("Failed to parse start message");
        return false;
    }

    m_total_bytes = start_msg->getFileSize();
    String filename = start_msg->getFilename();

    ORACON_LOG_INFO("Receiving file: ", filename, " (", m_total_bytes, " bytes)");

    // Open file for writing
    std::ios::openmode mode = std::ios::binary;
    if (start_offset > 0) {
        mode |= std::ios::app;  // Append mode for resume
    }

    std::ofstream file(save_path, mode);
    if (!file.is_open()) {
        setError("Failed to open file for writing: " + save_path);
        return false;
    }

    if (start_offset > 0) {
        m_bytes_transferred = start_offset;
    } else {
        m_bytes_transferred = 0;
    }

    m_status = TransferStatus::InProgress;
    m_should_cancel = false;
    m_start_time = std::chrono::steady_clock::now();
    m_last_update_time = m_start_time;
    m_last_bytes_transferred = m_bytes_transferred;

    // Receive loop
    while (m_bytes_transferred < m_total_bytes && !m_should_cancel) {
        // Receive message header
        if (socket.receive(header_buffer.data(), 16) != 16) {
            setError("Failed to receive message header");
            return false;
        }

        header = protocol::MessageHeader::deserialize(header_buffer.data(), 16);

        if (!header.isValid()) {
            setError("Invalid message header");
            return false;
        }

        if (header.type == protocol::MessageType::FILE_TRANSFER_END) {
            break;  // Transfer complete
        }

        if (header.type != protocol::MessageType::FILE_TRANSFER_DATA) {
            setError("Unexpected message type during transfer");
            return false;
        }

        // Receive payload
        payload_buffer.resize(header.payload_length);
        received = 0;
        while (received < header.payload_length) {
            core::i32 n = socket.receive(payload_buffer.data() + received,
                                  header.payload_length - received);
            if (n <= 0) {
                setError("Failed to receive data payload");
                return false;
            }
            received += n;
        }

        auto data_msg = protocol::FileTransferDataMessage::deserialize(
            payload_buffer.data(), payload_buffer.size());

        if (!data_msg) {
            setError("Failed to parse data message");
            return false;
        }

        // Get data
        auto data = data_msg->getData();

        // Optionally decompress
        if (m_config.use_compression) {
            data = decompressData(data);
        }

        // Write to file
        file.write(reinterpret_cast<const char*>(data.data()), data.size());

        if (!file) {
            setError("Failed to write to file");
            return false;
        }

        m_bytes_transferred += data.size();

        // Update progress
        updateProgress(data.size(), callback);
    }

    file.close();

    if (m_should_cancel) {
        m_status = TransferStatus::Cancelled;
        ORACON_LOG_INFO("File transfer cancelled");
        return false;
    }

    m_status = TransferStatus::Completed;
    ORACON_LOG_INFO("File transfer completed: ", m_bytes_transferred, " bytes received");

    return true;
}

void FileTransfer::pause() {
    if (m_status == TransferStatus::InProgress) {
        m_status = TransferStatus::Paused;
    }
}

void FileTransfer::resume() {
    if (m_status == TransferStatus::Paused) {
        m_status = TransferStatus::InProgress;
    }
}

void FileTransfer::cancel() {
    m_should_cancel = true;
    m_status = TransferStatus::Cancelled;
}

void FileTransfer::updateProgress(u64 bytes, ProgressCallback& callback) {
    (void)bytes;  // Reserved for future use
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_last_update_time).count();

    // Update speed every 500ms
    if (elapsed >= 500) {
        u64 bytes_since_last = m_bytes_transferred - m_last_bytes_transferred;
        m_speed_bps = (bytes_since_last * 1000) / elapsed;  // bytes per second

        m_last_update_time = now;
        m_last_bytes_transferred = m_bytes_transferred;

        if (callback) {
            callback(m_bytes_transferred, m_total_bytes, m_speed_bps);
        }
    }
}

void FileTransfer::setError(const String& error) {
    m_last_error = error;
    m_status = TransferStatus::Failed;
    ORACON_LOG_ERROR("File transfer error: ", error);
}

std::vector<u8> FileTransfer::compressData(const std::vector<u8>& data) {
#ifdef ORACON_HAS_ZSTD
    size_t max_compressed_size = ZSTD_compressBound(data.size());
    std::vector<u8> compressed(max_compressed_size);

    size_t compressed_size = ZSTD_compress(
        compressed.data(), compressed.size(),
        data.data(), data.size(),
        3  // Compression level
    );

    if (ZSTD_isError(compressed_size)) {
        ORACON_LOG_WARNING("Compression failed, sending uncompressed");
        return data;
    }

    compressed.resize(compressed_size);
    return compressed;
#else
    return data;  // No compression support
#endif
}

std::vector<u8> FileTransfer::decompressData(const std::vector<u8>& data) {
#ifdef ORACON_HAS_ZSTD
    unsigned long long decompressed_size = ZSTD_getFrameContentSize(data.data(), data.size());

    if (decompressed_size == ZSTD_CONTENTSIZE_ERROR ||
        decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
        ORACON_LOG_WARNING("Decompression size unknown, treating as uncompressed");
        return data;
    }

    std::vector<u8> decompressed(decompressed_size);

    size_t result = ZSTD_decompress(
        decompressed.data(), decompressed.size(),
        data.data(), data.size()
    );

    if (ZSTD_isError(result)) {
        ORACON_LOG_WARNING("Decompression failed, treating as uncompressed");
        return data;
    }

    return decompressed;
#else
    return data;  // No compression support
#endif
}

} // namespace filesystem
} // namespace integrate
} // namespace oracon
