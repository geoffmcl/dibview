
/* 
 * Copyright 1995,6 Informatique Rouge. All rights reserved.
 *
 *  MODULE   : SRSearch.c / DvSrch.c
 *
 *  PURPOSE  : Handle IDM_OPENAS message
 *
 *  FUNCTIONS:
 *
 *  HISTORY  :
 *  27 Jan 96  Created                                  Geoff R. McLane
 *
 */
#pragma message ( "Public SRSearch, SEARCHDLGPROC." )

#include	"dv.h"	/* General inclusive include ... */
#define  USELLIST2   // FIX20001210 - switch to using a LINKED LIST

#define     ATOM_MRU     0x11a
#define     ATOM_SRCH4   0x119
#define     ATOM_SRCH    0x118

// external items
extern   void  SRError( LPSTR lpm, LPSTR lpt );
extern   void  SRSetupPath( LPSTR, LPSTR, LPSTR );
extern   int   DVGetCwd( LPSTR lpb, DWORD siz );
extern	BOOL  CenterWindow(HWND hwndChild, HWND hwndParent);
extern   PMWL  Add2LListTail( PLIST_ENTRY pHead, LPTSTR lpb );
extern   PMWL  Add2LListHead( PLIST_ENTRY pHead, LPTSTR lpb );
extern   PMWL  Add2LListTail4( PLIST_ENTRY pHead, LPTSTR lpb, PBYTE pb, DWORD dwsz );
extern   int	DVGetFilePath( LPTSTR lpfull, LPTSTR lppath );
extern   int	DVGetFileTitle( LPTSTR lpfull, LPTSTR lptitle );
extern   void	DVGetFPath( LPTSTR pfull, LPTSTR pdisk, LPTSTR ppath, LPTSTR pname, LPTSTR pext );
extern   PMWL	AddToFileList4( PRDIB prd );
extern   INT_PTR  OpenMRUFile( HWND hWnd, PMWL pmwl );

// global items
LPTSTR   pRErr1 = TEXT("ERROR: String could NOT be added to list box!");
LPTSTR   pRErr2 = TEXT("ERROR: List box is FULL! No more space!");
LPTSTR   pRErr5 = TEXT("ERROR: Unable to create DIALOG BOX!");
LPTSTR   pRErr6 = TEXT("ERROR: Data Item could not be attached to String!");
LPTSTR   pRGlob   = TEXT("*.*");
BOOL	   fShwAFil = TRUE;

TCHAR    szDefSch[] = "TEMPACT1.LST";
//TCHAR    szSchFil[MXFILNAM+8];
//LPTSTR   pRCurSch = &szSchFil[0];
// local
void	AddGlob( LPTSTR lps );
BOOL  doFindDir( HWND hDlg, LPTSTR lpf );

// global functions
void	SRInitSch( LPSTR lpcd )
{
	/* set the DEFAULT drive, directory, and search mask */
	SRSetupPath( &gszSchFil[0], lpcd, &szDefSch[0] );
}

#ifdef  USELLIST2   // FIX20001210 - switch to using a LINKED LIST

LIST_ENTRY  sDirList = { &sDirList, &sDirList };

DWORD CopyLList( PLIST_ENTRY pSrc, PLIST_ENTRY pDst )
{
   DWORD       cnt = 0;
   PLIST_ENTRY pNext;
   PMWL        pmwls, pmwld;

   Traverse_List( pSrc, pNext )
   {
      pmwls = (PMWL)pNext;
      if( pmwld = MALLOC(sizeof(MWL)) )
      {
         memcpy(pmwld,pmwls,sizeof(MWL));
         InsertTailList( pDst, (PLIST_ENTRY) pmwld );
         cnt++;
      }
      else
      {
         chkme( "NO MALLOC MEMORY!!! %d bytes", sizeof(MWL) );
      }
   }
   return cnt;
}

DWORD GetLListIndex( PLIST_ENTRY pHead, PLIST_ENTRY pItem )
{
   DWORD       cnt = 0;
   PLIST_ENTRY pNext;
   Traverse_List( pHead, pNext )
   {
      if( pNext == pItem )
         break;
      cnt++;
   }
   return cnt;
}

typedef  struct tagSRCHSTR {
   BOOL        s_bOK;      // if all OK
   BOOL        s_bDoIter;  // interate into sub-directories
   DWORD       s_dwMasks;  // count of masks
   DWORD       s_dwFinds;  // count of finds
   HWND        s_hCombo;   // handle of the COMBOBOX
   HWND        s_hEdit;    // handle of the EDIT box
   HWND        s_hList;    // list box handle
   BOOL        s_bCombo;   // TRUE if using COMBO BOX, else EDIT control active
   DWORD       s_dwCount;  // count of finds
   DWORD       s_dwDirCnt; // count of directories
   DWORD       s_dwIndex;  // currect selected index
   LIST_ENTRY  s_sDirList; // directories found in search
   LIST_ENTRY  s_sFilList; // files found in search
   LIST_ENTRY  s_sMasks;   // copy of search masks (and any new entries)
   LIST_ENTRY  s_sFinds;   // copy of previous find list

   PMWL        s_pmwlMask; // pointer to SELECTED mask when OK pressed
   PMWL        s_pmwlFile;
   
   WIN32_FIND_DATA   s_sFD;   // find data

   TCHAR       s_szCurr[260];
   TCHAR       s_szNew[260];
   TCHAR       s_szPath[260];
   TCHAR       s_szTitle[260];
   TCHAR       s_szExt[260];

}SRCHSTR, * PSS;

// COMBOBOX
BOOL	FillComboList( PSS pss, HWND hList, PLIST_ENTRY pHead )
{
	BOOL	   fOk;
//   LPARAM   lPar;   // My own DATA item per string
   LRESULT  lRes;
	PLIST_ENTRY pNext;
	LPSTR	   lps;
	int		i;
   PMWL     pmwl;

	fOk  = FALSE;
//	lPar = 0;
	if( ( hList           ) &&
		 ( IsWindow(hList) ) )
	{
		SendMessage( hList, CB_RESETCONTENT, 0, 0 );
      Traverse_List( pHead, pNext )
      {
         pmwl = (PMWL)pNext;
            if( pmwl->wl_dwLen )
				{
               lps = &pmwl->wl_szFile[0];
					if( i = lstrlen(lps) )
					{
						{
								lRes = SendMessage( hList, CB_ADDSTRING, 0,
									(LPARAM)lps );
								if( ( lRes == CB_ERR      ) ||
									 ( lRes == CB_ERRSPACE ) )
								{
									break;
								}
								else
								{
									SendMessage( hList,
										CB_SETITEMDATA,
										(WPARAM)lRes,
                              (LPARAM)pmwl );
								}
						}
//                  lPar++;
					}
				}
      }

      // NOTE: On this message the text in the edit control of
      // the combo box changes to reflect the new selection,
		SendMessage( hList,
         CB_SETCURSEL,
         pss->s_dwIndex,    // item index
         0 );  // not used; must be zero
      SendMessage( hList,  // handle to destination window
         CB_GETLBTEXT,  // message to send
         pss->s_dwIndex,    // item index
         (LPARAM)&pss->s_szCurr[0] ); // receives string (LPCSTR)

	}

	return fOk;

}

//	Init the SEARCH dialog box. ie handle WM_INITDIALOG
BOOL	Do_Srch_Init( HWND hDlg, WPARAM wPara, LPARAM lParam )
{
	HWND	hEd, hLst;
	HWND	hAct;
	UINT	uiCtl;
   PSS   pss = (PSS)lParam;
   LPSTR lpf = &pss->s_szCurr[0];

   SET_PROP( hDlg, ATOM_SRCH, pss );
//	fGotMask = FALSE;	/* We have NO MASK yet ... */

   CenterWindow( hDlg, GetWindow( hDlg, GW_OWNER ) );

//	if( !fDnSrch )	/* If this is the first ... */
//	{
//		fDnSrch = TRUE;
//		if( gszSearchDir[0] == 0 )
//		{
//			GetSystemDirectory( &gszSearchDir[0], MXFILNAM );
//			AddGlob( &gszSearchDir[0] );
//			gfChgMask = TRUE;
//		}
//	}
   pss->s_dwIndex = 0;

	pss->s_hEdit  = hEd  = GetDlgItem( hDlg, IDM_SELECT );
   pss->s_hCombo = hLst = GetDlgItem( hDlg, IDC_COMBO1 );
//	if( ( gdwMaskCnt                  ) &&
//       ( !IsListEmpty( &gsMaskList ) ) )
	if( ( pss->s_dwMasks == 0            ) ||
       ( IsListEmpty( &pss->s_sMasks ) ) )
   {
      DVGetCwd( lpf, 256 );
      AddGlob(  lpf );
      if( Add2LListTail( &pss->s_sMasks, lpf ) )
         pss->s_dwMasks++;
   }

	if( ( pss->s_dwMasks               ) &&
       ( !IsListEmpty( &pss->s_sMasks ) ) )
   {
		uiCtl = IDC_COMBO1;
      pss->s_bCombo = TRUE;
		if( hEd )
		{
			EnableWindow( hEd, FALSE    );
			ShowWindow(   hEd, SW_HIDE  );
		}
		if( hLst )
		{
			EnableWindow( hLst, TRUE    );
			ShowWindow(   hLst, SW_SHOW );
			hAct = hLst;
		}

		FillComboList( pss, hAct, &pss->s_sMasks );

	}
	else
	{
      DVGetCwd( lpf, 256 );
      AddGlob(  lpf );
      pss->s_bCombo = FALSE;
		uiCtl = IDM_SELECT;
		if( hEd )
		{
			EnableWindow( hEd, TRUE     );
			ShowWindow(   hEd, SW_SHOW  );
			hAct = hEd;
		}
		if( hLst )
		{
			EnableWindow( hLst, FALSE   );
			ShowWindow(   hLst, SW_HIDE );
		}

		SetDlgItemText( hDlg, uiCtl, lpf );

	}


	CheckDlgButton( hDlg, IDD_AUTOWRAP, pss->s_bDoIter );    // gfDoIter

	return TRUE;	/* and return TRUE to display DIALOG ... */

}

INT_PTR  Do_Srch_Browse( HWND hDlg )
{
   INT_PTR  iRet = FALSE;
   PSS   pss;
   if( pss = (PSS)GET_PROP(hDlg, ATOM_SRCH) )
   {
      LPTSTR   lpf, lpn;
      lpn = &pss->s_szNew[0];
      lpf = &pss->s_szCurr[0];
      strcpy( lpn, lpf );
      iRet = doFindDir( hDlg, lpn );
      if( iRet )
      {
         PMWL  pmwl;
         if( pmwl = Add2LListTail( &pss->s_sMasks, lpn ) )
         {
            pss->s_dwMasks++;
            pss->s_dwIndex = GetLListIndex( &pss->s_sMasks, (PLIST_ENTRY)pmwl );
            FillComboList( pss, pss->s_hCombo, &pss->s_sMasks );
            iRet = TRUE;
         }
      }
   }
   return iRet;
}

BOOL  MatchFile( LPTSTR pMask, LPTSTR pFile )
{
   BOOL  bRet = FALSE;
   INT   i, j, ii, jj;
   INT   c, d, d1, d2;

   if( ( i = strlen(pMask) ) &&
       ( j = strlen(pFile) ) )
   {
      d1 = d2 = jj = 0;
      for( ii = 0; ii < i; ii++ )
      {
         c = toupper(pMask[ii]);
         d = toupper(pFile[jj]);
         if( c == '*' )
         {
            jj++;
            d = 0;
            for( ; jj < j; jj++ )
            {
               d = pFile[jj];
               if( d == '.' )
               {
                  //jj++;
                  d2++;
                  break;
               }
            }
            if( jj >= j )
            {
               // end of file name 2
               if( (ii + 1) == i )  // if BOTH are finished
                  bRet = TRUE;
               break;
            }
         }
         else if( c == '?' )
         {
            jj++;
         }
         else
         {
            if( c != d )
            {
               break;
            }
            else if( c == '.' )
            {
               d1++;
               if( !d2 )
                  d2++;
            }
            jj++;
         }
            if( jj >= j )
            {
               // end of file name
               if( (ii + 1) == i )  // both finishing
                  bRet = TRUE;
               break;
            }
      }
   }

   return bRet;
}

