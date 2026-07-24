#pragma once

#include <SFML/Graphics.hpp>

#include "circuits/CircuitGraph.hpp"
#include "core/Mode.hpp"
#include "core/Tools.hpp"
#include "core/World.hpp"
#include "graphing/Grapher.hpp"
#include "logging/Logger.hpp"
#include "render/Renderer.hpp"

// Owns the window and main loop; switches between Fields and Circuits mode, each with its
// own scene, tools, and panel content. Mirrors Schrodinger's Sketchbook's App shell.
class App
{
public:
    App();
    int run();

private:
    void processEvents();
    void update(float frameTime);
    void draw();

    void drawToolsPanel();
    void drawPropertiesPanel();
    void drawSettingsPanel();

    sf::RenderWindow m_window;
    sf::View m_view;
    sf::Clock m_clock;

    Mode m_mode;
    World m_world;
    CircuitGraph m_circuit;
    Tools m_tools;
    Renderer m_renderer;
    Logger m_logger;
    Grapher m_grapher;

    float m_accumulatedZoom;
    sf::Vector2f m_lastMousePos;
    bool m_isPanning;
    bool m_paused;

    float m_accumulator;
    float m_lastFrameTime;
};
