#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include "gdip.h"
#include "photoview.h"
#include "main.h"
#include "utils.h"

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
}

static BOOL PhotoView_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    FillRect(hdc, &rc, GetSysColorBrush(COLOR_BTNFACE));
    return TRUE;
}

static void DrawIdString(HWND hwnd, HDC hdc, RECT *rc, int id)
{
    TCHAR szBuf[MAX_PATH];
    if (LoadString((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), id, szBuf, NELEMS(szBuf))) {
        SelectObject(hdc, GetStockObject(OEM_FIXED_FONT));
        SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
        SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
        DrawText(hdc, szBuf, -1, rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    }
}

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
    HANDLE_MSG(hwnd, WM_ERASEBKGND, PhotoView_OnEraseBkgnd);
    HANDLE_MSG(hwnd, WM_PAINT, PhotoView_OnPaint);
    HANDLE_MSG(hwnd, WM_SIZE, PhotoView_OnSize);
    HANDLE_MSG(hwnd, WM_DESTROY, PhotoView_OnDestroy);
    default:
        return CallWindowProc(pPhotoViewProc, hwnd, msg, wParam, lParam);
    }
}
