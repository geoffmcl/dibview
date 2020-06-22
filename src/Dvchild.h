
// DvChild.h
#ifndef	_DvChild_h
#define	_DvChild_h

/* Required for OPTIONINFO ... */
#ifndef OPTIONS_INCLUDED
#include "DvOpts.h"
#endif	// OPTIONS_INCLUDED

#ifndef	_mwin32_h
#include "DvWin32.h"		// Some 16-32 bit fixes ...
#endif	// _mwin32_h

// ensure DvGif.h included
#include "DvGif.h"
// ensure LINKED LIST included
#include "DVList.h"
// ensure INFO structure ISTR, * PIS included before
#include "DvInfo.h"

#ifndef _DvMagnify_h_
#include "DvMagnify.h"  // for MAGNIFY and SWISH structures
#endif // _DvMagnify_h_

// Externally used variables.
extern BOOL  bUseDDBs;
extern HWND  hWndAnimate;
extern HWND  hWndClip;
extern POINT ptClipSize;
//extern int   nDIBsOpen;

// Defines

#ifndef MAX_FILENAME
#define MAX_FILENAME 264	// Max # of bytes in a filename/path.
#endif

// A magic number
#define	SCROLL_RATIO	4	// WM_VSCROLL scrolls DIB by 1/x of client

// A magic macro
#define		EndBuf(a)		( a + lstrlen(a) )
// Class extra bytes for MDI Children.
//  Room for DIBINFO structure handle.
#ifdef WIN64
#define CBWNDEXTRA		(2 * sizeof(void *))	// Set REQUIRED
// MDI Child Window Words.
#define WW_DIB_HINFO	0		// Handle to DIBINFO structure
#define	WW_DIB_EX2		sizeof(void *)		// Presently NOT used ...
#else // !WIN64
#define CBWNDEXTRA		(2 * sizeof(DWORD))	// Set REQUIRED
// MDI Child Window Words.
#define WW_DIB_HINFO	0		// Handle to DIBINFO structure
#define	WW_DIB_EX2		4		// Presently NOT used ...
#endif // WIN64 y/n

// Structure whose handle is stored in the MDI Children's window
//  extra bytes.  Has all the info needed to be stored on a per
//  bitmap basis.

// Defines for dwDIBFlag (Begins as dwCDIBFlag in create ... )
// INPUT in the rd_Caller member - GetDIB Callers!
#define	gd_CMask	0x00000fff	// Caller MASK
#define	gd_SF		0x00000001	// From Show_File() in DvFile.c
#define	gd_DP		0x00000002	// From Do_IDD_PREVIEW() in DvFile.c
#define	gd_OF		0x00000004	// From OpenDIBFile() in File.c
#define	gd_LI		0x00000008	// From LoadIFile() in File.c
#define	gd_PC		0x00000010	// From ParseCommandLine in File.c
#define	gd_AL		0x00000020	// From DoAutoLoad in Dvini.c
#define	gd_EX		0x00000040	// From EXTERNAL Source
// ========================...========

// Defines for dwDIBFlag (Begins as dwCDIBFlag in create ... )
#define	df_FMask	0x0ffff000	// Flag MASK
// ====================....=================
#define	df_EXTRNF	0x00004000	// External Flag
#define	df_EXTRNG	0x00008000	// External GRAF Flag
#define	df_IDOPEN	0x00010000	// From IDM_OPEN
#define	df_CAPTURE	0x00020000	// From IDM_Capture
#define	df_SEARCH	0x00040000	// From SrSearch set ...
#define	df_CONVJPEG	0x00080000	// After CONVERSION to JPEG ...
#define	df_COMMAND	0x00100000	// From Command line at startup ...
#define	df_GIFONE	0x00200000	// From Open Single GIF Image ...
#define	df_GIFGRP	0x00400000	// One of a GIF File Group ...
#define	df_CLIPBRD	0x00800000	// Pasted from the ClipBoard ...
#define	df_GIFONEOF	0x01000000	// One of a GROUP of GIF's ...
#define	df_GIFANIM	0x02000000	// Animate this NETSCAPE GIF ...
#define	df_AUTOLOAD	0x04000000	// From Frm_WM_TIMER(), DoAutoLoad() DvIni.c
#define	df_GIFGRPC	0x08000000	// COMBINED GIF Group
#define	df_DUPE		0x10000000	// From DvDupe.c (DUPLICATE)
#define  df_LOADSIB  0x20000000  // From Load_Sibling_File, from timer ...
#define  df_RENDERN  0x40000000  // From Do_IDM_RENDERNEW ...
// ==================================================

