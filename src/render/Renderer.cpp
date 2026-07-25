#include "render/Renderer.hpp"

#include "Config.hpp"
#include "circuits/CircuitGraph.hpp"
#include "core/World.hpp"
#include "electrostatics/EquipotentialTracer.hpp"
#include "electrostatics/FieldSampler.hpp"
#include "engine/FieldMath.hpp"
#include "math/Util.hpp"

#include <imgui.h>
#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace
{
// Shaft + two-line arrowhead from originMeters along directionMeters (both in world meters).
void drawArrow(sf::RenderWindow &window, const Vec2 &originMeters, const Vec2 &directionMeters, const sf::Color &color)
{
    const float length = directionMeters.length();
    if (length < 1e-6f)
        return;

    const sf::Vector2f start(metersToPixels(originMeters.x), metersToPixels(originMeters.y));
    const Vec2 tipMeters = originMeters + directionMeters;
    const sf::Vector2f tip(metersToPixels(tipMeters.x), metersToPixels(tipMeters.y));

    sf::VertexArray shaft(sf::PrimitiveType::Lines, 2);
    shaft[0] = {start, color};
    shaft[1] = {tip, color};
    window.draw(shaft);

    // Head size scales with (capped by) shaft length - a fixed size looks oversized on short,
    // weak-field shafts.
    const float headLength = std::min(0.08f, length * 0.4f);
    const float headWidth = std::min(0.05f, length * 0.25f);

    const Vec2 dirNorm = directionMeters / length;
    const Vec2 back = dirNorm * -metersToPixels(headLength);
    const Vec2 perp = dirNorm.perpendicular() * metersToPixels(headWidth);

    sf::VertexArray head(sf::PrimitiveType::Lines, 4);
    head[0] = {tip, color};
    head[1] = {sf::Vector2f(tip.x + back.x + perp.x, tip.y + back.y + perp.y), color};
    head[2] = {tip, color};
    head[3] = {sf::Vector2f(tip.x + back.x - perp.x, tip.y + back.y - perp.y), color};
    window.draw(head);
}
} // namespace

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
    // TODO(Phase 3/4): wires, particles + trajectory traces, loops.
    const auto &charges = world.charges();

    if (showEquipotentials)
        drawEquipotentials(window, charges);
    if (showFieldLines)
        drawFieldLines(window, charges);
    if (showFieldVectors)
        drawFieldVectors(window, charges);

    drawGaussianSurfaces(window, world.gaussianSurfaces(), charges);
    drawCharges(window, charges);
}

void Renderer::drawCircuit(sf::RenderWindow &window, const CircuitGraph &circuit) const
{
    // TODO(Phase 5): draw wires/components with live V/I/R/Q labels + current-flow animation.
    (void)window;
    (void)circuit;
}

void Renderer::drawCharges(sf::RenderWindow &window, const std::vector<PointCharge> &charges) const
{
    for (const auto &charge : charges)
    {
        const float radiusPixels = metersToPixels(POINT_CHARGE_RADIUS);
        sf::CircleShape shape(radiusPixels);
        shape.setOrigin(sf::Vector2f(radiusPixels, radiusPixels));
        shape.setPosition(sf::Vector2f(metersToPixels(charge.position.x), metersToPixels(charge.position.y)));
        shape.setFillColor(charge.charge >= 0.0f ? POSITIVE_CHARGE_COLOR : NEGATIVE_CHARGE_COLOR);
        window.draw(shape);
    }
}

void Renderer::drawFieldVectors(sf::RenderWindow &window, const std::vector<PointCharge> &charges) const
{
    if (charges.empty())
        return;

    const sf::View view = window.getView();
    const Vec2 minWorld = pixelsToMeters(Vec2(view.getCenter().x - view.getSize().x / 2.0f, view.getCenter().y - view.getSize().y / 2.0f));
    const Vec2 maxWorld = pixelsToMeters(Vec2(view.getCenter().x + view.getSize().x / 2.0f, view.getCenter().y + view.getSize().y / 2.0f));

    for (float y = minWorld.y; y <= maxWorld.y; y += FIELD_VECTOR_SPACING)
    {
        for (float x = minWorld.x; x <= maxWorld.x; x += FIELD_VECTOR_SPACING)
        {
            const Vec2 point(x, y);

            bool tooClose = false;
            for (const auto &charge : charges)
            {
                if ((point - charge.position).length() < FIELD_VECTOR_SPACING * 0.4f)
                {
                    tooClose = true;
                    break;
                }
            }
            if (tooClose)
                continue;

            const Vec2 field = FieldMath::coulombField(point, charges);
            const float magnitude = field.length();
            if (magnitude < FIELD_VECTOR_MIN_MAGNITUDE)
                continue;

            const float arrowLength = FIELD_VECTOR_MAX_LENGTH * (magnitude / (magnitude + FIELD_VECTOR_SATURATION));
            const Vec2 direction = (field / magnitude) * arrowLength;

            drawArrow(window, point - direction * 0.5f, direction, FIELD_VECTOR_COLOR);
        }
    }
}

