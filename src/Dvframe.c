
/* ***********************************************

      File:  DVFRAME.C

   Purpose:  Routines needed to support the frame window, menus, etc.

 Functions:  FRAMEWNDPROC
             Frm_WM_COMMAND
             OpenDIBWindow
             OpenPaletteWindow
             EnableWindowOptionsMenus

  Comments:  

   History:   Date     Reason

            06/01/91   Created
            01/28/92   IDM_PALDIB bug fix : delete COPY of palette
                         returned by CurrentDIBPalette().

   ******************************************** */
#define  COBJMACROS

#include "dv.h"
#include "DvPrint.h"
#include "DvClip.h"
#include "DvCapt.h"
#include <shlobj.h>
//#include <windowsx.h>

//#include "dvenum.h"
#define  ADD_NCCALCSIZE

// All this is in DvVers.h
//#ifndef	NDEBUG
// **************************************************************
//#	ifdef		ADDDBGW
//#define		SHOWWM		// OUPUT each WINDOW message
//#include	"WMDiag.h"	// redirection to, like "..\Utils\WMDiag.h"
//#	else		// !ADDDBGW
//#undef		SHOWWM
//#	endif		// ADDDBGW y/n
// **************************************************************
//#else	// NDEBUG is ON
// **************************************************************
//#undef		SHOWWM		// OUPUT each WINDOW message
// **************************************************************
//#endif	// !NDEBUG

// 2nd way to put on WIN MSGS in here
#ifdef	DIAGWM1		// Diag Win Msgs just in this module
#ifndef	_WMDiag_h
#pragma message( "ADVICE: DIAGWM1 is ON, including WMDiag.h..." )
#include	"WMDiag.h"	// redirection to, like "..\Utils\WMDiag.h"
#endif	// !_WMDiag_h
#endif	// DIAGWM1

#undef		ADDFRMOUT

//#define	DVHELPFILE		"DIBVIEW.HLP"
//#define	DVHELPFILE		"DV32.HLP"

#ifdef	WIN32
#define	_fmemcpy	memcpy
#endif	// WIN32

extern BOOL IsDiagOpen( VOID );

// SET Global items
//ghDIBInfo = hDIBInfo, which is normally in Extra;
//ghDIBx    = hDIBx, which is normally in Extra 2;
//extern	long InfoWndCreate( LPDIBCREATEINFO lpDIBCreateInfo );
extern	BOOL Draw3DRect( HDC hDC, RECT Rect, BOOL bSunken, LPDIBX lpDIBx );
//extern	BOOL	fChgAll;
extern	long	Do_IDM_ABOUT( HWND hWnd );
extern	BOOL	Dv_IDM_SAVEAS( HWND hWnd );
extern	void	chkit( char * );
extern   VOID  Do_IDM_PRINTSCRN( HWND hWnd );
extern   void  Do_IDM_PRINT2( HWND hWnd );

#define		ADDBKBMP

typedef HWND MLPTR LPHWND;
// External Functions
#ifdef	ADDTIMER1
extern	void	Frm_WM_TIMER( HWND, UINT, WPARAM, LPARAM );
extern	UINT	GotTimer;
//WORD	CTimeCnt = 0;
//HWND	CTimeHnd[MXANIMS+1];
BOOL	DeleteAnim( HWND );
BOOL	AddToAnims( HWND );
#endif	/* ADDTIMER1 */
//extern	BOOL	fSavINI;	/* = SI_Default; */
//extern	BOOL	fChgSI;	/* = FALSE; */
extern	BOOL	fDnLibLoad;
//extern	BOOL	fAutoLoad;
extern	HINSTANCE hLibInst;
extern	void Do_IDM_PRINT( HWND );
// NEW
extern   PMWL	AddToFileList4( PRDIB prd );
extern   long  Frm_WM_INITMENUPOPUP(HWND hWnd, UINT message,
						  WPARAM wParam, LPARAM lParam );
extern   VOID  Do_IDM_OPEN2( HWND hWnd );
extern   VOID  Do_IDM_OPENMRU( HWND hWnd );

// Forward references, or
// Local function prototypes
PMWL  CommonFileOpen( HWND hWnd, LPSTR lpf, DWORD Caller );
long	Frm_WM_COMMAND(HWND hWnd, WPARAM wParam, LPARAM lParam);
void	Frm_WM_DESTROY( HWND hWnd );

void	OpenPaletteWindow( HWND hParent, HWND hMDI, HPALETTE hPal );
void	Do_IDM_OPTIONS( HWND );
void	Do_IDM_OPEN( HWND hWnd );
void	Do_IDM_EXIT( HWND hWnd );
void	Do_IDO_SAVEINI( HWND hWnd );
void	Do_IDM_EXITWSAVE( HWND hWnd );
void	Do_IDM_EXITNOSAVE( HWND hWnd );
void	Do_IDM_HELP( HWND, UINT );
VOID  Do_IDM_COPYSAVE( hWnd );
VOID  Do_IDM_RESTORE2( hWnd );

extern	void PalInitSize( DWORD, LPPOINT );
extern	void ShowOption2( HWND );
//extern	void DvImageAtt( HWND );
extern	void Dv_IDM_IMAGEATT( HWND );	// Edit / Attributes...(IDD_IMAGEATT)
extern	void Dv_IDM_DUPLICATE( HWND );
extern	void Dv_IDM_EDITBMP( HWND );
extern	void ShowOption3( HWND );
extern	void Do_IDM_OPTION4( HWND );	// IDM_OPTION4
extern	void WriteIni( HWND );
extern	void Do_IDM_MAGNIFY( HWND );	// IDM_MAGNIFY
extern	void Do_IDM_OPTION6( HWND );	// IDM_OPTION6
extern	BOOL Do_IDM_CLEARCLIP( HWND );	// IDM_CLEARCLIP
extern	BOOL Do_IDM_ADDSWISH( HWND );	// IDM_ADDSWISH
extern	BOOL Do_IDM_RENDERNEW( HWND );	// IDM_RENDERNEW
extern VOID Do_IDM_PAINTSWISH( HWND hWndFrame );
extern VOID Do_IDM_PAINTOVAL( HWND hWndFrame );
extern VOID Do_IDM_PAINTSQUARE( HWND hWndFrame );
extern VOID Do_IDM_CLIPLISTMAX( hWnd );
extern VOID Do_IDM_CLIPLIST( HWND hWndFrame, UINT cmd );

#ifndef  USENEWWINSIZE
extern	void SetChgSize( HWND, WPARAM, LPARAM );
#endif // #ifndef  USENEWWINSIZE

//extern	void DoPrintSetup( void );
extern	void Do_IDM_PRTSETUP( HWND hWnd );
#ifdef	ADDCOLRF
// case IDM_BKCOLOR:
extern	void DvBkColor( HWND hWnd );
// case IDM_DEFFONT:
extern	void DvDefFont( HWND hWnd );
#endif	// ADDCOLRF

#ifdef	ADDWJPEG		/* Use the JPEG library for reading and writing */
							/* JPG, GIF, PPM, RLE ... and other we may add ... */
#ifdef	LDYNALIB	/* NO LINK with LIBRARY - Use Dynamic LoadLibrary ... */
// ===========================================================
extern	void CloseJPEGLib( void );
#endif	// LDYNALIB
#endif	// ADDWJPEG

#ifdef	ADDOPENALL
void	SROpenAs( HWND );	/* implement IDM_OPENAS */
#endif
#ifdef	ADDRESTORE
void	RestoreWindow( HWND );
void	RestAllWindow( HWND );
#endif	/* ADDRESTORE */

extern	void	AddToFileList( LPSTR );
extern	long	DoFileCommand( HWND, WPARAM, LPARAM );
extern	void	NewFileTerm( void );
extern	void	Err_No_Conv( LPSTR lpm );
extern	void	SetReadPath( LPSTR lpdf );
extern	void	DeleteTools( void );
extern	int		giScnWidth;		// = GetSystemMetrics( SM_CXSCREEN );
extern	int		giScnHeight;	// = GetSystemMetrics( SM_CYSCREEN );

// local
LPSTR	Frm_WM_COMMAND_Stg( WPARAM wParam );

BOOL  gbDbgMain = TRUE; // output message coming thru main proc

//char  szDIBPalTitle[] = "DIB Palette ";       // Title on DIB Palette Window
char  szSysPalTitle[] = "System Palette";    // Title on System Palette Wnd
//HMENU hFrameMenu      = NULL;                // Menu handle to frame menu.
//HWND  hFrameWnd       = NULL;

char   szOpenName[MAX_FILENAME];	/* Buffer of OPEN File ... */
char	szBS[] = "\\";
char	szDefHelpFile[] = DVHELPFILE;
char	gszHelpFileName[MAX_FILENAME] =
{ DVHELPFILE };
BOOL	fHelpUp = FALSE;
BOOL	fInDestroy = FALSE;

#ifdef	DIAGSAVEHOOK
extern	void	EndDiagHook( void );
#endif

HBRUSH	ghbrBackground = 0;

BOOL	bPalDev = FALSE;	// Display device can animate palettes?
WORD	wWinNumber = 0;		// Capture Window number.
int		iNumColors = 0;		// Number of colors suppored
int		gcxScr = 600;		// GetDeviceCaps( hDC, HORZRES );
int		gcyScr = 400;		//GetDeviceCaps( hDC, VERTRES );
// *********************************************
LPMALLOC      g_pMalloc = NULL;  // Shell malloc
// *********************************************
DWORD	dwFrmDep = 0;
//Members
//typedef struct _WINDOWPLACEMENT {     // wndpl 
//    UINT  length; 
//    UINT  flags; 
//    UINT  showCmd; 
//    POINT ptMinPosition; 
//    POINT ptMaxPosition; 
//    RECT  rcNormalPosition; 
//} WINDOWPLACEMENT; 
WINDOWPLACEMENT		gsWinPac;
RECT				grWinRec;
#ifdef   ADD_STATUS_BAR
HWND  ghStatusWnd = 0;  // CreateStatusWindow(...)
#endif // #ifdef   ADD_STATUS_BAR

// One time CREATE FRAME
// =====================
long Frm_WM_CREATE( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	long	            fret;
	CLIENTCREATESTRUCT ccs;
	HDC                hDC;

	fret = 0;

	// Find window menu where children will be listed
	//memset( &ccs, 0, sizeof(CLIENTCREATESTRUCT) );
	ccs.hWindowMenu  = GetSubMenu (GetMenu (hWnd), WINDOW_MENU);
	ccs.idFirstChild = IDM_WINDOWCHILD;
	// Create the MDI client filling the client area
#ifdef	WIN32
//HWND CreateWindowEx(
//    DWORD dwExStyle,	// extended window style
//    LPCTSTR lpClassName,	// pointer to reg.class name
//    LPCTSTR lpWindowName,	// pointer to window name
//    DWORD dwStyle,	// window style
//    int x,	// horizontal position of window
//    int y,	// vertical position of window
//    int nWidth,	// window width
//    int nHeight,	// window height
//    HWND hWndParent,	// handle to parent or owner window
//    HMENU hMenu,	// handle to menu, or child-window id.
//    HINSTANCE hInstance,	// handle to application instance
//    LPVOID lpParam 	// pointer to window-creation data
////   );	
	if( ghWndMDIClient = CreateWindowEx( WS_EX_NOPARENTNOTIFY,
		"MDICLIENT",
		NULL,
		WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL,
		0, 0, 0, 0,
		hWnd,
		(HMENU) 0xCAC,
		ghDvInst,
		(LPVOID)&ccs ) )
	{
  		ShowWindow( ghWndMDIClient, SW_SHOW );
	}

#else	// !WIN32
//HWND CreateWindow(lpszClassName, lpszWindowName, dwStyle, x, y, nWidth, nHeight,      //hwndParent, hmenu, hinst, lpvParam)		
//	LPCSTR lpszClassName;	/* address of registered class name
//	LPCSTR lpszWindowName;	/* address of window text	*/
//	DWORD dwStyle;	/* window style, */	
//	int x;	/* horizontal position of window	*/
//	int y;	/* vertical position of window	*/
//	int nWidth;	/* window width, */	
//	int nHeight;	/* window height, */	
//	HWND hwndParent;	/* handle of parent window	*/
//	HMENU hmenu;	/* handle of menu or child-window identifier
//	HINSTANCE hinst;	/* handle of application instance	*/
//	void MLPTR lpvParam;	/* address of window-creation data	
	if( ghWndMDIClient = CreateWindow( "MDICLIENT",
		NULL,
		WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL,
		0, 0, 0, 0,
		hWnd,
		(HMENU)0xCAC,
		ghDvInst,
		(LPSTR)&ccs ) )
	{
		ShowWindow( ghWndMDIClient, SW_SHOW );
	}
#endif	// WIN32 y/n

	if( hDC = GetDC( NULL ) )
	{
		bPalDev    = GetDeviceCaps( hDC, RASTERCAPS ) & RC_PALETTE;
		iNumColors = GetDeviceCaps( hDC, NUMCOLORS );
		ReleaseDC( NULL, hDC );
	}

	ghFrameMenu = GetMenu( hWnd );
	ghFrameWnd  = hWnd;
#ifdef   ADD_STATUS_BAR
   ghStatusWnd = CreateStatusWindow(
      WS_CHILD | WS_VISIBLE,  // LONG style,
      "Ready", // LPCTSTR lpszText,
      hWnd,    // HWND hwndParent, (or MAYBE ghWndMDIClient)
      IDC_STATUSBAR ); // UINT wID
#endif // #ifdef   ADD_STATUS_BAR

	return( fret );

}	// End - long Frm_WM_CREATE(HWND, UINT, WPARAM, LPARAM)



