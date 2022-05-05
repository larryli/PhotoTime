#pragma once

void InitGdip(void);
void DeinitGdip(void);
BOOL GdipGetTagSystemTime(LPCTSTR szFilepath, PSYSTEMTIME pSt);
void *GdipLoadImage(LPCTSTR szFilePath);
void GdipDestoryImage(void *data);
BOOL GdipDrawImage(void *data, HDC hdc, const RECT * rc);
BOOL GdipSaveImageWithTagSystemTime(LPCTSTR szFilePath, const PSYSTEMTIME pSt);
