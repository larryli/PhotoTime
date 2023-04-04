#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <windows.h>
#include <windowsx.h>
#include <stdarg.h>
#include <process.h>

#include <shellapi.h>
#include <Shlobj.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <commdlg.h>
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")

#define __STDC_WANT_LIB_EXT1__ 1
#include <tchar.h>

#include "about.h"
#include "commctrls.h"
#include "listview.h"
#include "toolbar.h"
#include "statusbar.h"
#include "photo.h"
#include "photoview.h"
#include "utils.h"
#include "gdip.h"
#include "export.h"

#include "main.h"

#define WM_SORT_START (WM_USER)
#define WM_SORT_DONE (WM_USER + 1)
#define WM_OPENDIR_DONE (WM_USER + 2)
#define WM_RELOAD_DONE (WM_USER + 3)

#define ID_TIMER_OPENDIR 1
#define ID_TIMER_RELOAD 2
#define ID_TIMER_AUTOPROC 3

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
static void Main_OnDropFiles(HWND, HDROP);

static void Main_OnSortStart(HWND, int, BOOL);
static void Main_OnSortDone(HWND);
static void Main_OnOpenDirDone(HWND, BOOL);
static void Main_OnReloadDone(HWND);

static BOOL ShowPhoto(int);

static void Lock(HWND);
static void UnLock(HWND);

static HANDLE ghInstance;
static HCURSOR ghCurSizeEW, ghCurArrow;
static HWND ghWndToolBar, ghWndListView, ghWndPhotoView, ghWndStatusBar;
static RECT gRcClient;
static BOOL bSplitDrag = FALSE;
static BOOL bLock = FALSE;

#define MIN_CX_LISTVIEW 320
#define MIN_CX_PHOTO 320
static int cxListView = 0;
static int cxPhoto = 0;

static int iTraverseStart = 0;
static int iTraverseEnd = -1;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pszCmdLine, int nCmdShow)
{
    ghInstance = hInstance;
    ghCurSizeEW = LoadCursor(NULL, IDC_SIZEWE);
    ghCurArrow = LoadCursor(NULL, IDC_ARROW);
    if (!hPrevInstance)
        ASSERT_FALSE(InitApplication(hInstance));
    InitCommCtrl();
    InitGdip();
    ASSERT_FALSE(InitInstance(hInstance, nCmdShow));

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
    DeinitGdip();
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
    ASSERT_FALSE(RegisterClassEx(&wc));
    return TRUE;
}

static BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    TCHAR szTitle[MAX_PATH] = L"";
    ASSERT_FALSE(LoadString(hInstance, IDS_APPTITLE, szTitle, NELEMS(szTitle)));
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
    ASSERT_FALSE(hwnd);

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
    HANDLE_MSG(hwnd, WM_DROPFILES, Main_OnDropFiles);
    case WM_SETFOCUS:
        return SetFocus(ghWndListView), 0;
    case WM_SORT_START:
        return Main_OnSortStart(hwnd, (int)wParam, (BOOL)lParam), 0;
    case WM_SORT_DONE:
        return Main_OnSortDone(hwnd), 0;
    case WM_OPENDIR_DONE:
        return Main_OnOpenDirDone(hwnd, (BOOL)wParam), 0;
    case WM_RELOAD_DONE:
        return Main_OnReloadDone(hwnd), 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

static LRESULT Main_OnCreate(HWND hwnd, LPCREATESTRUCT lParam)
{
    ghWndToolBar = CreateToolBarWnd(hwnd, ghInstance);
    ASSERT_FALSE(ghWndToolBar);
    TCHAR szReady[MAX_PATH] = L"", szUrl[MAX_PATH] = L"";
    ASSERT_FALSE(LoadString(ghInstance, IDS_READY, szReady, NELEMS(szReady)));
    ASSERT_FALSE(LoadString(ghInstance, IDS_LINK, szUrl, NELEMS(szUrl)));
    ghWndStatusBar = CreateStatusBarWnd(hwnd, ghInstance, szReady, szUrl);
    ASSERT_FALSE(ghWndStatusBar);
    ghWndListView = CreateListViewWnd(hwnd, ghInstance);
    ASSERT_FALSE(ghWndListView);
    ghWndPhotoView = CreatePhotoViewWnd(hwnd, ghInstance);
    ASSERT_FALSE(ghWndPhotoView);
    SetFocus(ghWndListView);
    DragAcceptFiles(hwnd, TRUE);
    return TRUE;
}

