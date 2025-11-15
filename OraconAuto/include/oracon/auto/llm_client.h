#ifndef ORACON_AUTO_LLM_CLIENT_H
#define ORACON_AUTO_LLM_CLIENT_H

#include "oracon/core/types.h"
#include <vector>
#include <memory>
#include <functional>

namespace oracon {
namespace auto_ns {

using core::String;
using core::u32;
using core::f32;

// Message role in a conversation
enum class Role {
    System,
    User,
    Assistant,
    Tool
};

// Single message in a conversation
struct Message {
    Role role;
    String content;
    String name;  // Optional: function/tool name for tool messages

    Message(Role role, const String& content, const String& name = "")
        : role(role), content(content), name(name) {}

    static Message system(const String& content) {
        return Message(Role::System, content);
    }

    static Message user(const String& content) {
        return Message(Role::User, content);
    }

    static Message assistant(const String& content) {
        return Message(Role::Assistant, content);
    }

    static Message tool(const String& content, const String& name) {
        return Message(Role::Tool, content, name);
    }
};

// LLM generation parameters
struct GenerationParams {
    f32 temperature = 0.7f;
    f32 topP = 1.0f;
    u32 maxTokens = 1024;
    std::vector<String> stopSequences;
    f32 frequencyPenalty = 0.0f;
    f32 presencePenalty = 0.0f;

    GenerationParams() = default;

    GenerationParams& withTemperature(f32 temp) {
        temperature = temp;
        return *this;
    }

    GenerationParams& withMaxTokens(u32 tokens) {
        maxTokens = tokens;
        return *this;
    }

    GenerationParams& withTopP(f32 p) {
        topP = p;
        return *this;
    }
};

// LLM response
struct LLMResponse {
    String content;
    String model;
    u32 promptTokens = 0;
    u32 completionTokens = 0;
    u32 totalTokens = 0;
    String finishReason;  // "stop", "length", "tool_calls", etc.
    bool success = false;
    String error;

    u32 getTotalTokens() const { return totalTokens; }
    bool isSuccess() const { return success; }
};

// Streaming callback
using StreamCallback = std::function<void(const String& chunk)>;

// Abstract LLM client interface
class LLMClient {
public:
    virtual ~LLMClient() = default;

    // Synchronous completion
    virtual LLMResponse complete(
        const std::vector<Message>& messages,
        const GenerationParams& params = GenerationParams()
    ) = 0;

    // Streaming completion
    virtual LLMResponse streamComplete(
        const std::vector<Message>& messages,
        StreamCallback callback,
        const GenerationParams& params = GenerationParams()
    ) = 0;

    // Single prompt completion (convenience)
    virtual LLMResponse prompt(
        const String& userMessage,
        const String& systemMessage = "",
        const GenerationParams& params = GenerationParams()
    ) {
        std::vector<Message> messages;
        if (!systemMessage.empty()) {
            messages.push_back(Message::system(systemMessage));
        }
        messages.push_back(Message::user(userMessage));
        return complete(messages, params);
    }

    // Get model information
    virtual String getModelName() const = 0;
    virtual bool isAvailable() const = 0;
};

// Factory for creating LLM clients
class LLMClientFactory {
public:
    enum class Provider {
        OpenAI,      // OpenAI GPT models
        Anthropic,   // Claude models
        Local,       // Local models (llama.cpp, etc.)
        Mock         // Mock client for testing
    };

    static std::unique_ptr<LLMClient> create(
        Provider provider,
        const String& apiKey = "",
        const String& model = "",
        const String& baseUrl = ""
    );
};

} // namespace auto_ns
} // namespace oracon

#endif // ORACON_AUTO_LLM_CLIENT_H
