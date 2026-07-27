#include "graphing/Grapher.hpp"

#include <algorithm>
#include <cmath>

Grapher::Grapher() = default;

Grapher::~Grapher()
{
    closeGraph();
}

void Grapher::select(const std::string &quantityName, std::function<double()> valueGetter)
{
    const auto it = std::find_if(m_selected.begin(), m_selected.end(),
                                  [&](const auto &entry) { return entry.first == quantityName; });
    if (it != m_selected.end())
        it->second = std::move(valueGetter);
    else
        m_selected.emplace_back(quantityName, std::move(valueGetter));
}

void Grapher::deselect(const std::string &quantityName)
{
    m_selected.erase(std::remove_if(m_selected.begin(), m_selected.end(),
                                     [&](const auto &entry) { return entry.first == quantityName; }),
                      m_selected.end());
}

bool Grapher::isSelected(const std::string &quantityName) const
{
    return std::any_of(m_selected.begin(), m_selected.end(),
                        [&](const auto &entry) { return entry.first == quantityName; });
}

void Grapher::clearSelection()
{
    m_selected.clear();
}

bool Grapher::hasSelection() const
{
    return !m_selected.empty();
}

std::vector<std::string> Grapher::selectedQuantities() const
{
    std::vector<std::string> names;
    names.reserve(m_selected.size());
    for (const auto &[name, getter] : m_selected)
        names.push_back(name);
    return names;
}

std::map<std::string, double> Grapher::sample() const
{
    std::map<std::string, double> snapshot;
    for (const auto &[name, getter] : m_selected)
        snapshot[name] = getter();
    return snapshot;
}

void Grapher::plot(const Logger &logger, const std::vector<std::string> &quantityNames)
{
    using namespace matplot;

    closeGraph();
    if (quantityNames.empty())
        return;

    std::vector<double> times;
    for (const auto &[time, snapshot] : logger.history())
        times.push_back(static_cast<double>(time));
    if (times.empty())
        return;

    m_fig = figure(true);
    m_fig->quiet_mode(false); // interactive gnuplot window, not saved to a file by default

    int plotted = 0;
    for (const std::string &name : quantityNames)
    {
        std::vector<double> values;
        values.reserve(times.size());
        for (const auto &[time, snapshot] : logger.history())
        {
            const auto it = snapshot.find(name);
            values.push_back(it != snapshot.end() ? it->second : std::nan(""));
        }

        hold(on);
        auto line = matplot::plot(times, values);
        line->display_name(name);
        ++plotted;
    }

    if (plotted > 0)
    {
        xlabel("Time (s)");
        ylabel("Value");
        title("Maxwell's Margins - Logged Quantities");
        legend();
        grid(on);
        m_fig->draw();
    }
    else
    {
        m_fig = nullptr;
    }
}

void Grapher::closeGraph()
{
    if (!m_fig)
        return;

    // Send an explicit exit to the gnuplot backend before dropping the handle, rather than
    // leaving an orphaned gnuplot process behind.
    auto backend = std::dynamic_pointer_cast<matplot::backend::gnuplot>(m_fig->backend());
    if (backend)
    {
        backend->run_command("exit");
        backend->flush_commands();
    }
    m_fig = nullptr;
}

void Grapher::toggleGraph(const Logger &logger)
{
    if (isGraphOpen())
        closeGraph();
    else
        plot(logger, selectedQuantities());
}

bool Grapher::isGraphOpen() const
{
    return m_fig != nullptr;
}

void Grapher::saveAsImage(const std::string &path) const
{
    if (m_fig)
        m_fig->save(path);
}