// OUPUT in the rd_dwFlag member - Results
#define	gd_RMask	0xf0000000	// Mask for RESULTS
#define	gd_GIFInfo2	0x40000000	// We are returning GIFHDREXT[1]!
#define	gd_GIFInfo	0x80000000	// We are returning GIFHDREXT[n]
// =============================================================

typedef struct tagRDIB {	/* rd */
	HWND	   rd_hWnd;
	HWND     rd_hMDI;    // MDI child window handle
	HANDLE* rd_pfHand;
	LPTSTR   rd_pTitle; // pointer file title - name only
	LPTSTR   rd_pPath;  // pointer file full path name     
	DWORD	   rd_Caller;
	DWORD	   rd_dwFlag;
	HGLOBAL	rd_hDIB;	// Special case for Show_File & GIF
	HGLOBAL	rd_hGIFInfo;	// A special special case for GIF
	DWORD	   rd_Res1;
	PMWL     rd_pMWL;
	 WIN32_FIND_DATA   rd_fd;   // file information - size, date
}RDIB;
typedef RDIB FAR * PRDIB;

// A ZERO structure macro
#define	NULPRDIB(a)    ZeroMemory(a, sizeof(RDIB))
//#define	NULRDIB(a)     NULPRDIB(&a) *** REMOVED *** - aloc/free a pointer

// A handle to memory containing this structure is passed into the
//  WM_CREATE case of the DIB window.  It's used to initialize the
//  DIBINFO structure stored in the DIB window's window words.
typedef struct {

	HANDLE	c_hDIB;	// Handle to the DIB.
//   DWORD    c_dwWid, c_dwHt;
	TCHAR    c_szFileName[MAX_FILENAME+4];	// Its filename.
	DWORD	   c_dwCDIBFlag;
	HGLOBAL	c_ghInfo;		// Handle to VARIOUS informations
   PRDIB    c_pRDib;      // pointer to RDIB info

}DIBCREATEINFO;

typedef DIBCREATEINFO MLPTR LPDIBCREATEINFO;

// And a function to ZERO it ...
#define	NULDIBCI(a)    ZeroMemory( &a, sizeof(DIBCREATEINFO) )

/* For SRWait( BOOL T/F, DWORD FLAG ) */
#define	SRW_BGN		1	/* Begin a WAIT if NOT already ON */
#define	SRW_INC		2	/* Just bump to next animation */
#define	SRW_END		3	/* If the LAST, remove WAIT */

// Bit flags for (DWORD) di_dwDnCount - Count flags completed
#define  fg_DnThread    0x00000001  // when thread count completes
#define  fg_DnMyWM      0x00000002  // received MYWM_THREADDONE2
#define  fg_GotFile     0x00000004  // read/written CLR file

#define  fg_Res32       0x80000000

