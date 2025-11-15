#ifndef ORACON_AUTO_AGENT_H
#define ORACON_AUTO_AGENT_H

#include "oracon/auto/llm_client.h"
#include "oracon/auto/tool.h"
#include "oracon/auto/memory.h"
#include <memory>

namespace oracon {
namespace auto_ns {

// Agent configuration
struct AgentConfig {
    String name;
    String systemPrompt;
    GenerationParams generationParams;
    u32 maxIterations = 10;  // Max tool-calling iterations
    bool verbose = false;

    AgentConfig(const String& name = "Agent", const String& prompt = "")
        : name(name), systemPrompt(prompt) {}
};

// Agent execution result
struct AgentResult {
    String finalResponse;
    u32 iterations = 0;
    u32 totalTokens = 0;
    bool success = false;
    String error;
    std::vector<String> toolCalls;  // History of tool calls made

    bool isSuccess() const { return success; }
};

// AI Agent that can use tools and maintain conversation
class Agent {
public:
    Agent(std::shared_ptr<LLMClient> llmClient, const AgentConfig& config = AgentConfig())
        : m_llmClient(llmClient)
        , m_config(config)
        , m_memory(std::make_shared<ConversationMemory>())
    {}

    // Set system prompt
    void setSystemPrompt(const String& prompt) {
        m_config.systemPrompt = prompt;
    }

    // Register a tool
    void registerTool(const Tool& tool) {
        m_tools.registerTool(tool);
    }

    // Execute a single query
    AgentResult execute(const String& query) {
        AgentResult result;
        result.success = false;

        // Add user message
        std::vector<Message> messages;
        if (!m_config.systemPrompt.empty()) {
            messages.push_back(Message::system(m_config.systemPrompt));
        }

        // Add conversation history
        auto history = m_memory->getMessages();
        messages.insert(messages.end(), history.begin(), history.end());

        // Add current query
        messages.push_back(Message::user(query));
        m_memory->addMessage(Message::user(query));

        // Main agent loop
        for (u32 iter = 0; iter < m_config.maxIterations; iter++) {
            result.iterations = iter + 1;

            // Get LLM response
            LLMResponse response = m_llmClient->complete(messages, m_config.generationParams);

            if (!response.isSuccess()) {
                result.error = response.error;
                return result;
            }

            result.totalTokens += response.getTotalTokens();
            result.finalResponse = response.content;

            // Check if LLM wants to call a tool
            // (In a real implementation, this would parse function calling format)
            if (response.finishReason == "stop") {
                // Normal completion, no tool calls needed
                m_memory->addMessage(Message::assistant(response.content));
                result.success = true;
                break;
            }

            // If tool call requested, execute it and continue
            // (Simplified - real implementation would parse structured tool calls)
            // For now, just return the response
            m_memory->addMessage(Message::assistant(response.content));
            result.success = true;
            break;
        }

        if (result.iterations >= m_config.maxIterations) {
            result.error = "Max iterations reached";
        }

        return result;
    }

    // Stream execution with callback
    AgentResult executeStreaming(const String& query, StreamCallback callback) {
        AgentResult result;

        std::vector<Message> messages;
        if (!m_config.systemPrompt.empty()) {
            messages.push_back(Message::system(m_config.systemPrompt));
        }

        auto history = m_memory->getMessages();
        messages.insert(messages.end(), history.begin(), history.end());
        messages.push_back(Message::user(query));

        LLMResponse response = m_llmClient->streamComplete(messages, callback, m_config.generationParams);

        result.finalResponse = response.content;
        result.totalTokens = response.getTotalTokens();
        result.success = response.isSuccess();
        result.error = response.error;

        if (result.success) {
            m_memory->addMessage(Message::user(query));
            m_memory->addMessage(Message::assistant(response.content));
        }

        return result;
    }

    // Clear conversation history
    void clearHistory() {
        m_memory->clear();
    }

    // Get conversation history
    std::vector<Message> getHistory() const {
        return m_memory->getMessages();
    }

    // Set memory backend
    void setMemory(std::shared_ptr<Memory> memory) {
        m_memory = memory;
    }

private:
    std::shared_ptr<LLMClient> m_llmClient;
    AgentConfig m_config;
    ToolRegistry m_tools;
    std::shared_ptr<Memory> m_memory;
};

// Specialized agents
class CodeAgent : public Agent {
public:
    CodeAgent(std::shared_ptr<LLMClient> llmClient)
        : Agent(llmClient, AgentConfig("CodeAgent",
            "You are an expert software engineer. "
            "You write clean, efficient, and well-documented code. "
            "Always consider edge cases and error handling."))
    {
        // Register code-related tools
        registerTool(tools::readFile());
        registerTool(tools::writeFile());
        registerTool(tools::executePython());
        registerTool(tools::executeBash());
    }
};

class ResearchAgent : public Agent {
public:
    ResearchAgent(std::shared_ptr<LLMClient> llmClient)
        : Agent(llmClient, AgentConfig("ResearchAgent",
            "You are a research assistant. "
            "You gather information from multiple sources "
            "and provide comprehensive, accurate answers."))
    {
        // Register research tools
        registerTool(tools::readFile());
        registerTool(tools::listFiles());
        registerTool(tools::executeBash());
    }
};

} // namespace auto_ns
} // namespace oracon

#endif // ORACON_AUTO_AGENT_H
