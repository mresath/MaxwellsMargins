#pragma once

// Top-level mode the app is in. Fields covers electrostatics/magnetism/induction on one
// continuous spatial canvas (they superpose via the Lorentz force); Circuits is a separate
// schematic/graph canvas solved via Kirchhoff's laws.
enum class Mode
{
    Fields,
    Circuits
};
