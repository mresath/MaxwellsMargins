#pragma once

#include "math/Vec2.hpp"

// An ideal (zero-resistance) connection between two grid points. CircuitGraph::solve()
// merges both endpoints into the same electrical node rather than solving it as a branch;
// `current` is a post-solve visualization-only readout (see CircuitGraph.cpp).
class CircuitWire
{
public:
    CircuitWire(Vec2 start, Vec2 end, int id) : start(start), end(end), id(id) {}

    Vec2 start;
    Vec2 end;
    int id;
    float current = 0.0f;
};
