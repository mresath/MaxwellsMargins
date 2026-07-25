#include "engine/Solver.hpp"

#include <cstddef>
#include <initializer_list>
#include <utility>

namespace
{
using State = Solver::State;

// state + h * sum(coeff_i * k_i), for the stage-combination step of a Runge-Kutta method.
State combine(const State &state, double h, std::initializer_list<std::pair<double, const State *>> terms)
{
    State result = state;
    for (const auto &[coeff, k] : terms)
        for (std::size_t i = 0; i < result.size(); ++i)
            result[i] += h * coeff * (*k)[i];
    return result;
}
} // namespace

// Dormand-Prince (DOPRI5), fixed-step: always takes the 5th-order solution, skipping the
// embedded 4th-order estimate and its k7 stage since there's no adaptive step-size control.
Solver::State Solver::step(const Derivative &derivative, double t, const State &state, double dt)
{
    const State k1 = derivative(t, state);
    const State k2 = derivative(t + dt * (1.0 / 5.0), combine(state, dt, {{1.0 / 5.0, &k1}}));
    const State k3 = derivative(t + dt * (3.0 / 10.0), combine(state, dt, {{3.0 / 40.0, &k1}, {9.0 / 40.0, &k2}}));
    const State k4 = derivative(t + dt * (4.0 / 5.0), combine(state, dt, {{44.0 / 45.0, &k1}, {-56.0 / 15.0, &k2}, {32.0 / 9.0, &k3}}));
    const State k5 = derivative(t + dt * (8.0 / 9.0), combine(state, dt, {{19372.0 / 6561.0, &k1}, {-25360.0 / 2187.0, &k2}, {64448.0 / 6561.0, &k3}, {-212.0 / 729.0, &k4}}));
    const State k6 = derivative(t + dt, combine(state, dt, {{9017.0 / 3168.0, &k1}, {-355.0 / 33.0, &k2}, {46732.0 / 5247.0, &k3}, {49.0 / 176.0, &k4}, {-5103.0 / 18656.0, &k5}}));

    return combine(state, dt, {{35.0 / 384.0, &k1}, {500.0 / 1113.0, &k3}, {125.0 / 192.0, &k4}, {-2187.0 / 6784.0, &k5}, {11.0 / 84.0, &k6}});
}
