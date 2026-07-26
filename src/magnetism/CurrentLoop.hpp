#pragma once

#include "math/Vec2.hpp"

// A stationary circular current loop (optionally a multi-turn coil via `turns`); produces
// a field via FieldMath::currentLoopField, peaking at its own center.
class CurrentLoop
{
public:
    CurrentLoop(Vec2 center, float radius, float current, int turns, int id)
        : center(center), radius(radius), current(current), turns(turns), id(id)
    {
    }

    Vec2 center;
    float radius;
    float current; // Amperes, signed - positive circulates counterclockwise as viewed from +z (out of page)
    int turns;
    int id; // stable identity for selection/grab, independent of vector index
};
