/*
 * test_parsest.c - Unit tests for timestamp parser
 *
 * Tests the ParseStringToSystemTime function with various filename formats.
 */

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "../src/parsest.h"

/* Test result tracking */
static int tests_passed = 0;
static int tests_failed = 0;

/* Test helper macro */
#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("  PASS: %s\n", message); \
            tests_passed++; \
        } else { \
            printf("  FAIL: %s\n", message); \
            tests_failed++; \
        } \
    } while (0)

/* Helper to check SYSTEMTIME values */
#define CHECK_TIME(st, y, m, d, h, min, s) \
    ((st).wYear == (y) && (st).wMonth == (m) && (st).wDay == (d) && \
     (st).wHour == (h) && (st).wMinute == (min) && (st).wSecond == (s))

/* Helper to check date only (no time) */
#define CHECK_DATE(st, y, m, d) \
    ((st).wYear == (y) && (st).wMonth == (m) && (st).wDay == (d))

/* Test 1: Standard format YYYYMMDDHHMMSS */
static void test_standard_format(void)
{
    printf("\nTest 1: Standard format YYYYMMDDHHMMSS\n");

    SYSTEMTIME st;
    PARSEST_RESULT result;
    BOOL ret;

    ret = ParseStringToSystemTime(L"20200928030405.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE");
    TEST_ASSERT(result == PARSEST_OK, "Result is PARSEST_OK");
    TEST_ASSERT(CHECK_TIME(st, 2020, 9, 28, 3, 4, 5), "Time is 2020-09-28 03:04:05");

    ret = ParseStringToSystemTime(L"19991231235959.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE");
    TEST_ASSERT(result == PARSEST_OK, "Result is PARSEST_OK");
    TEST_ASSERT(CHECK_TIME(st, 1999, 12, 31, 23, 59, 59), "Time is 1999-12-31 23:59:59");
}

/* Test 2: Standard format with separator */
static void test_standard_with_separator(void)
{
    printf("\nTest 2: Standard format with separator YYYY-MM-DD HH.MM.SS\n");

    SYSTEMTIME st;
    PARSEST_RESULT result;
    BOOL ret;

    ret = ParseStringToSystemTime(L"2020-09-28 03.04.05.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE");
    TEST_ASSERT(result == PARSEST_OK, "Result is PARSEST_OK");
    TEST_ASSERT(CHECK_TIME(st, 2020, 9, 28, 3, 4, 5), "Time is 2020-09-28 03:04:05");

    ret = ParseStringToSystemTime(L"2020-9-28 3.04.5.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE");
    TEST_ASSERT(result == PARSEST_OK, "Result is PARSEST_OK");
    TEST_ASSERT(CHECK_TIME(st, 2020, 9, 28, 3, 4, 5), "Time is 2020-09-28 03:04:05");
}

/* Test 3: IMG prefix format */
static void test_img_prefix(void)
{
    printf("\nTest 3: IMG prefix format IMG_YYYYMMDD_HHMMSS\n");

    SYSTEMTIME st;
    PARSEST_RESULT result;
    BOOL ret;

    ret = ParseStringToSystemTime(L"IMG_20200928_030405.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE");
    TEST_ASSERT(result == PARSEST_OK, "Result is PARSEST_OK");
    TEST_ASSERT(CHECK_TIME(st, 2020, 9, 28, 3, 4, 5), "Time is 2020-09-28 03:04:05");

    ret = ParseStringToSystemTime(L"IMG_20200928_030405_HDR.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE");
    TEST_ASSERT(result == PARSEST_OK, "Result is PARSEST_OK");
    TEST_ASSERT(CHECK_TIME(st, 2020, 9, 28, 3, 4, 5), "Time is 2020-09-28 03:04:05");
}

/* Test 4: Unix timestamp format (WeChat) */
static void test_unix_timestamp(void)
{
    printf("\nTest 4: Unix timestamp format (WeChat)\n");

    SYSTEMTIME st;
    PARSEST_RESULT result;
    BOOL ret;

    ret = ParseStringToSystemTime(L"wx_camera_1601234567682.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE");
    TEST_ASSERT(result == PARSEST_OK, "Result is PARSEST_OK");
    TEST_ASSERT(st.wYear == 2020, "Year is 2020");

    ret = ParseStringToSystemTime(L"microMsg.1601234567682.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE");
    TEST_ASSERT(result == PARSEST_OK, "Result is PARSEST_OK");

    ret = ParseStringToSystemTime(L"mmexport1601234567682.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE");
    TEST_ASSERT(result == PARSEST_OK, "Result is PARSEST_OK");
}

/* Test 5: Date only format */
static void test_date_only(void)
{
    printf("\nTest 5: Date only format YYYYMMDD\n");

    SYSTEMTIME st;
    PARSEST_RESULT result;
    BOOL ret;

    ret = ParseStringToSystemTime(L"20200928.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE");
    TEST_ASSERT(result == PARSEST_NO_TIME, "Result is PARSEST_NO_TIME");
    TEST_ASSERT(CHECK_DATE(st, 2020, 9, 28), "Date is 2020-09-28");
}

