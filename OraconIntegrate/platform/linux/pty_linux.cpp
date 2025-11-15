#include "oracon/integrate/terminal/pty.h"
#include "oracon/core/logger.h"

#ifndef _WIN32

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#include <cstdlib>
#include <cstring>

#ifdef __APPLE__
    #include <util.h>
#else
    #include <pty.h>
#endif

namespace oracon {
namespace integrate {
namespace terminal {

PTY::PTY()
    : m_master_fd(-1)
    , m_slave_fd(-1)
    , m_child_pid(-1)
    , m_cols(80)
    , m_rows(24)
    , m_is_open(false)
{}

PTY::~PTY() {
    close();
}

PTY::PTY(PTY&& other) noexcept
    : m_master_fd(other.m_master_fd)
    , m_slave_fd(other.m_slave_fd)
    , m_child_pid(other.m_child_pid)
    , m_cols(other.m_cols)
    , m_rows(other.m_rows)
    , m_is_open(other.m_is_open)
{
    other.m_master_fd = -1;
    other.m_slave_fd = -1;
    other.m_child_pid = -1;
    other.m_is_open = false;
}

PTY& PTY::operator=(PTY&& other) noexcept {
    if (this != &other) {
        close();

        m_master_fd = other.m_master_fd;
        m_slave_fd = other.m_slave_fd;
        m_child_pid = other.m_child_pid;
        m_cols = other.m_cols;
        m_rows = other.m_rows;
        m_is_open = other.m_is_open;

        other.m_master_fd = -1;
        other.m_slave_fd = -1;
        other.m_child_pid = -1;
        other.m_is_open = false;
    }
    return *this;
}

bool PTY::create(u16 cols, u16 rows) {
    if (m_is_open) {
        ORACON_LOG_WARNING("PTY already open");
        return false;
    }

    m_cols = cols;
    m_rows = rows;

    return createPlatform();
}

bool PTY::createPlatform() {
    // Set up window size
    struct winsize ws;
    ws.ws_col = m_cols;
    ws.ws_row = m_rows;
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;

    // Create PTY
    char slave_name[256];
    if (openpty(&m_master_fd, &m_slave_fd, slave_name, nullptr, &ws) < 0) {
        ORACON_LOG_ERROR("Failed to create PTY: ", strerror(errno));
        return false;
    }

    // Set non-blocking mode on master
    int flags = fcntl(m_master_fd, F_GETFL, 0);
    fcntl(m_master_fd, F_SETFL, flags | O_NONBLOCK);

    m_is_open = true;
    ORACON_LOG_INFO("PTY created: ", slave_name);

    return true;
}

bool PTY::spawnShell(const String& shell_path) {
    if (!m_is_open) {
        ORACON_LOG_ERROR("PTY not created");
        return false;
    }

    String shell = shell_path.empty() ? getDefaultShell() : shell_path;

    return spawnShellPlatform(shell);
}

bool PTY::spawnShellPlatform(const String& shell_path) {
    m_child_pid = fork();

    if (m_child_pid < 0) {
        ORACON_LOG_ERROR("Failed to fork: ", strerror(errno));
        return false;
    }

    if (m_child_pid == 0) {
        // Child process

        // Create new session
        setsid();

        // Set controlling terminal
        if (ioctl(m_slave_fd, TIOCSCTTY, 0) < 0) {
            _exit(1);
        }

        // Redirect stdin, stdout, stderr to slave
        dup2(m_slave_fd, STDIN_FILENO);
        dup2(m_slave_fd, STDOUT_FILENO);
        dup2(m_slave_fd, STDERR_FILENO);

        // Close all other file descriptors
        for (int fd = 3; fd < 256; fd++) {
            ::close(fd);
        }

        // Set environment variables
        setenv("TERM", "xterm-256color", 1);

        // Execute shell
        execl(shell_path.c_str(), shell_path.c_str(), nullptr);

        // If execl fails
        _exit(1);
    }

    // Parent process
    ::close(m_slave_fd);
    m_slave_fd = -1;

    ORACON_LOG_INFO("Shell spawned: ", shell_path, " (PID: ", m_child_pid, ")");

    return true;
}

i32 PTY::read(void* buffer, u32 size) {
    if (!m_is_open || m_master_fd < 0) {
        return -1;
    }

    return ::read(m_master_fd, buffer, size);
}

i32 PTY::write(const void* data, u32 size) {
    if (!m_is_open || m_master_fd < 0) {
        return -1;
    }

    return ::write(m_master_fd, data, size);
}

bool PTY::resize(u16 cols, u16 rows) {
    if (!m_is_open || m_master_fd < 0) {
        return false;
    }

    m_cols = cols;
    m_rows = rows;

    struct winsize ws;
    ws.ws_col = cols;
    ws.ws_row = rows;
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;

    if (ioctl(m_master_fd, TIOCSWINSZ, &ws) < 0) {
        ORACON_LOG_ERROR("Failed to resize PTY: ", strerror(errno));
        return false;
    }

    // Send SIGWINCH to child process
    if (m_child_pid > 0) {
        kill(m_child_pid, SIGWINCH);
    }

    return true;
}

void PTY::close() {
    closePlatform();
}

void PTY::closePlatform() {
    if (m_master_fd >= 0) {
        ::close(m_master_fd);
        m_master_fd = -1;
    }

    if (m_slave_fd >= 0) {
        ::close(m_slave_fd);
        m_slave_fd = -1;
    }

    if (m_child_pid > 0) {
        // Terminate child process
        kill(m_child_pid, SIGTERM);

        // Wait for child to exit
        int status;
        waitpid(m_child_pid, &status, WNOHANG);

        m_child_pid = -1;
    }

    m_is_open = false;
}

bool PTY::isOpen() const {
    return m_is_open && m_master_fd >= 0;
}

String PTY::getDefaultShell() {
    // Try environment variable first
    const char* shell_env = getenv("SHELL");
    if (shell_env && strlen(shell_env) > 0) {
        return String(shell_env);
    }

#ifdef __APPLE__
    return "/bin/zsh";  // macOS default
#else
    return "/bin/bash";  // Linux default
#endif
}

} // namespace terminal
} // namespace integrate
} // namespace oracon

#endif // !_WIN32
