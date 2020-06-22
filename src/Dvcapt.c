
/*
 * DvCapt.c
 *
 * Contains code for the following functions used in DibView:
 *
 *  CopyWindowToDIB()   - Copies a window to a DIB
 *  CopyScreenToDIB()   - Copies entire screen to a DIB
 *  DestroyDIB()        - Deletes DIB when finished using it
 *  SelectWindow()      - Allows user to point to a window on the screen
 *
 * Copyright (c) 1991 Microsoft Corporation. All rights reserved.
 *
 */

#include	"dv.h"	// All inclusinve include ...
#include	"DvCapt.h"

extern   int DVPalEntriesOnDevice( HDC hDC );

// Prototypes for local functions

HBITMAP CopyScreenToBitmap(LPRECT lpRect);
void HighlightWindow(HWND hwnd, BOOL fDraw);
VOID	DVHideWindow( HWND hWnd );

// Locally used "globals"
BOOL  gbHide    = TRUE;   // TRUE if MAIN window is to be hidden on capture
BOOL  gbChgHide = FALSE;

//#define gbInSelWin W.w_bInSelWin // g BOOL = ADDSELWIN2  = global 'sel' proc
#ifdef   ADDSELWIN2


#else    // !#ifdef   ADDSELWIN2

// ============ failed code ======== to be deleted, after ext/dup funcs done
#if   0
/***********************************************************************
 *
 * SelectWindow()
 *
 * This function allows the user to select a window on the screen.  The
 * cursor is changed to a custom cursor, then the user clicks on the title
 * bar of a window to capture, and the handle to this window is returned.
 *
 **********************************************************************/

HWND SelectWindow()
{
   HCURSOR hCursor, hOldCursor;     // Handle to old cursor
   POINT   pt;               // Stores mouse position on a mouse click
   HWND    hWndClicked;       // Window we clicked on
   MSG     msg;
   HWND     hWnd = ghMainWnd;
  
    /*
     * Capture all mouse messages
     */

    //SetCapture( ghWndMDIClient ); // why to MDI client????
   SetCapture( hWnd );  // main window = the frame - looks better
    /*
     * Load custom Cursor
     */
   hCursor = LoadCursor( ghDvInst, "SELECTCUR");
   if(hCursor)
      hOldCursor = SetCursor(hCursor);
   else
   {
      hOldCursor = 0;
      //hOldCursor = SetCursor(LoadCursor( ghDvInst, "SelectCur"));
      sprtf( "Warning: Failed to LOAD SELECTCUR!"MEOR );
   }

    /*
     * Eat mouse messages until a WM_LBUTTONUP is encountered.
     */
    for(;;)
    {
       WaitMessage();
       while( PeekMessage( &msg,NULL,0,0,PM_REMOVE ) != 0 )
       {
//       if( PeekMessage(&msg,NULL,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE) )
//       {
          if (msg.message == WM_LBUTTONUP)
          {
//          if (msg.message == WM_LBUTTONDOWN)
//          {
             /*
              * Get mouse position
              */

             pt.x = LOWORD(msg.lParam);
             pt.y = HIWORD(msg.lParam);

             /*
              * Convert to screen coordinates
              */

             //ClientToScreen( ghWndMDIClient, &pt);
             ClientToScreen( hWnd, &pt);

             /*
              * Get Window that we clicked on
              */

             hWndClicked = WindowFromPoint(pt);

             /*
              * If it's not a valid window, just return NULL
              */

             if(!hWndClicked)
             {
                 ReleaseCapture();
                 if(hOldCursor)
                    SetCursor(hOldCursor);
                 sprtf( "SelectWindow() returning NULL!"MEOR );
                 return NULL;
             }
             break;
          }

   	    TranslateMessage(&msg);
	   	 DispatchMessage(&msg);

       }
//       else
//          continue;  // perhaps not really necessary, since is a closed loop anyway

    } // forever loop

    ReleaseCapture();

    if( hOldCursor )
       SetCursor(hOldCursor);

    sprtf( "SelectWindow() returning %#x."MEOR, hWndClicked );

    return (hWndClicked);
}


/*************************************************************************
 *
 * CaptureWindow
 *
 * Parameters:
 *
 * HWND hWndParent  - specifies the "parent" window.  If gbHide, this window
 *                    is hidden during the capture operation.
 *
 * BOOL bCaptureClient - TRUE == Capture client area of window only.
 *                      FALSE == Capture entire area of window.
 *
 * Return Value:
 *
 * HDIB             - identifies the device-independent bitmap
 *
 * Description:
 *
 * This function copies allows the user to select a window to capture.
 * it then creates a device independent bitmap for this window, and
 * returns it.
 *
 ************************************************************************/

