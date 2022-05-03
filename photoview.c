#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include "gdipimage.h"
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
    GpImage *image = (GpImage *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (image) {
        GdipDisposeImage(image);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
    }
    if (Ok != GdipLoadImageFromFile(szPath, &image))
        return;
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)image);
}

static BOOL PhotoView_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    FillRect(hdc, &rc, GetSysColorBrush(COLOR_3DLIGHT));
    return TRUE;
}

static void PhotoView_OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    RECT rc;
    GetClientRect(hwnd, &rc);
    BeginPaint(hwnd, &ps);
    GpImage *image = (GpImage *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (image)
        GdipDrawImage(image, ps.hdc, &rc, TRUE);
    EndPaint(hwnd, &ps);
}

void PhotoView_OnDestroy(HWND hwnd)
{
    GpImage *image = (GpImage *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (image) {
        GdipDisposeImage(image);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
    }
    CallWindowProc(pPhotoViewProc, hwnd, WM_DESTROY, 0, 0);
}

static LRESULT CALLBACK PhotoViewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    HANDLE_MSG(hwnd, PVM_SETPATH, PhotoView_OnSetPath);
    HANDLE_MSG(hwnd, WM_ERASEBKGND, PhotoView_OnEraseBkgnd);
    HANDLE_MSG(hwnd, WM_PAINT, PhotoView_OnPaint);
    HANDLE_MSG(hwnd, WM_DESTROY, PhotoView_OnDestroy);
    default:
        return CallWindowProc(pPhotoViewProc, hwnd, msg, wParam, lParam);
    }
}
