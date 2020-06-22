
/* ******************************************************************

      File:  DVINIT.C

   Purpose:  Routines called when the application first runs to do
             all the necessary initialization.

 Functions:  InitMyDIB
             InitInstance

  Comments:  

   History:   Date      Reason
             6/ 1/91    Created

   ***************************************************************** */

#include "dv.h"	/* All inclusive include ... */
// #define	ADDLIBCFG	/* Add the DLL Configuration items */
#ifdef	ADDLIBCFG
//#include	"wjpeglib.h"
//#include	"WJPEG32\WJPEGLIB.h"
#endif	/* ADDLIBCFG */

// external
//extern	char	szSearchDir[MXFILNAM+2];
extern	void	AddGlob( LPSTR );
extern	void	SRInitSch( LPSTR );
extern	void	MemIni( LPSTR );
extern	void	ReadIni( LPSTR );
extern	void	NewFileIni( LPSTR );
#ifndef  USENEWWINSIZE
extern	RECT	rIniSiz;
#endif // #ifndef  USENEWWINSIZE
#ifdef	TICKINI
extern	void	SetINI( HWND, BOOL );
#endif	/* TICKINI */
//extern	BOOL	fSavINI;
//extern	HGLOBAL	hFileList;		// Global MEMORY handle for FILES
//extern	WORD	wFilCnt;	/* Count of files in File List ... */
extern	void	InitFileList( HWND, HGLOBAL );
extern   VOID	InitFileList2( HWND hMain, PLIST_ENTRY pHead );
//extern	BOOL	fDvInited;
extern	BOOL    GetRootDir( LPSTR lpf );

extern	HBRUSH	ghbrBackground;

// GLOBAL
//HWND		ghMainWnd = 0;
//HINSTANCE	hDvInst = 0;
//int	iHPPI = 0;	// = GetDeviceCaps( hDC, LOGPIXELSX );
//int	iVPPI = 0;	// = GetDeviceCaps( hDC, LOGPIXELSY );
HANDLE	ghImage = 0;

char szFrameClass[] = "MyDIBWClass";   // Class name of frame window.
char szPalClass[]   = "MyDIBPalClass"; // Class name of palette windows.
// Programs MENU
//#ifdef	ADDCOLRF
//char szFrameMenu[]  = "TESTMENU";      // Main menu (in .RC file).
//#endif	// ADDCOLRF
char szPalMenu[]    = "PalMenu";       // Palette windows' menu (in .RC file).
//char szMyIcon[]     = "MyIcon";        // Icon name (in .RC file).
// FIX20051128 - changed the above ICON to IDI_ICON4
int	giScnWidth;		// = GetSystemMetrics( SM_CXSCREEN );
int	giScnHeight;	// = GetSystemMetrics( SM_CYSCREEN );

char cNxtCmd[MAX_PATH+16];

void	DeleteTools( void )
{

	if( ghbrBackground )
		DeleteObject( ghbrBackground );
	ghbrBackground = 0;

}
void	CreateTools( HINSTANCE hI )
{

	DeleteTools();
	ghbrBackground = CreateSolidBrush( RGB(0,0,255) );

}

//---------------------------------------------------------------------
//
// Function:   InitMyDIB
//
// Purpose:    Does initialization for DIBView.  Registers all the
//             classes we want, etc.  Called by first running instance
//             of DIBView, only (in DIBVIEW.C).
//
// Parms:      hInst == Handle to *this* instance of the app.
//             lpCmdLine => Active command line
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------
#define   USE_WM_COPYDATA

static COPYDATASTRUCT MyCDS;
//#define  CMD_LINE    1
//typedef struct tagMYREC {
//   DWORD n;
//   char  cmd[1];
//} MYREC;