/* Test 6: Date and hour/minute only */
static void test_no_seconds(void)
{
    printf("\nTest 6: Date and hour/minute only YYYYMMDDHHMM\n");

    SYSTEMTIME st;
    PARSEST_RESULT result;
    BOOL ret;

    ret = ParseStringToSystemTime(L"202009280304.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE");
    TEST_ASSERT(result == PARSEST_NO_SECOND, "Result is PARSEST_NO_SECOND");
    TEST_ASSERT(CHECK_TIME(st, 2020, 9, 28, 3, 4, 0), "Time is 2020-09-28 03:04:00");

    ret = ParseStringToSystemTime(L"20200928_0304.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE");
    TEST_ASSERT(result == PARSEST_NO_SECOND, "Result is PARSEST_NO_SECOND");
    TEST_ASSERT(CHECK_TIME(st, 2020, 9, 28, 3, 4, 0), "Time is 2020-09-28 03:04:00");
}

/* Test 7: Leap year validation */
static void test_leap_year(void)
{
    printf("\nTest 7: Leap year validation\n");

    SYSTEMTIME st;
    PARSEST_RESULT result;
    BOOL ret;

    /* Valid leap year date */
    ret = ParseStringToSystemTime(L"20200229.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE for 2020-02-29");
    TEST_ASSERT(CHECK_DATE(st, 2020, 2, 29), "Date is 2020-02-29");

    /* Invalid leap year date */
    ret = ParseStringToSystemTime(L"20220229.jpg", &st, &result);
    TEST_ASSERT(ret == FALSE, "Parse returns FALSE for 2022-02-29");
}

/* Test 8: Invalid dates */
static void test_invalid_dates(void)
{
    printf("\nTest 8: Invalid dates\n");

    SYSTEMTIME st;
    PARSEST_RESULT result;
    BOOL ret;

    /* Invalid month */
    ret = ParseStringToSystemTime(L"20201328.jpg", &st, &result);
    TEST_ASSERT(ret == FALSE, "Parse returns FALSE for month 13");

    /* Invalid day */
    ret = ParseStringToSystemTime(L"20200932.jpg", &st, &result);
    TEST_ASSERT(ret == FALSE, "Parse returns FALSE for day 32");

    /* Invalid day for month (Sep has 30 days) */
    ret = ParseStringToSystemTime(L"20200931.jpg", &st, &result);
    TEST_ASSERT(ret == FALSE, "Parse returns FALSE for Sep 31");
}

/* Test 9: Mixed separators */
static void test_mixed_separators(void)
{
    printf("\nTest 9: Mixed separators\n");

    SYSTEMTIME st;
    PARSEST_RESULT result;
    BOOL ret;

    /* Mixed separators should fail */
    ret = ParseStringToSystemTime(L"2020-09.28_030405.jpg", &st, &result);
    TEST_ASSERT(ret == FALSE, "Parse returns FALSE for mixed separators");
}

/* Test 10: Invalid time values */
static void test_invalid_time(void)
{
    printf("\nTest 10: Invalid time values\n");

    SYSTEMTIME st;
    PARSEST_RESULT result;
    BOOL ret;

    /* Invalid hour */
    ret = ParseStringToSystemTime(L"20200928240405.jpg", &st, &result);
    TEST_ASSERT(ret == FALSE, "Parse returns FALSE for hour 24");

    /* Invalid minute */
    ret = ParseStringToSystemTime(L"20200928036005.jpg", &st, &result);
    TEST_ASSERT(ret == FALSE, "Parse returns FALSE for minute 60");
}

/* Test 11: Edge cases */
static void test_edge_cases(void)
{
    printf("\nTest 11: Edge cases\n");

    SYSTEMTIME st;
    PARSEST_RESULT result;
    BOOL ret;

    /* Minimum valid date */
    ret = ParseStringToSystemTime(L"19000101.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE for 1900-01-01");
    TEST_ASSERT(CHECK_DATE(st, 1900, 1, 1), "Date is 1900-01-01");

    /* Maximum valid date */
    ret = ParseStringToSystemTime(L"21001231.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE for 2100-12-31");
    TEST_ASSERT(CHECK_DATE(st, 2100, 12, 31), "Date is 2100-12-31");

    /* Midnight */
    ret = ParseStringToSystemTime(L"20200928000000.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE for midnight");
    TEST_ASSERT(CHECK_TIME(st, 2020, 9, 28, 0, 0, 0), "Time is 00:00:00");

    /* End of day */
    ret = ParseStringToSystemTime(L"20200928235959.jpg", &st, &result);
    TEST_ASSERT(ret == TRUE, "Parse returns TRUE for end of day");
    TEST_ASSERT(CHECK_TIME(st, 2020, 9, 28, 23, 59, 59), "Time is 23:59:59");
}

/* Main test runner */
int main(int argc, char *argv[])
{
    printf("=== PhotoTime parsest unit tests ===\n");

    test_standard_format();
    test_standard_with_separator();
    test_img_prefix();
    test_unix_timestamp();
    test_date_only();
    test_no_seconds();
    test_leap_year();
    test_invalid_dates();
    test_mixed_separators();
    test_invalid_time();
    test_edge_cases();

    printf("\n=== Test Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
