#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include "main.h"

void SizeStatusPanels(HWND hWndParent, HWND hWndStatusbar)
{
    int  ptArray[4];
    RECT rect;
    int  partsize;

    GetClientRect(hWndParent, &rect);
    partsize = (rect.right / 4);
    ptArray[0] = partsize * 2;
    for (int i = 1; i < 4; i++) {
        ptArray[i] = ptArray[0] + (partsize * i);
    }
    SendMessage(hWndStatusbar, SB_SETPARTS, 3, (LPARAM)(LPINT)ptArray);
}

HWND CreateStatusBarWnd(HWND hWndParent, TCHAR *initialText)
{
    HWND hWndStatusbar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_BORDER | SBARS_SIZEGRIP,
                                            initialText,
                                            hWndParent,
                                            ID_STATUSBAR);
    if (hWndStatusbar) {
        SizeStatusPanels(hWndParent, hWndStatusbar);
        return hWndStatusbar;
    }
    return 0;
}

void SetStatusBarText(HWND hwndStatusBar, int id, TCHAR *szStatusString)
{
    SendMessage(hwndStatusBar, SB_SETTEXT, id, (LPARAM)szStatusString);
}
