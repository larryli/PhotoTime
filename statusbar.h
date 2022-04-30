#pragma once

HWND CreateStatusBarWnd(HWND hWndParent, HINSTANCE hInst, TCHAR *szText, TCHAR *szUrl);
void SizeStatusPanels(HWND hWndParent, HWND hWndStatusBar);
void SetStatusBarText(HWND hwndStatusBar, int id, TCHAR *szStatusString);

inline HWND GetStatusBarSysLinkWnd(HWND hWndStatusBar)
{
    return (HWND)GetWindowLongPtr(hWndStatusBar, GWLP_USERDATA);
}
