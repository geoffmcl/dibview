
// DvPaint.h
#ifndef	_DvPaint_h
#define	_DvPaint_h

// Ensure LPDIBINFO has been defined
#ifndef	_DvChild_h_
#include	"DvChild.h"
#endif	// _DvChild_h

void DIBPaint( LPDIBINFO lpDIBInfo, HDC hDC,
			  LPRECT lpDCRect, HANDLE hDIB,
			  LPRECT lpDIBRect, HPALETTE hPal, DWORD dwROP );

void DDBPaint( LPDIBINFO lpDIBInfo, HDC hDC,
			  LPRECT lpDCRect, HBITMAP hDDB,
			  LPRECT lpDDBRect, HPALETTE hPal, DWORD dwROP );

void SetDIBitsPaint( LPDIBINFO lpDIBInfo, HDC hDC,
					LPRECT lpDCRect, HANDLE hDIB,
					LPRECT lpDIBRect, HPALETTE hPal, DWORD dwROP );


#endif	// _DvPaint_h
// eof
