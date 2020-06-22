
// ===============================================
//
//	File:  DVOPT4.C
//
//	Purpose:  Contains handler for the
//		Most Recent File Dialog box
//
//	Comments:
//
//	History:
//	Date			Reason
//	25 Nov 1997		Created
// ==============================================
#include	"dv.h"	// Single, all inclusive include

extern	HWND	   ChkFileOpen( LPSTR lpf );  // enmerate children seeing if loaded
// NEW - Nov 2000
extern   void	   AddToHistList( LPTSTR lpf );
extern   DWORD    SetFileMRU( HWND hwnd, PLIST_ENTRY pHead );
// NEW - Dec 2000
extern   BOOL  GeneralBrowsing( HWND hDlg, LPTSTR pCFilt, LPTSTR pFile, LPTSTR pCPath );
extern	BOOL CenterWindow(HWND hwndChild, HWND hwndParent);


//BOOL MEXPORTS OPTION4DLG( HWND hDlg,
//						 UINT message,
//						 WPARAM wParam,
//						 LPARAM lParam );
INT_PTR CALLBACK OPTION4DLG(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);
BOOL	Option4Term( HWND hDlg );
BOOL	Option4Delete( HWND hDlg );
BOOL	Option4DelAll( HWND hDlg );
BOOL	Option4Restore( HWND hDlg );
BOOL	Option4Browse( HWND hDlg );
BOOL	Option4Init( HWND hDlg, LPARAM lParam );
BOOL	FillHwndBox( HWND hDlg, LPFLST lpFL, HWND hWnd );
BOOL	FillListBox( HWND hDlg, LPFLST lpFL );

// ==============================================
//
//	Function:   ShowOption4
//
//	Purpose:    Brings up option4 Most Recent File
//				list dialog box on IDM_OPTION4.
//	Parms:      hWnd == Handle dialog box's parent.
//	History:
//	Date			Reason
//	25 Nov 1997		Created
// ==============================================
#ifdef	WIN32

int ShowOption4( HWND hWnd, LPFLST lpfl )
{
	int		i;
	i = DialogBoxParam( ghDvInst,
		MAKEINTRESOURCE( IDD_MRFDIALOG ),
		hWnd,
		OPTION4DLG,
		(LPARAM)lpfl );
	return i;
}

#else	/* !WIN32 */

int ShowOption4( HWND hWnd, LPFLST lpfl )
{
	int		i;
	FARPROC lpProcOpt;
	lpProcOpt = MakeProcInstance( OPTION4DLG, ghDvInst );
	i = DialogBoxParam( ghDvInst,
		MAKEINTRESOURCE( IDD_MRFDIALOG ),
		hWnd,
		lpProcOpt,
		(LPARAM)lpfl );
	FreeProcInstance( lpProcOpt );
	return i;
}

#endif	/* WIn32 y/n */

// Deal with MRF List
//#ifdef   USELLIST

void Do_IDM_OPTION4( HWND hWnd )
{
	int		i;
//	FLST	   FLst;
//	LPFLST	lpfl = &FLst;
	LPFLST	lpfl = MALLOC( sizeof(FLST) );
   if( !lpfl )
      return;

   ZeroMemory( lpfl, sizeof(FLST) );
	lpfl->dwFLCnt = gdwFilCnt;	// Count 1
   lpfl->pHead   = &gsFileList;
	lpfl->lpFLGChg = &gfChgFil;	// Pointer to active change flag
	lpfl->lpdwGCnt = &gdwFilCnt;	// Pointer to active count
	lpfl->lpdwGOff = &gdwFilOff;	// Pointer to active offset
	i = ShowOption4( hWnd, lpfl );
	if( ( i == TRUE      ) &&
       ( lpfl->bFLChg ) )
   {
      PLE   pHead, pNext, pNew;
      PMWL  pmwl;
      pHead = &gsFileList;
      i = 0;
      Traverse_List( pHead, pNext )
      {
         pmwl = (PMWL)pNext;
         if( pmwl->wl_dwFlag & flg_ToDelete )   // marked to be DELETED
         {
            i++;
            pNew = pNext->Blink;    // start from here for traverse
            RemoveEntryList( pNext );
            AddToHistList( &pmwl->wl_szFile[0] );  // add this to HISTORY
            MFREE( pNext );   // NOTE: pNext is NOW invalid
            if( gdwFilCnt )
               gdwFilCnt--;
            pNext = pNew; // BACK one so that travers will continue to NEXT
         }
      }
      if(i)
      {
				// SET CHANGE for INI writting
            gfChgFil = TRUE;
            SetFileMRU( ghFrameWnd, pHead );
      }
   }
   MFREE(lpfl);
}


