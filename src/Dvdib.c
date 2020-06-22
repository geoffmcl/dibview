
//-----------------------------------------------------------------------------
// DvDib.C
//
// This is a collection of useful DIB manipulation/information gathering
// functions.  Many functions are supplied simply to take the burden
// of taking into account whether a DIB is a Win30 style or OS/2 style
// DIB away from the application.
//
// The functions in this module assume that the DIB pointers or handles
// passed to them point to a block of memory in one of two formats:
//
//       a) BITMAPINFOHEADER + color table + DIB bits (3.0 style DIB)
//       b) BITMAPCOREHEADER + color table + DIB bits (OS/2 PM style)
//
// The SDK Reference, Volume 2 describes these data structures.
//
// A number of functions in this module were lifted from SHOWDIB,
// and modified to handle OS/2 DIBs.
//
// The functions in this module could be streamlined (made faster and
// smaller) by removing the OS/2 DIB specific code, and assuming all
// DIBs passed to it are Win30 style DIBs.  The DIB file reading code
// would need to be modified to always convert DIBs to Win30 style
// DIBs.  The only reason this isn't done in DIBView is because DIBView
// was written to test display and printer drivers (which are supposed
// to support OS/2 DIBs wherever they support Win30 style DIBs).  SHOWDIB
// is a great example of how to go about doing this.
//-----------------------------------------------------------------------------

#include "dv.h"		// Include general PROJECT include ...
// This includes such things as windows.h, and memory.h, plus
// local of errors.h, palette.h, dib.h, mem.h, ...
#include	<math.h>	// For floor and ceil

extern	LPSTR	GetDT4s( int Typ );
extern	BOOL	PntNeedsBmp( LPDIBINFO lpDIBInfo );

// FROM MSDN Online KB
// There is a two-megabyte limit on the size of the area of
//	a DIB that can be blitted using blting functions under
//	Win32s. In versions of Win32s up to 1.2, Microsoft set
//	this size to accommodate DIB blts of 1024*768*24 
//	bits-per-pixel. In version 1.25, the maximum size of
//	the blitted area will be enlarged to accommodate 
//	1280*1024*24 bits-per-pixel.
// WORKAROUND
// Breakup the DIB into SMALLER pieces!!!
// also another BYTES-PER_LINE macro
//
// Macro to determine the bytes in a DWORD aligned DIB scanline
#define		BYTESPERLINE(Width, BPP)	((WORD)((((DWORD)(Width) *\
	(DWORD)(BPP) + 31) >> 5)) << 2)

//#define		USECTHREAD		// Offload BITMAP rendering to a thread
//#define		USEMETHOD2		// Switch to PAINT Method 2 (DIBs)
//#define		DIAGDIB2
#undef		USECTHREAD		// Offload BITMAP rendering to a thread
#undef		USEMETHOD2		// Switch to PAINT Method 2 (DIBs)
#undef		DIAGDIB2

#ifndef	USEMETHOD2
HBITMAP	GetClipDIB( LPDIBINFO lpDIBInfo, LPBITMAPINFOHEADER lpbmih,
				   HDC hDC );
HANDLE	CreateClipThread( HANDLE hDIBInfo, LPDWORD lpThreadId );
#endif	// USEMETHOD2
//--------------------------------------------------------------
//
// Function: BitmapFileSize( LPSTR lpbmih )
//
// Purpose: Given a pointer to a DIB, calculate the TOTAL
//		BITMAP FILE size, including the BITMAPFILEHEADER.
//
// Params: lpbmih == pointer to DIB header
//
// History: Date		Reason
//		25 April, 1997	Created
//
//--------------------------------------------------------------
DWORD	BitmapFileSize( LPSTR lpbi )
{
	DWORD	ts;
	DWORD	dh, dw, adw;
	DWORD	bc;

	ts = sizeof( BITMAPFILEHEADER ) +	// Init this size
		sizeof( BITMAPINFOHEADER );
	ts += PaletteSize( lpbi );	// Add the COLOR TABLE (if any)

	dh = DIBHeight( lpbi );
	dw = DIBWidth( lpbi );

	// OK, how we calculate the DATA size
	// depends on the Bit count
	adw = dw;	// Set DEFAULT to 8 - Not correct, but what to do???
	switch( bc = DIBBitCount( lpbi) )
	{

	case 1:		// Monochrome
		adw = dw / 8;	// Each bit in each byte is a colour ON/OFF
		if( dw % 8 )
			adw++;
		break;
	case 4:		// 16 Colours
		adw = dw / 2;	// Each nibble (4-bits) is a color index
		if( dw % 2 )
			adw++;
		break;
	case 8:		// 256 Colours
		adw = dw;		// Each BYTE (8-bits) is a color index
		break;
	case 24:	// 24 Bit Colour
		adw = dw * 3;	// Each THREE (3) bytes is a COLOR
		break;
	}

	// Now onto 32-bit BOUNDARY
	if( adw % 4 )
		adw = ((adw / 4) + 1) * 4;

	// Return this FULL BITMAP SIZE
	ts += (adw * dh);

	return( ts );
}


//---------------------------------------------------------------------
//
// Function:   FindDIBBits
//
// Purpose:    Given a pointer to a DIB, returns a pointer to the
//             DIB's bitmap bits.
//
// Parms:      lpbi == pointer to DIB header (either BITMAPINFOHEADER
//                       or BITMAPCOREHEADER)
//
// History:   Date      Reason
//             6/01/91  Created
//             29/09/1999 Added "OffsetToColor();
//
//---------------------------------------------------------------------
DWORD	OffsetToColor( LPSTR lpbi )
{
//	return( *(LPDWORD)lpbi );
	DWORD	   dwi;
	LPDWORD	lpdw;

	lpdw = (LPDWORD)lpbi;
	dwi = *lpdw;
	if(( dwi == sizeof(BITMAPINFOHEADER) ) ||
		( dwi == sizeof(BITMAPCOREHEADER) ) )
	{
		/* this is ok */
	}
	else
	{
		dwi = sizeof(BITMAPINFOHEADER);
	}
	return dwi;
}

DWORD	OffsetToBits( LPSTR lpbi )
{
	return( OffsetToColor(lpbi) + PaletteSize(lpbi) );
}

LPSTR FindDIBBits (LPSTR lpbi)
{
   return( lpbi + OffsetToBits( lpbi ));
}

LPSTR FindDIBColor( LPSTR lpbi )
{
   return( lpbi + OffsetToColor( lpbi ));
}

DWORD	DIBBitCount( LPSTR lpbi )
{
	DWORD dwBC;
	if( IS_WIN30_DIB(lpbi) )
		dwBC = ((LPBITMAPINFOHEADER) lpbi)->biBitCount;
	else
		dwBC = ((LPBITMAPCOREHEADER) lpbi)->bcBitCount;
	return( dwBC );
}

DWORD DIBBPP( LPSTR lpDIB )
{
   DWORD dwBPP;
   if( IS_WIN30_DIB( lpDIB ) )
   {
		//wCompression = (WORD) ((LPBITMAPINFOHEADER) lpDIB)->biCompression;
		dwBPP = ((LPBITMAPINFOHEADER) lpDIB)->biBitCount;
	}
	else
	{
      //wCompression = BI_PM;
      dwBPP = ((LPBITMAPCOREHEADER) lpDIB)->bcBitCount;
	}
   return dwBPP;
}


// Only certain bitmaps have a color count
// That is a COLORREF table
DWORD	GetColorCnt( DWORD wBitCount )
{
	DWORD	wClrCnt = 0;
	switch( wBitCount )
	{

	case 1:
		wClrCnt = 2;
		break;

	case 4:
		wClrCnt = 16;
		break;

	case 8:
		wClrCnt = 256;
		break;

	}
	return wClrCnt;
}


DWORD	CalcDIBColors( LPSTR lpbi )
{
	DWORD	dwClrCnt, dwBitCount;
	dwBitCount = DIBBitCount( lpbi);
	dwClrCnt   = GetColorCnt( dwBitCount );
	return dwClrCnt;
}

//---------------------------------------------------------------------
//
// Function:   DIBNumColors
//
// Purpose:    Given a pointer to a DIB, returns a number of colors in
//             the DIB's color table.
//
// Parms:      lpbi == pointer to DIB header (either BITMAPINFOHEADER
//                       or BITMAPCOREHEADER)
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

DWORD DIBNumColors (LPSTR lpbi)
{
   DWORD wClrCount;


      // If this is a Windows style DIB, the number of colors in the
      //  color table can be less than the number of bits per pixel
      //  allows for (i.e. lpbi->biClrUsed can be set to some value).
      //  If this is the case, return the appropriate value.

//   if (IS_WIN30_DIB (lpbi))
//      {
      DWORD dwClrUsed;

      dwClrUsed = ((LPBITMAPINFOHEADER) lpbi)->biClrUsed;

      if (dwClrUsed)
         return (DWORD) dwClrUsed;
//      }


      // Calculate the number of colors in the color table based on
      //  the number of bits per pixel for the DIB.
	wClrCount = CalcDIBColors( lpbi );
	
	return wClrCount;

}

