#pragma once

/**
 * @brief Initialize GDI+ library
 *
 * This function initializes the GDI+ library for image processing operations.
 */
void InitGdip(void);

/**
 * @brief Deinitialize GDI+ library
 *
 * This function deinitializes and cleans up the GDI+ library resources.
 */
void DeinitGdip(void);

/**
 * @brief Get system time from image tag
 *
 * This function extracts the system time from the tag of an image file.
 *
 * @param szFilepath Path to the image file
 * @param pSt Pointer to SYSTEMTIME structure to store the extracted time
 * @return TRUE if successful, FALSE otherwise
 */
BOOL GdipGetTagSystemTime(LPCTSTR szFilepath, PSYSTEMTIME pSt);

/**
 * @brief Load an image using GDI+
 *
 * This function loads an image file using GDI+ and returns a pointer to the image data.
 *
 * @param szFilePath Path to the image file to load
 * @return Pointer to the loaded image data, or NULL on failure
 */
void *GdipLoadImage(LPCTSTR szFilePath);

/**
 * @brief Destroy image data
 *
 * This function destroys and frees the memory associated with an image.
 *
 * @param data Pointer to the image data to destroy
 */
void GdipDestoryImage(void *data);

/**
 * @brief Draw an image using GDI+
 *
 * This function draws an image onto a device context within the specified rectangle.
 *
 * @param data Pointer to the image data to draw
 * @param hdc Device context to draw on
 * @param rc Rectangle defining where to draw the image
 * @return TRUE if successful, FALSE otherwise
 */
BOOL GdipDrawImage(void *data, HDC hdc, const RECT *rc);

/**
 * @brief Save image with system time tag
 *
 * This function saves an image file with the specified system time in its tag.
 *
 * @param szFilePath Path to the output image file
 * @param pSt Pointer to SYSTEMTIME structure containing the time to embed
 * @return TRUE if successful, FALSE otherwise
 */
BOOL GdipSaveImageWithTagSystemTime(LPCTSTR szFilePath, const PSYSTEMTIME pSt);

/**
 * @brief Get image size
 *
 * This function retrieves the dimensions of an image.
 *
 * @param data Pointer to the image data
 * @param size Pointer to SIZE structure to store the image dimensions
 * @return TRUE if successful, FALSE otherwise
 */
BOOL GdipGetSize(void *data, SIZE *size);
