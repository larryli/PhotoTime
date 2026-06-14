#pragma once

/**
 * @brief Dialog procedure for the About dialog box
 *
 * This function handles messages sent to the About dialog box.
 *
 * @param hWnd Handle to the dialog box window
 * @param uMsg Specifies the message
 * @param wParam Additional message-specific information
 * @param lParam Additional message-specific information
 * @return The return value depends on the message
 */
LRESULT WINAPI AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