//---------------------------------------------------------------------
//
// Function:   PaletteSize
//
// Purpose:    Given a pointer to a DIB, returns number of bytes
//             in the DIB's color table.
//
// Parms:      lpbi == pointer to DIB header (either BITMAPINFOHEADER
//                       or BITMAPCOREHEADER)
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

DWORD PaletteSize (LPSTR lpbi)
{
//   if (IS_WIN30_DIB (lpbi))
      return (DIBNumColors (lpbi) * sizeof (RGBQUAD));
//   else
//      return (DIBNumColors (lpbi) * sizeof (RGBTRIPLE));
}

DWORD	GetSizeLP16( void )
{
	return( sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 16) );
}
DWORD	GetSizeLP2( void )
{
	return( sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 2) );
}
DWORD	GetSizeLP256( void )
{
	return( sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 256) );
}

//---------------------------------------------------------------------
//
// Function:   CreateDIBPalette
//
// Purpose:    Given a handle to a DIB, constructs a logical palette,
//             and returns a handle to this palette.
//
//             Stolen almost verbatim from ShowDIB.
//
// Parms:      hDIB == HANDLE to global memory with a DIB header 
//                     (either BITMAPINFOHEADER or BITMAPCOREHEADER)
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------
HPALETTE CreateDIBPalette( HANDLE hDIB )
{
	LPLOGPALETTE	lpPal;
	HANDLE			hLogPal;
	HPALETTE		   hPal;
	DWORD			   i, wNumColors;
	LPSTR			   lpbi;
	LPBITMAPINFO	lpbmi;
	LPBITMAPCOREINFO lpbmc;
	BOOL			   bWinStyleDIB;

	hPal = NULL;
	if( !hDIB )
		return( hPal );

	if( lpbi = DVGlobalLock( hDIB ) )   // LOCK DIB HANDLE
	{
		lpbmi = (LPBITMAPINFO)lpbi;
		lpbmc = (LPBITMAPCOREINFO)lpbi;

		wNumColors = DIBNumColors( lpbi );
		bWinStyleDIB = TRUE;
		//bWinStyleDIB = IS_WIN30_DIB (lpbi);
		if( wNumColors )	// ONLY IF the DIB has a color palette
		{
			// 24-bit TRUECOLOR BITMAP are
			// themselves like a color palette!
         i = 0;
			if( wNumColors == 2 )
			{
				i = GetSizeLP2();
			}
			else if( wNumColors == 16 )
			{
				i = GetSizeLP16();
			}
			else if( wNumColors == 256 )
			{
				i = GetSizeLP256();
			}
			else
			{
				i = sizeof (LOGPALETTE) +
					(sizeof(PALETTEENTRY) * wNumColors);
			}
         if( i )
         {
				//hLogPal = DVGlobalAlloc( GHND, i );
				hLogPal = DVGAlloc( "CreatPAL", GHND, i );
         }
			if( !hLogPal )
			{
				// Oops - Failed to get memory
				DIBError( ERR_MEMORY ); // ERR_CREATEPAL );
				DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE - for error exit
				return( NULL );
			}

			lpPal = (LPLOGPALETTE) DVGlobalLock( hLogPal ); 
			if(lpPal)
			{
				lpPal->palVersion    = PALVERSION;
				lpPal->palNumEntries = (WORD)wNumColors;
				for( i = 0;  i < wNumColors;  i++ )
				{
					if( bWinStyleDIB )
					{
						lpPal->palPalEntry[i].peRed   = lpbmi->bmiColors[i].rgbRed;
						lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
						lpPal->palPalEntry[i].peBlue  = lpbmi->bmiColors[i].rgbBlue;
						lpPal->palPalEntry[i].peFlags = 0;
					}
					else
					{
						lpPal->palPalEntry[i].peRed   = lpbmc->bmciColors[i].rgbtRed;
						lpPal->palPalEntry[i].peGreen = lpbmc->bmciColors[i].rgbtGreen;
						lpPal->palPalEntry[i].peBlue  = lpbmc->bmciColors[i].rgbtBlue;
						lpPal->palPalEntry[i].peFlags = 0;
					}
				}
				hPal = CreatePalette( lpPal );
				if( !hPal )
					DIBError( ERR_CREATEPAL );

				DVGlobalUnlock( hLogPal );
				DVGlobalFree( hLogPal );
			}
			else
			{
				DVGlobalFree( hLogPal );
				DIBError( ERR_LOCK );   // ERR_CREATEPAL );
			}

   		// DVGlobalUnlock( hDIB ); - moved outside below

		}	// IFF we have COLOR PALETTE as part of the BITMAP

      // FIX20001201 - this was inside the number of colors!!!
		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE

	}

	return( hPal );
}


//---------------------------------------------------------------------
//
// Function:   DIBHeight
//
// Purpose:    Given a pointer to a DIB, returns its height.  Note
//             that it returns a DWORD (since a Win30 DIB can have
//             a DWORD in its height field), but under Win30, the
//             high order word isn't used!
//
// Parms:      lpDIB == pointer to DIB header (either BITMAPINFOHEADER
//                       or BITMAPCOREHEADER)
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

DWORD DIBHeight (LPSTR lpDIB)
{
   LPBITMAPINFOHEADER lpbmi;
   LPBITMAPCOREHEADER lpbmc;

   lpbmi = (LPBITMAPINFOHEADER) lpDIB;
   lpbmc = (LPBITMAPCOREHEADER) lpDIB;

   if (lpbmi->biSize == sizeof (BITMAPINFOHEADER))
      return lpbmi->biHeight;
   else
      return (DWORD) lpbmc->bcHeight;
}



//---------------------------------------------------------------------
//
// Function:   DIBWidth
//
// Purpose:    Given a pointer to a DIB, returns its width.  Note
//             that it returns a DWORD (since a Win30 DIB can have
//             a DWORD in its width field), but under Win30, the
//             high order word isn't used!
//
// Parms:      lpDIB == pointer to DIB header (either BITMAPINFOHEADER
//                       or BITMAPCOREHEADER)
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

DWORD DIBWidth (LPSTR lpDIB)
{
   LPBITMAPINFOHEADER lpbmi;
   LPBITMAPCOREHEADER lpbmc;

   lpbmi = (LPBITMAPINFOHEADER) lpDIB;
   lpbmc = (LPBITMAPCOREHEADER) lpDIB;

   if (lpbmi->biSize == sizeof (BITMAPINFOHEADER))
      return lpbmi->biWidth;
   else
      return (DWORD) lpbmc->bcWidth;
}



#ifndef	USEMETHOD2
// Limited to 32-bits at one time
DWORD	NextThreadBit( void )
{
	gdwThreadBit = gdwThreadBit << 1;
	if( gdwThreadBit == 0 )
		gdwThreadBit = 1;
	return gdwThreadBit;
}

#endif	// !USEMETHOD2

