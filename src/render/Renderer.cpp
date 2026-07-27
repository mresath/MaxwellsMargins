#include "render/Renderer.hpp"

#include "Config.hpp"
#include "circuits/Battery.hpp"
#include "circuits/Capacitor.hpp"
#include "circuits/CircuitGraph.hpp"
#include "circuits/Inductor.hpp"
#include "circuits/Lightbulb.hpp"
#include "circuits/Probe.hpp"
#include "circuits/Resistor.hpp"
#include "circuits/Switch.hpp"
#include "core/World.hpp"
#include "electrostatics/EquipotentialTracer.hpp"
#include "electrostatics/FieldSampler.hpp"
#include "engine/FieldMath.hpp"
#include "induction/MovingLoop.hpp"
#include "magnetism/ChargedParticle.hpp"
#include "magnetism/CurrentLoop.hpp"
#include "magnetism/CurrentWire.hpp"
#include "math/Util.hpp"

#include <imgui.h>
#include <fmt/format.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

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

// Steps CurrentFlowDisplay markers along a straight segment at a speed set by |current| -
// shared by Fields' CurrentWire and Circuits' wires/component leads alike. saturationConstant
// differs between the two (Fields' wires are user-set up to MAX_WIRE_CURRENT = 20A; solved
// circuit currents are realistically mA-to-low-A, so they'd otherwise read as near-static).
void drawStraightFlowMarkers(sf::RenderWindow &window, const Vec2 &start, const Vec2 &end, float current, float simTime, CurrentFlowDisplay mode,
                              float saturationConstant = CURRENT_FLOW_SPEED_SATURATION)
{
    const float magnitude = std::abs(current);
    if (magnitude < 1e-6f)
        return;

    const Vec2 segment = end - start;
    const float length = segment.length();
    if (length < 1e-6f)
        return;

    const Vec2 direction = segment / length;
    const Vec2 travelDirection = direction * flowDirectionSign(current, mode);
    const float speed = CURRENT_FLOW_MAX_SPEED * (magnitude / (magnitude + saturationConstant)) * flowDirectionSign(current, mode);

    float offset = std::fmod(simTime * speed, CURRENT_FLOW_MARKER_SPACING);
    if (offset < 0.0f)
        offset += CURRENT_FLOW_MARKER_SPACING;

    for (float d = offset; d < length; d += CURRENT_FLOW_MARKER_SPACING)
        drawFlowMarker(window, start + direction * d, travelDirection, mode);
}

// A coil's turns, drawn as concentric rings stepping inward from the nominal radius
// (capped at LOOP_VISUAL_MAX_RINGS so a high turn count reads as "many" without becoming
// a solid disc); squish matches drawCircularFlowMarkers' foreshortening convention.
void drawLoopRings(sf::RenderWindow &window, const Vec2 &center, float radius, float squish, int turns, const sf::Color &color)
{
    const int rings = std::min(turns, LOOP_VISUAL_MAX_RINGS);
    for (int i = 0; i < rings; ++i)
    {
        const float ringRadius = radius - static_cast<float>(i) * LOOP_RING_SPACING;
        if (ringRadius <= 0.0f)
            break;

        const float radiusPixels = metersToPixels(ringRadius);
        sf::CircleShape shape(radiusPixels);
        shape.setOrigin(sf::Vector2f(radiusPixels, radiusPixels));
        shape.setPosition(sf::Vector2f(metersToPixels(center.x), metersToPixels(center.y)));
        shape.setScale(sf::Vector2f(squish, 1.0f));
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(color);
        shape.setOutlineThickness(2.0f);
        window.draw(shape);
    }
}

