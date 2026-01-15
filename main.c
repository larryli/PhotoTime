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

#ifndef HANDLE_WM_DROPFILES
#define HANDLE_WM_DROPFILES(hwnd,wParam,lParam,fn)  ((fn)((hwnd),(HDROP)(wParam)),0)
#endif
#ifndef ToolBar_EnableButton
#define ToolBar_EnableButton(hwnd,idButton,fEnable)  (BOOL)SNDMSG((hwnd),TB_ENABLEBUTTON,(WPARAM)(idButton),(LPARAM)MAKELONG(fEnable,0))
#endif

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
static void Main_OnTimer(HWND, UINT_PTR);
static void Main_OnDropFiles(HWND, HDROP);

static void Main_OnSortStart(HWND, int, BOOL);
static void Main_OnSortDone(HWND);
static void Main_OnOpenDirDone(HWND, BOOL);
static void Main_OnReloadDone(HWND);

static BOOL ShowPhoto(int);

static void Lock(HWND);
static void UnLock(HWND);

// Global application instance handle
static HANDLE ghInstance;

// Cursor handles for different mouse interactions
static HCURSOR ghCurSizeEW, ghCurArrow;

// Window handles for main UI components
static HWND ghWndToolBar, ghWndListView, ghWndPhotoView, ghWndStatusBar;

// Rectangle storing the client area dimensions
static RECT gRcClient;

// Flags for UI interaction states
static BOOL bSplitDrag = FALSE;  // TRUE when dragging the splitter between list and photo views
static BOOL bLock = FALSE;       // TRUE when UI is locked during operations

// Minimum widths for UI components
#define MIN_CX_LISTVIEW 320
#define MIN_CX_PHOTO 320

// Current widths of UI components
static int cxListView = 0;  // Current width of the list view
static int cxPhoto = 0;     // Current width of the photo view

// Variables for tracking progress of background operations
static int iTraverseStart = 0;  // Starting index for current operation
static int iTraverseEnd = -1;   // Ending index for current operation (-1 indicates not started)

/**
 * @brief Entry point for the application
 *
 * This function serves as the entry point for the Windows application. It initializes
 * the application, registers window classes, creates the main window, and enters
 * the message loop.
 *
 * @param hInstance Handle to the current instance of the application
 * @param hPrevInstance Handle to the previous instance of the application (always NULL)
 * @param pszCmdLine Command line arguments as a string
 * @param nCmdShow Specifies how the window should be shown
 * @return The exit code of the application
 */
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
    return (int)msg.wParam;
}

/**
 * @brief Initialize the application window class
 *
 * This function registers the main window class with the Windows operating system.
 *
 * @param hInstance Handle to the application instance
 * @return TRUE if the class was registered successfully, FALSE otherwise
 */
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

/**
 * @brief Initialize the application instance
 *
 * This function creates the main application window and shows it.
 *
 * @param hInstance Handle to the application instance
 * @param nCmdShow Specifies how the window should be shown
 * @return TRUE if the window was created successfully, FALSE otherwise
 */
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

/**
 * @brief Main window procedure
 *
 * This function handles messages sent to the main application window.
 *
 * @param hwnd Handle to the window
 * @param msg Specifies the message
 * @param wParam Additional message-specific information
 * @param lParam Additional message-specific information
 * @return The return value depends on the message
 */
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

/**
 * @brief Handle the WM_CREATE message
 *
 * This function handles the WM_CREATE message by creating the main UI controls:
 * toolbar, status bar, list view, and photo view windows.
 *
 * @param hwnd Handle to the window
 * @param lParam Pointer to a CREATESTRUCT structure that contains information about the window being created
 * @return TRUE if successful, FALSE otherwise
 */
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

/**
 * @brief Get the rectangle coordinates for the main window client area
 *
 * This function calculates and returns the rectangle coordinates for the main window's
 * client area, taking into account the positions of the status bar and toolbar.
 *
 * @param hwnd Handle to the window
 * @param p Pointer to a RECT structure that receives the coordinates
 * @return TRUE if successful, FALSE otherwise
 */
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

/**
 * @brief Handle the WM_SIZE message
 *
 * This function handles the WM_SIZE message by resizing the child windows appropriately
 * when the main window is resized.
 *
 * @param hwnd Handle to the window
 * @param flag Specifies the type of resizing requested
 * @param x New width of the client area
 * @param y New height of the client area
 * @return Always returns 0
 */
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

/**
 * @brief Handle the WM_LBUTTONDOWN message
 *
 * This function handles the WM_LBUTTONDOWN message by capturing mouse input
 * and setting the split drag flag when the left mouse button is pressed.
 *
 * @param hwnd Handle to the window
 * @param fDoubleClick TRUE if this is a double-click message, FALSE otherwise
 * @param x X-coordinate of the cursor position
 * @param y Y-coordinate of the cursor position
 * @param keyFlags Indicates whether various virtual keys are down
 */
