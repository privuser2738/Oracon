#include "oracon/auto/llm_client.h"
#include "http_client.h"
#include "oracon/core/logger.h"
#include <sstream>
#include <memory>

namespace oracon {
namespace auto_ns {

// Forward declaration
std::unique_ptr<HttpClient> createHttpClient();

// Simple JSON builder (minimal, just for API requests)
class JsonBuilder {
public:
    JsonBuilder& startObject() {
        m_json += "{";
        m_needsComma = false;
        return *this;
    }

    JsonBuilder& endObject() {
        m_json += "}";
        return *this;
    }

    JsonBuilder& startArray(const String& key) {
        addComma();
        m_json += "\"" + key + "\":[";
        m_needsComma = false;
        return *this;
    }

    JsonBuilder& endArray() {
        m_json += "]";
        m_needsComma = true;
        return *this;
    }

    JsonBuilder& addString(const String& key, const String& value) {
        addComma();
        m_json += "\"" + key + "\":\"" + escapeJson(value) + "\"";
        m_needsComma = true;
        return *this;
    }

    JsonBuilder& addNumber(const String& key, f32 value) {
        addComma();
        m_json += "\"" + key + "\":" + std::to_string(value);
        m_needsComma = true;
        return *this;
    }

    JsonBuilder& addInt(const String& key, u32 value) {
        addComma();
        m_json += "\"" + key + "\":" + std::to_string(value);
        m_needsComma = true;
        return *this;
    }

    JsonBuilder& addMessageObject(const String& role, const String& content) {
        addComma();
        m_json += "{\"role\":\"" + role + "\",\"content\":\"" + escapeJson(content) + "\"}";
        m_needsComma = true;
        return *this;
    }

    String build() const {
        return m_json;
    }

private:
    String m_json;
    bool m_needsComma = false;

    void addComma() {
        if (m_needsComma) {
            m_json += ",";
        }
    }

    String escapeJson(const String& str) const {
        String result;
        for (char c : str) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c; break;
            }
        }
        return result;
    }
};

// Simple JSON parser (minimal, just for API responses)
class JsonParser {
public:
    explicit JsonParser(const String& json) : m_json(json), m_pos(0) {}

    String getString(const String& key) {
        String searchKey = "\"" + key + "\":\"";
        size_t pos = m_json.find(searchKey, m_pos);
        if (pos == String::npos) return "";

        size_t start = pos + searchKey.length();
        size_t end = m_json.find("\"", start);

        while (end != String::npos && end > 0 && m_json[end - 1] == '\\') {
            end = m_json.find("\"", end + 1);
        }

        if (end == String::npos) return "";
        return m_json.substr(start, end - start);
    }

    u32 getInt(const String& key) {
        String searchKey = "\"" + key + "\":";
        size_t pos = m_json.find(searchKey, m_pos);
        if (pos == String::npos) return 0;

        size_t start = pos + searchKey.length();
        size_t end = start;

        while (end < m_json.length() && (isdigit(m_json[end]) || m_json[end] == '.')) {
            end++;
        }

        String numStr = m_json.substr(start, end - start);
        return numStr.empty() ? 0 : std::stoul(numStr);
    }

    String getNestedString(const String& path1, const String& path2) {
        // Find first level
        String searchKey1 = "\"" + path1 + "\":";
        size_t pos1 = m_json.find(searchKey1);
        if (pos1 == String::npos) return "";

        // Find second level after first
        String searchKey2 = "\"" + path2 + "\":\"";
        size_t pos2 = m_json.find(searchKey2, pos1);
        if (pos2 == String::npos) return "";

        size_t start = pos2 + searchKey2.length();
        size_t end = m_json.find("\"", start);

        while (end != String::npos && end > 0 && m_json[end - 1] == '\\') {
            end = m_json.find("\"", end + 1);
        }

        if (end == String::npos) return "";
        return m_json.substr(start, end - start);
    }

private:
    String m_json;
    size_t m_pos;
};

// Anthropic Claude client implementation
class AnthropicClient : public LLMClient {
public:
    AnthropicClient(const String& apiKey, const String& model, const String& baseUrl)
        : m_apiKey(apiKey)
        , m_model(model.empty() ? "claude-3-5-sonnet-20241022" : model)
        , m_baseUrl(baseUrl.empty() ? "https://api.anthropic.com/v1" : baseUrl)
        , m_httpClient(createHttpClient())
    {
        if (m_apiKey.empty()) {
            ORACON_LOG_ERROR("Anthropic API key is empty!");
        }
    }