//---------------------------------------------------------------------
//
// Function:   DIBToBitmap
//
// Purpose:    Given a handle to global memory with a DIB spec in it,
//             and a palette, returns a device dependent bitmap.  The
//             The DDB will be rendered with the specified palette.
//
// Parms:      hDIB == HANDLE to global memory containing a DIB spec
//                     (either BITMAPINFOHEADER or BITMAPCOREHEADER)
//             hPal == Palette to render the DDB with.  If it's NULL,
//                     use the default palette.
//
// History:   Date      Reason
//             6/01/91  Created
//
// AWK: 1998 April 26 - This function appears to FAIL with LARGE
//		DIB's!!! At first TRIED adding a GetClipDIB() to create
//		a PARTIAL DIB from the BIG DIB, and convert that to a BITMAP.
//
//	This WORKED up to around 2,000,000 BYTE size, and due to the
//	raw TIME taken to produce this PARTIAL BITMAP, moved this code
//	into a separate thread.
//	BUT has the BIG problem of scrolling to see the BALANCE of
//	the DIB!
//
//	HOWEVER, of the THREE display methods offered,
//	1.	DISP_USE_DDBS (IDRB_USEDDBS) - Use a previously built BITMAP
//	2.	DISP_USE_DIBS (IDRB_USEDIBS) - Directly use the DIB to device
//	3.	DISP_USE_SETDIBITS (IDRB_USESETDIBITS) - Dynamically builds
//			a BITMAP (in memory), then proceeds per USEDDBS above.
//	ONLY choice 1. actually USES the BITMAP!
//	Although there is some RAW TIME taken, Method 2. seems to work
//	OK, at least up to the 8MB DIB tried. It has the small problem
//	that it "appears" for a second or 2 that a scroll request has
//	FAILED, but it does then work ok.
//
//	And similarly Method 3. seems to FAIL the same
//	since it attempts to create the SAME very large BITMAP as
//	Method 1.
//
//	Current Decision: Since this is first called during the WM_CREATE
//	of the MDI Child Window to hold the image, why not -
//	(a) Mark it as TOO LARGE, and
//	(b) Force the PAINT to Method 2.
//	Implemented under a USEMETHOD2 switch.
//
// MORE - THe MAIN system function used to create a window BITMAP is
//  Platform SDK: Windows GDI 
// CreateDIBitmap
// The CreateDIBitmap function creates a device-dependent bitmap
//   (DDB) from a DIB and, optionally, sets the bitmap bits. 
// HBITMAP CreateDIBitmap(
//  HDC hdc,                        // handle to DC
//  CONST BITMAPINFOHEADER *lpbmih, // bitmap data
//  DWORD fdwInit,                  // initialization option
//  CONST VOID *lpbInit,            // initialization data
//  CONST BITMAPINFO *lpbmi,        // color-format data
//  UINT fuUsage                    // color-data usage
//  );
//Parameters
//hdc 
//[in] Handle to a device context. 
//lpbmih 
//[in] Pointer to a bitmap information header structure, which may
//be one of those shown in the following table. Operating system
//Bitmap information header 
// Windows NT 3.51 and earlier BITMAPINFOHEADER 
// Windows NT 4.0 and Windows 95 BITMAPV4HEADER 
// Windows 2000 and Windows 98 BITMAPV5HEADER 
//If fdwInit is CBM_INIT, the function uses the bitmap information
//header structure to obtain the desired width and height of the
//bitmap as well as other information. Note that a positive value
//for the height indicates a bottom-up DIB while a negative value
//for the height indicates a top-down DIB. Calling CreateDIBitmap
//with fdwInit as CBM_INIT is equivalent to calling the
//CreateCompatibleBitmap function to create a DDB in the format of
//the device and then calling the SetDIBits function to translate
//the DIB bits to the DDB. 
//fdwInit 
//[in] Specifies how the system initializes the bitmap bits. The
//following values is defined. Value Meaning 
//CBM_INIT If this flag is set, the system uses the data pointed
//to by the lpbInit and lpbmi parameters to initialize the
//bitmap's bits. 
//If this flag is clear, the data pointed to by those parameters
//is not used.
//If fdwInit is zero, the system does not initialize the bitmap's
//bits. 
//lpbInit 
//[in] Pointer to an array of bytes containing the initial bitmap
//data. The format of the data depends on the biBitCount member of
//the BITMAPINFO structure to which the lpbmi parameter points. 
//lpbmi 
//[in] Pointer to a BITMAPINFO structure that describes the
//dimensions and color format of the array pointed to by the
//lpbInit parameter. 
//fuUsage 
//[in] Specifies whether the bmiColors member of the BITMAPINFO
//structure was initialized and, if so, whether bmiColors contains
//explicit red, green, blue (RGB) values or palette indexes. The
//fuUsage parameter must be one of the following values. Value
//Meaning 
//DIB_PAL_COLORS A color table is provided and consists of an
//array of 16-bit indexes into the logical palette of the device
//context into which the bitmap is to be selected. 
//DIB_RGB_COLORS A color table is provided and contains literal
//RGB values. 
//Return Values
//If the function succeeds, the return value is a handle to the
//bitmap.
//If the function fails, the return value is NULL. 
//Windows NT/ 2000: To get extended error information, call
//GetLastError.
//Remarks
//The DDB that is created will be whatever bit depth your
//reference DC is. To create a bitmap that is of different bit
//depth, use CreateDIBSection.
//For a device to reach optimal bitmap-drawing speed, specify
//fdwInit as CBM_INIT. Then, use the same color depth DIB as the
//video mode. When the video is running 4- or 8-bpp, use DIB_PAL_COLORS.
//The CBM_CREATDIB flag for the fdwInit parameter is no longer
//supported. 
//When you no longer need the bitmap, call the DeleteObject
//function to delete it. 
//ICM: No color management is performed. The contents of the
//resulting bitmap are not color matched after the bitmap has been
//created. 
//Windows 95/98: The created bitmap cannot exceed 16MB in size.
//Requirements 
//  Windows NT/2000: Requires Windows NT 3.1 or later.
//  Windows 95/98: Requires Windows 95 or later.
//  Header: Declared in Wingdi.h; include Windows.h.
//  Library: Use Gdi32.lib.
//See Also
//Bitmaps Overview, Bitmap Functions, BITMAPINFOHEADER,
//BITMAPINFO, CreateCompatibleBitmap, CreateDIBSection,
//DeleteObject, GetDeviceCaps, GetSystemPaletteEntries,
//SelectObject, SetDIBits 
//Built on Wednesday, July 12, 2000Requirements 
//  Windows NT/2000: Requires Windows NT 3.1 or later.
//  Windows 95/98: Requires Windows 95 or later.
//  Header: Declared in Wingdi.h; include Windows.h.
//  Library: Use Gdi32.lib.
//See Also
//Bitmaps Overview, Bitmap Functions, BITMAPINFOHEADER,
//BITMAPINFO, CreateCompatibleBitmap, CreateDIBSection,
//DeleteObject, GetDeviceCaps, GetSystemPaletteEntries,
//SelectObject, SetDIBits
//
//---------------------------------------------------------------------
HBITMAP DIBToBitmap( HANDLE hDIB, HPALETTE hPal, LPVOID lpv )
{
	LPSTR		   lpDIBHdr, lpDIBBits;
	HBITMAP		hBitmap;
	HDC			hDC;
	HPALETTE	   hOldPal = NULL;
	DWORD		   dwErr;
	LPBITMAPINFOHEADER	lpbmih;
	LPDIBINFO	lpDIBInfo;
	BOOL		   bNeedsBmp;
	DWORD		   dwsz, dwnsz;
	LPSTR		   lpd;
#ifdef	DIAGDIB2
	LPSTR		   lpt;
#endif	// DIAGDIB2

#ifdef	DIAGDIB2
	lpt = GetDT4s(0);
#endif	// DIAGDIB2

	lpd = GetTmp1();
	bNeedsBmp = TRUE;
	if( lpDIBInfo = lpv )
	{
		lpDIBInfo->di_hThread = 0;
		bNeedsBmp = PntNeedsBmp( lpDIBInfo );
	}

	if( !hDIB )
	{
#ifdef	DIAGDIB2
		lstrcpy( lpd, "DIB2Bitmap: FAILED due to no DIB handle!"MEOR );
		DO(lpd);
#endif	// DIAGDIB2
		return NULL;
	}

	lpDIBHdr  = DVGlobalLock( hDIB );   // LOCK DIB HANDLE
   if( !lpDIBHdr )
   {
#ifdef	DIAGDIB2
		lstrcpy( lpd, "DIB2Bitmap: FAILED to LOCK DIB memory!"MEOR );
		DO(lpd);
#endif	// DIAGDIB2
		return NULL;
   }

	lpDIBBits = FindDIBBits( lpDIBHdr );   // get the OFFSET to the DIB bits
	hDC       = GetDC( NULL );
	if( !hDC )
	{
		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE - for error exit
#ifdef	DIAGDIB2
		lstrcpy( lpd, "DIB2Bitmap: FAILED due to no HDC!"MEOR );
		DO(lpd);
#endif	// DIAGDIB2
		return NULL;
	}

	if( hPal )
   {
      hOldPal = SelectPalette (hDC, hPal, FALSE);
      RealizePalette( hDC );
   }

   // cast the read-in DIB as a BITMAPINFOHEADER
	lpbmih = (LPBITMAPINFOHEADER)lpDIBHdr;

   // set in DIBToBitmap() functions from DIB values
   if( ( lpDIBInfo ) &&
      ( ( (DWORD)lpbmih->biHeight != lpDIBInfo->di_dwDIBHeight ) || // Height of the DIB
         ( (DWORD)lpbmih->biWidth  != lpDIBInfo->di_dwDIBWidth ) ) ) // Width of the DIB
   {
      lpDIBInfo->di_bChgdDIB    = TRUE;   // if the SIZE changed (for animations)
      lpDIBInfo->di_dwNEWWidth  = lpbmih->biWidth;	   // NEW Width  of the DIB
      lpDIBInfo->di_dwNEWHeight = lpbmih->biHeight;	// NEW Height of the DIB
      sprtf( "DIB Size changed: From %d x %d to %d x %d."MEOR,
         lpDIBInfo->di_dwDIBWidth,
         lpDIBInfo->di_dwDIBHeight,
         lpDIBInfo->di_dwNEWWidth,
         lpDIBInfo->di_dwNEWHeight );
   }

	// BITMAPINFOHEADER
	dwsz = lpbmih->biHeight *
      BYTESPERLINE( ( lpbmih->biWidth ), ( lpbmih->biBitCount ) );

   // SYSTEM FUNCTION THAT SOMETIMES GIVES TROUBLE
   // ********************************************
   //HBITMAP CreateDIBitmap(
   //  HDC hdc,                        // handle to DC
   //  CONST BITMAPINFOHEADER *lpbmih, // bitmap data
   //  DWORD fdwInit,                  // initialization option
   //  CONST VOID *lpbInit,            // initialization data
   //  CONST BITMAPINFO *lpbmi,        // color-format data
   //  UINT fuUsage );                 // color-data usage
   // NOTE: Is equivalent to CreateCompatibleBitmap(), then SetDIBits()!!!
	hBitmap = CreateDIBitmap( hDC,   // handle to DC
		lpbmih,     // bitmap data - in our case the BMP file data
		CBM_INIT,   // we want the bitmap data initialized
		lpDIBBits,  // pointer to the DIB bits
		(LPBITMAPINFO) lpDIBHdr,   // same as bitmap data in our case
		DIB_RGB_COLORS );    // and the DIB uses RGB colour (NOT palette indexes)

	// *** FAILED ***
	// ==============
	if( ( !hBitmap  ) &&
		 ( bNeedsBmp ) )
	{
		HWND		hWnd;
		RECT		rc;

		dwErr = GetLastError();
		dwnsz = dwsz;
		hWnd = 0;
		rc.left = 0;
      // *** output an error ***
		wsprintf( lpd,
				"File: [%s]. Error: %d (%#x)"MEOR
				"DIB2Bitmap: CreateDIBitmap FAILED! Bitsize = %u Bytes. (%dx%d)"MEOR,
            ( lpDIBInfo->di_sRD.rd_pPath ? lpDIBInfo->di_sRD.rd_pPath : "<noname>" ),
				dwErr, dwErr,
				dwsz,
				lpbmih->biWidth,
				lpbmih->biHeight );
		DO(lpd);
      // ***********************
      // seems that this function works better
      // ********************************************
		lpDIBInfo->Options.wDispOption = DISP_USE_DIBS;
		bNeedsBmp = FALSE;
      // ********************************************
      // ===========================================

		if( ( dwsz > 2000000 ) &&
			 ( lpDIBInfo      ) )
		{
#ifdef	DIAGDIB2
			sprintf( lpd,
				"Began at %s."MEOR
				"DIB2Bitmap: CreateDIBitmap FAILED! Bitsize = %u Bytes. (%dx%d)"MEOR,
				lpt,
				dwsz,
				lpbmih->biWidth,
				lpbmih->biHeight );
			DO(lpd);
#endif	// DIAGDIB2
			// ITS TOO LARGE - maybe!
			lpDIBInfo->di_bTooBig = TRUE;	// MARK as TOO BIG
#ifdef	USEMETHOD2
			lpDIBInfo->Options.wDispOption = DISP_USE_DIBS;
			bNeedsBmp = FALSE;
#else	// !USEMETHOD2
			// Ok, how to SPLIT it up
			if( ( hWnd = lpDIBInfo->di_hwnd ) &&
				( GetClientRect( hWnd, &rc ) ) )
			{
				dwsz = 0;
				if( ( dwnsz = ( rc.bottom * BYTESPERLINE( (rc.right),
					(lpbmih->biBitCount ) ) ) ) < 2000000 )
				{
#ifdef	USECTHREAD
               // NOT PRESENTLY USED!!!
               // *********************
					HANDLE		hNDI;
					LPDIBINFO	lpNDI;

					hNDI = 0;
					lpNDI = 0;
					//if( hNDI = DVGlobalAlloc( GHND, sizeof(DIBINFO) ) )
					if( hNDI = DVGAlloc( "NEWPDI", GHND, sizeof(DIBINFO) ) )
					{
						if( lpNDI = (LPDIBINFO)DVGlobalLock( hNDI ) )
						{
							lpDIBInfo->di_dwThreadBit =
								NextThreadBit();
							memcpy( lpNDI, lpDIBInfo, sizeof(DIBINFO) );
							DVGlobalUnlock( hNDI );
#ifdef	DIAGDIB2
							DO( "Creating THREAD to build BITMAP."MEOR );
#endif	// DIAGDIB2
							if( lpDIBInfo->di_hThread =
								CreateClipThread( hNDI, &lpDIBInfo->di_dwThreadId ) )
							{
								int		iprev, icurr;
								iprev = GetThreadPriority( lpDIBInfo->di_hThread );
								// Try REDUCING priority to KEEP
								// application code ACTIVE
								SetThreadPriority( lpDIBInfo->di_hThread,
									THREAD_PRIORITY_BELOW_NORMAL);
								icurr = GetThreadPriority( lpDIBInfo->di_hThread );
#ifdef	DIAGDIB2
								wsprintf( lpd,
									"THREAD H=0x%x ID=0x%x running.(P %d-%d)."MEOR,
									lpDIBInfo->di_hThread,
									lpDIBInfo->di_dwThreadId,
									iprev, icurr );
								DO(lpd);
#endif	// DIAGDIB2
							}
							else
							{
								lpDIBInfo->di_hThread = 0;
								lpDIBInfo->di_dwThreadId = 0;
								lpDIBInfo->di_dwThreadBit = 0;
								DVGlobalFree( hNDI );
								hNDI = 0;
#ifdef	DIAGDIB2
								DO( "Creating THREAD FAILED!"MEOR );
#endif	// DIAGDIB2
							}
						}
						else
						{
							DVGlobalFree( hNDI );
							hNDI = 0;
#ifdef	DIAGDIB2
							DO( "Memory to pointer FAILED!"MEOR );
#endif	// DIAGDIB2
						}
					}
					else
					{
#ifdef	DIAGDIB2
						DO( "Get memory FAILED!"MEOR );
#endif	// DIAGDIB2
					}
#else	// !USECTHREAD
					int		cx, cy, cx1, cy1;
					HANDLE	hnDIB;
					HBITMAP	hBmp2;
					BOOL	fLast;

#ifdef	DIAGDIB2
					wsprintf( lpd,
						"Retry: CreateDIBitmap with Bitsize = %u Bytes. (%dx%d)"MEOR,
						dwnsz,
						rc.right,
						rc.bottom );
					DO(lpd);
#endif	// DIAGDIB2
					dwsz = 0;
					lpDIBInfo->di_rcDib2 = rc;
					hBitmap = GetClipDIB( lpDIBInfo, lpbmih, hDC );
					if( ( hBitmap ) &&
						( hnDIB = lpDIBInfo->di_hDIB2 ) )
					{
						// Get the SCROLL RANGE
						cy = lpbmih->biHeight - rc.bottom - 1 +
							GetSystemMetrics( SM_CYHSCROLL );
						cx = lpbmih->biWidth - rc.right - 1 +
							GetSystemMetrics( SM_CXVSCROLL );
						cx1 = rc.right / SCROLL_RATIO;
						if( !cx1 )
							cx1 = 1;
						cy1 = rc.bottom / SCROLL_RATIO;
						if( !cy1 )
							cy1 = 1;
						// Set ONE UNIT (pixel) of scroll
						// for THIS window SIZE
						lpDIBInfo->di_iOnex = cx1;
						lpDIBInfo->di_iOney = cy1;
						lpDIBInfo->di_rcDib2.right += cx1;
						lpDIBInfo->di_rcDib2.bottom += cy1;
						fLast = FALSE;
#ifdef	DIAGDIB2
						dwnsz = ( lpDIBInfo->di_rcDib2.bottom *
							BYTESPERLINE( (lpDIBInfo->di_rcDib2.right),
								(lpbmih->biBitCount ) ) );
						wsprintf( lpd,
							"OK, Retry: CreateDIBitmap with Bitsize = %u Bytes. (%dx%d)"MEOR,
							dwnsz,
							lpDIBInfo->di_rcDib2.right,
							lpDIBInfo->di_rcDib2.bottom );
						DO(lpd);
#endif	// DIAGDIB2
						while( ( hBmp2 = GetClipDIB( lpDIBInfo, lpbmih, hDC ) ) != 0 )
						{
							DeleteObject( hBitmap );
							DVGlobalFree( hnDIB );
							hBitmap = hBmp2;
							hnDIB = lpDIBInfo->di_hDIB2;
							rc.right += cx1;
							rc.bottom += cy1;
							if( ( ( lpDIBInfo->di_rcDib2.right + cx1 ) < lpbmih->biWidth ) &&
								( ( lpDIBInfo->di_rcDib2.bottom + cy1 ) < lpbmih->biHeight ) )
							{
								lpDIBInfo->di_rcDib2.right += cx1;
								lpDIBInfo->di_rcDib2.bottom += cy1;
							}
							else
							{
								if( fLast )
								{
									lpDIBInfo->di_bTooBig = FALSE;
									break;
								}
								else
								{
									lpDIBInfo->di_rcDib2.right = lpbmih->biWidth;
									lpDIBInfo->di_rcDib2.bottom = lpbmih->biHeight;
									fLast = TRUE;
								}
							}
#ifdef	DIAGDIB2
							dwnsz = ( lpDIBInfo->di_rcDib2.bottom *
								BYTESPERLINE( (lpDIBInfo->di_rcDib2.right),
									(lpbmih->biBitCount ) ) );
							wsprintf( lpd,
								"OK, Retry: CreateDIBitmap with Bitsize = %u Bytes. (%dx%d)"MEOR,
								dwnsz,
								lpDIBInfo->di_rcDib2.right,
								lpDIBInfo->di_rcDib2.bottom );
							DO(lpd);
#endif	// DIAGDIB2
						}
						lpDIBInfo->di_rcDib2 = rc;
#ifdef	DIAGDIB2
						dwnsz = ( lpDIBInfo->di_rcDib2.bottom *
							BYTESPERLINE( (lpDIBInfo->di_rcDib2.right),
								(lpbmih->biBitCount ) ) );
						wsprintf( lpd,
							"OK, Ended: CreateDIBitmap with Bitsize = %u Bytes. (%dx%d)"MEOR
							"Ended at %s."MEOR,
							dwnsz,
							lpDIBInfo->di_rcDib2.right,
							lpDIBInfo->di_rcDib2.bottom,
							GetDT4s(0) );
						DO(lpd);
#endif	// DIAGDIB2
					}
					else
					{
#ifdef	DIAGDIB2
						lstrcpy( lpd, "DIB2Bitmap: New size also FAILED!"MEOR );
						DO(lpd);
#endif	// DIAGDIB2
					}
#endif	// USECTHREAD y/n

				}
			}
#endif	// USEMETHOD2 y/n
		}
		else if( lpDIBInfo )
		{
         // =====================================================
			// DOUBLE AWK!! FAILED but does NOT appear TOO BIG/LARGE
			// We'll deal with this IF it ever happens.
#ifdef	DIAGDIB2
			wsprintf( lpd,
				"DIB2Bitmap: CreateDIBitmap FAILED!"
				"Bitsize = %u Bytes. (%dx%d)(2)."MEOR,
				dwsz,
				lpbmih->biWidth,
				lpbmih->biHeight );
			DO(lpd);
#endif	// DIAGDIB2
		}
	}
	else if( !hBitmap )
	{
		// OK, we do NOT need the BITMAP for this PAINT method
		// so just output an advice.
#ifdef	DIAGDIB2
		wsprintf( lpd,
			"DIB2Bitmap: CreateDIBitmap FAILED! Bitsize = %u Bytes. (%dx%d)"MEOR,
			dwsz,
			lpbmih->biWidth,
			lpbmih->biHeight );
		DO(lpd);
#endif	// DIAGDIB2
	}

	if( hOldPal )
		SelectPalette( hDC, hOldPal, FALSE );

	ReleaseDC( NULL, hDC );
	DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE

#ifdef	USECTHREAD
	if(( !hBitmap ) &&
		( !lpDIBInfo->di_hThread ) &&
		( bNeedsBmp ) )
	{
		if( dwErr )
			SysDIBError( ERR_CREATEDDB, dwErr );
		else
			DIBError( ERR_CREATEDDB );
	}
#else	// !USECTHREAD
	if( !hBitmap )
	{
		if( dwErr )
			SysDIBError( ERR_CREATEDDB, dwErr );
		else
			DIBError( ERR_CREATEDDB );
	}
#endif	// USECTHREAD y/n

	return hBitmap;

}	// End - HBITMAP DIBToBitmap (HANDLE hDIB, HPALETTE hPal, LPVOID lpDIBInfo )


