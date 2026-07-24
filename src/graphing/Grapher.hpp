#pragma once

#include <string>
#include <vector>

#include "logging/Logger.hpp"

// Graphs logged quantities over time via matplot++/gnuplot (paused while the graph
// window is open, like both existing apps) and supports saving the graph as an image.
class Grapher
{
public:
    Grapher();

    void plot(const Logger &logger, const std::vector<std::string> &quantityNames);
    void saveAsImage(const std::string &path) const;
};
