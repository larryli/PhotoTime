#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>
#include <wctype.h>

#include "parsest.h"
#include "utils.h"

typedef enum {
    STATE_YEAR,
    STATE_MON,
    STATE_DAY,
    STATE_HOUR,
    STATE_MIN,
    STATE_SEC,
} STATE;

#define is_sep(c) ((c) == L' ' || (c) == L'.' || (c) == L'_' || (c) == L'-')

/**
 * @brief Convert a string segment to a WORD value based on its state (year, month, etc.)
 *
 * This function converts a substring to a WORD value and validates it according to the expected range
 * for the given state (year, month, day, hour, minute, or second).
 *
 * @param szStr Pointer to the string segment to convert
 * @param len Length of the string segment
 * @param state The expected time component (STATE_YEAR, STATE_MON, etc.)
 * @param pW Pointer to store the converted WORD value
 * @return TRUE if conversion and validation are successful, FALSE otherwise
 */
static BOOL ToWord(LPCTSTR szStr, int len, STATE state, PWORD pW)
{
    TCHAR szBuf[5];
    ASSERT_FALSE(len > 0 && len <= 4);
    lstrcpyn(szBuf, szStr, len + 1);
    WORD w = (WORD)_wtoi(szBuf);
    switch (state) {
    case STATE_YEAR:
        if (w >= 1900 && w <= 2100) // @todo: fixme
            goto ok;
        break;
    case STATE_MON:
        if (w >= 1 && w <= 12)
            goto ok;
        break;
    case STATE_DAY:
        if (w >= 1 && w <= 31)
            goto ok;
        break;
    case STATE_HOUR:
        if (w >= 0 && w <= 23)
            goto ok;
        break;
    case STATE_MIN:
    case STATE_SEC:
        if (w >= 0 && w <= 59)
            goto ok;
        break;
    }
    return FALSE;
ok:
    *pW = w;
    return TRUE;
}

/**
 * @brief Convert a timestamp string to a SYSTEMTIME structure
 *
 * This function converts a Unix timestamp string to a SYSTEMTIME structure by converting
 * it to a FILETIME first and then to local system time.
 *
 * @param szStr Pointer to the timestamp string to convert
 * @param pSt Pointer to SYSTEMTIME structure to store the converted time
 * @return TRUE if conversion is successful, FALSE otherwise
 */
static BOOL ToSystemTime(LPCTSTR szStr, PSYSTEMTIME pSt)
{
    TCHAR szBuf[11];
    lstrcpyn(szBuf, szStr, 11);
    int t = _wtoi(szBuf);
    LONGLONG ft = Int32x32To64(t, 10000000) + 116444736000000000;
    return FileTimeToLocalSystemTime((PFILETIME)&ft, pSt);
}

/**
 * @brief Validate if the date in the SYSTEMTIME structure is valid
 *
 * This function checks if the day value in the SYSTEMTIME structure is valid for the given month and year,
 * taking leap years into account.
 *
 * @param pSt Pointer to SYSTEMTIME structure containing the date to validate
 * @return TRUE if the date is valid, FALSE otherwise
 */
BOOL IsValidDate(PSYSTEMTIME pSt)
{
    WORD daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((!(pSt->wYear % 4) && pSt->wYear % 100) || !(pSt->wYear % 400))
        daysInMonth[1] = 29;
    return pSt->wDay <= daysInMonth[pSt->wMonth - 1];
}

/**
 * @brief Parse a string to SYSTEMTIME structure
 *
 * This function parses a string representation of a date/time and converts it to a SYSTEMTIME structure.
 * It supports various date/time formats and returns the parsing result code.
 *
 * @param szStr String containing date/time information
 * @param pSt Pointer to SYSTEMTIME structure to store the parsed time
 * @param result Pointer to PARSEST_RESULT to store the parsing result code
 * @return TRUE if parsing was successful, FALSE otherwise
 */