DWORD  Do_Srch_Files( HWND hDlg, PSS pss, BOOL bIt )
{
   HANDLE            hFind;
   LPTSTR            lpf, lpp, lpn, lpt;
   PWIN32_FIND_DATA  pfd;
   INT               ilen;
//   PLIST_ENTRY       pNext;
   DWORD             dwcnt = 0;
   if( pss )
   {
      lpf   = &pss->s_szCurr[0];
      lpt   = &pss->s_szTitle[0];
      lpn   = &pss->s_szNew[0];
      lpp   = &pss->s_szPath[0];

      pfd   = &pss->s_sFD;
      // get PATH and TITLE
      ilen  = DVGetFilePath( lpf, lpp );
      //DVGetFileTitle( lpf, &pss->s_szTitle[0] );
      // kill the actual title, and add "*.*"
      lpf[ilen] = 0;
      AddGlob(lpf);
      hFind = FindFirstFile( lpf, pfd );
      if( VFH( hFind ) )   // ie != INVALID_HANDLE_VALUE
      {
         do
         {
            if( pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            {
               if( pfd->cFileName[0] != '.' )
               {
                  strcpy( lpn, lpp );  // add current PATH
                  strcat( lpn, &pfd->cFileName[0] );  // append new
                  strcat( lpn, "\\" ); // and terminate the FOLDER
                  if( Add2LListTail( &pss->s_sDirList, lpn ) )
                  {
                     pss->s_dwDirCnt++;
                  }
               }
            }
            else if( MatchFile( lpt, &pfd->cFileName[0] ) )
            {
               strcpy( lpn, lpp );
               strcat( lpn, &pfd->cFileName[0] );
               if( Add2LListTail4( &pss->s_sFilList, lpn,
                  (PBYTE)pfd, sizeof(WIN32_FIND_DATA) ) )
               {
                  //pss->s_dwCount++;  // count of finds
                  dwcnt++;
               }
            }
         } while( FindNextFile( hFind, pfd ) );

         FindClose(hFind);

      }
   }

   return( dwcnt );

}

DWORD  Do_Srch_Find( HWND hDlg, PSS pss, BOOL bIt )
{
//   HANDLE            hFind;
   LPTSTR            lpf, lpt;
//   LPTSTR            lpp, lpn;
//   INT               ilen;
   PLIST_ENTRY       pNext;
   PMWL              pmwl;
   DWORD             dwcnt = 0;
   if( pss )
   {
      lpf   = &pss->s_szCurr[0];
      lpt   = &pss->s_szTitle[0];
      //lpn   = &pss->s_szNew[0];
      //lpp   = &pss->s_szPath[0];
      // get PATH and TITLE
      //ilen  = DVGetFilePath( lpf, lpp );
      DVGetFileTitle( lpf, lpt );
      // kill the actual title, and add "*.*"
      //lpf[ilen] = 0;
      //AddGlob(lpf);
      dwcnt = Do_Srch_Files( hDlg, pss, bIt );
      if( bIt && pss->s_dwDirCnt )
      {
         PLIST_ENTRY pHead = &pss->s_sDirList;
         Traverse_List( pHead, pNext )
         {
            pmwl = (PMWL)pNext;
            strcpy( lpf, &pmwl->wl_szFile[0] ); // copy in this DIRECTORY
            strcat( lpf, lpt ); // and our MASK
            // later we MUST accept MULTIPLE masks!!!
            dwcnt += Do_Srch_Files( hDlg, pss, bIt );
         }
      }
   }

   if( dwcnt )
   {
      pss->s_dwCount += dwcnt;
      if( pmwl = pss->s_pmwlMask )
      {
         pmwl->wl_dwFlag |= flg_SrchOK;   // mark as a GOOD "search" mask
      }
   }

   return dwcnt;

}


INT_PTR  Do_Srch_OK( HWND hDlg )
{
   INT_PTR  iRet = FALSE;
   BOOL     bIt;
   PSS      pss;
   HWND     hList;
   LRESULT  lRes;
   PMWL     pmwl;
   PLIST_ENTRY pHead, pNext;
   LPTSTR   lpf;
   if( pss = (PSS)GET_PROP(hDlg, ATOM_SRCH) )
   {
      pss->s_bDoIter = bIt = IsDlgButtonChecked( hDlg, IDD_AUTOWRAP );
      hList = pss->s_hCombo;
      lpf   = &pss->s_szCurr[0];
//      lpn   = &pss->s_szNew[0];
//      lpp   = &pss->s_szPath[0];
		lRes = SendMessage( hList,
         CB_GETCURSEL,
         0,    // not used
         0 );  // not used; must be zero
      if( lRes == CB_ERR )
      {
         // hmmm, not a SELECTED item, so
         if( GetWindowText( hList, lpf, 256 ) )
         {
            // got the EDIT text - it is probably a NEW item
            pHead = &pss->s_sMasks;
            Traverse_List( pHead, pNext )
            {
               pmwl = (PMWL)pNext;
               if( lstrcmpi( lpf, &pmwl->wl_szFile[0] ) == 0 )
               {
                  iRet = TRUE;
                  break;
               }
            }
            if( !iRet )
            {
               // not already a MASK in the list, whihc is to be expected
               pmwl = Add2LListTail( pHead, lpf ); // then ADD this mask NOW
            }

            iRet = 0;
            if( pmwl )
            {
               pss->s_pmwlMask  = pmwl;   // this is the MASK being used
               pmwl->wl_dwFlag |= flg_SrchUsed; // set it as the MASK being used
               // this is the associated PMWL item
               iRet = Do_Srch_Find( hDlg, pss, bIt );
               goto Exit_OK;
            }
         }
      }
      if( lRes != CB_ERR )
      {
         pss->s_dwIndex = lRes;  // this is the SELECTED item
         lRes = SendMessage( hList,     // handle to destination window
            CB_GETLBTEXT,        // message to send
            pss->s_dwIndex,      // item index
            (LPARAM) lpf );      // receives string (LPCSTR)
         if( lRes != CB_ERR )
         {
            lRes = SendMessage( hList, // handle to destination window
               CB_GETITEMDATA,         // message to send
               (WPARAM)pss->s_dwIndex, // item index
               0 );  // not used; must be zero
            if( lRes != CB_ERR )
            {
               pmwl = (PMWL)lRes;
               if( lstrcmpi( lpf, &pss->s_pmwlMask->wl_szFile[0] ) )
               {
                  // yeak! NOT a currently listed item mask!!!
                  pHead = &pss->s_sMasks;
                  if( pmwl = Add2LListTail( pHead, lpf ) )
                  {
                     pss->s_pmwlMask  = pmwl;
                     pmwl->wl_dwFlag |= flg_SrchUsed; // this is the MASK being used
                     // this is the associated PMWL item
                     iRet = Do_Srch_Find( hDlg, pss, bIt );
                  }
               }
               else
               {
                  // is the SAME as it was
                  pss->s_pmwlMask  = pmwl;
                  pmwl->wl_dwFlag |= flg_SrchUsed; // this is the MASK being used
                  // this is the associated PMWL item
                  iRet = Do_Srch_Find( hDlg, pss, bIt );
               }
            }
         }
      }
      if( lRes == CB_ERR )
      {
         MessageBox( hDlg, "No item appears selected. Or some other error. Choose again!",
            "ERROR: NO SELECTION",
            (MB_ICONINFORMATION | MB_OK) );
      }
   }

Exit_OK:

   lpf = 0;

   return iRet;
}

INT_PTR  Do_Srch_Command( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR  flg = FALSE;
   DWORD    cmd = LOWORD(wParam);
	switch( cmd )
	{
	case IDM_FIND:    // browse button
      flg = Do_Srch_Browse(hDlg);   //	doFindDir( hDlg, lpf );
		break;

  	case IDD_OK:
		if( flg = Do_Srch_OK( hDlg ) )
      {
         EndDialog( hDlg, flg );
      }
      flg = TRUE;
		break;

	case IDD_CANCEL:
   	EndDialog( hDlg, FALSE ) ;
      flg = TRUE;
		break;
   }	/* case of cmd (WPARAM) */

   return flg;

}

INT_PTR CALLBACK SEARCHDLGPROC(
    HWND hDlg,  // handle to dialog box
    UINT uMsg,     // message
    WPARAM wParam, // first message parameter
    LPARAM lParam ) // second message parameter
{
	INT_PTR	flg = FALSE;
	switch (uMsg)
	{

	case WM_INITDIALOG:
		flg = Do_Srch_Init( hDlg, wParam, lParam );
		break;

	case WM_CLOSE:	/* 0x0010 */
      EndDialog( hDlg, FALSE ) ;
      flg = TRUE;
		break;

	case WM_COMMAND:
      flg = Do_Srch_Command( hDlg, wParam, lParam );
		break;

   case WM_DESTROY:
      REMOVE_PROP( hDlg, ATOM_SRCH );
      flg = TRUE;
      break;


	}	/* case of uMsg */

	return( flg );

} /* end of SEARCHDLGPROC() */

DWORD Do_Srch4_List( HWND hDlg, PSS pss )
{
   DWORD    flg = 0;
   PLIST_ENTRY pHead, pNext;
   PMWL     pmwl;
   PWIN32_FIND_DATA  pfd;
   LPTSTR   lps;
   LRESULT  lRes;
   LARGE_INTEGER li;
   DWORD    dwcnt = 0;

   pHead = &pss->s_sFilList;
   lps   = &pss->s_szCurr[0];
   SendMessage( pss->s_hList, // handle to destination window
      LB_RESETCONTENT,          // message to send
      0, // not used; must be zero
      0 );  // not used; must be zero
   Traverse_List( pHead, pNext )
   {
      dwcnt++;
      pmwl = (PMWL)pNext;
      pfd  = (PWIN32_FIND_DATA)((PMWL)pmwl + 1);
      if( pmwl->wl_dwFlag & flg_DELETE )
         continue;

      li.LowPart  = pfd->nFileSizeLow;
      li.HighPart = pfd->nFileSizeHigh;
      wsprintf(lps,
         "[%s] of %s bytes at %s",
         &pmwl->wl_szFile[0],
         GetI64Stg( &li ),
         GetFDTStg( &pfd->ftLastWriteTime ) );

      lRes = SendDlgItemMessage(hDlg, IDD_MO_NAME,
         LB_ADDSTRING,
         0,
         (LPARAM) lps);
      if( (lRes == LB_ERR) || (lRes == LB_ERRSPACE) )
		{
         if( lRes == LB_ERR )
            SRError( pRErr1, NULL );
         else
            SRError( pRErr2, NULL );
         flg = 0;
			break;
		}
      lRes = SendDlgItemMessage( hDlg, IDD_MO_NAME,
         LB_SETITEMDATA,
         (WPARAM) lRes,
         (LPARAM) pmwl );
      if( lRes == LB_ERR )
		{
         SRError( pRErr6, NULL );
			flg = 0;
			break;
		}
      flg++;
   }

   // put the count at the TOP
   {
      lps = &pss->s_szNew[0];
      wsprintf(lps, "List of %d of total of %d files", flg, dwcnt );
      SetDlgItemText( hDlg, IDD_COLCNT, lps );
   }

   return flg;

}

// WM_INITDIALOG
INT_PTR Do_Srch4_Init(
    HWND hDlg,  // handle to dialog box
    WPARAM wParam, // first message parameter
    LPARAM lParam ) // second message parameter
{
	INT_PTR	flg = FALSE;
   PSS      pss = (PSS)lParam;

   SET_PROP( hDlg, ATOM_SRCH4, pss );

   CenterWindow( hDlg, GetWindow( hDlg, GW_OWNER ) );
   pss->s_hList = GetDlgItem(hDlg, IDD_MO_NAME);    // list box handle

   if( pss = (PSS)GET_PROP(hDlg, ATOM_SRCH4) )
   {
      flg = Do_Srch4_List( hDlg, pss );
   }

   return flg;
}

INT_PTR  Do_Srch4_OK( HWND hDlg )
{
   INT_PTR  iRet = FALSE;
   PSS      pss;
   PLIST_ENTRY pHead, pNext;
   PMWL     pmwl;
   if( pss = (PSS)GET_PROP(hDlg, ATOM_SRCH4) )
   {
      pHead = &pss->s_sFilList;
      Traverse_List( pHead, pNext )
      {
         pmwl = (PMWL)pNext;
         if( !(pmwl->wl_dwFlag & flg_DELETE) )
         {
            pmwl->wl_dwFlag |= flg_IsValid;
            iRet++;
         }
      }
   }
   return iRet;
}

INT_PTR  Do_Srch4_REST( HWND hDlg )
{
   INT_PTR  iRet = FALSE;
   PSS      pss;
   PLIST_ENTRY pHead, pNext;
   PMWL     pmwl;
   if( pss = (PSS)GET_PROP(hDlg, ATOM_SRCH4) )
   {
      pHead = &pss->s_sFilList;
      Traverse_List( pHead, pNext )
      {
         pmwl = (PMWL)pNext;
         if( pmwl->wl_dwFlag & flg_DELETE )
         {
            pmwl->wl_dwFlag &= ~( flg_DELETE );
            iRet++;
         }
      }
      if( iRet )
      {
         Do_Srch4_List( hDlg, pss );
      }
   }
   return iRet;
}

// handle IDD_DELETE
INT_PTR  Do_Srch4_DEL( HWND hDlg )
{
   INT_PTR  iRet = FALSE;
   PSS      pss;
   PLIST_ENTRY pHead;
//   PLIST_ENTRY pNext;
   PMWL     pmwl;
   LRESULT  lRes, lRes2, lRes3;
   DWORD    dwsize;
   PDWORD   pdw;

   lRes = SendDlgItemMessage( hDlg, IDD_MO_NAME,
         LB_GETSELCOUNT, 0, 0 );
   if( ( lRes == LB_ERR ) || ( lRes == 0 ) )
      return iRet;
   dwsize = (iRet + 1) * sizeof(DWORD);
   pdw = MALLOC(dwsize);
   if(!pdw)
      return iRet;

   lRes2 = SendDlgItemMessage( hDlg, IDD_MO_NAME,
         LB_GETSELITEMS,
         (WPARAM) lRes,
         (LPARAM) pdw );
   if( lRes2 == LB_ERR )
      goto DEL_RET;

   if( pss = (PSS)GET_PROP(hDlg, ATOM_SRCH4) )
   {
      pHead = &pss->s_sFilList;
      for( lRes2 = 0; lRes2 < lRes; lRes2++ )
      {
			lRes3 = SendDlgItemMessage( hDlg, IDD_MO_NAME,
            LB_GETITEMDATA,
            (WPARAM) pdw[lRes2],
            0L );
         if( lRes3 != LB_ERR )
         {
            //Traverse_List( pHead, pNext )
            //{
               pmwl = (PMWL)lRes3;
               if( !(pmwl->wl_dwFlag & flg_ToDelete) )
               {
                  pmwl->wl_dwFlag |= flg_ToDelete;
                  iRet++;
               }
            //}
         }
      }

      if( iRet )
      {
         Do_Srch4_List( hDlg, pss );
      }
   }

DEL_RET:

   MFREE(pdw);

   return iRet;
}

// IDD_SELECTED
INT_PTR  Do_Srch4_SEL( HWND hDlg )
{
   INT_PTR  ipRet = FALSE;
   PSS      pss;
   PLIST_ENTRY pHead;
   PLIST_ENTRY pNext;
   PMWL     pmwl;
   LRESULT  lRes, lRes2, lRes3;
   DWORD    dwsize;
   HANDLE  *pdw, *pdw2;

   lRes = SendDlgItemMessage( hDlg, IDD_MO_NAME,
         LB_GETSELCOUNT, 0, 0 );
   if( ( lRes == LB_ERR ) || ( lRes == 0 ) )
      return FALSE;
   dwsize = ((lRes + 1) * sizeof(void*)) * 2;
   pdw = MALLOC(dwsize);
   if(!pdw)
      return FALSE;

   pdw2 = &pdw[ (lRes + 1) ];
   lRes2 = SendDlgItemMessage( hDlg, IDD_MO_NAME,
         LB_GETSELITEMS,
         (WPARAM) lRes,
         (LPARAM) pdw );
   if( lRes2 == LB_ERR )
      goto SEL_RET;

   if( pss = (PSS)GET_PROP(hDlg, ATOM_SRCH4) )
   {
      pHead = &pss->s_sFilList;
      for( lRes2 = 0; lRes2 < lRes; lRes2++ )
      {
			lRes3 = SendDlgItemMessage( hDlg, IDD_MO_NAME,
            LB_GETITEMDATA,
            (WPARAM) pdw[lRes2],
            0L );
         //if( lRes3 != LB_ERR )
          pdw2[lRes2] = (HANDLE)lRes3;   // keep the data item
      }

      Traverse_List( pHead, pNext )
      {
         pmwl = (PMWL)pNext;
         for( lRes2 = 0; lRes2 < lRes; lRes2++ )
         {
            if( pdw2[lRes2] == pmwl )
            {
               if( pmwl->wl_dwFlag & flg_DELETE )
               {
                  pmwl->wl_dwFlag &= ~( flg_DELETE );
                  ipRet++;
               }
               break;   // exit on a find
            }
         }
         if( lRes2 == lRes )
         {
            // if NONE found, ie NOT selected, then if NOT deleted
               if( !( pmwl->wl_dwFlag & flg_DELETE ) )
               {
                  pmwl->wl_dwFlag |= flg_ToDelete;
                  ipRet++;
               }
         }
      }  // for each member in the list

      if( ipRet )
      {
         // if list changed
         Do_Srch4_List( hDlg, pss );
      }
   }

SEL_RET:

   MFREE(pdw);

   return ipRet;

}


INT_PTR Do_Srch4_Command(
    HWND hDlg,  // handle to dialog box
    WPARAM wParam, // first message parameter
    LPARAM lParam ) // second message parameter
{
	INT_PTR	flg = FALSE;
   DWORD    cmd = LOWORD(wParam);
	switch( cmd )
	{

  	case IDD_OK:
		flg = Do_Srch4_OK( hDlg );
   	EndDialog( hDlg, flg ) ;
      flg = TRUE;
		break;

	case IDD_CANCEL:
   	EndDialog( hDlg, FALSE ) ;
      flg = TRUE;
		break;

   case IDD_RESTORE:
		flg = Do_Srch4_REST( hDlg );
      break;

   case IDD_DELETE:
		flg = Do_Srch4_DEL( hDlg );
      break;

   case IDD_SELECTED:
		flg = Do_Srch4_SEL( hDlg );
      break;

   }	/* case of cmd (WPARAM) */

   return flg;

}

// was SEARCH2DLGPROC()
INT_PTR CALLBACK SEARCH4DLGPROC(
    HWND hDlg,  // handle to dialog box
    UINT uMsg,     // message
    WPARAM wParam, // first message parameter
    LPARAM lParam ) // second message parameter
{
	INT_PTR	flg = FALSE;
	switch (uMsg)
	{

	case WM_INITDIALOG:
      flg = Do_Srch4_Init( hDlg, wParam, lParam );
      if( !flg )
         EndDialog(hDlg, -1 );
      break;

	case WM_CLOSE:	/* 0x0010 */
      EndDialog( hDlg, FALSE ) ;
      flg = TRUE;
		break;

	case WM_COMMAND:
      flg = Do_Srch4_Command( hDlg, wParam, lParam );
		break;

   case WM_DESTROY:
      REMOVE_PROP( hDlg, ATOM_SRCH4 );
      flg = TRUE;
      break;

   }

   return flg;
}

UINT	SRSearch4( HWND hwnd, PSS pss )
{
   UINT	i, j;
	if( fShwAFil )
		j = SRSEARCH3;
	else
		j = SRSEARCH2;
   i = DialogBoxParam( ghDvInst,
			MAKEINTRESOURCE(j),
			hwnd,
			SEARCH4DLGPROC,   // was SEARCH2DLGPROC
         (LPARAM)pss );
	if( i == (UINT) -1 )
	{
		SRError( pRErr5, NULL );
      i = 0;
	}
   return i;
}


/*	=====================================================
	UINT	SRSearch( HWND hwnd )

	ONLY CALLED BY:
		void	SROpenAs( HWND hwnd ) - Impliments IDM_OPENAS from MENU command

	=====================================================	*/
UINT	SRSearch( HWND hwnd )
{
	UINT	   ui;
   PSS      pss;
   PLIST_ENTRY pNext, pHead;
   PMWL     pmwl, pmwl2;
   PRDIB    prd;
   LPTSTR   lpf;

   if( gfInSearch )
      return 0;

   gfInSearch = TRUE;
   ui = sizeof(SRCHSTR) + sizeof(RDIB);
   if( pss = (PSS)MALLOC(ui) )
   {
      ZeroMemory(pss, ui);
      prd = (PRDIB)((PSS)pss + 1);
      InitLList( &pss->s_sDirList );
      InitLList( &pss->s_sFilList );
      InitLList( &pss->s_sMasks   );
      InitLList( &pss->s_sFinds   );
      //pss->s_pFinds = &gsFindList;
      //pss->s_pMasks = &gsMaskList;
      ListCount2( &gsMaskList, &pss->s_dwMasks );
      if( pss->s_dwMasks != gdwMaskCnt )  // count of masks
      {
         chkme( "HEY! Count not equal to count!!!" );
         gdwMaskCnt = pss->s_dwMasks;  // count of masks
      }
      if( gdwMaskCnt != CopyLList( &gsMaskList, &pss->s_sMasks ) )
      {
         chkme( "HEY2! Count not equal to count!!!" );
      }
      pss->s_dwFinds = gdwFindCnt;  // count of finds
      if( gdwFindCnt != CopyLList( &gsFindList, &pss->s_sFinds ) )
      {
         chkme( "HEY3! Count not equal to count!!!" );
      }

      // DialogBoxParam()
	   ui = DialogBoxParam( ghDvInst,
			MAKEINTRESOURCE(SRSEARCH),
			hwnd,
			SEARCHDLGPROC,
         (LPARAM)pss );

	   if( ui == (UINT) -1 )
	   {
		   SRError( pRErr5, NULL );
         ui = 0;
	   }
      else if( ui )
	   {
		   //OrgSCnt = SrchCnt;	/* Keep the ORIGINAL Count in the LIST */
         ui = SRSearch4( hwnd, pss );
		   if( ui == (int) -1 )
		   {
			   SRError( pRErr5, NULL );
            ui = 0;
		   }

         if( ui )
         {
            ui = 0;
      		Hourglass( TRUE );
            pHead = &pss->s_sFilList;
            Traverse_List( pHead, pNext )
            {
               pmwl = (PMWL)pNext;
               if( pmwl->wl_dwFlag & flg_IsValid )
               {
                  lpf = &pmwl->wl_szFile[0];
      				NULPRDIB(prd);
                  prd->rd_pTitle = gszRPTit;
                  prd->rd_pPath  = gszRPNam;
                  strcpy( gszRPTit, lpf );
                  DVGetFullName2( gszRPNam, gszRPTit );
				      prd->rd_hWnd = hwnd;
      				OpenDIBFile2( prd );
                  if( prd->rd_hDIB )
				      {
                     /* We have a DIB ... */
					      prd->rd_Caller = df_SEARCH;
					      if( OpenDIBWindow2( prd ) )
                     {
                        ui++;
#ifdef	CHGADDTO
                        ADD2LIST(prd);
#endif	// CHGADDTO
                     }
				      }
               }
            }  // traverse sFilList looking just for flg_IsValid

            if(ui)
            {
               // we added a window or 2 ...
               pHead = &pss->s_sMasks;
               Traverse_List( pHead, pNext )
               {
                  pmwl = (PMWL)pNext;
                  if( pmwl->wl_dwFlag & flg_SrchOK )
                  {
                     PLIST_ENTRY pNxt, pHed;
                     // a potential addition to our used mask list
                     pHed = &gsMaskList;
                     Traverse_List( pHed, pNxt )
                     {
                        pmwl2 = (PMWL)pNxt;
                        if( lstrcmpi( &pmwl->wl_szFile[0], &pmwl2->wl_szFile[0] ) == 0 )
                        {
                           // it is already IN the list
                           pmwl->wl_dwFlag &= ~( flg_SrchOK ); // remove the FLAG
                           break;   // done this iteration of the list
                        }
                     }
                     if( pmwl->wl_dwFlag & flg_SrchOK )
                     {
                        // is NOT already in the LIST, so ADD IT NOW
                        if( Add2LListHead( pHed, &pmwl->wl_szFile[0] ) )
                        {
                           // now set as FIRST in the LIST
                           gdwMaskCnt++;     // bump the count
                           gbChgMsk = TRUE;  // and flag it to be put to INI file
                        }
                     }
                  }
               }
            }
         }
	   }

      FreeLList( &pss->s_sDirList, pNext );
      FreeLList( &pss->s_sFilList, pNext );
      FreeLList( &pss->s_sMasks  , pNext );
      FreeLList( &pss->s_sFinds  , pNext );

      MFREE(pss);

   }

   gfInSearch = FALSE;

   return( ui );

}

/*	======================================================
	void	SROpenAs( HWND hwnd )

	PURPOSE: Impliments IDM_OPENAS from MENU command
  
	======================================================	*/
void	SROpenAs( HWND hwnd )
{
   SRSearch( hwnd );
}	// void	SROpenAs( HWND hwnd )


// some utility functions
void	AddGlob( LPTSTR lps )
{
   INT   i;
   //chkme( "Check the NEW AddGlob() function!" );
   if( i = strlen(lps) )
   {
      if( lps[i-1] != '\\' )
         strcat(lps,"\\");
   }
   strcat(lps,"*.*");
}

LPTSTR	SRGetSFil( UINT ind )
{
   LPTSTR      lpf = 0;
   PLIST_ENTRY pHead, pNext;
   UINT        icnt = 0;

   pHead = &gsFindList;
   Traverse_List( pHead, pNext )
   {
      if( icnt == ind )
      {
         PMWL  pmwl = (PMWL)pNext;
         lpf = &pmwl->wl_szFile[0];
         break;
      }
      icnt++;
   }
   return lpf;
}


#else // !ifdef  USELLIST2   // FIX20001210 - switch to using a LINKED LIST

extern	char	UpIt( char c );

#ifdef	ADDOPENALL

//#include "srsearch.h"
#include "DvSrch.h"

#define	MXDPATH		48	/* Use this min. length of drive:path/filename */
#define	MXSDIR		8192	/* Buffer for DIRECTORIES ... */
#define	MXDIRBUFS	150	/* Max. Number of Directory buffers ... */
#define	DELMARK		0xfe	/* Marked as DELETED */
#define	SELMARK		0xfd	/* Mark as SELECTED */

/* external */
extern	char	SRUpperCase( char );
extern	DWORD	SRReadLong( HFILE, LPSTR, DWORD );
extern	BOOL	SRIsExeOrDll( LPSTR );

extern void SRUnlock( HGLOBAL );
extern HGLOBAL	SRAlloc( UINT, DWORD );
extern LPSTR SRLock( HGLOBAL );
extern void SRErrorN( WORD, LPSTR );
//extern void SRError( LPSTR, LPSTR );
extern DWORD MyDOSWrite( HFILE, LPSTR, DWORD );
extern void SRClose( LPSTR, HFILE );
extern HFILE SRCreat( LPSTR, DWORD );
extern HFILE SROpen( LPSTR, DWORD );
extern void SRWait( BOOL, DWORD );
extern void SRFree( HGLOBAL );
#ifdef	WIN32
extern DWORD SRFindFirst( LPDGETFILE );
extern BOOL SRFindNext( LPDGETFILE );
#else	// !WIN32
extern DWORD FAR SRFindFirst( LPDGETFILE );
extern BOOL FAR	SRFindNext( LPDGETFILE );
#endif	// WIN32 y/n

extern DWORD DVFileSize( HANDLE );
extern BOOL SRQuery( LPSTR );
//extern void SRSetupPath( LPSTR, LPSTR, LPSTR );

//extern void	DVGetFPath( LPSTR, LPSTR, LPSTR, LPSTR, LPSTR );
extern BOOL SetInIndex( LPSTR );

/* local */
void	SRTermSearch( void );
void SRCloseSch( void );
//BOOL	fInSearch = FALSE;	/* We are processing a GROUP of files ... */
//BOOL	fChgIter  = FALSE;
//BOOL	fDoIter   = TRUE;

BOOL	fDnSrch = FALSE;
//BOOL	fChgFile = FALSE;
//BOOL	fChgMask = FALSE;
//char	szSearchDir[MXFILNAM+2];
//BOOL	fDoIter = TRUE;
UINT	SrchCnt, FilCnt, DelCnt, DirCnt, DirOff, DirOff2, OrgSCnt;
HGLOBAL	hFiles = 0;
HFILE	hTFile = 0;
LPSTR	lpSavFil = 0;
BOOL	fGotMask;

char	*pRBSlash = "\\";
//char	*pRGlob = "*.*";
char	*pRNoFil = "ERROR: No files found!";
char	*pRFilNm = "\r\nNothing matching -\r\n[%s]?";
//char	szDefSch[] = "TEMPACT1.LST";
//char	szSchFil[MXFILNAM+1];
//char	szSchMask[MXFILNAM+1];	/* This is JUST the search MASK (No D:\DIr) */
//char	szSchFil[MXFILNAM+1];
//char	*pRCurSch = &szSchFil[0];
//char	szTpBuf[MXLTXT + 1];
//char	szTpBuf2[MXLTXT + 1];

char	*pRTmpHdr = "; Found %d entries of [%s]...\r\n";
char	*pRTmpHd2 = "; Plus  %d entries of [%s]...\r\n";
char	*pRDirEnt = " <DIR>      %02d/%02d/%02d %02d:%02d\r\n";
char	*pRSFilEnt = " %10ld %02d/%02d/%02d %02d:%02d\r\n";
char	*pRTmpEFm = "; End of List of %d files.\r\n";
char	*pRTotCnt = "Total of %d files. [Continue] when ready."; 

char	*pRWarn2 = "WARNING: Internal counters do not match!";
char	*pRWarn3 = "WARNING: Unable to allocate further memory for Directories!";

//char	*pRErr1 = "ERROR: String could NOT be added to list box!";
//char	*pRErr2 = "ERROR: List box is FULL! No more space!";
char	*pRErr3 = "ERROR: Unable to OPEN file [TEMPACT1.LST]!";
char	*pRErr4 = "ERROR: Appears NO items have been SELECTED, thus NO action can be done!";
//char	*pRErr5 = "ERROR: Unable to create DIALOG BOX!";
//char	*pRErr6 = "ERROR: Data Item could not be attached to String!";

char	*pRQuer1 = "QUERY: Delete %d selected items from list?";
char	*pRQuer2 = "QUERY: Delete ALL itmes from list, except %d selected?";

HWND	hParen;
HGLOBAL	hStrings = 0;
//BOOL	fShwAFil = TRUE;

HGLOBAL	hDirs[MXDIRBUFS] = {0};

LPSTR	lpDirs = 0;
LPSTR	lpDir2 = 0;
UINT	CurDirs = 0;
UINT	NxtDirs = 0;
BYTE	ActDrv;
WORD	nInDlgProc = 0;

/* ************************************************************************* */
void	chkdir( void )
{
int	i;
	i = 0;
}

int GoModalDialogBoxParam( HINSTANCE hInstance, LPCSTR lpszTemplate,
	HWND hWnd, DLGPROC lpDlgProc, LPARAM lParam )
{
   DLGPROC  lpProcInstance ;
   int	res;
	nInDlgProc++ ;
   lpProcInstance = (DLGPROC) MakeProcInstance( (FARPROC) lpDlgProc,
                                                hInstance ) ;
   res = DialogBoxParam( hInstance, lpszTemplate, hWnd, lpProcInstance, lParam ) ;
   FreeProcInstance( (FARPROC) lpProcInstance ) ;
	nInDlgProc--;
   return( res );
} /* GoModalDialogBoxParam() */

BOOL	IsDiffer( LPSTR lp1, LPSTR lp2 )
{
BOOL	flg;
int	i, j, k;
char	a, b;
	flg = TRUE;
	if( (i = lstrlen( lp1 )) == (j = lstrlen( lp2 )) )
	{
		for( k = 0; k < i; k++ )
		{
			a = SRUpperCase( lp1[k] );
			b = SRUpperCase( lp2[k] );
			if( a != b )
				break;
		}
		if( k == j )
			flg = FALSE;
	}
return( flg );
}

void	AddGlob( LPSTR lps )
{
int	a, i, j, k, l;
LPSTR	lpb;
char	c, d;
	a = 0;
	if( lpb = lps )
	{
		if( i = lstrlen( lpb ) )
		{
			k = 0;
			for( j = 0; j < i; j++ )
			{
				c = lpb[j];
				if( ( c == ':' ) || ( c == '\\' ) )
				{
					k = j + 1;
					d = c;
				}
			}
			lpb = lpb + k;
			l = lstrlen( lpb );
			if( k == 0 )
			{
				a = 2;
			}
			else if( k == i )
			{
				if( d == ':' )
					a = 2;
				else
					a = 1;
			}
			else
			{
				if( l )
				{
					for( j = 0; j < l; j++ )
					{
						c = lpb[j];
						if( (c == '*') || (c == '?') )
						{
							break;
						}
					}
					if( j == l )
					{
						a = 2;
					}
				}
				else
				{
					a = 2;
				}
			}
		}
		switch( a )
		{
			case 2:
				lstrcat( lps, pRBSlash );
			case 1:
				lstrcat( lps, pRGlob );
		}
	}
}

//#ifdef   USELLIST

BOOL	FillComboList( HWND hList, PLIST_ENTRY pHead )
{
	BOOL	   fOk;
   LPARAM   lPar;   // My own DATA item per string
   LRESULT  lRes;
	PLIST_ENTRY pNext;
	LPSTR	   lps;
	int		i;
   PMWL     pmwl;

	fOk  = FALSE;
	lPar = 0;
	if( ( hList           ) &&
		 ( IsWindow(hList) ) )
	{
		SendMessage( hList, CB_RESETCONTENT, 0, 0 );
      Traverse_List( pHead, pNext )
      {
         pmwl = (PMWL)pNext;
            if( pmwl->wl_dwLen )
				{
               lps = &pmwl->wl_szFile[0];
					if( i = lstrlen(lps) )
					{
						{
								lRes = SendMessage( hList, CB_ADDSTRING, 0,
									(LPARAM)lps );
								if( ( lRes == CB_ERR      ) ||
									 ( lRes == CB_ERRSPACE ) )
								{
									break;
								}
								else
								{
									SendMessage( hList,
										CB_SETITEMDATA,
										(WPARAM)lRes, lPar );
								}
						}
                  lPar++;
					}
				}
      }

		SendMessage( hList, CB_SETCURSEL, 0, 0 );

	}

	return fOk;

}

//	Init the SEARCH dialog box. ie handle WM_INITDIALOG

BOOL	SearchInit( HWND hDlg, WPARAM wPara, LPARAM lParam )
{
	HWND	hEd, hLst;
	HWND	hAct;
	UINT	uiCtl;

	fGotMask = FALSE;	/* We have NO MASK yet ... */

   CenterWindow( hDlg, GetWindow( hDlg, GW_OWNER ) );

	if( !fDnSrch )	/* If this is the first ... */
	{
		fDnSrch = TRUE;
		if( gszSearchDir[0] == 0 )
		{
			GetSystemDirectory( &gszSearchDir[0], MXFILNAM );
			AddGlob( &gszSearchDir[0] );
			gfChgMask = TRUE;
		}
	}

	hEd  = GetDlgItem( hDlg, IDM_SELECT );
	hLst = GetDlgItem( hDlg, IDC_COMBO1 );
	if( gdwFindCnt )
	{
		uiCtl = IDC_COMBO1;
		if( hEd )
		{
			EnableWindow( hEd, FALSE    );
			ShowWindow(   hEd, SW_HIDE  );
		}
		if( hLst )
		{
			EnableWindow( hLst, TRUE    );
			ShowWindow(   hLst, SW_SHOW );
			hAct = hLst;
		}

		FillComboList( hAct, &gsFindList );

	}
	else
	{
		uiCtl = IDM_SELECT;
		if( hEd )
		{
			EnableWindow( hEd, TRUE     );
			ShowWindow(   hEd, SW_SHOW  );
			hAct = hEd;
		}
		if( hLst )
		{
			EnableWindow( hLst, FALSE   );
			ShowWindow(   hLst, SW_HIDE );
		}

		SetDlgItemText( hDlg, uiCtl, (LPSTR) gszSearchDir );

	}


	CheckDlgButton( hDlg, IDD_AUTOWRAP, gfDoIter );

	NxtDirs = CurDirs = DirOff = DirOff2 = DirCnt = FilCnt = DelCnt = SrchCnt = 0;
	OrgSCnt = 0;

	return TRUE;	/* and return TRUE to display DIALOG ... */

}



#define	DFHour( a )		(( a & 0xf800 ) >> 11 )
#define	DFMins( a )		(( a & 0x07e0 ) >> 5  )
#define	DFSecs( a )		(( a & 0x001f ) *  2  )
#define	DFYear( a )		((( a & 0xfe00 ) >> 9  ) + 80)
#define	DFMonth( a )	(( a & 0x01e0 ) >> 5  )
#define	DFDay( a )		( a & 0x001f )

BOOL	Need2Save( LPFILEFIND lpff )
{
BOOL	flg;
LPSTR	lpf;
int	i;
char	b, c, d;
	flg = FALSE;
	lpf = &lpff->ffByte[0];
	if( lpff->ffAttr & dfDirectory )
	{
		if( i = lstrlen( lpf ) )
		{
			b = lpf[i - 1];	/* Get LAST char ... */
			if( i > 1 )
				c = lpf[i - 2];
			else
				c = '\\';
			if( i > 2 )
				d = lpf[i - 3];
			else
				d = '\\';
			if( ( (b == '.') && (c == '\\') ) ||
				( (b == '.') && (c == '.') && (d == '\\') ) )
			{
				flg = FALSE;
			}
			else
			{
				flg = TRUE;	/* Save THIS directory ... */
			}
		}
	}
	else
	{
		if( fGotMask )	/* If we HAVE a User's MASK, then SAVE THIS FILE */
			flg = TRUE;
		else
			flg = SRIsExeOrDll( lpf );
	}
return( flg );
}

BOOL	SaveDirs( LPSTR lpd )
{
   BOOL	flg = TRUE;
   int	i, j;
   LPSTR	lps;
	flg = FALSE;
	if( hDirs[CurDirs] && (lps = lpDirs) && (i = lstrlen( lpd )) )
	{
		if( (i + 1 + DirOff) < MXSDIR )
		{
			for( j = 0; j < i; j++ )
			{
				lps[DirOff++] = lpd[j];
			}
			lps[DirOff++] = 0;
			lps[DirOff] = 0;
			DirCnt++;
		}
		else if( (CurDirs + 1) < MXDIRBUFS )
		{
			SRUnlock( hDirs[CurDirs] );
			lpDirs = 0;
			CurDirs++;
			if( ( hDirs[CurDirs] = SRAlloc( GHND, MXSDIR+1 ) ) &&
				( lpDirs = SRLock( hDirs[CurDirs] ) ) )
			{
				lps = lpDirs;
				DirOff = 0;
				for( j = 0; j < i; j++ )
				{
					lps[DirOff++] = lpd[j];
				}
				lps[DirOff++] = 0;
				lps[DirOff] = 0;
				DirCnt++;
			}
			else
			{
				SRErrorN( IDS_MEMFAILED, NULL );
				flg = TRUE;
			}
		}
		else
		{
			SRError( pRWarn3, NULL );
			flg = TRUE;
		}
	}
   return( flg );
}

void	AddLine2File( LPSTR lpForm, UINT Cnt, LPSTR lpm, LPSTR lpf )
{
char	buf[128];
LPSTR	lps;
DWORD	len;
	if( hTFile && (hTFile != HFILE_ERROR) )
	{
		lps = &buf[0];
		wsprintf( lps, lpForm, Cnt, lpm );
		len = (DWORD) lstrlen( lps );
		if( (MyDOSWrite( hTFile, lps, len )) != len )
		{
			SRClose( lpf, hTFile );
			hTFile = HFILE_ERROR;
		}
	}
}

BOOL	SRSavFil( LPFILEFIND	lpff, LPSTR lpm )
{
BOOL	flg;
LPSTR	lpf, lpt;
DWORD	len;
UINT	hr, min, sec, yr, mth, dy;
char	buf[128];
LPSTR	lps;
int	i;
	lps = &buf[0];
	flg = FALSE;
	lpf = &lpff->ffByte[0];	/* Pointer to this file or directory ... */
	lpt = &gszSchFil[0];    // pRCurSch;
 if( Need2Save( lpff ) )
 {
	if( hTFile == 0 )
	{
		lpSavFil = lpt;
		hTFile = SRCreat( lpt, 0 );
		if( hTFile && (hTFile != HFILE_ERROR) )
		{
			AddLine2File( (LPSTR) pRTmpHdr, FilCnt, lpm, lpSavFil );
		}
	}
	if( hTFile && (hTFile != HFILE_ERROR) )
	{
		lstrcpy( lps, lpf );
		i = lstrlen( lps );
		while( i < MXDPATH )
		{
			lps[i] = ' ';
			i++;
			lps[i] = 0;
		}
		len = (DWORD) lstrlen( lps );
		if( (MyDOSWrite( hTFile, lps, len )) != len )
		{
			SRClose( lpSavFil, hTFile );
			hTFile = HFILE_ERROR;
			goto srsvret;
		}
		hr = DFHour( lpff->ffTime );
		min = DFMins( lpff->ffTime );
		sec = DFSecs( lpff->ffTime );
		yr = DFYear( lpff->ffDate );
		mth = DFMonth( lpff->ffDate );		
		dy = DFDay( lpff->ffDate );
		if( lpff->ffAttr & dfDirectory )	/* If a DIRECTORY ENTRY ... */
		{
			if( SaveDirs( lpf ) )
				goto srsvret;	/* Can NOT save any more ... */
			wsprintf( lps, pRDirEnt, dy, mth, yr, hr, min );
		}
		else
		{
			wsprintf( lps, pRSFilEnt, lpff->ffSize, dy, mth, yr, hr, min );
			SrchCnt++;	/* Bump the FILE counter ... */
		}
		len = (DWORD) lstrlen( lps );
		if( (MyDOSWrite( hTFile, lps, len )) != len )
		{
			SRClose( lpSavFil, hTFile );
			hTFile = HFILE_ERROR;
			goto srsvret;
		}
		flg = TRUE;	/* All ok ... */
	}
 }
 else
 {
	flg = TRUE;
 }
srsvret:
return( flg );
}

void	SREndFil( int i )
{
DWORD	len;
char	buf[128];
LPSTR	lps;
	if( hTFile && (hTFile != HFILE_ERROR ) )
	{
		lps = &buf[0];
		wsprintf( lps, pRTmpEFm, i );
		len = (DWORD) lstrlen( lps );
		MyDOSWrite( hTFile, lps, len );
		SRClose( lpSavFil, hTFile );
	}
	hTFile = 0;
}

/* ==============================================================
 * SetNxtDir( ... )
 *
 * ============================================================== */
#undef		NWIN32
//#define _MAX_PATH   260 /* max. length of full pathname */
//#define _MAX_DRIVE  3   /* max. length of drive component */
//#define _MAX_DIR    256 /* max. length of path component */
//#define _MAX_FNAME  256 /* max. length of file name component */
//#define _MAX_EXT    256 /* max. length of extension component */
BOOL	SetNxtDir( LPSTR lpm, LPSTR lpd, LPSTR lpdir )
{
	BOOL	flg;
	int		i, j, k;
#ifdef	NWIN32
	char	bufd[_MAX_PATH+1];
	char	bufp[_MAX_DIR+1];
	LPSTR	lpp;
#endif	// NWIN32
	flg = FALSE;
	if( lpm &&
		(k = lstrlen( lpm )) &&
		( i = lstrlen( (LPSTR) &lpd[DirOff2] ) ) )
	{
		j = 0;
#ifdef	NWIN32 // 
		lpp = &bufd[0];
		DVGetFPath( lpm, lpp, &bufp[0], NULL, NULL );
		lstrcat( lpp, &bufp[0] );
		lstrcpy( lpm, lpp );
		j = lstrlen( lpm );
		if( j && (lpm[j-1] != '\\') )
			lstrcat( lpm, "\\" );
#endif	// NWIN32
		for( ; j < i; j++ )
		{
			lpm[j] = lpd[DirOff2++];
		}
		lpm[j] = 0;
		if( j && (lpm[j-1] != '\\') )
			lstrcat( lpm, "\\" );
		lstrcpy( lpdir, lpm );
		lstrcat( lpdir, "*.*" );	/* Setup GLOBAL file for DIRECTORY */
		lstrcat( lpm, &gszSchMask[0] );	/* And a the MASK for the FILE */
		DirOff2++;
		flg = TRUE;
	}
	else
	{
		DirOff2 = 0;
	}
return( flg );
}

/* ======================================================
	WORD	FillInCP( LPSTR lps )

	======================================================	*/
WORD	FillInCP( LPSTR lps )
{
	int	i, j;
	BYTE	c;
	j = 0;

	lstrcpy( lps, &gszSchFil[0] );   // pRCurSch );
	if( i = lstrlen( lps ) )
	{
		j = i - 1;
		for( ; j >= 0; j-- )
		{
			c = lps[j];
			if( c == '\\' )
			{
				j++;
				lps[j] = 0;
				break;
			}
		}
	}
	return((WORD) j );
}

BOOL	GotMask( LPSTR lpf )
{
BOOL	flg;
WORD	i, j;
char	c;
	flg = FALSE;
	if( lpf && (i = lstrlen( lpf ) ) )
	{
		for( j = 0; j < i; j++ )
		{
			c = lpf[j];
			if( !((c == '.') || (c == '\\') || (c == '*') || (c == '?')) )
			{
				flg = TRUE;
				break;
			}
		}
	}
return( flg );
}

#define     MXDRVS      26
//#define       MXDBUF      ((MXDRVS+6) * 4)
#define     MXDBUF      288

typedef struct tagDRLIST {  /* dl */
    BOOL    dl_ok;
    HGLOBAL dl_gh;
    int     dl_cnt;
    DWORD   dl_sz;
    int     dl_sel;
    DWORD   dl_soff;
    int     dl_drv;
    DWORD   dl_drives;
    DWORD   dl_count;
    char    dl_szList[MXDBUF];
}DRLIST;

typedef DRLIST * PDRLIST;

DRLIST	gsDrives = { 0 };
extern	void	GMGetDL( PDRLIST pdrl );
/*	======================================================
	void	FixFileMask( LPSTR lpm )


	====================================================== */
void	FixFileMask( LPSTR lpm )
{
	int		flg = 0;
	int		i, j, k;
//	BYTE	buf[MXFILNAM+2];
	LPSTR	lps, lpf, lpe;
	BYTE	c, d;
	PDRLIST	pdrl = &gsDrives;
	DWORD	dmask;

	lps = &gszTmp[0];

	if( !pdrl->dl_ok )
	{
		GMGetDL( pdrl );
	}

	if( i = lstrlen(lpm) )
	{
		if( i > 1 )
		{
			if( lpm[1] == ':' )
			{
				/* got DRIVE */
				c = UpIt( *lpm );
				pdrl->dl_drv = (int)(c - ('A' - 1)); // Set NUMBER
				dmask = 0;
				if( pdrl->dl_drv > 0 )
				{
					dmask = 1 << (pdrl->dl_drv - 1);
				}
				if( ( dmask                   ) &&
					( pdrl->dl_drives & dmask ) )
				{
					flg = 1;
				}
			}
			else
			{
				/* NO DRIVE */

			}
		}
		else
		{

		}
	}
	else	/* if( (i = lstrlen( lpm )) == 0 ) */
	{
		/* if no selection, just use "*.*" as the MASK */
		DVGetCwd( lpm, MAX_PATH );
		if( i = lstrlen(lpm) )
		{
			if( lpm[i-1] != '\\' )
				lstrcat( lpm, "\\" );
			
		}
		lstrcat( lpm, pRGlob );
	}

	i = lstrlen( lpm );
	if( i > 1 )
		d = lpm[1];
	else
		d = 0;

	k = 0;
	for( j = 0; j < i; j++ )
	{
		c = lpm[j];
		if( j == 0 )
		{
			if( d != ':' )	/* If we DO NOT HAVE a DRIVE ... */
			{
				if( c == '\\' )
				{
					lps[k++] = gszSchFil[0];   // pRCurSch[0];	/* Use DEFAULT DRIVE */
					lps[k++] = ':';
				}
				else
				{
					k = FillInCP( lps );
				}
			}
		}

		/* care with MAXIMUM length */
		if( k < MXLTXT )
		{
			lps[k++] = c;
		}
		else
		{
			break;
		}
	}

	lps[k] = 0;
	lstrcpy( lpm, lps );
	ActDrv = lpm[0];	/* Get the ACTIVE DRIVE ... */
	lpf = &gszSchMask[0];
	lstrcpy( lps, lpf );	/* Take COPY of any PREVIOUS mask ... */
	lpe = &gszTpBuf1[0];
	DVGetFPath( lpm, NULL, NULL, lpf, lpe );	/* Get the NAME a EXTENT */
	lstrcat( lpf, lpe );	/* Get the MASK */
	if( GotMask( lpf ) )
	{	/* If we GOT a new MASK, then ... */
		fGotMask = TRUE;
		if( lpe[0] )	/* If we have an EXTENT, then ... */
			SetInIndex( lpe );	/* Fix the INDEX ... maybe ... */
	}
	else
	{
		fGotMask = FALSE;
	}
}

int	GetSelection( HWND hDlg, LPSTR lpm, DWORD dwMax )
{
	int		i;
	LPSTR	lpnew = &gszTmp[0];
	HWND	hList;
	LRESULT	lRes;
	UINT	ui;

	ui = IsDlgButtonChecked( hDlg, IDD_AUTOWRAP );
	if( ui )
	{
		if( !gfDoIter )
		{
			gfChgIter = TRUE;
			gfDoIter = TRUE;
		}
	}
	else
	{
		if( gfDoIter )
		{
			gfChgIter = TRUE;
			gfDoIter = FALSE;
		}
	}
	i = 0;
	if( gdwFindCnt )
	{
	    *lpnew = 0;
		if( hList = GetDlgItem( hDlg, IDC_COMBO1 ) )
		{
			lRes = SendMessage( hList, CB_GETCURSEL, 0, 0L );
		    if( lRes != CB_ERR )
			{
				// Get the SELECTED
				ui = SendMessage( hList, CB_GETLBTEXT, lRes,
					(LPARAM)lpnew );
			}
			else
			{
				// Else get the EDIT part contents
				ui = SendMessage( hList, WM_GETTEXT,
					(WPARAM) MAX_PATH,  // number of characters to copy
					(LPARAM) lpnew );       // address of buffer for text
			}
		}
		else
		{
			// ERROR - NO HANDLE !!!
		}
		if( i = lstrlen(lpnew) )
		{
			if( (DWORD)i < dwMax )
			{
				lstrcpy( lpm, lpnew );
			}
			else
			{
				int	j;
				if( j = (int)dwMax )
				{
					j--;
					for( i = 0; i < j; i++ )
					{
						lpm[i] = lpnew[i];
					}
					lpm[i] = 0;
				}
			}
		}
		else
		{
			// ERROR - NO SELECTION
		}
	}
	else
	{
//		i = GetDlgItemText( hDlg, IDM_SELECT, lpm, MXFILNAM );
		i = GetDlgItemText( hDlg, IDM_SELECT, lpm, dwMax );
	}
	return i;
}
// SearchOK( hDlg )
// We have been given a FILE MASK ...
// Search for ALL FILES matching this MASK, and
// if Interate is CHECKED, then we must iterate into EACH
// SubDirectory find ALL such files ...
BOOL	SearchOK( HWND hDlg )
{
	BOOL	flg;
	int	i;
	BOOL	dif;
	HGLOBAL	hg;
	LPSTR	lps, lpm, lpd, lpDir;
	UINT	fcnt, cycs;
	LPDGETFILE lpgf;
	LPFILEFIND	lpff;
	DWORD	msiz, ffret;
	LPSTR	lptmp1, lptmp2;

	flg  = (BOOL)-1;
	lps  = 0;
	hg   = 0;
	i    = sizeof( DGETFILE );
	msiz = (DWORD) (i + sizeof( FILEFIND ) + 3 );

	if( ( hg = SRAlloc( GHND, msiz ) ) && 
		( lps = SRLock( hg ) ) )
	{
		lpgf = (LPDGETFILE) lps;
//		lpm = (LPSTR) &lpgf->gfMask[0];
		lpm = &lpgf->gfMask[0];
		i = GetSelection( hDlg, lpm, MXFILNAM );
	}
	else
	{
		SRErrorN( IDS_MEMFAILED, NULL );
		return flg;
	}

	SRWait( TRUE, SRW_BGN );
	flg = TRUE;
	cycs = 0;
	if( hDirs[CurDirs] )
		SRTermSearch();
	i = sizeof( DGETFILE );
	msiz = (DWORD) (i + sizeof( FILEFIND ) + 3 );
// if( ( hg = SRAlloc( GHND, msiz ) ) && 
//	( lps = SRLock( hg ) ) &&
//	( hDirs[CurDirs] = SRAlloc( GHND, MXSDIR+1 ) ) &&
//	( lpd = SRLock( hDirs[CurDirs] ) ) )
// {
	if( ( hDirs[CurDirs] = SRAlloc( GHND, MXSDIR+1 ) ) &&
		( lpd = SRLock( hDirs[CurDirs] ) ) )
	{
		lpDirs = lpd;
		lpDir = (LPSTR) &lpgf->gfDire[0];
		lpff = (LPFILEFIND) (lps + i);
		lpgf->gfName = lpff;
		lpff->ffSize = 0;
		lpff->ffAttr = 0;
		lpff->ffFNLen = 0;
		lpff->ffTime = 0;
		lpff->ffDate = 0;
//	u = IsDlgButtonChecked( hDlg, IDD_AUTOWRAP );
//	if( u )
//	{
//		if( !gfDoIter )
//		{
//			gfChgIter = TRUE;
//			gfDoIter = TRUE;
//		}
//	}
//	else
//	{
//		if( gfDoIter )
//		{
//			gfChgIter = TRUE;
//			gfDoIter = FALSE;
//		}
//	}

	//i = GetDlgItemText( hDlg, IDM_SELECT, lpm, MXFILNAM );
//	i = GetSelection( hDlg, lpm, MXFILNAM );

	FixFileMask( lpm );
	dif = IsDiffer( lpm, (LPSTR) gszSearchDir );
	if( gfDoIter )
	{
		chkdir();
		lpgf->gfAttr = (dfDirectory | dfArchive);
		lptmp1 = &gszTpBuf1[0];
		lptmp2 = &gszTpBuf2[0];
		lptmp1[0] = 0;
		lptmp2[0] = 0;
		DVGetFPath( lpm, lptmp1, lptmp2, NULL, NULL );	/* Get PATH */
		lstrcpy( lpDir, lptmp1 );
		lstrcat( lpDir, lptmp2 );
		AddGlob( lpDir );	// Build a GLOBAL to search for DIRECTORIES
	}
	else
	{
		lpgf->gfAttr = dfArchive;
		lpDir[0] = 0;
	}
	i = 0;
	lpgf->gfMxCnt = (int)-1;

DoNxtDir:

	SRWait( TRUE, SRW_INC );
//	SetDlgItemText( hDlg, IDM_SELECT, lpm ) ;	/* DOES NOT APPEAR TO WORK */
	lpgf->gfFCount = 0;
	lpgf->gfDCount = 0;
	ffret = SRFindFirst( lpgf );	/* Get the FIRST of file and total count ... */
	if( HIWORD( ffret ) &&
	 ( fcnt = (lpgf->gfFCount + lpgf->gfDCount) ) )	/* If we have a TOTAL count ... */
	{
		if( cycs )
		{
			AddLine2File( (LPSTR) pRTmpHd2, fcnt, lpm, lpm );
		}
		FilCnt += fcnt;	/* Keep the TOTAL list of files and DIRECTORIES */
		if( dif )
		{
			dif = FALSE;
			lstrcpy( (LPSTR) gszSearchDir, lpm );
			gfChgMask = TRUE;
		}
		i++;
		if( SRSavFil( lpff, lpm ) )
		{
			lpff->ffSize = 0;
			lpff->ffAttr = 0;
			lpff->ffFNLen = 0;
			lpff->ffTime = 0;
			lpff->ffDate = 0;
			while( SRFindNext( lpgf ) )
			{
				if( !SRSavFil( lpff, lpm ) )
				{
					flg = FALSE;	/* Signal a PROBLEM ... */
					break;
				}
				i++;
			}
		}
		else
		{
			goto DnAllDir;
		}
		if( gfDoIter && DirCnt )	/* If we have some DIRECTORIES saved ... */
		{
			chkdir();
			if( cycs )
			{
				if( lpDir2 && SetNxtDir( lpm, lpDir2, lpDir ) )
				{
					cycs++;
					if( cycs == 0 )
					{
						goto DnAllDir;
					}
					goto DoNxtDir;
				}
				if( (NxtDirs != CurDirs) && hDirs[NxtDirs] && lpDir2 )
					SRUnlock( hDirs[NxtDirs] );
				lpDir2 = 0;
				if( (NxtDirs + 1) > CurDirs )
				{
					goto DnAllDir;
				}
				if( ( NxtDirs + 1 ) == CurDirs )	/* If reached current ... */
				{
					NxtDirs++;
					lpDir2 = lpDirs;
				}
				else
				{
					NxtDirs++;
					lpDir2 = SRLock( hDirs[NxtDirs] );
				}	
				cycs++;
				if( cycs == 0 )
				{
					goto DnAllDir;
				}
			}
			else
			{
				cycs++;
				if( NxtDirs == CurDirs )
				{
					lpDir2 = lpDirs;
				}
				else
				{
					lpDir2 = SRLock( hDirs[NxtDirs] );
				}
			}
			if( lpDir2 && SetNxtDir( lpm, lpDir2, lpDir ) )
			{
				goto DoNxtDir;
			}
		}
DnAllDir:
		SREndFil( i );
	}
	else	
	{
		/* NO FILES FOUND ... */

		if( ( hg = SRAlloc( GHND, MXLTXT ) ) && ( lps = SRLock( hg ) ) )
		{
			lstrcpy( lps, pRNoFil );
			i = lstrlen( lps );
			wsprintf( (lps+i), pRFilNm, lpm );
			SRError( lps, NULL );
		}
		else
		{
			SRError( pRNoFil, NULL );
		}
	}
 }
 else
 {
	SRErrorN( IDS_MEMFAILED, NULL );
 }

	if( hg && lps )
		SRUnlock( hg );

	if( hg )
		SRFree( hg );

	SRWait( FALSE, SRW_END );

	return( TRUE );

}

INT_PTR  SearchCommand( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR  flg = FALSE;
   DWORD    cmd = LOWORD(wParam);
	switch( cmd )
	{
	case IDM_FIND:
		if( doFindDir( hDlg, gszSearchDir ) )
      {

      }
		break;

  	case IDD_OK:
		flg = SearchOK( hDlg );
   	EndDialog( hDlg, flg ) ;
      flg = TRUE;
		break;

	case IDD_CANCEL:
   	EndDialog( hDlg, FALSE ) ;
      flg = TRUE;
		break;
   }	/* case of cmd (WPARAM) */

   return flg;

}

//---------------------------------------------------------------------------
// INT_PTR CALLBACK SEARCHDLGPROC(
//    HWND hDlg,  // handle to dialog box
//    UINT uMsg,     // message
//    WPARAM wParam, // first message parameter
//    LPARAM lParam ) // second message parameter
//
//  Description:
//     Simulates the Windows System Dialog Box.
//
//  Parameters:
//     Same as standard dialog procedures.
//
//  History:   Date       Author      Comment
//             Jan. 96    GeoffMc     Should be there for all.
//
//  Caller: From UINT SRSearch( HWND hwnd ) which is ONLY CALLED BY:
//		void	SROpenAs( HWND hwnd ) - Impliments IDM_OPENAS from MENU command
//
//---------------------------------------------------------------------------

//BOOL MEXPORTS SEARCHDLGPROC( HWND hDlg, UINT uMsg,
//                              WPARAM wP, LPARAM lParam )
INT_PTR CALLBACK SEARCHDLGPROC(
    HWND hDlg,  // handle to dialog box
    UINT uMsg,     // message
    WPARAM wParam, // first message parameter
    LPARAM lParam ) // second message parameter
{
	INT_PTR	flg = FALSE;
	switch (uMsg)
	{

	case WM_INITDIALOG:
		flg = SearchInit( hDlg, wParam, lParam );
		break;

	case WM_CLOSE:	/* 0x0010 */
      EndDialog( hDlg, FALSE ) ;
      flg = TRUE;
		break;

	case WM_COMMAND:
      flg = SearchCommand( hDlg, wParam, lParam );
		break;

	}	/* case of uMsg */

	return( flg );

} /* end of SEARCHDLGPROC() */

BYTE	GetB( LPSTR lps, DWORD off )
{
BYTE _huge *cp;
BYTE	c;
	cp = lps;
	c = cp[off];
return( c );
}

void	PutB( LPSTR lps, DWORD off, BYTE c )
{
BYTE _huge *cp;
	cp = lps;
	cp[off] = c;
}

LPSTR	GetPtr( LPSTR lps, DWORD off )
{
char _huge *cp;
char _huge *cp2;
	cp = lps;
	cp2 = cp + off;
return( cp2 );
}
#ifdef	WIN32
void CheckLongName( LPSTR lpsrc, DWORD csrc, DWORD msrc,
				   LPSTR lpdst, DWORD cdst )
{	// Moving has STOPPED because current char is LESS THAN
	// or EQUAL 20h, but there could be MORE ...
	char	c;
	DWORD	dw1, dw2, dw3;
	char	buf[MAX_PATH+1];
	int		i, stg;
	dw2 = 0;
	for( dw1 = csrc; dw1 < msrc; dw1++ )
	{
		c = lpsrc[dw1];
		if( c < 0x20 )
			break;
	}
	dw2 = dw1 - csrc;
	if( dw2 && (dw2 < MAX_PATH) )
	{
		dw2 += csrc;
		dw3 = 0;
		for( dw1 = csrc; dw1 < dw2; dw1++ )
		{
			c = lpsrc[dw1];
			buf[dw3++] = c;
		}
		buf[dw3] = 0;
		while( buf[dw3-1] <= ' ' )
		{
			dw3--;
			buf[dw3] = 0;
			if( dw3 == 0 )
				break;
		}
		// OK, we EXPECT a line like                    3
// c:\windows\Cristaux bleus.bmp         198 24/08/95 09:50
		if( (dw3 > 17) &&
			(buf[dw3-3] == ':') &&
			(buf[dw3-6] == ' ') &&
			(buf[dw3-9] == '/') &&
			(buf[dw3-12] == '/') &&
			(buf[dw3-15] == ' ') &&
			( (buf[dw3-16] >= '0') && (buf[dw3-16] <= '9') ) )
		{
			stg = 0;
			for( i = (int)(dw3 - 16); i >= 0; i-- )
			{
				c = buf[i];
				switch( stg )
				{
				case 0:
					{
						if( c == ' ' )
							stg = 1;
						break;
					}
				case 1:
					{
						if( c > ' ' )
						{
							i++;
							buf[i] = 0;
							stg = 2;
							break;
						}
					}
				}
				if( stg == 2 )
					break;
			}
			if( stg == 2 )
				lstrcat( lpdst, &buf[0] );
		}
	}
}
#endif	// WIN32

// WM_INITDIALOG
BOOL	Search2Init( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
BOOL	flg, flg2;
LPSTR	lps, lps2;
HGLOBAL	hg;
HFILE	hf;
LONG	lSiz;
int	i, j;
DWORD	rSiz, aSiz, bSiz, iOff;
BYTE	c, d;
DWORD dwIndex;
LRESULT	ddI;
	flg = TRUE;
	hf = 0;
	hg = 0;
	lps = 0;
	iOff = 0;
	if( ( hf = SROpen( &gszSchFil[0], (OF_READ | OF_SHARE_COMPAT) ) ) && 
		(hf != HFILE_ERROR) && (lSiz = DVFileSize( hf )) )
	{
		if( (hg = SRAlloc( GHND, (lSiz + MXLTXT + 2) )) &&
			( lps = SRLock( hg ) ) )
		{
			hStrings = hg;
			lps2 = lps + MXLTXT;
			lstrcpy( lps, pRTotCnt );
			if( i = lstrlen( lps ) )
			{
				wsprintf( lps2, lps, SrchCnt );
				SetDlgItemText( hDlg, IDD_COLCNT, lps2 ) ;
			}
			if( (rSiz = SRReadLong( hf, lps2, (DWORD) lSiz )) == (DWORD) lSiz )
			{
				j = 0;
				bSiz = 0;
				c = 0;
				for( aSiz = 0; aSiz < rSiz; aSiz++ )
				{
					d = c;
					c = GetB( lps2, aSiz );
					switch( j )
					{
						case 0:	/* Waiting for start of line ... */
						{
							if( c == ';' )
							{
								j = 1;
							}
							else if( c > 0x20 )
							{
								j = 2;	// START a FILE NAME ...
								PutB( lps, bSiz, c );
								bSiz++;
							}
							break;
						}
						case 1:	/* Waiting for End of Line ... */
						{
							if( c == 0x0a )
								j = 0;
							break;
						}
						case 2:	/* Filling in a LINE ... */
						{
							flg2 = FALSE;
							if( fShwAFil )
							{
								if( c < 0x20 )
								{
									flg2 = TRUE;
								}
							}
							else
							{
								if( c <= 0x20 )
								{
									flg2 = TRUE;
								}
							}
							if( flg2 )
							{
								j = 1;	/* Skip rest of the line ... */
								PutB( lps, bSiz, 0 );	/* Zero terminate entry */
								bSiz++;
//#ifdef	WIN32
//								CheckLongName( lps2, aSiz, rSiz, lps, bSiz );
//#endif	// WIN32
								dwIndex = SendDlgItemMessage(hDlg, IDD_MO_NAME,
									LB_ADDSTRING, 0, (LPARAM) lps);
								if( (dwIndex == LB_ERR) || (dwIndex == LB_ERRSPACE) )
								{
									if( dwIndex == LB_ERR )
										SRError( pRErr1, NULL );
									else
										SRError( pRErr2, NULL );
									flg = FALSE;
									break;
								}
								ddI = SendDlgItemMessage( hDlg, IDD_MO_NAME,
									LB_SETITEMDATA,
									(WPARAM) LOWORD (dwIndex),
									(LPARAM) iOff );
								if( ddI == LB_ERR )
								{
									SRError( pRErr6, NULL );
									flg = FALSE;
									break;
								}
								lps = GetPtr( lps, bSiz );
								iOff += bSiz;	/* Get the NEXT OFFSET into buffer */
								bSiz = 0;
							}
							else
							{
								if( c == 'D' )
								{
									if( d == '<' )
									{
/* ========== Do NOT add '<D' (ie Directories) to the LIST ========= */
										j = 1;	/* Skip this WHOLE line ... */
										bSiz = 0;	/* Nothing accumulated ... */
									}
									else
									{
										PutB( lps, bSiz, c );
										bSiz++;
									}
								}
								else
								{
									PutB( lps, bSiz, c );
									bSiz++;
								}
							}
						}
					}	/* Switch case 0, 1, 2 ... */
					if( !flg )
						break;
				}	/* For aSiz < rSiz loop */
			}
		}
	}
	else
	{
		SRError( pRErr3, NULL );
		flg = FALSE;
	}
	if( hf )
		SRClose( lpSavFil, hf );
	if( hg && lps )
		SRUnlock( hg );
	if( !flg && hg )
	{
		SRFree( hg );
		hStrings = 0;
	}
return( flg );
}

BOOL	Search2OK( HWND hDlg )
{
BOOL	flg;
	flg = TRUE;
return( flg );
}

/* Delete selected items - handle IDD_DELETE */
BOOL	DelSrchSel( HWND hDlg )
{
BOOL	flg, flg2;
WORD	i, j, k, l, sel1, sel2;
LRESULT	res;
HGLOBAL	hg;
int MLPTR lps;
LPSTR	lpb, lpb2;
PINT8	lpstg;
PINT8	lpfil;
DWORD	bSiz, iOff;
	flg = TRUE;
	hg = 0;
	lps = 0;
	lpb = 0;
	iOff = 0;
	if( sel1 = LOWORD ( SendDlgItemMessage(hDlg, IDD_MO_NAME,
		LB_GETSELCOUNT, 0, 0L ) ) )
	{
		bSiz = (DWORD) ((sel1 + 1) * sizeof( int ));
		if( hStrings &&
			(lpstg = SRLock( hStrings )) &&
			( hg = SRAlloc( GHND, (bSiz + (MXLTXT * 2) + 1) ) ) &&
			( lpb = SRLock( hg ) ) )
		{
			lps = (int MLPTR) (lpb + ((MXLTXT * 2) + 1));
			lpb2 = lpb + MXLTXT;
			sel2 = LOWORD ( SendDlgItemMessage( hDlg, IDD_MO_NAME,
					LB_GETSELITEMS, (WPARAM) sel1, (LPARAM) lps  ) );
			if( j = (sel2 - 1) )
			{
NxtSort:
				flg2 = FALSE;
				for( i = 0; i < j; i++ )
				{
					if( (k = lps[i]) < (l = lps[i+1]) )
					{
						lps[i] = l;
						lps[i+1] = k;
						flg2 = TRUE;
					}
				}
				if( flg2 )
				{
					j--;
					if( j )
						goto NxtSort;
				}
			}
			wsprintf( lpb, pRQuer1, sel2 );
			if( SRQuery( lpb ) )
			{
				for( i = 0; i < sel2; i++ )
				{
					iOff = SendDlgItemMessage( hDlg, IDD_MO_NAME, LB_GETITEMDATA,
						(WPARAM) lps[i], 0L );
					if( iOff == LB_ERR )
						break;
					lpfil = GetPtr( lpstg, iOff );
					lpfil[0] = DELMARK;	/* Set as DELETED ... */
					res = SendDlgItemMessage( hDlg, IDD_MO_NAME, LB_DELETESTRING,
						(WPARAM) lps[i], 0L );
					if( res == LB_ERR )
						break;
				}
				if( !( (i == sel2) && (sel1 == sel2) && 
					((SrchCnt - LOWORD( res )) == sel1 )) )
				{
					SRError( pRWarn2, NULL );
				}
				SrchCnt = LOWORD( res );	/* Get remainder ... */
				lstrcpy( lpb, pRTotCnt );
				if( i = lstrlen( lpb ) )
				{
					wsprintf( lpb2, lpb, SrchCnt );
					SetDlgItemText( hDlg, IDD_COLCNT, lpb2 ) ;
				}
			}
		}	/* Memory allocation ... */
		else
		{
			SRErrorN( IDS_MEMFAILED, NULL );
		}
	}
	else
	{
		SRError( pRErr4, NULL );
		flg = FALSE;
	}	
	if( hg && lps )
		SRUnlock( hg );
	if( hg )
		SRFree( hg );
return( flg );
}

BOOL	RestSrch( HWND hDlg )
{
BOOL	flg;
PINT8	lps;
PINT8	lps2;
UINT	i, j;
DWORD	iOff, dInd, dI;
BYTE	c;
LPSTR	lpb;
	flg = TRUE;
	iOff = 0;	/* Start the OFFSET ... */
	lps2 = 0;
	if( OrgSCnt && hStrings && (lps2 = SRLock( hStrings ) ) )
	{
		lps = lps2;
		SendDlgItemMessage( hDlg, IDD_MO_NAME, LB_RESETCONTENT, 0, 0L );
		for( i = 0; i < OrgSCnt; i++ )
		{
			if( (c = lps[0]) == DELMARK )
				lps[0] = ActDrv;
			dInd = SendDlgItemMessage(hDlg, IDD_MO_NAME,
					LB_ADDSTRING, 0, (LPARAM) lps);
			if( (dInd == LB_ERR) || (dInd == LB_ERRSPACE) )
			{
				if( dInd == LB_ERR )
					SRError( pRErr1, NULL );
				else
					SRError( pRErr2, NULL );
				flg = FALSE;
				break;
			}
			dI = SendDlgItemMessage( hDlg, IDD_MO_NAME,
				LB_SETITEMDATA,
				(WPARAM) LOWORD (dInd),
				(LPARAM) iOff );
			if( dI == LB_ERR )
			{
				SRError( pRErr6, NULL );
				flg = FALSE;
				break;
			}
			j = lstrlen( lps ) + 1;	/* Length + the nul */
			iOff += (DWORD) j;
			lps = GetPtr( lps2, iOff );	/* Move up to NEXT Entry ... */
		}
	}
	if( i != OrgSCnt )
	{
		SRError( pRWarn2, NULL );
	}
	SrchCnt = OrgSCnt;
	lpb = &gszTpBuf1[0];
	if( i = lstrlen( pRTotCnt ) )
	{
		wsprintf( lpb, pRTotCnt, SrchCnt );
		SetDlgItemText( hDlg, IDD_COLCNT, lpb ) ;
	}
	if( hStrings && lps2 )
		SRUnlock( hStrings );
return( flg );
}

// handle IDD_SELECTED
BOOL	UseSrchSel( HWND hDlg )
{
BOOL	flg, flg2;
WORD	i, j, k, l, sel1, sel2;
LRESULT	res;
HGLOBAL	hg;
int MLPTR	lps;
LPSTR	lpb, lpb2;
PINT8	lpstg;
PINT8	lpfil;
DWORD	bSiz, iOff, ddI;
	flg = TRUE;
	hg = 0;
	lps = 0;
	lpb = 0;
	iOff = 0;
	if( sel1 = LOWORD ( SendDlgItemMessage(hDlg, IDD_MO_NAME,
		LB_GETSELCOUNT, 0, 0L ) ) )
	{
		bSiz = (DWORD) ((sel1 + 1) * sizeof( int ));
		if( hStrings &&
			(lpstg = SRLock( hStrings )) &&
			( hg = SRAlloc( GHND, (bSiz + (MXLTXT * 2) + 1) ) ) &&
			( lpb = SRLock( hg ) ) )
		{
			lps = (int MLPTR) (lpb + ((MXLTXT * 2) + 1));
			lpb2 = lpb + MXLTXT;
			sel2 = LOWORD ( SendDlgItemMessage( hDlg, IDD_MO_NAME,
					LB_GETSELITEMS, (WPARAM) sel1, (LPARAM) lps  ) );
			if( j = (sel2 - 1) )
			{
NxtSort2:
				flg2 = FALSE;
				for( i = 0; i < j; i++ )
				{
					if( (k = lps[i]) < (l = lps[i+1]) )
					{
						lps[i] = l;
						lps[i+1] = k;
						flg2 = TRUE;
					}
				}
				if( flg2 )
				{
					j--;
					if( j )
						goto NxtSort2;
				}
			}
			wsprintf( lpb, pRQuer2, sel2 );
			if( SRQuery( lpb ) )
			{
				for( i = 0; i < sel2; i++ )
				{
					iOff = SendDlgItemMessage( hDlg, IDD_MO_NAME, LB_GETITEMDATA,
						(WPARAM) lps[i], 0L );
					if( iOff == LB_ERR )
						break;
					lpfil = GetPtr( lpstg, iOff );
					lpfil[0] = SELMARK;	/* Set as DELETED ... */
				}
				if( !( (i == sel2) && (sel1 == sel2) ) )
				{
					SRError( pRWarn2, NULL );
				}
				SrchCnt = sel2;	/* SET remainder ... */
				if( i = lstrlen( pRTotCnt ) )
				{
					wsprintf( lpb, pRTotCnt, SrchCnt );
					SetDlgItemText( hDlg, IDD_COLCNT, lpb ) ;
				}
				iOff = 0;
				for( i = 0; i < OrgSCnt; i++ )
				{
					lpfil = GetPtr( lpstg, iOff );
					if( lpfil[0] != SELMARK )
						lpfil[0] = DELMARK;
					iOff += (DWORD) ( lstrlen( lpfil ) + 1 );
				}
				SendDlgItemMessage( hDlg, IDD_MO_NAME, LB_RESETCONTENT, 0, 0L );
				iOff = 0;
				for( i = 0; i < OrgSCnt; i++ )
				{
					lpfil = GetPtr( lpstg, iOff );
					if( lpfil[0] == SELMARK )
					{
						lpfil[0] = ActDrv;
						res = SendDlgItemMessage( hDlg, IDD_MO_NAME,
							LB_ADDSTRING, 0, (LPARAM) lpfil );
						if( (res == LB_ERR) || (res == LB_ERRSPACE) )
						{
							if( res == LB_ERR )
								SRError( pRErr1, NULL );
							else
								SRError( pRErr2, NULL );
							flg = FALSE;
							break;
						}
						ddI = SendDlgItemMessage( hDlg, IDD_MO_NAME,
								LB_SETITEMDATA,
								(WPARAM) LOWORD (res),
								(LPARAM) iOff );
						if( ddI == LB_ERR )
						{
							SRError( pRErr6, NULL );
							flg = FALSE;
							break;
						}
					}	/* If it has a SELMARK, add to LIST BOX ... */
					iOff += (DWORD) (lstrlen( lpfil ) + 1);	/* Bump to NEXT */
				}	/* Do full list in the buffer */
				if( i != OrgSCnt )
				{
					SRError( pRWarn2, NULL );
				}
			}	/* Query to proceed ... */
		}	/* Memory allocation ... */
		else
		{
			SRErrorN( IDS_MEMFAILED, NULL );
		}
	}
	else
	{
		SRError( pRErr4, NULL );
		flg = FALSE;
	}	
	if( hg && lps )
		SRUnlock( hg );
	if( hg )
		SRFree( hg );
return( flg );
}

BOOL MEXPORTS SEARCH2DLGPROC( HWND hDlg, UINT uMsg,
                              WPARAM wP, LPARAM lParam )
{
	BOOL	flg;
	flg = FALSE;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		flg = Search2Init( hDlg, wP, lParam );
		if( !flg )
			PostMessage( hDlg, WM_COMMAND, IDD_CANCEL, 0L );
		break;
	}
	case WM_CLOSE:	/* 0x0010 */
	{
		EndDialog( hDlg, TRUE ) ;
		flg = TRUE;
		break;
	}
	case WM_COMMAND:
	{
      DWORD cmd = LOWORD(wP);
		switch( cmd )
		{
		case IDD_SELECTED:
			UseSrchSel( hDlg );
			break;
		case IDD_RESTORE:
			RestSrch( hDlg );
			break;
		case IDD_DELETE:
			DelSrchSel( hDlg );
			break;
        case IDD_OK:
			flg = Search2OK( hDlg );
			SRTermSearch();
        	EndDialog( hDlg, flg ) ;
			break;
		case IDD_CANCEL:
			SRCloseSch();
        	EndDialog( hDlg, flg ) ;
        	flg = TRUE;
			break;
         }
         break;
	}
	}
