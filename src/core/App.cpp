#include "core/App.hpp"

#include "Config.hpp"
#include "core/UI.hpp"
#include "electrostatics/FieldSampler.hpp"
#include "math/Util.hpp"
#include "math/Vec2.hpp"
#include "scenes/Presets.hpp"

#include <imgui-SFML.h>
#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
const std::string kWindowTitle = "Maxwell's Margins";

// Stats/Tools: fixed in place, no chrome. Tool Settings: movable, no resize.
constexpr int kFixedFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
constexpr int kMovableFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

const char *toolName(ToolType tool)
{
    switch (tool)
    {
    case ToolType::PlacePositiveCharge:
        return "Place Positive Charge";
    case ToolType::PlaceNegativeCharge:
        return "Place Negative Charge";
    case ToolType::PlaceCurrentWire:
        return "Place Current Wire";
    case ToolType::PlaceChargedParticle:
        return "Place Charged Particle";
    case ToolType::PlaceMovingLoop:
        return "Place Moving Loop";
    case ToolType::DrawGaussianSurface:
        return "Draw Gaussian Surface";
    case ToolType::FieldProbe:
        return "Field Probe";
    case ToolType::PlaceResistor:
        return "Place Resistor";
    case ToolType::PlaceCapacitor:
        return "Place Capacitor";
    case ToolType::PlaceBattery:
        return "Place Battery";
    case ToolType::PlaceSwitch:
        return "Place Switch";
    case ToolType::PlaceWire:
        return "Place Wire";
    case ToolType::PlaceAmmeter:
        return "Place Ammeter";
    case ToolType::PlaceVoltmeter:
        return "Place Voltmeter";
    case ToolType::Move:
        return "Move";
    case ToolType::Erase:
        return "Erase";
    case ToolType::Select:
    default:
        return "Select";
    }
}

const char *iconTextureFor(ToolType tool)
{
    switch (tool)
    {
    case ToolType::Select:
        return "cursor";
    case ToolType::Move:
        return "hand";
    case ToolType::Erase:
        return "trash";
    case ToolType::PlacePositiveCharge:
        return "positive_charge";
    case ToolType::PlaceNegativeCharge:
        return "negative_charge";
    case ToolType::DrawGaussianSurface:
        return "gaussian_surface";
    case ToolType::FieldProbe:
        return "field_probe";
    default:
        return "blank";
    }
}

std::vector<ToolType> toolsForMode(Mode mode)
{
    if (mode == Mode::Fields)
    {
        return {ToolType::Select, ToolType::FieldProbe, ToolType::Move, ToolType::PlacePositiveCharge, ToolType::PlaceNegativeCharge,
                ToolType::PlaceCurrentWire, ToolType::PlaceChargedParticle, ToolType::PlaceMovingLoop,
                ToolType::DrawGaussianSurface, ToolType::Erase};
    }
    return {ToolType::Select, ToolType::Move, ToolType::PlaceResistor, ToolType::PlaceCapacitor, ToolType::PlaceBattery,
            ToolType::PlaceSwitch, ToolType::PlaceWire, ToolType::PlaceAmmeter, ToolType::PlaceVoltmeter, ToolType::Erase};
}
} // namespace

