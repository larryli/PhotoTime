#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#define COBJMACROS
#include <objidl.h>
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

#include "gdipimage.h"
#include "gdip.h"
#include "utils.h"

static ULONG_PTR upToken;

typedef struct {
    GUID guid;
    CLSID clsid;
} GUID_TO_CLSID;

static GUID_TO_CLSID *pGuidToClsid = NULL;
static UINT uGuidToClsid = 0;

static void InitGuidToClsid(void)
{
    if (pGuidToClsid)
        return;
    UINT size = 0;
    ImageCodecInfo *pImageCodecInfo = NULL;
    if (Ok != GdipGetImageEncodersSize(&uGuidToClsid, &size))
        return;
    pImageCodecInfo = GdipAlloc(size);
    if (pImageCodecInfo == NULL)
        return;
    if (Ok != GdipGetImageEncoders(uGuidToClsid, size, pImageCodecInfo))
        goto end;
    pGuidToClsid = GdipAlloc(uGuidToClsid * sizeof(GUID_TO_CLSID));
    if (pGuidToClsid == NULL)
        goto end;
    for (UINT i = 0; i < uGuidToClsid; ++i) {
        pGuidToClsid[i].guid = pImageCodecInfo[i].FormatID;
        pGuidToClsid[i].clsid = pImageCodecInfo[i].Clsid;
    }
end:
    if (pImageCodecInfo)
        GdipFree(pImageCodecInfo);
}

static void DeinitGuidToClsid(void)
{
    if (pGuidToClsid) {
        GdipFree(pGuidToClsid);
        pGuidToClsid = NULL;
    }
    if (uGuidToClsid)
        uGuidToClsid = 0;
}

void InitGdip(void)
{
    GdiplusStartupInput gdiplusStartupInput = {1, NULL, FALSE, FALSE};
    GdiplusStartup(&upToken, &gdiplusStartupInput, NULL);
    InitGuidToClsid();
}

void DeinitGdip(void)
{
    DeinitGuidToClsid();
    GdiplusShutdown(upToken);
}

static CLSID *GetImageEncoderClsid(GpImage *image)
{
    GUID guid;
    if (Ok != GdipGetImageRawFormat(image, &guid))
        return NULL;
    for (UINT i = 0; i < uGuidToClsid; i++)
        if (IsEqualGUID(&guid, &pGuidToClsid[i].guid))
            return &pGuidToClsid[i].clsid;
    return NULL;
}

static GpImage *GdipLoadImageFile(LPCTSTR szFilePath)
{
    IStream *stream;
    if (S_OK != SHCreateStreamOnFile(szFilePath, STGM_READ, &stream))
        return NULL;
    GpImage *image;
    if (Ok != GdipLoadImageFromStream(stream, &image))
        image = NULL;
    IStream_Release(stream);
    return image;
}

static BOOL GdipSaveImageFile(GpImage *image, LPCTSTR szFilePath)
{
    BOOL bRet = FALSE;
    CLSID *clsid = GetImageEncoderClsid(image);
    if (!clsid)
        return bRet;
    IStream *stream;
    if (S_OK != SHCreateStreamOnFile(szFilePath, STGM_WRITE, &stream))
        return bRet;
    if (Ok == GdipSaveImageToStream(image, stream, clsid, NULL))
        bRet = TRUE;
    IStream_Release(stream);
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

static int GetImageRotaion(GpImage *image)
{
    int iRet = -1;
    UINT size = 0;
    if (Ok != GdipGetPropertyItemSize(image, PropertyTagOrientation, &size))
        return iRet;
    PropertyItem *pPropItem = (PropertyItem *)GdipAlloc(size);
    if (!pPropItem)
        return iRet;
    if (Ok != GdipGetPropertyItem(image, PropertyTagOrientation, size, pPropItem))
        goto end;
    if (pPropItem->type != PropertyTagTypeShort)
        goto end;
    iRet = (int)(*((SHORT *)pPropItem->value) - PropertyTagRotateNoneFlipX);
end:
    GdipFree(pPropItem);
    return iRet;
}

void *GdipLoadImage(LPCTSTR szFilePath)
{
    GpImage *image = GdipLoadImageFile(szFilePath);
    if (!image)
        return NULL;
    int iRotaion = GetImageRotaion(image);
    if (iRotaion >= 0 && iRotaion < (SHORT)NELEMS(rfts))
        GdipImageRotateFlip(image, rfts[iRotaion]);
    return image;
}

BOOL GdipGetTagSystemTime(LPCTSTR szFilePath, PSYSTEMTIME pSt)
{
    BOOL bRet = FALSE;
    PropertyItem *pPropItem = NULL;
    GpImage *image = GdipLoadImageFile(szFilePath);
    if (!image)
        return bRet;

    UINT size = 0;
    if (Ok != GdipGetPropertyItemSize(image, PropertyTagDateTime, &size))
        goto end;
    pPropItem = (PropertyItem *)GdipAlloc(size);
    if (!pPropItem)
        goto end;
    if (Ok != GdipGetPropertyItem(image, PropertyTagDateTime, size, pPropItem))
        goto end;
    if (pPropItem->type != PropertyTagTypeASCII)
        goto end;
    ZeroMemory(pSt, sizeof(SYSTEMTIME));
    sscanf((char *)pPropItem->value, "%hu:%hu:%hu %hu:%hu:%hu",
           &pSt->wYear, &pSt->wMonth, &pSt->wDay, &pSt->wHour, &pSt->wMinute, &pSt->wSecond);
    bRet = TRUE;

end:
    if (pPropItem)
        GdipFree(pPropItem);
    GdipDisposeImage(image);
    return bRet;
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

BOOL GdipSaveImageWithTagSystemTime(LPCTSTR szFilePath, const PSYSTEMTIME pSt)
{
    BOOL bRet = FALSE;
    GpImage *image = GdipLoadImageFile(szFilePath);
    if (!image)
        return bRet;
    char buf[20] = "\0"; // "YYYY:MM:DD hh:mm:ss"
    ULONG len = (ULONG)snprintf(buf, NELEMS(buf), "%4hu:%02hu:%02hu %02hu:%02hu:%02hu",
                                pSt->wYear, pSt->wMonth, pSt->wDay, pSt->wHour, pSt->wMinute, pSt->wSecond);
    PropertyItem propItem = {
        .id = PropertyTagDateTime,
        .length = len + 1,
        .type = PropertyTagTypeASCII,
        .value = buf,
    };
    if (Ok != GdipSetPropertyItem(image, &propItem))
        goto end;
    bRet = GdipSaveImageFile(image, szFilePath);
end:
    GdipDisposeImage(image);
    return bRet;
}

BOOL GdipGetSize(void *data, SIZE *size)
{
    GpImage *image = (GpImage *)data;
    if (!image)
        return FALSE;
    if (Ok != GdipGetImageWidth(image, &size->cx))
        return FALSE;
    if (Ok != GdipGetImageHeight(image, &size->cy))
        return FALSE;
    return TRUE;
}
