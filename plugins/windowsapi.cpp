#include "windowsapi.hpp"

#include <windows.h>
#include <format>
#include <SharedCppLib2/dbgtools.hpp>

__plugin_load_signature(register_command, vars, types) {
    register_command({ "iswindow" }, windowsApi::iswindow);
    register_command({ "findwindow" }, windowsApi::findWindow);
    register_command({ "showwindowtitle" }, windowsApi::showTitle);
    register_command({ "getwindowrect" }, windowsApi::getWindowRect);

    types.registerMetaType("RECT", {
        sc_make_metasubtype(RECT, int64_t, left),
        sc_make_metasubtype(RECT, int64_t, top),
        sc_make_metasubtype(RECT, int64_t, right),
        sc_make_metasubtype(RECT, int64_t, bottom)
    }, [](__deffmt_sig) {
        RECT rc = ba.convert_to<RECT>();
        return std::format("({},{}),{}x{}", rc.top, rc.left, rc.bottom-rc.top, rc.right-rc.left);
    });

    return true;
}

namespace windowsApi {

int findWindow(__fn_sig) {
    HWND hwnd = FindWindowA(sl.vat(0).c_str(), sl.vat(1).c_str());
    var[retl.vat(0, "hwnd")].meta_store(hwnd);
    return (hwnd == NULL)?1:0;
}

int showTitle(__fn_sig) {
    HWND hwnd = var[sl.vat(0, "hwnd`")].meta_read<HWND>();
    if(IsWindow(hwnd)) {
        int len = GetWindowTextLengthA(hwnd) + 1;
        std::string cont; cont.resize(len);
        GetWindowTextA(hwnd, cont.data(), len);
        std::cout << cont << std::endl;
    } else return 1;
    return 0;
}

int getWindowRect(__fn_sig) {
    HWND hwnd = var[sl.vat(0, "hwnd")].meta_read<HWND>();
    if(!IsWindow(hwnd)) throw std::runtime_error("windowsapi: not a valid window handle");

    RECT wndRect;
    ::GetWindowRect(hwnd, &wndRect);
    var[retl.vat(0, "rect")].meta_store(wndRect);
    return 0;
}

int click(__fn_sig) {
    HWND hwnd = var[sl.vat(0, "hwnd")].meta_read<HWND>();
    int x = std::stoi(sl.vat(1, "0")),
        y = std::stoi(sl.vat(2, "0"));

    SendMessage(hwnd, WM_LBUTTONDOWN, 0, 0);
    SendMessage(hwnd, WM_LBUTTONUP, 0, 0);
    return 0;
}

int iswindow(__fn_sig) {
    HWND hwnd = var[sl.vat(0, "hwnd")].meta_read<HWND>();
    var[retl.vat(0, "result")].meta_store(IsWindow(hwnd));
    return 0;
}

} // namespace windowsApi