App::App()
    : m_window(sf::VideoMode(sf::Vector2u(static_cast<unsigned>(DEF_WIDTH), static_cast<unsigned>(DEF_HEIGHT))), sf::String::fromUtf8(kWindowTitle.begin(), kWindowTitle.end())),
      m_view(sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(DEF_WIDTH, DEF_HEIGHT))),
      m_mode(Mode::Fields),
      m_accumulatedZoom(1.0f),
      m_lastMousePos(0.0f, 0.0f),
      m_mouseWorldMeters(0.0f, 0.0f),
      m_isPanning(false),
      m_paused(false),
      m_settingsOpen(false),
      m_grabbedKind(EntityKind::None),
      m_grabbedId(-1),
      m_selectedKind(EntityKind::None),
      m_selectedId(-1),
      m_accumulator(0.0f),
      m_lastFrameTime(0.0f)
{
    if (!ImGui::SFML::Init(m_window))
    {
        throw std::runtime_error("Failed to initialize ImGui-SFML.");
    }

    // Panel positions are recomputed every frame, so don't persist/restore a stale layout.
    ImGui::GetIO().IniFilename = nullptr;

    m_window.setFramerateLimit(MAX_FPS);

    for (const char *name : {"cursor", "hand", "trash", "positive_charge", "negative_charge", "gaussian_surface", "field_probe"})
    {
        sf::Texture texture;
        texture.setSmooth(true);
        if (texture.loadFromFile(std::string("assets/") + name + ".png"))
            m_toolTextures.emplace(name, std::move(texture));
    }

    // ImageButton (never plain Button - it sizes differently) keeps every tool's button
    // pixel-identical in size, even ones still using this blank placeholder texture.
    {
        sf::Image blankImage(sf::Vector2u(TOOLS_ICON_SIZE, TOOLS_ICON_SIZE), sf::Color::Transparent);
        sf::Texture blankTexture;
        if (blankTexture.loadFromImage(blankImage))
            m_toolTextures.emplace("blank", std::move(blankTexture));
    }

    // Window/taskbar icon (no-op on macOS, which uses the .app bundle icon instead)
    sf::Image icon;
    if (icon.loadFromFile("assets/logo/logo.png"))
        m_window.setIcon(icon.getSize(), icon.getPixelsPtr());

    m_view.setCenter(sf::Vector2f(DEF_WIDTH / 2.0f, DEF_HEIGHT / 2.0f));
    m_view.setViewport(calculateLetterboxViewport(m_window.getSize(), m_view.getSize()));
    m_window.setView(m_view);

    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, Y_ITEM_SPACING);
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

        if (!m_settingsOpen && !m_paused)
        {
            m_accumulator += std::min(rawFrameTime, MAX_DT);
            const float fixedDt = 1.0f / CALC_FREQ;
            int updatesThisFrame = 0;

            while (m_accumulator >= fixedDt && updatesThisFrame < MAX_UPDATES_PER_FRAME)
            {
                update(fixedDt);
                m_accumulator -= fixedDt;
                ++updatesThisFrame;
            }

            if (updatesThisFrame == MAX_UPDATES_PER_FRAME && m_accumulator >= fixedDt)
            {
                m_accumulator = fixedDt;
            }
        }

        draw();
    }

    ImGui::PopStyleVar();
    ImGui::SFML::Shutdown();
    return 0;
}

