#pragma once

typedef enum {
    PHOTO_DEFAULT,
    PHOTO_MISSING,
    PHOTO_RIGHT,
    PHOTO_ERROR,
    PHOTO_WARN
} PHOTOTYPE;

typedef struct {
    LPTSTR szFilename;
    LPTSTR szSubPath;
    LARGE_INTEGER filesize;
    PSYSTEMTIME pStFileTime;
    PSYSTEMTIME pStExifTime;
    PSYSTEMTIME pStFilenameTime;
    PHOTOTYPE type;
} PHOTO;

typedef struct {
    LPTSTR szPath;
    PHOTO **pPhotos;
    HGLOBAL hPhotos;
    int iCount;
    int iSize;
} PHOTOLIB;

typedef enum {
    AUTOPROC_ALL,
    AUTOPROC_FILE,
    AUTOPROC_EXIF,
} AUTOPROCTYPE;

extern PHOTOLIB gPhotoLib;

BOOL FindPhotos(LPCTSTR szPath);
void SortPhotos(int idx, BOOL isAscending);
void ReloadPhotos(int *done);
void AutoProcPhotos(int *done, AUTOPROCTYPE type);

#define PSYSTEMTIME_EQUAL(a, b) ((a)->wYear == (b)->wYear \
                                 && (a)->wMonth == (b)->wMonth \
                                 && (a)->wDay == (b)->wDay \
                                 && (a)->wHour == (b)->wHour \
                                 && (a)->wMinute == (b)->wMinute \
                                 && (a)->wSecond == (b)->wSecond)
