#pragma once

class World;
class CircuitGraph;

// Preloaded test scenes, one function per preset.
namespace Presets
{
void loadDipoleField(World &world);
void loadBasicResistorCircuit(CircuitGraph &circuit);
void loadLightbulbCircuit(CircuitGraph &circuit);
void loadSimpleRCCircuit(CircuitGraph &circuit);
void loadSimpleLRCircuit(CircuitGraph &circuit);
void loadSimpleLCCircuit(CircuitGraph &circuit);
void loadParticleInUniformB(World &world);
void loadGeneratorDemo(World &world);
} // namespace Presets
