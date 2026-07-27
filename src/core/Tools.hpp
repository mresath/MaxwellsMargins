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
    PlaceCurrentLoop,
    DrawGaussianSurface,
    FieldProbe,

    // Circuits mode
    PlaceResistor,
    PlaceLightbulb,
    PlaceCapacitor,
    PlaceInductor,
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

    float particleChargeMagnitude() const;
    void setParticleChargeMagnitude(float magnitude);
    bool particleChargePositive() const;
    void setParticleChargePositive(bool positive);
    float particleMass() const;
    void setParticleMass(float mass);
    float particleSpeed() const;
    void setParticleSpeed(float speed);

    float wireCurrent() const;
    void setWireCurrent(float current);

    float loopRadius() const;
    void setLoopRadius(float radius);
    int loopTurns() const;
    void setLoopTurns(int turns);
    float loopAngularVelocity() const;
    void setLoopAngularVelocity(float angularVelocity);

    float currentLoopRadius() const;
    void setCurrentLoopRadius(float radius);
    float currentLoopCurrent() const;
    void setCurrentLoopCurrent(float current);
    int currentLoopTurns() const;
    void setCurrentLoopTurns(int turns);

    // A wire needs two points, so it's placed by click-drag (start on press, end on
    // release) rather than the single-click Tools::onClick path.
    bool isDraggingWire() const;
    Vec2 wireDragStart() const;
    void beginWireDrag(const Vec2 &worldPos);
    void finishWireDrag(const Vec2 &worldPos, World &world);

    float resistance() const;
    void setResistance(float resistance);
    float capacitance() const;
    void setCapacitance(float capacitance);
    float inductance() const;
    void setInductance(float inductance);
    float batteryEmf() const;
    void setBatteryEmf(float emf);
    float batteryInternalResistance() const;
    void setBatteryInternalResistance(float resistance);
    bool switchClosed() const;
    void setSwitchClosed(bool closed);

    // Every Circuits component/wire tool places by click-drag (start on press, end on
    // release), same shape as PlaceCurrentWire above - both endpoints snap to the grid.
    bool isDraggingComponent() const;
    Vec2 componentDragStart() const;
    void beginComponentDrag(const Vec2 &worldPos);
    void finishComponentDrag(const Vec2 &worldPos, CircuitGraph &circuit);

private:
    ToolType m_activeTool;
    float m_chargeMagnitude;
    float m_gaussianRadius;

    float m_particleChargeMagnitude;
    bool m_particleChargePositive;
    float m_particleMass;
    float m_particleSpeed;

    float m_wireCurrent;
    bool m_wireDragActive;
    Vec2 m_wireDragStart;

    float m_loopRadius;
    int m_loopTurns;
    float m_loopAngularVelocity;

    float m_currentLoopRadius;
    float m_currentLoopCurrent;
    int m_currentLoopTurns;

    float m_resistance;
    float m_capacitance;
    float m_inductance;
    float m_batteryEmf;
    float m_batteryInternalResistance;
    bool m_switchClosed;

    bool m_componentDragActive;
    Vec2 m_componentDragStart;
};
