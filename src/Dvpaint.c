
/*************************************************************************

      File:  DVPAINT.C

   Purpose:  Contains the routines to do rendering of device dependent
             bitmaps (DDBs), device independent bitmaps (DIBs), and
             DDBs->DIBs via SetDIBits then DIB->device.

             These routines are called by CHILD.C to do the actual
             bitmap rendering.

 Functions:  DIBPaint
             DDBPaint
             SetDIBitsPaint

  Comments:  Note that a commercial application would probably only
             want to use DDBs, since they are much faster when rendering.
             The DIB and SetDIBits() routines are here to demonstrate
             how to manipulate rendering of DIBs, and DDB->DIB->Screen,
             which has some useful applications.

   History:   Date      Reason
             6/ 1/91    Created

*************************************************************************/
#include	"Dv.h"		// all inclusive include

//#include <windows.h>
//#include "DvDib.h"
//#include "DvErrors.h"
//#include "DvPaint.h"
//#include "DvView.h"
//#include "DvMem.h"

#define		DIAGDIBP	// Output some iformation

extern int NewStretchDIBits(
    HDC  hdc,     // handle of device context
    int  XDest,     // x-coordinate of upper-left corner of dest. rect.
    int  YDest,     // y-coordinate of upper-left corner of dest. rect.
    int  nDestWidth,  // width of destination rectangle
    int  nDestHeight, // height of destination rectangle
    int  XSrc,     // x-coordinate of upper-left corner of source rect.
    int  YSrc,     // y-coordinate of upper-left corner of source rect.
    int  nSrcWidth,   // width of source rectangle
    int  nSrcHeight,  // height of source rectangle
    VOID  * lpBits,    // address of bitmap bits
    BITMAPINFO *lpBitsInfo,   // address of bitmap data
    UINT  iUsage,  // usage
    DWORD  dwRop );  // raster operation code

//---------------------------------------------------------------------
//
// Function:   DIBPaint
//
// Purpose:    Painting routine for a DIB.  Calls StretchDIBits() or
//             SetDIBitsToDevice() to paint the DIB.  The DIB is
//             output to the specified DC, at the coordinates given
//             in lpDCRect.  The area of the DIB to be output is
//             given by lpDIBRect.  The specified palette is used.
//
// Parms:      hDC       == DC to do output to.
//             lpDCRect  == Rectangle on DC to do output to.
//             hDIB      == Handle to global memory with a DIB spec
//                          in it (either a BITMAPINFO or BITMAPCOREINFO
//                          followed by the DIB bits).
//             lpDIBRect == Rect of DIB to output into lpDCRect.
//             hPal      == Palette to be used.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

