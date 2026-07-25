#pragma once

#include "Config.hpp"

#include "math/Vec2.hpp"

// Conversion functions between screen pixels and simulation meters. Fields mode has no
// ground/wall reference, so world positions are just meters relative to the view origin.

/// Length
inline float pixelsPerMeter = PIXELS_PER_METER; // Conversion factor between pixels and meters
inline float pixelsToMeters(float pixels) { return pixels / pixelsPerMeter; }
inline float metersToPixels(float meters) { return meters * pixelsPerMeter; }
inline Vec2 pixelsToMeters(const Vec2 &pixels) { return Vec2(pixelsToMeters(pixels.x), pixelsToMeters(pixels.y)); }
inline Vec2 metersToPixels(const Vec2 &meters) { return Vec2(metersToPixels(meters.x), metersToPixels(meters.y)); }

// Shortest distance from a point to a finite segment (used for wire hit-testing).
inline float distanceToSegment(const Vec2 &point, const Vec2 &segStart, const Vec2 &segEnd)
{
    const Vec2 segment = segEnd - segStart;
    const float lengthSquared = segment.lengthSquared();
    if (lengthSquared < 1e-12f)
        return (point - segStart).length();

    float t = dot(point - segStart, segment) / lengthSquared;
    t = std::fmax(0.0f, std::fmin(1.0f, t));
    return (point - (segStart + segment * t)).length();
}