// check and EXPAND any input files found, per the current work directory,
// whihc may be DIFFERENT to the RUNNING PARENT
VOID Pre_Process_Cmd( PTSTR pdest, PTSTR psrc )
{
   size_t len, i, off;
   TCHAR c, d;
   PTSTR pfile;

   lstrcpy(pdest,psrc);
   len = strlen(psrc);
   off = 0;
   for( i = 0; i < len; i++ )
   {
      c = psrc[i];
      if(( c == '-' )||( c == '/' )) {
         // deal with an OPTION
         pdest[off++] = c;
         i++;
         for( ; i < len; i++ )
         {
            c = psrc[i];
            pdest[off++] = c;
            if( c <= ' ' )
               break;
         }
      } else if ( c > ' ' ) {
         // deal WITH a FILE NAME
         pfile = &psrc[i];
         d = 0;
         if( c == '"' ) {
            d = c;
            pfile++;
         }
         i++;
         for( ; i < len; i++ )
         {
            c = psrc[i];
            if(d) {
               if( c == d )
                  break;
            } else if( c <= ' ' )
               break;
         }
         psrc[i] = 0;   // zero terminate
         if(d)
            pdest[off++] = d;
         if( !_fullpath( &pdest[off], pfile, 256 ) )
            strcpy( &pdest[off], pfile );
         off = strlen(pdest); // re-establish current LENGTH

         if(d)
            pdest[off++] = d;
      } else {
         pdest[off++] = c;
      }
   }
   pdest[off] = 0;
}

BOOL InitMyDIB( HANDLE hInst, LPSTR lpCmdLn )
{
	WNDCLASS  wc;
	HWND      hwnd;
	HGLOBAL		hg = NULL;
	LPSTR		lps = lpCmdLn;
   size_t   len;

	ghDvInst = hInst;
    // Win32 will always set hPrevInstance to NULL, so lets check
    // things a little closer. This is because we only want a single
    // version of this app to run at a time
    hwnd = FindWindow( szFrameClass, NULL );
	if( hwnd )
	{
		// We found another version of ourself. Lets defer to it:
		if( IsIconic( hwnd ) )
		{
            ShowWindow(hwnd, SW_RESTORE);
      }
#ifdef	WIN32
		SetForegroundWindow( hwnd );
#endif	// WIN32
#ifdef   USE_WM_COPYDATA
      if( lps && ((len = strlen(lps)) != 0) ) {
         PTSTR ptmp1 = gszTpBuf1;
         PTSTR ptmp2 = gszTpBuf2;
         lstrcpy(ptmp2,lps);  // copy so it can be modified
         Pre_Process_Cmd( ptmp1, ptmp2 );
         lps = ptmp1;
         len = strlen(lps);
         hg = GlobalAlloc( GPTR, (sizeof(MYREC) + len) );
         if(hg) {
            HRESULT res;
            PMYREC pmr = (PMYREC)hg;
            pmr->n = sizeof(MYREC);
            strcpy( pmr->cmd, lps );
            MyCDS.cbData = (sizeof(MYREC) + len);
            MyCDS.dwData = CMD_LINE;
            MyCDS.lpData = hg;
            res = SendMessage( hwnd,
                   WM_COPYDATA,
                   (WPARAM)(HWND) hInst,  // we have NO handle at this time!!!
                   (LPARAM) (LPVOID) &MyCDS );
            GlobalFree(hg);
         }
      }
#else // !#ifdef   USE_WM_COPYDATA
		if( lpCmdLn && lpCmdLn[0] &&
			( hg = GlobalAlloc( GPTR, (MAX_PATH+16) ) ) )
		{
			// This would probably FAIL - NO DATA SHARING!!!
			lps = (LPSTR)hg;
			strcpy( lps, lpCmdLn );
			SendMessage( hwnd, MYWM_RUNTWO, 0, (LPARAM)lps );
			GlobalFree( hg );
		}
		//else {
         // we could just
		   // also communicate this action that our 'twin'
		   // should now perform based on how the user tried to
		   // execute us. - so send a command line
			//SendMessage( hwnd, MYWM_RUNTWO, 0,
			//	(LPARAM)(LPSTR)&cNxtCmd[0] );
		//}
#endif // #ifdef   USE_WM_COPYDATA y/n
		return FALSE;
	}

//	ghDvInst = hInst;
   wc.style          = CS_HREDRAW |       // Class style(s).
		       CS_VREDRAW;
   wc.lpfnWndProc    = FRAMEWNDPROC;      // Function to retrieve messages for
                                          //    windows of this class.
   wc.cbClsExtra     = 0;                 // No per-class extra data.
   wc.cbWndExtra     = CBWNDEXTRA;        // Not USED in MAIN, but is used in children.
   wc.hInstance      = hInst;             // Application that owns the class.
//   wc.hIcon          = LoadIcon(hInst, szMyIcon); FIX20051128 changed name to IDI_ICON4
   wc.hIcon          = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON4));
   wc.hCursor        = LoadCursor (NULL, IDC_ARROW);
   //wc.hbrBackground  = (HBRUSH)(COLOR_APPWORKSPACE + 1);  // Use system color for window bgrnd
   wc.hbrBackground  = (HBRUSH)(COLOR_BACKGROUND + 1);  // Use system color for window bgrnd