static BOOL GetRect(HWND hwnd, RECT *p)
{
    RECT rc;
    ASSERT_FALSE(GetClientRect(hwnd, p));
    ASSERT_FALSE(GetWindowRect(ghWndStatusBar, &rc));
    ASSERT_FALSE(ScreenToClient(hwnd, (LPPOINT)&rc.left));
    p->bottom = rc.top;
    ASSERT_FALSE(GetWindowRect(ghWndToolBar, &rc));
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
        MoveWindow(ghWndPhotoView, gRcClient.left + cxListView + 2, gRcClient.top, cxPhoto, cy, TRUE);
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
            MoveWindow(ghWndPhotoView, gRcClient.left + cxListView + 2, gRcClient.top, cxPhoto, cy, TRUE);
        }
    }
}

typedef struct {
    HWND hWnd;
    TCHAR szPath[MAX_PATH];
} OPENDIR_THREAD_PARAMS;

static void __cdecl OpenDirThread(PVOID pVoid)
{
    OPENDIR_THREAD_PARAMS *pParams = (OPENDIR_THREAD_PARAMS *)pVoid;
    WPARAM wParam = (WPARAM)FindPhotos(pParams->szPath);
    SendMessage(pParams->hWnd, WM_OPENDIR_DONE, wParam, 0);
    GlobalFree(pVoid);
    _endthread();
}

static void OpenDir(HWND hwnd, LPTSTR szPath)
{
    TCHAR szTitle[MAX_PATH];
    if (LoadString(ghInstance, IDS_APPTITLE_FMT, szTitle, NELEMS(szTitle))) {
        TCHAR szBuf[MAX_PATH * 2] = {0};
        swprintf(szBuf, NELEMS(szBuf), szTitle, szPath);
        SetWindowText(hwnd, szBuf);
    }

    OPENDIR_THREAD_PARAMS *pParams = (OPENDIR_THREAD_PARAMS *)GlobalAlloc(
        GMEM_FIXED | GMEM_ZEROINIT, sizeof(OPENDIR_THREAD_PARAMS));
    ASSERT_VOID(pParams);
    Lock(hwnd);
    pParams->hWnd = hwnd;
    (void)_tcscpy_s(pParams->szPath, NELEMS(pParams->szPath), szPath);
    ShowPhoto(-1);
    ListView_DeleteAllItems(ghWndListView);
    ListViewCleanSort(ghWndListView);
    _beginthread(OpenDirThread, 0, pParams);
    TCHAR szBuf[MAX_PATH];
    if (LoadString(ghInstance, IDS_OPENDIR_START, szBuf, NELEMS(szBuf)))
        SetStatusBarText(ghWndStatusBar, 0, szBuf);
#ifndef TIMER_OPENDIR_ELAPSE
#define TIMER_OPENDIR_ELAPSE 200
#endif
    SetTimer(hwnd, ID_TIMER_OPENDIR, TIMER_OPENDIR_ELAPSE, NULL);
}

