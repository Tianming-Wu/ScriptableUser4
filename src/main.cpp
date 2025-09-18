#include "main.hpp"
#include "scuser.hpp"
#include "internal_commands.hpp"

#include <SharedCppLib2/stringlist.hpp>

int main(int argc, char** argv) {
    std::stringlist args(argc, argv);

    ///TODO: complete argument parsing system
    // size_t ap = 1; // ignore the executable name
    // while(ap < args.size()) {

    // }

    fs::path file;
    if(argc > 1) {
        file = args.vat(1, "");
    }

    // if not specified, and autorun script exists, run it.
    if(!fs::exists(file) && fs::exists("autorun.scu"))
        file = fs::absolute("autorun.scu");

    scuser user;
    user.init();

    ScUser::Internal_Init([&] (const std::stringlist& commands, __fn_lambda callable) {
        user.registerCommand({commands, callable, 0});
    }, user._M_getVar(), user._M_getType());

    try {
        user.loadScript(file);
    } catch (std::runtime_error e) {
        std::cout << "failed to load script:\n\t" << e.what();
        return 1;
    }

    int ret;
    try {
        ret = user.exec();
    } catch (std::runtime_error e) {
        std::cout << "failed to execute script:\n\t" << e.what();
        return 1;
    }

    // user.unload();
    return ret;
}