// NOTE: ALLOCATED items should be CLEARED in
// void ChildWndDestroy( HWND hWnd )
// Of course if none ensure the item ZEROED
// in ChildWndCreate()
// ==========================================
typedef struct tagDIBINFO {		/* di */

	DWORD	   di_size;	   // just the size of this structure

	HWND	   di_hwnd;	   // Child Window handle owing this window
	
	HANDLE	hDIB;		// Handle to the DIB (device independant bitmap)
	HPALETTE hPal;		// Handle to the bitmap's palette.
	HBITMAP	hBitmap;	// Handle to the DDB (device dependant bitmap)

	DWORD	   wDIBType;	      // DIB's type - RGB, RLE4, RLE8, PM
	DWORD	   di_dwDIBBits;	   // Bits per pixel
	DWORD	   di_dwDIBWidth;	   // Width of the DIB
	DWORD	   di_dwDIBHeight;	// Height of the DIB
   DWORD    di_dwDIBSize;     // byte size of DIB bits

   // set in DIBToBitmap() functions from DIB values
   BOOL  di_bChgdDIB;      // if the SIZE changed (for animations)
	DWORD	di_dwNEWWidth;	   // NEW Width  of the DIB
	DWORD	di_dwNEWHeight;	// NEW Height of the DIB

	HANDLE	hBigFile;	// If ANIMATING GIF, Keep whole file
	DWORD	   wMaxCnt;	   // Maximum number of images (in GIF)
	DWORD	   wCurCnt;	   // Current image 1 to MaxCnt
	DWORD	   dwMilCnt;	// Count to next movement/action
	DWORD	   wBgnCnt;	   // Beginning Anim. GIF ... NOT USED
	DWORD	   wGifNum;	   // The number of this GIF ...
	DWORD	   wGifTot;	   // The Total of this GIF set ...
   GIFSTR   wGifStr;    // child copy of ANIMATED GIF

	HGLOBAL	hgGIFSize;	// If a MULTIPLE image GIF, keep INFO
	DWORD	   dwGIFNext;	// ms to NEXT GIF action / or USER!!!

	RECT	rcClip;		   // Clipboard cut rectangle.
	RECT	rcClip2;		   // Clipboard cut rectangle painted (INVERTED)
	RECT	rcClip3;		   // Clipboard cut rectangle (for restore).
   HWND  hwndIFrame;    // handle to FRAME on IMAGEAT dialog
   RECT  rcIFrame;      // and its size

	DWORD	dwDIBFlag;	   // Various flags ... SEE ABOVE
	RECT	rWindow;	      // Windows Screen co-ordinates ...
	RECT	rClient;	      // Client Rectangle Size ...
   // magnify and swish
   MAGNIF   sMagnif;
   CLIPSWISH sSwish;

	OPTIONSINFO Options; // Option info from Options dialog box.

	RDIB	stmrDInfo;	   // Used by TIMER (Zero'ed after copy)

	WIN32_FIND_DATA fdEditFile;	// file data

	HANDLE	di_hDIB2;		// Secondary DIB
	HBITMAP	di_hBitmap2;	// Secondary BITMAP
	RECT	   di_rcDib2;	   // Scroll and Size
	HANDLE	di_hCOLR;		// Color count / distrib info

	int		di_iVert, di_iHorz;
	int		di_iOney, di_iOnex;

	BOOL	   di_bTooBig;		// TOO BIG ie > 2000000 Set when can not create BITMAP
							      // equal to DIB size
	HANDLE	di_hBigDib;		// Handle to BIG DIB Information
	HANDLE	di_hThread;		// Handle to THREAD
	DWORD	   di_dwThreadId;	   // Thread ID
	DWORD	   di_dwThreadBit;	// BIT signifying this thread

   DIBCREATEINFO  di_sCI;     // create info
   RDIB           di_sRD;     // read DIB info
   PMWL           di_psMWL;   // work list pointer
   ISTR           di_sIS;     // information structure

   HANDLE   di_hCLRFile;      // handle to CLR file
   DWORD    di_dwCLRSize;     // size of CLR balance = PALEX structure

	HANDLE	di_hCntThread;		// Handle to THREAD
	DWORD	   di_dwCntThdId;	   // Thread ID
   DWORD    di_dwNumber;      // Just a child number
   BOOL     di_bThdExit;      // force thread to exit
   DWORD    di_dwDnCount;      // Count flags completed

   TCHAR    di_szDibTitle[MAX_FILENAME];  // DIB Title
	TCHAR    di_szDibFile[ MAX_FILENAME];  // DIB's filename
   // full window title text being shown
   TCHAR    di_szTitle[1024]; // title of window

	TCHAR    di_szCGenBuff[1024]; // A general WORK buffer
	TCHAR    di_szCGenBuf2[2048]; // A general WORK buffer 2

}DIBINFO, * PDI;

typedef DIBINFO MLPTR LPDIBINFO;

#define  GETDI(a) (PDI)DVGlobalLock(GetWindowExtra(a, WW_DIB_HINFO))
#define  RELDI(a)    DVGlobalUnlock(GetWindowExtra(a, WW_DIB_HINFO))

