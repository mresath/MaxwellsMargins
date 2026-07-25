#pragma once

class World;
class CircuitGraph;

// Preloaded test scenes, one function per preset.
namespace Presets
{
void loadDipoleField(World &world);
void loadParallelPlateCapacitor(CircuitGraph &circuit);
void loadSimpleRCCircuit(CircuitGraph &circuit);
void loadParticleInUniformB(World &world);
} // namespace Presets
