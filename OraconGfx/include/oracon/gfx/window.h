#ifndef ORACON_GFX_WINDOW_H
#define ORACON_GFX_WINDOW_H

#include "oracon/gfx/canvas.h"
#include "oracon/core/types.h"
#include <SDL2/SDL.h>
#include <string>

namespace oracon {
namespace gfx {

using core::String;
using core::u32;
using core::i32;

// Simple SDL2-based window for displaying canvas
class Window {
public:
    Window(const String& title, u32 width, u32 height)
        : m_width(width)
        , m_height(height)
        , m_window(nullptr)
        , m_renderer(nullptr)
        , m_texture(nullptr)
        , m_running(false)
    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            m_error = String("SDL init failed: ") + SDL_GetError();
            return;
        }

        m_window = SDL_CreateWindow(
            title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            static_cast<int>(width),
            static_cast<int>(height),
            SDL_WINDOW_SHOWN
        );

        if (!m_window) {
            m_error = String("Window creation failed: ") + SDL_GetError();
            return;
        }

        m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!m_renderer) {
            m_error = String("Renderer creation failed: ") + SDL_GetError();
            return;
        }

        m_texture = SDL_CreateTexture(
            m_renderer,
            SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_STREAMING,
            static_cast<int>(width),
            static_cast<int>(height)
        );

        if (!m_texture) {
            m_error = String("Texture creation failed: ") + SDL_GetError();
            return;
        }

        m_running = true;
    }

    ~Window() {
        if (m_texture) SDL_DestroyTexture(m_texture);
        if (m_renderer) SDL_DestroyRenderer(m_renderer);
        if (m_window) SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    // Check if window is valid
    bool isValid() const {
        return m_window && m_renderer && m_texture;
    }

    String getError() const {
        return m_error;
    }

    // Get dimensions
    u32 getWidth() const { return m_width; }
    u32 getHeight() const { return m_height; }

    // Display a canvas
    void present(const Canvas& canvas) {
        if (!isValid()) return;

        // Convert canvas to RGBA
        auto pixels = canvas.toRGBA32();

        // Update texture
        SDL_UpdateTexture(m_texture, nullptr, pixels.data(), static_cast<int>(m_width * 4));

        // Render
        SDL_RenderClear(m_renderer);
        SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
        SDL_RenderPresent(m_renderer);
    }

    // Event handling
    bool pollEvent(SDL_Event& event) {
        return SDL_PollEvent(&event) != 0;
    }

    bool isRunning() const {
        return m_running;
    }

    void close() {
        m_running = false;
    }

    // Get mouse position
    void getMousePosition(i32& x, i32& y) const {
        SDL_GetMouseState(&x, &y);
    }

private:
    u32 m_width;
    u32 m_height;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_texture;
    bool m_running;
    String m_error;
};

} // namespace gfx
} // namespace oracon

#endif // ORACON_GFX_WINDOW_H