    LLMResponse complete(
        const std::vector<Message>& messages,
        const GenerationParams& params
    ) override {
        LLMResponse response;
        response.model = m_model;

        // Build request JSON
        JsonBuilder json;
        json.startObject()
            .addString("model", m_model)
            .addInt("max_tokens", params.maxTokens)
            .addNumber("temperature", params.temperature)
            .addNumber("top_p", params.topP)
            .startArray("messages");

        // Add messages (skip system messages for now, handle separately)
        String systemPrompt;
        for (const auto& msg : messages) {
            if (msg.role == Role::System) {
                systemPrompt = msg.content;
            } else {
                String role = roleToString(msg.role);
                json.addMessageObject(role, msg.content);
            }
        }

        json.endArray();

        // Add system prompt if present
        if (!systemPrompt.empty()) {
            json.addString("system", systemPrompt);
        }

        json.endObject();

        String requestBody = json.build();

        // Set headers
        std::map<String, String> headers;
        headers["Content-Type"] = "application/json";
        headers["x-api-key"] = m_apiKey;
        headers["anthropic-version"] = "2023-06-01";

        // Make HTTP request
        HttpResponse httpResp = m_httpClient->post(
            m_baseUrl + "/messages",
            requestBody,
            headers
        );

        if (!httpResp.isSuccess()) {
            response.error = "HTTP Error " + std::to_string(httpResp.statusCode) + ": " + httpResp.error;
            response.success = false;
            ORACON_LOG_ERROR(response.error);
            return response;
        }

        // Parse response
        JsonParser parser(httpResp.body);

        response.content = parser.getNestedString("content", "text");
        response.promptTokens = parser.getInt("input_tokens");
        response.completionTokens = parser.getInt("output_tokens");
        response.totalTokens = response.promptTokens + response.completionTokens;
        response.finishReason = parser.getString("stop_reason");
        response.success = !response.content.empty();

        if (!response.success) {
            response.error = "Failed to parse response or empty content";
            ORACON_LOG_ERROR("Response body: " + httpResp.body.substr(0, 500));
        }

        return response;
    }

    LLMResponse streamComplete(
        const std::vector<Message>& messages,
        StreamCallback callback,
        const GenerationParams& params
    ) override {
        LLMResponse response;
        response.model = m_model;

        // Build request JSON (same as complete but with stream:true)
        JsonBuilder json;
        json.startObject()
            .addString("model", m_model)
            .addInt("max_tokens", params.maxTokens)
            .addNumber("temperature", params.temperature)
            .addNumber("top_p", params.topP)
            .startArray("messages");

        String systemPrompt;
        for (const auto& msg : messages) {
            if (msg.role == Role::System) {
                systemPrompt = msg.content;
            } else {
                String role = roleToString(msg.role);
                json.addMessageObject(role, msg.content);
            }
        }

        json.endArray();

        if (!systemPrompt.empty()) {
            json.addString("system", systemPrompt);
        }

        json.addString("stream", "true");  // Enable streaming
        json.endObject();

        String requestBody = json.build();

        // Set headers
        std::map<String, String> headers;
        headers["Content-Type"] = "application/json";
        headers["x-api-key"] = m_apiKey;
        headers["anthropic-version"] = "2023-06-01";

        // Accumulate content for final response
        String fullContent;

        // Stream callback wrapper to parse SSE and extract text
        auto sseCallback = [&fullContent, &callback](const String& chunk) {
            // Parse Server-Sent Events format
            // Look for data: lines with JSON
            size_t pos = 0;
            while ((pos = chunk.find("data: ", pos)) != String::npos) {
                size_t start = pos + 6;  // "data: " length
                size_t end = chunk.find("\n", start);
                if (end == String::npos) end = chunk.length();

                String jsonData = chunk.substr(start, end - start);

                // Parse JSON to extract text delta
                JsonParser parser(jsonData);
                String text = parser.getNestedString("delta", "text");

                if (!text.empty()) {
                    fullContent += text;
                    callback(text);
                }

                pos = end;
            }
        };

        // Make streaming HTTP request
        HttpResponse httpResp = m_httpClient->postStreaming(
            m_baseUrl + "/messages",
            requestBody,
            headers,
            sseCallback
        );

        response.content = fullContent;
        response.success = httpResp.isSuccess();
        response.error = httpResp.error;

        return response;
    }

    String getModelName() const override {
        return m_model;
    }

    bool isAvailable() const override {
        return !m_apiKey.empty() && m_httpClient != nullptr;
    }

private:
    String m_apiKey;
    String m_model;
    String m_baseUrl;
    std::unique_ptr<HttpClient> m_httpClient;

    String roleToString(Role role) const {
        switch (role) {
            case Role::User: return "user";
            case Role::Assistant: return "assistant";
            case Role::System: return "system";
            case Role::Tool: return "tool";
            default: return "user";
        }
    }
};

// Update factory to create Anthropic client
std::unique_ptr<LLMClient> createAnthropicClient(
    const String& apiKey,
    const String& model,
    const String& baseUrl
) {
    return std::make_unique<AnthropicClient>(apiKey, model, baseUrl);
}

} // namespace auto_ns
} // namespace oracon
