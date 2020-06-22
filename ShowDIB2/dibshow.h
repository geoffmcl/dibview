
// dibshow.h
#ifndef	_dibshow_H
#define	_didshow_H

#ifdef  __cplusplus
extern  "C"
{
#endif  // __cplusplus

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern   RECT  rcClip;
extern   int   gxScreen, gyScreen;
extern   CHAR  achFileName[]; // [264];
extern   VOID  FreeDib( VOID );
extern   NPLOGPALETTE   pLogPal;
extern   HDC PASCAL GetPrinterDC1(void);
extern   HDC PASCAL GetPrinterDC(void);
extern   void  ShowMemErr( HWND hWnd );
extern   LRESULT APIENTRY WndProc2( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam );
extern   BOOL MenuCommand( HWND hWnd, WPARAM id );
extern   int   iRasterCaps;
extern   DWORD iLogPixelsX, iLogPixelsY;  // = GetDeviceCaps( hDC, LOGPIXELSY );
extern   BOOL  bPalSupport;
extern   INT InitDIB(HWND hWnd);
extern   HWND     hWndApp; /* Handle to app. window                */
extern   BOOL DibInfo( HANDLE hbi, LPBITMAPINFOHEADER lpbi );
extern   HPALETTE hpalCurrent;   /* Handle to current palette            */
extern   HANDLE   hdibCurrent;   /* Handle to current memory DIB         */
extern   HBITMAP  hbmCurrent;    /* Handle to current memory BITMAP      */
extern   HANDLE   hbiCurrent;    /* Handle to current bitmap info struct */

extern   RECT     rcMonitor;  // size of monitor at CREATE time
extern   DWORD    iCyHScroll; // = GetSystemMetrics (SM_CYHSCROLL);
extern   DWORD    iCxVScroll; // = GetSystemMetrics (SM_CXVSCROLL);
extern   RECT     rcWorkArea; // SPI_GETWORKAREA
extern   DWORD   iCxBorder, iCyBorder; //  = GetSystemMetrics (SM_CYEDGE);

extern   BOOL CenterDialog( HWND hChild, HWND hParent );

// in print.c
extern   BOOL PASCAL InitPrinting(HDC hDC, HWND hWnd, HANDLE hInst, LPSTR msg);
extern   VOID PASCAL TermPrinting(HDC hDC);

// in drawdib.c
extern   VOID PrintDIB( HWND hWnd, HDC hDC, INT x, INT y, INT dx, INT dy );


#ifdef  __cplusplus
}
#endif  // __cplusplus

#endif	/* _dibshow_H */
// eof - dibshow.h