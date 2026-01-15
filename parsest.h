#pragma once

/**
 * @brief Result codes for parsing string to system time
 */
typedef enum {
    PARSEST_OK = 0,        /**< Parsing was successful */
    PARSEST_NO_TIME,       /**< No time information found in string */
    PARSEST_NO_SECOND,     /**< Time information found but no seconds */
} PARSEST_RESULT;

/**
 * @brief Parse a string to SYSTEMTIME structure
 *
 * This function parses a string representation of a date/time and converts it to a SYSTEMTIME structure.
 *
 * @param szStr String containing date/time information
 * @param pSt Pointer to SYSTEMTIME structure to store the parsed time
 * @param result Pointer to PARSEST_RESULT to store the parsing result code
 * @return TRUE if parsing was successful, FALSE otherwise
 */
BOOL ParseStringToSystemTime(LPCTSTR szStr, PSYSTEMTIME pSt, PARSEST_RESULT *result);
