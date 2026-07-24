#pragma once

#include <SFML/Graphics.hpp>

#include <map>
#include <string>

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

    // Stats and Tools are always visible and immovable, position recomputed every frame
    // (not left to ImGui's ini-persisted layout). Tool Settings sits right beside Tools
    // at the same top Y, also recomputed every frame. Properties only draws when
    // something is selected (nothing is selectable yet - Phase 2+). Settings is an
    // Esc-toggled modal.
    void drawStatsPanel();
    float drawToolsPanel();
    void drawToolSettingsPanel(float toolsPanelWidth);
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

    // Tool icon textures, keyed by name (e.g. "cursor", "hand"). Only tools with a
    // directly-applicable icon get one; everything else renders as a plain square
    // placeholder until real per-tool icons are designed (see PLAN.md).
    std::map<std::string, sf::Texture> m_toolTextures;

    float m_accumulatedZoom;
    sf::Vector2f m_lastMousePos;
    bool m_isPanning;
    bool m_paused;
    bool m_settingsOpen;

    float m_accumulator;
    float m_lastFrameTime;
};
