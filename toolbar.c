#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include "toolbar.h"
#include "utils.h"

#include "main.h"

HWND CreateToolBarWnd(HWND hWndParent, HINSTANCE hInst)
{
    HWND hWndTB = CreateWindowEx(WS_EX_CLIENTEDGE,
                                 TOOLBARCLASSNAME,
                                 (LPTSTR) NULL,
                                 WS_CHILD | WS_BORDER | CCS_TOP | TBSTYLE_TOOLTIPS,
                                 0, 0, 0, 0,
                                 hWndParent,
                                 (HMENU) ID_TOOLBAR,
                                 hInst,
                                 NULL);
    ASSERT_NULL(hWndTB);
    SendMessage(hWndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
    TBADDBITMAP tbab = {
        .hInst = 0,
        .nID = (UINT_PTR)LoadImage(hInst, MAKEINTRESOURCE(IDR_BMP_TOOLBAR),IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS),
    };
    SendMessage(hWndTB, TB_ADDBITMAP, (WPARAM)1, (LPARAM)&tbab);
    TBBUTTON tbb[] = {
        {.fsStyle = TBSTYLE_SEP},
        {.iBitmap = 0, .idCommand = IDM_OPEN, .fsState = TBSTATE_ENABLED, .fsStyle = TBSTYLE_BUTTON},
        {.iBitmap = 2, .idCommand = IDM_RELOAD, .fsStyle = TBSTYLE_BUTTON},
        {.fsStyle = TBSTYLE_SEP},
        {.iBitmap = 3, .idCommand = IDM_AUTOPROC, .fsStyle = TBSTYLE_BUTTON},
        {.fsStyle = TBSTYLE_SEP},
        {.iBitmap = 1, .idCommand = IDM_EXIT, .fsState = TBSTATE_ENABLED, .fsStyle = TBSTYLE_BUTTON},
    };
    SendMessage(hWndTB, TB_ADDBUTTONS, NELEMS(tbb), (LPARAM)tbb);
    SetWindowLong(hWndTB, GWL_STYLE, GetWindowLong(hWndTB, GWL_STYLE) | TBSTYLE_FLAT);
    ShowWindow(hWndTB, SW_SHOW);
    return hWndTB;
}

void ToolBarNeedText(HWND hWndParent, LPTOOLTIPTEXT lpttt)
{
    switch (lpttt->hdr.idFrom) {
    case IDM_OPEN:
        lpttt->lpszText = (LPTSTR)IDS_OPEN;
        break;
    case IDM_EXIT:
        lpttt->lpszText = (LPTSTR)IDS_EXIT;
        break;
    case IDM_RELOAD:
        lpttt->lpszText = (LPTSTR)IDS_RELOAD;
        break;
    case IDM_AUTOPROC:
        lpttt->lpszText = (LPTSTR)IDS_AUTOPROC;
        break;
    }
}
