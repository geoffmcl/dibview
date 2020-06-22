
// **********************************************************
//
//	File:  DvMain.C
//
//	Purpose:  Contains WinMain/the main message loop.
//
//	Functions:  WinMain
//
//	Comments:
//
//	History:
//	Date			Reason
//	6/1/91			Created
//	1997 Nov 24	A reset of ALL the files
//	1999 Jan 14 - Add an ACCELERATOR TABLE
//		For Ctrl+C=COPY Ctrl+V=Paste, etc...
//
// **********************************************************
#include	"dv.h"	// single inclusive project include ...

// external
extern	char	cNxtCmd[MAX_PATH+16];
extern   VOID  GetGlobPrinter( VOID );
extern   VOID  KillTifLists( VOID );
extern VOID Kill_Sibling_List( VOID );
extern   VOID  FreeClipList(VOID);

//extern	HWND ghMainWnd;
//extern	HANDLE hInst;    // Handle to this instance

// GLOBAL
// HWND   hWndMDIClient;        // MDI Client's window handle.
HANDLE	ghAccelTable;

#ifdef	ADDTIMER1
UINT	GotTimer = 0;
#endif	/* ADDTIMER1 */

#ifdef	ADDMDFORN
extern	void md_main (void);
extern	void md_end (void);
#endif	// ADDMDFORN
extern	void	KillEditBMPs( void );
extern	BOOL	InitWrkStr( HINSTANCE hInst );
extern	void	FreeWrkStr( void );

void	PassNm2Mem( void );

BOOL	bGotWMD = FALSE;
BOOL	bGotCCV = FALSE;

LPSTR	GetWMsgStg( UINT umsg )
{
	LPSTR	lpm;
	static char _szwmsg[32];

	lpm = &_szwmsg[0];

	switch( umsg )
	{
	case WM_KEYDOWN:
		lstrcpy( lpm, "WM_KEYDOWN" );
		break;

	default:
		lstrcpy( lpm, "[default]" );
		break;
	}

	return lpm;
}

//extern	char gszDiag[];
extern	void	ShowVKey( int nKey, SHORT pState, SHORT cState );

//WM_KEYDOWN 
//nVirtKey = (int) wParam;    // virtual-key code 
//lKeyData = lParam;          // key data 
WPARAM	wpLast = 0;
SHORT	wsLast;

BOOL	IsCtrlCV( MSG * pmsg )
{
	BOOL	flg = FALSE;
	SHORT	sn;

	if( ( pmsg->message == WM_KEYDOWN ) &&
		( GetKeyState( VK_CONTROL ) ) &&
		( ( pmsg->wParam == 'C' ) || ( pmsg->wParam == 'V' ) ) )
	{
		flg = TRUE;
		bGotWMD = TRUE;
		if( pmsg->wParam == 'C' )
		{
			DO( "Got Ctrl + C ...\n" );
		}
		else
		{
			DO( "Got Ctrl + V ...\n" );
		}

		bGotCCV = TRUE;

	}

	if( pmsg->message == WM_KEYDOWN )
	{
		sn = GetKeyState( pmsg->wParam );
		if( ( bGotWMD ) ||
			( pmsg->wParam != wpLast ) ||
			( wsLast != sn ) )
		{
			ShowVKey( pmsg->wParam, wsLast, sn );
			wpLast = pmsg->wParam;
			wsLast = sn;
		}
	}

	return flg;
}

void	chkme2( MSG * pmsg )
{
#ifndef  NDEBUG
	LPSTR	lps = &gszChkme2[0];
	wsprintf( lps,
		"After: hWnd=0x%x Msg=%d (%s) wP=0x%x lP=0x%x (M=%x C=%x)\r\n",
		pmsg->hwnd,
		pmsg->message,
		GetWMsgStg( pmsg->message ),
		pmsg->wParam,
		pmsg->lParam,
		ghMainWnd,
		ghWndMDIClient );
//	chkme( lps );
	DO( lps );
#endif   // #ifndef  NDEBUG
}

void	chkme3( void )
{
	if( bGotWMD )
		chkme( "NOTE: bGotWMD is TRUE" );
}