static void SelectDir(HWND hwnd)
{
    TCHAR szTitle[MAX_PATH];
    ASSERT_VOID(LoadString(ghInstance, IDS_SELECT_PHOTO_DIRECTORY, szTitle, NELEMS(szTitle)));
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr)) {
        IFileOpenDialog *pFileOpen;
        hr = CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_ALL, &IID_IFileOpenDialog,
                              (void **)(&pFileOpen));
        if (SUCCEEDED(hr)) {
            hr = IFileDialog_SetTitle(pFileOpen, szTitle);
            ASSERT_END(SUCCEEDED(hr));
            hr = IFileOpenDialog_SetOptions(pFileOpen, FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
            ASSERT_END(SUCCEEDED(hr));
            hr = IFileDialog_Show(pFileOpen, NULL);
            if (SUCCEEDED(hr)) {
                IShellItem *pItem;
                hr = IFileDialog_GetResult(pFileOpen, &pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszPath;
                    hr = IShellItem_GetDisplayName(pItem, SIGDN_FILESYSPATH, &pszPath);
                    if (SUCCEEDED(hr)) {
                        OpenDir(hwnd, pszPath);
                        CoTaskMemFree(pszPath);
                    }
                    IShellItem_Release(pItem);
                }
            }
end:
            IFileDialog_Release(pFileOpen);
            CoUninitialize();
            return; // new open folder dialog
        }
        CoUninitialize();
    }
    BROWSEINFO bInfo = {
        .hwndOwner = hwnd,
        .lpszTitle = szTitle,
        .ulFlags = BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_USENEWUI | BIF_NONEWFOLDERBUTTON,
    };
    LPITEMIDLIST lpDlist = SHBrowseForFolder(&bInfo);
    ASSERT_VOID(lpDlist);
    TCHAR szPath[MAX_PATH] = {0};
    ASSERT_VOID(SHGetPathFromIDList(lpDlist, szPath));
    OpenDir(hwnd, szPath);
}

static void UpdateStatus(UINT uId, ...)
{
    TCHAR szFmt[MAX_PATH];
    ASSERT_VOID(LoadString(ghInstance, uId, szFmt, NELEMS(szFmt)));
    TCHAR szBuf[MAX_PATH];
    va_list ap;
    va_start(ap, uId);
    vswprintf(szBuf, NELEMS(szBuf), szFmt, ap);
    va_end(ap);
    SetStatusBarText(ghWndStatusBar, 0, szBuf);
}

static void UpdateStatusDone(void)
{
    int iSelected = ListView_GetSelectedCount(ghWndListView);
    if (iSelected)
        UpdateStatus(IDS_SELECTED, gPhotoLib.iCount, iSelected);
    else 
        UpdateStatus(IDS_DONE, gPhotoLib.iCount);
}

typedef struct {
    HWND hWnd;
    int *done;
    PVOID pVoid;
} TRAVERSE_THREAD_PARAMS;

static void __cdecl ReloadThread(PVOID pVoid)
{
    TRAVERSE_THREAD_PARAMS *pParams = (TRAVERSE_THREAD_PARAMS *)pVoid;
    ReloadPhotos(pParams->done);
    SendMessage(pParams->hWnd, WM_RELOAD_DONE, 0, 0);
    GlobalFree(pVoid);
    _endthread();
}

static void __cdecl AutoProcThread(PVOID pVoid)
{
    TRAVERSE_THREAD_PARAMS *pParams = (TRAVERSE_THREAD_PARAMS *)pVoid;
#pragma warn(push)
#pragma warn(disable: 2215)
    AutoProcPhotos(pParams->done, (AUTOPROCTYPE)(pParams->pVoid));
#pragma warn(pop)
    SendMessage(pParams->hWnd, WM_RELOAD_DONE, 0, 0);
    GlobalFree(pVoid);
    _endthread();
}

