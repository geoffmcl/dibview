
/* ************************************************************************

      File:  DVCHILD.C

   Purpose:  Contains the routines to implement displaying a bitmap in
             an MDI child window.  Each window has its own set of
             information stored in its window words which identifies
             the bitmap to be displayed, etc..

 Functions:  CHILDWNDPROC
             ChildWndCreate
             ChildWndPaint
             ChildWndDestroy
             ChildWndScroll
             ChildWndKeyDown
             ChildWndQueryNewPalette
             ChildWndPaletteChanged
             ChildWndStartAnimate
             ChildWndLeftButton
             ChildWndSize
             SetupScrollBars
             ScrollBarsOff
             GetCurrentMDIWnd
             CurrentDIBPalette
             GetCurrentDIBStretchFlag
             SetCurrentDIBStretchFlag
             ReallyGetClientRect
             SetMessageToAllChildren
             CloseAllDIBWindows
             DrawSelect - add or remove CLIP rectangle
             GetCurrentClipRect
             GetCurrentDIBSize

  Comments:  Special considerations are made in this module to get
             Windows to handle the system palette correctly for
             child windows (MDI child windows in this case).  
             
             Noramlly, an application with the focus has the
             "foreground" palette.  Since this application uses
             multiple palettes (one per bitmap), special handling
             must be used to force all the windows without the focus
             to "background" palettes.

             We accomplish this by forcing the "active" child window's
             palette to be the whole application's foreground palette
             (i.e. get a DC for the frame, and realize the child's
             palette in that DC as a foreground palette).

             All other times we realize palettes, we realize them
             as background palettes.  If the palette being realized
             was realized as the frame's foreground palette, then
             when it is realized as a background palette, all the
             colors will map to the correct colors!  If the palette
             being realized was not the foreground palette, the colors
             are mapped into the system palette on a "best match"
             basis.

   History:   Date      Reason
             6/ 1/91     Created
            12/03/91     Fixed palette handling code.

************************************************************************ */
#include	"dv.h"	/* Single inclusive include ... */
#include	"DvClip.h"
#include	"DvPal24.h"
#include	"GMUtils.h"		// include GmUtil.h
#include	"WMDiag.h"		// include ..\Utils\WmDiag.h ...

#define		DIAGSCROLL
#undef      CHILDWNDPNT
#define		ONESCROLL

#define		USEDIBSZ		// Rely on lpDIBInfo, NOT hBitmap
#undef		SHOWDEP			// SHow depth at end
#undef		DIAGTE			// Check of TEXT EXTENT
#undef		SHOWEACLIP		// Output EACH Clip region
#define		SHOWCLIP		// Output CLIP begin, changes and end

#ifdef   NDEBUG
#undef   DIAGWM2
#else // !NDEBUG
#undef   DIAGWM2
//#define   DIAGWM2
#endif   // #ifdef   NDEBUG y/n

// Reasons for ABORT of CHILD WINDOW PAINT
// =======================================
#define		SA_NoDIB			1
#define		SA_NoPtr			2
#define		SA_NohDIB		3
#define		SA_NoBmp			4
#define		SA_NohmDC		5
#define		SA_WtThrd		6

// Some magic numbers.
//#define SCROLL_RATIO    4     // WM_VSCROLL scrolls DIB by 1/x of client area.
//#define	GFACTOR10		1000	// This seems MUCH larger
// than the DEFINITION which says 1/100th of a seconds. But this
// is STILL twice as fast as GIFCON!!! What should it be???

   // The following defines are the default values for the OPTIONSINFO
   //  structure stored in the child window's INFOSTRUCT (which is a
   //  structure containing information on the child window's bitmap,
   //  and list of options).  See ChildWndCreate() to see how these
   //  are used.

// Some useful macros.
//#define MAX(a,b)     ((a) > (b) ? (a) : (b))
//#define MIN(a,b)     ((a) < (b) ? (a) : (b))

#ifdef	WIN32
#ifdef _WIN64
#pragma message ( "Child window (64-Bit)" )
#else
#pragma message ( "Child window (32-Bit)" )
#endif // _WIN64 y/n

#ifndef	Dv32	// To be retired
extern	DWORD	GetTextExtent( HDC hDC, LPSTR lps, int len );
#endif	// !Dv32

#else	// !WIN32
// NOT in WIN32
#pragma message ( "This is a 16-bit port ..." )
extern	int wscanf( LPSTR, LPSTR, void MLPTR );
#endif	// !WIN32

// Some externals
extern	void AddToFileList( LPSTR );
extern   DWORD DVFileSize( HANDLE );
extern	DWORD	DVRead( HANDLE, LPSTR, DWORD );
extern	BOOL	AddToAnims( HWND );
extern	BOOL	DeleteAnim( HWND );
extern	DWORD	GetAnims( HWND FAR *, DWORD );
extern	DWORD Gif_Count( LPSTR, DWORD );

extern	void	ChildWndTimer(HWND hWnd, WPARAM wParam, LPARAM lParam );

// Combine a "transparent" BMP with a COLORED background
extern	BOOL	dvTransparentBlt( HDC hDC, HBITMAP srcBmp,
					   int destX, int destY,
					   COLORREF	dwTransColor );
// NEW - Dec 2000
extern   DWORD SetFileMRU( HWND hwnd, PLIST_ENTRY pHead );
extern   DWORD WINAPI CountColorProc( LPVOID lpv ); // thread data
extern   PMWL  UnloadMRU( HWND hMDI, LPTSTR lpFile );

#if	(defined( DIAGSCROLL ) || defined( SHOWCLIP ))
void	ShowRemove( LPRECT lprcClip );
void	ShowBegin( LPRECT lpClipRect,
				  LPPOINT lpptOrigin, LPPOINT lpptStart,
				  LPRECT lprcClient,
				  int cxDIB, int cyDIB );
void	DiagDrawSel( LPRECT lprcClip, LPPOINT lpptStart );
void	ShowFinal( LPRECT lpClipRect,
				  LPPOINT lpptOrigin, LPPOINT lpptStart,
				  LPRECT lprcClient,
				  int cxDIB, int cyDIB );
#endif	// DIAGSCROLL or SHOWCLIP

// Some globals.
HWND	   hWndAnimate = NULL;     // HWND of currently palette animated DIB.
HWND	   hWndClip    = NULL;     // Current Window to be rendered to clipboard.
POINT	   ptClipSize  = {0,0};    // Size of DIB at time of copy (i.e. was the DIB stretched?)
LPTSTR   pRDibTit    = "%s (%ldx%ld %d BPP)";	// For MDI Window Title
int		iGifDelay   = GFACTOR10;

// This is for the PARENT's use - NOT really a CHILD
//HGLOBAL	hgDIBInfo = 0;	// hDIBInfo;
//HGLOBAL	hgDIBx    = 0;	// hDIBx;
// =================================================
static	char	szFName[MAX_FILENAME];
//static	char	szTBuf[MAX_FILENAME];

// Local function prototypes.
long  ChildWndCreate      (HWND hWnd, LPDIBCREATEINFO lpDIBCreateInfo);
void  ChildWndPaint       (HWND hWnd, WPARAM wParam);
void  ChildWndDestroy     (HWND hWnd);
void  ChildWndScroll      (HWND hWnd, int message, WORD wPos, WORD wScrollType);
BOOL  ChildWndQueryNewPalette (HWND hWnd, HWND hWndFrame);
void  ChildWndPaletteChanged (HWND hWnd);
void  ChildWndStartAnimate(HWND hWnd);
void  ChildWndLeftButton  (HWND hWnd, int x, int y);
DWORD ChildWndKeyDown     (HWND hWnd, WORD wKeyCode, LPARAM lParam);
void  ChildWndSize        (HWND hWnd, WPARAM wParam, LPARAM lParam );
long	ChildWndClose( HWND hWnd, UINT message,
					  WPARAM wParam, LPARAM lParam);
void  SetupScrollBars     (LPDIBINFO lpDIBInfo, HWND hWnd,
						   DWORD cxDIB, DWORD cyDIB);
void  ScrollBarsOff       (LPDIBINFO lpDIBInfo, HWND hWnd);
void  ReallyGetClientRect (HWND hWnd, LPRECT lpRect);
void  DrawSelect          (HDC hDC, LPRECT lprcClip);
HPALETTE	CreateDIBPal24( PDI, HANDLE, BOOL );
#ifdef	ADDTIMER1
void	NetAnim( HWND );
#endif	/* ADDTIMER1 */
void	PalAnim( HWND );
#if	(defined(USECTHREAD) && defined(USETHRDS))
void	ChildWndThreadDone( HWND hWnd, WPARAM wParam, LPARAM lParam );
#endif	// USECTHREAD & USETHRDS
VOID  ChildWndCountDone( HWND hWnd );

#ifdef	ADDCOLRF
extern	BOOL	fChgGBack;
extern	BOOL	fChgGShad;
extern	BOOL	fChgGHigh;
extern	BOOL	fChgGText;
//	case WM_ERASEBKGND:
BOOL	ChildWndEraseBkGnd( HWND hWnd, HDC hDC );
DIBX	gDIBx = { 0 };
BOOL	fUdtgDIBx = TRUE;
BOOL	fChgUDIBx = FALSE;
#endif	// ADDCOLRF
BOOL	fAddPat = TRUE;	// Paint the PATTERNS also
DWORD	dwDepth = 0;
BOOL	bInWMLDown = FALSE;

void	chkchk( void )
{
	int	i;
	i = 0;
}

VOID  chkpaint( VOID )    // just a DEBUG stop
{
   int   i;
   i = 0;
}

#if	(defined( SHOWWM ) || defined( DIAGWM2 ))

BOOL	bChildExcl( UINT uMsg )
{
	BOOL	bExcl = FALSE;

//	if( bNotExcluded( uMsg ) )
	if( bInWMLDown )
	{
		// exclude nothing
	}
	else	// exclude semoe mousey stuff
	{
		if( ( uMsg == WM_MOUSEMOVE ) ||
			( uMsg == WM_NCHITTEST ) ||
			( uMsg == WM_SETCURSOR ) )
		{
			bExcl = TRUE;
		}
	}
	return bExcl;
}

void DiagChildMsg( HWND hWnd, UINT message,
				  WPARAM wParam, LPARAM lParam )
{
	if( VERBAL9 )
	{
		LPSTR	lpd;
		UINT	uMsg;
		static UINT	uiChildLast;
		static HWND hChildLast;
		static WPARAM wpChildLast;
		static LPARAM lpChildLast;
		static UINT uiChildCnt;
		uMsg = message;
//		if( bNotExcluded( uMsg ) )
//		{
		if( !bChildExcl( uMsg ) )
		{
			if( ( uiChildLast == uMsg ) &&
				( hChildLast == hWnd ) )
			{
				// update with latest params
				wpChildLast = wParam;
				lpChildLast = lParam;
				uiChildCnt++;
				return;
			}

//			else
//			{
//				lpd = GetDiagBuf();
				lpd = GetTmp3();
				if( uiChildCnt > 1 )
				{
					wsprintf( lpd, "Child: %s (H=0x%x) for %u"MEOR,
						GetWMStg( hChildLast,
							uiChildLast,
							wpChildLast,
							lpChildLast ), hChildLast,
							uiChildCnt );
					//DIAG1( lpd );
					DO(lpd);
				}
				else if( uiChildCnt )
				{
					wsprintf( lpd, "Child: %s (H=0x%x)"MEOR,
						GetWMStg( hChildLast,
							uiChildLast,
							wpChildLast,
							lpChildLast ), hChildLast );
					//DIAG1( lpd );
					DO( lpd );
				}

				uiChildCnt = 0;

				uiChildLast = uMsg;
				hChildLast  = hWnd;
				wpChildLast = wParam;
				lpChildLast = lParam;
				wsprintf( lpd, "Child: %s (H=0x%x)",
					GetWMStg( hWnd, uMsg, wParam, lParam ),
					hWnd );
#ifndef	SHOWDEP
				if( dwDepth > 1 )
				{
					wsprintf( EndBuf(lpd),
						" Dep=%d",
						dwDepth );
				}
#endif	// !SHOWDEP
				lstrcat( lpd, ""MEOR );
				//DIAG1( lpd );
				DO( lpd );
//			}
		}	// NOT in excluded list
	}
}

#endif	// SHOWWM or DIAGWM2

//---------------------------------------------------------------------
//
// Function:   CHILDWNDPROC
//
// Purpose:    Window procedure for DIB MDI child windows.
//             Handles all messages destined for these windows.
//
// Parms:      hWnd    == Handle to this MDI child window.
//             message == Message for window.
//             wParam  == Depends on message.
//             lParam  == Depends on message.
//
// History:   Date      Reason
//             6/01/91  Created
//            10/15/91  Moved WM_CREATE handler to own function.
//                      Moved WM_DESTROY handler to own function.
//                      Moved WM_PAINT handler to own function.
//                      Moved WM_?SCROLL handler to own function.
//             
//---------------------------------------------------------------------

//long MLIBCONV CHILDWNDPROC( HWND hWnd, UINT message,
//						   WPARAM wParam, LPARAM lParam )
LRESULT CALLBACK CHILDWNDPROC(HWND hWnd,      // handle to window
                              UINT uMsg,      // message identifier
                              WPARAM wParam,  // first message parameter
                              LPARAM lParam   // second message parameter
)
{
	LRESULT ret = 0;

	ret = 0;
	dwDepth++;

//#ifdef	SHOWWM
#if	(defined( SHOWWM ) || defined( DIAGWM2 ))
	if( VERBAL9 )
	{
		DiagChildMsg( hWnd, uMsg, wParam, lParam );
	}
#endif	// SHOWWM or DIAGWM2

	//switch( message )
	switch( uMsg )
	{
		// Window being created, do initialization.  In MDI, the lParam
		//  is a pointer to a CREATESTRUCT.  The CREATESTRUCT's
		//  lpCreateParams is a pointer to an MDICREATESTRUCT.
		//  The MDICREATESTRUCT's lParam is an application supplied
		//  LONG.  In DIBView, this LONG is actually a pointer to
		//  a DIBCREATEINFO structure.  This structure is initialized
		//  by the routine which sent the WM_MDICREATE message to the
		//  MDI frame (in FRAME.C).

	case WM_CREATE:
		ChildWndCreate( hWnd,
			(LPDIBCREATEINFO)
			((LPMDICREATESTRUCT)
			((LPCREATESTRUCT) lParam)->lpCreateParams)->lParam);
		break;

		// If this window is being activated, simulate a 
		//  MYWM_QUERYNEWPALETTE message.
	case WM_MDIACTIVATE:
		{
			HWND hWndFrame;
			if( wParam )
			{
				hWndFrame = GetParent( GetParent(hWnd) );
				SendMessage( hWnd, (UINT)MYWM_QUERYNEWPALETTE,
					(WPARAM)hWndFrame, 0L);
			}
		}
		break;

		// Need to paint, call the paint routine.
	case WM_PAINT:		// Paint a MDI child with a DIB
		ChildWndPaint( hWnd, wParam );
		break;

		// User's dragging a minimized MDI child, return the cursor to drag.
	case WM_QUERYDRAGICON:
		LoadCursor( ghDvInst, DRAGCURSOR);
		break;

		// NOTE: Unless CLOSE is FULLY handled it MUST
		// be PASSED to the MDI CLient to handle default.
	case WM_CLOSE:
		ChildWndClose( hWnd, uMsg, wParam, lParam );
		break;

		// Window's being destroyed, call the destroy routine.
	case WM_DESTROY:
		ChildWndDestroy( hWnd );
		break;

		// Ensure that the clipboard data can be rendered even though
		//  this window is being destroyed.  First open the clipboard.
		//  Then empty what we put there earlier and re-render everything.
	case WM_RENDERALLFORMATS:
		{
			if( !OpenClipboard( hWnd ) )
				break;

			EmptyClipboard();

			SendMessage( hWnd, WM_RENDERFORMAT, CF_DIB,     0L );
			SendMessage( hWnd, WM_RENDERFORMAT, CF_BITMAP,  0L );
			SendMessage( hWnd, WM_RENDERFORMAT, CF_PALETTE, 0L );

			CloseClipboard ();
		}
		break;


		// Format the data in the manner requested and pass the handle of
		// the data to the clipboard.

	case WM_RENDERFORMAT:
		{
			HANDLE hClipBoardData;

			hClipBoardData = RenderFormat( hWndClip, (int)wParam, ptClipSize );

			if( hClipBoardData )
				SetClipboardData((UINT) wParam, hClipBoardData );
		}
		break;


		// Window's being scrolled, call the scroll handler.
	case WM_HSCROLL:
	case WM_VSCROLL:
		ChildWndScroll( hWnd, (int)uMsg, LOWORD (lParam), (WORD)wParam);
		break;


		// Keypress -- go handle it.
	case WM_KEYDOWN:
		ChildWndKeyDown( hWnd, (WORD)wParam, lParam );
		break;


		// Window's getting focus, realize our palette.  We set it
		//  up so that the HWND of the frame window is in wParam.
		//  This is so we can realize our palette as the foreground
		//  palette of the *entire* application (Windows is designed
		//  so that the application has one foreground palette -- owned
		//  by the top-level window of the app).  We could realize it
		//  as foreground, and supply our own hWnd, but this can lead
		//  to some weird results...
	case MYWM_QUERYNEWPALETTE:
		//return ChildWndQueryNewPalette (hWnd, (HWND) wParam);
		ChildWndQueryNewPalette (hWnd, (HWND) wParam);
		break;

		// Someone changed the system's palette.  Update our window
		//  to reflect the new palette.
	case WM_PALETTECHANGED:
		if( hWnd == (HWND) wParam )
			break;
		ChildWndPaletteChanged( hWnd );
		break;


		// User wants to animate palette, call routine to start
		//  animation.
	case MYWM_ANIMATE:
		ChildWndStartAnimate( hWnd );
		break;

	case MYWM_THREADDONE:
#if	(defined(USECTHREAD) && defined(USETHRDS))
		ChildWndThreadDone( hWnd, wParam, lParam );
#endif	// USECTHREAD & USETHRDS
		break;

	case MYWM_THREADDONE2:
      // count thread has successfully terminated with COUNT of colours
      ChildWndCountDone( hWnd );
		break;

		// Timer went off -- this only happens when we're animating
		//  the palette. Or if we are ANIMATING GIF's
	case WM_TIMER:
		ChildWndTimer( hWnd, wParam, lParam );
		break;

		// Restore the DIB's palette after palette animation.  Stop
		//  animation.  Delete what we created for animation.  Also, 
		//  we need to re-create the bitmap, since we earlier re-created
		//  it to reflect the animation palette (see MYWM_ANIMATE).
		//  Finally, re-draw the bitmap.
	case MYWM_RESTOREPALETTE:
		{
			HANDLE		hDIB;
			HANDLE		hDIBInfo;
			LPDIBINFO	lpDIBInfo;

			SendMessage( hWnd, WM_RBUTTONDOWN, 0, 0L );

			hDIBInfo = GetWindowExtra( hWnd, WW_DIB_HINFO );

			if( !hDIBInfo )
				break;

			lpDIBInfo = (LPDIBINFO) DVGlobalLock (hDIBInfo);

			if( lpDIBInfo->hBitmap )
				DeleteObject( lpDIBInfo->hBitmap );

			if( lpDIBInfo->hPal )
				DeleteObject( lpDIBInfo->hPal );

			hDIB = lpDIBInfo->hDIB; 
			if(hDIB)
			{
				lpDIBInfo->hPal    = CreateDIBPalette( hDIB );
				lpDIBInfo->hBitmap = DIBToBitmap( hDIB, lpDIBInfo->hPal, lpDIBInfo );
			}

			DVGlobalUnlock( hDIBInfo );

			InvalidateRect( hWnd, NULL, FALSE );
		}
		break;


		// If the user presses the right mouse button and we're
		//  palette animating, stop animating.  The bitmap is
		//  left in it's animated state, as is the palette.  The
		//  restore option must be picked to return the bitmap
		//  palette to their original states.

	case WM_RBUTTONDOWN:
		if( hWndAnimate == hWnd )
		{
			KillTimer( hWnd, TIMER_ID2 );
			hWndAnimate = NULL;
		}
		break;


		// Left button pressed -- track a clipping rectangle for clipboard.
	case WM_LBUTTONDOWN:
#ifdef   ADDSELWIN2 // not working = OFF
      if( gbInSelWin ) // g BOOL = ADDSELWIN2  = global 'sel' proc
      {
         // nothing for child, children to do - frame will do capture/release
      }
      else
#endif   // #ifdef   ADDSELWIN2
      {
   		bInWMLDown = TRUE;
   		ChildWndLeftButton( hWnd, LOWORD (lParam),
   			HIWORD (lParam));
   		bInWMLDown = FALSE;
      }
		break;
#ifdef ADD_CLIP_RESIZING // FIX20051128
   case WM_MOUSEMOVE:
      ChildWndMouseMove( hWnd, wParam, lParam );
      break;
#endif // #ifdef ADD_CLIP_RESIZING // FIX20051128
		// Handle the WM_SIZE message.
		//
		// Note:  This routine calls SetupScrollBars, which can
		//        change the size of the window's client rectangle.
		//        This, in turn, can send a WM_SIZE message to
		//        the window.  An infinite loop occurs because of
		//        this -- therefore, a semaphore is set up to not
		//        allow WM_SIZE to be processed while another
		//        WM_SIZE is still being processed.
	case WM_SIZE:
		{
			static BOOL bInSize = FALSE;
			// Check the semaphore, return if it's set.
			if( !bInSize )
			{
				bInSize = TRUE;
				ChildWndSize( hWnd, wParam, lParam );
				bInSize = FALSE;
			}
		}
		/*** WM_SIZE also calls the default (necessary for MDI) ****/
		// Since the MDI default behavior is a little different,
		// call DefMDIChildProc instead of DefWindowProc().
		ret = DefMDIChildProc( hWnd, uMsg, wParam, lParam );
		break;

#ifdef	ADDCOLRF
	case WM_ERASEBKGND:
		if( !ChildWndEraseBkGnd( hWnd, (HDC)wParam ) )
			ret = DefMDIChildProc( hWnd, uMsg, wParam, lParam );
		break;
#endif	// ADDCOLRF

//   case WM_COMMAND:
//      SendMessage( 	ghFrameWnd, message, wParam, lParam );
      // fall thru ...
	default:
		ret = DefMDIChildProc( hWnd, uMsg, wParam, lParam );
		break;
	}

	dwDepth--;

#if		( ( defined( SHOWWM ) || defined( DIAGWM2 ) ) && defined( SHOWDEP ) )
	if( VERBAL9 )
	{
		LPSTR	lpd = GetTmp1();
		wsprintf( lpd, "Child: OUT 0x%04x Depth=%d"MEOR, uMsg, dwDepth );
		DO(lpd);
	}
#endif	// SHOWWM or DIAGWM2 AND SHOWDEP

	return ret;
}

