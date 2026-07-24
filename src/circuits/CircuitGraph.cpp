#include "circuits/CircuitGraph.hpp"

CircuitGraph::CircuitGraph() : m_nodeCount(0)
{
}

void CircuitGraph::reset()
{
    m_components.clear();
    m_nodeCount = 0;
}

void CircuitGraph::update(float dt)
{
    // TODO(Phase 5): run the MNA solve, then step any Capacitor/Inductor state via
    // engine/Solver for RC/RL transient behavior.
    (void)dt;
}

std::vector<std::unique_ptr<Component>> &CircuitGraph::components()
{
    return m_components;
}

int CircuitGraph::nodeCount() const
{
    return m_nodeCount;
}
