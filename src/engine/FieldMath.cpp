#include "engine/FieldMath.hpp"

#include "Config.hpp"
#include "electrostatics/PointCharge.hpp"
#include "magnetism/CurrentWire.hpp"

// TODO(Phase 3): implement biotSavartField/forceBetweenWires/lorentzForce.

namespace
{
// Clamped minimum separation avoids the field/potential blowing up to infinity directly
// on top of a point charge - both physically singular and numerically useless to render.
constexpr float kMinSeparation = 0.05f; // meters
} // namespace

namespace FieldMath
{

Vec2 coulombField(const Vec2 &point, const std::vector<PointCharge> &charges)
{
    Vec2 total(0.0f, 0.0f);
    for (const auto &charge : charges)
    {
        const Vec2 offset = point - charge.position;
        float distance = offset.length();
        if (distance < kMinSeparation)
            distance = kMinSeparation;

        const Vec2 direction = offset / distance;
        total += direction * (COULOMB_CONSTANT * charge.charge / (distance * distance));
    }
    return total;
}

float coulombPotential(const Vec2 &point, const std::vector<PointCharge> &charges)
{
    float total = 0.0f;
    for (const auto &charge : charges)
    {
        float distance = (point - charge.position).length();
        if (distance < kMinSeparation)
            distance = kMinSeparation;

        total += COULOMB_CONSTANT * charge.charge / distance;
    }
    return total;
}

float coulombForceMagnitude(float q1, float q2, float separation)
{
    return COULOMB_CONSTANT * q1 * q2 / (separation * separation);
}

Vec2 biotSavartField(const Vec2 &point, const CurrentWire &wire)
{
    (void)point;
    (void)wire;
    return Vec2(0.0f, 0.0f);
}

float forceBetweenWires(float current1, float current2, float separation, float length)
{
    (void)current1;
    (void)current2;
    (void)separation;
    (void)length;
    return 0.0f;
}

Vec2 lorentzForce(float charge, const Vec2 &velocity, const Vec2 &electricField, float bFieldStrength)
{
    (void)charge;
    (void)velocity;
    (void)electricField;
    (void)bFieldStrength;
    return Vec2(0.0f, 0.0f);
}

} // namespace FieldMath
