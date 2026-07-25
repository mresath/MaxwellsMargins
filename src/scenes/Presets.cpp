#include "scenes/Presets.hpp"

#include "Config.hpp"
#include "circuits/CircuitGraph.hpp"
#include "core/World.hpp"
#include "math/Util.hpp"
#include "math/Vec2.hpp"

namespace
{
// The view is centered on (DEF_WIDTH/2, DEF_HEIGHT/2) pixels, not on meters-space (0,0),
// so presets need to place entities relative to this point to land in the visible viewport.
Vec2 viewCenterMeters()
{
    return pixelsToMeters(Vec2(DEF_WIDTH / 2.0f, DEF_HEIGHT / 2.0f));
}
} // namespace

namespace Presets
{

void loadDipoleField(World &world)
{
    world.reset();
    const Vec2 center = viewCenterMeters();
    world.charges().emplace_back(center + Vec2(-1.0f, 0.0f), DEFAULT_CHARGE_MAGNITUDE, world.allocateEntityId());
    world.charges().emplace_back(center + Vec2(1.0f, 0.0f), -DEFAULT_CHARGE_MAGNITUDE, world.allocateEntityId());
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
    world.reset();
    world.uniformField().enabled = true;
    world.uniformField().strength = DEFAULT_B_FIELD_STRENGTH;
    world.particles().emplace_back(viewCenterMeters(), Vec2(DEFAULT_PARTICLE_SPEED, 0.0f), DEFAULT_PARTICLE_CHARGE_MAGNITUDE, DEFAULT_PARTICLE_MASS, world.allocateEntityId());
}

} // namespace Presets
