#include "core/World.hpp"

#include "Config.hpp"

#include <cstddef>

World::World() : m_simTime(0.0f), m_nextEntityId(0)
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
    // TODO(Phase 2/3/4): apply Coulomb/Lorentz forces to particles via engine/Solver,
    // update loop flux/EMF, advance m_simTime.
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
