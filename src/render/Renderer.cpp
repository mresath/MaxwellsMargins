#include "render/Renderer.hpp"

#include "Config.hpp"
#include "circuits/CircuitGraph.hpp"
#include "core/World.hpp"

Renderer::Renderer() = default;

void Renderer::draw(sf::RenderWindow &window, Mode mode, const World &world, const CircuitGraph &circuit) const
{
    drawGridlines(window);

    if (mode == Mode::Fields)
        drawWorld(window, world);
    else
        drawCircuit(window, circuit);
}

void Renderer::drawGridlines(sf::RenderWindow &window) const
{
    // TODO(Phase 1): port drawGridlines from NewtonsNotepad/src/main.cpp
    (void)window;
}

void Renderer::drawWorld(sf::RenderWindow &window, const World &world) const
{
    // TODO(Phase 2/3/4): draw charges, field vectors/lines, equipotentials, Gaussian
    // surfaces, wires, particles + trajectory traces, loops.
    (void)window;
    (void)world;
}

void Renderer::drawCircuit(sf::RenderWindow &window, const CircuitGraph &circuit) const
{
    // TODO(Phase 5): draw wires/components with live V/I/R/Q labels + current-flow animation.
    (void)window;
    (void)circuit;
}
