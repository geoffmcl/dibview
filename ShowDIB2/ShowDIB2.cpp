
//
// ShowDIB2.cpp : Defines the entry point for the application.
//
#include "stdafx.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;	// current instance
//HWND     ghWnd;   // main window handle
#define  ghWnd hWndApp  // main window handle

TCHAR szTitle[MAX_LOADSTRING];			// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];	// The main window class
TCHAR szDate[] = __DATE__;
TCHAR szTime[] = __TIME__;
TCHAR gszbuf[264];
TCHAR gszbuf2[264];
TCHAR gszTmpBuf[1024];  // pad

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int, LPSTR);
BOOL           ProcessCommandLine(LPSTR lpCmdLine);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL CenterDialog( HWND hChild, HWND hParent );

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

     /* Initialize clip rectangle */
   SetRectEmpty(&rcClip);

   /* Save the pointer to the command line */
   strcpy(achFileName, lpCmdLine);

#if   0  // move to WM_CREATE
   gxScreen = GetSystemMetrics (SM_CXSCREEN) ;
   gyScreen = GetSystemMetrics (SM_CYSCREEN) ;
   // or SM_CXFULLSCREEN, SM_CYFULLSCREEN
   // but better SPI_GETWORKAREA
   SystemParametersInfo( SPI_GETWORKAREA, // system parameter to retrieve or set
      0, // depends on action to be taken
      &rcWorkArea,   // depends on action to be taken
      0 );           // user profile update option
   iCyHScroll = GetSystemMetrics (SM_CYHSCROLL);
   iCxVScroll = GetSystemMetrics (SM_CXVSCROLL);
   iCxBorder  = GetSystemMetrics (SM_CXEDGE);
   iCyBorder  = GetSystemMetrics (SM_CYEDGE);
#endif   // 0 - to WM_CREATE

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SHOWDIB2, szWindowClass, MAX_LOADSTRING);

	if( !MyRegisterClass(hInstance) )
      return -1;

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_SHOWDIB2);
	if( !( hAccelTable) )
		return -2;

	// Perform application initialization:
	if( !InitInstance (hInstance, nCmdShow, lpCmdLine) ) 
		return -3;

   sprtf( "App ShowDIB2 (0x%x) entering message loop..." GEOL, ghWnd );

	// Main message loop:
	while( GetMessage( &msg, NULL, 0, 0 ) ) 
	{
		if( !TranslateAccelerator(msg.hwnd, hAccelTable, &msg) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

   sprtf( "App ShowDIB2 (0x%x) exiting        ..." GEOL, ghWnd );

	return msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_SHOWDIB2);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_SHOWDIB2;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, LPSTR lpCmdLine)
{
//   HWND hWnd;

   if( !ProcessCommandLine(lpCmdLine) )
      return FALSE;

   hInst = hInstance; // Store instance handle in our global variable

   ghWnd = CreateWindow( szWindowClass,
      szTitle,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,
      0,
      CW_USEDEFAULT,
      0,
      NULL,
      NULL,
      hInstance,
      NULL );
   if (!ghWnd)
   {
      return FALSE;
   }

   ShowWindow(ghWnd, nCmdShow);
   UpdateWindow(ghWnd);

   return TRUE;
}

#define  DEF_FILE_NAME  "C:\\GTools\\Tools\\Dv32\\Inst\\TEMPB203.bmp"
//#define  DEF_FILE_NAME  "D:\\GTools\\Samples\\GV21\\File32\\GVTest\\TEMPB203.bmp"
//#define  DEF_FILE_NAME  "D:\\GTools\\Samples\\GV21\\File32\\GVTest\\TEMPB202.bmp"
//#define  DEF_FILE_NAME  "D:\\GTools\\Samples\\GV21\\File32\\GVTest\\TEMPB101.bmp"
//#define  DEF_FILE_NAME  "D:\\GTools\\Samples\\GV21\\File32\\GVTest\\TEMPB102.bmp"
BOOL ProcessCommandLine(LPSTR lpCmdLine)
{
   LPTSTR   lps = lpCmdLine;

   while( *lps && (*lps <= ' ') )lps++;

   if(*lps) // we have a command
   {
      strcpy(achFileName, lps);  // establish the FILE
   }
   else  // no COMMAND, so post one to myself
   {
      // could check AFTER create, and do say
//      PostMessage( ghWnd, WM_COMMAND, IDM_OPEN, 0 );
//      sprtf( "No command, so posted open..."MEOR );
      // or, for TEST
      strcpy(achFileName, DEF_FILE_NAME);
   }

   return TRUE;
}

