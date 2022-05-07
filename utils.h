#pragma once

#ifndef NELEMS
#define NELEMS(a) (sizeof(a) / sizeof((a)[0]))
#endif

void CatFilePath(LPTSTR szBuf, int size, LPCTSTR szParent, LPCTSTR szPath);
void GetFileExt(LPTSTR szExt, int size, LPCTSTR szPath);
BOOL FileTimeToLocalSystemTime(CONST FILETIME *pFt, PSYSTEMTIME pSt);
BOOL LocalSystemTimeToFileTime(CONST SYSTEMTIME *pSt, PFILETIME pFt);
