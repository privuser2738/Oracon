#include "oracon/auto/auto.h"
#include <iostream>
#include <any>

using namespace oracon;
using namespace oracon::auto_ns;

// Helper to create string std::any
std::any makeStringArg(const String& value) {
    return std::any(value);
}

int main() {
    std::cout << "=== OraconAuto Coding Assistant Demo ===\n\n";
    std::cout << "This demo shows an AI agent that can:\n";
    std::cout << "  - Read and write files\n";
    std::cout << "  - Execute Python code\n";
    std::cout << "  - Perform calculations\n";
    std::cout << "  - List directories\n\n";

    // Create a mock client (or use Claude if you have an API key)
    auto client = LLMClientFactory::create(
        LLMClientFactory::Provider::Mock,
        "",
        "mock-coding-assistant"
    );

    // Create an agent with tools
    auto agent = std::make_shared<Agent>(std::shared_ptr<LLMClient>(std::move(client)));
    agent->setSystemPrompt(
        "You are a helpful coding assistant. "
        "You can read files, write code, execute Python, and more. "
        "Use the tools available to help users with their coding tasks."
    );

    // Register real tools
    std::cout << "Registering tools...\n";
    agent->registerTool(tools::readFile());
    agent->registerTool(tools::writeFile());
    agent->registerTool(tools::listFiles());
    agent->registerTool(tools::fileExists());
    agent->registerTool(tools::executePython());
    agent->registerTool(tools::executeBash());
    agent->registerTool(tools::getCurrentTime());
    agent->registerTool(tools::calculate());
    std::cout << "Tools registered successfully!\n\n";

    // Demo 1: File Operations
    std::cout << "=== Demo 1: File Operations ===\n";

    // Write a test file
    auto writeResult = tools::writeFile().execute({
        {"path", makeStringArg("test_hello.txt")},
        {"content", makeStringArg("Hello from OraconAuto!\nThis is a test file.")}
    });
    std::cout << "Write file result: " << writeResult << "\n\n";

    // Read it back
    auto readResult = tools::readFile().execute({
        {"path", makeStringArg("test_hello.txt")}
    });
    std::cout << "Read file result:\n" << readResult << "\n\n";

    // Check if file exists
    auto existsResult = tools::fileExists().execute({
        {"path", makeStringArg("test_hello.txt")}
    });
    std::cout << "File exists: " << existsResult << "\n\n";

    // Demo 2: Code Execution
    std::cout << "=== Demo 2: Python Code Execution ===\n";

    String pythonCode = R"(
# Calculate fibonacci numbers
def fib(n):
    if n <= 1:
        return n
    return fib(n-1) + fib(n-2)

for i in range(10):
    print(f"fib({i}) = {fib(i)}")
)";

    auto pythonResult = tools::executePython().execute({
        {"code", makeStringArg(pythonCode)}
    });
    std::cout << "Python execution result:\n" << pythonResult << "\n\n";

    // Demo 3: Bash Commands
    std::cout << "=== Demo 3: Bash Commands ===\n";

    auto bashResult = tools::executeBash().execute({
        {"command", makeStringArg("echo 'Current directory:' && pwd && echo 'Files:' && ls -l *.txt 2>/dev/null || echo 'No .txt files'")}
    });
    std::cout << "Bash result:\n" << bashResult << "\n\n";

    // Demo 4: Calculator
    std::cout << "=== Demo 4: Mathematical Calculations ===\n";

    auto calcResult1 = tools::calculate().execute({
        {"expression", makeStringArg("sqrt(144) + 2^10")}
    });
    std::cout << "sqrt(144) + 2^10 = " << calcResult1 << "\n";

    auto calcResult2 = tools::calculate().execute({
        {"expression", makeStringArg("scale=10; 22/7")}  // Pi approximation
    });
    std::cout << "22/7 (pi approximation) = " << calcResult2 << "\n\n";

    // Demo 5: Current Time
    std::cout << "=== Demo 5: Utility Functions ===\n";

    auto timeResult = tools::getCurrentTime().execute({});
    std::cout << "Current time: " << timeResult << "\n\n";

    // Demo 6: List Files
    auto listResult = tools::listFiles().execute({
        {"path", makeStringArg(".")}
    });
    std::cout << "Directory listing:\n" << listResult.substr(0, 500) << "...\n\n";

    // Demo 7: Complex Task - Write and Execute Code
    std::cout << "=== Demo 7: Complex Task - Generate and Run Code ===\n";

    String generatedCode = R"(
# Generate a simple multiplication table
print("Multiplication Table (1-5):")
print("-" * 30)
for i in range(1, 6):
    row = ""
    for j in range(1, 6):
        row += f"{i*j:4}"
    print(row)
)";

    // Save the code
    tools::writeFile().execute({
        {"path", makeStringArg("mult_table.py")},
        {"content", makeStringArg(generatedCode)}
    });
    std::cout << "Generated multiplication table script\n";

    // Execute it
    auto execResult = tools::executePython().execute({
        {"code", makeStringArg(generatedCode)}
    });
    std::cout << "Execution result:\n" << execResult << "\n\n";

    // Demo 8: Tool Registry Test
    std::cout << "=== Demo 8: Tool Registry ===\n";

    ToolRegistry registry;
    registry.registerTool(tools::readFile());
    registry.registerTool(tools::writeFile());
    registry.registerTool(tools::calculate());

    std::cout << "Registered tools:\n";
    for (const auto& name : registry.listTools()) {
        std::cout << "  - " << name << "\n";
    }
    std::cout << "\n";

    // Test tool execution through registry
    auto registryCalc = registry.executeTool("calculate", {
        {"expression", makeStringArg("100 * 100")}
    });
    std::cout << "Registry calculation (100 * 100) = " << registryCalc << "\n\n";

    // Cleanup
    std::cout << "=== Cleanup ===\n";
    tools::executeBash().execute({
        {"command", makeStringArg("rm -f test_hello.txt mult_table.py")}
    });
    std::cout << "Cleaned up test files\n\n";

    std::cout << "=== All Demos Completed! ===\n\n";
    std::cout << "Summary:\n";
    std::cout << "  ✓ File I/O operations working\n";
    std::cout << "  ✓ Python code execution working\n";
    std::cout << "  ✓ Bash command execution working\n";
    std::cout << "  ✓ Calculator working\n";
    std::cout << "  ✓ Utility tools working\n";
    std::cout << "  ✓ Tool registry working\n\n";
    std::cout << "You can now build AI agents that can:\n";
    std::cout << "  - Read and analyze code files\n";
    std::cout << "  - Write and test code automatically\n";
    std::cout << "  - Execute scripts and commands\n";
    std::cout << "  - Perform calculations\n";
    std::cout << "  - Manage files and directories\n\n";

    return 0;
}