static void Traverse(HWND hwnd, void __cdecl (*thread)(PVOID), PVOID pVoid, UINT uId, UINT_PTR uTimer, UINT uElapse)
{
    ASSERT_VOID(gPhotoLib.iCount > 0);
    ASSERT_VOID(gPhotoLib.pPhotos);
    TRAVERSE_THREAD_PARAMS *pParams = (TRAVERSE_THREAD_PARAMS *)GlobalAlloc(
        GMEM_FIXED | GMEM_ZEROINIT, sizeof(TRAVERSE_THREAD_PARAMS));
    ASSERT_VOID(pParams);
    Lock(hwnd);
    pParams->hWnd = hwnd;
    pParams->done = &iTraverseEnd;
    pParams->pVoid = pVoid;
    iTraverseStart = 0;
    iTraverseEnd = -1;
    _beginthread(thread, 0, pParams);
    UpdateStatus(uId, gPhotoLib.iCount);
    SetTimer(hwnd, uTimer, uElapse, NULL);
}

static void Reload(HWND hwnd)
{
#ifndef TIMER_RELOAD_ELAPSE
#define TIMER_RELOAD_ELAPSE 300
#endif
    Traverse(hwnd, ReloadThread, NULL, IDS_RELOAD_START, ID_TIMER_RELOAD, TIMER_RELOAD_ELAPSE);
}

static void AutoProc(HWND hwnd, AUTOPROCTYPE type)
{
#ifndef TIMER_AUTOPROC_ELAPSE
#define TIMER_AUTOPROC_ELAPSE 300
#endif
    Traverse(hwnd, AutoProcThread, (PVOID)type, IDS_AUTOPROC_START, ID_TIMER_AUTOPROC, TIMER_AUTOPROC_ELAPSE);
}

static BOOL GetPhotoPath(PTSTR szPath, int size, int idx)
{
    ASSERT_FALSE(idx >= 0 && idx < gPhotoLib.iCount);
    ASSERT_FALSE(gPhotoLib.pPhotos);
    PHOTO *pPhoto = gPhotoLib.pPhotos[idx];
    ASSERT_FALSE(pPhoto);
    CatFilePath(szPath, size, gPhotoLib.szPath, pPhoto->szSubPath);
    CatFilePath(szPath, size, szPath, pPhoto->szFilename);
    return TRUE;
}

static void ShellOpen(HWND hwnd)
{
    TCHAR szPath[MAX_PATH] = L"";
    ASSERT_VOID(GetPhotoPath(szPath, NELEMS(szPath), ListView_GetSelectionMark(ghWndListView)));
    ShellExecute(hwnd, NULL, szPath, NULL, NULL, SW_SHOW);
}

static void ShellFolder(HWND hwnd)
{
    TCHAR szPath[MAX_PATH] = L"";
    ASSERT_VOID(GetPhotoPath(szPath, NELEMS(szPath), ListView_GetSelectionMark(ghWndListView)));
    PCIDLIST_ABSOLUTE pidlFolder = ILCreateFromPath(szPath);
    ASSERT_VOID(pidlFolder);
    SHOpenFolderAndSelectItems(pidlFolder, 0, NULL, 0);
    ILFree((LPITEMIDLIST)pidlFolder);
}

static void ShellProperties(HWND hwnd)
{
    TCHAR szPath[MAX_PATH] = L"";
    ASSERT_VOID(GetPhotoPath(szPath, NELEMS(szPath), ListView_GetSelectionMark(ghWndListView)));
    SHELLEXECUTEINFO shi = {
        .cbSize = sizeof(SHELLEXECUTEINFO),
        .fMask = SEE_MASK_INVOKEIDLIST,
        .hwnd = hwnd,
        .lpVerb = L"properties",
        .lpFile = szPath,
        .lpParameters = L"",
        .lpDirectory  = NULL,
        .nShow = SW_SHOW,
        .hInstApp = NULL,
    };
    ShellExecuteEx(&shi);
}

static void CopyToClip(HWND hwnd, const PTSTR szBuf)
{
    ASSERT_VOID(OpenClipboard(hwnd));
    int size = (lstrlen(szBuf) + 1) * sizeof(TCHAR);
    HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, size);
    ASSERT_END(h);
    PTSTR p = GlobalLock(h);
    memcpy(p, szBuf, size);
    GlobalUnlock(h);
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, h); 
end:
    CloseClipboard();
}