VOID  SetSelString( HWND hDlg, PMWL pmwl )
{
   LPTSTR   lpf  = &pmwl->wl_szFile[0];
   LPTSTR   lptmp = GetTmp2();
   *lptmp = 0;
   if( lpf && *lpf )
   {
      WIN32_FIND_DATA   fd;
      if( IsValidFile( lpf, &fd ) & IS_FILE )
      {
         LARGE_INTEGER li;
         li.LowPart  = fd.nFileSizeLow;
         li.HighPart = fd.nFileSizeHigh;
         wsprintf( lptmp,
                  "%s - %s bytes at %s",
                  lpf,
                  GetI64Stg( &li ),
                  GetFDTStg( &fd.ftLastWriteTime ) );
      }
      else
      {
         wsprintf( lptmp,
                  "%s - *** IS NOT A VALID FILE! ***",
                  lpf );
      }
   }

   // set the "selected" string
   SetDlgItemText( hDlg, IDC_EDIT1, lptmp );

}


VOID  Option4Notify( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
   if( HIWORD(wParam) == LBN_SELCHANGE )
   {
      HWND  hWnd = (HWND)lParam;
      // Parameters
      // wParam
      // The low-order word is the list box identifier.
      // The high-order word is the notification message.
      // lParam
      // Handle to the list box.
      LRESULT  lRes = SendMessage( hWnd,  // handle to destination window
            LB_GETITEMDATA,         // message to send
            SendMessage( hWnd, LB_GETCURSEL, 0, 0 ),    // item index
            0 );     // not used; must be zero
      if( lRes != LB_ERR )
      {
         PMWL     pmwl = (PMWL)lRes;
         SetSelString( hDlg, pmwl );
      }
   }
}

// =======================================================
//	OPTION4DLG( ... ) from IDM_OPTIONS4 using IDD_MRFDIALOG
// =======================================================
//BOOL MEXPORTS OPTION4DLG( HWND hDlg,
//						 UINT message,
//						 WPARAM wParam,
//						 LPARAM lParam )
INT_PTR CALLBACK OPTION4DLG(
  HWND hDlg,  // handle to dialog box
  UINT message,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	BOOL	bRet = FALSE;
   DWORD wCmd;
	switch( message )
	{
	case WM_INITDIALOG:
		bRet = Option4Init( hDlg, lParam );
		break;

	case WM_COMMAND:
      wCmd = LOWORD( wParam );
		switch( wCmd )
		{
		case IDOK:
			if( Option4Term( hDlg ) )	// Get values
            EndDialog( hDlg, TRUE);
         else
            EndDialog( hDlg, FALSE);
			break;

		case IDCANCEL:
			EndDialog( hDlg, FALSE );
			break;

		case IDC_MRFDELETE:
			Option4Delete( hDlg );
			break;

		case IDC_MRFDELALL:
			Option4DelAll( hDlg );
			break;

		case IDC_MRFRESTORE:
			Option4Restore( hDlg );
			break;

      case IDC_MRFBROWSE:
         Option4Browse( hDlg );
         break;

      case IDC_MRFLIST:
         Option4Notify( hDlg, wParam, lParam );
         break;

		}
		bRet = TRUE;
		break;

	case WM_DESTROY:
		REMOVE_PROP( hDlg, ATOM_LPFL );
		break;
	}

	return( bRet );
}

BOOL  AskUser( HWND hDlg, LPTSTR lpf )
{
   BOOL  flg = FALSE;
   int   ir;
   LPTSTR lptmp = GetTmp3();

   {
		wsprintf( lptmp,
			"WARNING: This file\r\n"
			"%s\r\n"
			"is presently OPEN!\r\n"
			"To DELETE it will close this window\r\n"
			"Continue with DELETE?",
			lpf );
		ir = MessageBox( hDlg,
			lptmp,
			"DELETE MRF FILE",
			MB_ICONINFORMATION | MB_YESNO );

		if( ir == IDYES )
			flg = TRUE;
   }

   return flg;
}

BOOL	ChkDelete4( HWND hDlg, PMWL pmwl )
{
	BOOL	   flg = TRUE;
   HWND     hMDI;
   LPTSTR   lpf = &pmwl->wl_szFile[0];
	if( hMDI = ChkFileOpen( lpf ) )
	{
      flg = AskUser( hDlg, lpf );
	}
	return flg;
}

