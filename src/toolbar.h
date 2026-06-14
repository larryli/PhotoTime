#pragma once

/**
 * @brief Create a toolbar window
 *
 * This function creates a toolbar control as a child window.
 *
 * @param hWndParent Handle to the parent window
 * @param hInst Instance handle of the application
 * @return Handle to the created toolbar window
 */
HWND CreateToolBarWnd(HWND hWndParent, HINSTANCE hInst);

/**
 * @brief Handle tooltip text for toolbar buttons
 *
 * This function handles the TTN_NEEDTEXT notification to provide tooltip text for toolbar buttons.
 *
 * @param hWndParent Handle to the parent window
 * @param lpttt Pointer to TOOLTIPTEXT structure to fill with tooltip information
 */
void ToolBarNeedText(HWND hWndParent, LPTOOLTIPTEXT lpttt);
