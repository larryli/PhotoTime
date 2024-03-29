#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define __STDC_WANT_LIB_EXT1__ 1
#include <tchar.h>
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
static PHOTO *NewPhoto(WIN32_FIND_DATA *pWfd, PCTSTR szPath, PCTSTR szSub);
static void FreePhoto(PHOTO *pPhoto);
static BOOL FindFile(HANDLE *phFind, WIN32_FIND_DATA *pWfd, PCTSTR szPath);

static BOOL IsPhotoFile(PCTSTR szPath)
{
    static PCTSTR szExts[] = {
        L"jpg",
        L"jpeg",
    };
    PCTSTR szExt = GetFileExt(szPath);
    if (szExt)
        for (int i = 0; i < (int)NELEMS(szExts); i++)
            if (_wcsicmp(szExt, szExts[i]) == 0)
                return TRUE;
    return FALSE;
}

static BOOL IsIgnoreDirectory(PCTSTR szPath)
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

static BOOL FindPhotoWithSub(PCTSTR szPath, PCTSTR szSub)
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
                (void)_tcscpy_s(szSubs, NELEMS(szSubs), wfd.cFileName);
            ASSERT_FALSE(FindPhotoWithSub(szFind, szSubs));
            continue;
        }
        if (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            continue;
        TCHAR szFilePath[MAX_PATH];
        CatFilePath(szFilePath, NELEMS(szFilePath), szPath, wfd.cFileName);
        ASSERT_CONTINUE(IsPhotoFile(wfd.cFileName));
        PHOTO *pPhoto = NewPhoto(&wfd, szFilePath, szSub);
        ASSERT_CONTINUE(pPhoto); // skip
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

BOOL FindPhotos(PCTSTR szPath)
{
    FreePhotos();
    int size = lstrlen(szPath) + 1;
    gPhotoLib.szPath = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(TCHAR) * size);
    ASSERT_FALSE(gPhotoLib.szPath);
    (void)_tcscpy_s(gPhotoLib.szPath, size, szPath);
    gPhotoLib.iSize = PHOTOS_SIZE;
    gPhotoLib.hPhotos = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(PHOTO *) * gPhotoLib.iSize);
    if (!gPhotoLib.hPhotos) {
        GlobalFree(gPhotoLib.szPath);
        gPhotoLib.szPath = NULL;
        return FALSE;
    }
    gPhotoLib.pPhotos = (PHOTO **)GlobalLock(gPhotoLib.hPhotos);
    return FindPhotoWithSub(szPath, NULL);
}

static void FixFilenameTime(PSYSTEMTIME pSt, const PARSEST_RESULT result, const PHOTO *pPhoto)
{
    ASSERT_VOID(pPhoto);
    PSYSTEMTIME pSt2 = (pPhoto->pStExifTime) ? pPhoto->pStExifTime : pPhoto->pStFileTime;
    ASSERT_VOID(pSt2);
    if (result == PARSEST_NO_TIME) {
        pSt->wHour = pSt2->wHour;
        pSt->wMinute = pSt2->wMinute;
        pSt->wSecond = pSt2->wSecond;
    } else if (result == PARSEST_NO_SECOND)
        pSt->wSecond = pSt2->wSecond;
}

