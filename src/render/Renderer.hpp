#pragma once

#include <SFML/Graphics.hpp>

#include <optional>
#include <utility>
#include <vector>

#include "core/Mode.hpp"
#include "math/Vec2.hpp"

class World;
class CircuitGraph;
class PointCharge;
class GaussianSurface;
class ChargedParticle;
class CurrentWire;
class MovingLoop;

// Which way charge carriers are animated flowing through wires/loops (a rendering choice
// only). Shared across magnetism, induction, and (once built) circuits.
enum class CurrentFlowDisplay
{
    Off,
    Conventional, // positive-charge-color markers moving in the signed-current direction
    Electron      // negative-charge-color markers moving the opposite way
};

// Draws the grid, then mode-dependent content (Fields: charges/fields/wires/particles/
// Gaussian surfaces; Circuits: schematic). Toggle flags below gate the field overlays.
class Renderer
{
public:
    Renderer();

    // wirePreview, when set, is the in-progress PlaceCurrentWire drag (start, current
    // cursor position), drawn even before the wire actually exists in World.
    void draw(sf::RenderWindow &window, Mode mode, const World &world, const CircuitGraph &circuit,
              const std::optional<std::pair<Vec2, Vec2>> &wirePreview = std::nullopt) const;

    bool showFieldVectors = true;
    bool showFieldLines = true;
    bool showEquipotentials = true;
    bool showMagneticField = true;
    CurrentFlowDisplay currentFlowDisplay = CurrentFlowDisplay::Conventional;

private:
    void drawGridlines(sf::RenderWindow &window) const;
    void drawWorld(sf::RenderWindow &window, const World &world, const std::optional<std::pair<Vec2, Vec2>> &wirePreview) const;
    void drawCircuit(sf::RenderWindow &window, const CircuitGraph &circuit) const;

    void drawCharges(sf::RenderWindow &window, const std::vector<PointCharge> &charges) const;
    void drawFieldVectors(sf::RenderWindow &window, const std::vector<PointCharge> &charges) const;
    void drawFieldLines(sf::RenderWindow &window, const std::vector<PointCharge> &charges) const;
    void drawEquipotentials(sf::RenderWindow &window, const std::vector<PointCharge> &charges) const;
    void drawGaussianSurfaces(sf::RenderWindow &window, const std::vector<GaussianSurface> &surfaces, const std::vector<PointCharge> &charges) const;

    void drawMagneticField(sf::RenderWindow &window, const World &world) const;
    void drawWires(sf::RenderWindow &window, const World &world, const std::optional<std::pair<Vec2, Vec2>> &wirePreview) const;
    void drawWireForceReadouts(sf::RenderWindow &window, const World &world) const;
    void drawParticles(sf::RenderWindow &window, const std::vector<ChargedParticle> &particles) const;
    void drawLoops(sf::RenderWindow &window, const World &world) const;
};
