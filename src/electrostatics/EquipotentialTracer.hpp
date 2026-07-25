#pragma once

#include <vector>

#include "electrostatics/PointCharge.hpp"
#include "math/Vec2.hpp"

class EquipotentialTracer
{
public:
    // Marching squares over a grid spanning the full viewMin/viewMax viewport, so contours
    // aren't truncated before the screen edge and the grid stays bounded regardless of how
    // far apart charges (which may be drifting particles) are.
    std::vector<std::vector<Vec2>> traceContours(const std::vector<PointCharge> &charges, const std::vector<float> &potentialValues,
                                                  const Vec2 &viewMin, const Vec2 &viewMax) const;
};