return( flg );
} /* end of SEARCH2DLGPROC() */

UINT	SRSearch2( HWND hwnd )
{
int	i, j;
	if( fShwAFil )
		j = SRSEARCH3;
	else
		j = SRSEARCH2;
	i = GoModalDialogBoxParam( GETHINST( hwnd ),
		MAKEINTRESOURCE( j ),
		hwnd,
		SEARCH2DLGPROC,
		(LPARAM) NULL );
	if( i == (int) -1 )
	{
		SRError( pRErr5, NULL );
	}
return( SrchCnt );
}

/*	=====================================================
	UINT	SRSearch( HWND hwnd )

	ONLY CALLED BY:
		void	SROpenAs( HWND hwnd ) - Impliments IDM_OPENAS from MENU command

	=====================================================	*/
UINT	SRSearch( HWND hwnd )
{
	int	i;

	hParen = hwnd;
   // DialogBoxParam()
	i = GoModalDialogBoxParam( GETHINST( hwnd ),
		MAKEINTRESOURCE( SRSEARCH ),	/* IDM_OPENAS */
		hwnd,
		SEARCHDLGPROC,
		(LPARAM) NULL );

	if( i == (int) -1 )
	{
		SRError( pRErr5, NULL );
	}
	if( SrchCnt )
	{
		OrgSCnt = SrchCnt;	/* Keep the ORIGINAL Count in the LIST */
		i = (int) SRSearch2( hwnd );
		if( i == (int) -1 )
		{
			SRError( pRErr5, NULL );
		}
	}
   return( SrchCnt );
}

