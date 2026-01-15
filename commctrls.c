#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>

#include "commctrls.h"

/**
 * @brief Initialize common controls
 *
 * This function initializes the common controls library by calling InitCommonControlsEx
 * with the appropriate initialization structure.
 */
void InitCommCtrl(void)
{
    INITCOMMONCONTROLSEX icc = {
        .dwSize = sizeof(INITCOMMONCONTROLSEX),
        .dwICC = ICC_WIN95_CLASSES,
    };
    InitCommonControlsEx(&icc);
}

/**
 * @brief Check if common controls version 6 is available
 *
 * This function checks whether version 6 of the common controls library is available
 * by loading comctl32.dll and querying its version.
 *
 * @return TRUE if version 6 is available, FALSE otherwise
 */
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
