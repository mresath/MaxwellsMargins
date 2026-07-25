#include "core/World.hpp"

#include "Config.hpp"
#include "engine/FieldMath.hpp"
#include "engine/Solver.hpp"
#include "math/Util.hpp"

#include <cstddef>

World::World() : m_simTime(0.0f), m_nextEntityId(0), m_permeabilityFactor(DEFAULT_PERMEABILITY_FACTOR)
{
}

void World::reset()
{
    m_charges.clear();
    m_wires.clear();
    m_particles.clear();
    m_loops.clear();
    m_gaussianSurfaces.clear();
    m_uniformField = UniformBField();
    m_simTime = 0.0f;
    m_nextEntityId = 0;
}

void World::update(float dt)
{
    // TODO(Phase 4): update loop flux/EMF.
    for (auto &particle : m_particles)
    {
        // Fields are re-sampled at each RK stage's position, not frozen at the step's start.
        const auto derivative = [this, &particle](double, const Solver::State &s) -> Solver::State
        {
            const Vec2 position(static_cast<float>(s[0]), static_cast<float>(s[1]));
            const Vec2 velocity(static_cast<float>(s[2]), static_cast<float>(s[3]));

            const Vec2 electricField = electricFieldAt(position, particle.id);
            const float magneticField = magneticFieldAt(position, particle.id);
            const Vec2 acceleration = FieldMath::lorentzForce(particle.charge, velocity, electricField, magneticField) / particle.mass;

            return {s[2], s[3], acceleration.x, acceleration.y};
        };

        const Solver::State state = {particle.position.x, particle.position.y, particle.velocity.x, particle.velocity.y};
        const Solver::State next = Solver::step(derivative, m_simTime, state, dt);

        particle.position = Vec2(static_cast<float>(next[0]), static_cast<float>(next[1]));
        particle.velocity = Vec2(static_cast<float>(next[2]), static_cast<float>(next[3]));

        if (particle.trajectoryTraceEnabled)
        {
            particle.trajectoryTrace.push_back(particle.position);
            if (particle.trajectoryTrace.size() > TRAJECTORY_TRACE_MAX_POINTS)
                particle.trajectoryTrace.pop_front();
        }
    }

    m_simTime += dt;
}

std::vector<PointCharge> &World::charges() { return m_charges; }
std::vector<CurrentWire> &World::wires() { return m_wires; }
std::vector<ChargedParticle> &World::particles() { return m_particles; }
std::vector<MovingLoop> &World::loops() { return m_loops; }
std::vector<GaussianSurface> &World::gaussianSurfaces() { return m_gaussianSurfaces; }
UniformBField &World::uniformField() { return m_uniformField; }

const std::vector<PointCharge> &World::charges() const { return m_charges; }
const std::vector<CurrentWire> &World::wires() const { return m_wires; }
const std::vector<ChargedParticle> &World::particles() const { return m_particles; }
const std::vector<MovingLoop> &World::loops() const { return m_loops; }
const std::vector<GaussianSurface> &World::gaussianSurfaces() const { return m_gaussianSurfaces; }
const UniformBField &World::uniformField() const { return m_uniformField; }

int World::allocateEntityId() { return m_nextEntityId++; }

EntityRef World::findEntityAt(const Vec2 &pos) const
{
    for (const auto &charge : m_charges)
        if ((charge.position - pos).length() < ENTITY_HIT_RADIUS)
            return {EntityKind::Charge, charge.id};

    for (const auto &particle : m_particles)
        if ((particle.position - pos).length() < ENTITY_HIT_RADIUS)
            return {EntityKind::Particle, particle.id};

    for (const auto &wire : m_wires)
        if (distanceToSegment(pos, wire.start, wire.end) < ENTITY_HIT_RADIUS)
            return {EntityKind::Wire, wire.id};

    for (const auto &surface : m_gaussianSurfaces)
        if ((surface.center - pos).length() <= surface.radius)
            return {EntityKind::GaussianSurface, surface.id};

    return {};
}

void World::removeEntity(EntityKind kind, int id)
{
    if (kind == EntityKind::Charge)
    {
        for (std::size_t i = 0; i < m_charges.size(); ++i)
        {
            if (m_charges[i].id == id)
            {
                m_charges.erase(m_charges.begin() + i);
                return;
            }
        }
    }
    else if (kind == EntityKind::GaussianSurface)
    {
        for (std::size_t i = 0; i < m_gaussianSurfaces.size(); ++i)
        {
            if (m_gaussianSurfaces[i].id == id)
            {
                m_gaussianSurfaces.erase(m_gaussianSurfaces.begin() + i);
                return;
            }
        }
    }
    else if (kind == EntityKind::Particle)
    {
        for (std::size_t i = 0; i < m_particles.size(); ++i)
        {
            if (m_particles[i].id == id)
            {
                m_particles.erase(m_particles.begin() + i);
                return;
            }
        }
    }
    else if (kind == EntityKind::Wire)
    {
        for (std::size_t i = 0; i < m_wires.size(); ++i)
        {
            if (m_wires[i].id == id)
            {
                m_wires.erase(m_wires.begin() + i);
                return;
            }
        }
    }
}

PointCharge *World::findCharge(int id)
{
    for (auto &charge : m_charges)
        if (charge.id == id)
            return &charge;
    return nullptr;
}

GaussianSurface *World::findGaussianSurface(int id)
{
    for (auto &surface : m_gaussianSurfaces)
        if (surface.id == id)
            return &surface;
    return nullptr;
}

ChargedParticle *World::findParticle(int id)
{
    for (auto &particle : m_particles)
        if (particle.id == id)
            return &particle;
    return nullptr;
}

CurrentWire *World::findWire(int id)
{
    for (auto &wire : m_wires)
        if (wire.id == id)
            return &wire;
    return nullptr;
}

Vec2 World::electricFieldAt(const Vec2 &point, int excludeParticleId) const
{
    Vec2 total = FieldMath::coulombField(point, m_charges);
    for (const auto &particle : m_particles)
        if (particle.id != excludeParticleId)
            total += FieldMath::pointChargeField(point, particle.position, particle.charge);
    return total;
}

float World::electricPotentialAt(const Vec2 &point, int excludeParticleId) const
{
    float total = FieldMath::coulombPotential(point, m_charges);
    for (const auto &particle : m_particles)
        if (particle.id != excludeParticleId)
            total += FieldMath::pointChargePotential(point, particle.position, particle.charge);
    return total;
}

float World::magneticFieldAt(const Vec2 &point, int excludeParticleId) const
{
    float total = m_uniformField.enabled ? m_uniformField.strength : 0.0f;
    total += FieldMath::biotSavartField(point, m_wires, m_permeabilityFactor);
    for (const auto &particle : m_particles)
        if (particle.id != excludeParticleId)
            total += FieldMath::movingChargeField(point, particle.position, particle.velocity, particle.charge, m_permeabilityFactor);
    return total;
}

std::vector<PointCharge> World::allChargeSources() const
{
    std::vector<PointCharge> sources = m_charges;
    for (const auto &particle : m_particles)
        sources.emplace_back(particle.position, particle.charge, -1);
    return sources;
}

float World::permeabilityFactor() const { return m_permeabilityFactor; }
void World::setPermeabilityFactor(float factor) { m_permeabilityFactor = factor; }
