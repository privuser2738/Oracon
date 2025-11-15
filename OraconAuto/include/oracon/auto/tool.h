#ifndef ORACON_AUTO_TOOL_H
#define ORACON_AUTO_TOOL_H

#include "oracon/core/types.h"
#include <functional>
#include <map>
#include <vector>
#include <any>

namespace oracon {
namespace auto_ns {

using core::String;

// Tool parameter definition
struct ToolParameter {
    String name;
    String type;  // "string", "number", "boolean", "object", "array"
    String description;
    bool required;
    std::any defaultValue;

    ToolParameter(const String& name, const String& type, const String& desc, bool req = true)
        : name(name), type(type), description(desc), required(req) {}
};

// Tool definition
class Tool {
public:
    using ToolFunction = std::function<String(const std::map<String, std::any>&)>;

    // Default constructor for STL containers
    Tool() : m_name(""), m_description(""), m_function(nullptr) {}

    Tool(const String& name, const String& description, ToolFunction func)
        : m_name(name)
        , m_description(description)
        , m_function(func)
    {}

    // Add parameter
    Tool& addParameter(const ToolParameter& param) {
        m_parameters.push_back(param);
        return *this;
    }

    Tool& addParameter(const String& name, const String& type, const String& description, bool required = true) {
        m_parameters.push_back(ToolParameter(name, type, description, required));
        return *this;
    }

    // Execute the tool
    String execute(const std::map<String, std::any>& args) const {
        if (!m_function) {
            return "Error: Tool function is null";
        }

        // Validate required parameters
        for (const auto& param : m_parameters) {
            if (param.required && args.find(param.name) == args.end()) {
                return "Error: Missing required parameter: " + param.name;
            }
        }

        try {
            return m_function(args);
        } catch (const std::exception& e) {
            return String("Error executing tool: ") + e.what();
        }
    }

    // Getters
    const String& getName() const { return m_name; }
    const String& getDescription() const { return m_description; }
    const std::vector<ToolParameter>& getParameters() const { return m_parameters; }

    // Get JSON schema for function calling
    String toJSONSchema() const {
        String schema = "{\n";
        schema += "  \"name\": \"" + m_name + "\",\n";
        schema += "  \"description\": \"" + m_description + "\",\n";
        schema += "  \"parameters\": {\n";
        schema += "    \"type\": \"object\",\n";
        schema += "    \"properties\": {\n";

        for (size_t i = 0; i < m_parameters.size(); i++) {
            const auto& param = m_parameters[i];
            schema += "      \"" + param.name + "\": {\n";
            schema += "        \"type\": \"" + param.type + "\",\n";
            schema += "        \"description\": \"" + param.description + "\"\n";
            schema += "      }";
            if (i < m_parameters.size() - 1) schema += ",";
            schema += "\n";
        }

        schema += "    },\n";
        schema += "    \"required\": [";

        bool first = true;
        for (const auto& param : m_parameters) {
            if (param.required) {
                if (!first) schema += ", ";
                schema += "\"" + param.name + "\"";
                first = false;
            }
        }

        schema += "]\n";
        schema += "  }\n";
        schema += "}";
        return schema;
    }

private:
    String m_name;
    String m_description;
    ToolFunction m_function;
    std::vector<ToolParameter> m_parameters;
};

// Tool registry for managing available tools
class ToolRegistry {
public:
    void registerTool(const Tool& tool) {
        m_tools[tool.getName()] = tool;
    }

    bool hasTool(const String& name) const {
        return m_tools.find(name) != m_tools.end();
    }

    const Tool* getTool(const String& name) const {
        auto it = m_tools.find(name);
        return it != m_tools.end() ? &it->second : nullptr;
    }

    String executeTool(const String& name, const std::map<String, std::any>& args) const {
        const Tool* tool = getTool(name);
        if (!tool) {
            return "Error: Tool not found: " + name;
        }
        return tool->execute(args);
    }

    std::vector<String> listTools() const {
        std::vector<String> names;
        for (const auto& [name, _] : m_tools) {
            names.push_back(name);
        }
        return names;
    }

    std::vector<const Tool*> getAllTools() const {
        std::vector<const Tool*> tools;
        for (const auto& [_, tool] : m_tools) {
            tools.push_back(&tool);
        }
        return tools;
    }

private:
    std::map<String, Tool> m_tools;
};

// Built-in tool implementations
namespace tools {

// Real implementations (defined in tools_impl.cpp)
Tool readFileImpl();
Tool writeFileImpl();
Tool listFilesImpl();
Tool fileExistsImpl();
Tool executeBashImpl();
Tool executePythonImpl();
Tool getCurrentTimeImpl();
Tool calculateImpl();

// Convenience aliases
inline Tool readFile() { return readFileImpl(); }
inline Tool writeFile() { return writeFileImpl(); }
inline Tool listFiles() { return listFilesImpl(); }
inline Tool fileExists() { return fileExistsImpl(); }
inline Tool executeBash() { return executeBashImpl(); }
inline Tool executePython() { return executePythonImpl(); }
inline Tool getCurrentTime() { return getCurrentTimeImpl(); }
inline Tool calculate() { return calculateImpl(); }

} // namespace tools

} // namespace auto_ns
} // namespace oracon

#endif // ORACON_AUTO_TOOL_H
