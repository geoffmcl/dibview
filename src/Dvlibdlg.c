
// DvLibDlg.c
#include	"dv.h"	// Inclusive include
//#include	"DvInfo.h"	// Access to DLG exchange BLOCK

#ifdef   SRCHLIBOK   // when all this is FIXED - it appears a mess at the moment

// For Browse button
// =================
extern	BOOL GetFN2Open( LPSTR lpFn, DWORD dwTitleID, LPSTR lpFilter );
// Image information (if required)
extern	int GetUCCnt( LPDIBINFO lpDIBInfo,
					 LPSTR lpb, int Cols, DWORD Size,
					 int Wid, int Height,
					 DWORD wBPP, LPSTR lpDIB );
extern	BOOL CenterWindow(HWND hwndChild, HWND hwndParent);
extern	DWORD	CalcDIBColors( LPSTR lpbi );

#ifndef   USEITHREAD

BOOL	fNeedCols = FALSE;	// Do NOT do Colour Used Count

typedef	struct	{
	// This MUST be at START of structure
	ISTR	IiBlk;	// COMMON Image information BLOCK
	// inherited above plus Private item
	LPSTR	lpOldLib;	// Old name
	LPSTR	lpFNBuf;	// Buffer for NEW
	DWORD	dwTitleID;	// "Find ...." Title
	LPSTR	lpFilter;	// File Mask / Template, like *.DLL
	char MLPTR MLPTR lpRServs;	// Requested services
	UINT	uiFNCnt;
} My_lParam;

typedef My_lParam MLPTR lpMy_LParam;

// Create an instance
static lpMy_LParam	lpP;

// TBD: Dialog Specific Close (on OK)
BOOL	Do_GL_Close( HWND hDlg )
{
	BOOL	flg;

	flg = FALSE;	// Do/done nothing
	if( lpP && lpP->lpFNBuf )
	{
		lpP->uiFNCnt = GetDlgItemText( hDlg,	// handle of dialog box
			IDC_EDIT1,	// identifier of control
			lpP->lpFNBuf,	// Address for results
			MAX_PATH );	// Lenght
		if( lpP->uiFNCnt )
		{
			if( lstrcmpi( lpP->lpOldLib, lpP->lpFNBuf ) )
			{
				flg = TRUE;
				lpP->IiBlk.fChanged = TRUE;
			}
		}
	}
	return flg;
}

// TBD: Control Specific reactions (to BUTTONs say)
BOOL	DlgGetFileName( HWND hDlg )
{
	BOOL	flg;
	flg = FALSE;
	if( lpP && lpP->lpFNBuf && lpP->lpFilter && *lpP->lpFilter )
	{
		// Do it per the FILTER
		*lpP->lpFNBuf = 0;	// Nothing to start
		flg = GetFN2Open( lpP->lpFNBuf,
			lpP->dwTitleID,
			lpP->lpFilter );
		if( flg &&
			( lstrcmpi( lpP->lpOldLib, lpP->lpFNBuf ) ) )
		{
			SetDlgItemText( hDlg,	// handle of dialog box
				IDC_EDIT1,	// identifier of control
				lpP->lpFNBuf );	// text to set
		}
	}
	return flg;
}
// TBD:SET Main Dialog Box Reference - in this case
// GETLIBDLGPROC

BOOL CALLBACK GETLIBDLGPROC( HWND, UINT, WPARAM, LPARAM );