long Frm_WM_QUERYNEWPALETTE( HWND hWnd )
{
	long	ret;
	HWND hActive;
	ret = 0;
	if( hActive = GetCurrentMDIWnd() )
	{
		ret = (long) SendMessage( hActive,
			(UINT)MYWM_QUERYNEWPALETTE,
			(WPARAM)hWnd,
			0L);
	}
	return( ret );
}

#ifdef	ADDBKBMP
//	  case WM_ERASEBKGND:
// Like BOOL ChildWndEraseBkGnd( HWND hWnd, HDC hDC )
// This is for the PARENT's use - NOT really a CHILD
//extern	HGLOBAL	hgDIBInfo;	// hDIBInfo;
//extern	HGLOBAL	hgDIBx;	// hDIBx;
// =================================================
long	FrameEraseBkGnd( HWND hWnd, HDC hDC )
{
	long	flg;
	HANDLE	hInfo, hDIB;
	HBITMAP	hBMP, hOldB;
	RECT	rc;
	HDC		bmphDC;
	LPDIBINFO	lpDIBInfo;
	BITMAP		bm;
	BOOL	rFlg;
	RECT	Rect;
	HGLOBAL	hDIBx;
	LPDIBX	lpcwd;

	flg = 0;	// set DONE NOTHING
	// Return Values
	// An application should return nonzero
	// if it erases the background;
	// otherwise, it should return zero. 

	rFlg = FALSE;
	// Just how big is this window?
	GetClientRect( ghWndMDIClient, &rc );
	GetClientRect( hWnd, &Rect );

	// First the FLOOD FILL, with edges ...
//	if( hDIBx = hgDIBx )
	if( ( hDIBx = ghDIBx ) &&
		( hWnd == ghWndMDIClient ) )
	{
		if( lpcwd = (LPDIBX) DVGlobalLock( hDIBx ) )
		{
			// Paint the background
			rFlg = Draw3DRect( hDC, Rect, FALSE, lpcwd );
			DVGlobalUnlock( hDIBx );
		}
	}
	else
	{
		lpcwd = 0;
		//rFlg = TRUE;
//		if( hDC && ghbrBackground )
//		{
			//GetClientRect( hWnd, (LPRECT)&Rect );
			//SelectObject((HDC)wParam, hbrBackground);
			//FillRect((HDC)wParam, (LPRECT)&rect, hbrBackground);
//			SelectObject( hDC, ghbrBackground );
//			FillRect( hDC, &Rect, ghbrBackground );
//		}
	}
	// Then a BITMAP, which ever desired ...
	if( rFlg && ( hInfo = ghDIBInfo ) &&
		( lpDIBInfo = (LPDIBINFO)DVGlobalLock(hInfo) ) )
	{
		if( ( hDIB = lpDIBInfo->hDIB    ) &&
			 ( hBMP = lpDIBInfo->hBitmap ) )
		{
			GetClientRect( hWnd, &rc );
			GetObject( hBMP,
				sizeof( BITMAP ),	// size of buffer for object information
				&bm );
			bmphDC = CreateCompatibleDC(hDC);
			if( bmphDC )
			{
				RECT	dr;
				hOldB = SelectObject( bmphDC, hBMP );
// StretchBlt()
				// Set BEGIN PAINT location
				dr.left = rc.left + 2;
				dr.top  = rc.top  + 2;

				// Set WIDTH and HEIGHT
				if( rc.right > 4 )
					dr.right = rc.right - 4;
				else
					dr.right = 1;

				if( rc.bottom > 4 )
					dr.bottom = rc.bottom - 4;
				else
					dr.bottom = 1;

				if( ( dr.right == bm.bmWidth ) &&
					( dr.bottom == bm.bmHeight ) )
				{
					if( BitBlt( hDC,	// handle to destination device context
						dr.left,	// x-coordinate of destination rectangle's upper-left corner
						dr.top,		// y-coordinate of destination rectangle's upper-left corner
						bm.bmWidth,	// width of destination rectangle
						bm.bmHeight,// height of destination rectangle
						bmphDC,		// handle to source device context
						0,			// x-coordinate of source rectangle's upper-left corner
						0,			// y-coordinate of source rectangle's upper-left corner
						SRCCOPY ) )	// raster operation code
					{
						flg = 3;
					}
				}
				else
				{
					// Paint the BACKGROUND
					if( StretchBlt( hDC, // handle of destination device context
						dr.left,	// x-coordinate of upper-left corner of dest. rect.
						dr.top,		// y-coordinate of upper-left corner of dest. rect.
						dr.right,	// width of destination rectangle
						dr.bottom,	// height of destination rectangle
						bmphDC,		// handle of source device context
						0,			// x-coordinate of upper-left corner of source rectangle
						0,			// y-coordinate of upper-left corner of source rectangle
						bm.bmWidth,	// width of source rectangle
						bm.bmHeight,	// height of source rectangle
						SRCCOPY ) )	// raster operation code
					{
						// Successful BIT COPY, with STRETCH
						flg = 1;
					}
				}
				SelectObject( bmphDC, hOldB );
				DeleteDC( bmphDC );
			}
		}
		DVGlobalUnlock( hInfo );
	}
	if( !flg && hDC && ghbrBackground )
	{
		//GetClientRect( hWnd, (LPRECT)&Rect );
		//SelectObject((HDC)wParam, hbrBackground);
		//FillRect((HDC)wParam, (LPRECT)&rect, hbrBackground);
		//SelectObject( hDC, ghbrBackground );
		//if( FillRect( hDC, &Rect, ghbrBackground ) )
		//	flg = TRUE;
	}
	return flg;
}

#endif	// ADDBKBMP


long Frm_WM_PALETTECHANGED( HWND hWnd,
						   UINT message,
						   WPARAM wParam,
						   LPARAM lParam )
{
	long	lRet = 0;

	static BOOL bInPalChange = FALSE;

	if( !bInPalChange )
	{
		bInPalChange = TRUE;
		SendMessageToAllChildren( ghWndMDIClient,
			message,
			wParam,
			lParam );
		bInPalChange = FALSE;
	}

	return lRet;
}

// process WM_SIZE to Frame proc
// lParam 
// The low-order word of lParam specifies the new width of the client area. 
// The high-order word of lParam specifies the new height of the client area. 
long Frm_WM_SIZE( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	long	lRet = 0;
   // WM_SIZE - set new global FRAME size into grcSize rectangle
   grcSize.right  = LOWORD(lParam);
   grcSize.bottom = HIWORD(lParam);
   sprtf( "main: Frame WM_SIZE to %d x %d. e=%s"MEOR, grcSize.right, grcSize.bottom,
      ELAPSTG );
#ifdef  USENEWWINSIZE
   UpdateWP( hWnd ); // FIX20080316 - Use WINDOWPLACEMENT
#else // !#ifdef  USENEWWINSIZE
	SetChgSize( hWnd, wParam, lParam );
#endif // #ifdef  USENEWWINSIZE y/n

   // We didn't handle, pass to DefFrameProc.
	lRet = DefFrameProc( hWnd, ghWndMDIClient, message, wParam, lParam );
#ifdef   ADD_STATUS_BAR
   if( ghStatusWnd ) {
      LPARAM lp = MAKELONG(grcSize.right,14);
      SendMessage( ghStatusWnd, message, wParam, lp );
   }
#endif // #ifdef   ADD_STATUS_BAR

	return lRet;
}



//#ifdef	SHOWWM
#if		( defined( SHOWWM ) || defined( DIAGWM1 ) )
//
//extern	LPSTR	GetWMStg( HWND hWnd, UINT uMsg,
//				 WPARAM wParam, LPARAM lParam );
//
//#define		MAINHDR			"Main: "
#define		MAINHDR			"Frame: "
BOOL	bMainExcluded( UINT uMsg )
{
	BOOL	bRet = FALSE;

//	if( bNotExcluded( uMsg ) )
	if( uMsg == WM_TIMER )
	{
		// exclude frequent timer
		bRet = TRUE;
	}
	else	// also exclude some mousey stuff
	{
		if(( uMsg == WM_MOUSEMOVE ) ||
			( uMsg == WM_NCHITTEST ) ||
			( uMsg == WM_SETCURSOR ) ||
			( uMsg == WM_MENUSELECT) ||
			( uMsg == WM_ENTERIDLE ) ||
         ( uMsg == WM_GETTEXT   ) ||   // added 20021009
         ( uMsg == WM_NCPAINT   ) )
		{
			bRet = TRUE;
		}
	}

	return bRet;
}

#ifdef ADD_FRAME_DIAG2
//BOOL  gbDbgMain = TRUE;
void DiagFrameMessage(HWND hWnd, UINT message,
					   WPARAM wParam, LPARAM lParam )
{
	LPSTR	lpd;

	if( VERBAL9 )
	{
		LPSTR	lpc;
		UINT	uMsg;
		static UINT	uiMainLast;
		static HWND hMainLast;
		static WPARAM wpMainLast;
		static LPARAM lpMainLast;
		static UINT uiMainCnt;

		lpd = GetTmp3();
		uMsg = message;
//		if( bNotExcluded( uMsg ) )
		if( !bMainExcluded( uMsg ) )
		{
			if(( uiMainLast == uMsg ) &&
				( hMainLast == hWnd  ) )
			{
				// update parameters to latest
				wpMainLast = wParam;
				lpMainLast = lParam;
				uiMainCnt++;
				return;		// out of here
			}
//			else
//			{
				if( uiMainCnt > 1 )
				{
					sprintf( lpd,
                  MAINHDR"%s (H=0x%x) for %u"MEOR,
						GetWMStg( hMainLast,
							uiMainLast,
							wpMainLast,
							lpMainLast ), hMainLast,
							uiMainCnt );
					DO(lpd);
				}
				else if( uiMainCnt )
				{
					sprintf( lpd,
                  MAINHDR"%s (H=0x%x)"MEOR,
						GetWMStg( hMainLast,
							uiMainLast,
							wpMainLast,
							lpMainLast ), hMainLast );
					DO( lpd );
				}

				uiMainCnt = 0;

				uiMainLast = uMsg;
				hMainLast  = hWnd;
				wpMainLast = wParam;
				lpMainLast = lParam;

				if( ( uMsg == WM_COMMAND ) &&
					( lpc = Frm_WM_COMMAND_Stg( wParam ) ) )
				{
					sprintf( lpd,
                  MAINHDR"%s %s (H=0x%x)",
						GetWMStg( hWnd, uMsg, wParam, lParam ),
						lpc,
						hWnd );
				}
				else
				{
					sprintf( lpd,
                  MAINHDR"%s (H=0x%x)",
						GetWMStg( hWnd, uMsg, wParam, lParam ),
						hWnd );
				}
#ifndef	ADDFRMOUT
				if( dwFrmDep > 1 )
				{
					sprintf( EndBuf(lpd),
						" Dep=%d",
						(dwFrmDep - 1) );
				}
#endif	// !ADDFRMOUT
				strcat( lpd, ""MEOR );
				//DIAG1( lpd );
				DO( lpd );

//			}
		}	// excluded Main: messages
	}
}
#endif // #ifdef ADD_FRAME_DIAG2

#endif	// SHOWWM or DIAGWM1

//extern   VOID  GlazeWindow1( HDC hdc, PRECT lpr );
VOID  GlazeMDIClient( HWND hWnd )
{
   RECT        rc;
   HDC         hdc = GetDC( ghWndMDIClient ); 
   if(hdc)
   {
      GetClientRect( ghWndMDIClient, &rc );

      //FillRect( hdc, &rc, GetStockObject( BLACK_BRUSH ) );
      //PntRect( hdc, &rc, COLOR_SCALE_RED );
      GlazeWindow1( hdc, &rc );

      ReleaseDC( ghWndMDIClient, hdc );
   }
}

long  Frm_WM_PAINT( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
   long  lret = 0;
   PAINTSTRUCT ps;
   HDC         hdc = BeginPaint( hWnd, &ps );

//   GlazeWindow1( hdc, &ps.rcPaint );

   GlazeMDIClient( hWnd );

   EndPaint( hWnd, &ps );
   return lret;

}

long Frm_WM_ERASEBKGND( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	long	lRet = 1;
//   GlazeMDIClient( hWnd );
   return lRet;
}

long Frm_WM_ERASEBKGND2( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	long	lRet = 0;
	// Return Values
	// An application should return nonzero
	// if it erases the background;
	// otherwise, it should return zero.
	// in which case we do the DEFAULT action.
	//if( hWnd && wParam && ghbrBackground )
	{
		RECT	Rect;
//      HDC   hDC;
		GetClientRect( hWnd, (LPRECT)&Rect );
//		SelectObject((HDC)wParam, ghbrBackground );
		if( FillRect((HDC)wParam, (LPRECT)&Rect, GetStockObject(BLACK_BRUSH) ) )
         lRet = TRUE;

//		if( FillRect((HDC)wParam, (LPRECT)&Rect, ghbrBackground ) )
//			lRet = TRUE;
//      if( hDC = GetDC(ghWndMDIClient) )
//      {
   		//GetClientRect( ghWndMDIClient, (LPRECT)&Rect );
//         SelectObject( hDC, ghbrBackground );
//         FillRect( hDC, (LPRECT)&Rect, ghbrBackground );
//         ReleaseDC( ghWndMDIClient, hDC );
//      }
		//SelectObject( hDC, ghbrBackground );
		//if( FillRect( hDC, &Rect, ghbrBackground ) )
		//	flg = TRUE;
	}
//	if( (lRet = FrameEraseBkGnd( hWnd, (HDC)wParam )) == 0 )
//	{
//		lRet = DefFrameProc( hWnd, ghWndMDIClient, message, wParam, lParam );
//	}

	return lRet;

}

