#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <wchar.h>
#include <process.h>

#include <shellapi.h>
#include <Shlobj.h>
#include <commctrl.h>
#include <commdlg.h>

#pragma comment(lib, "Shell32.lib")

#include "main.h"

#include "about.h"
#include "commctrls.h"
#include "listview.h"
#include "toolbar.h"
#include "statusbar.h"
#include "photo.h"
#include "utils.h"

#define WM_FIND_DONE (WM_USER + 1)
#define WM_SORT_START (WM_USER + 2)
#define WM_SORT_DONE (WM_USER + 3)

#define ID_TIMER_FIND 1

static BOOL InitApplication(HINSTANCE);
static BOOL InitInstance(HINSTANCE, int);
static LRESULT WINAPI MainWndProc(HWND, UINT, WPARAM, LPARAM);
static LRESULT Main_OnCreate(HWND, LPCREATESTRUCT);
static LRESULT Main_OnSize(HWND, int, int, int);
static void Main_OnLButtonDown(HWND, BOOL, int, int, UINT);
static void Main_OnLButtonUp(HWND, int, int, UINT);
static void Main_OnMouseMove(HWND, int, int, UINT);
static LRESULT Main_OnNotify(HWND, int, NMHDR *);
static void Main_OnCommand(HWND, int, HWND, UINT);
static void Main_OnContextMenu(HWND, HWND, UINT, UINT);
static void Main_OnDestroy(HWND);
static void Main_OnTimer(HWND, UINT);

static void Main_OnFindDone(HWND, BOOL);
static void Main_OnSortStart(HWND, int, BOOL);
static void Main_OnSortDone(HWND);

static void Lock(HWND);
static void UnLock(HWND);

static HANDLE ghInstance;
static HCURSOR ghCurSizeEW, ghCurArrow;
static HWND ghWndToolBar, ghWndListView, ghWndPhoto, ghWndStatusBar;
static RECT gRcClient;
static BOOL bSplitDrag = FALSE;
static BOOL bLock = FALSE;

#define MIN_CX_LISTVIEW 320
#define MIN_CX_PHOTO 320
static int cxListView = 0;
static int cxPhoto = 0;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pszCmdLine, int nCmdShow)
{
    ghInstance = hInstance;
    ghCurSizeEW = LoadCursor(NULL, IDC_SIZEWE);
    ghCurArrow = LoadCursor(NULL, IDC_ARROW);
    if (!hPrevInstance)
        if (!InitApplication(hInstance))
            return FALSE;
    InitCommCtrl();
    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    MSG msg;
#if 0
    /* "Politically correct" code -- SEE MICROSOFT DOCUMENTATION */
    for (;;) {
        BOOL fRet = GetMessage(&msg, NULL, 0, 0);
        if (fRet == -1) { /* Error */
            /* TODO: handle the error from GetMessage() */
            __debugbreak();
            return -1;
        } else if (fRet == 0) { /* WM_QUIT */
            break;
        } else { /* Not error or WM_QUIT */
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
#else
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#endif
    return msg.wParam;
}

static BOOL InitApplication(HINSTANCE hInstance)
{
    WNDCLASSEX wc = {
        .cbSize = sizeof(WNDCLASSEX),
        .lpszClassName = L"PhotoTimeClass",
        .lpfnWndProc = MainWndProc,
        .style = 0,
        .hInstance = hInstance,
        .hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_ICO_MAIN)),
        .hIconSm = LoadImage(hInstance, MAKEINTRESOURCE(IDR_ICO_MAIN), IMAGE_ICON, 16, 16, 0),
        .hCursor = LoadCursor(NULL, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszMenuName = MAKEINTRESOURCE(IDR_MNU_MAIN),
        .cbClsExtra = 0,
        .cbWndExtra = 0,
    };
    if (!RegisterClassEx(&wc))
        return FALSE;
    return TRUE;
}

static BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    TCHAR szTitle[MAX_PATH] = L"PhotoTime";
    if (!LoadString(hInstance, IDS_APPTITLE, szTitle, NELEMS(szTitle)))
        return FALSE;
    HWND hwnd = CreateWindow(L"PhotoTimeClass",
                             szTitle,
                             WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             NULL,
                             NULL,
                             hInstance,
                             NULL);
    if (!hwnd)
        return FALSE;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return TRUE;
}