// Shared by MovingLoop (squish foreshortens with rotationAngle) and CurrentLoop (squish
// fixed at 1 - it's flat, not tilting), each passing its own signed physical quantity.
void drawCircularFlowMarkers(sf::RenderWindow &window, const Vec2 &center, float radius, float squish, float signedQuantity,
                              float saturationConstant, float simTime, CurrentFlowDisplay mode)
{
    const float magnitude = std::abs(signedQuantity);
    if (magnitude < 1e-6f)
        return;

    const float angularSpeed = (CURRENT_FLOW_MAX_SPEED * (magnitude / (magnitude + saturationConstant)) / radius) *
                                flowDirectionSign(signedQuantity, mode);
    const float angleStep = CURRENT_FLOW_MARKER_SPACING / radius;

    float phase = std::fmod(simTime * angularSpeed, angleStep);
    if (phase < 0.0f)
        phase += angleStep;

    constexpr float kTwoPi = 6.28318530718f;
    for (float angle = phase; angle < kTwoPi; angle += angleStep)
    {
        const Vec2 offset(std::cos(angle) * radius * squish, std::sin(angle) * radius);
        const Vec2 tangent(-std::sin(angle) * squish, std::cos(angle));
        const Vec2 travelDirection = angularSpeed >= 0.0f ? tangent : tangent * -1.0f;
        drawFlowMarker(window, center + offset, travelDirection, mode);
    }
}

// A component's own (along, across) coordinate frame - along the posA->posB direction and
// perpendicular to it - so every schematic symbol is built once in local space and drawn
// at whatever position/orientation/length the user actually dragged.
struct ComponentFrame
{
    Vec2 posA;
    Vec2 direction;
    Vec2 perpendicular;
    float span;
    float symbolHalf;
};

ComponentFrame frameFor(const Component &component)
{
    const Vec2 segment = component.posB - component.posA;
    const float span = segment.length();
    const Vec2 direction = span > 1e-6f ? segment / span : Vec2(1.0f, 0.0f);
    // Scales the symbol down on a short drag so it never overflows past the terminals.
    const float symbolHalf = std::min(COMPONENT_SYMBOL_HALF_SIZE, span * 0.45f);
    return {component.posA, direction, direction.perpendicular(), span, symbolHalf};
}

Vec2 localToWorld(const ComponentFrame &frame, float along, float across)
{
    return frame.posA + frame.direction * along + frame.perpendicular * across;
}

void drawComponentLeads(sf::RenderWindow &window, const ComponentFrame &frame, const sf::Color &color)
{
    const float midAlong = frame.span * 0.5f;
    const Vec2 symbolStart = localToWorld(frame, midAlong - frame.symbolHalf, 0.0f);
    const Vec2 symbolEnd = localToWorld(frame, midAlong + frame.symbolHalf, 0.0f);
    drawWireSegment(window, frame.posA, symbolStart, color);
    drawWireSegment(window, symbolEnd, localToWorld(frame, frame.span, 0.0f), color);
}

std::uint8_t lerpChannel(std::uint8_t from, std::uint8_t to, float t)
{
    return static_cast<std::uint8_t>(static_cast<float>(from) + t * (static_cast<float>(to) - static_cast<float>(from)));
}

// A glass circle (glowing toward LIGHTBULB_GLOW_COLOR as dissipated power increases) with
// an X filament - Lightbulb is electrically a plain Resistor, so only this symbol differs.
void drawLightbulbSymbol(sf::RenderWindow &window, const ComponentFrame &frame, float power, const sf::Color &lineColor)
{
    const float midAlong = frame.span * 0.5f;
    const Vec2 center = localToWorld(frame, midAlong, 0.0f);
    const float radiusPixels = metersToPixels(frame.symbolHalf * 0.9f);
    const sf::Vector2f centerPixels(metersToPixels(center.x), metersToPixels(center.y));

    const float brightness = std::abs(power) / (std::abs(power) + LIGHTBULB_POWER_SATURATION);
    const sf::Color &off = LIGHTBULB_OFF_COLOR;
    const sf::Color &glow = LIGHTBULB_GLOW_COLOR;
    const sf::Color bulbColor(lerpChannel(off.r, glow.r, brightness), lerpChannel(off.g, glow.g, brightness), lerpChannel(off.b, glow.b, brightness));

    sf::CircleShape bulb(radiusPixels);
    bulb.setOrigin(sf::Vector2f(radiusPixels, radiusPixels));
    bulb.setPosition(centerPixels);
    bulb.setFillColor(bulbColor);
    bulb.setOutlineColor(lineColor);
    bulb.setOutlineThickness(2.0f);
    window.draw(bulb);

    const float f = frame.symbolHalf * 0.5f;
    drawWireSegment(window, localToWorld(frame, midAlong - f, -f), localToWorld(frame, midAlong + f, f), lineColor);
    drawWireSegment(window, localToWorld(frame, midAlong - f, f), localToWorld(frame, midAlong + f, -f), lineColor);
}