void ReloadPhotos(int *done)
{
    ASSERT_VOID(gPhotoLib.iCount > 0);
    ASSERT_VOID(gPhotoLib.pPhotos);
    for (int i = 0; i < gPhotoLib.iCount; i++) {
        PHOTO *pPhoto = gPhotoLib.pPhotos[i];
        ASSERT_CONTINUE(pPhoto);
        pPhoto->type = PHOTO_DEFAULT;
        TCHAR szPath[MAX_PATH] = L"";
        CatFilePath(szPath, NELEMS(szPath), gPhotoLib.szPath, pPhoto->szSubPath);
        CatFilePath(szPath, NELEMS(szPath), szPath, pPhoto->szFilename);

        HANDLE hFile = CreateFile(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            pPhoto->type = PHOTO_MISSING;
            pPhoto->filesize.LowPart = 0;
            pPhoto->filesize.HighPart = 0;
            if (pPhoto->pStFileTime) {
                GlobalFree(pPhoto->pStFileTime);
                pPhoto->pStFileTime = NULL;
            }
            if (pPhoto->pStExifTime) {
                GlobalFree(pPhoto->pStExifTime);
                pPhoto->pStExifTime = NULL;
            }
            if (pPhoto->pStFilenameTime) {
                GlobalFree(pPhoto->pStFilenameTime);
                pPhoto->pStFilenameTime = NULL;
            }
            goto update;
        }
        if (!GetFileSizeEx(hFile, &pPhoto->filesize)) {
            pPhoto->filesize.LowPart = 0;
            pPhoto->filesize.HighPart = 0;
        }

        FILETIME ft;
        if (GetFileTime(hFile, NULL, NULL, &ft)) {
            if (!(pPhoto->pStFileTime))
                pPhoto->pStFileTime = (PSYSTEMTIME)GlobalAlloc(GMEM_FIXED, sizeof(SYSTEMTIME));
            if (pPhoto->pStFileTime)
                FileTimeToLocalSystemTime(&ft, pPhoto->pStFileTime);
        } else if (pPhoto->pStFileTime) {
            GlobalFree(pPhoto->pStFileTime);
            pPhoto->pStFileTime = NULL;
            pPhoto->type = PHOTO_MISSING;
        }
        CloseHandle(hFile);

        SYSTEMTIME st;
        if (GdipGetTagSystemTime(szPath, &st)) {
            if (!(pPhoto->pStExifTime))
                pPhoto->pStExifTime = (PSYSTEMTIME)GlobalAlloc(GMEM_FIXED, sizeof(SYSTEMTIME));
            if (pPhoto->pStExifTime)
                CopyMemory(pPhoto->pStExifTime, &st, sizeof(SYSTEMTIME));
            if (pPhoto->type == PHOTO_DEFAULT)
                pPhoto->type = PSYSTEMTIME_EQUAL(pPhoto->pStExifTime, pPhoto->pStFileTime)
                               ? PHOTO_RIGHT : PHOTO_ERROR;
        } else if (pPhoto->pStExifTime) {
            GlobalFree(pPhoto->pStExifTime);
            pPhoto->pStExifTime = NULL;
        }

        PARSEST_RESULT result;
        if (ParseStringToSystemTime(pPhoto->szFilename, &st, &result)) {
            FixFilenameTime(&st, result, pPhoto);
            if (!(pPhoto->pStFilenameTime))
                pPhoto->pStFilenameTime = (PSYSTEMTIME)GlobalAlloc(GMEM_FIXED, sizeof(SYSTEMTIME));
            if (pPhoto->pStFilenameTime)
                CopyMemory(pPhoto->pStFilenameTime, &st, sizeof(SYSTEMTIME));
            if (pPhoto->type == PHOTO_DEFAULT)
                pPhoto->type = PSYSTEMTIME_EQUAL(pPhoto->pStFilenameTime, pPhoto->pStFileTime)
                               ? PHOTO_RIGHT : PHOTO_WARN;
        } else if (pPhoto->pStFilenameTime) {
            GlobalFree(pPhoto->pStFilenameTime);
            pPhoto->pStFilenameTime = NULL;
        }
update:
        if (done)
            *done = i; // Updata
    }
}