static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    HANDLE_MSG(hwnd, WM_CREATE, Main_OnCreate);
    HANDLE_MSG(hwnd, WM_SIZE, Main_OnSize);
    HANDLE_MSG(hwnd, WM_COMMAND, Main_OnCommand);
    HANDLE_MSG(hwnd, WM_CONTEXTMENU, Main_OnContextMenu);
    HANDLE_MSG(hwnd, WM_NOTIFY, Main_OnNotify);
    HANDLE_MSG(hwnd, WM_DESTROY, Main_OnDestroy);
    HANDLE_MSG(hwnd, WM_LBUTTONDOWN, Main_OnLButtonDown);
    HANDLE_MSG(hwnd, WM_LBUTTONUP, Main_OnLButtonUp);
    HANDLE_MSG(hwnd, WM_MOUSEMOVE, Main_OnMouseMove);
    HANDLE_MSG(hwnd, WM_TIMER, Main_OnTimer);
    case WM_FIND_DONE:
        return Main_OnFindDone(hwnd, (BOOL)wParam), 0;
    case WM_SORT_START:
        return Main_OnSortStart(hwnd, (int)wParam, (BOOL)lParam), 0;
    case WM_SORT_DONE:
        return Main_OnSortDone(hwnd), 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

static LRESULT Main_OnCreate(HWND hwnd, LPCREATESTRUCT lParam)
{
    ghWndToolBar = CreateToolBarWnd(hwnd, ghInstance);
    if (!ghWndToolBar)
        return FALSE;
    TCHAR szReady[MAX_PATH] = L"", szUrl[MAX_PATH] = L"";
    if (!LoadString(ghInstance, IDS_READY, szReady, NELEMS(szReady)))
        return FALSE;
    if (!LoadString(ghInstance, IDS_LINK, szUrl, NELEMS(szUrl)))
        return FALSE;
    ghWndStatusBar = CreateStatusBarWnd(hwnd, ghInstance, szReady, szUrl);
    if (!ghWndStatusBar)
        return FALSE;
    ghWndListView = CreateListViewWnd(hwnd, ghInstance);
    if (!ghWndListView)
        return FALSE;
    ghWndPhoto = CreateWindowEx(WS_EX_STATICEDGE,
                                L"Static",
                                NULL,
                                WS_CHILD | WS_VISIBLE | LWS_TRANSPARENT,
                                0, 0, 0, 0,
                                hwnd,
                                0,
                                ghInstance,
                                NULL);
    if (!ghWndPhoto)
        return FALSE;
    SetFocus(ghWndListView);
    return TRUE;
}

static BOOL GetRect(HWND hwnd, RECT *p)
{
    RECT rc;
    if (!GetClientRect(hwnd, p))
        return FALSE;
    if (!GetWindowRect(ghWndStatusBar, &rc))
        return FALSE;
    if (!ScreenToClient(hwnd, (LPPOINT)&rc.left))
        return FALSE;
    p->bottom = rc.top;
    if (!GetWindowRect(ghWndToolBar, &rc))
        return FALSE;
    p->top = rc.bottom - rc.top - 3;
    return TRUE;
}

static LRESULT Main_OnSize(HWND hwnd, int flag, int x, int y)
{
    SendMessage(ghWndToolBar, WM_SIZE, x, y);
    SendMessage(ghWndStatusBar, WM_SIZE, x, y);
    SizeStatusPanels(hwnd, ghWndStatusBar);

    if (flag != SIZE_MINIMIZED) {
        GetRect(hwnd, &gRcClient);
        LONG cx = gRcClient.right - gRcClient.left;
        LONG cy = gRcClient.bottom - gRcClient.top;
        if (cx > MIN_CX_LISTVIEW + MIN_CX_PHOTO + 2) {
            if (cxListView > cx - MIN_CX_PHOTO - 2) {
                cxListView = cx - MIN_CX_PHOTO - 2;
            } else {
                int cxListViewCur = ListViewGetColumnWidth(ghWndListView);
                if (cxListView < cxListViewCur && cxListView + cxPhoto < cx - 2) {
                    cxListView = cx - cxPhoto - 2;
                    if (cxListView > cxListViewCur)
                        cxListView = cxListViewCur;
                }
            }
            if (cxListView < MIN_CX_LISTVIEW)
                cxListView = MIN_CX_LISTVIEW;
            cxPhoto = cx - cxListView - 2;
            if (cxPhoto < MIN_CX_PHOTO) {
                cxPhoto = MIN_CX_PHOTO;
                cxListView = cx - cxPhoto - 2;
            }
        } else {
            cxListView = cx;
            cxPhoto = 0;
        }
        MoveWindow(ghWndListView, gRcClient.left, gRcClient.top, cxListView, cy, TRUE);
        MoveWindow(ghWndPhoto, gRcClient.left + cxListView + 2, gRcClient.top, cxPhoto, cy, TRUE);
    }
    return 0;
}

static void Main_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    SetCapture(hwnd);
    bSplitDrag = TRUE;
}

static void Main_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    ReleaseCapture();
    bSplitDrag = FALSE;
}

