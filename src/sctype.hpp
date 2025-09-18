#pragma once

#include <string>
#include <map>
#include <SharedCppLib2/stringlist.hpp>
#include <SharedCppLib2/bytearray.hpp>
#include <regex>

#include "scdef.hpp"

// #include "scvar.hpp"
class scvar;

class sctype {
public:

    struct submetatypeinfo {
        std::string type;
        size_t dataSize;
        size_t offset;
    };

    struct metatypeinfo {
        int type_id = 0;
        size_t size;
        std::unordered_map<std::string, submetatypeinfo> members;
        __deffmt_lambda default_formatter;
        __fmt_lambda custom_formatter;
    };

    struct basetypeinfo {
        int type_id = 0;
        size_t size;
        __deffmt_lambda default_formatter;
        __fmt_lambda custom_formatter;
    };

    int registerType(const std::string& name, size_t size,
        __deffmt_lambda default_formatter = nullptr, __fmt_lambda custom_formatter = nullptr);
    int registerMetaType(const std::string &name, std::initializer_list<std::pair<std::string, submetatypeinfo>> subtypes,
        __deffmt_lambda default_formatter = nullptr, __fmt_lambda custom_formatter = nullptr);

    inline bool isBaseType(const std::string &name) const { return m_basetypes.contains(name); }
    inline bool isMetaType(const std::string &name) const { return m_metatypes.contains(name); }

    inline bool hasDefaultFormatter(const std::string &name) const {
        return (isBaseType(name) && m_basetypes.at(name).default_formatter != nullptr) || (isMetaType(name) && m_metatypes.at(name).default_formatter != nullptr);
    }
    inline bool hasCustomFormatter(const std::string &name) const {
        return (isBaseType(name) && m_basetypes.at(name).custom_formatter != nullptr) || (isMetaType(name) && m_metatypes.at(name).custom_formatter != nullptr);
    }

    template<typename T>
    static std::string format_var(const std::string& format, const T& value) {
        return std::vformat(format, std::make_format_args(value));
    }

    std::string defualt_format_type(scvar &vars, const std::string &var_name, const std::string &type_name) const;
    std::string custom_format_type(scvar &vars, const std::string &var_name, const std::string &type_name, const std::string &_format) const;

    std::string format_subtype(scvar &vars, const std::string &var_name, const std::string &subvar_name, const std::string &_format) const;

    inline std::string id2name(int id, bool &isMetaType) const {
        for(auto &[name, bti] : m_basetypes) if(bti.type_id == id) { isMetaType = false; return name; }
        for(auto &[name, mti] : m_metatypes) if(mti.type_id == id) { isMetaType = true; return name; }
        return "";
    };

    inline int name2id(const std::string &name) {
        if(m_basetypes.contains(name)) return m_basetypes.at(name).type_id;
        if(m_metatypes.contains(name)) return m_metatypes.at(name).type_id;
        return -1;
    }

private:
    int _p_GetNewID();

private:
    std::map<std::string, basetypeinfo> m_basetypes;
    std::map<std::string, metatypeinfo> m_metatypes;
};

#define sc_make_metasubtype(BASETYPE, TYPE, NAME) { std::string(#NAME), { std::string(#TYPE), sizeof(TYPE), offsetof(BASETYPE, NAME) }}
// #define sc_register_type_cvlambda(TYPE) [](const scvar::var &var) { return var.meta_read<TYPE>(); }

/*
    Here's the plan:
        pass in another std::function object, which is the lambda for registering metatypes.

        plugins uses them like this:

            fmt.registerMetaType("RECT",
            {
                sc_make_metasubtype(RECT, top, uint64_t)
            });


        (or we can start to consider making a whole api class, binded to the member values)

*/