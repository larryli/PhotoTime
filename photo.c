#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wchar.h>
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdlib.h>

#include "photo.h"
#include "utils.h"
#include "gdip.h"
#include "parsest.h"

#ifndef PHOTOS_SIZE
#define PHOTOS_SIZE 100
#endif

PHOTOLIB gPhotoLib = {
    .szPath = NULL,
    .pPhotos = NULL,
    .hPhotos = 0,
    .iCount = 0,
    .iSize = 0,
};

static void FreePhotos(void);
static PHOTO *NewPhoto(WIN32_FIND_DATA *pWfd, LPCTSTR szPath, LPCTSTR szSub);
static void FreePhoto(PHOTO *pPhoto);
static BOOL FindFile(HANDLE *phFind, WIN32_FIND_DATA *pWfd, LPCTSTR szPath);

static BOOL IsPhotoFile(LPCTSTR szPath)
{
    static LPCTSTR szExts[] = {
        L"jpg",
        L"jpeg",
    };

    TCHAR szExt[MAX_PATH];
    GetFileExt(szExt, NELEMS(szExt), szPath);
    for (int i = 0; i < (int)NELEMS(szExts); i++)
        if (_wcsicmp(szExt, szExts[i]) == 0)
            return TRUE;
    return FALSE;
}

static BOOL IsIgnoreDirectory(LPCTSTR szPath)
{
    static LPCTSTR szIgnores[] = {
        L".@__thumb",
        L"@Recycle"
    };
    for (int i = 0; i < (int)NELEMS(szIgnores); i++)
        if (_wcsicmp(szPath, szIgnores[i]) == 0)
            return TRUE;
    return FALSE;
}

static BOOL FindPhotoWithSub(LPCTSTR szPath, LPCTSTR szSub)
{
    HANDLE hFind = NULL;
    WIN32_FIND_DATA wfd;
    while (FindFile(&hFind, &wfd, szPath)) {
        if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || (wfd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM))
            continue;
        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (IsIgnoreDirectory(wfd.cFileName))
                continue;
            TCHAR szFind[MAX_PATH], szSubs[MAX_PATH];
            CatFilePath(szFind, NELEMS(szFind), szPath, wfd.cFileName);
            if (szSub)
                CatFilePath(szSubs, NELEMS(szSubs), szSub, wfd.cFileName);
            else
                lstrcpyn(szSubs, wfd.cFileName, MAX_PATH);
            if (!FindPhotoWithSub(szFind, szSubs))
                return FALSE;
            continue;
        }
        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            continue;
        TCHAR szFilePath[MAX_PATH];
        CatFilePath(szFilePath, NELEMS(szFilePath), szPath, wfd.cFileName);
        if (!IsPhotoFile(wfd.cFileName))
            continue;
        PHOTO *pPhoto = NewPhoto(&wfd, szFilePath, szSub);
        if (!pPhoto)
            continue; // skip
        if (gPhotoLib.iCount >= gPhotoLib.iSize) {
            gPhotoLib.iSize += PHOTOS_SIZE;
            gPhotoLib.pPhotos = NULL;
            GlobalUnlock(gPhotoLib.hPhotos);
            HGLOBAL h = GlobalReAlloc(gPhotoLib.hPhotos, sizeof(PHOTO *) * gPhotoLib.iSize,
                                      GMEM_MOVEABLE | GMEM_ZEROINIT);
            if (!h) {
                FreePhoto(pPhoto);
                return FALSE;
            }
            gPhotoLib.hPhotos = h;
            gPhotoLib.pPhotos = (PHOTO **)GlobalLock(gPhotoLib.hPhotos);
        }
        gPhotoLib.pPhotos[gPhotoLib.iCount++] = pPhoto;
    }
    return TRUE;
}

BOOL FindPhoto(LPCTSTR szPath)
{
    FreePhotos();
    int size = lstrlen(szPath) + 1;
    gPhotoLib.szPath = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(TCHAR) * size);
    if (!(gPhotoLib.szPath))
        return FALSE;
    lstrcpyn(gPhotoLib.szPath, szPath, size);
    gPhotoLib.iSize = PHOTOS_SIZE;
    gPhotoLib.hPhotos = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(PHOTO *) * gPhotoLib.iSize);
    if (!gPhotoLib.hPhotos) {
        GlobalFree(gPhotoLib.szPath);
        gPhotoLib.szPath = NULL;
        return FALSE;
    }
    gPhotoLib.pPhotos = (PHOTO **)GlobalLock(gPhotoLib.hPhotos);
    if (FindPhotoWithSub(szPath, NULL))
        return TRUE;
    // FreePhotos();
    return FALSE;
}

