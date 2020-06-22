

// DvOpen.c

#include	"dv.h"
#include	<direct.h>	// For _getcwd - This should ALWAYS be there

extern	int	DVGetCwd( LPSTR, DWORD );

OPENFILENAME	_gof;	// Open File Name Structure
char	szGTitle[MAX_PATH+4];
char	szGFile[MAX_PATH+4];
char	szGFileTitle[MAX_PATH];

// Here the user has to supply MORE of the details
// ===============================================
BOOL GetFN2Open( LPSTR lpFn, DWORD dwTitleID, LPSTR lpFilter )
{
	int				i;
	BOOL			rflag;
	DWORD			flags;
	int				fpDlgFunc;
	INFOSTRUCT		ExpInfo;
//	char			szWDirName[MAX_PATH];
	LPSTR			pADir;
	char			szFilter[MAX_PATH];
	LPSTR			lpFilt;
	LPSTR			lpTitle, lpFile, lpFT;
   LPOPENFILENAME	pgof = &_gof;	// Open File Name Structure

	// Init locals
   ZeroMemory( pgof, sizeof(OPENFILENAME) );
	szGFile[0] = '\0';
	lpFile = &szGFile[0];

	gszDirName[0] = 0;
	szGTitle[0] = 0;
	lpTitle = &szGTitle[0];
	szGFileTitle[0] = 0;
	lpFT = &szGFileTitle[0];

	szFilter[0] = 0;

	rflag = FALSE;
	fpDlgFunc = 0;
	lpFilt = 0;
	ExpInfo.is_szName[0] = 0;
	ExpInfo.is_hdlDib = 0;	// NO Dib handle
	ExpInfo.is_hPal = 0;	// NO Palette
	ExpInfo.is_hBitmap = 0;	// NO Bitmap
	// Initialize the OPENFILENAME members
	pADir = &gszDirName[0];
	DVGetCwd( pADir, 256 ); // sizeof(szWDirName) );
	i = 0;
	if( dwTitleID )
	{
		i = LoadString( ghDvInst, dwTitleID, lpTitle, MAX_PATH );
	}
	if( i == 0 )
	{
		lstrcpy( lpTitle, "Find File to Open" );
		i = lstrlen( lpTitle );
	}

//
//		LoadString( ghDvInst, IDS_FILEOPEN, szTemplate, sizeof (szTemplate));
	if( ( lpFilt = lpFilter ) &&
		( *lpFilt ) )
	{
		// We appear to have a filter - Use it;
	}
	else
	{
		LPSTR	lpf;
		lpFilt = szFilter;
		lstrcpy( lpFilt, "All Files (*.*)" );
		lpf = lpFilt + lstrlen( lpFilt ) + 1;
		lstrcpy( lpf, "*.*" );
		lpf = lpf + lstrlen( lpf ) + 1;
		*lpf = 0;
	}

	flags =	OFN_EXPLORER | OFN_HIDEREADONLY;

	pgof->lStructSize       = sizeof (OPENFILENAME);
	pgof->hwndOwner         = GetFocus ();
	pgof->hInstance         = ghDvInst;
	pgof->lpstrFilter       = lpFilt;	// Default or USES
	//pgof->lpstrCustomFilter = NULL;
	//pgof->nMaxCustFilter    = 0L;
	pgof->nFilterIndex      = (DWORD) 1;
	pgof->lpstrFile         = lpFile;
	pgof->nMaxFile          = MAX_PATH;
	pgof->lpstrFileTitle    = lpFT;
	pgof->nMaxFileTitle     = MAX_PATH;
	pgof->lpstrInitialDir   = pADir;	/* Should be SAVE(W) or OPEN(R) Directory */
	pgof->lpstrTitle        = lpTitle;
	pgof->Flags             = flags;
	//gof.nFileOffset       = 0;
	//gof.nFileExtension    = 0;
	//gof.lpstrDefExt       = NULL;
	//gof.lpfnHook			= NULL;
	//gof.lpTemplateName	= NULL;
	// Call the GetOpenFilename function
//	if( wIDString == IDS_OPENDLG )	/* Open - Load File ... */
//	{
		if( GetOpenFileName (pgof) )
		{
			lstrcpy( lpFn, pgof->lpstrFile );
			rflag = TRUE;
		}
		else
		{
#ifdef	WIN32
			flags = CommDlgExtendedError();
			switch( flags )
			{
			case CDERR_FINDRESFAILURE:
			case CDERR_NOHINSTANCE:
			case CDERR_INITIALIZATION:
			case CDERR_NOHOOK:
			case CDERR_LOCKRESFAILURE:
			case CDERR_NOTEMPLATE:
			case CDERR_LOADRESFAILURE:
			case CDERR_STRUCTSIZE:
			case CDERR_LOADSTRFAILURE:
			case FNERR_BUFFERTOOSMALL:
			case CDERR_MEMALLOCFAILURE:
			case FNERR_INVALIDFILENAME:
			case CDERR_MEMLOCKFAILURE:
			case FNERR_SUBCLASSFAILURE:
//				pExtErr = &szComErr[0];	// = "Common Error";
				break;
			default:
//				pExtErr = &szUnkErr[0];	// = "Unknown Error";
				break;
			}
#else	// !WIN32
//			pExtErr = &szUnkErr[0];	// = "Unknown Error";
#endif	// WIN32 y/n
 			rflag = FALSE;
		}
//	}

	if( fpDlgFunc )
		FreeProcInstance( fpDlgFunc );
	fpDlgFunc = 0;
	// NOTE: This COULD be used INSTEAD of RE-LOADING the file!!!
	// ==========================================================
	if( ExpInfo.is_hdlDib )	// IF Dib handle
		DVGlobalFree( ExpInfo.is_hdlDib );
	ExpInfo.is_hdlDib = 0;
	if( ExpInfo.is_hBitmap )
		DeleteObject( ExpInfo.is_hBitmap );
	ExpInfo.is_hBitmap = 0;
	if( ExpInfo.is_hPal )
		DeleteObject( ExpInfo.is_hPal );
	ExpInfo.is_hPal = 0;

	return( rflag );

}


