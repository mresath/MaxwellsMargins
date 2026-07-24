#pragma once

#include "induction/MovingLoop.hpp"

// Simple generator/motor demo: a rotating loop in a uniform field, driven either by a
// fixed angular velocity (generator) or by circuit current (motor), sharing MovingLoop's
// flux/EMF bookkeeping.
class Generator
{
public:
    explicit Generator(MovingLoop loop) : loop(loop), motorMode(false) {}

    MovingLoop loop;
    bool motorMode; // false = generator (drives EMF), true = motor (driven by current)
};