typedef struct {
    __cmpfunc_s *asc;
    __cmpfunc_s *desc;
} CMP;

static int AscFilename(const PHOTO **, const PHOTO **, void *);
static int DescFilename(const PHOTO **, const PHOTO **, void *);
static int AscSubDirectory(const PHOTO **, const PHOTO **, void *);
static int DescSubDirectory(const PHOTO **, const PHOTO **, void *);
static int AscSize(const PHOTO **, const PHOTO **, void *);
static int DescSize(const PHOTO **, const PHOTO **, void *);
static int AscFileTime(const PHOTO **, const PHOTO **, void *);
static int DescFileTime(const PHOTO **, const PHOTO **, void *);
static int AscExifTime(const PHOTO **, const PHOTO **, void *);
static int DescExifTime(const PHOTO **, const PHOTO **, void *);
static int AscFilenameTime(const PHOTO **, const PHOTO **, void *);
static int DescFilenameTime(const PHOTO **, const PHOTO **, void *);

static CMP cmps[] = {
    {AscFilename, DescFilename},
    {AscSubDirectory, DescSubDirectory},
    {AscSize, DescSize},
    {AscFileTime, DescFileTime},
    {AscExifTime, DescExifTime},
    {AscFilenameTime, DescFilenameTime},
};

void SortPhotos(int idx, BOOL isAscending)
{
    if (idx < (int)NELEMS(cmps)) {
        __cmpfunc_s *cmpfunc = isAscending ? cmps[idx].asc : cmps[idx].desc;
        (void)qsort_s(gPhotoLib.pPhotos, gPhotoLib.iCount, sizeof(PHOTO *), cmpfunc, NULL);
    }
}

static void FreePhotos(void)
{
    if (gPhotoLib.szPath) {
        GlobalFree(gPhotoLib.szPath);
        gPhotoLib.szPath = NULL;
    }
    if (gPhotoLib.pPhotos) {
        for (int i = 0; i < gPhotoLib.iCount; i++)
            FreePhoto(gPhotoLib.pPhotos[i]);
        gPhotoLib.pPhotos = NULL;
    }
    gPhotoLib.iCount = 0;
    if (gPhotoLib.hPhotos) {
        GlobalUnlock(gPhotoLib.hPhotos);
        GlobalFree(gPhotoLib.hPhotos);
        gPhotoLib.hPhotos = NULL;
    }
    gPhotoLib.iSize = 0;
}

