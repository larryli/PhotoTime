#pragma once

/** Number of rows in the list view */
#define LV_ROWS 6

/**
 * @brief Macro to get text of a column in a list view
 *
 * This macro retrieves the text of a specified column in a list view control.
 * It uses the LVM_GETCOLUMN message to get the column information, specifically
 * the text associated with the column header.
 *
 * @param hwnd Handle to the list view control
 * @param i Index of the column (0-based)
 * @param pszText_ Buffer to receive the column text
 * @param cchTextMax_ Maximum number of characters to copy to the buffer
 */
#define ListView_GetColumnText(hwnd,i,pszText_,cchTextMax_) { LV_COLUMN _ms_lvc = {.mask = LVCF_TEXT, .cchTextMax = cchTextMax_, .pszText = pszText_}; SNDMSG((hwnd),LVM_GETCOLUMN,(WPARAM)(i),(LPARAM)(LV_COLUMN*)&_ms_lvc); }

/**
 * @brief Create a list view window
 *
 * This function creates a list view control as a child window.
 *
 * @param hWndParent Handle to the parent window
 * @param hInst Instance handle of the application
 * @return Handle to the created list view window
 */
HWND CreateListViewWnd(HWND hWndParent, HINSTANCE hInst);

/**
 * @brief Handle column click event in list view
 *
 * This function handles the notification when a column header in the list view is clicked.
 *
 * @param hWndParent Handle to the parent window
 * @param nmlv Pointer to NMLISTVIEW structure containing the notification information
 */
void ListViewColumnClick(HWND hWndParent, NMLISTVIEW *nmlv);

/**
 * @brief Clean up sorting in list view
 *
 * This function cleans up any sorting applied to the list view.
 *
 * @param hListView Handle to the list view control
 */
void ListViewCleanSort(HWND hListView);

/**
 * @brief Handle display information request for list view
 *
 * This function handles the LVN_DISPINFO notification for the list view.
 *
 * @param hWndParent Handle to the parent window
 * @param lpdi Pointer to LV_DISPINFO structure containing the display information
 */
void ListViewDispInfo(HWND hWndParent, LV_DISPINFO *lpdi);

/**
 * @brief Handle custom drawing for list view
 *
 * This function handles custom drawing notifications for the list view control.
 *
 * @param hWndParent Handle to the parent window
 * @param lpcd Pointer to NMLVCUSTOMDRAW structure containing custom draw information
 * @return Result of the custom drawing operation
 */
LRESULT ListViewCustomDraw(HWND hWndParent, LPNMLVCUSTOMDRAW lpcd);

/**
 * @brief Get width of a list view column
 *
 * This function retrieves the width of a column in the list view control.
 *
 * @param hWndLV Handle to the list view control
 * @return Width of the column in pixels
 */
int ListViewGetColumnWidth(HWND hWndLV);
