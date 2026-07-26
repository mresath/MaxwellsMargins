#include "render/Renderer.hpp"

#include "Config.hpp"
#include "circuits/CircuitGraph.hpp"
#include "core/World.hpp"
#include "electrostatics/EquipotentialTracer.hpp"
#include "electrostatics/FieldSampler.hpp"
#include "engine/FieldMath.hpp"
#include "induction/MovingLoop.hpp"
#include "magnetism/ChargedParticle.hpp"
#include "magnetism/CurrentWire.hpp"
#include "math/Util.hpp"

#include <imgui.h>
#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
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

void drawWireSegment(sf::RenderWindow &window, const Vec2 &start, const Vec2 &end, const sf::Color &color)
{
    sf::VertexArray line(sf::PrimitiveType::Lines, 2);
    line[0] = {sf::Vector2f(metersToPixels(start.x), metersToPixels(start.y)), color};
    line[1] = {sf::Vector2f(metersToPixels(end.x), metersToPixels(end.y)), color};
    window.draw(line);
}

// Conventional current moves in the signed-current (or signed-EMF) direction; electron
// flow is the physically-opposite motion of the actual (negative) charge carriers.
float flowDirectionSign(float signedQuantity, CurrentFlowDisplay mode)
{
    const float conventionalSign = signedQuantity >= 0.0f ? 1.0f : -1.0f;
    return mode == CurrentFlowDisplay::Electron ? -conventionalSign : conventionalSign;
}

// Conventional is the textbook direction-only abstraction (an arrow chevron); Electron is
// the literal (negative) charge carriers (a dot).
void drawFlowMarker(sf::RenderWindow &window, const Vec2 &pointMeters, const Vec2 &travelDirectionMeters, CurrentFlowDisplay mode)
{
    const sf::Vector2f centerPixels(metersToPixels(pointMeters.x), metersToPixels(pointMeters.y));

    if (mode == CurrentFlowDisplay::Conventional)
    {
        const float sizePixels = metersToPixels(CURRENT_FLOW_MARKER_RADIUS) * 1.8f;
        sf::CircleShape shape(sizePixels, 3);
        shape.setOrigin(sf::Vector2f(sizePixels, sizePixels));
        shape.setPosition(centerPixels);
        shape.setFillColor(POSITIVE_CHARGE_COLOR);

        // Point 0 of an SFML regular polygon faces up (-y); rotate that tip to face the
        // travel direction (clockwise degrees, matching SFML's screen-space convention).
        constexpr float kRadToDeg = 180.0f / 3.14159265358979323846f;
        const float angleDeg = std::atan2(travelDirectionMeters.y, travelDirectionMeters.x) * kRadToDeg + 90.0f;
        shape.setRotation(sf::degrees(angleDeg));
        window.draw(shape);
    }
    else
    {
        const float radiusPixels = metersToPixels(CURRENT_FLOW_MARKER_RADIUS);
        sf::CircleShape shape(radiusPixels);
        shape.setOrigin(sf::Vector2f(radiusPixels, radiusPixels));
        shape.setPosition(centerPixels);
        shape.setFillColor(NEGATIVE_CHARGE_COLOR);
        window.draw(shape);
    }
}
} // namespace

Renderer::Renderer() = default;

