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

BOOL GdipDrawImage(void *data, HDC hdc, RECT * rc)
{
    BOOL bRet = FALSE;
    GpImage *image = (GpImage *)data;
    if (!image)
        return bRet;
    GpGraphics *graphics = NULL;
    if (Ok != GdipCreateFromHDC(hdc, &graphics))
        return bRet;
    UINT iw, ih;
    if (Ok != GdipGetImageWidth(image, &iw))
        goto end;
    if (Ok != GdipGetImageHeight(image, &ih))
        goto end;
    UINT cw = (UINT)(rc->right - rc->left);
    UINT ch = (UINT)(rc->bottom - rc->top);
    if (iw > cw) {
        float rw = iw / (float)cw;
        if (ih > ch) {
            float rh = (float)ih / (float)ch;
            if (rw > rh) {
                iw = cw;
                ih = (UINT)((float)ih / rw);
                rc->top += (LONG)((ch - ih) / 2);
            } else {
                ih = ch;
                iw = (UINT)((float)iw / rh);
                rc->left += (LONG)((cw - iw) / 2);
            }
        } else {
            iw = cw;
            ih = (UINT)((float)ih / rw);
            rc->top += (LONG)((ch - ih) / 2);
        }
    } else if (ih > ch) {
        float rh = ih / (float)ch;
        ih = ch;
        iw = (UINT)((float)iw / rh);
        rc->left += (LONG)((cw - iw) / 2);
    } else {
        rc->left += (LONG)((cw - iw) / 2);
        rc->top += (LONG)((ch - ih) / 2);
    }
    if (Ok == GdipDrawImageRectI(graphics, image, rc->left, rc->top, iw, ih))
        bRet = TRUE;
end:
    if (graphics)
        GdipDeleteGraphics(graphics);
    return bRet;
}
