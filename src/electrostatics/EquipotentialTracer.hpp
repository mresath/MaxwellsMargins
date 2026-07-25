#pragma once

#include <vector>

#include "electrostatics/PointCharge.hpp"
#include "math/Vec2.hpp"

class EquipotentialTracer
{
public:
    // Marching squares over a grid padded around the charges, clamped to viewMin/viewMax
    // since drifting particles could otherwise grow it unboundedly off-screen.
    std::vector<std::vector<Vec2>> traceContours(const std::vector<PointCharge> &charges, const std::vector<float> &potentialValues,
                                                  const Vec2 &viewMin, const Vec2 &viewMax) const;
};
