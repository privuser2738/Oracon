#include "oracon/auto/auto.h"
#include <iostream>

using namespace oracon;
using namespace oracon::auto_ns;

int main() {
    std::cout << "=== OraconAuto Basic Agent Demo ===\n\n";

    // Create Mock LLM clients
    auto client1 = LLMClientFactory::create(
        LLMClientFactory::Provider::Mock,
        "",  // No API key needed for mock
        "mock-gpt-4"
    );

    std::cout << "Created LLM client: " << client1->getModelName() << "\n";
    std::cout << "Client available: " << (client1->isAvailable() ? "Yes" : "No") << "\n\n";

    // Test 1: Simple prompt
    std::cout << "=== Test 1: Simple Prompt ===\n";
    auto response1 = client1->prompt("What is 2+2?");
    std::cout << "Response:\n" << response1.content << "\n";
    std::cout << "Tokens: " << response1.totalTokens << "\n";
    std::cout << "Success: " << (response1.isSuccess() ? "Yes" : "No") << "\n\n";

    // Test 2: Conversation
    std::cout << "=== Test 2: Conversation ===\n";
    std::vector<Message> messages;
    messages.push_back(Message::system("You are a helpful math tutor."));
    messages.push_back(Message::user("Explain calculus in simple terms."));

    auto response2 = client1->complete(messages);
    std::cout << "Response:\n" << response2.content << "\n";
    std::cout << "Tokens: " << response2.totalTokens << "\n\n";

    // Test 3: Agent with memory
    std::cout << "=== Test 3: Agent with Memory ===\n";
    auto client2 = LLMClientFactory::create(
        LLMClientFactory::Provider::Mock, "", "mock-gpt-4"
    );
    auto agent = std::make_shared<Agent>(std::shared_ptr<LLMClient>(std::move(client2)));
    agent->setSystemPrompt("You are a helpful coding assistant.");

    auto result1 = agent->execute("What is a for loop?");
    std::cout << "Query 1: What is a for loop?\n";
    std::cout << "Response:\n" << result1.finalResponse << "\n";
    std::cout << "Iterations: " << result1.iterations << "\n";
    std::cout << "Tokens: " << result1.totalTokens << "\n\n";

    auto result2 = agent->execute("Can you show me an example?");
    std::cout << "Query 2: Can you show me an example?\n";
    std::cout << "Response:\n" << result2.finalResponse << "\n";
    std::cout << "Tokens: " << result2.totalTokens << "\n\n";

    // Test 4: Memory strategies
    std::cout << "=== Test 4: Memory Strategies ===\n";

    // Sliding window memory
    auto client3 = LLMClientFactory::create(
        LLMClientFactory::Provider::Mock, "", "mock-gpt-4"
    );
    auto slidingAgent = std::make_shared<Agent>(std::shared_ptr<LLMClient>(std::move(client3)));
    slidingAgent->setMemory(std::make_shared<SlidingWindowMemory>(3));
    std::cout << "Created agent with SlidingWindowMemory (window=3)\n";

    for (int i = 1; i <= 5; i++) {
        String query = "Message " + std::to_string(i);
        slidingAgent->execute(query);
        std::cout << "  Sent: " << query << ", Memory size: "
                  << slidingAgent->getHistory().size() << "\n";
    }
    std::cout << "\n";

    // Test 5: Generation parameters
    std::cout << "=== Test 5: Generation Parameters ===\n";
    GenerationParams params;
    params.withTemperature(0.9f)
          .withMaxTokens(2048)
          .withTopP(0.95f);

    auto response3 = client1->prompt("Write a creative story.", "", params);
    std::cout << "Response with custom parameters:\n" << response3.content << "\n";
    std::cout << "Temperature: 0.9, MaxTokens: 2048, TopP: 0.95\n\n";

    // Test 6: Streaming (simulated)
    std::cout << "=== Test 6: Streaming ===\n";
    std::cout << "Streaming response: ";

    client1->streamComplete(
        {Message::user("Count to 5")},
        [](const String& chunk) {
            std::cout << chunk << std::flush;
        }
    );
    std::cout << "\n\n";

    std::cout << "=== All Tests Completed! ===\n";
    return 0;
}