/* ================================
 * MoveFN( Dest, Source )
 * Moves just the file name, removing square braces ... if existing ...
 * ================================ */
WORD	MoveFN( LPSTR lpd, LPSTR lps )
{
	WORD	i, j, k;
	char	c;
	lstrcpy( lpd, lps );
	if( i = lstrlen( lpd ) )
	{
		k = 0;
		for( j = 0; j < i; j++ )
		{
			c = lpd[j];
			if( c == '[' )
				k = j - 1;
		}
		if( k )
			lpd[k] = 0;
	}
	return( k );
}

#ifdef	ADDCOLRF
LPDIBX	GGetgDIBx( void )
{
	return( &gDIBx );
}
void GDefFace( void )
{
	LPDIBX lpDIBx = GGetgDIBx();
	lpDIBx->dx_Face = GetSysColor( COLOR_3DFACE );
}
void GDefShad( void )
{
	LPDIBX lpDIBx = GGetgDIBx();
	lpDIBx->dx_Shadow = GetSysColor( COLOR_3DSHADOW );
}
void GDefHiLite( void )
{
	LPDIBX lpDIBx = GGetgDIBx();
	lpDIBx->dx_HiLite = GetSysColor( COLOR_3DHILIGHT );
}
void GDefText( void )
{
	LPDIBX lpDIBx = GGetgDIBx();
	lpDIBx->dx_Text = GetSysColor( COLOR_WINDOWTEXT );
}
#endif	// ADDCOLRF

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

// NOTE: ghMaxCount member can not be larger than 0x7fff!
// That's 32,767 images - A reasonable MAXIMUM!!!
// ======================================================
//typedef struct tagGIFIMG {
//	WORD	giWidth;
//	WORD	giHeight;
//	WORD	giCMSize;
//	DWORD	giBMPSize;
//}GIFIMG;
//typedef GIFIMG MLPTR LPGIFIMG;
//
//typedef struct tagGIFHDR {
//	WORD	ghMaxCount;
//	WORD	ghImgCount;
//	WORD	ghWidth;
//	WORD	ghHeight;
//	WORD	ghCMSize;
//	DWORD	ghBMPSize;
//	GIFIMG ghGifImg[1];
//}GIFHDR;
//typedef GIFHDR MLPTR LPGIFHDR;

/* Multiple and Transparent GIF Image Support */
/* ========================================== */
//#define		gie_Flag		0x8000	// This is in the ghMaxCount
// if the application expect an EXTENDED description!

//typedef	struct	tagGIFIMGEXT {
//	GIFIMG	giGI;	// Width/Height/ColSize and BMP Size as above
// Image Descriptor - Wdith and Height in above
//	WORD	giLeft;		// Left (logical) column of image
//	WORD	giTop;		// Top (logical) row
//	BYTE	giBits;		// See below (packed field)
// Graphic Control Extension
//	BYTE	gceExt;		// Extension into - Value = 0x21
//	BYTE	gceLabel;	// Graphic Control Extension = 0xf9
//	DWORD	gceSize;	// Block Size (0x04 for GCE, Big for TEXT)
//	BYTE	gceBits;	// See below (packed field)
//	WORD	gceDelay;	// 1/100 secs to wait
//	BYTE	gceIndex;	// Transparency Index (if Bit set)
//	DWORD	gceColr;	// COLORREF (if SET)
//	DWORD	gceFlag;	// IN/OUT Options Flag - See Below
//	RGBQUAD	gceBkGrnd;	// Background Colour
//	HANDLE	hDIB;		// Handle to the DIB
//	HPALETTE hPal;		// Handle to the bitmap's palette.
//	HBITMAP	hBitmap;	// Handle to the DDB.
//	DWORD	gceRes1;	// Reserved
//	DWORD	gceRes2;	// ditto
//}GIFIMGEXT;
//typedef GIFIMGEXT MLPTR LPGIFIMGEXT;

//typedef struct tagGIFHDREXT {
//	WORD	gheMaxCount;	// gie_Flag + MAX. Count
//	WORD	gheImgCount;	// Images in GIF
//	WORD	gheWidth;		// Logical Screen Width
//	WORD	gheHeight;		// Logical Screen Height
//	WORD	gheCMSize;		// BMP Colour map size (byte count)
//	DWORD	gheBMPSize;		// Estimated final BMP size
//	BYTE	gheBits;		// See below (packed field)
//	BYTE	gheIndex;		// Background Colour Index
//	BYTE	ghePAR;			// Pixel Aspect Ration
//	DWORD	gheFlag;		// IN/OUT Options Flag - See Below
//	RGBQUAD	gheBkGrnd;		// Background Colour
//	HANDLE	hDIB;			// Handle to the DIB
//	HPALETTE hPal;			// Handle to the bitmap's palette.
//	HBITMAP	hBitmap;		// Handle to the DDB.
//	DWORD	gheRes1;		// Reserved
//	DWORD	gheRes2;		// ditto
//	GIFIMGEXT	gheGIE[1];	// 1 for Each Image follows
//}GIFHDREXT;
//typedef GIFHDREXT MLPTR LPGIFHDREXT;

// GIFHDREXT.gheBits
//#define		ghe_ColrMap		0x80	// A Global Color Map
//#define		ghe_ColrRes		0x70	// Colour Resolution
//#define		ghe_Sort		0x08	// Sorted Colour Map
//#define		ghe_ColrSize	0x07	// Size of Colour Table ((n+1)^2)

// GIFIMGEXT gceBits = GIF Graphic Control Extension Bits
// =================
//#define		gce_Reserved	0xe0	// Reserved 3 Bits
//#define		gce_Disposal	0x1c	// Disposal Method 3 Bits
//#define		gce_UserInput	0x02	// User Input Flag 1 Bit
//#define		gce_TransColr	0x01	// Transparent Color Flag 1 Bit

// GIFIMGEXT.giBits
//#define		gie_ColrLoc		0x80	// Local Colour Table
//#define		gie_Interlace	0x40	// Interlaced Scan lines
//#define		gie_SortFlag	0x20	// Sorted Color Table3
//#define		gie_Reserved	0x18	// 2 reserved bits
//#define		gie_ColrSize	0x07	// Colr Table Size ((n+1)^2)

//	WORD	wMaxCnt;	// Maximum number of images (in GIF)
//	WORD	wCurCnt;	// Current image 1 to MaxCnt
//	DWORD	dwMilCnt;	// Count to next movement/action
//	WORD	wBgnCnt;	// Beginning Anim. GIF ... NOT USED
//	WORD	wGifNum;	// The number of this GIF ...
//	WORD	wGifTot;	// The Total of this GIF set ...
//	DWORD	dwGIFNext;	// ms to NEXT GIF action / or USER!!!

void	Set_GIF_Group( HWND hWnd, LPDIBINFO lpDIBInfo )
{
	HGLOBAL	hgInfo;
	LPGIFHDREXT	lpGHE;
	LPGIFIMGEXT	lpGIE;
	WORD		max;
	if( hgInfo = lpDIBInfo->hgGIFSize )
	{
		if( lpGHE = (LPGIFHDREXT)DVGlobalLock( hgInfo ) )
		{
			if( max = lpGHE->gheImgCount )
			{
				lpGIE = &lpGHE->gheGIE[0];	// Point to FIRST Image
//	WORD	gceDelay;	// 1/100 secs to wait
				lpDIBInfo->dwGIFNext = lpGIE->gceDelay * iGifDelay;
				lpDIBInfo->wMaxCnt = max;	// Maximum number of images (in GIF)
				lpDIBInfo->wCurCnt = 0;	// Current image 0 to < MaxCnt
				lpDIBInfo->dwMilCnt = 0;	// Count to next movement/action
				lpDIBInfo->wGifTot = max;	// The Total of this GIF set ...
				AddToAnims( hWnd );
			}
			DVGlobalUnlock( hgInfo );
		}
	}
}

#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF


DWORD	DWSIZE( DWORD siz )
{
	DWORD	nsiz;
	if( siz % 4 )
		nsiz = ((siz / 4) + 1) * 4;
	else
		nsiz = siz;
	return nsiz;
}

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

HANDLE	LoadAnimFile( HWND hWnd, LPDIBINFO lpDIBInfo )
{
	HANDLE		hFile;
	HANDLE		hBig;
	OFSTRUCT    ofs;	/* Open File Name Structure ... */
	LPSTR		lpBig;
	DWORD		dwfs, dwrd;

	hBig = 0;
	lpBig = 0;
	if( ( hFile = 
			DVOpenFile( &lpDIBInfo->di_szDibFile[0], &ofs, OF_READ ) ) &&
		( hFile != (HANDLE)-1 ) )
	{
// FIX980501	( hBig = DVGlobalAlloc( GHND, dwDIBWidth ) ) &&
		if( ( dwfs = DVFileSize( hFile ) ) &&
			( hBig = DVGAlloc( &lpDIBInfo->di_szDibFile[0], GHND, DWSIZE(dwfs) ) ) &&
			( lpBig = DVGlobalLock( hBig ) ) )
		{
			dwrd = DVRead( hFile, lpBig, dwfs );
			if( dwrd != dwfs )
			{
Got_Err:
				DVGlobalUnlock( hBig );
				DVGlobalFree( hBig );
				lpBig = 0;
				hBig = 0;
			}
			if( hBig && lpBig )
			{
//				if( dfFlag & df_GIFANIM )
				if( lpDIBInfo->dwDIBFlag & df_GIFANIM ) //  = dfFlag;
				{
					if( ((lpDIBInfo->wMaxCnt = (WORD)
							Gif_Count( lpBig, dwfs )) < 2 ) ||
						AddToAnims( hWnd ) )
					{
						goto Got_Err;
					}
					lpDIBInfo->Options.bIsAGIF  = TRUE;	// Mark as ANIM GIF
					lpDIBInfo->Options.bAnimate = TRUE;
				}
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF // #ifdef	WJPEG4		//else if( dfFlag & df_GIFGRPC )
				else if( lpDIBInfo->dwDIBFlag & df_GIFGRPC )
				{
					Set_GIF_Group( hWnd, lpDIBInfo );
				}
#endif	// WJPEG4
				DVGlobalUnlock( hBig );
				lpBig = 0;
			}
		}
		DVlclose( hFile );
	}
	return hBig;
}

#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF


void	chkcreate( void )
{
	int	i;
	i = 0;
}

#define  W32FD    WIN32_FIND_DATA

typedef  struct   tagLOADED {
   LE    sList;   // llist header
   HWND  hWnd;    // handle of window
   DWORD dwWid;   // width of DIB
   DWORD dwHt;    // height of DIB
   TCHAR szFile[264];   // file name
   W32FD sFD;     // find data
}LOADED, * PLOADED;

//---------------------------------------------------------------------
//
// Function:   ChildWndCreate
//
// Purpose:    Called by ChildWndProc() on WM_CREATE.  Does initial
//             setup of MDI child winodw.  
//
//             The lpDIBCreateInfo contains a handle to the DIB to
//             be displayed in this window.  Get information on this
//             DIB, create an INFOSTRUCT, and store the handle to
//             this INFOSTRUCT in this window's window words.  This
//             information is then used extensively on many messages
//             handled by ChildWndProc().
//
//             An OPTIONSINFO structure is in the INFOSTRUCT.  All
//             the options are set by the options dialog (in OPTIONS.C).
//             During creation of the DIB window, options are set to
//             default values.
//
//             Also, on creation, set the focus to this window.
//             And, incremente the # of windows open global variable.
//
// Parms:      hWnd            == Handle to window being created.
//             lpDIBCreateInfo == Pointer to DIBCREATEINFO structure
//                                passed in during WM_MDICREATE message
//                                (by FRAME.C).
//
// History:   Date      Reason
//
//            10/15/91  Cut code out from WM_CREATE case.
//                      Also cleaned up code, and got rid
//                      of the hDIBCreateInfo handle.
//            10/27/91  Added bPrinterBand to options.
//                      Use #define's for options.
//            10/28/91  Added bUse31PrintAPIs
//            23 Apr96  Added Window and Client RECT (for Restore)
//			  13 Nov 96 WISH there was a WAY to NOT create the
//						window IF CreatPalette or DIBToBitmap FAIL!!!
//	April 97 Add a long return to ABORT window creation if it fails
//
// Dec 2000 - REMOVE bPrinterBand and bUse31PrintAPIs - grm
//
//---------------------------------------------------------------------
long ChildWndCreate( HWND hWnd, LPDIBCREATEINFO lpDIBCreateInfo ) // 	case WM_CREATE:
{
	long		   lRet = 0;
	HANDLE      hDIB = NULL;
	LPSTR       lpDIB;
	HANDLE      hDIBInfo;
	PDI         lpDIBInfo;
	DWORD       dwDIBHeight, dwDIBWidth;
	DWORD        wBPP, wCompression;
	DWORD        dfFlag;
	HWND        hPar;
	RECT        rp;
	WORD        wOff;
	WORD        wNums[2];
	HGLOBAL		hgInfo;
	HGLOBAL		hDIBx;
   LPTSTR      lpt;

	hDIB = lpDIBCreateInfo->c_hDIB;
	strcpy( szFName, lpDIBCreateInfo->c_szFileName );
	dfFlag = lpDIBCreateInfo->c_dwCDIBFlag;
	hgInfo = lpDIBCreateInfo->c_ghInfo;

	hDIBInfo = NULL;
	hDIBx    = 0;

	chkcreate();
   lpDIB = 0;
   if( hDIB )
      lpDIB = DVGlobalLock( hDIB ); // LOCK DIB HANDLE

	// Get some information about the DIB.  Some of the info
	//  is obtained from within the header (be it a BITMAPINFOHEADER
	//  or a BITMAPCOREHEADER).
	if( lpDIB ) // LOCKED DIB HANDLE
	{
		dwDIBHeight = DIBHeight(lpDIB);
		dwDIBWidth  = DIBWidth (lpDIB);
      wBPP        = DIBBitCount(lpDIB);
//		if( IS_WIN30_DIB( lpDIB ) )
//		{
			wCompression = (DWORD) ((LPBITMAPINFOHEADER) lpDIB)->biCompression;
//			wBPP = ((LPBITMAPINFOHEADER) lpDIB)->biBitCount;
//		}
//		else
//		{
//			wCompression = BI_PM;
//			wBPP = ((LPBITMAPCOREHEADER) lpDIB)->bcBitCount;
//		}
		DVGlobalUnlock( hDIB );    // UNLOCK DIB HANDLE
	}
	else if( hgInfo )
	{
		LPGIFHDREXT	lpghe;
		if( lpghe = (LPGIFHDREXT)DVGlobalLock( hgInfo ) )
		{
			dwDIBHeight = lpghe->gheHeight;
			dwDIBWidth  = lpghe->gheWidth;
			wCompression = 0;
			wBPP = 8;
			DVGlobalUnlock( hgInfo );
		}
		else
		{
			DVGlobalFree( hgInfo );
			hgInfo = 0;
		}
	}
	else
	{
		DIBError( ERR_PARAMS );
		lRet = (long)-1;
		return lRet;
	}

	// Allocate room for the DIBINFO structure and fill it in.
   // *******************************************************
	//	hDIBInfo = DVGlobalAlloc( GHND, sizeof (DIBINFO) );
	if( hDIB || hgInfo )
		hDIBInfo = DVGAlloc( "sDIBINFO", GHND, sizeof (DIBINFO) );

	if(hDIBInfo)
	{
		lpDIBInfo = (LPDIBINFO) DVGlobalLock (hDIBInfo); 
		if(lpDIBInfo)
		{
         ZeroMemory( lpDIBInfo, sizeof(DIBINFO) );    // start with ALL ZERO
			lpDIBInfo->di_size       = sizeof( DIBINFO );
         memcpy( &lpDIBInfo->di_sCI, lpDIBCreateInfo, sizeof(DIBCREATEINFO) );   // create info
         if( lpDIBCreateInfo->c_pRDib )  // pointer to Read DIB info
            memcpy( &lpDIBInfo->di_sRD, lpDIBCreateInfo->c_pRDib, sizeof(RDIB) );  // read DIB info
         lpDIBInfo->di_dwNumber   = gnDIBsOpen + 1;  // set just a NUMBER
			lpDIBInfo->di_hwnd       = hWnd;
			lpDIBInfo->hDIB          = hDIB;
			lpDIBInfo->wDIBType      = wCompression;

			lpDIBInfo->di_dwDIBBits  = wBPP;
			lpDIBInfo->di_dwDIBWidth = dwDIBWidth;
			lpDIBInfo->di_dwDIBHeight= dwDIBHeight;
         lpDIBInfo->di_dwDIBSize  = 
				(DWORD)( (WIDTHBYTES( dwDIBWidth * wBPP )) * dwDIBHeight );

			//lpDIBInfo->rcClip.left   = 0;
			//lpDIBInfo->rcClip.right  = 0;
			//lpDIBInfo->rcClip.top    = 0;
			//lpDIBInfo->rcClip.bottom = 0;

			// Establish the CHILD DEFAULT from the GLOBAL structure
         // ===========================================
			lpDIBInfo->Options.bStretch2W      = gfStretch; // stretch to window
         lpDIBInfo->Options.bAspect         = gbKeepAspect; // set from GLOBAL

//#ifdef  WIN32
			lpDIBInfo->Options.bCenter         = gfPrtCenter;
//#else // !#ifdef  WIN32
#ifndef  WIN32
			lpDIBInfo->Options.bPrinterBand    = gfPrinterBand;
			lpDIBInfo->Options.bUse31PrintAPIs = gfUse31PrintAPIs;
#endif   // !WIN32
			lpDIBInfo->Options.wDispOption     = gwDispOption;
			lpDIBInfo->Options.wPrintOption    = gwPrintOption;
			lpDIBInfo->Options.wXScale         = gwXScale;
			lpDIBInfo->Options.wYScale         = gwYScale;
         // set the DEFAULT ms timer for animations
			lpDIBInfo->Options.dwMilSecs       = gdwMilSecs;

			wOff = MoveFN( &lpDIBInfo->di_szDibFile[0], &szFName[0] );

			// If EITHER of these FAIL, there SHOULD be a WAY to
			// NOT continue with this WINDOW
			//lpDIBInfo->hPal          = 0;
			//lpDIBInfo->hBitmap       = 0;
			lpDIBInfo->dwDIBFlag     = dfFlag;
			wNums[0] = 0;
			wNums[1] = 0;
			if( ( wOff               ) &&
				 ( dfFlag & df_GIFGRP ) )
			{
				if( !(dfFlag & df_GIFGRPC ) )
					dv_wscanf( &szFName[wOff], "%d%d", ( PINT8 )&wNums[0] );
			}
			lpDIBInfo->wGifNum          = wNums[0];
			lpDIBInfo->wGifTot          = wNums[1];
			//lpDIBInfo->wMaxCnt          = 0;
			//lpDIBInfo->hgGIFSize        = 0;	// If a MULTIPLE image GIF, keep INFO
			lpDIBInfo->hgGIFSize        = hgInfo;
			lpDIBInfo->wCurCnt          = 1;
			//lpDIBInfo->hBigFile         = 0;
			//lpDIBInfo->Options.bIsAGIF  = FALSE;	// Mark as NOT ANIM GIF
			//lpDIBInfo->Options.bAnimate = FALSE;
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
			if( dfFlag & ( df_GIFANIM | df_GIFGRPC ) )
			{
				lpDIBInfo->hBigFile = 
					LoadAnimFile( hWnd, lpDIBInfo );
			}
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

			dwDIBWidth  = lpDIBInfo->di_dwDIBWidth;
			dwDIBHeight = lpDIBInfo->di_dwDIBHeight;

         lpt = &lpDIBInfo->di_szTitle[0];

			sprintf( lpt,
            pRDibTit,
            (LPSTR) szFName, dwDIBWidth, dwDIBHeight, wBPP );
			SetWindowText( hWnd, lpt );
			//if( VERBAL9 )
			{
				sprtf( "Child: Title[%s] Hand=%x"MEOR,
					lpt,
					hWnd );
			}

			// FINALLY, we try to create the BITMAP,
			// but this is ONLY really required for DISP_USE_DDB!!!
			if( hDIB )
			{
				lpDIBInfo->hPal     = CreateDIBPalette (hDIB);
				lpDIBInfo->hBitmap  = DIBToBitmap( hDIB, lpDIBInfo->hPal, lpDIBInfo );
            if(lpDIBInfo->hBitmap)
               sprtf( "Done BITMAP for %d x %d DIB = %#x"MEOR, dwDIBWidth, dwDIBHeight, lpDIBInfo->hBitmap );
            else
               sprtf( "NULL BITMAP for %d x %d DIB!"MEOR, dwDIBWidth, dwDIBHeight );

			}
			else if( hgInfo )
			{
				LPGIFHDREXT lpGHE;
				if( lpGHE = (LPGIFHDREXT)DVGlobalLock( hgInfo ) )
				{
					if( hDIB = lpGHE->hDIB )
					{
						lpGHE->hDIB        = 0;
						lpDIBInfo->hDIB    = hDIB;
						lpDIBInfo->hPal    = CreateDIBPalette (hDIB);
						lpDIBInfo->hBitmap = DIBToBitmap( hDIB, lpDIBInfo->hPal, lpDIBInfo );
					}
					DVGlobalUnlock( hgInfo );
				}
			}

#ifndef	CHGADDTO
			AddToFileList( &lpDIBInfo->di_szDibFile[0] );
#endif	// !CHGADDTO

			DVGlobalUnlock( hDIBInfo );
		}
		else
		{
			DVGlobalFree( hDIBInfo );
			DIBError( ERR_LOCK );
			hDIBInfo = 0;
			lRet = (long)-1;
			return lRet;
		}
	}
	else
	{
		DIBError( ERR_MEMORY );
		lRet = (long)-1;
		return lRet;
	}

#ifdef	ADDCOLRF
	//if( hDIBx = DVGlobalAlloc( GHND, sizeof( DIBX ) ) )
	if( hDIBx = DVGAlloc( "sDIBX", GHND, sizeof( DIBX ) ) )
	{
		LPDIBX	lpDIBx;
		if( lpDIBx = (LPDIBX)DVGlobalLock( hDIBx ) )
		{
			if( !gDIBx.dx_Valid )
			{
				GDefFace();
				GDefShad();
				GDefHiLite();
				GDefText();
				gDIBx.dx_Valid = TRUE;
				fChgGBack = TRUE;
				fChgGShad = TRUE;
				fChgGHigh = TRUE;
				fChgGText = TRUE;
			}
			dv_fmemcpy( lpDIBx, &gDIBx, sizeof( DIBX ) );
			DVGlobalUnlock( hDIBx );
		}
		else
		{
			DVGlobalFree( hDIBx );
			hDIBx = 0;
		}
	}

#endif	// ADDCOLRF

	// Set the window word for the handle to the DIBINFO structure.
	SetWindowExtra( hWnd, WW_DIB_HINFO, hDIBInfo );
	SetWindowExtra( hWnd, WW_DIB_EX2,   hDIBx    );

    // On initial creation, focus isn't set to us, so set it
    // explicitly.
	SetFocus( hWnd );

	if( hDIBInfo )
	{
		if( lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo ) )
		{
			GetWindowRect( hWnd, &lpDIBInfo->rWindow );	/* Screen co-ordinates */
			GetClientRect( hWnd, &lpDIBInfo->rClient );	/* Get cx & cy w/h */
			hPar = GetParent( hWnd ); 
			if(hPar)
			{
				GetWindowRect( hPar, &rp );	/* Get the Parent co-ordinates */
//				rp.left += GetSystemMetrics( SM_CXBORDER );
//				rp.top += (GetSystemMetrics( SM_CYCAPTION ) +
//							GetSystemMetrics( SM_CYMENU ) -
//							(2 * GetSystemMetrics( SM_CYBORDER )));
				if( ( rp.left <= lpDIBInfo->rWindow.left ) &&
					 ( rp.top  <= lpDIBInfo->rWindow.top  ) )
				{
					lpDIBInfo->rClient.left = lpDIBInfo->rWindow.left -
							rp.left;
					lpDIBInfo->rClient.top  = lpDIBInfo->rWindow.top -
							rp.top;
				}
			}

         // CountColorProc( (LPVOID)hDIBInfo ); // thread data
         // Create the COUNT THREAD
         lpDIBInfo->di_hCntThread =    // Handle to THREAD
            CreateThread(
               NULL, // SD
               0,    // initial stack size
               CountColorProc,   // LPTHREAD_START_ROUTINE - thread function
               (LPVOID)hDIBInfo, // thread argument
               0,    // creation option
               &lpDIBInfo->di_dwCntThdId );   // COUNT thread identifier

         if( lpDIBInfo->di_hCntThread )
         {
            // try to ensure the USER INTERFACE works!!!
            // *****************************************
            SetThreadPriority( lpDIBInfo->di_hCntThread, // handle to the thread
               THREAD_PRIORITY_LOWEST );  // thread priority level
            // *****************************************
         }
         else
         {
            lpDIBInfo->di_dwCntThdId = 0; // ensure this is ZERO
            CountColorProc( (LPVOID)hDIBInfo ); // thread data
         }

			DVGlobalUnlock( hDIBInfo );
		}
		else
		{
			DIBError( ERR_LOCK );
		}
	}

	// Increment the # of DIBs open variable and insure that the
	//  "Window" pull down menu is not grayed.
	gnDIBsOpen++;
	EnableWindowAndOptionsMenus( TRUE );