void	SRTermSearch( void )
{
   UINT	i;
	if( hDirs[CurDirs] && lpDirs )
		SRUnlock( hDirs[CurDirs] );
	lpDirs = 0;
	if( hDirs[CurDirs] )
		SRFree( hDirs[CurDirs] );
	hDirs[CurDirs] = 0;
	for( i = 0; i < CurDirs; i++ )
	{
		if( hDirs[i] )
			SRFree( hDirs[i] );
	}
}

//void	SRInitSch( LPSTR lpcd )
//{
	/* set the DEFAULT drive, directory, and search mask */
//	SRSetupPath( gszSchFil, lpcd, (LPSTR) &szDefSch[0] );
//	SRSetupPath( gszSchFil, lpcd, &szDefSch[0] );
//}

void SRCloseSch( void )
{
	SRTermSearch();
	if( hStrings )
		SRFree( hStrings );
	hStrings = 0;
	SrchCnt = 0;
}

LPSTR	SRGetSFil( UINT ind )
{
LPSTR	lpf, lpb;
UINT	i, cnt, j, k;
DWORD	iOff;
PINT8	lps;
BYTE	c;
	lpb = lpf = 0;
	iOff = 0;
	cnt = 0;
	if( (ind < SrchCnt) && OrgSCnt && hStrings && (lpb = SRLock( hStrings ) ) )
	{
		for( i = 0; i < OrgSCnt; i++ )
		{
			lps = GetPtr( lpb, iOff );
			if( lps[0] != DELMARK )
			{
				if( cnt == ind )		/* If this is the FILE ... */
				{
					if( j = lstrlen( lps ) )
					{	
						lpf = &gszTpBuf1[0];
						for( k = 0; k < j; k++ )
						{
							c = lps[k];
							if( c <= ' ' )
							{
								lpf[k] = 0;
#ifdef	WIN32
								CheckLongName( lps, k, j, lpf, k );
#endif	// WIN32
								break;
							}
							else
							{
								lpf[k] = c;
							}
						}
					}
					break;
				}
				cnt++;
			}
			iOff += (DWORD) (lstrlen( lps ) + 1);
		}	/* Cycle through the FILE STRING BUFFER ... */
	}	/* If things 'look' OK ... */
	if( lpb )
		SRUnlock( hStrings );
return( lpf );
}