void DIBPaint( LPDIBINFO lpDIBInfo,
			  HDC hDC,
			  LPRECT lpDCRect,
			  HANDLE hDIB,
			  LPRECT lpDIBRect,
			  HPALETTE hPal,
			  DWORD	dwROP )
{
	LPSTR	lpDIBHdr, lpDIBBits;
	int		i;
#ifdef	DIAGDIBP
	LPSTR	lpd;
#endif	// DIAGDIBP
   BOOL  bEqual = FALSE;
#ifdef	DIAGDIBP
	lpd = GetTmp3();
#endif	// DIAGDIBP

	if( !hDIB )
	{
#ifdef	DIAGDIBP
		lstrcpy( lpd, "DIBPaint failed since no hDIB passed!\r\n" );
		DO(lpd);
#endif	// DIAGDIBP
		return;
	}
	// Lock down the DIB, and get a pointer to the beginning
	// of the bit buffer.
	if( (lpDIBHdr = DVGlobalLock( hDIB )) == 0 ) // LOCK DIB HANDLE
	{
#ifdef	DIAGDIBP
		lstrcpy( lpd, "DIBPaint failed since no pointer to hDIB!\r\n" );
		DO(lpd);
#endif	// DIAGDIBP
		return;
	}

	lpDIBBits = FindDIBBits( lpDIBHdr );

	// Make sure to use the stretching mode best for color pictures.
	SetStretchBltMode( hDC, COLORONCOLOR );
   // setup for the system action
   i = 0;
   bEqual = FALSE;
	// Determine whether to call StretchDIBits() or
	// SetDIBitsToDevice().
	if(( RECTWIDTH(lpDCRect)  == RECTWIDTH (lpDIBRect)) &&
		( RECTHEIGHT (lpDCRect) == RECTHEIGHT (lpDIBRect)) )
	{
      bEqual = TRUE;
#ifdef	DIAGDIBP
		strcpy( lpd, "DIBPaint using SetDIBitsToDevice since dest = src.\r\n" );
		DO(lpd);
#endif	// DIAGDIBP
		// Destination Window EQUAL Source Bitmap
		i = SetDIBitsToDevice( hDC,	// hDC
			lpDCRect->left,		// DestX
			lpDCRect->top,		// DestY
			RECTWIDTH(lpDCRect),// nDestWidth
			RECTHEIGHT(lpDCRect),// nDestHeight
			lpDIBRect->left,	// SrcX
			(int) DIBHeight (lpDIBHdr) -
				lpDIBRect->top -
				RECTHEIGHT (lpDIBRect),	// SrcY
			0,					// nStartScan
			(WORD)DIBHeight(lpDIBHdr),	// nNumScans
			lpDIBBits,			// lpBits
			(LPBITMAPINFO)lpDIBHdr,	// lpBitsInfo
			DIB_RGB_COLORS );	// wUsage

	}

	if( i == 0 )   // then tryelse
	{
#ifdef	DIAGDIBP
      if( bEqual )
      {
   		sprintf( lpd,
   			"DIBPaint using StretchDIBits since\r\n"
   			"Dest.= Src = %d,%d,%d,%d but SetDIBitsToDevice() failed.\r\n",
   			lpDCRect->left,		// DestX
   			lpDCRect->top,		// DestY
   			RECTWIDTH(lpDCRect), // nDestWidth
   			RECTHEIGHT(lpDCRect) ); // nDestHeight
      }
      else
      {
   		sprintf( lpd,
   			"DIBPaint using StretchDIBits since\r\n"
   			"Dest.: %d,%d,%d,%d Source = %d,%d,%d,%d.\r\n",
   			lpDCRect->left,		// DestX
   			lpDCRect->top,		// DestY
   			RECTWIDTH(lpDCRect), // nDestWidth
   			RECTHEIGHT(lpDCRect), // nDestHeight
   			lpDIBRect->left,	// SrcX
   			lpDIBRect->top,		// SrcY
   			RECTWIDTH(lpDIBRect), // wSrcWidth
   			RECTHEIGHT(lpDIBRect) ); // wSrcHeight
      }
		DO(lpd);
#endif	// DIAGDIBP

		i = StretchDIBits( hDC,		// hDC
			lpDCRect->left,		// DestX
			lpDCRect->top,		// DestY
			RECTWIDTH(lpDCRect), // nDestWidth
			RECTHEIGHT(lpDCRect), // nDestHeight
			lpDIBRect->left,	// SrcX
			lpDIBRect->top,		// SrcY
			RECTWIDTH(lpDIBRect), // wSrcWidth
			RECTHEIGHT(lpDIBRect), // wSrcHeight
			lpDIBBits,			// lpBits
			(LPBITMAPINFO) lpDIBHdr, // lpBitsInfo
			DIB_RGB_COLORS,		// wUsage
			dwROP );			// dwROP

		if( i != RECTHEIGHT(lpDIBRect) )
		{
#ifdef	DIAGDIBP
			lstrcpy( lpd, "StretchDIBits FAILED! Trying NewStretchDIBits!\r\n" );
			DO(lpd);
#endif	// DIAGDIBP
//int NewStretchDIBits(
//    HDC  hdc,     // handle of device context
//    int  XDest,     // x-coordinate of upper-left corner of dest. rect.
//    int  YDest,     // y-coordinate of upper-left corner of dest. rect.
//    int  nDestWidth,  // width of destination rectangle
//    int  nDestHeight, // height of destination rectangle
//    int  XSrc,     // x-coordinate of upper-left corner of source rect.
//    int  YSrc,     // y-coordinate of upper-left corner of source rect.
//    int  nSrcWidth,   // width of source rectangle
//    int  nSrcHeight,  // height of source rectangle
//    VOID  * lpBits,    // address of bitmap bits
//    BITMAPINFO *lpBitsInfo,   // address of bitmap data
//    UINT  iUsage,  // usage
//    DWORD  dwRop )  // raster operation code
			i = NewStretchDIBits( hDC,		// hDC
				lpDCRect->left,		// DestX
				lpDCRect->top,		// DestY
				RECTWIDTH(lpDCRect), // nDestWidth
				RECTHEIGHT(lpDCRect), // nDestHeight
				lpDIBRect->left,	// SrcX
				lpDIBRect->top,		// SrcY
				RECTWIDTH(lpDIBRect), // wSrcWidth
				RECTHEIGHT(lpDIBRect), // wSrcHeight
				lpDIBBits,			// lpBits
				(LPBITMAPINFO) lpDIBHdr, // lpBitsInfo
				DIB_RGB_COLORS,		// wUsage
				dwROP );			// dwROP
			if( i != RECTHEIGHT(lpDIBRect) )
			{
#ifdef	DIAGDIBP
				wsprintf( lpd,
					"Appears NewStretchDIBits also FAILED!\r\n" );
				DO(lpd);
#endif	// DIAGDIBP
			}
		}
	}

	DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE

}


