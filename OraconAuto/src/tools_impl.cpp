#include "oracon/auto/tool.h"
#include "oracon/core/logger.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

namespace oracon {
namespace auto_ns {
namespace tools {

// ===== File System Tools =====

Tool readFileImpl() {
    return Tool("read_file", "Read the contents of a file",
        [](const std::map<String, std::any>& args) -> String {
            try {
                String path = std::any_cast<String>(args.at("path"));

                std::ifstream file(path);
                if (!file.is_open()) {
                    return "Error: Could not open file: " + path;
                }

                std::stringstream buffer;
                buffer << file.rdbuf();
                String content = buffer.str();

                if (content.empty()) {
                    return "File is empty: " + path;
                }

                // Limit size for safety
                if (content.length() > 100000) {
                    return "Error: File too large (>100KB): " + path;
                }

                return content;
            } catch (const std::bad_any_cast& e) {
                return "Error: Invalid argument type for 'path'";
            } catch (const std::exception& e) {
                return String("Error reading file: ") + e.what();
            }
        })
        .addParameter("path", "string", "Path to the file to read");
}

Tool writeFileImpl() {
    return Tool("write_file", "Write content to a file",
        [](const std::map<String, std::any>& args) -> String {
            try {
                String path = std::any_cast<String>(args.at("path"));
                String content = std::any_cast<String>(args.at("content"));

                // Safety check - don't allow absolute paths outside current dir
                if (path.empty() || path[0] == '/') {
                    return "Error: Only relative paths allowed for safety";
                }

                std::ofstream file(path);
                if (!file.is_open()) {
                    return "Error: Could not open file for writing: " + path;
                }

                file << content;
                file.close();

                return "Successfully wrote " + std::to_string(content.length()) +
                       " bytes to: " + path;
            } catch (const std::bad_any_cast& e) {
                return "Error: Invalid argument types";
            } catch (const std::exception& e) {
                return String("Error writing file: ") + e.what();
            }
        })
        .addParameter("path", "string", "Path to the file to write")
        .addParameter("content", "string", "Content to write to the file");
}

Tool listFilesImpl() {
    return Tool("list_files", "List files in a directory",
        [](const std::map<String, std::any>& args) -> String {
            try {
                String path = ".";  // Default to current directory
                if (args.find("path") != args.end()) {
                    path = std::any_cast<String>(args.at("path"));
                }

                // Use system command for simplicity
                String command = "ls -la " + path + " 2>&1";
                FILE* pipe = popen(command.c_str(), "r");
                if (!pipe) {
                    return "Error: Could not execute ls command";
                }

                char buffer[256];
                String result;
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    result += buffer;
                }

                int status = pclose(pipe);
                if (status != 0) {
                    return "Error listing directory: " + path;
                }

                return result;
            } catch (const std::exception& e) {
                return String("Error: ") + e.what();
            }
        })
        .addParameter("path", "string", "Directory path to list", false);
}

Tool fileExistsImpl() {
    return Tool("file_exists", "Check if a file exists",
        [](const std::map<String, std::any>& args) -> String {
            try {
                String path = std::any_cast<String>(args.at("path"));

                struct stat buffer;
                bool exists = (stat(path.c_str(), &buffer) == 0);

                return exists ? "true" : "false";
            } catch (const std::exception& e) {
                return String("Error: ") + e.what();
            }
        })
        .addParameter("path", "string", "Path to check");
}

// ===== Code Execution Tools =====

Tool executeBashImpl() {
    return Tool("execute_bash", "Execute a bash command safely",
        [](const std::map<String, std::any>& args) -> String {
            try {
                String command = std::any_cast<String>(args.at("command"));

                // Safety checks
                if (command.find("rm -rf") != String::npos ||
                    command.find("sudo") != String::npos ||
                    command.find("su ") != String::npos) {
                    return "Error: Dangerous command blocked for safety";
                }

                // Add timeout and size limits
                String safeCommand = "timeout 10s bash -c '" + command + "' 2>&1 | head -c 10000";

                FILE* pipe = popen(safeCommand.c_str(), "r");
                if (!pipe) {
                    return "Error: Could not execute command";
                }

                char buffer[256];
                String result;
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    result += buffer;
                }

                int exitCode = pclose(pipe);
                int realExitCode = WEXITSTATUS(exitCode);

                result += "\n[Exit code: " + std::to_string(realExitCode) + "]";

                return result;
            } catch (const std::exception& e) {
                return String("Error executing command: ") + e.what();
            }
        })
        .addParameter("command", "string", "Bash command to execute");
}

Tool executePythonImpl() {
    return Tool("execute_python", "Execute Python code safely",
        [](const std::map<String, std::any>& args) -> String {
            try {
                String code = std::any_cast<String>(args.at("code"));

                // Write code to temporary file
                String tempFile = "/tmp/oracon_python_" + std::to_string(getpid()) + ".py";

                std::ofstream file(tempFile);
                if (!file.is_open()) {
                    return "Error: Could not create temp file";
                }
                file << code;
                file.close();

                // Execute with timeout
                String command = "timeout 10s python3 " + tempFile + " 2>&1 | head -c 10000";

                FILE* pipe = popen(command.c_str(), "r");
                if (!pipe) {
                    unlink(tempFile.c_str());
                    return "Error: Could not execute Python code";
                }

                char buffer[256];
                String result;
                while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    result += buffer;
                }

                int exitCode = pclose(pipe);
                unlink(tempFile.c_str());  // Clean up

                int realExitCode = WEXITSTATUS(exitCode);
                result += "\n[Exit code: " + std::to_string(realExitCode) + "]";

                return result;
            } catch (const std::exception& e) {
                return String("Error executing Python: ") + e.what();
            }
        })
        .addParameter("code", "string", "Python code to execute");
}

// ===== Utility Tools =====

Tool getCurrentTimeImpl() {
    return Tool("get_current_time", "Get the current date and time",
        [](const std::map<String, std::any>& args) -> String {
            (void)args;

            time_t now = time(nullptr);
            char buffer[100];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));

            return String(buffer);
        });
}

Tool calculateImpl() {
    return Tool("calculate", "Perform a mathematical calculation using bc",
        [](const std::map<String, std::any>& args) -> String {
            try {
                String expression = std::any_cast<String>(args.at("expression"));

                // Use bc calculator
                String command = "echo '" + expression + "' | bc -l 2>&1";

                FILE* pipe = popen(command.c_str(), "r");
                if (!pipe) {
                    return "Error: Could not execute calculator";
                }

                char buffer[256];
                String result;
                if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                    result = buffer;
                    // Remove trailing newline
                    if (!result.empty() && result.back() == '\n') {
                        result.pop_back();
                    }
                }

                pclose(pipe);

                return result.empty() ? "Error: Invalid expression" : result;
            } catch (const std::exception& e) {
                return String("Error: ") + e.what();
            }
        })
        .addParameter("expression", "string", "Mathematical expression to calculate");
}

} // namespace tools
} // namespace auto_ns
} // namespace oracon
