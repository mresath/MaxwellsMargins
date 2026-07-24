#pragma once

#include <SFML/Graphics.hpp>

#include "core/Mode.hpp"

class World;
class CircuitGraph;

// Draws the grid, and mode-dependent content: field vectors/lines/equipotentials/Gaussian
// surfaces for Fields mode, or the schematic (wires/components/current-flow animation)
// for Circuits mode. Toggle flags below control the field-line/vector overlays.
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
};
