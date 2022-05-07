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

static BOOL ToWord(LPCTSTR szStr, int len, STATE state, PWORD pW)
{
    TCHAR szBuf[5];
    if (len <= 0 || len > 4)
        return FALSE;
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

static BOOL ToSystemTime(LPCTSTR szStr, PSYSTEMTIME pSt)
{
    TCHAR szBuf[11];
    lstrcpyn(szBuf, szStr, 11);
    int t = _wtoi(szBuf);
    LONGLONG ft = Int32x32To64(t, 10000000) + 116444736000000000;
    return FileTimeToLocalSystemTime((PFILETIME)&ft, pSt);
}

static BOOL IsValidDate(PSYSTEMTIME pSt)
{
  WORD daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if ((!(pSt->wYear % 4) && pSt->wYear % 100) || !(pSt->wYear % 400))
    daysInMonth[1] = 29;
  return pSt->wDay <= daysInMonth[pSt->wMonth - 1];
}

BOOL ParseStringToSystemTime(LPCTSTR szStr, PSYSTEMTIME pSt, PARSEST_RESULT *result)
{
    ZeroMemory(pSt, sizeof(SYSTEMTIME));

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
                if (!ToWord(szS, len, STATE_YEAR, &pSt->wYear))
                    return FALSE;
                cLast = *s;
                state = STATE_MON;
                szS = s + 1;
                len = 0;
            } else if (len == 8) { // 19920304
                if (!ToWord(szS, 4, STATE_YEAR, &pSt->wYear) ||
                    !ToWord(szS + 4, 2, STATE_MON, &pSt->wMonth) ||
                    !ToWord(szS + 6, 2, STATE_DAY, &pSt->wDay) || 
                    !IsValidDate(pSt))
                    return FALSE;
                if (!is_sep(*s)) {
                    *result = PARSEST_NO_TIME;
                    return TRUE;
                }
                state = STATE_HOUR;
                szS = s + 1;
                len = 0;
            } else if (len == 10 || len == 13) { // 1000000000(000) --> 20010909094640
                if (!ToSystemTime(szS, pSt))
                    return FALSE;
                *result = PARSEST_OK;
                return TRUE;
            } else if (len == 12) { // 19920304050607
                if (!ToWord(szS, 4, STATE_YEAR, &pSt->wYear) ||
                    !ToWord(szS + 4, 2, STATE_MON, &pSt->wMonth) ||
                    !ToWord(szS + 6, 2, STATE_DAY, &pSt->wDay) ||
                    !IsValidDate(pSt) ||
                    !ToWord(szS + 8, 2, STATE_HOUR, &pSt->wHour) ||
                    !ToWord(szS + 10, 2, STATE_MIN, &pSt->wMinute))
                    return FALSE;
                *result = PARSEST_NO_SECOND;
                return TRUE;
            } else if (len == 14) { // 19920304050607
                if (!ToWord(szS, 4, STATE_YEAR, &pSt->wYear) ||
                    !ToWord(szS + 4, 2, STATE_MON, &pSt->wMonth) ||
                    !ToWord(szS + 6, 2, STATE_DAY, &pSt->wDay) ||
                    !IsValidDate(pSt) ||
                    !ToWord(szS + 8, 2, STATE_HOUR, &pSt->wHour) ||
                    !ToWord(szS + 10, 2, STATE_MIN, &pSt->wMinute) ||
                    !ToWord(szS + 12, 2, STATE_SEC, &pSt->wSecond))
                    return FALSE;
                *result = PARSEST_OK;
                return TRUE;
            } else
                return FALSE;
            break;
        case STATE_MON:
            if (!is_sep(*s) || *s != cLast || len > 2 ||
                !ToWord(szS, len, STATE_MON, &pSt->wMonth))
                return FALSE;
            state = STATE_DAY;
            szS = s + 1;
            len = 0;
            break;
        case STATE_DAY:
            if (len > 2 || !ToWord(szS, len, STATE_DAY, &pSt->wDay) ||
                !IsValidDate(pSt))
                return FALSE;
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

#ifdef TEST_PARSEST

#include <stdio.h>

int main(int argc, char *argv[])
{
    wchar_t *szStrs[] = {
        L"2020-9-28 3.04.5.jpg",
        L"20200928_030405.jpg",
        L"IMG_20200928_030405.jpg",
        L"wx_camera_1601234567682.jpg",
        L"202009280304.jpg",
        L"20200928_0304.jpg",
        L"20200928_04.68.jpg",
        L"20200928-21.jpg",
        L"20200928.24.8.jpg",
        L"20200229.jpg",
        L"20220229.jpg",
        L"202009286.jpg",
        L"20200932.jpg",
        L"20200931_030405.jpg",
        L"2020-09-31_030405.jpg",
        L"2020-09.28_030405.jpg",
    };
    for (size_t i = 0; i < sizeof(szStrs) / sizeof(szStrs[0]); i++) {
        printf("%-40ls ==> ", szStrs[i]);
        SYSTEMTIME st;
        PARSEST_RESULT result;
        if (ParseStringToSystemTime(szStrs[i], &st, &result)) {
            switch (result) {
            case PARSEST_OK:
                printf("%.4d-%.2d-%.2d %.2d:%.2d:%.2d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
                break;
            case PARSEST_NO_TIME:
                printf("%.4d-%.2d-%.2d\n", st.wYear, st.wMonth, st.wDay);
                break;
            case PARSEST_NO_SECOND:
                printf("%.4d-%.2d-%.2d %.2d:%.2d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
                break;
            }
        } else
            printf("FAILED!\n");
    }
    return 0;
}

#endif
