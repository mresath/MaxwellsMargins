#include "electrostatics/FieldSampler.hpp"

#include "Config.hpp"
#include "engine/FieldMath.hpp"

Vec2 FieldSampler::fieldAt(const Vec2 &point, const std::vector<PointCharge> &charges) const
{
    return FieldMath::coulombField(point, charges);
}

float FieldSampler::potentialAt(const Vec2 &point, const std::vector<PointCharge> &charges) const
{
    return FieldMath::coulombPotential(point, charges);
}

namespace
{
// Zero vector where the field vanishes, so the caller can stop tracing.
Vec2 travelDirection(const Vec2 &pos, const std::vector<PointCharge> &charges, bool followField)
{
    const Vec2 field = FieldMath::coulombField(pos, charges);
    const float magnitude = field.length();
    if (magnitude < 1e-9f)
        return Vec2(0.0f, 0.0f);

    const Vec2 direction = field / magnitude;
    return followField ? direction : direction * -1.0f;
}
} // namespace

std::vector<Vec2> FieldSampler::traceFieldLine(const Vec2 &seed, const std::vector<PointCharge> &charges, bool followField) const
{
    std::vector<Vec2> polyline;
    polyline.push_back(seed);

    Vec2 pos = seed;
    for (int step = 0; step < FIELD_LINE_MAX_STEPS; ++step)
    {
        // Midpoint (RK2) step - forward Euler visibly facets the curve near a charge, where
        // the field direction turns sharply over a short distance.
        const Vec2 k1 = travelDirection(pos, charges, followField);
        if (k1.lengthSquared() < 1e-18f)
            break;

        const Vec2 midpoint = pos + k1 * (FIELD_LINE_STEP * 0.5f);
        Vec2 k2 = travelDirection(midpoint, charges, followField);
        if (k2.lengthSquared() < 1e-18f)
            k2 = k1;

        pos += k2 * FIELD_LINE_STEP;
        polyline.push_back(pos);

        bool captured = false;
        for (const auto &charge : charges)
        {
            if ((pos - charge.position).length() < FIELD_LINE_CAPTURE_RADIUS)
            {
                captured = true;
                break;
            }
        }
        if (captured || pos.length() > FIELD_LINE_MAX_RADIUS)
            break;
    }

    return polyline;
}