//---------------------------------------------------------------------
//
// Function:   DDBPaint
//
// Purpose:    Painting routine for a DDB.  Calls BitBlt() or
//             StretchBlt() to paint the DDB.  The DDB is
//             output to the specified DC, at the coordinates given
//             in lpDCRect.  The area of the DDB to be output is
//             given by lpDDBRect.  The specified palette is used.
//
//             IMPORTANT assumption:  The palette has been realized
//             elsewhere...  We won't bother figuring out whether it
//             should be realized as a foreground or background palette
//             here.
//
// Parms:      hDC       == DC to do output to.
//             lpDCRect  == Rectangle on DC to do output to.
//             hDDB      == Handle to the device dependent bitmap (DDB).
//             lpDDBRect == Rect of DDB to output into lpDCRect.
//             hPal      == Palette to be used.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

void DDBPaint( LPDIBINFO lpDIBInfo,
			  HDC hDC,
			  LPRECT lpDCRect,
			  HBITMAP hDDB,
			  LPRECT lpDDBRect,
			  HPALETTE hPal,
			  DWORD	dwROP )
{
	HDC			hMemDC;
	HBITMAP		hOldBitmap;
	HPALETTE	hOldPal1 = NULL;
	HPALETTE	hOldPal2 = NULL;

	hMemDC = CreateCompatibleDC (hDC);
	if( !hMemDC )
		return;

	if( hPal )
	{
		hOldPal1   = SelectPalette (hMemDC, hPal, FALSE);
		hOldPal2   = SelectPalette (hDC, hPal, FALSE);
		// Assume the palette's already been realized (no need to
		//  call RealizePalette().  It should have been realized in
		//  our WM_QUERYNEWPALETTE or WM_PALETTECHANGED messages...
	}

   // select new DDB into memory
	hOldBitmap = SelectObject( hMemDC, hDDB );

	SetStretchBltMode( hDC, COLORONCOLOR );
   // BUT empirically discovered DIBs from ANIMATE GIF are NOT always the SAME
   // ========================================================================
   if( ( lpDIBInfo->Options.bAnimate ) &&
       ( lpDIBInfo->di_bChgdDIB       ) )
   {
      // per Explorer animation this is NOT strictly correct
      // It appears to centre the image on the bottom if less that the window
      // It all further assumes the first image SETS the SIZE of the IMAGE
      // A lot more to learn here, but this LOOKS better than before.
      // ====================================================================
		StretchBlt( hDC,
			lpDCRect->left,
			lpDCRect->top,
			lpDCRect->right - lpDCRect->left,
			lpDCRect->bottom - lpDCRect->top,
			hMemDC,
			0,
			0,
			lpDIBInfo->di_dwNEWWidth,
			lpDIBInfo->di_dwNEWHeight,
			dwROP );
      // ====================================================================
      goto Done_ROP;
   }

	if( (RECTWIDTH (lpDCRect) == RECTWIDTH (lpDDBRect) ) &&
		 (RECTHEIGHT(lpDCRect) == RECTHEIGHT(lpDDBRect) ) )
	{
		BitBlt( hDC,
			lpDCRect->left,
			lpDCRect->top,
			lpDCRect->right - lpDCRect->left,
			lpDCRect->bottom - lpDCRect->top,
			hMemDC,
			lpDDBRect->left,
			lpDDBRect->top,
			dwROP );
	}
	else
	{
		StretchBlt( hDC,
			lpDCRect->left,
			lpDCRect->top,
			lpDCRect->right - lpDCRect->left,
			lpDCRect->bottom - lpDCRect->top,
			hMemDC,
			lpDDBRect->left,
			lpDDBRect->top,
			lpDDBRect->right - lpDDBRect->left,
			lpDDBRect->bottom - lpDDBRect->top,
			dwROP );
	}

Done_ROP:

	SelectObject( hMemDC, hOldBitmap );
   
	if( hOldPal1 )
		SelectPalette( hMemDC, hOldPal1, FALSE );

	if( hOldPal2 )
		SelectPalette( hDC, hOldPal2, FALSE );

	DeleteDC( hMemDC );

}


