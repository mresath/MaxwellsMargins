#include "engine/FieldMath.hpp"

#include "Config.hpp"
#include "electrostatics/PointCharge.hpp"
#include "magnetism/CurrentLoop.hpp"
#include "magnetism/CurrentWire.hpp"

#include <cmath>

namespace
{
// Clamps the physically-singular blowup directly on top of a point charge or wire.
constexpr float kMinSeparation = 0.05f; // meters
constexpr float kPi = 3.14159265358979323846f;

// Closed-form Biot-Savart field of a finite straight segment (integrated dB = (mu0 I /
// 4*pi) (dl x r_hat)/r^2 along the wire); reduces to B = mu0*I/(2*pi*d) as it lengthens.
float singleWireField(const Vec2 &point, const CurrentWire &wire, float permeabilityFactor)
{
    const Vec2 segment = wire.end - wire.start;
    const float length = segment.length();
    if (length < 1e-6f)
        return 0.0f;
    const Vec2 direction = segment / length;

    const Vec2 fromStart = point - wire.start;
    float perp = cross(direction, fromStart);
    if (std::abs(perp) < kMinSeparation)
        perp = perp < 0.0f ? -kMinSeparation : kMinSeparation;

    const float s1 = dot(fromStart, direction);
    const float s2 = s1 - length;

    return (VACUUM_PERMEABILITY * permeabilityFactor * wire.current / (4.0f * kPi * perp)) *
           (s1 / std::sqrt(perp * perp + s1 * s1) - s2 / std::sqrt(perp * perp + s2 * s2));
}

// On-axis field of a circular loop, evaluated at planar distance d from its center rather
// than true axial distance (see FieldMath.hpp) - exact at d=0, an approximation elsewhere.
float singleCurrentLoopField(const Vec2 &point, const CurrentLoop &loop, float permeabilityFactor)
{
    const float d = (point - loop.center).length();
    const float denom = std::pow(loop.radius * loop.radius + d * d, 1.5f);
    return VACUUM_PERMEABILITY * permeabilityFactor * static_cast<float>(loop.turns) * loop.current * loop.radius * loop.radius / (2.0f * denom);
}
} // namespace

namespace FieldMath
{

Vec2 pointChargeField(const Vec2 &point, const Vec2 &sourcePosition, float sourceCharge)
{
    const Vec2 offset = point - sourcePosition;
    float distance = offset.length();
    if (distance < kMinSeparation)
        distance = kMinSeparation;

    const Vec2 direction = offset / distance;
    return direction * (COULOMB_CONSTANT * sourceCharge / (distance * distance));
}

float pointChargePotential(const Vec2 &point, const Vec2 &sourcePosition, float sourceCharge)
{
    float distance = (point - sourcePosition).length();
    if (distance < kMinSeparation)
        distance = kMinSeparation;

    return COULOMB_CONSTANT * sourceCharge / distance;
}

Vec2 coulombField(const Vec2 &point, const std::vector<PointCharge> &charges)
{
    Vec2 total(0.0f, 0.0f);
    for (const auto &charge : charges)
        total += pointChargeField(point, charge.position, charge.charge);
    return total;
}

float coulombPotential(const Vec2 &point, const std::vector<PointCharge> &charges)
{
    float total = 0.0f;
    for (const auto &charge : charges)
        total += pointChargePotential(point, charge.position, charge.charge);
    return total;
}

float coulombForceMagnitude(float q1, float q2, float separation)
{
    return COULOMB_CONSTANT * q1 * q2 / (separation * separation);
}

float biotSavartField(const Vec2 &point, const std::vector<CurrentWire> &wires, float permeabilityFactor)
{
    float total = 0.0f;
    for (const auto &wire : wires)
        total += singleWireField(point, wire, permeabilityFactor);
    return total;
}

float currentLoopField(const Vec2 &point, const std::vector<CurrentLoop> &loops, float permeabilityFactor)
{
    float total = 0.0f;
    for (const auto &loop : loops)
        total += singleCurrentLoopField(point, loop, permeabilityFactor);
    return total;
}

float forceBetweenWires(float current1, float current2, float separation, float length, float permeabilityFactor)
{
    float d = separation;
    if (d < kMinSeparation)
        d = kMinSeparation;

    // Sign convention: positive = attractive, negative = repulsive (sign of current1*current2).
    return VACUUM_PERMEABILITY * permeabilityFactor * current1 * current2 * length / (2.0f * kPi * d);
}

float movingChargeField(const Vec2 &point, const Vec2 &sourcePosition, const Vec2 &sourceVelocity, float sourceCharge, float permeabilityFactor)
{
    const Vec2 offset = point - sourcePosition;
    float distance = offset.length();
    if (distance < kMinSeparation)
        distance = kMinSeparation;

    return (VACUUM_PERMEABILITY * permeabilityFactor / (4.0f * kPi)) * sourceCharge * cross(sourceVelocity, offset) / (distance * distance * distance);
}

Vec2 lorentzForce(float charge, const Vec2 &velocity, const Vec2 &electricField, float bFieldStrength)
{
    // v x B with B purely along the out-of-page axis: (vx,vy,0) x (0,0,Bz) = (vy*Bz, -vx*Bz, 0)
    const Vec2 magneticComponent(velocity.y * bFieldStrength, -velocity.x * bFieldStrength);
    return (electricField + magneticComponent) * charge;
}

float loopFlux(float bFieldAtCenter, float loopArea, float rotationAngle)
{
    return bFieldAtCenter * loopArea * std::cos(rotationAngle);
}

} // namespace FieldMath
