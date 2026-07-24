#pragma once

#include "math/Vec2.hpp"

// A conducting loop that can translate/rotate through a magnetic field. Its rate of
// change of flux induces an EMF (Faraday's law); direction follows Lenz's law.
class MovingLoop
{
public:
    MovingLoop(Vec2 center, float radius, float area)
        : center(center), velocity(0.0f, 0.0f), radius(radius), area(area),
          rotationAngle(0.0f), angularVelocity(0.0f), lastFlux(0.0f), inducedEMF(0.0f)
    {
    }

    Vec2 center;
    Vec2 velocity;
    float radius;
    float area;
    float rotationAngle;   // radians, for the generator/motor demo
    float angularVelocity; // rad/s

    float lastFlux;
    float inducedEMF;
};
