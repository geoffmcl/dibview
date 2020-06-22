
/* **************************************************************

      File:  DVPAL.C

   Purpose:  Contains routines to display a palette in a popup window.
             Allows the user to scroll around the palette and dump info
             on particular colors in the palette.  Has much the same
             look as the Clipboard's palette dump.

 Functions:  PALETTEWNDPROC
             HighlightSquare   
             UnHighlightSquare
             PalRowsAndColumns 
             SetPaletteWindowsPal
             DVPalEntriesOnDevice
             DVGetSystemPalette
             ColorsInPalette
             MyAnimatePalette
             CopyPaletteChangingFlags

  Comments:  The routines in this module are provided mostly for
             debugging purposes.  There are definite improvements
             that can be made.  For example, the scroll bars now
             let you scroll way beyond the existing colors in a
             palette.  They should probably be limited to the number
             of rows to be displayed in the palette window.

   History:   Date      Reason
             6/ 1/91     Created
            11/15/91     Use LoadString instead of a hardcoded
                         array of strings.  This frees up some
                         DS, and is better for localizing for
                         international markets.
             1/28/92     Added WM_QUERYNEWPALETTE message,
                         and always select palette as
                         a background palette in WM_PAINT.

   ************************************************************ */
#include	"dv.h"
#include	"DvPal2.h"		/* just some specials for this MODULE */

extern   void	DVHideWindow( HWND hWnd );
extern   BOOL  gbHide;   // TRUE if MAIN window is to be hidden on capture

//#define  MIN_HISTO      120
#define  MIN_HISTO      240

/* ======================
	ELECTROMAGNETC RADIATION
	infrared region (1/1,000 to 1/1,000,000 meters),
	the visible region,
	the ultraviolet region (3/10,000,000 to 1/10,000,000,000/meters),
	or the X-ray region (1/100,000,000 to 1/100,000,000,000 meters).
	Gama is shorter ...

	beam and passed through a glass prism,
	the beam will be spread apart as it is resolved 
	into its different spectral components.
	The visible light of longest wavelength, which 
	the eye sees as red, is deviated the least, 
	and the visible light of shortest wavelength, blue,
	is deviated the most.
	This phenomenon is known as DISPERSION.

    In nonconducting gases, liquids, and solids the
	electrons are tightly bound in atoms and are only
	slightly displaced by the field of an electromagnetic wave.
	For such media the index of refraction is greater than,
	but close to, unity, the index of refraction of a vacuum.
	For example, for light in the visible part of the
	spectrum,
		n = 1.00029 for air,
		n = 1.333 for water, and 
		n = 1.5 to 1.9 for glass

	RAINBOWS -
	Arc du ceil - Arc in the sky as the french might say.

    The Sun's spectrum, surface temperature, and 
	color lead to its further classification as a G2 dwarf
	in the scheme of spectral types used by astronomers.
	The spectral intensity of light radiated by its surface
	gases is a maximum at wavelengths near 5000 angstroms,
	thus giving sunlight its characteristic yellow color.

	The luminance (brightness) signal is produced
	by adding together, electronically,
	the three signals from the color camera,
	in the approximate ratios
	30% red,
	60% green, and
	10% blue.
	This combination produces white light on the screen of
	the color receiver.  During a color telecast, this is
	what is seen when the color-intensity control of the
	receiver is turned off.

    The chrominance (color) signal is also derived
	from the three color signals produced by the camera,
	but by more elaborate electronic circuits.
	In effect, these circuits produce a signal that
	represents the difference between the luminance signal and
	the individual color signals from the camera.

	... it conveys variations in the hue (the
	redness, greenness, blueness, and so on) of the
	colors of each area of the picture screen.
	... maybe BRIGHTNESS

    ... it conveys variations in the saturation of the
	colors (their vivid versus pastel character).
	... maybe CONTRAST
	
	These aspects may be controlled to suit the viewer's taste.

   ====================== */

// #ifdef	FRMENU
/* menu is Fichier Edition Affichage ... */
//#define		FP_FILE		0
// etc
// #else	/* !FRMENU */
/* org. english menu for palette window */
// #define		PP_FILE		0
// etc
//#endif	/* FRMENU y/n */

//int nEntriesPerInch[4] = {15,               // Tiny mode  squares/inch
//                          10,               // Small mode squares/inch
//                           6,               // Medium mode squares/inch
//                           3};              // Large mode squares/inch
int nEntriesPerInch[MXENTRIES] = {10,	// Tiny mode  squares/inch
					6,		// Small mode squares/inch
					4,		// Medium mode squares/inch
					2 };	// Large mode squares/inch

BOOL	   fShwBT = TRUE;
UINT	   guiRMCnt = 0;
BOOL	   gfDrwPol = DEFDRWPOL;
static   BOOL bInSize = FALSE;
//char	gszPText[260];
int		giChgDrwP = 0;
// 	case IDM_PAL_HISTO:
int		giDrwHisto   = 1;    // draw a HISTOGRAM of COLORS
int		giChgHisto   = 0;
int		giDrwSquares = 1;
int		giChgSquares = 0;
int		gfAddPenC  = 1;
int		gfAddPenC2 = 0;
int		gfEraseBkGnd = 1;
int		gcxPixs;	// = GetDeviceCaps( hDC, HORZRES ); //Width, in pixels, of the screen.
int		gcyPixs;	// = GetDeviceCaps( hDC, VERTRES ); //Height, in raster lines, of the screen.
//char	gszSpl[256];	// colour from PALETTE (realised)
//TEXTMETRIC  gsTM;	// height, width of text at last check of HDC
int		gfAddMTxt  = 0;	/* no text for just color cycle */
int		gfAddCycle = 0;
int      gfAddCText = 0;   // add TEXT to histogram

extern	COLORREF	GetRYBColr( DWORD dwi );
// Local function prototypes.
int  DVPalEntriesOnDevice( HDC );	// Forward reference to ...
//void PalRowsAndColumns  (HWND hWnd, 
void PalRowsAndColumns(PPI,int,LPINT,LPINT,LPINT,LPINT);

void UnHighlightSquare( HDC hDC, 
                        DWORD wEntry, 
                        int cxSquare, 
                        int cySquare,
                        int nCols,
                        int nScroll );
void	HiSquare( HWND hWnd, HDC hDC,
				 PPI lpPalInfo, LPSTR lpb );

long PalWM_INITMENUPOPUP( HWND hWnd, UINT message,
						  WPARAM wParam, LPARAM lParam );
long PalWM_ERASEBKGND( HWND hWnd, UINT message,
						  WPARAM wParam, LPARAM lParam );

void	PaintRainbow( HDC hDC, PPI lpPalInfo, LPSTR lpb,
				   LPRECT lpr, int nScroll );
void	PaintColorCyc( HDC hDC, PPI lpPalInfo,
				   LPRECT lpr );

void	ShadeRect( HDC hDC, LPRECT lpr, int iSz, LPRECT lpret );
VOID  Do_IDM_PAL_PRINT( HWND hWnd );

void chkpp( void )
{
   int i;
	i = 0;
}
extern	HGLOBAL	CopyhCOLR( HGLOBAL hg );

BOOL	GotFlags( PPI lpPalInfo )
{
	BOOL	flg = FALSE;
	if( lpPalInfo->pi_iNormal & pif_Flags )
		flg = TRUE;
	return flg;
}
BOOL	IsFlagIntensity( PPI lpPalInfo )
{
	BOOL	flg = FALSE;
	if( lpPalInfo->pi_iNormal & pif_Intensity )
		flg = TRUE;
	return flg;
}

BOOL	IsFlagGray( PPI lpPalInfo )
{
	BOOL	flg = FALSE;
	if( lpPalInfo->pi_iNormal & pif_Gray )
		flg = TRUE;
	return flg;
}
BOOL	IsFlagFreq( PPI lpPalInfo )
{
	BOOL	flg = FALSE;
	if( lpPalInfo->pi_iNormal & pif_Freq )
		flg = TRUE;
	return flg;
}

BOOL	IsFlagRainbow( PPI lpPalInfo )
{
	BOOL	flg = FALSE;
	if( lpPalInfo->pi_iNormal & pif_Rainbow )
		flg = TRUE;
	return flg;
}

BOOL	IsFlagNumeric( PPI lpPalInfo )
{
	BOOL	flg = FALSE;
	if( lpPalInfo->pi_iNormal & pif_Numeric )
		flg = TRUE;
	return flg;
}

COLORREF	GetNEXTColr( PPI lpPalInfo, LPSTR lpb, DWORD dwi )
{
	COLORREF	cr;
	/* default is get the color found in BMP order */
	cr  = GetCOLR( lpb, dwi );

	return	cr;
}

LPSTR	SetgszStatus( DWORD dwCnt )
{
	LPSTR	lpt;
//		lstrcpy( lpt, "Logical / Physical Colors!" );
	lpt = &gszStatus[0];
//	if( gfDrwPol )
//	{
//		wsprintf( lpt,
//			"%d Ranked / Palette ",
//			dwCnt );	// number of Colors in bitmap
//	}
//	else
	{
		wsprintf( lpt,
			"%d Colors ",	//  per BMP order!",
			dwCnt );
	}

	return lpt;

}

void	ZeroDIResHands( LPDIBINFO lpDIBInfo )
{
	//lpDIBIfno->di_size = 0;	// Size of this structure
	//lpDIBinfo->di_hwnd = 0;	// Child Window handle owing this window
	
	lpDIBInfo->hDIB    = 0;		// Handle to the DIB
	lpDIBInfo->hPal    = 0;		// Handle to the bitmap's palette.
	lpDIBInfo->hBitmap = 0;	// Handle to the DDB.

	//WORD	wDIBType;	// DIB's type - RGB, RLE4, RLE8, PM
	//WORD	di_dwDIBBits;	// Bits per pixel
	//DWORD	di_dwDIBWidth;	// Width of the DIB
	//DWORD	di_dwDIBHeight;	// Height of the DIB

	lpDIBInfo->hBigFile = 0;	// If ANIMATING GIF, Keep whole file
	//WORD	wMaxCnt;	// Maximum number of images (in GIF)
	//WORD	wCurCnt;	// Current image 1 to MaxCnt
	//DWORD	dwMilCnt;	// Count to next movement/action
	//WORD	wBgnCnt;	// Beginning Anim. GIF ... NOT USED
	//WORD	wGifNum;	// The number of this GIF ...
	//WORD	wGifTot;	// The Total of this GIF set ...

	lpDIBInfo->hgGIFSize = 0;	// If a MULTIPLE image GIF, keep INFO
	//DWORD	dwGIFNext;	// ms to NEXT GIF action / or USER!!!

	//RECT	rcClip;		// Clipboard cut rectangle.

	//DWORD	dwDIBFlag;	// Various flags ... SEE ABOVE
	//RECT	rWindow;	// Windows Screen co-ordinates ...
	//RECT	rClient;	// Client Rectangle Size ...

	//OPTIONSINFO Options; // Option info from Options dialog box.

	//WIN32_FIND_DATA fdEditFile;	// file data

	lpDIBInfo->di_hDIB2    = 0;		// Secondary DIB
	lpDIBInfo->di_hBitmap2 = 0;	// Secondary BITMAP
	//RECT	di_rcDib2;		// Scroll and Size
	lpDIBInfo->di_hCOLR    = 0;		// Color count / distrib info

	//int		di_iVert, di_iHorz;
	//int		di_iOney, di_iOnex;

	//BOOL	di_bTooBig;		// Set when can not create BITMAP
							// equal to DIB size
	lpDIBInfo->di_hBigDib = 0;		// Handle to BIG DIB Information
	lpDIBInfo->di_hThread = 0;		// Handle to THREAD
	lpDIBInfo->di_dwThreadId = 0;	// Thread ID
	lpDIBInfo->di_dwThreadBit = 0;	// BIT signifying this thread

}
//typedef struct tagCLIENTCREATESTRUCT { // ccs  
//    HANDLE hWindowMenu; 
//    UINT   idFirstChild; 
//} CLIENTCREATESTRUCT; 
CLIENTCREATESTRUCT  pCliCre; 
RECT	grPosSize;

long PalWM_CREATE( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	long			   pret;
	HWND			   hInfoBar;
	HANDLE			hPalInfo;
	LPCREATESTRUCT	lpC;
	LPPWCS			lpPWCS;
	PPI		lpPalInfo;
	HANDLE			hDIBInfo, hDIB;
	LPDIBINFO		lpDIBInfo;
	LPSTR			   lpdi, lpcols, lpsrc;
	DWORD			   wColCnt;
	HGLOBAL			hCols, hg;
	LPBITMAPINFO	lpbmi;
	LPBITMAPINFOHEADER lpbmih;
	DWORD			   dwSize;
	LPSTR			   lpb;

	pret = 0;

	// Extract the passed Palette Window Create Structure (PWCS)
	lpC = (LPCREATESTRUCT) lParam;
//typedef struct	tagPWCS	{	/* pw */
//	HWND	pw_hMDI;	// "Parent" MDI Child (But can be DESTROYED)
//	HGLOBAL	pw_hDIBInfo;// Likewise - DIB Info of child, but can be DESTROYED
//}PWCS;
//typedef PWCS MLPTR LPPWCS;
	if( !( lpPWCS = (LPPWCS) lpC->lpCreateParams ) )
      return -1;  // abandon the creation

	grPosSize.left   = 0;
	grPosSize.top    = 0;
	grPosSize.right  = 100;
	grPosSize.bottom = 30;

	pCliCre.hWindowMenu = 0;
	pCliCre.idFirstChild = IDM_WINDOWCHILD;
	/* Create an INFORMATION STATUS LINE child control window */
	/* ====================================================== */
//#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
//#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4))
//#define IS_WIN95 (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32
#ifdef	WIN32
	hInfoBar = CreateWindowEx( WS_EX_NOPARENTNOTIFY,
		"static",   /* Class of CONTROL ... */
		NULL,            /* Text (if any) ... */
		WS_CHILD | WS_VISIBLE | SS_CENTER, /* Style ... */
		grPosSize.left,               /* Position and */
		grPosSize.top,
		grPosSize.right,      /* Size ... */
		grPosSize.bottom,
		hWnd,           /* Parent Window = Owner */
		(HMENU)ID_INFO_CHILD, /* Menu or Child id */
		ghDvInst,
		NULL );

#else	// !WIN32
	hInfoBar = CreateWindow ("static",       // Class
		NULL,           // Name
		WS_CHILD |      // Styles
		WS_VISIBLE |
		SS_CENTER,
		grPosSize.left,      // x (set in WM_SIZE)
		grPosSize.top,       // y (set in WM_SIZE)
		grPosSize.right,     // Width (set in WM_SIZE)
		grPosSize.bottom,    // Height (set in WM_SIZE)
		hWnd,           // hParent
		ID_INFO_CHILD,  // Child Window ID
		ghDvInst,          // Instance
		&pCliCre );  // create struct.
#endif	// WIN32 y/n

	if( ( hPalInfo  = DVGAlloc( "sPALINFO", GHND, sizeof(PALINFO) ) ) &&
		 ( lpPalInfo	= (PPI) DVGlobalLock (hPalInfo)     ) )
	{
      // got the child palette window memory

      ZeroMemory( lpPalInfo, sizeof(PALINFO) );
		lpPalInfo->pi_hWnd      = hWnd;	// our window handle
		//lpPalInfo->hPal		   = NULL;
		//lpPalInfo->wPEntries    = 0;	// Start with NO Entries
		lpPalInfo->nSquareSize  = PAL_SIZE_DEF - IDM_PAL_TINY;
		lpPalInfo->hInfoWnd	   = hInfoBar;
		//lpPalInfo->nRows	      = 0;
		//lpPalInfo->nCols	      = 0;
		//lpPalInfo->cxSquare	   = 0;
		//lpPalInfo->cySquare	   = 0;
		//lpPalInfo->wEntry	      = 0;

		//lpPalInfo->pi_iNormal   = 0;	/* just paint in sequence   */
		//lpPalInfo->pi_iInvert   = 0;	/* invert the current order */
		//lpPalInfo->pi_hCopyPal  = 0;	/* COPY of palette - after pnt */
		//lpPalInfo->pi_hCopyCOLR = 0; /* copy of COLOUR count array */
		//lpPalInfo->pi_hSortCOLR = 0; /* sort of COLOUR count array */

		lpPalInfo->hMdi		   = lpPWCS->pw_hMDI;

		hCols   = 0;
		wColCnt = 0;
		hg      = 0;
		lpb     = 0;
		//if( hDIBInfo = (HANDLE) HIWORD( lpvI ) )
		if( hDIBInfo = lpPWCS->pw_hDIBInfo )
		{
			if( lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo ) )
			{
            /*
             * DIBINFO pi_sCopyDI;	-- copy of DIB information, but take care
             * since the MDI child can be CLOSED and certain items would then
             * become INVALID!!!
             */
				memcpy( &lpPalInfo->pi_sCopyDI, lpDIBInfo, sizeof(DIBINFO) );
				ZeroDIResHands( &lpPalInfo->pi_sCopyDI );
				if( hDIB = lpDIBInfo->hDIB )
				{
					if( lpdi = DVGlobalLock( hDIB ) )   // LOCK DIB HANDLE
					{
						dwSize =
							(DWORD)( ( WIDTHBYTES( lpDIBInfo->di_dwDIBWidth *
								lpDIBInfo->di_dwDIBBits ) ) *
								lpDIBInfo->di_dwDIBHeight );
						lpbmi = (LPBITMAPINFO) lpdi;
						lpbmih = (LPBITMAPINFOHEADER) lpdi;
						if( ( lpbmih->biSize == sizeof(BITMAPINFOHEADER) ) &&
							( wColCnt = DIBNumColors( lpdi ) ) )
						{
							if( hCols = DVGlobalAlloc( GHND, (wColCnt * sizeof(RGBQUAD)) ) )
							{
								if( lpcols = DVGlobalLock( hCols ) )
								{
									lpsrc = (LPSTR) &lpbmi->bmiColors[0];
									dv_fmemcpy( lpcols, lpsrc, (wColCnt * sizeof(RGBQUAD)));
									DVGlobalUnlock( hCols );
								}
								else
								{
									DVGlobalFree( hCols );
									hCols = 0;
									DIBError( ERR_MEMORY );
								}
							}
							else
							{
								DIBError( ERR_MEMORY );
							}
						}
						if( lpDIBInfo->di_hCOLR == 0 )
						{
							GetUCCnt( lpDIBInfo,
								FindDIBBits( lpdi ),
								DIBNumColors( lpdi ),
								dwSize,
								lpDIBInfo->di_dwDIBWidth,
								lpDIBInfo->di_dwDIBHeight,
								lpDIBInfo->di_dwDIBBits,
								lpdi );
						}
						if( hg = lpDIBInfo->di_hCOLR )
						{
							//HGLOBAL pi_hCopyCOLR;	-- handle of COPY of PALEX array
							lpPalInfo->pi_hCopyCOLR =	/* handle of COPY of PALEX array */
								CopyhCOLR( hg );
							lpPalInfo->pi_hSortCOLR =	/* handle of COPY of PALEX array */
								CopyhCOLR( hg );
						}

						DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE
					}
				}

				DVGlobalUnlock( hDIBInfo );
			}
		}

		lpPalInfo->hDInf    = hDIBInfo;
		lpPalInfo->wDColCnt = wColCnt;	// Bitmap COLOR COUNT
		lpPalInfo->hCols    = hCols;
		if( ( hg = lpPalInfo->pi_hCopyCOLR ) &&
			 ( lpb = DVGlobalLock(hg)       ) )
		{
			lpPalInfo->pi_dwCnt = GetCOLRCnt(lpb);
			DVGlobalUnlock(hg);
//			SetgszStatus( lpPalInfo->pi_dwCnt );
         // INITIAL SORT
			DoSORT( lpPalInfo );	/* get em in order desired */
		}
		else
		{
			chkit( "YIPES! COPY COLOUR COUNT DIDN'T CUT IT!!!" );
		}
		DVGlobalUnlock( hPalInfo );
	}
	else
	{
		DIBError( ERR_MEMORY );
	}

	SetWindowExtra( hWnd, WW_PAL_HPALINFO, hPalInfo );

	// Set the palette square size to the default value.
	//  This must be done AFTER the PALINFO structure's
	//  handle is put into the window words.
	SendMessage( hWnd, WM_COMMAND, PAL_SIZE_DEF, 0L );

	return( pret );

}

