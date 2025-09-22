#pragma once
#include <string>
#include <functional>
#include "scdef.hpp"
#include "scvar.hpp"
#include "sctype.hpp"

struct pluginInfo {
    std::string name, version;
};

#define __plugin_load_signature(RC_NAME, VAR_NAME, TYPE_NAME) extern "C" __declspec(dllexport) bool scPluginMain( \
    const std::function<void( \
        const std::stringlist&,__fn_lambda \
    )>& RC_NAME, \
    scvar& VAR_NAME, \
    sctype& TYPE_NAME \
)

#ifndef SCPLUGIN_DEFUALT_API
extern "C" __declspec(dllexport) int scPluginApiVersion() { return ScPluginApiVersion; }
extern "C" __declspec(dllexport) std::function<sctype&()> getTypeSystem(std::function<sctype&()> _del) { static std::function<sctype&()> del = _del; return del; }
#define SCPLUGIN_DEFAULT_API
#endif