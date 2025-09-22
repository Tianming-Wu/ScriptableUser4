#pragma once

#define ScPluginApiVersion 9


// simplify code
#define __fn_sig std::stringlist sl, scvar &var, std::stringlist retl
#define __fmt_sig const std::string &_format, const std::bytearray &ba
#define __deffmt_sig const std::bytearray &ba
#define __fn_lambda std::function<int(std::stringlist,scvar&,std::stringlist)>
#define __fmt_lambda std::function<std::string(const std::string&,const std::bytearray&)>
#define __deffmt_lambda std::function<std::string(const std::bytearray&)>


// complier spec
#ifdef _MSC_VER
#   define SC_FUNC_SIG __FUNCSIG__
#else
#   define SC_FUNC_SIG __PRETTY_FUNCTION__
#endif