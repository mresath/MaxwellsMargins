#include "scenes/Presets.hpp"

#include "Config.hpp"
#include "circuits/Battery.hpp"
#include "circuits/Capacitor.hpp"
#include "circuits/CircuitGraph.hpp"
#include "circuits/Inductor.hpp"
#include "circuits/Lightbulb.hpp"
#include "circuits/Probe.hpp"
#include "circuits/Resistor.hpp"
#include "circuits/Switch.hpp"
#include "core/World.hpp"
#include "engine/FieldMath.hpp"
#include "induction/MovingLoop.hpp"
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

void loadBasicResistorCircuit(CircuitGraph &circuit)
{
    circuit.reset();
    const Vec2 center = viewCenterMeters();
    const Vec2 topLeft = center + Vec2(-4.0f, -1.0f);
    const Vec2 topMid = center + Vec2(-1.0f, -1.0f);
    const Vec2 topRight = center + Vec2(2.0f, -1.0f);
    const Vec2 bottomLeft = center + Vec2(-4.0f, 1.0f);
    const Vec2 bottomRight = center + Vec2(2.0f, 1.0f);
    const Vec2 voltTop = center + Vec2(4.0f, -1.0f);
    const Vec2 voltBottom = center + Vec2(4.0f, 1.0f);

    // The simplest complete circuit: battery -> switch -> ammeter -> resistor -> back to
    // battery, with a voltmeter wired as a separate parallel branch reading straight across
    // the resistor - a non-invasive probe, so it doesn't affect the ammeter's reading.
    // Every component's posA is oriented toward the battery's + terminal (bottomLeft, via
    // the bottom rail) so current/voltage read positive, matching conventional-current flow.
    circuit.addComponent<Battery>(bottomLeft, topLeft, DEFAULT_EMF, DEFAULT_INTERNAL_RESISTANCE, circuit.allocateId());
    circuit.addComponent<Switch>(topMid, topLeft, true, circuit.allocateId());
    Probe &ammeter = circuit.addComponent<Probe>(topRight, topMid, Probe::Kind::Ammeter, circuit.allocateId());
    ammeter.showLabel = true;
    // The resistor's own label stays off by default - the ammeter/voltmeter already show
    // its current/voltage, so a third label would just repeat the same numbers.
    circuit.addComponent<Resistor>(bottomRight, topRight, DEFAULT_RESISTANCE, circuit.allocateId());
    circuit.wires().emplace_back(bottomRight, bottomLeft, circuit.allocateId());
    circuit.wires().emplace_back(topRight, voltTop, circuit.allocateId());
    circuit.wires().emplace_back(bottomRight, voltBottom, circuit.allocateId());
    Probe &voltmeter = circuit.addComponent<Probe>(voltBottom, voltTop, Probe::Kind::Voltmeter, circuit.allocateId());
    voltmeter.showLabel = true;
}

void loadLightbulbCircuit(CircuitGraph &circuit)
{
    circuit.reset();
    const Vec2 center = viewCenterMeters();
    const Vec2 topLeft = center + Vec2(-2.0f, -1.0f);
    const Vec2 topRight = center + Vec2(2.0f, -1.0f);
    const Vec2 bottomRight = center + Vec2(2.0f, 1.0f);
    const Vec2 bottomLeft = center + Vec2(-2.0f, 1.0f);

    // A single loop: battery -> switch -> lightbulb -> back to battery, starting open so
    // flipping the switch is what lights it up.
    circuit.addComponent<Battery>(bottomLeft, topLeft, DEFAULT_EMF, DEFAULT_INTERNAL_RESISTANCE, circuit.allocateId());
    circuit.addComponent<Switch>(topLeft, topRight, false, circuit.allocateId());
    Lightbulb &bulb = circuit.addComponent<Lightbulb>(topRight, bottomRight, DEFAULT_RESISTANCE, circuit.allocateId());
    bulb.showLabel = true; // the stat this whole preset is about
    circuit.wires().emplace_back(bottomRight, bottomLeft, circuit.allocateId());
}

void loadSimpleRCCircuit(CircuitGraph &circuit)
{
    circuit.reset();
    const Vec2 center = viewCenterMeters();
    const Vec2 topLeft = center + Vec2(-3.0f, -1.0f);
    const Vec2 topMid = center + Vec2(0.0f, -1.0f);
    const Vec2 topRight = center + Vec2(3.0f, -1.0f);
    const Vec2 bottomLeft = center + Vec2(-3.0f, 1.0f);
    const Vec2 bottomMid = center + Vec2(0.0f, 1.0f);
    const Vec2 bottomRight = center + Vec2(3.0f, 1.0f);

    // Two switches drive both halves of the classic RC story: with the charge switch
    // closed and the discharge switch open (the starting state), the battery charges the
    // capacitor through the resistor (V_C(t) = V0*(1 - e^(-t/RC)), see SCIENCE.md). Open
    // the charge switch and close the discharge switch instead to discharge the capacitor
    // back through the same resistor - a loop with no battery in it at all.
    circuit.addComponent<Battery>(bottomLeft, topLeft, DEFAULT_EMF, DEFAULT_INTERNAL_RESISTANCE, circuit.allocateId());
    circuit.addComponent<Switch>(topLeft, topMid, true, circuit.allocateId());
    circuit.addComponent<Resistor>(topMid, topRight, DEFAULT_RESISTANCE, circuit.allocateId());
    // posA=bottomRight (tied to the battery's + terminal via the bottom rail) so the
    // capacitor charges to a positive voltage, matching the textbook charging-curve picture.
    Capacitor &capacitor = circuit.addComponent<Capacitor>(bottomRight, topRight, DEFAULT_CAPACITANCE, circuit.allocateId());
    capacitor.showLabel = true; // the stat this whole preset is about
    circuit.addComponent<Switch>(topMid, bottomMid, false, circuit.allocateId());
    circuit.wires().emplace_back(bottomRight, bottomMid, circuit.allocateId());
    circuit.wires().emplace_back(bottomMid, bottomLeft, circuit.allocateId());
}

