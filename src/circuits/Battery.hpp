#pragma once

#include "circuits/Component.hpp"

// EMF source with internal resistance. By convention posA is the positive terminal:
// current is solved flowing from posA to posB through the source's internal branch.
class Battery : public Component
{
public:
    Battery(Vec2 posA, Vec2 posB, float emf, float internalResistance, int id)
        : Component(posA, posB, id), emf(emf), internalResistance(internalResistance)
    {
    }

    float emf;                // Volts
    float internalResistance; // Ohm

    std::string typeName() const override { return "Battery"; }
};
