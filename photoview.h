#pragma once

#define PVM_SETPATH (WM_USER)
#define PhotoView_SetPath(hwndCtl, szPath)  ((int)(DWORD)SNDMSG((hwndCtl),PVM_SETPATH,(WPARAM)(PCTSTR)szPath,0))
#define PVM_GETSIZE (WM_USER + 1)
#define PhotoView_GetSize(hwndCtl, pSize)  ((int)(DWORD)SNDMSG((hwndCtl),PVM_GETSIZE,(WPARAM)(PSIZE)pSize,0))

HWND CreatePhotoViewWnd(HWND hWndParent, HINSTANCE hInst);
void DestroyPhotoViewWnd(HWND hWndPV);
