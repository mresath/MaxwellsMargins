#pragma once

#include "Config.hpp"

#include "math/Vec2.hpp"

// Conversion functions

/// Length
inline float pixelsPerMeter = PIXELS_PER_METER; // Conversion factor between pixels and meters
inline float pixelsToMeters(float pixels) { return pixels / pixelsPerMeter; }
inline float metersToPixels(float meters) { return meters * pixelsPerMeter; }
inline Vec2 *pixelsToMeters(const Vec2 *pixels) { return new Vec2(pixelsToMeters(pixels->x), pixelsToMeters(pixels->y)); }
inline Vec2 *metersToPixels(const Vec2 *meters) { return new Vec2(metersToPixels(meters->x), metersToPixels(meters->y)); }
inline Vec2 standardizePosition(const Vec2 &pos) { return *pixelsToMeters(new Vec2(0, DEF_HEIGHT - WALL_THICKNESS)) - Vec2(-pos.x, pos.y); }
inline Vec2 destandardizePosition(const Vec2 &pos) { return Vec2(0, DEF_HEIGHT - WALL_THICKNESS) - *metersToPixels(&pos) ; }