//RECT		grInfoBar;
//TEXTMETRIC  gtm;
// WM_SIZE
long PalWM_SIZE( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	long	pret;
	HANDLE      hPalInfo;
	PPI   lpPalInfo;
	HWND        hInfoBar;
	HDC         hDC;
	RECT        rect;
	LPSTR		lpd;
	LPRECT		lpir;
	WPARAM		wPType;
	WORD		nWidth,nHeight;

	lpd  = &gszDiag[0];
	pret = 0;
	wPType  = wParam;			// resizing flag 
	nWidth  = LOWORD(lParam);	// width of client area 
	nHeight = HIWORD(lParam);	// height of client area 
	wsprintf( lpd,
		"PAL WM_SIZE of %d by %d ...",
		nWidth,
		nHeight );
	DO(lpd);
	// We implement a semaphore here to avoid an
	//  infinte loop.  Scroll bars are set up in
	//  PalRowsAndColumns().  This, in turn, can
	//  effect the size of the window's client area.
	//  Thus, sending the window another WM_SIZE
	//  message.  We ignore this 2nd WM_SIZE message
	//  if we're already processing one.
	if( bInSize )
	{
		lstrcpy( lpd, "NOTE: Avoided re-entrancy into WM_SIZE."MEOR );
		DO(lpd);
		return( pret );
	}
	bInSize = TRUE;

	// Get a handle to the info bar window.
	hPalInfo = GetWindowExtra( hWnd, WW_PAL_HPALINFO );
	if( !hPalInfo )
	{
		bInSize = FALSE;
		lstrcpy( lpd, "ERROR: Failed to get HANDLE???"MEOR );
		DO(lpd);
		chkit(lpd);
		return( pret );
	}

	lpPalInfo = (PPI) DVGlobalLock (hPalInfo);
	if( !lpPalInfo )
	{
		bInSize = FALSE;
		lstrcpy( lpd, "ERROR: Memory Failed for pointer???"MEOR );
		DO(lpd);
		chkit(lpd);
		return( pret );
	}

	/* now lpPalInfo is established ... */
	lpir      = &grInfoBar;
	hInfoBar  = lpPalInfo->hInfoWnd;
	lpPalInfo->pi_rClient.right  = nWidth;
	lpPalInfo->pi_rClient.bottom = nHeight;
	GetClientRect( hWnd, &grCli );

//	PalRowsAndColumns( hWnd, lpPalInfo->nSquareSize,
	PalRowsAndColumns( lpPalInfo,
      lpPalInfo->nSquareSize,
      &lpPalInfo->nCols, &lpPalInfo->nRows,
      &lpPalInfo->cxSquare, &lpPalInfo->cySquare);

//	DVGlobalUnlock (hPalInfo);
	if( !hInfoBar )
	{
		DVGlobalUnlock (hPalInfo);
		bInSize = FALSE;
		return( pret );
	}

	// The size of the info bar is dependent on the size of the
	//  system font.
	hDC = GetDC( NULL );
	GetTextMetrics( hDC, &gtm );
	ReleaseDC( NULL, hDC );
	GetClientRect( hWnd, &rect  );

	lpir->left   = 0;
   lpir->top    = rect.bottom - gtm.tmHeight;
	lpir->right  = rect.right - rect.left;
	lpir->bottom = gtm.tmHeight;

	GetClientRect( hWnd, &grCli );
	grCli.bottom -= gtm.tmHeight;

	// Now actually move the icon bar.  It should be flush
	//  left with the palette window's client area, and should
	//  be the height of some text.
//	SetWindowPos( hInfoBar,
  //               NULL,
    //             0,
      //           rect.bottom - tm.tmHeight,
        //         rect.right - rect.left,
          //       tm.tmHeight,
            //     SWP_NOZORDER | SWP_NOACTIVATE);
	SetWindowPos( hInfoBar,
                 NULL,
                 lpir->left,
                 lpir->top,
                 lpir->right,
                 lpir->bottom,
                 SWP_NOZORDER | SWP_NOACTIVATE );

	DVGlobalUnlock( hPalInfo );

	// Clear our semaphore.
	bInSize = FALSE;

	return( pret );
}

// HANDLE LEFT MOUSE BUTTON IN PALETTE CHILD WINDOW
long PalWM_LBUTT( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	long		   pret;
	HDC			hDC;
	HANDLE		hPalInfo;
	PPI	lpPalInfo;
	int			nRow, nCol;
	int			nScroll;
	LPSTR		   lpb = 0;
	HGLOBAL		hg  = 0;

	pret = 0;
	if( hPalInfo = GetWindowExtra (hWnd, WW_PAL_HPALINFO) )
	{
		if( hDC = GetDC (hWnd) )
		{
			if( lpPalInfo = (PPI) DVGlobalLock(hPalInfo) )
			{
//				if( hg = lpPalInfo->pi_hCopyCOLR )
				if( hg = lpPalInfo->pi_hSortCOLR )
					lpb = DVGlobalLock(hg);
				nScroll   = GetScrollPos (hWnd, SB_VERT);
				nRow      = (HIWORD (lParam) ) / lpPalInfo->cySquare;
				nCol      = LOWORD (lParam) / lpPalInfo->cxSquare;
				if( giDrwSquares )
				{
					UnHighlightSquare( hDC, 
                            lpPalInfo->wEntry, 
                            lpPalInfo->cxSquare,
                            lpPalInfo->cySquare,
                            lpPalInfo->nCols,
                            nScroll );
				}

				// Determine which entry is the new highlighted entry.
				//  Take into account the scroll bar position.
				lpPalInfo->wEntry = ( nRow * lpPalInfo->nCols ) + nCol +
									( lpPalInfo->nCols * nScroll );
				// Don't let the selected palette entry be greater
				//  than the # of palette entries available.
            // WRONG IDEA
				//if( lpPalInfo->wEntry >= lpPalInfo->wPEntries )
				//	lpPalInfo->wEntry = lpPalInfo->wPEntries - 1;
            // Do NOT let the selection EVER be greater than the PHYSICAL
            // COUNT of COLORS in the DIB!!!
            if( lpPalInfo->pi_dwCnt )
            {
               if( lpPalInfo->wEntry >= lpPalInfo->pi_dwCnt )
                  lpPalInfo->wEntry = lpPalInfo->pi_dwCnt - 1;
            }
            else  // since we have NO PHYSICAL COUNT, then
            {
               if( lpPalInfo->wEntry >= lpPalInfo->wPEntries )
				      lpPalInfo->wEntry = lpPalInfo->wPEntries - 1;
            }
				if( giDrwSquares )
				{
					if( hg && lpb )
					{
						HiSquare( hWnd, hDC, lpPalInfo, lpb );
					}
					else
					{
						chkit( "WHOA! We should be using HiSquare!!!" );
//						HighlightSquare (hDC, 
  //                        lpPalInfo->hPal, 
    //                      lpPalInfo->hInfoWnd, 
      //                    lpPalInfo->wEntry,
        //                  lpPalInfo->cxSquare, 
          //                lpPalInfo->cySquare, 
            //              lpPalInfo->nCols,
              //            lpPalInfo->wPEntries,
                //          nScroll,
                  //        guiRMCnt );
					}
				}

				if( hg && lpb )
					DVGlobalUnlock(hg);

				DVGlobalUnlock( hPalInfo );
			}

			ReleaseDC (hWnd, hDC);

		}	/* got the DC */
	}

	return( pret );

}


long PalWM_VSCROLL( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	long		pret;	// Return value
	HWND		hBar;	// HWND of scrollbar
	int			yBar;	// Where scrollbar is now.
	int			nMin;	// Minumum scroll bar value.
	int			nMax;	// Maximum scroll bar value.
	int			dy;		// How much to move.
	int			cyClient;	// Width of client area.
	int			cySquare;	// Height of a palette square.
	int			nRows;		// Number pixels per ROWS of images
	RECT		rect;		// Client area.
	HANDLE		hPalInfo;	// Handle to PALINFO struct.
	PPI	lpPalInfo;	// Pointer to PALINFO struct.

	pret = 0;
	hBar = (HWND)(HIWORD (lParam));
	hPalInfo = GetWindowExtra(hWnd, WW_PAL_HPALINFO);
	if( !hPalInfo )
		return pret;

	lpPalInfo = (PPI) DVGlobalLock( hPalInfo );
	if( lpPalInfo == 0 )
		return pret;

	cySquare  = lpPalInfo->cySquare;
    nRows = lpPalInfo->nRows;

	DVGlobalUnlock( hPalInfo );

	if( !hBar )
		hBar = hWnd;

	GetClientRect( hWnd, &rect );

	GetScrollRange( hBar, SB_VERT, &nMin, &nMax );

	cyClient = rect.bottom - rect.top;

	yBar     = GetScrollPos( hBar, SB_VERT );

	switch( wParam )
	{

	case SB_LINEDOWN:	// One line down.
		dy = 1;
		break;

	case SB_LINEUP:		// One line up.
		dy = -1;
		break;

	case SB_PAGEDOWN:	// One page down.
		dy = cyClient / cySquare;
		break;

	case SB_PAGEUP:		// One page up.
		dy = -( cyClient / cySquare );
		break;

	case SB_THUMBPOSITION:	// Absolute position.
		dy = LOWORD (lParam) - yBar;
		break;

	default:			// No change.
		dy = 0;
		break;

	}

	if( dy )
	{
		yBar += dy;
		if( yBar < nMin )
		{
			dy  -= yBar - nMin;
			yBar = nMin;
		}

		if( yBar > nMax )
		{
			dy  -= yBar - nMax;
			yBar = nMax;
		}

		if( dy )
		{
			SetScrollPos( hBar, SB_VERT, yBar, TRUE );
			InvalidateRect( hWnd, NULL, TRUE );
			UpdateWindow (hWnd);
			pret = 1;
		}
	}
	return pret;
}


void	PaintHisto( HDC hDC, PPI lpPalInfo, LPSTR lpb,
				   LPRECT lpRect, int nScroll )
{
	DWORD	   dwi, dwCnt, dwMax;
	int		i, iWidth, iHeight;
	int		iWid, iHit, iH;
	RECT	   rect;
	DWORD		dwf, dwc;
	COLORREF	cr;
	HBRUSH	hBrush, hOldBrush;
	LPSTR		lpt = &gszPText[0];
	BYTE		r,g,b;
//	POINT		pt;
	HPEN		hPen, hOldPen;
	RECT		rFxd;
	RECT		rc;
	RECT		rFrm;
//   PPX      pmpalx;
   DWORD    dx, dy, dwExt;
	int		id;
	int		iw;

	if( ( lpRect                               ) &&
		( iWidth  = ( lpRect->right - lpRect->left ) ) &&
		( iHeight = ( lpRect->bottom - lpRect->top ) ) &&
		( iWidth  > 10                          ) &&
		( iHeight > 10                          ) &&
		( dwCnt = GetCOLRCnt(lpb)              ) )
	{
		rFxd = *lpRect;
		rFrm = rFxd;
		rect = rFxd;
      iHit  = iHeight;     // assume NO row split
      dwc   = 1;     // ie ONE ROW

		iWid  = iWidth / dwCnt;
		//while( iWid == 0 )
		while( iWid < 2 )
      {
         dwc++;
         iHit = iHeight / dwc;            // get ROW height
			iWid = iWidth  / (dwCnt / dwc);  // and the width of a COLUMN
      }

      if( dwc > 1 )
      {
         sprtf( "PaintHisto %d colors to %s width %d of %d rows, each %d height."MEOR,
            dwCnt,
            Rect2Stg( lpRect ),
            iWid,
            dwc,
            iHit );
      }
      else
      {
         sprtf( "PaintHisto %d colors to %s width %d."MEOR,
            dwCnt,
            Rect2Stg( lpRect ),
            iWid,
            dwc );
      }

		FrameRect( hDC, lpRect, GetStockObject(WHITE_BRUSH) );

      // set first RECT as left + width
		rect.right = rect.left + iWid;
		if( lpRect->bottom )
		{
			/* remove the FRAME done */
			rect.bottom = lpRect->bottom - 1;
		}

		/* use adjusted top and bottom */
		rFrm.top    = rect.top;
		rFrm.bottom = rect.bottom;

		/* get the MAX frequency */
		dwMax = GetFREQMax(lpb);   // returns a min. of 1;

	   lstrcpy( lpt, "(125,125,125)" );
	   dwExt = GetTextExtent( hDC, lpt, lstrlen(lpt) );
	   dx = (DWORD)LOWORD(dwExt);
	   dy = (DWORD)HIWORD(dwExt);
      id = 0;

		if( ( rect.left + ( iWid * (int)(dwCnt / dwc) ) + 1 ) < iWidth )
		{
			/* bump first column over to centre graph */
			iw = ( rect.left + ( iWid * (int)(dwCnt / dwc) ) );
			id = iWidth - iw;
			id = id / 2;
			if( id )
			{
            // if we have a left width remainder
				rc = rect;  // copy rectangle
				rc.right = rc.left + id;
				if( ( rc.top + 1 ) < rc.bottom )
				{
					rc.top++;
					rc.bottom--;
				}
            FillRect( hDC, &rc, (HBRUSH) (COLOR_WINDOW+1));
				//FrameRect( hDC, &rc, GetStockObject(WHITE_BRUSH) );

				rc        = *lpRect;
				rc.top    = rect.top;
				rc.bottom = rect.bottom;
				rc.left   = rc.right - id;
				if( ( rc.top + 1 ) < rc.bottom )
				{
					rc.top++;
					rc.bottom--;
				}
            FillRect( hDC, &rc, (HBRUSH) (COLOR_WINDOW+1));
				//FrameRect( hDC, &rc, GetStockObject(WHITE_BRUSH) );
			}

			/* fix the FRAME of the graph */
			rFrm.left  += id;
			rFrm.right -= id;
			rFrm.right  = rFrm.left + iw;

			/* set RECTANGLE where HISTOGRAM will be drawn */
			rect.left  += id;
			rect.right  = rect.left + iWid;

		}

      // paint the COLOR COLUMNS for the HISTOGRAM
      rect.bottom = rect.top + iHit;    // adjust bottom
      rc = rect;  // copy rectangle

		for( dwi = 0; dwi < dwCnt; dwi++ )
		{
			// cr  = GetCOLR( lpb, dwi );
         // default is per order in PALEX struct. ie cr = GetCOLR( lpb, dwi );
			cr  = GetNEXTColr( lpPalInfo, lpb, dwi );
			r   = GetRValue(cr);
			g   = GetGValue(cr);
			b   = GetBValue(cr);
			dwf = GetFREQ( lpb, dwi );

			/* calculate the HEIGHT of this colour */
			iH = ( dwf * iHit ) / dwMax;
         if( iH < 2 )
            iH = 2;

			if( iH < iHit )   // ( rect.bottom - rect.top ) )
				rect.top = rect.bottom - iH;  // set the TOP
			else
				rect.top = rc.top;  // fill up to current top

         // create BRUSH and PEN
			hBrush = CreateSolidBrush( RGB(r,g,b) );
			hPen   = CreatePen( PS_SOLID, 1, RGB(r,g,b) );
         // select PEN and BRUSH
         if( hBrush && hPen )
         {
			   hOldBrush = SelectObject( hDC, hBrush );
			   hOldPen   = SelectObject( hDC, hPen );
            // ======================================================
            // The Rectangle function draws a rectangle.
            // The rectangle is outlined by using the current pen
            // and filled by using the current brush.
            // The rectangle that is drawn excludes the bottom and right edges.
			   Rectangle( hDC,
				   rect.left,
               rect.top,
               rect.right + 1,
               rect.bottom + 1);
            // ======================================================
            // de-seleact PEN and BRUSH
			   SelectObject( hDC, hOldPen );
			   SelectObject( hDC, hOldBrush );
         }
         else
         {
            sprtf( MEOR"ERROR: GDI get pen or brush FAILED!!!"MEOR );
         }

         // delete PEN and BRUSH
         if( hPen )
            DeleteObject( hPen );
         if( hBrush )
            DeleteObject( hBrush );

         if( iWid > 2 )
            FrameRect( hDC, &rect, GetStockObject(WHITE_BRUSH) );

#ifdef   DBGHIST2
			//wsprintf( lpt, "At (%3d,%3d) put (%3d,%3d,%3d) for %3d",
			//	pt.x, pt.y,
			//	r, g, b,
			//	iHt );
			//DO(lpt);
         sprtf( "Color: (%3d,%3d,%3d) to Rect %s. Ht=%d Freq.=%d"MEOR,
            ( r & 0xff ),
            ( g & 0xff ),
            ( b & 0xff ),
            Rect2Stg( &rect ),
            rect.bottom - rect.top,
            dwf );
#endif   // #ifdef   DBGHIST2

			/* move to right painting rectangles in the colour */
			rect.left  += iWid;
			rect.right  = rect.left + iWid;
//			pt.x       += iWid;

         if( rect.left > rFrm.right )
         {
            if( dwc > 1 )
            {
               // roll down one ROW height
               // ============================
               rect.top     = rc.top   + iHit;
               rect.bottom  = rect.top + iHit;
               // reset left and right
      			rect.left    = rc.left;
			      rect.right   = rect.left + iWid;
               rc = rect;  // keep copy
               // ============================
            }
            else
            {
               sprtf( "Break Paint Histogram rect %s."MEOR, Rect2Stg( &rect ) );
               break;
            }
         }
		}

		wsprintf( lpt,
			"Historgram of %d Colors.",
			dwCnt );	// number of Colors in bitmap
		if( i = lstrlen( lpt ) )
		{
			dx = (DWORD)LOWORD(GetTextExtent(hDC,lpt,i));
			if( dx > (DWORD)iWidth )
					dx = (DWORD)iWidth;
			dx = (( iWidth - (int)dx ) / 2);
			TextOut( hDC,
				( rFxd.left + 1 + dx ),	// pt.x,
				( rFxd.top  + 1      ),	// pt.y,
				lpt,
				i );
			DO(lpt);
		}
	}
	else
	{
		chkit( "HUH! NO paint of histogram *** " );
	}
}