//   CountColorProc( (LPVOID)hDIBInfo ); // thread data

	return( lRet );

}	// End - long ChildWndCreate(HWND, LPDIBCREATEINFO)

void	PutBox( HDC hDC, LPRECT prc, int ht )
{
	PatBlt( hDC,
		prc->left,
		prc->top,
		prc->right - prc->left,
		ht,
		DSTINVERT );
	PatBlt( hDC,
		prc->left,
		prc->bottom,
		ht,
		-(prc->bottom - prc->top),
		DSTINVERT );
	PatBlt( hDC,
		prc->right - ht,
		prc->top,
		ht,
		prc->bottom - prc->top,
		DSTINVERT );
	PatBlt( hDC,
		prc->right,
		prc->bottom - ht,
		-(prc->right - prc->left),
		ht,
		DSTINVERT );
}

#ifdef	WIN32
// Since this is the product signature - RETIRE this servie soonest
#ifndef	Dv32
#ifndef	_RedGMUtils_h

// Taken over by GMUtil.c - cut out when desired ...
DWORD	GetTextExtent( HDC hDC, LPSTR szStr, int nLen )
{
	DWORD	dwExt;
	SIZE	tSize;

//BOOL GetTextExtentPoint(
//    HDC hdc,  // handle of device context
//    LPCTSTR lpString, // address of text string
//    int cbString,     // number of characters in string
//    LPSIZE lpSize     // address of structure for string size
////);
	dwExt = (((WORD)1 << 16) | ((WORD)1));
	if( GetTextExtentPoint( hDC, szStr, nLen, &tSize ) )
	{
		dwExt =  (((WORD)tSize.cy << 16) | ((WORD)tSize.cx) );
	}
	return dwExt;
}

#endif	// !_RedGMUtils_h
#endif	// !Dv32
#endif	// WIN32

void	PutCenteredText1( HWND hWnd, HDC hDC, LPSTR lpText )
{
	RECT	rcC;
	DWORD   dwExt;
	int     x, y, nLen, dx, dy;
	HDC     hDCBits;
	HBITMAP hBitmap;

	if( ( hWnd ) &&
		( hDC ) &&
		( lpText ) &&
		( nLen = lstrlen( lpText ) ) &&
		( GetClientRect( hWnd, &rcC ) ) &&
		( hDCBits = CreateCompatibleDC( hDC ) ) )
	{
		// Looks ok
		dwExt = GetTextExtent( hDC, lpText, nLen );
		dx      = LOWORD (dwExt);	// Get LENGTH
		dy      = HIWORD (dwExt);	// Get HEIGHT

		// Draw rectangular clip region
		if( (dx+2) < rcC.right )
		{
			rcC.left   = ((rcC.right - dx) - 1) / 2;
			rcC.right  = rcC.left + dx + 2;
		}
		if( (dy+2) < rcC.bottom )
		{
			rcC.top    = ((rcC.bottom - dy) - 1) / 2;
			rcC.bottom = rcC.top + dy + 2;
		}

		PutBox( hDC, &rcC, 1 );

		// and center text in the rectangle
		x = rcC.left + 1;
		y = rcC.top  + 1;

		// Output the text to the DC 
		SetTextColor( hDCBits, RGB(255, 255, 255) );
		SetBkColor(   hDCBits, RGB(  0,   0,   0) );
		if( hBitmap = CreateBitmap( dx, dy, 1, 1, NULL ) )
		{
			hBitmap = SelectObject( hDCBits, hBitmap );
			ExtTextOut( hDCBits, 0, 0, 0, NULL, lpText, nLen, NULL );
			BitBlt( hDC, x, y, dx, dy, hDCBits, 0, 0, SRCINVERT );
			hBitmap = SelectObject( hDCBits, hBitmap );
			DeleteObject( hBitmap );
		}
		DeleteDC( hDCBits );
	}
}

void ShowAbort( HWND hWnd, HDC hDC, UINT typ )
{
	char    szStr[80];

	switch( typ )
	{
	case SA_NoDIB:
		lstrcpy( szStr,
			"ERROR: No handle DIBINFO!" );
		break;
	case SA_NoPtr:
		lstrcpy( szStr,
			"ERROR: No DIBINFO Pointer!" );
		break;
	case SA_NohDIB:
		lstrcpy( szStr,
			"ERROR: No handle DIB!" );
		break;
	case SA_NoBmp:
		lstrcpy( szStr,
			"ERROR: No BITMAP handle!" );
		break;
	case SA_NohmDC:
		lstrcpy( szStr,
			"ERROR: Compatible memory DC failed!" );
		break;
	case SA_WtThrd:
		lstrcpy( szStr,
			"MOMENT: Waiting for thread to render bitmap..." );
		break;
	default:
		wsprintf( szStr,
			"ERROR: ChildWndPaint failed! (%d)",
			typ );
		break;
	}

	// NOTE: This is for ONE LINE of TEXT
	PutCenteredText1( hWnd, hDC, szStr );

}	// End - void ShowAbort( HWND hWnd, HDC hDC, UINT typ )

void ShowPErr( HWND hWnd, HDC hDC, DWORD typ )
{
	char    szStr[80];
	wsprintf(szStr,
		"ERROR: ChildWndPaint failed! Error = %d (%#x)",
		typ, typ );

	// NOTE: This is for ONE LINE of TEXT
	PutCenteredText1( hWnd, hDC, szStr );

}	// End - void ShowPErr( HWND hWnd, HDC hDC, DWORD typ )

//---------------------------------------------------------------------
//
// Function:   ChildWndPaint
//
// Purpose:    Called by ChildWndProc() on WM_PAINT.  Does all paints
//             for this MDI child window.
//
//             Reads position of scroll bars to find out what part
//             of the DIB to display.
//
//             Checks the stretching flag in the DIBINFO structure for
//             this window to see if we are stretching to the window
//             (if we're iconic, we always stretch to a tiny bitmap).
//
//             Selects/Realizes the palette as a background palette.
//             ChildWndQueryNewPalette realized it already as the
//             foreground palette if this window is the active MDI
//             child.
//
//             Calls the appropriate paint routine depending on the
//             option set for this window (i.e. DIB, DDB, or SetDIBits).
//
//             Draws the selection rectangle for copying to the
//             clipboard.
//
// Parms:      hWnd == Handle to window being painted.
//
// History:   Date      Reason
//
//            10/15/91  Cut code out from WM_PAINT case.
//            12/03/91  Always force SelectPalette() to a
//                        background palette.  If it was
//                        the foreground palette, it would
//                        have been realized during
//                        WM_QUERYNEWPALETTE.
//             
//---------------------------------------------------------------------
//#define		TRYCOMP
#undef		TRYCOMP
void	chkp1( void )
{
	int i;
	i = 0;
}


//Value	Meaning
//TA_BASELINE	The reference point is on the base line of the text.
//TA_BOTTOM	The reference point is on the bottom edge of the bounding rectangle.
//TA_TOP	The reference point is on the top edge of the bounding rectangle.
//TA_CENTER	The reference point is aligned horizontally with the center of the bounding rectangle.
//TA_LEFT	The reference point is on the left edge of the bounding rectangle.
//TA_RIGHT	The reference point is on the right edge of the bounding rectangle.
//TA_RTLREADING	Windows 95 only: The text is laid out in right to left reading order, as opposed to the default left to right order. This only applies when the font selected into the device context is either Hebrew or Arabic.
//TA_NOUPDATECP	The current position is not updated after each text output call.
//TA_UPDATECP	The current position is updated after each text output call.
BOOL	ChkTextAlign( HDC hDC )
{
	UINT	ta;
	BOOL	flg = FALSE;

	ta = GetTextAlign( hDC );
	if( ta & (TA_BASELINE|TA_BOTTOM) )
	{
		ta &= ~(TA_BASELINE|TA_BOTTOM);
		ta |= TA_TOP;
		flg = TRUE;
	}
	if( ta & (TA_CENTER|TA_RIGHT) )
	{
		ta &= ~(TA_CENTER|TA_RIGHT);
		ta |= TA_LEFT;
		flg = TRUE;
	}
	if( ta & TA_UPDATECP )
	{
		ta &= ~(TA_UPDATECP);
		ta |= TA_NOUPDATECP;
		flg = TRUE;
	}
	if( flg )
	{
		SetTextAlign( hDC, ta );
	}
	return flg;
}

void chktext( WORD w )
{
	int	i;
	if( w == 5 )
		i = 0;
	else if( w == 6 )
		i = 1;
	else if( w == 7 )
		i = 2;
	else
		i = 3;
}

int		iTPI = 1440;
//rc.right = rc.left + GetHPixels( lpGIE ); //(lpGIE->giGI.giWidth & 0xffff);
//rc.bottom = rc.top + GetVPixels( lpGIE ); //(lpGIE->giGI.giHeight & 0xffff);
int	GetHPixels( LPGIFIMGEXT lpGIE )
{
	int		i, j, k;
	i = (lpGIE->giGI.giWidth & 0xffff);		// Get Width
	if( giHPPI )
	{
		j = ((i * giHPPI) / iTPI);
		if( j == 0 )
			j++;
		k = j * i;
	}
	else
	{
		k = i;
	}
	return( k );
}

int	GetVPixels( LPGIFIMGEXT lpGIE )
{
	int		i, j, k;
	i = (lpGIE->giGI.giHeight & 0xffff);		// Get Height
	if( giVPPI )
	{
		j = ((i * giVPPI) / iTPI);
		if( j == 0 )
			j++;
		k = j * i;
	}
	else
	{
		k = i;
	}
	return( k );
}

void	PutShtTxt( HDC hDC, LPRECT lpDest, LPSTR lps )
{
	int		i;
	DWORD	dwExt;
	WORD	dx, dy;
	RECT	rct;
	if( hDC &&
		lps &&
		( i = lstrlen( lps )) )
	{
		dwExt = GetTextExtent( hDC, lps, i );
		dx      = LOWORD (dwExt);	// Get LENGTH
		dy      = HIWORD (dwExt);	// Get HEIGHT
		if( ( dx < (WORD)lpDest->right ) &&
			( dy < (WORD)lpDest->bottom ) )
		{
			rct.left = lpDest->left + ((lpDest->right -  dx) / 2);
			rct.top  = lpDest->top  + ((lpDest->bottom - dy) / 2);
			TextOut( hDC,	// handle of device context
				rct.left,	// x-coordinate of starting position
				rct.top,	// y-coordinate of starting position 
				lps,		// address of string
				i );		// number of characters in string 
		}
	}
}

#define	ShowDC( a, b ) \
if( iycnt < 2 )\
{\
	LPSTR	lps;\
	lps = b;\
	StretchBlt( hDC,\
		destRC.left,\
		destRC.top,\
		destRC.right,\
		destRC.bottom,\
		a,\
		0,0,\
		bmp.bmWidth,\
		bmp.bmHeight,\
		SRCCOPY );\
		PutShtTxt( hDC, &destRC, lps );\
		destRC.left += destRC.right;\
		ixcnt++;\
		if( ixcnt >= 4 )\
		{\
			ixcnt = 0;\
			destRC.left = lpDest->left;\
			destRC.top += destRC.bottom;\
			iycnt++;\
		}\
}

BOOL	PatternBlt( HDC hDC, HBITMAP srcBmp,
				   LPRECT	lpDest,
				   COLORREF	dwTransColor )
{
	BOOL	rFlg = FALSE;
// Const PIXEL = 3
//        destScale = dest.ScaleMode 'Store ScaleMode to restore later
//        dest.ScaleMode = PIXEL 'Set ScaleMode to pixels for Windows GDI
//        'Retrieve bitmap to get width (bmp.bmWidth) & height (bmp.bmHeight)
//typedef struct tagBITMAP {  // bm  
//   LONG   bmType; 
//   LONG   bmWidth; 
//   LONG   bmHeight; 
//   LONG   bmWidthBytes; 
//   WORD   bmPlanes; 
//   WORD   bmBitsPixel; 
//   LPVOID bmBits; 
//} BITMAP;
	int destX;
	int destY;
	int		i;
	BITMAP	bmp;
	HDC	srcDC, saveDC, maskDC, invDC, resultDC;
	HBITMAP	hMaskBmp, hInvBmp, hResultBmp,hPrevBmp, hSaveBmp;
	HGDIOBJ	hSrcPrevBmp, hSavePrevBmp;
	HGDIOBJ	hMaskPrevBmp, hInvPrevBmp, hDestPrevBmp;
	COLORREF	OrigColor, dwOldTransColor;
	int		iOrigMode;
	RECT	destRC;
	int		ixcnt, iycnt;

	// Eststablish first paint rectangle
	destRC.left = lpDest->left;
	destRC.top  = lpDest->top;
	destRC.right = lpDest->right;
	destRC.bottom = lpDest->bottom;
	ixcnt = 0;
	iycnt = 0;

	// Clear some tool handles
	srcDC = 0;
	saveDC = 0;
	maskDC = 0;
	invDC = 0;
	resultDC = 0;
	hMaskBmp = 0;
	hInvBmp = 0;
	hResultBmp = 0;
	hSaveBmp = 0;

	destX = lpDest->left;
	destY = lpDest->top;

	// Success = GetObj(srcBmp, Len(bmp), bmp)
	i = GetObject( srcBmp,	// handle to graphics object of interest
		sizeof( BITMAP ),	// size of buffer for object information 
		&bmp );				// pointer to buffer for object information
	if( i )
	{
		int	iSuccess;

		// Create NEEDED Objects
		srcDC = CreateCompatibleDC(hDC);	//' stage
		saveDC = CreateCompatibleDC(hDC);	//' stage
		maskDC = CreateCompatibleDC(hDC);	//' stage
		invDC = CreateCompatibleDC(hDC);	//' stage
		resultDC = CreateCompatibleDC(hDC);	//' stage

//        'Create monochrome bitmaps for the mask-related bitmaps:
		hMaskBmp = CreateBitmap( bmp.bmWidth, bmp.bmHeight, 1, 1, 0);
		hInvBmp = CreateBitmap(bmp.bmWidth, bmp.bmHeight, 1, 1, 0);
//        'Create color bitmaps for final result & stored copy of source
		hResultBmp = CreateCompatibleBitmap(hDC, bmp.bmWidth,
			bmp.bmHeight );
		hSaveBmp = CreateCompatibleBitmap(hDC, bmp.bmWidth,
			bmp.bmHeight );

		if( srcDC && saveDC && maskDC && invDC && resultDC &&
			hMaskBmp && hInvBmp && hResultBmp &&
			hSaveBmp )
		{
			hSrcPrevBmp = SelectObject( srcDC, srcBmp );	//'Select bitmap in DC
			hSavePrevBmp = SelectObject(saveDC, hSaveBmp);	//'Select bitmap in DC
			hMaskPrevBmp = SelectObject(maskDC, hMaskBmp);	//'Select bitmap in DC
			hInvPrevBmp = SelectObject(invDC, hInvBmp);	//'Select bitmap in DC
			hDestPrevBmp = SelectObject(resultDC, hResultBmp); //'Select bitmap

// 'Copy background bitmap to result
// where the final transparent bitmap is created
			iSuccess = BitBlt(resultDC, 0, 0,
				bmp.bmWidth, bmp.bmHeight,
				hDC,
				destX, destY,
				SRCCOPY );

			ShowDC( resultDC, "CopyBG" );
			// Make COPY of current BITMAP
			// from srcDC/srcBmp to SaveDC/hSaveBmp
			iSuccess = BitBlt(saveDC, 0, 0,
				bmp.bmWidth, bmp.bmHeight,
				srcDC,
				0, 0, SRCCOPY);	//'Make backup of source bitmap to restore later

			ShowDC( saveDC, "CopyBMP" );

//        'Create mask: set background color of source to transparent color.
			// Set BACKGROUND to dwTransColor
			// ==============================
			iOrigMode = GetBkMode( srcDC );
			OrigColor = SetBkColor(srcDC, dwTransColor);

			iSuccess = BitBlt(maskDC, 0, 0,
				bmp.bmWidth, bmp.bmHeight,
				srcDC,
				0, 0, SRCCOPY);

			dwOldTransColor = SetBkColor(srcDC, OrigColor);

			ShowDC( maskDC, "MaskTC" );

// 'Create inverse of mask to AND w/ source & combine w/ background.
			iSuccess = BitBlt(invDC, 0, 0,
				bmp.bmWidth, bmp.bmHeight,
				maskDC,
				0, 0, NOTSRCCOPY);

			ShowDC( invDC, "Invert" );

// 'AND mask bitmap w/ result DC to punch hole in the background by
//        'painting black area for non-transparent portion of source bitmap.
			iSuccess = BitBlt(resultDC, 0, 0,
				bmp.bmWidth, bmp.bmHeight,
				maskDC, 0, 0, SRCAND);

			ShowDC( resultDC, "AndRes" );

// 'AND inverse mask w/ source bitmap to turn off bits associated
//        'with transparent area of source bitmap by making it black.
			iSuccess = BitBlt(srcDC, 0, 0,
				bmp.bmWidth, bmp.bmHeight,
				invDC,
				0, 0, SRCAND);

			ShowDC( srcDC, "AndInv" );

// 'XOR result w/ source bitmap to make background show through.
			iSuccess = BitBlt(resultDC, 0, 0,
				bmp.bmWidth, bmp.bmHeight,
				srcDC, 0, 0,
				SRCPAINT);

			ShowDC( resultDC, "Result" );

			// OUPUT Result to HDC
			//iSuccess = BitBlt( hDC, destX, destY,
			//	bmp.bmWidth, bmp.bmHeight,
			//	resultDC, 0, 0,
			//	SRCCOPY);	//'Display transparent bitmap on backgrnd

			iSuccess = BitBlt(srcDC, 0, 0,
				bmp.bmWidth, bmp.bmHeight,
				saveDC,
				0, 0, SRCCOPY);	//'Restore backup of bitmap.

			//ShowDC( srcDC, "CopySRC" );

			hPrevBmp = SelectObject(srcDC, hSrcPrevBmp);	//'Select orig object
			hPrevBmp = SelectObject(saveDC, hSavePrevBmp); //'Select orig object
			hPrevBmp = SelectObject(resultDC, hDestPrevBmp); //'Select orig object
			hPrevBmp = SelectObject(maskDC, hMaskPrevBmp); //'Select orig object
			hPrevBmp = SelectObject(invDC, hInvPrevBmp); //'Select orig object
// dest.ScaleMode = destScale 'Restore ScaleMode of destination.
			rFlg = TRUE;
// End If
		}

		if( hSaveBmp )
			DeleteObject(hSaveBmp);	//'Deallocate system resources.
		if( hMaskBmp )
			DeleteObject(hMaskBmp);	//'Deallocate system resources.
		if( hInvBmp )
			DeleteObject(hInvBmp);	//'Deallocate system resources.
		if( hResultBmp )
			DeleteObject(hResultBmp); //'Deallocate system resources.
		if( srcDC )
			DeleteDC(srcDC);	//'Deallocate system resources.
		if( saveDC )
			DeleteDC(saveDC);	//'Deallocate system resources.
		if( invDC )
			DeleteDC(invDC);	//'Deallocate system resources.
		if( maskDC )
			DeleteDC(maskDC);	//'Deallocate system resources.
		if( resultDC )
			DeleteDC(resultDC);	//'Deallocate system resources.

	}	// If we got the srcBmp INFORMATION
	return rFlg;	// TRUE = Successful PAINT done.
//   End Sub
////======================
}	// PatternBlt( ... )

