
// DvOpts.h
#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED

#ifndef _mwin32_h
#include "DvWin32.h"
#endif	// _mwin32_h

// Defines for help string table.
// ONLY uses values from DvRes.h
#define IDS_HELP_STRETCHWINDOW PLACEHOLDER_NOSTRETCH
#define IDS_HELP_NORMALWINDOW  IDRB_STRETCHWINDOW
#define IDS_HELP_USEDIBS       IDRB_USEDIBS
#define IDS_HELP_USEDDBS       IDRB_USEDDBS
#define IDS_HELP_USESETDIBITS  IDRB_USESETDIBITS
#define IDS_HELP_BESTFIT       IDRB_BESTFIT
#define IDS_HELP_STRETCH       IDRB_STRETCH
#define IDS_HELP_SCALE         IDRB_SCALE
#define IDS_HELP_XAXIS         IDEF_XAXIS
#define IDS_HELP_YAXIS         IDEF_YAXIS
#define IDS_HELP_USEBANDING    PLACEHOLDER_NOBANDING
#define IDS_HELP_NOBANDING     IDRB_USEBANDING
#define IDS_HELP_USE31APIS     PLACEHOLDER_NO31APIS
#define IDS_HELP_NO31APIS      IDRB_USE31APIS

#define IDS_HELP_NOANIM        IDRB_ANIMATE
#define IDS_HELP_ANIM          IDRB_NOANIM
#define IDS_HELP_MSECS         IDEF_MSECS

// Some magic numbers.
//#define SCROLL_RATIO    4     // WM_VSCROLL scrolls DIB by 1/x of client area.

#define	GFACTOR10		1000	// This seems MUCH larger
// than the DEFINITION which says 1/100th of a seconds. But this
// is STILL twice as fast as GIFCON!!! What should it be???

// The following defines are the default values for the OPTIONSINFO
//  structure stored in the child window's INFOSTRUCT (which is a
//  structure containing information on the child window's bitmap,
//  and list of options). Previously see ChildWndCreate() to see
//	how these are used, but in April 1998 moved this into InitWrkStr()
//	in DvData.c as the GLOBAL Options, and added INI Read/Write
//	of these items.

#define OPT_DEF_STRETCH   FALSE     // Don't stretch the DIB on display.
#define  OPT_DEF_CENTER    TRUE     // Center it on the PRINT page
//#ifdef	WIN32
//#define OPT_DEF_BANDING   FALSE         // NOT FUNCTIONING.
//#else
#define OPT_DEF_BANDING   TRUE          // Band DIB to printer.
//#endif
#define OPT_DEF_USE31PRNAPIS FALSE      // Don't use the 3.1 Printing APIs.
#define OPT_DEF_DISP      DISP_USE_DDBS // Display DDBs instead of DIBs.
#define OPT_DEF_PRNSIZE   PRINT_BESTFIT // Print in "best fit" mode.
#define OPT_DEF_PRNSCALEX 1             // X stretch factor = 1 (in PRINT_STRETCH mode)
#define OPT_DEF_PRNSCALEY 1             // Y stretch factor = 1 (in PRINT_STRETCH mode)
#define OPT_DEF_MILSECS   200           // Default Animate each 200 ms

// Some useful macros.
#define MAX(a,b)     ((a) > (b) ? (a) : (b))
#define MIN(a,b)     ((a) < (b) ? (a) : (b))

// There is now a GLOBAL set of these NOT in this structure,
// and a current COPY of the GLOBAL items is made when a NEW
// Window is created into the big DIBINFO structure.
// Also in IDM_OPTIONS the structure copied to/from dialog box.
typedef struct tagOPTIONINFO {

	BOOL	bStretch2W;		// True = stretch to window
#ifdef  WIN32
   BOOL  bCenter;       // True to CENTER DIB on PRINT page
#else // !WIN32
	BOOL	bPrinterBand;	// True = want to band DIB to printer.
	BOOL	bUse31PrintAPIs;// True = Use the 3.1 Printing API
#endif   // WIN32 y/n
	DWORD	wDispOption;	// See defines below
	DWORD	wPrintOption;	// See defines below
	DWORD	wXScale;		// X Scale Edit control value
	DWORD	wYScale;		// Y Scale Edit control value
	BOOL	bSetDefault;	// Set DEFAULT to these
	BOOL	bApplyAll;		// Apply to ALL current (if more than 1)
	BOOL	bIsAGIF;		// Is an animation GIF
	BOOL	bAnimate;		// Run animation (if NETSCAPE GIF)
	DWORD	dwMilSecs;		// Gap between each animation
   BOOL  bAspect; // True = keep original aspect ration

}OPTIONSINFO, MLPTR LPOPTIONSINFO;