LRESULT  sd_WM_COMMAND( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
   LRESULT lRes = 0;
	int wmId, wmEvent;

   wmId    = LOWORD(wParam); 
	wmEvent = HIWORD(wParam); 
	// Parse the menu selections:
	switch (wmId)
	{
	case IDM_ABOUT:
	   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
	   break;
//	case IDM_EXIT:
//	   DestroyWindow(hWnd);
	   break;
	default:
//	   lRes = DefWindowProc(hWnd, message, wParam, lParam);
      lRes = MenuCommand( hWnd, wParam );
      break;
   }
   return lRes;

}


LRESULT  sd_WM_WININICHANGE( HWND hWnd )
{
   LRESULT  lRes = 0;
   HMENU hMenu;
   HDC   hDC;
   UINT  ui;
   if( hMenu = GetMenu(hWnd) )
   {
      if( hDC = GetPrinterDC1() )
      {
         ui = ( RC_DIBTODEV &
            ( (GetDeviceCaps(hDC, RASTERCAPS)) ? MF_ENABLED : MF_GRAYED | MF_DISABLED ) );
         EnableMenuItem( hMenu,
            IDM_PRINT,
            ui );
         DeleteDC(hDC);
      }
   }
   return lRes;
}

LRESULT  sd_WM_CREATE( HWND hWnd, LPARAM lParam )
{
   LRESULT  lRes = 0;
   HDC      hDC;
   RECT     rc;

   GetClientRect( hWnd, &rc );   // get current SIZE of window in creation

   /* Allocate space for our logical palette */
   pLogPal = (NPLOGPALETTE) LocalAlloc( LMEM_FIXED,
                  ( sizeof(LOGPALETTE) +
                  ( sizeof(PALETTEENTRY) * 256 ) ) );
                  //(sizeof(PALETTEENTRY)*(MAXPALETTE))));
   if( !pLogPal )
   {
      ShowMemErr( hWnd );
      //LoadString(hInst, IDS_MEMLOW, lpBuffer, sizeof(lpBuffer));
      //MessageBox(hWnd, lpBuffer, NULL, MB_OK | MB_ICONHAND);
      PostQuitMessage(0);
      //break;
      lRes = 1;   // show FAILED
   }

   gxScreen = GetSystemMetrics (SM_CXSCREEN) ;
   gyScreen = GetSystemMetrics (SM_CYSCREEN) ;
   // or SM_CXFULLSCREEN, SM_CYFULLSCREEN
   // but better SPI_GETWORKAREA
   SystemParametersInfo( SPI_GETWORKAREA, // system parameter to retrieve or set
      0, // depends on action to be taken
      &rcWorkArea,   // depends on action to be taken
      0 );           // user profile update option
   iCyHScroll = GetSystemMetrics (SM_CYHSCROLL);
   iCxVScroll = GetSystemMetrics (SM_CXVSCROLL);
   iCxBorder  = GetSystemMetrics (SM_CXEDGE);
   iCyBorder  = GetSystemMetrics (SM_CYEDGE);
   // SM_CXBORDER
   iCxBorder  = GetSystemMetrics (SM_CXBORDER);
   iCyBorder  = GetSystemMetrics (SM_CYBORDER);
   hDC = GetDC(hWnd); 
   if(hDC)
   {
      iRasterCaps = GetDeviceCaps( hDC, RASTERCAPS );
      bPalSupport = ( iRasterCaps & RC_PALETTE );
      iLogPixelsX = GetDeviceCaps( hDC, LOGPIXELSX );
      iLogPixelsY = GetDeviceCaps( hDC, LOGPIXELSY );
      GetClientRect( GetDesktopWindow(), &rcMonitor );
      rcMonitor.left = GetDeviceCaps( hDC, HORZRES );
      rcMonitor.top  = GetDeviceCaps( hDC, VERTRES );
      ReleaseDC(hWnd,hDC);
   }

   if(( achFileName[0]   ) &&
      ( !InitDIB( hWnd ) ) )
   {
      PostQuitMessage( 3 );
   }
   return lRes;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   LRESULT  lRes = 0;
	switch (message) 
	{
   case WM_CREATE:
      if( sd_WM_CREATE( hWnd, lParam ) )
         break;
      /* fall through */
   case WM_WININICHANGE:
         sd_WM_WININICHANGE(hWnd);
         break;
   case WM_COMMAND:
         lRes = sd_WM_COMMAND(hWnd,message,wParam,lParam);
			break;

   // messages handled in Showdib.c - more from original MS sample
   // *****************************
	case WM_DESTROY:
   case WM_PALETTEISCHANGING:
   case WM_ACTIVATE:
   case WM_QUERYNEWPALETTE:
   case WM_PALETTECHANGED:
   case WM_RENDERALLFORMATS:
   case WM_RENDERFORMAT:
   case WM_TIMER:
   case WM_SIZE:
   case WM_KEYDOWN:
   case WM_KEYUP:
   case WM_VSCROLL:
   case WM_HSCROLL:
   case WM_LBUTTONDOWN:
   case WM_LBUTTONDBLCLK:
   case WM_INITMENU:
   case WM_INITMENUPOPUP:
   case WM_PAINT:
   // ***************************** new item should be added to BOTH case statements
      lRes = WndProc2( hWnd, message, wParam, lParam );
      break;

	default:
         lRes = DefWindowProc(hWnd, message, wParam, lParam);
         break;

   }  // switch(message)
   return lRes;
}

