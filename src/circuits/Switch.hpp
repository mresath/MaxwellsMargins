#pragma once

#include "circuits/Component.hpp"

// Open/closed switch; when open, acts as an effectively-infinite resistance so it drops
// out of the Kirchhoff solve rather than needing special-cased topology.
class Switch : public Component
{
public:
    Switch(int nodeA, int nodeB, bool closed) : Component(nodeA, nodeB), closed(closed) {}

    bool closed;

    std::string typeName() const override { return "Switch"; }
};