BOOL	PntNeedsBmp( LPDIBINFO lpDIBInfo )
{
	BOOL	bNeedBmp = TRUE;
//	if( ( lpDIBInfo->Options.wDispOption == DISP_USE_SETDIBITS ) ||
//		( lpDIBInfo->Options.wDispOption == DISP_USE_DIBS ) )
	if( lpDIBInfo->Options.wDispOption == DISP_USE_DIBS )
		bNeedBmp = FALSE;
//	}
	return bNeedBmp;
}

void ChildWndPaint( HWND hWnd, WPARAM wParam )  // from WM_PAINT
{

	HDC			hDC, rhDC;
	PAINTSTRUCT	ps;
	int			xScroll, yScroll;
	HPALETTE	hOldPal = NULL;
	RECT		rectC, rectClient, rectDDB;
	BOOL		bStretch, bNeedBmp;
	HANDLE		hDIBInfo;
	LPDIBINFO	lpDIBInfo;
	BITMAP		Bitmap;
	DWORD		dwROP;
	HANDLE		hDIB;
	HBITMAP		hBmp;
#ifdef	WJPEG4
	HGLOBAL		hgInfo;
	LPGIFHDREXT	lpGHE;
	LPGIFIMGEXT	lpGIE;
	WORD		w, max;
	RECT		rc;
	HFONT		hFont, hOldFont;
	UINT		ta;
	BOOL		fta, ffta;
	int			mm;
	LPSTR		lps, lpbmh;
	LPRGBQUAD	lpq;
	LPPTEHDR	lppte;
	int			len;
	COLORREF	fgcolr, bgcolr, trcolr;
	int			BkMode;
	RECT		crect;
	UINT		opts;
	int			xChar, yChar, cy;
	TEXTMETRIC  tm;
	int			ii, jj, kk;
	BOOL		fTrPaint;
#endif	// WJPEG4
#ifdef	TRYCOMP
	HBITMAP		hbm, hbmold;
#endif	
   BOOL bKeepAsp;

	if( rhDC = (HDC)wParam )
		BeginPaint( hWnd, &ps );
	else
		rhDC = BeginPaint( hWnd, &ps );
	hDC = rhDC;	// Establish default

//	if( !GetUpdateRect( hWnd, NULL, FALSE ) )
//		goto PaintEnd;

	//Hourglass (TRUE);
   //chkpaint();    // just a DEBUG stop

#ifdef	WJPEG4
	hgInfo = 0;
	lpGHE = 0;
	lpGIE = 0;
#endif	// WJPEG4

	GetClientRect( hWnd, &rectClient );
	CopyRect( &rectC, &rectClient );
   lpDIBInfo = 0;
	hDIBInfo  = GetWindowExtra(hWnd, WW_DIB_HINFO);
	if( hDIBInfo == 0 )
	{
		ShowAbort( hWnd, hDC, SA_NoDIB );
		goto ABORTPAINT;
	}

	lpDIBInfo = (LPDIBINFO) DVGlobalLock (hDIBInfo);
	if( lpDIBInfo == 0 )
	{
		ShowAbort( hWnd, hDC, SA_NoPtr );
		goto ABORTPAINT;
	}

	if( !(hDIB = lpDIBInfo->hDIB) )
	{
		ShowAbort( hWnd, hDC, SA_NohDIB );
		//DVGlobalUnlock( hDIBInfo );
		goto ABORTPAINT;
	}

	bNeedBmp = PntNeedsBmp( lpDIBInfo );
	hBmp = lpDIBInfo->hBitmap;
	if( !( hBmp   ) &&
		( bNeedBmp ) )
	{
		if( lpDIBInfo->di_hThread )
			ShowAbort( hWnd, hDC, SA_WtThrd );
		else
			ShowAbort( hWnd, hDC, SA_NoBmp );
		//DVGlobalUnlock( hDIBInfo );
		goto ABORTPAINT;
	}


#ifdef	TRYCOMP
	if( (hDC = CreateCompatibleDC( rhDC )) == 0 )
		goto ABORTPAINT;
	if( ( hbm = CreateCompatibleBitmap( rhDC, rectClient.right,
		rectClient.bottom ) ) == 0 )
	{
		DeleteDC( hDC );
		goto ABORTPAINT;
	}
	hbmold = SelectObject( hDC, hbm );
#else // !TRYCOMP

   // this works ok
	hDC = rhDC;

#endif   // TRYCOMP y/n

	xScroll  = GetScrollPos( hWnd, SB_HORZ );
	yScroll  = GetScrollPos( hWnd, SB_VERT );

	bStretch = lpDIBInfo->Options.bStretch2W;
   bKeepAsp = lpDIBInfo->Options.bAspect;

	// When we're iconic, we'll always stretch the DIB
	//  to our icon.  Otherwise, we'll use the stretching
	//  option the user picked.
   if( IsIconic( hWnd ) ) {
		bStretch = TRUE;
      bKeepAsp = TRUE;
   }

	// Set up the scroll bars appropriately.
	if( bStretch )
	{
		ScrollBarsOff( lpDIBInfo, hWnd );
	}
	else
	{
		SetupScrollBars( lpDIBInfo,
			hWnd,
			lpDIBInfo->di_dwDIBWidth,
			lpDIBInfo->di_dwDIBHeight );
	}

	GetObject( hBmp,	// lpDIBInfo->hBitmap - Created from DIB
		sizeof (Bitmap),
		(LPSTR) &Bitmap );
   
	// Set up the necessary rectangles -- i.e. the rectangle
	//  we're rendering into, and the rectangle in the DIB.

#ifdef	USEDIBSZ
	if( bStretch ) {
      // use WHOLE of DIB size = whole image
	   rectDDB.left   = 0;
	   rectDDB.top    = 0;
	   rectDDB.right  = lpDIBInfo->di_dwDIBWidth;
	   rectDDB.bottom = lpDIBInfo->di_dwDIBHeight;
      if( gbAspect && bKeepAsp ) { // *** default = MAINTAIN ASPECT ***
         // fit to current window, maintaining aspect ratio
         int split;
         double dw = rectDDB.right;
         double dh = rectDDB.bottom;
         double ww = RECTWIDTH( &rectClient );
         double wh = RECTHEIGHT( &rectClient );
         double rat = dw / dh;
         // using current window size,
         // calculate new width and height
         int nw = (int)(wh * rat);
         int nh = (int)(ww / rat);
         if( nw <= RECTWIDTH( &rectClient ) ) {
            // set new paint height
            nh = (int)((double)nw / rat);
         } else {
            // recalculate paint width
            nw = (int)((double)nh * rat);
         }
         // center the image within the 'new' values
         if( nw < RECTWIDTH( &rectClient ) ) {
            // center the image
            split = (RECTWIDTH( &rectClient ) - nw) / 2;
            rectClient.left += split;
            rectClient.right -= split;
         }
         if( nh < RECTHEIGHT( &rectClient ) ) {
            split = (RECTHEIGHT( &rectClient ) - nh) / 2;
            rectClient.top += split;
            rectClient.bottom -= split;
         }
      }
   } else {

      // adjust paint of image for any SCROLL action
		rectDDB.left   = xScroll;
		rectDDB.top    = yScroll;
		rectDDB.right  = xScroll + rectClient.right - rectClient.left;
		rectDDB.bottom = yScroll + rectClient.bottom - rectClient.top;

		if( rectDDB.right > (int)lpDIBInfo->di_dwDIBWidth ) {
			int dx;
			dx = lpDIBInfo->di_dwDIBWidth - rectDDB.right;
			rectDDB.right     += dx;
			rectClient.right  += dx;
		}

		if( rectDDB.bottom > (int)lpDIBInfo->di_dwDIBHeight ) {
			int dy;
			dy = lpDIBInfo->di_dwDIBHeight - rectDDB.bottom;
			rectDDB.bottom    += dy;
			rectClient.bottom += dy;
		}
	}

#else	// !USEDIBSZ

//	GetObject( hBmp,	// lpDIBInfo->hBitmap - Created from DIB
//		sizeof (Bitmap),
//		(LPSTR) &Bitmap);
   
	if( bStretch )
	{
		rectDDB.left   = 0;
		rectDDB.top    = 0;
		rectDDB.right  = Bitmap.bmWidth;
		rectDDB.bottom = Bitmap.bmHeight;
		if( ( lpDIBInfo->di_bTooBig ) &&    // too big ie > 2000000
			( lpDIBInfo->Options.wDispOption == DISP_USE_DIBS ) )
		{
			rectDDB.right  = lpDIBInfo->di_dwDIBWidth;
			rectDDB.bottom = lpDIBInfo->di_dwDIBHeight;
		}
	}
	else
	{
		rectDDB.left   = xScroll;
		rectDDB.top    = yScroll;
		rectDDB.right  = xScroll + rectClient.right - rectClient.left;
		rectDDB.bottom = yScroll + rectClient.bottom - rectClient.top;

		if( rectDDB.right > Bitmap.bmWidth )
		{
			int dx;
			dx = Bitmap.bmWidth - rectDDB.right;
			rectDDB.right     += dx;
			rectClient.right  += dx;
		}

		if( rectDDB.bottom > Bitmap.bmHeight )
		{
			int dy;
			dy = Bitmap.bmHeight - rectDDB.bottom;
			rectDDB.bottom    += dy;
			rectClient.bottom += dy;
		}
	}

#endif		// USEDIBSZ

	// Setup the palette.
	if( lpDIBInfo->hPal )
		hOldPal = SelectPalette (hDC, lpDIBInfo->hPal, TRUE);

	RealizePalette (hDC);

	dwROP = SRCCOPY;

#ifdef	WJPEG4
//==========================================
	trcolr = 0;
	fTrPaint = FALSE;
	if( hgInfo = lpDIBInfo->hgGIFSize )
	{
		if( lpGHE = (LPGIFHDREXT)DVGlobalLock( hgInfo ) )
		{
			if( lpGHE->gheImgCount == 1 )
			{
				lpGIE = &lpGHE->gheGIE[0];
				if( lpGIE->gceBits & gce_TransColr ) // Transparent Color Flag 1 Bit
				{
					fTrPaint = TRUE;	// Use Transparent PAINT
					trcolr = (COLORREF)lpGIE->gceColr;
					dwROP = SRCPAINT;
				}
				else
				{
					dwROP = SRCCOPY;
				}
			}
			//DVGlobalUnlock( hgInfo );
		}
	}
//=============================
#endif	// WJPEG4

	// Go do the actual painting.
	switch( lpDIBInfo->Options.wDispOption )
	{

	case DISP_USE_DIBS:
		DIBPaint( lpDIBInfo, hDC, &rectClient, hDIB, &rectDDB,
			lpDIBInfo->hPal, dwROP );
		break;

	case DISP_USE_SETDIBITS:
		SetDIBitsPaint( lpDIBInfo, hDC, &rectClient, hDIB, &rectDDB,
			lpDIBInfo->hPal, dwROP );
		break;

	case DISP_USE_DDBS:
	default:
#ifdef	WJPEG4
		if( fTrPaint && hBmp && hgInfo && lpGHE )
		{
			RECT	destRC;
			COLORREF	trcolr;
			trcolr = lpGIE->gceColr;	// Graphic Control Extension
//extern	BOOL	dvTransparentBlt( HDC hDC, HBITMAP srcBmp,
//					   int destX, int destY,
//					   COLORREF	dwTransColor );
			dvTransparentBlt( hDC, hBmp,
				rectClient.left,	// Left
				rectClient.top,		// Top
				trcolr );			// The COLOUR
			if( ( fAddPat || ( rectClient.bottom > Bitmap.bmHeight ) ) &&
				( destRC.right = Bitmap.bmWidth / 4 ) &&
				( destRC.bottom = Bitmap.bmHeight / 2 ) )
			{
				destRC.left = 0;
				destRC.top = Bitmap.bmHeight;
				PatternBlt( hDC, hBmp,
					&destRC,
					trcolr );
			}
		}
		else
#endif	// WJPEG4
		{
			DDBPaint( lpDIBInfo, hDC, &rectClient,
				lpDIBInfo->hBitmap, &rectDDB,
				lpDIBInfo->hPal, dwROP );
		}
		break;
	}

#ifdef	WJPEG4
	if( hgInfo = lpDIBInfo->hgGIFSize )
	{
		hOldFont = 0;
		ta = 0;
		fta = FALSE;
		ffta = FALSE;
		if( lpGHE = (LPGIFHDREXT)DVGlobalLock( hgInfo ) )
		{
			if( ( max = lpGHE->gheImgCount ) &&
				( max > 1 ) )
			{
				for( w = 0; w < max; w++ )
				{
					lpGIE = &lpGHE->gheGIE[w];
					if( lpGIE->hDIB && lpGIE->hBitmap )
					{
						if( lpGIE->gceBits & gce_TransColr ) // Transparent Color Flag 1 Bit
							dwROP = SRCPAINT;
						else
							dwROP = SRCCOPY;
						GetObject( lpGIE->hBitmap,
							sizeof (Bitmap),
							(LPSTR) &Bitmap);
						rc.left = (lpGIE->giLeft & 0xffff);
						rc.top  = (lpGIE->giTop & 0xffff);
						rc.right = rc.left + (lpGIE->giGI.giWidth & 0xffff);
						rc.bottom = rc.top + (lpGIE->giGI.giHeight & 0xffff);
						rectDDB.left   = 0;
						rectDDB.top    = 0;
						rectDDB.right  = Bitmap.bmWidth;
						rectDDB.bottom = Bitmap.bmHeight;
						//if( Bitmap.bmWidth != lpGIE->giGI.giWidth )
						//	rectDDB.bottom = lpGIE->giGI.giWidth;
						//if( Bitmap.bmHeight != lpGIE->giGI.giHeight )
						//	rectDDB.right = lpGIE->giGI.giHeight;
						DDBPaint( lpDIBInfo, hDC, &rc, lpGIE->hBitmap, &rectDDB,
							lpGIE->hPal, dwROP );
					}
					else if( lpGIE->hDIB &&
						( hFont = lpGIE->hFont ) &&
						( lpGIE->gceFlag & gie_PTE ) )	// Is PLAIN TEXT EXTENT
					{
						if( lps = DVGlobalLock( lpGIE->hDIB ) )   // LOCK DIB HANDLE
						{
							chktext( w );
							lppte = (LPPTEHDR)lps;
							lps += sizeof( PTEHDR );
							lps += (*lps + 1);
							if( ( len = (int)lppte->pt_Missed ) &&
								( len == lstrlen( lps )) )
							{
								if( (mm = GetMapMode( hDC )) != MM_TEXT )
									SetMapMode( hDC, MM_TEXT );
								if( hOldFont == 0 )
									hOldFont = SelectObject( hDC, hFont );
								else
									SelectObject( hDC, hFont );
								if( !fta )
								{
									ta = GetTextAlign( hDC );
									ffta = ChkTextAlign( hDC );
									fta = TRUE;
								}
								if( lpbmh = DVGlobalLock( hDIB ) )  // LOCK DIB HANDLE
								{
									int	index;
									// Step up to the COLOR TABLE
									lpq = (LPRGBQUAD)(lpbmh + *(LPDWORD)lpbmh);
									index = (int)lpGIE->gceRes2;
									fgcolr = RGB( lpq[index].rgbRed,
										lpq[index].rgbGreen,
										lpq[index].rgbBlue );
//			lpGIE->gceRes2 = (DWORD)lpb[10];	// Foreground index
									index = lpGIE->gceIndex;
//	DWORD	gceColr;	// COLORREF (if SET)
//			lpGIE->gceIndex = lpb[11];	// Backgound INDEX
									bgcolr = RGB( lpq[index].rgbRed,
										lpq[index].rgbGreen,
										lpq[index].rgbBlue );
									DVGlobalUnlock( hDIB );    // UNLOCK DIB HANDLE
								}
								BkMode = GetBkMode( hDC );
								//opts = ETO_CLIPPED;
								opts = 0;
								if( lpGIE->gceBits & gce_TransColr )	// Transparent Color Flag 1 Bit
								{
									SetBkMode( hDC, TRANSPARENT );
								}
								else
								{
									SetBkMode( hDC, OPAQUE ); 
									SetBkColor( hDC, bgcolr );
									opts |= ETO_OPAQUE;
								}
								SetTextColor( hDC, fgcolr );
								crect.left = (lpGIE->giLeft & 0xffff);
								crect.top  = (lpGIE->giTop & 0xffff);
								crect.right = crect.left + GetHPixels( lpGIE ); //(lpGIE->giGI.giWidth & 0xffff);
								crect.bottom = crect.top + GetVPixels( lpGIE ); //(lpGIE->giGI.giHeight & 0xffff);
								GetTextMetrics( hDC, &tm );
								xChar = tm.tmAveCharWidth;
								yChar = tm.tmHeight + tm.tmExternalLeading;
//								if( yChar < (int)( HIBYTE(LOWORD(lpGIE->gceRes1)) & 0xff ) )
//								{
//									yChar = (int)( HIBYTE(LOWORD(lpGIE->gceRes1)) & 0xff );
//								}
								if( yChar > (int)( HIBYTE(LOWORD(lpGIE->gceRes1)) & 0xff ) )
								{
									yChar = (int)( HIBYTE(LOWORD(lpGIE->gceRes1)) & 0xff );
								}
								cy = 0;
								jj = 0;
								kk = 0;
								for( ii = 0; ii < len; ii++ )
								{
									if( lps[ii] == 0x0d )
									{
										if( kk )
										{
											ExtTextOut( hDC,	// handle to device context
												(lpGIE->giLeft & 0xffff),	// x-coordinate of reference point
												((lpGIE->giTop & 0xffff) + cy),	// y-coordinate of reference point
												opts,	// text-output options
												&crect,	// optional clipping and/or opaquing rectangle
												&lps[jj],	// points to string
												kk,	// number of characters in string
												0 );	// pointer to array of intercharacter spacing values
										}
										cy += yChar;	// Bump a ROW
										kk = 0;
									}
									else if( lps[ii] == 0x0a )
									{
										jj = ii + 1;
									}
									else
									{
										kk++;
									}
								}
								if( kk )
								{
									ExtTextOut( hDC,	// handle to device context
									(lpGIE->giLeft & 0xffff),	// x-coordinate of reference point
									((lpGIE->giTop & 0xffff) + cy),	// y-coordinate of reference point
									opts,	// text-output options
									&crect,	// optional clipping and/or opaquing rectangle
									&lps[jj],	// points to string
									kk,	// number of characters in string
									0 );	// pointer to array of intercharacter spacing values
								}
								//TextOut( hDC,	// handle of device context
								//	(lpGIE->giLeft & 0xffff), // x-coordinate of starting position
								//	(lpGIE->giTop & 0xffff), // y-coordinate of starting position
								//	lps,	// address of string
								//	len );	// number of characters in string
								//PutBox( hDC, &crect, 1 );
								SetBkMode( hDC, BkMode );
							}
							DVGlobalUnlock( lpGIE->hDIB );   // UNLOCK DIB HANDLE
						}
					}
				}	// For count fo images
				if( hOldFont )
					SelectObject( hDC, hOldFont );
				if( ffta )
					SetTextAlign( hDC, ta );
			}
			DVGlobalUnlock( hgInfo );
		}
	}

#endif	// WJPEG4

//   if( !IsRectEmpty( &lpDIBInfo->rcClip ) )
//   {
	   // Draw the clipboard selection rubber-band.
#ifdef	WIN32
		SetWindowOrgEx( hDC,
			GetScrollPos( hWnd, SB_HORZ ),
			GetScrollPos( hWnd, SB_VERT ),
			NULL );
#else	// !WIN32
		SetWindowOrg( hDC,
			GetScrollPos( hWnd, SB_HORZ ),
			GetScrollPos( hWnd, SB_VERT ) );
#endif	// WIN32 y/n
      // REPAINT MAGNIFY OR SWISH, IF EXISTING!!!
      if( gi_MouseMagnify ) {
         RePaintMagnify( hDC, lpDIBInfo );
      } else {
         DrawSelect( hDC, &lpDIBInfo->rcClip ); // put up CLIP
         if( Got_Swish( &lpDIBInfo->sSwish ) )
            Draw_CLIP( hDC, &lpDIBInfo->rcClip, lpDIBInfo );  // redraw swish
      }
//   }

#ifdef	TRYCOMP
	BitBlt( rhDC,
		rectClient.left,
		rectClient.top,
		rectClient.right,
		rectClient.bottom,
		hDC,
		0,
		0,
		SRCCOPY );
#endif   // TRYCOMP

	if( hOldPal )
		SelectPalette( hDC, hOldPal, FALSE );

#ifdef	TRYCOMP
	if( hbmold )
		SelectObject( hDC, hbmold );
	DeleteObject( hbm );
	DeleteDC( hDC );
#endif   // TRYCOMP

ABORTPAINT:

#ifdef	WJPEG4
	if( hgInfo && lpGHE )
		DVGlobalUnlock( hgInfo );
#endif	// WJPEG4

#if   (defined(DIAGSCROLL) && defined(CHILDWNDPNT))
	{
		LPSTR	lpd;
		lpd = GetTmp2();
		lstrcpy( lpd, "ChildWndPaint: " );
      if( ( hDIBInfo && lpDIBInfo       ) &&
          ( lpDIBInfo->Options.bAnimate ) )  // Run animation (if NETSCAPE GIF)
      {
         if( lpDIBInfo->di_bChgdDIB )      // if the SIZE changed (for animations)
         {
            wsprintf( EndBuf(lpd),
               "DIB changed from %d x %d to %d x %d."MEOR,
               lpDIBInfo->di_dwDIBWidth,
               lpDIBInfo->di_dwDIBHeight,
               lpDIBInfo->di_dwNEWWidth,	   // NEW Width  of the DIB
               lpDIBInfo->di_dwNEWHeight );	// NEW Height of the DIB
         }
         else
         {
            wsprintf( EndBuf(lpd),
               "DIB %d x %d to child %d x %d."MEOR,
               lpDIBInfo->di_dwDIBWidth,
               lpDIBInfo->di_dwDIBHeight,
	   		   rectClient.right,
   			   rectClient.bottom );
         }
         lpDIBInfo->di_bChgdDIB = FALSE;  // if the SIZE changed (for animations)
      }
      else
      {
         wsprintf( EndBuf(lpd),
			   "Child %d.%d.%d.%d Paint %d.%d.%d.%d"MEOR,
			   rectClient.left,
			   rectClient.top,
			   rectClient.right,
			   rectClient.bottom,
			   ps.rcPaint.left,
			   ps.rcPaint.top,
			   ps.rcPaint.right,
			   ps.rcPaint.bottom );
      }
		DO(lpd);
	}
#endif	// DIAGSCROLL && CHILDWNDPNT

//PaintEnd:

   if( hDIBInfo && lpDIBInfo )
      DVGlobalUnlock( hDIBInfo );

	EndPaint( hWnd, &ps );

}	// End - void ChildWndPaint( HWND )