static void Main_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
    if (x > cxListView && x <= cxListView + 2 && y >= gRcClient.top && y <= gRcClient.bottom)
        SetClassLongPtr(hwnd, GCLP_HCURSOR, (LPARAM)ghCurSizeEW);
    else
        SetClassLongPtr(hwnd, GCLP_HCURSOR, (LPARAM)ghCurArrow);
    if (bSplitDrag) {
        LONG cx = gRcClient.right - gRcClient.left;
        LONG cy = gRcClient.bottom - gRcClient.top;
        if ((x - gRcClient.left > MIN_CX_LISTVIEW + 1) && (x - gRcClient.left < cx - MIN_CX_PHOTO - 1)) {
            cxListView = x - gRcClient.left - 1;
            cxPhoto = cx - cxListView - 2;
            MoveWindow(ghWndListView, gRcClient.left, gRcClient.top, cxListView, cy, TRUE);
            MoveWindow(ghWndPhoto, gRcClient.left + cxListView + 2, gRcClient.top, cxPhoto, cy, TRUE);
        }
    }
}

typedef struct {
    HWND hWnd;
    TCHAR szPath[MAX_PATH];
} FIND_THREAD_PARAMS;

static void FindThread(PVOID pVoid)
{
    FIND_THREAD_PARAMS *pParams = (FIND_THREAD_PARAMS *)pVoid;
    WPARAM wParam = (WPARAM)FindPhoto(pParams->szPath);
    SendMessage(pParams->hWnd, WM_FIND_DONE, wParam, 0);
    GlobalFree(pVoid);
    _endthread();
}

static void OpenDirectory(HWND hwnd)
{
    TCHAR szTitle[MAX_PATH];
    if (!LoadString(ghInstance, IDS_SELECT_PHOTO_DIRECTORY, szTitle, NELEMS(szTitle)))
        return;
    BROWSEINFO bInfo = {
        .lpszTitle = szTitle,
        .ulFlags = BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_USENEWUI | BIF_NONEWFOLDERBUTTON,
    };
    LPITEMIDLIST lpDlist = SHBrowseForFolder(&bInfo);
    if (lpDlist == NULL)
        return;
    TCHAR szPath[MAX_PATH] = {0};
    if (!SHGetPathFromIDList(lpDlist, szPath))
        return;
    SetStatusBarText(ghWndStatusBar, 1, szPath);
    if (LoadString(ghInstance, IDS_APPTITLE_FMT, szTitle, NELEMS(szTitle))) {
        TCHAR szBuf[MAX_PATH * 2] = {0};
        _snwprintf(szBuf, NELEMS(szBuf), szTitle, szPath);
        SetWindowText(hwnd, szBuf);
    }

    FIND_THREAD_PARAMS *pParams = (FIND_THREAD_PARAMS *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(FIND_THREAD_PARAMS));
    if (pParams) {
        pParams->hWnd = hwnd;
        lstrcpyn(pParams->szPath, szPath, NELEMS(pParams->szPath));
        Lock(hwnd);
        ListView_DeleteAllItems(ghWndListView);
        ListViewCleanSort(ghWndListView);
        _beginthread(FindThread, 0, pParams);
        TCHAR szBuf[MAX_PATH];
        if (LoadString(ghInstance, IDS_FIND_START, szBuf, NELEMS(szBuf)))
            SetStatusBarText(ghWndStatusBar, 0, szBuf);
        SetTimer(hwnd, ID_TIMER_FIND, 200, NULL);
    }
}

static void Main_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {
    case IDM_ABOUT:
        DialogBox(ghInstance, MAKEINTRESOURCE(DLG_ABOUT), hwnd, (DLGPROC)AboutDlgProc);
        return;
    case IDM_OPEN:
        OpenDirectory(hwnd);
        return;
    case IDM_EXIT:
        PostMessage(hwnd, WM_CLOSE, 0, 0L);
        return;
    }
}

static void Main_OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos)
{
    if (hwndContext == ghWndListView && ListView_GetSelectedCount(ghWndListView)) {
        HMENU hMenuLoad = LoadMenu(ghInstance, MAKEINTRESOURCE(IDR_MNU_CONTEXT));
        HMENU hMenu = GetSubMenu(hMenuLoad, 0);

        TrackPopupMenu(hMenu,
                       TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                       xPos, yPos,
                       0,
                       hwnd,
                       NULL);

        DestroyMenu(hMenuLoad);
    }
}