static void Main_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    SetCapture(hwnd);
    bSplitDrag = TRUE;
}

/**
 * @brief Handle the WM_LBUTTONUP message
 *
 * This function handles the WM_LBUTTONUP message by releasing mouse capture
 * and resetting the split drag flag when the left mouse button is released.
 *
 * @param hwnd Handle to the window
 * @param x X-coordinate of the cursor position
 * @param y Y-coordinate of the cursor position
 * @param keyFlags Indicates whether various virtual keys are down
 */
static void Main_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    ReleaseCapture();
    bSplitDrag = FALSE;
}

/**
 * @brief Handle the WM_MOUSEMOVE message
 *
 * This function handles the WM_MOUSEMOVE message by updating the cursor based on position
 * and resizing the list view and photo view windows when dragging the splitter bar.
 *
 * @param hwnd Handle to the window
 * @param x X-coordinate of the cursor position
 * @param y Y-coordinate of the cursor position
 * @param keyFlags Indicates whether various virtual keys are down
 */
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

/**
 * @brief Thread function for opening a directory
 *
 * This function runs in a separate thread to find photos in a specified directory
 * and sends a message when the operation is complete.
 *
 * @param pVoid Pointer to OPENDIR_THREAD_PARAMS structure containing the window handle and path
 */
static void __cdecl OpenDirThread(PVOID pVoid)
{
    OPENDIR_THREAD_PARAMS *pParams = (OPENDIR_THREAD_PARAMS *)pVoid;
    WPARAM wParam = (WPARAM)FindPhotos(pParams->szPath);
    SendMessage(pParams->hWnd, WM_OPENDIR_DONE, wParam, 0);
    GlobalFree(pVoid);
    _endthread();
}

/**
 * @brief Open a directory and load photos
 *
 * This function opens a specified directory, updates the window title, and starts a thread
 * to find and load photos from the directory.
 *
 * @param hwnd Handle to the main window
 * @param szPath Path to the directory to open
 */
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

/**
 * @brief Open a directory selection dialog
 *
 * This function displays a directory selection dialog allowing the user to choose
 * a directory containing photos to open in the application.
 *
 * @param hwnd Handle to the parent window
 */
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

/**
 * @brief Update the status bar text with formatted message
 *
 * This function loads a string resource, formats it with variable arguments,
 * and updates the status bar text with the resulting message.
 *
 * @param uId Resource ID of the string format
 * @param ... Variable arguments for string formatting
 */
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

/**
 * @brief Update the status bar with the count of photos
 *
 * This function updates the status bar text to show either the total number of photos
 * or the number of selected photos, depending on the current selection state.
 */
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

/**
 * @brief Thread function for reloading photos
 *
 * This function runs in a separate thread to reload photos and sends a message
 * when the operation is complete.
 *
 * @param pVoid Pointer to TRAVERSE_THREAD_PARAMS structure containing the window handle and done flag
 */
static void __cdecl ReloadThread(PVOID pVoid)
{
    TRAVERSE_THREAD_PARAMS *pParams = (TRAVERSE_THREAD_PARAMS *)pVoid;
    ReloadPhotos(pParams->done);
    SendMessage(pParams->hWnd, WM_RELOAD_DONE, 0, 0);
    GlobalFree(pVoid);
    _endthread();
}

/**
 * @brief Thread function for automatic photo processing
 *
 * This function runs in a separate thread to automatically process photos and sends a message
 * when the operation is complete.
 *
 * @param pVoid Pointer to TRAVERSE_THREAD_PARAMS structure containing the window handle and processing type
 */
static void __cdecl AutoProcThread(PVOID pVoid)
{
    TRAVERSE_THREAD_PARAMS *pParams = (TRAVERSE_THREAD_PARAMS *)pVoid;
#ifdef __POCC__
#pragma warn(push)
#pragma warn(disable: 2215)
#endif
    AutoProcPhotos(pParams->done, (AUTOPROCTYPE)(pParams->pVoid));
#ifdef __POCC__
#pragma warn(pop)
#endif
    SendMessage(pParams->hWnd, WM_RELOAD_DONE, 0, 0);
    GlobalFree(pVoid);
    _endthread();
}

/**
 * @brief Generic function to traverse photos with a specified thread
 *
 * This function allocates parameters, locks the UI, starts a traversal thread,
 * and sets up a timer to update the UI during the traversal operation.
 *
 * @param hwnd Handle to the window
 * @param thread Pointer to the thread function to execute
 * @param pVoid Parameter to pass to the thread function
 * @param uId Resource ID of the status message
 * @param uTimer Timer identifier
 * @param uElapse Timer interval in milliseconds
 */