void	RelChildMem( LPDIBINFO lpDIBInfo )
{
	/* *** ENSURE ALL MEMORY IS RELEASED *** */
	//				if( lpDIBInfo->di_hDIB2 )
	//					DVGlobalFree( lpDIBInfo->di_hDIB2 );
	//				lpDIBInfo->di_hDIB2 = 0;
	if( lpDIBInfo )
	{
      sprtf( "Release CHILD memory of %#x."MEOR, lpDIBInfo );
		if( lpDIBInfo->hDIB )
			DVGlobalFree( lpDIBInfo->hDIB );
		lpDIBInfo->hDIB = 0;

		if( lpDIBInfo->di_hDIB2 )
			DVGlobalFree( lpDIBInfo->di_hDIB2 );
		lpDIBInfo->di_hDIB2 = 0;

		if( lpDIBInfo->di_hCOLR )
			DVGlobalFree( lpDIBInfo->di_hCOLR );
		lpDIBInfo->di_hCOLR = 0;
	}

}

BOOL  WaitThreadExit( PDI lpDIBInfo )
{
   BOOL  bDnThread = TRUE;
   lpDIBInfo->di_bThdExit = TRUE;   // incase COUNT thread is still running
	if( lpDIBInfo->di_hThread )
	{
		// Potentially have a THREAD running doing rendering.
		// Request its EXIT
		gdwThreadSig |= lpDIBInfo->di_dwThreadBit;
		sprtf( "WM_CLOSE: Setting THREAD exit bit 0x%x!"MEOR,
			lpDIBInfo->di_dwThreadBit );
	}

   // if the COUNT thread is STILL running
   // *******************************************************
   if( lpDIBInfo->di_dwCntThdId )   // count thread id
   {
      sprtf( "COUNT THREAD still running ... waiting!"MEOR );
      do
      {
         // ******************************************
         Sleep( 200 );  // switch to thread, hopefully
         // ******************************************
      } while( lpDIBInfo->di_dwCntThdId );

      sprtf( "COUNT THREAD closed..."MEOR );

      bDnThread = FALSE;

   }
   // *******************************************************

   return bDnThread;

}

//---------------------------------------------------------------------
//
// Function:   ChildWndDestroy
//
// Purpose:    Called by ChildWndProc() on WM_DESTROY.  Window is
//             being destroyed, do all necessary cleanup.
//
//             Window's going away, free up the DIB, DDB, Palette, and
//             DIBINFO structure.
//
//             If we're palette animating, kill the timer and free
//             up animation palette.  
//
//             If we have the clipboard, send the WM_RENDERALLFORMATS to 
//             our window (Windows will only send it to our app if the
//             main window owns the clipboard;  in this app, the MDI child
//             window owns the clipboard).
//
//             Decrement the # of DIB windows open global variable.
//
//
// Parms:      hWnd == Handle to window being destroyed.
//
// History:   Date      Reason
//
//            10/15/91  Cut code out from WM_DESTROY case.
//             
//---------------------------------------------------------------------
void	DestroyDIBInfo( HWND hChild, HANDLE hDIBInfo )
{
	PDI      	lpDIBInfo;
	HGLOBAL		hgGIF;
#ifdef	ADDCOLRF
	HGLOBAL		hDIBx;
#endif	// ADDCOLRF
	HWND		   hWnd;
   PMWL        pmwl;

	hWnd = hChild;
	if( lpDIBInfo = (PDI) DVGlobalLock (hDIBInfo) )
	{
      lpDIBInfo->di_bThdExit = TRUE;   // incase COUNT thread is still running
		gdwThreadSig |= lpDIBInfo->di_dwThreadBit;
      // =========================
      WaitThreadExit( lpDIBInfo );  // wait for the thread exit
      // =========================
		if( lpDIBInfo->di_hThread )
		{
			sprtf( "DestroyDibInfo: Setting and awaiting THREAD exit!"MEOR );
			// Potentially have a THREAD running doing rendering
			if( lpDIBInfo->di_dwThreadBit )
			{
				gdwThreadSig |= lpDIBInfo->di_dwThreadBit;
				WaitForSingleObject( lpDIBInfo->di_hThread, 60000 );
			}
		}
		// and if a thread is still running!!!!!!!!!!!!!!!!!!!!

		/* RELEASE ALL CHILD MEMORY */
		RelChildMem( lpDIBInfo );
//		if( lpDIBInfo->hDIB )
//			DVGlobalFree( lpDIBInfo->hDIB );
//		lpDIBInfo->hDIB = 0;

//		if( lpDIBInfo->di_hDIB2 )
//			DVGlobalFree( lpDIBInfo->di_hDIB2 );
//		lpDIBInfo->di_hDIB2 = 0;

//		if( lpDIBInfo->di_hCOLR )
//			DVGlobalFree( lpDIBInfo->di_hCOLR );
//		lpDIBInfo->di_hCOLR = 0;

		if( lpDIBInfo->hPal )
			DeleteObject( lpDIBInfo->hPal );

		if( lpDIBInfo->hBitmap )
			DeleteObject( lpDIBInfo->hBitmap );
		lpDIBInfo->hBitmap = 0;
		lpDIBInfo->di_hBitmap2 = 0;
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
		if( hWnd && lpDIBInfo->hBigFile )
		{
			DeleteAnim( hWnd );
			DVGlobalFree( lpDIBInfo->hBigFile );
		}
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

//#ifdef  USELLIST
      if( !gbInClose )  // NOT in SHUTDOWN of application
      {
         if( pmwl = lpDIBInfo->di_psMWL )
         {
            // remove these flags from the MRU list
            if( pmwl->wl_dwFlag & (flg_IsLoaded | flg_MDIOpen) )
            {
               gfChgFil = TRUE;
               pmwl->wl_dwFlag &= ~(flg_IsLoaded | flg_MDIOpen);
               // fix the MRU list to reflect the UNLOAD
               SetFileMRU( ghFrameWnd, &gsFileList );
            }
            else
            {
               pmwl = 0;   // also try using UnloadMRU() function
            }
         }
         if( !pmwl )
         {
            // even WITHOUT the work list pointer (pmwl)
            // this file should be REMOVED from the MRU
            // or at least the CHECK not added
            if( pmwl = UnloadMRU( hChild, &lpDIBInfo->di_szDibFile[0] ) )
            {
               gfChgFil = TRUE;
               SetFileMRU( ghFrameWnd, &gsFileList );
            }
         }
      }

		// If a GIF, keep INFO
		hgGIF = lpDIBInfo->hgGIFSize; 
		if(hgGIF)
		{
			LPGIFHDREXT	lpGHE;
			LPGIFIMGEXT	lpGIE;
			WORD	w, max;
			lpGHE = (LPGIFHDREXT)DVGlobalLock( hgGIF ); 
			if(lpGHE)
			{
				max = lpGHE->gheImgCount; 
				if(max)
				{
					for( w = 0; w <= max; w++ )
					{
						if( w == 0 ) {
							if( lpGHE->hDIB )
								DVGlobalFree( lpGHE->hDIB );
							if( lpGHE->hPal )
								DeleteObject( lpGHE->hPal );
							if( lpGHE->hBitmap )
								DeleteObject( lpGHE->hBitmap );
							if( lpGHE->hFont )
								DeleteObject( lpGHE->hFont );
							lpGHE->hDIB    = 0;
							lpGHE->hPal    = 0;
							lpGHE->hBitmap = 0;
							lpGHE->hFont   = 0;
                  } else {
							lpGIE = &lpGHE->gheGIE[w-1];
							if( lpGIE->hDIB )
								DVGlobalFree( lpGIE->hDIB );
							if( lpGIE->hPal )
								DeleteObject( lpGIE->hPal );
							if( lpGIE->hBitmap )
								DeleteObject( lpGIE->hBitmap );
							if( lpGIE->hFont )
								DeleteObject( lpGIE->hFont );
							lpGIE->hDIB    = 0;
							lpGIE->hPal    = 0;
							lpGIE->hBitmap = 0;
							lpGIE->hFont   = 0;
						}
					}
				}
				DVGlobalUnlock( hgGIF );
			}
			DVGlobalFree( hgGIF );
		}

      // any other actions?

		DVGlobalUnlock( hDIBInfo );
	}
		// This can be DESTROYED
		// =====================
#ifdef	ADDCOLRF
		// Check if there is an EXTRA for the FRAME
		// ========================================
	hDIBx = ghDIBx; 
	if(hDIBx) {
		DVGlobalFree( hDIBx );
		ghDIBx = 0;
	}

#endif	// ADDCOLRF
}

// from CHILD MDI PROOC ON WM_DESTROY
void ChildWndDestroy( HWND hWnd )
{
	HANDLE		hDIBInfo;
//	LPDIBINFO	lpDIBInfo;
//	LPSTR		lpf;
//	HGLOBAL		hgGIF;
#ifdef	ADDCOLRF
	HGLOBAL		hDIBx;
#endif	// ADDCOLRF

   sprtf( "Child: WM_DESTROY on handle %#x."MEOR, hWnd );

	// If we have the clipboard, render all our formats now.
	if( hWnd == GetClipboardOwner() )
	{
		SendMessage( hWnd, WM_RENDERALLFORMATS, 0, 0L );
		hWndClip = NULL;
	}

	// If we're animating, turn off the timer.
	if( hWndAnimate == hWnd )
	{
		KillTimer( hWnd, TIMER_ID2 );
		hWndAnimate = NULL;
	}

	// Free up resources connected to this window.
	hDIBInfo = GetWindowExtra( hWnd, WW_DIB_HINFO );
	if( hDIBInfo )
	{
      DestroyDIBInfo( hWnd, hDIBInfo );
		DVGlobalFree( hDIBInfo );
		SetWindowExtra( hWnd, WW_DIB_HINFO, NULL );
	}

#ifdef	ADDCOLRF
	hDIBx = GetWindowExtra( hWnd, WW_DIB_EX2 ); 
	if(hDIBx)
	{
		DVGlobalFree( hDIBx );
		hDIBx = 0;
		SetWindowExtra( hWnd, WW_DIB_EX2, NULL );
	}
#endif	// ADDCOLRF

   // last thing - reduce open count
   if( gnDIBsOpen )
      gnDIBsOpen--;

   if( !gbInClose )
   {
		if( gnDIBsOpen == 0 )
			EnableWindowAndOptionsMenus( FALSE );
   }

}	// End - void ChildWndDestroy( HWND hWnd )

//---------------------------------------------------------------------
//
// Function:   ChildWndScroll
//
// Purpose:    Called by ChildWndProc() on WM_HSCROLL and WM_VSCROLL.
//             Window needs to be scrolled (user has clicked on one
//             of the scroll bars.
//
//             Does scrolling in both horiziontal and vertical directions.
//             Note that the variables are all named as if we were
//             doing a horizontal scroll.  However, if we're doing a
//             vertical scroll, they are initialized to the appropriate
//             values for a vertical scroll.
//
//             If we scroll by one (i.e. user clicks on one of the
//             scrolling arrows), we scroll the window by 1/SCROLL_RATIO
//             of the client area.  In other words, if SCROLL_RATION==4,
//             then we move the client area over a 1/4 of the width/height
//             of the screen.
//
//             If the user is paging up/down we move a full client area's
//             worth.
//
//             If the user moves the thumb to an absolute position, we
//             just move there.
//
//             ScrollWindow/re-painting do the actual work of scrolling.
//
// Parms:      hWnd        == Handle to window being scrolled.
//             message     == Message being handled (WM_HSCROLL or WM_VSCROLL)
//             wPos        == Thumb position (only valid for SB_THUMBPOSITION
//                            and SB_THUMBTRACK).
//             wScrollType == wParam to WM_SCROLL (one of the SB_* constants)
//
// History:   Date      Reason
//
//            10/15/91  Cut code out from WM_?SCROLL case.
//             
//---------------------------------------------------------------------

void ChildWndScroll( HWND hWnd, int message, WORD wPos, WORD wScrollType)
{
	int		xBar;		// Where scrollbar is now.
	int		nMin;		// Minumum scroll bar value.
	int		nMax;		// Maximum scroll bar value.
	int		dx;			// How much to move.
	int		nOneUnit;	// # of pixels for LINEUP/LINEDOWN
	int		cxClient;	// Width of client area.
	int		nHorzOrVert;// Doing the horizontal or vertical?
	RECT	rect;		// Client area.

	GetClientRect( hWnd, &rect );

	if( message == WM_HSCROLL )
	{
		nHorzOrVert = SB_HORZ;
		cxClient    = rect.right - rect.left;	// Get WIDTH
	}
	else
	{
		nHorzOrVert = SB_VERT;
		cxClient    = rect.bottom - rect.top;	// Get HEIGHT
	}

	// One a SB_LINEUP/SB_LINEDOWN we will move the DIB by
	//  1/SCROLL_RATIO of the client area (i.e. if SCROLL_RATIO
	//  is 4, it will scroll the DIB a quarter of the client
	//  area's height or width.

	nOneUnit = cxClient / SCROLL_RATIO;
	if( !nOneUnit )
		nOneUnit = 1;

	xBar = GetScrollPos( hWnd, nHorzOrVert );
	GetScrollRange( hWnd, nHorzOrVert, &nMin, &nMax );

	switch( wScrollType ) {
	case SB_LINEDOWN:	// One line right.
		dx = nOneUnit;
		break;
	case SB_LINEUP:		// One line left.
		dx = -nOneUnit;
		break;
	case SB_PAGEDOWN:	// One page right.
		dx = cxClient;
		break;
	case SB_PAGEUP:		// One page left.
		dx = -cxClient;
		break;
	case SB_THUMBPOSITION:	// Absolute position.
		dx = wPos - xBar;
		break;
	default:			// No change.
		dx = 0;
		break;
	}

	if( dx ) {
		xBar += dx;
		if( xBar < nMin )
		{
			dx  -= xBar - nMin;
			xBar = nMin;
		}
		if( xBar > nMax )
		{
			dx  -= xBar - nMax;
			xBar = nMax;
		}
		if( dx )
		{
			SetScrollPos( hWnd, nHorzOrVert, xBar, TRUE );

			if( nHorzOrVert == SB_HORZ )
				ScrollWindow( hWnd, -dx, 0, NULL, NULL );
			else
				ScrollWindow( hWnd, 0, -dx, NULL, NULL );

			UpdateWindow( hWnd );
		}
	}

#ifdef	DIAGSCROLL
	{
		LPSTR	lpd;
		lpd = GetTmp3();
		wsprintf( lpd, "ChildWndScroll: (%x)", hWnd );
		if( nHorzOrVert == SB_HORZ )
			lstrcat( lpd, " H = " );
		else
			lstrcat( lpd, " V = " );
		wsprintf( EndBuf(lpd),
			"%d Scroll = %d"MEOR,
			xBar,
			dx );
		DO(lpd);
	}
#endif	// DIAGSCROLL


}


//---------------------------------------------------------------------
//
// Function:   ChildWndKeyDown
//
// Purpose:    Called by ChildWndProc() on WM_KEYDOWN.
//				Keyboard interface for MDI DIB Child window.
//
//             Keyboard interface.  Handles scrolling around the DIB
//             using the keypad, and translates ESC's into WM_RBUTTONDOWN
//             messages (to stop any palette animation in progress).
//
//             The numeric keypad/arrows are translated into scroll
//             bar messages.
//
// Parms:      hWnd     == Handle to window where key was pressed.
//             wKeyCode == Key code pressed (wParam to WM_KEYDOWN -- one
//                         of the VK_* constants)
//             lParam   == lParam for WM_KEYDOWN.
//
// History:   Date      Reason
//
//            10/15/91  Cut code out from WM_KEYDOWN case.
//             
//---------------------------------------------------------------------

DWORD ChildWndKeyDown( HWND hWnd, WORD wKeyCode, LPARAM lParam )
{
	unsigned	uMsg;
	WORD		wSB;
	LPARAM		lPar;

	uMsg = WM_KEYDOWN;
	wSB = wKeyCode;
	lPar = 0;

	switch( wKeyCode ) {

	case VK_ESCAPE:
		SendMessage( hWnd, WM_RBUTTONDOWN, 0, 0L );
		break;

	case VK_NUMPAD8:
	case VK_UP:
		uMsg = WM_VSCROLL;
		wSB  = SB_LINEUP;
		break;

	case VK_NUMPAD2:
	case VK_DOWN:
		uMsg = WM_VSCROLL;
		wSB  = SB_LINEDOWN;
		break;

	case VK_NUMPAD4:
	case VK_LEFT:
		uMsg = WM_HSCROLL;
		wSB  = SB_LINEUP;
		break;

	case VK_NUMPAD6:
	case VK_RIGHT:
		uMsg = WM_HSCROLL;
		wSB  = SB_LINEDOWN;
		break;

	case VK_PRIOR:
	case VK_NUMPAD9:
		uMsg = WM_VSCROLL;
		wSB  = SB_PAGEUP;
		break;

	case VK_NEXT:
	case VK_NUMPAD3:
		uMsg = WM_VSCROLL;
		wSB  = SB_PAGEDOWN;
		break;

//	case VK_NUMPAD7:
//		uMsg = WM_HSCROLL;
//		wSB  = SB_PAGEUP;
//		break;

//	case VK_NUMPAD1:
//		uMsg = WM_HSCROLL;
//		wSB  = SB_PAGEDOWN;
//		break;

	case VK_NUMPAD7:
	case VK_HOME:
		uMsg = WM_HSCROLL;
		wSB = SB_THUMBPOSITION;		// Absolute position.
		// Window's being scrolled, call the scroll handler.
		// case WM_HSCROLL:
		// case WM_VSCROLL:
		// ChildWndScroll( hWnd, (int)message, LOWORD (lParam), (WORD)wParam);
		SendMessage( hWnd, uMsg, wSB, 0L );	// Horiz. to HOME
		uMsg = WM_VSCROLL;
		break;		// and final Vertical to HOME

	case VK_NUMPAD1:
	case VK_END:
		{
			int	nMin, nMax;
			uMsg = WM_HSCROLL;
			wSB = SB_THUMBPOSITION;		// Absolute position.
			GetScrollRange( hWnd, SB_HORZ, &nMin, &nMax );
			lPar = nMax;
			SendMessage( hWnd, uMsg, wSB, lPar );	// Horiz. to END
			uMsg = WM_VSCROLL;
			GetScrollRange( hWnd, SB_VERT, &nMin, &nMax );
			lPar = nMax;
			// final Vert. to END
		}
		break;

	default:
		return DefMDIChildProc( hWnd, uMsg, wSB, lParam );

	}

	return SendMessage( hWnd, uMsg, wSB, lPar );

}

//---------------------------------------------------------------------
//
// Function:   ChildWndQueryNewPalette
//
// Purpose:    Called by ChildWndProc() on WM_QUERYNEWPALETTE.
//
//             We get this message when an MDI child is getting
//             focus (by hocus pockus in FRAME.C, and by passing
//             this message when we get WM_MDIACTIVATE).  Normally
//             this message is passed only to the top level window(s)
//             of an application.
//
//             We want this window to have the foreground palette when this
//             happens, so we select and realize the palette as
//             a foreground palette (of the frame Window).  Then make
//             sure the window repaints, if necessary.
//
// Parms:      hWnd      == Handle to window getting WM_QUERYNEWPALETTE.
//             hWndFrame == Handle to the frame window (i.e. the top-level
//                            window of this app.
//
// History:   Date      Reason
//
//            10/15/91  Cut code out from WM_QUERYNEWPALETTE case.
//            12/03/91  Added hWndFrame parameter, realization
//                      of palette as palette of frame window,
//                      and updating the window only if the
//                      palette changed.
//             
//---------------------------------------------------------------------

