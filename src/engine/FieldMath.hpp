#pragma once

#include <vector>

#include "math/Vec2.hpp"

class PointCharge;
class CurrentWire;
class CurrentLoop;

// Shared field/force math: Coulomb's law superposition, Biot-Savart/Ampere for wires, and
// the Lorentz force. Used by electrostatics, magnetism, and induction modules alike.
namespace FieldMath
{
// Single-source E field/potential; also used directly by World for charged particles,
// which aren't stored in a PointCharge list.
Vec2 pointChargeField(const Vec2 &point, const Vec2 &sourcePosition, float sourceCharge);
float pointChargePotential(const Vec2 &point, const Vec2 &sourcePosition, float sourceCharge);

Vec2 coulombField(const Vec2 &point, const std::vector<PointCharge> &charges);
float coulombPotential(const Vec2 &point, const std::vector<PointCharge> &charges);
float coulombForceMagnitude(float q1, float q2, float separation);

// A wire's field at any in-plane point is purely perpendicular to the plane (right-hand
// rule) - a signed scalar, matching UniformBField, not an in-plane Vec2.
float biotSavartField(const Vec2 &point, const std::vector<CurrentWire> &wires, float permeabilityFactor);
float forceBetweenWires(float current1, float current2, float separation, float length, float permeabilityFactor);

// On-axis circular-loop formula, reused radially instead of the true off-axis field (which
// needs elliptic integrals) - exact at the loop's own center, an approximation elsewhere.
float currentLoopField(const Vec2 &point, const std::vector<CurrentLoop> &loops, float permeabilityFactor);

// B field of a single moving point charge (point-charge Biot-Savart: mu0/4*pi * q * (v x
// r_hat) / r^2) - signed scalar for the same reason as biotSavartField.
float movingChargeField(const Vec2 &point, const Vec2 &sourcePosition, const Vec2 &sourceVelocity, float sourceCharge, float permeabilityFactor);

Vec2 lorentzForce(float charge, const Vec2 &velocity, const Vec2 &electricField, float bFieldStrength);

// Flux through a loop small enough that the field is ~uniform across it, sampled at its
// center; rotationAngle is the angle between the loop's normal and the out-of-page axis.
float loopFlux(float bFieldAtCenter, float loopArea, float rotationAngle);
} // namespace FieldMath
