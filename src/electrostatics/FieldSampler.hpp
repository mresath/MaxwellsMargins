#pragma once

#include <vector>

#include "electrostatics/PointCharge.hpp"
#include "math/Vec2.hpp"

// Samples the superposed E field / potential on a grid for vector-arrow and field-line
// visualization, and traces continuous field lines seeded around each charge.
class FieldSampler
{
public:
    Vec2 fieldAt(const Vec2 &point, const std::vector<PointCharge> &charges) const;
    float potentialAt(const Vec2 &point, const std::vector<PointCharge> &charges) const;

    // TODO(Phase 2): traceFieldLine(seed, charges) -> std::vector<Vec2> polyline
};