HCURSOR  ghCursor = 0;  // my resource
HCURSOR  ghOldCursor = 0;  // GDI resource
HWND     ghWndClicked = 0; // user clicked on
extern   BOOL  gbHide;   // TRUE if MAIN window is to be hidden on capture
extern   BOOL  gbChgHide;
extern   void	DVHideWindow( HWND hWnd );

HANDLE GetSelWinDIB(HWND hWndSelect, BOOL bCaptureClient)
{
   // User wants to capture a window, either the entire window
   // (including title bar and such), or the client area only
 
//   HWND hWndSelect;              // Handle to window which was selected
   HWND hWndDesktop;             // Handle to desktop window
   HDIB hDib;                    // Handle to memory containing DIB

   // Hide the DIBVIEW window
   if( gbHide )
	   DVHideWindow( ghMainWnd );    //hWndParent );

#if   0  // now done in main left button UP
   // ======================================
   sprtf( "Entering SelectWindow(%#x) ... "MEOR, ghMainWnd );
   // Ask user to select a window
   hWndSelect = SelectWindow();
   // ======================================
#endif   // 0 - removed

   // Check to see that they didn't try to capture desktop window
   hWndDesktop = GetDesktopWindow();

   if( hWndSelect == hWndDesktop )
   {
      if( gbHide ) 
         ShowWindow( ghMainWnd, SW_SHOW );   // hWndParent, SW_SHOW);

      MessageBox(NULL,"Cannot capture Desktop window."
                        "  Use 'Desktop' option to capture"
                        " the entire screen.", NULL,
                        MB_ICONEXCLAMATION | MB_OK);


      return NULL;
   }

   // Do some sanity checks to make sure we have a valid hWnd

   if( !hWndSelect )
   {
      if( gbHide )
         ShowWindow( ghMainWnd, SW_SHOW );   // hWndParent, SW_SHOW);

      MessageBox(NULL,
               "Unable to capture that window! (hWnd is NULL)",
               NULL, MB_ICONEXCLAMATION | MB_OK);

      return NULL;

   }


   // Move window which was selected to top of Z-order for
   // the capture, and make it redraw itself

   SetWindowPos(hWndSelect, NULL, 0, 0, 0, 0,
                  SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOMOVE);

   UpdateWindow(hWndSelect);

   // Call the function which captures screen

   hDib = CopyWindowToDIB (hWndSelect, (DWORD)(bCaptureClient ? PW_CLIENT : PW_WINDOW) );

   // Restore the DIBVIEW window

   if( gbHide )
      ShowWindow( ghMainWnd, SW_SHOW );   // hWndParent, SW_SHOW);

   return hDib;
}


VOID  SetSelWinCap( VOID )
{
   HWND     hWnd = ghMainWnd;
  
    /*
     * Capture all mouse messages
     */

    //SetCapture( ghWndMDIClient ); // why to MDI client????
   SetCapture( hWnd );  // main window = the frame - looks better
    /*
     * Load custom Cursor
     */
//   if(ghCursor == 0)
      ghCursor = LoadCursor( ghDvInst, "SELECTCUR");

   if(ghCursor)
   {
      HCURSOR hc = SetCursor(ghCursor);
      if( hc && (ghOldCursor == 0) )
      {
         sprtf( "Done first DOWN. Cursor %#x off."MEOR, hc );
         ghOldCursor = hc;
      }
      else
      {
         sprtf( "Repeat     DOWN. Cursor %#x ???."MEOR, hc );
      }
   }
   else
   {
      ghOldCursor = 0;
      //hOldCursor = SetCursor(LoadCursor( ghDvInst, "SelectCur"));
      sprtf( "Warning: Failed to LOAD SELECTCUR!"MEOR );
   }

}

BOOL  gbCaptureClient = FALSE;

VOID	OpenCapDIB( HANDLE hDIB )
{
	PRDIB prd = (PRDIB)MALLOC( sizeof(RDIB) );
   if(!prd)
      chkme( "C:ERROR: No memory!!!"MEOR );
	NULPRDIB( prd );
   prd->rd_pTitle = gszRPTit;
   prd->rd_pPath  = gszRPNam;
	if( hDIB )
	{
		// Open up a new MDI child with the
		// specified window title
		wWinNumber   = (wWinNumber + 1) % 10;
      DVNextRDName( prd, "TEMPC%d.BMP", (DWORD) wWinNumber );
		//szCapture[0] = '\0';
		//LoadString( ghDvInst, IDS_CAPTURE, szCapture, 20);
		//wsprintf( gszRDTit, szCapture, wWinNumber );

		prd->rd_hDIB = hDIB;
		//rd.rd_pFName = szWindowText;
		prd->rd_Caller = df_CAPTURE;
		OpenDIBWindow2( prd );
		//OpenDIBWindow (hDIB, szWindowText, df_CAPTURE );
#ifdef	CHGADDTO
		//AddToFileList( szWindowText );
#endif	// CHGADDTO
	}
   MFREE(prd);
}

VOID  ResetSelWinCap( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
   POINT pt;
   HDIB hDIB = 0;

    ReleaseCapture();

    if( ghOldCursor )
       SetCursor(ghOldCursor);

    ghOldCursor = 0;

//    sprtf( "SelectWindow() returning %#x."MEOR, hWndClicked );
             /*
              * Get mouse position
              */

             pt.x = LOWORD(lParam);
             pt.y = HIWORD(lParam);

             /*
              * Convert to screen coordinates
              */

             //ClientToScreen( ghWndMDIClient, &pt);
             ClientToScreen( hWnd, &pt);

             /*
              * Get Window that we clicked on
              */

             ghWndClicked = WindowFromPoint(pt);


   if(( ghWndClicked           ) &&
      ( IsWindow(ghWndClicked) ) )
   {
      hDIB = GetSelWinDIB(ghWndClicked, gbCaptureClient);
      OpenCapDIB( hDIB );

   }

   sprtf( "Done UP on Window %#x, with DIB %#x, on display"MEOR,
      ghWndClicked,
      hDIB );

}

typedef struct tagCMDFILE {
   LE link;
   TCHAR file[1];
}CMDFILE, * PCMDFILE;

LE sCmdFiles = { &sCmdFiles, &sCmdFiles };

PLE Get_Cmd_File_List(VOID) { return &sCmdFiles; }

//#define  CHKMEM2(a,b)   if(!a) { chkme( "C:ERROR: MEMORY FAILED! On %d bytes"MEOR, b ); }

void Add_2_Cmd_Files( PTSTR pfile )
{
   size_t len = lstrlen(pfile);
   if(len) {
      PCMDFILE pcf = (PCMDFILE)MALLOC( sizeof(CMDFILE) + len );
      //CHKMEM2(pcf, ( sizeof(CMDFILE) + len ));
      if(pcf) {
         PLE ph = Get_Cmd_File_List();
         lstrcpy( pcf->file, pfile );
         InsertTailList( ph, (PLE)pcf );
      }
   }
}

BOOL  Process_Cmd( HWND hWnd, PTSTR pcmd )
{
   size_t len = 0;
   PTSTR ptmp = gszTpBuf2;
   PTSTR pfile;
   if(pcmd) {
      lstrcpy(ptmp, pcmd);
      len = strlen(ptmp);
   }
   if(len) {
      size_t i;
      int   c, d;
      for( i = 0; i < len; i++ )
      {
         c = ptmp[i];
         if(( c == '-' )||( c == '/' )) {
            i++;
            c = toupper(ptmp[i]);
            switch(c)
            {
            case 'F':
               break;
            default:
               break;
            }
            // go to end of OPTION
            for( ; i < len; i++ )
            {
               c = ptmp[i];
               if( c <= ' ' )
                  break;
            }
         } else {
            // assume a FILE TO LOAD
            pfile = &ptmp[i];
            d = 0;
            if( c == '"' ) {  // if double inverted comma delimited
               d = c;   // set the DELIMITER
               pfile++;
            }
            i++;  // bump to NEXT
            for( ; i < len; i++ )
            {
               c = ptmp[i];
               if(d) {
                  if( c == d )   // break on delimiter
                     break;
               } else if( c <= ' ' )
                  break;   // break on spacey
            }
            ptmp[i] = 0;   // send that patch ZERO (if not already!)
            Add_2_Cmd_Files( pfile );  // and ADD to LOADABLE files
         }
      }
      return TRUE;
   }
   return FALSE;
}

DWORD g_dwSibCount = 0;
int   Get_Sib_Count( VOID )
{
   int   cnt = 0;
   PLE ph = Get_Cmd_File_List();
   ListCount2(ph,&cnt);
   return cnt;
}

PTSTR Get_Sib_File( DWORD num )
{
   DWORD cnt = 0;
   PLE ph = Get_Cmd_File_List();
   PLE pn;
   Traverse_List( ph, pn )
   {
      PCMDFILE pcf = (PCMDFILE)pn;
      cnt++;
      if( cnt == num )
         return pcf->file;
   }
   return NULL;
}

VOID Kill_Sibling_List( VOID )
{
   int cnt = Get_Sib_Count();
   if(cnt) {
      PLE ph = Get_Cmd_File_List();
      g_dwSibCount = 0;
      if( IsDiagOpen() ) {
         sprtf( "Killing Sibling list ... %d files."MEOR, cnt );
      }
      KillLList( ph );
   }
}

VOID Load_Sibling_File( HWND hWnd, DWORD num )
{
   PTSTR pfile = Get_Sib_File( num );
   if(pfile) {
      sprtf( "Load_Sibling_File: [%s], num %d"MEOR, pfile, num );
      CommonFileOpen( hWnd, pfile, df_LOADSIB );   // "Load_Sibling_File"
   } else {
      sprtf( "WARNING: Load_Sibling_File: FAILED TO GET %d FILE!"MEOR, num );
   }
   if(( num == 1 )||(g_dwSibCount == 0)) {
      g_dwSibCount = 0;
      Kill_Sibling_List();
   }
}

VOID Load_Cmd_Files( HWND hWnd )
{
   int   cnt = Get_Sib_Count();
   if(cnt) {
      sprtf( "Got %d files to load from sibling ..."MEOR, cnt );
      g_dwSibCount = cnt;  // set for TIMER to find!!!
   }
}

LRESULT Frm_WM_COPYDATA( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
   LRESULT lres = FALSE;
   COPYDATASTRUCT * pcds;
   pcds = (COPYDATASTRUCT *)lParam;
   sprtf( "Got WM_COPYDATA message ..."MEOR );
   if(pcds) {
      PMYREC pmr = (PMYREC)pcds->lpData;
      if(pmr && (pmr->n == sizeof(MYREC))) {
         PTSTR ps = pmr->cmd;
         if(*ps) {
            sprtf( "Processing [%s] command ..."MEOR, ps );
            lres = Process_Cmd( hWnd, ps );
         }  // quietly ignore blank
      } // not our record type
   }
   Load_Cmd_Files( hWnd );
   return lres;
}


#ifdef  ADD_NCCALCSIZE
/* ===================
typedef struct {
    RECT rgrc[3];
    PWINDOWPOS lppos;
} NCCALCSIZE_PARAMS, *LPNCCALCSIZE_PARAMS;
   =================== */
LRESULT Frm_WM_NCCALCSIZE( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
   LRESULT lres;
   LPNCCALCSIZE_PARAMS str = (LPNCCALCSIZE_PARAMS)lParam;
   if(wParam && str)
   {
      sprtf( "WM_NCCALCSIZE: RECT 1 %s, 2 %s, 3 %s\n",
         Rect2Stg(&str->rgrc[0]),
         Rect2Stg(&str->rgrc[1]),
         Rect2Stg(&str->rgrc[2]) );

   }      
   lres = DefFrameProc( hWnd, ghWndMDIClient, uMsg, wParam, lParam );
   if(wParam && str)
   {
      sprtf( "WM_NCCALCSIZE: AFTR 1 %s, 2 %s, 3 %s, return %d\n",
         Rect2Stg(&str->rgrc[0]),
         Rect2Stg(&str->rgrc[1]),
         Rect2Stg(&str->rgrc[2]),
         lres );
   }      

#ifdef   ADD_STATUS_BAR
   if(wParam)
   {
      LPNCCALCSIZE_PARAMS str = (LPNCCALCSIZE_PARAMS)lParam;
      if(str && ghStatusWnd)
      {
         PRECT prc = &str->rgrc[0]; // proposed new window coordinates
         //prc->bottom -= 24;
      }
   }
#endif // #ifdef   ADD_STATUS_BAR
   sprtf( "WM_NCCALCSIZE: Returning %d (wParam=%s)\n", lres,
      (wParam ? "TRUE" : "FALSE") );
   return lres;
}
#endif // ADD_NCCALCSIZE

//---------------------------------------------------------------------
//
// Function:   FRAMEWNDPROC
//
// Purpose:    Window procedure for MDI frame window.
//             Handles all messages destined for the frame.
//
// Parms:      hWnd    == Handle to the frame window.
//             message == Message for window.
//             wParam  == Depends on message.
//             lParam  == Depends on message.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------
//BOOL  gbDbgMain = TRUE;