#define  ADD2LIST(a)\
{\
   PDI   _pdi;\
   if( _pdi = GETDI( (*a).rd_hMDI ) ) {\
      _pdi->di_psMWL = (*a).rd_pMWL = AddToFileList4( a );\
      RELDI( (*a).rd_hMDI );\
   } else {\
      chkdi( __FILE__, __LINE__ );\
   }\
}

// Kept in WW_DIB_EX2 extra child window data
// ==========================================
typedef struct tagDIBX	{
	BOOL		dx_Valid;
	RECT		dx_Rc;
	COLORREF	dx_Face;
	COLORREF	dx_Shadow;
	COLORREF	dx_HiLite;
	COLORREF	dx_Text;
	LOGFONT	dx_LogFont;
}DIBX;
typedef DIBX MLPTR LPDIBX;

/* kept in lpDIBInfo->di_hCOLR */
/* =========================== */
typedef struct tagCOLR {
	COLORREF	cr_Color;
	DWORD		cr_Freq;
}COLR, * PCR;
typedef COLR MLPTR LPCOLR;

typedef struct tagPALEX {
	/* ======================================================= */
	DWORD	px_Size;		/* SIZE of this instance of struct */
	DWORD	px_Count;		/* COUNT of color listed in array  */
	COLR	px_Colrs[1];	/* array of COLOR and FREQUENCY    */
	/* ======================================================= */
}PALEX, * PPX;
typedef PALEX MLPTR LPPALEX;

#ifdef	Dv16_App
#define	COLOR_3DFACE	COLOR_BTNFACE
#define	COLOR_3DSHADOW	COLOR_BTNSHADOW
#define	COLOR_3DHILIGHT	COLOR_BTNHIGHLIGHT
#endif	// Dv16_App

// User defined message for MDI child windows.

#define MYWM_ANIMATE         WM_USER     // Start palette animation
#define MYWM_RESTOREPALETTE  WM_USER+1   // Stop animation and restore palette
#define MYWM_QUERYNEWPALETTE WM_USER+2   // Frame got a WM_QUERYNEWPALETTE, 
                                         //   realize child's palette as 
                                         //   foreground palette.
#define	MYWM_THREADDONE		(WM_USER + 3)	// Thread has completed
#define	MYWM_THREADDONE2		(WM_USER + 4)	// Count Thread has completed


//long MLIBCONV CHILDWNDPROC(HWND hWnd, 
//			     UINT message,
//			     WPARAM wParam,
//			     LPARAM lParam);
LRESULT CALLBACK CHILDWNDPROC(HWND hWnd,      // handle to window
                              UINT uMsg,      // message identifier
                              WPARAM wParam,  // first message parameter
                              LPARAM lParam   // second message parameter
);


HPALETTE CurrentDIBPalette        (HWND);	// Return PALETTE of MDI Child
HWND     GetCurrentMDIWnd         (void);
BOOL     GetCurrentDIBStretchFlag (void);
void     SetCurrentDIBStretchFlag (BOOL bFlag);

void     SendMessageToAllChildren(HWND hWnd,
								  UINT message,
								  WPARAM wParam,
								  LPARAM lParam);

void     CloseAllDIBWindows (void);

RECT     GetCurrentClipRect (HWND hWnd);
POINT    GetCurrentDIBSize  (HWND hWnd);

extern	void Hourglass( BOOL );

/* GLOBALLY EXPOSED child services */
extern	void	RelChildMem( LPDIBINFO lpDIBInfo );
// NOTE: Under MS Studio 4.2 this does NOT work, since it
//	maintains a COMPLETE list of DEPENDANCIES which is
//	continually UPDATED. But the speed of COMPILE ALL is OK
//	on the 133Mhz Pentium machine.
//#include	"childn.h"	// Where new items can be added without FULL COMPILE
// Should be EMPTY eventually ...
extern BOOL Get_DIB_Info( HWND hWnd, HANDLE * ph, LPDIBINFO * pdi );
extern VOID Release_DIB_Info( HWND hWnd, HANDLE * ph, LPDIBINFO * pdi );

#endif	// _DvChild_h
// eof - DvChild.h