HANDLE CaptureWindow (HWND hWndParent, BOOL bCaptureClient)
{
   // User wants to capture a window, either the entire window
   // (including title bar and such), or the client area only
 
   HWND hWndSelect;              // Handle to window which was selected
   HWND hWndDesktop;             // Handle to desktop window
   HDIB hDib;                    // Handle to memory containing DIB


   // Hide the DIBVIEW window
   if( gbHide )
	   DVHideWindow( ghMainWnd );    //hWndParent );

   sprtf( "Entering SelectWindow(%#x) ... "MEOR, ghMainWnd );
   // Ask user to select a window
   hWndSelect = SelectWindow();

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

#endif   // #if   0

#endif   // #ifdef   ADDSELWIN2 y/n - devel new, hopefully better code for this


/*************************************************************************
 *
 * CopyWindowToDIB()
 *
 * Parameters:
 *
 * HWND hWnd        - specifies the window
 *
 * WORD fPrintArea  - specifies the window area to copy into the device-
 *                    independent bitmap
 *
 * Return Value:
 *
 * HDIB             - identifies the device-independent bitmap
 *
 * Description:
 *
 * This function copies the specified part(s) of the window to a device-
 * independent bitmap.
 *
 ************************************************************************/


HDIB CopyWindowToDIB(HWND hWnd, DWORD fPrintArea)
{
  HDIB hDIB=NULL;  // handle to DIB

  /* check for a valid window handle */
  if (!hWnd)
    return NULL;

  switch (fPrintArea)
  {
    case PW_WINDOW: // copy entire window
    {
      RECT rectWnd;

      /* get the window rectangle */
      GetWindowRect(hWnd, &rectWnd);

      /*  get the DIB of the window by calling
       *  CopyScreenToDIB and passing it the window rect
       */
      hDIB = CopyScreenToDIB(&rectWnd);
    }
    break;

    case PW_CLIENT: // copy client area
    {
      RECT rectClient;
      POINT pt1, pt2;

      /* get the client area dimensions */
      GetClientRect(hWnd, &rectClient);

      /* convert client coords to screen coords */
      pt1.x = rectClient.left;
      pt1.y = rectClient.top;
      pt2.x = rectClient.right;
      pt2.y = rectClient.bottom;
      ClientToScreen(hWnd, &pt1);
      ClientToScreen(hWnd, &pt2);
      rectClient.left   = pt1.x;
      rectClient.top    = pt1.y;
      rectClient.right  = pt2.x;
      rectClient.bottom = pt2.y;

      /*  get the DIB of the client area by calling
       *  CopyScreenToDIB and passing it the client rect
       */
      hDIB = CopyScreenToDIB(&rectClient);
    }
    break;

    default:    // invalid print area
      return NULL;
  }

  /* return the handle to the DIB */
  return hDIB;
}


/* ************************************************************************
 *
 * CopyScreenToDIB()
 *
 * Parameter:
 *
 * LPRECT lpRect    - specifies the window
 *
 * Return Value:
 *
 * HDIB             - identifies the device-independent bitmap
 *
 * Description:
 *
 * This function copies the specified part of the screen to a device-
 * independent bitmap.
 * Called during IDM_CAPTFULLSCREEN and others
 *
 *********************************************************************** */

HDIB CopyScreenToDIB(LPRECT lpRect)
{
  HBITMAP hBitmap;    // handle to device-dependent bitmap
  HPALETTE hPalette;  // handle to palette
  HDIB hDIB=NULL;     // handle to DIB

  /*  get the device-dependent bitmap in lpRect by calling
   *  CopyScreenToBitmap and passing it the rectangle to grab
   */
  hBitmap = CopyScreenToBitmap(lpRect);

  /* check for a valid bitmap handle */
  if (!hBitmap)
    return NULL;

  /* get the current palette */
  hPalette = DVGetSystemPalette( TRUE );

  /* convert the bitmap to a DIB */
  hDIB = DVBitmapToDIB(hBitmap, hPalette);

  /* clean up */
  DeleteObject(hBitmap);
  DeleteObject(hPalette);

  /* return handle to the packed-DIB */
  return hDIB;

}




/*************************************************************************
 *
 * DestroyDIB ()
 *
 * Purpose:  Frees memory associated with a DIB
 *
 * Returns:  Nothing
 *
 *************************************************************************/

WORD DestroyDIB(HDIB hDib)
{
  DVGlobalFree(hDib);
  return 0;
}

/* ******************************************************************
 *
 * CopyScreenToBitmap()
 *
 * Parameter:
 *
 * LPRECT lpRect    - specifies the window
 *
 * Return Value:
 *
 * HDIB             - identifies the device-dependent bitmap
 *
 * Description:
 *
 * This function copies the specified part of the screen to a device-
 * dependent bitmap.
 * Called during IDM_CAPTFULLSCREEN for examples, and others
 *
 * ***************************************************************** */

HBITMAP CopyScreenToBitmap(LPRECT lpRect)
{
	HDC hScrDC, hMemDC;           // screen DC and memory DC
	HBITMAP hBitmap, hOldBitmap;  // handles to deice-dependent bitmaps
	int nX, nY, nX2, nY2;         // coordinates of rectangle to grab
	int nWidth, nHeight;          // DIB width and height
	int xScrn, yScrn;             // screen resolution

	// check for an empty rectangle
	if( IsRectEmpty( lpRect ) )
		return NULL;

	// create a DC for the screen and create
	// a memory DC compatible to screen DC
	hScrDC = CreateDC( "DISPLAY", NULL, NULL, NULL );
	if( !hScrDC )	// If no memory DC
		return NULL;

	hMemDC = CreateCompatibleDC( hScrDC );
	if( !hMemDC )
	{
		// No compatible DC
		DeleteDC( hScrDC );
		return NULL;
	}

	// get points of rectangle to grab
	nX  = lpRect->left;
	nY  = lpRect->top;
	nX2 = lpRect->right;
	nY2 = lpRect->bottom;

	// get screen resolution
	xScrn = GetDeviceCaps( hScrDC, HORZRES );
	yScrn = GetDeviceCaps( hScrDC, VERTRES );

	// make sure bitmap rectangle is visible
	if( nX < 0 )
		nX = 0;
	if( nY < 0 )
		nY = 0;

	if( nX2 > xScrn )
		nX2 = xScrn;
	if( nY2 > yScrn )
		nY2 = yScrn;

	nWidth  = nX2 - nX;
	nHeight = nY2 - nY;

	// create a bitmap compatible with the screen DC
	hBitmap = CreateCompatibleBitmap( hScrDC, nWidth, nHeight );
	if( !hBitmap )
	{
		DeleteDC( hScrDC );
		DeleteDC( hMemDC );
		return NULL;
	}

	// select new bitmap into memory DC
	hOldBitmap = SelectObject( hMemDC, hBitmap );

	// bitblt screen DC to memory DC
	BitBlt( hMemDC,	// Destination
		0, 0,
		nWidth, nHeight,	// Size
		hScrDC,				// Source
		nX, nY,				// from location
		SRCCOPY );			// COPY operation

	// select old bitmap back into memory DC and get handle to
	// bitmap of the screen
	hBitmap = SelectObject( hMemDC, hOldBitmap );

	// clean up
	DeleteDC( hScrDC );
	DeleteDC( hMemDC );

	// return handle to the bitmap
	return hBitmap;

}

VOID	DVHideWindow( HWND hWnd )
{
	MSG	msg;	// Message for the PeekMessage()
   INT   iCnt = 0;
   sprtf( "HIDING Window %#x ..."MEOR, hWnd );
//   CloseWindow( hWnd );
	ShowWindow( hWnd, SW_HIDE );

	// Wait here until the apps on the screen have a chance to
	// repaint themselves.  Make our own message loop, and process
	// the messages we see.  When PeekMessage() returns 0, we know
	// that all the other app's WM_PAINT messages have been
	// processed.
	while( PeekMessage( &msg,NULL,0,0,PM_REMOVE ) != 0 )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
      iCnt++;
	}
   Sleep( 10 );
	while( PeekMessage( &msg,NULL,0,0,PM_REMOVE ) != 0 )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
      iCnt++;
	}
   Sleep( 10 );
	while( PeekMessage( &msg,NULL,0,0,PM_REMOVE ) != 0 )
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
      iCnt++;
	}

   if( iCnt )
      sprtf( "Processed %d messages during HIDE process."MEOR, iCnt );
   else
      sprtf( "HUH! Processed ZERo messages during HIDE process."MEOR, iCnt );

}