static BOOL DoInit( HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL	flg;	// TRUE = Add focus, FALSE = Done focus -1 = ABORT
	char	buf[260];
	LPSTR	lpb;
	int		i;
	HWND	hWnd, hOwnr;

	lpb = &buf[0];
	// if DialogBoxParam used
	//void MLPTR	lpP;	// Pointer to INPUT params
	flg = TRUE;	// General DO NOTHING init
	if( hOwnr = GetWindow( hDlg, GW_OWNER ) )
	{
		CenterWindow( hDlg, hOwnr );
	}
	lpP = (lpMy_LParam)lParam;	// Get PASSED
	// TBD: Add Specific Dialog Initialisation
	// IDD_FINDLIB has
	// 1 - IDC_EDIT1	- Get new text
	// 2 - IDC_EDIT2	- A Description of hwat is wrong.
	// 2 - IDC_BUTTON1	- Do FIND FILE service
	lstrcpy( lpb, "Enter the Name of the new LIbrary. ie([D:][\\Folders\\]FileName.DLL of the NEW Library" );
	if( lpP && lpP->lpOldLib )
	{
		wsprintf( lpb, "Warning: Unable to load [%s] library!\r\n"\
			"Enter the NEW Library. ([[D:\\]Path\\]FileName.dll)\r\n",
			lpP->lpOldLib );
		if( lpP->lpFNBuf && lpP->lpFilter && *lpP->lpFilter )
		{
			lstrcat( lpb, "Use BROWSE button to Search.\r\n" );
		}
		if( lpP->lpRServs && *lpP->lpRServs )
		{
			lstrcat( lpb, "Missing [" );
			for( i = 0; ; i++ )
			{
				if( lpP->lpRServs[i] )
				{
					if( i )
					{
						lstrcat( lpb, ", " );
					}
					lstrcat( lpb, lpP->lpRServs[i] );
				}
				else
				{
					break;
				}
			}
			if( i )
			{
				lstrcat( lpb, "]!\r\n" );
			}
		}
	}
	SetDlgItemText( hDlg,	// handle of dialog box
		IDC_EDIT2,	// identifier of control
		lpb );	// text to set
	SetDlgItemText( hDlg,	// handle of dialog box
		IDC_EDIT1,	// identifier of control
		lpP->lpOldLib );	// text to set
	if( hWnd = GetDlgItem( hDlg,	// handle of dialog box
		IDC_EDIT1 ) )
	{
		if( SetFocus( hWnd ) )
			flg = FALSE;
	}

	return flg;
}

static void DoExit( HWND hDlg, BOOL flg )
{
	lpP = 0;	// Kill the instance
}

static BOOL DoCommand( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL	flg;	// TRUE = Add focus, FALSE = Done focus -1 = ABORT
   DWORD cmd = LOWORD(wParam);
	flg = FALSE;	// General DONE NOTHING

	switch( cmd )
	{
	case IDOK:
	case IDCANCEL:
		{
			if( cmd == IDOK )
			{
				flg = Do_GL_Close( hDlg );
			}
			DoExit( hDlg, flg );
			EndDialog(hDlg, flg );
		}
		break;

	case IDC_BUTTON1:
		flg = DlgGetFileName( hDlg );
		break;

	default:
		break;
	}
	return	flg;
}

BOOL CALLBACK GETLIBDLGPROC( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL	flg = FALSE;
	switch( message )
	{

	case WM_INITDIALOG:
		flg = DoInit( hDlg, wParam, lParam );
		break;
		
	case WM_COMMAND:
		flg = DoCommand( hDlg, wParam, lParam );
		break;
	}
	return( flg );
}


