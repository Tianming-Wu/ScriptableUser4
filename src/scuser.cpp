#include "scuser.hpp"
#include "platform.hpp"

#include <fstream>
#include <regex>
#include <stdlib.h>
#include <format>

#include <SharedCppLib2/dbgtools.hpp>

scuser::scuser()
    : m_state(sc::InvalidState), m_vars(m_types)
{
}

scuser::~scuser()
{}

void scuser::unload() {
    for (auto& [handle, pluginInfo] : m_plugins) {
        FreeLibrary(handle);
    }
}

void scuser::init()
{
    // initialize environment variables
    for(int i = 0; environ[i]; i++) {
        std::string envd = environ[i];
        size_t posx = envd.find("=");
        std::string name = envd.substr(0, posx), value = envd.substr(posx+1);
        m_environ_s.push_back(envdata { name, value });
    }

    // initialize static variables
    m_vars["SCUSER_DIR"] = pf::executableDir().string();
    m_vars["SCUSER_EXE"] = pf::executablePath().string();

    // reserved command names
    // m_reservedCommands = std::stringlist({"for if while repeat"});
}

void scuser::loadScript(fs::path file)
{
    if(!fs::exists(file)) throw std::runtime_error("script file not exist");
    m_script_file = fs::absolute(file);

    std::ifstream ifs(m_script_file);
    if(ifs.bad()) throw std::runtime_error("failed to open script file");

    m_script_content.clear();

    std::string lnbuf;
    while(std::getline(ifs, lnbuf)) {
        m_script_content.push_back(lnbuf);
    }

    // load tags
    for(size_t lnc = 0; lnc < m_script_content.size(); lnc++) {
        if(!m_script_content[lnc].empty() && m_script_content[lnc][0] == ':') {
            m_tags.insert(std::make_pair(m_script_content[lnc].substr(1), lnc));
        }
    }

    m_state = sc::Loaded;
}

void scuser::loadParam(const std::stringlist &script_param) { m_script_param = script_param; }

int scuser::exec()
{
    bool running = true;
    int ret = 0;
    size_t curln = 0;
    
    // builtin commands
    registerCommand({{"exit"},          [&](__fn_sig) { running = false; return std::stoi(sl.vat(0,"0")); } });
    registerCommand({{"restart"},       [&](__fn_sig) { curln = 0; return 0; } });
    registerCommand({{"jump","goto"},   [&](__fn_sig) {
        if(m_tags.contains(sl.vat(0))) {
            curln = m_tags[sl.at(0)];
        } else throw std::runtime_error("jump/goto: tag not exists");
        return 0;
    } });
    registerCommand({{"if"},        [&](__fn_sig) {
        std::stringlist binded = std::stringlist::exsplit(sl.join(), " ", "(", ")");
        bool reverse = false;
        if(binded[0] == "not") {
            reverse = true;
            binded = binded.subarr(1); // remove the first element
        }

        std::stringlist cond = std::stringlist::xsplit(binded.at(0), " ", "\'\"");
        // std::stringlist retl;

        std::stringlist cmd1 = std::stringlist::xsplit(binded.at(1), " ", "\'\"");
        std::stringlist cmd2 = std::stringlist::xsplit(binded.at(2), " ", "\'\"");
        
        int cond_ret, cmd_ret;
        bool cf = call(cond, var, retl, cond_ret);
        bool cc = call(((cond_ret > 0) ^ reverse)? cmd1 : cmd2, var, retl, cmd_ret);

        if(!(cf && cc)) throw std::runtime_error("if: cannot found one of the statements or commands");

        return 0;
    } });
    registerCommand({{"loadplugin","require","include"},[&](__fn_sig) {
        loadPlugin(sl.vat(0));
        return 0;
    }} );

    // builtin variables
    // other constant variables are set in init()
    m_vars["ERRORLEVEL"] = 0;
    m_vars["SCRIPT_DIR"] = m_script_file.parent_path().string();
    m_vars["SCRIPT_FILE"] = m_script_file.string();

    m_state = sc::Running;

    while(running && curln < m_script_content.size()) {
        const std::string& ln = m_script_content[curln];
        if(ln.empty() || ln.substr(0, 2) == "//" || ln[0] == ':') { // empty line, tag or comment
            curln++;
            continue;
        }

        std::stringlist lncom = std::stringlist::xsplit(ln, " ", "\'\"");
        std::stringlist retl;
        lncom.remove_empty();

        size_t equpos;
        if((equpos = lncom.find("->")) != std::stringlist::npos) {
            retl = lncom.subarr(equpos + 1);
            lncom = lncom.subarr(0, equpos);
        } 

        lncom.exec_foreach([=](size_t id, std::string &str) { str = translate(str); });
        
        if(!call(lncom, m_vars, retl, ret)) {
            std::cout << "command \"" << lncom[0] << "\" not found on line " << curln << std::endl;
            running = false;
            ret = 1;
        } else if(m_strict && ret) {
            running = false;
        }

        curln++;
    }

    if(running && ret) { // if not quit normally
        std::cout << "Script crashed with return code " << ret << " at line " << curln << std::endl;
    }

    m_state = sc::Loaded;
    return ret;
}

