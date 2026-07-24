#include "render/Renderer.hpp"

#include "Config.hpp"
#include "circuits/CircuitGraph.hpp"
#include "core/World.hpp"
#include "math/Util.hpp"

#include <cmath>

Renderer::Renderer() = default;

void Renderer::draw(sf::RenderWindow &window, Mode mode, const World &world, const CircuitGraph &circuit) const
{
    drawGridlines(window);

    if (mode == Mode::Fields)
        drawWorld(window, world);
    else
        drawCircuit(window, circuit);
}

void Renderer::drawGridlines(sf::RenderWindow &window) const
{
    const sf::View view = window.getView();
    const sf::Vector2f viewCenter = view.getCenter();
    const sf::Vector2f viewSize = view.getSize();

    const float left = viewCenter.x - viewSize.x / 2.0f;
    const float right = viewCenter.x + viewSize.x / 2.0f;
    const float top = viewCenter.y - viewSize.y / 2.0f;
    const float bottom = viewCenter.y + viewSize.y / 2.0f;

    const float minorSpacingPixels = metersToPixels(GRID_MINOR_SPACING);
    const float majorSpacingPixels = metersToPixels(GRID_MAJOR_SPACING);

    const sf::Color &minorGridColor = GRID_MINOR_COLOR;
    const sf::Color &majorGridColor = GRID_MAJOR_COLOR;
    const sf::Color &axisColor = GRID_AXIS_COLOR;

    const float startXMinor = std::floor(left / minorSpacingPixels) * minorSpacingPixels;
    for (float x = startXMinor; x <= right; x += minorSpacingPixels)
    {
        const float xMeters = x / metersToPixels(1.0f);
        const bool isMajor = std::abs(std::fmod(xMeters, GRID_MAJOR_SPACING)) < 0.01f;
        if (!isMajor)
        {
            sf::VertexArray line(sf::PrimitiveType::Lines, 2);
            line[0] = {{x, top}, minorGridColor};
            line[1] = {{x, bottom}, minorGridColor};
            window.draw(line);
        }
    }

    const float startYMinor = std::floor(top / minorSpacingPixels) * minorSpacingPixels;
    for (float y = startYMinor; y <= bottom; y += minorSpacingPixels)
    {
        const float yMeters = y / metersToPixels(1.0f);
        const bool isMajor = std::abs(std::fmod(yMeters, GRID_MAJOR_SPACING)) < 0.01f;
        if (!isMajor)
        {
            sf::VertexArray line(sf::PrimitiveType::Lines, 2);
            line[0] = {{left, y}, minorGridColor};
            line[1] = {{right, y}, minorGridColor};
            window.draw(line);
        }
    }

    const float startXMajor = std::floor(left / majorSpacingPixels) * majorSpacingPixels;
    for (float x = startXMajor; x <= right; x += majorSpacingPixels)
    {
        const bool isAxis = std::abs(x) < 0.1f;
        const sf::Color color = isAxis ? axisColor : majorGridColor;

        sf::VertexArray line(sf::PrimitiveType::Lines, 2);
        line[0] = {{x, top}, color};
        line[1] = {{x, bottom}, color};
        window.draw(line);
    }

    const float startYMajor = std::floor(top / majorSpacingPixels) * majorSpacingPixels;
    for (float y = startYMajor; y <= bottom; y += majorSpacingPixels)
    {
        const bool isAxis = std::abs(y) < 0.1f;
        const sf::Color color = isAxis ? axisColor : majorGridColor;

        sf::VertexArray line(sf::PrimitiveType::Lines, 2);
        line[0] = {{left, y}, color};
        line[1] = {{right, y}, color};
        window.draw(line);
    }
}

void Renderer::drawWorld(sf::RenderWindow &window, const World &world) const
{
    // TODO(Phase 2/3/4): draw charges, field vectors/lines, equipotentials, Gaussian
    // surfaces, wires, particles + trajectory traces, loops.
    (void)window;
    (void)world;
}

void Renderer::drawCircuit(sf::RenderWindow &window, const CircuitGraph &circuit) const
{
    // TODO(Phase 5): draw wires/components with live V/I/R/Q labels + current-flow animation.
    (void)window;
    (void)circuit;
}
