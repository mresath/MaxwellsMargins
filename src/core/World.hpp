#pragma once

#include <vector>

#include "electrostatics/GaussianSurface.hpp"
#include "electrostatics/PointCharge.hpp"
#include "induction/MovingLoop.hpp"
#include "magnetism/ChargedParticle.hpp"
#include "magnetism/CurrentWire.hpp"
#include "magnetism/UniformBField.hpp"
#include "math/Vec2.hpp"

enum class EntityKind
{
    None,
    Charge,
    GaussianSurface
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

    std::vector<PointCharge> &charges();
    std::vector<CurrentWire> &wires();
    std::vector<ChargedParticle> &particles();
    std::vector<MovingLoop> &loops();
    std::vector<GaussianSurface> &gaussianSurfaces();
    UniformBField &uniformField();

    const std::vector<PointCharge> &charges() const;
    const std::vector<CurrentWire> &wires() const;
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

private:
    std::vector<PointCharge> m_charges;
    std::vector<CurrentWire> m_wires;
    std::vector<ChargedParticle> m_particles;
    std::vector<MovingLoop> m_loops;
    std::vector<GaussianSurface> m_gaussianSurfaces;

    UniformBField m_uniformField;

    float m_simTime;
    int m_nextEntityId;
};
