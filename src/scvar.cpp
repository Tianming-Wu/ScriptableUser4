#include "scvar.hpp"

scvar::var::var(const std::string& s)
    :m_value(s), m_type(scvar::m_types.name2id("string"))
{}

scvar::var::var(const char* s)
    :m_value(s), m_type(scvar::m_types.name2id("string"))
{}

scvar::var::var(const std::stringlist &s)
    :m_value(s.pack()), m_type(scvar::m_types.name2id("stringlist"))
{}

std::stringlist scvar::var::toStringList() const {
    if(m_type != scvar::m_types.name2id("stringlist")) return std::stringlist::split(m_value.tostdstring(), " "); // use default split
    else return std::stringlist::unpack(m_value.tostdstring());
}

scvar::scvar(sctype &typeref)
{
    m_types = typeref;
    m_types.registerType("string", 0, [](__deffmt_sig) { return ba.tostdstring(); }, [](__fmt_sig) { return sctype::format_var(_format, ba.tostdstring()); });
    m_types.registerType("stringlist", 0, [](__deffmt_sig) { return ba.tostdstring(); },
    [](__fmt_sig) {
        return ba.tostringlist().join();
    });
}

void scvar::remove(const std::string &key)
{
    for(auto it = m_variables.begin(); it != m_variables.end(); it++) {
        if((*it).first == key) {
            m_variables.erase(it);
            break;
        }
    }
}

void scvar::set(const std::string &key, const std::string &value)
{
    if(key.empty()) return;
    if(!m_variables.contains(key)) m_variables.insert(std::make_pair(key, value));
    else m_variables[key] = value;
}

void scvar::swap(const std::string &key1, const std::string &key2)
{
    std::map<std::string, var>::iterator it1, it2;

    ///TODO: optimize this
    for(auto it = m_variables.begin(); it != m_variables.end(); it++) {
        if((*it).first == key1) {
            it1 = it;
        } else if((*it).first == key2) {
            it2 = it;
        }
    }

    if((*it1).second.type() != (*it2).second.type())
        throw std::runtime_error("var swap: cannot swap variables with different types");

    std::bytearray temp = (*it1).second.value();
    (*it1).second.value() = (*it2).second.value();
    (*it2).second.value() = temp;
}