static PHOTO *NewPhoto(WIN32_FIND_DATA *pWfd, LPCTSTR szPath, LPCTSTR szSub)
{
    PHOTO *pPhoto = (PHOTO *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(PHOTO));
    if (!pPhoto)
        return NULL;

    int size = lstrlen(pWfd->cFileName) + 1;
    pPhoto->szFilename = (LPTSTR)GlobalAlloc(GMEM_FIXED, sizeof(TCHAR) * size);
    if (!(pPhoto->szFilename))
        goto failed;
    lstrcpyn(pPhoto->szFilename, pWfd->cFileName, size);

    if (szSub) {
        size = lstrlen(szSub) + 1;
        pPhoto->szSubDirectory = (LPTSTR)GlobalAlloc(GMEM_FIXED, sizeof(TCHAR) * size);
        if (!(pPhoto->szSubDirectory))
            goto failed;
        lstrcpyn(pPhoto->szSubDirectory, szSub, size);
    }

    pPhoto->filesize.LowPart = pWfd->nFileSizeLow;
    pPhoto->filesize.HighPart  = pWfd->nFileSizeHigh;

    pPhoto->pStFileTime = (PSYSTEMTIME)GlobalAlloc(GMEM_FIXED, sizeof(SYSTEMTIME));
    if (!(pPhoto->pStFileTime))
        goto failed;
    FileTimeToLocalSystemTime(&pWfd->ftLastWriteTime, pPhoto->pStFileTime);

    SYSTEMTIME st;
    if (GdipGetTagSystemTime(szPath, &st)) {
        pPhoto->pStExifTime = (PSYSTEMTIME)GlobalAlloc(GMEM_FIXED, sizeof(SYSTEMTIME));
        if (!(pPhoto->pStExifTime))
            goto failed;
        CopyMemory(pPhoto->pStExifTime, &st, sizeof(SYSTEMTIME));
    }
    PARSEST_RESULT result;
    if (ParseStringToSystemTime(pPhoto->szFilename, &st, &result)) {
        PSYSTEMTIME pSt = (pPhoto->pStExifTime) ? pPhoto->pStExifTime : pPhoto->pStFileTime;
        if (pSt) {
            if (result == PARSEST_NO_TIME) {
                st.wHour = pSt->wHour;
                st.wMinute = pSt->wMinute;
                st.wSecond = pSt->wSecond;
            } else if (result == PARSEST_NO_SECOND)
                st.wSecond = pSt->wSecond;
        }
        pPhoto->pStFilenameTime = (PSYSTEMTIME)GlobalAlloc(GMEM_FIXED, sizeof(SYSTEMTIME));
        if (!(pPhoto->pStFilenameTime))
            goto failed;
        CopyMemory(pPhoto->pStFilenameTime, &st, sizeof(SYSTEMTIME));
    }

    return pPhoto;
failed:
    if (pPhoto->szFilename)
        GlobalFree(pPhoto->szFilename);
    if (pPhoto->szSubDirectory)
        GlobalFree(pPhoto->szSubDirectory);
    if (pPhoto->pStFileTime)
        GlobalFree(pPhoto->pStFileTime);
    if (pPhoto->pStExifTime)
        GlobalFree(pPhoto->pStExifTime);
    if (pPhoto->pStFilenameTime)
        GlobalFree(pPhoto->pStFilenameTime);
    GlobalFree(pPhoto);
    return NULL;
}

static void FreePhoto(PHOTO *pPhoto)
{
    if (!pPhoto)
        return;
    if (pPhoto->szFilename)
        GlobalFree(pPhoto->szFilename);
    if (pPhoto->szSubDirectory)
        GlobalFree(pPhoto->szSubDirectory);
    if (pPhoto->pStFileTime)
        GlobalFree(pPhoto->pStFileTime);
    if (pPhoto->pStExifTime)
        GlobalFree(pPhoto->pStExifTime);
    if (pPhoto->pStFilenameTime)
        GlobalFree(pPhoto->pStFilenameTime);
    GlobalFree(pPhoto);
}

static inline BOOL IsFileOrSubDirectory(LPCTSTR szPath)
{
    return wcscmp(szPath, L".") != 0 && wcscmp(szPath, L"..") != 0;
}

static BOOL FindFile(HANDLE *phFind, WIN32_FIND_DATA *pWfd, LPCTSTR szPath)
{
    if (*phFind == NULL) {
        TCHAR szFind[MAX_PATH];
        CatFilePath(szFind, NELEMS(szFind), szPath, L"*");
        HANDLE h = FindFirstFile(szFind, pWfd);
        if (h == INVALID_HANDLE_VALUE)
            return FALSE;
        *phFind = h;
        if (IsFileOrSubDirectory(pWfd->cFileName))
            return TRUE;
    }
    while (FindNextFile(*phFind, pWfd)) {
        if (IsFileOrSubDirectory(pWfd->cFileName))
            return TRUE;
    }
    FindClose(*phFind);
    *phFind = NULL;
    return FALSE;
}

static int AscFilename(const PHOTO **a, const PHOTO **b, void *d)
{
    return _wcsicmp((*a)->szFilename, (*b)->szFilename);
}

static int DescFilename(const PHOTO **a, const PHOTO **b, void *d)
{
    return _wcsicmp((*b)->szFilename, (*a)->szFilename);
}

static int AscSubDirectory(const PHOTO **a, const PHOTO **b, void *d)
{
    if ((*a)->szSubDirectory == NULL) {
        if ((*b)->szSubDirectory == NULL)
            return 0;
        else
            return -1;
    } else if ((*b)->szSubDirectory == NULL)
        return 1;
    else
        return _wcsicmp((*a)->szSubDirectory, (*b)->szSubDirectory);
}

