#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#define __STDC_WANT_LIB_EXT1__ 1
#include <tchar.h>

#include "commctrls.h"
#include "listview.h"
#include "photo.h"
#include "utils.h"

#include "main.h"

#define WM_SORT_START (WM_USER)

static void FormatSystemTime(LPTSTR, int, SYSTEMTIME *);
static void ListView_SetHeaderSortImage(HWND, int, BOOL);

static int lastnColumnIndex = -1;
static BOOL bAscendingOrder = FALSE;

HWND CreateListViewWnd(HWND hWndParent, HINSTANCE hInst)
{
    HWND hWndLV = CreateWindowEx(WS_EX_CLIENTEDGE,
                                 WC_LISTVIEW,
                                 (LPTSTR) NULL,
                                 WS_TABSTOP | WS_CHILD | WS_VISIBLE
                                 | LVS_SHOWSELALWAYS | LVS_REPORT | LVS_OWNERDATA,
                                 0, 0, 0, 0,
                                 hWndParent,
                                 (HMENU) ID_LISTVIEW,
                                 hInst,
                                 NULL);
    ASSERT_NULL(hWndLV);
    ListView_SetExtendedListViewStyle(hWndLV,
                                      LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
                                      | LVS_EX_INFOTIP | LVS_EX_DOUBLEBUFFER);

    int cxTime = 130;
    SIZE size;
    if (GetTextExtentPoint32(GetDC(hWndLV), L"1234-67-90 12:45:78", 18, &size))
        cxTime = size.cx + 6;
    struct {
        UINT uID;
        int cx;
    } headers[LV_ROWS] = {
        {.uID = IDS_FILENAME, .cx = 180},
        {.uID = IDS_SUBDIRECTORY, .cx = 120},
        {.uID = IDS_SIZE, .cx = 80},
        {.uID = IDS_LAST_WRITE, .cx = cxTime},
        {.uID = IDS_EXIF_DATETIME, .cx = cxTime},
        {.uID = IDS_FILENAME_DATETIME, .cx = cxTime},
    };
    TCHAR szBuf[MAX_PATH];
    LV_COLUMN lvc = {
        .mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
        .fmt = LVCFMT_LEFT,
        .pszText = szBuf,
    };
    for (int i = 0; i < LV_ROWS; i++) {
        ASSERT_BREAK(LoadString(hInst, headers[i].uID, szBuf, NELEMS(szBuf)));
        lvc.cx = headers[i].cx;
        ListView_InsertColumn(hWndLV, i, &lvc);
    }
    return hWndLV;
}

int ListViewGetColumnWidth(HWND hWndLV)
{
    int w = 24; // vscroll width
    for (int i = 0; i < LV_ROWS; i++)
        w += ListView_GetColumnWidth(hWndLV, i);
    return w;
}

void ListViewColumnClick(HWND hWndParent, NMLISTVIEW *nmlv)
{
    bAscendingOrder = (lastnColumnIndex == nmlv->iSubItem) ? !bAscendingOrder : FALSE;
    ListView_SetHeaderSortImage(nmlv->hdr.hwndFrom, nmlv->iSubItem, bAscendingOrder);
    lastnColumnIndex = nmlv->iSubItem;
    SendMessage(hWndParent, WM_SORT_START, (WPARAM)nmlv->iSubItem, (LPARAM)bAscendingOrder);
}

void ListViewCleanSort(HWND hListView)
{
    lastnColumnIndex = -1;
    bAscendingOrder = FALSE;
    ListView_SetHeaderSortImage(hListView, lastnColumnIndex, bAscendingOrder);
}

