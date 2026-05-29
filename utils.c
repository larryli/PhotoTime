#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define __STDC_WANT_LIB_EXT1__ 1
#include <tchar.h>

#include "utils.h"

/**
 * @brief Concatenate file path components
 *
 * This function concatenates a parent directory path with a file/subdirectory path to form a complete path.
 *
 * @param szBuf Buffer to store the concatenated path
 * @param size Size of the buffer in characters
 * @param szParent Parent directory path
 * @param szPath File or subdirectory path to append to the parent
 */
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

/**
 * @brief Get file extension from path
 *
 * This function extracts the file extension from a file path.
 *
 * @param szPath Path to the file
 * @return Pointer to the extension part of the path or NULL if no extension exists
 */
PCTSTR GetFileExt(PCTSTR szPath)
{
    TCHAR *p;
    if ((p = _tcsrchr(szPath, TEXT('.'))) != NULL)
        return p + 1;
    else
        return NULL;
}

/**
 * @brief Convert FILETIME to local SYSTEMTIME
 *
 * This function converts a FILETIME structure to a local SYSTEMTIME structure, adjusting for the local timezone.
 * The conversion involves two steps: first converting from UTC FILETIME to local FILETIME, then converting
 * the local FILETIME to SYSTEMTIME format.
 *
 * @param pFt Pointer to FILETIME structure to convert (in UTC)
 * @param pSt Pointer to SYSTEMTIME structure to store the converted time (in local timezone)
 * @return TRUE if successful, FALSE otherwise
 */
BOOL FileTimeToLocalSystemTime(CONST FILETIME *pFt, PSYSTEMTIME pSt)
{
    FILETIME local;
    ASSERT_FALSE(FileTimeToLocalFileTime(pFt, &local));
    return FileTimeToSystemTime(&local, pSt);
}

/**
 * @brief Convert local SYSTEMTIME to FILETIME
 *
 * This function converts a local SYSTEMTIME structure to a FILETIME structure, adjusting for the local timezone.
 *
 * @param pSt Pointer to SYSTEMTIME structure to convert
 * @param pFt Pointer to FILETIME structure to store the converted time
 * @return TRUE if successful, FALSE otherwise
 */
BOOL LocalSystemTimeToFileTime(CONST SYSTEMTIME *pSt, PFILETIME pFt)
{
    FILETIME local;
    ASSERT_FALSE(SystemTimeToFileTime(pSt, &local));
    return LocalFileTimeToFileTime(&local, pFt);
}
