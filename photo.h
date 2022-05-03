#pragma once

typedef struct {
    LPTSTR szFilename;
    LPTSTR szSubDirectory;
    LARGE_INTEGER filesize;
    PSYSTEMTIME pStFileTime;
    PSYSTEMTIME pStExifTime;
    PSYSTEMTIME pStFilenameTime;
} PHOTO;

typedef struct {
    LPTSTR szPath;
    PHOTO **pPs;
    int iCount;
} PHOTOS;

extern PHOTOS gPhotos;

BOOL FindPhoto(LPCTSTR szPath);
void SortPhotos(int idx, BOOL isAscending);
