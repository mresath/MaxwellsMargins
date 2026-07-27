#include "circuits/CircuitGraph.hpp"

#include "Config.hpp"
#include "circuits/Battery.hpp"
#include "circuits/Capacitor.hpp"
#include "circuits/Inductor.hpp"
#include "circuits/Probe.hpp"
#include "circuits/Resistor.hpp"
#include "circuits/Switch.hpp"
#include "engine/Solver.hpp"
#include "math/Util.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <utility>

namespace
{
struct UnionFind
{
    std::vector<int> parent;
    explicit UnionFind(int n) : parent(n)
    {
        for (int i = 0; i < n; ++i)
            parent[i] = i;
    }
    int find(int i) { return parent[i] == i ? i : (parent[i] = find(parent[i])); }
    void unite(int a, int b)
    {
        a = find(a);
        b = find(b);
        if (a != b)
            parent[b] = a;
    }
};

using GridKey = std::pair<long long, long long>;

GridKey gridKeyOf(const Vec2 &pos)
{
    return {std::lround(pos.x / GRID_MINOR_SPACING), std::lround(pos.y / GRID_MINOR_SPACING)};
}

// A voltage-source-like branch: an ideal EMF (possibly zero) in series with a resistance,
// contributing both a KCL current and its own constitutive-relation row to the MNA matrix.
// Battery, Capacitor (voltage frozen at charge/capacitance for this step - a resistive
// "companion model"), and Ammeter (an ideal 0V source, so its branch current is the
// reading) all reduce to this same shape.
struct SourceBranch
{
    int nodeA;
    int nodeB;
    float sourceVoltage;
    float seriesResistance;
    Component *component;
};

struct ConductanceEdge
{
    int nodeA;
    int nodeB;
    float conductance;
    Component *component;
};

// A wire or closed switch: an ideal zero-resistance link between two grid points, whose
// endpoints solve() already merged into one electrical node. `a`/`b` fix the component's
// own start/end (or posA/posB) orientation, independent of any later graph traversal.
struct UnionEdge
{
    int a;
    int b;
    CircuitWire *wire;
    Switch *switchComponent;
};
} // namespace

CircuitGraph::CircuitGraph() : m_simTime(0.0f), m_nextId(0)
{
}

void CircuitGraph::reset()
{
    m_components.clear();
    m_wires.clear();
    m_simTime = 0.0f;
}

void CircuitGraph::update(float dt)
{
    solve(dt);

    for (auto &comp : m_components)
    {
        if (Capacitor *cap = dynamic_cast<Capacitor *>(comp.get()))
        {
            const double current = static_cast<double>(cap->current);
            const Solver::State next = Solver::step(
                [current](double, const Solver::State &) -> Solver::State { return {current}; },
                0.0, Solver::State{static_cast<double>(cap->charge)}, static_cast<double>(dt));
            cap->charge = static_cast<float>(next[0]);
        }
        else if (Inductor *inductor = dynamic_cast<Inductor *>(comp.get()))
        {
            // The backward-Euler solve already produced the exact new current directly
            // (inductor->current), so there's no separate dt-scaled quantity left to
            // integrate - unlike Capacitor's charge, which is a genuine running total.
            // Routed through Solver::step anyway for the same single-solver architecture: a
            // constant rate from the old to the new value, evaluated with the OLD current
            // fixed in the closure (not the solver's intermediate state), integrates over
            // dt to land exactly on the new value regardless of Solver::step's RK weights.
            const double initial = static_cast<double>(inductor->storedCurrent);
            const double rate = (static_cast<double>(inductor->current) - initial) / std::max(static_cast<double>(dt), 1e-9);
            const Solver::State next = Solver::step(
                [rate](double, const Solver::State &) -> Solver::State { return {rate}; },
                0.0, Solver::State{initial}, static_cast<double>(dt));
            inductor->storedCurrent = static_cast<float>(next[0]);
        }
    }

    m_simTime += dt;
}

float CircuitGraph::simTime() const
{
    return m_simTime;
}

std::vector<std::unique_ptr<Component>> &CircuitGraph::components()
{
    return m_components;
}

