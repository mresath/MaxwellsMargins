#pragma once

#include <memory>
#include <vector>

#include "circuits/Component.hpp"

// Container for the Circuits-mode scene: a node/component graph solved each step via
// modified nodal analysis (Kirchhoff's current/voltage laws), with RC/RL transients
// integrated over time using engine/Solver.
class CircuitGraph
{
public:
    CircuitGraph();

    void reset();
    void update(float dt);

    template <typename T, typename... Args>
    T &addComponent(Args &&...args)
    {
        m_components.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        return static_cast<T &>(*m_components.back());
    }

    std::vector<std::unique_ptr<Component>> &components();
    int nodeCount() const;

    // TODO(Phase 5): solve() running modified nodal analysis over m_components

private:
    std::vector<std::unique_ptr<Component>> m_components;
    int m_nodeCount;
};
