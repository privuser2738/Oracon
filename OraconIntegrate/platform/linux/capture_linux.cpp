#include "oracon/integrate/desktop/capture.h"
#include "oracon/core/logger.h"

#ifdef ORACON_PLATFORM_LINUX

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <cstring>
#include <chrono>

namespace oracon {
namespace integrate {
namespace desktop {

// Factory method
std::unique_ptr<DesktopCapture> DesktopCapture::create() {
    return std::make_unique<X11Capture>();
}

X11Capture::X11Capture()
    : m_display(nullptr)
    , m_window(0)
    , m_image(nullptr)
    , m_capturing(false)
    , m_width(0)
    , m_height(0)
{}

X11Capture::~X11Capture() {
    stop();

    if (m_image) {
        XDestroyImage(static_cast<XImage*>(m_image));
        m_image = nullptr;
    }

    if (m_display) {
        XCloseDisplay(static_cast<Display*>(m_display));
        m_display = nullptr;
    }
}

bool X11Capture::initialize(u32 display_id) {
    // Open X display
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        ORACON_LOG_ERROR("Failed to open X display");
        return false;
    }

    Display* display = static_cast<Display*>(m_display);

    // Get screen
    int screen = display_id < static_cast<u32>(ScreenCount(display))
                     ? display_id
                     : DefaultScreen(display);

    // Get root window
    m_window = reinterpret_cast<void*>(RootWindow(display, screen));

    // Get window attributes
    XWindowAttributes attrs;
    XGetWindowAttributes(display, reinterpret_cast<Window>(m_window), &attrs);

    m_width = attrs.width;
    m_height = attrs.height;

    ORACON_LOG_INFO("Initialized X11 capture: ", m_width, "x", m_height);

    return true;
}

bool X11Capture::start() {
    if (m_capturing) {
        return true;
    }

    if (!m_display || !m_window) {
        ORACON_LOG_ERROR("X11 capture not initialized");
        return false;
    }

    m_capturing = true;
    m_stats = CaptureStats{};

    ORACON_LOG_INFO("Started X11 capture");
    return true;
}

void X11Capture::stop() {
    if (!m_capturing) {
        return;
    }

    m_capturing = false;
    ORACON_LOG_INFO("Stopped X11 capture");
}

std::unique_ptr<Frame> X11Capture::captureFrame() {
    if (!m_capturing) {
        return nullptr;
    }

    Display* display = static_cast<Display*>(m_display);
    Window window = reinterpret_cast<Window>(m_window);

    // Capture the screen
    XImage* image = XGetImage(display, window, 0, 0, m_width, m_height,
                              AllPlanes, ZPixmap);

    if (!image) {
        m_stats.frames_dropped++;
        return nullptr;
    }

    // Create frame
    auto frame = std::make_unique<Frame>();
    frame->width = m_width;
    frame->height = m_height;
    frame->stride = image->bytes_per_line;

    // Determine pixel format based on image format
    if (image->bits_per_pixel == 32) {
        frame->format = PixelFormat::BGRA32;
    } else if (image->bits_per_pixel == 24) {
        frame->format = PixelFormat::BGR24;
    } else {
        ORACON_LOG_WARNING("Unsupported pixel format: ", image->bits_per_pixel, " bpp");
        XDestroyImage(image);
        m_stats.frames_dropped++;
        return nullptr;
    }

    // Copy image data
    size_t data_size = image->bytes_per_line * image->height;
    frame->data.resize(data_size);
    std::memcpy(frame->data.data(), image->data, data_size);

    // Timestamp
    auto now = std::chrono::system_clock::now();
    frame->timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    XDestroyImage(image);

    // Update stats
    m_stats.frames_captured++;
    m_stats.bytes_captured += data_size;

    return frame;
}

bool X11Capture::isCapturing() const {
    return m_capturing;
}

CaptureStats X11Capture::getStats() const {
    return m_stats;
}

u32 X11Capture::getDisplayCount() const {
    if (!m_display) {
        return 0;
    }

    Display* display = static_cast<Display*>(m_display);
    return ScreenCount(display);
}

String X11Capture::getDisplayName(u32 display_id) const {
    if (!m_display) {
        return "";
    }

    Display* display = static_cast<Display*>(m_display);

    if (display_id >= static_cast<u32>(ScreenCount(display))) {
        return "";
    }

    // Get screen info using XRandR
    Window root = RootWindow(display, display_id);
    XRRScreenResources* resources = XRRGetScreenResources(display, root);

    if (!resources) {
        return "Display " + std::to_string(display_id);
    }

    String name = "Display " + std::to_string(display_id);

    if (resources->noutput > 0) {
        XRROutputInfo* output = XRRGetOutputInfo(display, resources, resources->outputs[0]);
        if (output && output->name) {
            name = String(output->name);
        }
        if (output) {
            XRRFreeOutputInfo(output);
        }
    }

    XRRFreeScreenResources(resources);
    return name;
}

void X11Capture::getDisplayResolution(u32 display_id, u32& width, u32& height) const {
    if (!m_display) {
        width = height = 0;
        return;
    }

    Display* display = static_cast<Display*>(m_display);

    if (display_id >= static_cast<u32>(ScreenCount(display))) {
        width = height = 0;
        return;
    }

    width = DisplayWidth(display, display_id);
    height = DisplayHeight(display, display_id);
}

} // namespace desktop
} // namespace integrate
} // namespace oracon

#endif // ORACON_PLATFORM_LINUX
