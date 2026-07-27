#pragma once

#include <vector>

#include "electrostatics/GaussianSurface.hpp"
#include "electrostatics/PointCharge.hpp"
#include "induction/MovingLoop.hpp"
#include "magnetism/ChargedParticle.hpp"
#include "magnetism/CurrentLoop.hpp"
#include "magnetism/CurrentWire.hpp"
#include "magnetism/DipoleMagnet.hpp"
#include "magnetism/UniformBField.hpp"
#include "math/Vec2.hpp"

enum class EntityKind
{
    None,
    Charge,
    GaussianSurface,
    Particle,
    Wire,
    Loop,
    CurrentLoop,
    DipoleMagnet
};

struct EntityRef
{
    EntityKind kind = EntityKind::None;
    int id = -1;
};

// Container for the Fields-mode scene: point charges, magnetic field sources, moving
// charged particles/loops, and Gaussian surfaces, all on one spatial canvas so their
// fields and forces superpose.
class World
{
public:
    World();

    void reset();
    void update(float dt);
    float simTime() const;

    std::vector<PointCharge> &charges();
    std::vector<CurrentWire> &wires();
    std::vector<CurrentLoop> &currentLoops();
    std::vector<DipoleMagnet> &dipoleMagnets();
    std::vector<ChargedParticle> &particles();
    std::vector<MovingLoop> &loops();
    std::vector<GaussianSurface> &gaussianSurfaces();
    UniformBField &uniformField();

    const std::vector<PointCharge> &charges() const;
    const std::vector<CurrentWire> &wires() const;
    const std::vector<CurrentLoop> &currentLoops() const;
    const std::vector<DipoleMagnet> &dipoleMagnets() const;
    const std::vector<ChargedParticle> &particles() const;
    const std::vector<MovingLoop> &loops() const;
    const std::vector<GaussianSurface> &gaussianSurfaces() const;
    const UniformBField &uniformField() const;

    int allocateEntityId();

    // Entity id stays valid across insertions/erasures elsewhere in the world, unlike a
    // vector index - so App can hold a selection/grab across frames safely.
    EntityRef findEntityAt(const Vec2 &pos) const;
    void removeEntity(EntityKind kind, int id);
    PointCharge *findCharge(int id);
    GaussianSurface *findGaussianSurface(int id);
    ChargedParticle *findParticle(int id);
    CurrentWire *findWire(int id);
    MovingLoop *findLoop(int id);
    CurrentLoop *findCurrentLoop(int id);
    DipoleMagnet *findDipoleMagnet(int id);

    // The single global field: every source at once (charges+particles for E; uniform
    // field+wires+particles for B). excludeParticleId skips that particle's own self-field.
    Vec2 electricFieldAt(const Vec2 &point, int excludeParticleId = -1) const;
    float electricPotentialAt(const Vec2 &point, int excludeParticleId = -1) const;
    float magneticFieldAt(const Vec2 &point, int excludeParticleId = -1) const;

    // Every charge source expressed as a PointCharge (static charges plus particles), for
    // field-line/equipotential visualization, which only needs position+charge.
    std::vector<PointCharge> allChargeSources() const;

    // User-adjustable multiplier on mu_0 (see Config.hpp) - not reset by reset(), since
    // it's a physics-realism setting rather than scene content.
    float permeabilityFactor() const;
    void setPermeabilityFactor(float factor);

private:
    std::vector<PointCharge> m_charges;
    std::vector<CurrentWire> m_wires;
    std::vector<CurrentLoop> m_currentLoops;
    std::vector<DipoleMagnet> m_dipoleMagnets;
    std::vector<ChargedParticle> m_particles;
    std::vector<MovingLoop> m_loops;
    std::vector<GaussianSurface> m_gaussianSurfaces;

    UniformBField m_uniformField;

    float m_simTime;
    int m_nextEntityId;
    float m_permeabilityFactor;
};