void	BlankInfo( PPI lpPalInfo )
{
	HWND hInfoBar;
	if( ( hInfoBar = lpPalInfo->hInfoWnd ) &&
		( IsWindow(hInfoBar)             ) )
	{
		SetWindowText( hInfoBar, "" );
	}
}

//void HighlightSquare (HDC hDC, 
//                 HPALETTE hPal,
//                      int cxSquare, 
//                      int nColors,
//                      int nScroll,
//                     UINT MCnt)
//{
void	HiSquare( HWND hWnd, HDC hDC, PPI lpPalInfo, LPSTR lpb )
{
	HWND hInfoBar = lpPalInfo->hInfoWnd;
	DWORD wEntry  = lpPalInfo->wEntry;
	int cxSquare  = lpPalInfo->cxSquare;
	int cySquare  = lpPalInfo->cySquare;
	int nCols     = lpPalInfo->nCols;
	RECT		rect;
	HBRUSH		hBrush;
	PALETTEENTRY pe;
//	char		szFlag[20], szFormat[70];
	DWORD		dwOrg;
	BYTE		pFlg;
	POINT		pt, pt2;
	int			nScroll;
	LPSTR		lpd = &gszDiag[0];
	DWORD		dwf, dwc;
	COLORREF	cref;

	nScroll   = GetScrollPos( hWnd, SB_VERT );
	rect.left   = (wEntry % nCols) * cxSquare;
	rect.top    = (wEntry / nCols) * cySquare;
	rect.right  = rect.left + cxSquare;
	rect.bottom = rect.top  + cySquare;
	hBrush      = CreateHatchBrush( HS_BDIAGONAL,
		GetSysColor( COLOR_HIGHLIGHT ) );
	dwOrg = 0;
	pt.x = 0;
	pt.y = 0;
	pt2.x = 0;
	pt2.y = 0;
#ifdef	WIN32
	if( hBrush )
	{
		SetWindowOrgEx( hDC,
			0,
			nScroll * cySquare,
			&pt );
		FrameRect( hDC, &rect, hBrush );
		SetWindowOrgEx( hDC,
			pt.x,
			pt.y,
			&pt2 );
	}
#else	// !WIN32
	dwOrg = SetWindowOrg (hDC, 0, nScroll * cySquare);
	FrameRect (hDC, &rect, hBrush);
	dwOrg = SetWindowOrg (hDC, LOWORD (dwOrg), HIWORD (dwOrg));
#endif	// WIN32 t/n


//   GetPaletteEntries( hPal, wEntry, 1, &pe );

	dwc = GetCOLRCnt(lpb);	// nColors,
	// If the palette entry we just got is just an index into the
	//  system palette, get it's RGB value.
//	pFlg = pe.peFlags = PC_EXPLICIT;
	pFlg = pe.peFlags = 0;
	//if( (pFlg = pe.peFlags) == PC_EXPLICIT )
//	{
//		COLORREF cref;
//		HPALETTE hOldPal;

//		cref       = PALETTEINDEX ((WORD) pe.peRed + (pe.peGreen << 4));
		cref       = GetCOLR(lpb,wEntry);
		dwf        = GetFREQ(lpb,wEntry);
//		hOldPal    = SelectPalette (hDC, hPal, FALSE);
//		cref       = GetNearestColor( hDC, cref );
		//pe.peRed   = (BYTE)  (cref & 0x0000FF);
		//pe.peGreen = (BYTE) ((cref & 0x00FF00) >> 8);
		//pe.peBlue  = (BYTE) ((cref & 0xFF0000) >> 16);  
		pe.peRed   = GetRValue(cref);
		pe.peGreen = GetGValue(cref);
		pe.peBlue  = GetBValue(cref);

//		SelectPalette( hDC, hOldPal, FALSE );
//	}

	wsprintf( lpd,
		"RGB(%3d,%3d,%3d) Entry %d of %d. f=%d.",
		pe.peRed, pe.peGreen, pe.peBlue,
		( wEntry + 1 ),
		dwc,	// GetCOLRCnt(lpb),	// nColors,
		dwf );
	SetWindowText( hInfoBar, lpd );

	DO(lpd);

	if( hBrush )
		DeleteObject( hBrush );

	
}

void UnHiSquare( HWND hWnd, HDC hDC, PPI lpPalInfo )
{
	DWORD wEntry  = lpPalInfo->wEntry;
	int cxSquare = lpPalInfo->cxSquare;
	int cySquare = lpPalInfo->cySquare;
	int nCols    = lpPalInfo->nCols;
	int nScroll  = GetScrollPos( hWnd, SB_VERT );
	RECT		rect;
	DWORD		dwOrg;
	POINT		pt, pt2;

	rect.left   = (wEntry % nCols) * cxSquare;
	rect.top    = (wEntry / nCols) * cySquare;
	rect.right  = rect.left + cxSquare;
	rect.bottom = rect.top  + cySquare;
	pt.x = pt.y = pt2.x = pt2.y = 0;

#ifdef	WIN32
	SetWindowOrgEx( hDC,
		0,
		nScroll * cySquare,
		&pt);
	FrameRect( hDC, &rect, GetStockObject(BLACK_BRUSH) );
	SetWindowOrgEx( hDC,
		pt.x,
		pt.y,
		&pt2 );
#else	// !WIN31
	dwOrg = SetWindowOrg (hDC, 0, nScroll * cySquare);
	FrameRect( hDC, &rect, GetStockObject (BLACK_BRUSH));
	SetWindowOrg (hDC, LOWORD (dwOrg), HIWORD (dwOrg));
#endif	// WIN32 y/n
	dwOrg = 0;
}


COLORREF	GetRGB( HPALETTE hPal, HDC hDC, PALETTEENTRY * lpPIn,
				   WORD wEntry )
{
	COLORREF	    cr = 0;
	PALETTEENTRY	pe;
	PALETTEENTRY *	lppe;

	lppe = &pe;
	lppe->peFlags = 0;
	lppe->peRed   = 0;
	lppe->peGreen = 0;
	lppe->peBlue  = 0;
	if( hPal )
	{
		GetPaletteEntries( hPal, wEntry, 1, lppe );
		if( ( lppe->peFlags == PC_EXPLICIT ) &&
			( hDC ) )
		{
			COLORREF cref;
			HPALETTE hOldPal;
			cref       = PALETTEINDEX ((WORD) lppe->peRed +
				(lppe->peGreen << 4));
			hOldPal    = SelectPalette (hDC, hPal, FALSE);
			cref       = GetNearestColor( hDC, cref );
			lppe->peRed   = (BYTE)  (cref & 0x0000FF);
			lppe->peGreen = (BYTE) ((cref & 0x00FF00) >> 8);
			lppe->peBlue  = (BYTE) ((cref & 0xFF0000) >> 16);  
			if( hOldPal )
				SelectPalette( hDC, hOldPal, FALSE );
		}
	}

	cr = RGB( lppe->peRed, lppe->peGreen, lppe->peBlue );
	if( lpPIn )
	{
		memcpy( lpPIn, lppe, sizeof(PALETTEENTRY) );
	}
	return cr;
}


void	PaintColors( HWND hWnd, HDC hDC, PPI lpPalInfo, LPSTR lpb,
				   LPRECT lpr, int nScroll )
{
	DWORD	dwi, dwCnt, dwMax;
	int		i, iWidth, iHeight;
	int		iWid, iHt;
	RECT	rect;
	DWORD		dwf;
	COLORREF	cr;
	HBRUSH		hBrush, hOldBrush;
	LPSTR		lpt = &gszPText[0];
	BYTE		r,g,b;
	POINT		pt;
	HPEN		hPen, hOldPen, hpBlack;
	int			cxSquare;
	int			cySquare;
	int			nCols;
	HPALETTE	hPal;
	if( ( lpPalInfo                            ) &&
		( cxSquare  = lpPalInfo->cxSquare      ) &&
		( cySquare  = lpPalInfo->cySquare      ) &&
		( lpr                                  ) &&
		( iWidth  = ( lpr->right - lpr->left ) ) &&
		( iHeight = ( lpr->bottom - lpr->top ) ) &&
		( iWidth  > 0                          ) &&
		( iHeight > 0                          ) &&
		( dwCnt = GetCOLRCnt(lpb)              ) )
	{

		DWORD	dwExt, dx, dy;
		iWid  = iWidth / dwCnt;
		hpBlack = CreatePen( PS_SOLID, 1, RGB(0,0,0) );
		hOldPen = SelectObject( hDC, hpBlack );
		nCols   = lpPalInfo->nCols;
		guiRMCnt = dwCnt;
		hPal = lpPalInfo->hPal;

		if( iWid <= 0 )
			iWid = 1;

//		lstrcpy( lpt, "(255,255,255)" );
		iHeight = (( iHeight * 9 ) / 10 );
		lstrcpy( lpt, "(125,125,125)" );
		dwExt = GetTextExtent( hDC, lpt, lstrlen(lpt) );
		dx = (DWORD)LOWORD(dwExt);
		dy = (DWORD)HIWORD(dwExt);
		rect = *lpr;
		rect.left  = lpr->left;
		rect.right = rect.left + iWid;
		rect.bottom = lpr->bottom;
		dwMax = 1;
		pt.x = lpr->left;
		// grCli.bottom -= gtm.tmHeight;
		if( dy < (DWORD)gtm.tmHeight )
			dy = gtm.tmHeight;

		if( ( dy + 3 ) < (DWORD)iHeight )
		{
			pt.y = lpr->top + ( iHeight - ( dy + 3 ) );
		}
		else
		{
			pt.y = lpr->top + 3;
		}

		/* get the MAX frequency */
		dwMax = GetFREQMax(lpb);
      if( dwMax == 0 )
         dwMax = 1;

		for( dwi = 0; dwi < dwCnt; dwi++ )
		{
			/* was simple cr  = GetCOLR( lpb, dwi );, but apply sorts */
			cr  = GetNEXTColr( lpPalInfo, lpb, dwi );
			r = GetRValue(cr);
			g = GetGValue(cr);
			b = GetBValue(cr);
			dwf = GetFREQ( lpb, dwi );
			/* calculate the HEIGHT of this colour */
			iHt = ( dwf * iHeight ) / dwMax;
//			if( iHt < rect.bottom )
//				rect.top = rect.bottom - iHt;
//			else
//				rect.top = 0;
//			hBrush    = CreateSolidBrush( cr );
			pt.x = (dwi % nCols) * cxSquare;
			pt.y = (dwi / nCols) * cySquare;

         // establish the rectangle of the PAINT
			rect.left   = pt.x;
			rect.top    = pt.y;
			rect.right  = rect.left + cxSquare;
			rect.bottom = rect.top  + cySquare;

			if( hBrush = CreateSolidBrush( RGB(r,g,b) ) )
			{
				hOldBrush = SelectObject( hDC, hBrush );
			}
			else
			{
				chkit( "Holy smokes! An RGB brush FAILED!!" );
			}
			if( gfAddPenC2 )
			{
				hPen = CreatePen( PS_SOLID, 1, RGB(r,g,b) );
//				hOldPen = SelectObject( hDC, hPen );
				SelectObject( hDC, hPen );
			}
			if( gfDrwPol )
			{
				POINT	aPts[8];
				HBRUSH	hB;
				/* draw the first HALF per our extracted color */
				aPts[0].x = pt.x + cxSquare;
				aPts[0].y = pt.y;
				aPts[1].x = pt.x;
				aPts[1].y = pt.y;
				aPts[2].x = pt.x;
				aPts[2].y = pt.y + cySquare;
				Polygon( hDC, &aPts[0], 3 );
				aPts[1].x = pt.x + cxSquare;
				aPts[1].y = pt.y + cySquare;
				// if( hB2 = CreateSolidBrush( RGB(br, bg, bb) ) )
				if( hB = CreateSolidBrush( PALETTEINDEX(dwi) ) )
				{
					HBRUSH			_hcb;
					COLORREF		cr2;
					PALETTEENTRY	pe;
					BYTE			r2,g2,b2;
					_hcb = SelectObject( hDC, hB );
					Polygon( hDC, &aPts[0], 3 );
					SelectObject( hDC, _hcb );
					DeleteObject(hB);
					if( dwi < 0x10000 )
					{
						cr2  = GetRGB( hPal, hDC, &pe, (WORD)dwi );
						r2 = GetRValue(cr2);
						g2 = GetGValue(cr2);
						b2 = GetBValue(cr2);
						wsprintf( &gszSpl[0],
							"(%d,%d,%d)",
							r2,g2,b2 );
					}
					else
					{
						wsprintf( &gszSpl[0],
							"(%d!!!)",
							dwi );
					}
				}
				else
				{
					chkit( "Eek! A simple color brush FAILED!" );
				}
			}
			else
			{
				Rectangle( hDC,
					rect.left,
					rect.top,
					rect.right,
					rect.bottom );
			}
			if( gfAddPenC2 )
			{
				SelectObject( hDC, hOldPen );
				DeleteObject( hPen );
			}

			SelectObject( hDC, hOldBrush );
			DeleteObject( hBrush );
			wsprintf( lpt,
				"(%d,%d,%d)",
				r, g, b );
			i = lstrlen(lpt);
			if( dx <= (DWORD)iWid )
			{
				int		ix;
				DWORD	dxx;
				dxx = (DWORD)LOWORD(GetTextExtent(hDC,lpt,i));
				if( dxx > (DWORD)iWid )
					dxx = (DWORD)iWid;
				ix = (( iWid - (int)dxx ) / 2);
				//TextOut( hDC,
				//	(pt.x + ix),
				//	pt.y,
				//	lpt,
				//	i );
			}

#ifdef   DBGPAINTC2
			wsprintf( lpt,
            "Square: (%3d,%3d) put (%3d,%3d,%3d) F=%3d ",
				pt.x, pt.y,
				r, g, b,
				dwf );	// or the iHt = PIXELS up the screen
			if( gfDrwPol )
			{
				// wsprintf( &gszSpl[0],
				lstrcat( lpt, " " );
				lstrcat( lpt, &gszSpl[0] );
			}

			DO(lpt);
#endif   // #ifdef   DBGPAINTC2

			/* move to right painting rectangles in the colour */
			//rect.left  += iWid;
			//rect.right = rect.left + iWid;
			//pt.x       += iWid;
		}	/* for EACH color */

		while( pt.y == (int)( ( dwi / nCols ) * cySquare ) )
		{
			dwi++;
		}

		pt.x = ( dwi % nCols ) * cxSquare;
		pt.y = ( dwi / nCols ) * cySquare;
//		lstrcpy( lpt, "Logical / Physical Colors!" );
//		if( gfDrwPol )
//		{
//			wsprintf( lpt,
//				"%d Ranked / Palette Colors!",
//				dwCnt );	// number of Colors in bitmap
//		}
//		else
//		{
//			wsprintf( lpt,
//				"%d Colors per BMP order!",
//				dwCnt );	// number of Colors in bitmap
//		}
		lstrcpy( lpt, &gszStatus[0] );
		if( i = lstrlen( lpt ) )
		{
			DWORD	dxx;
			int		ix;
			dxx = (DWORD)LOWORD(GetTextExtent(hDC,lpt,i));
			if( dxx > (DWORD)iWidth )
					dxx = (DWORD)iWidth;
			ix = (( iWidth - (int)dxx ) / 2);
			TextOut( hDC,
				ix,			// pt.x,
				pt.y,
				lpt,
				i );
			DO(lpt);
		}


	// Highlight the currently selected palette square,
	//  and change the info window to reflect this
	//  square.
		if( giDrwSquares )
		{
			HiSquare( hWnd, hDC, lpPalInfo, lpb );
		}

		SelectObject( hDC, hOldPen );
		DeleteObject( hpBlack );
	}
	else
	{
		chkit( "HUH! NO paint of colored squares *** " );
	}

	*lpt = 0;

}


