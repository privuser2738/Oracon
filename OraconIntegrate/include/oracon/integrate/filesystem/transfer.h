#ifndef ORACON_INTEGRATE_FILESYSTEM_TRANSFER_H
#define ORACON_INTEGRATE_FILESYSTEM_TRANSFER_H

#include "oracon/core/types.h"
#include "oracon/integrate/network/socket.h"
#include <string>
#include <functional>
#include <fstream>
#include <memory>
#include <chrono>

namespace oracon {
namespace integrate {
namespace filesystem {

using core::u8;
using core::u32;
using core::u64;
using core::String;
using core::usize;

// Transfer direction
enum class TransferDirection {
    Send,
    Receive
};

// Transfer status
enum class TransferStatus {
    Idle,
    InProgress,
    Paused,
    Completed,
    Failed,
    Cancelled
};

// Progress callback: (bytes_transferred, total_bytes, speed_bps)
using ProgressCallback = std::function<void(u64, u64, u64)>;

// File transfer configuration
struct TransferConfig {
    u32 chunk_size = 1048576;  // 1 MB
    bool use_compression = false;
    bool allow_resume = true;
    u32 timeout_ms = 30000;  // 30 seconds
};

class FileTransfer {
public:
    FileTransfer(const TransferConfig& config = TransferConfig());
    ~FileTransfer();

    // Send file to remote
    bool sendFile(network::Socket& socket, const String& filepath,
                  ProgressCallback callback = nullptr);

    // Receive file from remote
    bool receiveFile(network::Socket& socket, const String& save_path,
                     ProgressCallback callback = nullptr);

    // Resume transfer from offset
    bool resumeSend(network::Socket& socket, const String& filepath,
                    u64 offset, ProgressCallback callback = nullptr);

    bool resumeReceive(network::Socket& socket, const String& save_path,
                       u64 offset, ProgressCallback callback = nullptr);

    // Control
    void pause();
    void resume();
    void cancel();

    // Status
    TransferStatus getStatus() const { return m_status; }
    u64 getBytesTransferred() const { return m_bytes_transferred; }
    u64 getTotalBytes() const { return m_total_bytes; }
    u64 getTransferSpeed() const { return m_speed_bps; }  // bytes per second
    String getLastError() const { return m_last_error; }

    // Get filename from path
    static String getFilename(const String& path);
    static u64 getFileSize(const String& path);

private:
    bool sendFileImpl(network::Socket& socket, const String& filepath,
                      u64 start_offset, ProgressCallback callback);
    bool receiveFileImpl(network::Socket& socket, const String& save_path,
                         u64 start_offset, ProgressCallback callback);

    std::vector<u8> compressData(const std::vector<u8>& data);
    std::vector<u8> decompressData(const std::vector<u8>& data);

    void updateProgress(u64 bytes, ProgressCallback& callback);
    void setError(const String& error);

    TransferConfig m_config;
    TransferStatus m_status;
    u64 m_bytes_transferred;
    u64 m_total_bytes;
    u64 m_speed_bps;
    String m_last_error;
    bool m_should_cancel;

    // Timing for speed calculation
    std::chrono::steady_clock::time_point m_start_time;
    std::chrono::steady_clock::time_point m_last_update_time;
    u64 m_last_bytes_transferred;
};

} // namespace filesystem
} // namespace integrate
} // namespace oracon

#endif // ORACON_INTEGRATE_FILESYSTEM_TRANSFER_H