//long MLIBCONV FRAMEWNDPROC(HWND hWnd, UINT message,
//						   WPARAM wParam, LPARAM lParam )
LRESULT CALLBACK FRAMEWNDPROC(HWND hWnd,      // handle to window
                              UINT uMsg,      // message identifier
                              WPARAM wParam,  // first message parameter
                              LPARAM lParam   // second message parameter
)
{
	LRESULT	lret = 0;

	dwFrmDep++;

#if	(defined(SHOWWM)||defined(DIAGWM1) && defined(ADD_FRAME_DIAG2))
	if( VERBAL9 && gbDbgMain )
	{
		DiagFrameMessage( hWnd, uMsg, wParam, lParam ); // MAINHDR = "Frame: "
	}
#endif	// SHOWWM or DIAGWM1

	switch( uMsg ) // message ) 
	{
   case WM_NCCREATE:
      SHGetMalloc(&g_pMalloc);
      goto Do_Default;

	case WM_CREATE:
		Frm_WM_CREATE( hWnd, uMsg, wParam, lParam );
		break;

		// Initialize pull down menus.
	case WM_INITMENUPOPUP:
		Frm_WM_INITMENUPOPUP( hWnd, uMsg, wParam, lParam );
		break;

		// Go handle all WM_COMMAND messages in Frm_WM_COMMAND().
	case WM_COMMAND:
		lret = Frm_WM_COMMAND( hWnd, wParam, lParam );
		break;

   case WM_CONTEXTMENU:
      {
extern   VOID  DoContextMenu( HWND hwnd, WPARAM wParam, LPARAM lParam );
         //DWORD xPos,yPos;
         //xPos = GET_X_LPARAM(lParam);
         //yPos = GET_Y_LPARAM(lParam);
         DoContextMenu( hWnd, wParam, lParam );
      }
      break;

   case WM_SPOOLERSTATUS:
      {
         // wParam
         // Specifies the PR_JOBSTATUS flag. = ZERO???
         // lParam
         // The low-order word specifies the number of jobs remaining
         // in the Print Manager queue.
         sprtf( "SPOOLER: Status = %#x Jobs = %d."MEOR,
            wParam,
            LOWORD(lParam) );
         if( ghDlgAbort )
         {
            SendMessage( ghDlgAbort, MYWM_CHANGEJOBS, (WPARAM)LOWORD(lParam), 0L );
         }
      }
      goto Do_Default;

		// Palette changed message -- someone changed the palette
		//  somehow.  We must make sure that all the MDI child windows
		//  realize their palettes here.
	case WM_PALETTECHANGED:
		Frm_WM_PALETTECHANGED( hWnd, uMsg, wParam, lParam );
		break;


         // We get a QUERYNEWPALETTE message when our app gets the
         //  focus.  We need to realize the currently active MDI
         //  child's palette as the foreground palette for the app.
         //  We do this by sending our MYWM_QUERYNEWPALETTE message
         //  to the currently active child's window procedure.  See
         //  the comments in CHILD.C (at the top of the file) for
         //  more info on this.

	case WM_QUERYNEWPALETTE:
		lret = Frm_WM_QUERYNEWPALETTE( hWnd );
		break;


	case WM_SIZE:
		lret = Frm_WM_SIZE( hWnd, uMsg, wParam, lParam );
		break;
#ifdef  ADD_NCCALCSIZE
   case WM_NCCALCSIZE:
		lret = Frm_WM_NCCALCSIZE( hWnd, uMsg, wParam, lParam );
		break;
#endif   //  ADD_NCCALCSIZE
   case WM_MOVE:
      UpdateWP( hWnd );
      break;

#ifdef   ADDSELWIN2
//   case WM_NCLBUTTONDOWN:

   case WM_LBUTTONDOWN:
      if( gbInSelWin )  // g BOOL = ADDSELWIN2  = global 'sel' proc
      {
         // capture and change mouse
         SetSelWinCap();
      }
      goto Do_Default;
      break;
//   case WM_NCLBUTTONUP:
   case WM_LBUTTONUP:
      if( gbInSelWin )  // g BOOL = ADDSELWIN2  = global 'sel' proc
      {
         // capture window pointed at ...

         ResetSelWinCap( hWnd, wParam, lParam );
         gbInSelWin = FALSE;  // out of mouse control ===

      }
      goto Do_Default;
      break;

#endif   // #ifdef   ADDSELWIN2

#ifdef	ADDTIMER1
	case WM_TIMER:
		Frm_WM_TIMER( hWnd, uMsg, wParam, lParam );
		break;
#endif	/* ADDTIMER1 */

   case WM_CLOSE:
      gbInClose = TRUE;       // we are CLOSING
      CloseAllDIBWindows();   // process the CHILDREN first
      DestroyWindow( hWnd );  // handle to window to destroy
      break;
		// Terminate this app by posting a WM_QUIT message,
		// or a WM_CLOSE
	case WM_DESTROY:
      gbInClose = TRUE; // we are CLOSING
		Frm_WM_DESTROY( hWnd );
		break;

#ifdef	ADDBKBMP

      // HOW TO GLAZE THE MDI CLIENT ???? ***TBD***
//	case WM_ERASEBKGND:
//		lret = Frm_WM_ERASEBKGND( hWnd, uMsg, wParam, lParam );
//		break;
// case WM_NCPAINT:
// case WM_PARENTNOTIFY:
   case WM_PAINT:
      lret = Frm_WM_PAINT( hWnd, uMsg, wParam, lParam );
      break;

#endif	// ADDBKBMP

   case WM_COPYDATA:
      lret = Frm_WM_COPYDATA( hWnd, wParam, lParam );
      break;

		// We didn't handle, pass to DefFrameProc.
   case WM_NCDESTROY:
      if( g_pMalloc )
         IMalloc_Release(g_pMalloc);
      //g_pMalloc->Release();
      g_pMalloc = NULL;
      // NOTE: Fall into default!!!
	default:
Do_Default:
		lret = DefFrameProc( hWnd, ghWndMDIClient, uMsg, wParam, lParam );
		break;

    }

	dwFrmDep--;

#if	( (defined(SHOWWM)||defined(DIAGWM1)) && defined(ADDFRMOUT) )
	if( VERBAL9 )
	{
		LPSTR	lpd = GetTmp1();
		wsprintf( lpd, "Main: Out 0x%04x Dep=%d"MEOR, uMsg, dwFrmDep );
		DO(lpd);
	}
#endif	// SHOWWM or DIAGWM1

    return( lret );

}	// End - LRESULT CALLBACK FRAMEWNDPROC(HWND, UINT, WPARAM, LPARAM )


//  ===========================================================
//  Put up the DIB palette window, even if there is a previous.
//  Remember to copy the DIB's palette, since if the
//  DIB window is destroyed, the palette is destroyed
//  with it.  Also, remember to change the palette flags
//  so if the DIB is being animated, the palette window's
//  palette doesn't have the PC_RESERVED flag set.
//long OpenDIBPal( HWND hWnd, WPARAM wParam, LPARAM lParam )
//  ===========================================================
long Dv_IDM_PALDIB( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	long	retv = 0;
	HPALETTE hPalDIB, hNewPal;
	HWND	hMDI;

	Hourglass( TRUE );
	// Get a handle to the current MDI Child.
	if( hMDI = GetCurrentMDIWnd () )
	{
      // NOTE: This can take a while if we have to COUNT the color
      // in a 24-bit DIB!!!
		if( hPalDIB = CurrentDIBPalette( hMDI ) )
		{
			hNewPal = CopyPaletteChangingFlags( hPalDIB, 0 );	/* Copy AGAIN */
			DeleteObject( hPalDIB );	/* Delete this COPY (or Gen24) ... */
         if( hNewPal )
            OpenPaletteWindow( hWnd, hMDI, hNewPal ); // IDM_PALDIB
			retv = 1;
		}
	}
	Hourglass( FALSE );
	return( retv );
}

#ifdef	TICKINI
BOOL	ToggleINI( void )
{
	if( gfChgAll )
	{
		if( gfSavINI )
		{
			//gfSavINI = FALSE;
			//gfChgAll = FALSE;
		}
	}
	// Toggle it ALL
	if( gfSavINI )
		gfSavINI = FALSE;
	else
		gfSavINI = TRUE;
	gfChgSI = TRUE;
	return( gfSavINI );
}

//	{ &gfSavINI, &fChgSI, IDO_SAVEINI, SI_Default, 0, 0, 0 },
void InvertINI( HWND hWnd )
{
	BOOL	flg;
	HMENU hMenu;
	if( hMenu = GetMenu( hWnd ) )
	{
		flg = ToggleINI();
		CheckMenuItem( hMenu, IDO_SAVEINI, MF_BYCOMMAND | 
                       (flg ? MF_CHECKED : MF_UNCHECKED));
	}
}

void SetINI( HWND hWnd, BOOL	flg )
{
	HMENU hMenu;
	if( hMenu = GetMenu( hWnd ) )
	{
		CheckMenuItem( hMenu, IDO_SAVEINI, MF_BYCOMMAND | 
                       (flg ? MF_CHECKED : MF_UNCHECKED));
	}
}

#endif	/* TICKINI */

void	InvertHide( HWND hWnd )
{
	BOOL  flg;
	HMENU hMenu = GetMenu( hWnd ); 
	if(hMenu)
	{
		flg = ToggleCaptureHide ();
		CheckMenuItem (hMenu, IDM_CAPTUREHIDE, MF_BYCOMMAND | 
                       (flg ? MF_CHECKED : MF_UNCHECKED));
	}
}

#define  MBI(t)   MessageBox( ghMainWnd, t, "PROGRAM INFORMATION", MB_ICONINFORMATION|MB_OK )

long	Do_Case_Capture( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	long		lRet = 0;
	HANDLE      hDIB;
//	TCHAR    szCapture[20];
	PRDIB prd = (PRDIB)MALLOC( sizeof(RDIB) );
   if(!prd)
      chkme( "C:ERROR: No memory!!!"MEOR );
	NULPRDIB( prd );

   prd->rd_pTitle = gszRPTit;
   prd->rd_pPath  = gszRPNam;

	if( wParam == IDM_CAPTFULLSCREEN )
   {
      // seems to work ok ...
		hDIB = CaptureFullScreen( hWnd );
   }
	else
   {
      // not working - removed from menu - needs fixes
#ifdef   ADDSELWIN2
      hDIB = 0;
      gbInSelWin = TRUE; // g BOOL = ADDSELWIN2  = global 'sel' proc
      // now, on LEFT BUTTON DOWN, capture mouse, change cursor, and roam
      // around. *** TBD *** highlight window as mouse passes over them.
      // must include icons of windows as selection, well any window
      // should do ...
      gbCaptureClient = (wParam == IDM_CAPTCLIENT);
#else //  !#ifdef   ADDSELWIN2
//        MENUITEM "&Window",                     IDM_CAPTWINDOW = SelWin
//        MENUITEM "&Client Area",                IDM_CAPTCLIENT
		//hDIB = CaptureWindow( hWnd, (wParam == IDM_CAPTCLIENT) );
      MBI( "Unfortunately the function has not been completed,"MEOR
         "but the full screen capture works, and you can clip"MEOR
         " the specific area of interest later." );

#endif   // #ifdef   ADDSELWIN2 y/n

   }
	if( hDIB )
	{
		// Open up a new MDI child with the
		// specified window title
		wWinNumber   = (wWinNumber + 1) % 10;
      DVNextRDName( prd, "TEMPC%d.BMP", (DWORD) wWinNumber );
		//szCapture[0] = '\0';
		//LoadString( ghDvInst, IDS_CAPTURE, szCapture, 20);
		//wsprintf( gszRDTit, szCapture, wWinNumber );

		prd->rd_hDIB = hDIB;
		//rd.rd_pFName = szWindowText;
		prd->rd_Caller = df_CAPTURE;
		OpenDIBWindow2( prd );
		//OpenDIBWindow (hDIB, szWindowText, df_CAPTURE );
#ifdef	CHGADDTO
		//AddToFileList( szWindowText );
#endif	// CHGADDTO
	}

   MFREE(prd);

	return lRet;

}

//---------------------------------------------------------------------
//
// Function:   Frm_WM_COMMAND
//
// Purpose:    WM_COMMAND handler for MDI frame window.
//
// Parms:      hWnd    == Handle to this MDI child window.
//             wParam  == Depends on command.
//             lParam  == Depends on command.
//
// History:   Date      Reason
//             6/01/91  Created
//            10/26/91  Removed "message" parm (unused)
//                      Fixed bugs in IDM_PRINT:  was
//                        assigning incorrect values to
//                        wUnits for PRINT_BESTFIT and
//                        PRINT_STRETCH. 
//            10/29/91  Added passing of bUse31APIs to
//                        DIBPrint() in IDM_PRINT case.
//            10/30/91  Added capture code.
//            11/15/91  Added szFileName parm to OpenDIBFile
//                        in IDM_OPEN.  Added use of IDS_CAPTURE
//                        string table entry to IDM_CAPT*.
//            01/28/92  IDM_PALDIB bug fix : delete COPY palette
//                        returned by CurrentDIBPalette(). (or the 24-bit
//            06May96   Add passing of HWND (Window OWNER) to DIBPrint
//
// NOTE: If an application processes this message,
//	it should return zero. 
//
//---------------------------------------------------------------------
extern	void chkme3( void );

