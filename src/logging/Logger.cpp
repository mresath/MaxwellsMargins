#include "logging/Logger.hpp"

#include "Config.hpp"

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>

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
    m_history[simTime] = namedValues;

    // A session left running indefinitely would otherwise grow this unbounded - drop the
    // oldest sample(s) past the cap, same spirit as the trajectory/EMF trace caps elsewhere.
    while (m_history.size() > LOG_MAX_SAMPLES)
        m_history.erase(m_history.begin());
}

void Logger::reset()
{
    m_history.clear();
}

void Logger::save(const std::string &path) const
{
    nlohmann::json j;
    for (const auto &[time, namedValues] : m_history)
    {
        nlohmann::json &sample = j[std::to_string(time)];
        for (const auto &[name, value] : namedValues)
            sample[name] = value;
    }

    std::ofstream file(path);
    if (file.is_open())
        file << j.dump(4);
}

const std::map<float, std::map<std::string, double>> &Logger::history() const
{
    return m_history;
}

void Logger::openLogFolder() const
{
#if defined(_WIN32)
    std::system(("explorer " + std::string(LOG_DIRECTORY)).c_str());
#elif defined(__APPLE__)
    std::system(("open " + std::string(LOG_DIRECTORY)).c_str());
#elif defined(__linux__)
    std::system(("xdg-open " + std::string(LOG_DIRECTORY)).c_str());
#endif
}
