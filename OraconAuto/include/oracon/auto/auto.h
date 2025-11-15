#ifndef ORACON_AUTO_AUTO_H
#define ORACON_AUTO_AUTO_H

// OraconAuto - AI/LLM Automation Suite
// Main include file

#include "oracon/auto/llm_client.h"
#include "oracon/auto/prompt.h"
#include "oracon/auto/tool.h"
#include "oracon/auto/memory.h"
#include "oracon/auto/agent.h"
#include "oracon/auto/workflow.h"

namespace oracon {
namespace auto_ns {

// Version information
inline const char* getVersion() {
    return "0.1.0";
}

} // namespace auto_ns
} // namespace oracon

#endif // ORACON_AUTO_AUTO_H
