#pragma once

#include "circuits/Component.hpp"

// Open/closed switch. Closed merges posA/posB into the same electrical node (an ideal
// zero-resistance connection, like a wire); open drops it from the node/edge list for that
// solve entirely, rather than modeling it as a large-but-finite resistance.
class Switch : public Component
{
public:
    Switch(Vec2 posA, Vec2 posB, bool closed, int id) : Component(posA, posB, id), closed(closed) {}

    bool closed;

    std::string typeName() const override { return "Switch"; }
};
