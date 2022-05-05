#pragma once
// From fGdiPlusFlat.h

#pragma comment (linker, "GdiPlus.lib")

#ifndef GDIPVER
#define GDIPVER 0x0100
#endif

#define WINGDIPAPI __stdcall
#define GDIPCONST const

typedef enum {
    Ok = 0,
    GenericError = 1,
    InvalidParameter = 2,
    OutOfMemory = 3,
    ObjectBusy = 4,
    InsufficientBuffer = 5,
    NotImplemented = 6,
    Win32Error = 7,
    WrongState = 8,
    Aborted = 9,
    FileNotFound = 10,
    ValueOverflow = 11,
    AccessDenied = 12,
    UnknownImageFormat = 13,
    FontFamilyNotFound = 14,
    FontStyleNotFound = 15,
    NotTrueTypeFont = 16,
    UnsupportedGdiplusVersion = 17,
    GdiplusNotInitialized = 18,
    PropertyNotFound = 19,
    PropertyNotSupported = 20,
#if (GDIPVER >= 0x0110)
    ProfileNotFound = 21,
#endif //(GDIPVER >= 0x0110)
} Status;

typedef Status GpStatus;

typedef enum {
    DebugEventLevelFatal,
    DebugEventLevelWarning
} DebugEventLevel;

typedef VOID (WINAPI *DebugEventProc)(DebugEventLevel level, CHAR *message);
typedef Status (WINAPI *NotificationHookProc)(ULONG_PTR *token);
typedef VOID (WINAPI *NotificationUnhookProc)(ULONG_PTR token);

typedef struct {
    UINT32 GdiplusVersion;
    DebugEventProc DebugEventCallback;
    BOOL SuppressBackgroundThread;
    BOOL SuppressExternalCodecs;
} GdiplusStartupInput;

typedef struct {
    NotificationHookProc NotificationHook;
    NotificationUnhookProc NotificationUnhook;
} GdiplusStartupOutput;

#ifndef PROPID
typedef unsigned long PROPID;
#endif

typedef struct tagGpGraphics GpGraphics;
typedef struct tagGpImage GpImage;

typedef enum {
    RotateNoneFlipNone = 0,
    Rotate90FlipNone   = 1,
    Rotate180FlipNone  = 2,
    Rotate270FlipNone  = 3,

    RotateNoneFlipX    = 4,
    Rotate90FlipX      = 5,
    Rotate180FlipX     = 6,
    Rotate270FlipX     = 7,

    RotateNoneFlipY    = Rotate180FlipX,
    Rotate90FlipY      = Rotate270FlipX,
    Rotate180FlipY     = RotateNoneFlipX,
    Rotate270FlipY     = Rotate90FlipX,

    RotateNoneFlipXY   = Rotate180FlipNone,
    Rotate90FlipXY     = Rotate270FlipNone,
    Rotate180FlipXY    = RotateNoneFlipNone,
    Rotate270FlipXY    = Rotate90FlipNone
} RotateFlipType;

#define PropertyTagRotateNoneFlipNone 1
#define PropertyTagRotateNoneFlipX 2
#define PropertyTagRotate180FlipNone 3
#define PropertyTagRotate180FlipX 4
#define PropertyTagRotate270FlipX 5
#define PropertyTagRotate270FlipNone 6
#define PropertyTagRotate90FlipX 7
#define PropertyTagRotate90FlipNone 8
#define PropertyTagOrientation 0x0112

typedef struct {
    GUID Guid;
    ULONG NumberOfValues;
    ULONG Type;
    VOID *Value;
} EncoderParameter;

typedef struct {
    UINT Count;
    EncoderParameter Parameter[1];
} EncoderParameters;

#define PropertyTagTypeASCII 2
#define PropertyTagTypeShort 3
#define PropertyTagDateTime 0x0132

typedef struct {
    PROPID id;
    ULONG length;
    WORD type;
    VOID* value;
} PropertyItem;

typedef struct {
    CLSID Clsid;
    GUID  FormatID;
    const WCHAR *CodecName;
    const WCHAR *DllName;
    const WCHAR *FormatDescription;
    const WCHAR *FilenameExtension;
    const WCHAR *MimeType;
    DWORD Flags;
    DWORD Version;
    DWORD SigCount;
    DWORD SigSize;
    const BYTE *SigPattern;
    const BYTE *SigMask;
} ImageCodecInfo;

GpStatus WINAPI GdiplusStartup(ULONG_PTR *token, GdiplusStartupInput *input, GdiplusStartupOutput *output);
VOID WINAPI GdiplusShutdown(ULONG_PTR token);

void *WINGDIPAPI GdipAlloc(size_t size);
void WINGDIPAPI GdipFree(void *ptr);

GpStatus WINGDIPAPI GdipCreateFromHDC(HDC hdc, GpGraphics **graphics);
GpStatus WINGDIPAPI GdipDrawImageRectI(GpGraphics *graphics, GpImage *image, INT x, INT y, INT width, INT height);
GpStatus WINGDIPAPI GdipGetImageWidth(GpImage *image, UINT *width);
GpStatus WINGDIPAPI GdipGetImageHeight(GpImage *image, UINT *height);
GpStatus WINGDIPAPI GdipLoadImageFromFile(GDIPCONST WCHAR* filename, GpImage **image);
GpStatus WINGDIPAPI GdipLoadImageFromStream(IStream* stream, GpImage **image);
GpStatus WINGDIPAPI GdipSaveImageToFile(GpImage *image, GDIPCONST WCHAR *filename, GDIPCONST CLSID *clsidEncoder, GDIPCONST EncoderParameters *encoderParams);
GpStatus WINGDIPAPI GdipSaveImageToStream(GpImage *image, IStream* stream, GDIPCONST CLSID* clsidEncoder, GDIPCONST EncoderParameters* encoderParams);
GpStatus WINGDIPAPI GdipDisposeImage(GpImage *image);
GpStatus WINGDIPAPI GdipDeleteGraphics(GpGraphics *graphics);
GpStatus WINGDIPAPI GdipGetPropertyItemSize(GpImage *image, PROPID propId, UINT *size);
GpStatus WINGDIPAPI GdipGetPropertyItem(GpImage *image, PROPID propId, UINT propSize, PropertyItem *buffer);
GpStatus WINGDIPAPI GdipSetPropertyItem(GpImage *image, GDIPCONST PropertyItem *item);
GpStatus WINGDIPAPI GdipImageRotateFlip(GpImage *image, RotateFlipType rfType);
GpStatus WINGDIPAPI GdipGetImageRawFormat(GpImage *image, GUID *format);
GpStatus WINGDIPAPI GdipGetImageEncodersSize(UINT *numEncoders, UINT *size);
GpStatus WINGDIPAPI GdipGetImageEncoders(UINT numEncoders, UINT size, ImageCodecInfo *encoders);