//---------------------------------------------------------------------
//
// Function:   InitBitmapInfoHeader
//
// Purpose:    Does a "standard" initialization of a BITMAPINFOHEADER,
//             given the Width, Height, and Bits per Pixel for the
//             DIB.
//
//             By standard, I mean that all the relevant fields are set
//             to the specified values.  biSizeImage is computed, the
//             biCompression field is set to "no compression," and all
//             other fields are 0.
//
//             Note that DIBs only allow BitsPixel values of 1, 4, 8, or
//             24.  This routine makes sure that one of these values is
//             used (whichever is most appropriate for the specified
//             nBPP).
//
// Parms:      lpBmInfoHdr == Pointer to a BITMAPINFOHEADER structure
//                            to be filled in.
//             dwWidth     == Width of DIB (not in Win 3.0 & 3.1, high
//                            word MUST be 0).
//             dwHeight    == Height of DIB (not in Win 3.0 & 3.1, high
//                            word MUST be 0).
//             nBPP        == Bits per Pixel for the DIB.
//
// History:   Date      Reason
//            11/07/91  Created
//             
//---------------------------------------------------------------------

void InitBitmapInfoHeader( LPBITMAPINFOHEADER lpBmInfoHdr,
						  DWORD dwWidth,
						  DWORD dwHeight,
						  int nBPP )
{
	int		iBPP;
	dv_fmemset( lpBmInfoHdr, 0, sizeof (BITMAPINFOHEADER) );

	lpBmInfoHdr->biSize      = sizeof(BITMAPINFOHEADER);
	lpBmInfoHdr->biWidth     = dwWidth;
	lpBmInfoHdr->biHeight    = dwHeight;
	lpBmInfoHdr->biPlanes    = 1;

	// Fix Bits per Pixel
	if( nBPP <= 1 )
		iBPP = 1;
	else if( nBPP <= 4 )
		iBPP = 4;
	else if( nBPP <= 8 )
		iBPP = 8;
	else
		iBPP = 24;

	lpBmInfoHdr->biBitCount  = iBPP;
	lpBmInfoHdr->biSizeImage = WIDTHBYTES(dwWidth * iBPP) * dwHeight;
}

