#pragma once

#include "math/Vec2.hpp"

// A point charge (positive or negative, adjustable magnitude) contributing to the
// superposed E field and potential in Fields mode via Coulomb's law.
class PointCharge
{
public:
    PointCharge(Vec2 position, float charge, int id) : position(position), charge(charge), id(id) {}

    Vec2 position;
    float charge; // Coulombs, signed
    int id;       // stable identity for selection/grab, independent of vector index
};