static void CopyPathToClip(HWND hwnd)
{
    TCHAR szPath[MAX_PATH] = L"";
    ASSERT_VOID(GetPhotoPath(szPath, NELEMS(szPath), ListView_GetSelectionMark(ghWndListView)));
    CopyToClip(hwnd, szPath);
}

static void CopyTsvToClip(HWND hwnd)
{
    TCHAR szBuf[MAX_PATH * 3] = L"";
    int size = NELEMS(szBuf);
    int idx = ListView_GetSelectionMark(ghWndListView);
    for (int i = 0; i < LV_ROWS; i++) {
        TCHAR szItemBuf[MAX_PATH] = L"";
        ListView_GetItemText(ghWndListView, idx, i, szItemBuf, NELEMS(szItemBuf));
        if (i) {
            (void)_tcscat_s(szBuf, size, L"\t");
            (void)_tcscat_s(szBuf, size, szItemBuf);
        } else
            (void)_tcscpy_s(szBuf, size, szItemBuf);
    }
    CopyToClip(hwnd, szBuf);
}

static BOOL GetSavePath(HWND hwnd, int uID, PTSTR szPath, int size, PCTSTR szExt)
{
    TCHAR szBuf[MAX_PATH];
    ASSERT_FALSE(LoadString(ghInstance, uID, szBuf, NELEMS(szBuf)));
    OPENFILENAME ofn = {
        .lStructSize = sizeof(OPENFILENAME),
        .hwndOwner = hwnd,
        .lpstrFile = szPath,
        .nMaxFile = size,
        .lpstrFilter = szBuf,
        .Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_LONGNAMES,
    };
    ASSERT_FALSE(GetSaveFileName(&ofn));
    if (!GetFileExt(szPath))
        (void)_tcscat_s(szPath, size, szExt);
    return TRUE;
}

static void ExportToTsv(HWND hwnd)
{
    TCHAR szBuf[MAX_PATH] = L"";
    ASSERT_VOID(GetSavePath(hwnd, IDS_TSV_FILE, szBuf, NELEMS(szBuf), L".tsv"));
    Lock(hwnd);
    UpdateStatus(IDS_EXPORT_START);
    BOOL bRet = ExportToTsvFile(ghWndListView, szBuf);
    UpdateStatusDone();
    UnLock(hwnd);
    if (bRet)
        return;
    ASSERT_VOID(LoadString(ghInstance, IDS_SAVE_FILE_FAILED, szBuf, NELEMS(szBuf)));
    MessageBox(hwnd, szBuf, NULL, MB_OK | MB_ICONERROR);
}

static void ExportToHtml(HWND hwnd)
{
    TCHAR szTitle[MAX_PATH], szBuf[MAX_PATH] = L"";
    GetWindowText(hwnd, szTitle, NELEMS(szTitle));
    ASSERT_VOID(GetSavePath(hwnd, IDS_HTML_FILE, szBuf, NELEMS(szBuf), L".html"));
    Lock(hwnd);
    UpdateStatus(IDS_EXPORT_START);
    BOOL bRet = ExportToHtmlFile(ghWndListView, szBuf, szTitle);
    UpdateStatusDone();
    UnLock(hwnd);
    if (bRet)
        return;
    ASSERT_VOID(LoadString(ghInstance, IDS_SAVE_FILE_FAILED, szBuf, NELEMS(szBuf)));
    MessageBox(hwnd, szBuf, NULL, MB_OK | MB_ICONERROR);
}

#define HANDLE_ID(id, sen) case (id): sen; return

