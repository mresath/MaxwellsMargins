#pragma once

#include "circuits/Component.hpp"

class Resistor : public Component
{
public:
    Resistor(int nodeA, int nodeB, float resistance) : Component(nodeA, nodeB), resistance(resistance) {}

    float resistance; // Ohm

    std::string typeName() const override { return "Resistor"; }
};