BOOL ChildWndQueryNewPalette (HWND hWnd, HWND hWndFrame)
{
   HPALETTE  hOldPal;
   HDC       hDC;
   HANDLE    hDIBInfo;
   LPDIBINFO lpDIBInfo;
   int       nColorsChanged;

   hDIBInfo = GetWindowExtra(hWnd, WW_DIB_HINFO);

   if (!hDIBInfo)
      return FALSE;

   lpDIBInfo = (LPDIBINFO) DVGlobalLock (hDIBInfo);
   if( !lpDIBInfo )
      return FALSE;

   if (!lpDIBInfo->hPal)
   {
      DVGlobalUnlock (hDIBInfo);
      return FALSE;
   }


      // We're going to make our palette the foreground palette for
      //  this application.  Window's palette manager expects the
      //  top-level window of the application to have the palette,
      //  so, we get a DC for the frame here!

   hDC     = GetDC (hWndFrame);
   hOldPal = SelectPalette (hDC, lpDIBInfo->hPal, FALSE);
   
   nColorsChanged = RealizePalette (hDC);

   if (nColorsChanged)
      InvalidateRect (hWnd, NULL, FALSE);

   if (hOldPal)
      SelectPalette (hDC, hOldPal, FALSE);

   ReleaseDC (hWndFrame, hDC);

   DVGlobalUnlock (hDIBInfo);

   return (nColorsChanged != 0);
}




//---------------------------------------------------------------------
//
// Function:   ChildWndPaletteChanged
//
// Purpose:    Called by ChildWndProc() on WM_PALETTECHANGED.
//
//             WM_PALETTECHANGED messages are passed to all MDI
//             children by the frame window (in FRAME.C).  Normally,
//             these messages are only sent to the top-level window
//             in an application.
//
//             On a palette changed, we want to realize this window's
//             palette.  We realize it always as a background palette.
//             See the comments section at the top of this file for
//             an explanation why.
//
// Parms:      hWnd == Handle to window getting WM_PALETTECHANGED.
//
// History:   Date      Reason
//
//            10/15/91  Cut code out from WM_PALETTECHANGED case.
//            12/03/91  Always force SelectPalette() to a
//                        background palette.  If it was
//                        the foreground palette, it would
//                        have been realized during
//                         WM_QUERYNEWPALETTE.
//             
//---------------------------------------------------------------------

void ChildWndPaletteChanged (HWND hWnd)
{
   HPALETTE  hOldPal;
   HDC       hDC;
   HANDLE    hDIBInfo;
   LPDIBINFO lpDIBInfo;

   hDIBInfo = GetWindowExtra(hWnd, WW_DIB_HINFO);

   if( !hDIBInfo )
      return;

   lpDIBInfo = (LPDIBINFO) DVGlobalLock (hDIBInfo);
   if( !lpDIBInfo )
      return;

   if( !lpDIBInfo->hPal)
   {
      DVGlobalUnlock (hDIBInfo);
      return;
   }

   hDC     = GetDC (hWnd);
   hOldPal = SelectPalette (hDC, lpDIBInfo->hPal, TRUE);

   DVGlobalUnlock (hDIBInfo);
   
   RealizePalette (hDC);
   UpdateColors (hDC);

   if (hOldPal)
      SelectPalette (hDC, hOldPal, FALSE);

   ReleaseDC (hWnd, hDC);
}




//---------------------------------------------------------------------
//
// Function:   ChildWndStartAnimate
//
// Purpose:    Called by ChildWndProc() on MYWM_ANIMATE.
//
//             Animate this DIB's palette.  First do some setup (see if
//             we're already doing animation, and start a timer).  Then
//             Create a new palette with the PC_RESERVED flag so it can
//             be animated.  Then re-create the bitmap so it uses this
//             new palette.  Finally, set our window words and re-draw
//             the bitmap.
//
//
// Parms:      hWnd == Handle to window getting MYWM_ANIMATE.
//
// History:   Date      Reason
//
//            10/15/91  Cut code out from MYWM_ANIMATE case.
//             
//---------------------------------------------------------------------

void ChildWndStartAnimate (HWND hWnd)
{
   HPALETTE  hNewPal;
   HANDLE    hDIBInfo;
   LPDIBINFO lpDIBInfo;


      // Don't allow more than one window to animate at a time.

   if (hWndAnimate)
      {
      DIBError (ERR_ANIMATE);
      return;
      }


      // Set a timer to animate on.

   if (!SetTimer( hWnd, TIMER_ID2, TIMER_INTERVAL2, NULL))
      {
      DIBError (ERR_NOTIMERS);
      return;
      }


      // Remember who's animating, get the palette for this window,
      //  copy it changing its flags to PC_RESERVED (so each palette
      //  entry can be animated).

   hWndAnimate = hWnd;
   hDIBInfo    = GetWindowExtra(hWnd, WW_DIB_HINFO);
   if( !hDIBInfo )
      return;
   lpDIBInfo   = (LPDIBINFO) DVGlobalLock (hDIBInfo);
   if( !lpDIBInfo )
      return;

   hNewPal     = CopyPalForAnimation (lpDIBInfo->hPal);


      // Delete the old device dependent bitmap, and palette.  Device
      //  dependent bitmaps rely on colors mapping to the exact same
      //  place in the system palette (for speed reasons).  Se we
      //  changed the palette flags to PC_RESERVED, the palette entries
      //  might not map to the same palette entries.  Therefore, it is
      //  necessary to create a new device dependent bitmap here!

   if (lpDIBInfo->hBitmap)
      DeleteObject (lpDIBInfo->hBitmap);

   if (lpDIBInfo->hPal)
      DeleteObject (lpDIBInfo->hPal);

   lpDIBInfo->hBitmap = DIBToBitmap( lpDIBInfo->hDIB, hNewPal, lpDIBInfo );
   lpDIBInfo->hPal    = hNewPal;

   DVGlobalUnlock (hDIBInfo);

   InvalidateRect (hWnd, NULL, FALSE);
}

//---------------------------------------------------------------------
//
// Function:   ChildWndSize
//
// Purpose:    Called by ChildWndProc() on WM_SIZE.
//
//             When the window is sized -- set up the scroll bars.
//             Also, if we're in "stretch to window" mode, the entire
//             client area must be repainted.  
//
//             The window will be repainted if the new size, combined
//             with the current scroll bar positions would create "white
//             space at the left or bottom of the window.  For example,
//             if the DIB is 100x100, the window _was_ 50x50, the new
//             size of the window is 75x75, and the current window is
//             scrolled 50 units to the right; then, if the current
//             scroll position weren't changed, we'd need to paint
//             starting at row 50, and extending through row 125.  BUT
//             since the DIB is only 100 pixels wide, white space would
//             appear at the right margin!  Instead, the thumb is placed
//             at column 25 (in SetScrollPos), and columns 25 through 
//             100 are displayed (by invalidating the client window!
//
//             Re-read the above paragraph (slowly this time)!
//
// Parms:      hWnd == Handle to window getting WM_SIZE.
//
// History:   Date      Reason
//            10/15/91  Cut code out from WM_SIZE case.
//             
//---------------------------------------------------------------------
// WM_SIZE  
// fwSizeType = wParam;      // resizing flag 
// nWidth = LOWORD(lParam);  // width of client area 
// nHeight = HIWORD(lParam); // height of client area 

void ChildWndSize( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	HANDLE		hDIBInfo;
	LPDIBINFO	lpDIBInfo;
	LPSTR		lpDIB;
	int			cxScroll, cyScroll;
	int			cxDIB, cyDIB;
	RECT		rect;
	WORD		fwSizeType, nWidth, nHeight;

	cxDIB = 0;
	cyDIB = 0;

	if( ( hDIBInfo = GetWindowExtra(hWnd, WW_DIB_HINFO)   ) &&
		( lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo ) ) )
	{
		// Get parameters passed - But NOT yet used here
		fwSizeType = (WORD) wParam;
		nWidth     = LOWORD( lParam );	// Client WIDTH
		nHeight    = HIWORD( lParam );	// Client HEIGHT

		// Find out the DIB's height/width.
		if( lpDIBInfo->hDIB )
		{
			if( lpDIB = DVGlobalLock( lpDIBInfo->hDIB ) )   // LOCK DIB HANDLE
         {
            cxDIB = (int) DIBWidth( lpDIB );
			   cyDIB = (int) DIBHeight(lpDIB );
            DVGlobalUnlock( lpDIBInfo->hDIB );  // UNLOCK DIB HANDLE
         }
		}

		// Find out the dimensions of the window, and the current
		//  thumb positions.

		GetClientRect( hWnd, &rect );
		cxScroll = GetScrollPos( hWnd, SB_HORZ );
		cyScroll = GetScrollPos( hWnd, SB_VERT );

		// If we are in "stretch to window" more, or the current
		//  thumb positions would cause "white space" at the right
		//  or bottom of the window, repaint.

		if( ( lpDIBInfo->Options.bStretch2W  ) ||
			 ( cxScroll + rect.right > cxDIB  ) ||
			 ( cyScroll + rect.bottom > cyDIB ) )
		{
			InvalidateRect( hWnd, NULL, FALSE );
		}

		if( !( IsIconic( hWnd )              ) &&
			 !( lpDIBInfo->Options.bStretch2W ) )
		{
			SetupScrollBars( lpDIBInfo,
				hWnd,
				lpDIBInfo->di_dwDIBWidth,
				lpDIBInfo->di_dwDIBHeight );
		}

		DVGlobalUnlock( hDIBInfo );
	}
}


//---------------------------------------------------------------------
//
// Function:   SetupScrollBars
//
// Purpose:    Sets up MDI Child's scroll bars
//
//             Either we display both scroll bars, or no scroll bars.
//
// Parms:      hWnd == Handle to window who's scroll bars we'll set up.
//             hDIB == A handle to the current DIB.
//
// History:   Date      Reason
//            6/1/91    Created.
//             
//---------------------------------------------------------------------

void SetupScrollBars( LPDIBINFO lpDIBInfo, HWND hWnd,
					 DWORD cxDIB, DWORD cyDIB )
{
	RECT	rect;			// Client Rectangle.
	BOOL	bNeedScrollBars;// Need Scroll bars?
	unsigned	cxWindow,	// Width of client area.
				cyWindow;	// Height of client area.
	int		cxRange = 0,	// Range needed for horz bar.
			cyRange = 0;	// Range needed for vert bar.
	BOOL	flg;


	// Do some initialization.
	bNeedScrollBars = FALSE;

	ReallyGetClientRect( hWnd, &rect );

	cxWindow = rect.right - rect.left;
	cyWindow = rect.bottom - rect.top;

	// Now determine if we need the scroll bars.  Since the
	//  window is not allowed to be larger than the DIB, if
	//  we need one scroll bar, we may need _both_.  Since if
	//  one scroll bar is turned on, it eats up some of
	//  the client area.

	if(( cxWindow < (unsigned) cxDIB ) ||
		( cyWindow < (unsigned) cyDIB ) )
		bNeedScrollBars = TRUE;

	// Setup the scroll bar ranges.  We want to be able to
	//  scroll the window so that all the DIB can appear
	//  within the client area.  Take into account that
	//  if the opposite scroll bar is activated, it eats
	//  up some client area.

	if( bNeedScrollBars )
	{
		cyRange = (unsigned) cyDIB - cyWindow - 1 +
			GetSystemMetrics( SM_CYHSCROLL );

		cxRange = (unsigned) cxDIB - cxWindow - 1 +
			GetSystemMetrics( SM_CXVSCROLL );
	}

	// Set the ranges we've calculated.
	// ( 0 -> 0 means invisible scrollbar )
	flg = FALSE;
	if( ( cyRange == 0 ) ||
		( lpDIBInfo->di_iVert != cyRange ) )
	{
		flg = TRUE;
		lpDIBInfo->di_iVert = cyRange;
		SetScrollRange( hWnd, SB_VERT, 0, cyRange, TRUE );
	}
	if( ( cxRange == 0 ) ||
		( lpDIBInfo->di_iHorz != cxRange ) )
	{
		flg = TRUE;
		lpDIBInfo->di_iHorz = cxRange;
		SetScrollRange( hWnd, SB_HORZ, 0, cxRange, TRUE );
	}

#ifdef	DIAGSCROLL
	if( flg )
	{
		LPSTR	lpd;
		lpd = GetStgBuf();
		wsprintf( lpd,
			"SetupScrollBars: Rect %s H0-%d V0-%d"MEOR,
         Rect2Stg(&rect),
			cxRange,
			cyRange );
		DO(lpd);
	}
#endif	// DIAGSCROLL

}



//---------------------------------------------------------------------
//
// Function:   ScrollBarsOff
//
// Purpose:    Turns off scroll bars on the specified window.
//
// Parms:      hWnd == Handle to window to turn the scroll bars off in.
//
// History:   Date      Reason
//
//            6/1/91    Created.
//             
//----------------------------------------------------------------------

void ScrollBarsOff( LPDIBINFO lpDIBInfo, HWND hWnd )
{

	lpDIBInfo->di_iVert = 0;
	lpDIBInfo->di_iHorz = 0;
	SetScrollRange( hWnd, SB_VERT, 0, 0, TRUE );
	SetScrollRange( hWnd, SB_HORZ, 0, 0, TRUE );

}


//---------------------------------------------------------------------
// Function:   GetCurrentMDIWnd
// Purpose:    Returns the currently active MDI child window.
// Parms:      None.
// History:   Date      Reason
//            6/1/91    Created.
// get current active MDI child window handle
//---------------------------------------------------------------------

HWND GetCurrentMDIWnd (void)
{
#ifdef	WIN32
	ghActMDIWnd = (HWND)SendMessage ( ghWndMDIClient, (UINT)WM_MDIGETACTIVE, 0, 0L);
#else
	ghActMDIWnd = LOWORD (SendMessage ( ghWndMDIClient, WM_MDIGETACTIVE, 0, 0L));
#endif	// WIN32 y/n
   return ghActMDIWnd;
}

//---------------------------------------------------------------------
//
// Function:   CurrentDIBPalette
//
// Purpose:    Returns a handle to a duplicate of the current MDI
//             child window's palette.
//
//             This is used whenever anyone wants to use the same
//             palette -- a duplicate is created, since the MDI
//             child window could be destroyed at any time (and it's
//             palette is destroyed when the window goes away).
//
// Parms:      None.
//
// History:   Date      Reason
//
//            6/1/91    Created.
//            1998      circa - created a BIG 24-bit palette
//             
//---------------------------------------------------------------------

HPALETTE CurrentDIBPalette( HWND hWnd )
{
	HPALETTE     hDIBPal;
	HANDLE       hDIBInfo, hDIB;
	LPDIBINFO    lpDIBInfo;
	HPALETTE		 hCopy, hPal24;

	hCopy = 0;
	if( hWnd )
	{
		// Get the current palette from the MDI Child's
		// window "extra" section.
		if( hDIBInfo = GetWindowExtra( hWnd, WW_DIB_HINFO ) )
		{
			if( lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo ) )
         {
			   hDIBPal   = lpDIBInfo->hPal;
			   if( hDIBPal )
			   {
				   /* copy of the palette               */
				   /* like #define CopyPalette(hPal) as */
				   /* CopyPaletteChangingFlags( hPal,   */
				   /* (BYTE) DONT_CHANGE_FLAGS )        */
				   hCopy = (HPALETTE) CopyPalette( hDIBPal );
			   }
			   else	/* NOTE: 24 BBP Bitmaps do NOT have a palette ... */
			   {  	/* So let's take the TIME to 'create' one ... */
				   if( hDIB = lpDIBInfo->hDIB ) /* Get the DIB handle ... */
				   {
                  /* Big effort to CREATE a LOGICAL PALETTE ... */
					   hPal24 = CreateDIBPal24( lpDIBInfo, hDIB, FALSE ); /* Create a PALETTE */
					   if( hPal24 == 0 )
						   DIBError( ERR_COPYPAL );
					   else
						   hCopy = hPal24;
				   }
			   }
			   DVGlobalUnlock( hDIBInfo );
         }
		}
	}

	return( hCopy );
}

//---------------------------------------------------------------------
//
// Function:   GetCurrentDIBStretchFlag
//
// Purpose:    Returns the current MDI child window's stretch flag
//             (stored in the INFOSTRUCT stored in a DIB window's
//             window words).
//
// Parms:      None.
//
// History:   Date      Reason
//
//            6/1/91    Created.
//             
//---------------------------------------------------------------------

BOOL GetCurrentDIBStretchFlag (void)
{
   HWND      hWndDIB;
   HANDLE    hDIBInfo;
   LPDIBINFO lpDIBInfo;
   BOOL      bStretch;

   hWndDIB = GetCurrentMDIWnd ();

   if( hWndDIB )
   {
      hDIBInfo = GetWindowExtra(hWndDIB, WW_DIB_HINFO);
      if (!hDIBInfo)
         return FALSE;
      lpDIBInfo = (LPDIBINFO) DVGlobalLock (hDIBInfo);
      if( !lpDIBInfo )
         return FALSE;

      bStretch  = lpDIBInfo->Options.bStretch2W;
      DVGlobalUnlock (hDIBInfo);

      return bStretch;
   }
   else
      return FALSE;
}


//---------------------------------------------------------------------
//
// Function:   SetCurrentDIBStretchFlag
//
// Purpose:    Sets the current MDI child window's stretch flag
//             (stored in the INFOSTRUCT stored in a DIB window's
//             window words).
//
// Parms:      bFlag == New flag setting.
//
// History:   Date      Reason
//
//            6/1/91    Created.
//             
//---------------------------------------------------------------------

void SetCurrentDIBStretchFlag (BOOL bFlag)
{
   HANDLE    hDIBInfo;
   LPDIBINFO lpDIBInfo;
   HWND      hWnd;

   hWnd = GetCurrentMDIWnd ();
   
   if( !hWnd )
      return;
      
   hDIBInfo = GetWindowExtra(hWnd, WW_DIB_HINFO);
   if( !hDIBInfo )
      return;

   if( lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo ) )
   {
      lpDIBInfo->Options.bStretch2W = bFlag;
      DVGlobalUnlock( hDIBInfo );
   }
}



//---------------------------------------------------------------------
//
// Function:   ReallyGetClientRect
//
// Purpose:    Gets the rectangular area of the client rect including
//             the area underneath visible scroll bars.  Stolen from
//             ShowDIB.
//
// Parms:      hWnd   == Window to get the client area of.
//             lpRect == Where to copy the rectangle to.
//
// History:   Date      Reason
//
//            6/1/91    Created.
//             
//---------------------------------------------------------------------

void ReallyGetClientRect (HWND hWnd, LPRECT lpRect)
{
   DWORD dwWinStyle;

   dwWinStyle = GetWindowLong (hWnd, GWL_STYLE);

   GetClientRect (hWnd, lpRect);

   if (dwWinStyle & WS_HSCROLL)
      lpRect->bottom -= GetSystemMetrics (SM_CYHSCROLL);

   if (dwWinStyle & WS_VSCROLL)
      lpRect->right  -= GetSystemMetrics (SM_CXVSCROLL);
}

void ReallyGetClientRect_PLUS (HWND hWnd, LPRECT lpRect)
{
   DWORD dwWinStyle;

   dwWinStyle = GetWindowLong (hWnd, GWL_STYLE);

   GetClientRect (hWnd, lpRect);

   if (dwWinStyle & WS_HSCROLL)
      lpRect->bottom += GetSystemMetrics (SM_CYHSCROLL);

   if (dwWinStyle & WS_VSCROLL)
      lpRect->right  += GetSystemMetrics (SM_CXVSCROLL);
}


//---------------------------------------------------------------------
//
// Function:   SetMessageToAllChildren
//
// Purpose:    Passes a message to all children of the specified window.
//
// Parms:      hWnd == Parent window.
//             message == message to pass to all children.
//             wParam  == wParam of message to pass to all children.
//             lParam  == lParam of message to pass to all children.
//
// History:   Date      Reason
//
//            6/1/91    Created.
//             
//---------------------------------------------------------------------
void SendMessageToAllChildren( HWND hWnd,
							  UINT message,
							  WPARAM wParam,
							  LPARAM lParam)
{
	HWND hChild;
	if( hChild = GetWindow( hWnd, GW_CHILD ) )     // Get 1st child.
	{
		do
		{
			/* Actions in do loop ... */
			SendMessage( hChild, message, wParam, lParam);

		}while( hChild = GetWindow( hChild, GW_HWNDNEXT ));
	}
}


/*-------------------  CloseAllChildren  -----------------------------*/
//---------------------------------------------------------------------
//
// Function:   CloseAllDIBWindows
//
// Purpose:    Close all DIB windows currently open.
//
//             First hides the MDI client -- this insures that no drawing
//             occurs in the DIB windows while we delete windows.
//             Then enumerate all the MDI client's children.  If the
//             child is a DIB window, delete it.  If it's a title bar,
//             leave it alone, and let Windows take care of it.
//
//
// Parms:      None
//
// History:   Date      Reason
//
//            6/1/91    Created.
//             
//---------------------------------------------------------------------
#define MAX_CHILD 32
void CloseAllDIBWindows( void )
{
   HWND  hChild;
   BOOL  bWasVisible;
   HWND  hList[MAX_CHILD];
   UINT  ui, uc = 0;
   int done = 0;

   // hide the MDI client window to avoid multiple repaints
   bWasVisible = ShowWindow( ghWndMDIClient, SW_HIDE);

   // As long as the MDI client has a child, destroy it
	while( hChild = GetWindow( ghWndMDIClient, GW_CHILD ) )
	{
		// Skip the icon title windows (they have owners, MDI children
		//  don't -- child windows have parents, not owners; title
		//  bars' owners are the MDI child windows themselves).
		while( hChild && GetWindow( hChild, GW_OWNER ) )
		{
	         hChild = GetWindow( hChild, GW_HWNDNEXT );
		}

		if( !hChild )
			break;
		if (uc) {
			for (ui = 0; ui < uc; ui++) {
				if (hList[ui] == hChild) {
					done = 1;
					break;
				}
			}
		}

		SendMessage( ghWndMDIClient, WM_MDIDESTROY, (WPARAM)hChild, 0L );
		if (done)
			break;

		hList[uc++] = hChild;

		if (uc >= MAX_CHILD)
			break;
	}

   // Make the MDI Client visible again, if it was visible when
   //  we called ShowWindow(..., SW_HIDE).
   if (bWasVisible)
      ShowWindow( ghWndMDIClient, SW_SHOWNORMAL );
}