//---------------------------------------------------------------------
//
// Function:   DVBitmapToDIB
//
// Purpose:    Given a device dependent bitmap and a palette, returns
//             a handle to global memory with a DIB spec in it.  The
//             DIB is rendered using the colors of the palette passed in.
//
//             Stolen almost verbatim from ShowDIB.
//
// Parms:      hBitmap == Handle to device dependent bitmap compatible
//                        with default screen display device.
//             hPal    == Palette to render the DDB with.
//		NOTE:  If it's NULL,
//                        use the default palette.
//
// History:   Date      Reason
//             6/01/91  Created
//
// NOTE: On say 24-bit devices, the HPALETTE hPal can be NULL!
//
//---------------------------------------------------------------------

HANDLE DVBitmapToDIB( HBITMAP hBitmap, HPALETTE hPal )
{
	BITMAP				Bitmap;
	BITMAPINFOHEADER	bmInfoHdr;
	LPBITMAPINFOHEADER	lpbmInfoHdr;
	LPSTR				lpBits;
	HDC					hMemDC;
	HANDLE				hDIB;
	HPALETTE			hOldPal;
   DWORD          dwSize;

	// Do some setup -- make sure the Bitmap passed in is valid,
	//  get info on the bitmap (like its height, width, etc.),
	//  then setup a BITMAPINFOHEADER.
	hOldPal = NULL;
	hDIB    = NULL;

	if( !hBitmap )
		return hDIB;

	if( !GetObject( hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap ) )
		return hDIB;

	InitBitmapInfoHeader( &bmInfoHdr,
		Bitmap.bmWidth,
		Bitmap.bmHeight,
		Bitmap.bmPlanes * Bitmap.bmBitsPixel );

	// Now allocate memory for the DIB.
	// Then, set the BITMAPINFOHEADER
	//  into this memory, and
	// find out where the bitmap bits go.
	dwSize = ( sizeof(BITMAPINFOHEADER) +
               PaletteSize((LPSTR) &bmInfoHdr) + bmInfoHdr.biSizeImage );
	//hDIB = DVGlobalAlloc( GHND, dwSize );
	hDIB = DVGAlloc( "BMP2DIB", GHND, dwSize );
	if( !hDIB )
		return hDIB;

	lpbmInfoHdr  = (LPBITMAPINFOHEADER) DVGlobalLock( hDIB );   // LOCK DIB HANDLE
	if( !lpbmInfoHdr )
	{
		DVGlobalFree( hDIB );
		hDIB = NULL;
		return hDIB;
	}

	*lpbmInfoHdr = bmInfoHdr;
	lpBits       = FindDIBBits( (LPSTR)lpbmInfoHdr );

	// Now, we need a DC to hold our bitmap.
	// If the app passed us
	//  a palette, it should be selected into the DC.
	hMemDC = GetDC( NULL );
	if( !hMemDC )
	{
		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE - for error free and return NULL
		DVGlobalFree( hDIB );
		hDIB = NULL;
		return hDIB;
	}

	if( hPal )
	{
		hOldPal = SelectPalette( hMemDC, hPal, FALSE );
		RealizePalette( hMemDC );
	}

	// We're finally ready to get the DIB.
	// Call the driver and let it party on our bitmap.
	// It will fill in the color table,
	//  and bitmap bits of our global memory block.
	if( !GetDIBits( hMemDC,
            		hBitmap,
		            0,
		            Bitmap.bmHeight,
		            lpBits,
		            (LPBITMAPINFO)lpbmInfoHdr,
		            DIB_RGB_COLORS ) )
	{
		DVGlobalUnlock( hDIB );    // UNLOCK DIB HANDLE - for error FREE and NULL
		DVGlobalFree( hDIB );
		hDIB = NULL;
	}
	else
	{
		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE
	}

	// Finally, clean up and return.
	if( hOldPal )
		SelectPalette( hMemDC, hOldPal, FALSE );

	ReleaseDC( NULL, hMemDC );

	return hDIB;

}