void AutoProcPhotos(int *done, AUTOPROCTYPE type)
{
    BOOL bFile = (type == AUTOPROC_ALL || type == AUTOPROC_FILE);
    BOOL bExif = (type == AUTOPROC_ALL || type == AUTOPROC_EXIF);
    ASSERT_VOID(gPhotoLib.iCount > 0);
    ASSERT_VOID(gPhotoLib.pPhotos);
    for (int i = 0; i < gPhotoLib.iCount; i++) {
        PHOTO *pPhoto = gPhotoLib.pPhotos[i];
        ASSERT_CONTINUE(pPhoto);
        ASSERT_CONTINUE(pPhoto->pStFileTime);
        TCHAR szPath[MAX_PATH] = L"";
        CatFilePath(szPath, NELEMS(szPath), gPhotoLib.szPath, pPhoto->szSubPath);
        CatFilePath(szPath, NELEMS(szPath), szPath, pPhoto->szFilename);

        if (pPhoto->pStExifTime) {
            if (bFile && !PSYSTEMTIME_EQUAL(pPhoto->pStExifTime, pPhoto->pStFileTime)) {
                FILETIME ftTime;
                if (!LocalSystemTimeToFileTime(pPhoto->pStExifTime, &ftTime))
                    goto update;
                HANDLE hFile = CreateFile(szPath, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile == INVALID_HANDLE_VALUE)
                    goto update;
                if (SetFileTime(hFile, &ftTime, NULL, &ftTime)) {
                    CopyMemory(pPhoto->pStFileTime, pPhoto->pStExifTime, sizeof(SYSTEMTIME));
                    pPhoto->type = PHOTO_RIGHT;
                }
                CloseHandle(hFile);
            }
        } else if (pPhoto->pStFilenameTime) {
            if (bExif && GdipSaveImageWithTagSystemTime(szPath, pPhoto->pStFilenameTime)) {
                if (!(pPhoto->pStExifTime))
                    pPhoto->pStExifTime = (PSYSTEMTIME)GlobalAlloc(GMEM_FIXED, sizeof(SYSTEMTIME));
                if (pPhoto->pStExifTime)
                    CopyMemory(pPhoto->pStExifTime, pPhoto->pStFilenameTime, sizeof(SYSTEMTIME));

                FILETIME ftTime;
                PHOTOTYPE type = PHOTO_ERROR;
                if (bFile) { // update sync
                    if (!LocalSystemTimeToFileTime(pPhoto->pStFilenameTime, &ftTime))
                        goto update;
                    type = PHOTO_RIGHT;
                } else {     // keep origin
                    if (!LocalSystemTimeToFileTime(pPhoto->pStFileTime, &ftTime))
                        goto update;
                    type = PSYSTEMTIME_EQUAL(pPhoto->pStFileTime, pPhoto->pStFilenameTime)
                           ? PHOTO_RIGHT : PHOTO_ERROR;
                }
                HANDLE hFile = CreateFile(szPath, GENERIC_READ | FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile == INVALID_HANDLE_VALUE)
                    goto update;
                if (SetFileTime(hFile, &ftTime, NULL, &ftTime)) {
                    CopyMemory(pPhoto->pStFileTime, pPhoto->pStFilenameTime, sizeof(SYSTEMTIME));
                    pPhoto->type = type;
                } else
                    pPhoto->type = PHOTO_ERROR; // gdip write time
                if (!GetFileSizeEx(hFile, &pPhoto->filesize)) { // update filesize after update exif
                    pPhoto->filesize.LowPart = 0;
                    pPhoto->filesize.HighPart = 0;
                }
                CloseHandle(hFile);
            } else if (bFile) {
                FILETIME ftTime;
                if (!LocalSystemTimeToFileTime(pPhoto->pStFilenameTime, &ftTime))
                    goto update;
                HANDLE hFile = CreateFile(szPath, FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile == INVALID_HANDLE_VALUE)
                    goto update;
                if (SetFileTime(hFile, &ftTime, NULL, &ftTime)) {
                    CopyMemory(pPhoto->pStFileTime, pPhoto->pStFilenameTime, sizeof(SYSTEMTIME));
                    pPhoto->type = PHOTO_RIGHT;
                }
                CloseHandle(hFile);
            }
        }
update:
        if (done)
            *done = i; // Updata
    }
}

typedef struct {
    __cmpfunc_s *asc;
    __cmpfunc_s *desc;
} CMP;

