#pragma once

#define LV_ROWS 6

#define ListView_GetColumnText(hwnd,i,pszText_,cchTextMax_) { LV_COLUMN _ms_lvc = {.mask = LVCF_TEXT, .cchTextMax = cchTextMax_, .pszText = pszText_}; SNDMSG((hwnd),LVM_GETCOLUMN,(WPARAM)(i),(LPARAM)(LV_COLUMN*)&_ms_lvc); }

HWND CreateListViewWnd(HWND hWndParent, HINSTANCE hInst);
void ListViewColumnClick(HWND hWndParent, NMLISTVIEW *nmlv);
void ListViewCleanSort(HWND hListView);
void ListViewDispInfo(HWND hWndParent, LV_DISPINFO *lpdi);
LRESULT ListViewCustomDraw(HWND hWndParent, LPNMLVCUSTOMDRAW lpcd);
int ListViewGetColumnWidth(HWND hWndLV);
