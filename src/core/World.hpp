#pragma once

#include <vector>

#include "electrostatics/GaussianSurface.hpp"
#include "electrostatics/PointCharge.hpp"
#include "induction/MovingLoop.hpp"
#include "magnetism/ChargedParticle.hpp"
#include "magnetism/CurrentWire.hpp"
#include "magnetism/UniformBField.hpp"

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

private:
    std::vector<PointCharge> m_charges;
    std::vector<CurrentWire> m_wires;
    std::vector<ChargedParticle> m_particles;
    std::vector<MovingLoop> m_loops;
    std::vector<GaussianSurface> m_gaussianSurfaces;

    UniformBField m_uniformField;

    float m_simTime;
};
