#pragma once

#include <deque>

#include "math/Vec2.hpp"

// A moving point charge subject to the Lorentz force (F = qE + qv x B), producing
// circular/helical motion in a uniform B field. Integrated by engine/Solver.
class ChargedParticle
{
public:
    ChargedParticle(Vec2 position, Vec2 velocity, float charge, float mass)
        : position(position), velocity(velocity), charge(charge), mass(mass),
          trajectoryTraceEnabled(true)
    {
    }

    Vec2 position;
    Vec2 velocity;
    float charge; // Coulombs
    float mass;   // kg

    bool trajectoryTraceEnabled;
    std::deque<Vec2> trajectoryTrace;
};