void drawResistorSymbol(sf::RenderWindow &window, const ComponentFrame &frame, const sf::Color &color)
{
    const float midAlong = frame.span * 0.5f;
    constexpr int kZigzags = 6;
    const float amplitude = frame.symbolHalf * 0.6f;

    std::vector<Vec2> points(kZigzags + 1);
    for (int i = 0; i <= kZigzags; ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(kZigzags);
        const float along = midAlong - frame.symbolHalf + t * (2.0f * frame.symbolHalf);
        const bool isEndpoint = (i == 0 || i == kZigzags);
        const float across = isEndpoint ? 0.0f : ((i % 2 == 0) ? amplitude : -amplitude);
        points[i] = localToWorld(frame, along, across);
    }

    sf::VertexArray line(sf::PrimitiveType::LineStrip, points.size());
    for (std::size_t i = 0; i < points.size(); ++i)
        line[i] = {sf::Vector2f(metersToPixels(points[i].x), metersToPixels(points[i].y)), color};
    window.draw(line);
}

void drawCapacitorSymbol(sf::RenderWindow &window, const ComponentFrame &frame, const sf::Color &color)
{
    const float midAlong = frame.span * 0.5f;
    const float gap = frame.symbolHalf * 0.3f;
    const float plateHalfHeight = frame.symbolHalf * 0.8f;

    for (float along : {midAlong - gap, midAlong + gap})
        drawWireSegment(window, localToWorld(frame, along, plateHalfHeight), localToWorld(frame, along, -plateHalfHeight), color);
}

// Long thin plate near posA (the + terminal by convention), short thick-looking plate near
// posB - the standard schematic battery symbol.
void drawBatterySymbol(sf::RenderWindow &window, const ComponentFrame &frame, const sf::Color &color)
{
    const float midAlong = frame.span * 0.5f;
    const float gap = frame.symbolHalf * 0.3f;
    const float longHalfHeight = frame.symbolHalf * 0.9f;
    const float shortHalfHeight = frame.symbolHalf * 0.45f;

    drawWireSegment(window, localToWorld(frame, midAlong - gap, longHalfHeight), localToWorld(frame, midAlong - gap, -longHalfHeight), color);
    drawWireSegment(window, localToWorld(frame, midAlong + gap, shortHalfHeight), localToWorld(frame, midAlong + gap, -shortHalfHeight), color);
}

// A row of same-direction bumps, the standard schematic coil symbol.
void drawInductorSymbol(sf::RenderWindow &window, const ComponentFrame &frame, const sf::Color &color)
{
    const float midAlong = frame.span * 0.5f;
    constexpr int kBumps = 3;
    constexpr int kPointsPerBump = 8;
    const float amplitude = frame.symbolHalf * 0.7f;

    std::vector<Vec2> points(kBumps * kPointsPerBump + 1);
    for (int i = 0; i < static_cast<int>(points.size()); ++i)
    {
        const float t = static_cast<float>(i) / static_cast<float>(points.size() - 1);
        const float along = midAlong - frame.symbolHalf + t * (2.0f * frame.symbolHalf);
        const float bumpPhase = t * static_cast<float>(kBumps) * 3.14159265f;
        const float across = -std::abs(std::sin(bumpPhase)) * amplitude;
        points[i] = localToWorld(frame, along, across);
    }

    sf::VertexArray line(sf::PrimitiveType::LineStrip, points.size());
    for (std::size_t i = 0; i < points.size(); ++i)
        line[i] = {sf::Vector2f(metersToPixels(points[i].x), metersToPixels(points[i].y)), color};
    window.draw(line);
}