/*************************************************************************
 * CaptureFullScreen
 * Parameters:
 * HWND hWndParent  - specifies the "parent" window.  If gbHide, this window
 *                    is hidden during the capture operation.
 * Return Value:
 * HDIB             - identifies the device-independent bitmap
 * Description:
 * This function copies the entire screen into a DIB, returning a
 * handle to the global memory with the DIB in it.
 * It is invoked from the MENU item - IDM_CAPTFULLSCREEN
 *
 ************************************************************************/

HANDLE CaptureFullScreen (HWND hWndParent)
{
   RECT rScreen;        // Rect containing entire screen
   HDC  hDC;            // DC to screen
   HDIB hDib = 0;       // Handle to DIB 

   // Create a DC for the display
   hDC = CreateDC("DISPLAY", NULL, NULL, NULL); 
   if(hDC)
   {
      // Hide the DIBVIEW window
      if( gbHide )
	      DVHideWindow( ghMainWnd );    // hWndParent );

      rScreen.left   = rScreen.top = 0;
      rScreen.right  = GetDeviceCaps(hDC, HORZRES);
      rScreen.bottom = GetDeviceCaps(hDC, VERTRES);

      // Call the function which captures screen
      hDib = CopyScreenToDIB( &rScreen );

      // Restore the DIBVIEW window
      if( gbHide ) 
         ShowWindow( ghMainWnd, SW_SHOW );   // hWndParent, SW_SHOW);

      DeleteDC(hDC);

   }

   return hDib;

}