const std::vector<std::unique_ptr<Component>> &CircuitGraph::components() const
{
    return m_components;
}

std::vector<CircuitWire> &CircuitGraph::wires()
{
    return m_wires;
}

const std::vector<CircuitWire> &CircuitGraph::wires() const
{
    return m_wires;
}

int CircuitGraph::allocateId()
{
    return m_nextId++;
}

CircuitEntityRef CircuitGraph::findEntityAt(const Vec2 &pos) const
{
    for (const auto &comp : m_components)
        if (distanceToSegment(pos, comp->posA, comp->posB) < ENTITY_HIT_RADIUS)
            return {CircuitEntityKind::Component, comp->id};

    for (const auto &wire : m_wires)
        if (distanceToSegment(pos, wire.start, wire.end) < ENTITY_HIT_RADIUS)
            return {CircuitEntityKind::Wire, wire.id};

    return {};
}

void CircuitGraph::removeEntity(CircuitEntityKind kind, int id)
{
    if (kind == CircuitEntityKind::Component)
    {
        for (std::size_t i = 0; i < m_components.size(); ++i)
        {
            if (m_components[i]->id == id)
            {
                m_components.erase(m_components.begin() + i);
                return;
            }
        }
    }
    else if (kind == CircuitEntityKind::Wire)
    {
        for (std::size_t i = 0; i < m_wires.size(); ++i)
        {
            if (m_wires[i].id == id)
            {
                m_wires.erase(m_wires.begin() + i);
                return;
            }
        }
    }
}

Component *CircuitGraph::findComponent(int id)
{
    for (auto &comp : m_components)
        if (comp->id == id)
            return comp.get();
    return nullptr;
}

CircuitWire *CircuitGraph::findWire(int id)
{
    for (auto &wire : m_wires)
        if (wire.id == id)
            return &wire;
    return nullptr;
}

