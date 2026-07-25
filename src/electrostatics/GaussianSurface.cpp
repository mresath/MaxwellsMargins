#include "electrostatics/GaussianSurface.hpp"

#include "Config.hpp"

float GaussianSurface::enclosedCharge(const std::vector<PointCharge> &charges) const
{
    float total = 0.0f;
    for (const auto &charge : charges)
    {
        if ((charge.position - center).length() <= radius)
            total += charge.charge;
    }
    return total;
}

float GaussianSurface::flux(const std::vector<PointCharge> &charges) const
{
    return enclosedCharge(charges) / VACUUM_PERMITTIVITY;
}
