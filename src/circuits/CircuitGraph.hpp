#pragma once

#include <memory>
#include <vector>

#include "circuits/CircuitWire.hpp"
#include "circuits/Component.hpp"
#include "math/Vec2.hpp"

enum class CircuitEntityKind
{
    None,
    Component,
    Wire
};

struct CircuitEntityRef
{
    CircuitEntityKind kind = CircuitEntityKind::None;
    int id = -1;
};

// Container for the Circuits-mode scene (mirrors World's role for Fields mode): components
// and wires placed on the schematic grid, solved each step via modified nodal analysis
// (Kirchhoff's current/voltage laws), with capacitor charge integrated over time using
// engine/Solver. See CircuitGraph.cpp for the node-reduction and solve algorithm.
class CircuitGraph
{
public:
    CircuitGraph();

    void reset();
    void update(float dt);
    float simTime() const;

    template <typename T, typename... Args>
    T &addComponent(Args &&...args)
    {
        m_components.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        return static_cast<T &>(*m_components.back());
    }

    std::vector<std::unique_ptr<Component>> &components();
    const std::vector<std::unique_ptr<Component>> &components() const;
    std::vector<CircuitWire> &wires();
    const std::vector<CircuitWire> &wires() const;

    int allocateId();

    // Id stays valid across insertions/erasures elsewhere, unlike a vector index - so App
    // can hold a selection/grab across frames safely (mirrors World::EntityRef).
    CircuitEntityRef findEntityAt(const Vec2 &pos) const;
    void removeEntity(CircuitEntityKind kind, int id);
    Component *findComponent(int id);
    CircuitWire *findWire(int id);

private:
    void solve(float dt);

    std::vector<std::unique_ptr<Component>> m_components;
    std::vector<CircuitWire> m_wires;
    float m_simTime;
    int m_nextId;
};
