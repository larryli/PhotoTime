#pragma once

HWND CreateListViewWnd(HWND hWndParent, HINSTANCE hInst);
void ListViewColumnClick(HWND hWndParent, NMLISTVIEW *nmlv);
void ListViewCleanSort(HWND hListView);
void ListViewDispInfo(HWND hWndParent, LV_DISPINFO *lpdi);
void ListViewOdFindItem(HWND hWndParent, LPNMLVFINDITEM lpfi);
LRESULT ListViewCustomDraw(HWND hWndParent, LPNMLVCUSTOMDRAW lpcd);
int ListViewGetColumnWidth(HWND hWndLV);
