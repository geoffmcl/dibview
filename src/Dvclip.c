
/* ================================================================

      File:  DVCLIP.C

   Purpose:  Contains routines related to cutting/pasting bitmaps to/from
             the clipboard.

 Functions:  CopyHandle
             CropBitmap
             RenderFormat
             HandleCopyClipboard
             HandlePasteClipboard

  Comments:  

   History:   Date      Reason
             6/ 1/91     Created

=================================================================== */
#include	"dv.h"		// Single inclusive include
//#include <windows.h>
//#include "DvChild.h"
//#include "DvClip.h"
//#include "DvDib.h"
//#include "DvErrors.h"
//#include "DvFrame.h"
//#include "DvPal.h"
//#include "DvMem.h"
#ifdef ADD_CLIP_RESIZING // FIX20051128 - add CLIP resizing
#include <windowsx.h>
#endif // #ifdef ADD_CLIP_RESIZING

#ifdef   ADD_STATUS_BAR
#include <commctrl.h>
extern HWND ghStatusWnd;
#endif // #ifdef   ADD_STATUS_BAR

extern void TrackMouse( HWND hWnd, LPRECT lpClipRect, int cxDIB, int cyDIB);
extern void TrackMouse2 (HWND hWnd, LPRECT lpClipRect, int cxDIB, int cyDIB,
                  LPDIBINFO lpDIBInfo );

#define  dbg_track   sprtf

// NEW
extern   PMWL	AddToFileList4( PRDIB prd );
extern   BOOL  WriteBMPFile2( LPTSTR lpf, HGLOBAL hDIB, BOOL bShow );
extern DWORD GetTextExtent( HDC hdc, LPSTR lpS, int len );
extern VOID  Add2ClipList( PRECT prc );
extern VOID  Remove_and_Add2ClipList( PRECT prc, PRECT rem );

//RECT  grcClip     = {0,0,0,0};    // Clipboard rectangle at time of copy.
//DWORD		gnPasteNum = 1;      // For window title
TCHAR gszRPFrm[] = DEF_FM_STG;   // "TEMPB%03d.BMP" - when NO FILE, use this

RECT	   rcPrevClip = { 0 };

PTSTR GetClipPosStg( DV_CURPOS cp )
{
   PTSTR ps = "";
   switch( cp )
   {
   case DV_ON_LEFT:   ps = "ON LEFT"; break;
   case DV_ON_RIGHT:  ps = "ON RIGHT"; break;
   case DV_ON_TOP:    ps = "ON TOP"; break;
   case DV_ON_BOTTOM: ps = "ON BOTTOM"; break;
   case DV_ON_TOPLEFT: ps = "TOPLEFT"; break;
   case DV_ON_BOTTRIGHT: ps = "BOTTRIGHT"; break;
   case DV_ON_TOPRIGHT: ps = "TOPRIGHT"; break;
   case DV_ON_BOTTLEFT: ps = "BOTTLEFT"; break;
   case DV_ON_ALL:    ps = "IN CLIP"; break;
   default: ps = "OUTSIDE"; break;
   }
   return ps;
}

VOID StartMouseText( PTSTR pd, HWND hWnd, INT xPos, INT yPos )
{
   HDC hdc = GetDC( hWnd );
   if( hdc ) {
      // FIX20081019 - add RGB color to mouse position
      COLORREF cr = GetPixel( hdc, xPos, yPos );
      ReleaseDC( hWnd, hdc );
      sprintf( pd, "Mouse x,y = %d,%d - (%d,%d,%d)",
         xPos, yPos,
         GetRValue(cr), GetGValue(cr), GetBValue(cr) );
   } else {
      sprintf( pd, "Mouse x,y = %d,%d", xPos, yPos );
   }
}

VOID SetMouseText( PTSTR pd, HWND hWnd, INT xPos, INT yPos, DV_CURPOS cp, PRECT prcClip )
{
   StartMouseText( pd, hWnd, xPos, yPos );
   sprintf( EndBuf(pd), " - %s %dx%d",
         GetClipPosStg(cp),
         (prcClip->right - prcClip->left),
         (prcClip->bottom - prcClip->top) );
}

