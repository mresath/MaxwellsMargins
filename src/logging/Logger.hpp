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

private:
    // TODO(Phase 6): time -> {quantity name -> value} history
};
