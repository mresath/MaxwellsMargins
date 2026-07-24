#include "electrostatics/GaussianSurface.hpp"

// TODO(Phase 2): implement (sum charge magnitudes within `radius` of `center`; flux from Gauss's law)

float GaussianSurface::enclosedCharge(const std::vector<PointCharge> &charges) const
{
    (void)charges;
    return 0.0f;
}

float GaussianSurface::flux(const std::vector<PointCharge> &charges) const
{
    (void)charges;
    return 0.0f;
}
