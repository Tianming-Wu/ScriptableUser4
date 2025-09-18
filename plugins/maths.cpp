#include "maths.hpp"
#include <regex>

__plugin_load_signature(register_command, vars, types) {
    register_command({"maths"}, scmaths::maths);

    return true;
}

namespace scmaths {

int maths(__fn_sig) {
    bool outp = false;
    std::stringlist ref = sl;

    if(sl.contains("=")) {


    } else {
        outp = true;
    }


    static std::regex simple_number_pattern("-?\\d+");
    static std::regex simple_exp_pattern("-?\\d+ ?[+-*/] ?-?\\d+");

    if(std::regex_match(sl.vat(0), simple_number_pattern)) {
        
    }


    return 0;
}

} // namespace scmaths