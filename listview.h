#pragma once

HWND CreateListViewWnd(HWND hWndParent, HINSTANCE hInst);
void ListViewColumnClick(HWND hWndParent, NMLISTVIEW *nmlv);
void ListViewDispInfo(HWND hWndParent, LV_DISPINFO *lpdi);
void ListViewOdFindItem(HWND hWndParent, LPNMLVFINDITEM lpfi);
int ListViewGetColumnWidth(HWND hWndLV);
