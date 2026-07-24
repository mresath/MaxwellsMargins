#include "core/App.hpp"

#include <filesystem>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
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

int main()
{
    chdirToExecutableDirectory();

    App app;
    return app.run();
}
