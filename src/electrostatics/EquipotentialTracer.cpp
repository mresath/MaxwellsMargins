#include "electrostatics/EquipotentialTracer.hpp"

#include "Config.hpp"
#include "engine/FieldMath.hpp"

#include <algorithm>
#include <cmath>

namespace
{
Vec2 interpolate(const Vec2 &a, float va, const Vec2 &b, float vb, float value)
{
    const float t = (vb != va) ? (value - va) / (vb - va) : 0.5f;
    return Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}

// Standard marching-squares table: corners bottom-left/right/top-right/top-left, edges
// e0..e3 bottom/right/top/left. Cases 5 and 10 are the ambiguous saddles (diagonal corners
// on the same side): which of the two valid segment pairings is correct depends on whether
// the cell's actual center value sits with the high or low corners - a fixed choice here
// misdraws contours near any real saddle (e.g. directly between two like charges).
void addCaseSegments(int caseIndex, const Vec2 &e0, const Vec2 &e1, const Vec2 &e2, const Vec2 &e3,
                      bool centerAboveLevel, std::vector<std::vector<Vec2>> &segments)
{
    switch (caseIndex)
    {
    case 1:
    case 14:
        segments.push_back({e3, e0});
        break;
    case 2:
    case 13:
        segments.push_back({e0, e1});
        break;
    case 3:
    case 12:
        segments.push_back({e3, e1});
        break;
    case 4:
    case 11:
        segments.push_back({e1, e2});
        break;
    case 6:
    case 9:
        segments.push_back({e0, e2});
        break;
    case 7:
    case 8:
        segments.push_back({e2, e3});
        break;
    case 5:
        if (centerAboveLevel)
        {
            segments.push_back({e0, e1});
            segments.push_back({e2, e3});
        }
        else
        {
            segments.push_back({e3, e0});
            segments.push_back({e1, e2});
        }
        break;
    case 10:
        if (centerAboveLevel)
        {
            segments.push_back({e3, e0});
            segments.push_back({e1, e2});
        }
        else
        {
            segments.push_back({e0, e1});
            segments.push_back({e2, e3});
        }
        break;
    default:
        break;
    }
}
} // namespace

std::vector<std::vector<Vec2>> EquipotentialTracer::traceContours(const std::vector<PointCharge> &charges, const std::vector<float> &potentialValues,
                                                                    const Vec2 &viewMin, const Vec2 &viewMax) const
{
    if (charges.empty() || potentialValues.empty())
        return {};

    // The grid always spans the full viewport - not just a padded box around the charges -
    // so a contour is never truncated before it reaches the screen edge; the viewport bound
    // alone already keeps the grid finite even if charges drift far apart.
    const float minX = viewMin.x, maxX = viewMax.x;
    const float minY = viewMin.y, maxY = viewMax.y;
    if (minX >= maxX || minY >= maxY)
        return {};

    const float step = EQUIPOTENTIAL_GRID_STEP;
    const int cols = static_cast<int>(std::ceil((maxX - minX) / step));
    const int rows = static_cast<int>(std::ceil((maxY - minY) / step));

    std::vector<std::vector<Vec2>> segments;

    for (int iy = 0; iy < rows; ++iy)
    {
        for (int ix = 0; ix < cols; ++ix)
        {
            const Vec2 p0(minX + ix * step, minY + iy * step);
            const Vec2 p1(minX + (ix + 1) * step, minY + iy * step);
            const Vec2 p2(minX + (ix + 1) * step, minY + (iy + 1) * step);
            const Vec2 p3(minX + ix * step, minY + (iy + 1) * step);

            const float v0 = FieldMath::coulombPotential(p0, charges);
            const float v1 = FieldMath::coulombPotential(p1, charges);
            const float v2 = FieldMath::coulombPotential(p2, charges);
            const float v3 = FieldMath::coulombPotential(p3, charges);
            const Vec2 center((p0.x + p2.x) * 0.5f, (p0.y + p2.y) * 0.5f);
            const float vCenter = FieldMath::coulombPotential(center, charges);

            for (float level : potentialValues)
            {
                int caseIndex = 0;
                if (v0 > level)
                    caseIndex |= 1;
                if (v1 > level)
                    caseIndex |= 2;
                if (v2 > level)
                    caseIndex |= 4;
                if (v3 > level)
                    caseIndex |= 8;

                if (caseIndex == 0 || caseIndex == 15)
                    continue;

                const Vec2 e0 = interpolate(p0, v0, p1, v1, level);
                const Vec2 e1 = interpolate(p1, v1, p2, v2, level);
                const Vec2 e2 = interpolate(p2, v2, p3, v3, level);
                const Vec2 e3 = interpolate(p3, v3, p0, v0, level);

                addCaseSegments(caseIndex, e0, e1, e2, e3, vCenter > level, segments);
            }
        }
    }

    return segments;
}
