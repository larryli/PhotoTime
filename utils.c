#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define __STDC_WANT_LIB_EXT1__ 1
#include <tchar.h>

#include "utils.h"

void CatFilePath(PTSTR szBuf, int size, PCTSTR szParent, PCTSTR szPath)
{
    if (!szBuf || size <= 0)
        return;
    if (szBuf != szParent && szParent)
        (void)_tcscpy_s(szBuf, size, szParent);
    if (szPath) {
        (void)_tcscat_s(szBuf, size, TEXT("\\"));
        (void)_tcscat_s(szBuf, size, szPath);
    }
}

PCTSTR GetFileExt(PCTSTR szPath)
{
    TCHAR *p;
    if ((p = _tcsrchr(szPath, TEXT('.'))) != NULL)
        return p + 1;
    else
        return NULL;
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
