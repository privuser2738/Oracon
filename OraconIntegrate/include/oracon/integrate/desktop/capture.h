#ifndef ORACON_INTEGRATE_DESKTOP_CAPTURE_H
#define ORACON_INTEGRATE_DESKTOP_CAPTURE_H

#include "oracon/core/types.h"
#include <vector>
#include <memory>

namespace oracon {
namespace integrate {
namespace desktop {

using core::u8;
using core::u16;
using core::u32;
using core::u64;
using core::String;

// Pixel format
enum class PixelFormat {
    RGB24,      // 24-bit RGB
    RGBA32,     // 32-bit RGBA
    BGR24,      // 24-bit BGR
    BGRA32,     // 32-bit BGRA
    NV12,       // YUV 4:2:0
    I420        // YUV 4:2:0 planar
};

// Frame data
struct Frame {
    std::vector<u8> data;
    u32 width;
    u32 height;
    PixelFormat format;
    u32 stride;       // Bytes per row
    u64 timestamp_ms; // Milliseconds since epoch

    size_t getSize() const {
        return data.size();
    }

    bool isValid() const {
        return width > 0 && height > 0 && !data.empty();
    }
};

// Capture statistics
struct CaptureStats {
    u64 frames_captured = 0;
    u64 frames_dropped = 0;
    double fps = 0.0;
    u64 bytes_captured = 0;
};

// Desktop capture interface
class DesktopCapture {
public:
    virtual ~DesktopCapture() = default;

    // Initialize capture
    virtual bool initialize(u32 display_id = 0) = 0;

    // Start capturing
    virtual bool start() = 0;

    // Stop capturing
    virtual void stop() = 0;

    // Capture a single frame
    virtual std::unique_ptr<Frame> captureFrame() = 0;

    // Status
    virtual bool isCapturing() const = 0;
    virtual CaptureStats getStats() const = 0;

    // Display info
    virtual u32 getDisplayCount() const = 0;
    virtual String getDisplayName(u32 display_id) const = 0;
    virtual void getDisplayResolution(u32 display_id, u32& width, u32& height) const = 0;

    // Factory method
    static std::unique_ptr<DesktopCapture> create();
};

// Platform-specific implementations
#ifdef ORACON_PLATFORM_LINUX
class X11Capture : public DesktopCapture {
public:
    X11Capture();
    ~X11Capture() override;

    bool initialize(u32 display_id = 0) override;
    bool start() override;
    void stop() override;
    std::unique_ptr<Frame> captureFrame() override;
    bool isCapturing() const override;
    CaptureStats getStats() const override;
    u32 getDisplayCount() const override;
    String getDisplayName(u32 display_id) const override;
    void getDisplayResolution(u32 display_id, u32& width, u32& height) const override;

private:
    void* m_display;      // Display*
    void* m_window;       // Window
    void* m_image;        // XImage*
    bool m_capturing;
    u32 m_width;
    u32 m_height;
    CaptureStats m_stats;
};
#endif

#ifdef ORACON_PLATFORM_WINDOWS
class DirectXCapture : public DesktopCapture {
public:
    DirectXCapture();
    ~DirectXCapture() override;

    bool initialize(u32 display_id = 0) override;
    bool start() override;
    void stop() override;
    std::unique_ptr<Frame> captureFrame() override;
    bool isCapturing() const override;
    CaptureStats getStats() const override;
    u32 getDisplayCount() const override;
    String getDisplayName(u32 display_id) const override;
    void getDisplayResolution(u32 display_id, u32& width, u32& height) const override;

private:
    void* m_d3d_device;
    void* m_output_duplication;
    bool m_capturing;
    u32 m_width;
    u32 m_height;
    CaptureStats m_stats;
};
#endif

#ifdef ORACON_PLATFORM_MACOS
class CoreGraphicsCapture : public DesktopCapture {
public:
    CoreGraphicsCapture();
    ~CoreGraphicsCapture() override;

    bool initialize(u32 display_id = 0) override;
    bool start() override;
    void stop() override;
    std::unique_ptr<Frame> captureFrame() override;
    bool isCapturing() const override;
    CaptureStats getStats() const override;
    u32 getDisplayCount() const override;
    String getDisplayName(u32 display_id) const override;
    void getDisplayResolution(u32 display_id, u32& width, u32& height) const override;

private:
    u32 m_display_id;
    bool m_capturing;
    u32 m_width;
    u32 m_height;
    CaptureStats m_stats;
};
#endif

} // namespace desktop
} // namespace integrate
} // namespace oracon

#endif // ORACON_INTEGRATE_DESKTOP_CAPTURE_H
