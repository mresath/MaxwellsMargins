#pragma once

#include "math/Vec2.hpp"

// A current-carrying wire segment; produces a field via the right-hand rule (Biot-Savart)
// and feels a force from other wires/fields (F = IL x B).
class CurrentWire
{
public:
    CurrentWire(Vec2 start, Vec2 end, float current, int id) : start(start), end(end), current(current), id(id) {}

    Vec2 start;
    Vec2 end;
    float current; // Amperes, signed for direction along start->end
    int id;         // stable identity for selection/grab, independent of vector index
};
