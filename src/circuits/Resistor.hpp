#pragma once

#include "circuits/Component.hpp"

class Resistor : public Component
{
public:
    Resistor(Vec2 posA, Vec2 posB, float resistance, int id) : Component(posA, posB, id), resistance(resistance) {}

    float resistance; // Ohm

    std::string typeName() const override { return "Resistor"; }
};