static void Traverse(HWND hwnd, void (__cdecl *thread)(PVOID), PVOID pVoid, UINT uId, UINT_PTR uTimer, UINT uElapse)
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

/**
 * @brief Reload the photo library
 *
 * This function initiates a reload operation for the photo library by calling the
 * Traverse function with the ReloadThread.
 *
 * @param hwnd Handle to the main window
 */
static void Reload(HWND hwnd)
{
#ifndef TIMER_RELOAD_ELAPSE
#define TIMER_RELOAD_ELAPSE 300
#endif
    Traverse(hwnd, ReloadThread, NULL, IDS_RELOAD_START, ID_TIMER_RELOAD, TIMER_RELOAD_ELAPSE);
}

/**
 * @brief Automatically process photos with a specified processing type
 *
 * This function initiates an automatic photo processing operation by calling the
 * Traverse function with the AutoProcThread and specified processing type.
 *
 * @param hwnd Handle to the main window
 * @param type Type of automatic processing to perform
 */
static void AutoProc(HWND hwnd, AUTOPROCTYPE type)
{
#ifndef TIMER_AUTOPROC_ELAPSE
#define TIMER_AUTOPROC_ELAPSE 300
#endif
    Traverse(hwnd, AutoProcThread, (PVOID)type, IDS_AUTOPROC_START, ID_TIMER_AUTOPROC, TIMER_AUTOPROC_ELAPSE);
}

/**
 * @brief Get the full path of a photo by index
 *
 * This function constructs the full path of a photo by combining the library path,
 * sub-path, and filename based on the provided index.
 *
 * @param szPath Buffer to receive the constructed path
 * @param size Size of the buffer
 * @param idx Index of the photo in the library
 * @return TRUE if successful, FALSE otherwise
 */
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

/**
 * @brief Open the selected photo with the default application
 *
 * This function gets the path of the selected photo and opens it using the default
 * application associated with its file type.
 *
 * @param hwnd Handle to the parent window
 */
static void ShellOpen(HWND hwnd)
{
    TCHAR szPath[MAX_PATH] = L"";
    ASSERT_VOID(GetPhotoPath(szPath, NELEMS(szPath), ListView_GetSelectionMark(ghWndListView)));
    ShellExecute(hwnd, NULL, szPath, NULL, NULL, SW_SHOW);
}

/**
 * @brief Open the folder containing the selected photo and highlight the file
 *
 * This function gets the path of the selected photo and opens its containing folder
 * with the file highlighted in Windows Explorer.
 *
 * @param hwnd Handle to the parent window
 */
static void ShellFolder(HWND hwnd)
{
    TCHAR szPath[MAX_PATH] = L"";
    ASSERT_VOID(GetPhotoPath(szPath, NELEMS(szPath), ListView_GetSelectionMark(ghWndListView)));
    PCIDLIST_ABSOLUTE pidlFolder = ILCreateFromPath(szPath);
    ASSERT_VOID(pidlFolder);
    SHOpenFolderAndSelectItems(pidlFolder, 0, NULL, 0);
    ILFree((LPITEMIDLIST)pidlFolder);
}

/**
 * @brief Open the properties dialog for the selected photo
 *
 * This function gets the path of the selected photo and opens its properties dialog
 * in Windows Explorer.
 *
 * @param hwnd Handle to the parent window
 */
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

/**
 * @brief Copy text to the clipboard
 *
 * This function copies the specified text to the Windows clipboard.
 *
 * @param hwnd Handle to the window
 * @param szBuf Text to copy to the clipboard
 */