static void Main_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id) {
    HANDLE_ID(IDM_EXIT, PostMessage(hwnd, WM_CLOSE, 0, 0L));
    HANDLE_ID(IDM_ABOUT, DialogBox(ghInstance, MAKEINTRESOURCE(DLG_ABOUT), hwnd, (DLGPROC)AboutDlgProc));
    HANDLE_ID(IDM_OPEN, SelectDir(hwnd));
    HANDLE_ID(IDM_RELOAD, Reload(hwnd));
    HANDLE_ID(IDM_EXPORT_TSV, ExportToTsv(hwnd));
    HANDLE_ID(IDM_EXPORT_HTML, ExportToHtml(hwnd));
    HANDLE_ID(IDM_AUTOPROC, AutoProc(hwnd, AUTOPROC_ALL));
    HANDLE_ID(IDM_AUTOPROC_FILE, AutoProc(hwnd, AUTOPROC_FILE));
    HANDLE_ID(IDM_AUTOPROC_EXIF, AutoProc(hwnd, AUTOPROC_EXIF));
    HANDLE_ID(IDM_ITEM_OPEN, ShellOpen(hwnd));
    HANDLE_ID(IDM_ITEM_FOLDER, ShellFolder(hwnd));
    HANDLE_ID(IDM_ITEM_PROPERTIES, ShellProperties(hwnd));
    HANDLE_ID(IDM_ITEM_COPYPATH, CopyPathToClip(hwnd));
    HANDLE_ID(IDM_ITEM_COPYTSV, CopyTsvToClip(hwnd));
    }
}

static void Main_OnContextMenu(HWND hwnd, HWND hwndContext, UINT xPos, UINT yPos)
{
    if (hwndContext == ghWndListView && ListView_GetSelectedCount(ghWndListView)) {
        HMENU hMenuLoad = LoadMenu(ghInstance, MAKEINTRESOURCE(IDR_MNU_CONTEXT));
        HMENU hMenu = GetSubMenu(hMenuLoad, 0);
        TrackPopupMenu(hMenu,
                       TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                       xPos, yPos, 0, hwnd, NULL);
        DestroyMenu(hMenuLoad);
    }
}

static BOOL ShowPhoto(int idx)
{
    ASSERT_FAILED(idx >= 0 && idx < gPhotoLib.iCount);
    ASSERT_FAILED(gPhotoLib.pPhotos);
    PHOTO *pPhoto = gPhotoLib.pPhotos[idx];
    ASSERT_FAILED(pPhoto);
    TCHAR szPath[MAX_PATH] = L"";
    CatFilePath(szPath, NELEMS(szPath), gPhotoLib.szPath, pPhoto->szSubPath);
    CatFilePath(szPath, NELEMS(szPath), szPath, pPhoto->szFilename);
    BOOL bRet = PhotoView_SetPath(ghWndPhotoView, szPath);
    if (pPhoto->szSubPath)
        CatFilePath(szPath, NELEMS(szPath), pPhoto->szSubPath, pPhoto->szFilename);
    else
        (void)_tcscpy_s(szPath, NELEMS(szPath), pPhoto->szFilename);
    SIZE size;
    if (bRet && PhotoView_GetSize(ghWndPhotoView, &size)) {
        TCHAR szBuf[MAX_PATH] = L"";
        swprintf(szBuf, NELEMS(szBuf), L"%ls  %d x %d", szPath, size.cx, size.cy);
        SetStatusBarText(ghWndStatusBar, 1, szBuf);
    } else
        SetStatusBarText(ghWndStatusBar, 1, szPath);
    return bRet;
failed:
    bRet = PhotoView_SetPath(ghWndPhotoView, NULL);
    SetStatusBarText(ghWndStatusBar, 1, L""); // clean
    return bRet;
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
#define lpNmLv ((LPNM_LISTVIEW)lParam)
        if (lParam->hwndFrom == ghWndListView) {
            if (lpNmLv->uChanged == LVIF_STATE) {
                UpdateStatusDone();
                if (lpNmLv->uNewState & (LVIS_FOCUSED | LVIS_SELECTED))
                    ShowPhoto(lpNmLv->iItem);
                else if (lpNmLv->uNewState == 0)
                    ShowPhoto(-1); // not selected
            }
        }
        break;
    case NM_CUSTOMDRAW:
        return ListViewCustomDraw(hwnd, (LPNMLVCUSTOMDRAW)lParam);
    case TBN_DROPDOWN:
#define lpNmTb ((LPNMTOOLBAR)lParam)
        if (lpNmTb->iItem == IDM_AUTOPROC) {
            RECT rc;
            SendMessage(lpNmTb->hdr.hwndFrom, TB_GETRECT, (WPARAM)lpNmTb->iItem, (LPARAM)&rc);
            MapWindowPoints(lpNmTb->hdr.hwndFrom, HWND_DESKTOP, (LPPOINT)&rc, 2);
            HMENU hMenuLoaded = LoadMenu(ghInstance, MAKEINTRESOURCE(IDR_MNU_AUTOPROC)); 
            HMENU hPopupMenu = GetSubMenu(hMenuLoaded, 0);
            TPMPARAMS tpm = {
                .cbSize    = sizeof(TPMPARAMS),
                .rcExclude = rc,
            };
            TrackPopupMenuEx(hPopupMenu, 
                            TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL, 
                             rc.left, rc.bottom, hwnd, &tpm);
            DestroyMenu(hMenuLoaded);
            return TBDDRET_DEFAULT;
        }
        return TBDDRET_NODEFAULT;
    case TTN_NEEDTEXT:
        ToolBarNeedText(hwnd, (LPTOOLTIPTEXT)lParam);
        break;
    case NM_CLICK:
        if (lParam->hwndFrom == GetStatusBarSysLinkWnd(ghWndStatusBar))
            ShellExecute(NULL, L"open", ((PNMLINK)lParam)->item.szUrl, NULL, NULL, SW_SHOW);
        break;
    case NM_DBLCLK:
        if (lParam->hwndFrom == ghWndListView)
            ShellOpen(hwnd);
        break;
    }
    return 0;
}

