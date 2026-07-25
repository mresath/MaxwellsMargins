#pragma once

#include "math/Vec2.hpp"

class World;
class CircuitGraph;

// Routed to the active World or CircuitGraph depending on the current Mode.
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

    // Shared (camera pan is unconditional via right-mouse-drag, not a tool - see core/App)
    Select,
    Move,
    Erase
};

class Tools
{
public:
    Tools();

    ToolType activeTool() const;
    void setActiveTool(ToolType tool);

    void onClick(const Vec2 &worldPos, World &world);
    void onClick(const Vec2 &worldPos, CircuitGraph &circuit);

    // Applied to the next placed entity, not retroactive to already-placed ones.
    float chargeMagnitude() const;
    void setChargeMagnitude(float magnitude);
    float gaussianRadius() const;
    void setGaussianRadius(float radius);

private:
    ToolType m_activeTool;
    float m_chargeMagnitude;
    float m_gaussianRadius;
};
