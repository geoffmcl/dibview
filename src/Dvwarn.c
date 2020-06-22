
// ===============================================
//
//	File:  DvWarn.C
//
//	Purpose:  Contains handler for the
//		Dibview Warning Dialog box
//
//	Comments:
//
//	History:
//	Date			Reason
//	25 Nov 1997		Created
// ==============================================

#include	"Dv.h"	// All inclusive include

extern	BOOL CenterWindow( HWND, HWND );
	//PlaySound( "SystemQuestion" );
extern	void StartQuestionSound( void );

OPENFILENAME   g_of;	/* GLOBAL Open File Name Structure ... */

//BOOL MEXPORTS WARNINGDLG( HWND hDlg,
//						 UINT message,
//						 WPARAM wParam,
//						 LPARAM lParam );
INT_PTR CALLBACK WARNINGDLG(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);
BOOL	WarningTerm( HWND hDlg );
BOOL	WarningInit( HWND hDlg, LPARAM lParam );
BOOL  WarningBrowse( HWND hDlg );

// ==============================================
//
//	Function:   ShowWarning
//
//	Purpose:    Brings up WARNING dialog box.
//
//	Parms:      hWnd == Handle dialog box's parent.
//	History:
//	Date			Reason
//	25 Nov 1997		Created
// ==============================================

UINT ShowWarning( HWND hWnd, LPWARNSTR lpWS )
{
	UINT	ui;
#ifdef	WIN32
	ui = DialogBoxParam( ghDvInst,
		MAKEINTRESOURCE( IDD_DVERROR ),
		hWnd,
		WARNINGDLG,
		(LPARAM)lpWS );
#else	/* !WIN32 */
	FARPROC lpProcOpt;
	lpProcOpt = MakeProcInstance( WARNINGDLG, ghDvInst );
	ui = DialogBoxParam( ghDvInst,
		MAKEINTRESOURCE( IDD_DVERROR ),
		hWnd,
		lpProcOpt,
		(LPARAM)lpWS );
	FreeProcInstance( lpProcOpt );
#endif	/* WIN32 y/n */
	return ui;
}

// =============================================
//	WARNINGDLG( ... )
// =============================================
//BOOL MEXPORTS WARNINGDLG( HWND hDlg,
//						 UINT message,
//						 WPARAM wParam,
//						 LPARAM lParam )
INT_PTR CALLBACK WARNINGDLG(
  HWND hDlg,  // handle to dialog box
  UINT message,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	BOOL	bRet = FALSE;
	switch( message )
	{
	case WM_INITDIALOG:
		bRet = WarningInit( hDlg, lParam );
		break;

	case WM_COMMAND:
		switch( LOWORD( wParam ) )
		{
		case IDOK:
			WarningTerm( hDlg );	// Get values
			EndDialog( hDlg, IDYES );
			break;

		case IDCANCEL:
			EndDialog( hDlg, IDNO );
			break;

      case IDC_BROWSE:
         WarningBrowse( hDlg );
         break;

      case IDC_RETRY:
			EndDialog( hDlg, IDC_RETRY );
         break;
		}
		bRet = TRUE;
		break;

	case WM_DESTROY:
		REMOVE_PROP( hDlg, ATOM_LPWN );
		break;
	}

	return( bRet );
}


// ===========================================
//	WarningInit( ... )
// ===========================================

BOOL	WarningInit( HWND hDlg, LPARAM lParam )
{
	HWND		hWnd;
	BOOL		flg;
	LPWARNSTR	lpWS;
	UINT		ui;

	flg = TRUE;	// Default to all OK

	//PlaySound( "SystemQuestion" );
	StartQuestionSound();

	if( lpWS = (LPWARNSTR)lParam ) 
	{
		SET_PROP( hDlg, ATOM_LPWN, (HANDLE) lpWS );
		if( hWnd = GetWindow( hDlg, GW_OWNER ) )
		{
			CenterWindow( hDlg, hWnd );
		}

		if( lpWS->lpTitle )
		{
			SetWindowText( hDlg, lpWS->lpTitle );
		}

		if( hWnd = GetDlgItem( hDlg, IDC_EDIT1 ) )
		{
			SetWindowText( hWnd, lpWS->lpText );
		}

		if( lpWS->bJustOK )
		{
			if( hWnd = GetDlgItem( hDlg, IDOK ) )
			{
				ShowWindow( hWnd, SW_HIDE );
				EnableWindow( hWnd, FALSE );
			}
			if( hWnd = GetDlgItem( hDlg, IDCANCEL ) )
			{
				SetWindowText( hWnd, "OK" );
			}
		}

		if( !lpWS->bAddCheck )
		{
			if( hWnd = GetDlgItem( hDlg, IDC_CHECK1 ) )
			{
				ShowWindow( hWnd, SW_HIDE );
				EnableWindow( hWnd, FALSE );
			}
		}
		else  // we ADD the Check BUTTON
		{
			if( lpWS->bCheck )
				ui = BST_CHECKED;
			else
				ui = BST_UNCHECKED;
			CheckDlgButton( hDlg, IDC_CHECK1, ui );
		}

      if( !lpWS->bAddBrowse )
      {
			if( hWnd = GetDlgItem( hDlg, IDC_BROWSE ) )
			{
				ShowWindow( hWnd, SW_HIDE );
				EnableWindow( hWnd, FALSE );
			}
      }
			if( hWnd = GetDlgItem( hDlg, IDC_RETRY ) )
			{
				ShowWindow( hWnd, SW_HIDE );
				EnableWindow( hWnd, FALSE );
			}
	}
	else
	{
		flg = (BOOL)-1;
	}
	return( flg );

}

