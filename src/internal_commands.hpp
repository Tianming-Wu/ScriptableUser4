#pragma once
#include "scvar.hpp"
#include "sctype.hpp"

#define __intcs(Name) int Name(__fn_sig)
#define __int_register(Name) registerCommand({Name}, Name)

namespace ScUser {

void Internal_Init(std::function<void(const std::stringlist&,__fn_lambda)>, scvar& svar, sctype &stype);

__intcs(print);
__intcs(println);
__intcs(var);

} // namespace ScUser