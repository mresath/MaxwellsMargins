#pragma once

#include <vector>

#include "electrostatics/PointCharge.hpp"
#include "math/Vec2.hpp"

class FieldSampler
{
public:
    Vec2 fieldAt(const Vec2 &point, const std::vector<PointCharge> &charges) const;
    float potentialAt(const Vec2 &point, const std::vector<PointCharge> &charges) const;

    // followField=true marches along E (seed near a positive charge, lines flow outward);
    // false marches against E (seed near a negative charge, lines flow inward).
    std::vector<Vec2> traceFieldLine(const Vec2 &seed, const std::vector<PointCharge> &charges, bool followField = true) const;
};