static void Main_OnTimer(HWND hwnd, UINT id)
{
    ASSERT_VOID(gPhotoLib.iCount > 0);
    if (id == ID_TIMER_OPENDIR) {
        ListView_SetItemCount(ghWndListView, gPhotoLib.iCount);
        UpdateStatus(IDS_OPENDIR_RUN, gPhotoLib.iCount);
    } else if ((id == ID_TIMER_RELOAD || ID_TIMER_AUTOPROC) && iTraverseEnd >= iTraverseStart) {
        ListView_RedrawItems(ghWndListView, iTraverseStart, iTraverseEnd);
        iTraverseStart = iTraverseEnd + 1; // (1, 0) do not redraw
        UpdateStatus(id == ID_TIMER_RELOAD ? IDS_RELOAD_RUN : IDS_AUTOPROC_RUN,
            gPhotoLib.iCount, iTraverseStart);
    }
}

static void Main_OnDestroy(HWND hwnd)
{
    DragAcceptFiles(hwnd, FALSE);
    DestroyPhotoViewWnd(ghWndPhotoView);
    PostQuitMessage(0);
}

static void Main_OnDropFiles(HWND hwnd, HDROP hdrop)
{
    int nDrops = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
    if (nDrops > 0) {
        TCHAR szPath[MAX_PATH];
        for (int i = 0; i < nDrops; i++) {
            if (DragQueryFile(hdrop, i, szPath, MAX_PATH)) {
                if (PathIsDirectory(szPath)) {
                    OpenDir(hwnd, szPath);
                    break;
                }
            }
        }
    }
    DragFinish(hdrop);
}

typedef struct {
    HWND hWnd;
    int columnIndex;
    BOOL isAscending;
} SORT_THREAD_PARAMS;

static void __cdecl SortThread(PVOID pVoid)
{
    SORT_THREAD_PARAMS *pParams = (SORT_THREAD_PARAMS *)pVoid;
    SortPhotos(pParams->columnIndex, pParams->isAscending);
    SendMessage(pParams->hWnd, WM_SORT_DONE, 0, 0);
    GlobalFree(pVoid);
    _endthread();
}

