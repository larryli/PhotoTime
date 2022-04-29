#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include "toolbar.h"

#include "main.h"

#define NELEMS(a) (sizeof(a) / sizeof((a)[0]))

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
    if (!hWndTB)
        return NULL;
    SendMessage(hWndTB, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
    TBADDBITMAP tbab = {
        .hInst = 0,
        .nID = (UINT_PTR)LoadImage(hInst, MAKEINTRESOURCE(IDR_BMP_TOOLBAR),IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS),
    };
    SendMessage(hWndTB, TB_ADDBITMAP, (WPARAM)1, (LPARAM)&tbab);
    TBBUTTON tbb[] = {
        {.fsStyle = TBSTYLE_SEP},
        {.iBitmap = 0, .idCommand = IDM_OPEN, .fsState = TBSTATE_ENABLED, .fsStyle = TBSTYLE_BUTTON},
        {.iBitmap = 1, .idCommand = IDM_EXIT, .fsState = TBSTATE_ENABLED, .fsStyle = TBSTYLE_BUTTON},
        // {.fsStyle = TBSTYLE_SEP},
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
    }
}
