#include "platform.hpp"

namespace pf {

fs::path executablePath() {
    static std::string cached = [&]() {
        std::string path;
        path.resize(SC_MAX_PATH);
        GetModuleFileNameA(NULL, path.data(), SC_MAX_PATH);
        if(path.empty()) throw std::runtime_error("executablePath(): failed");
        return path;
    } ();
    return cached;
}

fs::path executableDir() {
    static std::string cached = [&]() {
        return fs::path(executablePath()).parent_path().string();
    } ();
    return cached;
}

} // namespace pf