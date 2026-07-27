#pragma once

#include "circuits/Component.hpp"

// Supports series/parallel wiring and charge/discharge transients (integrated via
// engine/Solver as an RC circuit).
class Capacitor : public Component
{
public:
    Capacitor(Vec2 posA, Vec2 posB, float capacitance, int id) : Component(posA, posB, id), capacitance(capacitance) {}

    float capacitance;   // Farad
    float charge = 0.0f; // Coulombs, current stored charge

    std::string typeName() const override { return "Capacitor"; }
};