#ifdef	ADDCOLRF
   // Name of menu resource in .RC file.
//	wc.lpszMenuName   = szFrameMenu;        // Name of menu resource in .RC file.
   // ID of menu resource in .RC file.
   wc.lpszMenuName   = MAKEINTRESOURCE( IDR_FRAMEMENU );
#else	// !ADDCOLRF
   wc.lpszMenuName   = MAKEINTRESOURCE( IDR_MENU1 );
#endif	// ADDCOLRF y/n
   wc.lpszClassName  = szFrameClass;       // Name used in call to CreateWindow.

      // Register the window class and return success/failure code.
   if( !RegisterClass(&wc) )
	   return FALSE;

   // Register the MDI child class

   wc.style         = 0;
   wc.lpfnWndProc   = CHILDWNDPROC;
   wc.lpszMenuName  = NULL;
   wc.cbWndExtra    = CBWNDEXTRA;	// Each CHILD the DibInfo HANDLE
   wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
   //wc.hbrBackground  = (HBRUSH)(COLOR_BACKGROUND + 1);  // Use system color for window bgrnd
   wc.lpszClassName = szMDIChild;
   wc.hIcon         = NULL;            // Icon -- draws part of DIB

   if( !RegisterClass(&wc) )
   {
	   UnregisterClass (szFrameClass, hInst);
	   return FALSE;
   }

      // Register the Palette window class.
   wc.style         = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc   = PALETTEWNDPROC;
#ifdef	FRMENU
   wc.lpszMenuName  = MAKEINTRESOURCE(IDR_PALMENU);
#else	/* !FRMENU */
   wc.lpszMenuName  = szPalMenu;	/* = "PALMENU" */
#endif	/* FRMENU y/n */
   wc.cbWndExtra    = PAL_CBWNDEXTRA;
   wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
   wc.lpszClassName = szPalClass;
   wc.hIcon         = LoadIcon (NULL, IDI_APPLICATION);

   if( !RegisterClass(&wc) )
   {
	   UnregisterClass (szFrameClass, hInst);
	   UnregisterClass (szMDIChild, hInst);
	   return FALSE;
   }

   ghImage = 0;
   ghImage = LoadImage( hInst,	// handle of the instance that contains the image
	   MAKEINTRESOURCE(IDB_BITMAP1),	// name or identifier of image
	   IMAGE_BITMAP,		// type of image
	   0,			// desired width
	   0,			// desired height
	   LR_DEFAULTCOLOR );	// load flags

   CreateTools( hInst );

   return TRUE;
}

//---------------------------------------------------------------------
//
// Function:   InitInstance
//
// Purpose:    Do necessary initialization for this instance of the
//             app.  Creates the main, overlapped window, sets the
//             global hInst, sets our working directory for FILE.C
//             routines.  Called from DIBView.C.
//
// Parms:      hInstance == Handle to this instance (passed to WinMain()).
//             nCmdShow  == How window should come up (passed to WinMain()).
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

