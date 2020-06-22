
// DvUnused.c
// just some unused code, or testing code ...

#include "dv.h"

/* ================= Just for some TESTS ===============

#ifdef	WIN32
// ============================================
void ChildWndPaint2( HWND hWnd, WPARAM wParam )
{
	BOOL		flg;
	HDC			hDC, rhDC;
	PAINTSTRUCT	ps;
	int			xScroll, yScroll;
	HPALETTE	hOldPal = NULL;
	RECT		rectC, rectClient, rectDDB;
	BOOL		bStretch;
	HANDLE		hDIBInfo;
	LPDIBINFO	lpDIBInfo;
	BITMAP		Bitmap;
	HDC			hMemDC;
	HBITMAP		hOldBitmap;
	HPALETTE	hOldPal1 = NULL;
	HPALETTE	hOldPal2 = NULL;
	HPALETTE	hPal;
	HBITMAP		hDDB;
	LPRECT		lpDCRect, lpDDBRect;
#ifdef	WJPEG4
	HGLOBAL		hgInfo;
	DWORD		dwErr;
#endif	// WJPEG4
#ifdef	TRYCOMP
	HBITMAP		hbm, hbmold;
#endif	
	if( !GetUpdateRect( hWnd, NULL, FALSE ) )
		return;

	//Hourglass (TRUE);
	GetClientRect( hWnd, &rectClient );
	CopyRect( &rectC, &rectClient );
	hDIBInfo = GetWindowExtra(hWnd, WW_DIB_HINFO);

	if( rhDC = (HDC)wParam )
		BeginPaint( hWnd, &ps );
	else
		rhDC = BeginPaint( hWnd, &ps );

	hDC = rhDC;	// Establish default

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

	if( ( !lpDIBInfo->hDIB ) ||
		!(hDDB = lpDIBInfo->hBitmap) )
	{
		if( !lpDIBInfo->hDIB )
			ShowAbort( hWnd, hDC, SA_NohDIB );
		else
			ShowAbort( hWnd, hDC, SA_NoBmp );
		DVGlobalUnlock( hDIBInfo );
		goto ABORTPAINT;
	}

	hMemDC = CreateCompatibleDC( rhDC );
	if( !hMemDC )
	{
		ShowAbort( hWnd, hDC, SA_NohmDC );
		DVGlobalUnlock( hDIBInfo );
		goto ABORTPAINT;
		//return;
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
#else
	hDC = rhDC;
#endif
	xScroll  = GetScrollPos  (hWnd, SB_HORZ);
	yScroll  = GetScrollPos  (hWnd, SB_VERT);

	bStretch = lpDIBInfo->Options.bStretch;

	// When we're iconic, we'll always stretch the DIB
	//  to our icon.  Otherwise, we'll use the stretching
	//  option the user picked.
	if( IsIconic( hWnd ) )
		bStretch = TRUE;
	else
		bStretch = lpDIBInfo->Options.bStretch;

	// Set up the scroll bars appropriately.

	if( bStretch )
		ScrollBarsOff( lpDIBInfo, hWnd );
	else
		SetupScrollBars( lpDIBInfo, hWnd,
			lpDIBInfo->di_dwDIBWidth, lpDIBInfo->di_dwDIBHeight);

	// Set up the necessary rectangles -- i.e. the rectangle
	//  we're rendering into, and the rectangle in the DIB.

	GetObject( lpDIBInfo->hBitmap,
		sizeof (Bitmap),
		(LPSTR) &Bitmap);
   
	if( bStretch )
	{
		rectDDB.left   = 0;
		rectDDB.top    = 0;
		rectDDB.right  = Bitmap.bmWidth;
		rectDDB.bottom = Bitmap.bmHeight;
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

	// Setup the palette.
	if( hPal = lpDIBInfo->hPal )
		hOldPal = SelectPalette( hDC, hPal, TRUE );

	RealizePalette (hDC);

	// Go do the actual painting.
	//switch( lpDIBInfo->Options.wDispOption )
	//{
//	case DISP_USE_DIBS:
//		DIBPaint( lpDIBInfo, hDC, &rectClient, lpDIBInfo->hDIB, &rectDDB,
//			lpDIBInfo->hPal, SRCCOPY );
//		break;
//
//	case DISP_USE_SETDIBITS:
//		SetDIBitsPaint( lpDIBInfo, hDC, &rectClient, lpDIBInfo->hDIB, &rectDDB,
//			lpDIBInfo->hPal, SRCCOPY );
//		break;
//
//	case DISP_USE_DDBS:
//	default:
//		DDBPaint( lpDIBInfo, hDC, &rectClient, lpDIBInfo->hBitmap, &rectDDB,
//			lpDIBInfo->hPal, SRCCOPY );
//		break;
//	}
//void DDBPaint( LPDIBINFO lpDIBInfo, HDC hDC,
//			  LPRECT lpDCRect,
//			  HBITMAP hDDB,
//			  LPRECT lpDDBRect,
//			  HPALETTE hPal,
//			  DWORD dwROP )
//{

//	hMemDC = CreateCompatibleDC (hDC);
//	if( !hMemDC )
//		return;

	if( hPal )
	{
		hOldPal1   = SelectPalette (hMemDC, hPal, FALSE);
		hOldPal2   = SelectPalette (hDC, hPal, FALSE);
	}

	hOldBitmap = SelectObject( hMemDC, hDDB );

	SetStretchBltMode( hDC, COLORONCOLOR );

	lpDCRect = &rectClient;
	lpDDBRect = &rectDDB;

	if( (RECTWIDTH (lpDCRect)  == RECTWIDTH (lpDDBRect) ) &&
		(RECTHEIGHT (lpDCRect) == RECTHEIGHT (lpDDBRect)) )
	{
		flg = BitBlt( hDC,
			lpDCRect->left,
			lpDCRect->top,
			lpDCRect->right - lpDCRect->left,
			lpDCRect->bottom - lpDCRect->top,
			hMemDC,
			lpDDBRect->left,
			lpDDBRect->top,
			SRCCOPY );
	}
	else
	{
		flg = StretchBlt( hDC,
			lpDCRect->left,
			lpDCRect->top,
			lpDCRect->right - lpDCRect->left,
			lpDCRect->bottom - lpDCRect->top,
			hMemDC,
			lpDDBRect->left,
			lpDDBRect->top,
			lpDDBRect->right - lpDDBRect->left,
			lpDDBRect->bottom - lpDDBRect->top,
			SRCCOPY );
	}

//	SelectObject( hMemDC, hOldBitmap );
   
//	if( hOldPal1 )
//		SelectPalette( hMemDC, hOldPal1, FALSE );

//	if( hOldPal2 )
//		SelectPalette( hDC, hOldPal2, FALSE );

//	DeleteDC( hMemDC );

//}

#ifdef	WJPEG4
	dwErr = 0;
	if( hgInfo = lpDIBInfo->hgGIFSize )
	{
		LPGIFHDREXT	lpGHE;
		LPGIFIMGEXT	lpGIE;
		WORD	w, max;
		RECT	rc;
		HGDIOBJ	hgo, hgoo;
		HPALETTE	hPal2;
		DWORD		dwRop;

		hgoo = hDDB;
		dwRop = SRCCOPY;
		if( lpGHE = (LPGIFHDREXT)DVGlobalLock( hgInfo ) )
		{
			if( max = lpGHE->gheImgCount )
			{
				for( w = 0; w < max; w++ )
				{
					lpGIE = &lpGHE->gheGIE[w];
					if( lpGIE->hDIB &&
						( hDDB = lpGIE->hBitmap ) )
					{
						//if( lpGIE->giBits & 
						if( lpGIE->gceBits & gce_TransColr ) // Transparent Color Flag 1 Bit
							dwRop = SRCPAINT;
						else
							dwRop = SRCCOPY;

						GetObject( hDDB,
							sizeof (Bitmap),
							(LPSTR) &Bitmap);
						rc.left = lpGIE->giLeft;
						rc.top  = lpGIE->giTop;
						rc.right = rc.left + lpGIE->giGI.giWidth;
						rc.bottom = rc.top + lpGIE->giGI.giHeight;
						rectDDB.left   = 0;
						rectDDB.top    = 0;
						rectDDB.right  = Bitmap.bmWidth;
						rectDDB.bottom = Bitmap.bmHeight;
						//if( Bitmap.bmWidth != lpGIE->giGI.giWidth )
						//	rectDDB.bottom = lpGIE->giGI.giWidth;
						//if( Bitmap.bmHeight != lpGIE->giGI.giHeight )
						//	rectDDB.right = lpGIE->giGI.giHeight;
// =================================================
//						DDBPaint( lpDIBInfo, hDC, &rc, lpGIE->hBitmap, &rectDDB,
//							lpGIE->hPal, dwRop );
// pppppppppppppppppp
	if( hPal2 = lpGIE->hPal )
	{
//		SelectPalette( hDC, hPal2, TRUE );
//		RealizePalette( hDC );
//		SelectPalette( hMemDC, hPal2, FALSE );
//		SelectPalette( hDC, hPal2, FALSE );
	}
// pppppppppppppppppp
	hgo = SelectObject( hMemDC, hDDB );
	if( (hgo == 0 ) ||
		(hgo == (HGDIOBJ)GDI_ERROR) )
	{
		dwErr = GetLastError();
	}
	else if( hgo != hgoo )
	{
		chkp1();
	}
	hgoo = hDDB;

	lpDCRect = &rc;
	lpDDBRect = &rectDDB;
	if( ( rc.left < rectC.right ) &&
		( rc.top  < rectC.bottom ) &&
		( rc.right > rc.left ) &&
		( rc.bottom > rc.top ) )
	{
	  if( (RECTWIDTH (lpDCRect)  == RECTWIDTH (lpDDBRect) ) &&
		(RECTHEIGHT (lpDCRect) == RECTHEIGHT (lpDDBRect)) )
	  {
		flg = BitBlt( hDC,
			lpDCRect->left,
			lpDCRect->top,
			lpDCRect->right - lpDCRect->left,
			lpDCRect->bottom - lpDCRect->top,
			hMemDC,
			lpDDBRect->left,
			lpDDBRect->top,
			dwRop ); //DSTINVERT );	//SRCCOPY );
	  }
	  else
	  {
		flg = StretchBlt( hDC,
			lpDCRect->left,
			lpDCRect->top,
			lpDCRect->right - lpDCRect->left,
			lpDCRect->bottom - lpDCRect->top,
			hMemDC,
			lpDDBRect->left,
			lpDDBRect->top,
			lpDDBRect->right - lpDDBRect->left,
			lpDDBRect->bottom - lpDDBRect->top,
			dwRop ); //DSTINVERT ); //SRCCOPY );
	  }
	  //PutBox( hDC, lpDDBRect, 1 );
	}
	else
	{
		PutBox( hDC, lpDDBRect, 1 );
	}
	if( flg == 0 )
	{
		if( dwErr == 0 )
			dwErr = GetLastError();
		if( dwErr == 0 )
			dwErr = 10;
	}
// =================================================
					}
					else
					{
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
//			lpGIE->gceRes1 = (DWORD)LM_to_uint(lpb[8],lpb[9]); // Char CELL
//			lpGIE->gceRes2 = (DWORD)lpb[10];	// Foreground index
//			lpGIE->gceIndex = lpb[11];	// Backgound INDEX
//	DWORD	gceColr;	// COLORREF (if SET)
					}
				}
			}
			DVGlobalUnlock( hgInfo );
		}
	}
//	if( dwErr )
//	{
//		ShowPErr( hWnd, hDC, dwErr );
//	}
#endif	// WJPEG4

	// Draw the clipboard selection rubber-band.
#ifndef	WIN32
	SetWindowOrg( hDC,
		GetScrollPos( hWnd, SB_HORZ ),
		GetScrollPos( hWnd, SB_VERT ) );
#endif	// !WIN32
	DrawSelect( hDC, &lpDIBInfo->rcClip );

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
#endif

	if( hOldPal )
		SelectPalette( hDC, hOldPal, FALSE );

#ifdef	TRYCOMP
	if( hbmold )
		SelectObject( hDC, hbmold );
	DeleteObject( hbm );
	DeleteDC( hDC );
#endif

	DVGlobalUnlock (hDIBInfo);

	if( hOldBitmap )
		SelectObject( hMemDC, hOldBitmap );
   
	if( hOldPal1 )
		SelectPalette( hMemDC, hOldPal1, FALSE );

	if( hOldPal2 )
		SelectPalette( hDC, hOldPal2, FALSE );

	if( hMemDC )
		DeleteDC( hMemDC );

#ifdef	WJPEG4
	if( dwErr )
	{
		ShowPErr( hWnd, rhDC, dwErr );
	}
#endif	// WJPEG4

ABORTPAINT:

	EndPaint( hWnd, &ps );

	//Hourglass (FALSE);

}	// End - void ChildWndPaint2( HWND )
#endif	// WIN32
     ========== Above JUST for some TESTS ============= */

// DvUnused.c
