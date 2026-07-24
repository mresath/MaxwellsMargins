#include "core/App.hpp"

#include "Config.hpp"
#include "core/UI.hpp"

#include <imgui-SFML.h>
#include <imgui.h>

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>

namespace
{
const std::string kWindowTitle = "Maxwell's Margins";
}

App::App()
    : m_window(sf::VideoMode(sf::Vector2u(static_cast<unsigned>(DEF_WIDTH), static_cast<unsigned>(DEF_HEIGHT))), sf::String::fromUtf8(kWindowTitle.begin(), kWindowTitle.end())),
      m_view(sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(DEF_WIDTH, DEF_HEIGHT))),
      m_mode(Mode::Fields),
      m_accumulatedZoom(1.0f),
      m_lastMousePos(0.0f, 0.0f),
      m_isPanning(false),
      m_paused(false),
      m_accumulator(0.0f),
      m_lastFrameTime(0.0f)
{
    if (!ImGui::SFML::Init(m_window))
    {
        throw std::runtime_error("Failed to initialize ImGui-SFML.");
    }

    m_window.setFramerateLimit(MAX_FPS);

    // Window/taskbar icon (no-op on macOS, which uses the .app bundle icon instead)
    sf::Image icon;
    if (icon.loadFromFile("assets/logo/logo.png"))
        m_window.setIcon(icon.getSize(), icon.getPixelsPtr());

    m_view.setCenter(sf::Vector2f(DEF_WIDTH / 2.0f, DEF_HEIGHT / 2.0f));
    m_window.setView(m_view);

    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, Y_ITEM_SPACING);

    // TODO(Phase 1): load a default scene and wire up Fields/Circuits mode switching
}

int App::run()
{
    while (m_window.isOpen())
    {
        processEvents();

        const sf::Time dtTime = m_clock.restart();
        const float rawFrameTime = dtTime.asSeconds();
        m_lastFrameTime = rawFrameTime;

        ImGui::SFML::Update(m_window, dtTime);

        if (!m_paused)
        {
            update(std::min(rawFrameTime, MAX_DT));
        }

        draw();
    }

    ImGui::PopStyleVar();
    ImGui::SFML::Shutdown();
    return 0;
}

void App::processEvents()
{
    // TODO(Phase 1): mirror event handling from Newton's Notepad / Schrodinger's Sketchbook
    // (resize, pan/zoom, tool clicks routed to m_world or m_circuit depending on m_mode)
    while (const std::optional<sf::Event> event = m_window.pollEvent())
    {
        ImGui::SFML::ProcessEvent(m_window, *event);

        if (event->is<sf::Event::Closed>())
        {
            m_window.close();
        }
    }
}

void App::update(float frameTime)
{
    // TODO(Phase 1+): step m_world or m_circuit (per m_mode) using engine/Solver, then
    // m_logger.record(...) and m_renderer.update(...)
    (void)frameTime;
}

void App::draw()
{
    drawSettingsPanel();
    drawToolsPanel();
    drawPropertiesPanel();

    m_window.clear(BACKGROUND_COLOR);
    // TODO(Phase 1+): m_renderer.draw(m_window, m_mode, m_world, m_circuit);

    ImGui::SFML::Render(m_window);
    m_window.display();
}

void App::drawToolsPanel()
{
    // TODO(Phase 1): tool selection buttons, mode-dependent (see Tools::ToolType)
}

void App::drawPropertiesPanel()
{
    // TODO(Phase 1): selected-entity properties (charge magnitude, R/C/L/EMF, ...) + graph buttons
}

void App::drawSettingsPanel()
{
    // TODO(Phase 1): Fields/Circuits mode tabs, solver dt, presets (see scenes/Presets.hpp)
}
