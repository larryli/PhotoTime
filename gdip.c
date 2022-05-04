#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "gdipimage.h"
#include "gdip.h"
#include "utils.h"

#pragma comment(lib, "GdiPlus.lib")

static ULONG_PTR upToken;

void InitGdip(void)
{
    GdiplusStartupInput gdiplusStartupInput = {1, NULL, FALSE, FALSE};
    GdiplusStartup(&upToken, &gdiplusStartupInput, NULL);
}

void DeinitGdip(void)
{
    GdiplusShutdown(upToken);
}

BOOL GdipGetPropertyTagDateTime(LPCTSTR szFilepath, LPSTR *pSzBuf)
{
    BOOL bRet = FALSE;
    PropertyItem *pPropBuffer = NULL;
    GpImage *image;
    if (Ok != GdipLoadImageFromFile(szFilepath, &image))
        return bRet;

    UINT size = 0;
    if (Ok != GdipGetPropertyItemSize(image, PropertyTagDateTime, &size))
        goto end;
    pPropBuffer = (PropertyItem *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, size);
    if (!pPropBuffer)
        goto end;
    if (Ok != GdipGetPropertyItem(image, PropertyTagDateTime, size, pPropBuffer))
        goto end;
    if (pPropBuffer->type != PropertyTagTypeASCII)
        goto end;
    LPSTR szBuf = (LPSTR)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, pPropBuffer->length);
    if (!szBuf)
        goto end;
    strncpy(szBuf, pPropBuffer->value, pPropBuffer->length);
    *pSzBuf = szBuf;
    bRet = TRUE;

end:
    if (pPropBuffer)
        GlobalFree(pPropBuffer);
    GdipDisposeImage(image);
    return bRet;
}

static RotateFlipType rfts[] = {
    RotateNoneFlipX,    // PropertyTagRotateNoneFlipX
    Rotate180FlipNone,  // PropertyTagRotate180FlipNone
    Rotate180FlipX,     // PropertyTagRotate180FlipX
    Rotate90FlipX,      // PropertyTagRotate270FlipX
    Rotate90FlipNone,   // PropertyTagRotate270FlipNone
    Rotate270FlipX,     // PropertyTagRotate90FlipX
    Rotate270FlipNone,  // PropertyTagRotate90FlipNone
};

void *GdipLoadImage(LPCTSTR szPath)
{
    GpImage *image;
    if (Ok != GdipLoadImageFromFile(szPath, &image))
        return NULL;
    UINT size = 0;
    if (Ok == GdipGetPropertyItemSize(image, PropertyTagOrientation, &size)) {
        PropertyItem *pPropBuffer = (PropertyItem *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, size);
        if (pPropBuffer) {
            if (Ok == GdipGetPropertyItem(image, PropertyTagOrientation, size, pPropBuffer)) {
                if (pPropBuffer->type == PropertyTagTypeShort) {
                    SHORT n = *((SHORT *)pPropBuffer->value) - PropertyTagRotateNoneFlipX;
                    if (n >= 0 && n < (SHORT)NELEMS(rfts))
                        GdipImageRotateFlip(image, rfts[n]);
                }
            }
            GlobalFree(pPropBuffer);
        }
    }
    return image;
}

void GdipDestoryImage(void *data)
{
    GpImage *image = (GpImage *)data;
    if (image)
        GdipDisposeImage(image);
}

BOOL GdipDrawImage(void *data, HDC hdc, const RECT * rc)
{
    BOOL bRet = FALSE;
    GpImage *image = (GpImage *)data;
    if (!image)
        return bRet;
    GpGraphics *graphics = NULL;
    if (Ok != GdipCreateFromHDC(hdc, &graphics))
        return bRet;
    UINT w, h;
    if (Ok != GdipGetImageWidth(image, &w))
        goto end;
    if (Ok != GdipGetImageHeight(image, &h))
        goto end;
    UINT x = rc->left;
    UINT y = rc->top;
    UINT cw = (UINT)(rc->right - rc->left);
    UINT ch = (UINT)(rc->bottom - rc->top);
    if (w > cw) {
        float rw = w / (float)cw;
        if (h > ch) {
            float rh = (float)h / (float)ch;
            if (rw > rh) {
                w = cw;
                h = (UINT)((float)h / rw);
                y += (LONG)((ch - h) / 2);
            } else {
                h = ch;
                w = (UINT)((float)w / rh);
                x += (LONG)((cw - w) / 2);
            }
        } else {
            w = cw;
            h = (UINT)((float)h / rw);
            y += (LONG)((ch - h) / 2);
        }
    } else if (h > ch) {
        float rh = h / (float)ch;
        h = ch;
        w = (UINT)((float)w / rh);
        x += (LONG)((cw - w) / 2);
    } else {
        x += (LONG)((cw - w) / 2);
        y += (LONG)((ch - h) / 2);
    }
    if (Ok == GdipDrawImageRectI(graphics, image, x, y, w, h))
        bRet = TRUE;
end:
    if (graphics)
        GdipDeleteGraphics(graphics);
    return bRet;
}