void Renderer::drawFieldLines(sf::RenderWindow &window, const std::vector<PointCharge> &charges) const
{
    const FieldSampler sampler;
    constexpr float kSeedOffset = 0.2f; // meters from the charge center for the seed ring
    constexpr float kTwoPi = 6.28318530718f;

    for (const auto &charge : charges)
    {
        const bool followField = charge.charge >= 0.0f;
        for (int i = 0; i < FIELD_LINE_COUNT_PER_CHARGE; ++i)
        {
            const float theta = kTwoPi * static_cast<float>(i) / static_cast<float>(FIELD_LINE_COUNT_PER_CHARGE);
            const Vec2 seed = charge.position + Vec2(std::cos(theta), std::sin(theta)) * kSeedOffset;

            const std::vector<Vec2> polyline = sampler.traceFieldLine(seed, charges, followField);
            if (polyline.size() < 2)
                continue;

            sf::VertexArray lineStrip(sf::PrimitiveType::LineStrip, polyline.size());
            for (std::size_t p = 0; p < polyline.size(); ++p)
                lineStrip[p] = {sf::Vector2f(metersToPixels(polyline[p].x), metersToPixels(polyline[p].y)), sf::Color(FIELD_LINE_COLOR)};
            window.draw(lineStrip);
        }
    }
}

void Renderer::drawEquipotentials(sf::RenderWindow &window, const std::vector<PointCharge> &charges) const
{
    if (charges.empty())
        return;

    const EquipotentialTracer tracer;
    constexpr std::array<float, 6> kRadii = {0.3f, 0.6f, 1.0f, 1.6f, 2.5f, 4.0f}; // meters from each charge

    std::vector<float> levels;
    levels.reserve(charges.size() * kRadii.size());
    for (const auto &charge : charges)
        for (float radius : kRadii)
            levels.push_back(FieldMath::coulombPotential(charge.position + Vec2(radius, 0.0f), {charge}));

    const auto segments = tracer.traceContours(charges, levels);
    for (const auto &segment : segments)
    {
        if (segment.size() < 2)
            continue;

        sf::VertexArray line(sf::PrimitiveType::LineStrip, segment.size());
        for (std::size_t p = 0; p < segment.size(); ++p)
            line[p] = {sf::Vector2f(metersToPixels(segment[p].x), metersToPixels(segment[p].y)), sf::Color(EQUIPOTENTIAL_LINE_COLOR)};
        window.draw(line);
    }
}

void Renderer::drawGaussianSurfaces(sf::RenderWindow &window, const std::vector<GaussianSurface> &surfaces, const std::vector<PointCharge> &charges) const
{
    for (const auto &surface : surfaces)
    {
        const float radiusPixels = metersToPixels(surface.radius);
        const sf::Vector2f centerPixels(metersToPixels(surface.center.x), metersToPixels(surface.center.y));

        sf::CircleShape shape(radiusPixels);
        shape.setOrigin(sf::Vector2f(radiusPixels, radiusPixels));
        shape.setPosition(centerPixels);
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(GAUSSIAN_SURFACE_COLOR);
        shape.setOutlineThickness(2.0f);
        window.draw(shape);

        const float enclosed = surface.enclosedCharge(charges);
        const std::string label = fmt::format("Q_enc = {:.3g} C", enclosed);
        const sf::Vector2f labelWorldPos(centerPixels.x, centerPixels.y - radiusPixels - 16.0f);
        const sf::Vector2i screenPos = window.mapCoordsToPixel(labelWorldPos, window.getView());

        const sf::Color &c = GAUSSIAN_SURFACE_COLOR;
        ImGui::GetForegroundDrawList()->AddText(ImVec2(static_cast<float>(screenPos.x), static_cast<float>(screenPos.y)),
                                                 IM_COL32(c.r, c.g, c.b, c.a), label.c_str());
    }
}