VOID SetMouseMessage( HWND hWnd, INT xPos, INT yPos, DV_CURPOS cp, PRECT prcClip )
{
#ifdef   ADD_STATUS_BAR
   if( ghStatusWnd ) {
      PTSTR pd = GetStgBuf();
      SetMouseText( pd, hWnd, xPos, yPos, cp, prcClip );
      SendMessage( ghStatusWnd, SB_SETTEXT, 0, (LPARAM) pd);
   }
#endif // #ifdef   ADD_STATUS_BAR
}

#ifdef   ADD_STATUS_BAR
#define  SSM(a)   if(ghStatusWnd) SendMessage( ghStatusWnd, SB_SETTEXT, 0, (LPARAM) a)
#else
#define  SSM(a)
#endif

VOID ChildWndMouseMove( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
   LPDIBINFO   lpDIBInfo;
	HANDLE      hDIBInfo;   // = GetWindowExtra( hWnd, WW_DIB_HINFO );
   //PTSTR       pd; // = GetTmp3();
   INT         xPos; // = GET_X_LPARAM(lParam);
   INT         yPos; // = GET_Y_LPARAM(lParam);
   RECT        rcc;

   xPos = GET_X_LPARAM(lParam);
   yPos = GET_Y_LPARAM(lParam);
   //pd = GetTmp3();

   //*pd = 0;
#ifdef   ADD_STATUS_BAR
   //if( ghStatusWnd ) {
   //   StartMouseText( pd, hWnd, xPos, yPos );
   //}
#endif // #ifdef   ADD_STATUS_BAR

   if( !Get_DIB_Info( hWnd, &hDIBInfo, &lpDIBInfo ) )
      return;

   rcc = lpDIBInfo->rcClip; // Get the current CLIP region
   if( IsRectEmpty( &lpDIBInfo->rcClip ) ) {
      // sprintf(pd, "WM_MOUSEMOVE: Position x,y %d,%d NO rcClip!"MEOR, xPos, yPos);
	   // DO(pd);
      //strcat(pd, " Empty CLIP!");
      //SSM(pd);
      //DVGlobalUnlock( hDIBInfo );	// Unlock pointer
      Clippos = DV_ON_NONE;
      SetCursorType( Clippos );
      SetMouseMessage( hWnd, xPos, yPos, Clippos, &rcc );
      Release_DIB_Info( hWnd, &hDIBInfo, &lpDIBInfo );
      return;
   }

   if((( rcc.left      == xPos ) || ( rcc.left + 1   == xPos ) || ( rcc.left - 1  == xPos )) &&
      (( rcc.top       == yPos ) || ( rcc.top  + 1   == yPos ) || ( rcc.top  - 1  == yPos )) ) {
         // of TOP LEFT CORNER
         Clippos = DV_ON_TOPLEFT;
   } else if((( rcc.left      == xPos ) || ( rcc.left   + 1 == xPos ) || ( rcc.left  - 1  == xPos )) &&
      (( rcc.bottom    == yPos ) || ( rcc.bottom + 1 == yPos ) || ( rcc.bottom - 1 == yPos )) ) {
         // of BOTTOM LEFT CORNER
         Clippos = DV_ON_BOTTLEFT;
   } else if((( rcc.right    == xPos ) || ( rcc.right  + 1 == xPos ) || ( rcc.right  - 1 == xPos )) &&
      (( rcc.bottom    == yPos ) || ( rcc.bottom + 1 == yPos ) || ( rcc.bottom - 1 == yPos )) ) {
         // of BOTTOM RIGHT CORNER
         Clippos = DV_ON_BOTTRIGHT;
   } else if((( rcc.right  == xPos ) || ( rcc.right + 1 == xPos ) || ( rcc.right - 1  == xPos )) &&
      (( rcc.top       == yPos ) || ( rcc.top  + 1   == yPos ) || ( rcc.top  - 1  == yPos )) ) {
         // of TOP RIGHT CORNER
         Clippos = DV_ON_TOPRIGHT;
   } else if((( rcc.left     == xPos ) || ( rcc.right      == xPos )||
       ( rcc.left + 1 == xPos ) || ( rcc.right + 1  == xPos )||
       ( rcc.left - 1 == xPos ) || ( rcc.right - 1  == xPos )) &&
       ( rcc.top + 1  <  yPos  )&& ( rcc.bottom - 1 >  yPos )) {
      // matching WE == x parameter
      if(( rcc.left     == xPos ) ||
         ( rcc.left + 1 == xPos ) ||
         ( rcc.left - 1 == xPos ))
      {
         Clippos = DV_ON_LEFT;
      } else {
         Clippos = DV_ON_RIGHT;
      }
   } else if((( rcc.top      == yPos  ) || ( rcc.bottom     == yPos )||
              ( rcc.top + 1  == yPos  ) || ( rcc.bottom + 1 == yPos )||
              ( rcc.top - 1  == yPos  ) || ( rcc.bottom - 1 == yPos )) &&
              ( rcc.left - 1 <  xPos  ) && ( rcc.right - 1  >  xPos )) {
      // matching NS == y parameter
      if(( rcc.top      == yPos  ) ||
         ( rcc.top + 1  == yPos  ) ||
         ( rcc.top - 1  == yPos  ))
      {
         Clippos = DV_ON_TOP;
      } else {
         Clippos = DV_ON_BOTTOM;
      }
   } else if( ( xPos > rcc.left   ) &&
              ( xPos < rcc.right  ) &&
              ( yPos > rcc.top    ) &&
              ( yPos < rcc.bottom ) ) {
      // could use BOOL PtInRect(CONST RECT *lprc,  // rectangle
      // POINT pt ); // point, but here in can NOT be ON top or left
      Clippos = DV_ON_ALL;
   } else {
      //SetCursor( hCurArrow );
      Clippos = DV_ON_NONE;
   }

   Release_DIB_Info( hWnd, &hDIBInfo, &lpDIBInfo );
   SetCursorType( Clippos );
   SetMouseMessage( hWnd, xPos, yPos, Clippos, &rcc );
}

