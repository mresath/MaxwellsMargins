#include "core/App.hpp"

#include "Config.hpp"
#include "circuits/Battery.hpp"
#include "circuits/Capacitor.hpp"
#include "circuits/Inductor.hpp"
#include "circuits/Lightbulb.hpp"
#include "circuits/Probe.hpp"
#include "circuits/Resistor.hpp"
#include "circuits/Switch.hpp"
#include "core/UI.hpp"
#include "math/Util.hpp"
#include "math/Vec2.hpp"
#include "scenes/Presets.hpp"

#include <imgui-SFML.h>
#include <imgui.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <ctime>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
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
    case ToolType::PlaceCurrentLoop:
        return "Place Current Loop";
    case ToolType::DrawGaussianSurface:
        return "Draw Gaussian Surface";
    case ToolType::FieldProbe:
        return "Field Probe";
    case ToolType::PlaceResistor:
        return "Place Resistor";
    case ToolType::PlaceLightbulb:
        return "Place Lightbulb";
    case ToolType::PlaceCapacitor:
        return "Place Capacitor";
    case ToolType::PlaceInductor:
        return "Place Inductor";
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
    case ToolType::PlaceChargedParticle:
        return "charged_particle";
    case ToolType::PlaceCurrentWire:
        return "current_wire";
    case ToolType::PlaceMovingLoop:
        return "moving_loop";
    case ToolType::PlaceCurrentLoop:
        return "current_loop";
    case ToolType::PlaceResistor:
        return "resistor";
    case ToolType::PlaceLightbulb:
        return "lightbulb";
    case ToolType::PlaceCapacitor:
        return "capacitor";
    case ToolType::PlaceInductor:
        return "inductor";
    case ToolType::PlaceBattery:
        return "battery";
    case ToolType::PlaceSwitch:
        return "switch";
    case ToolType::PlaceWire:
        return "wire";
    case ToolType::PlaceAmmeter:
        return "ammeter";
    case ToolType::PlaceVoltmeter:
        return "voltmeter";
    default:
        return "blank";
    }
}

// Canonical name for a graphable quantity - doubles as both the Grapher's selection key and
// the exported plot's legend label, so it needs no separate units-lookup table.
std::string quantityLabel(const std::string &type, int id, const std::string &property, const std::string &unit)
{
    return type + " #" + std::to_string(id) + " - " + property + " (" + unit + ")";
}

std::string timestampedFilename(const std::string &directory, const std::string &prefix, const std::string &extension)
{
    return directory + "/" + prefix + "_" + std::to_string(std::time(nullptr)) + "." + extension;
}

