
// DvView.h
// Was dibview.h - main program header - included in EVERY module!
#ifndef	_DvView_h
#define	_DvView_h

#ifdef	WIN32
#ifndef _WINUSER_
#include	<winuser.h>
#endif	// _WINUSER_
#endif	// WIN#@

// Some magic numbers
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
#define	ADDTIMER1		/* ON to have a WM_TIMER to the Frame ... */
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

// For GIF Animation
#ifdef	ADDTIMER1   // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
#define TIMER_ID1         1     // Timer ID when palette animating.
#define TIMER_INTERVAL1 200   // # of ms between timer ticks (for animation)
#endif	/* ADDTIMER1 */

// For palette animation
#define TIMER_ID2         2     // Timer ID when palette animating.
#define TIMER_INTERVAL2 100   // # of ms between timer ticks when animating.
#define	ADDPROGM

#define	MXANIMS		5	/* Maximum number of ANIMATED images ... */

// User defined messages.
#define  MYWM_CHANGEPCT    WM_USER + 1000    // Sent msg to dialog change %
#define  MYWM_ADDINFO      WM_USER + 1001    // Sent text to info line
#define	MYWM_RUNTWO			WM_USER + 1002	   // Send FROM 2nd invocation
#define  MYWM_ADDINFO2     WM_USER + 1003    // Sent text 2 to info line
#define  MYWM_CHANGEJOBS   WM_USER + 1004    // send the print dialog JOBS count

// Some macros.
#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)


#define szMDIChild "MyDIBMDI"          // Class name of MDI children.
#define DRAGCURSOR "DragCursor"        // Name of dragging cursor in .RC file.

#define SWAP(x,y)   ((x)^=(y)^=(x)^=(y))
//extern	HINSTANCE	hInst;
#ifdef	WIN32
#define	GETHINST( x )	ghDvInst
#else	// !WIN32
#define GETHINST( x )  ((HINSTANCE) GetWindowWord( x, GWW_HINSTANCE ))
#endif	// WIN32 y/n

// Some FEATURES - NOTE: Some of these are ONLY development switches,
//	and thus CAN NOT BE OFF in FINAL Version ...

#define	ADDOPENALL
#define	CVLIBBUG	// Can NOT uload and re-load library, so as a fix
// once the library is loaded, leave it LOADED. Actually, may also be
// required to KEEP the MODIFIED instance DATA when CONFIG Changed.
#define	ADDRESTORE
#define	TICKINI		// Change from an Option 3 to CHECK on POPUP ... */
//#ifdef	ADDTIMER1
//#define	TIMEDPUT
//#else
#undef	TIMEDPUT		// Feature NOT completed ...
//#endif	// ADDTIMER1

#undef	DIAGSAVEHOOK	// Write the HOOK messages to a FILE

// Global variables.

//extern HANDLE hInst;                   // Handle to this instance
//extern HWND   hWndMDIClient;           // MDI Client's window handle.


#define	MXFILNAM		260
#define	MXLTXT			260

//  Defines for File Save format combo box
//#define ID_FS_BEGIN       IDS_PM
//#define ID_FS_END         IDS_24

// Menu Defines for main menu.
// NOTE: MUST agree with RC file
// MENU_FILE - Zero relative position
#define	FILE_MENU			0
#define	EDIT_MENU			1
#define	PALETTE_MENU		2
#define  OPTION_MENU       3          // position of option menu
#define	CAPTURE_MENU		4
#define  WINDOW_MENU       5          // position of window menu
#define	HELP_MENU			6
// ==================================

#define	MXTMPSTR		256	// Max. stack temporary string

// Function Prototypes
// int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);

#endif	// _DvView_h
// eof - DibView.h
