#pragma once

#include <string>
#include <filesystem>
#include <functional>
#include <vector>
#include <map>
#include <SharedCppLib2/stringlist.hpp>

#include <windows.h>

#include "scdef.hpp"
#include "scvar.hpp"
#include "sctype.hpp"

namespace fs = std::filesystem;

namespace sc {
    enum UserState {
        InvalidState = -1,
        Loaded = 0,
        Running = 1
    };
}

struct pluginInfo {
    std::string name, version;
    int apiVersion;
};

using pluginRegisterFunc = std::function<void(
    const std::stringlist& commands,
    __fn_lambda callable
)>;
using getPluginApiVersion = int(*)();
using pluginLoadFunc = bool(*)(const pluginRegisterFunc&,scvar&,sctype&);

// command
struct sccmd {
    std::stringlist command;
    __fn_lambda callable;
    HMODULE handle = 0;
};

class scuser
{
public:
    scuser();
    ~scuser();

    void init();

    void loadScript(fs::path file);
    void loadParam(const std::stringlist& script_param);
    int exec();

    void registerCommand(const sccmd& cmd);

    // causes severe lag for no reason
    void unload();
    
    inline scvar& _M_getVar() { return m_vars; };
    inline sctype& _M_getType() { return m_types; };

private:
    std::string translate(std::string source);
    std::string unescape_string(const std::string &orig);
    bool call(const std::stringlist &cmdline, scvar &var, const std::stringlist &retl, int &ret);

    void loadPlugin(const std::string &name);

private:
    fs::path m_script_file;
    std::stringlist m_script_content;
    std::stringlist m_script_param;
    sc::UserState m_state;

    std::map<std::string, size_t> m_tags;
    sctype m_types;
    scvar m_vars;

    std::list<sccmd> m_commands;
    std::unordered_map<std::string, std::reference_wrapper<const sccmd>> m_command_index;
    std::map<HMODULE, pluginInfo> m_plugins;

    struct envdata { std::string name, value; };
    std::vector<envdata> m_environ_s;

    bool m_strict = false;
};