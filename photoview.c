#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include "gdip.h"
#include "photoview.h"

#define HANDLE_PVM_SETPATH(hwnd,wParam,lParam,fn) ((fn)((hwnd),(LPCTSTR)(wParam)),0)

static WNDPROC pPhotoViewProc = NULL;
static LRESULT CALLBACK PhotoViewWndProc(HWND, UINT, WPARAM, LPARAM);

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
    if (!hWndPV)
        return NULL;
    WNDPROC p = (WNDPROC)SetWindowLongPtr(hWndPV, GWLP_WNDPROC, (LONG_PTR)PhotoViewWndProc);
    if (!pPhotoViewProc)
        pPhotoViewProc = p;
    return hWndPV;
}

void DestroyPhotoViewWnd(HWND hWndPV)
{
    if (pPhotoViewProc)
        SetWindowLongPtr(hWndPV, GWLP_WNDPROC, (LONG_PTR)pPhotoViewProc);
}

static void PhotoView_OnSetPath(HWND hwnd, LPCTSTR szPath)
{
    InvalidateRect(hwnd, NULL, TRUE);
    void *data = (void *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    GdipDestoryImage(data);
    data = GdipLoadImage(szPath);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);
}

static BOOL PhotoView_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    FillRect(hdc, &rc, GetSysColorBrush(COLOR_BTNFACE));
    return TRUE;
}

static void PhotoView_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    RECT rc;
    GetClientRect(hwnd, &rc);
    BeginPaint(hwnd, &ps);
    void *data = (void *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    GdipDrawImage(data, ps.hdc, &rc);
    EndPaint(hwnd, &ps);
}

static void PhotoView_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    FORWARD_WM_SIZE(hwnd, state, cx, cy, pPhotoViewProc);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void PhotoView_OnDestroy(HWND hwnd)
{
    void *data = (void *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    GdipDestoryImage(data);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
    FORWARD_WM_DESTROY(hwnd, pPhotoViewProc);
}

static LRESULT CALLBACK PhotoViewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    HANDLE_MSG(hwnd, PVM_SETPATH, PhotoView_OnSetPath);
    HANDLE_MSG(hwnd, WM_ERASEBKGND, PhotoView_OnEraseBkgnd);
    HANDLE_MSG(hwnd, WM_PAINT, PhotoView_OnPaint);
    HANDLE_MSG(hwnd, WM_SIZE, PhotoView_OnSize);
    HANDLE_MSG(hwnd, WM_DESTROY, PhotoView_OnDestroy);
    default:
        return CallWindowProc(pPhotoViewProc, hwnd, msg, wParam, lParam);
    }
}
