#pragma once

#define LV_ROWS 6

HWND CreateListViewWnd(HWND hWndParent, HINSTANCE hInst);
void ListViewColumnClick(HWND hWndParent, NMLISTVIEW *nmlv);
void ListViewCleanSort(HWND hListView);
void ListViewDispInfo(HWND hWndParent, LV_DISPINFO *lpdi);
LRESULT ListViewCustomDraw(HWND hWndParent, LPNMLVCUSTOMDRAW lpcd);
int ListViewGetColumnWidth(HWND hWndLV);
