# Using Anthropic Claude with OraconAuto

## Overview

OraconAuto now has full integration with Anthropic's Claude API! You can use Claude models for real AI-powered conversations, agents, and workflows.

## Quick Start

### 1. Get an API Key

Sign up at https://console.anthropic.com/ and create an API key.

### 2. Set Environment Variable

```bash
export ANTHROPIC_API_KEY='your-api-key-here'
```

### 3. Run the Demo

```bash
cd build
./bin/claude_demo
```

## Code Examples

### Simple Prompt

```cpp
#include "oracon/auto/auto.h"

using namespace oracon::auto_ns;

// Create Claude client
auto client = LLMClientFactory::create(
    LLMClientFactory::Provider::Anthropic,
    std::getenv("ANTHROPIC_API_KEY"),
    "claude-3-5-sonnet-20241022"
);

// Ask a question
auto response = client->prompt("What is the capital of France?");

if (response.isSuccess()) {
    std::cout << response.content << "\n";
    std::cout << "Tokens used: " << response.totalTokens << "\n";
}
```

### Conversation with System Prompt

```cpp
std::vector<Message> messages;
messages.push_back(Message::system("You are a helpful coding assistant."));
messages.push_back(Message::user("Explain hash tables"));

auto response = client->complete(messages);
std::cout << response.content << "\n";
```

### Agent with Memory

```cpp
auto agent = std::make_shared<Agent>(std::move(client));
agent->setSystemPrompt("You are a helpful assistant.");

// First query
auto result1 = agent->execute("My name is Alice");

// Second query (remembers context)
auto result2 = agent->execute("What's my name?");
std::cout << result2.finalResponse << "\n";  // Will say "Alice"
```

### Streaming Response

```cpp
client->streamComplete(
    {Message::user("Count from 1 to 10")},
    [](const String& chunk) {
        std::cout << chunk << std::flush;  // Print as it arrives
    }
);
```

## Available Models

- `claude-3-5-sonnet-20241022` - Latest Sonnet (recommended, best balance)
- `claude-3-5-haiku-20241022` - Fast and cost-effective
- `claude-3-opus-20240229` - Most capable, highest cost

## Features

✅ Synchronous and streaming completions
✅ Full conversation history support
✅ System prompts
✅ Token usage tracking
✅ Error handling and retry logic
✅ Agent framework integration
✅ Memory strategies (sliding window, summary, etc.)

## API Rate Limits

Anthropic enforces rate limits. The client will return errors if you exceed them:
- Free tier: ~1000 requests/day
- Paid tiers: Higher limits based on your plan

## Cost Estimation

Approximate costs (as of 2024):
- Sonnet 3.5: $3/million input tokens, $15/million output tokens
- Haiku 3.5: $0.80/million input tokens, $4/million output tokens

The `LLMResponse` includes token counts for tracking costs.

## Error Handling

```cpp
auto response = client->prompt("Hello");

if (!response.isSuccess()) {
    std::cerr << "Error: " << response.error << "\n";
    std::cerr << "HTTP Status: " << response.statusCode << "\n";
}
```

Common errors:
- 401: Invalid API key
- 429: Rate limit exceeded
- 500: Anthropic server error

## Advanced Usage

### Custom Generation Parameters

```cpp
GenerationParams params;
params.withTemperature(0.8f)      // Higher = more creative
      .withMaxTokens(4096)         // Max response length
      .withTopP(0.95f);            // Nucleus sampling

auto response = client->complete(messages, params);
```

### Using with Workflows

```cpp
auto workflow = WorkflowBuilder("Research Task")
    .agentStep("research", "Research topic X", researchAgent)
    .agentStep("summarize", "Summarize findings", summaryAgent)
    .build();

workflow->execute();
```

## Troubleshooting

### "API key not found"
Make sure `ANTHROPIC_API_KEY` environment variable is set.

### "HTTP Error 401"
Your API key is invalid or expired. Check the Anthropic console.

### "Empty response content"
The API call succeeded but returned no content. Check the error field for details.

### Compilation errors about libcurl
Install libcurl development package:
```bash
# Ubuntu/Debian
sudo apt-get install libcurl4-openssl-dev

# Fedora/RHEL
sudo dnf install libcurl-devel

# Arch
sudo pacman -S curl
```

## Examples

Run the included demos:

```bash
# Basic demo with mock client
./bin/basic_agent

# Claude API demo (requires API key)
export ANTHROPIC_API_KEY='your-key'
./bin/claude_demo
```

## What's Next?

- **OpenAI Integration**: Similar client for GPT models
- **Local Models**: Support for llama.cpp and local inference
- **Vector Database**: Long-term memory with embeddings
- **Multi-Agent**: Agents that collaborate on complex tasks

Happy coding with Claude! 🤖