// DECEMBER, 2000 - ANOTHER TRY AT A SOFISTICATED OPEN FILE DIALOG BOX
// ===================================================================
#ifdef	_WIN98_
/******************************************************************************
   Main_CreateImageList
******************************************************************************/
DWORD_PTR Main_CreateImageList2(BOOL fLarge)
{
   DWORD_PTR hImageList;
   SHFILEINFO  sfi;
   //get the system image list
   hImageList = SHGetFileInfo( TEXT("C:\\"),
                                 0,
                                 &sfi,
                                 sizeof(SHFILEINFO),
                                 SHGFI_SYSICONINDEX | (fLarge ? 0 : SHGFI_SMALLICON) );
   return hImageList;
}
#endif // #ifdef	_WIN98_

INT_PTR  opn_INIT( HWND hDlg, LPARAM lParam )
{
   INT_PTR  iRet = TRUE;
   HWND     hWnd;

   if( hWnd = GetDlgItem(hDlg, IDOK) )
      EnableWindow( hWnd, FALSE );


   return iRet;
}
INT_PTR  opn_COMMAND( HWND hDlg, WPARAM wParam )
{
   INT_PTR  iRet = TRUE;
   DWORD    cmd  = LOWORD(wParam);
   switch(cmd)
   {
   case IDOK:
      EndDialog(hDlg, TRUE);
      break;
   case IDCANCEL:
      EndDialog(hDlg, FALSE);
      break;
   }
   return iRet;
}

INT_PTR CALLBACK OPENDLGPROC(
  HWND hDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
   INT_PTR  iRet = 0;
   switch(uMsg)
   {
   case WM_INITDIALOG:
      iRet = opn_INIT( hDlg, lParam );
      break;
   case WM_COMMAND:
      iRet = opn_COMMAND( hDlg, wParam );
      break;
   case WM_PAINT:
      break;
   case WM_DESTROY:
      break;
   }
   return iRet;
}

VOID  Do_IDM_OPEN2( HWND hWnd )
{
   PRDIB   prd = (PRDIB)MALLOC( sizeof(RDIB) ); 
   if(prd)
   {
      int   iret;
      ZeroMemory(prd, sizeof(RDIB));
      iret = DialogBoxParam( ghDvInst,
         MAKEINTRESOURCE(IDD_OPEN),
         hWnd,
         OPENDLGPROC,
         (LPARAM)prd );
      MFREE(prd);
   }
   else
      chkme( "C:ERROR: No MEMORY!!!"MEOR );

}

// ===================================================================
// eof - DvOpen.c
