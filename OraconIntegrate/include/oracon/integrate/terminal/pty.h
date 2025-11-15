#ifndef ORACON_INTEGRATE_TERMINAL_PTY_H
#define ORACON_INTEGRATE_TERMINAL_PTY_H

#include "oracon/core/types.h"
#include <string>
#include <functional>

namespace oracon {
namespace integrate {
namespace terminal {

using core::i32;
using core::u16;
using core::u32;
using core::String;

// PTY (Pseudo Terminal) abstraction for cross-platform terminal access
class PTY {
public:
    PTY();
    ~PTY();

    // Disable copy, allow move
    PTY(const PTY&) = delete;
    PTY& operator=(const PTY&) = delete;
    PTY(PTY&& other) noexcept;
    PTY& operator=(PTY&& other) noexcept;

    // Create a new pseudo-terminal
    bool create(u16 cols = 80, u16 rows = 24);

    // Spawn a shell in the PTY
    bool spawnShell(const String& shell_path = "");

    // Read from PTY
    i32 read(void* buffer, u32 size);

    // Write to PTY
    i32 write(const void* data, u32 size);

    // Resize PTY
    bool resize(u16 cols, u16 rows);

    // Close PTY
    void close();

    // Status
    bool isOpen() const;
    i32 getProcessId() const { return m_child_pid; }

    // Get default shell for current platform
    static String getDefaultShell();

private:
    bool createPlatform();
    bool spawnShellPlatform(const String& shell_path);
    void closePlatform();

#ifdef _WIN32
    void* m_console_handle;  // HANDLE
    void* m_pipe_read;       // HANDLE
    void* m_pipe_write;      // HANDLE
    void* m_process_handle;  // HANDLE
#else
    i32 m_master_fd;
    i32 m_slave_fd;
#endif

    i32 m_child_pid;
    u16 m_cols;
    u16 m_rows;
    bool m_is_open;
};

} // namespace terminal
} // namespace integrate
} // namespace oracon

#endif // ORACON_INTEGRATE_TERMINAL_PTY_H