long PalWM_PAINT1( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	long		pret = 0;
	PAINTSTRUCT ps;
	HDC			hDC;
	RECT		rect;
	int			i, nScroll;
	HANDLE		hPalInfo;
	PPI	lpPalInfo;
	HGLOBAL		hg;
	LPSTR		lpb;
	LPSTR		lpt;
	LPRECT		lpr;

	lpt       = &gszPText[0];
	*lpt      = 0;
	lpPalInfo = 0;

//	GetClientRect (hWnd, &rect);
	hDC       = BeginPaint (hWnd, &ps);
	nScroll   = GetScrollPos( hWnd, SB_VERT );
	hPalInfo  = GetWindowExtra( hWnd, WW_PAL_HPALINFO );
	if( !hPalInfo )
	{
		wsprintf( lpt,
			"Handle to Memory pointer is ZERO!"MEOR,
			hPalInfo );
		chkit(lpt);
		goto ENDPAINT1;
	}

	lpPalInfo = (PPI) DVGlobalLock (hPalInfo);
	if( lpPalInfo == 0 )
	{
		wsprintf( lpt,
			"Handle (0x%x) to Memory pointer failed!"MEOR,
			hPalInfo );
		chkit(lpt);
		goto ENDPAINT1;
	}

	lpr = &lpPalInfo->pi_rClient;
	rect = *lpr;
	hg = lpPalInfo->pi_hCopyCOLR;
	if( !hg )
	{
		wsprintf( lpt,
			"NO Handle to COLR Memory!"MEOR,
			hPalInfo );
		chkit(lpt);
		goto ENDPAINT0;
	}
	lpb = DVGlobalLock(hg);
	if( !lpb )
	{
		wsprintf( lpt,
			"Handle (0x%x) to COLR Memory FAILED!"MEOR,
			hg );
		chkit(lpt);
		goto ENDPAINT0;
	}

	if( ( grCli.left ) &&
		( rect.left != grCli.left ) )
	{
		rect.left = grCli.left;
	}
	if( ( grCli.top ) &&
		( rect.top != grCli.top ) )
	{
		rect.top = grCli.top;
	}
	if( grCli.right < rect.right )
	{
		rect.right = grCli.right;
	}
	if( grCli.bottom < rect.bottom )
	{
		rect.bottom = grCli.bottom;
	}


	PaintHisto( hDC, lpPalInfo, lpb, &rect, nScroll );
	BlankInfo( lpPalInfo );

	DVGlobalUnlock(hg);
	pret = 1;
	*lpt = 0;
	
ENDPAINT0:

	if( hPalInfo && lpPalInfo )
		DVGlobalUnlock(hPalInfo);

ENDPAINT1:

	if( i = lstrlen( lpt ) )
	{
		TextOut( hDC, 0, 0, lpt, i );
		DO(lpt);
	}

	EndPaint( hWnd, &ps );

	return pret;
}

long PalWM_PAINTC( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	long		   pret = 0;
	PAINTSTRUCT ps;
	HDC			hDC;
	RECT		   rect;
	int			i, nScroll;
	HANDLE		hPalInfo;
	PPI	lpPalInfo;
	HGLOBAL		hg;
	LPSTR		   lpb;  // for the COLOR MEMORY - actually a PALEX structure
	LPSTR		   lpt;
	HPALETTE	   hPal;
	HPALETTE	   hOldPal = 0;
	POINT		   opt;
	int			cySquare;
	LPRECT		lpr;
	DWORD		   dwCnt;

	lpt       = &gszPText[0];
	*lpt      = 0;
	lpPalInfo = 0;

//	GetClientRect (hWnd, &rect);
	hDC       = BeginPaint (hWnd, &ps);
	nScroll   = GetScrollPos( hWnd, SB_VERT );
	hPalInfo  = GetWindowExtra( hWnd, WW_PAL_HPALINFO );
	if( !hPalInfo )
	{
		wsprintf( lpt,
			"Handle to Memory pointer is ZERO!"MEOR,
			hPalInfo );
		chkit(lpt);
		goto ENDPAINT1;
	}

	lpPalInfo = (PPI) DVGlobalLock (hPalInfo);
	if( lpPalInfo == 0 )
	{
		wsprintf( lpt,
			"Handle (0x%x) to Memory pointer failed!"MEOR,
			hPalInfo );
		chkit(lpt);
		goto ENDPAINT1;
	}

	lpr = &lpPalInfo->pi_rClient;
	rect = *lpr;
	cySquare = lpPalInfo->cySquare;

   // extract the HANDLE to the PALEX color memory
   // NOTE: Use the "sorted" array
	hg = lpPalInfo->pi_hSortCOLR;
	if( !hg )
	{
		wsprintf( lpt,
			"NO Handle to (sorted) COLR Memory!"MEOR,
			hPalInfo );
		chkit(lpt);
		goto ENDPAINT0;
	}

   // lock it down
   // extract from DVChild.h
   ///* kept in lpDIBInfo->di_hCOLR */
   ///* =========================== */
   //typedef struct tagCOLR {
   //	COLORREF	cr_Color;
   //	DWORD		cr_Freq;
   //}COLR;
   //typedef COLR MLPTR LPCOLR;
   //typedef struct tagPALEX {
   //	/* ======================================================= */
   //	DWORD	px_Size;		/* SIZE of this instance of struct */
   //	DWORD	px_Count;		/* COUNT of color listed in array  */
   //	COLR	px_Colrs[1];	/* array of COLOR and FREQUENCY    */
   //	/* ======================================================= */
   //}PALEX;
   //typedef PALEX MLPTR LPPALEX;
   // NOTE: The separation of the COLOR and the FREQUENCY has been done - NO RANK
	lpb = DVGlobalLock(hg);
	if( !lpb )
	{
		wsprintf( lpt,
			"Handle (0x%x) to COLR Memory FAILED!"MEOR,
			hg );
		chkit(lpt);
		goto ENDPAINT0;
	}


	if( !( dwCnt = GetCOLRCnt(lpb) ) )
	{
		wsprintf( lpt,
			"COLOR COUNT IS ZERO!!!!!!!!!!"MEOR,
			hg );
		chkit(lpt);
		goto ENDPAINT0;

	}
	if( ( grCli.left ) &&
		( rect.left != grCli.left ) )
	{
		rect.left = grCli.left;
	}
	if( ( grCli.top ) &&
		( rect.top != grCli.top ) )
	{
		rect.top = grCli.top;
	}
	if( grCli.right < rect.right )
	{
		rect.right = grCli.right;
	}
	if( grCli.bottom < rect.bottom )
	{
		rect.bottom = grCli.bottom;
	}

	/* important handle to the palette being displayed */
	if(	hPal = lpPalInfo->hPal )
	{
		hOldPal = SelectPalette( hDC, hPal, TRUE );
	}        
	else
	{
		*lpt = 0;
		LoadString( ghDvInst, IDS_PAL_NOPAL, lpt, 255 );
		if( i = lstrlen(lpt) )
		{
			TextOut( hDC, 0, 0, lpt, i );
		}
		goto ENDPAINT0;
	}

	guiRMCnt = RealizePalette( hDC );	// Map this PALETTE into system

	// Change our window origin to reflect the current
	//  scroll bar state.
#ifdef	WIN32
	SetWindowOrgEx(	hDC,	// handle of device context
		0,	// new x-coordinate of window origin
		(nScroll * cySquare),	// new y-coordinate of window origin
		&opt );		// address of structure receiving original origin
#else	// !WIN32
	SetWindowOrg( hDC, 0,
		nScroll * cySquare );	// = GetScrollPos (hWnd, SB_VERT)
#endif	// WIN32 y/n

//	if( dwCnt = GetCOLRCnt(lpb) )
	if( giDrwSquares )
	{
		RECT	rcc;
		RECT	rc = rect;
		DWORD	dwi;

		rc.top = rc.bottom / 2;
		dwi = (dwCnt - 1);
		rcc = *lpr;
//		rcc.left = (dwi % lpPalInfo->nCols) * lpPalInfo->cxSquare;
		rcc.top  = (dwi / lpPalInfo->nCols) * lpPalInfo->cySquare;
//		rcc.right  = rcc.left + lpPalInfo->cxSquare;
//		rcc.bottom = rcc.top  + lpPalInfo->cySquare;
		while( rcc.top == (int)( ( dwi / lpPalInfo->nCols ) * lpPalInfo->cySquare ) )
		{
			dwi++;
		}
		//rcc.left = ( dwi % lpPlaInfo->nCols ) * lpPalInfo->cxSquare;
		rcc.top  = ( dwi / lpPalInfo->nCols ) * lpPalInfo->cySquare;
		rcc.top += 20;
		if( rcc.top <= rc.top )
		{
			rc.top = rcc.top;
		}
		else	/* if( rcc.top > rc.top ) */
		{
			rc.top = rcc.top;
			if( rcc.top > rc.bottom )
			{
				rc.bottom = rc.top + MIN_HISTO;  // 120;
			}
			else
			{
				if( ( rc.bottom - rc.top ) < MIN_HISTO )  // 120 )
				{
					rc.bottom = rc.top + MIN_HISTO;  // 120;
				}
			}
		}

		if( giDrwHisto )
		{
//			PaintColorCyc( hDC, lpPalInfo, &rc, );
			PaintHisto( hDC, lpPalInfo, lpb, &rc, nScroll );
		}
		else
		{
			PaintRainbow( hDC, lpPalInfo, lpb, &rc, nScroll );
		}

		PaintColors( hWnd, hDC, lpPalInfo, lpb, &rect, nScroll );

	}
	else
	{
		PaintHisto( hDC, lpPalInfo, lpb, &rect, nScroll );
	}

	if( hOldPal )
		SelectPalette( hDC, hOldPal, FALSE );

	DVGlobalUnlock(hg);	/* unlock the sorted color array */
	pret = 1;
	*lpt = 0;
	
ENDPAINT0:

	if( hPalInfo && lpPalInfo )
		DVGlobalUnlock(hPalInfo);

ENDPAINT1:

	if( i = lstrlen( lpt ) )
	{
		TextOut( hDC, 0, 0, lpt, i );
		DO(lpt);
	}

	EndPaint( hWnd, &ps );

	return pret;
}


long PalWM_PAINT( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	long		pret = 0;

		pret = PalWM_PAINTC( hWnd, message, wParam, lParam );

	return pret;
}

void	Do_IDM_PAL_OPTIONS( HWND hWnd, WPARAM wParam )
{
	HANDLE      hPalInfo;
	PPI         lpPalInfo;
//	HGLOBAL		hg;
   DWORD       cmd = LOWORD(wParam);

	if( ( hPalInfo = GetWindowExtra (hWnd, WW_PAL_HPALINFO) ) &&
		 ( lpPalInfo = (PPI) DVGlobalLock( hPalInfo )  ) )
	{
		//switch( wParam )
		switch( cmd )
		{

		case IDM_PAL_NORMAL:
			lpPalInfo->pi_iNormal = 0;
			break;

		case IDM_PAL_INTENSITY:
			if( lpPalInfo->pi_iNormal & pif_Intensity )
			{
				lpPalInfo->pi_iNormal &= ~( pif_Intensity );
			}
			else
			{
				lpPalInfo->pi_iNormal &= pif_Clear;
				lpPalInfo->pi_iNormal |= pif_Intensity;
			}
			break;

		case IDM_PAL_SPLIT:
			if( gfDrwPol )
				gfDrwPol = 0;
			else
				gfDrwPol = 1;
			giChgDrwP = 1;
			break;

		case IDM_PAL_HISTO:
			if( giDrwHisto )
				giDrwHisto = 0;
			else
				giDrwHisto = 1;
			giChgHisto = 1;
			break;

		case IDM_PAL_RYBIV:
			if( lpPalInfo->pi_iNormal & pif_Rainbow )
			{
				lpPalInfo->pi_iNormal &= ~( pif_Rainbow );
			}
			else
			{
				lpPalInfo->pi_iNormal &= pif_Clear;
				lpPalInfo->pi_iNormal |= pif_Rainbow;
			}
			break;

		case IDM_PAL_NUMERIC:
			if( lpPalInfo->pi_iNormal & pif_Numeric )
			{
				lpPalInfo->pi_iNormal &= ~( pif_Numeric );
			}
			else
			{
				lpPalInfo->pi_iNormal &= pif_Clear;
				lpPalInfo->pi_iNormal |= pif_Numeric;
			}
			break;

		case IDM_PAL_GRAY:
			if( lpPalInfo->pi_iNormal & pif_Gray )
			{
				lpPalInfo->pi_iNormal &= ~( pif_Gray );
			}
			else
			{
				lpPalInfo->pi_iNormal &= pif_Clear;
				lpPalInfo->pi_iNormal |= pif_Gray;
			}
			break;

		case IDM_PAL_FREQ:
			if( lpPalInfo->pi_iNormal & pif_Freq )
			{
				lpPalInfo->pi_iNormal &= ~( pif_Freq );
			}
			else
			{
				lpPalInfo->pi_iNormal &= pif_Clear;
				lpPalInfo->pi_iNormal |= pif_Freq;
			}
			break;


		case IDM_PAL_INVERT:
			// simple toggle off /on /off / ...
			if( lpPalInfo->pi_iInvert )
				lpPalInfo->pi_iInvert = 0;
			else
				lpPalInfo->pi_iInvert = 1;
			break;

		case IDM_PAL_SQUARES:	// MENUITEM "&Squares"
			if( giDrwSquares )
				giDrwSquares = 0;
			else
				giDrwSquares = 1;
			giChgSquares = 1;
			break;
		}

		/* rebuild the LIST */
		//if( hg = lpPalInfo->pi_hCopyPal )
		//	DVGlobalFree(hg);
		//lpPalInfo->pi_hCopyPal = 0;

		DoSORT( lpPalInfo );

		DVGlobalUnlock( hPalInfo );

		InvalidateRect( hWnd, NULL, TRUE );

	}
	else
	{
		chkit( "WHAT! No HANDLE in EXTRA memory!!! DRAT THAT!" );
	}

}

void	Do_IDM_PAL_SIZE( HWND hWnd, WPARAM wP )
{
	HANDLE    hPalInfo;
	PPI lpPalInfo;
	HMENU     hMenu;
	WORD      wOldItem;
   DWORD    cmd = LOWORD(wP);
	if( ( hMenu    = GetMenu( hWnd ) ) &&
		( hPalInfo = GetWindowExtra (hWnd, WW_PAL_HPALINFO) ) &&
		( lpPalInfo = (PPI) DVGlobalLock( hPalInfo )  ) )
	{
		wOldItem  = lpPalInfo->nSquareSize;
				CheckMenuItem( hMenu, 
					wOldItem + IDM_PAL_TINY, 
					MF_BYCOMMAND | MF_UNCHECKED);
				CheckMenuItem( hMenu, 
					cmd, 
					MF_BYCOMMAND | MF_CHECKED);
		lpPalInfo->nSquareSize = cmd - IDM_PAL_TINY;
//				PalRowsAndColumns( hWnd, lpPalInfo->nSquareSize,
		PalRowsAndColumns( lpPalInfo,
					lpPalInfo->nSquareSize,
                    &lpPalInfo->nCols, &lpPalInfo->nRows,
                    &lpPalInfo->cxSquare, &lpPalInfo->cySquare );
		DVGlobalUnlock (hPalInfo);
		InvalidateRect( hWnd, NULL, TRUE );
	}
	else
	{
		chkit( "WHAT! No HANDLE in EXTRA memory!!! DRAT THAT TOO!" );
	}

}



long PalWM_COMMAND( HWND hWnd, WPARAM wP, LPARAM lParam )
{
	long	pret;
   DWORD cmd = LOWORD(wP);
//	HANDLE    hPalInfo;
//	PPI lpPalInfo;
//	HMENU     hMenu;
//	WORD      wOldItem;

	pret = 0;
	switch( cmd )
	{
	case IDM_EXIT:
		{
			PostMessage( hWnd, WM_CLOSE, 0, 0L );
		}
		break;

   case IDM_PAL_PRINT:
      Do_IDM_PAL_PRINT( hWnd );  // print this child
      break;

	case IDM_PAL_TINY:
	case IDM_PAL_SMALL:
	case IDM_PAL_MEDIUM:
	case IDM_PAL_LARGE:
		{
			Do_IDM_PAL_SIZE( hWnd, cmd ); // wParam );
		}
		break;

	case IDM_PAL_NORMAL:
	case IDM_PAL_INTENSITY:
	case IDM_PAL_INVERT:
	case IDM_PAL_SPLIT:
	case IDM_PAL_HISTO:
	case IDM_PAL_RYBIV:		/* sort in RAINBOW order */
	case IDM_PAL_NUMERIC:
	case IDM_PAL_SQUARES:	// MENUITEM "&Squares"
   case IDM_PAL_GRAY:      // Grayness on MENU
   case IDM_PAL_FREQ:      // Frequency on SUB-MENU
		{
//			hMenu    = GetMenu( hWnd );
			Do_IDM_PAL_OPTIONS( hWnd, cmd ); // wParam );
		}
		break;

	default:
		{
			pret = DefWindowProc( hWnd, WM_COMMAND, wP, lParam );
		}
		break;
	}

	return( pret );

}	// PalWM_COMMAND(...)


long	PalWM_QUERYNEWPALETTE( HWND hWnd,
							  UINT message,
							  WPARAM wParam,
							  LPARAM lParam )
{
	long		lRet;
	HANDLE		hPalInfo;
	PPI	lpPalInfo;
	HPALETTE	hPal, hOldPal;
	HDC			hDC;

	lRet = 0;
	hPalInfo  = GetWindowExtra(hWnd, WW_PAL_HPALINFO);
	if( !hPalInfo )
		return lRet;

	lpPalInfo = (PPI) DVGlobalLock (hPalInfo);
	if( !lpPalInfo )
		return lRet;

	hPal      = lpPalInfo->hPal;
	DVGlobalUnlock (hPalInfo);

	if( hPal )
	{
		hDC       = GetDC (hWnd);
		hOldPal   = SelectPalette (hDC, hPal, FALSE);

		RealizePalette (hDC);
		SelectPalette (hDC, hOldPal, FALSE);
		ReleaseDC (hWnd, hDC);

		InvalidateRect (hWnd, NULL, FALSE);

		lRet = 1;
	}

	return lRet;
}

void	FreePalMemory( PPI lpPalInfo )
{
	HGLOBAL		hg;
	if( hg = lpPalInfo->hCols )
		DVGlobalFree(hg);
	lpPalInfo->hCols = 0;

	/* COPY OF PALETTE TO EXPOSE COLOR VALUES */
	if( hg = lpPalInfo->pi_hCopyPal )	/* COPY of palette - after pnt */
		DVGlobalFree(hg);
	lpPalInfo->pi_hCopyPal = 0;	/* COPY of palette - after pnt */

	if( hg = lpPalInfo->pi_hCopyCOLR )
		DVGlobalFree(hg);
	lpPalInfo->pi_hCopyCOLR = 0;

	if( hg = lpPalInfo->pi_hSortCOLR )
		DVGlobalFree(hg);
	lpPalInfo->pi_hSortCOLR = 0;

}

