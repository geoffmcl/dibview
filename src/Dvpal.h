

// DvPal.h (was palette.h)
#ifndef	_DvPal_h
#define	_DvPal_h

#ifndef	_mwin32_h
#include	"win32.h"	// For MLPTR = 32-bit flat pointer
#endif

// Size of window extra bytes (we store a handle to a PALINFO structure).
// NOTE: Allocate MORE than used - ie some spare ...
#define PAL_CBWNDEXTRA  (2 * sizeof (DWORD))


// Window Extra - words or 32-bit handles, etc
#define WW_PAL_HPALINFO		0	// Handle to PALINFO structure.
// Note: Offsets must take account of items size ...

typedef struct	tagPWCS	{	/* pw */
	HWND	pw_hMDI;	// "Parent" MDI Child (But can be DESTROYED)
	HGLOBAL	pw_hDIBInfo;// Likewise - DIB Info of child, but can be DESTROYED
}PWCS;
typedef PWCS MLPTR LPPWCS;

#define		MXRES		8

typedef struct tagPALINFO {

	HWND		pi_hWnd;	// PAL window handle
	RECT		pi_rClient;	// GetClientRect(hWnd,.) size
	RECT		pi_grCli;	// Client rectangle, after size done

	HPALETTE	hPal;		// Handle to palette being displayed.
	DWORD		wPEntries;	// # of entries in the palette.
	int			nSquareSize; // Size of palette square (see PAL_SIZE)
	int			nRows, nCols; // # of Rows/Columns in window.
	int			cxSquare, cySquare; // Pixel width/height of palette square.
	DWORD		wEntry;		// Currently selected palette square.
	HWND		hMdi;		// Handle of the MDI Child
	HANDLE		hDInf;		// Handle of MDI Child Info
	DWORD		wDColCnt;	// Count of COLORS in DIB
	HGLOBAL		hCols;		// Handle to COPY of DIB Colors
	int			pi_iNormal;
	int			pi_iInvert;
	HGLOBAL		pi_hCopyPal;	/* handle of COPY - after paint  */

	DWORD		pi_dwCnt;		/* physical COUNT of colors */
	HGLOBAL		pi_hCopyCOLR;	/* handle of COPY of PALEX array */
	HGLOBAL		pi_hSortCOLR;	/* handle of SORT of PALEX array */

	HWND		hInfoWnd;		// Handle to the info bar window.
	RECT		pi_grInfoBar;	// Position and Size of control
	TEXTMETRIC  pi_gtm;			// from last SIZE message

	DIBINFO		pi_sCopyDI;	/* copy of DIB information, but take */
	/* CARE with handles since the CHILD may be destroyed, and   */
	/* thus the handle(s) may NOT be VALID ********************* */

	/* some spares ... */
	HANDLE		pi_hRes[MXRES];
	DWORD		pi_dwRes[MXRES];
	RECT		pi_rRes[MXRES];
	HGLOBAL		pi_hgRes[MXRES];

}PALINFO, * PPI;

//typedef PALINFO MLPTR LPPALINFO;


//RECT		grInfoBar;
//TEXTMETRIC  gtm;
#define	grInfoBar		lpPalInfo->pi_grInfoBar
#define	gtm				lpPalInfo->pi_gtm
// Client rectangle, after size done
#define	grCli			lpPalInfo->pi_grCli

// The following define is for CopyPaletteChangingFlags().

#define DONT_CHANGE_FLAGS -1

// The following is the palette version that goes in a
//  LOGPALETTE's palVersion field.
#define PALVERSION   0x300

// This is an enumeration for the various ways we can display
//  a palette in PALETTEWNDPROC().
enum PAL_SIZE
   {
   PALSIZE_TINY = 0,
   PALSIZE_SMALL,
   PALSIZE_MEDIUM,
   PALSIZE_LARGE
   };

// Function prototypes.
//long MLIBCONV PALETTEWNDPROC (HWND hwnd,
//				UINT message,
//				WPARAM wParam,
//				LPARAM lParam);
LRESULT CALLBACK PALETTEWNDPROC(HWND hwnd,      // handle to window
                                UINT message,   // message identifier
                                WPARAM wParam,  // first message parameter
                                LPARAM lParam );// second message parameter

void SetPaletteWindowsPal (HWND hWnd, HPALETTE hPal);

HPALETTE DVGetSystemPalette( BOOL bShwErr );

HPALETTE CopyPaletteChangingFlags (HPALETTE hPal, BYTE bNewFlag);

void MyAnimatePalette (HWND hWnd, HPALETTE hPal);

int ColorsInPalette (HPALETTE hPal);

#define CopyPalette(hPal)  CopyPaletteChangingFlags (hPal,(BYTE) DONT_CHANGE_FLAGS)

#define CopyPalForAnimation(hPal) CopyPaletteChangingFlags (hPal, PC_RESERVED)

#endif	// _DvPal_h
// eof - DvPal.h (was Palette.h)

