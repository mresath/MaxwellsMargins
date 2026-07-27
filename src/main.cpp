#include "core/App.hpp"

#include <cstdlib>
#include <filesystem>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <csignal>
#include <mach-o/dyld.h>
#else
#include <csignal>
#include <unistd.h>
#endif

// Assets are located relative to the executable, but the OS may launch the app
// (e.g. from Finder or the Start Menu) with an unrelated working directory, so resolve
// and switch to the executable's directory first.
static void chdirToExecutableDirectory()
{
    std::filesystem::path exePath;

#if defined(_WIN32)
    char buffer[MAX_PATH];
    DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length > 0 && length < MAX_PATH)
        exePath = std::filesystem::path(buffer, buffer + length);
#elif defined(__APPLE__)
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0)
        exePath = std::filesystem::canonical(buffer);
#else
    char buffer[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length > 0)
        exePath = std::filesystem::path(buffer, buffer + length);
#endif

    if (!exePath.empty())
    {
        std::error_code ec;
        std::filesystem::current_path(exePath.parent_path(), ec);
    }
}

// The Grapher shells out to `gnuplot` (matplot++'s pipe backend runs `/bin/sh -c "gnuplot
// ..."`). A GUI-launched app (Finder double-click, or `open`) inherits macOS/Linux's
// minimal default PATH, not an interactive shell's PATH - so a Homebrew-installed gnuplot
// (e.g. /opt/homebrew/bin on Apple Silicon) isn't found there even though it works fine
// from a terminal. When that lookup fails, the forked shell exits immediately and the next
// write to its now-closed pipe raises SIGPIPE, which by default terminates the whole app -
// not just the plot. Prepending gnuplot's common install locations fixes this at the
// source; ignoring SIGPIPE is a second line of defense so a still-missing gnuplot (e.g. not
// installed at all) makes the graph silently fail to open instead of crashing the app.
static void makeGnuplotFindable()
{
#if defined(__APPLE__) || defined(__linux__)
    const char *existingPath = std::getenv("PATH");
    const std::string newPath = std::string("/opt/homebrew/bin:/usr/local/bin:") + (existingPath ? existingPath : "");
    setenv("PATH", newPath.c_str(), 1);
    std::signal(SIGPIPE, SIG_IGN);
#endif
}

int main()
{
    makeGnuplotFindable();
    chdirToExecutableDirectory();

    App app;
    return app.run();
}
