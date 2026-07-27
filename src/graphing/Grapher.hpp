#pragma once

#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <matplot/matplot.h>

#include "logging/Logger.hpp"

// Graphs logged quantities over time via matplot++/gnuplot (paused while the graph
// window is open, like both existing apps) and supports saving the graph as an image.
class Grapher
{
public:
    Grapher();
    ~Grapher();

    // A selected quantity is sampled (via `valueGetter`) into the Logger every update tick,
    // and included the next time a graph is opened. The getter re-looks-up its entity by id
    // each call (rather than capturing a raw pointer) so it stays valid across placement/
    // erasure, reading NaN - a gap in the plotted line - once the entity is gone.
    void select(const std::string &quantityName, std::function<double()> valueGetter);
    void deselect(const std::string &quantityName);
    bool isSelected(const std::string &quantityName) const;
    void clearSelection();
    bool hasSelection() const;
    std::vector<std::string> selectedQuantities() const;

    // Evaluates every selected quantity's getter into a name->value snapshot, for the
    // caller to hand to Logger::record() each tick.
    std::map<std::string, double> sample() const;

    void plot(const Logger &logger, const std::vector<std::string> &quantityNames);
    void closeGraph();
    void toggleGraph(const Logger &logger);
    bool isGraphOpen() const;

    void saveAsImage(const std::string &path) const;

private:
    std::vector<std::pair<std::string, std::function<double()>>> m_selected;
    matplot::figure_handle m_fig{nullptr};
};
