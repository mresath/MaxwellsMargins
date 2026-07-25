#pragma once

// WORLD & WINDOW CONFIGURATION
#define PIXELS_PER_METER 50.f
#define DEF_WIDTH 1280.f
#define DEF_HEIGHT 800.f
#define BACKGROUND_COLOR sf::Color(30, 30, 30, 255)

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
#define MIN_CHARGE_MAGNITUDE 1e-9f       // C
#define MAX_CHARGE_MAGNITUDE 1e-6f       // C
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

#define ENTITY_HIT_RADIUS 0.25f // meters, click/hit-test tolerance for Move/Select/Erase on a charge

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
#define MIN_B_FIELD_STRENGTH -5.0f // T
#define MAX_B_FIELD_STRENGTH 5.0f  // T
#define DEFAULT_B_FIELD_STRENGTH 1.0f
#define B_FIELD_OUT_OF_PAGE_COLOR sf::Color(80, 180, 255, 255)
#define B_FIELD_INTO_PAGE_COLOR sf::Color(255, 140, 80, 255)
#define CURRENT_WIRE_COLOR sf::Color(230, 230, 230, 255)
#define TRAJECTORY_TRACE_COLOR sf::Color(100, 150, 255, 255)

// INDUCTION CONFIGURATION
#define LOOP_COLOR sf::Color(230, 230, 230, 255)
#define INDUCED_EMF_ARROW_COLOR sf::Color(255, 210, 80, 255)
#define LENZ_INDICATOR_COLOR sf::Color(120, 220, 160, 255)

// CIRCUITS CONFIGURATION
#define MIN_RESISTANCE 1.0f       // Ohm
#define MAX_RESISTANCE 1.0e6f     // Ohm
#define DEFAULT_RESISTANCE 100.0f // Ohm

#define MIN_CAPACITANCE 1.0e-9f     // F
#define MAX_CAPACITANCE 1.0e-3f     // F
#define DEFAULT_CAPACITANCE 1.0e-6f // F

#define MIN_INDUCTANCE 1.0e-6f    // H
#define MAX_INDUCTANCE 10.0f      // H
#define DEFAULT_INDUCTANCE 1.0e-3f // H

#define MIN_EMF 0.0f      // V
#define MAX_EMF 24.0f     // V
#define DEFAULT_EMF 9.0f  // V
#define DEFAULT_INTERNAL_RESISTANCE 1.0f // Ohm

#define WIRE_COLOR sf::Color(230, 230, 230, 255)
#define COMPONENT_COLOR sf::Color(200, 200, 200, 255)
#define PROBE_COLOR sf::Color(255, 210, 80, 255)
#define CURRENT_FLOW_COLOR sf::Color(80, 180, 255, 255)

// LOGGING & GRAPHING CONFIGURATION
#define LOG_DIRECTORY "logs"
#define SCREENSHOT_DIRECTORY "screenshots"
#define GRAPH_EXPORT_DIRECTORY "graphs"
