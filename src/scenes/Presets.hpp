#pragma once

class World;
class CircuitGraph;

// Preloaded test scenes, one function per preset, mirroring Newton's Notepad's
// World::loadTestScene() pattern.
namespace Presets
{
void loadDipoleField(World &world);
void loadParallelPlateCapacitor(CircuitGraph &circuit);
void loadSimpleRCCircuit(CircuitGraph &circuit);
void loadParticleInUniformB(World &world);
} // namespace Presets