BOOL	ChkDelete( HWND hDlg, LPSTR lpf )
{
	BOOL	flg = TRUE;
   HWND  hMDI;
	if( hMDI = ChkFileOpen( lpf ) )
	{
      flg = AskUser( hDlg, lpf );
	}
	return flg;
}

//#ifdef   USELLIST

BOOL	FillHwndBox( HWND hDlg, LPFLST lpFL, HWND hWnd )
{
	BOOL	   flg = FALSE;	// Default to FAILED
	LRESULT	lRes;
	LPTSTR	lpf;
	DWORD	   wc1, wc2;
	PLE      pHead, pNext;
   PMWL     pmwl, pmwl1;

   wc2 = wc1 = 0;
   pmwl1 = 0;
	if( ( hDlg && lpFL && hWnd ) &&
       ( pHead = lpFL->pHead  ) )
	{
		lRes = SendMessage( hWnd,
			LB_RESETCONTENT,
			0, 0 );
      // traverse add files NOT marked flg_ToDelete
      Traverse_List( pHead, pNext )
		{
         wc2++;
         pmwl = (PMWL)pNext;
         if( pmwl1 == 0 )
            pmwl1 = pmwl;
         lpf  = &pmwl->wl_szFile[0];
			if( !( pmwl->wl_dwFlag & flg_ToDelete ) )
			{
            // add to list
            lRes = SendMessage( hWnd,
					LB_ADDSTRING,
					0,
					(LPARAM)lpf );
				if( ( lRes == LB_ERR      ) ||
					 ( lRes == LB_ERRSPACE ) )
				{
					break;
				}
				else
				{
					// Associate a POINTER
					lRes = SendMessage( hWnd,
						LB_SETITEMDATA,
						(WPARAM)lRes,
						(LPARAM)pNext );
               wc1++;
				}
			}
		}
      if( wc1 )
      {
		   lRes = SendMessage( hWnd,
				LB_SETCURSEL,
				0,
				0 );
         if( pmwl1 && ( lRes != LB_ERR ) )
            SetSelString( hDlg, pmwl1 );

         flg = TRUE;
      }
	}

	return flg;
}

BOOL	Option4Restore( HWND hDlg )
{
	BOOL	   flg = FALSE;
   LPFLST   lpFL;
   HWND     hWnd;
   PMWL     pmwl;
   PLE      pHead, pNext;
   DWORD    dwc = 0;

	if( ( lpFL = (LPFLST) GET_PROP( hDlg, ATOM_LPFL ) ) &&
       ( pHead = lpFL->pHead                         ) &&
		 ( hWnd = GetDlgItem( hDlg, IDC_MRFLIST )      ) )
	{
         Traverse_List( pHead, pNext )
         {
               pmwl = (PMWL)pNext;
               if( pmwl->wl_dwFlag & flg_ToDelete )   // if DELETE FLAG
               {
                  dwc++;   // count a change
                  pmwl->wl_dwFlag &= ~( flg_ToDelete );  // remove it
               }
         }

         if(dwc)
         {
            FillHwndBox( hDlg, lpFL, hWnd );
            flg = TRUE;
         }
   }

   return flg;
}

