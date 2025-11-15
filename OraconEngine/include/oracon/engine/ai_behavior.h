#ifndef ORACON_ENGINE_AI_BEHAVIOR_H
#define ORACON_ENGINE_AI_BEHAVIOR_H

#include "oracon/engine/component.h"
#include "oracon/auto/agent.h"
#include <memory>
#include <functional>

namespace oracon {
namespace engine {

using auto_ns::Agent;
using auto_ns::AgentConfig;
using auto_ns::AgentResult;
using auto_ns::LLMClient;

// AI Behavior component - gives entities AI-powered behavior
class AIBehavior : public Script {
public:
    // Perception callback - allows AI to perceive its environment
    using PerceptionCallback = std::function<String()>;

    // Action callback - allows AI to act on its decisions
    using ActionCallback = std::function<void(const String&)>;

    AIBehavior(
        std::shared_ptr<LLMClient> llmClient,
        const String& personality = "You are a helpful NPC in a game."
    )
        : m_agent(nullptr)
        , m_llmClient(llmClient)
        , m_personality(personality)
        , m_lastThinkTime(0.0f)
        , m_thinkInterval(2.0f)
        , m_enabled(true)
    {
        initializeAgent();
    }

    // Configure AI personality
    void setPersonality(const String& personality) {
        m_personality = personality;
        if (m_agent) {
            m_agent->setSystemPrompt(personality);
        }
    }

    const String& getPersonality() const {
        return m_personality;
    }

    // Set how often the AI thinks (in seconds)
    void setThinkInterval(f32 interval) {
        m_thinkInterval = interval;
    }

    f32 getThinkInterval() const {
        return m_thinkInterval;
    }

    // Enable/disable AI processing
    void setEnabled(bool enabled) {
        m_enabled = enabled;
    }

    bool isEnabled() const {
        return m_enabled;
    }

    // Set perception callback - called when AI needs to understand environment
    void setPerceptionCallback(PerceptionCallback callback) {
        m_perceptionCallback = callback;
    }

    // Set action callback - called when AI wants to perform an action
    void setActionCallback(ActionCallback callback) {
        m_actionCallback = callback;
    }

    // Manually trigger AI thinking
    String think(const String& stimulus = "") {
        if (!m_agent) {
            return "Error: Agent not initialized";
        }

        // Build perception context
        String context;
        if (m_perceptionCallback) {
            context = m_perceptionCallback();
        }

        // Combine context with stimulus
        String query;
        if (!context.empty() && !stimulus.empty()) {
            query = "Context: " + context + "\n\nEvent: " + stimulus + "\n\nWhat do you do?";
        } else if (!context.empty()) {
            query = "Context: " + context + "\n\nWhat do you think about your current situation?";
        } else if (!stimulus.empty()) {
            query = stimulus;
        } else {
            query = "What are you thinking about?";
        }

        // Get AI response
        AgentResult result = m_agent->execute(query);

        if (result.isSuccess()) {
            m_lastResponse = result.finalResponse;

            // Execute action callback if provided
            if (m_actionCallback) {
                m_actionCallback(result.finalResponse);
            }

            return result.finalResponse;
        } else {
            return "Error: " + result.error;
        }
    }

    // Send a message to the AI (for player interaction)
    String talk(const String& message) {
        if (!m_agent) {
            return "Error: Agent not initialized";
        }

        AgentResult result = m_agent->execute(message);

        if (result.isSuccess()) {
            m_lastResponse = result.finalResponse;
            return result.finalResponse;
        } else {
            return "Error: " + result.error;
        }
    }

    // Get the last response from the AI
    const String& getLastResponse() const {
        return m_lastResponse;
    }

    // Clear conversation history
    void clearMemory() {
        if (m_agent) {
            m_agent->clearHistory();
        }
    }

    // Get the agent for advanced usage
    Agent* getAgent() {
        return m_agent.get();
    }

    // Script component overrides
    void onStart() override {
        m_lastThinkTime = 0.0f;
    }

    void onUpdate(f32 deltaTime) override {
        if (!m_enabled || !m_agent) {
            return;
        }

        m_lastThinkTime += deltaTime;

        // Periodic autonomous thinking
        if (m_lastThinkTime >= m_thinkInterval) {
            m_lastThinkTime = 0.0f;

            // Autonomous thinking (only if callback is set)
            if (m_perceptionCallback) {
                think();
            }
        }
    }

    void onCollision(Entity* other) override {
        if (!m_enabled || !m_agent || !other) {
            return;
        }

        // React to collision
        String stimulus = "You collided with: " + other->getName();
        think(stimulus);
    }

private:
    void initializeAgent() {
        if (!m_llmClient) {
            return;
        }

        AgentConfig config("AIBehavior", m_personality);
        config.maxIterations = 5;
        config.verbose = false;

        m_agent = std::make_unique<Agent>(m_llmClient, config);
    }

    std::unique_ptr<Agent> m_agent;
    std::shared_ptr<LLMClient> m_llmClient;
    String m_personality;
    String m_lastResponse;
    f32 m_lastThinkTime;
    f32 m_thinkInterval;
    bool m_enabled;

    PerceptionCallback m_perceptionCallback;
    ActionCallback m_actionCallback;
};

} // namespace engine
} // namespace oracon

#endif // ORACON_ENGINE_AI_BEHAVIOR_H
