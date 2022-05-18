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

#define ASSERT_OK_RETURN(a, s) ASSERT_RETURN(Ok == a, s)
#define ASSERT_OK_VOID(a) ASSERT_VOID(Ok == a)
#define ASSERT_OK_NULL(a) ASSERT_OK_RETURN(a, NULL)
#define ASSERT_OK_FALSE(a) ASSERT_OK_RETURN(a, FALSE)
#define ASSERT_OK_GOTO(a, label) ASSERT_GOTO(Ok == a, label)
#define ASSERT_OK_END(a) ASSERT_OK_GOTO(a, end)

#define ASSERT_S_OK_RETURN(a, ret) ASSERT_RETURN(S_OK == a, ret)
#define ASSERT_S_OK_NULL(a) ASSERT_S_OK_RETURN(a, NULL)
#define ASSERT_S_OK_FALSE(a) ASSERT_S_OK_RETURN(a, FALSE)

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
    ASSERT_OK_VOID(GdipGetImageEncodersSize(&uGuidToClsid, &size));
    pImageCodecInfo = GdipAlloc(size);
    ASSERT_VOID(pImageCodecInfo);
    ASSERT_OK_END(GdipGetImageEncoders(uGuidToClsid, size, pImageCodecInfo));
    pGuidToClsid = GdipAlloc(uGuidToClsid * sizeof(GUID_TO_CLSID));
    ASSERT_GOTO(pGuidToClsid, end);
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
    ASSERT_OK_NULL(GdipGetImageRawFormat(image, &guid));
    for (UINT i = 0; i < uGuidToClsid; i++)
        if (IsEqualGUID(&guid, &pGuidToClsid[i].guid))
            return &pGuidToClsid[i].clsid;
    return NULL;
}

static GpImage *GdipLoadImageFile(LPCTSTR szFilePath)
{
    IStream *stream;
    ASSERT_S_OK_NULL(SHCreateStreamOnFile(szFilePath, STGM_READ, &stream));
    GpImage *image;
    if (Ok != GdipLoadImageFromStream(stream, &image))
        image = NULL;
    IStream_Release(stream);
    return image;
}

static BOOL GdipSaveImageFile(GpImage *image, LPCTSTR szFilePath)
{
    CLSID *clsid = GetImageEncoderClsid(image);
    ASSERT_FALSE(clsid);
    IStream *stream;
    ASSERT_S_OK_FALSE(SHCreateStreamOnFile(szFilePath, STGM_WRITE, &stream));
    BOOL bRet = (Ok == GdipSaveImageToStream(image, stream, clsid, NULL));
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
    UINT size = 0;
    ASSERT_OK_RETURN(GdipGetPropertyItemSize(image, PropertyTagOrientation, &size), -1);
    PropertyItem *pPropItem = (PropertyItem *)GdipAlloc(size);
    ASSERT(pPropItem, -1);
    int iRet = -1;
    ASSERT_OK_END(GdipGetPropertyItem(image, PropertyTagOrientation, size, pPropItem));
    ASSERT_END(pPropItem->type == PropertyTagTypeShort);
    iRet = (int)(*((SHORT *)pPropItem->value) - PropertyTagRotateNoneFlipX);
end:
    GdipFree(pPropItem);
    return iRet;
}

void *GdipLoadImage(LPCTSTR szFilePath)
{
    GpImage *image = GdipLoadImageFile(szFilePath);
    ASSERT_NULL(image);
    int iRotaion = GetImageRotaion(image);
    if (iRotaion >= 0 && iRotaion < (SHORT)NELEMS(rfts))
        GdipImageRotateFlip(image, rfts[iRotaion]);
    return image;
}

BOOL GdipGetTagSystemTime(LPCTSTR szFilePath, PSYSTEMTIME pSt)
{
    GpImage *image = GdipLoadImageFile(szFilePath);
    ASSERT_FALSE(image);

    PropertyItem *pPropItem = NULL;
    BOOL bRet = FALSE;
    UINT size = 0;
    ASSERT_OK_END(GdipGetPropertyItemSize(image, PropertyTagDateTime, &size));
    pPropItem = (PropertyItem *)GdipAlloc(size);
    ASSERT_END(pPropItem);
    ASSERT_OK_END(GdipGetPropertyItem(image, PropertyTagDateTime, size, pPropItem));
    ASSERT_END(pPropItem->type == PropertyTagTypeASCII);
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
    ASSERT_VOID(image);
    GdipDisposeImage(image);
}

BOOL GdipDrawImage(void *data, HDC hdc, const RECT * rc)
{
    GpImage *image = (GpImage *)data;
    ASSERT_FALSE(image);
    GpGraphics *graphics = NULL;
    ASSERT_OK_FALSE(GdipCreateFromHDC(hdc, &graphics));
    BOOL bRet = FALSE;
    UINT w, h;
    ASSERT_OK_END(GdipGetImageWidth(image, &w));
    ASSERT_OK_END(GdipGetImageHeight(image, &h));
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
    bRet = (Ok == GdipDrawImageRectI(graphics, image, x, y, w, h));
end:
    if (graphics)
        GdipDeleteGraphics(graphics);
    return bRet;
}

BOOL GdipSaveImageWithTagSystemTime(LPCTSTR szFilePath, const PSYSTEMTIME pSt)
{
    BOOL bRet = FALSE;
    GpImage *image = GdipLoadImageFile(szFilePath);
    ASSERT_FALSE(image);
    char buf[20] = "\0"; // "YYYY:MM:DD hh:mm:ss"
    ULONG len = (ULONG)snprintf(buf, NELEMS(buf), "%4hu:%02hu:%02hu %02hu:%02hu:%02hu",
                                pSt->wYear, pSt->wMonth, pSt->wDay, pSt->wHour, pSt->wMinute, pSt->wSecond);
    PropertyItem propItem = {
        .id = PropertyTagDateTime,
        .length = len + 1,
        .type = PropertyTagTypeASCII,
        .value = buf,
    };
    ASSERT_OK_END(GdipSetPropertyItem(image, &propItem));
    bRet = GdipSaveImageFile(image, szFilePath);
end:
    GdipDisposeImage(image);
    return bRet;
}

BOOL GdipGetSize(void *data, SIZE *size)
{
    GpImage *image = (GpImage *)data;
    ASSERT_FALSE(image);
    ASSERT_OK_FALSE(GdipGetImageWidth(image, &size->cx));
    ASSERT_OK_FALSE(GdipGetImageHeight(image, &size->cy));
    return TRUE;
}
