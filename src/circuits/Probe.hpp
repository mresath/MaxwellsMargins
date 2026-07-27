#pragma once

#include "circuits/Component.hpp"

// Ammeter/Voltmeter probe: click-to-measure at a point in the circuit. Ammeters are
// modeled as an ideal (zero-volt) voltage source in series, so their solved branch current
// is the reading; voltmeters just read two node potentials without being wired into the
// solve at all (non-invasive).
class Probe : public Component
{
public:
    enum class Kind
    {
        Ammeter,
        Voltmeter
    };

    Probe(Vec2 posA, Vec2 posB, Kind kind, int id) : Component(posA, posB, id), kind(kind) {}

    Kind kind;

    std::string typeName() const override { return kind == Kind::Ammeter ? "Ammeter" : "Voltmeter"; }
};
