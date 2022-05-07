#pragma once

typedef enum {
    PARSEST_OK = 0,
    PARSEST_NO_TIME,
    PARSEST_NO_SECOND,
} PARSEST_RESULT;

BOOL ParseStringToSystemTime(LPCTSTR szStr, PSYSTEMTIME pSt, PARSEST_RESULT *result);
