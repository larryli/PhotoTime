#pragma once

/**
 * @brief Create a status bar window
 *
 * This function creates a status bar control as a child window with optional text and URL.
 *
 * @param hWndParent Handle to the parent window
 * @param hInst Instance handle of the application
 * @param szText Initial text to display in the status bar
 * @param szUrl URL to associate with the status bar (for clickable links)
 * @return Handle to the created status bar window
 */
HWND CreateStatusBarWnd(HWND hWndParent, HINSTANCE hInst, TCHAR *szText, TCHAR *szUrl);

/**
 * @brief Size the panels in the status bar
 *
 * This function adjusts the sizes of the panels in the status bar based on the parent window size.
 *
 * @param hWndParent Handle to the parent window
 * @param hWndStatusBar Handle to the status bar window
 */
void SizeStatusPanels(HWND hWndParent, HWND hWndStatusBar);

/**
 * @brief Set text in a status bar panel
 *
 * This function sets the text in a specific panel of the status bar.
 *
 * @param hwndStatusBar Handle to the status bar window
 * @param id ID of the panel to update
 * @param szStatusString Text to display in the panel
 */
void SetStatusBarText(HWND hwndStatusBar, int id, TCHAR *szStatusString);

/**
 * @brief Get the system link window associated with the status bar
 *
 * This inline function retrieves the handle to the system link window associated with the status bar.
 *
 * @param hWndStatusBar Handle to the status bar window
 * @return Handle to the system link window, or NULL if none is associated
 */
inline HWND GetStatusBarSysLinkWnd(HWND hWndStatusBar)
{
    return (HWND)GetWindowLongPtr(hWndStatusBar, GWLP_USERDATA);
}