//---------------------------------------------------------------------
//
// Function:   WinMain
//
// Purpose:    What Windows calls when our application is started up.
//             Here, we do all necessary initialization, then enter
//             our message loop.  The command line is also parsed,
//             and if it lists any DIBs to open, they're opened up.
//
//             This is a pretty standard WinMain.
//
//             Since we're an MDI app, we call 
//             TranslateMDISysAccel (ghWndMDIClient, &msg) during
//             message loop processing.
//
// Parms:      hInstance     == Instance of this task.
//             hPrevInstance == Instance of previous DIBView task (NULL if none).
//             lpCmdLine     == Command line.
//             nCmdShow      == How DIBView should come up (i.e. normally,
//                              minimized, maximized, etc.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------
//int PASCAL WinMain(HANDLE hInstance,            // This instance
//                   HANDLE hPrevInstance,        // Last instance
//                   LPSTR  lpCmdLine,            // Command Line
//                   int    nCmdShow)             // Minimized or Normal?
int WINAPI WinMain(
  HINSTANCE hInstance,      // handle to current instance
  HINSTANCE hPrevInstance,  // handle to previous instance
  LPSTR lpCmdLine,          // command line
  int nCmdShow              // show state
)
{
	MSG msg;

	if( !InitWrkStr(hInstance) )  // initwork
      return -1;

	ghDvInst = hInstance;
	ghMainWnd = 0;
	ghAccelTable = 0;

	if( lpCmdLine )
		strcpy( &cNxtCmd[0], lpCmdLine );
	else
		cNxtCmd[0] = 0;

	if( !hPrevInstance )
	{
		// To DvInit.c to -
		// 1. Check if app already running
		// *******************************
		// 2. To REGISTER a class of
		// char szFrameClass[] = "MyDIBWClass";
		// Class name of frame window.
		// wc.lpfnWndProc = FRAMEWNDPROC;
		// Function to retrieve messages
		// 3. To REGISTER a class of
		// #define szMDIChild "MyDIBMDI"
		// Class name of MDI children.
		// wc.lpfnWndProc = CHILDWNDPROC;
		// 4. To Register a class of
		// char szPalClass[] = "MyDIBPalClass";
		// Class name of palette windows.
		// wc.lpfnWndProc = PALETTEWNDPROC;
		if( !InitMyDIB( hInstance, lpCmdLine ) )
		{
   		msg.wParam = 255; // Returns the value from PostQuitMessage
	   	goto WPgmExit;
		}
	}

   // no parent running, so SAFE to CREATE DIAG FILE
	DiagOpen();

	// Again to DvInit.c to do a bunch of things,
	// and create the FRAME window.
   if( !InitInstance( hInstance, nCmdShow ) ) {
  		msg.wParam = 255; // Returns the value from PostQuitMessage
   	goto WPgmExit;
   }

	ghAccelTable = LoadAccelerators( hInstance,
		MAKEINTRESOURCE( IDR_ACCELERATOR1 ) );

   if( !ghAccelTable ) {
  		msg.wParam = 255; // Returns the value from PostQuitMessage
   	goto WPgmExit;
   }

	// Parses Command line for DIB's
	ParseCommandLine( lpCmdLine );

#ifdef	ADDTIMER1
	if( ghMainWnd )
	{
		if( !(GotTimer = SetTimer( ghMainWnd, TIMER_ID1, TIMER_INTERVAL1, NULL)) )
		{
			DIBError( ERR_NOTIMERS );
		}
	}
#endif	/* ADDTIMER1 */

#ifdef	ADDMDFORN
	md_main();
#endif	// ADDMDFORN

	if( ghMainWnd == 0 )
	{
		msg.wParam = 255; // Returns the value from PostQuitMessage
		goto WPgmExit;
	}

   GetGlobPrinter();
   if( !InitCursors() ) {
		msg.wParam = 255; // Returns the value from PostQuitMessage
		goto WPgmExit;
   }

	bGotWMD    = FALSE;
   gfDvInited = TRUE;	// Initial size messages done ...

	// THE WINDOWS LOOP
	// ================
	//	BOOL GetMessage(
	//    LPMSG lpMsg,	// address of structure with message
	//    HWND hWnd,	// handle of window
	//    UINT wMsgFilterMin,	// first message
	//    UINT wMsgFilterMax );	// last message
	while( GetMessage( &msg,	// Put Message Here
		0,						// Handle of win receiving msg
		0,						// lowest message to examine
		0 ) )					// highest message to examine
	{

		//bGotWMD = FALSE;
//		if( IsCtrlCV( &msg ) )
//			chkme( "NOTE: Got CRTL+C or CTRL+V" );

//		if( !TranslateAccelerator( msg.hwnd, ghAccelTable, &msg ) )
//		if( !TranslateAccelerator( ghWndMDIClient, ghAccelTable, &msg ) )
		if( !TranslateAccelerator( ghMainWnd, ghAccelTable, &msg ) )
		{
			if( !TranslateMDISysAccel( ghWndMDIClient, &msg ) )
			{
				TranslateMessage(&msg);	// Translates virtual key codes
				DispatchMessage(&msg);	// Dispatches message to window
			}
			else
			{
				//chkme( "NOTE: Has been translated by MDI Accel" );
			}
		}
//		else
//		{
//			chkme2( &msg );
//		}
	}

WPgmExit:

#ifdef	ADDMDFORN
	md_end();
#endif	// ADDMDFORN
	KillEditBMPs();
   KillTifLists();
   Kill_Sibling_List(); // if any outstanding SIBLING files
   FreeClipList();

#ifndef	NDEBUG
// **************************************************************
	PassNm2Mem();
// **************************************************************
#endif	// !NDEBUG

	DiagClose();

	FreeWrkStr();  // freework

	FreeAllocs(); // Ensure ALL memory (via DVGlobalAlloc) is FREED!

	return( msg.wParam ); // Returns the value from PostQuitMessage

}

