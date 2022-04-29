#pragma once

HWND CreateStatusBarWnd(HWND hWndParent, TCHAR *initialText);
void SizeStatusPanels(HWND hWndParent, HWND hWndStatusbar);
void SetStatusBarText(HWND hwndStatusBar, int id, TCHAR *szStatusString);