long Frm_WM_COMMAND( HWND hWnd,
				WPARAM wParam,
				LPARAM lParam )
{
	HWND hMDIChild;
	long	retv;
   DWORD cmd = LOWORD(wParam);
	retv = 0;
	chkme3();
//	if( (wParam >= IDM_FILE1) && (wParam <= IDM_FILEMax) )
	if( ( cmd >= IDM_FILE1   ) &&
       ( cmd <= IDM_FILEMax ) )
	{
		DoFileCommand( hWnd, cmd, lParam );
	} else if(( cmd >= IDM_CLIPLIST )&&
      ( cmd < IDM_CLIPLISTMAX ))
   {
      Do_IDM_CLIPLIST( hWnd, cmd ); // add a CLIP rectangle
   }
	else
	{
// ==============================================
//		switch( wParam )
		switch( cmd )
		{

			// Put up the About box.
		case IDM_ABOUT:
			Do_IDM_ABOUT( hWnd );
			break;

		case IDM_HELP:
			Do_IDM_HELP( hWnd, HELP_CONTENTS );
			break ;

			// Open up a DIB MDI child window.
		case IDM_OPEN:
			Do_IDM_OPEN( hWnd );
			//Do_IDM_OPEN2( hWnd );
			break;

		case IDM_OPENMRU:
			Do_IDM_OPENMRU( hWnd );
			break;

#ifdef	ADDOPENALL		/* controls IDM_OPENAS implementation */
		case IDM_OPENAS:
			SROpenAs( hWnd );
			break;
#endif	// ADDOPENALL

//		case IDM_PRTSETUP:
//			//DoPrintSetup();
//			Do_IDM_PRTSETUP( hWnd );
//			break;
      case IDM_PPOPTIONS:
         Do_IDM_PRINT2( hWnd );
         break;

		case IDM_PRINT:
			Do_IDM_PRINT( hWnd );
			break;

		case IDM_PRINTSCRN:
			Do_IDM_PRINTSCRN( hWnd );
			break;

			// Save the current DIB to disk.
		case IDM_SAVE:
			Dv_IDM_SAVE( hWnd );	// See DvFile.c
			break;

		case IDM_SAVEAS:
			Dv_IDM_SAVEAS( hWnd );	// See DvFile.c
			break;


			// Handle the clipboard copy operation.
		case IDM_COPY:
			HandleCopyClipboard ();
			break;


			// Handle the clipboard paste operation.
		case IDM_PASTE:
			HandlePasteClipboard ();
			break;

			// Put up the DIB palette window.
		case IDM_PALDIB:
			Dv_IDM_PALDIB( hWnd, wParam, lParam );
			//OpenDIBPal( hWnd, wParam, lParam );
			break;


			// Put up the system palette window.
		case IDM_PALSYS:
			OpenPaletteWindow( hWnd, 0, DVGetSystemPalette(TRUE) );
			break;


			// Animate the DIB's palette.
		case IDM_PALANIMATE:
			{
				hMDIChild = GetCurrentMDIWnd ();
				if (hMDIChild)
					SendMessage( hMDIChild, MYWM_ANIMATE, 0, 0L );
			}
			break;

			// Restore the DIB's palette (after animation)
			// to its original state.
		case IDM_PALRESTORE:
			{
				hMDIChild = GetCurrentMDIWnd ();
				if( hMDIChild )
					SendMessage( hMDIChild, MYWM_RESTOREPALETTE, 0, 0L );

			}
			break;

			// Tile MDI windows
		case IDM_WINDOWTILE:
			SendMessage( ghWndMDIClient, WM_MDITILE, 0, 0L);
			break;

			// Cascade MDI windows
		case IDM_WINDOWCASCADE:
			SendMessage( ghWndMDIClient, WM_MDICASCADE, 0, 0L);
			break;

			// Auto - arrange MDI icons
		case IDM_WINDOWICONS:
			SendMessage( ghWndMDIClient, WM_MDIICONARRANGE, 0, 0L);
			break;

			// Close all MDI child windows.
		case IDM_WINDOWCLOSEALL:
			CloseAllDIBWindows ();
			break;

#ifdef	ADDRESTORE
		case IDM_WINDOWRESTORE:
			RestoreWindow( hWnd );
			break;

		case IDM_WINDOWRESTALL:
			RestAllWindow( hWnd );
			break;
#endif	/* ADDRESTORE */

			// User wants to see the stretch dialog box.
		case IDM_OPTIONS:
			Do_IDM_OPTIONS( hWnd );
			break;

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
		case IDM_OPTION2:
			ShowOption2( hWnd );
			break;
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

		case IDM_OPTION3:
//ShowOption3 uses szOption3Dlg "GENOPTIONS", proc OPTION3DLG in DvOpt3.c
			ShowOption3( hWnd );	// OPTION3DLG in DvOpt3.c
			break;

		case IDM_OPTION4:
			Do_IDM_OPTION4( hWnd );
			break;

			// User wants to perform a capture operation.
			// Call the appropriate routine in CAPTURE.C
			// for what the user wants to do.
			// If we get a handle to a DIB back, open
			//  up a new MDI child.
		case IDM_CAPTCLIENT:
		case IDM_CAPTWINDOW:
		case IDM_CAPTFULLSCREEN:
			Do_Case_Capture( hWnd, wParam, lParam );
			break;	// case CAPTURE (IDM_CAPTCLIENT or IDM_CAPTWINDOW or IDM_CAPTFULLSCREEN )

			// Toggle the "Hide on Capture" option.
		case IDM_CAPTUREHIDE:
			InvertHide( hWnd );
			break;

			// User wants to exit DIBView.
			// Send ourselves a WM_CLOSE
			//  message (WM_CLOSE does some necessary clean
			// up operations).
		case IDM_EXIT:
			Do_IDM_EXIT( hWnd );
			break;

		case IDM_EXITNOSAVE:
			Do_IDM_EXITNOSAVE( hWnd );
			break;

		case IDM_EXITWSAVE:
			Do_IDM_EXITWSAVE( hWnd );
			break;

#ifdef	TICKINI
//	{ &gfSavINI, &fChgSI, IDO_SAVEINI, SI_Default, 0, 0, 0 },
		case IDO_SAVEINI:
			Do_IDO_SAVEINI( hWnd );
			break;
#endif	/* TICKINI */

#ifdef	ADDROT
		case IDM_ROTATE:
			DvRotate( hWnd );
			break;
#endif	// ADDROT

		case IDM_IMAGEATT:	// (uses IDD_IMAGEATT dialog template)
			//DvImageAtt( hWnd ); // Edit / Attributes
			Dv_IDM_IMAGEATT( hWnd );	// (IDD_IMAGEATT template)
			break;

		case IDM_DUPLICATE:
			Dv_IDM_DUPLICATE( hWnd );
			break;

		case IDM_EDITBMP:
			Dv_IDM_EDITBMP( hWnd );
			break;

#ifdef	ADDCOLRF
		case IDM_BKCOLOR:
			DvBkColor( hWnd );
			break;

		case IDM_DEFFONT:
			DvDefFont( hWnd );
			break;

#endif	// ADDCOLRF
      case IDM_COPYSAVE:
         Do_IDM_COPYSAVE( hWnd );
         break;

      case IDM_RESTORE2:
         Do_IDM_RESTORE2( hWnd );
         break;
		case IDM_MAGNIFY:
			Do_IDM_MAGNIFY( hWnd );
			break;
		case IDM_OPTION6:
			Do_IDM_OPTION6( hWnd );
			break;
      case IDM_CLEARCLIP:
			Do_IDM_CLEARCLIP( hWnd );
			break;
      case IDM_ADDSWISH:
			Do_IDM_ADDSWISH( hWnd );
			break;
      case IDM_RENDERNEW:
			Do_IDM_RENDERNEW( hWnd );
			break;
      case IDM_PAINTSWISH:
         Do_IDM_PAINTSWISH( hWnd );
         break;
      case IDM_PAINTOVAL:
         Do_IDM_PAINTOVAL( hWnd );
         break;
      case IDM_PAINTSQUARE:
         Do_IDM_PAINTSQUARE( hWnd );
         break;
      case IDM_CLIPLISTMAX:
         Do_IDM_CLIPLISTMAX( hWnd );
         break;
      case IDM_REPAINT:
         hMDIChild = GetCurrentMDIWnd ();
         if (hMDIChild)
            InvalidateRect( hMDIChild, NULL, TRUE );
         break;
			// Must be some system command -- 
			// pass it on to the default
			//  frame window procedure.
		default:
			retv = DefFrameProc( hWnd, ghWndMDIClient, WM_COMMAND,
				wParam, lParam );
			break;

		}	// switch pre wParam
// ==============================================
	} // If IDM_FILE1 to FILEMax y/n

	return( retv );

}	// End - long Frm_WM_COMMAND(HWND, WPARAM, LPARAM )


//LPSTR	Frm_WM_COMMAND_Stg( HWND hWnd,
//				WPARAM wParam,
//				LPARAM lParam )
LPSTR	Frm_WM_COMMAND_Stg( WPARAM wParam )
{
	LPSTR	lpr = 0;
   DWORD cmd = LOWORD(wParam);
	//if( (wParam >= IDM_FILE1) && (wParam <= IDM_FILEMax) )
	if( (cmd >= IDM_FILE1) && (cmd <= IDM_FILEMax) )
	{
		//DoFileCommand( hWnd, wParam, lParam );
		lpr = "DoFileCommand";
	}
	else
	{
// ==============================================
//		switch( wParam )
		switch( cmd )
		{

			// Put up the About box.
		case IDM_ABOUT:
			//Do_IDM_ABOUT( hWnd );
			lpr = "IDM_ABOUT";
			break;

		case IDM_HELP:
			//Do_IDM_HELP( hWnd, HELP_CONTENTS );
			lpr = "IDM_HELP";
			break ;

			// Open up a DIB MDI child window.
		case IDM_OPEN:
			//Do_IDM_OPEN( hWnd );
			lpr = "IDM_OPEN";
			break;


#ifdef	ADDOPENALL
		case IDM_OPENAS:
			//SROpenAs( hWnd );
			lpr = "IDM_OPENAS";
			break;
#endif	// ADDOPENALL

		case IDM_PRTSETUP:
			//DoPrintSetup();
			lpr = "IDM_PRTSETUP";
			break;

		case IDM_PRINT:
			//DoIDM_PRINT( hWnd );
			lpr = "IDM_PRINT";
			break;

			// Save the current DIB to disk.
		case IDM_SAVE:
			//Dv_IDM_SAVE( hWnd );	// See DvFile.c
			lpr = "IDM_SAVE";
			break;

		case IDM_SAVEAS:
			//Dv_IDM_SAVEAS( hWnd );	// See DvFile.c
			lpr = "IDM_SAVEAS";
			break;


			// Handle the clipboard copy operation.
		case IDM_COPY:
			//HandleCopyClipboard ();
			lpr = "IDM_COPY";
			break;


			// Handle the clipboard paste operation.
		case IDM_PASTE:
			//HandlePasteClipboard ();
			lpr = "IDM_PASTE";
			break;

			// Put up the DIB palette window.
		case IDM_PALDIB:
			//Dv_IDM_PALDIB( hWnd, wParam, lParam );
			//OpenDIBPal( hWnd, wParam, lParam );
			lpr = "IDM_PALDIB";
			break;


			// Put up the system palette window.
		case IDM_PALSYS:
			//OpenPaletteWindow( hWnd, 0, DVGetSystemPalette (TRUE) );
			lpr = "IDM_PALSYS";
			break;


			// Animate the DIB's palette.
		case IDM_PALANIMATE:
			//{
			//	HWND hMDIChild;
			//	hMDIChild = GetCurrentMDIWnd ();
			//	if (hMDIChild)
			//		SendMessage( hMDIChild, MYWM_ANIMATE, 0, 0L );
			//}
			lpr = "IDM_PALANIMATE";
			break;

			// Restore the DIB's palette (after animation)
			// to its original state.
		case IDM_PALRESTORE:
			//{
			//	HWND hMDIChild;
//
//				hMDIChild = GetCurrentMDIWnd ();
//
//				if( hMDIChild )
//					SendMessage( hMDIChild, MYWM_RESTOREPALETTE, 0, 0L );
//
//			}
			lpr = "IDM_PARESTORE";
			break;

			// Tile MDI windows
		case IDM_WINDOWTILE:
			//SendMessage( ghWndMDIClient, WM_MDITILE, 0, 0L);
			lpr = "IDM_WINDOWTILE";
			break;

			// Cascade MDI windows
		case IDM_WINDOWCASCADE:
			//SendMessage( ghWndMDIClient, WM_MDICASCADE, 0, 0L);
			lpr = "IDM_WINDOWCASCADE";
			break;

			// Auto - arrange MDI icons
		case IDM_WINDOWICONS:
			//SendMessage( ghWndMDIClient, WM_MDIICONARRANGE, 0, 0L);
			lpr = "IDM_WINDOWICONS";
			break;

			// Close all MDI child windows.
		case IDM_WINDOWCLOSEALL:
			//CloseAllDIBWindows ();
			lpr = "IDM_WINDOWCLOSEALL";
			break;

#ifdef	ADDRESTORE
		case IDM_WINDOWRESTORE:
			//RestoreWindow( hWnd );
			lpr = "IDM_WINDOWRESTORE";
			break;

		case IDM_WINDOWRESTALL:
			//RestAllWindow( hWnd );
			lpr = "IDM_WINDOWRESTALL";
			break;
#endif	/* ADDRESTORE */

			// User wants to see the stretch dialog box.
		case IDM_OPTIONS:
			//Do_IDM_OPTIONS( hWnd );
			lpr = "IDM_OPTIONS";
			break;

		case IDM_OPTION2:
			//ShowOption2( hWnd );
			lpr = "IDM_OPTION2";
			break;

		case IDM_OPTION3:
//ShowOption3 uses szOption3Dlg "GENOPTIONS", proc OPTION3DLG in DvOpt3.c
			//ShowOption3( hWnd );	// OPTION3DLG in DvOpt3.c
			lpr = "IDM_OPTION3";
			break;

		case IDM_OPTION4:
			//Do_IDM_OPTION4( hWnd );
			lpr = "IDM_OPTION4";
			break;

			// User wants to perform a capture operation.
			// Call the appropriate routine in CAPTURE.C
			// for what the user wants to do.
			// If we get a handle to a DIB back, open
			//  up a new MDI child.
		case IDM_CAPTCLIENT:
		case IDM_CAPTWINDOW:
		case IDM_CAPTFULLSCREEN:
			//Do_Case_Capture( hWnd, wParam, lParam );
			lpr = "IDM_CAPTxxxxx";
			break;	// case CAPTURE (IDM_CAPTCLIENT or IDM_CAPTWINDOW or IDM_CAPTFULLSCREEN )

			// Toggle the "Hide on Capture" option.
		case IDM_CAPTUREHIDE:
			//InvertHide( hWnd );
			lpr = "IDM_CAPTUREHIDE";
			break;

			// User wants to exit DIBView.
			// Send ourselves a WM_CLOSE
			//  message (WM_CLOSE does some necessary clean
			// up operations).
		case IDM_EXIT:
			//Do_IDM_EXIT( hWnd );
			lpr = "IDM_EXIT";
			break;

		case IDM_EXITNOSAVE:
			//Do_IDM_EXITNOSAVE( hWnd );
			lpr = "IDM_EXITNOSAVE";
			break;

		case IDM_EXITWSAVE:
			//Do_IDM_EXITWSAVE( hWnd );
			lpr = "IDM_EXITWSAVE";
			break;

#ifdef	TICKINI
//	{ &gfSavINI, &fChgSI, IDO_SAVEINI, SI_Default, 0, 0, 0 },
		case IDO_SAVEINI:
			//Do_IDO_SAVEINI( hWnd );
			lpr = "IDO_SAVEINI";
			break;
#endif	/* TICKINI */

#ifdef	ADDROT
		case IDM_ROTATE:
			//DvRotate( hWnd );
			lpr = "IDM_ROTATE";
			break;
#endif	// ADDROT

		case IDM_IMAGEATT:	// (IDD_IMAGEATT)
			lpr = "IDM_IMAGEATT";
			break;

		case IDM_DUPLICATE:
			//Dv_IDM_DUPLICATE( hWnd );
			lpr = "IDM_DUPLICATE";
			break;

		case IDM_EDITBMP:
			//Dv_IDM_EDITBMP( hWnd );
			lpr = "IDM_EDITBMP";
			break;

#ifdef	ADDCOLRF
		case IDM_BKCOLOR:
			//DvBkColor( hWnd );
			lpr = "IDM_BKCOLOR";
			break;

		case IDM_DEFFONT:
			//DvDefFont( hWnd );
			lpr = "IDM_DEFFONT";
			break;

#endif	// ADDCOLRF
			// Must be some system command -- 
			// pass it on to the default
			//  frame window procedure.
		default:
			lpr = "NOT Dv32 command";
			//retv = DefFrameProc( hWnd, ghWndMDIClient, WM_COMMAND,
			//	wParam, lParam );
			break;

		}	// switch pre wParam
// ==============================================
	} // If IDM_FILE1 to FILEMax y/n

//	return( retv );
	return( lpr );

}	// End - LPSTR Frm_WM_COMMAND_Stg(HWND, WPARAM, LPARAM )


