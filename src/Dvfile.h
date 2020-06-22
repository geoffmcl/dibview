
// File.h

#ifndef	_FILE_H_
#define	_FILE_H_

// Just in case some necessary items NOT included already ...
#ifndef _mwin32_h
#include "DvWin32.h"
#endif	// _mwin32_h

#ifndef	_DvChild_h_
#include	"DvChild.h"
#endif	// _DvChild_h

//#define	MXDIR		260
#define	MXDIR		(MAX_PATH+16)

//--------------  DIB header Marker Define -------------------------

#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')	/* Simple "BM" ... */


/*--------------  MyRead Function Define ---------------------------*/

// When we read in a DIB, we read it in in chunks.  We read half a segment
//  at a time.  This way we insure that we don't cross any segment
//  boundries in _lread() during a read.  We don't read in a full segment
//  at a time, since _lread takes some "int" type parms instead of
//  WORD type params (it'd work, but the compiler would give you warnings)...

#define BYTES_PER_READ  32767

/*--------------  Define for PM DIB  -------------------------------*/
// The constants for RGB, RLE4, RLE8 are already defined inside
// of Windows.h

#define BI_PM       3L

/*-------------- Magic numbers -------------------------------------*/
// Maximum length of a filename for DOS is 128 characters.
// But in Windows 32-bit it is 260 bytes!!!
#define MAX_FILENAME 264	// MAX_PATH plus 4

/*--------------  TypeDef Structures -------------------------------*/

typedef struct tagINFOSTRUCT {		/* is */

	HWND		is_hMDIWnd;
	HANDLE	is_hDibInfo;
	HANDLE	is_hdlDib;		// Handle of DIB read in
	HPALETTE	is_hPal;		// Bitmap's palette. (if exists)
	HBITMAP	is_hBitmap;		// Handle to the DDB. (if exists)
	DWORD		is_cbWidth;		// Width
	DWORD		is_cbHeight;	// Height
	DWORD		is_cbColors;	// Color count
	DWORD		is_dwType;	// One of BI_RGB, BI_RLE4, BI_RLE8, BI_PM
	DWORD		is_dwBits;	// One of 1, 4, 8 or 24.
	BOOL		is_bOpenWnd;	// Open a CHILD Window
	UINT		is_uType, is_uRType, is_uBits, is_uRBits;
	DWORD		is_Res1, is_Res2, is_Res3;	// Just some RESERVED
	TCHAR		is_szCompress[8];	// Compression type
	TCHAR		is_szType[16];	// Type

	TCHAR		is_szName[MAX_PATH+8];	// File Name
	TCHAR		is_szTitle[MAX_PATH+8];	// File Title

}INFOSTRUCT, * PI;

typedef INFOSTRUCT FAR * LPINFOSTRUCT;
#define		NULPINFOSTRUCT(p)\
		dv_fmemset( p, 0, sizeof(INFOSTRUCT) )

typedef WORD (CALLBACK* FARHOOK)(HWND,UINT,WPARAM,LPARAM);
/*--------------  Global Variables ---------------------------------*/

//extern char szFileName[256];        // Filename of DIB (valid iff (hDIB))
//extern char szRDirName[MXDIR+1];
//extern char szWDirName[MXDIR+1];

/*--------------  Local Function Prototypes ------------------------*/

HANDLE   OpenDIBFile       (LPSTR szFileName);
void	OpenDIBFile2		( PRDIB );
BOOL     GetFileName       (LPSTR, WORD);
HANDLE   ReadDIBFile       ( PRDIB );
BOOL     MyRead            (HANDLE, LPSTR, DWORD);
int      CheckIfFileExists (LPSTR);
int      GetDibInfo        (LPSTR, LPINFOSTRUCT, BOOL);
BOOL     Dv_IDM_SAVE       ( HWND );
WORD     ExtractDibType    (LPINFOSTRUCT);
WORD     ExtractDibBits    (LPINFOSTRUCT);
HANDLE   WinDibFromBitmap  (HBITMAP, DWORD, WORD, HPALETTE);
HANDLE   PMDibFromBitmap   (HBITMAP, DWORD, WORD, HPALETTE);
extern	BOOL     WriteDIB          (LPSTR, HANDLE);	// DvWrite.c
VOID     ParseCommandLine  (LPSTR);
HANDLE   GetDIB            (LPSTR, DWORD);
HANDLE   GetDIB2           ( PRDIB );
DWORD PASCAL lwrite        (int, VOID MLPTR, DWORD);

/*--------------  Exported Function Prototypes ---------------------*/
// Common
#ifdef   WIN32
UINT_PTR CALLBACK FILEOPENHOOKPROC( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
UINT_PTR CALLBACK FILESAVEHOOKPROC (HWND, UINT, WPARAM, LPARAM);
#else // !WIN32
//EXPORT32
//BOOL MLIBCALL FILEOPENHOOKPROC (HWND, UINT, WPARAM, LPARAM);
//EXPORT32
//BOOL MLIBCALL FILESAVEHOOKPROC (HWND, UINT, WPARAM, LPARAM);
#endif   // #ifdef   WIN32

#ifdef	WIN32
BOOL MLIBCALL INFODLGPROC      (HWND, UINT, WPARAM, LPARAM);
BOOL MLIBCALL VIEWDLGPROC      (HWND, UINT, WPARAM, LPARAM);
#else
BOOL MLIBCALL INFODLGPROC      (HWND, WORD, WORD, LONG);
BOOL MLIBCALL VIEWDLGPROC      (HWND, WORD, WORD, LONG);
#endif

// Moved to File2.c - April, 1997
// ==============================
extern	BOOL	IsJPEG( LPSTR );
extern	BOOL	IsGIF( LPSTR );
extern	BOOL	IsBMP( LPSTR );
extern	BOOL	IsRLE( LPSTR );
extern	BOOL	IsPPM( LPSTR );
extern	BOOL	IsTARGA( LPSTR );

// In DvNewF.c
// DVGetFPath( lpf, gszDrive, gszDir, gszFname, gszExt ); /* Get components */
extern	void	DVGetFPath( LPTSTR, LPTSTR, LPTSTR, LPTSTR, LPTSTR );

extern   BOOL  DVGetFullName( LPTSTR lpd, LPTSTR lpf );
extern   int	DVGetFileTitle( LPTSTR lpfull, LPTSTR lptitle );
extern   VOID  DVGetFullName2( LPTSTR lpd, LPTSTR lpf ); // combine get FULL and TITLE

extern   BOOL	DVGetNewName( LPTSTR lpnf );
extern   VOID  DVNextRDName( PRDIB prd, LPTSTR pfm, DWORD dwn );
extern   VOID  DVNextRDName2( PRDIB prd, LPTSTR pfm, PDWORD pdwn, PBOOL pChg );

extern BOOL gbAspect, gbKeepAspect, gbChgKeepAsp;

// some macro help
#define  CDI(a,b) CheckDlgButton( hDlg, a, ( b ? BST_CHECKED : BST_UNCHECKED ) )
//      ISCDI( IDC_KEEPASPECT, &gbKeepAspect, &gbChgKeepAsp );
#define  ISCDI(a,b,c) ToggleBool( hDlg, a, b, c )

#endif	// _FILE_H_

// eof
