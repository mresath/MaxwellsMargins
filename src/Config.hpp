#pragma once

// WORLD & WINDOW CONFIGURATION
#define PIXELS_PER_METER 50.f
#define DEF_WIDTH 1280.f
#define DEF_HEIGHT 800.f
#define BACKGROUND_COLOR sf::Color(30, 30, 30, 255)

// Fields mode has no ground/walls, so the camera can zoom/pan well past the initial view.
#define MAX_VIEW_WIDTH (DEF_WIDTH * 8.0f)
#define MAX_VIEW_HEIGHT (DEF_HEIGHT * 8.0f)

// GRID CONFIGURATION
#define GRID_MAJOR_SPACING 5.0f
#define GRID_MINOR_SPACING 1.0f
#define GRID_MINOR_COLOR sf::Color(50, 50, 50, 255)
#define GRID_MAJOR_COLOR sf::Color(80, 80, 80, 255)
#define GRID_AXIS_COLOR sf::Color(120, 120, 120, 255)

// UI CONFIGURATION
#define Y_ITEM_SPACING 6.0f
#define ZOOM_STEP 0.025f
#define PAN_SPEED 15.f

// TOOLS CONFIGURATION
#define TOOLS_ICON_SIZE 32
#define TOOL_BG_COLOR sf::Color(41, 74, 122, 102)
#define TOOL_SELECT_COLOR sf::Color(66, 150, 250, 200)

// ENGINE CONFIGURATION (single shared DOPRI5-style solver, see engine/Solver.hpp)
#define CALC_FREQ 240.f // Hz
#define MAX_DT 0.05f    // seconds
#define MAX_FPS 120
#define MAX_UPDATES_PER_FRAME 2

// ELECTROSTATICS CONFIGURATION
#define COULOMB_CONSTANT 8.9875517923e9f // N*m^2/C^2 (k)
#define ELEMENTARY_CHARGE 1.602176634e-19f // C (e), selectable via the "e" button next to charge fields
#define MIN_CHARGE_MAGNITUDE ELEMENTARY_CHARGE
#define MAX_CHARGE_MAGNITUDE 1e-6f // C
#define DEFAULT_CHARGE_MAGNITUDE 1e-7f   // C
#define POSITIVE_CHARGE_COLOR sf::Color(220, 60, 60, 255)
#define NEGATIVE_CHARGE_COLOR sf::Color(60, 100, 220, 255)
#define FIELD_VECTOR_COLOR sf::Color(255, 210, 80, 255)
#define FIELD_LINE_COLOR sf::Color(255, 210, 80, 150)
#define EQUIPOTENTIAL_LINE_COLOR sf::Color(120, 220, 160, 180)
#define GAUSSIAN_SURFACE_COLOR sf::Color(180, 140, 255, 200)

#define VACUUM_PERMITTIVITY 8.8541878128e-12f // F/m (epsilon_0), for Gauss's law flux
#define POINT_CHARGE_RADIUS 0.15f             // meters, drawn circle radius for a placed charge
#define CHARGE_MAGNITUDE_STEP 1e-8f            // Coulombs, Tool Settings drag-slider step

#define GAUSSIAN_SURFACE_DEFAULT_RADIUS 2.0f // meters, radius of a newly-drawn Gaussian surface
#define MIN_GAUSSIAN_SURFACE_RADIUS 0.5f     // meters
#define MAX_GAUSSIAN_SURFACE_RADIUS 6.0f     // meters
#define GAUSSIAN_SURFACE_RADIUS_STEP 0.1f    // meters, Tool Settings drag-slider step

#define ENTITY_HIT_RADIUS 0.25f // meters, click/hit-test tolerance for Move/Select/Erase

#define FIELD_VECTOR_SPACING 1.0f       // meters between sampled field-vector arrows
#define FIELD_VECTOR_MAX_LENGTH 0.4f    // meters, cap on drawn arrow length regardless of field magnitude
#define FIELD_VECTOR_MIN_MAGNITUDE 1.0f // N/C, below this the field is too weak to draw
#define FIELD_VECTOR_SATURATION 60.0f   // N/C, half-saturation point for arrow length scaling

