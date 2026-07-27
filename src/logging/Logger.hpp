#pragma once

#include <map>
#include <string>

// Records simulation state (field strength, potential, current, voltage, charge, ...)
// at each update for later graphing, and saves it on exit or on demand.
class Logger
{
public:
    Logger();

    void record(float simTime, const std::map<std::string, double> &namedValues);
    void reset();
    void save(const std::string &path) const;
    void openLogFolder() const;

    // time -> {quantity name -> value}, the full recorded history - Grapher reads this
    // directly to extract a per-quantity time series rather than duplicating storage.
    const std::map<float, std::map<std::string, double>> &history() const;

private:
    std::map<float, std::map<std::string, double>> m_history;
};
