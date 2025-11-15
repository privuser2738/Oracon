#include "oracon/lang/lexer/lexer.h"
#include "oracon/lang/parser/parser.h"
#include "oracon/lang/interpreter/interpreter.h"
#include "oracon/core/logger.h"
#include "oracon/core/common.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace oracon::lang;
using namespace oracon::core;

void printUsage(const char* program) {
    std::cout << "Oracon Language Interpreter v" << getVersionString() << std::endl;
    std::cout << "Usage: " << program << " [options] [file]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help     Show this help message" << std::endl;
    std::cout << "  -v, --version  Show version information" << std::endl;
    std::cout << "  -t, --tokens   Show tokens from lexer" << std::endl;
    std::cout << std::endl;
    std::cout << "If no file is provided, starts in REPL mode." << std::endl;
}

void runRepl(bool showTokens) {
    std::cout << "Oracon REPL v" << getVersionString() << std::endl;
    std::cout << "Type 'exit' or press Ctrl+D to quit." << std::endl;
    std::cout << std::endl;

    String line;
    while (true) {
        std::cout << ">>> ";

        if (!std::getline(std::cin, line)) {
            std::cout << std::endl;
            break;
        }

        if (line == "exit" || line == "quit") {
            break;
        }

        if (line.empty()) {
            continue;
        }

        // Tokenize the input
        Lexer lexer(line, "<repl>");
        auto tokens = lexer.tokenize();

        if (lexer.hasError()) {
            for (const auto& error : lexer.getErrors()) {
                std::cerr << "Error: " << error << std::endl;
            }
            continue;
        }

        if (showTokens) {
            std::cout << "Tokens:" << std::endl;
            for (const auto& token : tokens) {
                if (token.getType() != TokenType::EOF_TOKEN) {
                    std::cout << "  " << token.toString() << std::endl;
                }
            }
        }

        // Parse the tokens
        Parser parser(tokens);
        auto program = parser.parse();

        if (parser.hasError()) {
            for (const auto& error : parser.getErrors()) {
                std::cerr << "Parse error: " << error << std::endl;
            }
            continue;
        }

        // Execute the program
        Interpreter interpreter;
        interpreter.execute(program.get());

        if (interpreter.hasError()) {
            for (const auto& error : interpreter.getErrors()) {
                std::cerr << "Runtime error: " << error << std::endl;
            }
        }
    }
}

void runFile(const String& filename, bool showTokens) {
    // Read the file
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    String source = buffer.str();

    // Tokenize
    Lexer lexer(source, filename);
    auto tokens = lexer.tokenize();

    if (lexer.hasError()) {
        for (const auto& error : lexer.getErrors()) {
            std::cerr << error << std::endl;
        }
        return;
    }

    if (showTokens) {
        std::cout << "Tokens:" << std::endl;
        for (const auto& token : tokens) {
            if (token.getType() != TokenType::EOF_TOKEN) {
                std::cout << "  " << token.toString() << std::endl;
            }
        }
    }

    // Parse the tokens
    Parser parser(tokens);
    auto program = parser.parse();

    if (parser.hasError()) {
        for (const auto& error : parser.getErrors()) {
            std::cerr << "Parse error: " << error << std::endl;
        }
        return;
    }

    // Execute the program
    Interpreter interpreter;
    interpreter.execute(program.get());

    if (interpreter.hasError()) {
        for (const auto& error : interpreter.getErrors()) {
            std::cerr << "Runtime error: " << error << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    bool showTokens = false;
    String filename;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        String arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "Oracon v" << getVersionString() << std::endl;
            return 0;
        } else if (arg == "-t" || arg == "--tokens") {
            showTokens = true;
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        } else {
            filename = arg;
        }
    }

    try {
        if (filename.empty()) {
            runRepl(showTokens);
        } else {
            runFile(filename, showTokens);
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
