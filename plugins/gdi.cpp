#include "gdi.hpp"
#include <windows.h>

__plugin_load_signature(register_command, vars, types) {
    register_command({"beginpaint"}, wingdi::BeginPaint);
    register_command({"endpaint"}, wingdi::EndPaint);

    return true;
}

namespace wingdi {

int BeginPaint(__fn_sig) {
    PAINTSTRUCT ps;
    HWND hwnd = var["hwnd"].meta_read<HWND>();
    HDC hdc = ::BeginPaint(hwnd, &ps);


    var[retl.vat(0, "ps")].meta_store(ps);


    return 0;
}


int EndPaint(__fn_sig) {

    HWND hwnd = var[sl.vat(0, "hwnd")].meta_read<HWND>();
    PAINTSTRUCT ps = var[sl.vat(1, "ps")].meta_read<PAINTSTRUCT>();

    ::EndPaint(hwnd, &ps);

    return 0;
}

} // namespace wingdi