VOID  _trim(LPTSTR pc) 
{
   INT   ilen;
   ilen = strlen(pc);
   while(ilen)
   {
      ilen--;
      if( pc[ilen] > ' ' )
         break;
      pc[ilen] = 0;
   }
}

//		case WM_INITDIALOG:
LRESULT  Abt_WM_INITDIALOG( HWND hDlg )
{
   LPTSTR   lpb = gszbuf;

   sprintf(lpb, "On %s", szDate);
   _trim(lpb);
   strcat(lpb," ");
   strcat(lpb, szTime);
   _trim(lpb);
   SetDlgItemText(hDlg, IDC_LABEL1, lpb);

   GetModuleFileName( NULL, lpb, 256 );
   SetDlgItemText(hDlg, IDC_LABEL2, lpb);

   _getcwd( lpb, 256 );
   SetDlgItemText(hDlg, IDC_LABEL3, lpb);

   CenterDialog( hDlg, ghWnd );

   return TRUE;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
         return( Abt_WM_INITDIALOG( hDlg ) );
         break;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}

// **************** some utility functions **********
///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : CenterDialog
// Return type: BOOL 
// Arguments  : HWND hChild
//            : HWND hParent
// Description: 
// Added from grmLib.c - April, 2001 (FROM GMUtils.c - December, 1999)
// ===================================================================
//  FUNCTION: CenterDialog(HWND, HWND)
//	(was CenterWindow in GMUtils)
//  PURPOSE:  Center one window over another.
//  PARAMETERS:
//    hwndChild - The handle of the window to be centered.
//    hwndParent- The handle of the window to center on.
//  RETURN VALUE:
//    TRUE  - Success
//    FALSE - Failure
//  COMMENTS:
//    Dialog boxes take on the screen position that they were designed
//    at, which is not always appropriate. Centering the dialog over a
//    particular window usually results in a better position.
///////////////////////////////////////////////////////////////////////////////
BOOL CenterDialog( HWND hChild, HWND hParent )
{
	BOOL	bret = FALSE;
    RECT    rcChild, rcParent;
    int     cxChild, cyChild, cxParent, cyParent;
    int     cxScreen, cyScreen, xNew, yNew;
    HDC     hdc;
	HWND	hwndChild, hwndParent;

	hwndChild = hChild;
	hwndParent = hParent;
   if( !hParent && hChild )
      hwndParent = GetParent(hChild);

	if( hwndChild && hwndParent )
	{

		// Get the Height and Width of the child window
		if( GetWindowRect( hwndChild, &rcChild ) )
		{
			cxChild = rcChild.right - rcChild.left;
			cyChild = rcChild.bottom - rcChild.top;

			// Get the Height and Width of the parent window
			if( GetWindowRect( hwndParent, &rcParent ) )
			{
				cxParent = rcParent.right - rcParent.left;
				cyParent = rcParent.bottom - rcParent.top;

				// Get the display limits
				if( hdc = GetDC(hwndChild) )
				{
					cxScreen = GetDeviceCaps(hdc, HORZRES);
					cyScreen = GetDeviceCaps(hdc, VERTRES);
					ReleaseDC( hwndChild, hdc );

					// Calculate new X position,
					// then adjust for screen
					xNew = rcParent.left +
						( (cxParent - cxChild) / 2 );
					if( xNew < 0 )
					{
						xNew = 0;
					}
					else if( (xNew + cxChild) > cxScreen )
					{
						xNew = cxScreen - cxChild;
					}
					// Calculate new Y position,
					// then adjust for screen
					yNew = rcParent.top  +
						( (cyParent - cyChild) / 2 );
					if( yNew < 0 )
					{
						yNew = 0;
					}
					else if( (yNew + cyChild) > cyScreen )
					{
						yNew = cyScreen - cyChild;
					}

					// Set it, and return
					bret = SetWindowPos( hwndChild,
                        NULL,
                        xNew, yNew,
                        0, 0,
                        SWP_NOSIZE | SWP_NOZORDER );
				}
			}
		}
	}
	return bret;
}
// END CenterDialog(HWND,HWND) ADDED FROM GMUtils.c
// December, 1999
// =====================================


// **************************************************
// ShowDIB2.cpp
