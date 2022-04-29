#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include "main.h"

#include "commctrls.h"
#include "listview.h"

#define NELEMS(a) (sizeof(a) / sizeof((a)[0]))

static void ListView_SetHeaderSortImage(HWND, int, BOOL);

static int iCount = 0;

HWND CreateListViewWnd(HWND hWndParent, HINSTANCE hInst)
{
    HWND hWndLV = CreateWindowEx(WS_EX_CLIENTEDGE,
                                 WC_LISTVIEW,
                                 (LPTSTR) NULL,
                                 WS_TABSTOP | WS_CHILD | WS_VISIBLE | LVS_AUTOARRANGE | LVS_REPORT | LVS_OWNERDATA,
                                 0, 0, 0, 0,
                                 hWndParent,
                                 (HMENU) ID_LISTVIEW,
                                 hInst,
                                 NULL);
    if (!hWndLV)
        return NULL;
    ListView_SetExtendedListViewStyle(hWndLV, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

    TCHAR szBuf[MAX_PATH];
    struct {
        UINT uID;
        int cx;
    } headers[] = {
        {.uID = IDS_FILENAME, .cx = 180},
        {.uID = IDS_SUBDIRECTORY, .cx = 180},
        {.uID = IDS_SIZE, .cx = 120},
        {.uID = IDS_LAST_MODIFIED, .cx = 120},
        {.uID = IDS_EXIF_DATETIME, .cx = 120},
        {.uID = IDS_FILENAME_DATETIME, .cx = 120},
    };
    LV_COLUMN lvc = {
        .mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
        .fmt = LVCFMT_LEFT,
        .pszText = szBuf,
    };
    iCount = (int)NELEMS(headers);
    for (int i = 0; i < iCount; i++) {
        if (!LoadString(hInst, headers[i].uID, szBuf, NELEMS(szBuf)))
            break;
        lvc.cx = headers[i].cx;
        ListView_InsertColumn(hWndLV, i, &lvc);
    }
    return hWndLV;
}

int ListViewGetColumnWidth(HWND hWndLV)
{
    int w = 24; // vscroll width
    for (int i = 0; i < iCount; i++)
        w += ListView_GetColumnWidth(hWndLV, i);
    return w;
}

void ListViewColumnClick(HWND hWndParent, NMLISTVIEW *nmlv)
{
    static int lastnListViewColumnIndex = -1;
    static BOOL bListViewAscendingOrder = FALSE;
    HWND hListView = nmlv->hdr.hwndFrom;
    INT nListViewColumnIndex = nmlv->iSubItem;
    if (lastnListViewColumnIndex == nListViewColumnIndex) {
        bListViewAscendingOrder = !bListViewAscendingOrder;
    } else {
        bListViewAscendingOrder = FALSE;
    }
    ListView_SetHeaderSortImage(hListView, nListViewColumnIndex, bListViewAscendingOrder);
    lastnListViewColumnIndex = nListViewColumnIndex;
}

void ListViewDispInfo(HWND hWndParent, LV_DISPINFO *lpdi)
{
    TCHAR szString[MAX_PATH];

    if (lpdi->item.iSubItem) {
        if (lpdi->item.mask & LVIF_TEXT) {
            wsprintf(szString, TEXT("More %d.%d"), lpdi->item.iItem + 1, lpdi->item.iSubItem);
            lstrcpyn(lpdi->item.pszText, szString, lpdi->item.cchTextMax);
        }
    } else {
        if (lpdi->item.mask & LVIF_TEXT) {
            wsprintf(szString, TEXT("Item %d"), lpdi->item.iItem + 1);
            lstrcpyn(lpdi->item.pszText, szString, lpdi->item.cchTextMax);
        }
        if (lpdi->item.mask & LVIF_IMAGE) {
            lpdi->item.iImage = 0;
        }
    }
}

void ListViewOdFindItem(HWND hWndParent, LPNMLVFINDITEM lpfi)
{
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
