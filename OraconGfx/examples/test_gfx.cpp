#include "oracon/gfx/gfx.h"
#include <iostream>
#include <fstream>

using namespace oracon::gfx;
using namespace oracon::math;

// Save canvas as PPM (simple image format)
void savePPM(const Canvas& canvas, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    file << "P6\n" << canvas.getWidth() << " " << canvas.getHeight() << "\n255\n";
    
    for (u32 y = 0; y < canvas.getHeight(); y++) {
        for (u32 x = 0; x < canvas.getWidth(); x++) {
            Color c = canvas.getPixel(x, y);
            file.put(c.r);
            file.put(c.g);
            file.put(c.b);
        }
    }
}

int main() {
    std::cout << "=== OraconGfx 2D Graphics Library Test ===" << std::endl;
    std::cout << std::endl;

    // Create a canvas
    Canvas canvas(800, 600);
    Renderer renderer(&canvas);

    std::cout << "Drawing to " << canvas.getWidth() << "x" << canvas.getHeight() << " canvas" << std::endl;

    // Clear to dark gray
    renderer.clear(Color::darkGray());

    // Draw some shapes
    std::cout << "Drawing primitives..." << std::endl;

    // Red rectangle
    renderer.drawRect(50, 50, 200, 150, Color::red(), true);

    // Blue circle
    renderer.drawCircle(Vec2f(500, 150), 75, Color::blue(), true);

    // Green triangle
    Triangle tri(Vec2f(400, 400), Vec2f(550, 500), Vec2f(300, 500), Color::green());
    renderer.drawTriangle(tri, true);

    // Yellow lines
    renderer.drawLine(Vec2f(100, 400), Vec2f(200, 500), Color::yellow(), 2.0f);
    renderer.drawLine(Vec2f(200, 400), Vec2f(100, 500), Color::yellow(), 2.0f);

    // Cyan ellipse
    Ellipse ellipse(Vec2f(650, 450), 100, 60, Color::cyan());
    renderer.drawEllipse(ellipse, false);

    // Magenta polygon
    Polygon poly;
    poly.color = Color::magenta();
    poly.filled = false;
    for (int i = 0; i < 6; i++) {
        f32 angle = i * (360.0f / 6.0f);
        f32 rad = radians(angle);
        poly.addVertex(
            150 + std::cos(rad) * 50,
            250 + std::sin(rad) * 50
        );
    }
    renderer.drawPolygon(poly);

    // Test color blending
    std::cout << "Testing color blending..." << std::endl;
    renderer.setBlendMode(Renderer::BlendMode::Alpha);
    
    Color semiRed = Color::red().withAlpha(128);
    renderer.drawCircle(Vec2f(300, 100), 50, semiRed, true);
    
    Color semiBlue = Color::blue().withAlpha(128);
    renderer.drawCircle(Vec2f(350, 100), 50, semiBlue, true);

    // Test canvas operations
    std::cout << "Testing canvas operations..." << std::endl;
    Canvas smallCanvas(100, 100);
    Renderer smallRenderer(&smallCanvas);
    smallRenderer.clear(Color::orange());
    smallRenderer.drawCircle(Vec2f(50, 50), 40, Color::purple(), true);
    
    // Blit small canvas onto main canvas
    canvas.blit(smallCanvas, 600, 50);

    // Save result
    std::cout << "Saving to output.ppm..." << std::endl;
    savePPM(canvas, "output.ppm");

    std::cout << std::endl;
    std::cout << "Test completed successfully!" << std::endl;
    std::cout << "Output saved to output.ppm (view with image viewer)" << std::endl;

    return 0;
}
