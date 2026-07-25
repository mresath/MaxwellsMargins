#include "core/Tools.hpp"

#include "Config.hpp"
#include "circuits/CircuitGraph.hpp"
#include "core/World.hpp"

Tools::Tools()
    : m_activeTool(ToolType::Select), m_chargeMagnitude(DEFAULT_CHARGE_MAGNITUDE), m_gaussianRadius(GAUSSIAN_SURFACE_DEFAULT_RADIUS),
      m_particleChargeMagnitude(DEFAULT_PARTICLE_CHARGE_MAGNITUDE), m_particleChargePositive(true), m_particleMass(DEFAULT_PARTICLE_MASS), m_particleSpeed(DEFAULT_PARTICLE_SPEED),
      m_wireCurrent(DEFAULT_WIRE_CURRENT), m_wireDragActive(false), m_wireDragStart(0.0f, 0.0f)
{
}

ToolType Tools::activeTool() const
{
    return m_activeTool;
}

void Tools::setActiveTool(ToolType tool)
{
    m_activeTool = tool;
    m_wireDragActive = false; // switching tools mid-drag abandons the in-progress wire
}

float Tools::chargeMagnitude() const { return m_chargeMagnitude; }
void Tools::setChargeMagnitude(float magnitude) { m_chargeMagnitude = magnitude; }
float Tools::gaussianRadius() const { return m_gaussianRadius; }
void Tools::setGaussianRadius(float radius) { m_gaussianRadius = radius; }

float Tools::particleChargeMagnitude() const { return m_particleChargeMagnitude; }
void Tools::setParticleChargeMagnitude(float magnitude) { m_particleChargeMagnitude = magnitude; }
bool Tools::particleChargePositive() const { return m_particleChargePositive; }
void Tools::setParticleChargePositive(bool positive) { m_particleChargePositive = positive; }
float Tools::particleMass() const { return m_particleMass; }
void Tools::setParticleMass(float mass) { m_particleMass = mass; }
float Tools::particleSpeed() const { return m_particleSpeed; }
void Tools::setParticleSpeed(float speed) { m_particleSpeed = speed; }

float Tools::wireCurrent() const { return m_wireCurrent; }
void Tools::setWireCurrent(float current) { m_wireCurrent = current; }

bool Tools::isDraggingWire() const { return m_wireDragActive; }
Vec2 Tools::wireDragStart() const { return m_wireDragStart; }

void Tools::beginWireDrag(const Vec2 &worldPos)
{
    m_wireDragActive = true;
    m_wireDragStart = worldPos;
}

void Tools::finishWireDrag(const Vec2 &worldPos, World &world)
{
    if (m_wireDragActive && (worldPos - m_wireDragStart).length() >= MIN_WIRE_LENGTH)
        world.wires().emplace_back(m_wireDragStart, worldPos, m_wireCurrent, world.allocateEntityId());
    m_wireDragActive = false;
}

void Tools::onClick(const Vec2 &worldPos, World &world)
{
    // Move/Select are handled in App (press/drag/release); PlaceCurrentWire needs two
    // points, so it goes through beginWireDrag/finishWireDrag instead.
    switch (m_activeTool)
    {
    case ToolType::PlacePositiveCharge:
        world.charges().emplace_back(worldPos, m_chargeMagnitude, world.allocateEntityId());
        break;
    case ToolType::PlaceNegativeCharge:
        world.charges().emplace_back(worldPos, -m_chargeMagnitude, world.allocateEntityId());
        break;
    case ToolType::PlaceChargedParticle:
    {
        const float charge = m_particleChargePositive ? m_particleChargeMagnitude : -m_particleChargeMagnitude;
        world.particles().emplace_back(worldPos, Vec2(m_particleSpeed, 0.0f), charge, m_particleMass, world.allocateEntityId());
        break;
    }
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
