#pragma once

/** Custom message for setting photo path */
#define PVM_SETPATH (WM_USER)

/**
 * @brief Set the photo path for the photo view control
 *
 * This macro sends a message to the photo view control to set the path of the photo to display.
 *
 * @param hwndCtl Handle to the photo view control
 * @param szPath Path to the photo file to display
 * @return Integer result of the message
 */
#define PhotoView_SetPath(hwndCtl, szPath)  ((int)(DWORD)SNDMSG((hwndCtl),PVM_SETPATH,(WPARAM)(PCTSTR)szPath,0))

/** Custom message for getting photo size */
#define PVM_GETSIZE (WM_USER + 1)

/**
 * @brief Get the size of the photo in the photo view control
 *
 * This macro sends a message to the photo view control to retrieve the size of the displayed photo.
 *
 * @param hwndCtl Handle to the photo view control
 * @param pSize Pointer to SIZE structure to store the photo dimensions
 * @return Integer result of the message
 */
#define PhotoView_GetSize(hwndCtl, pSize)  ((int)(DWORD)SNDMSG((hwndCtl),PVM_GETSIZE,(WPARAM)(PSIZE)pSize,0))

/**
 * @brief Create a photo view window
 *
 * This function creates a photo view control as a child window.
 *
 * @param hWndParent Handle to the parent window
 * @param hInst Instance handle of the application
 * @return Handle to the created photo view window
 */
HWND CreatePhotoViewWnd(HWND hWndParent, HINSTANCE hInst);

/**
 * @brief Destroy the photo view window
 *
 * This function destroys the photo view control and cleans up its resources.
 *
 * @param hWndPV Handle to the photo view window to destroy
 */
void DestroyPhotoViewWnd(HWND hWndPV);
