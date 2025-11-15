#include "oracon/lang/lexer/lexer.h"
#include "oracon/lang/parser/parser.h"
#include "oracon/lang/interpreter/interpreter.h"
#include <iostream>

using namespace oracon;
using namespace lang;

int main() {
    std::cout << "=== Testing Function Execution ===\n";

    String code = R"(
        let counter = 0;

        func increment() {
            print("Inside increment!");
            counter = counter + 1;
            print("Counter is now: " + type(counter));
        }

        print("Before calling increment");
        increment();
        print("After calling increment");
        print("Final counter: " + type(counter));
    )";

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

    Interpreter interp;
    interp.execute(program.get());

    if (interp.hasError()) {
        std::cout << "Runtime errors:\n";
        for (const auto& err : interp.getErrors()) {
            std::cout << "  " << err << "\n";
        }
        return 1;
    }

    std::cout << "=== Test completed ===\n";
    return 0;
}
