#include "logging/Logger.hpp"

Logger::Logger() = default;

void Logger::record(float simTime, const std::map<std::string, double> &namedValues)
{
    // TODO(Phase 6): append to history, keyed by simTime
    (void)simTime;
    (void)namedValues;
}

void Logger::reset()
{
    // TODO(Phase 6): clear history
}

void Logger::save(const std::string &path) const
{
    // TODO(Phase 6): write JSON log to `path` (nlohmann::json), like NN's Logger
    (void)path;
}