/* ************************************************************************* */

#endif   // #ifdef	ADDOPENALL

#endif   // #ifdef  USELLIST2 y/n  // FIX20001210 - switch to using a LINKED LIST

//char	szFStr1[] = "All (*.*)";
//char	szFStr2[] = "*.*";
static char szFStr[] = "All (*.*)\0*.*\0";
static OPENFILENAME _s_ofn;

/* User wants some assistance in FINDING the desired directory ... */
BOOL  doFindDir( HWND hDlg, LPTSTR lpf )
{
   BOOL           flg  = FALSE;
   LPOPENFILENAME pofn = &_s_ofn;
//   LPSTR	lpf, lpt, lpf2;
   LPTSTR         lpt;
//   DWORD	i, j;
   HGLOBAL	      hgB;
   LPTSTR         lpB1, lpB2, lpB3, lpB4;

//   hgB = 0;
//	j = 0;
//	lpB1 = lpB2 = lpB3 = lpB4 = 0;
	lpt  = &gszTpBuf1[0];
	//lpf2 = &gszTpBuf2[0];
	//lpf  = &gszSearchDir[0];
	lstrcpy( lpt, lpf );
	//lstrcpy( lpf2, &szFStr1[0] );
	//i = lstrlen( &szFStr1[0] ) + 1;
	//lstrcpy( (lpf2+i), &szFStr2[0] );
	//i += lstrlen( &szFStr2[0] ) + 1;
	//lpf2[i] = 0;
	//if( ( hgB = DVGlobalAlloc( GHND, (260*4) ) ) &&
	if( ( hgB = DVGAlloc( "doFindDir", GHND, (260*4) ) ) &&
		 ( lpB1 = DVGlobalLock( hgB )                   ) )
	{
		lpB2 = lpB1 + 260;
		lpB3 = lpB2 + 260;
		lpB4 = lpB3 + 260;
		DVGetFPath( lpf, lpB1, lpB2, lpB3, lpB4 );
		lstrcpy( lpt, lpB1 );
		lstrcat( lpt, lpB2 );
		lstrcat( lpt, pRGlob );
		lstrcat( lpB1, lpB2 );
		//j = 260;
	}
   else
   {
      if( hgB )
         DVGlobalFree(hgB);
      return FALSE;
   }

   ZeroMemory( pofn, sizeof(OPENFILENAME) );
   pofn->lStructSize       = sizeof( OPENFILENAME );
	pofn->hwndOwner         = hDlg;
	pofn->hInstance         = ghDvInst;
	pofn->lpstrFilter       = szFStr;  //lpf2;
	//ofn.lpstrCustomFilter = lpB2;
	//ofn.nMaxCustFilter    = 260;
	pofn->nFilterIndex      = 1;
	pofn->lpstrFile         = lpt;     // current mask
	pofn->nMaxFile          = MXLTXT;
	//ofn.lpstrFileTitle    = NULL;
	//ofn.nMaxFileTitle     = 0;
	pofn->lpstrInitialDir   = lpB1;
	pofn->lpstrTitle        = "ENTER SEARCH DIRECTORY AND MASK";
	pofn->Flags             = OFN_HIDEREADONLY | OFN_NOTESTFILECREATE |
		OFN_NOVALIDATE | OFN_PATHMUSTEXIST;
	//ofn.nFileOffset       = 0;
	//ofn.nFileExtension    = 0;
	//ofn.lpstrDefExt       = NULL;
	//ofn.lCustData         = (LPARAM) NULL;
	//ofn.lpfnHook          = NULL;
	//ofn.lpTemplateName    = NULL;
	if( GetSaveFileName( pofn ) )
	{
		if( lstrcmpi( lpt, lpf ) )
		{
			lstrcpy( lpf, lpt );
			//SetDlgItemText( hDlg, IDM_SELECT, lpf );
			//i = 0;
         flg = TRUE;
		}
	}

	if( hgB && lpB1 )
		DVGlobalUnlock( hgB );

	if( hgB )
		DVGlobalFree( hgB );

   return flg;
}