static void CopyToClip(HWND hwnd, const PTSTR szBuf)
{
    ASSERT_VOID(OpenClipboard(hwnd));
    size_t size = (lstrlen(szBuf) + 1) * sizeof(TCHAR);
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

/**
 * @brief Copy the path of the selected photo to the clipboard
 *
 * This function gets the path of the selected photo and copies it to the Windows clipboard.
 *
 * @param hwnd Handle to the parent window
 */
static void CopyPathToClip(HWND hwnd)
{
    TCHAR szPath[MAX_PATH] = L"";
    ASSERT_VOID(GetPhotoPath(szPath, NELEMS(szPath), ListView_GetSelectionMark(ghWndListView)));
    CopyToClip(hwnd, szPath);
}

/**
 * @brief Copy tab-separated values of the selected photo to the clipboard
 *
 * This function retrieves all the displayed information about the selected photo
 * as tab-separated values and copies it to the Windows clipboard.
 *
 * @param hwnd Handle to the parent window
 */
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

/**
 * @brief Display a save file dialog to get a file path
 *
 * This function displays a standard Windows save file dialog to allow the user to
 * specify a file path for saving, with the option to append a default extension.
 *
 * @param hwnd Handle to the parent window
 * @param uID Resource ID of the file filter string
 * @param szPath Buffer to receive the selected file path
 * @param size Size of the buffer
 * @param szExt Default file extension to append if none is specified
 * @return TRUE if a file path was selected, FALSE otherwise
 */
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

/**
 * @brief Export the photo list to a TSV file
 *
 * This function exports the photo list displayed in the list view to a tab-separated values file.
 *
 * @param hwnd Handle to the parent window
 */
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

/**
 * @brief Export the photo list to an HTML file
 *
 * This function exports the photo list displayed in the list view to an HTML file.
 *
 * @param hwnd Handle to the parent window
 */
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

/**
 * @brief Handle the WM_TIMER message
 *
 * This function handles timer events for directory opening, reloading, and auto-processing operations,
 * updating the UI accordingly.
 *
 * @param hwnd Handle to the window
 * @param id Timer identifier
 */
static void Main_OnTimer(HWND hwnd, UINT_PTR id)
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

/**
 * @brief Handle the WM_DESTROY message
 *
 * This function handles the WM_DESTROY message by cleaning up resources and posting a quit message.
 *
 * @param hwnd Handle to the window
 */
static void Main_OnDestroy(HWND hwnd)
{
    DragAcceptFiles(hwnd, FALSE);
    DestroyPhotoViewWnd(ghWndPhotoView);
    PostQuitMessage(0);
}

/**
 * @brief Handle the WM_DROPFILES message
 *
 * This function handles the WM_DROPFILES message by processing dropped files or directories.
 * If a directory is dropped, it opens that directory in the application.
 *
 * @param hwnd Handle to the window
 * @param hdrop Handle to the dropped files
 */
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

/**
 * @brief Thread function for sorting photos
 *
 * This function runs in a separate thread to sort photos by a specified column
 * and sends a message when the operation is complete.
 *
 * @param pVoid Pointer to SORT_THREAD_PARAMS structure containing sort parameters
 */
static void __cdecl SortThread(PVOID pVoid)
{
    SORT_THREAD_PARAMS *pParams = (SORT_THREAD_PARAMS *)pVoid;
    SortPhotos(pParams->columnIndex, pParams->isAscending);
    SendMessage(pParams->hWnd, WM_SORT_DONE, 0, 0);
    GlobalFree(pVoid);
    _endthread();
}

/**
 * @brief Handle the WM_SORT_START message
 *
 * This function handles the WM_SORT_START message by initiating a sorting operation on a background thread.
 *
 * @param hwnd Handle to the window
 * @param columnIndex Index of the column to sort by
 * @param isAscending TRUE to sort in ascending order, FALSE for descending
 */
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

/**
 * @brief Handle the WM_SORT_DONE message
 *
 * This function handles the WM_SORT_DONE message by redrawing the list view and updating the lock state.
 *
 * @param hwnd Handle to the window
 */
static void Main_OnSortDone(HWND hwnd)
{
    ListView_RedrawItems(ghWndListView, 0, gPhotoLib.iCount - 1);
    UpdateStatusDone();
    UnLock(hwnd);
}

/**
 * @brief Handle the WM_OPENDIR_DONE message
 *
 * This function handles the WM_OPENDIR_DONE message by updating the UI after directory opening is complete.
 *
 * @param hwnd Handle to the window
 * @param b TRUE if the directory was opened successfully, FALSE otherwise
 */
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

/**
 * @brief Handle the WM_RELOAD_DONE message
 *
 * This function handles the WM_RELOAD_DONE message by updating the UI after the reload operation is complete.
 *
 * @param hwnd Handle to the window
 */
static void Main_OnReloadDone(HWND hwnd)
{
    if (iTraverseEnd >= iTraverseStart)
        ListView_RedrawItems(ghWndListView, iTraverseStart, iTraverseEnd);
    iTraverseStart = 0;
    iTraverseEnd = -1;
    UpdateStatusDone();
    UnLock(hwnd);
}

/**
 * @brief Lock the UI during operations
 *
 * This function disables menu items to prevent user interaction during operations like
 * opening directories, reloading, or auto-processing.
 *
 * @param hwnd Handle to the window
 */
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

/**
 * @brief Unlock the UI after operations
 *
 * This function enables menu items and toolbar buttons to allow user interaction after operations like
 * opening directories, reloading, or auto-processing are complete.
 *
 * @param hwnd Handle to the window
 */
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
