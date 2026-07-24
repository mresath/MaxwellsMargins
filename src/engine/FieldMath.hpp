#pragma once

#include <vector>

#include "math/Vec2.hpp"

class PointCharge;
class CurrentWire;

// Shared field/force math: Coulomb's law superposition, Biot-Savart/Ampere for wires, and
// the Lorentz force. Used by electrostatics, magnetism, and induction modules alike.
namespace FieldMath
{
Vec2 coulombField(const Vec2 &point, const std::vector<PointCharge> &charges);
float coulombPotential(const Vec2 &point, const std::vector<PointCharge> &charges);
float coulombForceMagnitude(float q1, float q2, float separation);

Vec2 biotSavartField(const Vec2 &point, const CurrentWire &wire);
float forceBetweenWires(float current1, float current2, float separation, float length);

Vec2 lorentzForce(float charge, const Vec2 &velocity, const Vec2 &electricField, float bFieldStrength);
} // namespace FieldMath
