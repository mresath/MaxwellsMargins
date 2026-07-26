#pragma once

#include <deque>

#include "math/Vec2.hpp"

// A conducting loop that can translate and/or rotate (about an axis in the page) through
// the magnetic field; World::update derives its induced EMF via Faraday's law.
class MovingLoop
{
public:
    MovingLoop(Vec2 center, float radius, int turns, int id)
        : center(center), velocity(0.0f, 0.0f), radius(radius), turns(turns),
          rotationAngle(0.0f), angularVelocity(0.0f), lastFlux(0.0f), inducedEMF(0.0f), id(id)
    {
    }

    float area() const { return 3.14159265358979323846f * radius * radius; }

    Vec2 center;
    Vec2 velocity;
    float radius;
    int turns;

    float rotationAngle;   // radians, 0 = loop normal fully out of page
    float angularVelocity; // rad/s

    float lastFlux; // weber, previous update's single-turn flux, for the EMF finite-difference
    float inducedEMF;
    std::deque<float> emfTrace;

    int id; // stable identity for selection/grab, independent of vector index
};
