#pragma once
#include "scPluginApi.hpp"

namespace windowsApi {

int iswindow(__fn_sig);

int findWindow(__fn_sig);
int click(__fn_sig);

int getWindowRect(__fn_sig);

int showTitle(__fn_sig);

} //namespace windowsApi