#pragma once

#include "circuits/Component.hpp"

// Supports series/parallel wiring and charge/discharge transients (integrated via
// engine/Solver as an RC circuit).
class Capacitor : public Component
{
public:
    Capacitor(int nodeA, int nodeB, float capacitance) : Component(nodeA, nodeB), capacitance(capacitance) {}

    float capacitance; // Farad
    float charge = 0.0f; // Coulombs, current stored charge

    std::string typeName() const override { return "Capacitor"; }
};
