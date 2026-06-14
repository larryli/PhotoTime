#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include "gdip.h"
#include "photoview.h"
#include "utils.h"

#include "main.h"

#define HANDLE_PVM_SETPATH(hwnd,wParam,lParam,fn) ((fn)((hwnd),(PCTSTR)(wParam)))
#define HANDLE_PVM_GETSIZE(hwnd,wParam,lParam,fn) ((fn)((hwnd),(PSIZE)(wParam)))

static WNDPROC pPhotoViewProc = NULL;
static LRESULT CALLBACK PhotoViewWndProc(HWND, UINT, WPARAM, LPARAM);

/**
 * @brief Create a photo view window
 *
 * This function creates a photo view control as a child window with a custom window procedure.
 *
 * @param hWndParent Handle to the parent window
 * @param hInst Instance handle of the application
 * @return Handle to the created photo view window
 */
HWND CreatePhotoViewWnd(HWND hWndParent, HINSTANCE hInst)
{
    HWND hWndPV = CreateWindowEx(WS_EX_STATICEDGE,
                               L"Static",
                               NULL,
                               WS_CHILD | WS_VISIBLE,
                               0, 0, 0, 0,
                               hWndParent,
                               0,
                               hInst,
                               NULL);
    ASSERT_NULL(hWndPV);
    WNDPROC p = (WNDPROC)SetWindowLongPtr(hWndPV, GWLP_WNDPROC, (LONG_PTR)PhotoViewWndProc);
    if (!pPhotoViewProc)
        pPhotoViewProc = p;
    return hWndPV;
}

/**
 * @brief Destroy the photo view window
 *
 * This function restores the original window procedure and cleans up the photo view window.
 *
 * @param hWndPV Handle to the photo view window to destroy
 */
void DestroyPhotoViewWnd(HWND hWndPV)
{
    ASSERT_VOID(pPhotoViewProc);
    SetWindowLongPtr(hWndPV, GWLP_WNDPROC, (LONG_PTR)pPhotoViewProc);
}

/**
 * @brief Handle the PVM_SETPATH message to set the photo path
 *
 * This function handles the custom PVM_SETPATH message to set the path of the photo to display
 * in the photo view window, loading the image and updating the window data.
 *
 * @param hwnd Handle to the photo view window
 * @param szPath Path to the photo file to display
 * @return TRUE if the photo was loaded successfully, FALSE otherwise
 */
static BOOL PhotoView_OnSetPath(HWND hwnd, PCTSTR szPath)
{
    InvalidateRect(hwnd, NULL, TRUE);
    LONG_PTR p = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (p != -1)
        GdipDestoryImage((void *)p);
    if (szPath) {
        p = (LONG_PTR)GdipLoadImage(szPath);
        if (!p)
            p = -1;
    } else
        p = 0;
    SetWindowLongPtr(hwnd, GWLP_USERDATA, p);
    return (p != -1); // NULL is TRUE
}

/**
 * @brief Handle the PVM_GETSIZE message to get the photo size
 *
 * This function handles the custom PVM_GETSIZE message to retrieve the size of the currently
 * displayed photo.
 *
 * @param hwnd Handle to the photo view window
 * @param pSize Pointer to SIZE structure to store the photo dimensions
 * @return TRUE if the size was retrieved successfully, FALSE otherwise
 */
static BOOL PhotoView_OnGetSize(HWND hwnd, PSIZE pSize)
{
    LONG_PTR p = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (p == -1)
        return FALSE;
    ASSERT_FALSE(p);
    return GdipGetSize((void *)p, pSize);
}

/**
 * @brief Handle the WM_ERASEBKGND message to erase the background
 *
 * This function handles the WM_ERASEBKGND message by filling the client area with the button face color.
 *
 * @param hwnd Handle to the photo view window
 * @param hdc Handle to the device context for erasing
 * @return TRUE to indicate the background was erased
 */
static BOOL PhotoView_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    FillRect(hdc, &rc, GetSysColorBrush(COLOR_BTNFACE));
    return TRUE;
}

/**
 * @brief Draw a string resource in the center of a rectangle
 *
 * This function loads a string from resources and draws it centered in the specified rectangle.
 *
 * @param hwnd Handle to the window
 * @param hdc Handle to the device context to draw on
 * @param rc Pointer to the rectangle to draw in
 * @param id Resource ID of the string to draw
 */
static void DrawIdString(HWND hwnd, HDC hdc, RECT *rc, int id)
{
    TCHAR szBuf[MAX_PATH];
    ASSERT_VOID(LoadString((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), id, szBuf, NELEMS(szBuf)));
    SelectObject(hdc, GetStockObject(OEM_FIXED_FONT));
    SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
    DrawText(hdc, szBuf, -1, rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
}

/**
 * @brief Handle the WM_PAINT message to paint the photo view
 *
 * This function handles the WM_PAINT message by drawing the currently loaded photo or an error message.
 *
 * @param hwnd Handle to the photo view window
 */
static void PhotoView_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    RECT rc;
    GetClientRect(hwnd, &rc);
    BeginPaint(hwnd, &ps);
    LONG_PTR p = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (p == -1)
        DrawIdString(hwnd, ps.hdc, &rc, IDS_LOAD_PHOTO_FAILED);
    else if (p && !GdipDrawImage((void *)p, ps.hdc, &rc))
        DrawIdString(hwnd, ps.hdc, &rc, IDS_SHOW_PHOTO_FAILED);
    EndPaint(hwnd, &ps);
}

static void PhotoView_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    FORWARD_WM_SIZE(hwnd, state, cx, cy, pPhotoViewProc);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void PhotoView_OnDestroy(HWND hwnd)
{
    LONG_PTR p = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (p != -1)
        GdipDestoryImage((void *)p);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
    FORWARD_WM_DESTROY(hwnd, pPhotoViewProc);
}

static LRESULT CALLBACK PhotoViewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    HANDLE_MSG(hwnd, PVM_SETPATH, PhotoView_OnSetPath);
    HANDLE_MSG(hwnd, PVM_GETSIZE, PhotoView_OnGetSize);
    HANDLE_MSG(hwnd, WM_ERASEBKGND, PhotoView_OnEraseBkgnd);
    HANDLE_MSG(hwnd, WM_PAINT, PhotoView_OnPaint);
    HANDLE_MSG(hwnd, WM_SIZE, PhotoView_OnSize);
    HANDLE_MSG(hwnd, WM_DESTROY, PhotoView_OnDestroy);
    default:
        return CallWindowProc(pPhotoViewProc, hwnd, msg, wParam, lParam);
    }
}