#define FIELD_LINE_COUNT_PER_CHARGE 12 // field lines seeded around each charge
#define FIELD_LINE_STEP 0.05f          // meters, integration step size for tracing
#define FIELD_LINE_MAX_STEPS 400
#define FIELD_LINE_CAPTURE_RADIUS 0.12f // meters, line terminates within this distance of a charge
#define FIELD_LINE_MAX_RADIUS 30.0f     // meters from origin before a line is considered escaped

#define EQUIPOTENTIAL_GRID_STEP 0.2f // meters, marching-squares sampling resolution

// MAGNETISM CONFIGURATION
#define VACUUM_PERMEABILITY 1.25663706212e-6f // T*m/A (mu_0)

// User-adjustable multiplier on mu_0 (Settings panel, "1x" button resets to real physics).
// MAX is capped so a particle grazing a wire can't exceed the fixed-step cyclotron stability limit.
#define MIN_PERMEABILITY_FACTOR 1.0f
#define MAX_PERMEABILITY_FACTOR 3.0e6f
#define DEFAULT_PERMEABILITY_FACTOR 1.0e6f
#define PERMEABILITY_FACTOR_STEP 1000.0f

#define MIN_B_FIELD_STRENGTH -5.0f // T
#define MAX_B_FIELD_STRENGTH 5.0f  // T
#define DEFAULT_B_FIELD_STRENGTH 1.0f
#define B_FIELD_OUT_OF_PAGE_COLOR sf::Color(80, 180, 255, 255)
#define B_FIELD_INTO_PAGE_COLOR sf::Color(255, 140, 80, 255)
#define CURRENT_WIRE_COLOR sf::Color(230, 230, 230, 255)
#define TRAJECTORY_TRACE_COLOR sf::Color(100, 150, 255, 255)

// Animated charge-carrier markers along a wire/loop (Renderer::CurrentFlowDisplay) - shared
// by magnetism's wires, induction's loops, and (once built) circuits' wires alike.
#define CURRENT_FLOW_MARKER_SPACING 0.35f     // meters between markers along the path
#define CURRENT_FLOW_MARKER_RADIUS 0.05f      // meters, drawn radius of each marker
#define CURRENT_FLOW_MAX_SPEED 3.0f           // meters/second, marker speed at saturation
#define CURRENT_FLOW_SPEED_SATURATION 5.0f    // A, half-saturation point for marker-speed scaling

#define B_FIELD_MARKER_SPACING 1.0f        // meters between sampled into/out-of-page markers
#define B_FIELD_MARKER_MIN_MAGNITUDE 0.02f // T, below this the field is too weak to draw
#define B_FIELD_MARKER_SATURATION 2.0f     // T, half-saturation point for marker size scaling
#define B_FIELD_MARKER_MAX_RADIUS 0.12f    // meters, cap on drawn marker size regardless of field strength

// Bigger than MIN/MAX/DEFAULT_CHARGE_MAGNITUDE for a visible self-generated B field; paired
// with DEFAULT_PARTICLE_MASS to preserve the charge/mass ratio that sets cyclotron frequency.
#define MIN_PARTICLE_CHARGE_MAGNITUDE ELEMENTARY_CHARGE
#define MAX_PARTICLE_CHARGE_MAGNITUDE 1.0f     // C
#define DEFAULT_PARTICLE_CHARGE_MAGNITUDE 0.1f // C
#define PARTICLE_CHARGE_MAGNITUDE_STEP 0.001f  // C, Tool Settings drag-slider step

#define ELECTRON_MASS 9.1093837015e-31f // kg, selectable via the "e-" button next to mass fields
#define PROTON_MASS 1.67262192369e-27f  // kg, selectable via the "p+" button next to mass fields

#define MIN_PARTICLE_MASS ELECTRON_MASS
#define MAX_PARTICLE_MASS 1.0f    // kg
#define DEFAULT_PARTICLE_MASS 0.1f // kg
#define PARTICLE_MASS_STEP 0.01f  // kg, Tool Settings drag-slider step

#define MIN_PARTICLE_SPEED 0.0f     // m/s
#define MAX_PARTICLE_SPEED 20.0f    // m/s
#define DEFAULT_PARTICLE_SPEED 5.0f // m/s
#define PARTICLE_SPEED_STEP 0.1f    // m/s, Tool Settings drag-slider step

#define PARTICLE_RADIUS 0.12f            // meters, drawn circle radius for a placed particle
#define TRAJECTORY_TRACE_MAX_POINTS 600u // cap on stored trajectory points before the oldest is dropped