// ===================================================================
// IDM_OPENMRU -> IDD_OPENMRU DIALOG SERVICES
// ******************************************
typedef  struct   tagOMRU {
   PLIST_ENTRY om_pList;
   HWND        om_hDlg;
   HWND        om_hList;
   HWND        om_hFrame;
   HWND        om_hOK;
   RECT        om_rcFrame;
   DWORD       om_dwFlag;
   DWORD       om_dwSel;
   LRESULT     om_dwIndex;
   PMWL        om_pmwl;
   BOOL        om_bFile;   // when file is VALID
}OMRU, *POMRU;

static   TCHAR szNoImg[] = "No Image Available";

VOID  PutNoImageText( HDC hdc, PRECT prc )
{
   FillRect(hdc, prc, GetStockObject(BLACK_BRUSH) );
   SetTextColor(hdc, RGB(255,255,255));
   SetBkColor(hdc, RGB(0,0,0));
   DrawText( hdc, // handle to DC
      szNoImg,   // "No image available", // text to draw
      strlen(szNoImg), // text length
      prc,   // formatting dimensions
      DT_SINGLELINE | DT_VCENTER | DT_CENTER ); // text-drawing options
}

BOOL  PutSelString2( HWND hDlg, POMRU pom )
{
   BOOL     bRet = FALSE;
   PMWL     pmwl;
   HDC      hdc;
   LPTSTR   lpf;
   LPTSTR   lptmp;

   if( !pom )
      return FALSE;

   pom->om_bFile = FALSE;
   if( !( pmwl = pom->om_pmwl ) )
      return FALSE;

   lptmp = &gszTmpBuf1[0]; // GetTmp2();
   lpf   = &pmwl->wl_szFile[0];
#define  rcFrame     pom->om_rcFrame

   *lptmp = 0;
   SetDlgItemText(hDlg, IDD_WIDTH,  lptmp);
   SetDlgItemText(hDlg, IDD_HEIGHT, lptmp);
   SetDlgItemText(hDlg, IDD_COLORS, lptmp);

   hdc = GetDC( pom->om_hFrame );
   GetClientRect( pom->om_hFrame, &rcFrame );
   if( lpf && *lpf )
   {
      WIN32_FIND_DATA   fd;
      if( IsValidFile( lpf, &fd ) & IS_FILE )
      {
         LARGE_INTEGER li;
         pom->om_bFile = TRUE;
         bRet = TRUE;
         li.LowPart  = fd.nFileSizeLow;
         li.HighPart = fd.nFileSizeHigh;
         wsprintf( lptmp,
                  "%s - %s bytes at %s",
                  lpf,
                  GetI64Stg( &li ),
                  GetFDTStg( &fd.ftLastWriteTime ) );
         if( hdc )
         {
            HANDLE   hDIB;
            LPSTR    lpDIB;
            int      ir = GDI_ERROR;
                        if( hDIB = GetDIB( lpf, gd_DP ) )
                        {
                           if( lpDIB = DVGlobalLock(hDIB) )
                           {
                              gdwDIBWidth  = DIBWidth(lpDIB);
                              gdwDIBHeight = DIBHeight(lpDIB);
                              gdwDIBBPP    = DIBBitCount(lpDIB);
                        	   SetStretchBltMode( hdc, COLORONCOLOR );
                              ir = StretchDIBits( hdc,		// hDC
                                 0,		// DestX
                                 0,    // DestY
                                 rcFrame.right,    // nDestWidth
                                 rcFrame.bottom,   // nDestHeight
                                 0,    // SrcX
                                 0,    // SrcY
                                 gdwDIBWidth,   // wSrcWidth
                                 gdwDIBHeight,  // wSrcHeight
                                 FindDIBBits(lpDIB),  
                                 (LPBITMAPINFO)lpDIB, // lpBitsInfo
                                 DIB_RGB_COLORS,		// wUsage
                                 SRCCOPY );
                              DVGlobalUnlock(hDIB);
                           }
                           DVGlobalFree(hDIB);
                        }
            if( ir == GDI_ERROR )
            {
               PutNoImageText( hdc, &rcFrame );
            }
            else
            {
                           wsprintf( lptmp, "Width %d pixels", gdwDIBWidth );
                           SetDlgItemText(hDlg, IDD_WIDTH,  lptmp);
                           wsprintf( lptmp, "Height %d pixels", gdwDIBHeight );
                           SetDlgItemText(hDlg, IDD_HEIGHT,  lptmp);
                           wsprintf( lptmp, "%d BPP", gdwDIBBPP );
                           SetDlgItemText(hDlg, IDD_COLORS,  lptmp);

            }
         }
      }
      else
      {
         wsprintf( lptmp,
                  "%s - *** IS NOT A VALID FILE! ***",
                  lpf );
         if( hdc )
            PutNoImageText( hdc, &rcFrame );
      }

   }
   else
   {
         if( hdc )
            PutNoImageText( hdc, &rcFrame );
   }
   if( hdc )
      ReleaseDC( pom->om_hFrame, hdc );

   // set the "selected" string
   SetDlgItemText( hDlg, IDC_EDIT2, lptmp );

   return bRet;

}

