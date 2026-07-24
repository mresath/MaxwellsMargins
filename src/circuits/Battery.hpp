#pragma once

#include "circuits/Component.hpp"

// EMF source with internal resistance.
class Battery : public Component
{
public:
    Battery(int nodeA, int nodeB, float emf, float internalResistance)
        : Component(nodeA, nodeB), emf(emf), internalResistance(internalResistance)
    {
    }

    float emf;               // Volts
    float internalResistance; // Ohm

    std::string typeName() const override { return "Battery"; }
};
