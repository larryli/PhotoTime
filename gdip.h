#pragma once

void InitGdip(void);
void DeinitGdip(void);
BOOL GdipGetPropertyTagDateTime(LPCTSTR szFilepath, LPSTR *pSzBuf);
void *GdipLoadImage(LPCTSTR szPath);
void GdipDestoryImage(void *data);
BOOL GdipDrawImage(void *data, HDC hdc, RECT * rc);