//---------------------------------------------------------------------
//
// Function:   DrawSelect ( HDC hDc, LPRECT lpClip )
//
// Purpose:    Draw the rubberbanding rectangle with the specified
//             dimensions on the specified DC.  Rectangle includes
//             a string with its dimensions centered within it.
//             ie paintclip
//             Code was stolen almost verbatim from ShowDIB.
//
// Parms:      hDC      == DC to draw into.
//             lprcClip == Pointer to Rectangle to draw.
//
// History:   Date      Reason
//             ????     Created
//			1998 May 4	There appears to be a PROBLEM with the CLIP
//						when the image is SCROLLED!
//			1998 Dec 27 Changed to POINTER to RECT to draw
//
//---------------------------------------------------------------------

void DrawSelect( HDC hDC, LPRECT lprcClip )
{
	char	szStr[80];
	DWORD	dwExt;
	int		x, y, nLen, dx, dy;
	HDC		hDCBits;
	HBITMAP	hBitmap, hOldmap;

	// Don't have anything to do if the rectangle is empty.
	if( ( hDC == 0 ) ||
		( lprcClip == 0 ) ||
		( IsRectEmpty( lprcClip ) ) )
		return;

	// Draw rectangular clip region
	PatBlt( hDC,
		lprcClip->left,
		lprcClip->top,
		lprcClip->right - lprcClip->left,
		1,
		DSTINVERT );

	PatBlt( hDC,
		lprcClip->left,
		lprcClip->bottom,
		1,
		-(lprcClip->bottom - lprcClip->top),
		DSTINVERT );

	PatBlt( hDC,
		lprcClip->right - 1,
		lprcClip->top,
		1,
		lprcClip->bottom - lprcClip->top,
		DSTINVERT );

	PatBlt( hDC,
		lprcClip->right,
		lprcClip->bottom - 1,
		-(lprcClip->right - lprcClip->left),
		1,
		DSTINVERT );

	// Format the dimensions string
	wsprintf( szStr,
		"%dx%d",
		(lprcClip->right - lprcClip->left),
		(lprcClip->bottom - lprcClip->top) );

	nLen = lstrlen( szStr );

	// and center it in the rectangle
	dwExt   = GetTextExtent( hDC, szStr, nLen );
#ifdef	DIAGTE
	if( dwExt != GetTE( hDC, szStr, nLen ) )
		chkchk();
#endif	// DIAGTE
	dx      = LOWORD (dwExt);
	dy      = HIWORD (dwExt);
	x       = (lprcClip->right  + lprcClip->left - dx) / 2;
	y       = (lprcClip->bottom + lprcClip->top  - dy) / 2;

	hDCBits = CreateCompatibleDC( hDC );

	// Output the text to the DC
	SetTextColor( hDCBits, RGB(255, 255, 255) );
	SetBkColor(   hDCBits, RGB(  0,   0,   0) );

	hBitmap = CreateBitmap( dx, dy, 1, 1, NULL );
	if( hBitmap )
	{
		hOldmap = SelectObject( hDCBits, hBitmap );
		ExtTextOut( hDCBits, 0, 0, 0, NULL, szStr, nLen, NULL );
		BitBlt( hDC, x, y, dx, dy, hDCBits, 0, 0, SRCINVERT );
		hOldmap = SelectObject( hDCBits, hOldmap );
		DeleteObject( hBitmap );
	}

	DeleteDC( hDCBits );

#if	(defined( DIAGSCROLL ) && defined( SHOWEACLIP ))
	DiagDrawSel( lprcClip, NULL );
#endif	// DIAGSCROLL nad SHOWEACLIP

	rcPrevClip.left   = lprcClip->left;
	rcPrevClip.top    = lprcClip->top;
	rcPrevClip.right  = lprcClip->right;
	rcPrevClip.bottom = lprcClip->bottom;

}


