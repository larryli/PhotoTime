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

/**
 * @brief Initialize the GUID to CLSID mapping table
 *
 * This function initializes a lookup table that maps image format GUIDs to their corresponding encoder CLSIDs.
 * This is used when saving images to determine the appropriate encoder to use.
 */
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

/**
 * @brief Deinitialize the GUID to CLSID mapping table
 *
 * This function cleans up and frees the memory allocated for the GUID to CLSID mapping table.
 */
static void DeinitGuidToClsid(void)
{
    if (pGuidToClsid) {
        GdipFree(pGuidToClsid);
        pGuidToClsid = NULL;
    }
    if (uGuidToClsid)
        uGuidToClsid = 0;
}

/**
 * @brief Initialize GDI+ library
 *
 * This function initializes the GDI+ library for image processing operations and sets up
 * the GUID to CLSID mapping table for image encoders.
 */
void InitGdip(void)
{
    GdiplusStartupInput gdiplusStartupInput = {1, NULL, FALSE, FALSE};
    GdiplusStartup(&upToken, &gdiplusStartupInput, NULL);
    InitGuidToClsid();
}

/**
 * @brief Deinitialize GDI+ library
 *
 * This function deinitializes and cleans up the GDI+ library resources and cleans up
 * the GUID to CLSID mapping table.
 */
void DeinitGdip(void)
{
    DeinitGuidToClsid();
    GdiplusShutdown(upToken);
}

/**
 * @brief Get the encoder CLSID for an image
 *
 * This function determines the appropriate encoder CLSID for the given image based on its format.
 *
 * @param image Pointer to the GDI+ image object
 * @return Pointer to the CLSID of the appropriate encoder, or NULL if not found
 */
static CLSID *GetImageEncoderClsid(GpImage *image)
{
    GUID guid;
    ASSERT_OK_NULL(GdipGetImageRawFormat(image, &guid));
    for (UINT i = 0; i < uGuidToClsid; i++)
        if (IsEqualGUID(&guid, &pGuidToClsid[i].guid))
            return &pGuidToClsid[i].clsid;
    return NULL;
}

/**
 * @brief Load an image file using GDI+
 *
 * This function loads an image file using GDI+ through a COM stream interface.
 *
 * @param szFilePath Path to the image file to load
 * @return Pointer to the loaded GDI+ image object, or NULL on failure
 */
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

/**
 * @brief Save an image to a file using GDI+
 *
 * This function saves a GDI+ image object to a file using the appropriate encoder.
 *
 * @param image Pointer to the GDI+ image object to save
 * @param szFilePath Path to the output file
 * @return TRUE if the save operation was successful, FALSE otherwise
 */
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

/**
 * @brief Get the rotation value from an image's EXIF orientation property
 *
 * This function retrieves the orientation value from the image's EXIF data and converts it
 * to a rotation index that can be used with GDI+ rotation functions.
 *
 * @param image Pointer to the GDI+ image object
 * @return The rotation index based on the EXIF orientation property, or -1 if not found
 */
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

/**
 * @brief Load an image using GDI+ and apply rotation based on EXIF data
 *
 * This function loads an image file using GDI+ and automatically applies rotation
 * based on the image's EXIF orientation data.
 *
 * @param szFilePath Path to the image file to load
 * @return Pointer to the loaded GDI+ image object, or NULL on failure
 */
void *GdipLoadImage(LPCTSTR szFilePath)
{
    GpImage *image = GdipLoadImageFile(szFilePath);
    ASSERT_NULL(image);
    int iRotaion = GetImageRotaion(image);
    if (iRotaion >= 0 && iRotaion < (SHORT)NELEMS(rfts))
        GdipImageRotateFlip(image, rfts[iRotaion]);
    return image;
}

BOOL IsValidDate(PSYSTEMTIME pSt);

/**
 * @brief Get system time from image tag
 *
 * This function extracts the system time from the EXIF DateTime tag of an image file.
 *
 * @param szFilePath Path to the image file
 * @param pSt Pointer to SYSTEMTIME structure to store the extracted time
 * @return TRUE if successful, FALSE otherwise
 */
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
    int nArgs = sscanf((char *)pPropItem->value, "%hu:%hu:%hu %hu:%hu:%hu",
                       &pSt->wYear, &pSt->wMonth, &pSt->wDay, &pSt->wHour, &pSt->wMinute, &pSt->wSecond);
    if (nArgs == 6) {
        if (pSt->wHour == 24) // fix
            pSt->wHour = 0;
        if (IsValidDate(pSt) && pSt->wHour < 24 && pSt->wMinute < 60 && pSt->wSecond < 60)
            bRet = TRUE;
    }

end:
    if (pPropItem)
        GdipFree(pPropItem);
    GdipDisposeImage(image);
    return bRet;
}

/**
 * @brief Destroy image data
 *
 * This function destroys and frees the memory associated with a GDI+ image object.
 *
 * @param data Pointer to the GDI+ image object to destroy
 */
void GdipDestoryImage(void *data)
{
    GpImage *image = (GpImage *)data;
    ASSERT_VOID(image);
    GdipDisposeImage(image);
}

/**
 * @brief Draw an image using GDI+
 *
 * This function draws a GDI+ image onto a device context within the specified rectangle,
 * scaling the image to fit while maintaining aspect ratio.
 *
 * @param data Pointer to the GDI+ image object to draw
 * @param hdc Device context to draw on
 * @param rc Rectangle defining where to draw the image
 * @return TRUE if successful, FALSE otherwise
 */
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

/**
 * @brief Save image with system time tag
 *
 * This function saves an image file with the specified system time embedded in its EXIF DateTime tag.
 *
 * @param szFilePath Path to the output image file
 * @param pSt Pointer to SYSTEMTIME structure containing the time to embed
 * @return TRUE if successful, FALSE otherwise
 */
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

/**
 * @brief Get image size
 *
 * This function retrieves the dimensions of a GDI+ image object.
 *
 * @param data Pointer to the GDI+ image object
 * @param size Pointer to SIZE structure to store the image dimensions
 * @return TRUE if successful, FALSE otherwise
 */
BOOL GdipGetSize(void *data, SIZE *size)
{
    GpImage *image = (GpImage *)data;
    ASSERT_FALSE(image);
    ASSERT_OK_FALSE(GdipGetImageWidth(image, (UINT *)&size->cx));
    ASSERT_OK_FALSE(GdipGetImageHeight(image, (UINT *)&size->cy));
    return TRUE;
}