BOOL ParseStringToSystemTime(LPCTSTR szStr, PSYSTEMTIME pSt, PARSEST_RESULT *result)
{
    ZeroMemory(pSt, sizeof(SYSTEMTIME));

    LPCTSTR s = szStr;
    for (; *s; s++)
        if (iswdigit(*s))
            break;
    ASSERT_FALSE(*s);
    STATE state = STATE_YEAR;
    int len = 0;
    TCHAR cLast = L'\0';
    LPCTSTR szS = s;
    for (; *s; s++) {
        if (iswdigit(*s)) {
            len++;
            continue;
        }
        switch (state) {
        case STATE_YEAR:
            if (len == 4 && is_sep(*s)) { // 1992
                ASSERT_FALSE(ToWord(szS, len, STATE_YEAR, &pSt->wYear));
                cLast = *s;
                state = STATE_MON;
                szS = s + 1;
                len = 0;
            } else if (len == 8) { // 19920304
                ASSERT_FALSE(ToWord(szS, 4, STATE_YEAR, &pSt->wYear));
                ASSERT_FALSE(ToWord(szS + 4, 2, STATE_MON, &pSt->wMonth));
                ASSERT_FALSE(ToWord(szS + 6, 2, STATE_DAY, &pSt->wDay));
                ASSERT_FALSE(IsValidDate(pSt));
                if (!is_sep(*s)) {
                    *result = PARSEST_NO_TIME;
                    return TRUE;
                }
                state = STATE_HOUR;
                szS = s + 1;
                len = 0;
            } else if (len == 10 || len == 13) { // 1000000000(000) --> 20010909094640
                ASSERT_FALSE(ToSystemTime(szS, pSt));
                *result = PARSEST_OK;
                return TRUE;
            } else if (len == 12) { // 19920304050607
                ASSERT_FALSE(ToWord(szS, 4, STATE_YEAR, &pSt->wYear));
                ASSERT_FALSE(ToWord(szS + 4, 2, STATE_MON, &pSt->wMonth));
                ASSERT_FALSE(ToWord(szS + 6, 2, STATE_DAY, &pSt->wDay));
                ASSERT_FALSE(IsValidDate(pSt));
                ASSERT_FALSE(ToWord(szS + 8, 2, STATE_HOUR, &pSt->wHour));
                ASSERT_FALSE(ToWord(szS + 10, 2, STATE_MIN, &pSt->wMinute));
                *result = PARSEST_NO_SECOND;
                return TRUE;
            } else if (len == 14) { // 19920304050607
                ASSERT_FALSE(ToWord(szS, 4, STATE_YEAR, &pSt->wYear));
                ASSERT_FALSE(ToWord(szS + 4, 2, STATE_MON, &pSt->wMonth));
                ASSERT_FALSE(ToWord(szS + 6, 2, STATE_DAY, &pSt->wDay));
                ASSERT_FALSE(IsValidDate(pSt));
                ASSERT_FALSE(ToWord(szS + 8, 2, STATE_HOUR, &pSt->wHour));
                ASSERT_FALSE(ToWord(szS + 10, 2, STATE_MIN, &pSt->wMinute));
                ASSERT_FALSE(ToWord(szS + 12, 2, STATE_SEC, &pSt->wSecond));
                *result = PARSEST_OK;
                return TRUE;
            } else
                return FALSE;
            break;
        case STATE_MON:
            ASSERT_FALSE(is_sep(*s));
            ASSERT_FALSE(*s == cLast);
            ASSERT_FALSE(len <= 2);
            ASSERT_FALSE(ToWord(szS, len, STATE_MON, &pSt->wMonth));
            state = STATE_DAY;
            szS = s + 1;
            len = 0;
            break;
        case STATE_DAY:
            ASSERT_FALSE(len <= 2);
            ASSERT_FALSE(ToWord(szS, len, STATE_DAY, &pSt->wDay));
            ASSERT_FALSE(IsValidDate(pSt));
            if (!is_sep(*s)) {
                *result = PARSEST_NO_TIME;
                return TRUE;
            }
            state = STATE_HOUR;
            szS = s + 1;
            len = 0;
            break;
        case STATE_HOUR:
            if (len == 6) { // 010203
                if (!ToWord(szS, 2, STATE_HOUR, &pSt->wHour) ||
                    !ToWord(szS + 2, 2, STATE_MIN, &pSt->wMinute) ||
                    !ToWord(szS + 4, 2, STATE_SEC, &pSt->wSecond))
                    *result = PARSEST_NO_TIME;
                else
                    *result = PARSEST_OK;
                return TRUE;
            } else if (len == 4) { // 0102
                if (!ToWord(szS, 2, STATE_HOUR, &pSt->wHour) ||
                    !ToWord(szS + 2, 2, STATE_MIN, &pSt->wMinute))
                    *result = PARSEST_NO_TIME;
                else
                    *result = PARSEST_NO_SECOND;
                return TRUE;
            } else if (len > 2 || !is_sep(*s) ||
                       !ToWord(szS, len, STATE_HOUR, &pSt->wHour)) {
                *result = PARSEST_NO_TIME;
                return TRUE;
            }
            cLast = *s;
            state = STATE_MIN;
            szS = s + 1;
            len = 0;
            break;
        case STATE_MIN:
            if (len > 2 || !ToWord(szS, len, STATE_MIN, &pSt->wMinute)) {
                *result = PARSEST_NO_TIME;
                return TRUE;
            }
            if (!is_sep(*s) || *s != cLast) {
                *result = PARSEST_NO_SECOND;
                return TRUE;
            }
            state = STATE_SEC;
            szS = s + 1;
            len = 0;
            break;
        case STATE_SEC:
            if (len > 2 || !ToWord(szS, len, STATE_SEC, &pSt->wSecond))
                *result = PARSEST_NO_SECOND;
            else
                *result = PARSEST_OK;
            return TRUE;
        }
    }
    return FALSE;
}
