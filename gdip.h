#pragma once

void InitGdip(void);
void DeinitGdip(void);
BOOL GdipGetPropertyTagDateTime(LPCTSTR szFilepath, LPSTR *pSzBuf);
void GdipDrawImage(void *data, HDC hdc, RECT * rc, BOOL fit);
