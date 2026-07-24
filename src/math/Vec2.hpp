#pragma once

#include <cmath>
#include <string>
#include <fmt/format.h>

struct Vec2
{
    // 2D vector components
    float x = 0.0f;
    float y = 0.0f;

    // Constructors
    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}

    // Operators
    Vec2 operator+(const Vec2 &rhs) const
    {
        return Vec2(x + rhs.x, y + rhs.y);
    }

    Vec2 operator-(const Vec2 &rhs) const
    {
        return Vec2(x - rhs.x, y - rhs.y);
    }

    Vec2 operator*(float scalar) const
    {
        return Vec2(x * scalar, y * scalar);
    }

    Vec2 operator/(float scalar) const
    {
        return Vec2(x / scalar, y / scalar);
    }

    void operator+=(const Vec2 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
    }

    void operator-=(const Vec2 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
    }

    void operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
    }

    void operator/=(float scalar)
    {
        x /= scalar;
        y /= scalar;
    }

    // Methods
    void set(float newX, float newY)
    {
        x = newX;
        y = newY;
    }

    float lengthSquared() const
    {
        return x * x + y * y;
    }

    float length() const
    {
        return std::sqrt(lengthSquared());
    }

    float angle() const
    {
        return std::atan2(y, x);
    }

    void rotate(float angleRad)
    {
        float cosA = std::cos(angleRad);
        float sinA = std::sin(angleRad);
        float newX = x * cosA - y * sinA;
        float newY = x * sinA + y * cosA;
        x = newX;
        y = newY;
    }

    Vec2 rotated(float angleRad) const
    {
        float cosA = std::cos(angleRad);
        float sinA = std::sin(angleRad);
        return Vec2(x * cosA - y * sinA, x * sinA + y * cosA);
    }

    Vec2 normalized() const
    {
        float len = length();
        if (len > 0.0f)
            return Vec2(x / len, y / len);
        return Vec2(0, 0);
    }

    Vec2 perpendicular() const
    {
        return Vec2(-y, x);
    }

    Vec2 constrained(Vec2 min, Vec2 max) const
    {
        return Vec2(std::fmax(min.x, std::fmin(x, max.x)),
                    std::fmax(min.y, std::fmin(y, max.y)));
    }

    void constrain(Vec2 min, Vec2 max)
    {
        x = std::fmax(min.x, std::fmin(x, max.x));
        y = std::fmax(min.y, std::fmin(y, max.y));
    }

    // Conversions
    std::string toString() const
    {
        return fmt::format("({:.2f}, {:.2f})", x, y);
    }
};

// Dot product of two vectors
inline float dot(const Vec2 &a, const Vec2 &b)
{
    return a.x * b.x + a.y * b.y;
}

// 2D cross product (returns scalar magnitude of z-component)
inline float cross(const Vec2 &a, const Vec2 &b)
{
    return a.x * b.y - a.y * b.x;
}