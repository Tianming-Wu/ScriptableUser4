#include "sctype.hpp"
#include "scvar.hpp"

int sctype::registerType(const std::string& name, size_t size,
    __deffmt_lambda default_formatter, __fmt_lambda custom_formatter) {
    m_basetypes[name] = { _p_GetNewID(), size, default_formatter, custom_formatter };
    return m_basetypes[name].type_id;
}

int sctype::registerMetaType(const std::string &name, std::initializer_list<std::pair<std::string, submetatypeinfo>> subtypes,
    __deffmt_lambda default_formatter, __fmt_lambda custom_formatter) {
    metatypeinfo mti { _p_GetNewID(), 0, {}, default_formatter, custom_formatter };
    for(auto &[subname, submetadata] : subtypes) {
        mti.members[subname] = submetadata;
        mti.size += submetadata.dataSize;
    }
    m_metatypes[name] = mti;
    return mti.type_id;
}

std::string sctype::defualt_format_type(scvar &vars, const std::string &var_name, const std::string &type_name) const {
    try {
        if(isBaseType(type_name)) {
            if(m_basetypes.at(type_name).default_formatter != nullptr)
                return m_basetypes.at(type_name).default_formatter(vars[var_name].toByteArray());
        } else if(isMetaType(type_name)) {
            if(m_metatypes.at(type_name).default_formatter != nullptr)
                return m_metatypes.at(type_name).default_formatter(vars[var_name].toByteArray());
        }
    
        return vars[var_name].toString(); // format like a bytearray (just output)

    } catch (const std::format_error &e) {
        throw std::runtime_error(std::string("format_string: format_error triggered: ") + e.what());
    }
}

std::string sctype::custom_format_type(scvar &vars, const std::string &var_name, const std::string &type_name, const std::string &_format) const {
    try {
        if(isBaseType(type_name)) {
            return m_basetypes.at(type_name).custom_formatter(_format, vars[var_name].toByteArray());
        } else if(isMetaType(type_name)) {
            return m_metatypes.at(type_name).custom_formatter(_format, vars[var_name].toByteArray());
        }
    
        return vars[var_name].toString(); // format like a bytearray (just output)

    } catch (const std::format_error &e) {
        throw std::runtime_error(std::string("format_string: format_error triggered: ") + e.what());
    }
}

std::string sctype::format_subtype(scvar &vars, const std::string &var_name, const std::string &subvar_name, const std::string &_format) const {
    if(vars.contains(var_name)) {
        const scvar::var &var = vars[var_name];

        int vtype = var.type();
        bool isMetaType;
        std::string vstype = id2name(vtype, isMetaType);
        if(!isMetaType) throw std::runtime_error("typeerror: attempt to take member of " + var_name + ", which is not a sturct");

        const metatypeinfo &mti = m_metatypes.at(vstype);
        if(!mti.members.contains(subvar_name)) throw std::runtime_error("typeerror: " + subvar_name + " is not a member of " + vstype);
        const submetatypeinfo &smti = mti.members.at(subvar_name);
        
        std::bytearray ba = std::bytearray::fromHex(var.toString());
        ba = ba.subarr(smti.offset, smti.dataSize);

        if(!m_basetypes.contains(smti.type))
            throw std::runtime_error("typeerror: attempt taking member " + subvar_name + " with an invalid type of " + smti.type + "(not registered)\n\t"
                "Is the type registered correctly?"
            );
        const basetypeinfo &mbti = m_basetypes.at(subvar_name);
        return mbti.custom_formatter(_format, ba);

    } else throw std::runtime_error("typeerror: attempt to interpret " + var_name + " as a struct, which doesn't exists");
}

int sctype::_p_GetNewID() {
    static int baseid = 0;
    return ++baseid;
}
