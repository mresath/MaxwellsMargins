#include "core/World.hpp"

World::World() : m_simTime(0.0f)
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
