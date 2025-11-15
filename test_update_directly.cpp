#include "oracon/lang/lexer/lexer.h"
#include "oracon/lang/parser/parser.h"
#include "oracon/lang/interpreter/interpreter.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace oracon;
using namespace lang;

int main() {
    std::cout << "=== Testing Update Function Directly ===\n";

    // Load script
    std::ifstream file("test_simple_update.ora");
    std::stringstream buffer;
    buffer << file.rdbuf();
    String code = buffer.str();

    // Parse
    Lexer lexer(code);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto program = parser.parse();

    if (parser.hasError()) {
        std::cout << "Parse errors:\n";
        for (const auto& err : parser.getErrors()) {
            std::cout << "  " << err << "\n";
        }
        return 1;
    }

    // Execute
    Interpreter interp;
    interp.execute(program.get());

    if (interp.hasError()) {
        std::cout << "Runtime errors:\n";
        for (const auto& err : interp.getErrors()) {
            std::cout << "  " << err << "\n";
        }
        return 1;
    }

    // Now try to call update manually
    std::cout << "\n=== Calling update() manually ===\n";

    if (!interp.getGlobalEnv().has("update")) {
        std::cout << "ERROR: update function not found in global environment!\n";
        return 1;
    }

    try {
        auto updateFn = interp.getGlobalEnv().get("update");
        std::cout << "update found, isFunction: " << updateFn.isFunction() << "\n";

        if (updateFn.isFunction()) {
            auto fn = updateFn.asFunction();
            std::cout << "Function arity: " << fn->arity() << "\n";

            // Call it 3 times
            for (int i = 0; i < 3; i++) {
                std::vector<Value> args;
                args.push_back(Value(0.016)); // ~60 FPS

                std::cout << "Calling update, iteration " << (i+1) << "\n";
                fn->call(args, &interp.getGlobalEnv());

                if (interp.hasError()) {
                    std::cout << "Error during call:\n";
                    for (const auto& err : interp.getErrors()) {
                        std::cout << "  " << err << "\n";
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
        return 1;
    }

    std::cout << "\n=== Test completed ===\n";
    return 0;
}
