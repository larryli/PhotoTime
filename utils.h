#pragma once

#ifndef NELEMS
#define NELEMS(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifdef ASSERT
#pragma message("ERROR! Failed to define marco ASSERT!!")
#else
#define ASSERT(a, s) if (!(a)) s
#define ASSERT_BREAK(a) ASSERT(a, break)
#define ASSERT_CONTINUE(a) ASSERT(a, continue)
#define ASSERT_VOID(a) ASSERT(a, return)
#define ASSERT_RETURN(a, r) ASSERT(a, return (r))
#define ASSERT_NULL(a) ASSERT_RETURN(a, NULL)
#define ASSERT_FALSE(a) ASSERT_RETURN(a, FALSE)
#define ASSERT_GOTO(a, l) ASSERT(a, goto l)
#define ASSERT_END(a) ASSERT_GOTO(a, end)
#define ASSERT_FAILED(a) ASSERT_GOTO(a, failed)
#endif

void CatFilePath(PTSTR szBuf, int size, PCTSTR szParent, PCTSTR szPath);
PCTSTR GetFileExt(PCTSTR szPath);
BOOL FileTimeToLocalSystemTime(CONST FILETIME *pFt, PSYSTEMTIME pSt);
BOOL LocalSystemTimeToFileTime(CONST SYSTEMTIME *pSt, PFILETIME pFt);
