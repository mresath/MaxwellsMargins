#include "electrostatics/FieldSampler.hpp"

#include "engine/FieldMath.hpp"

Vec2 FieldSampler::fieldAt(const Vec2 &point, const std::vector<PointCharge> &charges) const
{
    return FieldMath::coulombField(point, charges);
}

float FieldSampler::potentialAt(const Vec2 &point, const std::vector<PointCharge> &charges) const
{
    return FieldMath::coulombPotential(point, charges);
}