long	PalWM_DESTROY( HWND hWnd,
					  UINT message,
					  WPARAM wParam,
					  LPARAM lParam )
{
	long		lRet;
	HANDLE    hPalInfo;
	PPI lpPalInfo;

	lRet = 0;

	hPalInfo = GetWindowExtra(hWnd, WW_PAL_HPALINFO);
	if( !hPalInfo )
		return lRet;

	lpPalInfo = (PPI) DVGlobalLock (hPalInfo);
	if( !lpPalInfo )
		goto Do_Free;

	if( lpPalInfo->hPal )
		DeleteObject( lpPalInfo->hPal );

	FreePalMemory( lpPalInfo );

	DVGlobalUnlock( hPalInfo );

	lRet = 1;

Do_Free:

	DVGlobalFree( hPalInfo );

	SetWindowExtra( hWnd, WW_PAL_HPALINFO, NULL );

	return lRet;
}

//---------------------------------------------------------------------
//
// Function:   PALETTEWNDPROC
//
// Purpose:    Window procedure for palette child windows.  These
//             windows are responsible for displaying a color palette
//             a-la the Clipboard Viewer.  It also dumps info on each
//             palette color.
//
// Parms:      hWnd    == Handle to this child window.
//             message == Message for window.
//             wParam  == Depends on message.
//             lParam  == Depends on message.
//
// History:   Date      Reason
//             6/01/91  Created
//             1/28/92  Added WM_QUERYNEWPALETTE message,
//                        and always select palette as
//                        a background palette in WM_PAINT.
//             
//---------------------------------------------------------------------

//long MLIBCONV PALETTEWNDPROC (HWND hWnd,
//				UINT message,
//				WPARAM wParam,
//				LPARAM lParam)
LRESULT CALLBACK PALETTEWNDPROC(
  HWND hWnd,      // handle to window
  UINT message,   // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
)
{
	LRESULT	lRet = 0;

	switch( message )
	{
	case WM_CREATE:
		{
			PalWM_CREATE( hWnd, message, wParam, lParam );
		}
		break;

		// If the window is re-sized, move the information bar
		// (a static text control) accordingly.
	case WM_SIZE:
		{
			PalWM_SIZE( hWnd, message, wParam, lParam );
		}
		break;

	case WM_PAINT:
		{
			PalWM_PAINT( hWnd, message, wParam, lParam );
		}
		break;

		// If the user hits the left mouse button, change the
		//  selected palette entry.
	case WM_LBUTTONDOWN:
		{
			PalWM_LBUTT( hWnd, message, wParam, lParam );
		}
		break;

         // Do that vertical scroll bar thing.
	case WM_VSCROLL:
         {
			PalWM_VSCROLL( hWnd, message, wParam, lParam );
         }
		break;

         // If the system palette changes, we need to re-draw, re-mapping
         //  our palette colors.  Otherwise they will be completely
         //  wrong (and if any palette animation is going on, they
         //  will start animating)!!

    case WM_PALETTECHANGED:
         if (hWnd != (HWND) wParam)
            InvalidateRect (hWnd, NULL, FALSE);
         break;

		// If we get the focus on this window, we want to insure
		//  that we have the foreground palette.
    case WM_QUERYNEWPALETTE:
         {
			   lRet = PalWM_QUERYNEWPALETTE( hWnd, message, wParam, lParam );
         }
		break;


         // Window's going away, destroy the palette, and the PALINFO
         //  structure.

    case WM_DESTROY:
         {
			   PalWM_DESTROY( hWnd, message, wParam, lParam );
         }
		break;

         // If we're getting the focus, our palette may not be the
         //  currently selected palette.  Therefore, force a repaint.
    case WM_SETFOCUS:
		  InvalidateRect (hWnd, NULL, TRUE);
		  break;

      // Something was picked off a menu.  If the user is asking
      //  to change the way we represent the palette, uncheck the
      //  old way, check the new way, and force a repaint.  If not,
      //  pass the value on to DefWindowProc().
	case WM_COMMAND:
		{
			lRet = PalWM_COMMAND( hWnd, wParam, lParam );
		}
		break;

	case WM_INITMENUPOPUP:
		{
			lRet = PalWM_INITMENUPOPUP( hWnd, message,
						  wParam, lParam );
		}
		break;
	case WM_ERASEBKGND:
		{
			lRet = PalWM_ERASEBKGND( hWnd, message,
						  wParam, lParam );
		}
		break;

	default:
			lRet = DefWindowProc( hWnd, message, wParam, lParam );
			break;

	}	/* all the cases of message */


	return lRet;

}



//---------------------------------------------------------------------
//
// Function:   UnHighlightSquare
//
// Purpose:    Un-Highlight a palette entry.
//
// Parms:      hDC      == DC where we want to unhighlight a pal. square.
//             wEntry   == Entry to highlight.
//             cxSquare == Width a a palette square.
//             cySquare == Height of a palette square.
//             nCols    == # of columns currently displayed in window.
//             nScroll  == # of rows the window has scrolled.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

void UnHighlightSquare (HDC hDC, 
                       DWORD wEntry, 
                        int cxSquare, 
                        int cySquare,
                        int nCols,
                        int nScroll)
{
	RECT  rect;
	DWORD dwOrg;
	POINT	pt, pt2;

	rect.left   = (wEntry % nCols) * cxSquare;
	rect.top    = (wEntry / nCols) * cySquare;
	rect.right  = rect.left + cxSquare;
	rect.bottom = rect.top  + cySquare;
	pt.x = pt.y = pt2.x = pt2.y = 0;

#ifdef	WIN32
	SetWindowOrgEx( hDC,
		0,
		nScroll * cySquare,
		&pt);
	FrameRect (hDC, &rect, GetStockObject(BLACK_BRUSH) );
	SetWindowOrgEx( hDC,
		pt.x,
		pt.y,
		&pt2 );
#else	// !WIN31
	dwOrg = SetWindowOrg (hDC, 0, nScroll * cySquare);
	FrameRect (hDC, &rect, GetStockObject (BLACK_BRUSH));
	SetWindowOrg (hDC, LOWORD (dwOrg), HIWORD (dwOrg));
#endif	// WIN32 y/n
	dwOrg = 0;
}



//---------------------------------------------------------------------
//
// Function:   PalRowsAndColumns
//
// Purpose:    Given a square size, determine the # of Rows/Columns
//             that will fit in the client area of a palette window.
//             Also, set the # of pixels per square for height/width.
//             Finally, set up the scroll bar.
//
// Parms:      hWnd       == Window we're doing this for.
//             nSquareSize== Size to make squares (offset into 
//                            nEntriesPerInch array).
//             lpnCols    == Pointer to # Cols to display, set dependent
//                            on nSquareSize and dimeinsions of window.
//             lpnRows    == Ditto -- for # of rows to display.
//             lpcxSquare == Ditto -- width of a palette square.
//             lpcySquare == Ditto -- height of a palette square.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

//void PalRowsAndColumns( HWND hWnd,
void PalRowsAndColumns( PPI lpPalInfo,
					   int nSquareSize,
					   LPINT lpnCols,
					   LPINT lpnRows,
					   LPINT lpcxSquare,
					   LPINT lpcySquare )
{
	HDC		hDC;
	int		cxInch, cyInch;
	int		nPerInch;
//	RECT	rect;
	HWND	hWnd;
	LPSTR	lpd = &gszDiag[0];
	int		iRows, iCols, icx, icy;
	HGLOBAL	hg;
	LPSTR	lpb;
	POINT	pt;

	hWnd   = lpPalInfo->pi_hWnd;
	hDC    = GetDC(NULL);
	/* get number of pixels per logical inch */
	cxInch = GetDeviceCaps( hDC, LOGPIXELSX );
	cyInch = GetDeviceCaps( hDC, LOGPIXELSY );
	gcxPixs = GetDeviceCaps( hDC, HORZRES ); //Width, in pixels, of the screen.
	gcyPixs = GetDeviceCaps( hDC, VERTRES ); //Height, in raster lines, of the screen.
	GetTextMetrics( hDC, &gtm );
	ReleaseDC(NULL, hDC);

//	GetClientRect( hWnd, &rect );
//	rect = lpPalInfo->pi_rClient;
	pt.x = lpPalInfo->pi_rClient.right -
			lpPalInfo->pi_rClient.left;
	pt.y = lpPalInfo->pi_rClient.bottom -
			lpPalInfo->pi_rClient.top;

	/* was 10(tiny),6(small),4(medium),2(large) */
	nPerInch    = nEntriesPerInch [nSquareSize];
//	*lpcxSquare = cxInch / nPerInch;
//	*lpcySquare = cyInch / nPerInch;
	icx = cxInch / nPerInch;
	icy = cyInch / nPerInch;

//	if( *lpcxSquare == 0 )
	if( icx == 0 )
		icx = 1;

//	if( *lpcySquare == 0 )
	if( icy == 0 )
		icy = 1;

	*lpcxSquare = icx;
	*lpcySquare = icy;

	// Translate palette squares per inch into # of columns,
	//  and pixels per square.  Insure that we have at least
	//  one column, and 1 pix per side on our squares.

//	*lpnCols = (int) ((((long) rect.right - rect.left) *
//		nPerInch) /
//		cxInch );
//	*lpnRows = (int) ((((long) rect.bottom - rect.top) *
//		nPerInch) /
//		cyInch );
//	iCols = (int) ((((long) rect.right - rect.left) *
//		nPerInch) /
//		cxInch );
//	iRows = (int) ((((long) rect.bottom - rect.top) *
//		nPerInch) /
//		cyInch );
	iCols = (int) ((((long) pt.x ) * nPerInch ) /
		cxInch );
	iRows = (int) ((((long) pt.y ) * nPerInch ) /
		cyInch );

//	if( !*lpnCols )
	if( !iCols )
		iCols = 1;

//	if( !*lpnRows )
	if( !iRows )
		iRows = 1;

	*lpnCols = iCols;
	*lpnRows = iRows;

//		"At %d per inch, using square (%d,%d) gives %d x %d (cols x rows) oer window.",
	wsprintf( lpd,
		"At %d/ins, square (%d,%d) is %dx%d (cols & rows) per window.",
		nPerInch,
		icx,icy,
		iCols,iRows );
	DO(lpd);

	if( ( hg  = lpPalInfo->pi_hCopyCOLR ) &&
		( lpb = DVGlobalLock(hg)        ) )
	{
		DWORD	dwCnt = GetCOLRCnt(lpb);
		DVGlobalUnlock(hg);
		wsprintf( lpd,
			"Window (%d x %d) to hold %d colours. (%d per row)",
			lpPalInfo->pi_rClient.right,
			lpPalInfo->pi_rClient.bottom,
			dwCnt,
			(lpPalInfo->pi_rClient.right / icx) );
		DO(lpd);
	}
}

// what does this REALLY do???
void PalInitSize( DWORD ccnt, LPPOINT lpp )
{
	HDC  hDC;
	int  cxInch, cyInch;
	int  nPerInch;
	int  cxSquare, cySquare;
	int  xCols, yRows;
	int  xAdd, yAdd;
	TEXTMETRIC  tm;
	int		iOff;
	LPSTR	lpd = &gszDiag[0];

	/* should be from INI */
	xCols = 32;
	yRows =  8 * 2;

	hDC    = GetDC (NULL);
	cxInch = GetDeviceCaps( hDC, LOGPIXELSX );
	cyInch = GetDeviceCaps( hDC, LOGPIXELSY );
	GetTextMetrics( hDC, &gsTM );
	ReleaseDC (NULL, hDC);

	tm = gsTM;
	/* was 10(tiny),6(small),4(medium),2(large) */
//	nPerInch    = nEntriesPerInch[PAL_SIZE_DEF - IDM_PAL_TINY];
	iOff  = PAL_SIZE_DEF - IDM_PAL_TINY;
	nPerInch    = nEntriesPerInch[iOff];
	iOff += IDM_PAL_TINY;
	switch( iOff )
	{
	case IDM_PAL_TINY:
		break;
	case IDM_PAL_SMALL:
		xCols = 32;
		yRows =  8 * 2;
		break;
	case IDM_PAL_MEDIUM:
		break;
	case IDM_PAL_LARGE:
		break;
	}

	/* at say 6 per inch, square is about 16 x 16 */
	cxSquare = cxInch / nPerInch;
	cySquare = cyInch / nPerInch;
	if( cxSquare == 0 )
		cxSquare = 1;
	if( cySquare == 0 )
		cySquare = 1;
	/* default is about 16   x 32 coloums = 512 ok */
	lpp->x  = (int) cxSquare * xCols;
	/* also 16 thus about 4 = 64 + Others = 138 */
	lpp->y  = (int) cySquare * yRows;

	xAdd  = GetSystemMetrics( SM_CXFRAME ) * 2;
	xAdd += GetSystemMetrics( SM_CXVSCROLL );

	yAdd  = GetSystemMetrics( SM_CYCAPTION );
	yAdd += GetSystemMetrics( SM_CYMENU );
	yAdd += GetSystemMetrics( SM_CYBORDER ) * 2;

//	yAdd += tm.tmHeight;
	yAdd += gsTM.tmHeight;

	wsprintf( lpd,
		"Initialising PAL window to %d x %d (Client %dx%d).",
		lpp->x + xAdd,
		lpp->y + yAdd,
		lpp->x,
		(lpp->y + gsTM.tmHeight) );
//		(lpp->y + tm.tmHeight) );
	DO(lpd);

	lpp->x += xAdd;
	lpp->y += yAdd;

}


//---------------------------------------------------------------------
//
// Function:   SetPaletteWindowsPal
//
// Purpose:    Set a palette Window's hPal in its PALINFO structure.
//             This sets the palette that will be displayed in the
//             given window.  Also sets up the other structure members
//             of the PALINFO structure.
//
// Parms:      hWnd == Window we're going to display palette in.
//             hPal == Palette to display in the window.
//
// Notes:      This is called during the CREATE message of the
//             Pallette child window to set the NUMBER of
//             entries (wDEntries) in the Palette info structure.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

void SetPaletteWindowsPal( HWND hWnd, HPALETTE hPal )
{
	HANDLE    hPalInfo;
	PPI lpPalInfo;

	if( !hPal )
		return;

	hPalInfo = GetWindowExtra( hWnd, WW_PAL_HPALINFO );

	if( ( hPalInfo ) &&
		( lpPalInfo = (PPI) DVGlobalLock( hPalInfo ) ) )
	{
		lpPalInfo->hPal     = hPal;
		lpPalInfo->wPEntries = ColorsInPalette (hPal);
		DVGlobalUnlock( hPalInfo );
	}

}



//---------------------------------------------------------------------
//
// Function:   DVPalEntriesOnDevice
//
// Purpose:    Returns the number of colors a device supports.
//
// Parms:      hDC == DC for the device we want # of colors for.
//
// Returns:    # of colors that the given device can represent.
//
// History:   Date			Reason
//             6/01/91		Created
//				June, 1997	Realization that this can return -1
//							which indicates a 24-bit system.
//							*** NO PALETTES, except 256 colors ***
//		grm	 Oct 99 - more support for 24-bit color machines
//             
//---------------------------------------------------------------------
int DVPalEntriesOnDevice( HDC hDC )
{
	int	i;

	i = 0;	// Set NONE

	// Find out the number of palette entries on this
	// device.
	if( GetDeviceCaps( hDC, RASTERCAPS ) & RC_PALETTE )
	{
		// Only if it IS a PALETTE based system will this
		// yield a meaningful value.
		i = GetDeviceCaps( hDC, SIZEPALETTE );
	}

	// For non-palette devices, we'll use the # of system
	//  colors for our palette size.
	if( !i )
	{
		i = GetDeviceCaps( hDC, NUMCOLORS );
	}

//	assert( i );

	return i;	// NOTE: This value can be -1
	// when the system supports 24-bit color.
	// ======================================

}


//---------------------------------------------------------------------
//
// Function:   DVGetSystemPalette
//
// Purpose:    This routine returns a handle to a palette,
//				which represents the system palette.
//				Each entry is an offset into the system
//				palette instead of an RGB with a flag of
//				PC_EXPLICIT.
//
//             Useful for dumping the system palette.
//
// Parms:      None
//
// Returns:    Handle to a palette consisting of the system palette
//             colors.
//
// History:   Date			Reason
//             6/01/91		Created
//				June 1997	What about 24-bit systems?
//
// Called during IDM_CAPTFULLSCREEN, and others?
//---------------------------------------------------------------------

HPALETTE DVGetSystemPalette( BOOL bShwErr )
{
	HDC			   hDC;
	HPALETTE	      hPal;
	HANDLE		   hLogPal;
	LPLOGPALETTE   lpLogPal;
	int			   i, nColors;
	LPDWORD		   lpdw;

	hPal = NULL;	// NO, we do not have a PALETTE

	// Find out how many palette entries we want.
	hDC = GetDC( NULL );
	if( !hDC )
	{
      if( bShwErr )
      {
		   DIBError( ERR_GETDC );
      }
		return NULL;
	}

	nColors = DVPalEntriesOnDevice( hDC );

	ReleaseDC (NULL, hDC);

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
			   DIBError( ERR_MEMORY ); // ERR_CREATEPAL );
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
      if(( bShwErr       ) &&
	      ( nColors != -1 ) )   // FIX980412 - add FIX20020916
      {
		   DIBError( ERR_CREATEPAL );
      }
		return NULL;
	}

	return hPal;
}



// Return a HDC for the default printer.
HDC GetPrinterDC2( void )
{
   PRINTDLG pdlg;
   ZeroMemory( &pdlg, sizeof(PRINTDLG) );
   pdlg.lStructSize = sizeof(PRINTDLG);
   pdlg.Flags = PD_RETURNDEFAULT | PD_RETURNDC;
   PrintDlg( &pdlg );
   return pdlg.hDC; 
} 

