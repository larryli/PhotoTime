#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include "main.h"
#include "statusbar.h"

#define CX_SYSLINK 248

#define NELEMS(a) (sizeof(a) / sizeof((a)[0]))

void SizeStatusPanels(HWND hWndParent, HWND hWndStatusBar)
{
    RECT rect;
    GetClientRect(hWndParent, &rect);
    int cxSysLink = 0;
    HWND hWndSysLink = GetStatusBarSysLinkWnd(hWndStatusBar);
    if (hWndSysLink != 0) {
        cxSysLink = CX_SYSLINK;
    }
    int partsize = (rect.right - rect.left - cxSysLink) / 2;
    if (partsize < cxSysLink) {
        partsize = (rect.right - rect.left) / 3;
    }
    int ptArray[] = {partsize,  partsize * 2, -1};
    SendMessage(hWndStatusBar, SB_SETPARTS, NELEMS(ptArray), (LPARAM)(LPINT)ptArray);
    if (hWndSysLink != 0) {
        MoveWindow(hWndSysLink, ptArray[1] + 2, 2, cxSysLink - 14, 16, TRUE);
    }
}

HWND CreateStatusBarWnd(HWND hWndParent, HINSTANCE hInst, TCHAR *szText, TCHAR *szUrl)
{
    HWND hWndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_BORDER | SBARS_SIZEGRIP,
                                            szText,
                                            hWndParent,
                                            ID_STATUSBAR);
    if (!hWndStatusBar)
        return 0;
    HWND hWndSysLink = CreateWindowEx(0, WC_LINK, szUrl, WS_VISIBLE | WS_CHILD,
                                      0, 0, 0, 0, hWndStatusBar, NULL, hInst, NULL);
    if (hWndSysLink) {
        SetWindowLongPtr(hWndStatusBar, GWLP_USERDATA, (LONG_PTR)hWndSysLink);
    }
    SizeStatusPanels(hWndParent, hWndStatusBar);
    return hWndStatusBar;
}

void SetStatusBarText(HWND hwndStatusBar, int id, TCHAR *szStatusString)
{
    SendMessage(hwndStatusBar, SB_SETTEXT, id, (LPARAM)szStatusString);
}
