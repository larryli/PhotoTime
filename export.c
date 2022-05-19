#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>
#include <tchar.h>
#include <commctrl.h>
#include <commdlg.h>

#include "export.h"
#include "listview.h"
#include "utils.h"

#include "main.h"

static inline BOOL SaveData(HANDLE hFile, PVOID pData, DWORD dwSize)
{
    DWORD dwBytesWritten = 0;
    return WriteFile(hFile, pData, dwSize, &dwBytesWritten, NULL);
}

static BOOL SaveString(HANDLE hFile, PCTSTR szBuf)
{
    int size = WideCharToMultiByte(CP_UTF8, 0, szBuf, -1, NULL, 0, NULL, NULL);
    PVOID p = GlobalAlloc(GMEM_FIXED, size);
    ASSERT_FALSE(p);
    WideCharToMultiByte(CP_UTF8, 0, szBuf, -1, p, size, NULL, NULL);
    BOOL bRet = SaveData(hFile, p, size - 1); // fix '\0'
    GlobalFree(p);
    return bRet;
}

BOOL ExportToTsvFile(HWND hWndLV, PCTSTR szPath)
{
    HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    ASSERT_FALSE(hFile != INVALID_HANDLE_VALUE);
    BOOL bRet = FALSE;
    TCHAR szBuf[MAX_PATH] = L"";
    for (int i = 0; i < LV_ROWS; i++) {
        ListView_GetColumnText(hWndLV, i, szBuf, NELEMS(szBuf));
        if (i)
            ASSERT_END(SaveData(hFile, "\t", 1));
        ASSERT_END(SaveString(hFile, szBuf));
    }
    ASSERT_END(SaveData(hFile, "\r\n", 2));
    int count = ListView_GetItemCount(hWndLV);
    for (int idx = 0; idx < count; idx++) {
        for (int i = 0; i < LV_ROWS; i++) {
            if (i)
                ASSERT_END(SaveData(hFile, "\t", 1));
            ListView_GetItemText(hWndLV, idx, i, szBuf, NELEMS(szBuf));
            ASSERT_END(SaveString(hFile, szBuf));
        }
        ASSERT_END(SaveData(hFile, "\r\n", 2));
    }
    bRet = TRUE;
end:
    CloseHandle(hFile);
    return bRet;
}

BOOL ExportToHtmlFile(HWND hWndLV, PCTSTR szPath, PCTSTR szTitle)
{
    HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    ASSERT_FALSE(hFile != INVALID_HANDLE_VALUE);
    BOOL bRet = FALSE;
    const char *head = "<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\">\n<style>\n"
                        "table{border-collapse:collapse}table,th,td{border:1px solid;padding:0.2em 0.5em}\n</style>\n<title>";
    const char *body = "</title>\n</head><body>\n<h1>";
    const char *table = "</h1>\n<table><thead><tr>\n";
    const char *tbody = "</tr></thead><tbody>\n";
    const char *end = "</tbody></table></body></html>\n";
    ASSERT_END(SaveData(hFile, (void *)head, strlen(head)));
    ASSERT_END(SaveString(hFile, szTitle));
    ASSERT_END(SaveData(hFile, (void *)body, strlen(body)));
    ASSERT_END(SaveString(hFile, szTitle));
    ASSERT_END(SaveData(hFile, (void *)table, strlen(table)));
    TCHAR szBuf[MAX_PATH] = L"";
    for (int i = 0; i < LV_ROWS; i++) {
        ListView_GetColumnText(hWndLV, i, szBuf, NELEMS(szBuf));
        ASSERT_END(SaveData(hFile, "<th>", 4));
        ASSERT_END(SaveString(hFile, szBuf));
        ASSERT_END(SaveData(hFile, "</th>\n", 6));
    }
    ASSERT_END(SaveData(hFile, (void *)tbody, strlen(tbody)));
    int count = ListView_GetItemCount(hWndLV);
    for (int idx = 0; idx < count; idx++) {
        ASSERT_END(SaveData(hFile, "<tr>\n", 5));
        for (int i = 0; i < LV_ROWS; i++) {
            ListView_GetItemText(hWndLV, idx, i, szBuf, NELEMS(szBuf));
            ASSERT_END(SaveData(hFile, "<td>", 4));
            ASSERT_END(SaveString(hFile, szBuf));
            ASSERT_END(SaveData(hFile, "</td>\n", 6));
        }
        ASSERT_END(SaveData(hFile, "</tr>\n", 6));
    }
    ASSERT_END(SaveData(hFile, (void *)end, strlen(end)));
    bRet = TRUE;
end:
    CloseHandle(hFile);
    return bRet;
}