// Values used for wDispOption in OPTIONSINFO structure.
#define DISP_USE_DIBS         IDRB_USEDIBS
#define DISP_USE_DDBS         IDRB_USEDDBS
#define DISP_USE_SETDIBITS    IDRB_USESETDIBITS

// Values used for wPrintOption in OPTIONSINFO structure.
#define PRINT_BESTFIT         IDRB_BESTFIT         // Best proportional stretch fit
#define PRINT_STRETCH         IDRB_STRETCH         // Stretch to fill page
#define PRINT_SCALE           IDRB_SCALE           // Independent X/Y scaling


void ShowOptions (HWND hWnd, LPOPTIONSINFO lpInfo);

#define	ChkDlg( a, b )	CheckDlgButton( hDlg, a, b )
#define	IsChk( a )		IsDlgButtonChecked( hDlg, a )

#define	it_None				0	// END of OPTIONS
#define	it_String			1
#define	it_Size4			2	// Four int's - Also see it_SizeW
#define	it_Dir				3
#define	it_Size1			4	// WORD size item
#define	it_BoolF			5	// Default is FALSE
#define	it_BoolT			6	// Default is TRUE
#define	it_Size2			7	// DWORD sized item
#define	it_Files			8	// Used file list
#define	it_Table			9	// Match to TABLE text
#define	it_General		10	// Just a general number
#define	it_Colr			11	// Read/Write Color Index
#define	it_SizeW			12	// Specifically INIT Window size
#define  it_WinSize  it_SizeW // FIX20080316
#define	it_StgVal		13	// String from/to INI equal Value
#define	it_Files2      14	// Used file list to LINKED LIST
#define  it_ListClip    15 // LInked list of CLIP entries

// it_StgVal TYPE - Matches an INI string with a Value
typedef struct {
	int		sv_Val;
	LPSTR	sv_Stg;
}STGVAL;
typedef STGVAL MLPTR LPSTGVAL;

#ifdef WIN64
typedef struct	tagINILIST {	/* i */
	LPSTR	i_Sect;
	LPSTR	i_Item;
	WORD	i_Type;
	LPSTR	i_Deft;
	LPINT	i_Chg;
	LPVOID	i_Void;
	LPVOID	i_Res1;
}INILIST, * PINILIST;
#else // !WIN64
typedef struct	tagINILIST {	/* i */
	LPSTR	i_Sect;
	LPSTR	i_Item;
	WORD	i_Type;
	LPSTR	i_Deft;
	LPINT	i_Chg;
	LPVOID	i_Void;
	DWORD	i_Res1;
}INILIST, * PINILIST;
#endif // WIN64 y/n
typedef INILIST MLPTR LPINILIST;

typedef	struct tagOPTLIST	{
	WORD	op_Type;
	LPINT	op_Flag;
	LPINT	op_Chg;
	WORD	op_ID;
	BOOL	op_Def;
	LPWORD	op_Num;
	WORD	op_DNum;
	WORD	op_NID;
	LPINT	op_Chgn;
	LPSTR	opi_Sect;
	LPSTR	opi_Item;
	WORD	opi_Type;
	LPSTR	opi_Deft;
}OPTLIST, * POPTLIST;
typedef	OPTLIST MLPTR LPOPTLIST;

extern	void	PutDefOpt( HWND, LPOPTLIST );
extern	BOOL	GetCurOpt( HWND, LPOPTLIST );
extern	void	PutCurOpt( HWND, LPOPTLIST );

#endif	// OPTIONS_INCLUDED
// eof - DvOpts.h
