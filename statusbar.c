#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>
#include <commctrl.h>

#include "statusbar.h"
#include "utils.h"

#include "main.h"

static LONG scxSysLink = 248;

/**
 * @brief Size the panels in the status bar
 *
 * This function adjusts the sizes of the panels in the status bar based on the parent window size
 * and the presence of a system link window.
 *
 * @param hWndParent Handle to the parent window
 * @param hWndStatusBar Handle to the status bar window
 */
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

/**
 * @brief Create a status bar window
 *
 * This function creates a status bar control as a child window with optional text and URL link.
 *
 * @param hWndParent Handle to the parent window
 * @param hInst Instance handle of the application
 * @param szText Initial text to display in the status bar
 * @param szUrl URL to associate with the status bar (for clickable links)
 * @return Handle to the created status bar window
 */
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
    if (GetTextExtentPoint32(GetDC(hWndSysLink), szUrl, (int)wcslen(szUrl), &size))
        scxSysLink = size.cx + 18;
    SizeStatusPanels(hWndParent, hWndStatusBar);
    return hWndStatusBar;
}

/**
 * @brief Set text in a status bar panel
 *
 * This function sets the text in a specific panel of the status bar.
 *
 * @param hwndStatusBar Handle to the status bar window
 * @param id ID of the panel to update
 * @param szStatusString Text to display in the panel
 */
void SetStatusBarText(HWND hwndStatusBar, int id, TCHAR *szStatusString)
{
    SendMessage(hwndStatusBar, SB_SETTEXT, id, (LPARAM)szStatusString);
}
