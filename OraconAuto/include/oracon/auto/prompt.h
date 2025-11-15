#ifndef ORACON_AUTO_PROMPT_H
#define ORACON_AUTO_PROMPT_H

#include "oracon/core/types.h"
#include <map>
#include <vector>

namespace oracon {
namespace auto_ns {

using core::String;

// Prompt template with variable substitution
class PromptTemplate {
public:
    explicit PromptTemplate(const String& templateStr)
        : m_template(templateStr) {}

    // Set a variable value
    PromptTemplate& set(const String& key, const String& value) {
        m_variables[key] = value;
        return *this;
    }

    // Render the template with variable substitution
    String render() const {
        String result = m_template;
        for (const auto& [key, value] : m_variables) {
            String placeholder = "{" + key + "}";
            size_t pos = 0;
            while ((pos = result.find(placeholder, pos)) != String::npos) {
                result.replace(pos, placeholder.length(), value);
                pos += value.length();
            }
        }
        return result;
    }

    // Clear all variables
    void clear() {
        m_variables.clear();
    }

    // Static factory methods for common templates
    static PromptTemplate codeGeneration() {
        return PromptTemplate(
            "You are a code generation assistant.\n\n"
            "Task: {task}\n"
            "Language: {language}\n"
            "Requirements:\n{requirements}\n\n"
            "Generate clean, well-documented code."
        );
    }

    static PromptTemplate codeReview() {
        return PromptTemplate(
            "You are a code review assistant.\n\n"
            "Code to review:\n```{language}\n{code}\n```\n\n"
            "Provide feedback on:\n"
            "- Code quality\n"
            "- Potential bugs\n"
            "- Performance issues\n"
            "- Best practices"
        );
    }

    static PromptTemplate taskPlanning() {
        return PromptTemplate(
            "You are a task planning assistant.\n\n"
            "Goal: {goal}\n"
            "Context: {context}\n\n"
            "Break down this goal into concrete, actionable steps.\n"
            "Format as a numbered list."
        );
    }

    static PromptTemplate questionAnswering() {
        return PromptTemplate(
            "You are a helpful assistant.\n\n"
            "Context: {context}\n\n"
            "Question: {question}\n\n"
            "Answer the question based on the provided context."
        );
    }

private:
    String m_template;
    std::map<String, String> m_variables;
};

// Prompt builder for constructing complex prompts
class PromptBuilder {
public:
    PromptBuilder& addSection(const String& title, const String& content) {
        if (!title.empty()) {
            m_sections.push_back("## " + title);
        }
        if (!content.empty()) {
            m_sections.push_back(content);
        }
        m_sections.push_back("");  // Empty line
        return *this;
    }

    PromptBuilder& addList(const String& title, const std::vector<String>& items) {
        if (!title.empty()) {
            m_sections.push_back("## " + title);
        }
        for (size_t i = 0; i < items.size(); i++) {
            m_sections.push_back(String(std::to_string(i + 1)) + ". " + items[i]);
        }
        m_sections.push_back("");
        return *this;
    }

    PromptBuilder& addCodeBlock(const String& code, const String& language = "") {
        m_sections.push_back("```" + language);
        m_sections.push_back(code);
        m_sections.push_back("```");
        m_sections.push_back("");
        return *this;
    }

    PromptBuilder& addExample(const String& input, const String& output) {
        m_sections.push_back("Input: " + input);
        m_sections.push_back("Output: " + output);
        m_sections.push_back("");
        return *this;
    }

    String build() const {
        String result;
        for (const auto& section : m_sections) {
            result += section + "\n";
        }
        return result;
    }

    void clear() {
        m_sections.clear();
    }

private:
    std::vector<String> m_sections;
};

// Prompt library for storing and retrieving prompts
class PromptLibrary {
public:
    void store(const String& name, const String& prompt) {
        m_prompts[name] = prompt;
    }

    String get(const String& name) const {
        auto it = m_prompts.find(name);
        return it != m_prompts.end() ? it->second : "";
    }

    bool has(const String& name) const {
        return m_prompts.find(name) != m_prompts.end();
    }

    void remove(const String& name) {
        m_prompts.erase(name);
    }

    std::vector<String> list() const {
        std::vector<String> names;
        for (const auto& [name, _] : m_prompts) {
            names.push_back(name);
        }
        return names;
    }

private:
    std::map<String, String> m_prompts;
};

} // namespace auto_ns
} // namespace oracon

#endif // ORACON_AUTO_PROMPT_H
