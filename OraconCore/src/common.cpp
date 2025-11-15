#include "oracon/core/common.h"
#include <sstream>

namespace oracon {
namespace core {

String getVersionString() {
    std::ostringstream oss;
    oss << ORACON_VERSION_MAJOR << "."
        << ORACON_VERSION_MINOR << "."
        << ORACON_VERSION_PATCH;
    return oss.str();
}

} // namespace core
} // namespace oracon
