#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>
#include <commctrl.h>

#include "main.h"
#include "statusbar.h"
#include "utils.h"

static LONG scxSysLink = 248;

void SizeStatusPanels(HWND hWndParent, HWND hWndStatusBar)
{
    RECT rect;
    GetClientRect(hWndParent, &rect);
    int cxSysLink = 0;
    HWND hWndSysLink = GetStatusBarSysLinkWnd(hWndStatusBar);
    if (hWndSysLink)
        cxSysLink = scxSysLink;
    int partsize = (rect.right - rect.left - cxSysLink) / 2;
    if (partsize < cxSysLink)
        partsize = (rect.right - rect.left) / 3;
    int ptArray[] = {partsize,  partsize * 2, -1};
    SendMessage(hWndStatusBar, SB_SETPARTS, NELEMS(ptArray), (LPARAM)(LPINT)ptArray);
    if (hWndSysLink)
        MoveWindow(hWndSysLink, ptArray[1] + 4, 2, cxSysLink - 16, 16, TRUE);
}

HWND CreateStatusBarWnd(HWND hWndParent, HINSTANCE hInst, TCHAR *szText, TCHAR *szUrl)
{
    HWND hWndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_BORDER | SBARS_SIZEGRIP,
                                            szText,
                                            hWndParent,
                                            ID_STATUSBAR);
    ASSERT_NULL(hWndStatusBar);
    TCHAR szBuf[MAX_PATH];
    swprintf(szBuf, NELEMS(szBuf), L"<a href=\"%ls\">%ls</a>", szUrl, szUrl);
    HWND hWndSysLink = CreateWindowEx(0, WC_LINK, szBuf, WS_VISIBLE | WS_CHILD,
                                      0, 0, 0, 0, hWndStatusBar, NULL, hInst, NULL);
    if (hWndSysLink)
        SetWindowLongPtr(hWndStatusBar, GWLP_USERDATA, (LONG_PTR)hWndSysLink);
    SIZE size;
    if (GetTextExtentPoint32(GetDC(hWndSysLink), szUrl, wcslen(szUrl), &size))
        scxSysLink = size.cx + 18;
    SizeStatusPanels(hWndParent, hWndStatusBar);
    return hWndStatusBar;
}

void SetStatusBarText(HWND hwndStatusBar, int id, TCHAR *szStatusString)
{
    SendMessage(hwndStatusBar, SB_SETTEXT, id, (LPARAM)szStatusString);
}
