#pragma once

#include "math/Vec2.hpp"

class World;
class CircuitGraph;

// User-interaction tools available per mode. A click/drag is routed to the active
// World or CircuitGraph depending on the current Mode (see core/Mode.hpp).
enum class ToolType
{
    // Fields mode
    PlacePositiveCharge,
    PlaceNegativeCharge,
    PlaceCurrentWire,
    PlaceChargedParticle,
    PlaceMovingLoop,
    DrawGaussianSurface,
    FieldProbe,

    // Circuits mode
    PlaceResistor,
    PlaceCapacitor,
    PlaceBattery,
    PlaceSwitch,
    PlaceWire,
    PlaceAmmeter,
    PlaceVoltmeter,

    // Shared
    Select,
    Pan
};

class Tools
{
public:
    Tools();

    ToolType activeTool() const;
    void setActiveTool(ToolType tool);

    void onClick(const Vec2 &worldPos, World &world);
    void onClick(const Vec2 &worldPos, CircuitGraph &circuit);

private:
    ToolType m_activeTool;
};