// ================================================================
// HANDLE BitmapToDIB2( HBITMAP hBitmap, HPALETTE hPal, int iBPP )
//
// Purpose: Take a HBITMAP handle, and use GetDIBits() to CHANGE
//		the current BITMAP to ANOTHER BPP by building a
//		BITMAPINFOHEADER with the REQUESTED size and BPP
//		and pass to GetDIBits()
//
// What to do when there is NO BITMAP as in the BIG DIB case?
// Well, what really is the DIFERENCE between -
// (a) a BITMAP as represented by its handle, and
// (b) a BITMAPINFOHEADER as represented by hDIB (the BIG DIB)?
//
// a BITMAP is -
// typedef struct tagBITMAP {  // bm
//    LONG   bmType;
//    LONG   bmWidth;
//    LONG   bmHeight;
//    LONG   bmWidthBytes;
//    WORD   bmPlanes;
//    WORD   bmBitsPixel;
//    LPVOID bmBits;
//} BITMAP;
// while a BITMAPINFOHEADER is :-
//typedef struct tagBITMAPINFOHEADER{ // bmih
//    DWORD  biSize;
//    LONG   biWidth;
//    LONG   biHeight;
//    WORD   biPlanes;
//    WORD   biBitCount;
//    DWORD  biCompression;
//    DWORD  biSizeImage;
//    LONG   biXPelsPerMeter;
//    LONG   biYPelsPerMeter;
//    DWORD  biClrUsed;
//    DWORD  biClrImportant;
//} BITMAPINFOHEADER;
// Can I manually CONSTRUCT a HBITMAP, and pass that?
// Or maybe do the WHOLE THING manually in say DIBToDIB2( ... )
//
HANDLE	CopyDIB( HANDLE hDIB )
{
	HANDLE	hDIB2 = 0;  // NONE YET
	LPSTR	lp1, lp2;
	DWORD	dwLen;

	if( ( hDIB                       ) &&
		 ( dwLen = GlobalSize( hDIB ) ) )
	{
		if( lp1 = DVGlobalLock( hDIB ) ) // LOCK DIB HANDLE
		{
			//if( hDIB2 = DVGlobalAlloc( GHND, dwLen ) )   // allocate copy
			if( hDIB2 = DVGAlloc( "CopyDIB", GHND, dwLen ) )   // allocate copy
			{
				if( lp2 = DVGlobalLock( hDIB2 ) )   // lock cop
				{
					memcpy( lp2, lp1, dwLen );    // copy and
					DVGlobalUnlock( hDIB2 );   // unlock copy for return
				}
				else
				{
					DVGlobalFree( hDIB2 );  // free copy, and
					hDIB2 = 0;              // kill handle
				}
			}

			DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE
		}
	}

	return hDIB2;
}

BOOL	IsValidBPP( int iBPP )
{
	BOOL	bRet = FALSE;
	if( ( iBPP == 1 ) ||
		( iBPP == 4 ) ||
		( iBPP == 8 ) ||
		( iBPP == 24 ) ||
		( iBPP == 32 ) )
		bRet = TRUE;
	return bRet;
}

HANDLE	DIBToDIB2( HANDLE hDIB, int iBPP )
{
	HANDLE	hDIB2 = 0;
	DWORD	dwOff, dwLen;
	LPBITMAPINFOHEADER	lpDIB, lpDIB2;
	HDC		hMemDC;

	hMemDC = 0;
   lpDIB  = 0;
	if( ( hDIB ) && ( IsValidBPP( iBPP )                  ) &&
		( lpDIB = (LPBITMAPINFOHEADER)DVGlobalLock( hDIB ) ) &&  // LOCK DIB HANDLE
		( hMemDC = GetDC( NULL )                           ) )
	{
		if( ( lpDIB->biSize == sizeof(BITMAPINFOHEADER) ) &&
			 ( lpDIB->biWidth                            ) &&
			 ( lpDIB->biHeight                           ) )
		{
			if( lpDIB->biBitCount == iBPP )
			{
				DVGlobalUnlock(hDIB);      // UNLOCK DIB HANDLE - for early exit with COPY
				ReleaseDC( NULL, hMemDC );
				return( CopyDIB( hDIB ) );	// Just COPY
			}
			else
			{
				// ok, different BPPs
				dwOff = sizeof(BITMAPINFOHEADER) +
					(sizeof(RGBQUAD) * GetColorCnt((DWORD)iBPP));
				dwLen = ( lpDIB->biHeight * BYTESPERLINE( lpDIB->biWidth, iBPP ) );
				//if( hDIB2 = DVGlobalAlloc( GHND, (dwLen+dwOff) ) )
				if( hDIB2 = DVGAlloc( "DIB2DIB2", GHND, (dwLen+dwOff) ) )
				{
					if( lpDIB2 = (LPBITMAPINFOHEADER)DVGlobalLock( hDIB2 ) )
					{
						lpDIB2->biSize      = sizeof(BITMAPINFOHEADER);
						lpDIB2->biWidth	  = lpDIB->biWidth;
						lpDIB2->biHeight    = lpDIB->biHeight;
						lpDIB2->biPlanes    = 1;
						lpDIB2->biBitCount  = (WORD)iBPP;
						lpDIB2->biCompression = BI_RGB;
						lpDIB2->biSizeImage = dwLen;
						lpDIB2->biXPelsPerMeter = 0;
						lpDIB2->biYPelsPerMeter = 0;
						lpDIB2->biClrUsed   = 0;
						lpDIB2->biClrImportant = 0;
						// ok, only make/synthesis palette, and
						// move the BITS over.
						// Hmmm, how to efficently handle the move
						// from one BPP to another?
						// *** TBD ***
						DVGlobalFree(hDIB2);
						hDIB2 = 0;
					}
					else
					{
						DVGlobalFree(hDIB2);
						hDIB2 = 0;
					}
				}
			}
		}

		// DVGlobalUnlock( hDIB ); // moved outside this if

	}

   if( lpDIB )
      DVGlobalUnlock(hDIB); // UNLOCK DIB HANDLE - FIX20001201 - move outside if

	if( hMemDC )
		ReleaseDC( NULL, hMemDC );

	return hDIB2;

}
// ================================================================
HANDLE BitmapToDIB2( HBITMAP hBitmap, HPALETTE hPal,
					int	iBPP )
{
	BITMAP				Bitmap;
	BITMAPINFOHEADER	bmInfoHdr;
	LPBITMAPINFOHEADER	lpbmInfoHdr;
	LPSTR				   lpBits;
	HDC					hMemDC;
	HANDLE				hDIB;
	HPALETTE			   hOldPal;
   DWORD             dwSize;

	// Do some setup -- make sure the Bitmap passed in is valid,
	//  get info on the bitmap (like its height, width, etc.),
	//  then setup a BITMAPINFOHEADER.
	hOldPal = NULL;
	hDIB    = NULL;

	if( !hBitmap )
		return hDIB;

	if( !GetObject( hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap ) )
		return hDIB;

	InitBitmapInfoHeader( &bmInfoHdr,
		Bitmap.bmWidth,
		Bitmap.bmHeight,
		iBPP );

	// Now allocate memory for the DIB.
	// Then, set the BITMAPINFOHEADER
	//  into this memory, and
	// find out where the bitmap bits go.
	dwSize = ( sizeof(BITMAPINFOHEADER) +
		         PaletteSize((LPSTR) &bmInfoHdr) + bmInfoHdr.biSizeImage );
	//hDIB = DVGlobalAlloc( GHND, dwSize );
	hDIB = DVGAlloc( "BMP2DIB2", GHND, dwSize );

	if( !hDIB )
		return hDIB;

	lpbmInfoHdr  = (LPBITMAPINFOHEADER) DVGlobalLock( hDIB );   // LOCK DIB HANDLE
	if( !lpbmInfoHdr )
	{
		DVGlobalFree( hDIB );
		hDIB = NULL;
		return hDIB;
	}

	*lpbmInfoHdr = bmInfoHdr;
	lpBits       = FindDIBBits( (LPSTR)lpbmInfoHdr );

	// Now, we need a DC to hold our bitmap.
	// If the app passed us
	//  a palette, it should be selected into the DC.
	hMemDC = GetDC( NULL );
	if( !hMemDC )
	{
		DVGlobalUnlock( hDIB );    // UNLOCK DIB HANDLE - for FREE and error exit
		DVGlobalFree( hDIB );
		hDIB = NULL;
		return hDIB;
	}

	if( hPal )
	{
		hOldPal = SelectPalette( hMemDC, hPal, FALSE );
		RealizePalette( hMemDC );
	}

	// We're finally ready to get the DIB.
	// Call the driver and let it party on our bitmap.
	// It will fill in the amended color table,
	//  and bitmap bits of our global memory block.
	if( !GetDIBits( hMemDC,
		hBitmap,
		0,
		Bitmap.bmHeight,
		lpBits,
		(LPBITMAPINFO)lpbmInfoHdr,
		DIB_RGB_COLORS ) )
	{
		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE - for FREE and error exit
		DVGlobalFree( hDIB );
		hDIB = NULL;
	}
	else
	{
		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE - for succesful return
	}

	// Finally, clean up and return.
	if( hOldPal )
		SelectPalette( hMemDC, hOldPal, FALSE );

	ReleaseDC( NULL, hMemDC );

	return hDIB;
}

BOOL	CopyDIBPalette( LPSTR lpdest, LPSTR lpsrc )
{
	BOOL		flg = FALSE;
	DWORD		wd, wClrCnt;
	LPBITMAPINFOHEADER lpnew, lpold;
	if( ( lpnew = (LPBITMAPINFOHEADER)lpdest ) &&
		( lpold = (LPBITMAPINFOHEADER)lpsrc ) &&
		( (wClrCnt = CalcDIBColors( lpdest )) == CalcDIBColors( lpsrc ) ) )
	{
		if( wClrCnt == 0 )
			return TRUE;

		if( ( lpnew->biSize == sizeof(BITMAPINFOHEADER) ) &&
			( lpold->biSize == lpnew->biSize ) &&
			( lpnew->biBitCount == lpold->biBitCount ) )
		{
			LPRGBQUAD	lpqo, lpqn;
			lpqo = (LPRGBQUAD)((LPBITMAPINFOHEADER)lpold + 1);
			lpqn = (LPRGBQUAD)((LPBITMAPINFOHEADER)lpnew + 1);
			for( wd = 0; wd < wClrCnt; wd++ )
				lpqn[wd] = lpqo[wd];
			flg = TRUE;
		}
	}
	return flg;
}