// Create a copy of the current system palette.
HPALETTE GetSystemPalette2( VOID )
{
   HDC            hDC;
   HPALETTE       hPal;
   //HANDLE         hLogPal;
   LPLOGPALETTE   lpLogPal; 

   // Get a DC for the desktop.
   hDC = GetDC(NULL);
   if( !hDC )
      return NULL;

   // Check to see if you are a running in a palette-based video mode.
   if( !( GetDeviceCaps( hDC, RASTERCAPS ) & RC_PALETTE ) )
   {
      ReleaseDC(NULL, hDC);
      return NULL;
   }

   // Allocate memory for the palette.
   lpLogPal = DVGlobalAlloc( GPTR, sizeof(LOGPALETTE) + 256 * sizeof(PALETTEENTRY));
   if( !lpLogPal )
   {
      ReleaseDC(NULL, hDC);
      return NULL;
   }


   // Initialize.
   lpLogPal->palVersion    = 0x300;
   lpLogPal->palNumEntries = 256; 

   // Copy the current system palette into the logical palette.
   GetSystemPaletteEntries( hDC, 0, 256,
      (LPPALETTEENTRY)(&lpLogPal->palPalEntry) ); 

   // Create the palette.
   hPal = CreatePalette(lpLogPal); 

   // Clean up.
   DVGlobalFree(lpLogPal);
   ReleaseDC(NULL, hDC); 

   return hPal; 

} 


// Create a 24-bit-per-pixel surface.
HBITMAP Create24BPPDIBSection2( HDC hDC, int iWidth, int iHeight )
{
   BITMAPINFO bmi;
   HBITMAP hbm;
   LPBYTE pBits;

   // Initialize to 0s.
   ZeroMemory( &bmi, sizeof(BITMAPINFO) );
   // Initialize the header.
   bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
   bmi.bmiHeader.biWidth       = iWidth;
   bmi.bmiHeader.biHeight      = iHeight;
   bmi.bmiHeader.biPlanes      = 1;
   bmi.bmiHeader.biBitCount    = 24;
   bmi.bmiHeader.biCompression = BI_RGB;


   // Create the surface.
   hbm = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0 ); 

   return(hbm);
}

// Print the entire contents (including the non-client area) of
// the specified window to the default printer.
BOOL  PrintWindowToDC2( HWND hWnd )
{
   HBITMAP hbm;
   HDC     hdcPrinter;
   HDC     hdcMemory;
   HDC     hdcWindow;
   int     iWidth;
   int     iHeight;
   DOCINFO di;
   RECT    rc;
   DIBSECTION ds;
   HPALETTE   hPal; 


   // Do you have a valid window?
   if( !IsWindow(hWnd) )
      return FALSE; 


   // Get a HDC for the default printer.
   hdcPrinter = GetPrinterDC2();
   if( !hdcPrinter )
      return FALSE; 


   // Get the HDC for the entire window.
   hdcWindow  = GetWindowDC( hWnd ); 
   if( !hdcWindow )
   {
      DeleteDC(hdcPrinter);
      return FALSE;
   }

   // Get the rectangle bounding the window.
   GetWindowRect( hWnd, &rc ); 

   // Adjust coordinates to client area.
   OffsetRect( &rc, -rc.left, -rc.top ); 

   // Get the resolution of the printer device.
   iWidth  = GetDeviceCaps(hdcPrinter, HORZRES);
   iHeight = GetDeviceCaps(hdcPrinter, VERTRES); 

   // Create the intermediate drawing surface at window resolution.
   hbm = Create24BPPDIBSection2( hdcWindow, rc.right, rc.bottom );
   if( !hbm )
   {
      DeleteDC(hdcPrinter);
      ReleaseDC(hWnd, hdcWindow);
      return FALSE;
   } 

   // Prepare the surface for drawing.
   hdcMemory = CreateCompatibleDC( hdcWindow );
   if( !hdcMemory )
   {
      DeleteObject(hbm);
      DeleteDC(hdcPrinter);
      ReleaseDC(hWnd, hdcWindow);
      return FALSE;
   }

   SelectObject( hdcMemory, hbm ); 


   // Get the current system palette.
   hPal = GetSystemPalette2(); 


   // If a palette was returned.
   if( hPal )
   {
      // Apply the palette to the source DC.
      SelectPalette( hdcWindow, hPal, FALSE );
      RealizePalette( hdcWindow ); 

      // Apply the palette to the destination DC.
      SelectPalette( hdcMemory, hPal, FALSE );
      RealizePalette( hdcMemory );
   } 


   // Copy the window contents to the memory surface.
   BitBlt (hdcMemory,
      0, 0, rc.right, rc.bottom,
      hdcWindow,
      0, 0,
      SRCCOPY ); 


   // Prepare the DOCINFO.
   ZeroMemory(&di, sizeof(DOCINFO) );
   di.cbSize = sizeof(DOCINFO);
   di.lpszDocName = "Window Contents"; 


   // Initialize the print job.
   if( StartDoc( hdcPrinter, &di ) > 0 )
   { 
      // Prepare to send a page.
      if( StartPage( hdcPrinter ) > 0 )
      { 
         // Retrieve the information describing the surface.
         GetObject( hbm, sizeof(DIBSECTION), &ds );
         // Print the contents of the surface.
         StretchDIBits( hdcPrinter,
            0, 0, iWidth, iHeight,
            0, 0, rc.right, rc.bottom,
            ds.dsBm.bmBits,
            (LPBITMAPINFO)&ds.dsBmih,
            DIB_RGB_COLORS,
            SRCCOPY ); 

         // Let the driver know the page is done.
         EndPage( hdcPrinter );
      } 

      // Let the driver know the document is done.
      EndDoc( hdcPrinter );
   } 


   // Clean up the objects you created.
   DeleteDC(hdcPrinter);
   DeleteDC(hdcMemory);
   ReleaseDC(hWnd, hdcWindow);
   DeleteObject(hbm);
   if( hPal )
      DeleteObject(hPal);

   return TRUE;

}

VOID  Do_IDM_PRINTSCRN( HWND hWnd )
{
   HWND  hwnd;
   if( hwnd = GetDesktopWindow() )
   {
      if( gbHide )
         DVHideWindow( ghMainWnd );

      PrintWindowToDC2( hwnd );

      if( gbHide )
         ShowWindow( ghMainWnd, SW_SHOW );
   }
}

VOID  Do_IDM_PAL_PRINT( HWND hWnd )
{
   PrintWindowToDC2( hWnd );
}


//---------------------------------------------------------------------
//
// Function:   ColorsInPalette
//
// Purpose:    Given a handle to a palette, returns the # of colors
//             in that palette.
//
// Parms:      hPal == Handle to palette we want info on.
//
// Returns:    # of colors in the palette.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

int ColorsInPalette( HPALETTE hPal )
{
	int nColors;

	if( !hPal )
		return 0;

	GetObject( hPal, sizeof( nColors ), (LPSTR) &nColors );

	return nColors;
}

//---------------------------------------------------------------------
//
// Function:   MyAnimatePalette
//
// Purpose:    This routine animates the given palette.  It does this
//             by moving all the palette entries down one in the palette,
//             and putting the first entry at the end of the palette (we
//             could do something different here, like run various shades
//             of a certain color through the palette, or run random colors
//             through the palette).
//
//             Not really that useful -- it just creates a pretty funky
//             "psychadelic" effect.  It does show how to do palette
//             animation, though.
//
//             This routine is called by CHILD.C.
//
// Parms:      hWnd == Window we're animating.
//             hPal == Palette we're animating.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

void MyAnimatePalette (HWND hWnd, HPALETTE hPal)
{
	HDC				hDC;
	HANDLE			hPalEntries;
	LPPALETTEENTRY	lpPalEntries;
	WORD			wEntries, i;
	HPALETTE		hOldPal;
	PALETTEENTRY	pe;

	if( !hPal )
		return;

	wEntries = ColorsInPalette( hPal );

	if( !wEntries )
		return;

	hPalEntries = DVGlobalAlloc( GHND,
		sizeof (PALETTEENTRY) * wEntries );

	if( !hPalEntries )
		return;

	lpPalEntries = (LPPALETTEENTRY) DVGlobalLock( hPalEntries );

	if( lpPalEntries == 0 )
	{
		DVGlobalFree( hPalEntries );	// Toss allocation
		return;
	}

	GetPaletteEntries( hPal, 0, wEntries, lpPalEntries );

	pe = lpPalEntries[0];	// Get FIRST
	for( i = 0;  i < wEntries - 1;  i++ )
	{
		// Move all down one
		lpPalEntries[i] = lpPalEntries[i+1];
	}
	lpPalEntries[wEntries - 1] = pe;	// Insert 1st at END

	if( hDC = GetDC( hWnd ) )
	{
		hOldPal = SelectPalette( hDC, hPal, FALSE );

		AnimatePalette( hPal, 0, wEntries, lpPalEntries );

		if( hOldPal )
			SelectPalette( hDC, hOldPal, FALSE );

		ReleaseDC( hWnd, hDC );
	}

	DVGlobalUnlock( hPalEntries );
	DVGlobalFree( hPalEntries );

}


//---------------------------------------------------------------------
//
// Function:   CopyPaletteChangingFlags
//
// Purpose:    Duplicate a given palette, changing all the flags in
//             it to a certain flag value (i.e. peFlags member of
//             the PALETTEENTRY structure).
//
// Parms:      hPal     == Handle to palette to duplicate.
//             bNewFlag == New peFlags PALETTEENTRY flag.  Set
//                         to DONT_CHANGE_FLAGS if don't want
//                         to touch the flags.
//
// Returns:    Handle to the new palette.  NULL on error.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

HPALETTE CopyPaletteChangingFlags (HPALETTE hPal, BYTE bNewFlag)
{
	WORD			wEntries, i;
	HANDLE			hLogPal;
	LPLOGPALETTE	lpLogPal;

	if( !hPal )
		return NULL;

	wEntries = ColorsInPalette( hPal );

	if( !wEntries )
		return NULL;

	hLogPal = DVGlobalAlloc( GHND,
		sizeof (LOGPALETTE) + (sizeof (PALETTEENTRY) * wEntries) );

	if( !hLogPal )
		return NULL;

	lpLogPal = (LPLOGPALETTE) DVGlobalLock( hLogPal );

	if( lpLogPal == 0 )
	{
		DVGlobalFree( hLogPal );
		return NULL;
	}

	lpLogPal->palVersion    = PALVERSION;
	lpLogPal->palNumEntries = wEntries;

	GetPaletteEntries( hPal, 0, wEntries, lpLogPal->palPalEntry );

	if( bNewFlag != DONT_CHANGE_FLAGS )
	{
		for( i = 0;  i < wEntries;  i++ )
		{
			lpLogPal->palPalEntry[i].peFlags = bNewFlag;
		}
	}

	hPal = CreatePalette( lpLogPal );

	DVGlobalUnlock (hLogPal);
	DVGlobalFree (hLogPal);

	return hPal;
}

#ifdef	FRMENU

/*	========================================================
	void	SetFrChecks( HMENU hmenuPopup, PPI lpPalInfo )

	NOTE: Changes in the RC MUST be reflected HERE !!!
	**************************************************
    POPUP "&Affichage"
    BEGIN
        POPUP "&Type"
        BEGIN
            MENUITEM "&Squares",    IDM_PAL_SQUARES
            POPUP "&Square Size"
            BEGIN
                MENUITEM "&Tiny",       IDM_PAL_TINY
                MENUITEM "&Small",      IDM_PAL_SMALL
                MENUITEM "&Medium",     IDM_PAL_MEDIUM
                MENUITEM "&Large",      IDM_PAL_LARGE
                MENUITEM SEPARATOR
                MENUITEM "S&plit",      IDM_PAL_SPLIT
            END
            MENUITEM SEPARATOR
            MENUITEM "&Histogram",  IDM_PAL_HISTO
        END
        POPUP "Order"
        BEGIN
            MENUITEM "&Natural",    IDM_PAL_NORMAL
            MENUITEM "&Rainbow",    IDM_PAL_RYBIV
            MENUITEM "&Intensity",  IDM_PAL_INTENSITY
            MENUITEM "N&umeric",    IDM_PAL_NUMERIC
            MENUITEM "&Grayness",   IDM_PAL_GRAY
            MENUITEM "&Grayness",   IDM_PAL_FREQ
            MENUITEM SEPARATOR
            MENUITEM "In&vert",     IDM_PAL_INVERT
        END
    END


	========================================================	*/

void	SetFrChecks( HMENU hmenuPopup, PPI lpPalInfo )
{

//        POPUP "&Type"
//        BEGIN
			//if( giDrwHisto )
		// MENUITEM "&Squares",    IDM_PAL_SQUARES
//		CheckMenuItem( hmenuPopup,
//			IDM_PAL_SQUARES,
//			( MF_BYCOMMAND | MF_ENABLED |
//			( giDrwHisto ? MF_UNCHECKED : MF_CHECKED ) ) );
		CheckMenuItem( hmenuPopup,
			IDM_PAL_SQUARES,
			( MF_BYCOMMAND | MF_ENABLED |
			( giDrwSquares ? MF_CHECKED : MF_UNCHECKED ) ) );
//            POPUP "&Square Size"
//            BEGIN

//                MENUITEM "&Tiny",       IDM_PAL_TINY
//                MENUITEM "&Small",      IDM_PAL_SMALL
//                MENUITEM "&Medium",     IDM_PAL_MEDIUM
//                MENUITEM "&Large",      IDM_PAL_LARGE
//                MENUITEM SEPARATOR

//                MENUITEM "S&plit",      IDM_PAL_SPLIT
				CheckMenuItem( hmenuPopup,
					IDM_PAL_SPLIT,
					( MF_BYCOMMAND | MF_ENABLED |
					( gfDrwPol ? MF_CHECKED : MF_UNCHECKED ) ) );

//            END
//            MENUITEM SEPARATOR
//            MENUITEM "&Histogram",  IDM_PAL_HISTO
				CheckMenuItem( hmenuPopup,
					IDM_PAL_HISTO,
					( MF_BYCOMMAND | MF_ENABLED |
					( giDrwHisto ? MF_CHECKED : MF_UNCHECKED ) ) );

//        END
//        POPUP "Order"
//        BEGIN
//            MENUITEM "&Natural",    IDM_PAL_NORMAL
				CheckMenuItem( hmenuPopup,
					IDM_PAL_NORMAL,
					( MF_BYCOMMAND | MF_ENABLED |
					( lpPalInfo->pi_iNormal ? MF_UNCHECKED : MF_CHECKED ) ) );

//            MENUITEM "&Rainbow",    IDM_PAL_RYBIV
				CheckMenuItem( hmenuPopup,
					IDM_PAL_RYBIV,
					( MF_BYCOMMAND | MF_ENABLED |
					( ( lpPalInfo->pi_iNormal & pif_Rainbow ) ? MF_CHECKED : MF_UNCHECKED  ) ) );


//            MENUITEM "&Intensity",  IDM_PAL_INTENSITY
				CheckMenuItem( hmenuPopup,
					IDM_PAL_INTENSITY,
					( MF_BYCOMMAND | MF_ENABLED |
					( ( lpPalInfo->pi_iNormal & pif_Intensity ) ? MF_CHECKED : MF_UNCHECKED  ) ) );
//            MENUITEM "N&umeric",    IDM_PAL_NUMERIC
				CheckMenuItem( hmenuPopup,
					IDM_PAL_NUMERIC,
					( MF_BYCOMMAND | MF_ENABLED |
					( ( lpPalInfo->pi_iNormal & pif_Numeric ) ? MF_CHECKED : MF_UNCHECKED  ) ) );
				CheckMenuItem( hmenuPopup,
					IDM_PAL_GRAY,
					( MF_BYCOMMAND | MF_ENABLED |
					( ( lpPalInfo->pi_iNormal & pif_Gray ) ? MF_CHECKED : MF_UNCHECKED  ) ) );
				CheckMenuItem( hmenuPopup,
					IDM_PAL_FREQ,
					( MF_BYCOMMAND | MF_ENABLED |
					( ( lpPalInfo->pi_iNormal & pif_Freq ) ? MF_CHECKED : MF_UNCHECKED  ) ) );
//            MENUITEM SEPARATOR
//            MENUITEM "In&vert",     IDM_PAL_INVERT
				CheckMenuItem( hmenuPopup,
					IDM_PAL_INVERT,
					( MF_BYCOMMAND | MF_ENABLED |
					( lpPalInfo->pi_iInvert ? MF_CHECKED : MF_UNCHECKED ) ) );
//        END
}

