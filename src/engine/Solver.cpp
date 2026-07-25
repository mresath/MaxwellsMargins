#include "engine/Solver.hpp"

// TODO(Phase 3): implement the DOPRI5-style fixed-step update (see PLAN.md).
Solver::State Solver::step(const Derivative &derivative, double t, const State &state, double dt)
{
    (void)derivative;
    (void)t;
    (void)dt;
    return state;
}