void loadSimpleLRCircuit(CircuitGraph &circuit)
{
    circuit.reset();
    const Vec2 center = viewCenterMeters();
    const Vec2 topLeft = center + Vec2(-3.0f, -1.0f);
    const Vec2 topMid = center + Vec2(0.0f, -1.0f);
    const Vec2 topRight = center + Vec2(3.0f, -1.0f);
    const Vec2 bottomLeft = center + Vec2(-3.0f, 1.0f);
    const Vec2 bottomMid = center + Vec2(0.0f, 1.0f);
    const Vec2 bottomRight = center + Vec2(3.0f, 1.0f);

    // Two switches mirror the RC preset: with the charge switch closed and the decay
    // switch open (the starting state), the battery drives current up through the resistor
    // and inductor (I(t) = (V0/R)*(1 - e^(-tR/L)), see SCIENCE.md). Open the charge switch
    // and close the decay switch instead to disconnect the battery, letting the inductor's
    // stored current decay back down through just the resistor.
    circuit.addComponent<Battery>(bottomLeft, topLeft, DEFAULT_EMF, DEFAULT_INTERNAL_RESISTANCE, circuit.allocateId());
    circuit.addComponent<Switch>(topLeft, topMid, true, circuit.allocateId());
    circuit.addComponent<Resistor>(topMid, topRight, DEFAULT_RESISTANCE, circuit.allocateId());
    Inductor &inductor = circuit.addComponent<Inductor>(bottomRight, topRight, DEFAULT_INDUCTANCE, circuit.allocateId());
    inductor.showLabel = true; // the stat this whole preset is about
    circuit.addComponent<Switch>(topMid, bottomMid, false, circuit.allocateId());
    circuit.wires().emplace_back(bottomRight, bottomMid, circuit.allocateId());
    circuit.wires().emplace_back(bottomMid, bottomLeft, circuit.allocateId());
}

void loadSimpleLCCircuit(CircuitGraph &circuit)
{
    circuit.reset();
    const Vec2 center = viewCenterMeters();
    const Vec2 topLeft = center + Vec2(-3.0f, -1.0f);
    const Vec2 topMid = center + Vec2(0.0f, -1.0f);
    const Vec2 topRight = center + Vec2(3.0f, -1.0f);
    const Vec2 bottomLeft = center + Vec2(-3.0f, 1.0f);
    const Vec2 bottomMid = center + Vec2(0.0f, 1.0f);
    const Vec2 bottomRight = center + Vec2(3.0f, 1.0f);

    // Two switches again: with the charge switch closed and the oscillate switch open (the
    // starting state), the battery charges the capacitor through the inductor. Open the
    // charge switch and close the oscillate switch instead to disconnect the battery
    // entirely, leaving a pure L-C loop that rings at angular frequency 1/sqrt(LC) (period
    // 2*pi*sqrt(LC)) - see SCIENCE.md.
    circuit.addComponent<Battery>(bottomLeft, topLeft, DEFAULT_EMF, DEFAULT_INTERNAL_RESISTANCE, circuit.allocateId());
    circuit.addComponent<Switch>(topLeft, topMid, true, circuit.allocateId());
    Inductor &inductor = circuit.addComponent<Inductor>(topMid, topRight, DEFAULT_INDUCTANCE, circuit.allocateId());
    inductor.showLabel = true; // both L and C are central to the oscillation story
    // posA=bottomRight (tied to the battery's + terminal via the bottom rail) so the
    // capacitor charges to a positive voltage, matching the textbook charging-curve picture.
    Capacitor &capacitor = circuit.addComponent<Capacitor>(bottomRight, topRight, DEFAULT_CAPACITANCE, circuit.allocateId());
    capacitor.showLabel = true;
    circuit.addComponent<Switch>(topMid, bottomMid, false, circuit.allocateId());
    circuit.wires().emplace_back(bottomRight, bottomMid, circuit.allocateId());
    circuit.wires().emplace_back(bottomMid, bottomLeft, circuit.allocateId());
}

void loadParticleInUniformB(World &world)
{
    world.reset();
    world.uniformField().enabled = true;
    world.uniformField().strength = DEFAULT_B_FIELD_STRENGTH;
    world.particles().emplace_back(viewCenterMeters(), Vec2(DEFAULT_PARTICLE_SPEED, 0.0f), DEFAULT_PARTICLE_CHARGE_MAGNITUDE, DEFAULT_PARTICLE_MASS, world.allocateEntityId());
}

void loadGeneratorDemo(World &world)
{
    world.reset();
    world.uniformField().enabled = true;
    world.uniformField().strength = DEFAULT_B_FIELD_STRENGTH;

    MovingLoop loop(viewCenterMeters(), DEFAULT_LOOP_RADIUS, 5, world.allocateEntityId());
    loop.angularVelocity = 2.0f;
    loop.lastFlux = FieldMath::loopFlux(world.magneticFieldAt(loop.center), loop.area(), loop.rotationAngle);
    world.loops().push_back(loop);
}

} // namespace Presets