#define MIN_WIRE_CURRENT -20.0f     // A
#define MAX_WIRE_CURRENT 20.0f      // A
#define DEFAULT_WIRE_CURRENT 5.0f   // A
#define WIRE_CURRENT_STEP 0.1f      // A, Tool Settings drag-slider step
#define MIN_WIRE_LENGTH 0.1f       // meters, shorter click-drags are discarded as accidental clicks

#define WIRE_PARALLEL_DOT_THRESHOLD 0.98f // |dot of unit directions| above this counts as "parallel" for the force readout

#define MIN_CURRENT_LOOP_RADIUS 0.3f     // meters
#define MAX_CURRENT_LOOP_RADIUS 3.0f     // meters
#define DEFAULT_CURRENT_LOOP_RADIUS 1.0f // meters
#define CURRENT_LOOP_RADIUS_STEP 0.05f   // meters, Tool Settings drag-slider step

#define MIN_CURRENT_LOOP_CURRENT -20.0f   // A
#define MAX_CURRENT_LOOP_CURRENT 20.0f    // A
#define DEFAULT_CURRENT_LOOP_CURRENT 5.0f // A
#define CURRENT_LOOP_CURRENT_STEP 0.1f    // A, Tool Settings drag-slider step

// "Loop count" - ampere-turns (turns * current) scales the generated field the same way
// a multi-turn coil would, without modeling each individual winding.
#define MIN_CURRENT_LOOP_TURNS 1
#define MAX_CURRENT_LOOP_TURNS 100
#define DEFAULT_CURRENT_LOOP_TURNS 1

// A dipole magnet's axis points through the page (matching CurrentLoop's model), so its
// "surface field" - the real Tesla value at its own center, by construction of the formula
// in FieldMath::dipoleMagnetField - is a directly user-set physical quantity, unlike
// current-driven sources: it needs no permeabilityFactor amplification to be visible.
#define MIN_DIPOLE_MAGNET_RADIUS 0.1f     // meters
#define MAX_DIPOLE_MAGNET_RADIUS 1.5f     // meters
#define DEFAULT_DIPOLE_MAGNET_RADIUS 0.3f // meters - a compact object, unlike CurrentLoop's coil
#define DIPOLE_MAGNET_RADIUS_STEP 0.02f   // meters, Tool Settings drag-slider step

#define MIN_DIPOLE_MAGNET_SURFACE_FIELD -2.0f    // T
#define MAX_DIPOLE_MAGNET_SURFACE_FIELD 2.0f      // T
#define DEFAULT_DIPOLE_MAGNET_SURFACE_FIELD 1.0f  // T, a strong neodymium-magnet-scale value
#define DIPOLE_MAGNET_SURFACE_FIELD_STEP 0.05f    // T, Tool Settings drag-slider step

// Positive surfaceField = North pole toward the viewer (field out of page at center).
#define DIPOLE_MAGNET_NORTH_COLOR sf::Color(220, 70, 70, 255)
#define DIPOLE_MAGNET_SOUTH_COLOR sf::Color(70, 110, 220, 255)

// INDUCTION CONFIGURATION
#define MIN_LOOP_RADIUS 0.3f     // meters
#define MAX_LOOP_RADIUS 3.0f     // meters
#define DEFAULT_LOOP_RADIUS 1.0f // meters
#define LOOP_RADIUS_STEP 0.05f   // meters, Tool Settings drag-slider step

#define MIN_LOOP_TURNS 1
#define MAX_LOOP_TURNS 100
#define DEFAULT_LOOP_TURNS 1

#define MIN_LOOP_ANGULAR_VELOCITY -20.0f   // rad/s
#define MAX_LOOP_ANGULAR_VELOCITY 20.0f    // rad/s
#define DEFAULT_LOOP_ANGULAR_VELOCITY 0.0f // rad/s
#define LOOP_ANGULAR_VELOCITY_STEP 0.1f    // rad/s, Tool Settings drag-slider step

#define LOOP_COLOR sf::Color(230, 230, 230, 255)
#define INDUCED_EMF_ARROW_COLOR sf::Color(255, 210, 80, 255)

// A coil's turns are drawn as concentric rings stepping inward from the nominal radius,
// capped well below MAX_*_TURNS so a high turn count doesn't collapse into a solid disc.
#define LOOP_VISUAL_MAX_RINGS 5
#define LOOP_RING_SPACING 0.06f // meters between successive rings