BOOL  IsValidClip( PRECT prc )
{
   if( IsRectEmpty(prc) )
      return FALSE;
   if( RECTWIDTH(prc) <= 0 )
      return FALSE;
   if( RECTHEIGHT(prc) <= 0 )
      return FALSE;

   return TRUE;
}

//---------------------------------------------------------------------
//
// Function:   ChildWndLeftButton
//
// Purpose:    Called by ChildWndProc() on WM_LBUTTONDOWN.
//
//             If the user presses the left button, erase the currently
//             selected rectangle.  Then, start drawing a
//             rectangle (for the area of the DIB to be put in the
//             clipboard on an Edit/Paste operation).
//
//
// Parms:      hWnd == Handle to window getting WM_LBUTTONDOWN.
//
// History:   Date      Reason
//
//            10/15/91  Cut code out from WM_LBUTTONDOWN case.
//             
//---------------------------------------------------------------------
void ChildWndLeftButton( HWND hWnd, int x, int y )
{
	HANDLE		hDIBInfo;
	LPDIBINFO	lpDIBInfo;
	HDC			hDC;
	RECT		   rcClip, rcCurrent;
	int			cxDIB, cyDIB;
   LPARAM      lp = MAKELONG(x,y);

	// Start a new clip rectangle.  Track the rubber band. Rubber
	//  band won't be allowed to extend past the extents of the
	//  DIB.
	rcClip.top  = y;
	rcClip.left = x;

	// Find the old clip rectangle and erase it.
	hDIBInfo = GetWindowExtra( hWnd, WW_DIB_HINFO );
	if( !hDIBInfo )
		return;

	lpDIBInfo = (LPDIBINFO) DVGlobalLock (hDIBInfo);
	if( !lpDIBInfo )
		return;

   rcCurrent = lpDIBInfo->rcClip;   // KEEP CURRENT

	// Determine the DIB's extents.  This is different than the
	//  DIB's height/width when the DIB's stretched.
	if( lpDIBInfo->Options.bStretch2W ) {
		RECT rcClient;
		GetClientRect( hWnd, &rcClient );
		cxDIB = rcClient.right;
		cyDIB = rcClient.bottom;
	} else {
		cxDIB = lpDIBInfo->di_dwDIBWidth;
		cyDIB = lpDIBInfo->di_dwDIBHeight;
	}

   if( Clippos == DV_ON_NONE ) {

	   hDC = GetDC( hWnd );
      if( !hDC ) {
         DVGlobalUnlock( hDIBInfo );
		   return;
      }

   #ifdef	WIN32
	   SetWindowOrgEx( hDC,
		   GetScrollPos( hWnd, SB_HORZ ),
		   GetScrollPos( hWnd, SB_VERT ),
		   NULL );
   #else	// !WIN32
	   SetWindowOrg( hDC,
		   GetScrollPos (hWnd, SB_HORZ),
		   GetScrollPos (hWnd, SB_VERT) );
   #endif	// WIN32 y/n

	   // REMOVE ANY OLD CLIP
      DC_ClearClip( hDC, &lpDIBInfo->rcClip, lpDIBInfo ); // includes DrawSelect( ... );

   #ifdef	SHOWCLIP
	   ShowRemove( &lpDIBInfo->rcClip );
   #endif	// SHOWCLIP

	   ReleaseDC( hWnd, hDC );

      if( gi_MouseMagnify ) {
         MouseMagnify( hWnd, lp, lpDIBInfo );
         SetRect( &rcClip, 0, 0, 0, 0 );
      } else {
         if( IsMagnifyOn( &lpDIBInfo->sMagnif ) )
            Magnify2Off( hWnd, &lpDIBInfo->sMagnif );
         TrackMouse( hWnd, &rcClip, cxDIB, cyDIB );   // NEW TRACKING
         if( IsValidClip( &rcClip ) ) {
            Add2ClipList( &rcClip );
         }
      }

   } else {
      // NOT if( Clippos == DV_ON_NONE ) {
      // that is mouse is ON or WITHIN the CLIP
      TrackMouse2( hWnd, &rcClip, cxDIB, cyDIB, lpDIBInfo );   // MOVE or RESIZCLIP
      if( IsValidClip( &rcClip ) )
      {
         if( IsValidClip( &rcCurrent ) ) {
            if( EqualRect( &rcClip, &rcCurrent ) )
               Add2ClipList( &rcClip );   // place at HEAD
            else
               Remove_and_Add2ClipList( &rcClip, &rcCurrent ); // remove and ADD
         } else {
            Add2ClipList( &rcClip );   // just add or put at head
         }
      }
   }

	// Store the new clipboard coordinates.
	lpDIBInfo->rcClip = rcClip;

	DVGlobalUnlock( hDIBInfo );

}

