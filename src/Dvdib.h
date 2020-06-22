
// DvDib.h
#ifndef	_DvDib_h
#define	_DvDib_h

#define	CHGADDTO	// Only ADD file in certain cases ...
#define	Win30_Bmp	(sizeof (BITMAPINFOHEADER))

#define	MXELST	10
#define	MXCLST	3	/* Currently only 3 entries ... */

// WIDTHBYTES takes # of bits in a scan line and rounds up to nearest
//  dword (32-bits). The # of bits in a scan line would typically be
//  the pixel width (bmWidth) times the BPP (bits-per-pixel = bmBitsPixel)
#define WIDTHBYTES(bits)      (((bits) + 31) / 32 * 4)

   // Given a pointer to a DIB header, return TRUE if is a Windows 3.0 style
   //  DIB, false if otherwise (PM style DIB).

#define IS_WIN30_DIB(lpbi)  ( (*(LPDWORD) (lpbi)) == Win30_Bmp )
// Turn OFF support for NOT RGBQUAD - RGB Bitmap is array 4 bytes
// each Byte is RED GREEN BLUE, and the 4th is also used
// in some GDI calls
// Accept any value as Window 4 byte Bitmap display arrays
// =======================================================
// WHY?????????
//#define IS_WIN30_DIB( lpbi )  ((*(LPDWORD) (lpbi)) != 0 ))


void     RealizeDIBPalette    (HDC hDC, LPBITMAPINFO lpbmi);
DWORD     DIBNumColors         (LPSTR lpbi);
LPSTR    FindDIBBits          (LPSTR lpbi);
DWORD     PaletteSize          (LPSTR lpbi);
HPALETTE CreateDIBPalette     (HANDLE hDIB);
DWORD    DIBHeight            (LPSTR lpDIB);
DWORD    DIBWidth             (LPSTR lpDIB);
extern	HBITMAP	DIBToBitmap		( HANDLE hDIB, HPALETTE hPal,
								 LPVOID lpDIBInfo );
HANDLE   DVBitmapToDIB        (HBITMAP hBitmap, HPALETTE hPal);
void     InitBitmapInfoHeader (LPBITMAPINFOHEADER lpBmInfoHdr,
                                            DWORD dwWidth,
                                            DWORD dwHeight,
                                              int nBPP);
DWORD	DIBBitCount( LPSTR );
DWORD	CalcDIBColors( LPSTR lpbi );

#endif	// _DvDib_h
// eof - DvDib.h

