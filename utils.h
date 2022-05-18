#pragma once

#ifndef NELEMS
#define NELEMS(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef ASSERT
#define ASSERT(a, ret)  if(!(a)) return (ret)
#define ASSERT_VOID(a) if(!(a)) return
#define ASSERT_NULL(a) ASSERT(a, NULL)
#define ASSERT_FALSE(a) ASSERT(a, FALSE)
#define ASSERT_GOTO(a, label) if(!(a)) goto label
#define ASSERT_END(a) ASSERT_GOTO(a, end)
#define ASSERT_FAILED(a) ASSERT_GOTO(a, failed)
#define ASSERT_CONTINUE(a) if(!(a)) continue
#define ASSERT_BREAK(a) if(!(a)) break
#endif

void CatFilePath(LPTSTR szBuf, int size, LPCTSTR szParent, LPCTSTR szPath);
void GetFileExt(LPTSTR szExt, int size, LPCTSTR szPath);
BOOL FileTimeToLocalSystemTime(CONST FILETIME *pFt, PSYSTEMTIME pSt);
BOOL LocalSystemTimeToFileTime(CONST SYSTEMTIME *pSt, PFILETIME pFt);
