#include "engine/FieldMath.hpp"

#include "Config.hpp"
#include "electrostatics/PointCharge.hpp"
#include "magnetism/CurrentWire.hpp"

// TODO(Phase 2/3): implement. Signatures fixed now so electrostatics/magnetism/induction
// modules can be written against a stable interface.

namespace FieldMath
{

Vec2 coulombField(const Vec2 &point, const std::vector<PointCharge> &charges)
{
    (void)point;
    (void)charges;
    return Vec2(0.0f, 0.0f);
}

float coulombPotential(const Vec2 &point, const std::vector<PointCharge> &charges)
{
    (void)point;
    (void)charges;
    return 0.0f;
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