void ListViewDispInfo(HWND hWndParent, LV_DISPINFO *lpdi)
{
    ASSERT_VOID(lpdi->item.mask & LVIF_TEXT);
    ASSERT_VOID(lpdi->item.iItem >= 0 && lpdi->item.iItem < gPhotoLib.iCount);
    ASSERT_VOID(gPhotoLib.pPhotos);
    PHOTO *pPhoto = gPhotoLib.pPhotos[lpdi->item.iItem];
    ASSERT_VOID(pPhoto);
    TCHAR szBuf[MAX_PATH] = L"";
    switch (lpdi->item.iSubItem) {
    case 0:
        (void)_tcscpy_s(lpdi->item.pszText, lpdi->item.cchTextMax, pPhoto->szFilename);
        return;
    case 1:
        if (pPhoto->szSubPath)
            (void)_tcscpy_s(lpdi->item.pszText, lpdi->item.cchTextMax, pPhoto->szSubPath);
        return;
    case 2:
        swprintf(szBuf, NELEMS(szBuf), L"%lld", pPhoto->filesize.QuadPart);
        break;
    case 3:
        ASSERT_VOID(pPhoto->pStFileTime);
        FormatSystemTime(szBuf, NELEMS(szBuf), pPhoto->pStFileTime);
        break;
    case 4:
        ASSERT_VOID(pPhoto->pStExifTime);
        FormatSystemTime(szBuf, NELEMS(szBuf), pPhoto->pStExifTime);
        break;
    case 5:
        ASSERT_VOID(pPhoto->pStFilenameTime);
        FormatSystemTime(szBuf, NELEMS(szBuf), pPhoto->pStFilenameTime);
        break;
    }
    (void)_tcscpy_s(lpdi->item.pszText, lpdi->item.cchTextMax, szBuf);
}

LRESULT ListViewCustomDraw(HWND hWndParent, LPNMLVCUSTOMDRAW lpcd)
{
    switch (lpcd->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
        return CDRF_NOTIFYITEMDRAW;
    case CDDS_ITEMPREPAINT:
        if ((int)lpcd->nmcd.dwItemSpec >= gPhotoLib.iCount)
            lpcd->clrText = GetSysColor(COLOR_GRAYTEXT);
        else if (gPhotoLib.pPhotos) {
            PHOTO *pPhoto = gPhotoLib.pPhotos[(int)lpcd->nmcd.dwItemSpec];
            if (!pPhoto || pPhoto->type == PHOTO_MISSING)
                lpcd->clrText = RGB(255, 0, 0); // RED
            else if (pPhoto->type == PHOTO_ERROR)
                lpcd->clrText = RGB(191, 191, 0); // YELLOW
            else if (pPhoto->type == PHOTO_WARN)
                lpcd->clrText = RGB(0, 0, 191); // BLUE
            else if (pPhoto->type == PHOTO_RIGHT)
                lpcd->clrText = RGB(0, 127, 0); // GREEN
        }
        if (lpcd->nmcd.dwItemSpec % 2)
            lpcd->clrTextBk = GetSysColor(COLOR_BTNFACE);
        break;
    }
    return CDRF_DODEFAULT;
}

static void FormatSystemTime(LPTSTR szBuf, int iMax, SYSTEMTIME *pSt)
{
    swprintf(szBuf, iMax, L"%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
             pSt->wYear, pSt->wMonth, pSt->wDay,
             pSt->wHour, pSt->wMinute, pSt->wSecond);
}

static void ListView_SetHeaderSortImage(HWND hWndLV, int columnIndex, BOOL isAscending)
{
    HWND header = ListView_GetHeader(hWndLV);
    BOOL isCommonControlVersion6 = IsCommCtrlVersion6();
    int columnCount = Header_GetItemCount(header);

    for (int i = 0; i < columnCount; i++) {
        HDITEM hi = {
            .mask = HDI_FORMAT | (isCommonControlVersion6 ? 0 : HDI_BITMAP),
        };
        Header_GetItem(header, i, &hi);
        if (i == columnIndex) {
            if (isCommonControlVersion6) {
                hi.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
                hi.fmt |= isAscending ? HDF_SORTUP : HDF_SORTDOWN;
            } else {
                UINT bitmapID = isAscending ? IDR_BMP_UP : IDR_BMP_DOWN;
                if (hi.hbm)
                    DeleteObject(hi.hbm);
                hi.fmt |= HDF_BITMAP | HDF_BITMAP_ON_RIGHT;
                hi.hbm = (HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(bitmapID), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
            }
        } else {
            if (isCommonControlVersion6)
                hi.fmt &= ~(HDF_SORTDOWN | HDF_SORTUP);
            else {
                if (hi.hbm)
                    DeleteObject(hi.hbm);
                hi.mask &= ~HDI_BITMAP;
                hi.fmt &= ~(HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
            }
        }
        Header_SetItem(header, i, &hi);
    }
}