/*
 *
 *  FUNCTION   : CloseAllChildren ()       
 *                 
 *  PURPOSE    : Destroys all MDI child windows.
 *
 */
VOID CloseAllChildren(VOID)
{
   CloseAllDIBWindows();
}


#ifdef	DIAGTE
DWORD	GetTE( HDC hDC, LPSTR szStr, int nLen )
{
	DWORD	dwExt;
	SIZE	tSize;

//BOOL GetTextExtentPoint(
//    HDC hdc,  // handle of device context
//    LPCTSTR lpString, // address of text string
//    int cbString,     // number of characters in string
//    LPSIZE lpSize     // address of structure for string size
////);
	dwExt = (((WORD)1 << 16) | ((WORD)1));
	if( GetTextExtentPoint( hDC, szStr, nLen, &tSize ) )
	{
		dwExt =  (((WORD)tSize.cy << 16) | ((WORD)tSize.cx) );
	}
	return dwExt;
}


#endif	// DIAGTE



#if	(defined( DIAGSCROLL ) || defined( SHOWCLIP ))

void	AddCrLf( LPSTR lpd )
{
	if( lpd )
		lstrcat( lpd, ""MEOR );
}

void	AddSzs( LPSTR lpd, LPSTR lpf, LPRECT lprcClip )
{
	if( ( lpd ) &&
		( lpf ) &&
		( lprcClip ) )
	{
		wsprintf( EndBuf(lpd),
			lpf,
			lprcClip->left,
			lprcClip->top,
			(lprcClip->right - lprcClip->left),
			(lprcClip->bottom - lprcClip->top) );
	}
}

void	AddSN( LPSTR lpd, LPRECT lprcClip )
{
	if( ( lpd ) &&
		( lprcClip ) )
	{
		if( ( rcPrevClip.left   != lprcClip->left   ) ||
			( rcPrevClip.top    != lprcClip->top    ) ||
			( rcPrevClip.right  != lprcClip->right  ) ||
			( rcPrevClip.bottom != lprcClip->bottom ) )
		{
			lstrcat( lpd, " New." );
		}
		else
		{
			lstrcat( lpd, " Same!" );
		}
		AddCrLf(lpd);
	}
}

LPSTR	GetFrm( LPRECT lprcClip )
{
	LPSTR	lpf;
	if( ( lprcClip->left == lprcClip->right  ) &&
		( lprcClip->top  == lprcClip->bottom ) )
	{
		lpf = "Start(%d,%d) for (0,0)";
	}
	else
	{
		lpf = "From(%d,%d) for (%d,%d)";
	}
	return lpf;
}

int	AddStrt( LPSTR lpd, LPRECT lprcClip, LPPOINT lpptStart )
{
	int	i = 0;
		if( ( lpptStart ) &&
			( ( lpptStart->x != lprcClip->left ) ||
			  ( lpptStart->y != lprcClip->top  ) ) )
		{
			wsprintf( EndBuf(lpd),
				" Start=(%d,%d)",
				lpptStart->x,
				lpptStart->y );
			i++;
		}
	return i;
}
// REMOVE ANY OLD CLIP
//	DrawSelect( hDC, &lpDIBInfo->rcClip );
// #ifdef	SHOWCLIP
void	ShowRemove( LPRECT lprcClip )
{
	if( lprcClip )
	{
		LPSTR	lpf;
		LPSTR	lpd = GetTmp2();
		lstrcpy( lpd, "Clip Remove: " );
		lpf = GetFrm( lprcClip );
		AddSzs( lpd, lpf, lprcClip );
//		AddSN( lpd, lprcClip );
		AddCrLf(lpd);
		DO(lpd);
	}
}

//#ifdef	SHOWCLIP
void	ShowBegin( LPRECT lprcClip,
				  LPPOINT lpptOrigin, LPPOINT lpptStart,
				  LPRECT lprcClient, int cxDIB, int cyDIB )
{
	if( lprcClip )
	{
		LPSTR	lpf;
		LPSTR	lpd = GetTmp3();
		lstrcpy( lpd, "Clip Begin: " );
		lpf = GetFrm( lprcClip );
		AddSzs( lpd, lpf, lprcClip );
		if( ( 0 == AddStrt( lpd, lprcClip, lpptStart ) ) &&
			( lpptOrigin ) )
		{
			wsprintf( EndBuf(lpd),
				" Orig=(%d,%d)",
				lpptOrigin->x,
				lpptOrigin->y );
		}
		AddCrLf(lpd);
//		AddSN( lpd, lprcClip );
		DO(lpd);
	}
}

void	ShowFinal( LPRECT lprcClip,
				  LPPOINT lpptOrigin, LPPOINT lpptStart,
				  LPRECT lprcClient, int cxDIB, int cyDIB )
{
	if( lprcClip )
	{
		LPSTR	lpf;
		LPSTR	lpd = GetTmp1();
		lstrcpy( lpd, "Clip Final: " );
		lpf = GetFrm( lprcClip );
		AddSzs( lpd, lpf, lprcClip );
		AddStrt( lpd, lprcClip, lpptStart );
		AddCrLf(lpd);
//		AddSN( lpd, lprcClip );
		DO(lpd);

	}
}
//#endif	// SHOWCLIP

// #endif	// SHOWCLIP

void	DiagDrawSel( LPRECT lprcClip, LPPOINT lpptStart )
{
	if( lprcClip )
	{
		LPSTR	lpf;
		LPSTR	lpd = GetTmp2();
		lstrcpy( lpd, "Clip Draw: " );
		lpf = GetFrm( lprcClip );
		AddSzs( lpd, lpf, lprcClip );
		AddStrt( lpd, lprcClip, lpptStart );
		AddCrLf(lpd);
//		AddSN( lpd, lprcClip );
		DO(lpd);
	}

}

#endif	// DIAGSCROLL



//---------------------------------------------------------------------
//
// Function:   GetCurrentClipRect
//
// Purpose:    Return the rectangular dimensions of the clipboard
//             rectangle for the specified window.
//
// Parms:      hWnd == Window to retrieve the clipboard rectangle info
//                     from.
//
// History:   Date      Reason
//             6/1/91   Created
//             
//---------------------------------------------------------------------

RECT GetCurrentClipRect (HWND hWnd)
{
	RECT      rect = {0,0,0,0};
	HANDLE    hDIBInfo;
	LPDIBINFO lpDIBInfo;
	
	if( hWnd )
	{
		if( hDIBInfo = GetWindowExtra(hWnd, WW_DIB_HINFO) )
		{
			if( lpDIBInfo = (LPDIBINFO) DVGlobalLock (hDIBInfo) )
			{
				rect      = lpDIBInfo->rcClip;
				DVGlobalUnlock( hDIBInfo );
			}
		}
	}
	return( rect );
}



//---------------------------------------------------------------------
//
// Function:   GetCurrentDIBSize
//
// Purpose:    Return the dimensions of the DIB for the given MDI child
//             window.  Dimensions are returned as a POINT.
//
//             If the DIB is stretched on the screen, return the client
//             area of the window.  Otherwise, retrieve the info from
//             the DIBINFO structure stored in the window's words.
//
// Parms:      hWnd == Window to retrieve the DIB dimensions from.
//
// History:   Date      Reason
//             6/1/91   Created
//             
//---------------------------------------------------------------------

POINT GetCurrentDIBSize( HWND hWnd )
{
   HANDLE    hDIBInfo;
   LPDIBINFO lpDIBInfo;
   POINT     pt = {0,0};
   RECT      rcClient;

	if( hWnd )
	{
		if( hDIBInfo = GetWindowExtra( hWnd, WW_DIB_HINFO ) )
		{
			if( lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo ) )
			{
	         if( lpDIBInfo->Options.bStretch2W )
				{
					GetClientRect (hWnd, &rcClient);
					pt.x = rcClient.right;
					pt.y = rcClient.bottom;
				}
				else
				{
					pt.x = lpDIBInfo->di_dwDIBWidth;
					pt.y = lpDIBInfo->di_dwDIBHeight;
				}
				DVGlobalUnlock( hDIBInfo );
			}
			else
			{
				DIBError( ERR_LOCK );
			}
		}
	}
	return( pt );
}

#ifdef	ADDTIMER1

// ANIMATE GIF - Animating GIF Images
#ifdef ADD_JPEG_SUPPORT // This also seems to include GIF support
//////////////////////////////////////////////////////////////////'

extern	BOOL LIBGifNBmp( HGLOBAL, DWORD, HGLOBAL, HGLOBAL, WORD );

void	Do_GIF_Animate( HWND hWnd, LPDIBINFO lpDIBInfo )
{
	DWORD		Nxt;
	HANDLE		hDIB, hDIB2;
	HPALETTE	hPal;
	DWORD		ddSz1, ddSz2;
	HGLOBAL		hBig;
	HBITMAP		hBM;
	DWORD		mCnt;
	BOOL		fOneInc;

	if( ( mCnt = lpDIBInfo->Options.dwMilSecs ) &&
		 ( lpDIBInfo->dwMilCnt >= mCnt         ) )
	{
		fOneInc = FALSE;
		while( lpDIBInfo->dwMilCnt >= mCnt )
		{
			lpDIBInfo->dwMilCnt -= mCnt;
			if( !fOneInc )
			{
				fOneInc = TRUE;
				lpDIBInfo->wCurCnt++;	// Bump to NEXT
				Nxt = lpDIBInfo->wCurCnt;	// Set NEXT GIF Image
				//if( Nxt >= lpDIBInfo->wMaxCnt )
				if( Nxt > lpDIBInfo->wMaxCnt )
				{	// If too big
					lpDIBInfo->wCurCnt = 1;	// Restart at image 1
					Nxt = 1;
				}
			}
		}	// while away the timer

		hPal = lpDIBInfo->hPal;		// hPal MAY be ZERO!!!
		if( ( hBig = lpDIBInfo->hBigFile ) &&
			 ( hDIB = lpDIBInfo->hDIB     ) &&
			 ( hBM  = lpDIBInfo->hBitmap  ) )
		{
			ddSz1 = GlobalSize( hDIB );
			ddSz2 = GlobalSize( hBig );
			if( hDIB2 = DVGlobalAlloc( GMEM_SHARE, ddSz1 ) )
			{
				if( LIBGifNBmp( hBig, ddSz2, NULL, hDIB2, (WORD)Nxt ) )
				{
               // FAILED to get NEXT
					DVGlobalFree( hDIB2 );
				}
				else
				{
					// Kill the OLD image(s)
					DVGlobalFree( hDIB );
					lpDIBInfo->hDIB = 0;

					RelChildMem( lpDIBInfo );
//					if( lpDIBInfo->di_hDIB2 )
//						DVGlobalFree( lpDIBInfo->di_hDIB2 );
//					lpDIBInfo->di_hDIB2 = 0;

					if( lpDIBInfo->di_hBitmap2 )
						DeleteObject( lpDIBInfo->di_hBitmap2 );
					lpDIBInfo->di_hBitmap2 = 0;

					lpDIBInfo->hDIB = hDIB2;
					DeleteObject( hBM );

					lpDIBInfo->hBitmap = DIBToBitmap( hDIB2, hPal, lpDIBInfo );

               // ANIMATE GIF - new BITMAP loaded - Invalidate to PAINT next
					InvalidateRect( hWnd, NULL, FALSE );
               // ==========================================================

				}
			}
		}
	}	// If reached the time ... 
}


// NOTE: ghMaxCount member can not be larger than 0x7fff!
// That's 32,767 images - A reasonable MAXIMUM!!!
// ======================================================
//typedef struct tagGIFIMG {
//	WORD	giWidth;
//	WORD	giHeight;
//	WORD	giCMSize;
//	DWORD	giBMPSize;
//}GIFIMG;
//typedef GIFIMG MLPTR LPGIFIMG;
//
//typedef struct tagGIFHDR {
//	WORD	ghMaxCount;
//	WORD	ghImgCount;
//	WORD	ghWidth;
//	WORD	ghHeight;
//	WORD	ghCMSize;
//	DWORD	ghBMPSize;
//	GIFIMG ghGifImg[1];
//}GIFHDR;
//typedef GIFHDR MLPTR LPGIFHDR;

/* Multiple and Transparent GIF Image Support */
/* ========================================== */
//#define		gie_Flag		0x8000	// This is in the ghMaxCount
// if the application expect an EXTENDED description!

//typedef	struct	tagGIFIMGEXT {
//	GIFIMG	giGI;	// Width/Height/ColSize and BMP Size as above
// Image Descriptor - Wdith and Height in above
//	WORD	giLeft;		// Left (logical) column of image
//	WORD	giTop;		// Top (logical) row
//	BYTE	giBits;		// See below (packed field)
// Graphic Control Extension
//	BYTE	gceExt;		// Extension into - Value = 0x21
//	BYTE	gceLabel;	// Graphic Control Extension = 0xf9
//	DWORD	gceSize;	// Block Size (0x04 for GCE, Big for TEXT)
//	BYTE	gceBits;	// See below (packed field)
//	WORD	gceDelay;	// 1/100 secs to wait
//	BYTE	gceIndex;	// Transparency Index (if Bit set)
//	DWORD	gceColr;	// COLORREF (if SET)
//	DWORD	gceFlag;	// IN/OUT Options Flag - See Below
//	RGBQUAD	gceBkGrnd;	// Background Colour
//	HANDLE	hDIB;		// Handle to the DIB
//	HPALETTE hPal;		// Handle to the bitmap's palette.
//	HBITMAP	hBitmap;	// Handle to the DDB.
//	DWORD	gceRes1;	// Reserved
//	DWORD	gceRes2;	// ditto
//}GIFIMGEXT;
//typedef GIFIMGEXT MLPTR LPGIFIMGEXT;

//typedef struct tagGIFHDREXT {
//	WORD	gheMaxCount;	// gie_Flag + MAX. Count
//	WORD	gheImgCount;	// Images in GIF
//	WORD	gheWidth;		// Logical Screen Width
//	WORD	gheHeight;		// Logical Screen Height
//	WORD	gheCMSize;		// BMP Colour map size (byte count)
//	DWORD	gheBMPSize;		// Estimated final BMP size
//	BYTE	gheBits;		// See below (packed field)
//	BYTE	gheIndex;		// Background Colour Index
//	BYTE	ghePAR;			// Pixel Aspect Ration
//	DWORD	gheFlag;		// IN/OUT Options Flag - See Below
//	RGBQUAD	gheBkGrnd;		// Background Colour
//	HANDLE	hDIB;			// Handle to the DIB
//	HPALETTE hPal;			// Handle to the bitmap's palette.
//	HBITMAP	hBitmap;		// Handle to the DDB.
//	DWORD	gheRes1;		// Reserved
//	DWORD	gheRes2;		// ditto
//	GIFIMGEXT	gheGIE[1];	// 1 for Each Image follows
//}GIFHDREXT;
//typedef GIFHDREXT MLPTR LPGIFHDREXT;

// GIFHDREXT.gheBits
//#define		ghe_ColrMap		0x80	// A Global Color Map
//#define		ghe_ColrRes		0x70	// Colour Resolution
//#define		ghe_Sort		0x08	// Sorted Colour Map
//#define		ghe_ColrSize	0x07	// Size of Colour Table ((n+1)^2)

// GIFIMGEXT gceBits = GIF Graphic Control Extension Bits
// =================
//#define		gce_Reserved	0xe0	// Reserved 3 Bits
//#define		gce_Disposal	0x1c	// Disposal Method 3 Bits
//#define		gce_UserInput	0x02	// User Input Flag 1 Bit
//#define		gce_TransColr	0x01	// Transparent Color Flag 1 Bit

// GIFIMGEXT.giBits
//#define		gie_ColrLoc		0x80	// Local Colour Table
//#define		gie_Interlace	0x40	// Interlaced Scan lines
//#define		gie_SortFlag	0x20	// Sorted Color Table3
//#define		gie_Reserved	0x18	// 2 reserved bits
//#define		gie_ColrSize	0x07	// Colr Table Size ((n+1)^2)

//	WORD	wMaxCnt;	// Maximum number of images (in GIF)
//	WORD	wCurCnt;	// Current image 0 to < MaxCnt
//	DWORD	dwMilCnt;	// Count to next movement/action
//	WORD	wBgnCnt;	// Beginning Anim. GIF ... NOT USED
//	WORD	wGifNum;	// The number of this GIF ...
//	WORD	wGifTot;	// The Total of this GIF set ...
//	DWORD	dwGIFNext;	// ms to NEXT GIF action / or USER!!!
//     +---------------+
//  1  |               | 0 Text Grid Left Position Unsigned
//     +-             -+
//  2  |               | 1
//     +---------------+
//  3  |               | 2 Text Grid Top Position        Unsigned
//     +-             -+
//  4  |               | 3
//     +---------------+
//  5  |               | 4 Text Grid Width               Unsigned
//     +-             -+
//  6  |               | 5
//     +---------------+
//  7  |               | 6 Text Grid Height              Unsigned
//     +-             -+
//  8  |               | 7
//     +---------------+
//  9  |               | 8 Character Cell Width          Byte
//     +---------------+
// 10  |               | 9 Character Cell Height         Byte
//     +---------------+
// 11  |               | 10 Text Foreground Color Index   Byte
//     +---------------+
// 12  |               | 11 Text Background Color Index   Byte
//     +---------------+
//			lpGIE->gceFlag |=	gie_PTE;	// Plain Text Extension
//			lpGIE->giLeft = LM_to_uint(lpb[0],lpb[1]);// Left (logical) column of TEXT
//			lpGIE->giTop  = LM_to_uint(lpb[2],lpb[3]);// Top (logical) row
//			lpGIE->giGI.giWidth = LM_to_uint(lpb[4],lpb[5]);
//			lpGIE->giGI.giHeight = LM_to_uint(lpb[6],lpb[7]);
			// Char CELL
//			lpGIE->gceRes1 = (DWORD)LM_to_uint(lpb[8],lpb[9]);
//			lpGIE->gceRes2 = (DWORD)lpb[10];	// Foreground index
//			lpGIE->gceIndex = lpb[11];	// Backgound INDEX
//	DWORD	gceColr;	// COLORREF (if SET)
void	Do_GIF_Text( HWND hWnd, HGLOBAL hDIB, LPGIFIMGEXT lpGIE )
{
	LOGFONT	cursfont;
	HANDLE	hnewfont;
	LPSTR	lps, lpb;
	LPPTEHDR	lppte;
	DWORD	dwCnt;
	BYTE	b, cnt;
	DWORD	tot;
	BOOL	flg;
	char	c;

	flg = FALSE;
	lps = 0;
	hnewfont = 0;
	if( ( hDIB && lpGIE            ) &&	// Ensure PASSED handle & pointer
		 ( lps = DVGlobalLock(hDIB) ) )	// and can LOCK buffer // LOCK DIB HANDLE
	{
		lppte = (LPPTEHDR)lps;	// Cast as DWORD pointer
		lps += sizeof(PTEHDR);		// Bump past 2 DWORDS
		dwCnt = lppte->pt_Total;	// Get total count
		tot = 0;
		if( ( lppte->pt_Missed == 0 ) &&		// That has to be ZERO
			( cnt = *lps ) &&		// and there has to be count
			( cnt == 0x0c ) &&		// and the first count = 0x0c
			( dwCnt > (DWORD)cnt ) ) // and larger total count
		{
			lps += cnt + 1;	// Bumpt past "Plain Text Extension" 
			lpb = lps;	// Make this the start of TEXT Buffer
			dwCnt -= (cnt + 1);	// and reduce overall total
			while( dwCnt && (cnt = *lps) )
			{	// while there is count
				// move the TEXT to the start of the buffer
				lps++;
				dwCnt--;
				if( dwCnt )
				{	// Always while thre remains total
					for( b = 0; b < cnt; b++ )
					{
						c = *lps++;
						//if( c >= ' ' )
						if( c )
						{	// Move ALL chars
							lpb[tot++] = c;
						}
						dwCnt--;
						if( dwCnt == 0 )
							break;
					}
				}
			}
			lpb[tot] = 0;	
			if( tot && dwCnt )
			{
				flg = TRUE;	// SUCCESSFUL move of TEXT
			}
		}
	}
	else
	{
		return;
	}

	// Build fixed screen font.
	//if( flg &&
	//	( cursfont.lfHeight = LOBYTE(LOWORD(lpGIE->gceRes1)) ) &&
	//	( cursfont.lfWidth  = HIBYTE(LOWORD(lpGIE->gceRes1)) ) )
	//if( flg &&
	//	( cursfont.lfHeight = LOBYTE(LOWORD(lpGIE->gceRes1)) ) )
	if( flg &&
		( cursfont.lfHeight = HIBYTE(LOWORD(lpGIE->gceRes1)) ) &&
		( cursfont.lfWidth  = LOBYTE(LOWORD(lpGIE->gceRes1)) ) )
	{
	//	cursfont.lfWidth  = cursfont.lfHeight;
		cursfont.lfEscapement     =  0;
		cursfont.lfOrientation    =  0;
		cursfont.lfWeight         =  FW_NORMAL;
		cursfont.lfItalic         =  FALSE;
		cursfont.lfUnderline      =  FALSE;
		cursfont.lfStrikeOut      =  FALSE;
		cursfont.lfCharSet        =  ANSI_CHARSET;
		cursfont.lfOutPrecision   =  OUT_DEFAULT_PRECIS;
		cursfont.lfClipPrecision  =  CLIP_DEFAULT_PRECIS;
		cursfont.lfQuality        =  DEFAULT_QUALITY;
		cursfont.lfPitchAndFamily =  FIXED_PITCH | FF_DONTCARE;
		//cursfont.lfPitchAndFamily =  DEFAULT_PITCH | FF_DONTCARE;
		//strcpy(cursfont.lfFaceName, "System");
		cursfont.lfFaceName[0] = 0;

		hnewfont = CreateFontIndirect( &cursfont );
	}
	if( lps )
	{
		if( flg && hnewfont )
		{
			lppte->pt_Missed = tot;
			lpGIE->hFont = hnewfont;
			//DeleteObject( hnewfont );
		}
		else if( hnewfont )
		{
			DeleteObject( hnewfont );
		}

		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE
	}
}