//---------------------------------------------------------------------
// Function:   CopyHandle
// Purpose:    Makes a copy of the given global memory block.  Returns
//             a handle to the new memory block (NULL on error).
//             Routine stolen verbatim out of ShowDIB.
// Parms:      h == Handle to global memory to duplicate.
// Returns:    Handle to new global memory block.
// History:   Date      Reason
//             ???      Created
//---------------------------------------------------------------------
HANDLE CopyHandle( HANDLE h )
{
	PINT8	lpCopy;
	PINT8	lp;
	HANDLE     hCopy;
	DWORD      dwLen;

	hCopy = 0;
	if( ( h ) &&
		( dwLen = GlobalSize( h ) ) )
	{
		//if( hCopy = DVGlobalAlloc( GHND, dwLen ) )
		if( hCopy = DVGAlloc( "CopyHand", GHND, dwLen ) )
		{
			if( lpCopy = (PINT8)DVGlobalLock( hCopy ) )
         {
			   // NOTE: Could be a handle from CLIPBOARD
			   if( lp = (PINT8)GlobalLock( h ) )
            {
#ifdef	WIN32
			      memcpy( lpCopy, lp, dwLen );
#else	// !WIN32
			      while( dwLen-- )
				      *lpCopy++ = *lp++;	// Slow BYTE by BYTE copy
#endif	// WIn32 y/n
      			GlobalUnlock( h );	// Note: A Clipboard Handle, for ex.
            }

            DVGlobalUnlock( hCopy );
         }
		}
	}
	return( hCopy );
}