long PalWM_INITMENUPOPUP( HWND hWnd, UINT message,
						  WPARAM wParam, LPARAM lParam )
{
	long	pret = 0;
	HWND	hCurWnd;
	HMENU	hmenuPopup, hMen, hSub = 0;
	UINT	uPos;
	BOOL	fSystemMenu;
	HGLOBAL	    hPalInfo  = 0;
	PPI	lpPalInfo = 0;

	hmenuPopup = (HMENU) wParam;         // handle of submenu
	uPos = (UINT) LOWORD(lParam);        // submenu item position
	fSystemMenu = (BOOL) HIWORD(lParam); // window menu flag 
	pret = 0;
	hCurWnd = GetCurrentMDIWnd ();
	if( hMen = GetMenu( hWnd ) )
		hSub = GetSubMenu( hMen, uPos );

	if( ( !fSystemMenu                                        ) &&
		 ( hPalInfo  = GetWindowExtra( hWnd, WW_PAL_HPALINFO ) ) &&
		 ( lpPalInfo = (PPI) DVGlobalLock( hPalInfo )          ) )
	{
		switch( uPos )
		{
		case FP_FILE:
         // nothing to do at this position
			break;
		case FP_EDIT:
			{
				/* disable the EDIT until completed */
				/* IDM_PAL_COPY and IDM_PAL_PASTE */
				EnableMenuItem( hmenuPopup,	// handle to menu
					IDM_PAL_COPY,	// menu item to enable, disable, or gray
					( MF_BYCOMMAND | MF_DISABLED | MF_GRAYED ) );	// menu item flags
 
				EnableMenuItem( hmenuPopup,	// handle to menu
					IDM_PAL_PASTE,	// menu item to enable, disable, or gray
					( MF_BYCOMMAND | MF_DISABLED | MF_GRAYED ) );	// menu item flags

			}
			break;

		case FP_VIEW:
			{
				SetFrChecks( hmenuPopup, lpPalInfo );
				/* check 1 - Is order NORMAL ie as found in BMP */

//				CheckMenuItem( hmenuPopup,
//					IDM_PAL_NORMAL,
//					( MF_BYCOMMAND | MF_ENABLED |
//					( lpPalInfo->pi_iNormal ? MF_UNCHECKED : MF_CHECKED ) ) );

//				CheckMenuItem( hmenuPopup,
//					IDM_PAL_INTENSITY,
//					( MF_BYCOMMAND | MF_ENABLED |
//					( ( lpPalInfo->pi_iNormal & pif_Intensity ) ? MF_CHECKED : MF_UNCHECKED  ) ) );
//				CheckMenuItem( hmenuPopup,
//					IDM_PAL_INVERT,
//					( MF_BYCOMMAND | MF_ENABLED |
//					( lpPalInfo->pi_iInvert ? MF_CHECKED : MF_UNCHECKED ) ) );
//				CheckMenuItem( hmenuPopup,
//					IDM_PAL_SPLIT,
//					( MF_BYCOMMAND | MF_ENABLED |
//					( gfDrwPol ? MF_CHECKED : MF_UNCHECKED ) ) );
//				CheckMenuItem( hmenuPopup,
//					IDM_PAL_HISTO,
//					( MF_BYCOMMAND | MF_ENABLED |
//					( giDrwHisto ? MF_CHECKED : MF_UNCHECKED ) ) );
//
			}
			break;
		}

//		lpPalInfo->hPal     = hPal;
//		lpPalInfo->wPEntries = ColorsInPalette (hPal);
		DVGlobalUnlock( hPalInfo );
	}
	else if( !fSystemMenu )
	{
		DIBError( ERR_MEMORY );	/* Some memory error ... */
	}

	return pret;
}

#else	/* !FRMENU */

//#define		PP_FILE		0
//#define		PP_SQUARE	1
//#define		PP_ORDER	2
//#define		pif_Intensity		0x00000001

long PalWM_INITMENUPOPUP( HWND hWnd, UINT message,
						  WPARAM wParam, LPARAM lParam )
{
	long	pret;
	HWND	hCurWnd;
	HMENU	hmenuPopup, hMen, hSub = 0;
	UINT	uPos;
	BOOL	fSystemMenu;
	HGLOBAL	    hPalInfo  = 0;
	PPI	lpPalInfo = 0;

	hmenuPopup = (HMENU) wParam;         // handle of submenu
	uPos = (UINT) LOWORD(lParam);        // submenu item position
	fSystemMenu = (BOOL) HIWORD(lParam); // window menu flag 
	pret = 0;
	hCurWnd = GetCurrentMDIWnd ();
	if( hMen = GetMenu( hWnd ) )
		hSub = GetSubMenu( hMen, uPos );

	if( ( !fSystemMenu                                        ) &&
		( hPalInfo  = GetWindowExtra( hWnd, WW_PAL_HPALINFO ) ) &&
		( lpPalInfo = (PPI) DVGlobalLock( hPalInfo )    ) )
	{
		switch( uPos )
		{
		case PP_FILE:
			break;
		case PP_SQUARE:
			break;
		case PP_ORDER:
			{
				CheckMenuItem( hmenuPopup,
					IDM_PAL_NORMAL,
					( MF_BYCOMMAND | MF_ENABLED |
					( lpPalInfo->pi_iNormal ? MF_UNCHECKED : MF_CHECKED ) ) );
				CheckMenuItem( hmenuPopup,
					IDM_PAL_INTENSITY,
					( MF_BYCOMMAND | MF_ENABLED |
					( ( lpPalInfo->pi_iNormal & pif_Intensity ) ? MF_CHECKED : MF_UNCHECKED  ) ) );
				CheckMenuItem( hmenuPopup,
					IDM_PAL_GRAY,
					( MF_BYCOMMAND | MF_ENABLED |
					( ( lpPalInfo->pi_iNormal & pif_Gray ) ? MF_CHECKED : MF_UNCHECKED  ) ) );
				CheckMenuItem( hmenuPopup,
					IDM_PAL_FREQ,
					( MF_BYCOMMAND | MF_ENABLED |
					( ( lpPalInfo->pi_iNormal & pif_Freq ) ? MF_CHECKED : MF_UNCHECKED  ) ) );
				CheckMenuItem( hmenuPopup,
					IDM_PAL_INVERT,
					( MF_BYCOMMAND | MF_ENABLED |
					( lpPalInfo->pi_iInvert ? MF_CHECKED : MF_UNCHECKED ) ) );
				CheckMenuItem( hmenuPopup,
					IDM_PAL_SPLIT,
					( MF_BYCOMMAND | MF_ENABLED |
					( gfDrwPol ? MF_CHECKED : MF_UNCHECKED ) ) );
				CheckMenuItem( hmenuPopup,
					IDM_PAL_HISTO,
					( MF_BYCOMMAND | MF_ENABLED |
					( giDrwHisto ? MF_CHECKED : MF_UNCHECKED ) ) );
			}
			break;
		}

//		lpPalInfo->hPal     = hPal;
//		lpPalInfo->wPEntries = ColorsInPalette (hPal);
		DVGlobalUnlock( hPalInfo );
	}
	else if( !fSystemMenu )
	{
		DIBError( ERR_MEMORY );	/* Some memory error ... */
	}

	return( pret );
}

#endif	/* FRMENU y/n */


// WM_ERASEBKGND
long PalWM_ERASEBKGND( HWND hWnd, UINT message,
						  WPARAM wParam, LPARAM lParam )
{
	long	pret;
	HDC		hDC;
	RECT	rc;
	HBRUSH	hb;

	if( ( hDC = (HDC) wParam         ) &&	// handle of device context
		( gfEraseBkGnd               ) &&
		( GetClientRect( hWnd, &rc ) ) &&
		( hb = CreateSolidBrush( RGB(0,0,0) ) ) )
	{
		FillRect( hDC, &rc, hb );
		DeleteObject(hb);
		pret = 1;
	}
	else
	{
		pret = DefWindowProc( hWnd, message, wParam, lParam );
	}
	return pret;
}


// Just PAINT a RAINBOW for me, and blend it into mist ...
void	PaintRainbow( HDC hDC, PPI lpPalInfo, LPSTR lpb,
				   LPRECT lpr, int nScroll )
{
	DWORD		dwi;
//	DWORD		dwCnt, dwMax;
	int			i, iWidth, iHeight;
	int			iWid, iHt;
	RECT		rect;
//	DWORD		dwf;
	COLORREF	cr;
	HBRUSH		hBrush, hOldBrush;
	LPSTR		lpt = &gszPText[0];
	BYTE		r,g,b;
	POINT		pt;
	HPEN		hPen, hOldPen;
	RECT		rTxt;
	RECT		rc;

	rTxt.left   = 0;
	rTxt.top    = 0;
	rTxt.right  = 1;
	rTxt.bottom = 1;
//		( dwCnt = GetCOLRCnt(lpb)              ) )
	if( ( lpr                                  ) &&
		( iWidth  = ( lpr->right - lpr->left ) ) &&
		( iHeight = ( lpr->bottom - lpr->top ) ) &&
		( iWidth  > 0                          ) &&
		( iHeight > 0                          ) )
	{
		DWORD	dwExt, dx, dy;

//		iWid  = iWidth / dwCnt;
//		if( iWid <= 0 )
//			iWid = 1;

//		lstrcpy( lpt, "(255,255,255)" );
		lstrcpy( lpt, "(125,125,125)" );
		dwExt = GetTextExtent( hDC, lpt, lstrlen(lpt) );
		dx = (DWORD)LOWORD(dwExt);
		dy = (DWORD)HIWORD(dwExt);

		if( dy > (DWORD)gsTM.tmHeight )
			iHt = dy;
		else
			iHt = gsTM.tmHeight;

		iHt += 4;
		if( iHeight <= iHt )
		{
			// only some TEXT
			chkit( "TOO SMALL to cut it." );
			goto Exit_PR;
		}

		rTxt = *lpr;

//		FrameRect( hDC, lpr, GetStockObject(WHITE_BRUSH) );
		ShadeRect( hDC, lpr, 6, &rTxt );

		if( ( iWidth  = ( rTxt.right - rTxt.left ) ) &&
			( iHeight = ( rTxt.bottom - rTxt.top ) ) &&
			( (DWORD)iWidth  > dx                  ) &&
			( iHeight > iHt                        ) &&
			( iWid  = iWidth / MCYCSET             ) ) /* say 12 */
		{
			// we can continue
		}
		else
		{
			iWid = 1;
			// only some TEXT
			chkit( "TOO SMALL to cut it 2." );
			goto Exit_PR;
		}

//		iHeight = (( iHeight * 9 ) / 10 );
		iHeight = iHeight - iHt;
//		rect = *lpr;
		rect = rTxt;
		rect.top  += iHt;
		rect.right = rect.left + iWid;

		pt.x = rTxt.left;

		// grCli.bottom -= gtm.tmHeight;
		if( dy < (DWORD)gtm.tmHeight )
			dy = gtm.tmHeight;

		if( ( dy + 3 ) < (DWORD)iHeight )
		{
			pt.y = rTxt.top + ( iHeight - ( dy + 3 ) );
		}
		else
		{
			pt.y = rTxt.top + 3;
		}

		/* get the MAX frequency */
//		dwMax = GetFREQMax(lpb);
		if( ( rect.left + ( iWid * (int)12 ) ) < iWidth )
		{
			int		id;
			/* bump first column over to centre graph */
			id = iWidth - ( rect.left + ( iWid * 12 ) );
			id = id / 2;
			if( id )
			{
				rc = rect;
				rc.right = rc.left + id;
//				if( ( rc.top + 1 ) < rc.bottom )
//				{
//					rc.top++;
//					rc.bottom--;
//				}
				FrameRect( hDC, &rc, GetStockObject(WHITE_BRUSH) );
//				wsprintf( lpt,
//					"%d",
//					dwMax );
//				i = lstrlen(lpt);
//				dx = LOWORD( GetTextExtent( hDC, lpt, i ) );
//				if( dx < (DWORD)id  )
//				{
//					TextOut( hDC,
//						( rc.left + ( ( id - dx ) / 2 ) ),
//						rc.top,
//						lpt,
//						i );
//				}

//				rc = *lpr;
				rc = rTxt;
				rc.top    = rect.top;
				rc.bottom = rect.bottom;
				rc.left = rc.right - id;
//				if( ( rc.top + 1 ) < rc.bottom )
//				{
//					rc.top++;
//					rc.bottom--;
//				}
				FrameRect( hDC, &rc, GetStockObject(WHITE_BRUSH) );
			}

			/* bump right a little */

			rect.left += id;
			rect.right = rect.left + iWid;
		}

		for( dwi = 0; dwi < 12; dwi++ )
		{
//	nColors   = lpPalInfo->wPEntries;	// Entries in the PALETTE
//	cxSquare  = lpPalInfo->cxSquare;
//	cySquare  = lpPalInfo->cySquare;
//	wCurrent  = lpPalInfo->wEntry;
			// cr  = GetCOLR( lpb, dwi );
			/* get the NEXT per the list */
			cr  = GetRYBColr( dwi );
			r = GetRValue(cr);
			g = GetGValue(cr);
			b = GetBValue(cr);
//			dwf = GetFREQ( lpb, dwi );
			/* calculate the HEIGHT of this colour */
//			iHt = ( dwf * iHeight ) / dwMax;
//			if( iHt < rect.bottom )
//				rect.top = rect.bottom - iHt;
//			else
//				rect.top = 0;
//			hBrush    = CreateSolidBrush( cr );
			hBrush    = CreateSolidBrush( RGB(r,g,b) );
			hOldBrush = SelectObject( hDC, hBrush );
			if( gfAddPenC )
			{
				hPen = CreatePen( PS_SOLID, 1, RGB(r,g,b) );
				hOldPen = SelectObject( hDC, hPen );
			}

			/* using current PEN and BRUSH, outline and fill resp */

			Rectangle( hDC,
				rect.left,
                rect.top,
                rect.right,
                rect.bottom );

			if( gfAddPenC )
			{
				SelectObject( hDC, hOldPen );
				DeleteObject( hPen );
			}
			SelectObject( hDC, hOldBrush );
			DeleteObject( hBrush );
			wsprintf( lpt,
				"(%d,%d,%d)",
				r, g, b );
			i = lstrlen(lpt);
			if( dx <= (DWORD)iWid )
			{
				int		ix;
				DWORD	dxx;
				dxx = (DWORD)LOWORD(GetTextExtent(hDC,lpt,i));
				if( dxx > (DWORD)iWid )
					dxx = (DWORD)iWid;
				ix = (( iWid - (int)dxx ) / 2);
				TextOut( hDC,
					(pt.x + ix),
					pt.y,
					lpt,
					i );
			}

			wsprintf( lpt, "SET12: At (%3d,%3d) put (%3d,%3d,%3d).",
				pt.x, pt.y,
				r, g, b );
			DO(lpt);

			/* move to right painting rect's in the colours */
			rect.left  += iWid;
			rect.right = rect.left + iWid;
			pt.x       += iWid;

		}	/* for COUNT of RAINBOW */

Exit_PR:

		wsprintf( lpt,
			"Rainbow of %d Colors.",
			12 );	// number of Colors in bitmap
		if( i = lstrlen( lpt ) )
		{
			DWORD	dxx;
			int		ix;
			dxx = (DWORD)LOWORD(GetTextExtent(hDC,lpt,i));
			if( dxx > (DWORD)iWidth )
					dxx = (DWORD)iWidth;
			ix = (( iWidth - (int)dxx ) / 2);
			TextOut( hDC,
				( rTxt.left + ix + 1 ),			// pt.x,
				( rTxt.top  + 1 ),	// pt.y,
				lpt,
				i );
			DO(lpt);
		}

		/* done each colour of the histogra */
		*lpt = 0;

	}
	else
	{
		chkit( "HUH! NO paint of RAINBOW *** " );
	}

	*lpt = 0;

}

void	PaintColorCyc( HDC hDC, PPI lpPalInfo, LPRECT lpRect )
{
	DWORD		dwi;
//	DWORD		dwCnt, dwMax;
	int			i, iWidth, iHeight;
	int			iWid, iHt;
	RECT		rect;
//	DWORD		dwf;
	COLORREF	cr;
	HBRUSH		hBrush, hOldBrush;
	LPSTR		lpt = &gszPText[0];
	BYTE		r,g,b;
	POINT		pt;
	HPEN		hPen, hOldPen;
	RECT		rTxt;
	RECT		rc;

	rTxt.left   = 0;
	rTxt.top    = 0;
	rTxt.right  = 1;
	rTxt.bottom = 1;
//		( dwCnt = GetCOLRCnt(lpb)              ) )
	if( ( lpRect                                      ) &&
		( iWidth  = ( lpRect->right  - lpRect->left ) ) &&
		( iHeight = ( lpRect->bottom - lpRect->top  ) ) &&
		( iWidth  >= MCYCSET                   ) &&
		( iHeight > 0                          ) )
	{
		DWORD	dwExt, dx, dy;

		rTxt = *lpRect;
		iWid  = iWidth / MCYCSET;
//		iWid  = iWidth / dwCnt;
//		if( iWid <= 0 )
//			iWid = 1;

		if( gfAddMTxt )
		{
//			lstrcpy( lpt, "(255,255,255)" );
			lstrcpy( lpt, "(125,125,125)" );
			dwExt = GetTextExtent( hDC, lpt, lstrlen(lpt) );
			dx = (DWORD)LOWORD(dwExt);
			dy = (DWORD)HIWORD(dwExt);

			if( dy > (DWORD)gsTM.tmHeight )
				iHt = dy;
			else
				iHt = gsTM.tmHeight;

			iHt += 4;
			if( iHeight <= iHt )
			{
				// only some TEXT
//				chkit( "TOO SMALL to cut it. Check BEFORE call!!!" );
				goto Exit_CC;
			}
		}

//		FrameRect( hDC, lpr, GetStockObject(WHITE_BRUSH) );
//		ShadeRect( hDC, lpr, 6, &rTxt );
//		if( ( iWidth  = ( rTxt.right - rTxt.left ) ) &&
//			( iHeight = ( rTxt.bottom - rTxt.top ) ) &&
//			( iHeight > 0                          ) &&
//			( iWidth  >= MCYCSET                   ) &&
//			( iWid  = iWidth / MCYCSET             ) ) /* say 12 */
//		{
//			// we can continue
//		}
//		else
//		{
//			iWid = 1;
//			// only some TEXT
//			chkit( "TOO SMALL to cut it 2." );
//			goto Exit_CC;
//		}

//		iHeight = (( iHeight * 9 ) / 10 );
//		iHeight = iHeight - iHt;
//		rect = *lpr;
		rect = rTxt;
//		rect.top  += iHt;
		rect.right = rect.left + iWid;
		pt.x = rTxt.left;
		pt.y = rTxt.top;

		// grCli.bottom -= gtm.tmHeight;
		if( dy < (DWORD)gtm.tmHeight )
			dy = gtm.tmHeight;

		if( ( dy + 3 ) < (DWORD)iHeight )
		{
			pt.y = rTxt.top + ( iHeight - ( dy + 3 ) );
		}
		else
		{
			pt.y = rTxt.top + 3;
		}

		if( ( rect.left + ( iWid * (int)MCYCSET ) + 1 ) < iWidth )
		{
			int		id;
			int		iw;
			/* bump first column over to centre graph */
			iw = ( rect.left + ( iWid * MCYCSET ) );
//			id = iWidth - ( rect.left + ( iWid * MCYCSET ) );
			id = iWidth - iw;
			id = id / 2;
			if( id )
			{
				rc = rect;
				rc.right = rc.left + id;
				FrameRect( hDC, &rc, GetStockObject(WHITE_BRUSH) );
				rc = rTxt;
				rc.top    = rect.top;
				rc.bottom = rect.bottom;
				rc.left = rc.right - id;
				FrameRect( hDC, &rc, GetStockObject(WHITE_BRUSH) );
			}
			/* bump right a little */
			rect.left += id;
			rect.right = rect.left + iWid;
		}

		for( dwi = 0; dwi < MCYCSET; dwi++ )
		{
//	nColors   = lpPalInfo->wPEntries;	// Entries in the PALETTE
//	cxSquare  = lpPalInfo->cxSquare;
//	cySquare  = lpPalInfo->cySquare;
//	wCurrent  = lpPalInfo->wEntry;
			// cr  = GetCOLR( lpb, dwi );
			/* get the NEXT per the list */
			cr  = GetRYBColr( dwi );
			r = GetRValue(cr);
			g = GetGValue(cr);
			b = GetBValue(cr);
//			dwf = GetFREQ( lpb, dwi );
			/* calculate the HEIGHT of this colour */
//			iHt = ( dwf * iHeight ) / dwMax;
//			if( iHt < rect.bottom )
//				rect.top = rect.bottom - iHt;
//			else
//				rect.top = 0;
//			hBrush    = CreateSolidBrush( cr );
			hBrush    = CreateSolidBrush( RGB(r,g,b) );
			hOldBrush = SelectObject( hDC, hBrush );
			if( gfAddPenC )
			{
				hPen = CreatePen( PS_SOLID, 1, RGB(r,g,b) );
				hOldPen = SelectObject( hDC, hPen );
			}

			/* using current PEN and BRUSH, outline and fill resp */

			Rectangle( hDC,
				rect.left,
                rect.top,
                rect.right,
                rect.bottom );

			if( gfAddPenC )
			{
				SelectObject( hDC, hOldPen );
				DeleteObject( hPen );
			}
			SelectObject( hDC, hOldBrush );
			DeleteObject( hBrush );

			if( gfAddMTxt )
			{
				wsprintf( lpt,
					"(%d,%d,%d)",
					r, g, b );
				i = lstrlen(lpt);
				if( dx <= (DWORD)iWid )
				{
					int		ix;
					DWORD	dxx;
					dxx = (DWORD)LOWORD(GetTextExtent(hDC,lpt,i));
					if( dxx > (DWORD)iWid )
						dxx = (DWORD)iWid;
					ix = (( iWid - (int)dxx ) / 2);
					TextOut( hDC,
						(pt.x + ix),
						pt.y,
						lpt,
						i );
				}

				wsprintf( lpt, "MCYCSET: At (%3d,%3d) put (%3d,%3d,%3d).",
					pt.x, pt.y,
					r, g, b );
				DO(lpt);

			}

			/* move to right painting rect's in the colours */
			rect.left  += iWid;
			rect.right = rect.left + iWid;
			pt.x       += iWid;

		}	/* for COUNT of MCYCSET */

Exit_CC:

		wsprintf( lpt,
			"Cycle of %d Colors.",
			MCYCSET );	// number of Colors in bitmap

		if( gfAddMTxt )
		{
			if( i = lstrlen( lpt ) )
			{
				DWORD	dxx;
				int		ix;
				dxx = (DWORD)LOWORD(GetTextExtent(hDC,lpt,i));
				if( dxx > (DWORD)iWidth )
						dxx = (DWORD)iWidth;
				ix = (( iWidth - (int)dxx ) / 2);
				TextOut( hDC,
					( rTxt.left + ix + 1 ),			// pt.x,
					( rTxt.top  + 1 ),	// pt.y,
					lpt,
					i );
				DO(lpt);
			}
		}

		/* done each colour of the fixed set */
		*lpt = 0;

	}
	else
	{
		chkit( "HUH! NO paint of COLOR CYCLE *** " );
	}

	*lpt = 0;

}


