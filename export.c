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

/**
 * @brief Save data to a file
 *
 * This function writes the specified amount of data to the given file handle.
 *
 * @param hFile Handle to the file to write to
 * @param pData Pointer to the data to write
 * @param dwSize Number of bytes to write
 * @return TRUE if the write operation was successful, FALSE otherwise
 */
static inline BOOL SaveData(HANDLE hFile, PVOID pData, DWORD dwSize)
{
    DWORD dwBytesWritten = 0;
    return WriteFile(hFile, pData, dwSize, &dwBytesWritten, NULL);
}

/**
 * @brief Save a string to a file in UTF-8 encoding
 *
 * This function converts a wide character string to UTF-8 and writes it to the specified file.
 *
 * @param hFile Handle to the file to write to
 * @param szBuf Pointer to the wide character string to save
 * @return TRUE if the operation was successful, FALSE otherwise
 */
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

/**
 * @brief Export list view content to TSV file
 *
 * This function exports the content of a list view control to a tab-separated values file.
 *
 * @param hWndLV Handle to the list view control
 * @param szPath Path to the output TSV file
 * @return TRUE if the export was successful, FALSE otherwise
 */
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

/**
 * @brief Export list view content to HTML file
 *
 * This function exports the content of a list view control to an HTML file with basic styling.
 *
 * @param hWndLV Handle to the list view control
 * @param szPath Path to the output HTML file
 * @param szTitle Title for the HTML document and heading
 * @return TRUE if the export was successful, FALSE otherwise
 */
BOOL ExportToHtmlFile(HWND hWndLV, PCTSTR szPath, PCTSTR szTitle)
{
    HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    ASSERT_FALSE(hFile != INVALID_HANDLE_VALUE);
    BOOL bRet = FALSE;
    const char *head = "<!DOCTYPE html>\n<html><head><meta charset=\"utf-8\">\n<style>\n"
                        "table{border-collapse:collapse}table,th,td{border:1px solid;padding:0.2em 0.5em}"
                        "tr:nth-child(even){background:#eee}\n</style>\n<title>";
    const char *body = "</title>\n</head><body>\n<h1>";
    const char *table = "</h1>\n<table><thead><tr>\n";
    const char *tbody = "</tr></thead><tbody>\n";
    const char *end = "</tbody></table></body></html>\n";
    ASSERT_END(SaveData(hFile, (void *)head, (DWORD)strlen(head)));
    ASSERT_END(SaveString(hFile, szTitle));
    ASSERT_END(SaveData(hFile, (void *)body, (DWORD)strlen(body)));
    ASSERT_END(SaveString(hFile, szTitle));
    ASSERT_END(SaveData(hFile, (void *)table, (DWORD)strlen(table)));
    TCHAR szBuf[MAX_PATH] = L"";
    for (int i = 0; i < LV_ROWS; i++) {
        ListView_GetColumnText(hWndLV, i, szBuf, NELEMS(szBuf));
        ASSERT_END(SaveData(hFile, "<th>", 4));
        ASSERT_END(SaveString(hFile, szBuf));
        ASSERT_END(SaveData(hFile, "</th>\n", 6));
    }
    ASSERT_END(SaveData(hFile, (void *)tbody, (DWORD)strlen(tbody)));
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
    ASSERT_END(SaveData(hFile, (void *)end, (DWORD)strlen(end)));
    bRet = TRUE;
end:
    CloseHandle(hFile);
    return bRet;
}