DWORD  PopulateListBox( HWND hDlg, POMRU pom )
{
   PLIST_ENTRY pNext;
   LRESULT  lRes;
   PMWL     pmwl;
   LPTSTR   lpf;
   HWND     hList;
   PLIST_ENTRY pHead;
   DWORD    dwc1 = 0;

   if( ( pom ) &&
       ( pHead = pom->om_pList ) &&
       ( hList = pom->om_hList ) )
   {
      lRes = SendMessage( hList,
		   LB_RESETCONTENT,
		   0, 0 );
      Traverse_List( pHead, pNext )
      {
         pmwl = (PMWL)pNext;
         lpf  = &pmwl->wl_szFile[0];
		   if( !( pmwl->wl_dwFlag & flg_ToDelete ) )
		   {
            // add to list
            lRes = SendMessage( hList,
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
				   SendMessage( hList,
					   LB_SETITEMDATA,
					   (WPARAM)lRes,
					   (LPARAM)pNext );
               if( pom->om_dwSel == dwc1 )
               {
                  pom->om_dwIndex = lRes;
                  pom->om_pmwl    = pmwl;
               }
               dwc1++;
			   }
		   }
      }

      if( dwc1 )
      {
         if( !pom->om_pmwl )
         {
            pom->om_pmwl = (PMWL)pHead;
            pom->om_dwIndex = 0;
            pom->om_dwSel  = 0;
         }

	      lRes = SendMessage( hList,
			   LB_SETCURSEL,
			   pom->om_dwIndex,
			   0 );

         if( lRes != LB_ERR )
         {
            if( PutSelString2( hDlg, pom ) )
               EnableWindow( pom->om_hOK, TRUE );

         }
         else
         {
            pom->om_pmwl = (PMWL)pHead;
            pom->om_dwIndex = 0;
            pom->om_dwSel  = 0;
	         lRes = SendMessage( hList,
			      LB_SETCURSEL,
			      pom->om_dwIndex,
			      0 );
            if( PutSelString2( hDlg, pom ) )
               EnableWindow( pom->om_hOK, TRUE );
         }
      }
   }

   return dwc1;

}

