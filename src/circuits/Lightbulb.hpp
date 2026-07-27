#pragma once

#include "circuits/Resistor.hpp"

// Electrically a plain fixed-resistance Resistor (a simplified filament model, not a
// physically accurate nonlinear one) - CircuitGraph::solve() treats it identically via
// dynamic_cast<Resistor*>. Only the schematic symbol (a glowing bulb, not a zigzag) and
// typeName() differ.
class Lightbulb : public Resistor
{
public:
    Lightbulb(Vec2 posA, Vec2 posB, float resistance, int id) : Resistor(posA, posB, resistance, id) {}

    float power() const { return voltage * current; }

    std::string typeName() const override { return "Lightbulb"; }
};
