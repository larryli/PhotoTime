#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>

#include "commctrls.h"

void InitCommCtrl(void)
{
    INITCOMMONCONTROLSEX icc = {
        .dwSize = sizeof(INITCOMMONCONTROLSEX),
        .dwICC = ICC_WIN95_CLASSES,
    };
    InitCommonControlsEx(&icc);
}

BOOL IsCommCtrlVersion6(void)
{
    static BOOL isCommCtrlVersion6 = -1;
    if (isCommCtrlVersion6 != -1)
        return isCommCtrlVersion6;

    isCommCtrlVersion6 = FALSE;

    HINSTANCE commCtrlDll = LoadLibrary(L"comctl32.dll");
    if (commCtrlDll) {
        DLLGETVERSIONPROC pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(commCtrlDll, "DllGetVersion");
        if (pDllGetVersion) {
            DLLVERSIONINFO dvi = {
                .cbSize = sizeof(DLLVERSIONINFO),
            };
            (*pDllGetVersion)(&dvi);
            isCommCtrlVersion6 = (dvi.dwMajorVersion == 6);
        }
        FreeLibrary(commCtrlDll);
    }

    return isCommCtrlVersion6;
}