//---------------------------------------------------------------------
//
// Function:   CropBitmap
//
// Purpose:    Crops a bitmap to a new size specified by the lpRect
//             parameter.  The lpptSize parameter is used to determine
//             how much to stretch/compress the bitmap.  Returns a
//             handle to a new bitmap.  If lpRect is empty, copies the
//             bitmap to a new one.
//
//             Stolen almost verbatim out of ShowDIB.
//
// Parms:      hbm      == Handle to device dependent bitmap to crop.
//             hPal     == Palette to use in cropping (NULL for default pal.)
//             lpRect   == New bitmap's size (size we're cropping to).
//             lpptSize == A scaling factor scale by the proportion:
//                              Bitmap Width / lpptSize->x horizontally,
//                              Bitmap Height / lpptSize->y horizontally.
//                           Note that if lpptSize is set to the bitmap's
//                           dimensions, no scaling occurs.
//             
//
// History:   Date      Reason
//            6/15/91   Stolen from ShowDIB
//             
//---------------------------------------------------------------------

HBITMAP CropBitmap (HBITMAP hbm, 
                   HPALETTE hPal, 
                     LPRECT lpRect, 
                    LPPOINT lpptSize)
{
	HDC      hMemDCsrc;
	HDC      hMemDCdst;
	BITMAP   bm;
	int      dxDst,dyDst, dxSrc, dySrc;
	double   cxScale, cyScale;
	HPALETTE hOldPal1 = NULL;
	HPALETTE hOldPal2 = NULL;
	HBITMAP  hNewBm = NULL;

	if( !hbm )		// Oops, no BITMAP!!!
		return( hNewBm );


	GetObject( hbm, sizeof(BITMAP), (LPSTR)&bm );


	hMemDCsrc = CreateCompatibleDC( NULL );
	hMemDCdst = CreateCompatibleDC( NULL );


	if( hPal )
	{
		hOldPal1 = SelectPalette( hMemDCsrc, hPal, FALSE );
		hOldPal2 = SelectPalette( hMemDCdst, hPal, FALSE );
		RealizePalette( hMemDCdst );
	}


	dxDst     = lpRect->right  - lpRect->left;
	dyDst     = lpRect->bottom - lpRect->top;
	cxScale   = (double) bm.bmWidth  / lpptSize->x;
	cyScale   = (double) bm.bmHeight / lpptSize->y;
	dxSrc     = (int) ((lpRect->right - lpRect->left) * cxScale);
	dySrc     = (int) ((lpRect->bottom - lpRect->top) * cyScale);


	if( (dxDst == 0) || (dyDst == 0) )
	{
		dxDst = bm.bmWidth;
		dyDst = bm.bmHeight;
	}


	if( dxSrc == 0 )
		dxSrc = 1;

	if( dySrc == 0 )
		dySrc = 1;


	hNewBm = CreateBitmap( dxDst, dyDst, bm.bmPlanes, bm.bmBitsPixel, NULL );

	if( hNewBm )
	{
		HBITMAP hOldBitmap1, hOldBitmap2;

		hOldBitmap1 = SelectObject( hMemDCsrc, hbm );
		hOldBitmap2 = SelectObject( hMemDCdst, hNewBm );

		StretchBlt( hMemDCdst,
			0,
			0,
			dxDst,
			dyDst,
			hMemDCsrc,
			(int) (lpRect->left * cxScale),
			(int) (lpRect->top  * cyScale),
			dxSrc,
			dySrc,
			SRCCOPY );

		SelectObject( hMemDCsrc, hOldBitmap1 );
		SelectObject( hMemDCdst, hOldBitmap2 );
	}

	if( hOldPal1 )
		SelectPalette( hMemDCsrc, hOldPal1, FALSE );

	if( hOldPal2 )
		SelectPalette( hMemDCdst, hOldPal1, FALSE );

	DeleteDC( hMemDCsrc );
	DeleteDC( hMemDCdst );

	return( hNewBm );
}	// End - HBITMAP CropBitmap(HBITMAP, HPALETTE, LPRECT, LPPOINT )



