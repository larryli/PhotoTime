#pragma once

typedef struct {
    LPTSTR szFilename;
    LPTSTR szSubPath;
    LARGE_INTEGER filesize;
    PSYSTEMTIME pStFileTime;
    PSYSTEMTIME pStExifTime;
    PSYSTEMTIME pStFilenameTime;
} PHOTO;

typedef struct {
    LPTSTR szPath;
    PHOTO **pPhotos;
    HGLOBAL hPhotos;
    int iCount;
    int iSize;
} PHOTOLIB;

extern PHOTOLIB gPhotoLib;

BOOL FindPhotos(LPCTSTR szPath);
void SortPhotos(int idx, BOOL isAscending);
void RefreshPhotos(int *done);
void AutoProcPhotos(int *done);
