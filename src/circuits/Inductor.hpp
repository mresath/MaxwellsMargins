#pragma once

#include "circuits/Component.hpp"

// A lumped RL element (V = L*dI/dt) - unlike induction/MovingLoop or magnetism/CurrentLoop,
// it has no spatial extent and neither generates nor senses a Fields-mode magnetic field;
// its physics is entirely local to CircuitGraph's solve.
class Inductor : public Component
{
public:
    Inductor(Vec2 posA, Vec2 posB, float inductance, int id) : Component(posA, posB, id), inductance(inductance) {}

    float inductance;            // Henry
    float storedCurrent = 0.0f; // Amperes, current from the previous solved step (see CircuitGraph.cpp)

    std::string typeName() const override { return "Inductor"; }
};