BOOL	Option4DelAll( HWND hDlg )
{
	BOOL	   flg = FALSE;
   LPFLST   lpFL;
   HWND     hWnd;
   PMWL     pmwl;
   PLE      pHead, pNext;
   DWORD    dwc1, dwc2;
   LPTSTR   lpo = 0;
   HWND     hMDI;

   dwc1 = dwc2 = 0;
	if( ( lpFL = (LPFLST) GET_PROP( hDlg, ATOM_LPFL ) ) &&
       ( pHead = lpFL->pHead                         ) &&
		 ( hWnd = GetDlgItem( hDlg, IDC_MRFLIST )      ) )
	{
         Traverse_List( pHead, pNext )
         {
               pmwl = (PMWL)pNext;
               if( !( pmwl->wl_dwFlag & flg_ToDelete ) ) // if NOT already so marked
               {
                  LPTSTR   lpf;
                  lpf = &pmwl->wl_szFile[0];
                 	if( hMDI = ChkFileOpen( lpf ) )
                  {
                     if( lpo == 0 )
                        lpo = lpf;
                     dwc1++;
                  }
                  dwc2++;
                  pmwl->wl_dwFlag |= flg_MarkDel;  // temporary mark delete
               }
         }
         if( dwc1 )
         {
            int      ir;
            LPTSTR   lptmp = GetTmp1();
		      wsprintf( lptmp,
			      "WARNING: File\r\n"
			      "%s\r\n"
			      "and %d others, are presently OPEN!\r\n"
			      "DELETE ALL will close all child windows\r\n"
			      "Continue with DELETE ALL?",
			      lpo,
               dwc1 );
		      ir = MessageBox( hDlg,
			      lptmp,
			      "DELETE MRF FILE",
			      MB_ICONINFORMATION | MB_YESNO );
            if( ir == IDYES )
            {
			      flg = TRUE;
            }
         }
         else
         {
			      flg = TRUE;
         }
         dwc1 = 0;
         if( dwc2 )
         {
            Traverse_List( pHead, pNext )
            {
               pmwl = (PMWL)pNext;
               if( pmwl->wl_dwFlag & flg_MarkDel )    // if a temporary mark
               {
                  pmwl->wl_dwFlag &= ~( flg_MarkDel );   // remove temporary
                  if( flg )
                  {
                     dwc1++;
                     pmwl->wl_dwFlag |= flg_ToDelete; // add the delete flag
                  }
               }
            }
         }
         if( dwc1 )
         {
            FillHwndBox( hDlg, lpFL, hWnd );
         }
   }
   return flg;
}

BOOL	Option4Delete( HWND hDlg )
{
	BOOL	   flg = FALSE;
   LPFLST   lpFL;
   HWND     hWnd;
   PMWL     pmwl;
   LRESULT  lRet;
   PLE      pHead, pNext;
   DWORD    dwc1, dwc2;
   LPTSTR   lpo = 0;

   dwc1 = dwc2 = 0;
	if( ( lpFL = (LPFLST) GET_PROP( hDlg, ATOM_LPFL ) ) &&
       ( pHead = lpFL->pHead                         ) &&
		 ( hWnd = GetDlgItem( hDlg, IDC_MRFLIST )      ) )
	{
      // retrieve the index of the currently selected item,
      // if any, in a single-selection list box. 
		lRet = SendMessage( hWnd,
			LB_GETCURSEL,
			0,
			0 );
		if( lRet != LB_ERR )
		{
         lRet = SendMessage( hWnd,  // handle to destination window
            LB_GETITEMDATA,         // message to send
            lRet,    // item index
            0 );     // not used; must be zero
         if( lRet != LB_ERR )
         {
            Traverse_List( pHead, pNext )
            {
               if( pNext == (PLE)lRet )
               {
                  pmwl = (PMWL)pNext;
                  if( !( pmwl->wl_dwFlag & flg_ToDelete ) ) // if NOT already deleted
                  {
                     LPTSTR   lpf;
                     lpf = &pmwl->wl_szFile[0];
                 	   if( ChkDelete4( hDlg, pmwl ) )
                     {
                        pmwl->wl_dwFlag |= flg_ToDelete;  // add marking
                        dwc1++;
                     }
                  }
                  break;   // marked selected as deleted - only removed on OK
               }
            }
            if( dwc1 )
            {
               FillHwndBox( hDlg, lpFL, hWnd );
            }
         }
      }
   }
   return flg;
}

