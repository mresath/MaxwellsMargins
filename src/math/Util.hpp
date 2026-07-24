#pragma once

#include "Config.hpp"

#include "math/Vec2.hpp"

// Conversion functions between screen pixels and simulation meters. Unlike Newton's
// Notepad, Fields mode has no ground/wall reference, so there is no position
// standardization step here - world positions are meters relative to the view origin.

/// Length
inline float pixelsPerMeter = PIXELS_PER_METER; // Conversion factor between pixels and meters
inline float pixelsToMeters(float pixels) { return pixels / pixelsPerMeter; }
inline float metersToPixels(float meters) { return meters * pixelsPerMeter; }
inline Vec2 pixelsToMeters(const Vec2 &pixels) { return Vec2(pixelsToMeters(pixels.x), pixelsToMeters(pixels.y)); }
inline Vec2 metersToPixels(const Vec2 &meters) { return Vec2(metersToPixels(meters.x), metersToPixels(meters.y)); }