#ifndef	NDEBUG
// **************************************************************
extern	void	SetGetName( LPVOID lpv );
extern	DWORD	GetSizeMxEdit( void );
extern	DWORD	GetSizeMxI( void );
extern	DWORD	GetSizeCC( void );
extern	DWORD	GetSizeCSL( void );
extern	DWORD	GetSizeGHE( void );
extern	DWORD	GetSizeSLP256( void );
extern	DWORD	GetSizeMxIBuf( void );
extern	DWORD	GetSizeLP2( void );
extern	DWORD	GetSizeLP16( void );
extern	DWORD	GetSizeLP256( void );
extern	DWORD	GetSizeMGI( void );

LPSTR	Cmp4Name( DWORD dws )
{
	LPSTR	lpn = 0;
	if( dws == sizeof(DIBINFO) )
		lpn = "DIBINFO";
	else if( dws == (MXFILNAM * gdwMaxFiles) )
		lpn = "File & Hist.";
	else if( dws == sizeof( DIBX ) )
		lpn = "DIBX";
	else if( dws == GetSizeMxEdit() )
		lpn = "MXEDITSTR";
	else if( dws == GetSizeMxI() )
		lpn = "Info Buf.";
	else if( dws == GetSizeCC() )
		lpn = "ChildCnt.";
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
	else if( dws == GetSizeCSL() )
		lpn = "GetStgLen";
	else if( dws == GetSizeGHE() )
		lpn = "GIFHDREXT";
	else if (dws == GetSizeMGI())
		lpn = "MXGIFINFO";
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
	else if( dws == GetSizeSLP256() )
		lpn = "LOGPAL256";
	else if( dws == GetSizeMxIBuf() )
		lpn = "MXIBUF";
	else if( dws == GetSizeLP2() )
		lpn = "LOGPAL2";
	else if( dws == GetSizeLP16() )
		lpn = "LOGPAL16";
	else if( dws == GetSizeLP256() )
		lpn = "LOGPAL256";

	return lpn;
}

void	PassNm2Mem( void )
{
	SetGetName( &Cmp4Name );
}


// **************************************************************
#endif	// !NDEBUG

// eof - End DvMain.c

