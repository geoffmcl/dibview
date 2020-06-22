
// DvInfo.h
#ifndef		_DvInfo_h
#define		_DvInfo_h

#define		fc_1		0x00000001	// Changed WIDTH
#define		fc_2		0x00000002	// Changed HEIGHT
#define		fc_3		0x00000004	// Changed bm WIDTH
#define		fc_4		0x00000008	// Changed bm HEIGHT
#define		fc_5		0x00000010	// Changed Clip WIDTH
#define		fc_6		0x00000020	// Changed Clip HEIGHT
#define		fc_7		0x00000040	// Changed Clip left
#define		fc_8		0x00000080	// Changed Clip top
#define		fc_9		0x00000100	// Changed Clip right
#define		fc_10		0x00000200	// Changed Clip bottom
#define		fc_11		0x00000400	// Changed bm Colors
#define		fc_12		0x00000800	// Changed bm Colors

// FIX980430 - Do NOT use the BITMAP bm member since in the case
// of BIG DIB's this may NOT be the FULL BITMAP due to Windows
// limitation. Most info is in the DIBINFO/lpDIBInfo structure,
// and or the HANDLE hDIB for even more.
// FIX20001202 - Make this PART of DIBINFO structure, and
// use a THREAD to "party" with the image as soon as it is LOADED.
// Only ENABLE the menu item WHEN this thread has completed its WORK
//
typedef	struct	tagISTR	{
	HWND		   hMDI;
	HBITMAP		is_hBitmap;	// NOTE: May NOT exist for BIG DIBS
	HANDLE		hDIB;
	HANDLE		hDIBInfo;
#ifndef  USEITHREAD
	LPDIBINFO	lpDIBInfo;
	DWORD			cxDIB, cyDIB;
#endif   // !USEITHREAD
	RECT		   rcClient, rcClip;
	DWORD			iColors, iUsedCols, iCalcCols;
	BOOL		   fChanged;
	DWORD		   dwFlag;
	DWORD		   wBPP;
	HGLOBAL		hDIBx;
	LPSTR		   lpFN;	// File name
	UINT		   uActive;
	HWND		   hDlg;
	HPALETTE	   hPal;
	BOOL		   bIsAnimGIF;
	DWORD		   dwGIFCnt;
}ISTR, * PIS;
typedef ISTR FAR * LPISTR;

#define	NULPISTR(a)		ZeroMemory( a, sizeof(ISTR) )

// In DvInfo.c - Modeless diaglog with ONE line of information
// ===========================================================
extern	void	InfoInit( void );	// Create BRUSHES, etc
extern	void	InfoDestroy( void );		// Destroy BRUSHES, etc
extern	DWORD	PutInfo( LPSTR lpt );	// Put up INFO DLG with text in lpt
extern	DWORD	KillInfo( void );		// If instance count 0, take down

#endif	// _DvInfo_h
// eof - DvInfo.h