BOOL	Option4Browse( HWND hDlg )
{
	BOOL	   flg = FALSE;
   LPFLST   lpFL;
   HWND     hWnd;
   PMWL     pmwl;
   LRESULT  lRet;
   PLE      pHead, pNext;
   DWORD    dwc1, dwc2;
   LPTSTR   lpo;
   LPTSTR   lpf;
   PWARNSTR pw;
   PWIN32_FIND_DATA   pfd;
   LPTSTR   lpmsg;

   pw  = (PWARNSTR)MALLOC( (sizeof(WARNSTR) + sizeof(WIN32_FIND_DATA) + 1024) );
   if( !pw )
      return FALSE;
   // set other pointers
   pfd = (PWIN32_FIND_DATA) ((PWARNSTR)pw + 1);
   lpmsg = (LPTSTR)((PWIN32_FIND_DATA)pfd + 1);
   pmwl = 0;
   dwc1 = dwc2 = 0;
   lpf = lpo = 0;
	if( ( lpFL = (LPFLST) GET_PROP( hDlg, ATOM_LPFL ) ) &&
       ( pHead = lpFL->pHead                         ) &&
		 ( hWnd = GetDlgItem( hDlg, IDC_MRFLIST )      ) )
	{
      // retrieve the index of the currently selected item,
      // if any, in a single-selection list box. 
		lRet = SendMessage( hWnd,
			LB_GETCURSEL,
			0,
			0 );
		if( lRet != LB_ERR )
		{
         lRet = SendMessage( hWnd,  // handle to destination window
            LB_GETITEMDATA,         // message to send
            lRet,    // item index
            0 );     // not used; must be zero
         if( lRet != LB_ERR )
         {
            Traverse_List( pHead, pNext )
            {
               if( pNext == (PLE)lRet )
               {
                  pmwl = (PMWL)pNext;
                  lpo  = &pmwl->wl_szFile[0];
                  lpf  = &lpFL->szFilBuf[0];
                  strcpy( lpf, lpo );
                  break;
               }
               pmwl = 0;
            }
         }
      }
   }
   if( ( pmwl ) &&
       ( lpf  ) &&
       ( &lpf ) )
   {
      if( GeneralBrowsing( hDlg, NULL, lpf, NULL ) )
      {
         if( strcmpi( lpo, lpf ) )
         {
            UINT     ir;
            if( IsValidFile( lpf, pfd ) & IS_FILE )
            {
               LARGE_INTEGER li;
               li.LowPart  = pfd->nFileSizeLow;
               li.HighPart = pfd->nFileSizeHigh;
               wsprintf(lpmsg,
                  "CHANGE MRU LIST"MEOR
                  "Do you want to change from"MEOR
                  "File [%s] to"MEOR
                  "File [%s] %s %s?",
                  lpo, lpf,
                  GetI64Stg( &li ),
                  GetFDTStg( &pfd->ftLastWriteTime ) );
               ZeroMemory( pw, sizeof(WARNSTR) );
               pw->lpText  = lpmsg;
               pw->lpTitle = "AMEND MRU LIST";
               ir = ShowWarning( hDlg, pw );
               if( ir == IDYES )
               {
                  // if the EXISTING file is also VALID
                  if( IsValidFile( lpo, pfd ) & IS_FILE )
                     AddToHistList( lpo );   // put this entry into HISTORY
                  // else dump it completely

                  strcpy( lpo, lpf );
                  lpFL->bFLChg++;   // mark a CHANGE
                  FillHwndBox( hDlg, lpFL, hWnd );
               }
            }
         }
      }
   }

   MFREE(pw);

   return flg;
}

BOOL	Option4Term( HWND hDlg )
{
	BOOL	   flg = FALSE;
   LPFLST	lpFL;
   PLE      pHead, pNext;
   PMWL     pmwl;
	if( ( lpFL = (LPFLST) GET_PROP( hDlg, ATOM_LPFL ) ) &&
       ( pHead = lpFL->pHead                         ) )
	{
      Traverse_List( pHead, pNext )
      {
         pmwl = (PMWL)pNext;
         if( pmwl->wl_dwFlag & flg_ToDelete )   // if this still has the temporary mark
         {
			   lpFL->bFLChg++;   // count a CHANGES
         }
      }

		if( lpFL->bFLChg )   // any CHANGE
         flg = TRUE;
   }
	return( flg );
}


BOOL	FillListBox( HWND hDlg, LPFLST lpFL )
{
	BOOL	flg;
	HWND	hWnd;

	flg = FALSE;	// Default to FAILED

	if( hDlg && lpFL &&
		( hWnd = GetDlgItem( hDlg, IDC_MRFLIST ) ) )
	{
		flg = FillHwndBox( hDlg, lpFL, hWnd );
	}
	return( flg );
}

BOOL	Option4Init( HWND hDlg, LPARAM lParam )
{
	BOOL	   flg;
	LPFLST	lpFL;
   HWND     hWnd;
	flg = TRUE;	// Default to all OK

	if( lpFL = (LPFLST)lParam ) 
	{
		SET_PROP( hDlg, ATOM_LPFL, (HANDLE) lpFL );
		if( hWnd = GetWindow( hDlg, GW_OWNER ) )
		{
			CenterWindow( hDlg, hWnd );
		}
		if( !FillListBox( hDlg, lpFL ) )
		{
			REMOVE_PROP( hDlg, ATOM_LPFL );
			flg = (BOOL)-1;
		}
	}
	else
	{
		flg = (BOOL)-1;
	}
	return( flg );

}


// eof - DvOpt4.c
