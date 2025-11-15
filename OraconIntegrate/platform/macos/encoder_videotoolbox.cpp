#include "oracon/integrate/desktop/encoder.h"
#include "oracon/core/logger.h"

#ifdef ORACON_PLATFORM_MACOS

// macOS VideoToolbox encoder stub

namespace oracon {
namespace integrate {
namespace desktop {

bool VideoToolboxEncoder::isAvailable() {
    // VideoToolbox is always available on macOS 10.8+
    return true;
}

VideoToolboxEncoder::VideoToolboxEncoder()
    : m_session(nullptr)
    , m_encoding(false)
{}

VideoToolboxEncoder::~VideoToolboxEncoder() {
    stop();
}

bool VideoToolboxEncoder::initialize(const EncoderConfig& config) {
    ORACON_LOG_ERROR("VideoToolbox encoder not yet implemented");
    // TODO: Implement using VTCompressionSessionCreate
    return false;
}

bool VideoToolboxEncoder::start() { return false; }
void VideoToolboxEncoder::stop() {}
std::unique_ptr<EncodedPacket> VideoToolboxEncoder::encodeFrame(const Frame& frame) { return nullptr; }
bool VideoToolboxEncoder::encodeFrameAsync(const Frame& frame, PacketCallback callback) { return false; }
std::vector<std::unique_ptr<EncodedPacket>> VideoToolboxEncoder::flush() { return {}; }
bool VideoToolboxEncoder::isEncoding() const { return false; }
EncoderStats VideoToolboxEncoder::getStats() const { return m_stats; }

} // namespace desktop
} // namespace integrate
} // namespace oracon

#endif // ORACON_PLATFORM_MACOS
