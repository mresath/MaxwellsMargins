#pragma once

#include "math/Vec2.hpp"

// A point charge (positive or negative, adjustable magnitude) contributing to the
// superposed E field and potential in Fields mode via Coulomb's law.
class PointCharge
{
public:
    PointCharge(Vec2 position, float charge) : position(position), charge(charge) {}

    Vec2 position;
    float charge; // Coulombs, signed

    // TODO(Phase 2): electricFieldAt(point), potentialAt(point) using engine/FieldMath
};
