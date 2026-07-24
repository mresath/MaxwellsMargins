#pragma once

#include <string>

// Base class for a two-terminal circuit component placed on the schematic grid.
// Node indices refer to CircuitGraph's node list.
class Component
{
public:
    Component(int nodeA, int nodeB) : nodeA(nodeA), nodeB(nodeB) {}
    virtual ~Component() = default;

    int nodeA;
    int nodeB;

    // Live measured/derived values, updated each solve, for the properties panel.
    float voltage = 0.0f;
    float current = 0.0f;

    virtual std::string typeName() const = 0;
};