//---------------------------------------------------------------------
//
// Function:   OpenDIBWindow
//
// Purpose:    This routine opens up an MDI child window.  The child
//             window will be sized to the height/width of the DIB.
//
// Parms:      hDIB    == Handle to the DIB to put in the MDI child.
//             szTitle == Title for title bar (must be a valid DOS
//                        filename.
//             Flag    == Where it was created from
//
// History:   Date      Reason
//             6/01/91  Created
//            10/15/91  Got rid of use of handle for
//                      DIBCREATEINFO, it's kosher to
//                      pass a pointer instead.
//             
//---------------------------------------------------------------------
MDICREATESTRUCT mcs;

//HWND OpenDIBWindow(HANDLE hDIB, LPSTR szTitle, DWORD dflg )
HWND OpenDIBWindow2( PRDIB prd )
{
	HANDLE			hDIB;
	LPSTR          lpDIB;
	HWND           hChild;
	DWORD          dwDIBHeight, dwDIBWidth;
	DIBCREATEINFO  DIBCreateInfo;
	RECT           rcWindow;
	LPSTR			   lpTitle;

	Hourglass( TRUE );	/* SetCursor ... */
	NULDIBCI( DIBCreateInfo );	// Ensure ALL null

   DIBCreateInfo.c_pRDib = prd; // establish pointer to (transient) PRDIB
	hDIB = prd->rd_hDIB; 
	if(hDIB)
	{
		lpDIB = DVGlobalLock( hDIB ); // LOCK DIB HANDLE
		if(lpDIB)  
		{
			dwDIBHeight = DIBHeight (lpDIB);
			dwDIBWidth  = DIBWidth (lpDIB);
			DVGlobalUnlock( hDIB );    // UNLOCK DIB HANDLE
		}
		else
		{
			Hourglass( FALSE );
			DIBError (ERR_MEMHANDLE);
			return NULL;
		}
	}
	else if( prd->rd_hGIFInfo )
	{
		LPGIFHDREXT lpghe;
		if( lpghe = (LPGIFHDREXT)DVGlobalLock( prd->rd_hGIFInfo ) )
		{
			dwDIBHeight = lpghe->gheHeight;
			dwDIBWidth  = lpghe->gheWidth;
			DVGlobalUnlock( prd->rd_hGIFInfo );
		}
		else
		{
			DVGlobalFree( prd->rd_hGIFInfo );
			Hourglass( FALSE );
			DIBError (ERR_MEMHANDLE);
			return NULL;
		}
	}
	else
	{
		return NULL;
	}

	DIBCreateInfo.c_hDIB = hDIB;
	lpTitle = prd->rd_pPath;
	if( ( lpTitle ) && ( lpTitle[0] ) )
		strcpy( DIBCreateInfo.c_szFileName, lpTitle );
	else
		strcpy( DIBCreateInfo.c_szFileName, "Untitled" );

	DIBCreateInfo.c_dwCDIBFlag = prd->rd_Caller;
	DIBCreateInfo.c_ghInfo     = prd->rd_hGIFInfo;

	// Determine the necessary window size to hold the DIB.
	rcWindow.left   = 0;
	rcWindow.top    = 0;
	rcWindow.right  = (int) dwDIBWidth;
	rcWindow.bottom = (int) dwDIBHeight;
   // ====================================================

	AdjustWindowRect( &rcWindow,
         ( WS_CHILD | WS_SYSMENU | WS_CAPTION |
			  WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX ),
			FALSE );

   sprtf( "Using rect %s in WM_MDICREATE."MEOR, 
      Rect2Stg(&rcWindow) );

	// Setup to call WM_MDICREATE.
	mcs.szTitle = lpTitle;
	mcs.szClass = szMDIChild;
	mcs.hOwner  = ghDvInst;
	mcs.x       = CW_USEDEFAULT;
	mcs.y       = CW_USEDEFAULT;
	mcs.cx      = rcWindow.right  - rcWindow.left;
	mcs.cy      = rcWindow.bottom - rcWindow.top;
	mcs.style   = WS_HSCROLL | WS_VSCROLL;
	mcs.lParam  = (LPARAM)(LPDIBCREATEINFO) &DIBCreateInfo;
	
	// If no other DIBs are being displayed, always force this
	//  window to the upper left corner of the MDI client.
	if( gnDIBsOpen == 0 )
	{
		mcs.x = 0;
		mcs.y = 0;
	}

   if(( giScnWidth ) &&
		( mcs.cx > giScnWidth ) )
		mcs.cx = giScnWidth; // reduce at least to SCREEN WIDTH

   if(( giScnHeight ) &&
		( mcs.cy > giScnHeight ) )
		mcs.cy = giScnHeight;   // reduce to SCREEN HEIGHT

//	if( ghMDIClient )
	if( ghFrameWnd )
	{
		RECT	rcFrame;
//		if( GetWindowRect( ghFrameWnd, &rcFrame ) )
		if( GetClientRect( ghFrameWnd, &rcFrame ) )
		{
			int		x, y;
			x = rcFrame.right  - rcFrame.left;
			y = rcFrame.bottom - rcFrame.top;
			if(( x > 0 ) &&
				( x > (giScnWidth / 2) ) &&
				( mcs.cx > x ) )
				mcs.cx = x;
			if(( y > 0 ) &&
				( y > (giScnHeight / 2) ) &&
				( mcs.cy > y ) )
				mcs.cy = y;
		}
	}

	// Tell the MDI Client to create the child.
	// NEXT DEBUG TRAP CAN BE AT case WM_CREATE,
	// ie ChildWndCreate( hWnd, ... ) - BREAKPOINT = Use chkcreate()!!!
#ifdef	WIN32
	hChild = (HWND)SendMessage( (HWND)ghWndMDIClient,  // handle of destination window
		(UINT)WM_MDICREATE,  // message to send
		(WPARAM)0,           // first message parameter
		(LPARAM)&mcs );       // second message parameter
#else	// WIN32
	hChild = (WORD) SendMessage( ghWndMDIClient,
		WM_MDICREATE,
		0,
		(LPARAM) (LPMDICREATESTRUCT) &mcs);
#endif	// WIN32 y/n

	Hourglass( FALSE );

	if( hChild )
   {
      prd->rd_hMDI = hChild;  // fill in the CHILD (MDI) window
   }
   else
	{
		DIBError (ERR_CREATECHILD);
	}

	return( hChild );

}	// End - HWND OpenDIBWindow2( PRDIB )

// Styles -- Popup Window
//  Thick (resizable) frame
//  Has a system menu
//  Clips other siblings
//  Clips its children
//  Maximize button
//  Minimize button
//  Vertical scroll bar
//  Caption
#define	DEFPALSTYLE		(WS_POPUP | \
                         WS_THICKFRAME | \
                         WS_SYSMENU | \
                         WS_CLIPSIBLINGS | \
                         WS_CLIPCHILDREN | \
                         WS_MAXIMIZEBOX | \
                         WS_MINIMIZEBOX | \
                         WS_VSCROLL | \
                         WS_CAPTION)

//typedef struct	tagPWCS	{	/* pw */
//	HWND	pw_hMDI;	// "Parent" MDI Child (But can be DESTROYED)
//	HGLOBAL	pw_hDIBInfo;// Likewise - DIB Info of child, but can be DESTROYED
//}PWCS;
//typedef PWCS MLPTR LPPWCS;
//Members
//typedef struct _WINDOWPLACEMENT {     // wndpl 
//    UINT  length; 
//    UINT  flags; 
//    UINT  showCmd; 
//    POINT ptMinPosition; 
//    POINT ptMaxPosition; 
//    RECT  rcNormalPosition; 
//} WINDOWPLACEMENT; 
//WINDOWPLACEMENT		gsWinPac;

//---------------------------------------------------------------------
//
// Function:   OpenPaletteWindow
//
// Purpose:    This routine opens up a popup window which displays
//             a palette.
//             Handles the IDM_PALDIB menu item
//
//             Note that all the popup window's routines are in
//             PALETTE.C.
//
// Parms:      szTitle == Title for title bar.
//             hOwner  == Handle to the Window which owns this popup
//             hPal    == Palette to display.
//
// History:   Date      Reason
//          6/01/91     Created
//			   26Apr97		Use PWCS (Palette Window Create Structure)
//          14DEC2000   Fix the left click palette selection
//
//---------------------------------------------------------------------
void OpenPaletteWindow( HWND hOwner, HWND hMDI, HPALETTE hPal )
{
	HWND	   hWnd;
	HDC		hDC;
	int		cxScr, cyScr;
	PWCS	   pwcs;
	HANDLE	hDIBInfo;
	PDI      lpDIBInfo;
	POINT	   pt;
	DWORD	   PalStyle;
	LPTSTR	lpt;
	BOOL	   flg = FALSE;
#ifdef   USEISIZE2
   HANDLE   hDIB;
	DWORD	   wNCols;
	LPSTR	   lpbi;
#endif   // #ifdef   USEISIZE2

	hDC = GetDC( NULL );
	gcxScr = GetDeviceCaps( hDC, HORZRES );
	gcyScr = GetDeviceCaps( hDC, VERTRES );
	ReleaseDC( NULL, hDC );

	/* get 1 / 4 across */
	//cxScr = gcxScr / 4;
	//cyScr = gcyScr / 4;
	cxScr = gcxScr * 9 / 10;   // nearly FILL the screen
	cyScr = gcyScr * 9 / 10;
//	cyScr = gcyScr / 2;
//typedef struct	tagPWCS	{	/* pw */
//	HWND	pw_hMDI;	// "Parent" MDI Child (But can be DESTROYED)
//	HGLOBAL	pw_hDIBInfo;// Likewise - DIB Info of child, but can be DESTROYED
//}PWCS;
//typedef PWCS MLPTR LPPWCS;
	pwcs.pw_hMDI     = 0;
	pwcs.pw_hDIBInfo = 0;
	hDIBInfo         = 0;
	PalStyle         = DEFPALSTYLE;
	//lpt              = lpTit;
	//if( ( lpt[0] == szDIBPalTitle[0] ) &&  // Title on DIB Palette Window
	//	 ( hMDI = GetCurrentMDIWnd() ) )
   lpt = &gszTmp[0];
   if( hMDI )
	{
      strcpy( lpt, "DIB Palette - " );
      GetWindowText( hMDI, EndBuf(lpt), 256 );
		pwcs.pw_hMDI     = hMDI;
		//gsWinPac.length = sizeof(WINDOWPLACEMENT);
		//flg = GetWindowPlacement( hMDI, &gsWinPac );
		//flg = GetWindowRect( hMDI, &grWinRec );
		if( hDIBInfo = GetWindowExtra( hMDI, WW_DIB_HINFO ) )
		{
			if( lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo ) )
			{
      		pwcs.pw_hDIBInfo = hDIBInfo;
#ifdef   USEISIZE2
				if( hDIB = lpDIBInfo->hDIB )
				{
					if( lpbi = DVGlobalLock( hDIB ) )   // LOCK DIB HANDLE
					{
						wNCols = DIBNumColors( lpbi );
						//DVGlobalUnlock( hDIB );
						PalInitSize( wNCols, &pt );
						if( pt.x > cxScr )
							cxScr = pt.x;
						if( pt.y > cyScr )
							cyScr = pt.y;
						DVGlobalUnlock(hDIB);   // UNLOCK DIB HANDLE
					}
					else
					{
						chkit( "MEMORY!!! FAILED TO GET POINTER!!!" );
					}
				}
#endif   // #ifdef   USEISIZE2

				DVGlobalUnlock( hDIBInfo );
			}
			else
			{
				chkit( "MEMORY ERROR!!! FAILED TO GET POINTER!!!" );
			}
		}
	}
   else
   {
      strcpy( lpt, "System Palette" );
   }

   pt.x = ( gcxScr - cxScr ) / 2;
   pt.y = ( gcyScr - cyScr ) / 2;

