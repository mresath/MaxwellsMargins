#include "core/Tools.hpp"

#include "Config.hpp"
#include "circuits/CircuitGraph.hpp"
#include "core/World.hpp"
#include "engine/FieldMath.hpp"
#include "induction/MovingLoop.hpp"

Tools::Tools()
    : m_activeTool(ToolType::Select), m_chargeMagnitude(DEFAULT_CHARGE_MAGNITUDE), m_gaussianRadius(GAUSSIAN_SURFACE_DEFAULT_RADIUS),
      m_particleChargeMagnitude(DEFAULT_PARTICLE_CHARGE_MAGNITUDE), m_particleChargePositive(true), m_particleMass(DEFAULT_PARTICLE_MASS), m_particleSpeed(DEFAULT_PARTICLE_SPEED),
      m_wireCurrent(DEFAULT_WIRE_CURRENT), m_wireDragActive(false), m_wireDragStart(0.0f, 0.0f),
      m_loopRadius(DEFAULT_LOOP_RADIUS), m_loopTurns(DEFAULT_LOOP_TURNS), m_loopAngularVelocity(DEFAULT_LOOP_ANGULAR_VELOCITY),
      m_currentLoopRadius(DEFAULT_CURRENT_LOOP_RADIUS), m_currentLoopCurrent(DEFAULT_CURRENT_LOOP_CURRENT), m_currentLoopTurns(DEFAULT_CURRENT_LOOP_TURNS)
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

float Tools::loopRadius() const { return m_loopRadius; }
void Tools::setLoopRadius(float radius) { m_loopRadius = radius; }
int Tools::loopTurns() const { return m_loopTurns; }
void Tools::setLoopTurns(int turns) { m_loopTurns = turns; }
float Tools::loopAngularVelocity() const { return m_loopAngularVelocity; }
void Tools::setLoopAngularVelocity(float angularVelocity) { m_loopAngularVelocity = angularVelocity; }

float Tools::currentLoopRadius() const { return m_currentLoopRadius; }
void Tools::setCurrentLoopRadius(float radius) { m_currentLoopRadius = radius; }
float Tools::currentLoopCurrent() const { return m_currentLoopCurrent; }
void Tools::setCurrentLoopCurrent(float current) { m_currentLoopCurrent = current; }
int Tools::currentLoopTurns() const { return m_currentLoopTurns; }
void Tools::setCurrentLoopTurns(int turns) { m_currentLoopTurns = turns; }

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
    case ToolType::PlaceMovingLoop:
    {
        MovingLoop loop(worldPos, m_loopRadius, m_loopTurns, world.allocateEntityId());
        loop.angularVelocity = m_loopAngularVelocity;
        // Seeds lastFlux with the flux the loop is actually sitting in, rather than 0, so
        // the first update tick doesn't register a fake EMF spike from an instantaneous jump.
        loop.lastFlux = FieldMath::loopFlux(world.magneticFieldAt(worldPos), loop.area(), loop.rotationAngle);
        world.loops().push_back(loop);
        break;
    }
    case ToolType::PlaceCurrentLoop:
        world.currentLoops().emplace_back(worldPos, m_currentLoopRadius, m_currentLoopCurrent, m_currentLoopTurns, world.allocateEntityId());
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
