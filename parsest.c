#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>
#include <wctype.h>

#include "parsest.h"

typedef enum {
    STATE_YEAR,
    STATE_MON,
    STATE_DAY,
    STATE_HOUR,
    STATE_MIN,
    STATE_SEC,
} STATE;

#define is_sep(c) ((c) == L' ' || (c) == L'.' || (c) == L'_' || (c) == L'-')

static int ToWord(LPCTSTR szStr, int len, STATE state)
{
    TCHAR szBuf[5];
    if (len > 4)
        return -1;
    lstrcpyn(szBuf, szStr, len + 1);
    WORD d = (WORD)_wtoi(szBuf);
    switch (state) {
    case STATE_YEAR:
        if (d >= 1900 && d <= 2100) // @todo: fixme
            return d;
        break;
    case STATE_MON:
        if (d >= 1 && d <= 12)
            return d;
        break;
    case STATE_DAY:
        if (d >= 1 && d <= 31)
            return d;
        break;
    case STATE_HOUR:
        if (d >= 0 && d <= 23)
            return d;
        break;
    case STATE_MIN:
    case STATE_SEC:
        if (d >= 0 && d <= 59)
            return d;
        break;
    }
    return -1;
}

static BOOL ToSystemTime(LPCTSTR szStr, int len, PSYSTEMTIME pSt)
{
    TCHAR szBuf[11];
    if (len == 8 || len == 12)
        len = 9;
    else if (len == 10 || len == 13)
        len = 11;
    else
        return FALSE;
    lstrcpyn(szBuf, szStr, len);
    int t = _wtoi(szBuf);
    LONGLONG ll;
    ll = Int32x32To64(t, 10000000) + 116444736000000000;
    FILETIME ftLocal, ft = {
        .dwLowDateTime = (DWORD)ll,
        .dwHighDateTime = ll >> 32,
    };
    FileTimeToLocalFileTime(&ft, &ftLocal);
    return FileTimeToSystemTime(&ftLocal, pSt);
}

