#include "logging/Logger.hpp"

#include "Config.hpp"

#include <filesystem>

namespace
{
void ensureDirectory(const std::string &path)
{
    if (!std::filesystem::exists(path))
        std::filesystem::create_directory(path);
}
} // namespace

// Created relative to the working directory, which main() has already chdir'd to the
// executable's own directory (see chdirToExecutableDirectory() in main.cpp), so these
// land next to the binary even inside a packaged .app bundle rather than wherever the
// app happened to be launched from.
Logger::Logger()
{
    ensureDirectory(LOG_DIRECTORY);
    ensureDirectory(SCREENSHOT_DIRECTORY);
    ensureDirectory(GRAPH_EXPORT_DIRECTORY);
}

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