void Renderer::draw(sf::RenderWindow &window, Mode mode, const World &world, const CircuitGraph &circuit,
                     const std::optional<std::pair<Vec2, Vec2>> &wirePreview) const
{
    drawGridlines(window);

    if (mode == Mode::Fields)
        drawWorld(window, world, wirePreview);
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

void Renderer::drawWorld(sf::RenderWindow &window, const World &world, const std::optional<std::pair<Vec2, Vec2>> &wirePreview) const
{
    // Moving charged particles are E field sources too, so field vectors/lines/
    // equipotentials sample every charge source (static + particles), not just world.charges().
    const auto fieldSources = world.allChargeSources();

    if (showEquipotentials)
        drawEquipotentials(window, fieldSources);
    if (showFieldLines)
        drawFieldLines(window, fieldSources);
    if (showFieldVectors)
        drawFieldVectors(window, fieldSources);
    if (showMagneticField)
        drawMagneticField(window, world);

    drawGaussianSurfaces(window, world.gaussianSurfaces(), world.charges());
    drawWires(window, world, wirePreview);
    drawWireForceReadouts(window, world);
    drawParticles(window, world.particles());
    drawLoops(window, world);
    drawCharges(window, world.charges());
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

    const sf::View view = window.getView();
    const Vec2 minWorld = pixelsToMeters(Vec2(view.getCenter().x - view.getSize().x / 2.0f, view.getCenter().y - view.getSize().y / 2.0f));
    const Vec2 maxWorld = pixelsToMeters(Vec2(view.getCenter().x + view.getSize().x / 2.0f, view.getCenter().y + view.getSize().y / 2.0f));

    const auto segments = tracer.traceContours(charges, levels, minWorld, maxWorld);
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

namespace
{
// Out of the page: a dot, like the tip of an arrow pointing at the viewer. Into the page:
// a cross, like the fletching of an arrow flying away.
void drawFieldMarker(sf::RenderWindow &window, const Vec2 &pointMeters, float radiusMeters, bool outOfPage, const sf::Color &color)
{
    const float radiusPixels = metersToPixels(radiusMeters);
    const sf::Vector2f centerPixels(metersToPixels(pointMeters.x), metersToPixels(pointMeters.y));

    if (outOfPage)
    {
        sf::CircleShape dot(radiusPixels);
        dot.setOrigin(sf::Vector2f(radiusPixels, radiusPixels));
        dot.setPosition(centerPixels);
        dot.setFillColor(color);
        window.draw(dot);
    }
    else
    {
        sf::VertexArray cross(sf::PrimitiveType::Lines, 4);
        cross[0] = {sf::Vector2f(centerPixels.x - radiusPixels, centerPixels.y - radiusPixels), color};
        cross[1] = {sf::Vector2f(centerPixels.x + radiusPixels, centerPixels.y + radiusPixels), color};
        cross[2] = {sf::Vector2f(centerPixels.x - radiusPixels, centerPixels.y + radiusPixels), color};
        cross[3] = {sf::Vector2f(centerPixels.x + radiusPixels, centerPixels.y - radiusPixels), color};
        window.draw(cross);
    }
}
} // namespace

void Renderer::drawMagneticField(sf::RenderWindow &window, const World &world) const
{
    const sf::View view = window.getView();
    const Vec2 minWorld = pixelsToMeters(Vec2(view.getCenter().x - view.getSize().x / 2.0f, view.getCenter().y - view.getSize().y / 2.0f));
    const Vec2 maxWorld = pixelsToMeters(Vec2(view.getCenter().x + view.getSize().x / 2.0f, view.getCenter().y + view.getSize().y / 2.0f));

    for (float y = minWorld.y; y <= maxWorld.y; y += B_FIELD_MARKER_SPACING)
    {
        for (float x = minWorld.x; x <= maxWorld.x; x += B_FIELD_MARKER_SPACING)
        {
            const float field = world.magneticFieldAt(Vec2(x, y));
            const float magnitude = std::abs(field);
            if (magnitude < B_FIELD_MARKER_MIN_MAGNITUDE)
                continue;

            const float radius = B_FIELD_MARKER_MAX_RADIUS * (magnitude / (magnitude + B_FIELD_MARKER_SATURATION));
            const sf::Color &color = field >= 0.0f ? B_FIELD_OUT_OF_PAGE_COLOR : B_FIELD_INTO_PAGE_COLOR;
            drawFieldMarker(window, Vec2(x, y), radius, field >= 0.0f, color);
        }
    }
}

void Renderer::drawWires(sf::RenderWindow &window, const World &world, const std::optional<std::pair<Vec2, Vec2>> &wirePreview) const
{
    for (const auto &wire : world.wires())
    {
        drawWireSegment(window, wire.start, wire.end, CURRENT_WIRE_COLOR);

        if (currentFlowDisplay == CurrentFlowDisplay::Off)
            continue;

        const Vec2 segment = wire.end - wire.start;
        const float length = segment.length();
        if (length < 1e-6f)
            continue;

        const Vec2 direction = segment / length;
        const Vec2 travelDirection = direction * flowDirectionSign(wire.current, currentFlowDisplay);
        const float magnitude = std::abs(wire.current);
        const float speed = CURRENT_FLOW_MAX_SPEED * (magnitude / (magnitude + CURRENT_FLOW_SPEED_SATURATION)) * flowDirectionSign(wire.current, currentFlowDisplay);

        float offset = std::fmod(world.simTime() * speed, CURRENT_FLOW_MARKER_SPACING);
        if (offset < 0.0f)
            offset += CURRENT_FLOW_MARKER_SPACING;

        for (float d = offset; d < length; d += CURRENT_FLOW_MARKER_SPACING)
            drawFlowMarker(window, wire.start + direction * d, travelDirection, currentFlowDisplay);
    }

    if (wirePreview)
        drawWireSegment(window, wirePreview->first, wirePreview->second, CURRENT_WIRE_COLOR);
}

void Renderer::drawWireForceReadouts(sf::RenderWindow &window, const World &world) const
{
    const auto &wires = world.wires();
    for (std::size_t i = 0; i < wires.size(); ++i)
    {
        for (std::size_t j = i + 1; j < wires.size(); ++j)
        {
            const Vec2 segmentA = wires[i].end - wires[i].start;
            const Vec2 segmentB = wires[j].end - wires[j].start;
            const float lengthA = segmentA.length();
            const float lengthB = segmentB.length();
            if (lengthA < 1e-6f || lengthB < 1e-6f)
                continue;

            const Vec2 dirA = segmentA / lengthA;
            const Vec2 dirB = segmentB / lengthB;
            const float alignment = dot(dirA, dirB);
            if (std::abs(alignment) < WIRE_PARALLEL_DOT_THRESHOLD)
                continue;

            const float separation = std::abs(cross(dirA, wires[j].start - wires[i].start));
            // Flip current2's sign for an anti-parallel pair, so it still means "same
            // real-world direction" rather than "same as this wire's own start->end".
            const float current2 = alignment >= 0.0f ? wires[j].current : -wires[j].current;
            const float force = FieldMath::forceBetweenWires(wires[i].current, current2, separation, std::min(lengthA, lengthB), world.permeabilityFactor());

            const Vec2 midpoint = ((wires[i].start + wires[i].end) * 0.5f + (wires[j].start + wires[j].end) * 0.5f) * 0.5f;
            const std::string label = fmt::format("F = {:.3g} N ({})", std::abs(force), force >= 0.0f ? "Attract" : "Repel");

            const sf::Vector2f labelWorldPos(metersToPixels(midpoint.x), metersToPixels(midpoint.y));
            const sf::Vector2i screenPos = window.mapCoordsToPixel(labelWorldPos, window.getView());

            const sf::Color &c = CURRENT_WIRE_COLOR;
            ImGui::GetForegroundDrawList()->AddText(ImVec2(static_cast<float>(screenPos.x), static_cast<float>(screenPos.y)),
                                                     IM_COL32(c.r, c.g, c.b, c.a), label.c_str());
        }
    }
}

void Renderer::drawParticles(sf::RenderWindow &window, const std::vector<ChargedParticle> &particles) const
{
    for (const auto &particle : particles)
    {
        if (particle.trajectoryTraceEnabled && particle.trajectoryTrace.size() >= 2)
        {
            sf::VertexArray trace(sf::PrimitiveType::LineStrip, particle.trajectoryTrace.size());
            for (std::size_t p = 0; p < particle.trajectoryTrace.size(); ++p)
                trace[p] = {sf::Vector2f(metersToPixels(particle.trajectoryTrace[p].x), metersToPixels(particle.trajectoryTrace[p].y)), sf::Color(TRAJECTORY_TRACE_COLOR)};
            window.draw(trace);
        }

        const float radiusPixels = metersToPixels(PARTICLE_RADIUS);
        sf::CircleShape shape(radiusPixels);
        shape.setOrigin(sf::Vector2f(radiusPixels, radiusPixels));
        shape.setPosition(sf::Vector2f(metersToPixels(particle.position.x), metersToPixels(particle.position.y)));
        shape.setFillColor(particle.charge >= 0.0f ? POSITIVE_CHARGE_COLOR : NEGATIVE_CHARGE_COLOR);
        shape.setOutlineColor(sf::Color::White);
        shape.setOutlineThickness(1.0f);
        window.draw(shape);
    }
}

void Renderer::drawLoops(sf::RenderWindow &window, const World &world) const
{
    constexpr float kTwoPi = 6.28318530718f;

    for (const auto &loop : world.loops())
    {
        const float radiusPixels = metersToPixels(loop.radius);
        // Correct orthographic projection of a true 3D circle tilting out of the page.
        const float squish = std::abs(std::cos(loop.rotationAngle));

        sf::CircleShape shape(radiusPixels);
        shape.setOrigin(sf::Vector2f(radiusPixels, radiusPixels));
        shape.setPosition(sf::Vector2f(metersToPixels(loop.center.x), metersToPixels(loop.center.y)));
        shape.setScale(sf::Vector2f(squish, 1.0f));
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(LOOP_COLOR);
        shape.setOutlineThickness(2.0f);
        window.draw(shape);

        if (std::abs(loop.inducedEMF) < MIN_DISPLAYED_EMF)
            continue;

        if (currentFlowDisplay != CurrentFlowDisplay::Off)
        {
            // No resistance is modeled for a bare loop (that's a circuits concept), so
            // marker speed saturates against the induced EMF itself rather than an Amp value.
            const float magnitude = std::abs(loop.inducedEMF);
            const float angularSpeed = (CURRENT_FLOW_MAX_SPEED * (magnitude / (magnitude + CURRENT_FLOW_EMF_SATURATION)) / loop.radius) *
                                        flowDirectionSign(loop.inducedEMF, currentFlowDisplay);
            const float angleStep = CURRENT_FLOW_MARKER_SPACING / loop.radius;

            float phase = std::fmod(world.simTime() * angularSpeed, angleStep);
            if (phase < 0.0f)
                phase += angleStep;

            // Parametrized over the true circle's angle (equal arc-length steps in 3D), with
            // only the rendered x-position squished - same projection reasoning as the outline.
            for (float angle = phase; angle < kTwoPi; angle += angleStep)
            {
                const Vec2 offset(std::cos(angle) * loop.radius * squish, std::sin(angle) * loop.radius);
                const Vec2 tangent(-std::sin(angle) * squish, std::cos(angle));
                const Vec2 travelDirection = angularSpeed >= 0.0f ? tangent : tangent * -1.0f;
                drawFlowMarker(window, loop.center + offset, travelDirection, currentFlowDisplay);
            }
        }

        const std::string label = fmt::format("EMF = {:.3g} V", loop.inducedEMF);
        const sf::Vector2f labelWorldPos(metersToPixels(loop.center.x), metersToPixels(loop.center.y) - radiusPixels - 16.0f);
        const sf::Vector2i screenPos = window.mapCoordsToPixel(labelWorldPos, window.getView());

        const sf::Color &c = INDUCED_EMF_ARROW_COLOR;
        ImGui::GetForegroundDrawList()->AddText(ImVec2(static_cast<float>(screenPos.x), static_cast<float>(screenPos.y)),
                                                 IM_COL32(c.r, c.g, c.b, c.a), label.c_str());
    }
}