void scuser::registerCommand(const sccmd &cmd) {
    m_commands.push_back(cmd);
    for (const auto& name : cmd.command) {
        m_command_index.insert(std::make_pair(name, std::ref(m_commands.back())));
    }
}

std::string scuser::translate(std::string source) {
    static std::regex namepattern ("[A-Za-z0-9_]+", std::regex::optimize);
    static std::regex namespattern ("[A-Za-z0-9_]+\\.[A-Za-z0-9_]+", std::regex::optimize);
    static std::regex numpattern("\\d+", std::regex::optimize);

    size_t lpos = 0, pos;
    while((pos = source.find('$', lpos)) != std::string::npos) { // translate all variables.
        if (pos >= 1 && source[pos-1] == '\\') { // skip special characters
            lpos = pos+1;
            continue;
        }

        if(source[pos+1] != '{')
            throw std::runtime_error("translate(): wrong varient format: '$' must be followed with '{...}'.");

        size_t poslb = pos + 1, posrb = std::string::npos, br_depth = 1;
        for(size_t i = poslb + 1; i < source.length(); i++) {
            if(source[i] == '{') br_depth++;
            else if(source[i] == '}') {
                if(--br_depth == 0) {
                    posrb = i;
                    break;
                } 
            }
        }

        if (posrb == std::string::npos)
            throw std::runtime_error("translate(): wrong varient format: bracket '{}' not closed.");

        std::string varient_name = source.substr(poslb+1, posrb-poslb-1), replace_string = "";
        std::string subvar_name;
        std::map<char,std::stringlist> format_option;

        size_t optpos; // size_t formatting_length = 0;
        if((optpos = varient_name.find(':')) != std::string::npos) {
            std::stringlist formatting_option = std::stringlist::exsplit(varient_name.substr(optpos+1), ",", "[{", "]}");
            for(const std::string &s : formatting_option) {
                std::stringlist optl = std::stringlist::exsplit(s.substr(2, s.length()-3), ",", "[{\"\'", "]}\"\'");
                optl.exec_foreach([](size_t id, std::string &s) {
                    if(s[0] == s[s.length()-1] && std::charmatch(s[0], "\"\'")) s = s.substr(1, s.length()-2);
                });
                format_option[s.at(0)] = optl;
            }
            // formatting_length = varient_name.length() - optpos;
            varient_name = varient_name.substr(0, optpos);
        }

        if(!std::regex_match(varient_name, namepattern)) {
            if(std::regex_match(varient_name, namespattern)) {
                size_t dotpos = varient_name.find(".");
                subvar_name = varient_name.substr(dotpos+1);
                varient_name = varient_name.substr(0, dotpos);
            } else throw std::runtime_error("translate(): wrong varient format. varient name can only include A-Z,a-z,0-9,\"_\"");
        }
            

        if(std::regex_match(varient_name, numpattern)) { // is only number, refer to parameters
            int paramId = std::stoi(varient_name);
            replace_string = m_script_param.vat(paramId);
        } else { // is not number only
            if(m_vars.contains(varient_name)) {
                if(!format_option.empty()) {
                    if(format_option.contains('f')) {
                        if(!subvar_name.empty()) {
                            replace_string = m_types.format_subtype(m_vars, varient_name, subvar_name, format_option['f'][0]);
                        } else {
                            replace_string = m_types.custom_format_type(m_vars, varient_name, format_option['f'][1], format_option['f'][0]);
                        }
                    }
                } else {
                    bool isMetaType;
                    std::string type_name = m_types.id2name(m_vars[varient_name].type(), isMetaType);
                    if(m_types.hasDefaultFormatter(type_name)) {
                        m_types.defualt_format_type(m_vars, varient_name, type_name);
                    } else replace_string = m_vars[varient_name].toString();
                }
            }
            else { //Search system environment variable.
                for(envdata d : m_environ_s) {
                    if(d.name == varient_name) {
                        replace_string = d.value;
                        break;
                    }
                }
            }
        }

        size_t vsize = posrb-pos+1;
        source.replace(pos, vsize, replace_string);
        // lpos = pos+replace_string.length();
        lpos = pos; // This allows ${...} in variables to be unpacked 'recursively'.
    }

    return unescape_string(source);
}