void App::processEvents()
{
    while (const std::optional<sf::Event> event = m_window.pollEvent())
    {
        ImGui::SFML::ProcessEvent(m_window, *event);

        sf::View newView(m_window.getView());

        if (event->is<sf::Event::Closed>())
        {
            m_window.close();
        }
        else if (const sf::Event::Resized *resized = event->getIf<sf::Event::Resized>())
        {
            handleResize(&m_window, &newView, &m_view, resized);
        }
        else if (const auto *keyReleased = event->getIf<sf::Event::KeyReleased>())
        {
            if (keyReleased->code == sf::Keyboard::Key::Escape)
            {
                m_settingsOpen = !m_settingsOpen;
            }
            else if (keyReleased->code == sf::Keyboard::Key::P)
            {
                // Pauses independently of the settings modal - does not open it.
                m_paused = !m_paused;
            }
        }
        else if (!ImGui::GetIO().WantCaptureMouse)
        {
            if (const auto *scroll = event->getIf<sf::Event::MouseWheelScrolled>())
            {
                handleZoom(&m_window, &newView, &m_view, scroll->delta, &m_accumulatedZoom);
            }
            else if (const auto *mouseMoved = event->getIf<sf::Event::MouseMoved>())
            {
                const sf::Vector2f worldPixelPos = m_window.mapPixelToCoords(sf::Vector2i(mouseMoved->position), m_view);
                m_mouseWorldMeters = pixelsToMeters(Vec2(worldPixelPos.x, worldPixelPos.y));

                if (m_grabbedKind == EntityKind::Charge)
                {
                    if (PointCharge *charge = m_world.findCharge(m_grabbedId))
                        charge->position = m_mouseWorldMeters;
                }
                else if (m_grabbedKind == EntityKind::GaussianSurface)
                {
                    if (GaussianSurface *surface = m_world.findGaussianSurface(m_grabbedId))
                        surface->center = m_mouseWorldMeters;
                }

                if (m_isPanning)
                {
                    handlePanMouse(&m_window, &newView, &m_view, mouseMoved, m_lastMousePos, m_accumulatedZoom);
                }
            }
            else if (const auto *mouseDown = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (mouseDown->button == sf::Mouse::Button::Middle)
                {
                    newView.setSize(sf::Vector2f(DEF_WIDTH, DEF_HEIGHT));
                    newView.setCenter(sf::Vector2f(DEF_WIDTH * 0.5f, DEF_HEIGHT * 0.5f));
                    m_accumulatedZoom = 1.0f;
                }
                else if (mouseDown->button == sf::Mouse::Button::Right)
                {
                    m_isPanning = true;
                    m_lastMousePos = sf::Vector2f(sf::Mouse::getPosition(m_window));
                }
                else if (mouseDown->button == sf::Mouse::Button::Left)
                {
                    const sf::Vector2f worldPos = m_window.mapPixelToCoords(sf::Vector2i(mouseDown->position), m_view);
                    const Vec2 pos = pixelsToMeters(Vec2(worldPos.x, worldPos.y));
                    if (m_mode == Mode::Fields)
                    {
                        if (m_tools.activeTool() == ToolType::Move)
                            beginGrab(pos);
                        else if (m_tools.activeTool() == ToolType::Select)
                            selectAt(pos);
                        else
                            m_tools.onClick(pos, m_world);
                    }
                    else
                    {
                        m_tools.onClick(pos, m_circuit);
                    }
                }
            }
        }

        // Outside the WantCaptureMouse gate so a release over an ImGui panel still stops
        // panning/dragging, instead of leaving it stuck following the mouse.
        if (const auto *mouseUp = event->getIf<sf::Event::MouseButtonReleased>())
        {
            if (mouseUp->button == sf::Mouse::Button::Right)
                m_isPanning = false;
            else if (mouseUp->button == sf::Mouse::Button::Left)
                m_grabbedKind = EntityKind::None;
        }

        m_view = newView;
        m_window.setView(m_view);
    }
}

void App::beginGrab(const Vec2 &pos)
{
    const EntityRef hit = m_world.findEntityAt(pos);
    m_grabbedKind = hit.kind;
    m_grabbedId = hit.id;

    // Matches Newton's Notepad: grabbing an object with Move also selects it.
    if (hit.kind != EntityKind::None)
    {
        m_selectedKind = hit.kind;
        m_selectedId = hit.id;
    }
}

void App::selectAt(const Vec2 &pos)
{
    const EntityRef hit = m_world.findEntityAt(pos);
    m_selectedKind = hit.kind;
    m_selectedId = hit.id;
}

void App::update(float dt)
{
    if (m_mode == Mode::Fields)
        m_world.update(dt);
    else
        m_circuit.update(dt);
}

void App::draw()
{
    drawStatsPanel();
    const float toolsWidth = drawToolsPanel();
    drawToolSettingsPanel(toolsWidth);
    drawPropertiesPanel();
    if (m_settingsOpen)
        drawSettingsPanel();

    m_window.clear(BACKGROUND_COLOR);
    m_renderer.draw(m_window, m_mode, m_world, m_circuit);

    ImGui::SFML::Render(m_window);
    m_window.display();
}

