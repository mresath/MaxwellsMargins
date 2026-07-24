#include "engine/Solver.hpp"

// TODO(Phase 1): implement the DOPRI5 (or RK4, per PLAN.md decision) fixed-step update,
// ported from NewtonsNotepad/src/engine/ODE.cpp's DOPRI5 solver.
Solver::State Solver::step(const Derivative &derivative, double t, const State &state, double dt)
{
    (void)derivative;
    (void)t;
    (void)dt;
    return state;
}
