#pragma once

/**
 * @brief Export list view content to TSV file
 *
 * This function exports the content of a list view control to a tab-separated values file.
 *
 * @param hWndLV Handle to the list view control
 * @param szPath Path to the output TSV file
 * @return TRUE if the export was successful, FALSE otherwise
 */
BOOL ExportToTsvFile(HWND hWndLV, PCTSTR szPath);

/**
 * @brief Export list view content to HTML file
 *
 * This function exports the content of a list view control to an HTML file.
 *
 * @param hWndLV Handle to the list view control
 * @param szPath Path to the output HTML file
 * @param szTitle Title for the HTML document
 * @return TRUE if the export was successful, FALSE otherwise
 */
BOOL ExportToHtmlFile(HWND hWndLV, PCTSTR szPath, PCTSTR szTitle);