BOOL ParseStringToSystemTime(LPCTSTR szStr, PSYSTEMTIME pSt)
{
#if 1
    pSt->wHour = (WORD)-1; // no set
    pSt->wMinute = (WORD)-1; // no set
    pSt->wSecond = (WORD)-1; // no set
    pSt->wDayOfWeek = 0;
    pSt->wMilliseconds = 0;
#else
    ZeroMemory(pSt, sizeof(SYSTEMTIME));
#endif

    LPCTSTR s = szStr;
    for (; *s; s++)
        if (iswdigit(*s))
            break;
    if (!(*s))
        return FALSE;
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
                pSt->wYear = ToWord(szS, len, STATE_YEAR);
                if (pSt->wYear == (WORD)-1)
                    return FALSE;
                state = STATE_MON;
                szS = s + 1;
                len = 0;
                cLast = *s;
            } else if (len == 8) { // 19920304
                pSt->wYear = ToWord(szS, 4, STATE_YEAR);
                pSt->wMonth = ToWord(szS + 4, 2, STATE_MON);
                pSt->wDay = ToWord(szS + 6, 2, STATE_DAY);
                if (pSt->wYear == (WORD)-1 || pSt->wMonth == (WORD)-1 || pSt->wDay == (WORD)-1)
                    return FALSE;
                if (is_sep(*s)) {
                    state = STATE_HOUR;
                    szS = s + 1;
                    len = 0;
                } else
                    return TRUE;
            } else if (len == 10) // 1000000000 --> 20010909094640
                return ToSystemTime(szS, len, pSt);
            else if (len == 12) { // 19920304050607
                pSt->wYear = ToWord(szS, 4, STATE_YEAR);
                pSt->wMonth = ToWord(szS + 4, 2, STATE_MON);
                pSt->wDay = ToWord(szS + 6, 2, STATE_DAY);
                pSt->wHour = ToWord(szS + 8, 2, STATE_HOUR);
                pSt->wMinute = ToWord(szS + 10, 2, STATE_MIN);
                if (pSt->wYear == (WORD)-1 || pSt->wMonth == (WORD)-1 || pSt->wDay == (WORD)-1 ||
                    pSt->wHour == (WORD)-1 || pSt->wMinute == (WORD)-1)
                    return FALSE;
                return TRUE;
            } else if (len == 13) //  1000000000000 --> 20010909094640
                return ToSystemTime(szS, len, pSt);
            else if (len == 14) { // 19920304050607
                pSt->wYear = ToWord(szS, 4, STATE_YEAR);
                pSt->wMonth = ToWord(szS + 4, 2, STATE_MON);
                pSt->wDay = ToWord(szS + 6, 2, STATE_DAY);
                pSt->wHour = ToWord(szS + 8, 2, STATE_HOUR);
                pSt->wMinute = ToWord(szS + 10, 2, STATE_MIN);
                pSt->wSecond = ToWord(szS + 12, 2, STATE_SEC);
                if (pSt->wYear == (WORD)-1 || pSt->wMonth == (WORD)-1 || pSt->wDay == (WORD)-1 ||
                    pSt->wHour == (WORD)-1 || pSt->wMinute == (WORD)-1 || pSt->wSecond == (WORD)-1)
                    return FALSE;
                return TRUE;
            } else
                return FALSE;
            break;
        case STATE_MON:
            if (is_sep(*s)) {
                if (*s != cLast)
                    return FALSE;
                else if (len <= 2) { // 03
                    pSt->wMonth = ToWord(szS, len, STATE_MON);
                    if (pSt->wMonth == (WORD)-1)
                        return FALSE;
                    state = STATE_DAY;
                    szS = s + 1;
                    len = 0;
                }
            } else
                return FALSE;
            break;
        case STATE_DAY:
            if (len <= 2) { // 04
                pSt->wDay = ToWord(szS, len, STATE_DAY);
                if (pSt->wDay == (WORD)-1)
                    return FALSE;
                if (is_sep(*s)) {
                    state = STATE_HOUR;
                    szS = s + 1;
                    len = 0;
                } else
                    return TRUE;
            }
            break;
        case STATE_HOUR:
            if (len <= 2 && is_sep(*s)) { // 05
                pSt->wHour = ToWord(szS, len, STATE_HOUR);
                if (pSt->wHour == (WORD)-1)
                    return FALSE;
                state = STATE_MIN;
                szS = s + 1;
                len = 0;
                cLast = *s;
            } else if (len == 4) { // 0506
                pSt->wHour = ToWord(szS, 2, STATE_HOUR);
                pSt->wMinute = ToWord(szS + 2, 2, STATE_MIN);
                if (pSt->wHour == (WORD)-1 || pSt->wMinute == (WORD)-1)
                    return FALSE;
                return TRUE;
            } else if (len == 6) { // 050607
                pSt->wHour = ToWord(szS, 2, STATE_HOUR);
                pSt->wMinute = ToWord(szS + 2, 2, STATE_MIN);
                pSt->wSecond = ToWord(szS + 4, 2, STATE_SEC);
                if (pSt->wHour == (WORD)-1 || pSt->wMinute == (WORD)-1 || pSt->wSecond == (WORD)-1)
                    return FALSE;
                return TRUE;
            } else
                return TRUE;
            break;
        case STATE_MIN:
            if (len <= 2) { // 06
                pSt->wMinute = ToWord(szS, len, STATE_MIN);
                if (pSt->wMinute == (WORD)-1)
                    return FALSE;
                if (is_sep(*s) && *s == cLast) {
                    state = STATE_SEC;
                    szS = s + 1;
                    len = 0;
                } else
                    return TRUE;
            } else
                return FALSE;
            break;
        case STATE_SEC:
            if (len <= 2) { // 07
                pSt->wSecond = ToWord(szS, len, STATE_SEC);
                if (pSt->wSecond == (WORD)-1)
                    return FALSE;
                return TRUE;
            }
            return FALSE;
        }
    }
    return FALSE;
}
