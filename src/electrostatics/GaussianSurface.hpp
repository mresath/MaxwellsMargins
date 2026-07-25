#pragma once

#include <vector>

#include "electrostatics/PointCharge.hpp"
#include "math/Vec2.hpp"

// A user-drawn closed Gaussian surface (circle, for now) used to visualize enclosed charge
// and flux per Gauss's law: flux = Q_enclosed / epsilon_0.
class GaussianSurface
{
public:
    GaussianSurface(Vec2 center, float radius, int id) : center(center), radius(radius), id(id) {}

    Vec2 center;
    float radius;
    int id; // stable identity for selection/grab, independent of vector index

    float enclosedCharge(const std::vector<PointCharge> &charges) const;
    float flux(const std::vector<PointCharge> &charges) const;
};