static void UpdateStatus(UINT uSel, UINT uDef)
{
    TCHAR szFmt[MAX_PATH];
    int iSelected = ListView_GetSelectedCount(ghWndListView);
    if (iSelected) {
        if (LoadString(ghInstance, uSel, szFmt, NELEMS(szFmt))) {
            TCHAR szBuf[MAX_PATH];
            _snwprintf(szBuf, NELEMS(szBuf), szFmt, gPhotos.iCount, iSelected);
            SetStatusBarText(ghWndStatusBar, 0, szBuf);
        }
    } else if (LoadString(ghInstance, uDef, szFmt, NELEMS(szFmt))) {
        TCHAR szBuf[MAX_PATH];
        _snwprintf(szBuf, NELEMS(szBuf), szFmt, gPhotos.iCount);
        SetStatusBarText(ghWndStatusBar, 0, szBuf);
    }
}

static LRESULT Main_OnNotify(HWND hwnd, int wParam, NMHDR *lParam)
{
    switch (lParam->code) {
    case LVN_COLUMNCLICK:
        if (!bLock && lParam->hwndFrom == ghWndListView)
            ListViewColumnClick(hwnd, (NMLISTVIEW *)lParam);
        break;
    case LVN_GETDISPINFO:
        if (lParam->hwndFrom == ghWndListView)
            ListViewDispInfo(hwnd, (LV_DISPINFO *)lParam);
        break;
    case LVN_ITEMCHANGED:
        if (lParam->hwndFrom == ghWndListView) {
            UpdateStatus(IDS_FIND_DONE_SEL, IDS_FIND_DONE);
        }
        break;
    case LVN_ODFINDITEM:
        if (lParam->hwndFrom == ghWndListView)
            ListViewOdFindItem(hwnd, (LPNMLVFINDITEM)lParam);
        break;
    case TTN_NEEDTEXT:
        ToolBarNeedText(hwnd, (LPTOOLTIPTEXT)lParam);
        break;
    case NM_CLICK:
        if (lParam->hwndFrom == GetStatusBarSysLinkWnd(ghWndStatusBar))
            ShellExecute(NULL, L"open", ((PNMLINK)lParam)->item.szUrl, NULL, NULL, SW_SHOW);
        break;
    }
    return 0;
}

static void Main_OnTimer(HWND hwnd, UINT id)
{
    if (id == ID_TIMER_FIND && gPhotos.iCount > 0) {
        ListView_SetItemCount(ghWndListView, gPhotos.iCount);
        UpdateStatus(IDS_FINDING_SEL, IDS_FINDING);
    }
}

static void Main_OnDestroy(HWND hwnd)
{
    PostQuitMessage(0);
}

static void Main_OnFindDone(HWND hwnd, BOOL b)
{
    KillTimer(hwnd, ID_TIMER_FIND);
    ListView_SetItemCount(ghWndListView, gPhotos.iCount);
    if (b)
        UpdateStatus(IDS_FIND_DONE_SEL, IDS_FIND_DONE);
    else
        UpdateStatus(IDS_FIND_FAILED_SEL, IDS_FIND_FAILED);
    UnLock(hwnd);
}

typedef struct {
    HWND hWnd;
    int columnIndex;
    BOOL isAscending;
} SORT_THREAD_PARAMS;

static void SortThread(PVOID pVoid)
{
    SORT_THREAD_PARAMS *pParams = (SORT_THREAD_PARAMS *)pVoid;
    SortPhotos(pParams->columnIndex, pParams->isAscending);
    SendMessage(pParams->hWnd, WM_SORT_DONE, 0, 0);
    GlobalFree(pVoid);
    _endthread();
}

static void Main_OnSortStart(HWND hwnd, int columnIndex, BOOL isAscending)
{
    if (!(gPhotos.pPs))
        return;
    if (gPhotos.iCount <= 1)
        return;
    Lock(hwnd);
    SORT_THREAD_PARAMS *pParams = (SORT_THREAD_PARAMS *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(SORT_THREAD_PARAMS));
    if (pParams) {
        pParams->hWnd = hwnd;
        pParams->columnIndex = columnIndex;
        pParams->isAscending = isAscending;
        _beginthread(SortThread, 0, pParams);
    } else
        UnLock(hwnd);
}

static void Main_OnSortDone(HWND hwnd)
{
    ListView_RedrawItems(ghWndListView, 0, gPhotos.iCount - 1);
    UnLock(hwnd);
}

static void Lock(HWND hwnd)
{
    bLock = TRUE;
    EnableMenuItem(GetMenu(hwnd), IDM_OPEN, MF_DISABLED);
    ToolBar_EnableButton(ghWndToolBar, IDM_OPEN, FALSE);
}

static void UnLock(HWND hwnd)
{
    bLock = FALSE;
    EnableMenuItem(GetMenu(hwnd), IDM_OPEN, MF_ENABLED);
    ToolBar_EnableButton(ghWndToolBar, IDM_OPEN, TRUE);
}