#pragma once
// From fGdiPlusFlat.h

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

#define PropertyTagTypeASCII 2
#define PropertyTagDateTime 0x0132

typedef struct {
    PROPID id;
    ULONG length;
    WORD type;
    VOID* value;
} PropertyItem;

GpStatus WINAPI GdiplusStartup(ULONG_PTR *token, GdiplusStartupInput *input, GdiplusStartupOutput *output);
VOID WINAPI GdiplusShutdown(ULONG_PTR token);

GpStatus WINGDIPAPI GdipCreateFromHDC(HDC hdc, GpGraphics **graphics);
GpStatus WINGDIPAPI GdipDrawImageRectI(GpGraphics *graphics, GpImage *image, INT x, INT y, INT width, INT height);
GpStatus WINGDIPAPI GdipGetImageWidth(GpImage *image, UINT *width);
GpStatus WINGDIPAPI GdipGetImageHeight(GpImage *image, UINT *height);
GpStatus WINGDIPAPI GdipDisposeImage(GpImage *image);
GpStatus WINGDIPAPI GdipDeleteGraphics(GpGraphics *graphics);
GpStatus WINGDIPAPI GdipLoadImageFromFile(GDIPCONST WCHAR* filename, GpImage **image);
GpStatus WINGDIPAPI GdipGetPropertyItemSize(GpImage *image, PROPID propId, UINT* size);
GpStatus WINGDIPAPI GdipGetPropertyItem(GpImage *image, PROPID propId,UINT propSize, PropertyItem* buffer);