BOOL InitInstance( HANDLE hInstance, 
                  int    nCmdShow )
{
	LPSTR	lpRoot;
	LPRECT	lprc;
	HDC		hDC;
    LPSTR   pTmp;
#ifdef  USENEWWINSIZE
   RECT     rc;
#endif   // #ifdef  USENEWWINSIZE

	// Save the instance handle in static variable, which will be used in
	// many subsequence calls from this application to Windows.
	//hInst = hInstance;
	ghDvInst = hInstance;

	// Gets the default runtime directory for file open/save
	lpRoot = &gszRDirName[0];
    pTmp = gszTmpBuf1;
	//_getcwd( lpRoot, MXDIR );	/* Setup defaults - as current */
	// if( dwL = GetModuleFileName( NULL, lpf, MAX_PATH ) )
	GetRootDir( lpRoot );	// Used to use GetModuleName
	// NOW ACTUALLY USES if( i = DVGetCwd( lpb, 256 ) )	// Get DIRECTORY

	HelpIni( lpRoot );
	///   _getcwd( szWDirName, MXDIR );
	lstrcpy( (LPSTR)gszWDirName, lpRoot );	/* And a COPY ... */
	lstrcpy( (LPSTR)gszSearchDir, lpRoot );
	AddGlob( (LPSTR)gszSearchDir );
	SRInitSch( lpRoot );
#ifdef	DIAGSAVEHOOK
	SRInitDiag( lpRoot );
#endif
    if (gszAppData[0])
        strcpy(pTmp, gszAppData);
    else
        strcpy(pTmp, lpRoot);
	MemIni( pTmp );
	ReadIni( NULL );  // FIX20200602: DibView.ini should be in %APPDATA%\DibView/
                      // FIX20080316 - INI was where RUNTIME is ... was ReadIni( lpRoot );
	NewFileIni( lpRoot );

   // Create a main window for this application instance.
#ifdef  USENEWWINSIZE
   rc.left   = CW_USEDEFAULT;
   rc.top    = CW_USEDEFAULT;
   rc.bottom = CW_USEDEFAULT;
   rc.right  = CW_USEDEFAULT;
   lprc = &rc;
#else // !#ifdef  USENEWWINSIZE
	lprc = &rIniSiz;
#endif   // #ifdef  USENEWWINSIZE y/n
	
	ghMainWnd = CreateWindow(szFrameClass,	// See RegisterClass() call.
		"DibView",		// Text for window title bar.
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,	// Window style.
		lprc->left,		// x top left position
		lprc->top,		// y top left position
		lprc->right,	// cx width
		lprc->bottom,	// cy height
		NULL,			// Overlapped windows have no parent.
		NULL,			// Use the window class menu.
		hInstance,		// This instance owns this window.
		NULL );			// Pointer (not used).               

	// If window could not be created, return "failure"
	if( !ghMainWnd )
	{
		return( FALSE );	// Kill it all now
	}

	if( hDC = GetDC( ghMainWnd ) )
	{
		giHPPI = GetDeviceCaps( hDC, LOGPIXELSX );
		giVPPI = GetDeviceCaps( hDC, LOGPIXELSY );
		ReleaseDC( ghMainWnd, hDC );
	}

	// GetSystemMetrics
	giScnWidth  = GetSystemMetrics( SM_CXSCREEN );
	giScnHeight = GetSystemMetrics( SM_CYSCREEN );

#ifdef	TICKINI
	//	{ &gfSavINI, &fChgSI, IDO_SAVEINI, SI_Default, 0, 0, 0 },
	SetINI( ghMainWnd, gfSavINI );	/* Set the MENU  */
#endif	/* TICKINI */

   // Make the window visible; update its client area; and return "success"
#ifdef   USENEWWINSIZE
   if( gbGotWP )
      SetWindowPlacement( ghMainWnd, &g_sWP );
   else
      ShowWindow( ghMainWnd, nCmdShow);
#else // !#ifdef   USENEWWINSIZE
   ShowWindow( ghMainWnd, nCmdShow );	// Show the window
#endif // #ifdef   USENEWWINSIZE y/n
   UpdateWindow( ghMainWnd );			// Sends WM_PAINT message

   {
      PLIST_ENTRY pHead = &gsFileList; // = W.w_sWL1;
      if( !IsListEmpty(pHead) )
      {
   	   InitFileList2( ghMainWnd, pHead );
      }
   }

//   fDvInited = TRUE;	// Initial size messages done ...

   return( TRUE );	// Continue...      
}

/* eof - End DvInit.c */
