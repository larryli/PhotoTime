#pragma once

/**
 * @brief Calculate number of elements in an array
 *
 * This macro calculates the number of elements in an array by dividing the total size
 * of the array by the size of its first element. This is a common technique in C/C++
 * to determine array length at compile time.
 *
 * @param a The array to count elements of
 * @return The number of elements in the array
 */
#ifndef NELEMS
#define NELEMS(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifdef ASSERT
#pragma message("ERROR! Failed to define marco ASSERT!!")
#else
/**
 * @brief Assertion macro that executes statement if condition is false
 *
 * This macro evaluates a condition and executes the specified statement if the condition is false.
 *
 * @param a The condition to evaluate
 * @param s The statement to execute if the condition is false
 */
#define ASSERT(a, s) if (!(a)) s

/**
 * @brief Assertion macro that breaks from loop if condition is false
 *
 * This macro evaluates a condition and breaks from the current loop if the condition is false.
 *
 * @param a The condition to evaluate
 */
#define ASSERT_BREAK(a) ASSERT(a, break)

/**
 * @brief Assertion macro that continues loop if condition is false
 *
 * This macro evaluates a condition and continues to the next iteration if the condition is false.
 *
 * @param a The condition to evaluate
 */
#define ASSERT_CONTINUE(a) ASSERT(a, continue)

/**
 * @brief Assertion macro that returns from function if condition is false
 *
 * This macro evaluates a condition and returns from the current function if the condition is false.
 *
 * @param a The condition to evaluate
 */
#define ASSERT_VOID(a) ASSERT(a, return)

/**
 * @brief Assertion macro that returns a value from function if condition is false
 *
 * This macro evaluates a condition and returns the specified value from the current function if the condition is false.
 *
 * @param a The condition to evaluate
 * @param r The value to return if the condition is false
 */
#define ASSERT_RETURN(a, r) ASSERT(a, return (r))

/**
 * @brief Assertion macro that returns NULL from function if condition is false
 *
 * This macro evaluates a condition and returns NULL from the current function if the condition is false.
 *
 * @param a The condition to evaluate
 */
#define ASSERT_NULL(a) ASSERT_RETURN(a, NULL)

/**
 * @brief Assertion macro that returns FALSE from function if condition is false
 *
 * This macro evaluates a condition and returns FALSE from the current function if the condition is false.
 *
 * @param a The condition to evaluate
 */
#define ASSERT_FALSE(a) ASSERT_RETURN(a, FALSE)

/**
 * @brief Assertion macro that jumps to label if condition is false
 *
 * This macro evaluates a condition and jumps to the specified label if the condition is false.
 *
 * @param a The condition to evaluate
 * @param l The label to jump to if the condition is false
 */
#define ASSERT_GOTO(a, l) ASSERT(a, goto l)

/**
 * @brief Assertion macro that jumps to 'end' label if condition is false
 *
 * This macro evaluates a condition and jumps to the 'end' label if the condition is false.
 *
 * @param a The condition to evaluate
 */
#define ASSERT_END(a) ASSERT_GOTO(a, end)

/**
 * @brief Assertion macro that jumps to 'failed' label if condition is false
 *
 * This macro evaluates a condition and jumps to the 'failed' label if the condition is false.
 *
 * @param a The condition to evaluate
 */
#define ASSERT_FAILED(a) ASSERT_GOTO(a, failed)
#endif

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
void CatFilePath(PTSTR szBuf, int size, PCTSTR szParent, PCTSTR szPath);

/**
 * @brief Get file extension from path
 *
 * This function extracts the file extension from a file path.
 *
 * @param szPath Path to the file
 * @return Pointer to the extension part of the path (including the dot), or NULL if no extension exists
 */
PCTSTR GetFileExt(PCTSTR szPath);

/**
 * @brief Convert FILETIME to local SYSTEMTIME
 *
 * This function converts a FILETIME structure to a local SYSTEMTIME structure, adjusting for the local timezone.
 *
 * @param pFt Pointer to FILETIME structure to convert
 * @param pSt Pointer to SYSTEMTIME structure to store the converted time
 * @return TRUE if successful, FALSE otherwise
 */
BOOL FileTimeToLocalSystemTime(CONST FILETIME *pFt, PSYSTEMTIME pSt);

/**
 * @brief Convert local SYSTEMTIME to FILETIME
 *
 * This function converts a local SYSTEMTIME structure to a FILETIME structure, adjusting for the local timezone.
 *
 * @param pSt Pointer to SYSTEMTIME structure to convert
 * @param pFt Pointer to FILETIME structure to store the converted time
 * @return TRUE if successful, FALSE otherwise
 */
BOOL LocalSystemTimeToFileTime(CONST SYSTEMTIME *pSt, PFILETIME pFt);