//---------------------------------------------------------------------
//
// Function:   RenderFormat
//
// Purpose:    Renders an object for the clipboard.  The format is
//             specified in the "cf" variable (either CF_BITMAP,
//             CF_DIB, or CF_PALETTE).
//
//             This happens after a HandleCopyClipboard ...
//
//             Stolen almost verbatim out of ShowDIB.
//
// Parms:      hWndClip == Window clipboard belongs to, and where our
//                         image is stored).
//             cf       == Format to render (CF_BITMAP, CF_DIB, CF_PALETTE)
//                         Is actually the wParam of the WM_ message.
//             ptDIBSize== Size of the DIB in the given window.
//
// History:   Date      Reason
//             ???      Created
//             
//---------------------------------------------------------------------

HANDLE RenderFormat( HWND hWndClip, int cf, POINT ptDIBSize )
{
	HBITMAP   hBitmap;
//	HANDLE    hDIB;
	HANDLE    hDIBInfo;
	LPDIBINFO lpDIBInfo;
	HPALETTE  hPalette;        // Handle to the bitmap's palette.
	HANDLE    h = NULL;		// Start with NOTHING

	if( !hWndClip )		// Oops, NO ACTIVE WINDOW!!!
		return( h );

	hDIBInfo = GetWindowExtra( hWndClip, WW_DIB_HINFO );
	if( !hDIBInfo )		// Double Oops - NO INFORMATION!!!
		return( h );

	lpDIBInfo    = (LPDIBINFO) DVGlobalLock (hDIBInfo);
   if( !lpDIBInfo )     // triple oooops - NO LOCK HANDLE
      return( h );

//	hDIB         = lpDIBInfo->hDIB;
	hPalette     = lpDIBInfo->hPal;
	hBitmap      = lpDIBInfo->hBitmap;
	DVGlobalUnlock (hDIBInfo);
	switch( cf )
	{
	case CF_BITMAP:
		h = CropBitmap( hBitmap, hPalette, &grcClip, &ptDIBSize );
		break;

	case CF_DIB:
		{
			HBITMAP hbm;
         // NOTE:  For simplicity, we use the display device to crop
			//        the bitmap.  This means that we may lose color
			//        precision (if the display device has less color
			//        precision than the DIB).  This isn't usually a
			//        problem, as users shouldn't really be editting
			//        images on devices that can't display them.
			hbm = RenderFormat( hWndClip, CF_BITMAP, ptDIBSize );
			if( hbm )
			{
				h = DVBitmapToDIB( hbm, hPalette );
				DeleteObject( hbm );
			}
			break;
		}

	case CF_PALETTE:
		if( hPalette )
			h = CopyPaletteChangingFlags( hPalette, 0 );
		break;

	}

	return h;

}	// End - HANDLE RenderFormat( HWND hWndClip, int cf, POINT ptDIBSize )


//---------------------------------------------------------------------
//
// Function:   HandleCopyClipboard
//
// Purpose:    User wants to copy the current DIB to the clipboard.
//             Tell the clipboard we can render a DIB, DDB, and a
//             palette (defer rendering until we get a WM_RENDERFORMAT
//             in our MDI child window procedure in CHILD.C).
//             Handle IDM_COPY from MENU
//
// Parms:      None
//             
//
// History:   Date      Reason
//            6/1/91    Created
//           11/4/91    Init grcClip to full DIB size if
//                      it is currently empty.
//             
//---------------------------------------------------------------------

HWND  HandleCopyClipboard( void )   // from frame->IDM_COPY
{
	HWND    hDIBWnd = GetCurrentMDIWnd();
	if( hDIBWnd )
   {
		// Clean clipboard of contents, and tell it we can render
		//  a DIB, a DDB, and/or a palette.
		if( OpenClipboard( hDIBWnd ) )
		{
			EmptyClipboard();
			SetClipboardData( CF_DIB,     NULL );
			SetClipboardData( CF_BITMAP,  NULL );
			SetClipboardData( CF_PALETTE, NULL );
			CloseClipboard ();

			// Set our globals to tell our app which child window
			//  owns the clipboard, and the clipping rectangle at
			//  the time of the copy.  If the clipping rectangle is
			//  empty, then use the entire DIB window.
			hWndClip   = hDIBWnd;
			grcClip    = GetCurrentClipRect( hWndClip );
			ptClipSize = GetCurrentDIBSize( hWndClip );
			if( IsRectEmpty( &grcClip ) )
			{
				grcClip.left   = 0;
				grcClip.top    = 0;
				grcClip.right  = ptClipSize.x;
				grcClip.bottom = ptClipSize.y;
			}
		}
		else
		{
			DIBError( ERR_CLIPBUSY );
         hDIBWnd = 0;
		}
   }
   else
	{
		DIBError( ERR_NOCLIPWINDOW );
	}

   return hDIBWnd;

}