BOOL	DvAddInfoDlg( FARPROC lpProc, UINT	DlgID,
					 LPARAM lPar )
{
	BOOL	   flg;
	PIS   	lpi;
	LPSTR	   lpb, lpbits;
#ifndef	WIN32
	FARPROC	lpInfo;	// NOT required in WIN32!!!
#endif	/* !WIN32 */
	DWORD	   dwSize;

	flg = FALSE;
	lpi = (LPISTR) lPar;
	NULPISTR( lpi );			// Start as BLANK
	lpi->fChanged = FALSE;
	lpi->dwFlag = 0;
	lpi->hDIBInfo = 0;
	lpi->lpDIBInfo = 0;
	if( ( lpi->hMDI = GetCurrentMDIWnd() ) &&
		( lpi->hDIBInfo = GetWindowExtra( lpi->hMDI, WW_DIB_HINFO ) ) &&
		( lpi->lpDIBInfo = (LPDIBINFO) DVGlobalLock( lpi->hDIBInfo ) ) &&
		( lpi->hDIB = lpi->lpDIBInfo->hDIB ) )
	{

// FIX980430( GetObject( lpi->hBitmap, sizeof(BITMAP), &lpi->bm ) ) )
// FIX980504 - the BITMAP may not exist in case of BIG DIBs
		lpi->is_hBitmap = lpi->lpDIBInfo->hBitmap;
		lpi->cxDIB = lpi->lpDIBInfo->di_dwDIBWidth;
		lpi->cyDIB = lpi->lpDIBInfo->di_dwDIBHeight;
		if( lpb = DVGlobalLock( lpi->hDIB ) )  // LOCK DIB HANDLE
		{
			lpbits = FindDIBBits( lpb );
			lpi->iColors = DIBNumColors( lpb );
			lpi->iCalcCols = CalcDIBColors( lpb );
			if( ( lpi->wBPP = lpi->lpDIBInfo->di_dwDIBBits ) &&
				( dwSize = 
				(DWORD)( (WIDTHBYTES( lpi->cxDIB * lpi->wBPP )) *
				lpi->cyDIB ) ) &&
				fNeedCols )
			{
				lpi->iUsedCols = GetUCCnt( lpi->lpDIBInfo,
					lpbits, lpi->iColors,
					dwSize, lpi->cxDIB, lpi->cyDIB,
					lpi->wBPP, lpb );
			}
			DVGlobalUnlock( lpi->hDIB );  // UNLOCK DIB HANDLE
		}
		else
			lpi->iColors = 0;
		GetClientRect( lpi->hMDI, &lpi->rcClient );
		lpi->rcClip.left   = lpi->lpDIBInfo->rcClip.left;
		lpi->rcClip.top    = lpi->lpDIBInfo->rcClip.top;
		lpi->rcClip.right  = lpi->lpDIBInfo->rcClip.right;
		lpi->rcClip.bottom = lpi->lpDIBInfo->rcClip.bottom;
		if( IsRectEmpty( &lpi->rcClip ) )
		{
			lpi->rcClip.left = 0;
			lpi->rcClip.top = 0;
			lpi->rcClip.right = lpi->cxDIB;
			lpi->rcClip.bottom = lpi->cyDIB;
		}
	}	// Only IF information is available
	if( lpi->hMDI == 0 )
		lpi->hMDI = GetFocus();	// Get ACTIVE window - FRAME?
#ifdef	WIN32
	DialogBoxParam( ghDvInst,
		MAKEINTRESOURCE( DlgID ),	// Like IDD_FINDLIB
		lpi->hMDI,
		(DLGPROC)lpProc,
		(DWORD) lpi );
#else	/* !WIN32 */
	lpInfo = MakeProcInstance( lpProc, // Like GETLIBDLGPROC = Address
		ghDvInst );
//int DialogBoxParam(
//    HINSTANCE hInstance,	// handle to application instance
//    LPCTSTR lpTemplateName,	// identifies dialog box template
//    HWND hWndParent,	// handle to owner window
//    DLGPROC lpDialogFunc,	// pointer to dialog box procedure  
//    LPARAM dwInitParam ); 	// initialization value
	DialogBoxParam( ghDvInst,
		MAKEINTRESOURCE( DlgID ),	// Like IDD_FINDLIB
		lpi->hMDI,
		lpInfo,
		(DWORD) lpi );
	FreeProcInstance( lpInfo );
#endif	/* WIN32 y/n */
	if( lpi->fChanged )	// Something is changed
	{
		flg = TRUE;
	}
	if( lpi->hDIBInfo && lpi->lpDIBInfo )
		DVGlobalUnlock( lpi->hDIBInfo );
	return flg;
}

#endif   // ifndef   USEITHREAD
#endif   // ifdef   SRCHLIBOK   // when all this is FIXED - it appears a mess at the moment

// BOOL GetFN2Open( LPSTR lpFn, DWORD dwTitleID, LPSTR lpFilter )
BOOL GetNewLib( LPSTR lpOld,	// Old name
			   LPSTR lpFn,		// Buffer for NEW
			   DWORD dwTitleID,	// "Find ...." Title
			   LPSTR lpFilter,	// File Mask / Template, like *.DLL
			   char MLPTR MLPTR lpRServs )	// Requested services
{
	BOOL	flg = FALSE;
//#ifndef   USEITHREAD
#ifdef   SRCHLIBOK   // when all this is FIXED - it appears a mess at the moment
	My_lParam	mp;
	lpMy_LParam	pmp;

	flg = FALSE;
	pmp = &mp;
	pmp->lpOldLib  = lpOld;	// Old name
	pmp->lpFNBuf   = lpFn;	// Buffer for NEW
	pmp->dwTitleID = dwTitleID;	// "Find ...." Title
	pmp->lpFilter  = lpFilter;	// File Mask / Template, like *.DLL
	pmp->lpRServs  = lpRServs;	// Requested services

	flg = DvAddInfoDlg( GETLIBDLGPROC, // FARPROC lpProc
		IDD_FINDLIB,		// UINT	DlgID
		(LPARAM) pmp );
	// flg = GetFN2Open( lpFn, dwTitleID, lpFilter );
//#endif   // ifndef   USEITHREAD
#endif   // ifdef   SRCHLIBOK   // when all this is FIXED - it appears a mess at the moment

	return flg;
}
	

// eof - DvLibDlg.c
