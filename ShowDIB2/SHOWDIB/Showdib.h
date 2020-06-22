
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright 1993 - 1998 Microsoft Corporation.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
\******************************************************************************/
#include "..\SDVers.h"  // establish the VERSION and FEATURES supported

#include <windows.h>
#include <io.h>
#include <stdio.h>
#include <commdlg.h>
//#include	"res.h"
#include	"..\resource.h"
#include "..\diagfile.h"   // for sprtf() access
#include	"dlgopen.h"
#include "preview.h"
#ifdef   ADDRGB2
#include "rgb.h"
#endif   // ADDRGB2

#ifndef  _Showdib_HH
#define  _Showdib_HH

#ifdef  __cplusplus
extern "C" {
#endif

/****************************************************************************
 *                                                                          *
 *  FILE        : SHOWDIB.H                                                 *
 *                                                                          *
 *  DESCRIPTION : Header/include file for ShowDIB example.                  *
 *                                                                          *
 ****************************************************************************/

typedef    POINTS       MPOINT         ;

#ifdef _WIN32
#define GET_WM_ACTIVATE_STATE(wp, lp)   LOWORD(wp)
#define GET_WM_VSCROLL_CODE(wp, lp)     LOWORD(wp)
#define GET_WM_VSCROLL_POS(wp, lp)      HIWORD(wp)
#define GET_WM_HSCROLL_CODE(wp, lp)     LOWORD(wp)
#define GET_WM_HSCROLL_POS(wp, lp)      HIWORD(wp)
#define GET_EM_SETSEL_MPS(iStart, iEnd) (UINT)(iStart), (LONG)(iEnd)
#define GET_WM_COMMAND_CMD(wp, lp)      HIWORD(wp)
#define MAKEMPOINT(l)                   (*((MPOINT *)&(l)))
#define HUGE_T
#else
#define GET_WM_ACTIVATE_STATE(wp, lp)               (wp)
#define GET_WM_HSCROLL_CODE(wp, lp)                 (wp)
#define GET_WM_HSCROLL_POS(wp, lp)                  LOWORD(lp)
#define GET_WM_VSCROLL_CODE(wp, lp)                 (wp)
#define GET_WM_VSCROLL_POS(wp, lp)                  LOWORD(lp)
#define GET_EM_SETSEL_MPS(iStart, iEnd) 0, MAKELONG(iStart, iEnd)
#define GET_WM_COMMAND_CMD(wp, lp)      HIWORD(lp)
#define MAKEMPOINT(l)                   (*((MPOINT FAR *)&(l)))
#define HUGE_T         huge
#endif

/* Macro to restrict a given value to an upper or lower boundary value */
#define BOUND(x,min,max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/* Macro to swap two values */
#define SWAP(x,y)   ((x)^=(y)^=(x)^=(y))

/* Macro to find the minimum of two values */
#define MIN(x,y) (((x) <= (y)) : x ? y)

/* Macros to display/remove hourglass cursor for lengthy operations */
#define StartWait() hcurSave = SetCursor(LoadCursor(NULL,IDC_WAIT))
#define EndWait()   SetCursor(hcurSave)

/* WinFlags, __WinFlags is a ABS external, thus the need for the & */
#define WinFlags    ((WORD)(&_WinFlags))

#define MINBAND         50     /* Minimum band size used by the program */
#define BANDINCREMENT   20     /* Decrement for band size while trying  */
                               /* to determine optimum band size.       */

/* Bit values for the DIB attributes flag (fFileOptions). Also used as control
 * IDs for the radiobuttons for DIB bitcount in the File/Open dialog
 */
#define F_1BPP      DLGOPEN_1BPP
#define F_4BPP      DLGOPEN_4BPP
#define F_8BPP      DLGOPEN_8BPP
#define F_24BPP     DLGOPEN_24BPP

/* Bit values for the DIB attributes flag (fFileOptions), also used as control
 * IDs for the radiobuttons for DIB compression type in the File/Open dialog
 */
#define F_RLE4      DLGOPEN_RLE4
#define F_RLE8      DLGOPEN_RLE8
#define F_RGB       DLGOPEN_RGB

#ifdef NOT_IN_STDIO
/* flags for _lseek */
#define  SEEK_CUR 1
#define  SEEK_END 2
#define  SEEK_SET 0
#endif

/* struct to be passed in for the SETDIBSCALING printer escape */
typedef struct {
        SHORT ScaleMode;
        SHORT dx, dy;
} DIBPARAMS;

extern HWND hWndApp;                   /* The handle to the app. window     */

#define MAXREAD  32768                 /* Number of bytes to be read during */
                                       /* each read operation.              */

/* Header signatutes for various resources */
#define BFT_ICON   0x4349   /* 'IC' */
#define BFT_BITMAP 0x4d42   /* 'BM' */
#define BFT_CURSOR 0x5450   /* 'PT' */

/* macro to determine if resource is a DIB */
#define ISDIB(bft) ((bft) == BFT_BITMAP)

/* Macro to align given value to the closest DWORD (unsigned long ) */
#define ALIGNULONG(i)   ((i+3)/4*4)

/* Macro to determine to round off the given value to the closest byte */
#define WIDTHBYTES(i)   ((i+31)/32*4)

#define PALVERSION      0x300
#define MAXPALETTE      256       /* max. # supported palette entries */

/***************** GLOBAL VARIABLES *************************/

extern TCHAR          achFileName[264]; /* File pathname                       */
extern DWORD          dwOffset;     /* Current position if DIB file pointer    */
extern RECT           rcClip;       /* Current clip rectangle.                 */
extern BOOL           fPalColors;   /* TRUE if the current DIB's color table   */
                                    /* contains palette indexes not rgb values */
extern BOOL           bDIBToDevice; /* Use SetDIBitsToDevice() to BLT data.    */
extern BOOL           bLegitDraw;   /* We have a valid bitmap to draw          */
extern WORD           wTransparent; /* Mode of DC                              */
extern CHAR           szAppName[];  /* App. name                               */
extern HPALETTE       hpalCurrent;  /* Handle to current palette               */
extern HANDLE         hdibCurrent;  /* Handle to current memory DIB            */
extern HBITMAP        hbmCurrent;   /* Handle to current memory BITMAP         */
extern HANDLE         hbiCurrent;   /* Handle to current bitmap info struct    */
extern DWORD          dwStyle;      /* Style bits of the App. window           */
extern HINSTANCE      hInst;        /* Global instance handle                  */
extern TCHAR          gszTmpBuf[];  // [1024] pad

/***************** FUNCTION DECLARATIONS *******************/

DWORD PASCAL lread(INT fh, VOID FAR *pv, DWORD ul);
DWORD PASCAL lwrite(INT fh, VOID FAR *pv, DWORD ul);

/***********************************************************/
/* Declarations of functions used in showdib.c module      */
/***********************************************************/

LRESULT     APIENTRY WndProc2  (HWND, UINT, WPARAM, LPARAM) ;
BOOL               MenuCommand (HWND hWnd, WPARAM wParam);
INT                InitDIB (HWND);
VOID               FreeDib (VOID);
BOOL     CenterDialog( HWND hChild, HWND hParent );


/***********************************************************/
/* Declarations of functions used in dib.c module          */
/***********************************************************/

HANDLE      OpenDIB (LPTSTR pszFile);
BOOL        WriteDIB (LPSTR szFile,HANDLE hdib);
WORD        PaletteSize (VOID FAR * pv);
WORD        DibNumColors (VOID FAR * pv);
HPALETTE    CreateDibPalette (HANDLE hdib);
HPALETTE    CreateBIPalette (LPBITMAPINFOHEADER lpbi);
HANDLE      DibFromBitmap (HBITMAP hbm, DWORD biStyle, WORD biBits, HPALETTE hpal);
HBITMAP     BitmapFromDib (HANDLE hdib, HPALETTE hpal);
BOOL        DibBlt (HDC hdc, INT x0, INT y0, INT dx, INT dy, HANDLE hdib, INT x1, INT y1, LONG rop);
INT         StretchDibBlt (HDC hdc, INT x0, INT y0, INT dx, INT dy, HANDLE hdib, INT x1, INT y1, INT dx1, INT dy1, LONG rop);
BOOL        DibInfo (HANDLE hdib,LPBITMAPINFOHEADER lpbi);
HANDLE      ReadDibBitmapInfo (INT fh);
BOOL        DrawBitmap (HDC hdc, INT x, INT y, HBITMAP hbm, DWORD rop);
VOID            ReadBitMapFileHeaderandConvertToDwordAlign(HFILE fh, LPBITMAPFILEHEADER pbf, LPDWORD lpdwoff);
VOID    WriteMapFileHeaderandConvertFromDwordAlignToPacked(HFILE fh, LPBITMAPFILEHEADER pbf);

/***********************************************************/
/* Declarations of functions used in drawdib. c module     */
/***********************************************************/

VOID               PrintDIB (HWND hWnd, HDC hDC, INT x, INT y, INT dx, INT dy);
VOID               AppPaint (HWND hWnd, HDC hDC, INT x, INT y);
INT                ErrMsg (PSTR sz,...);
BOOL               fDialog (INT id, HWND hwnd, FARPROC fpfn);
VOID               BandDIB (HWND hWnd, HDC hDC, INT x, INT y);
BOOL     APIENTRY AppAbout (HWND, UINT, UINT, LONG) ;
HANDLE             CopyHandle (HANDLE h);
VOID               SizeWindow (HWND hWnd);
VOID               GetRealClientRect (HWND hwnd, PRECT lprc);
VOID               SetScrollRanges (HWND hwnd, WORD wid, WORD ht);
VOID               DrawSelect (HDC hdc, BOOL fDraw);
VOID        PASCAL NormalizeRect (RECT *prc);
VOID               TrackMouse (HWND hwnd, MPOINT pt);
HBITMAP            CopyBitmap (HBITMAP hbm);
HPALETTE           CopyPalette (HPALETTE hpal);
HBITMAP            CropBitmap (HBITMAP hbm, PRECT prc);
HANDLE             RenderFormat (INT_PTR cf);
HANDLE             RealizeDibFormat (DWORD biStyle, WORD biBits);

/***********************************************************/
/* Declarations of functions used in the print.c module    */
/***********************************************************/

BOOL    PASCAL InitPrinting (HDC hDC, HWND hWnd, HANDLE hInst, LPSTR msg);
VOID    PASCAL TermPrinting (HDC hDC);
HDC     PASCAL GetPrinterDC (VOID);
HDC     PASCAL GetPrinterDC1 (VOID);

/***********************************************************/
/* Declarations of functions used in the dlgopen.c module  */
/***********************************************************/

INT_PTR APIENTRY DlgfnOpen(    
        HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
);
HANDLE APIENTRY DlgOpenFile (
        HWND            hwndParent,
        CHAR            *szTitleIn,
        DWORD           flags,
        CHAR            *szExtIn,
    CHAR        *szFileNameIn,
    WORD                *pfOpt
);

/*****************************************************/
/* make new sizeof structs to cover dword alignment  */
/*****************************************************/

#define SIZEOF_BITMAPFILEHEADER_PACKED  (   \
    sizeof(WORD) +      /* bfType      */   \
    sizeof(DWORD) +     /* bfSize      */   \
    sizeof(WORD) +      /* bfReserved1 */   \
    sizeof(WORD) +      /* bfReserved2 */   \
    sizeof(DWORD))      /* bfOffBits   */

#ifdef  __cplusplus
//extern "C" {
}
#endif

#endif   // #ifndef  _Showdib_HH

// eof showdib.h
