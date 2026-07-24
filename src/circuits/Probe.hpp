#pragma once

#include "circuits/Component.hpp"

// Ammeter/Voltmeter probe: click-to-measure at a point in the circuit. Ammeters are
// modeled as zero-resistance components in series; voltmeters just read two node
// potentials without being wired into the solve.
class Probe : public Component
{
public:
    enum class Kind
    {
        Ammeter,
        Voltmeter
    };

    Probe(int nodeA, int nodeB, Kind kind) : Component(nodeA, nodeB), kind(kind) {}

    Kind kind;

    std::string typeName() const override { return kind == Kind::Ammeter ? "Ammeter" : "Voltmeter"; }
};
