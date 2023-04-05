#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <wchar.h>

#include <commctrl.h>
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")

#include <winver.h>
#pragma comment(lib, "Version.lib")

#include "about.h"
#include "utils.h"

#include "main.h"

static void InitDialog(HWND hwnd)
{
    TCHAR szPath[MAX_PATH];
    GetModuleFileName((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), szPath, NELEMS(szPath));
    DWORD dwInfo;
    UINT size = GetFileVersionInfoSize(szPath, &dwInfo);
    void *pInfo = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, size);
    ASSERT_VOID(pInfo);
    ASSERT_END(GetFileVersionInfo(szPath, dwInfo, size, pInfo));
    void far *ptr;
    ASSERT_END(VerQueryValue(pInfo, L"\\VarFileInfo\\Translation", &ptr, &size));
    WORD far *pLang = (WORD far *)ptr;
    TCHAR szBuf[MAX_PATH];
    swprintf(szBuf, NELEMS(szBuf), L"\\StringFileInfo\\%04X%04X\\%ls", *pLang, *(pLang + 1), L"ProductName");
    if (VerQueryValue(pInfo, szBuf, &ptr, &size)) {
        void far *ptr2;
        swprintf(szBuf, NELEMS(szBuf), L"\\StringFileInfo\\%04X%04X\\%ls", *pLang, *(pLang + 1), L"ProductVersion");
        if (VerQueryValue(pInfo, szBuf, &ptr2, &size)) {
            swprintf(szBuf, NELEMS(szBuf), L"%ls v%ls", (LPTSTR)ptr, (LPTSTR)ptr2);
            Static_SetText(GetDlgItem(hwnd, IDD_APPNAME), szBuf);
        }
    }
    swprintf(szBuf, NELEMS(szBuf), L"\\StringFileInfo\\%04X%04X\\%ls", *pLang, *(pLang + 1), L"LegalCopyright");
    if (VerQueryValue(pInfo, szBuf, &ptr, &size)) {
        swprintf(szBuf, NELEMS(szBuf), L"Copyright (C) %ls", (LPTSTR)ptr);
        Static_SetText(GetDlgItem(hwnd, IDD_COPYRIGHT), szBuf);
    }
    swprintf(szBuf, NELEMS(szBuf), L"\\StringFileInfo\\%04X%04X\\%ls", *pLang, *(pLang + 1), L"Comments");
    if (VerQueryValue(pInfo, szBuf, &ptr, &size)) {
        swprintf(szBuf, NELEMS(szBuf), L"<a href=\"%ls\">%ls</a>", (LPTSTR)ptr, (LPTSTR)ptr);
        Static_SetText(GetDlgItem(hwnd, IDD_WEBSITE), szBuf);
    }
end:
    GlobalFree(pInfo);
}

LRESULT CALLBACK AboutDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        InitDialog(hDlg);
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
