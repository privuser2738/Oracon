#ifndef ORACON_AUTO_MEMORY_H
#define ORACON_AUTO_MEMORY_H

#include "oracon/auto/llm_client.h"
#include <deque>

namespace oracon {
namespace auto_ns {

using core::u32;

// Abstract memory interface
class Memory {
public:
    virtual ~Memory() = default;

    virtual void addMessage(const Message& message) = 0;
    virtual std::vector<Message> getMessages() const = 0;
    virtual void clear() = 0;
    virtual u32 size() const = 0;
};

// Simple conversation memory - stores all messages
class ConversationMemory : public Memory {
public:
    explicit ConversationMemory(u32 maxMessages = 0)
        : m_maxMessages(maxMessages) {}

    void addMessage(const Message& message) override {
        m_messages.push_back(message);

        // Trim if exceeding max (keep newest)
        if (m_maxMessages > 0 && m_messages.size() > m_maxMessages) {
            m_messages.pop_front();
        }
    }

    std::vector<Message> getMessages() const override {
        return std::vector<Message>(m_messages.begin(), m_messages.end());
    }

    void clear() override {
        m_messages.clear();
    }

    u32 size() const override {
        return static_cast<u32>(m_messages.size());
    }

    void setMaxMessages(u32 max) {
        m_maxMessages = max;
        while (m_maxMessages > 0 && m_messages.size() > m_maxMessages) {
            m_messages.pop_front();
        }
    }

private:
    std::deque<Message> m_messages;
    u32 m_maxMessages;
};

// Sliding window memory - keeps only recent messages
class SlidingWindowMemory : public Memory {
public:
    explicit SlidingWindowMemory(u32 windowSize = 10)
        : m_windowSize(windowSize) {}

    void addMessage(const Message& message) override {
        m_messages.push_back(message);

        if (m_messages.size() > m_windowSize) {
            m_messages.pop_front();
        }
    }

    std::vector<Message> getMessages() const override {
        return std::vector<Message>(m_messages.begin(), m_messages.end());
    }

    void clear() override {
        m_messages.clear();
    }

    u32 size() const override {
        return static_cast<u32>(m_messages.size());
    }

    void setWindowSize(u32 size) {
        m_windowSize = size;
        while (m_messages.size() > m_windowSize) {
            m_messages.pop_front();
        }
    }

private:
    std::deque<Message> m_messages;
    u32 m_windowSize;
};

// Summary memory - periodically summarizes old messages
class SummaryMemory : public Memory {
public:
    explicit SummaryMemory(std::shared_ptr<LLMClient> llmClient, u32 summarizeThreshold = 10)
        : m_llmClient(llmClient)
        , m_summarizeThreshold(summarizeThreshold)
    {}

    void addMessage(const Message& message) override {
        m_recentMessages.push_back(message);

        // If we have too many recent messages, summarize them
        if (m_recentMessages.size() >= m_summarizeThreshold) {
            summarizeRecentMessages();
        }
    }

    std::vector<Message> getMessages() const override {
        std::vector<Message> all;

        // Add summary if exists
        if (!m_summary.empty()) {
            all.push_back(Message::system("Previous conversation summary: " + m_summary));
        }

        // Add recent messages
        all.insert(all.end(), m_recentMessages.begin(), m_recentMessages.end());

        return all;
    }

    void clear() override {
        m_summary.clear();
        m_recentMessages.clear();
    }

    u32 size() const override {
        return static_cast<u32>(m_recentMessages.size());
    }

private:
    std::shared_ptr<LLMClient> m_llmClient;
    String m_summary;
    std::vector<Message> m_recentMessages;
    u32 m_summarizeThreshold;

    void summarizeRecentMessages() {
        // Build prompt to summarize conversation
        String conversationText;
        for (const auto& msg : m_recentMessages) {
            String roleStr;
            switch (msg.role) {
                case Role::User: roleStr = "User"; break;
                case Role::Assistant: roleStr = "Assistant"; break;
                case Role::System: roleStr = "System"; break;
                case Role::Tool: roleStr = "Tool"; break;
            }
            conversationText += roleStr + ": " + msg.content + "\n\n";
        }

        String summarizationPrompt =
            "Summarize the following conversation concisely, "
            "preserving key information and context:\n\n" +
            conversationText;

        // Get summary from LLM
        LLMResponse response = m_llmClient->prompt(
            summarizationPrompt,
            "You are a helpful assistant that summarizes conversations."
        );

        if (response.isSuccess()) {
            m_summary = response.content;
        }

        // Clear recent messages after summarizing
        m_recentMessages.clear();
    }
};

// Token-based memory - keeps messages under token limit
class TokenLimitMemory : public Memory {
public:
    explicit TokenLimitMemory(u32 maxTokens = 4096)
        : m_maxTokens(maxTokens)
        , m_estimatedTokens(0)
    {}

    void addMessage(const Message& message) override {
        // Rough token estimation (4 chars H 1 token)
        u32 messageTokens = static_cast<u32>(message.content.length() / 4);

        m_messages.push_back(message);
        m_estimatedTokens += messageTokens;

        // Remove oldest messages if over limit
        while (m_estimatedTokens > m_maxTokens && m_messages.size() > 1) {
            u32 removedTokens = static_cast<u32>(m_messages.front().content.length() / 4);
            m_messages.pop_front();
            m_estimatedTokens -= removedTokens;
        }
    }

    std::vector<Message> getMessages() const override {
        return std::vector<Message>(m_messages.begin(), m_messages.end());
    }

    void clear() override {
        m_messages.clear();
        m_estimatedTokens = 0;
    }

    u32 size() const override {
        return static_cast<u32>(m_messages.size());
    }

    u32 getEstimatedTokens() const {
        return m_estimatedTokens;
    }

private:
    std::deque<Message> m_messages;
    u32 m_maxTokens;
    u32 m_estimatedTokens;
};

} // namespace auto_ns
} // namespace oracon

#endif // ORACON_AUTO_MEMORY_H