static int __cdecl AscFilename(const PHOTO **, const PHOTO **, void *);
static int __cdecl DescFilename(const PHOTO **, const PHOTO **, void *);
static int __cdecl AscSubDirectory(const PHOTO **, const PHOTO **, void *);
static int __cdecl DescSubDirectory(const PHOTO **, const PHOTO **, void *);
static int __cdecl AscSize(const PHOTO **, const PHOTO **, void *);
static int __cdecl DescSize(const PHOTO **, const PHOTO **, void *);
static int __cdecl AscFileTime(const PHOTO **, const PHOTO **, void *);
static int __cdecl DescFileTime(const PHOTO **, const PHOTO **, void *);
static int __cdecl AscExifTime(const PHOTO **, const PHOTO **, void *);
static int __cdecl DescExifTime(const PHOTO **, const PHOTO **, void *);
static int __cdecl AscFilenameTime(const PHOTO **, const PHOTO **, void *);
static int __cdecl DescFilenameTime(const PHOTO **, const PHOTO **, void *);

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
    ASSERT_VOID(gPhotoLib.iCount > 0);
    ASSERT_VOID(gPhotoLib.pPhotos);
    ASSERT_VOID(idx >= 0 && idx < (int)NELEMS(cmps));
    __cmpfunc_s *cmpfunc = isAscending ? cmps[idx].asc : cmps[idx].desc;
    (void)qsort_s(gPhotoLib.pPhotos, gPhotoLib.iCount, sizeof(PHOTO *), cmpfunc, NULL);
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
    ASSERT_NULL(pPhoto);
    pPhoto->type = PHOTO_DEFAULT;

    int size = lstrlen(pWfd->cFileName) + 1;
    pPhoto->szFilename = (LPTSTR)GlobalAlloc(GMEM_FIXED, sizeof(TCHAR) * size);
    ASSERT_FAILED(pPhoto->szFilename);
    (void)_tcscpy_s(pPhoto->szFilename, size, pWfd->cFileName);

    if (szSub) {
        size = lstrlen(szSub) + 1;
        pPhoto->szSubPath = (LPTSTR)GlobalAlloc(GMEM_FIXED, sizeof(TCHAR) * size);
        ASSERT_FAILED(pPhoto->szSubPath);
        (void)_tcscpy_s(pPhoto->szSubPath, size, szSub);
    }

    pPhoto->filesize.LowPart = pWfd->nFileSizeLow;
    pPhoto->filesize.HighPart = pWfd->nFileSizeHigh;

    pPhoto->pStFileTime = (PSYSTEMTIME)GlobalAlloc(GMEM_FIXED, sizeof(SYSTEMTIME));
    ASSERT_FAILED(pPhoto->pStFileTime);
    FileTimeToLocalSystemTime(&pWfd->ftLastWriteTime, pPhoto->pStFileTime);

    SYSTEMTIME st;
    if (GdipGetTagSystemTime(szPath, &st)) {
        pPhoto->pStExifTime = (PSYSTEMTIME)GlobalAlloc(GMEM_FIXED, sizeof(SYSTEMTIME));
        ASSERT_FAILED(pPhoto->pStExifTime);
        CopyMemory(pPhoto->pStExifTime, &st, sizeof(SYSTEMTIME));
        if (pPhoto->type == PHOTO_DEFAULT)
            pPhoto->type = PSYSTEMTIME_EQUAL(pPhoto->pStExifTime, pPhoto->pStFileTime)
                           ? PHOTO_RIGHT : PHOTO_ERROR;
    }

    PARSEST_RESULT result;
    if (ParseStringToSystemTime(pPhoto->szFilename, &st, &result)) {
        FixFilenameTime(&st, result, pPhoto);
        pPhoto->pStFilenameTime = (PSYSTEMTIME)GlobalAlloc(GMEM_FIXED, sizeof(SYSTEMTIME));
        ASSERT_FAILED(pPhoto->pStFilenameTime);
        CopyMemory(pPhoto->pStFilenameTime, &st, sizeof(SYSTEMTIME));
        if (pPhoto->type == PHOTO_DEFAULT)
            pPhoto->type = PSYSTEMTIME_EQUAL(pPhoto->pStFilenameTime, pPhoto->pStFileTime)
                           ? PHOTO_RIGHT : PHOTO_WARN;
    }

    return pPhoto;

