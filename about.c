#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include <shellapi.h>

#pragma comment(lib, "Shell32.lib")

#include "main.h"

#include "about.h"

LRESULT CALLBACK AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        if (((NMHDR *)lParam)->code == NM_CLICK)
            ShellExecute(NULL, L"open", ((PNMLINK)lParam)->item.szUrl, NULL, NULL, SW_SHOW);
    }

    return FALSE;
}
