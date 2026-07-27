#pragma once

#include "math/Vec2.hpp"

// A stationary permanent magnet whose dipole axis points through the page (matching
// CurrentLoop's model); produces a field via FieldMath::dipoleMagnetField, peaking at its
// own center.
class DipoleMagnet
{
public:
    DipoleMagnet(Vec2 center, float radius, float surfaceField, int id)
        : center(center), radius(radius), surfaceField(surfaceField), id(id)
    {
    }

    Vec2 center;
    float radius;       // meters, pole-face radius
    float surfaceField; // Tesla, signed - positive = North pole toward the viewer (field out of page)
    int id; // stable identity for selection/grab, independent of vector index
};