void drawSwitchSymbol(sf::RenderWindow &window, const ComponentFrame &frame, bool closed, const sf::Color &color)
{
    const float midAlong = frame.span * 0.5f;
    const Vec2 hinge = localToWorld(frame, midAlong - frame.symbolHalf, 0.0f);
    const Vec2 contact = localToWorld(frame, midAlong + frame.symbolHalf, 0.0f);

    const float dotRadiusPixels = metersToPixels(frame.symbolHalf * 0.15f);
    for (const Vec2 &point : {hinge, contact})
    {
        sf::CircleShape dot(dotRadiusPixels);
        dot.setOrigin(sf::Vector2f(dotRadiusPixels, dotRadiusPixels));
        dot.setPosition(sf::Vector2f(metersToPixels(point.x), metersToPixels(point.y)));
        dot.setFillColor(color);
        window.draw(dot);
    }

    const Vec2 armEnd = closed ? contact : localToWorld(frame, midAlong + frame.symbolHalf * 0.3f, frame.symbolHalf * 0.9f);
    drawWireSegment(window, hinge, armEnd, color);
}

void drawProbeSymbol(sf::RenderWindow &window, const ComponentFrame &frame, bool isAmmeter, const sf::Color &color)
{
    const float midAlong = frame.span * 0.5f;
    const Vec2 center = localToWorld(frame, midAlong, 0.0f);
    const float radiusPixels = metersToPixels(frame.symbolHalf * 0.9f);
    const sf::Vector2f centerPixels(metersToPixels(center.x), metersToPixels(center.y));

    sf::CircleShape circle(radiusPixels);
    circle.setOrigin(sf::Vector2f(radiusPixels, radiusPixels));
    circle.setPosition(centerPixels);
    circle.setFillColor(sf::Color::Transparent);
    circle.setOutlineColor(color);
    circle.setOutlineThickness(2.0f);
    window.draw(circle);

    const sf::Vector2i screenPos = window.mapCoordsToPixel(centerPixels, window.getView());
    const float halfCharWidth = ImGui::CalcTextSize(isAmmeter ? "A" : "V").x * 0.5f;
    const float halfCharHeight = ImGui::CalcTextSize(isAmmeter ? "A" : "V").y * 0.5f;
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(static_cast<float>(screenPos.x) - halfCharWidth, static_cast<float>(screenPos.y) - halfCharHeight),
                                             IM_COL32(color.r, color.g, color.b, color.a), isAmmeter ? "A" : "V");
}

void drawComponentLabel(sf::RenderWindow &window, const ComponentFrame &frame, const std::string &text, const sf::Color &color)
{
    const float midAlong = frame.span * 0.5f;
    const Vec2 labelPos = localToWorld(frame, midAlong, -(frame.symbolHalf + 0.3f));
    const sf::Vector2f labelWorldPos(metersToPixels(labelPos.x), metersToPixels(labelPos.y));
    const sf::Vector2i screenPos = window.mapCoordsToPixel(labelWorldPos, window.getView());
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(static_cast<float>(screenPos.x), static_cast<float>(screenPos.y)), IM_COL32(color.r, color.g, color.b, color.a), text.c_str());
}
} // namespace

Renderer::Renderer() = default;

void Renderer::draw(sf::RenderWindow &window, Mode mode, const World &world, const CircuitGraph &circuit,
                     const std::optional<std::pair<Vec2, Vec2>> &dragPreview) const
{
    drawGridlines(window);

    if (mode == Mode::Fields)
        drawWorld(window, world, dragPreview);
    else
        drawCircuit(window, circuit, dragPreview);
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
    drawCurrentLoops(window, world);
    drawParticles(window, world.particles());
    drawLoops(window, world);
    drawCharges(window, world.charges());
}

