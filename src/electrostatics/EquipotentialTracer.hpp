#pragma once

#include <vector>

#include "electrostatics/PointCharge.hpp"
#include "math/Vec2.hpp"

// Traces closed equipotential contours (marching-squares style) around a set of charges
// for a given potential value.
class EquipotentialTracer
{
public:
    std::vector<std::vector<Vec2>> traceContours(const std::vector<PointCharge> &charges, float potentialValue) const;

    // TODO(Phase 2): implement contour extraction
};
