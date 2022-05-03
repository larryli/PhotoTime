#pragma once

#define PVM_SETPATH (WM_USER + 0)
#define PhotoView_SetPath(hwndCtl, szPath)  ((int)(DWORD)SNDMSG((hwndCtl),PVM_SETPATH,(WPARAM)(LPCTSTR)szPath,0))

HWND CreatePhotoViewWnd(HWND hWndParent, HINSTANCE hInst);
void DestroyPhotoViewWnd(HWND hWndPV);
