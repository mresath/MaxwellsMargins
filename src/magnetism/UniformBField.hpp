#pragma once

#include "Config.hpp"

// A uniform magnetic field region (into/out of the page, adjustable strength) applied
// across the whole Fields-mode canvas.
class UniformBField
{
public:
    UniformBField() : strength(DEFAULT_B_FIELD_STRENGTH), enabled(false) {}

    float strength; // Tesla, signed: positive = out of page, negative = into page
    bool enabled;
};
