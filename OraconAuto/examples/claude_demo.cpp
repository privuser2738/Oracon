#include "oracon/auto/auto.h"
#include <iostream>
#include <cstdlib>

using namespace oracon;
using namespace oracon::auto_ns;

int main() {
    std::cout << "=== Anthropic Claude API Demo ===\n\n";

    // Get API key from environment variable
    const char* apiKeyEnv = std::getenv("ANTHROPIC_API_KEY");
    if (!apiKeyEnv) {
        std::cout << "Error: ANTHROPIC_API_KEY environment variable not set!\n";
        std::cout << "Usage: export ANTHROPIC_API_KEY='your-api-key-here'\n";
        std::cout << "\nFalling back to Mock client for demonstration...\n\n";

        // Use mock client as fallback
        auto mockClient = LLMClientFactory::create(
            LLMClientFactory::Provider::Mock,
            "",
            "mock-claude"
        );

        auto response = mockClient->prompt("Hello!");
        std::cout << "Mock Response:\n" << response.content << "\n\n";
        return 0;
    }

    String apiKey(apiKeyEnv);

    std::cout << "API Key found (length: " << apiKey.length() << ")\n";
    std::cout << "Creating Claude client...\n\n";

    // Create Anthropic client
    auto client = LLMClientFactory::create(
        LLMClientFactory::Provider::Anthropic,
        apiKey,
        "claude-3-5-sonnet-20241022"  // Latest Sonnet model
    );

    std::cout << "Client created: " << client->getModelName() << "\n";
    std::cout << "Client available: " << (client->isAvailable() ? "Yes" : "No") << "\n\n";

    // Test 1: Simple prompt
    std::cout << "=== Test 1: Simple Question ===\n";
    std::cout << "Question: What is the capital of France?\n\n";

    auto response1 = client->prompt("What is the capital of France?");

    if (response1.isSuccess()) {
        std::cout << "Claude Response:\n" << response1.content << "\n\n";
        std::cout << "Tokens used: " << response1.totalTokens
                  << " (input: " << response1.promptTokens
                  << ", output: " << response1.completionTokens << ")\n\n";
    } else {
        std::cout << "Error: " << response1.error << "\n\n";
        return 1;
    }

    // Test 2: Conversation with system prompt
    std::cout << "=== Test 2: Conversation with System Prompt ===\n";

    std::vector<Message> messages;
    messages.push_back(Message::system("You are a helpful coding assistant. Keep responses concise."));
    messages.push_back(Message::user("Explain what a hash table is in one sentence."));

    auto response2 = client->complete(messages);

    if (response2.isSuccess()) {
        std::cout << "Claude Response:\n" << response2.content << "\n\n";
        std::cout << "Tokens: " << response2.totalTokens << "\n\n";
    } else {
        std::cout << "Error: " << response2.error << "\n\n";
    }

    // Test 3: Agent with Claude
    std::cout << "=== Test 3: Agent with Claude ===\n";

    auto client2 = LLMClientFactory::create(
        LLMClientFactory::Provider::Anthropic,
        apiKey,
        "claude-3-5-sonnet-20241022"
    );

    auto agent = std::make_shared<Agent>(std::shared_ptr<LLMClient>(std::move(client2)));
    agent->setSystemPrompt("You are a helpful assistant. Be concise.");

    std::cout << "Query: Write a haiku about coding\n\n";
    auto result = agent->execute("Write a haiku about coding");

    if (result.isSuccess()) {
        std::cout << "Claude's Haiku:\n" << result.finalResponse << "\n\n";
        std::cout << "Iterations: " << result.iterations << "\n";
        std::cout << "Total tokens: " << result.totalTokens << "\n\n";
    } else {
        std::cout << "Error: " << result.error << "\n\n";
    }

    // Test 4: Streaming response
    std::cout << "=== Test 4: Streaming Response ===\n";
    std::cout << "Question: Count from 1 to 5 and explain each number\n\n";
    std::cout << "Streaming output:\n";

    auto client3 = LLMClientFactory::create(
        LLMClientFactory::Provider::Anthropic,
        apiKey,
        "claude-3-5-sonnet-20241022"
    );

    auto streamResponse = client3->streamComplete(
        {Message::user("Count from 1 to 5 and explain each number briefly.")},
        [](const String& chunk) {
            std::cout << chunk << std::flush;
        }
    );

    std::cout << "\n\n";

    if (!streamResponse.isSuccess()) {
        std::cout << "Streaming error: " << streamResponse.error << "\n";
    }

    std::cout << "=== All Tests Completed! ===\n";
    return 0;
}
