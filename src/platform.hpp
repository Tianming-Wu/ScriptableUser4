#pragma once

#include <filesystem>

#ifdef _WIN64
#   include <windows.h>
#   define SC_MAX_PATH MAX_PATH
#else
#   include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace pf {

fs::path executablePath();
fs::path executableDir();

} //namespace pf