static int DescSubDirectory(const PHOTO **a, const PHOTO **b, void *d)
{
    if ((*a)->szSubDirectory == NULL) {
        if ((*b)->szSubDirectory == NULL)
            return 0;
        else
            return 1;
    } else if ((*b)->szSubDirectory == NULL)
        return -1;
    else
        return _wcsicmp((*b)->szSubDirectory, (*a)->szSubDirectory);
}

static int AscSize(const PHOTO **a, const PHOTO **b, void *d)
{
    if ((*a)->filesize.QuadPart > (*b)->filesize.QuadPart)
        return 1;
    else if ((*a)->filesize.QuadPart == (*b)->filesize.QuadPart)
        return 0;
    else
        return -1;
}

static int DescSize(const PHOTO **a, const PHOTO **b, void *d)
{
    if ((*a)->filesize.QuadPart > (*b)->filesize.QuadPart)
        return -1;
    else if ((*a)->filesize.QuadPart == (*b)->filesize.QuadPart)
        return 0;
    else
        return 1;
}

static int AscFileTime(const PHOTO **a, const PHOTO **b, void *d)
{
    if ((*a)->pStFileTime == NULL) {
        if ((*b)->pStFileTime == NULL)
                return 0;
            else
                return -1;
    } else if ((*b)->pStFileTime == NULL)
        return 1;
    else {
        FILETIME ftA, ftB;
        SystemTimeToFileTime((*a)->pStFileTime, &ftA);
        SystemTimeToFileTime((*b)->pStFileTime, &ftB);
        return CompareFileTime(&ftA, &ftB);
    }
}

static int DescFileTime(const PHOTO **a, const PHOTO **b, void *d)
{
    if ((*a)->pStFileTime == NULL) {
        if ((*b)->pStFileTime == NULL)
                return 0;
            else
                return 1;
    } else if ((*b)->pStFileTime == NULL)
        return -1;
    else {
        FILETIME ftA, ftB;
        SystemTimeToFileTime((*a)->pStFileTime, &ftA);
        SystemTimeToFileTime((*b)->pStFileTime, &ftB);
        return CompareFileTime(&ftB, &ftA);
    }
}

static int AscExifTime(const PHOTO **a, const PHOTO **b, void *d)
{
    if ((*a)->pStExifTime == NULL) {
        if ((*b)->pStExifTime == NULL)
                return 0;
            else
                return -1;
    } else if ((*b)->pStExifTime == NULL)
        return 1;
    else {
        FILETIME ftA, ftB;
        SystemTimeToFileTime((*a)->pStExifTime, &ftA);
        SystemTimeToFileTime((*b)->pStExifTime, &ftB);
        return CompareFileTime(&ftA, &ftB);
    }
}

static int DescExifTime(const PHOTO **a, const PHOTO **b, void *d)
{
    if ((*a)->pStExifTime == NULL) {
        if ((*b)->pStExifTime == NULL)
                return 0;
            else
                return 1;
    } else if ((*b)->pStExifTime == NULL)
        return -1;
    else {
        FILETIME ftA, ftB;
        SystemTimeToFileTime((*a)->pStExifTime, &ftA);
        SystemTimeToFileTime((*b)->pStExifTime, &ftB);
        return CompareFileTime(&ftB, &ftA);
    }
}

static int AscFilenameTime(const PHOTO **a, const PHOTO **b, void *d)
{
    if ((*a)->pStFilenameTime == NULL) {
        if ((*b)->pStFilenameTime == NULL)
                return 0;
            else
                return -1;
    } else if ((*b)->pStFilenameTime == NULL)
        return 1;
    else {
        FILETIME ftA, ftB;
        SystemTimeToFileTime((*a)->pStFilenameTime, &ftA);
        SystemTimeToFileTime((*b)->pStFilenameTime, &ftB);
        return CompareFileTime(&ftA, &ftB);
    }
}

static int DescFilenameTime(const PHOTO **a, const PHOTO **b, void *d)
{
   if ((*a)->pStFilenameTime == NULL) {
        if ((*b)->pStFilenameTime == NULL)
                return 0;
            else
                return 1;
    } else if ((*b)->pStFilenameTime == NULL)
        return -1;
    else {
        FILETIME ftA, ftB;
        SystemTimeToFileTime((*a)->pStFilenameTime, &ftA);
        SystemTimeToFileTime((*b)->pStFilenameTime, &ftB);
        return CompareFileTime(&ftB, &ftA);
    }
}
