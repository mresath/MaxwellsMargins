#pragma once

#include <functional>
#include <vector>

// Single shared ODE solver (Dormand-Prince / DOPRI5-style, fixed-step) used across
// domains: charged-particle motion, RC/RL transients, and the rotating-loop generator.
// State is a flat vector of doubles; derivative(t, state) returns d(state)/dt.
class Solver
{
public:
    using State = std::vector<double>;
    using Derivative = std::function<State(double t, const State &state)>;

    static State step(const Derivative &derivative, double t, const State &state, double dt);
};
