#include "scenes/Presets.hpp"

#include "Config.hpp"
#include "circuits/CircuitGraph.hpp"
#include "core/World.hpp"
#include "math/Vec2.hpp"

namespace Presets
{

void loadDipoleField(World &world)
{
    world.reset();
    world.charges().emplace_back(Vec2(-1.0f, 0.0f), DEFAULT_CHARGE_MAGNITUDE, world.allocateEntityId());
    world.charges().emplace_back(Vec2(1.0f, 0.0f), -DEFAULT_CHARGE_MAGNITUDE, world.allocateEntityId());
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