void	Do_GIF_Group( HWND hWnd, LPDIBINFO lpDIBInfo )
{
	HGLOBAL	hgInfo;
	DWORD	mCnt;
	HANDLE	hDIB;
	LPGIFHDREXT	lpGHE;
	LPGIFIMGEXT	lpGIE;
	mCnt = lpDIBInfo->dwGIFNext;
	if( lpDIBInfo->dwMilCnt >= mCnt )
	{	// Next TIMER ended
		lpDIBInfo->dwMilCnt = 0;	// Start counter again
		if( hgInfo = lpDIBInfo->hgGIFSize )
		{
			if( lpGHE = (LPGIFHDREXT) DVGlobalLock( hgInfo ) )
			{
				lpGIE = &lpGHE->gheGIE[lpDIBInfo->wCurCnt];
Do_Nxt_Img:
				if( hDIB = lpGIE->hDIB )
				{
					if( lpGIE->gceFlag & gie_PTE )	// Is PLAIN TEXT EXTENT
					{
						Do_GIF_Text( hWnd, hDIB, lpGIE );
					}
					else
					{
						if( lpGIE->hPal == 0 )
						{
							if( lpGIE->hPal = CreateDIBPalette( hDIB ) )
							{
								if( lpGIE->hBitmap == 0 )
								{
									if( lpGIE->hBitmap = DIBToBitmap( hDIB, lpGIE->hPal, lpDIBInfo ) )
									{
										InvalidateRect( hWnd, NULL, FALSE );
									}
								} // No Bitmap
							}
						}	// No HPal
					}
				}
				lpDIBInfo->wCurCnt++;
				if( lpDIBInfo->wCurCnt >= lpDIBInfo->wMaxCnt )
				{
					DeleteAnim( hWnd );
				}
				else
				{
					lpGIE = &lpGHE->gheGIE[lpDIBInfo->wCurCnt];
					lpDIBInfo->dwGIFNext = lpGIE->gceDelay * iGifDelay;
					if( lpDIBInfo->dwGIFNext == 0 )
						goto Do_Nxt_Img;
				}
				DVGlobalUnlock( hgInfo );
			}
		}
	}
}

void	NetAnim( HWND hWnd )
{
	HANDLE		hDIBInfo;
	LPDIBINFO	lpDIBInfo;
	HWND		   aHwnd[MXANIMS+1];
	DWORD		   aCnt, i;

	if( aCnt = GetAnims( &aHwnd[0], MXANIMS ) )
	{
		for( i = 0; i < aCnt; i++ )
		{
			if( aHwnd[i] &&
				( hDIBInfo = GetWindowExtra( aHwnd[i], WW_DIB_HINFO) ) &&
				( lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo ) ) )
			{
				lpDIBInfo->dwMilCnt += TIMER_INTERVAL1;  // # of ms between timer ticks
				if( lpDIBInfo->dwDIBFlag & df_GIFANIM )
				{
					Do_GIF_Animate( aHwnd[i], lpDIBInfo );
				}
				else if( lpDIBInfo->dwDIBFlag & df_GIFGRPC )
				{
					Do_GIF_Group( aHwnd[i], lpDIBInfo );
				}
				DVGlobalUnlock( hDIBInfo );
			}
		}	// For list ...
	}
}
#endif // #ifdef ADD_JPEG_SUPPORT // This also seems to include GIF support
//////////////////////////////////////////////////////////////////'

#endif	/* ADDTIMER1 */

void PalAnim( HWND hWnd )
{
	HANDLE    hDIBInfo;
	LPDIBINFO lpDIBInfo;
	if( hDIBInfo = GetWindowExtra(hWnd, WW_DIB_HINFO) )
	{
		if( lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo ) )
		{
			MyAnimatePalette( hWnd, lpDIBInfo->hPal );
			DVGlobalUnlock( hDIBInfo );
		}
	}
}

#ifdef	ADDCOLRF

//
//
//     FUNCTION: Draw3DRect
//
//     PURPOSE:  draws a rectangle in 3d colors
//
//     PARAMS:   HDC hDC      - The DC on which to draw
//               RECT Rect    - The rectangle itself
//               BOOL bSunken - TRUE  = rect should look sunken
//                              FALSE = rect should look raised
//
//     RETURNS:  BOOL - TRUE for success, FALSE for failure
//
// History:
//		April, 97	Added to DibView
//
BOOL Draw3DRect( HDC hDC, RECT Rect, BOOL bSunken, LPDIBX lpDIBx )
{
	BOOL	rflg;
    HBRUSH	hBrush;
    HPEN    hPen, hOldPen;

	rflg = FALSE;
    // Get the color for the main foreground
	if( lpDIBx &&
		lpDIBx->dx_Valid )
	{
		if( hBrush = CreateSolidBrush( lpDIBx->dx_Face ) )
		{
			// paint it
			FillRect( hDC, &Rect, hBrush );
			DeleteObject( hBrush );
			rflg = TRUE;
		}
		else
		{
			return rflg;
		}

		// Get the pen for the top and left sides
		if( bSunken )
			hPen = CreatePen( PS_SOLID, 1, lpDIBx->dx_Shadow );
		else
			hPen = CreatePen( PS_SOLID, 1, lpDIBx->dx_HiLite );

		if( hPen )
		{
			hOldPen = SelectObject( hDC, hPen);
			// Draw the top and left sides
			MoveToEx( hDC, Rect.right, Rect.top, NULL );
			LineTo( hDC, Rect.left, Rect.top );
			LineTo( hDC, Rect.left, Rect.bottom );
			SelectObject( hDC, hOldPen);
			DeleteObject( hPen );
		}

		// Get the pen for the bottom and right sides
		if( bSunken )
			hPen = CreatePen( PS_SOLID, 1, lpDIBx->dx_HiLite );
		else
			hPen = CreatePen( PS_SOLID, 1, lpDIBx->dx_Shadow );

		if( hPen )
		{
			hOldPen = SelectObject( hDC, hPen);
			// Draw the bottom and right sides
			LineTo( hDC, Rect.right, Rect.bottom );
			LineTo( hDC, Rect.right, Rect.top );
			SelectObject( hDC, hOldPen);
			DeleteObject( hPen );
		}
	}
	else
	{
		if( hBrush = CreateSolidBrush( GetSysColor(COLOR_3DFACE) ) )
		{
			// paint it
			FillRect( hDC, &Rect, hBrush );
			DeleteObject( hBrush );
			rflg = TRUE;
		}
		else
		{
			return rflg;
		}

		// Get the pen for the top and left sides
		if( bSunken )
			hPen = CreatePen( PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW) );
		else
			hPen = CreatePen( PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT) );

		if( hPen )
		{
			hOldPen = SelectObject( hDC, hPen);
			// Draw the top and left sides
			MoveToEx( hDC, Rect.right, Rect.top, NULL );
			LineTo( hDC, Rect.left, Rect.top );
			LineTo( hDC, Rect.left, Rect.bottom );
			SelectObject( hDC, hOldPen);
			DeleteObject( hPen );
		}

		// Get the pen for the bottom and right sides
		if( bSunken )
			hPen = CreatePen( PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT) );
		else
			hPen = CreatePen( PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW) );

		if( hPen )
		{
			hOldPen = SelectObject( hDC, hPen);
			// Draw the bottom and right sides
			LineTo( hDC, Rect.right, Rect.bottom );
			LineTo( hDC, Rect.right, Rect.top );
			SelectObject( hDC, hOldPen);
			DeleteObject( hPen );
		}
	}

	return rflg;

}	// End Draw3DRect()


//
//
//     FUNCTION: ChildWndEraseBkGnd
//
//     PURPOSE:  Draws the background for the MDI child
//
//     PARAMS:   HWND hWnd - The MDI window of interest
//               HDC  hDC  - The DC on which to draw
//
//     RETURNS:  void
//
// History:
//		April, 97	Added to DibView
//		Like long FrameEraseBkGnd( HWND hWnd, HDC hDC )
//
BOOL ChildWndEraseBkGnd( HWND hWnd, HDC hDC )
{
	BOOL	rFlg;
	RECT	Rect;
	HGLOBAL	hDIBx;
	LPDIBX	lpcwd;
//	HBRUSH	hBrush;
//	SIZE	TextSize;

	rFlg = FALSE;
	// Just how big is this window?
	GetClientRect( hWnd, &Rect );

	if( ( hDIBx = (HGLOBAL) GetWindowExtra( hWnd, WW_DIB_EX2 ) ) &&
		( lpcwd = (LPDIBX) DVGlobalLock( hDIBx ) ) )
	{
		// Paint the background
		rFlg = Draw3DRect( hDC, Rect, FALSE, lpcwd );
		DVGlobalUnlock( hDIBx );
	}
//	else
//	{
		// Paint the background
//		rFlg = Draw3DRect( hDC, Rect, FALSE, NULL );
//	}

    // If there is no icon resource yet, bail out
		// Draw 3d rectangles around areas of interest
		//Draw3DRect( hDC, lpcwd->WhiteRect, TRUE );
		//Draw3DRect( hDC, lpcwd->WhiteTextRect, TRUE );
		//Draw3DRect( hDC, lpcwd->BlackRect, TRUE );
		//Draw3DRect( hDC, lpcwd->BlackTextRect, TRUE );
		//Draw3DRect( hDC, lpcwd->XORRect, TRUE );
		//Draw3DRect( hDC, lpcwd->XORTextRect, TRUE );
		//Draw3DRect( hDC, lpcwd->ANDRect, TRUE );
		//Draw3DRect( hDC, lpcwd->ANDTextRect, TRUE );
		// Fill in the white area
		//hBrush = GetStockObject( WHITE_BRUSH );
		//SelectObject( hDC, hBrush );
		//Rectangle( hDC, lpcwd->WhiteRect.left, lpcwd->WhiteRect.top, lpcwd->WhiteRect.right, lpcwd->WhiteRect.bottom );
		// Fill in the black area
		//hBrush = GetStockObject( BLACK_BRUSH );
		//SelectObject( hDC, hBrush );
		//Rectangle( hDC, lpcwd->BlackRect.left, lpcwd->BlackRect.top, lpcwd->BlackRect.right, lpcwd->BlackRect.bottom );

		// Set texts for the various sections
		//SetBkMode( hDC, TRANSPARENT );
		//GetTextExtentPoint32( hDC, "Icon On Black", 13, &TextSize );
		//TextOut( hDC, lpcwd->BlackTextRect.left + ((RectWidth(lpcwd->BlackTextRect)-TextSize.cx)/2),
		//	lpcwd->BlackTextRect.top + ((RectHeight(lpcwd->BlackTextRect)-TextSize.cy)/2), "Icon On Black", 13 );
		//GetTextExtentPoint32( hDC, "Icon On White", 13, &TextSize );
		//TextOut( hDC, lpcwd->WhiteTextRect.left + ((RectWidth(lpcwd->WhiteTextRect)-TextSize.cx)/2),
		//	lpcwd->WhiteTextRect.top + ((RectHeight(lpcwd->WhiteTextRect)-TextSize.cy)/2), "Icon On White", 13 );
		//GetTextExtentPoint32( hDC, "XOR Mask", 8, &TextSize );
		//TextOut( hDC, lpcwd->XORTextRect.left + ((RectWidth(lpcwd->XORTextRect)-TextSize.cx)/2),
		//	lpcwd->XORTextRect.top + ((RectHeight(lpcwd->XORTextRect)-TextSize.cy)/2), "XOR Mask", 8 );
		//GetTextExtentPoint32( hDC, "AND Mask", 8, &TextSize );
		//TextOut( hDC, lpcwd->ANDTextRect.left + ((RectWidth(lpcwd->ANDTextRect)-TextSize.cx)/2),
		//	lpcwd->ANDTextRect.top + ((RectHeight(lpcwd->ANDTextRect)-TextSize.cy)/2), "AND Mask", 8 );

	return rFlg;

}	// End ChildWndEraseBkGnd()


#endif	// ADDCOLRF

//#ifdef	USECTHREAD
void	ChildWndThreadDone( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
	HANDLE		hDIBInfo, hNDI;
	LPDIBINFO	lpDIBInfo, lpNDI;

	if( hNDI = (HANDLE)lParam )
	{
		if( lpNDI = (LPDIBINFO)DVGlobalLock( hNDI ) )
		{
			if( hDIBInfo = GetWindowExtra( hWnd, WW_DIB_HINFO) )
			{
				if( lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo ) )
				{
					if( ( lpNDI->di_hDIB2 ) &&
						( lpNDI->di_hBitmap2 ) )
					{
						// *** SUCCESS ***
						lpDIBInfo->hBitmap = lpNDI->di_hBitmap2;
						lpDIBInfo->di_hDIB2 = lpNDI->di_hDIB2;
						lpDIBInfo->di_hBitmap2 = lpNDI->di_hBitmap2;
						lpDIBInfo->di_rcDib2 = lpNDI->di_rcDib2;
						// OK, force a new PAINT
						InvalidateRect( hWnd, NULL, FALSE );
					}
					else
					{
						if( lpNDI->di_hDIB2 )
							DVGlobalFree( lpNDI->di_hDIB2 );
						lpNDI->di_hDIB2 = 0;
						if( lpNDI->di_hBitmap2 )
							DeleteObject( lpNDI->di_hBitmap2 );
						lpNDI->di_hBitmap2 = 0;
					}

					lpDIBInfo->di_hThread = 0;
					lpDIBInfo->di_dwThreadId = 0;
					if( lpDIBInfo->di_dwThreadBit )
					{
						gdwThreadSig &= ~( lpDIBInfo->di_dwThreadBit );
					}
					lpDIBInfo->di_dwThreadBit = 0;
					DVGlobalUnlock( hDIBInfo );
				}
			}
			DVGlobalUnlock( hNDI );
		}
		DVGlobalFree( hNDI );
	}
}
//#endif	// USECTHREAD

// =================================================================
// long	ChildWndClose( HWND hWnd, UINT message,
//					  WPARAM wParam, LPARAM lParam )
//
// CHILD WM_CLOSE
// If an application processes this message, it should return zero. 
//
// Default Action
// The DefWindowProc function calls the DestroyWindow
//	function to destroy the window.
//
// =================================================================
long	ChildWndClose( HWND hWnd, UINT message,
					  WPARAM wParam, LPARAM lParam )
{
	long		lRet;
	HANDLE	hDIBInfo;
	PDI   	lpDIBInfo;

	lRet = 1;
#ifdef	DIAGSCROLL
	sprtf( "Child: WM_CLOSE to Handle 0x%x."MEOR, hWnd );
#endif	// DIAGSCROLL

	if( hDIBInfo = GetWindowExtra( hWnd, WW_DIB_HINFO) )
	{
		if( lpDIBInfo = (PDI)DVGlobalLock( hDIBInfo ) )
		{
         WaitThreadExit( lpDIBInfo );
			DVGlobalUnlock( hDIBInfo );
		}
	}

	// Until we handle this message fully,
	// pass on to default handler
	lRet = DefMDIChildProc( hWnd, message, wParam, lParam );

	return	lRet;
}

#ifdef   WRTCLRFILE
extern   DWORD  DVIsValidPalEx( PPX ppx );
//typedef struct tagCLRHDR {
//   char  ch_szHdr[8];
static   char  sszClrSig[] = "PALEX\0x1a";

VOID  SetClrHdr( PDI lpDIBInfo, PCLRHDR pch, DWORD dwSize )
{
   ZeroMemory( pch, sizeof(CLRHDR) );
   lstrcpy( &pch->ch_cSig[0], sszClrSig );
   pch->ch_dwWidth  = lpDIBInfo->di_dwDIBWidth;
   pch->ch_dwHeight = lpDIBInfo->di_dwDIBHeight;
   pch->ch_dwBPP    = lpDIBInfo->di_dwDIBBits;
   pch->ch_dwSize   = dwSize;
}

DWORD  IsClrHdr( PDI lpDIBInfo, PCLRHDR pch )
{
   DWORD dwRet = 0;
   if( ( lpDIBInfo && pch ) &&
       ( strcmp( &pch->ch_cSig[0], sszClrSig ) == 0 ) &&
       ( pch->ch_dwWidth  == lpDIBInfo->di_dwDIBWidth ) &&
       ( pch->ch_dwHeight == lpDIBInfo->di_dwDIBHeight ) &&
       ( pch->ch_dwBPP    == lpDIBInfo->di_dwDIBBits   ) )
   {
      dwRet = pch->ch_dwSize;
   }
   return dwRet;
}

DWORD IsClrFile( PDI lpDIBInfo )  // #ifdef   WRTCLRFILE
{
   DWORD    dwRet = 0;
   LPTSTR   lpt   = &lpDIBInfo->di_szTitle[0];
   HANDLE   h     = 0;

   GetCLRFile( lpt, lpDIBInfo );
   if( h = DVOpenFile2( lpt ) )
   {
      CLRHDR   ch;
      DWORD    dwr = 0;
      if( ( ReadFile(h, &ch, sizeof(CLRHDR), &dwr, NULL) ) &&
          ( dwr == sizeof(CLRHDR) ) &&
          ( dwRet = IsClrHdr( lpDIBInfo, &ch ) ) )
      {
         // success
         lpDIBInfo->di_hCLRFile  = h;
         lpDIBInfo->di_dwCLRSize = dwRet;
      }
      else
      {
         // failed
         lpDIBInfo->di_hCLRFile  = 0;
         lpDIBInfo->di_dwCLRSize = 0;
      }
   }
   return dwRet;
}

BOOL  DVWriteFile( PHANDLE ph, PBYTE lpb, DWORD dws ) // #ifdef   WRTCLRFILE
{
   BOOL     bRet = FALSE;
   HANDLE   h    = *ph;
   DWORD    dww  = 0;
   if( ( VFH(h) ) &&
       ( WriteFile(h,lpb,dws,&dww,NULL) ) &&
       ( dww == dws ) )
   {
      bRet = TRUE;
   }
   else if( VFH(h) )
   {
      CloseHandle(h);
      *ph = INVALID_HANDLE_VALUE;
   }
   return bRet;
}

BOOL  WriteClrFile( PDI lpDIBInfo, PPX ppx ) // #ifdef   WRTCLRFILE
{
   BOOL     bRet = FALSE;
   LPTSTR   lpt = &lpDIBInfo->di_szTitle[0];
   HANDLE   hf;
   DWORD    dwSize;
   if( dwSize = DVIsValidPalEx( ppx ) )
   {
      GetCLRFile( lpt, lpDIBInfo ); // get the ????.clr file name, and PATH
      if( hf = DVCreateFile(lpt) )
      {
         CLRHDR   ch;
         SetClrHdr( lpDIBInfo, &ch, dwSize );
         if( DVWriteFile( &hf, (PBYTE)&ch, sizeof(CLRHDR) ) )
         {
            if( DVWriteFile( &hf, (PBYTE)ppx, dwSize ) )
               bRet = TRUE;
         }
         if( VFH(hf) )
            CloseHandle(hf);
      }
   }
   return bRet;
}

#endif   // WRTCLRFILE

// handle 	case MYWM_THREADDONE2:
VOID  ChildWndCountDone( HWND hWnd )
{
	HANDLE	hDIBInfo;
	PDI   	lpDIBInfo;
	if( ( hDIBInfo = GetWindowExtra( hWnd, WW_DIB_HINFO) ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock( hDIBInfo )      ) )
   {
		LPPALEX	lppx;
      HGLOBAL  hg;
      DWORD    k = 0;
      if( ( hg = lpDIBInfo->di_hCOLR         ) &&
          ( lppx = (LPPALEX)DVGlobalLock(hg) ) )
      {
			k = lppx->px_Count;

#ifdef   WRTCLRFILE  // output a *persistent* colour (my PALETX header and array)

         if( !( lpDIBInfo->di_dwDnCount & fg_GotFile ) )   // when thread count completes
         {
            if( WriteClrFile( lpDIBInfo, lppx ) )
            {
               lpDIBInfo->di_dwDnCount |= fg_GotFile;
            }
         }

#endif   // WRTCLRFILE
			DVGlobalUnlock(hg);
		}	// got GetUCCnt( )
      if(k)
      {
         LPTSTR lpt = &lpDIBInfo->di_szTitle[0];
         LPTSTR lpf = &lpDIBInfo->di_szDibFile[0];
         wsprintf( lpt,
            pRDibTit,
            lpf,
            lpDIBInfo->di_dwDIBWidth,
            lpDIBInfo->di_dwDIBHeight,
            lpDIBInfo->di_dwDIBBits );
         wsprintf( EndBuf(lpt),
            " Colors=%u",
            k );
         //wOff = MoveFN( &lpDIBInfo->di_szDibFile[0], &szFName[0] );
         SetWindowText( hWnd, lpt );
         // #define  fg_DnMyWM      0x00000002  // received MYWM_THREADDONE2
         lpDIBInfo->di_dwDnCount |= fg_DnMyWM;   // when thread count completes
      }
      DVGlobalUnlock(hDIBInfo);
   }
}

BOOL Get_DIB_Info( HWND hWnd, HANDLE * ph, LPDIBINFO * pdi )
{
   HANDLE		hDIBInfo;
	LPDIBINFO	lpDIBInfo;
	hDIBInfo  = GetWindowExtra(hWnd, WW_DIB_HINFO);
	if( hDIBInfo == 0 ) {
      return FALSE;
	}
	lpDIBInfo = (LPDIBINFO) DVGlobalLock (hDIBInfo);
	if( lpDIBInfo == 0 ) {
      return FALSE;
	}
   *ph = hDIBInfo;
   *pdi = lpDIBInfo;
   return TRUE;
}

VOID Release_DIB_Info( HWND hWnd, HANDLE * ph, LPDIBINFO * pdi )
{
   HANDLE		hDIBInfo = *ph;
   if( hDIBInfo )
      DVGlobalUnlock( hDIBInfo );
}


// eof - DvChild.c
