#include "oracon/integrate/config.h"
#include "oracon/core/logger.h"
#include <iostream>

using namespace oracon;

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    core::Logger::getInstance().setLevel(core::LogLevel::Info);
    ORACON_LOG_INFO("OraconIntegrate Service starting...");

    // TODO: Implement service mode with proper platform-specific service handling

    ORACON_LOG_ERROR("Service mode not yet implemented");
    return 1;
}