#ifndef	USEMETHOD2

BOOL	ClipDIBBits( LPSTR lpnew, LPSTR lpold, LPRECT lprc )
{
	BOOL	flg = FALSE;
	LPBITMAPINFOHEADER lpbmih;
	int		x, y, yyo, yyn, dxo, dxn;
	POINT	pt;
	int		oldoff, newoff;

	if( ( lpnew ) &&
		( lpbmih = (LPBITMAPINFOHEADER)lpold ) &&
		( lpbmih->biSize == sizeof(BITMAPINFOHEADER) ) &&
		( lprc ) && 
		( lprc->right ) &&
		( lprc->bottom ) &&
		( pt.x = lpbmih->biWidth ) &&
		( pt.y = lpbmih->biHeight ) &&
		( lprc->right <= pt.x ) &&
		( lprc->bottom <= pt.y ) )
	{
		LPSTR	lpn, lpo;
		dxo = BYTESPERLINE( (pt.x), (lpbmih->biBitCount) );
		dxn = BYTESPERLINE( (lprc->right), (lpbmih->biBitCount) );
		lpn = FindDIBBits( lpnew );
		lpo = FindDIBBits( lpold );
		for( y = 0; y < lprc->bottom; y++ )
		{
			yyo = pt.y - y - 1;			// Scan line of OLD
			yyn = lprc->bottom - y - 1;	// Scan line of NEW
			oldoff = yyo * dxo;
			newoff = yyn * dxn;
			for( x = 0; x < dxn; x++ )
			{
				lpn[newoff+x] = lpo[oldoff+x];
			}
		}
		flg = TRUE;
	}
	return flg;
}

#ifdef	USECRTHRD
//#ifndef	USEMETHOD2
// ================================================================
// DWORD WINAPI ClipThreadProc( LPVOID lpParameter ) // thread data
//
// Purpose: To build the BIG BITMAP
//
// SetCursor
// ================================================================
DWORD WINAPI ClipThreadProc( LPVOID lpParameter )	// thread data
{
	DWORD		dwRet = 0;
	HANDLE		hDIBInfo;
	LPDIBINFO	lpDIBInfo;
	HWND		hWnd;
	HANDLE		hDIB;
	LPSTR		lpDIBHdr, lpDIBBits;
	HDC			hDC;
	RECT		rc;
	HCURSOR		hOldCur;
	BOOL		bAbort, bFail;

#ifdef	DIAGDIB2
	LPSTR		lpd;
#endif	// DIAGDIB2

	hOldCur = SetCursor( LoadCursor( NULL, IDC_APPSTARTING ) );
	bFail = bAbort = FALSE;
#ifdef	DIAGDIB2
	lpd = GetTmp3();
	wsprintf( lpd, "Thread: Beginning at %s."MEOR, GetDT4s(0) );
	DO(lpd);
#endif	// DIAGDIB2
	hDIB = 0;
	hWnd = 0;
	lpDIBHdr = 0;
	hDC = 0;
	// This is a COPY of the hDIBInfo
	if( ( hDIBInfo = lpParameter                          ) &&
		 ( lpDIBInfo = (LPDIBINFO)DVGlobalLock( hDIBInfo ) ) )
	{
		if( ( hDIB = lpDIBInfo->hDIB          ) &&
			( hWnd = lpDIBInfo->di_hwnd        ) &&
			( lpDIBHdr  = DVGlobalLock( hDIB ) ) &&   // LOCK DIB HANDLE
			( hDC = GetDC( NULL )              ) &&
			( IsWindow( hWnd )                 ) &&
			( GetClientRect( hWnd, &rc )       ) )
		{
// lpDIBInfo->hBitmap = DIBToBitmap( hDIB, lpDIBInfo->hPal, lpDIBInfo );
// =================================================================
			HBITMAP		hBitmap;
			HPALETTE	hPal, hOldPal;
			DWORD		dwsz, dwnsz;
			LPBITMAPINFOHEADER	lpbmih;

			hOldPal = 0;
			lpDIBBits = FindDIBBits( lpDIBHdr );

			if( hPal = lpDIBInfo->hPal )
				hOldPal = SelectPalette( hDC, hPal, FALSE );

			RealizePalette( hDC );
			lpbmih = (LPBITMAPINFOHEADER)lpDIBHdr;
// **************************************************
				if( ( dwnsz = ( rc.bottom * BYTESPERLINE( (rc.right),
					(lpbmih->biBitCount ) ) ) ) < 2000000 )
				{
					int		cx, cy, cx1, cy1;
					HANDLE	hnDIB;
					HBITMAP	hBmp2;
					BOOL	fLast;

#ifdef	DIAGDIB2
					wsprintf( lpd,
						"Thread: Retry CreateDIBitmap with Bitsize = %u Bytes. (%dx%d)"MEOR,
						dwnsz,
						rc.right,
						rc.bottom );
					DO(lpd);
#endif	// DIAGDIB2
					dwsz = 0;
					lpDIBInfo->di_rcDib2 = rc;
					if( ( gdwThreadSig ) &&
						( gdwThreadSig & lpDIBInfo->di_dwThreadBit ) )
					{
						bAbort = TRUE;
						goto ThreadExit;
					}
					hBitmap = GetClipDIB( lpDIBInfo, lpbmih, hDC );
					if( ( hBitmap ) &&
						( hnDIB = lpDIBInfo->di_hDIB2 ) )
					{
						// Get the SCROLL RANGE
						cy = lpbmih->biHeight - rc.bottom - 1 +
							GetSystemMetrics( SM_CYHSCROLL );
						cx = lpbmih->biWidth - rc.right - 1 +
							GetSystemMetrics( SM_CXVSCROLL );
						cx1 = rc.right / SCROLL_RATIO;
						if( !cx1 )
							cx1 = 1;
						cy1 = rc.bottom / SCROLL_RATIO;
						if( !cy1 )
							cy1 = 1;
						// Set ONE UNIT (pixel) of scroll
						// for THIS window SIZE
						lpDIBInfo->di_iOnex = cx1;
						lpDIBInfo->di_iOney = cy1;
						lpDIBInfo->di_rcDib2.right += cx1;
						lpDIBInfo->di_rcDib2.bottom += cy1;
						fLast = FALSE;
#ifdef	DIAGDIB2
						dwnsz = ( lpDIBInfo->di_rcDib2.bottom *
							BYTESPERLINE( (lpDIBInfo->di_rcDib2.right),
								(lpbmih->biBitCount ) ) );
						wsprintf( lpd,
							"Thread: Retry OK CreateDIBitmap with Bitsize = %u Bytes. (%dx%d)"MEOR,
							dwnsz,
							lpDIBInfo->di_rcDib2.right,
							lpDIBInfo->di_rcDib2.bottom );
						DO(lpd);
#endif	// DIAGDIB2
						while( hBmp2 = GetClipDIB( lpDIBInfo, lpbmih, hDC ) )
						{
							DeleteObject( hBitmap );
							DVGlobalFree( hnDIB );
							hBitmap = hBmp2;
							hnDIB = lpDIBInfo->di_hDIB2;
							rc.right += cx1;
							rc.bottom += cy1;
							if( ( gdwThreadSig ) &&
								( gdwThreadSig & lpDIBInfo->di_dwThreadBit ) )
							{
								bAbort = TRUE;
								goto ThreadExit;
							}
							if( ( ( lpDIBInfo->di_rcDib2.right + cx1 ) < lpbmih->biWidth ) &&
								( ( lpDIBInfo->di_rcDib2.bottom + cy1 ) < lpbmih->biHeight ) )
							{
								lpDIBInfo->di_rcDib2.right += cx1;
								lpDIBInfo->di_rcDib2.bottom += cy1;
							}
							else
							{
								if( fLast )
								{
									lpDIBInfo->di_bTooBig = FALSE;
									break;
								}
								else
								{
									lpDIBInfo->di_rcDib2.right = lpbmih->biWidth;
									lpDIBInfo->di_rcDib2.bottom = lpbmih->biHeight;
									fLast = TRUE;
								}
							}
#ifdef	DIAGDIB2
							dwnsz = ( lpDIBInfo->di_rcDib2.bottom *
								BYTESPERLINE( (lpDIBInfo->di_rcDib2.right),
									(lpbmih->biBitCount ) ) );
							wsprintf( lpd,
								"Thread: Retry OK CreateDIBitmap with Bitsize = %u Bytes. (%dx%d)"MEOR,
								dwnsz,
								lpDIBInfo->di_rcDib2.right,
								lpDIBInfo->di_rcDib2.bottom );
							DO(lpd);
#endif	// DIAGDIB2
						}
						lpDIBInfo->di_rcDib2 = rc;
#ifdef	DIAGDIB2
						dwnsz = ( lpDIBInfo->di_rcDib2.bottom *
							BYTESPERLINE( (lpDIBInfo->di_rcDib2.right),
								(lpbmih->biBitCount ) ) );
						wsprintf( lpd,
							"Thread: Ended OK CreateDIBitmap with Bitsize = %u Bytes. (%dx%d)"MEOR,
							dwnsz,
							lpDIBInfo->di_rcDib2.right,
							lpDIBInfo->di_rcDib2.bottom );
						DO(lpd);
#endif	// DIAGDIB2
					}
					else
					{
#ifdef	DIAGDIB2
						lstrcpy( lpd, "Thread: New size also FAILED!"MEOR );
						DO(lpd);
#endif	// DIAGDIB2
						bFail = TRUE;
					}
				}
				else
				{
#ifdef	DIAGDIB2
					lstrcpy( lpd, "Thread: Windows size also too large!"MEOR );
					DO(lpd);
#endif	// DIAGDIB2
					bFail = TRUE;
				}
// **************************************************
ThreadExit:
			if( hOldPal )
				SelectPalette( hDC, hOldPal, FALSE );

			ReleaseDC( NULL, hDC );
			DVGlobalUnlock( hDIB );    // UNLOCK DIB HANDLE - for successful thread exit
// =================================================================
		}
		else
		{
			if( !hDIB )
			{
#ifdef	DIAGDIB2
				lstrcpy( lpd, "Thread: FAILED due to no DIB handle!"MEOR );
				DO(lpd);
#endif	// DIAGDIB2
			}
			else if( !hWnd )
			{
#ifdef	DIAGDIB2
				lstrcpy( lpd, "Thread: FAILED due to no WINDOW handle!"MEOR );
				DO(lpd);
#endif	// DIAGDIB2
			}
			else if( !lpDIBHdr )
			{
#ifdef	DIAGDIB2
				lstrcpy( lpd, "Thread: FAILED due to hDIB did NOT give pointer!"MEOR );
				DO(lpd);
#endif	// DIAGDIB2
			}
			else if( !hDC )
			{
#ifdef	DIAGDIB2
				lstrcpy( lpd, "Thread: FAILED due to NO HDC!"MEOR );
				DO(lpd);
#endif	// DIAGDIB2
			}
			else
			{
#ifdef	DIAGDIB2
				lstrcpy( lpd, "Thread: FAILED due to WINDOW handle NOT A WINDOW!"MEOR );
				DO(lpd);
#endif	// DIAGDIB2
			}
			if( hDC )
				ReleaseDC( NULL, hDC );
			if( hDIB && lpDIBHdr )
				DVGlobalUnlock( hDIB );    // UNLOCK DIB HANDLE - for non-successful thread exit
			bFail = TRUE;
		}

		DVGlobalUnlock( hDIBInfo );

		// If ALL APPEARS OK
		// =================
		if( ( !bFail           ) &&
			 ( !bAbort          ) &&
			 ( hWnd             ) &&
			 ( IsWindow( hWnd ) ) )
		{
			// POST A MESSAGE
			PostMessage( hWnd, MYWM_THREADDONE, 0, (LPARAM)hDIBInfo );
		}
		else
		{
			// else TOSS the objects, and memory
			if( lpDIBInfo = (LPDIBINFO)DVGlobalLock( hDIBInfo ) )
			{
//				if( lpDIBInfo->di_hDIB2 )
//					DVGlobalFree( lpDIBInfo->di_hDIB2 );
				RelChildMem( lpDIBInfo );
				if( lpDIBInfo->di_hBitmap2 )
					DeleteObject( lpDIBInfo->di_hBitmap2 );
				DVGlobalUnlock( hDIBInfo );
			}
			DVGlobalFree( hDIBInfo );
		}
	}

#ifdef	DIAGDIB2
	*lpd = 0;

	if( bAbort )
		lstrcpy( lpd, "Thread: Responding to ABORT request!"MEOR );

	wsprintf( EndBuf(lpd),
		"Thread: Ending at %s."MEOR, GetDT4s(0) );
	DO(lpd);
#endif	// DIAGDIB2

	if( hOldCur )
		SetCursor( hOldCur );

	return dwRet;

}