BOOL	WarningTerm( HWND hDlg )
{
	BOOL		flg;
    LPWARNSTR	lpWS;
	UINT		ui;

	if( lpWS = (LPWARNSTR) GET_PROP( hDlg, ATOM_LPWN ) )
	{
		if( lpWS->bAddCheck )
		{
			ui = IsDlgButtonChecked( hDlg, IDC_CHECK1 );
			if( ui == BST_CHECKED )
			{
				if( !lpWS->bCheck )
				{
					lpWS->bCheck = TRUE;
					lpWS->bChgCheck = TRUE;
				}
			}
			else
			{
				if( lpWS->bCheck )
				{
					lpWS->bCheck = FALSE;
					lpWS->bChgCheck = TRUE;
				}
			}
		}
	}
	flg = TRUE;
	return( flg );

}

static   TCHAR sszFilt[] = "Graphic Files.\0*.bmp;*.gif;*jpg\0All Files (*.*)\0*.*\0";

BOOL  GeneralBrowsing( HWND hDlg, LPTSTR pCFilt, LPTSTR pFile, LPTSTR pCPath )
{
   BOOL  bRet = FALSE;
   OPENFILENAME * pof = &g_of;	/* GLOBAL Open File Name Structure ... */

	{
      ZeroMemory( pof, sizeof(OPENFILENAME) );
//typedef struct tagOFN { 
      pof->lStructSize = sizeof(OPENFILENAME);
      pof->hwndOwner   = hDlg;
      pof->hInstance   = ghDvInst;
      if( pCFilt )
         pof->lpstrFilter = pCFilt;
      else
         pof->lpstrFilter = sszFilt;
//  LPTSTR        lpstrCustomFilter; 
//  DWORD         nMaxCustFilter; 
//  DWORD         nFilterIndex;
      pof->lpstrFile   = pFile;
      pof->nMaxFile    = 256;
//  LPTSTR        lpstrFileTitle; 
//  DWORD         nMaxFileTitle;
      if( pCPath )
         pof->lpstrInitialDir = pCPath;
      else
         pof->lpstrInitialDir = pFile;
//  LPCTSTR       lpstrTitle;
      pof->Flags           = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST; 
//  WORD          nFileOffset; 
//  WORD          nFileExtension; 
//  LPCTSTR       lpstrDefExt; 
//  LPARAM        lCustData; 
//  LPOFNHOOKPROC lpfnHook; 
//  LPCTSTR       lpTemplateName; 
//#if (_WIN32_WINNT >= 0x0500)
//  void *        pvReserved;
//  DWORD         dwReserved;
//  DWORD         FlagsEx;
//#endif // (_WIN32_WINNT >= 0x0500)
//} OPENFILENAME, *LPOPENFILENAME;

      if( GetOpenFileName( pof ) )  // initialization data
      {
         if( CheckIfFileExists( pof->lpstrFile ) )
         {
            bRet = TRUE;
         }
      }
   }
   return bRet;
}

BOOL  WarningBrowse( HWND hDlg )
{
   BOOL  bRet = FALSE;
   LPWARNSTR	lpWS;

	if( lpWS = (LPWARNSTR) GET_PROP( hDlg, ATOM_LPWN ) )
	{
      bRet = GeneralBrowsing( hDlg, sszFilt, lpWS->pszFile, lpWS->pszPath );
      if( bRet )  // initialization data
      {
         if( CheckIfFileExists( lpWS->pszFile ) )
         {
            HWND  hWnd;
   			if( hWnd = GetDlgItem( hDlg, IDC_RETRY ) )
	   		{
		   		ShowWindow( hWnd, SW_SHOW );
			   	EnableWindow( hWnd, TRUE );
			   }
         }
      }
   }
   return bRet;
}

// eof - DvWarn.c