std::string scuser::unescape_string(const std::string &orig)
{
    static const std::map<char, char> escape_map = {
        {'\\', '\\'}, {'n', '\n'}, {'t', '\t'}, {'b', '\b'},
        {'r', '\r'}, {'f', '\f'}, {'a', '\a'}, {'v', '\v'},
        {'\'', '\''}, {'\"', '\"'}, {'?', '\?'}, {'0', '\0'}
    };

    std::string result = orig, replace;
    size_t pos, lpos = 0;
    while ((pos = result.find("\\", lpos)) != std::string::npos) {
        if (pos + 1 >= result.size()) break;
        char next_char = result[pos + 1];
        
        if (escape_map.count(next_char)) {
            replace = escape_map.at(next_char);
            result.replace(pos, 2, replace);
        }
        lpos = pos + 1;
    }

    return result;
}

bool scuser::call(const std::stringlist &cmdline, scvar &var, const std::stringlist &retl, int &ret)
{
    auto it = m_command_index.find(cmdline[0]);
    if (it != m_command_index.end()) {
        const sccmd& cmd = it->second.get();
        ret = cmd.callable(cmdline.subarr(1), var, retl);
        var["ERRORLEVEL"].meta_store(ret);
        return true;
    }
    return false;
}

void scuser::loadPlugin(const std::string &name) {
    for(auto &[hmodule, pinfo] : m_plugins) {
        if(pinfo.name == name) // throw std::runtime_error("plugin already loaded");
            return; // prevent repeated load
    }

    fs::path plugin_path = pf::executableDir() / "scplugins" / (name + ".dll");
    if(!fs::exists(plugin_path)) plugin_path = pf::executableDir() / "scplugins" / (name + ".scup");
    if(!fs::exists(plugin_path)) throw std::runtime_error("plugin " + name + " not found in ./scplugins");

    HMODULE handle = LoadLibraryW(plugin_path.wstring().c_str());
    if (!handle) throw std::runtime_error("failed to load plugin: " + name);

    auto load_func = (pluginLoadFunc)GetProcAddress(handle, "scPluginMain");
    auto api_version_fn = (getPluginApiVersion)GetProcAddress(handle, "scPluginApiVersion");

    if (!api_version_fn) {
        FreeLibrary(handle);
        throw std::runtime_error("Api version not found for plugin " + name);
    }

    if (!load_func) {
        FreeLibrary(handle);
        throw std::runtime_error("scPluginMain entry not found for plugin " + name);
    }

    pluginInfo pinfo;
    pinfo.apiVersion = api_version_fn();
    if(pinfo.apiVersion != ScPluginApiVersion)
        throw std::runtime_error("attempt to load plugin " + name + " with an incompatible api version " + std::to_string(pinfo.apiVersion) + "\n\t"
        "(program: " + std::to_string(ScPluginApiVersion) + ")"
    );

    auto register_callback = [this, handle](
        const std::stringlist& commands, __fn_lambda callable
    ) {
        registerCommand({commands, callable, handle});
    };

    
    if (!load_func(register_callback, m_vars, m_types)) {
        FreeLibrary(handle);
        throw std::runtime_error("plugin initialization failed: " + name);
    }

    m_plugins[handle] = pinfo;
}