void App::drawStatsPanel()
{
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_window.getSize().x) * 0.5f, 10.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::Begin("Stats", nullptr, kFixedFlags);

    if (!m_settingsOpen && !m_paused)
    {
        const float fps = m_lastFrameTime > 0.0f ? (1.0f / m_lastFrameTime) : 0.0f;
        ImGui::Text("FPS: %.1f  |  Calc Freq: %.0f Hz", fps, CALC_FREQ);
    }
    else
    {
        const std::string pauseReason = m_settingsOpen ? "Settings Open" : "User Paused";
        const std::string pauseText = "Paused: " + pauseReason;
        const float statsWidth = ImGui::GetWindowSize().x;
        const float pauseTextWidth = ImGui::CalcTextSize(pauseText.c_str()).x;
        ImGui::SetCursorPosX((statsWidth - pauseTextWidth) * 0.5f);
        ImGui::Text("%s", pauseText.c_str());
    }

    ImGui::End();
}

float App::drawToolsPanel()
{
    ImGui::SetNextWindowPos(ImVec2(10.0f, 40.0f), ImGuiCond_Always);
    ImGui::Begin("Tools", nullptr, kFixedFlags);

    const sf::Vector2f iconSize(static_cast<float>(TOOLS_ICON_SIZE), static_cast<float>(TOOLS_ICON_SIZE));

    for (ToolType tool : toolsForMode(m_mode))
    {
        const bool selected = (tool == m_tools.activeTool());
        const sf::Color bg = selected ? TOOL_SELECT_COLOR : TOOL_BG_COLOR;

        const auto it = m_toolTextures.find(iconTextureFor(tool));
        if (it == m_toolTextures.end())
            continue;

        if (ImGui::ImageButton(toolName(tool), it->second, iconSize, bg))
            m_tools.setActiveTool(tool);
    }

    const float width = ImGui::GetWindowSize().x;
    ImGui::End();
    return width;
}

void App::drawToolSettingsPanel(float toolsPanelWidth)
{
    ImGui::SetNextWindowPos(ImVec2(10.0f + toolsPanelWidth + 10.0f, 40.0f), ImGuiCond_Always);
    ImGui::Begin("Tool Settings", nullptr, kMovableFlags);
    ImGui::Text("%s Tool", toolName(m_tools.activeTool()));
    ImGui::Separator();

    const ToolType tool = m_tools.activeTool();
    if (m_mode == Mode::Fields && (tool == ToolType::PlacePositiveCharge || tool == ToolType::PlaceNegativeCharge))
    {
        ImGui::Text("Left Click to place a charge");
        ImGui::Separator();
        float magnitude = m_tools.chargeMagnitude();
        if (ImGui::DragFloat("Magnitude (C)", &magnitude, CHARGE_MAGNITUDE_STEP, MIN_CHARGE_MAGNITUDE, MAX_CHARGE_MAGNITUDE, "%.3g"))
            m_tools.setChargeMagnitude(magnitude);
    }
    else if (m_mode == Mode::Fields && tool == ToolType::DrawGaussianSurface)
    {
        ImGui::Text("Left Click to place a Gaussian surface");
        ImGui::Separator();
        float radius = m_tools.gaussianRadius();
        if (ImGui::DragFloat("Radius (m)", &radius, GAUSSIAN_SURFACE_RADIUS_STEP, MIN_GAUSSIAN_SURFACE_RADIUS, MAX_GAUSSIAN_SURFACE_RADIUS, "%.2f"))
            m_tools.setGaussianRadius(radius);
    }
    else if (m_mode == Mode::Fields && tool == ToolType::FieldProbe)
    {
        const FieldSampler sampler;
        const Vec2 field = sampler.fieldAt(m_mouseWorldMeters, m_world.charges());
        const float potential = sampler.potentialAt(m_mouseWorldMeters, m_world.charges());
        constexpr float kRadToDeg = 180.0f / 3.14159265358979f;

        ImGui::Text("Hover to read the field/potential at the cursor");
        ImGui::Separator();
        ImGui::Text("Position: %s m", m_mouseWorldMeters.toString().c_str());
        ImGui::Text("Potential: %.4g V", potential);
        ImGui::Text("Field: %.4g N/C @ %.1f deg", field.length(), field.angle() * kRadToDeg);
    }
    else if (tool == ToolType::Move)
    {
        ImGui::Text("Left Click and drag to move a charge or Gaussian surface");
    }
    else if (tool == ToolType::Erase)
    {
        ImGui::Text("Left Click to erase a charge or Gaussian surface");
    }
    else if (tool == ToolType::Select)
    {
        ImGui::Text("Left Click to select a charge or Gaussian surface");
    }
    else
    {
        ImGui::TextDisabled("(settings added per-phase, see PLAN.md)");
    }

    ImGui::End();
}

