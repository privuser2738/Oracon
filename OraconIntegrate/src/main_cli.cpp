#include "oracon/integrate/config.h"
#include "oracon/integrate/network/socket.h"
#include "oracon/integrate/protocol/message.h"
#include "oracon/core/logger.h"
#include "oracon/core/common.h"
#include <iostream>
#include <string>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

using namespace oracon;
using namespace oracon::integrate;

void printUsage(const char* program) {
    std::cout << "OraconIntegrate v" << INTEGRATE_VERSION_MAJOR << "."
              << INTEGRATE_VERSION_MINOR << "." << INTEGRATE_VERSION_PATCH << std::endl;
    std::cout << "Cross-platform remote desktop, file sharing, and system integration" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: " << program << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help              Show this help message" << std::endl;
    std::cout << "  -v, --version           Show version information" << std::endl;
    std::cout << "  --server                Start in server mode" << std::endl;
    std::cout << "  --connect HOST[:PORT]   Connect to remote host" << std::endl;
    std::cout << "  --port PORT             Set listening port (default: 7788)" << std::endl;
    std::cout << "  --send FILE --to HOST   Send file to remote host" << std::endl;
    std::cout << "  --shell HOST            Open remote shell" << std::endl;
    std::cout << "  --list                  List connected devices" << std::endl;
    std::cout << "  --install-service       Install as system service" << std::endl;
    std::cout << "  --config FILE           Load configuration file" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    // Initialize socket library
    network::SocketInitializer socket_init;
    if (!socket_init.isInitialized()) {
        std::cerr << "Failed to initialize socket library" << std::endl;
        return 1;
    }

    // Parse command line arguments
    bool server_mode = false;
    std::string connect_host;
    u16 port = DEFAULT_PORT;
    std::string config_file;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "OraconIntegrate v" << INTEGRATE_VERSION_MAJOR << "."
                      << INTEGRATE_VERSION_MINOR << "." << INTEGRATE_VERSION_PATCH << std::endl;
            std::cout << "Protocol version: " << PROTOCOL_VERSION << std::endl;
            return 0;
        } else if (arg == "--server") {
            server_mode = true;
        } else if (arg == "--connect") {
            if (i + 1 < argc) {
                connect_host = argv[++i];
            }
        } else if (arg == "--port") {
            if (i + 1 < argc) {
                port = static_cast<u16>(std::stoi(argv[++i]));
            }
        } else if (arg == "--config") {
            if (i + 1 < argc) {
                config_file = argv[++i];
            }
        } else if (arg == "--install-service") {
            std::cout << "Service installation not yet implemented" << std::endl;
            return 1;
        } else if (arg == "--list") {
            std::cout << "Device listing not yet implemented" << std::endl;
            return 1;
        }
    }

    core::Logger::getInstance().setLevel(core::LogLevel::Info);

    if (server_mode) {
        ORACON_LOG_INFO("Starting OraconIntegrate in server mode on port ", port);

        // Create server socket
        network::Socket server_socket(network::SocketType::TCP);
        if (!server_socket.isValid()) {
            ORACON_LOG_ERROR("Failed to create server socket");
            return 1;
        }

        server_socket.setReuseAddress(true);

        if (!server_socket.bind("0.0.0.0", port)) {
            ORACON_LOG_ERROR("Failed to bind to port ", port);
            return 1;
        }

        if (!server_socket.listen()) {
            ORACON_LOG_ERROR("Failed to listen on socket");
            return 1;
        }

        ORACON_LOG_INFO("Server listening on port ", port);

        // Accept loop
        while (true) {
            ORACON_LOG_INFO("Waiting for connections...");

            auto client = server_socket.accept();
            if (client) {
                ORACON_LOG_INFO("Client connected from ",
                               client->getRemoteAddress(), ":", client->getRemotePort());

                // TODO: Handle client in separate thread
                // For now, just close it
                client->close();
            }
        }

    } else if (!connect_host.empty()) {
        ORACON_LOG_INFO("Connecting to ", connect_host);

        // Parse host:port
        std::string host = connect_host;
        u16 remote_port = DEFAULT_PORT;

        size_t colon_pos = connect_host.find(':');
        if (colon_pos != std::string::npos) {
            host = connect_host.substr(0, colon_pos);
            remote_port = static_cast<u16>(std::stoi(connect_host.substr(colon_pos + 1)));
        }

        // Create client socket
        network::Socket client_socket(network::SocketType::TCP);
        if (!client_socket.isValid()) {
            ORACON_LOG_ERROR("Failed to create client socket");
            return 1;
        }

        if (!client_socket.connect(host, remote_port)) {
            ORACON_LOG_ERROR("Failed to connect to ", host, ":", remote_port);
            return 1;
        }

        ORACON_LOG_INFO("Connected to ", host, ":", remote_port);

        // Send HELLO message
        #ifdef _WIN32
        std::string os_type = "Windows";
        #elif __APPLE__
        std::string os_type = "macOS";
        #else
        std::string os_type = "Linux";
        #endif

        char hostname[256];
        #ifdef _WIN32
        DWORD size = sizeof(hostname);
        GetComputerNameA(hostname, &size);
        #else
        gethostname(hostname, sizeof(hostname));
        #endif

        protocol::HelloMessage hello(hostname, os_type);
        auto hello_bytes = hello.serialize();

        client_socket.send(hello_bytes.data(), hello_bytes.size());

        ORACON_LOG_INFO("Sent HELLO message");

        // TODO: Implement client protocol handling

        client_socket.close();

    } else {
        std::cout << "No mode specified. Use --help for usage information." << std::endl;
        std::cout << "Starting interactive mode..." << std::endl;
        std::cout << "(Interactive mode not yet implemented)" << std::endl;
        return 1;
    }

    return 0;
}