//	pt.x = pt.y = 0;
//	if( flg )
//	{
//		pt.x = gsWinPac.rcNormalPosition.left;
//		pt.y = gsWinPac.rcNormalPosition.top;
//		if( ( grWinRec.left + cxScr ) < gcxScr )
//			pt.x = grWinRec.left;
//		else
//			pt.x = gcxScr - cxScr;
//
//		pt.y = grWinRec.top;
//		if( ( grWinRec.bottom + cyScr ) < gcyScr )
//			pt.y = grWinRec.bottom;
//		else
//			pt.y = gcyScr - cyScr;
//	}


	// NOTE: The pwcs will be extracted in PalWM_CREATE()
	// ==================================================
	hWnd = CreateWindow( szPalClass,	// Class name using (wc.lpfnWndProc=) PALETTEWNDPROC
		lpt,		   // Title
		PalStyle,	// style
		pt.x,		   // x
		pt.y,		   // y
		cxScr,		// Width
		cyScr,		// Height
		hOwner,		// Parent
		NULL,		   // Menu
		ghDvInst,	// Instance
		(LPVOID) &pwcs );	// Extra params

	if( hWnd )
	{
		SetPaletteWindowsPal( hWnd, hPal );	// Set wDEntries count
		ShowWindow( hWnd, SW_SHOWNORMAL );
		UpdateWindow( hWnd );
	}

}

//---------------------------------------------------------------------
//
// Function:   EnableWindowOptionsMenus
//
// Purpose:    Enable or gray the "Window" and "Options" pull down 
//             menus.
//
// Parms:      bEnable == TRUE if "Window" should be enabled,
//                        FALSE if it should be gray.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------
void EnableWindowAndOptionsMenus( BOOL bEnable )
{

#ifdef   ADDENABWO

	EnableMenuItem( ghFrameMenu,
		WINDOW_MENU,
		MF_BYPOSITION | (bEnable ? MF_ENABLED : MF_GRAYED) );

	EnableMenuItem( ghFrameMenu,
		IDM_OPTIONS,
		MF_BYCOMMAND | (bEnable ? MF_ENABLED : MF_GRAYED));

//#ifdef	ADDRESTORE
//   EnableMenuItem( ghFrameMenu, 
//                   IDM_WINDOWRESTORE,
//                   MF_BYCOMMAND | (bEnable ? MF_ENABLED : MF_GRAYED));
//#endif	/* ADDRESTORE */

	DrawMenuBar( ghFrameWnd );
#endif   // #ifdef ADDENABWO

}

BOOL	ChkChg( LPBOOL lpret, LPBOOL lpg, LPBOOL lpchg )
{
	BOOL	chg;
	BOOL	b1, b2;

	b1 = *lpret;
	b2 = *lpg;
	chg = FALSE;
	if( b1 )
	{
		if( b2 )
		{
			// No change
		}
		else
		{
			*lpg = TRUE;
			chg = TRUE;
		}
	}
	else
	{
		if( b2 )
		{
			*lpg = FALSE;
			chg = TRUE;
		}
	}
	if( chg )
		*lpchg = TRUE;

	return chg;
}

BOOL	ChkChgw( LPWORD lpret, LPWORD lpg, LPBOOL lpchg )
{
	BOOL	chg;
	WORD	b1, b2;

	b1 = *lpret;
	b2 = *lpg;
	chg = FALSE;
	if( b1 != b2 )
	{
		*lpg = b1;
		chg = TRUE;
	}
	if( chg )
		*lpchg = TRUE;

	return chg;
}

BOOL	ChkChgdw( LPDWORD lpret, LPDWORD lpg, LPBOOL lpchg )
{
	BOOL	chg;
	DWORD	b1, b2;

	b1 = *lpret;
	b2 = *lpg;
	chg = FALSE;
	if( b1 != b2 )
	{
		*lpg = b1;
		chg = TRUE;
	}
	if( chg )
		*lpchg = TRUE;

	return chg;
}

void	Chk4Change( LPOPTIONSINFO lpo )
{
	ChkChg( &lpo->bStretch2W,
		&gfStretch,
		&gfChgStretch );
   ChkChg( &lpo->bAspect, &gbKeepAspect, &gbChgKeepAsp );
//#ifdef  WIN32
	ChkChg( &lpo->bCenter,
		&gfPrtCenter,
		&gfChgPrtCent );
//#else // !WIN32
#ifndef  WIN32
	ChkChg( &lpo->bPrinterBand,
		&gfPrinterBand,
		&gfChgPrinterBand );
	ChkChg( &lpo->bUse31PrintAPIs,
		&gfUse31PrintAPIs,
		&gfChgPrintAPIs );
#endif   // !WIN32
#ifdef   WIN32
	ChkChg( &lpo->wDispOption,
		&gwDispOption,
		&gfChgDispOption );
	ChkChg( &lpo->wPrintOption,
		&gwPrintOption,
		&gfChgPrintOption );
	ChkChg( &lpo->wXScale,
		&gwXScale,
		&gfChgXScale );
	ChkChg( &lpo->wYScale,
		&gwYScale,
		&gfChgYScale );
#else // !WIN32
	ChkChgw( &lpo->wDispOption,
		&gwDispOption,
		&gfChgDispOption );
	ChkChgw( &lpo->wPrintOption,
		&gwPrintOption,
		&gfChgPrintOption );
	ChkChgw( &lpo->wXScale,
		&gwXScale,
		&gfChgXScale );
	ChkChgw( &lpo->wYScale,
		&gwYScale,
		&gfChgYScale );
#endif   // WIN32 y/n
   // update a possible CHANGED default ms animation timer
	ChkChgdw( &lpo->dwMilSecs,
		&gdwMilSecs,
		&gfChgMilSecs );
   // =====================================================
	ChkChg( &lpo->bSetDefault,
		&gfSetDefault,
		&gfChgSetDefault );
	ChkChg( &lpo->bApplyAll,
		&gfApplyAll,
		&gfChgApplyAll );
	ChkChg( &lpo->bAnimate,
		&gfAnimate,
		&gfChgAnimate );
}

// handle the IDM_OPTIONS
void Do_IDM_OPTIONS( HWND hWnd )
{
   static OPTIONSINFO OptionsInfo;  // static option set
	HWND        hDIBWnd   = 0;
	HANDLE      hDIB      = 0;
	HANDLE      hDIBInfo  = 0;
	LPDIBINFO   lpDIBInfo = 0;
	BOOL        bOldStretch, bOldAnim;

	//if( ( hDIBWnd = GetCurrentMDIWnd() ) &&
	//	( hDIBInfo = GetWindowExtra( hDIBWnd, WW_DIB_HINFO ) ) &&
	//	( 	lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo ) ) )
	hDIBWnd = GetCurrentMDIWnd();
   if(hDIBWnd)
      hDIBInfo = GetWindowExtra( hDIBWnd, WW_DIB_HINFO );
   if(hDIBInfo)
      lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo );
   if(lpDIBInfo)
	{
		// If a current window, info handle and pointer
		// Set up data for stretch dialog box.
		hDIB = lpDIBInfo->hDIB; 
		if(hDIB)
		{
         // copy the current options
			OptionsInfo = lpDIBInfo->Options;

			bOldStretch = OptionsInfo.bStretch2W;
			bOldAnim    = OptionsInfo.bAnimate;

			ShowOptions( hWnd, &OptionsInfo );
         if( OptionsInfo.bSetDefault ) { // transfer these OPTIONS
				Chk4Change( &OptionsInfo ); // to the GLOBAL, for INI save
			}
			lpDIBInfo->Options = OptionsInfo;

			// If the stretch option changed, need to repaint.
         if( lpDIBInfo->Options.bStretch2W != bOldStretch ) {
            // effect a re-paint
				InvalidateRect( hDIBWnd, NULL, bOldStretch );
         }

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
		 // If it is an Animation GIF 
			if(( OptionsInfo.bIsAGIF              ) && 
				( OptionsInfo.bAnimate != bOldAnim ) ) {
				// and a Change in Animation status
				if( OptionsInfo.bAnimate ) {
               if( AddToAnims( hDIBWnd ) ) { // If an error in adding
						lpDIBInfo->Options.bAnimate = FALSE;	/* Kill request */
               } else {
						lpDIBInfo->dwMilCnt = lpDIBInfo->Options.dwMilSecs -
								TIMER_INTERVAL1;	// Set to move on NEXT tick
					}
            } else {
					DeleteAnim( hDIBWnd );	// Remove form Timer List
				}
			}
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

		}
		DVGlobalUnlock( hDIBInfo );	// Unlock pointer
	}
	else
	{
		// NO CHILD WINDOWS OPEN
		//OptionsInfo = lpDIBInfo->Options;
		OptionsInfo.bStretch2W = gfStretch;
      OptionsInfo.bAspect= gbKeepAspect;
//#ifdef  WIN32
		OptionsInfo.bCenter         = gfPrtCenter;
//#else // !WIN32
#ifndef  WIN32
		OptionsInfo.bPrinterBand    = gfPrinterBand;
		OptionsInfo.bUse31PrintAPIs = gfUse31PrintAPIs;
#endif   // WIN32 y/n
		OptionsInfo.wDispOption = gwDispOption;
		OptionsInfo.wPrintOption = gwPrintOption;
		OptionsInfo.wXScale = gwXScale;
		OptionsInfo.wYScale = gwYScale;
      // set the DEFAULT ms animation timer
		OptionsInfo.dwMilSecs = gdwMilSecs;
      // ==================================
		OptionsInfo.bSetDefault = gfSetDefault;
		OptionsInfo.bApplyAll = gfApplyAll;
		OptionsInfo.bAnimate = gfAnimate;
		OptionsInfo.bIsAGIF = FALSE; 
		ShowOptions( hWnd, &OptionsInfo );
		Chk4Change( &OptionsInfo );
	}
	return;

}	// end -  Do_IDM_OPTIONS()

#ifdef	ADDOPENALL

extern	LPSTR	SRGetSFil( UINT ind );  // get next file from gsFindList
// return ZERO if none

extern	UINT	SRSearch( HWND );	/* In SRSearch.c ... */
//extern	BOOL	fInSearch;

/*	======================================================
	void	SROpenAs( HWND hwnd )

	PURPOSE: Impliments IDM_OPENAS from MENU command
  
	======================================================	*/
void	SROpenAs_OLD( HWND hwnd )
{
	HANDLE hDIB;
	UINT	i, scnt;
	LPSTR	lpf;
	PRDIB prd = (PRDIB)MALLOC( sizeof(RDIB) );
   if(!prd)
      chkme( "C:ERROR: No memory!!!"MEOR );
	NULPRDIB( prd );

	if( scnt = SRSearch( hwnd ) )	/* In SRSearch.c ... */
	{
		Hourglass( TRUE );
		gfInSearch = TRUE;
		// When this is a LARGE NUMBER, need a way to
		// BREAK the continuation if ERRORS start, usually
		// due to the OS running out of resources!!!
		for( i = 0; i < scnt; i++ )
		{
			if( lpf = SRGetSFil( i ) )    // if we can get the NEXT
			{
            prd->rd_pTitle = gszRPTit;
            prd->rd_pPath  = gszRPNam;
            strcpy( gszRPTit, lpf );
            DVGetFullName2( gszRPNam, gszRPTit );
				prd->rd_hWnd = hwnd;
				//rd.rd_pFName = lpf;
				OpenDIBFile2( prd );
				//hDIB = OpenDIBFile( lpf );
				if( hDIB = prd->rd_hDIB )
				{	/* We have a DIB ... */
					prd->rd_Caller = df_SEARCH;
					if( OpenDIBWindow2( prd ) )
               {
					   //OpenDIBWindow( hDIB, lpf, df_SEARCH );
#ifdef	CHGADDTO
					   //AddToFileList( lpf );
                  //AddToFileList4( &rd );
                  ADD2LIST(prd);
#endif	// CHGADDTO
               }
				}
			}
		}
		Hourglass( FALSE );
		gfInSearch = FALSE;
	}
   MFREE(prd);

}	// void	SROpenAs( HWND hwnd )


#endif	/* ADDOPENALL */

#ifdef	ADDRESTORE

