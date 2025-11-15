# OraconAuto Architecture

## Overview

OraconAuto is an AI/LLM automation suite that provides a comprehensive framework for building intelligent agents, managing conversations, and automating complex workflows with Large Language Models.

## Core Components

### 1. LLM Client (`llm_client.h`)

Abstract interface for interacting with different LLM providers.

**Key Classes:**
- `LLMClient`: Abstract base class for LLM interactions
- `Message`: Represents a single message in a conversation (System, User, Assistant, Tool)
- `GenerationParams`: Configuration for LLM generation (temperature, max tokens, etc.)
- `LLMResponse`: Response from LLM with metadata (tokens, finish reason, etc.)
- `LLMClientFactory`: Factory for creating provider-specific clients

**Supported Providers:**
- OpenAI (GPT-3.5, GPT-4)
- Anthropic (Claude)
- Local models (llama.cpp integration)
- Mock (for testing)

**Features:**
- Synchronous and streaming completions
- Token usage tracking
- Error handling
- Provider abstraction

### 2. Prompt Management (`prompt.h`)

Tools for building, templating, and managing prompts.

**Key Classes:**
- `PromptTemplate`: Variable substitution in prompts
- `PromptBuilder`: Fluent API for building complex prompts
- `PromptLibrary`: Storage and retrieval of reusable prompts

**Features:**
- Variable interpolation (`{variable}`)
- Section-based prompt construction
- Code block support
- Built-in templates (code generation, code review, task planning)

### 3. Tool System (`tool.h`)

Function calling framework for agents to interact with external systems.

**Key Classes:**
- `Tool`: Definition of a callable tool with parameters
- `ToolParameter`: Parameter specification (type, description, required/optional)
- `ToolRegistry`: Management of available tools
- `tools::*`: Built-in tool implementations

**Built-in Tools:**
- File system (read, write)
- Code execution
- Web search
- More can be easily added

**Features:**
- JSON schema generation for function calling
- Parameter validation
- Error handling
- Extensible tool registration

### 4. Memory System (`memory.h`)

Context and conversation history management with various strategies.

**Memory Implementations:**
- `ConversationMemory`: Stores all messages (with optional limit)
- `SlidingWindowMemory`: Keeps only recent N messages
- `SummaryMemory`: Periodically summarizes old messages using LLM
- `TokenLimitMemory`: Keeps messages under a token budget

**Features:**
- Flexible memory strategies
- Automatic context window management
- Summary generation
- Token estimation

### 5. Agent System (`agent.h`)

Intelligent agents that can use tools and maintain conversations.

**Key Classes:**
- `Agent`: Main agent class with tool-calling capabilities
- `AgentConfig`: Agent configuration (system prompt, parameters)
- `AgentResult`: Execution result with metadata
- `CodeAgent`: Specialized agent for code-related tasks
- `ResearchAgent`: Specialized agent for information gathering

**Features:**
- Multi-turn conversations
- Tool calling and execution
- Streaming support
- Conversation history management
- Specialized agent types

### 6. Workflow System (`workflow.h`)

Task automation and orchestration framework.

**Key Classes:**
- `Task`: Single executable task with status tracking
- `Workflow`: Sequence of tasks with execution management
- `WorkflowBuilder`: Fluent API for workflow construction
- `TaskStatus`: Pending, Running, Completed, Failed, Skipped
- `TaskResult`: Task execution result with output/error

**Features:**
- Sequential task execution
- Status tracking
- Error handling and reporting
- Agent task integration
- Custom task functions

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────┐
│                     OraconAuto                          │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐            │
│  │ Workflow │◄─┤  Agent   │◄─┤ LLMClient│            │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘            │
│       │             │              │                   │
│       │   ┌─────────┴──────┐      │                   │
│       │   │                 │      │                   │
│       ▼   ▼                 ▼      ▼                   │
│  ┌────────┐  ┌────────┐  ┌────────┐                  │
│  │  Task  │  │  Tool  │  │ Memory │                  │
│  └────────┘  └────────┘  └────────┘                  │
│                    ▲                                   │
│                    │                                   │
│              ┌─────┴──────┐                           │
│              │   Prompt   │                           │
│              └────────────┘                           │
│                                                        │
└────────────────────────────────────────────────────────┘
```

## Usage Examples

### Basic LLM Interaction

```cpp
#include "oracon/auto/auto.h"

using namespace oracon::auto_ns;

// Create LLM client
auto client = LLMClientFactory::create(
    LLMClientFactory::Provider::OpenAI,
    "api-key",
    "gpt-4"
);

// Simple prompt
auto response = client->prompt("What is 2+2?");
std::cout << response.content << std::endl;
```

### Agent with Tools

```cpp
// Create agent
auto agent = std::make_shared<Agent>(client);
agent->setSystemPrompt("You are a helpful coding assistant");

// Register tools
agent->registerTool(tools::readFile());
agent->registerTool(tools::writeFile());

// Execute query
auto result = agent->execute("Read config.json and explain its structure");
std::cout << result.finalResponse << std::endl;
```

### Workflow Automation

```cpp
// Build workflow
auto workflow = WorkflowBuilder("Code Review Workflow")
    .agentStep("analyze", "Analyze code for bugs", codeAgent)
    .agentStep("optimize", "Suggest optimizations", codeAgent)
    .agentStep("document", "Add documentation", codeAgent)
    .build();

// Execute
workflow->execute();
std::cout << workflow->getStatusReport() << std::endl;
```

### Custom Memory Strategy

```cpp
// Use sliding window memory
auto memory = std::make_shared<SlidingWindowMemory>(5);
agent->setMemory(memory);

// Or use summary memory for long conversations
auto summaryMemory = std::make_shared<SummaryMemory>(client, 10);
agent->setMemory(summaryMemory);
```

## Design Principles

1. **Modularity**: Each component can be used independently
2. **Extensibility**: Easy to add new providers, tools, and agents
3. **Type Safety**: Strong typing with C++ templates
4. **Abstraction**: Provider-agnostic interfaces
5. **Flexibility**: Multiple strategies for memory, prompts, etc.
6. **Error Handling**: Comprehensive error reporting

## Implementation Notes

- This is a header-only library providing interfaces and base classes
- Concrete implementations (OpenAI client, Anthropic client, etc.) are in separate libraries
- Requires C++17 or later
- Dependencies: OraconCore for type definitions

## Future Extensions

- Vector database integration for semantic search
- Multi-agent collaboration
- Reinforcement learning from human feedback (RLHF)
- Cost tracking and optimization
- Rate limiting and retry logic
- Caching layer for responses
- Plugin system for custom extensions
