#pragma once

#include <vector>

#include "electrostatics/PointCharge.hpp"
#include "math/Vec2.hpp"

class EquipotentialTracer
{
public:
    // Marching squares over a grid padded around the charges. Takes every level in one call
    // (rather than one call per level) so each cell's corner potentials are sampled once and
    // reused across levels. Returns unmerged per-cell segments, not stitched polylines - fine
    // since every segment for a level is drawn together anyway.
    std::vector<std::vector<Vec2>> traceContours(const std::vector<PointCharge> &charges, const std::vector<float> &potentialValues) const;
};