void	RestWin( HWND hc, BOOL fPos )
{
   HGLOBAL	hg;
   LPDIBINFO	lpdi;
   RECT	rc, rw;
	if( hc )
	{
		if( hg = GetWindowExtra( hc, WW_DIB_HINFO ) )
		{
			if( lpdi = (LPDIBINFO) DVGlobalLock( hg ) )
			{
				GetClientRect( hc, &rc );
				GetWindowRect( hc, &rw );
				if( (lpdi->di_dwDIBWidth != (DWORD) rc.right) ||
					(lpdi->di_dwDIBHeight != (DWORD) rc.bottom) ||
					(lpdi->rClient.left != rw.left) ||
					(lpdi->rClient.top  != rw.top ) )
				{
					if( fPos )
					{
						SetWindowPos( hc,
							HWND_BOTTOM,
							lpdi->rClient.left,
							lpdi->rClient.top,
							(lpdi->rWindow.right - lpdi->rWindow.left),
							(lpdi->rWindow.bottom - lpdi->rWindow.top),
							SWP_SHOWWINDOW );
					}
					else
					{
						MoveWindow( hc,
							lpdi->rClient.left,
							lpdi->rClient.top,
							(lpdi->rWindow.right - lpdi->rWindow.left),
							(lpdi->rWindow.bottom - lpdi->rWindow.top),
							TRUE );
					}
				}
				DVGlobalUnlock( hg );
			}	
		}
	}
}

//	Check if CHILD is Open
BOOL	IsChildOpen( HWND hwnd )
{
	BOOL	rflg;
//	HGLOBAL	hg;
//	WORD	cnt;
//	LPWORD	lpw;
	ENUSTR	es;
//	cnt = 0;

	if( hwnd )
	{
		memset( &es, 0, sizeof(ENUSTR) );
//	if( hg = DVGlobalAlloc( GHND, 10 ) )
//	{
//		if( lpw = (LPWORD) DVGlobalLock( hg ) )
//		{
//			*lpw = 0;
//			DVGlobalUnlock( hg );
			es.es_code = ENU_FINDHWND;
			es.es_hwnd = hwnd;
			EnumAllKids( &es );
			if( es.es_hwnd == 0 )
			{
				rflg = TRUE;	// We have an ANSWER
			}
//			if( lpw = (LPWORD) DVGlobalLock( hg ) )
//			{
//				cnt = *lpw;
//				DVGlobalUnlock( hg );
//			}
//		}
//		DVGlobalFree( hg );
//
	}
	return( rflg );
}
DWORD	GetSizeCC( void )
{
	return( 10 );
}

WORD	GetChildCount( void )
{
	HGLOBAL	hg;
	WORD	cnt;
	LPWORD	lpw;
	ENUSTR	es;
	cnt = 0;
//	if( hg = DVGlobalAlloc( GHND, GetSizeCC() ) )
	if( hg = DVGAlloc( "CHILDCNT", GHND, GetSizeCC() ) )
	{
		if( lpw = (LPWORD) DVGlobalLock( hg ) )
		{
			*lpw = 0;
			DVGlobalUnlock( hg );
			es.es_code = ENU_COUNT;
			es.es_hand = hg;
			EnumAllKids( &es );
			if( lpw = (LPWORD) DVGlobalLock( hg ) )
			{
				cnt = *lpw;
				DVGlobalUnlock( hg );
			}
		}
		DVGlobalFree( hg );
	}
return( cnt );
}

void	RestoreWindow( HWND hwnd )
{
HWND	hc;
	if( hc = GetCurrentMDIWnd () )
		RestWin( hc, FALSE );
}

void	RestAllWindow( HWND hwnd )
{
	ENUSTR	es;
//WORD	cnt;
//	cnt = GetChildCount();
	es.es_code = ENU_RESTORE;
	es.es_hand = 0;
	EnumAllKids( &es );
}
#endif	/* ADDRESTORE */

void HelpIni( LPSTR lpCD )
{
	char tmp[MAX_FILENAME+2];
	int	i, j;
	LPSTR	lpb;
	HFILE	hf;
	if( lpCD && (i = lstrlen( lpCD )) )
	{
		lpb = &tmp[0];
		lstrcpy( lpb, lpCD );
		if( lpb[i-1] != '\\' )
			lstrcat( lpb, &szBS[0] );
		lstrcat( lpb, &szDefHelpFile[0] );
		hf = _lopen( lpb, (OF_READ | OF_SHARE_COMPAT) );
		if( hf && (hf != HFILE_ERROR) )
		{
			DVlclose( IntToPtr(hf) );
			lstrcpy( &gszHelpFileName[0], lpb );
		}
		else if( i > 2 )
		{
			lstrcpy( lpb, lpCD );
			if( lpb[i - 1] == '\\' )
				lpb[i - 1] = 0;
			for( j = (i - 2); j > 0; j-- )
			{
				if( lpb[j] == '\\' )
				{
					lpb[j+1] = 0;
					break;
				}
			}
			if( j )
			{
				lstrcat( lpb, &szDefHelpFile[0] );
				hf = _lopen( lpb, (OF_READ | OF_SHARE_COMPAT) );
				if( hf && (hf != HFILE_ERROR) )
				{
					DVlclose( IntToPtr(hf) );
					lstrcpy( &gszHelpFileName[0], lpb );
				}
			}
		}
	}
}

/* =========================================
				NULRDIB( rd );
				rd.rd_hWnd   = hWnd;
				rd.rd_pFName = lps;
				OpenDIBFile2( &rd );
				//hDIB = OpenDIBFile( lps );
				if( hDIB = rd.rd_hDIB )
				{
					rd.rd_Caller = df_IDOPEN;
					OpenDIBWindow2( &rd );
					//OpenDIBWindow( hDIB, lps, df_IDOPEN );
#ifdef	CHGADDTO
					AddToFileList( lps );
#endif	// CHGADDTO
				}
#ifdef	TRANSGIF
				else if( ( rd.rd_dwFlag & gd_GIFInfo ) && // We are returning GIFHDREXT[n]
					( rd.rd_hGIFInfo ) )
				{
					rd.rd_Caller |= df_IDOPEN;
					OpenDIBWindow2( &rd );
					//OpenDIBWindow( hDIB, lps, df_IDOPEN );
#ifdef	CHGADDTO
					AddToFileList( lps );
#endif	// CHGADDTO
				}
#endif	// TRANSGIF
			}

   ========================================= */

// called from several places,
// but one is an IDM_FILE?? command for an MRU file that is NOT
// already OPEN, so LOAD IT

PMWL  CommonFileOpen( HWND hWnd, LPSTR lpf, DWORD Caller )
{
   PMWL     pmwl = 0;
	HANDLE	hDIB;
	int	   npDIBs = gnDIBsOpen;	// If already HAVE CHILDREN
	PRDIB prd = (PRDIB)MALLOC( sizeof(RDIB) );
   if(!prd)
      chkme( "C:ERROR: No memory!!!"MEOR );
	NULPRDIB( prd );

   prd->rd_pTitle = gszRPTit;
   prd->rd_pPath  = gszRPNam;
	prd->rd_hWnd   = hWnd;			// frame parent window
	//rd.rd_pFName = lpf;			// name of file
   strcpy(gszRPTit,lpf);
   DVGetFullName2( gszRPNam, gszRPTit );
	prd->rd_Caller = Caller;		// caller ID

	/* go do the OPEN A FILE */
	OpenDIBFile2( prd );
	if( hDIB = prd->rd_hDIB )
	{
		prd->rd_Caller |= Caller;
		if( OpenDIBWindow2( prd ) )
      {
//#ifdef	CHGADDTO
         //AddToFileList4( &rd );
		   //AddToFileList( lpf );
         ADD2LIST(prd);
//#endif	// CHGADDTO
         pmwl = prd->rd_pMWL;
      }
	}
#ifdef	TRANSGIF
	else if( ( prd->rd_dwFlag & gd_GIFInfo ) && // We are returning GIFHDREXT[n]
		( prd->rd_hGIFInfo ) )
	{
		prd->rd_Caller |= Caller;
		if( OpenDIBWindow2( prd ) )
      {
		//OpenDIBWindow( hDIB, lps, df_IDOPEN );
//#ifdef	CHGADDTO
         //AddToFileList4( &rd );
		   //AddToFileList( lpf );
         ADD2LIST(prd);
//#endif	// CHGADDTO
         pmwl = prd->rd_pMWL;
         //bRet = TRUE;
      }
	}
#endif	// TRANSGIF
	else	// We FAILED to LOAD FILE
	{
		// BUT remember say GIF has already
		// established a WINDOW by now ...
		if( npDIBs == gnDIBsOpen )
		{
			// NOTHING GOT OPENNED
			SetReadPath( lpf );
		}
	}

   MFREE(prd);

   return pmwl;   // VALID if something openned

}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Do_IDM_OPEN
// Return type: void 
// Argument   : HWND hWnd
// Description: handle IDM_OPEN input from user
//              Establish a read DIB (PRDIB) structure to
// pass the parameters as a single pointer.
///////////////////////////////////////////////////////////////////////////////
void	Do_IDM_OPEN( HWND hWnd )
{
   HANDLE   hDIB;
   LPTSTR   lpf = &gszRPNam[0];
   PRDIB    prd = (PRDIB)MALLOC(sizeof(RDIB));
   if(!prd)
   {
      chkme( "C:ERROR: Meory failed!"MEOR );
      return;
   }
	//NULRDIB(rd);
   ZeroMemory(prd, sizeof(RDIB));

   *lpf = 0;   // Open means NO FILE NAME yet ...
   prd->rd_pTitle = gszRPTit;
   prd->rd_pPath  = lpf;  // = gszRPNam
	prd->rd_hWnd   = hWnd;
	prd->rd_Caller = df_IDOPEN;
   hDIB = OpenDIBFile( lpf ); 
   if(hDIB)
   {
      prd->rd_hDIB = hDIB;
      DVGetFullName2( gszRPNam, gszRPTit );
		if( OpenDIBWindow2( prd ) )
      {
#ifdef	CHGADDTO
         //AddToFileList4( &rd );
         ADD2LIST(prd);
#endif	// CHGADDTO
      }
   }
   MFREE(prd);
}

// process IDM_EXIT from the MENI
void	Do_IDM_EXIT( HWND hWnd )
{
   sprtf( "Processing IDM_EXIT for frame %#x."MEOR, hWnd );
   UpdateWP( hWnd );
   gbInClose = TRUE;       // we are CLOSING
	SendMessage( hWnd, WM_CLOSE, 0, 0L );
}


void	Do_IDO_SAVEINI( HWND hWnd )
{
	InvertINI( hWnd );
}

void	Do_IDM_EXITNOSAVE( HWND hWnd )
{
	if( gfSavINI )
	{
		gfSavINI = FALSE;
		gfChgSI = TRUE;
	}
	Do_IDM_EXIT( hWnd );
}

void	Do_IDM_EXITWSAVE( HWND hWnd )
{
	if( !gfSavINI )
	{
		gfSavINI = TRUE;
		gfChgSI = TRUE;
	}
	Do_IDM_EXIT( hWnd );
}


void	Frm_WM_DESTROY( HWND hWnd )
{
	fInDestroy = TRUE;	// set GLOBAL EXIT FLAG!!!
	NewFileTerm();	// Delete NEW files ...

#ifdef	DIAGSAVEHOOK
	EndDiagHook();
#endif
	if( fHelpUp )
		Do_IDM_HELP( hWnd, HELP_QUIT ) ;
	fHelpUp = 0;

	WriteIni( hWnd );

#ifdef	ADDTIMER1
	if( GotTimer )
	{
		KillTimer( ghMainWnd, TIMER_ID1 );
		GotTimer = 0;
	}
#endif	/* ADDTIMER1 */

#ifdef	ADDWJPEG		/* Use the JPEG library for reading and writing */
							/* JPG, GIF, PPM, RLE ... and other we may add ... */
#ifdef	LDYNALIB	/* NO LINK with LIBRARY - Use Dynamic LoadLibrary ... */
	CloseJPEGLib();	/* Reduce instance use count by 1 ... */
#endif	// LDYNALIB
#endif	// ADDWJPEG

	DeleteTools();

	// Program Termination.
	PostQuitMessage(0);

}

VOID  Do_IDM_COPYSAVE( hWnd )
{
   // Handle the clipboard copy operation.
   //		case IDM_COPY:
   if( HandleCopyClipboard() )
   {
		// Handle the clipboard paste operation.
//		case IDM_PASTE:
      HandlePasteClipboard ();
//			break;
   }
}


VOID  Do_IDM_RESTORE2( hWnd )
{
   HWND     hMDI      = 0;
   HGLOBAL  hDIBInfo  = 0;
   PDI      lpDIBInfo = 0;
   RECT     rcWindow;

	//if( ( hMDI = GetCurrentMDIWnd()                       ) &&
	//	 ( hDIBInfo = GetWindowExtra( hMDI, WW_DIB_HINFO ) ) &&
	//	 ( lpDIBInfo = (PDI) DVGlobalLock( hDIBInfo )      ) )
	hMDI = GetCurrentMDIWnd();
   if(hMDI)
      hDIBInfo = GetWindowExtra( hMDI, WW_DIB_HINFO );
   if(hDIBInfo)
      lpDIBInfo = (PDI) DVGlobalLock( hDIBInfo );
   if(lpDIBInfo)
	{
      // Determine the necessary window size to hold the DIB.
      rcWindow.left   = 0;
      rcWindow.top    = 0;
      rcWindow.right  = (int) lpDIBInfo->di_dwDIBWidth;
	   rcWindow.bottom = (int) lpDIBInfo->di_dwDIBHeight;
      // ===============================================

      AdjustWindowRect( &rcWindow,
         ( WS_CHILD | WS_SYSMENU | WS_CAPTION |
			  WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX ),
         FALSE );

      sprtf( "Restoring to %s."MEOR,
         Rect2Stg( &rcWindow ) );
      
      SetWindowPos( hMDI,  // handle to window
         0,    // placement-order handle
         0,    // horizontal position
         0,    // vertical position
         rcWindow.right  - rcWindow.left,   // width
         rcWindow.bottom - rcWindow.top,  // height
         SWP_NOMOVE | SWP_NOZORDER );  // UINT - window-positioning options

      DVGlobalUnlock(hDIBInfo);
   }
}

// eof - End DvFrame.c