std::vector<ToolType> toolsForMode(Mode mode)
{
    if (mode == Mode::Fields)
    {
        return {ToolType::Select, ToolType::FieldProbe, ToolType::Move, ToolType::PlacePositiveCharge, ToolType::PlaceNegativeCharge,
                ToolType::PlaceCurrentWire, ToolType::PlaceChargedParticle, ToolType::PlaceMovingLoop, ToolType::PlaceCurrentLoop,
                ToolType::DrawGaussianSurface, ToolType::Erase};
    }
    // Ordered by expected frequency of use: wiring, switches, and the power source first
    // (needed in nearly every circuit), then the two probes, then passive components from
    // most to least common.
    return {ToolType::Select, ToolType::Move, ToolType::PlaceWire, ToolType::PlaceSwitch, ToolType::PlaceBattery, ToolType::PlaceAmmeter, ToolType::PlaceVoltmeter,
            ToolType::PlaceResistor, ToolType::PlaceLightbulb, ToolType::PlaceCapacitor, ToolType::PlaceInductor, ToolType::Erase};
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
      m_circuitGrabbedKind(CircuitEntityKind::None),
      m_circuitGrabbedId(-1),
      m_circuitSelectedKind(CircuitEntityKind::None),
      m_circuitSelectedId(-1),
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

    for (const char *name : {"cursor", "hand", "trash", "positive_charge", "negative_charge", "gaussian_surface", "field_probe", "charged_particle", "current_wire", "moving_loop", "current_loop",
                             "resistor", "lightbulb", "capacitor", "inductor", "battery", "switch", "wire", "ammeter", "voltmeter"})
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

        if (!m_settingsOpen && !m_paused && !m_grapher.isGraphOpen())
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
                // A graph closes on Escape before the settings modal would toggle, so it
                // takes exactly one press to get back to the canvas from either state.
                if (m_grapher.isGraphOpen())
                    m_grapher.closeGraph();
                else
                    m_settingsOpen = !m_settingsOpen;
            }
            else if (keyReleased->code == sf::Keyboard::Key::G)
            {
                if (!m_settingsOpen)
                    m_grapher.toggleGraph(m_logger);
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
                const Vec2 previousMouseWorldMeters = m_mouseWorldMeters;
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
                else if (m_grabbedKind == EntityKind::Particle)
                {
                    if (ChargedParticle *particle = m_world.findParticle(m_grabbedId))
                        particle->position = m_mouseWorldMeters;
                }
                else if (m_grabbedKind == EntityKind::Wire)
                {
                    // A wire has two points, so Move translates both by the cursor's
                    // frame-to-frame delta instead of snapping a single point to it.
                    if (CurrentWire *wire = m_world.findWire(m_grabbedId))
                    {
                        const Vec2 delta = m_mouseWorldMeters - previousMouseWorldMeters;
                        wire->start += delta;
                        wire->end += delta;
                    }
                }
                else if (m_grabbedKind == EntityKind::Loop)
                {
                    if (MovingLoop *loop = m_world.findLoop(m_grabbedId))
                        loop->center = m_mouseWorldMeters;
                }
                else if (m_grabbedKind == EntityKind::CurrentLoop)
                {
                    if (CurrentLoop *loop = m_world.findCurrentLoop(m_grabbedId))
                        loop->center = m_mouseWorldMeters;
                }
                else if (m_circuitGrabbedKind == CircuitEntityKind::Component)
                {
                    // Two terminals, so Move translates both by the cursor's frame-to-frame
                    // delta instead of snapping a single point to it (snapped to grid on release).
                    if (Component *comp = m_circuit.findComponent(m_circuitGrabbedId))
                    {
                        const Vec2 delta = m_mouseWorldMeters - previousMouseWorldMeters;
                        comp->posA += delta;
                        comp->posB += delta;
                    }
                }
                else if (m_circuitGrabbedKind == CircuitEntityKind::Wire)
                {
                    if (CircuitWire *wire = m_circuit.findWire(m_circuitGrabbedId))
                    {
                        const Vec2 delta = m_mouseWorldMeters - previousMouseWorldMeters;
                        wire->start += delta;
                        wire->end += delta;
                    }
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
                        else if (m_tools.activeTool() == ToolType::PlaceCurrentWire)
                            m_tools.beginWireDrag(pos);
                        else
                            m_tools.onClick(pos, m_world);
                    }
                    else
                    {
                        if (m_tools.activeTool() == ToolType::Move)
                            beginCircuitGrab(pos);
                        else if (m_tools.activeTool() == ToolType::Select)
                            selectCircuitAt(pos);
                        else if (m_tools.activeTool() == ToolType::Erase)
                            m_tools.onClick(pos, m_circuit);
                        else
                            m_tools.beginComponentDrag(pos);
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
            {
                if (m_mode == Mode::Fields && m_tools.activeTool() == ToolType::PlaceCurrentWire && m_tools.isDraggingWire())
                    m_tools.finishWireDrag(m_mouseWorldMeters, m_world);
                else if (m_mode == Mode::Circuits && m_tools.isDraggingComponent())
                    m_tools.finishComponentDrag(m_mouseWorldMeters, m_circuit);

                if (m_circuitGrabbedKind == CircuitEntityKind::Component)
                {
                    if (Component *comp = m_circuit.findComponent(m_circuitGrabbedId))
                    {
                        comp->posA = snapToGrid(comp->posA);
                        comp->posB = snapToGrid(comp->posB);
                    }
                }
                else if (m_circuitGrabbedKind == CircuitEntityKind::Wire)
                {
                    if (CircuitWire *wire = m_circuit.findWire(m_circuitGrabbedId))
                    {
                        wire->start = snapToGrid(wire->start);
                        wire->end = snapToGrid(wire->end);
                    }
                }

                m_grabbedKind = EntityKind::None;
                m_circuitGrabbedKind = CircuitEntityKind::None;
            }
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

    // Grabbing an object with Move also selects it.
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

void App::beginCircuitGrab(const Vec2 &pos)
{
    const CircuitEntityRef hit = m_circuit.findEntityAt(pos);
    m_circuitGrabbedKind = hit.kind;
    m_circuitGrabbedId = hit.id;

    if (hit.kind != CircuitEntityKind::None)
    {
        m_circuitSelectedKind = hit.kind;
        m_circuitSelectedId = hit.id;
    }
}

void App::selectCircuitAt(const Vec2 &pos)
{
    const CircuitEntityRef hit = m_circuit.findEntityAt(pos);
    m_circuitSelectedKind = hit.kind;
    m_circuitSelectedId = hit.id;
}

void App::drawGraphToggle(const std::string &quantityName, const std::string &imguiId, const std::function<double()> &valueGetter)
{
    ImGui::SameLine();
    bool graphed = m_grapher.isSelected(quantityName);
    if (ImGui::Checkbox(("Graph##" + imguiId).c_str(), &graphed))
    {
        if (graphed)
            m_grapher.select(quantityName, valueGetter);
        else
            m_grapher.deselect(quantityName);
    }
}

void App::update(float dt)
{
    if (m_mode == Mode::Fields)
        m_world.update(dt);
    else
        m_circuit.update(dt);

    if (m_grapher.hasSelection())
    {
        const float simTime = (m_mode == Mode::Fields) ? m_world.simTime() : m_circuit.simTime();
        m_logger.record(simTime, m_grapher.sample());
    }
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

    std::optional<std::pair<Vec2, Vec2>> dragPreview;
    if (m_mode == Mode::Fields && m_tools.isDraggingWire())
        dragPreview = std::make_pair(m_tools.wireDragStart(), m_mouseWorldMeters);
    else if (m_mode == Mode::Circuits && m_tools.isDraggingComponent())
        dragPreview = std::make_pair(m_tools.componentDragStart(), snapToGrid(m_mouseWorldMeters));
    m_renderer.draw(m_window, m_mode, m_world, m_circuit, dragPreview);

    ImGui::SFML::Render(m_window);
    m_window.display();
}

void App::drawStatsPanel()
{
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_window.getSize().x) * 0.5f, 10.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::Begin("Stats", nullptr, kFixedFlags);

    if (!m_settingsOpen && !m_paused && !m_grapher.isGraphOpen())
    {
        const float fps = m_lastFrameTime > 0.0f ? (1.0f / m_lastFrameTime) : 0.0f;
        ImGui::Text("FPS: %.1f  |  Calc Freq: %.0f Hz", fps, CALC_FREQ);
    }
    else
    {
        const std::string pauseReason = m_settingsOpen ? "Settings Open" : (m_grapher.isGraphOpen() ? "Graph Open" : "User Paused");
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
        ImGui::SameLine();
        if (ImGui::Button("e"))
            m_tools.setChargeMagnitude(ELEMENTARY_CHARGE);
    }
    else if (m_mode == Mode::Fields && tool == ToolType::DrawGaussianSurface)
    {
        ImGui::Text("Left Click to place a Gaussian surface");
        ImGui::Separator();
        float radius = m_tools.gaussianRadius();
        if (ImGui::DragFloat("Radius (m)", &radius, GAUSSIAN_SURFACE_RADIUS_STEP, MIN_GAUSSIAN_SURFACE_RADIUS, MAX_GAUSSIAN_SURFACE_RADIUS, "%.2f"))
            m_tools.setGaussianRadius(radius);
    }
    else if (m_mode == Mode::Fields && tool == ToolType::PlaceChargedParticle)
    {
        ImGui::Text("Left Click to place a charged particle");
        ImGui::Separator();

        bool positive = m_tools.particleChargePositive();
        if (ImGui::RadioButton("Positive", positive))
            m_tools.setParticleChargePositive(true);
        ImGui::SameLine();
        if (ImGui::RadioButton("Negative", !positive))
            m_tools.setParticleChargePositive(false);

        float magnitude = m_tools.particleChargeMagnitude();
        if (ImGui::DragFloat("Charge (C)", &magnitude, PARTICLE_CHARGE_MAGNITUDE_STEP, MIN_PARTICLE_CHARGE_MAGNITUDE, MAX_PARTICLE_CHARGE_MAGNITUDE, "%.3g"))
            m_tools.setParticleChargeMagnitude(magnitude);
        ImGui::SameLine();
        if (ImGui::Button("e"))
            m_tools.setParticleChargeMagnitude(ELEMENTARY_CHARGE);

        float mass = m_tools.particleMass();
        if (ImGui::DragFloat("Mass (kg)", &mass, PARTICLE_MASS_STEP, MIN_PARTICLE_MASS, MAX_PARTICLE_MASS, "%.3g"))
            m_tools.setParticleMass(mass);
        ImGui::SameLine();
        if (ImGui::Button("e-"))
            m_tools.setParticleMass(ELECTRON_MASS);
        ImGui::SameLine();
        if (ImGui::Button("p+"))
            m_tools.setParticleMass(PROTON_MASS);

        float speed = m_tools.particleSpeed();
        if (ImGui::DragFloat("Initial Speed (m/s)", &speed, PARTICLE_SPEED_STEP, MIN_PARTICLE_SPEED, MAX_PARTICLE_SPEED, "%.2f"))
            m_tools.setParticleSpeed(speed);
        ImGui::TextDisabled("Launched along +x");
    }
    else if (m_mode == Mode::Fields && tool == ToolType::PlaceCurrentWire)
    {
        ImGui::Text("Left Click and drag to draw a wire");
        ImGui::Separator();
        float current = m_tools.wireCurrent();
        if (ImGui::DragFloat("Current (A)", &current, WIRE_CURRENT_STEP, MIN_WIRE_CURRENT, MAX_WIRE_CURRENT, "%.2f"))
            m_tools.setWireCurrent(current);
    }
    else if (m_mode == Mode::Fields && tool == ToolType::PlaceMovingLoop)
    {
        ImGui::Text("Left Click to place a moving loop");
        ImGui::Separator();

        float radius = m_tools.loopRadius();
        if (ImGui::DragFloat("Radius (m)", &radius, LOOP_RADIUS_STEP, MIN_LOOP_RADIUS, MAX_LOOP_RADIUS, "%.2f"))
            m_tools.setLoopRadius(radius);

        int turns = m_tools.loopTurns();
        if (ImGui::DragInt("Turns", &turns, 1.0f, MIN_LOOP_TURNS, MAX_LOOP_TURNS))
            m_tools.setLoopTurns(turns);

        float angularVelocity = m_tools.loopAngularVelocity();
        if (ImGui::DragFloat("Angular Velocity (rad/s)", &angularVelocity, LOOP_ANGULAR_VELOCITY_STEP, MIN_LOOP_ANGULAR_VELOCITY, MAX_LOOP_ANGULAR_VELOCITY, "%.2f"))
            m_tools.setLoopAngularVelocity(angularVelocity);
        ImGui::TextDisabled("Set translational velocity after placing, via Properties");
    }
    else if (m_mode == Mode::Fields && tool == ToolType::PlaceCurrentLoop)
    {
        ImGui::Text("Left Click to place a stationary current loop");
        ImGui::Separator();

        float radius = m_tools.currentLoopRadius();
        if (ImGui::DragFloat("Radius (m)", &radius, CURRENT_LOOP_RADIUS_STEP, MIN_CURRENT_LOOP_RADIUS, MAX_CURRENT_LOOP_RADIUS, "%.2f"))
            m_tools.setCurrentLoopRadius(radius);

        float current = m_tools.currentLoopCurrent();
        if (ImGui::DragFloat("Current (A)", &current, CURRENT_LOOP_CURRENT_STEP, MIN_CURRENT_LOOP_CURRENT, MAX_CURRENT_LOOP_CURRENT, "%.2f"))
            m_tools.setCurrentLoopCurrent(current);

        int turns = m_tools.currentLoopTurns();
        if (ImGui::DragInt("Turns", &turns, 1.0f, MIN_CURRENT_LOOP_TURNS, MAX_CURRENT_LOOP_TURNS))
            m_tools.setCurrentLoopTurns(turns);
    }
    else if (m_mode == Mode::Fields && tool == ToolType::FieldProbe)
    {
        const Vec2 field = m_world.electricFieldAt(m_mouseWorldMeters);
        const float potential = m_world.electricPotentialAt(m_mouseWorldMeters);
        const float magneticField = m_world.magneticFieldAt(m_mouseWorldMeters);
        constexpr float kRadToDeg = 180.0f / 3.14159265358979f;

        ImGui::Text("Hover to read the field/potential at the cursor");
        ImGui::Separator();
        ImGui::Text("Position: %s m", m_mouseWorldMeters.toString().c_str());
        ImGui::Text("Potential: %.4g V", potential);
        ImGui::Text("E Field: %.4g N/C @ %.1f deg", field.length(), field.angle() * kRadToDeg);
        ImGui::Text("B Field: %.4g T (%s)", magneticField, magneticField >= 0.0f ? "out of page" : "into page");
    }
    else if (m_mode == Mode::Circuits && tool == ToolType::PlaceResistor)
    {
        ImGui::Text("Left Click and drag to place a resistor");
        ImGui::Separator();
        float resistance = m_tools.resistance();
        if (ImGui::DragFloat("Resistance (Ohm)", &resistance, RESISTANCE_STEP, MIN_RESISTANCE, MAX_RESISTANCE, "%.3g"))
            m_tools.setResistance(resistance);
    }
    else if (m_mode == Mode::Circuits && tool == ToolType::PlaceLightbulb)
    {
        ImGui::Text("Left Click and drag to place a lightbulb");
        ImGui::Separator();
        float resistance = m_tools.resistance();
        if (ImGui::DragFloat("Resistance (Ohm)", &resistance, RESISTANCE_STEP, MIN_RESISTANCE, MAX_RESISTANCE, "%.3g"))
            m_tools.setResistance(resistance);
    }
    else if (m_mode == Mode::Circuits && tool == ToolType::PlaceCapacitor)
    {
        ImGui::Text("Left Click and drag to place a capacitor");
        ImGui::Separator();
        float capacitance = m_tools.capacitance();
        if (ImGui::DragFloat("Capacitance (F)", &capacitance, CAPACITANCE_STEP, MIN_CAPACITANCE, MAX_CAPACITANCE, "%.3g"))
            m_tools.setCapacitance(capacitance);
    }
    else if (m_mode == Mode::Circuits && tool == ToolType::PlaceInductor)
    {
        ImGui::Text("Left Click and drag to place an inductor");
        ImGui::Separator();
        float inductance = m_tools.inductance();
        if (ImGui::DragFloat("Inductance (H)", &inductance, INDUCTANCE_STEP, MIN_INDUCTANCE, MAX_INDUCTANCE, "%.3g"))
            m_tools.setInductance(inductance);
    }
    else if (m_mode == Mode::Circuits && tool == ToolType::PlaceBattery)
    {
        ImGui::Text("Left Click and drag to place a battery (start = + terminal)");
        ImGui::Separator();
        float emf = m_tools.batteryEmf();
        if (ImGui::DragFloat("EMF (V)", &emf, EMF_STEP, MIN_EMF, MAX_EMF, "%.2f"))
            m_tools.setBatteryEmf(emf);
        float internalResistance = m_tools.batteryInternalResistance();
        if (ImGui::DragFloat("Internal Resistance (Ohm)", &internalResistance, INTERNAL_RESISTANCE_STEP, MIN_INTERNAL_RESISTANCE, MAX_INTERNAL_RESISTANCE, "%.2f"))
            m_tools.setBatteryInternalResistance(internalResistance);
    }
    else if (m_mode == Mode::Circuits && tool == ToolType::PlaceSwitch)
    {
        ImGui::Text("Left Click and drag to place a switch");
        ImGui::Separator();
        bool closed = m_tools.switchClosed();
        if (ImGui::Checkbox("Closed", &closed))
            m_tools.setSwitchClosed(closed);
    }
    else if (m_mode == Mode::Circuits && tool == ToolType::PlaceWire)
    {
        ImGui::Text("Left Click and drag to draw a wire");
    }
    else if (m_mode == Mode::Circuits && tool == ToolType::PlaceAmmeter)
    {
        ImGui::Text("Left Click and drag to place an ammeter (in series)");
    }
    else if (m_mode == Mode::Circuits && tool == ToolType::PlaceVoltmeter)
    {
        ImGui::Text("Left Click and drag to place a voltmeter (across two points, non-invasive)");
    }
    else if (tool == ToolType::Move)
    {
        if (m_mode == Mode::Fields)
            ImGui::Text("Left Click and drag to move a charge, particle, wire, loop, current loop, or Gaussian surface");
        else
            ImGui::Text("Left Click and drag to move a component or wire");
    }
    else if (tool == ToolType::Erase)
    {
        if (m_mode == Mode::Fields)
            ImGui::Text("Left Click to erase a charge, particle, wire, loop, current loop, or Gaussian surface");
        else
            ImGui::Text("Left Click to erase a component or wire");
    }
    else if (tool == ToolType::Select)
    {
        if (m_mode == Mode::Fields)
            ImGui::Text("Left Click to select a charge, particle, wire, loop, current loop, or Gaussian surface");
        else
            ImGui::Text("Left Click to select a component or wire");
    }
    else
    {
        ImGui::TextDisabled("(settings added per-phase, see PLAN.md)");
    }

    ImGui::End();
}

void App::drawPropertiesPanel()
{
    if (m_mode == Mode::Circuits)
    {
        drawCircuitPropertiesPanel();
        return;
    }

    if (m_selectedKind == EntityKind::None)
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
        ImGui::SameLine();
        if (ImGui::Button("e"))
            charge->charge = positive ? ELEMENTARY_CHARGE : -ELEMENTARY_CHARGE;
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
        drawGraphToggle(quantityLabel("Gaussian Surface", surface->id, "Enclosed Charge", "C"), "gaussQ" + std::to_string(surface->id),
                        [this, id = surface->id]() -> double
                        {
                            GaussianSurface *s = m_world.findGaussianSurface(id);
                            return s ? static_cast<double>(s->enclosedCharge(m_world.charges())) : std::nan("");
                        });

        ImGui::End();
    }
    else if (m_selectedKind == EntityKind::Particle)
    {
        ChargedParticle *particle = m_world.findParticle(m_selectedId);
        if (!particle)
        {
            m_selectedKind = EntityKind::None;
            return;
        }

        ImGui::SetNextWindowPos(ImVec2(10.0f, 300.0f), ImGuiCond_Once);
        ImGui::Begin("Properties", nullptr, kMovableFlags);
        ImGui::Text("Charged Particle");
        ImGui::Separator();

        bool positive = particle->charge >= 0.0f;
        float magnitude = std::abs(particle->charge);
        if (ImGui::RadioButton("Positive", positive))
            particle->charge = magnitude;
        ImGui::SameLine();
        if (ImGui::RadioButton("Negative", !positive))
            particle->charge = -magnitude;
        if (ImGui::DragFloat("Charge (C)", &magnitude, PARTICLE_CHARGE_MAGNITUDE_STEP, MIN_PARTICLE_CHARGE_MAGNITUDE, MAX_PARTICLE_CHARGE_MAGNITUDE, "%.3g"))
            particle->charge = positive ? magnitude : -magnitude;
        ImGui::SameLine();
        if (ImGui::Button("e"))
            particle->charge = positive ? ELEMENTARY_CHARGE : -ELEMENTARY_CHARGE;

        ImGui::DragFloat("Mass (kg)", &particle->mass, PARTICLE_MASS_STEP, MIN_PARTICLE_MASS, MAX_PARTICLE_MASS, "%.3g");
        ImGui::SameLine();
        if (ImGui::Button("e-"))
            particle->mass = ELECTRON_MASS;
        ImGui::SameLine();
        if (ImGui::Button("p+"))
            particle->mass = PROTON_MASS;

        float velocity[2] = {particle->velocity.x, particle->velocity.y};
        if (ImGui::DragFloat2("Velocity (m/s)", velocity, 0.1f))
            particle->velocity = Vec2(velocity[0], velocity[1]);

        ImGui::Text("Position: %s m", particle->position.toString().c_str());
        ImGui::Text("Speed: %.4g m/s", particle->velocity.length());
        drawGraphToggle(quantityLabel("Particle", particle->id, "Speed", "m/s"), "particleSpeed" + std::to_string(particle->id),
                        [this, id = particle->id]() -> double
                        {
                            ChargedParticle *p = m_world.findParticle(id);
                            return p ? static_cast<double>(p->velocity.length()) : std::nan("");
                        });
        ImGui::Checkbox("Trajectory Trace", &particle->trajectoryTraceEnabled);
        ImGui::SameLine();
        if (ImGui::Button("Clear"))
            particle->trajectoryTrace.clear();

        ImGui::End();
    }
    else if (m_selectedKind == EntityKind::Wire)
    {
        CurrentWire *wire = m_world.findWire(m_selectedId);
        if (!wire)
        {
            m_selectedKind = EntityKind::None;
            return;
        }

        ImGui::SetNextWindowPos(ImVec2(10.0f, 300.0f), ImGuiCond_Once);
        ImGui::Begin("Properties", nullptr, kMovableFlags);
        ImGui::Text("Current Wire");
        ImGui::Separator();

        ImGui::DragFloat("Current (A)", &wire->current, WIRE_CURRENT_STEP, MIN_WIRE_CURRENT, MAX_WIRE_CURRENT, "%.2f");
        ImGui::Text("Start: %s m", wire->start.toString().c_str());
        ImGui::Text("End: %s m", wire->end.toString().c_str());

        ImGui::End();
    }
    else if (m_selectedKind == EntityKind::Loop)
    {
        MovingLoop *loop = m_world.findLoop(m_selectedId);
        if (!loop)
        {
            m_selectedKind = EntityKind::None;
            return;
        }

        ImGui::SetNextWindowPos(ImVec2(10.0f, 300.0f), ImGuiCond_Once);
        ImGui::Begin("Properties", nullptr, kMovableFlags);
        ImGui::Text("Moving Loop");
        ImGui::Separator();

        ImGui::DragFloat("Radius (m)", &loop->radius, LOOP_RADIUS_STEP, MIN_LOOP_RADIUS, MAX_LOOP_RADIUS, "%.2f");
        ImGui::DragInt("Turns", &loop->turns, 1.0f, MIN_LOOP_TURNS, MAX_LOOP_TURNS);

        float velocity[2] = {loop->velocity.x, loop->velocity.y};
        if (ImGui::DragFloat2("Velocity (m/s)", velocity, 0.1f))
            loop->velocity = Vec2(velocity[0], velocity[1]);
        ImGui::DragFloat("Angular Velocity (rad/s)", &loop->angularVelocity, LOOP_ANGULAR_VELOCITY_STEP, MIN_LOOP_ANGULAR_VELOCITY, MAX_LOOP_ANGULAR_VELOCITY, "%.2f");

        ImGui::Text("Position: %s m", loop->center.toString().c_str());
        ImGui::Text("Induced EMF: %.4g V", loop->inducedEMF);
        drawGraphToggle(quantityLabel("Moving Loop", loop->id, "Induced EMF", "V"), "loopEMF" + std::to_string(loop->id),
                        [this, id = loop->id]() -> double
                        {
                            MovingLoop *l = m_world.findLoop(id);
                            return l ? static_cast<double>(l->inducedEMF) : std::nan("");
                        });

        if (!loop->emfTrace.empty())
        {
            const std::vector<float> trace(loop->emfTrace.begin(), loop->emfTrace.end());
            ImGui::PlotLines("EMF (V)", trace.data(), static_cast<int>(trace.size()), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(0.0f, 60.0f));
        }
        if (ImGui::Button("Clear EMF Trace"))
            loop->emfTrace.clear();

        ImGui::End();
    }
    else if (m_selectedKind == EntityKind::CurrentLoop)
    {
        CurrentLoop *loop = m_world.findCurrentLoop(m_selectedId);
        if (!loop)
        {
            m_selectedKind = EntityKind::None;
            return;
        }

        ImGui::SetNextWindowPos(ImVec2(10.0f, 300.0f), ImGuiCond_Once);
        ImGui::Begin("Properties", nullptr, kMovableFlags);
        ImGui::Text("Current Loop");
        ImGui::Separator();

        ImGui::DragFloat("Radius (m)", &loop->radius, CURRENT_LOOP_RADIUS_STEP, MIN_CURRENT_LOOP_RADIUS, MAX_CURRENT_LOOP_RADIUS, "%.2f");
        ImGui::DragFloat("Current (A)", &loop->current, CURRENT_LOOP_CURRENT_STEP, MIN_CURRENT_LOOP_CURRENT, MAX_CURRENT_LOOP_CURRENT, "%.2f");
        ImGui::DragInt("Turns", &loop->turns, 1.0f, MIN_CURRENT_LOOP_TURNS, MAX_CURRENT_LOOP_TURNS);
        ImGui::Text("Position: %s m", loop->center.toString().c_str());

        ImGui::End();
    }
}