failed:
    if (pPhoto->szFilename)
        GlobalFree(pPhoto->szFilename);
    if (pPhoto->szSubPath)
        GlobalFree(pPhoto->szSubPath);
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
    ASSERT_VOID(pPhoto);
    if (pPhoto->szFilename)
        GlobalFree(pPhoto->szFilename);
    if (pPhoto->szSubPath)
        GlobalFree(pPhoto->szSubPath);
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

static int __cdecl AscFilename(const PHOTO **a, const PHOTO **b, void *d)
{
    return _wcsicmp((*a)->szFilename, (*b)->szFilename);
}

static int __cdecl DescFilename(const PHOTO **a, const PHOTO **b, void *d)
{
    return AscFilename(b, a, d);
}

static int __cdecl AscSubDirectory(const PHOTO **a, const PHOTO **b, void *d)
{
    if ((*a)->szSubPath == NULL) {
        if ((*b)->szSubPath == NULL)
            return 0;
        return -1;
    } else if ((*b)->szSubPath == NULL)
        return 1;
    return _wcsicmp((*a)->szSubPath, (*b)->szSubPath);
}

static int __cdecl DescSubDirectory(const PHOTO **a, const PHOTO **b, void *d)
{
    return AscSubDirectory(b, a, d);
}

static int __cdecl AscSize(const PHOTO **a, const PHOTO **b, void *d)
{
    if ((*a)->filesize.QuadPart > (*b)->filesize.QuadPart)
        return 1;
    else if ((*a)->filesize.QuadPart == (*b)->filesize.QuadPart)
        return 0;
    return -1;
}

static int __cdecl DescSize(const PHOTO **a, const PHOTO **b, void *d)
{
    return AscSize(b, a, d);
}

static int CompareSystemTime(const PSYSTEMTIME pStA, const PSYSTEMTIME pStB)
{
    if (pStA == NULL) {
        if (pStB == NULL)
            return 0;
        return -1;
    } else if (pStB == NULL)
        return 1;
    FILETIME ftA, ftB;
    if (SystemTimeToFileTime(pStA, &ftA)) {
        if (SystemTimeToFileTime(pStB, &ftB))
            return CompareFileTime(&ftA, &ftB);
        return 1;
    } else if (SystemTimeToFileTime(pStB, &ftB))
        return -1;
    return 0;
}

static int __cdecl AscFileTime(const PHOTO **a, const PHOTO **b, void *d)
{
    return CompareSystemTime((*a)->pStFileTime, (*b)->pStFileTime);
}

static int __cdecl DescFileTime(const PHOTO **a, const PHOTO **b, void *d)
{
    return CompareSystemTime((*b)->pStFileTime, (*a)->pStFileTime);
}

static int __cdecl AscExifTime(const PHOTO **a, const PHOTO **b, void *d)
{
    return CompareSystemTime((*a)->pStExifTime, (*b)->pStExifTime);
}

static int __cdecl DescExifTime(const PHOTO **a, const PHOTO **b, void *d)
{
    return CompareSystemTime((*b)->pStExifTime, (*a)->pStExifTime);
}

static int __cdecl AscFilenameTime(const PHOTO **a, const PHOTO **b, void *d)
{
    return CompareSystemTime((*a)->pStFilenameTime, (*b)->pStFilenameTime);
}

static int __cdecl DescFilenameTime(const PHOTO **a, const PHOTO **b, void *d)
{
    return CompareSystemTime((*b)->pStFilenameTime, (*a)->pStFilenameTime);
}
