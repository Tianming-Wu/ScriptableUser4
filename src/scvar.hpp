#pragma once

#include <string>
#include <map>
#include <SharedCppLib2/stringlist.hpp>
#include <SharedCppLib2/bytearray.hpp>
#include <regex>

#include "sctype.hpp"
#include "scdef.hpp"

// variable handler

class scvar {
public:
    scvar(scvar&) = delete;
    scvar(scvar&&) = delete;

    scvar(sctype &typeref);

    void set(const std::string &key, const std::string &value);
    void remove(const std::string &key);
    void swap(const std::string &key1, const std::string &key2);

    class var {
    public:
        inline var() :m_type(0) {}

        var(const std::string& s);
        var(const char* s);

        var(const std::stringlist &s);

        inline std::string toString() const { return m_value.tostdstring(); }
        std::stringlist toStringList() const;

        inline std::bytearray toByteArray() const { return m_value; };

        std::bytearray& value() { return m_value; }
        inline int type() const { return m_type; }

        inline void setConst(bool con) { m_const = true; }
        inline bool isConst() const { return m_const; }

        inline void _M_inject(const std::bytearray& value, int type) {
            m_value = value; m_type = type;
        }

        template<typename _T>
        void meta_store(const _T &v) {
            constexpr auto type_name = []() constexpr {
                std::string_view fn = SC_FUNC_SIG;
                auto begin = fn.find("_T=") + 3;
                auto end = fn.find(">", begin);
                return fn.substr(begin, end - begin);
            }();

            _M_inject(std::bytearray(v) , scvar::m_types.name2id(std::string(type_name)));
        }

        template<typename _T>
        _T meta_read() const {
            return m_value.convert_to<_T>();
        }

    private:
        int m_type;
        std::bytearray m_value;
        bool m_const = false;
    };

    inline bool contains(const std::string &key) { return m_variables.contains(key); }
    inline std::map<std::string, var>::mapped_type & operator[](const std::string &key) { return m_variables[key]; }

protected:
    std::map<std::string, var> m_variables;
    static sctype &m_types;
};