void CircuitGraph::solve(float dt)
{
    // Step 1: every distinct grid point touched by a terminal or wire endpoint becomes a
    // raw node, in first-seen order (deterministic, so repeated solves without any editing
    // reproduce the same matrix layout).
    std::map<GridKey, int> rawIndexOf;
    auto rawIndex = [&](const Vec2 &pos) -> int
    {
        const GridKey key = gridKeyOf(pos);
        const auto it = rawIndexOf.find(key);
        if (it != rawIndexOf.end())
            return it->second;
        const int index = static_cast<int>(rawIndexOf.size());
        rawIndexOf.emplace(key, index);
        return index;
    };

    for (const auto &comp : m_components)
    {
        rawIndex(comp->posA);
        rawIndex(comp->posB);
    }
    for (const auto &wire : m_wires)
    {
        rawIndex(wire.start);
        rawIndex(wire.end);
    }

    const int rawCount = static_cast<int>(rawIndexOf.size());
    if (rawCount == 0)
        return;

    // Step 2: wires and closed switches are ideal zero-resistance links, so their two
    // endpoints are literally the same electrical node - merge via union-find rather than
    // giving them their own branch-current unknown.
    UnionFind electricalUF(rawCount);
    for (const auto &wire : m_wires)
        electricalUF.unite(rawIndex(wire.start), rawIndex(wire.end));
    for (const auto &comp : m_components)
    {
        if (const Switch *sw = dynamic_cast<const Switch *>(comp.get()); sw != nullptr && sw->closed)
            electricalUF.unite(rawIndex(sw->posA), rawIndex(sw->posB));
    }

    // Step 3: compact union-find roots into consecutive canonical node ids.
    std::vector<int> canonicalOfRoot(rawCount, -1);
    std::vector<int> canonicalOf(rawCount, -1);
    int nodeCount = 0;
    for (int raw = 0; raw < rawCount; ++raw)
    {
        const int root = electricalUF.find(raw);
        if (canonicalOfRoot[root] == -1)
            canonicalOfRoot[root] = nodeCount++;
        canonicalOf[raw] = canonicalOfRoot[root];
    }

    auto nodeOf = [&](const Vec2 &pos) { return canonicalOf[rawIndex(pos)]; };
    for (auto &comp : m_components)
    {
        comp->nodeA = nodeOf(comp->posA);
        comp->nodeB = nodeOf(comp->posB);
    }

    // Step 4: every other component becomes either a conductance edge (Resistor) or a
    // voltage-source branch with its own current unknown (Battery/Capacitor/Ammeter).
    // Open switches and voltmeters contribute nothing to the matrix at all.
    std::vector<ConductanceEdge> conductanceEdges;
    std::vector<SourceBranch> sourceBranches;

    for (auto &comp : m_components)
    {
        if (Resistor *resistor = dynamic_cast<Resistor *>(comp.get()))
        {
            conductanceEdges.push_back({resistor->nodeA, resistor->nodeB, 1.0f / std::max(resistor->resistance, 1e-6f), resistor});
        }
        else if (Battery *battery = dynamic_cast<Battery *>(comp.get()))
        {
            sourceBranches.push_back({battery->nodeA, battery->nodeB, battery->emf, battery->internalResistance, battery});
        }
        else if (Capacitor *capacitor = dynamic_cast<Capacitor *>(comp.get()))
        {
            // Backward-Euler companion model: V = Q_n/C + I*(dt/C), i.e. a "voltage
            // source" at last step's charge in series with a dt/C resistance. Solving
            // this each step - rather than freezing V=Q_n/C with zero series resistance
            // and stepping explicitly - stays stable even when dt is much larger than the
            // circuit's own RC time constant (a plain explicit step diverges in that case).
            const float capacitance = std::max(capacitor->capacitance, 1e-12f);
            const float voltage = capacitor->charge / capacitance;
            sourceBranches.push_back({capacitor->nodeA, capacitor->nodeB, voltage, dt / capacitance, capacitor});
        }
        else if (Probe *probe = dynamic_cast<Probe *>(comp.get()); probe != nullptr && probe->kind == Probe::Kind::Ammeter)
        {
            sourceBranches.push_back({probe->nodeA, probe->nodeB, 0.0f, 0.0f, probe});
        }
        else if (Inductor *inductor = dynamic_cast<Inductor *>(comp.get()))
        {
            // Backward-Euler companion model, the dual of Capacitor's: V = (L/dt)*I -
            // (L/dt)*I_n, i.e. an (L/dt) series resistance with a voltage source set by
            // last step's current - same unconditional stability rationale as Capacitor's.
            const float seriesResistance = inductor->inductance / std::max(dt, 1e-9f);
            sourceBranches.push_back({inductor->nodeA, inductor->nodeB, -seriesResistance * inductor->storedCurrent, seriesResistance, inductor});
        }
    }

    // Step 5: pick one reference (ground, V=0) node per connected island, rather than one
    // global ground - so components the user hasn't wired up yet (a floating resistor with
    // no path to the rest of the circuit) don't leave the whole matrix singular.
    UnionFind islandUF(nodeCount);
    for (const auto &edge : conductanceEdges)
        islandUF.unite(edge.nodeA, edge.nodeB);
    for (const auto &branch : sourceBranches)
        islandUF.unite(branch.nodeA, branch.nodeB);

    std::vector<int> groundOfIsland(nodeCount, -1);
    for (int node = 0; node < nodeCount; ++node)
    {
        const int island = islandUF.find(node);
        if (groundOfIsland[island] == -1)
            groundOfIsland[island] = node;
    }

    std::vector<int> nodeUnknownIndex(nodeCount, -1);
    int nodeUnknownCount = 0;
    for (int node = 0; node < nodeCount; ++node)
    {
        const int island = islandUF.find(node);
        if (node != groundOfIsland[island])
            nodeUnknownIndex[node] = nodeUnknownCount++;
    }

    const int totalUnknowns = nodeUnknownCount + static_cast<int>(sourceBranches.size());
    if (totalUnknowns == 0)
        return;

    // Step 6: assemble and solve the MNA system A*x = b (node voltages, then one branch
    // current per voltage source), via Gaussian elimination with partial pivoting.
    std::vector<std::vector<double>> A(totalUnknowns, std::vector<double>(totalUnknowns, 0.0));
    std::vector<double> b(totalUnknowns, 0.0);

    for (const auto &edge : conductanceEdges)
    {
        const int ia = nodeUnknownIndex[edge.nodeA];
        const int ib = nodeUnknownIndex[edge.nodeB];
        const double g = static_cast<double>(edge.conductance);
        if (ia >= 0)
            A[ia][ia] += g;
        if (ib >= 0)
            A[ib][ib] += g;
        if (ia >= 0 && ib >= 0)
        {
            A[ia][ib] -= g;
            A[ib][ia] -= g;
        }
    }

    for (std::size_t k = 0; k < sourceBranches.size(); ++k)
    {
        const SourceBranch &branch = sourceBranches[k];
        const int ik = nodeUnknownCount + static_cast<int>(k);
        const int ia = nodeUnknownIndex[branch.nodeA];
        const int ib = nodeUnknownIndex[branch.nodeB];

        if (ia >= 0)
        {
            A[ia][ik] += 1.0;
            A[ik][ia] += 1.0;
        }
        if (ib >= 0)
        {
            A[ib][ik] -= 1.0;
            A[ik][ib] -= 1.0;
        }
        A[ik][ik] -= static_cast<double>(branch.seriesResistance);
        b[ik] += static_cast<double>(branch.sourceVoltage);
    }

    std::vector<double> x(totalUnknowns, 0.0);
    {
        std::vector<std::vector<double>> M = A;
        std::vector<double> rhs = b;

        for (int col = 0; col < totalUnknowns; ++col)
        {
            int pivotRow = col;
            double pivotMag = std::abs(M[col][col]);
            for (int row = col + 1; row < totalUnknowns; ++row)
            {
                if (std::abs(M[row][col]) > pivotMag)
                {
                    pivotMag = std::abs(M[row][col]);
                    pivotRow = row;
                }
            }
            if (pivotRow != col)
            {
                std::swap(M[col], M[pivotRow]);
                std::swap(rhs[col], rhs[pivotRow]);
            }
            // A near-singular pivot means a redundant/indeterminate sub-configuration (e.g.
            // two ideal sources shorted together at different EMFs); nudging the diagonal
            // keeps the system solvable instead of dividing by ~0, at the cost of an
            // arbitrary (not physically meaningful) result for that one degenerate unknown.
            if (std::abs(M[col][col]) < 1e-9)
                M[col][col] += 1e-6;

            for (int row = col + 1; row < totalUnknowns; ++row)
            {
                const double factor = M[row][col] / M[col][col];
                if (factor == 0.0)
                    continue;
                for (int c = col; c < totalUnknowns; ++c)
                    M[row][c] -= factor * M[col][c];
                rhs[row] -= factor * rhs[col];
            }
        }

        for (int row = totalUnknowns - 1; row >= 0; --row)
        {
            double sum = rhs[row];
            for (int c = row + 1; c < totalUnknowns; ++c)
                sum -= M[row][c] * x[c];
            x[row] = sum / M[row][row];
        }
    }

    std::vector<double> nodeVoltage(nodeCount, 0.0);
    for (int node = 0; node < nodeCount; ++node)
        nodeVoltage[node] = (nodeUnknownIndex[node] >= 0) ? x[nodeUnknownIndex[node]] : 0.0;

    for (const auto &edge : conductanceEdges)
    {
        const double v = nodeVoltage[edge.nodeA] - nodeVoltage[edge.nodeB];
        edge.component->voltage = static_cast<float>(v);
        edge.component->current = static_cast<float>(v * edge.conductance);
    }

    for (std::size_t k = 0; k < sourceBranches.size(); ++k)
    {
        const SourceBranch &branch = sourceBranches[k];
        branch.component->current = static_cast<float>(x[nodeUnknownCount + static_cast<int>(k)]);
        branch.component->voltage = static_cast<float>(nodeVoltage[branch.nodeA] - nodeVoltage[branch.nodeB]);
    }

    for (auto &comp : m_components)
    {
        if (Switch *sw = dynamic_cast<Switch *>(comp.get()))
        {
            sw->voltage = static_cast<float>(nodeVoltage[sw->nodeA] - nodeVoltage[sw->nodeB]);
            sw->current = 0.0f; // filled in below for closed switches, via the flow-decomposition pass
        }
        else if (Probe *probe = dynamic_cast<Probe *>(comp.get()); probe != nullptr && probe->kind == Probe::Kind::Voltmeter)
        {
            probe->voltage = static_cast<float>(nodeVoltage[probe->nodeA] - nodeVoltage[probe->nodeB]);
            probe->current = 0.0f; // non-invasive: never part of the solve, never carries current
        }
    }

    // Step 7: wire/closed-switch current, for the flow-animation visualization only. An
    // ideal wire has zero voltage drop, so a multi-way split at a junction can't be read
    // off node voltages the way a resistor's can - approximated by taking a BFS spanning
    // tree of each merged node's wire/switch subgraph and assigning each tree edge the net
    // injected current in its subtree (exact for a plain series/parallel tree of wiring;
    // any redundant loop the user drew on top of that gets 0 rather than an arbitrary split).
    std::vector<double> injection(rawCount, 0.0);
    auto addInjection = [&](Component *comp)
    {
        const int a = rawIndex(comp->posA);
        const int b = rawIndex(comp->posB);
        injection[a] -= static_cast<double>(comp->current);
        injection[b] += static_cast<double>(comp->current);
    };
    for (const auto &edge : conductanceEdges)
        addInjection(edge.component);
    for (const auto &branch : sourceBranches)
        addInjection(branch.component);

    std::vector<UnionEdge> unionEdges;
    unionEdges.reserve(m_wires.size() + m_components.size());
    for (auto &wire : m_wires)
        unionEdges.push_back({rawIndex(wire.start), rawIndex(wire.end), &wire, nullptr});
    for (auto &comp : m_components)
    {
        if (Switch *sw = dynamic_cast<Switch *>(comp.get()); sw != nullptr && sw->closed)
            unionEdges.push_back({rawIndex(sw->posA), rawIndex(sw->posB), nullptr, sw});
    }

    std::vector<std::vector<int>> adjacency(rawCount);
    for (std::size_t e = 0; e < unionEdges.size(); ++e)
    {
        adjacency[unionEdges[e].a].push_back(static_cast<int>(e));
        adjacency[unionEdges[e].b].push_back(static_cast<int>(e));
    }

    std::vector<bool> visited(rawCount, false);
    for (int start = 0; start < rawCount; ++start)
    {
        if (visited[start])
            continue;

        std::vector<int> order;
        std::vector<int> parentEdge(rawCount, -1);
        std::vector<int> parentNode(rawCount, -1);
        std::vector<int> queue = {start};
        visited[start] = true;

        for (std::size_t head = 0; head < queue.size(); ++head)
        {
            const int node = queue[head];
            order.push_back(node);
            for (int e : adjacency[node])
            {
                const UnionEdge &edge = unionEdges[e];
                const int other = (edge.a == node) ? edge.b : edge.a;
                if (visited[other])
                    continue;
                visited[other] = true;
                parentEdge[other] = e;
                parentNode[other] = node;
                queue.push_back(other);
            }
        }

        std::vector<double> subtreeInjection(rawCount, 0.0);
        for (int node : order)
            subtreeInjection[node] += injection[node];

        for (auto it = order.rbegin(); it != order.rend(); ++it)
        {
            const int node = *it;
            if (parentEdge[node] == -1)
                continue;

            const double flowParentToNode = -subtreeInjection[node];
            UnionEdge &edge = unionEdges[parentEdge[node]];
            const double flowAtoB = (node == edge.b) ? flowParentToNode : -flowParentToNode;
            if (edge.wire)
                edge.wire->current = static_cast<float>(flowAtoB);
            else if (edge.switchComponent)
                edge.switchComponent->current = static_cast<float>(flowAtoB);

            subtreeInjection[parentNode[node]] += subtreeInjection[node];
        }
    }
}