static void Main_OnSortStart(HWND hwnd, int columnIndex, BOOL isAscending)
{
    ASSERT_VOID(gPhotoLib.iCount > 0);
    ASSERT_VOID(gPhotoLib.pPhotos);
    SORT_THREAD_PARAMS *pParams = (SORT_THREAD_PARAMS *)GlobalAlloc(
        GMEM_FIXED | GMEM_ZEROINIT, sizeof(SORT_THREAD_PARAMS));
    ASSERT_VOID(pParams);
    Lock(hwnd);
    pParams->hWnd = hwnd;
    pParams->columnIndex = columnIndex;
    pParams->isAscending = isAscending;
    _beginthread(SortThread, 0, pParams);
    UpdateStatus(IDS_SORT_START, gPhotoLib.iCount);
}

static void Main_OnSortDone(HWND hwnd)
{
    ListView_RedrawItems(ghWndListView, 0, gPhotoLib.iCount - 1);
    UpdateStatusDone();
    UnLock(hwnd);
}

static void Main_OnOpenDirDone(HWND hwnd, BOOL b)
{
    KillTimer(hwnd, ID_TIMER_OPENDIR);
    ListView_SetItemCount(ghWndListView, gPhotoLib.iCount);
    if (b)
        UpdateStatusDone();
    else
        UpdateStatus(IDS_OPENDIR_FAILED, gPhotoLib.iCount);
    UnLock(hwnd);
}

static void Main_OnReloadDone(HWND hwnd)
{
    if (iTraverseEnd >= iTraverseStart)
        ListView_RedrawItems(ghWndListView, iTraverseStart, iTraverseEnd);
    iTraverseStart = 0;
    iTraverseEnd = -1;
    UpdateStatusDone();
    UnLock(hwnd);
}

static void Lock(HWND hwnd)
{
    bLock = TRUE;
    EnableMenuItem(GetMenu(hwnd), IDM_OPEN, MF_DISABLED);
    EnableMenuItem(GetMenu(hwnd), IDM_RELOAD, MF_DISABLED);
    EnableMenuItem(GetMenu(hwnd), IDM_AUTOPROC, MF_DISABLED);
    EnableMenuItem(GetMenu(hwnd), IDM_AUTOPROC_SPEC, MF_DISABLED);
    EnableMenuItem(GetMenu(hwnd), IDM_EXPORT, MF_DISABLED);
    ToolBar_EnableButton(ghWndToolBar, IDM_OPEN, FALSE);
    ToolBar_EnableButton(ghWndToolBar, IDM_RELOAD, FALSE);
    ToolBar_EnableButton(ghWndToolBar, IDM_AUTOPROC, FALSE);
    DragAcceptFiles(hwnd, FALSE);
}

static void UnLock(HWND hwnd)
{
    bLock = FALSE;
    DragAcceptFiles(hwnd, TRUE);
    EnableMenuItem(GetMenu(hwnd), IDM_OPEN, MF_ENABLED);
    ToolBar_EnableButton(ghWndToolBar, IDM_OPEN, TRUE);
    if (gPhotoLib.iCount > 0 && gPhotoLib.pPhotos) {
        EnableMenuItem(GetMenu(hwnd), IDM_RELOAD, MF_ENABLED);
        EnableMenuItem(GetMenu(hwnd), IDM_AUTOPROC, MF_ENABLED);
        EnableMenuItem(GetMenu(hwnd), IDM_AUTOPROC_SPEC, MF_ENABLED);
        EnableMenuItem(GetMenu(hwnd), IDM_EXPORT, MF_ENABLED);
        ToolBar_EnableButton(ghWndToolBar, IDM_RELOAD, TRUE);
        ToolBar_EnableButton(ghWndToolBar, IDM_AUTOPROC, TRUE);
    }
}