void Renderer::drawCircuit(sf::RenderWindow &window, const CircuitGraph &circuit, const std::optional<std::pair<Vec2, Vec2>> &dragPreview) const
{
    drawCircuitWires(window, circuit, dragPreview);
    drawCircuitComponents(window, circuit);
}

void Renderer::drawCircuitWires(sf::RenderWindow &window, const CircuitGraph &circuit, const std::optional<std::pair<Vec2, Vec2>> &dragPreview) const
{
    for (const auto &wire : circuit.wires())
    {
        drawWireSegment(window, wire.start, wire.end, WIRE_COLOR);
        if (currentFlowDisplay != CurrentFlowDisplay::Off)
            drawStraightFlowMarkers(window, wire.start, wire.end, wire.current, circuit.simTime(), currentFlowDisplay, CIRCUIT_CURRENT_FLOW_SATURATION);
    }

    if (dragPreview)
        drawWireSegment(window, dragPreview->first, dragPreview->second, WIRE_COLOR);
}

void Renderer::drawCircuitComponents(sf::RenderWindow &window, const CircuitGraph &circuit) const
{
    for (const auto &comp : circuit.components())
    {
        const ComponentFrame frame = frameFor(*comp);

        if (Lightbulb *bulb = dynamic_cast<Lightbulb *>(comp.get()))
        {
            // Checked before Resistor - Lightbulb IS-A Resistor, so the plain dynamic_cast
            // below would otherwise match it first and draw a zigzag instead of a bulb.
            drawComponentLeads(window, frame, COMPONENT_COLOR);
            drawLightbulbSymbol(window, frame, bulb->power(), COMPONENT_COLOR);
            if (bulb->showLabel)
                drawComponentLabel(window, frame, fmt::format("R={:.3g} Ohm  V={:.3g} V  I={:.3g} A", bulb->resistance, bulb->voltage, bulb->current), COMPONENT_LABEL_COLOR);
        }
        else if (Resistor *resistor = dynamic_cast<Resistor *>(comp.get()))
        {
            drawComponentLeads(window, frame, COMPONENT_COLOR);
            drawResistorSymbol(window, frame, COMPONENT_COLOR);
            if (resistor->showLabel)
                drawComponentLabel(window, frame, fmt::format("R={:.3g} Ohm  V={:.3g} V  I={:.3g} A", resistor->resistance, resistor->voltage, resistor->current), COMPONENT_LABEL_COLOR);
        }
        else if (Capacitor *capacitor = dynamic_cast<Capacitor *>(comp.get()))
        {
            drawComponentLeads(window, frame, COMPONENT_COLOR);
            drawCapacitorSymbol(window, frame, COMPONENT_COLOR);
            if (capacitor->showLabel)
                drawComponentLabel(window, frame, fmt::format("C={:.3g} F  V={:.3g} V  Q={:.3g} C", capacitor->capacitance, capacitor->voltage, capacitor->charge), COMPONENT_LABEL_COLOR);
        }
        else if (Inductor *inductor = dynamic_cast<Inductor *>(comp.get()))
        {
            drawComponentLeads(window, frame, COMPONENT_COLOR);
            drawInductorSymbol(window, frame, COMPONENT_COLOR);
            if (inductor->showLabel)
                drawComponentLabel(window, frame, fmt::format("L={:.3g} H  V={:.3g} V  I={:.3g} A", inductor->inductance, inductor->voltage, inductor->current), COMPONENT_LABEL_COLOR);
        }
        else if (Battery *battery = dynamic_cast<Battery *>(comp.get()))
        {
            drawComponentLeads(window, frame, COMPONENT_COLOR);
            drawBatterySymbol(window, frame, COMPONENT_COLOR);
            if (battery->showLabel)
                drawComponentLabel(window, frame, fmt::format("EMF={:.3g} V  V={:.3g} V  I={:.3g} A", battery->emf, battery->voltage, battery->current), COMPONENT_LABEL_COLOR);
        }
        else if (Switch *sw = dynamic_cast<Switch *>(comp.get()))
        {
            const sf::Color &color = sw->closed ? COMPONENT_COLOR : OPEN_SWITCH_COLOR;
            drawComponentLeads(window, frame, color);
            drawSwitchSymbol(window, frame, sw->closed, color);
            if (sw->showLabel)
                drawComponentLabel(window, frame, fmt::format("{}  V={:.3g} V  I={:.3g} A", sw->closed ? "Closed" : "Open", sw->voltage, sw->current), COMPONENT_LABEL_COLOR);
        }
        else if (Probe *probe = dynamic_cast<Probe *>(comp.get()))
        {
            const bool isAmmeter = probe->kind == Probe::Kind::Ammeter;
            drawComponentLeads(window, frame, PROBE_COLOR);
            drawProbeSymbol(window, frame, isAmmeter, PROBE_COLOR);
            if (probe->showLabel)
                drawComponentLabel(window, frame, isAmmeter ? fmt::format("I={:.3g} A", probe->current) : fmt::format("V={:.3g} V", probe->voltage), COMPONENT_LABEL_COLOR);
        }

        if (currentFlowDisplay != CurrentFlowDisplay::Off)
            drawStraightFlowMarkers(window, comp->posA, comp->posB, comp->current, circuit.simTime(), currentFlowDisplay, CIRCUIT_CURRENT_FLOW_SATURATION);
    }
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
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(static_cast<float>(screenPos.x), static_cast<float>(screenPos.y)),
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
        if (currentFlowDisplay != CurrentFlowDisplay::Off)
            drawStraightFlowMarkers(window, wire.start, wire.end, wire.current, world.simTime(), currentFlowDisplay);
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
            ImGui::GetBackgroundDrawList()->AddText(ImVec2(static_cast<float>(screenPos.x), static_cast<float>(screenPos.y)),
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
    for (const auto &loop : world.loops())
    {
        const float radiusPixels = metersToPixels(loop.radius);
        // Correct orthographic projection of a true 3D circle tilting out of the page.
        const float squish = std::abs(std::cos(loop.rotationAngle));
        drawLoopRings(window, loop.center, loop.radius, squish, loop.turns, LOOP_COLOR);

        if (std::abs(loop.inducedEMF) < MIN_DISPLAYED_EMF)
            continue;

        // No resistance is modeled for a bare loop (that's a circuits concept), so marker
        // speed saturates against the induced EMF itself rather than an Amp value.
        if (currentFlowDisplay != CurrentFlowDisplay::Off)
            drawCircularFlowMarkers(window, loop.center, loop.radius, squish, loop.inducedEMF, CURRENT_FLOW_EMF_SATURATION, world.simTime(), currentFlowDisplay);

        const std::string label = fmt::format("EMF = {:.3g} V", loop.inducedEMF);
        const sf::Vector2f labelWorldPos(metersToPixels(loop.center.x), metersToPixels(loop.center.y) - radiusPixels - 16.0f);
        const sf::Vector2i screenPos = window.mapCoordsToPixel(labelWorldPos, window.getView());

        const sf::Color &c = INDUCED_EMF_ARROW_COLOR;
        ImGui::GetBackgroundDrawList()->AddText(ImVec2(static_cast<float>(screenPos.x), static_cast<float>(screenPos.y)),
                                                 IM_COL32(c.r, c.g, c.b, c.a), label.c_str());
    }
}

void Renderer::drawCurrentLoops(sf::RenderWindow &window, const World &world) const
{
    for (const auto &loop : world.currentLoops())
    {
        drawLoopRings(window, loop.center, loop.radius, 1.0f, loop.turns, LOOP_COLOR);

        if (currentFlowDisplay != CurrentFlowDisplay::Off)
            drawCircularFlowMarkers(window, loop.center, loop.radius, 1.0f, loop.current, CURRENT_FLOW_SPEED_SATURATION, world.simTime(), currentFlowDisplay);
    }
}