//---------------------------------------------------------------------
//
// Function:   SetDIBitsPaint
//
// Purpose:    Paint routine used when the SetDIBits option is being
//             used.  Routine first call SetDIBits() to convert a DIB
//             to a DDB, then it calls DDBPaint.
//
//     NOTE:  This routine was included for two reasons -- first,
//      to test drivers SetDIBits() functions.  Second, to demo
//      such a technique.  Most applications wouldn't bother to
//      do anything like this (why not just SetDIBitsToDevice()!
//
// Parms:      hDC       == DC to do output to.
//             lpDCRect  == Rectangle on DC to do output to.
//             hDIB      == Handle to global memory with a DIB spec
//                          in it (either a BITMAPINFO or BITMAPCOREINFO
//                          followed by the DIB bits).
//             lpDIBRect == Rect of DIB to output into lpDCRect.
//             hPal      == Palette to be used.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

void SetDIBitsPaint( LPDIBINFO lpDIBInfo,
					HDC hDC,
					LPRECT lpDCRect,
					HANDLE hDIB,
					LPRECT lpDIBRect,
					HPALETTE hPal,
					DWORD dwROP )
{
	LPSTR	lpDIBHdr, lpDIBBits;
	HBITMAP hBitmap;

	// Return if we don't have a DIB.
	if( !hDIB )
		return;

	// Lock down the DIB, and get a pointer to the
	// beginning of the bit buffer.
	if( (lpDIBHdr = DVGlobalLock( hDIB )) == 0 ) // LOCK DIB HANDLE
		return;

	lpDIBBits = FindDIBBits( lpDIBHdr );

	// Create the DDB.  Note that the palette has already been
	//  selected into the DC before calling this routine.
	hBitmap = CreateCompatibleBitmap( hDC,
		(int) DIBWidth (lpDIBHdr),
		(int) DIBHeight (lpDIBHdr) );

	if( !hBitmap )
	{
		DWORD	dwi;
		if( dwi = GetLastError() )
			SysDIBError( ERR_CREATEDDB, dwi );
		else
			DIBError( ERR_CREATEDDB );
		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE - for error exit
		return;
	}

	if( SetDIBits( hDC,		// hDC compat. with DDB
			hBitmap,		// handle to DDB
			0,				// Start scan in lpBits
			(int) DIBHeight (lpDIBHdr), // Num Scans in lpBits
			lpDIBBits,		// Pointer to bits
			(LPBITMAPINFO)lpDIBHdr,	// Pointer to DIB header
			DIB_RGB_COLORS ) == 0 )	// DIB contains RGBs in color table
	{
		DIBError (ERR_SETDIBITS);
	}

	// Call DDBPaint to paint the bitmap.  Then clean up.
	DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE

	DDBPaint( lpDIBInfo, hDC, lpDCRect, hBitmap, lpDIBRect, hPal, dwROP );

	DeleteObject( hBitmap );

}

// eof - DvPaint.c