//#ifndef	USEMETHOD2
// CreateThread
HANDLE	CreateClipThread_NOT_USED( HANDLE hDIBInfo, LPDWORD lpThreadId )
{
	HANDLE	hThread;

	hThread = CreateThread(
		NULL,	// pointer to thread security attributes
		0,		// initial thread stack size, in bytes
		&ClipThreadProc,	// pointer to thread function
		(LPVOID)hDIBInfo,	// argument for new thread
		0,		// creation flags
		lpThreadId );	// pointer to returned thread identifier

	return hThread; 
}

#endif	// USECRTHRD

//#ifndef	USEMETHOD2
HBITMAP	GetClipDIB( LPDIBINFO lpDIBInfo, LPBITMAPINFOHEADER lpbmih,
				   HDC hDC )
{
	HBITMAP	hBmp = 0;
	POINT	pt;
	HANDLE	hnDib, hDIB;
	LPSTR	lpb, lpnd;

	hDIB = 0;
	hnDib = 0;
	lpnd = 0;
	lpb = 0;
	if( ( lpDIBInfo && lpbmih && hDC ) &&
		 ( hDIB = lpDIBInfo->hDIB     ) &&
		 ( lpb = DVGlobalLock( hDIB ) ) )   // LOCK DIB HANDLE
	{
		if( ( pt.x = lpDIBInfo->di_rcDib2.right ) &&
			( pt.y = lpDIBInfo->di_rcDib2.bottom ) )
		{
			DWORD	ndsz;
			ndsz = sizeof(BITMAPINFOHEADER) +
				PaletteSize( (LPSTR)lpbmih ) +
				( pt.y * BYTESPERLINE( (pt.x),
					(lpbmih->biBitCount) ) );
			//if( ( hnDib = DVGlobalAlloc( GHND, ndsz ) ) &&
			if( ( hnDib = DVGAlloc( "ClipDIB", GHND, ndsz ) ) &&
				 ( lpnd = DVGlobalLock( hnDib )        ) )   // LOCK DIB HANDLE - new allocation
			{
				InitBitmapInfoHeader( (LPBITMAPINFOHEADER)lpnd,
					pt.x,
					pt.y,
					lpbmih->biBitCount );
				if( CopyDIBPalette( lpnd, lpb ) )
				{
					if( ClipDIBBits( lpnd, lpb, &lpDIBInfo->di_rcDib2 ) )
					{
						LPSTR	lpnb;
						lpnb = FindDIBBits(lpnd);
						hBmp = CreateDIBitmap( hDC,
							(LPBITMAPINFOHEADER)lpnd,
							CBM_INIT,
							lpnb,
							(LPBITMAPINFO) lpnd,
							DIB_RGB_COLORS );
						if( hBmp )
						{
							lpDIBInfo->di_hDIB2    = hnDib;
							lpDIBInfo->di_hBitmap2 = hBmp;
							DVGlobalUnlock( hnDib );   // UNLOCK DIB HANDLE - new, for succes return
							hnDib = 0;  // kill handle so will NOT be freed below
							lpnd = 0;
						}
					}
				}
   		}
		}
	}

   // clean up what needs to be cleaned up
	if( hnDib && lpnd )
		DVGlobalUnlock( hnDib );   // UNLOCK DIB HANDLE - new BMP failed unlock to free
	if( hnDib )
		DVGlobalFree( hnDib );
	if( hDIB && lpb )
		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE

	return hBmp;
}

#endif	// !USEMETHOD2

// ===========================================================
int NewStretchDIBits(
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
    DWORD  dwRop   // raster operation code
   )
{
	BITMAPINFOHEADER	bmiTemp;
	float				fDestYDelta;
    LPBYTE				lpNewBits;		// Changing pointer to SCAN LINE(s)
    int					i;

	// Check for NULL pointers and return error
    if( lpBits == NULL )
		return 0;
	if( lpBitsInfo == NULL )
		return 0;

	// Get increment value for Y axis of destination
	fDestYDelta = (float)nDestHeight / (float)nSrcHeight;

	// Make backup copy of BITMAPINFOHEADER
	bmiTemp = lpBitsInfo->bmiHeader;

	// Adjust image sizes for one scan line
	lpBitsInfo->bmiHeader.biSizeImage =
		BYTESPERLINE( lpBitsInfo->bmiHeader.biWidth,
			lpBitsInfo->bmiHeader.biBitCount );

	lpBitsInfo->bmiHeader.biHeight = 1;

	// Initialize pointer to the image data
	lpNewBits = (LPBYTE)lpBits;

    // Do the stretching
    for( i = 0; i < nSrcHeight; i++ )
	{
		if( !StretchDIBits( hdc,
			XDest,
			YDest + (int)floor(fDestYDelta * (nSrcHeight - (i+1))),
			nDestWidth,
			(int)ceil(fDestYDelta),
			XSrc,
			0,
			nSrcWidth,
			1,
			lpNewBits,
			lpBitsInfo,
			iUsage,
			SRCCOPY ) )
		{
			break; // Error!
		}
		else	// Increment image pointer by one scan line
		{
			lpNewBits += lpBitsInfo->bmiHeader.biSizeImage;
		}
	}

    // Restore BITMAPINFOHEADER
    lpBitsInfo->bmiHeader = bmiTemp;

    return(i);
}

// ===========================================================
// eof - DvDib.c