INT_PTR  OMRU_Init( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
   INT_PTR     iRet = TRUE;
   POMRU       pom  = (POMRU)lParam;
   PLIST_ENTRY pHead, pNext;
   HWND        hList, hFrame, hOK;
   PMWL        pmwl;

   pom->om_hDlg = hDlg;
   SET_PROP( hDlg, ATOM_MRU, pom );

   if( ( pHead = pom->om_pList ) &&
       ( hList = GetDlgItem( hDlg, IDC_LIST1 ) ) &&
       ( hFrame = GetDlgItem( hDlg, IDC_PREVIEW ) ) &&
       ( hOK    = GetDlgItem( hDlg, IDOK )        ) )
   {
      pom->om_hList  = hList;
      pom->om_hFrame = hFrame;
      pom->om_hOK    = hOK;
      Traverse_List( pHead, pNext )
      {
         pmwl = (PMWL)pNext;
         pmwl->wl_dwFlag &= ~(flg_ToDelete);
      }
      pom->om_pmwl = (PMWL)pHead;
      pom->om_dwIndex = 0;

      EnableWindow( hOK, FALSE );

      PopulateListBox( hDlg, pom );

   }
   else
   {
      EndDialog(hDlg, -1);
      iRet = -1;
   }

   return iRet;
}

INT_PTR  OMRU_GetSel( HWND hDlg, HWND hWnd, POMRU pom )
{
   INT_PTR  iRet = FALSE;
   LRESULT  lRes = SendMessage( hWnd,  // handle to destination window
         LB_GETITEMDATA,         // message to send
         SendMessage( hWnd, LB_GETCURSEL, 0, 0 ),    // item index
         0 );     // not used; must be zero
   if( lRes != LB_ERR )
   {
      pom->om_pmwl = (PMWL)lRes;
      PutSelString2( hDlg, pom );
      if( pom->om_bFile )
         iRet = TRUE;
   }
   else
   {
      sprtf( "ERROR: Failed to get SELECTION from %#x (%#x)."MEOR,
         hWnd,
         pom->om_hList );
   }

   return iRet;

}

// Parameters
// wParam
// The low-order word is the list box identifier.
// The high-order word is the notification message.
// lParam
// Handle to the list box.
VOID  OMRU_Notify( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
   POMRU pom;
   DWORD code = HIWORD(wParam);
   HWND  hWnd = (HWND)lParam;
   if( ( code == LBN_SELCHANGE ) &&
       ( pom = GET_PROP( hDlg, ATOM_MRU ) ) )
   {
      if( OMRU_GetSel( hDlg, hWnd, pom ) )
         EnableWindow( pom->om_hOK, TRUE );
      else
         EnableWindow( pom->om_hOK, FALSE );
   }
   else
   {

   }
}

INT_PTR  OMRU_Paint( HWND hDlg )
{
   INT_PTR  flg = TRUE;
   POMRU pom;
   PAINTSTRUCT ps;
   HDC         hDC;

   hDC = BeginPaint(hDlg,&ps);
   if( pom = GET_PROP( hDlg, ATOM_MRU ) )
   {
      PutSelString2( hDlg, pom );
   }

   EndPaint(hDlg, &ps);

   return flg;

}

INT_PTR  OMRU_OK( HWND hDlg )
{
   INT_PTR  iRet = FALSE;
   POMRU pom;
   if( pom = GET_PROP( hDlg, ATOM_MRU ) )
   {
      if( OMRU_GetSel( hDlg, pom->om_hList, pom ) )
         iRet = TRUE;
   }
   return iRet;
}

INT_PTR  OMRU_Command( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
   INT_PTR  iRet = FALSE;
   DWORD    cmd = LOWORD(wParam);
   switch(cmd)
   {

   case IDOK:
      if( OMRU_OK( hDlg ) )
      {
         EndDialog(hDlg, TRUE);
         iRet = TRUE;
      }
      break;
   case IDCANCEL:
      EndDialog(hDlg, FALSE);
      iRet = TRUE;
      break;

   case IDC_LIST1:
      OMRU_Notify( hDlg, wParam, lParam );
      iRet = TRUE;
      break;

   }

   return iRet;
}


INT_PTR CALLBACK OPENMRUDLGPROC(
    HWND hDlg,  // handle to dialog box
    UINT uMsg,     // message
    WPARAM wParam, // first message parameter
    LPARAM lParam ) // second message parameter
{
	INT_PTR	flg = FALSE;
	switch (uMsg)
	{

	case WM_INITDIALOG:
		flg = OMRU_Init( hDlg, wParam, lParam );
		break;

	case WM_CLOSE:	/* 0x0010 */
      EndDialog( hDlg, FALSE ) ;
      flg = TRUE;
		break;

	case WM_COMMAND:
      flg = OMRU_Command( hDlg, wParam, lParam );
		break;

   case WM_DESTROY:
      REMOVE_PROP( hDlg, ATOM_MRU );
      flg = TRUE;
      break;

   case WM_PAINT:
      flg = OMRU_Paint( hDlg );
      break;

	}	/* case of uMsg */

	return( flg );

} /* end of OPENMRUDLGPROC() */


VOID  Do_IDM_OPENMRU( HWND hWnd )
{
   PLIST_ENTRY pHead = &gsFileList;
   INT_PTR     ui;
   POMRU       pom;
   DWORD       dws;

   dws = sizeof(OMRU);
   if( ( !IsListEmpty( pHead ) ) &&
       ( pom = (POMRU)DVGAlloc( "sOMRU", GPTR, dws ) ) )
   {
      ZeroMemory( pom, dws );
      pom->om_pList = pHead;
      pom->om_pmwl  = (PMWL)pHead;
      // DialogBoxParam()
	   ui = DialogBoxParam( ghDvInst,
			MAKEINTRESOURCE(IDD_OPENMRU),
			hWnd,
			OPENMRUDLGPROC,
         (LPARAM) pom );
      if( ( ui ) &&
          ( ui != -1 ) )
      {
         if( ( pom->om_pmwl ) &&
             ( pom->om_bFile ) )
         {
            OpenMRUFile( hWnd, pom->om_pmwl );
         }
      }

      DVGlobalFree(pom);

   }

}

// ******************************************

/* eof */

