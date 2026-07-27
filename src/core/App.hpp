#pragma once

#include <SFML/Graphics.hpp>

#include <functional>
#include <map>
#include <string>

#include "circuits/CircuitGraph.hpp"
#include "core/Mode.hpp"
#include "core/Tools.hpp"
#include "core/World.hpp"
#include "graphing/Grapher.hpp"
#include "logging/Logger.hpp"
#include "math/Vec2.hpp"
#include "render/Renderer.hpp"

// Owns the window and main loop; switches between Fields and Circuits mode, each with its
// own scene, tools, and panel content.
class App
{
public:
    App();
    int run();

private:
    void processEvents();
    void update(float frameTime);
    void draw();

    // Stats/Tools/Tool Settings are fixed-position and recomputed every frame (not
    // ImGui-ini-persisted). Properties only draws once something is selectable. Settings
    // is an Esc-toggled modal.
    void drawStatsPanel();
    float drawToolsPanel();
    void drawToolSettingsPanel(float toolsPanelWidth);
    void drawPropertiesPanel();
    void drawCircuitPropertiesPanel();
    void drawSettingsPanel();

    void beginGrab(const Vec2 &pos);
    void selectAt(const Vec2 &pos);
    void beginCircuitGrab(const Vec2 &pos);
    void selectCircuitAt(const Vec2 &pos);

    // Draws a "Graph" checkbox (ImGui::SameLine of the caller's own stat line) that toggles
    // whether `quantityName` is logged/graphed; `valueGetter` re-reads its live value each
    // tick by id, so it stays valid even if the entity is later moved or erased.
    void drawGraphToggle(const std::string &quantityName, const std::string &imguiId, const std::function<double()> &valueGetter);

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

    std::map<std::string, sf::Texture> m_toolTextures;

    float m_accumulatedZoom;
    sf::Vector2f m_lastMousePos;
    Vec2 m_mouseWorldMeters; // continuously updated cursor position in world meters, for FieldProbe
    bool m_isPanning;
    bool m_paused;
    bool m_settingsOpen;

    EntityKind m_grabbedKind;
    int m_grabbedId;
    EntityKind m_selectedKind;
    int m_selectedId;

    CircuitEntityKind m_circuitGrabbedKind;
    int m_circuitGrabbedId;
    CircuitEntityKind m_circuitSelectedKind;
    int m_circuitSelectedId;

    float m_accumulator;
    float m_lastFrameTime;
};
