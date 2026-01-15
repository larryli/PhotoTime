#pragma once

/**
 * @brief Type of photo based on time information
 */
typedef enum {
    PHOTO_DEFAULT,    /**< Default photo type */
    PHOTO_MISSING,    /**< Missing photo */
    PHOTO_RIGHT,      /**< Photo with correct time information */
    PHOTO_ERROR,      /**< Photo with time information error */
    PHOTO_WARN        /**< Photo with time information warning */
} PHOTOTYPE;

/**
 * @brief Structure representing a photo
 *
 * This structure holds all relevant information about a photo file, including its
 * name, location, size, timestamps from different sources, and a classification
 * based on the consistency of time information.
 */
typedef struct {
    LPTSTR szFilename;          /**< Filename of the photo */
    LPTSTR szSubPath;           /**< Subdirectory path of the photo (relative to root) */
    LARGE_INTEGER filesize;     /**< Size of the photo file in bytes */
    PSYSTEMTIME pStFileTime;    /**< System time from file properties (last write time) */
    PSYSTEMTIME pStExifTime;    /**< System time from EXIF data embedded in the image */
    PSYSTEMTIME pStFilenameTime;/**< System time parsed from the filename */
    PHOTOTYPE type;             /**< Type of photo based on comparison of time information */
} PHOTO;

/**
 * @brief Structure representing a photo library
 */
typedef struct {
    LPTSTR szPath;              /**< Path to the photo library */
    PHOTO **pPhotos;            /**< Array of pointers to photos */
    HGLOBAL hPhotos;            /**< Global memory handle for photos */
    int iCount;                 /**< Number of photos in the library */
    int iSize;                  /**< Size of the photo array */
} PHOTOLIB;

/**
 * @brief Types of automatic processing for photos
 */
typedef enum {
    AUTOPROC_ALL,       /**< Process all time information */
    AUTOPROC_FILE,      /**< Process file time information */
    AUTOPROC_EXIF,      /**< Process EXIF time information */
} AUTOPROCTYPE;

/** Global photo library instance */
extern PHOTOLIB gPhotoLib;

/**
 * @brief Find photos in a directory
 *
 * This function searches for photos in the specified directory and populates the global photo library.
 *
 * @param szPath Path to the directory to search for photos
 * @return TRUE if successful, FALSE otherwise
 */
BOOL FindPhotos(LPCTSTR szPath);

/**
 * @brief Sort photos by specified index
 *
 * This function sorts the photos in the library based on the specified index in ascending or descending order.
 *
 * @param idx Index to sort by
 * @param isAscending TRUE to sort in ascending order, FALSE for descending
 */
void SortPhotos(int idx, BOOL isAscending);

/**
 * @brief Reload photos from the directory
 *
 * This function reloads the photos from the directory and updates the global photo library.
 *
 * @param done Pointer to integer to store the number of processed photos
 */
void ReloadPhotos(int *done);

/**
 * @brief Automatically process photos based on time information
 *
 * This function performs automatic processing of photos based on the specified type of time information.
 *
 * @param done Pointer to integer to store the number of processed photos
 * @param type Type of automatic processing to perform
 */
void AutoProcPhotos(int *done, AUTOPROCTYPE type);

/**
 * @brief Compare two SYSTEMTIME structures for equality
 *
 * This macro compares two SYSTEMTIME structures to check if they represent the same time (year, month, day, hour, minute, second).
 *
 * @param a First SYSTEMTIME structure
 * @param b Second SYSTEMTIME structure
 * @return TRUE if the times are equal, FALSE otherwise
 */
#define PSYSTEMTIME_EQUAL(a, b) ((a)->wYear == (b)->wYear \
                                 && (a)->wMonth == (b)->wMonth \
                                 && (a)->wDay == (b)->wDay \
                                 && (a)->wHour == (b)->wHour \
                                 && (a)->wMinute == (b)->wMinute \
                                 && (a)->wSecond == (b)->wSecond)
