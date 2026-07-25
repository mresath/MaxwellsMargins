#include "core/Tools.hpp"

#include "Config.hpp"
#include "circuits/CircuitGraph.hpp"
#include "core/World.hpp"

Tools::Tools() : m_activeTool(ToolType::Select), m_chargeMagnitude(DEFAULT_CHARGE_MAGNITUDE), m_gaussianRadius(GAUSSIAN_SURFACE_DEFAULT_RADIUS)
{
}

ToolType Tools::activeTool() const
{
    return m_activeTool;
}

void Tools::setActiveTool(ToolType tool)
{
    m_activeTool = tool;
}

float Tools::chargeMagnitude() const { return m_chargeMagnitude; }
void Tools::setChargeMagnitude(float magnitude) { m_chargeMagnitude = magnitude; }
float Tools::gaussianRadius() const { return m_gaussianRadius; }
void Tools::setGaussianRadius(float radius) { m_gaussianRadius = radius; }

void Tools::onClick(const Vec2 &worldPos, World &world)
{
    // worldPos is in meters. Move is handled in App (press/drag/release, not a click).
    switch (m_activeTool)
    {
    case ToolType::PlacePositiveCharge:
        world.charges().emplace_back(worldPos, m_chargeMagnitude, world.allocateEntityId());
        break;
    case ToolType::PlaceNegativeCharge:
        world.charges().emplace_back(worldPos, -m_chargeMagnitude, world.allocateEntityId());
        break;
    case ToolType::DrawGaussianSurface:
        world.gaussianSurfaces().emplace_back(worldPos, m_gaussianRadius, world.allocateEntityId());
        break;
    case ToolType::Erase:
    {
        const EntityRef hit = world.findEntityAt(worldPos);
        if (hit.kind != EntityKind::None)
            world.removeEntity(hit.kind, hit.id);
        break;
    }
    default:
        break;
    }
}

void Tools::onClick(const Vec2 &worldPos, CircuitGraph &circuit)
{
    // TODO(Phase 5): create/select components in `circuit` based on m_activeTool
    (void)worldPos;
    (void)circuit;
}
