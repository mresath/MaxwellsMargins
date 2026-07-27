#pragma once

#include <string>

#include "math/Vec2.hpp"

// Base class for a two-terminal component on the Circuits schematic grid. posA/posB are
// grid-snapped terminal positions; nodeA/nodeB are electrical node indices, reassigned by
// CircuitGraph::solve() each step from those positions rather than persisted identity - two
// terminals landing on the same grid point are the same node regardless of which components
// they belong to.
class Component
{
public:
    Component(Vec2 posA, Vec2 posB, int id) : posA(posA), posB(posB), id(id) {}
    virtual ~Component() = default;

    Vec2 posA;
    Vec2 posB;
    int id;

    int nodeA = -1;
    int nodeB = -1;

    // Live measured/derived values, updated each solve, for the properties panel.
    float voltage = 0.0f;
    float current = 0.0f;

    // Off by default - many components' floating V/I/R/Q labels crowd/overlap on a dense
    // schematic, so each is shown individually (toggled from its own Properties panel).
    bool showLabel = false;

    virtual std::string typeName() const = 0;
};
