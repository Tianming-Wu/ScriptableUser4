#include "internal_commands.hpp"

namespace ScUser {

__intcs(print) {
    std::cout << sl.join();
    return 0;
}

__intcs(println) {
    std::cout << sl.join() << std::endl;
    return 0;
}

__intcs(var) {
    if(sl.empty()) throw std::runtime_error("var: no subcommand");
    if(sl[0] == "set") {
        if(sl.size() != 3) throw std::runtime_error("var set: require 2 parameters");
        var.set(sl[1], sl[2]);
    } else if(sl[0] == "remove") {
        if(sl.size() != 2) throw std::runtime_error("var remove: require 1 parameter");
        var.remove(sl[1]);
    } else if(sl[0] == "swap") {
        if(sl.size() != 3) throw std::runtime_error("var swap: require 2 parameters");
        var.swap(sl[1], sl[2]);
    } else throw std::runtime_error("var: unknown subcommand");
        
    return 0;
}

void Internal_Init(std::function<void(const std::stringlist&,__fn_lambda)> register_func, scvar& svar, sctype& stype) {
    register_func({"print"}, print);
    register_func({"println"}, println);
    register_func({"var"}, var);

    stype.registerType("int8_t", sizeof(int8_t),    [](__deffmt_sig) { return std::format("{}", ba.convert_to<int8_t>()); },
                                                    [](__fmt_sig) { return sctype::format_var(_format, ba.convert_to<int8_t>());});
    stype.registerType("int16_t", sizeof(int16_t),  [](__deffmt_sig) { return std::format("{}", ba.convert_to<int16_t>()); },
                                                    [](__fmt_sig) { return sctype::format_var(_format, ba.convert_to<int16_t>());});
    stype.registerType("int32_t", sizeof(int32_t),  [](__deffmt_sig) { return std::format("{}", ba.convert_to<int32_t>()); },
                                                    [](__fmt_sig) { return sctype::format_var(_format, ba.convert_to<int32_t>());});
    stype.registerType("int64_t", sizeof(int64_t),  [](__deffmt_sig) { return std::format("{}", ba.convert_to<int64_t>()); },
                                                    [](__fmt_sig) { return sctype::format_var(_format, ba.convert_to<int64_t>());});
    stype.registerType("uint8_t", sizeof(uint8_t),  [](__deffmt_sig) { return std::format("{}", ba.convert_to<uint8_t>()); },
                                                    [](__fmt_sig) { return sctype::format_var(_format, ba.convert_to<uint8_t>());});
    stype.registerType("uint16_t", sizeof(uint16_t),[](__deffmt_sig) { return std::format("{}", ba.convert_to<uint16_t>()); },
                                                    [](__fmt_sig) { return sctype::format_var(_format, ba.convert_to<uint16_t>());});
    stype.registerType("uint32_t", sizeof(uint32_t),[](__deffmt_sig) { return std::format("{}", ba.convert_to<uint32_t>()); },
                                                    [](__fmt_sig) { return sctype::format_var(_format, ba.convert_to<uint32_t>());});
    stype.registerType("uint64_t", sizeof(uint64_t),[](__deffmt_sig) { return std::format("{}", ba.convert_to<uint64_t>()); },
                                                    [](__fmt_sig) { return sctype::format_var(_format, ba.convert_to<uint64_t>());});

    stype.registerType("bool", sizeof(bool),        [](__deffmt_sig) { return std::string(ba.convert_to<bool>() ? "true" : "false"); });
    stype.registerType("double", sizeof(double),    [](__deffmt_sig) { return std::format("{}", ba.convert_to<double>()); },
                                                    [](__fmt_sig) { return sctype::format_var(_format, ba.convert_to<double>());} );
    stype.registerType("char", sizeof(char),        [](__deffmt_sig) { return std::format("{}", ba.convert_to<char>()); },
                                                    [](__fmt_sig) { return sctype::format_var(_format, ba.convert_to<char>());  } );
    stype.registerType("unsigned char", sizeof(unsigned char), [](__deffmt_sig) { return std::format("{}", ba.convert_to<unsigned>()); },
                                                    [](__fmt_sig) { return sctype::format_var(_format, ba.convert_to<unsigned char>());});

    stype.registerType("uintptr_t", sizeof(uintptr_t), [](__deffmt_sig){ return std::format("{}", ba.convert_to<uintptr_t>()); },
                                                    [](__fmt_sig) { return sctype::format_var(_format, ba.convert_to<uintptr_t>());});
}


} // namespace ScUser