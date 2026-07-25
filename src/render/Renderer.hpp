#pragma once

#include <SFML/Graphics.hpp>

#include <vector>

#include "core/Mode.hpp"

class World;
class CircuitGraph;
class PointCharge;
class GaussianSurface;

// Draws the grid, then mode-dependent content (Fields: charges/fields/Gaussian surfaces;
// Circuits: schematic). Toggle flags below gate the field overlays.
class Renderer
{
public:
    Renderer();

    void draw(sf::RenderWindow &window, Mode mode, const World &world, const CircuitGraph &circuit) const;

    bool showFieldVectors = true;
    bool showFieldLines = true;
    bool showEquipotentials = true;
    bool showCurrentFlowAnimation = true;

private:
    void drawGridlines(sf::RenderWindow &window) const;
    void drawWorld(sf::RenderWindow &window, const World &world) const;
    void drawCircuit(sf::RenderWindow &window, const CircuitGraph &circuit) const;

    void drawCharges(sf::RenderWindow &window, const std::vector<PointCharge> &charges) const;
    void drawFieldVectors(sf::RenderWindow &window, const std::vector<PointCharge> &charges) const;
    void drawFieldLines(sf::RenderWindow &window, const std::vector<PointCharge> &charges) const;
    void drawEquipotentials(sf::RenderWindow &window, const std::vector<PointCharge> &charges) const;
    void drawGaussianSurfaces(sf::RenderWindow &window, const std::vector<GaussianSurface> &surfaces, const std::vector<PointCharge> &charges) const;
};