void App::drawCircuitPropertiesPanel()
{
    if (m_circuitSelectedKind == CircuitEntityKind::None)
        return;

    if (m_circuitSelectedKind == CircuitEntityKind::Component)
    {
        Component *comp = m_circuit.findComponent(m_circuitSelectedId);
        if (!comp)
        {
            m_circuitSelectedKind = CircuitEntityKind::None;
            return;
        }

        ImGui::SetNextWindowPos(ImVec2(10.0f, 300.0f), ImGuiCond_Once);
        ImGui::Begin("Properties", nullptr, kMovableFlags);
        ImGui::Text("%s", comp->typeName().c_str());
        ImGui::Separator();

        const std::string typeName = comp->typeName();
        const std::string idSuffix = std::to_string(comp->id);

        if (Resistor *resistor = dynamic_cast<Resistor *>(comp))
        {
            ImGui::DragFloat("Resistance (Ohm)", &resistor->resistance, RESISTANCE_STEP, MIN_RESISTANCE, MAX_RESISTANCE, "%.3g");
            ImGui::Text("Voltage: %.4g V", resistor->voltage);
            drawGraphToggle(quantityLabel(typeName, comp->id, "Voltage", "V"), "resV" + idSuffix,
                            [this, id = comp->id]() -> double
                            {
                                Resistor *r = dynamic_cast<Resistor *>(m_circuit.findComponent(id));
                                return r ? static_cast<double>(r->voltage) : std::nan("");
                            });
            ImGui::Text("Current: %.4g A", resistor->current);
            drawGraphToggle(quantityLabel(typeName, comp->id, "Current", "A"), "resI" + idSuffix,
                            [this, id = comp->id]() -> double
                            {
                                Resistor *r = dynamic_cast<Resistor *>(m_circuit.findComponent(id));
                                return r ? static_cast<double>(r->current) : std::nan("");
                            });
        }
        else if (Capacitor *capacitor = dynamic_cast<Capacitor *>(comp))
        {
            ImGui::DragFloat("Capacitance (F)", &capacitor->capacitance, CAPACITANCE_STEP, MIN_CAPACITANCE, MAX_CAPACITANCE, "%.3g");
            ImGui::Text("Voltage: %.4g V", capacitor->voltage);
            drawGraphToggle(quantityLabel(typeName, comp->id, "Voltage", "V"), "capV" + idSuffix,
                            [this, id = comp->id]() -> double
                            {
                                Capacitor *c = dynamic_cast<Capacitor *>(m_circuit.findComponent(id));
                                return c ? static_cast<double>(c->voltage) : std::nan("");
                            });
            ImGui::Text("Charge: %.4g C", capacitor->charge);
            drawGraphToggle(quantityLabel(typeName, comp->id, "Charge", "C"), "capQ" + idSuffix,
                            [this, id = comp->id]() -> double
                            {
                                Capacitor *c = dynamic_cast<Capacitor *>(m_circuit.findComponent(id));
                                return c ? static_cast<double>(c->charge) : std::nan("");
                            });
            ImGui::Text("Current: %.4g A", capacitor->current);
            drawGraphToggle(quantityLabel(typeName, comp->id, "Current", "A"), "capI" + idSuffix,
                            [this, id = comp->id]() -> double
                            {
                                Capacitor *c = dynamic_cast<Capacitor *>(m_circuit.findComponent(id));
                                return c ? static_cast<double>(c->current) : std::nan("");
                            });
        }
        else if (Inductor *inductor = dynamic_cast<Inductor *>(comp))
        {
            ImGui::DragFloat("Inductance (H)", &inductor->inductance, INDUCTANCE_STEP, MIN_INDUCTANCE, MAX_INDUCTANCE, "%.3g");
            ImGui::Text("Voltage: %.4g V", inductor->voltage);
            drawGraphToggle(quantityLabel(typeName, comp->id, "Voltage", "V"), "indV" + idSuffix,
                            [this, id = comp->id]() -> double
                            {
                                Inductor *i = dynamic_cast<Inductor *>(m_circuit.findComponent(id));
                                return i ? static_cast<double>(i->voltage) : std::nan("");
                            });
            ImGui::Text("Current: %.4g A", inductor->current);
            drawGraphToggle(quantityLabel(typeName, comp->id, "Current", "A"), "indI" + idSuffix,
                            [this, id = comp->id]() -> double
                            {
                                Inductor *i = dynamic_cast<Inductor *>(m_circuit.findComponent(id));
                                return i ? static_cast<double>(i->current) : std::nan("");
                            });
        }
        else if (Battery *battery = dynamic_cast<Battery *>(comp))
        {
            ImGui::DragFloat("EMF (V)", &battery->emf, EMF_STEP, MIN_EMF, MAX_EMF, "%.2f");
            ImGui::DragFloat("Internal Resistance (Ohm)", &battery->internalResistance, INTERNAL_RESISTANCE_STEP, MIN_INTERNAL_RESISTANCE, MAX_INTERNAL_RESISTANCE, "%.2f");
            ImGui::Text("Terminal Voltage: %.4g V", battery->voltage);
            drawGraphToggle(quantityLabel(typeName, comp->id, "Terminal Voltage", "V"), "battV" + idSuffix,
                            [this, id = comp->id]() -> double
                            {
                                Battery *b = dynamic_cast<Battery *>(m_circuit.findComponent(id));
                                return b ? static_cast<double>(b->voltage) : std::nan("");
                            });
            ImGui::Text("Current: %.4g A", battery->current);
            drawGraphToggle(quantityLabel(typeName, comp->id, "Current", "A"), "battI" + idSuffix,
                            [this, id = comp->id]() -> double
                            {
                                Battery *b = dynamic_cast<Battery *>(m_circuit.findComponent(id));
                                return b ? static_cast<double>(b->current) : std::nan("");
                            });
        }
        else if (Switch *sw = dynamic_cast<Switch *>(comp))
        {
            ImGui::Checkbox("Closed", &sw->closed);
            ImGui::Text("Voltage: %.4g V", sw->voltage);
            drawGraphToggle(quantityLabel(typeName, comp->id, "Voltage", "V"), "swV" + idSuffix,
                            [this, id = comp->id]() -> double
                            {
                                Switch *s = dynamic_cast<Switch *>(m_circuit.findComponent(id));
                                return s ? static_cast<double>(s->voltage) : std::nan("");
                            });
            ImGui::Text("Current: %.4g A", sw->current);
            drawGraphToggle(quantityLabel(typeName, comp->id, "Current", "A"), "swI" + idSuffix,
                            [this, id = comp->id]() -> double
                            {
                                Switch *s = dynamic_cast<Switch *>(m_circuit.findComponent(id));
                                return s ? static_cast<double>(s->current) : std::nan("");
                            });
        }
        else if (Probe *probe = dynamic_cast<Probe *>(comp))
        {
            if (probe->kind == Probe::Kind::Ammeter)
            {
                ImGui::Text("Current: %.4g A", probe->current);
                drawGraphToggle(quantityLabel(typeName, comp->id, "Current", "A"), "probeI" + idSuffix,
                                [this, id = comp->id]() -> double
                                {
                                    Probe *p = dynamic_cast<Probe *>(m_circuit.findComponent(id));
                                    return p ? static_cast<double>(p->current) : std::nan("");
                                });
            }
            else
            {
                ImGui::Text("Voltage: %.4g V", probe->voltage);
                drawGraphToggle(quantityLabel(typeName, comp->id, "Voltage", "V"), "probeV" + idSuffix,
                                [this, id = comp->id]() -> double
                                {
                                    Probe *p = dynamic_cast<Probe *>(m_circuit.findComponent(id));
                                    return p ? static_cast<double>(p->voltage) : std::nan("");
                                });
            }
        }

        ImGui::Text("A: %s m", comp->posA.toString().c_str());
        ImGui::Text("B: %s m", comp->posB.toString().c_str());
        ImGui::Checkbox("Show Label", &comp->showLabel);

        ImGui::End();
    }
    else if (m_circuitSelectedKind == CircuitEntityKind::Wire)
    {
        CircuitWire *wire = m_circuit.findWire(m_circuitSelectedId);
        if (!wire)
        {
            m_circuitSelectedKind = CircuitEntityKind::None;
            return;
        }

        ImGui::SetNextWindowPos(ImVec2(10.0f, 300.0f), ImGuiCond_Once);
        ImGui::Begin("Properties", nullptr, kMovableFlags);
        ImGui::Text("Wire");
        ImGui::Separator();
        ImGui::Text("Current: %.4g A", wire->current);
        drawGraphToggle(quantityLabel("Wire", wire->id, "Current", "A"), "circWireI" + std::to_string(wire->id),
                        [this, id = wire->id]() -> double
                        {
                            CircuitWire *w = m_circuit.findWire(id);
                            return w ? static_cast<double>(w->current) : std::nan("");
                        });
        ImGui::Text("Start: %s m", wire->start.toString().c_str());
        ImGui::Text("End: %s m", wire->end.toString().c_str());
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

    ImGui::Separator();
    ImGui::Text("Current Flow:");
    ImGui::SameLine();
    if (ImGui::RadioButton("Off", m_renderer.currentFlowDisplay == CurrentFlowDisplay::Off))
        m_renderer.currentFlowDisplay = CurrentFlowDisplay::Off;
    ImGui::SameLine();
    if (ImGui::RadioButton("Conventional", m_renderer.currentFlowDisplay == CurrentFlowDisplay::Conventional))
        m_renderer.currentFlowDisplay = CurrentFlowDisplay::Conventional;
    ImGui::SameLine();
    if (ImGui::RadioButton("Electron", m_renderer.currentFlowDisplay == CurrentFlowDisplay::Electron))
        m_renderer.currentFlowDisplay = CurrentFlowDisplay::Electron;

    ImGui::Separator();
    ImGui::Text("Data Logging & Graphing:");
    ImGui::Text("Quantities selected: %d", static_cast<int>(m_grapher.selectedQuantities().size()));
    if (ImGui::Button(m_grapher.isGraphOpen() ? "Close Graph" : "Open Graph"))
        m_grapher.toggleGraph(m_logger);
    ImGui::SameLine();
    ImGui::TextDisabled("(G)");
    ImGui::SameLine();
    if (ImGui::Button("Clear Selection"))
        m_grapher.clearSelection();
    if (m_grapher.isGraphOpen() && ImGui::Button("Save Graph as Image"))
        m_grapher.saveAsImage(timestampedFilename(GRAPH_EXPORT_DIRECTORY, "graph", "png"));
    if (ImGui::Button("Save Log"))
        m_logger.save(timestampedFilename(LOG_DIRECTORY, "log", "json"));
    ImGui::SameLine();
    if (ImGui::Button("Open Log Folder"))
        m_logger.openLogFolder();
    ImGui::SameLine();
    if (ImGui::Button("Clear Log"))
        m_logger.reset();

    if (fieldsMode)
    {
        ImGui::Separator();
        ImGui::Checkbox("Field Vectors", &m_renderer.showFieldVectors);
        ImGui::Checkbox("Field Lines", &m_renderer.showFieldLines);
        ImGui::Checkbox("Equipotential Lines", &m_renderer.showEquipotentials);
        ImGui::Checkbox("Magnetic Field", &m_renderer.showMagneticField);
        ImGui::Separator();
        ImGui::Checkbox("Uniform B Field", &m_world.uniformField().enabled);
        if (m_world.uniformField().enabled)
            ImGui::DragFloat("B Strength (T)", &m_world.uniformField().strength, 0.05f, MIN_B_FIELD_STRENGTH, MAX_B_FIELD_STRENGTH, "%.2f");
        float permeabilityFactor = m_world.permeabilityFactor();
        if (ImGui::DragFloat("Permeability Factor", &permeabilityFactor, PERMEABILITY_FACTOR_STEP, MIN_PERMEABILITY_FACTOR, MAX_PERMEABILITY_FACTOR, "%.3g"))
            m_world.setPermeabilityFactor(permeabilityFactor);
        ImGui::SameLine();
        if (ImGui::Button("1x"))
            m_world.setPermeabilityFactor(1.0f);
        ImGui::Separator();
        // Loading a preset (or resetting) replaces the scene wholesale, so any graphed
        // quantity/logged history would otherwise dangle onto entities that no longer
        // exist - clear both alongside the scene itself, same as Reset Simulation below.
        if (ImGui::Button("Load Dipole Field Preset"))
        {
            Presets::loadDipoleField(m_world);
            m_logger.reset();
            m_grapher.clearSelection();
        }
        if (ImGui::Button("Load Particle in Uniform B Preset"))
        {
            Presets::loadParticleInUniformB(m_world);
            m_logger.reset();
            m_grapher.clearSelection();
        }
        if (ImGui::Button("Load Generator Demo Preset"))
        {
            Presets::loadGeneratorDemo(m_world);
            m_logger.reset();
            m_grapher.clearSelection();
        }
        if (ImGui::Button("Reset Simulation"))
        {
            m_world.reset();
            m_grabbedKind = EntityKind::None;
            m_selectedKind = EntityKind::None;
            m_logger.reset();
            m_grapher.clearSelection();
        }
    }
    else
    {
        ImGui::Separator();
        if (ImGui::Button("Load Basic Resistor Circuit Preset"))
        {
            Presets::loadBasicResistorCircuit(m_circuit);
            m_logger.reset();
            m_grapher.clearSelection();
        }
        if (ImGui::Button("Load Lightbulb Circuit Preset"))
        {
            Presets::loadLightbulbCircuit(m_circuit);
            m_logger.reset();
            m_grapher.clearSelection();
        }
        if (ImGui::Button("Load Simple RC Circuit Preset"))
        {
            Presets::loadSimpleRCCircuit(m_circuit);
            m_logger.reset();
            m_grapher.clearSelection();
        }
        if (ImGui::Button("Load Simple LR Circuit Preset"))
        {
            Presets::loadSimpleLRCircuit(m_circuit);
            m_logger.reset();
            m_grapher.clearSelection();
        }
        if (ImGui::Button("Load Simple LC Circuit Preset"))
        {
            Presets::loadSimpleLCCircuit(m_circuit);
            m_logger.reset();
            m_grapher.clearSelection();
        }
        if (ImGui::Button("Reset Simulation"))
        {
            m_circuit.reset();
            m_circuitGrabbedKind = CircuitEntityKind::None;
            m_circuitSelectedKind = CircuitEntityKind::None;
            m_logger.reset();
            m_grapher.clearSelection();
        }
    }

    const ImVec2 settingsWindowSize = ImGui::GetWindowSize();
    ImGui::SetWindowPos(ImVec2((static_cast<float>(m_window.getSize().x) - settingsWindowSize.x) * 0.5f, (static_cast<float>(m_window.getSize().y) - settingsWindowSize.y) * 0.5f), ImGuiCond_Always);

    ImGui::End();
}
