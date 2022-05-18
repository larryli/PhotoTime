#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

#include "utils.h"

void CatFilePath(LPTSTR szBuf, int size, LPCTSTR szParent, LPCTSTR szPath)
{
    if (!szBuf || size <= 0)
        return;
    if (szBuf != szParent && szParent)
        lstrcpyn(szBuf, szParent, size);
    if (szPath) {
        _tcsncat(szBuf, TEXT("\\"), size);
        _tcsncat(szBuf, szPath, size);
    }
}

void GetFileExt(LPTSTR szExt, int size, LPCTSTR szPath)
{
    TCHAR *p;
    if ((p = _tcsrchr(szPath, TEXT('.'))) != NULL)
        lstrcpyn(szExt, p + 1, size);
    else
        *szExt = TEXT('\0');
}

BOOL FileTimeToLocalSystemTime(CONST FILETIME *pFt, PSYSTEMTIME pSt)
{
    FILETIME local;
    ASSERT_FALSE(FileTimeToLocalFileTime(pFt, &local));
    return FileTimeToSystemTime(&local, pSt);
}

BOOL LocalSystemTimeToFileTime(CONST SYSTEMTIME *pSt, PFILETIME pFt)
{
    FILETIME local;
    ASSERT_FALSE(SystemTimeToFileTime(pSt, &local));
    return LocalFileTimeToFileTime(&local, pFt);
}