// A loop has no defined resistance yet (that's a circuits concept), so its current-flow
// marker speed saturates against induced EMF directly rather than an Amp value.
#define CURRENT_FLOW_EMF_SATURATION 2.0f // V, half-saturation point for loop marker-speed scaling

#define MIN_DISPLAYED_EMF 1e-6f   // V, below this the direction indicator/label is hidden as noise
#define EMF_TRACE_MAX_POINTS 300u // cap on stored EMF-trace samples before the oldest is dropped

// CIRCUITS CONFIGURATION
#define MIN_RESISTANCE 1.0f       // Ohm
#define MAX_RESISTANCE 1.0e6f     // Ohm
#define DEFAULT_RESISTANCE 100.0f // Ohm, paired with DEFAULT_CAPACITANCE for a ~2s RC time constant at a punchy ~90mA
#define RESISTANCE_STEP 10.0f     // Ohm, Tool Settings drag-slider step

#define MIN_CAPACITANCE 1.0e-9f    // F
#define MAX_CAPACITANCE 0.1f       // F
#define DEFAULT_CAPACITANCE 2e-2f  // F
#define CAPACITANCE_STEP 2.0e-3f   // F, Tool Settings drag-slider step

// A lumped RL element - no coupling to Fields mode's spatial B field (see induction/, which
// models a real coil's own generated/enclosed field separately).
#define MIN_INDUCTANCE 1.0e-3f  // H
#define MAX_INDUCTANCE 10.0f    // H
#define DEFAULT_INDUCTANCE 1.0f // H
#define INDUCTANCE_STEP 0.05f   // H, Tool Settings drag-slider step

#define MIN_EMF 0.0f      // V
#define MAX_EMF 24.0f     // V
#define DEFAULT_EMF 9.0f  // V
#define EMF_STEP 0.5f     // V, Tool Settings drag-slider step

#define MIN_INTERNAL_RESISTANCE 0.0f       // Ohm
#define MAX_INTERNAL_RESISTANCE 100.0f     // Ohm
#define DEFAULT_INTERNAL_RESISTANCE 1.0f   // Ohm
#define INTERNAL_RESISTANCE_STEP 0.5f      // Ohm, Tool Settings drag-slider step

#define WIRE_COLOR sf::Color(230, 230, 230, 255)
#define COMPONENT_COLOR sf::Color(200, 200, 200, 255)
#define PROBE_COLOR sf::Color(255, 210, 80, 255)
#define OPEN_SWITCH_COLOR sf::Color(220, 90, 90, 255)
#define COMPONENT_LABEL_COLOR sf::Color(180, 210, 255, 255)

// A component's schematic symbol is drawn centered on its posA-posB span, scaled down on
// short spans so it never overflows past the terminals.
#define COMPONENT_SYMBOL_HALF_SIZE 0.35f // meters, half-width of a component's drawn symbol

// Solved circuit currents are realistically mA-to-low-A scale, unlike Fields' wires (up to
// MAX_WIRE_CURRENT = 20A) - a separate half-saturation point keeps the flow-marker
// animation visibly responsive at circuit scale instead of reading as nearly static.
#define CIRCUIT_CURRENT_FLOW_SATURATION 0.2f // A, half-saturation point for circuit marker-speed scaling

// Lightbulb: electrically a plain Resistor (see circuits/Lightbulb.hpp); the bulb glows by
// interpolating toward LIGHTBULB_GLOW_COLOR as dissipated power (V*I) approaches saturation.
#define LIGHTBULB_POWER_SATURATION 0.5f // W, half-saturation point for glow-brightness scaling
#define LIGHTBULB_OFF_COLOR sf::Color(90, 85, 60, 255)
#define LIGHTBULB_GLOW_COLOR sf::Color(255, 235, 120, 255)

// LOGGING & GRAPHING CONFIGURATION
#define LOG_DIRECTORY "logs"
#define SCREENSHOT_DIRECTORY "screenshots"
#define GRAPH_EXPORT_DIRECTORY "graphs"

// A session left running indefinitely would otherwise grow Logger's history unbounded -
// oldest samples are dropped past this cap, same spirit as TRAJECTORY_TRACE_MAX_POINTS.
#define LOG_MAX_SAMPLES 5000u