/* ************************************************************************
 * ToggleCaptureHide
 *
 * Parameters:
 *    None
 *
 * Return Value:
 *    New "Hide" state.
 *
 * Description:
 *    Toggle the "Hide window on capture" option
 *
 * ********************************************************************** */
BOOL ToggleCaptureHide (void)    // on IDM_CAPTUREHIDE
{
	if( gbHide )
   {
		gbHide = FALSE;
      gbChgHide = TRUE;
   }
	else
   {
		gbHide = TRUE;
      gbChgHide = TRUE;
   }

   return gbHide;
}


/* ===================================
   ************************************************************************
   JUST FOR PRINTER HDC
   ************************************************************************
   =================================== */

HBITMAP CopyHDCToBitmap(HDC hScrDC, LPRECT lpRect)
{
	HDC hMemDC;                   // memory DC
	HBITMAP hBitmap, hOldBitmap;  // handles to deice-dependent bitmaps
	int nX, nY, nX2, nY2;         // coordinates of rectangle to grab
	int nWidth, nHeight;          // DIB width and height
	int xScrn, yScrn;             // screen resolution

	// check for an empty rectangle
	if( IsRectEmpty( lpRect ) )
		return NULL;

	hMemDC = CreateCompatibleDC( hScrDC );
	if( !hMemDC )
	{
		// No compatible DC
		return NULL;
	}

	// get points of rectangle to grab
	nX  = lpRect->left;
	nY  = lpRect->top;
	nX2 = lpRect->right;
	nY2 = lpRect->bottom;

	// get screen resolution
	xScrn = GetDeviceCaps( hScrDC, HORZRES );
	yScrn = GetDeviceCaps( hScrDC, VERTRES );

   // BUT for PRINTER devices maybe should use
   // ===============================================
   nWidth  = GetDeviceCaps( hScrDC, PHYSICALWIDTH  );
   nHeight = GetDeviceCaps( hScrDC, PHYSICALHEIGHT );
   if( nWidth > xScrn )
      xScrn = nWidth;
   if( nHeight > yScrn )
      yScrn = nHeight;
   // ===============================================

	// make sure bitmap rectangle is visible
	if( nX < 0 )
		nX = 0;
	if( nY < 0 )
		nY = 0;

	if( nX2 > xScrn )
		nX2 = xScrn;
	if( nY2 > yScrn )
		nY2 = yScrn;

   if( ( nX  != lpRect->left   ) ||
       ( nY  != lpRect->top    ) ||
       ( nX2 != lpRect->right  ) ||
       (	nY2 != lpRect->bottom ) )
   {
      RECT  rc;
      rc.left   = nX;
      rc.top    = nY;
      rc.right  = nX2;
      rc.bottom = nY2;
      sprtf( "CopyHDCToBitmap(): Adjust rectangle from %s to %s!"MEOR,
         Rect2Stg( lpRect ),
         Rect2Stg( &rc ) );
   }

	nWidth  = nX2 - nX;
	nHeight = nY2 - nY;

	// create a bitmap compatible with the screen DC
   sprtf( "Creating bitmap of %d x %d compatible to hdc %#x."MEOR,
      nWidth, nHeight, hScrDC );
	hBitmap = CreateCompatibleBitmap( hScrDC, nWidth, nHeight );
	if( !hBitmap )
	{
		DeleteDC( hMemDC );
		return NULL;
	}

	// select new bitmap into memory DC
	hOldBitmap = SelectObject( hMemDC, hBitmap );

	// bitblt screen DC to memory DC
	BitBlt( hMemDC,	// Destination
		0, 0,
		nWidth, nHeight,	// Size
		hScrDC,				// Source
		nX, nY,				// from location
		SRCCOPY );			// COPY operation

	// select old bitmap back into memory DC and get handle to
	// bitmap of the screen
	hBitmap = SelectObject( hMemDC, hOldBitmap );

	// clean up
	DeleteDC( hMemDC );

	// return handle to the bitmap
	return hBitmap;

}