//---------------------------------------------------------------------
//
// Function:   HandlePasteClipboard
//
// Purpose:    User wants to paste the clipboard's contents to our
//             app.  Open a new DIB window with the bitmap in the
//             clipboard.
//
// Parms:      None
//             
// History:   Date      Reason
//            6/1/91    Created
//             
//---------------------------------------------------------------------
HWND  HandlePasteClipboard( VOID )
{
   HWND        hWnd = 0;
	HANDLE      hDIB;
	HBITMAP     hBitmap;
	HPALETTE    hPal;
	PRDIB		   prd = (PRDIB)MALLOC( sizeof(RDIB) );
   if(!prd)
      chkme( "C:ERROR: No memory!!!"MEOR );
	NULPRDIB( prd );

   hDIB = 0;   // no DIB (device independant bitmap) yet

	// Open up the clipboard.  This routine assumes our app has
	//  the focus (which it should, as the user just picked the
	//  paste operation off the menu, and Windows is a non-preemptive
	//  system.  First we try for a DIB; if that's not available go
	//  for a bitmap (and a palette if one can be had).  Whatever
	//  format's available, we have to copy immediately (since the
	//  handle returned by GetClipboardData() belongs to the clipboard.
	//  Finally, go create the new MDI child window.
	if( OpenClipboard( GetFocus() ) )
	{
		if( IsClipboardFormatAvailable( CF_DIB ) )
		{
			hDIB = CopyHandle( GetClipboardData( CF_DIB ) );
		}
		else if( IsClipboardFormatAvailable( CF_BITMAP ) )
		{
			// HANDLE GetClipboardData(
			//    UINT uFormat );	// clipboard format  
			hBitmap = GetClipboardData( CF_BITMAP );

			if( IsClipboardFormatAvailable( CF_PALETTE ) )
				hPal = GetClipboardData( CF_PALETTE );
			else
				hPal = GetStockObject( DEFAULT_PALETTE );

			hDIB = DVBitmapToDIB( hBitmap, hPal );
		}
		else
		{
			DIBError( ERR_NOCLIPFORMATS );
		}

		CloseClipboard();

      if( !hDIB )
         goto Exit_Paste;

		prd->rd_pTitle = gszRPTit;     // set TITLE file name
      prd->rd_pPath  = gszRPNam;     // but keep the FULL NAME
		// The window title is of the form: "Clipboard1".  The
		//  number in the title is changed for each paste operation.
      // Get Next Name( &rd, &gszRPFrm[0], gnPasteNum++ );
      DVNextRDName2( prd, &gszRPFrm[0], &gnPasteNum, &gbChgPaste );   // = "TEMPB%03d.BMP"

		// Perform the actual window opening.
		prd->rd_hDIB    = hDIB;
		prd->rd_Caller  = df_CLIPBRD;

		//OpenDIBWindow( hDIB, szTitle, df_CLIPBRD );
		hWnd = OpenDIBWindow2( prd ); 
		if(hWnd)
      {
         if( WriteBMPFile2( gszRPNam, hDIB, TRUE ) )
         {
#ifdef	CHGADDTO
            //AddToFileList( szTitle );
            ADD2LIST( prd );
#endif	// CHGADDTO
         }
      }
	}
	else
	{
		DIBError (ERR_CLIPBUSY);
	}

Exit_Paste:

   MFREE(prd);

   return hWnd;

}	// End - void HandlePasteClipboard (void)

// eof
