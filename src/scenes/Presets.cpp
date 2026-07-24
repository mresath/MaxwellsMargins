#include "scenes/Presets.hpp"

#include "circuits/CircuitGraph.hpp"
#include "core/World.hpp"

namespace Presets
{

void loadDipoleField(World &world)
{
    // TODO(Phase 2): world.reset(); add a +/- charge pair
    (void)world;
}

void loadParallelPlateCapacitor(CircuitGraph &circuit)
{
    // TODO(Phase 5): circuit.reset(); battery + capacitor + switch
    (void)circuit;
}

void loadSimpleRCCircuit(CircuitGraph &circuit)
{
    // TODO(Phase 5): circuit.reset(); battery + resistor + capacitor
    (void)circuit;
}

void loadParticleInUniformB(World &world)
{
    // TODO(Phase 3): world.reset(); enable uniform B field, add one ChargedParticle
    (void)world;
}

} // namespace Presets