void App::drawPropertiesPanel()
{
    if (m_mode != Mode::Fields || m_selectedKind == EntityKind::None)
        return;

    if (m_selectedKind == EntityKind::Charge)
    {
        PointCharge *charge = m_world.findCharge(m_selectedId);
        if (!charge)
        {
            m_selectedKind = EntityKind::None;
            return;
        }

        ImGui::SetNextWindowPos(ImVec2(10.0f, 300.0f), ImGuiCond_Once);
        ImGui::Begin("Properties", nullptr, kMovableFlags);
        ImGui::Text("Point Charge");
        ImGui::Separator();

        bool positive = charge->charge >= 0.0f;
        float magnitude = std::abs(charge->charge);
        if (ImGui::RadioButton("Positive", positive))
            charge->charge = magnitude;
        ImGui::SameLine();
        if (ImGui::RadioButton("Negative", !positive))
            charge->charge = -magnitude;
        if (ImGui::DragFloat("Magnitude (C)", &magnitude, CHARGE_MAGNITUDE_STEP, MIN_CHARGE_MAGNITUDE, MAX_CHARGE_MAGNITUDE, "%.3g"))
            charge->charge = positive ? magnitude : -magnitude;
        ImGui::Text("Position: %s m", charge->position.toString().c_str());

        ImGui::End();
    }
    else if (m_selectedKind == EntityKind::GaussianSurface)
    {
        GaussianSurface *surface = m_world.findGaussianSurface(m_selectedId);
        if (!surface)
        {
            m_selectedKind = EntityKind::None;
            return;
        }

        ImGui::SetNextWindowPos(ImVec2(10.0f, 300.0f), ImGuiCond_Once);
        ImGui::Begin("Properties", nullptr, kMovableFlags);
        ImGui::Text("Gaussian Surface");
        ImGui::Separator();

        ImGui::DragFloat("Radius (m)", &surface->radius, GAUSSIAN_SURFACE_RADIUS_STEP, MIN_GAUSSIAN_SURFACE_RADIUS, MAX_GAUSSIAN_SURFACE_RADIUS, "%.2f");
        ImGui::Text("Center: %s m", surface->center.toString().c_str());
        ImGui::Text("Enclosed charge: %.3g C", surface->enclosedCharge(m_world.charges()));

        ImGui::End();
    }
}

void App::drawSettingsPanel()
{
    ImGui::Begin("Simulation Settings", &m_settingsOpen, kMovableFlags | ImGuiWindowFlags_NoMove);

    const bool fieldsMode = (m_mode == Mode::Fields);
    if (ImGui::RadioButton("Fields", fieldsMode))
        m_mode = Mode::Fields;
    ImGui::SameLine();
    if (ImGui::RadioButton("Circuits", !fieldsMode))
        m_mode = Mode::Circuits;

    if (fieldsMode)
    {
        ImGui::Separator();
        ImGui::Checkbox("Field Vectors", &m_renderer.showFieldVectors);
        ImGui::Checkbox("Field Lines", &m_renderer.showFieldLines);
        ImGui::Separator();
        if (ImGui::Button("Load Dipole Field Preset"))
            Presets::loadDipoleField(m_world);
    }

    const ImVec2 settingsWindowSize = ImGui::GetWindowSize();
    ImGui::SetWindowPos(ImVec2((static_cast<float>(m_window.getSize().x) - settingsWindowSize.x) * 0.5f, (static_cast<float>(m_window.getSize().y) - settingsWindowSize.y) * 0.5f), ImGuiCond_Always);

    ImGui::End();
}