#define	MINSHADE	6

void	DoTop( HDC hDC, LPRECT lpr )
{
	MoveToEx( hDC,
		lpr->left,
		lpr->top,
		NULL );
	LineTo( hDC,
		lpr->right,
		lpr->top );
}

void	DoLeft( HDC hDC, LPRECT lpr )
{
	MoveToEx( hDC,
		lpr->left,
		lpr->top + 1,
		NULL );
	LineTo( hDC,
		lpr->left,
		lpr->bottom );
}

void	DoLeftUp( HDC hDC, LPRECT lpr, LPPOINT lpp )
{
	if( lpp )
	{
		MoveToEx( hDC,
			lpr->left,
			lpr->bottom,
			lpp );
	}
	LineTo( hDC,
		lpr->left,
		lpr->top );
}

void	DoTopR( HDC hDC, LPRECT lpr, LPPOINT lpp )
{
	if( lpp )
	{
		MoveToEx( hDC,
			lpr->left,
			lpr->top,
			lpp );
	}
	LineTo( hDC,
		lpr->right,
		lpr->top );
}

void	DoTopR1( HDC hDC, LPRECT lpr, LPPOINT lpp )
{
	if( lpp )
	{
		MoveToEx( hDC,
			lpr->left,
			lpr->top,
			lpp );
	}
	LineTo( hDC,
		lpr->left + 1,
		lpr->top );
}

void	DoRightDn( HDC hDC, LPRECT lpr, LPPOINT lpp )
{
	if( lpp )
	{
		MoveToEx( hDC,
			lpr->right,
			lpr->top,
			lpp );
	}
	LineTo( hDC,
		lpr->right,
		lpr->bottom );
}

void	DoBottomL( HDC hDC, LPRECT lpr, LPPOINT lpp )
{
	if( lpp )
	{
		MoveToEx( hDC,
			lpr->right,
			lpr->bottom,
			lpp );
	}
	LineTo( hDC,
		lpr->left,
		lpr->bottom );
}
void	DoBottomLU1( HDC hDC, LPRECT lpr, LPPOINT lpp )
{
	if( lpp )
	{
		MoveToEx( hDC,
			lpr->right,
			lpr->bottom,
			lpp );
	}
	LineTo( hDC,
		lpr->right,
		lpr->bottom - 1 );
}


void	DoRight( HDC hDC, LPRECT lpr )
{
	MoveToEx( hDC,
		lpr->right,
		lpr->top + 1,
		NULL );
	LineTo( hDC,
		lpr->right,
		lpr->bottom );
}

void	DoBottom( HDC hDC, LPRECT lpr )
{
	MoveToEx( hDC,
		lpr->left,
		lpr->bottom,
		NULL );
	LineTo( hDC,
		lpr->right,
		lpr->bottom );
}

void	PutFrame( HDC hDC, LPRECT lpr, HPEN htl, HPEN hbr )
{
	RECT	rshd;
	POINT	pt;

	rshd = *lpr;
	SelectObject( hDC, htl );
	DoLeftUp( hDC, lpr, &pt );
	DoTopR( hDC, lpr, 0 );
	SelectObject( hDC, hbr );
	DoRightDn( hDC, lpr, 0 );
	DoBottomL( hDC, lpr, 0 );
	DoTopR1( hDC, lpr, &pt );
	SelectObject( hDC, htl );
	DoBottomLU1( hDC, lpr, &pt );

}

//void	PaintShade( HDC hDC, LPSDRV1 lpsd, HPEN hpb, LPRECT lpr )
void	ShadeRect( HDC hDC, LPRECT lpr, int iSz, LPRECT lpret )
{
	RECT	rshd;
	HPEN	hOldPen;
	int		i, j;
	LPRECT	lprc;
	HPEN	htl, hbr;

	htl = hbr = hOldPen = 0;
	j   = iSz;
	if( ( htl = CreatePen( PS_SOLID,
			01,			/* width of the PEN */
			RGB(250,250,250) ) ) &&		/* Create PEN(bright) */
		( hbr = CreatePen( PS_SOLID,
			01,			/* width of the PEN */
			RGB(15,15,15) ) ) )		/* Create PEN(dull) */
	{
		// continue ...
	}
	else
	{
		if(htl)
			DeleteObject(htl);
		chkit( "WHAT! A RESOURCE FAILURE? TOO MANY HANDLES???" );
		return;
	}

	hOldPen = SelectObject( hDC, htl );
//	rshd = rc;
//	if( lpr )
		lprc = lpr;
//	else
//		lprc = &lpsd->sd_rMax;

	/* rshd = *lpsd->sd_rMax;	-- contents of MAX rect */
	rshd = *lprc;	/* contents of rect structure */
//	if( ( giHorAdd  ) &&
//		( giVertAdd ) )
//	{
//		j = min(giHorAdd, giVertAdd);
		if( j < MINSHADE )
			j = MINSHADE;
//	}
//	else
//	{
//		j = 0;
//	}

	for( i = 0; i < j; i++ )
	{
		if( ( rshd.left < rshd.right  ) &&
			( rshd.top  < rshd.bottom ) )
		{
			/* we can cycle some more */
			//PaintShFrame( hDC, &rshd, htl, hbr );
			PutFrame( hDC, &rshd, htl, hbr );

			/* in from left edge */
			rshd.left++;

			/* down a little     */
			rshd.top++;

			/* if right, reduce */
			if( rshd.right > 0 )
				rshd.right--;

			/* if bottom, reduce */
			if( rshd.bottom > 0 )
				rshd.bottom--;

		}
		else
		{
			break;
		}
	}

	if( hOldPen )
		SelectObject( hDC, hOldPen );


	/* kill the Pens used to make the SHADE FRAME */
	DeleteObject(htl);
	DeleteObject(hbr);

	if( lpret )
	{
		if( i == j )
		{
			/* in from left edge */
			rshd.left++;

			/* down a little     */
			rshd.top++;

			/* if right, reduce */
			if( rshd.right > 0 )
				rshd.right--;

			/* if bottom, reduce */
			if( rshd.bottom > 0 )
				rshd.bottom--;
		}

		*lpret = rshd;	/* return last paint */
	}

}


void	PaintHisto_OK( HDC hDC, PPI lpPalInfo, LPSTR lpb,
				   LPRECT lpRect, int nScroll )
{
	DWORD	   dwi, dwCnt, dwMax;
	int		i, iWidth, iHeight;
	int		iWid, iHt;
	RECT	   rect;
	DWORD		dwf;
	COLORREF	cr;
	HBRUSH	hBrush, hOldBrush;
	LPSTR		lpt = &gszPText[0];
	BYTE		r,g,b;
	POINT		pt;
	HPEN		hPen, hOldPen;
	RECT		rTxt;
	RECT		rc;
	RECT		rFrm;
   LPPALEX  pmpalx;

	if( ( lpRect                               ) &&
		( iWidth  = ( lpRect->right - lpRect->left ) ) &&
		( iHeight = ( lpRect->bottom - lpRect->top ) ) &&
		( iWidth  > 0                          ) &&
		( iHeight > 0                          ) &&
		( dwCnt = GetCOLRCnt(lpb)              ) )
	{
		DWORD	dwExt, dx, dy;
		iWid  = iWidth / dwCnt;
		if( iWid <= 0 )
			iWid = 1;

      pmpalx = (LPPALEX)lpb;
      sprtf( "PaintHisto to %s width %d of %d (%d) colors."MEOR,
         Rect2Stg( lpRect ),
         iWid,
         dwCnt,
         pmpalx->px_Count );

      iHt = 0;    // assume NO text
      if( gfAddCText )
      {
         //lstrcpy( lpt, "(255,255,255)" );
		   lstrcpy( lpt, "(125,125,125)" );
		   dwExt = GetTextExtent( hDC, lpt, lstrlen(lpt) );
		   dx = (DWORD)LOWORD(dwExt);
		   dy = (DWORD)HIWORD(dwExt);

		   if( dy > (DWORD)gsTM.tmHeight )
			   iHt = dy;
		   else
			   iHt = gsTM.tmHeight;

		   iHt += 4;
		   if( iHeight <= iHt )
		   {
			   // only some TEXT
			   goto Exit_PH;
		   }
      }

		rTxt = *lpRect;
		rFrm = rTxt;
		rect = rTxt;

		FrameRect( hDC, lpRect, GetStockObject(WHITE_BRUSH) );

//		iHeight = (( iHeight * 9 ) / 10 );
		iHeight    = iHeight - iHt;    // subtract any TEXT height
		rect.top  += iHt;              // and move DOWN by the TEXT height (if any)
      // set first RECT as left + width
		rect.right = rect.left + iWid;
		if( lpRect->bottom )
		{
			/* remove the FRAME done */
			rect.bottom = lpRect->bottom - 1;
		}

		/* use adjusted top and bottom */
		rFrm.top    = rect.top;
		rFrm.bottom = rect.bottom;

		dwMax = 1;
		pt.x = lpRect->left;
		pt.y = lpRect->top;

		// grCli.bottom -= gtm.tmHeight;
		if( dy < (DWORD)gtm.tmHeight )
			dy = gtm.tmHeight;

		if( ( dy + 3 ) < (DWORD)iHeight )
		{
			pt.y = lpRect->top + ( iHeight - ( dy + 3 ) );
		}
		else
		{
			pt.y = lpRect->top + 3;
		}

		/* get the MAX frequency */
		dwMax = GetFREQMax(lpb);   // returns a MINIMUM of 1
		if( ( rect.left + ( iWid * (int)dwCnt ) + 1 ) < iWidth )
		{
			int		id;
			int		iw;
			/* bump first column over to centre graph */
			iw = ( rect.left + ( iWid * (int)dwCnt ) );
//			id = iWidth - ( rect.left + ( iWid * dwCnt ) );
			id = iWidth - iw;
			id = id / 2;
			if( id )
			{
				rc = rect;
				rc.right = rc.left + id;
				if( ( rc.top + 1 ) < rc.bottom )
				{
					rc.top++;
					rc.bottom--;
				}
				FrameRect( hDC, &rc, GetStockObject(WHITE_BRUSH) );
				wsprintf( lpt,
					"%d",
					dwMax );
				i = lstrlen(lpt);
				dx = LOWORD( GetTextExtent( hDC, lpt, i ) );
				if( dx < (DWORD)id  )
				{
					TextOut( hDC,
						( rc.left + ( ( id - dx ) / 2 ) ),
						rc.top,
						lpt,
						i );
				}

				rc = *lpRect;
				rc.top    = rect.top;
				rc.bottom = rect.bottom;
				rc.left = rc.right - id;
				if( ( rc.top + 1 ) < rc.bottom )
				{
					rc.top++;
					rc.bottom--;
				}
				FrameRect( hDC, &rc, GetStockObject(WHITE_BRUSH) );
			}

			/* fix the FRAME of the graph */
			rFrm.left  += id;
			rFrm.right -= id;
			rFrm.right  = rFrm.left + iw;
			/* set RECTANGLE where HISTOGRAM will be drawn */
			rect.left  += id;
			rect.right  = rect.left + iWid;

		}

		if( gfAddCycle )
		{
			// for the defines set
			PaintColorCyc( hDC, lpPalInfo, &rFrm );
		}

      // paint the COLOR COLUMNS for the HISTOGRAM
		for( dwi = 0; dwi < dwCnt; dwi++ )
		{
			// cr  = GetCOLR( lpb, dwi );
         // default is per order in PALEX struct. ie cr = GetCOLR( lpb, dwi );
			cr  = GetNEXTColr( lpPalInfo, lpb, dwi );
			r   = GetRValue(cr);
			g   = GetGValue(cr);
			b   = GetBValue(cr);
			dwf = GetFREQ( lpb, dwi );

			/* calculate the HEIGHT of this colour */
			iHt = ( dwf * iHeight ) / dwMax;
			if( iHt < rect.bottom )
				rect.top = rect.bottom - iHt;
			else
				rect.top = 0;
//			hBrush    = CreateSolidBrush( cr );
			hBrush    = CreateSolidBrush( RGB(r,g,b) );
			hOldBrush = SelectObject( hDC, hBrush );
			if( gfAddPenC )
			{
				hPen = CreatePen( PS_SOLID, 1, RGB(r,g,b) );
				hOldPen = SelectObject( hDC, hPen );
			}
			Rectangle( hDC,
				rect.left,
            rect.top,
            rect.right,
            rect.bottom );
			if( gfAddPenC )
			{
				SelectObject( hDC, hOldPen );
				DeleteObject( hPen );
			}
			SelectObject( hDC, hOldBrush );
			DeleteObject( hBrush );

         if( gfAddCText )
         {
			   wsprintf( lpt,
				   "(%d,%d,%d)",
				   r, g, b );
			   i = lstrlen(lpt);
			   if( dx <= (DWORD)iWid )
			   {
				   int		ix;
				   DWORD	   dxx;
				   dxx = (DWORD)LOWORD(GetTextExtent(hDC,lpt,i));
				   if( dxx > (DWORD)iWid )
					   dxx = (DWORD)iWid;
				   ix = (( iWid - (int)dxx ) / 2);
				   TextOut( hDC,
					   (pt.x + ix),
					   pt.y,
					   lpt,
					   i );
			   }
         }
         else
         {
				FrameRect( hDC, &rect, GetStockObject(WHITE_BRUSH) );
         }

#ifdef   DBGHIST2
			//wsprintf( lpt, "At (%3d,%3d) put (%3d,%3d,%3d) for %3d",
			//	pt.x, pt.y,
			//	r, g, b,
			//	iHt );
			//DO(lpt);
         sprtf( "Color: (%3d,%3d,%3d) to Rect %s. Ht=%d Freq. = %d"MEOR,
            ( r & 0xff ),
            ( g & 0xff ),
            ( b & 0xff ),
            Rect2Stg( &rect ),
            rect.bottom - rect.top,
            dwf );
#endif   // #ifdef   DBGHIST2

			/* move to right painting rectangles in the colour */
			rect.left  += iWid;
			rect.right  = rect.left + iWid;
			pt.x       += iWid;

		}

Exit_PH:

		wsprintf( lpt,
			"Historgram of %d Colors.",
			dwCnt );	// number of Colors in bitmap
		if( i = lstrlen( lpt ) )
		{
			DWORD	dxx;
			int		ix;
			dxx = (DWORD)LOWORD(GetTextExtent(hDC,lpt,i));
			if( dxx > (DWORD)iWidth )
					dxx = (DWORD)iWidth;
			ix = (( iWidth - (int)dxx ) / 2);
			TextOut( hDC,
				( rTxt.left + ix + 1 ),			// pt.x,
				( rTxt.top  + 1 ),	// pt.y,
				lpt,
				i );
			DO(lpt);
		}

		/* done each colour of the histogra */
		*lpt = 0;

	}
	else
	{
		chkit( "HUH! NO paint of histogram *** " );
	}

	*lpt = 0;

}


// eof - DvPal.c