HPALETTE DVGetHDCPalette( HDC hDC, BOOL bShwErr )
{
	HPALETTE	      hPal;
	HANDLE		   hLogPal;
	LPLOGPALETTE   lpLogPal;
	int			   i, nColors;
	LPDWORD		   lpdw;

	hPal = NULL;	// NO, we do not have a PALETTE

	// Find out how many palette entries we want.
	// hDC = GetDC( NULL );
	if( !hDC )
	{
		return NULL;
	}

	nColors = DVPalEntriesOnDevice( hDC );

	//ReleaseDC (NULL, hDC);

	// Allocate room for the palette and lock it.
	if( ( nColors != (int) -1 ) &&		// FIX980412
	    ( nColors <= (int)256 ) )
	{
		hLogPal = DVGlobalAlloc (GHND, sizeof (LOGPALETTE) +
			nColors * sizeof (PALETTEENTRY) );
		if( !hLogPal )
		{
         if( bShwErr )
         {
			   DIBError( ERR_MEMORY ); // ERR_CREATEPAL
         }
			return NULL;
		}

		lpLogPal = (LPLOGPALETTE)DVGlobalLock( hLogPal );
		lpLogPal->palVersion    = PALVERSION;
		lpLogPal->palNumEntries = nColors;
		for( i = 0;  i < nColors;  i++ )
		{
			lpdw = (LPDWORD)&lpLogPal->palPalEntry[i];
			lpLogPal->palPalEntry[i].peBlue  = 0;
			*((LPWORD) (&lpLogPal->palPalEntry[i].peRed)) = i;
			//*lpdw = (DWORD)i;
			lpLogPal->palPalEntry[i].peFlags = PC_EXPLICIT;
		}

		// Go ahead and create the palette.  Once it's created
		//  we no longer need the LOGPALETTE, so free it.
		hPal = CreatePalette( lpLogPal );
		DVGlobalUnlock( hLogPal );
		DVGlobalFree( hLogPal );
	}
	else
	{
      if( bShwErr )
      {
		   DIBError( ERR_CREATEPAL );
      }
		return NULL;
	}

	return hPal;
}

HANDLE CopyHDCToDIB(HDC hScrDC, LPRECT lpRect)
{
  HBITMAP   hBitmap;       // handle to device-dependent bitmap
  HPALETTE  hPalette;      // handle to palette
  HANDLE    hDIB = NULL;   // handle to DIB

  /*  get the device-dependent bitmap in lpRect by calling
   *  CopyScreenToBitmap and passing it the rectangle to grab
   */
  hBitmap = CopyHDCToBitmap( hScrDC, lpRect );

  /* check for a valid bitmap handle */
  if(!hBitmap)
    return NULL;

  /* get the current palette */
  //hPalette = DVGetSystemPalette( FALSE );
  hPalette = DVGetHDCPalette( hScrDC, FALSE );

  /* convert the bitmap to a DIB */
  hDIB = DVBitmapToDIB( hBitmap, hPalette );

  /* clean up */
  DeleteObject(hBitmap);
  DeleteObject(hPalette);

  /* return handle to the packed-DIB */
  return hDIB;

}

/* ***************************************************************
   END PRINTER SECTION
   *************************************************************** */


// eof - DvCapt.c
