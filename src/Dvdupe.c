

// DvDupe.c

#include	"dv.h"
//#include	"DvInfo.h"	// And information BLOCK (about object)

extern	LPSTR	GetFNBuf( void );
extern	int		GetUCCnt( LPDIBINFO lpdi, LPSTR lpb, int Cols, DWORD Size,
						 int Wid, int Height, DWORD wBPP, LPSTR lpDIB );
extern	HANDLE	CopyHandle( HANDLE );
extern	BOOL	CenterWindow(HWND hwndChild, HWND hwndParent);
extern	DWORD	CalcDIBColors( LPSTR lpbi );
extern	void	Do_HELP( UINT uHelp, UINT dwData );
extern	HANDLE	BitmapToDIB2( HBITMAP hBitmap, HPALETTE hPal,
					int	iBPP );
extern	BOOL	LIBGifNBmp( HGLOBAL, DWORD, HGLOBAL, HGLOBAL, WORD );
extern	void	DumpDIB( HANDLE );
extern	HGLOBAL	ghCOLR;
extern   TCHAR gszRPFrm[];    // default FORMAT string = "TEMPB%03d.BMP"

typedef	struct	{
	UINT	cb_ID;
	LPSTR	cb_Stg;
}CBSTR;
typedef CBSTR FAR * LPCBSTR;

CBSTR	CbStr[] = {
	{ IDC_RADIO1, "Exactly the same" },
	{ IDC_RADIO2, "As Monochrome" },
	{ IDC_RADIO3, "As 16-Color Bitmap" },
	{ IDC_RADIO4, "As 256-Color Bitmap" },
	{ IDC_RADIO5, "As 24-bit Bitmap" },
	{ 0, 0 }
};

#ifndef  USEITHREAD
/* used for the duplicate dialog box */
static LPISTR	lpDIStr;
/* ================================= */
#endif   // ifndef  USEITHREAD

LPSTR	GetCBStg( UINT ui )
{
	UINT	cui;
	LPCBSTR	lpcb;
	LPSTR	lps = 0;

	lpcb = &CbStr[0];
	while( cui = lpcb->cb_ID )
	{
		if( cui == ui )
		{
			lps = lpcb->cb_Stg;
			break;
		}
		lpcb++;
	}
	return lps;
}

void	SetDupStg( LPISTR lpi )
{
	LPSTR	lps;
	HWND	hDlg;
	if( lpi &&
		( hDlg = lpi->hDlg ) &&
		( lps = GetCBStg( lpi->uActive ) ) )
	{
		SetDlgItemText( hDlg,
			IDC_EDIT6,
			lps );
	}
}

void	SetEdit1( HWND hDlg, LPSTR lpf )
{
	LPSTR	lps;
	int		i, j, k;
	char	szbuf[MAX_PATH+16];

	if( hDlg &&
		( lps = lpf ) &&
		( i = lstrlen( lps ) ) )
	{
		if( i > 32 )
		{
			k = 0;
			lps = &szbuf[0];
			for( j = 0; j < 14; j++ )
			{
				// Get first 14 characters
				lps[k++] = lpf[j];
			}
			lps[k] = 0;
			lstrcat( lps, "..." );
			k = lstrlen( lps );
			for( j = (i - 14); j < i; j++ )
			{
				// Get last 14 characters
				lps[k++] = lpf[j];
			}
			lps[k] = 0;
		}
		SetDlgItemText( hDlg,
			IDC_EDIT1,
			lps );
	}
}

BOOL	Do_Dupe_Init( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL	bRet;
#ifdef   USEITHREAD
   HGLOBAL  hDibInfo;
   PDI      pdi;
   PIS      lpi;
#else // !USEITHREAD
	LPISTR	lpi;
#endif   // USEITHREAD y/n
	HWND	hWnd;

	bRet = TRUE;	// TRUE to let windows set focus
#ifdef   USEITHREAD
   SET_PROP( hDlg, ATOM_HDI, (HANDLE)lParam );
   if( ( hDibInfo = (HGLOBAL)lParam        ) &&
       ( pdi = (PDI)DVGlobalLock(hDibInfo) ) )
   {
      lpi = &pdi->di_sIS;  // set a specific INFO pointer
   }
   else
   {
		EndDialog(hDlg, (INT_PTR)-1);
      return (BOOL)-1;
   }
#else // !USEITHREAD
	lpi = (LPISTR)lParam
	lpDIStr = lpi;
#endif   // USEITHREAD y/n
	if( lpi )
	{
		if( hWnd = GetWindow( hDlg, GW_OWNER ) )
			CenterWindow( hDlg, hWnd );
		if( lpi->lpFN )
			SetEdit1( hDlg, lpi->lpFN );
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT2,		// identifier of control
			pdi->di_dwDIBWidth,  // lpi->cxDIB,	// NOT bm.bmWidth, // value to set
			FALSE );
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT3,		// identifier of control
			pdi->di_dwDIBHeight, //lpi->cyDIB,	// NOT bm.bmHeight, // value to set
			FALSE );
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT4,		// identifier of control
			lpi->wBPP, // value to set
			FALSE );
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT5,		// identifier of control
			lpi->iCalcCols, // value to set
			FALSE );
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT7,		// identifier of control
			lpi->iUsedCols, // value to set
			FALSE );
		CheckRadioButton( hDlg,	// handle to dialog box
			IDC_RADIO1,	// identifier of first radio button in group
			IDC_RADIO5,	// identifier of last radio button in group
			IDC_RADIO1 );	// identifier of radio button to select

		lpi->hDlg = hDlg;
		lpi->uActive = IDC_RADIO1;
		SetDupStg( lpi );

		if( hWnd = GetDlgItem( hDlg, IDC_CHECK1 ) )
		{
         BOOL  bIsAnim;
#ifdef   USEITHREAD
			lpi->dwGIFCnt = pdi->wMaxCnt;
			if( ( pdi->dwDIBFlag & df_GIFANIM ) &&
				 ( pdi->wMaxCnt                ) )
            bIsAnim = TRUE;
         else
            bIsAnim = FALSE;
#else // !USEITHREAD
			lpi->dwGIFCnt = lpi->lpDIBInfo->wMaxCnt;
			if( ( lpi->lpDIBInfo->dwDIBFlag & df_GIFANIM ) &&
				( lpi->lpDIBInfo->wMaxCnt ) )
            bIsAnim = TRUE;
         else
            bIsAnim = FALSE;
#endif   // ifdef   USEITHREAD y/n

         if( bIsAnim )
			{
				EnableWindow( hWnd, TRUE );
				ShowWindow( hWnd, SW_SHOW );
				CheckDlgButton( hDlg, IDC_CHECK1, BST_CHECKED );
				lpi->bIsAnimGIF = TRUE;
			}
			else
			{
				EnableWindow( hWnd, FALSE );
				ShowWindow( hWnd, SW_HIDE );
				lpi->bIsAnimGIF = FALSE;
				lpi->dwGIFCnt = 0;
			}
		}

		if( hWnd = GetDlgItem( hDlg, IDC_EDIT8 ) )
		{
			if( ( lpi->bIsAnimGIF ) &&
				( lpi->dwGIFCnt ) )
			{
				EnableWindow( hWnd, TRUE );
				ShowWindow( hWnd, SW_SHOW );
				SetDlgItemInt( hDlg,	// handle of dialog box
					IDC_EDIT8,		// identifier of control
					lpi->dwGIFCnt, // value to set
					FALSE );
			}
			else
			{
				EnableWindow( hWnd, FALSE );
				ShowWindow( hWnd, SW_HIDE );
			}
		}
	}

#ifdef   USEITHREAD
   DVGlobalUnlock(hDibInfo);
#endif   // ifdef   USEITHREAD

	return bRet;

}

#ifdef ADD_JPEG_SUPPORT // This also seems to include GIF support
BOOL	Do_GIF_Dupe( HWND hDlg, PRDIB lprd, LPISTR lpIS )
{
	BOOL		   bRet;
	WORD		   Nxt;
	HANDLE		hDIB, hDIB2, hCopy;
	HPALETTE	   hPal;
	DWORD		   ddSz1, ddSz2;
	HGLOBAL		hBig;
	HBITMAP		hBM;
	int			iBPP, iCnt;
	LPDIBINFO	lpDIBInfo;
   PIS         lpi;
#ifdef   USEITHREAD
   HGLOBAL     hDibInfo;
   if( ( hDibInfo = GET_PROP( hDlg, ATOM_HDI )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hDibInfo) ) )
   {
      lpi = &lpDIBInfo->di_sIS;
   }
   else
   {
      return FALSE;
   }
#else    // !USEITHREAD
   lpi = lpIS;
	lpDIBInfo = lpi->lpDIBInfo;
#endif   // ifdef   USEITHREAD y/n

	bRet = FALSE;
	iCnt = 0;
	if( lpDIBInfo )
	{
		hPal = lpDIBInfo->hPal;		// hPal MAY be ZERO!!!
		if( ( hBig = lpDIBInfo->hBigFile ) &&
			( hDIB = lpDIBInfo->hDIB ) &&
			( ddSz1 = GlobalSize( hDIB ) ) &&
			( ddSz2 = GlobalSize( hBig ) ) &&
			( hDIB2 = DVGlobalAlloc( GMEM_SHARE, ddSz1 ) ) )
		{
			switch( lpi->uActive )
			{
			//	{ IDC_RADIO2, "As Monochrome" },
			case IDC_RADIO2:
				iBPP = 1;
				break;
			//	{ IDC_RADIO3, "As 16-Color Bitmap" },
			case IDC_RADIO3:
				iBPP = 4;
				break;
			//	{ IDC_RADIO4, "As 256-Color Bitmap" },
			case IDC_RADIO4:
				iBPP = 8;
				break;
			//	{ IDC_RADIO5, "As 24-bit Bitmap" },
			case IDC_RADIO5:
				iBPP = 24;
				break;
			default:
				iBPP = 0;
				break;
			}
			for( Nxt = 1; Nxt <= (WORD)lpi->dwGIFCnt; Nxt++ )
			{
				if( LIBGifNBmp( hBig, ddSz2, NULL, hDIB2, Nxt ) )
				{
					// FAILED to get this one!!!
				}
				else
				{
					if( lpi->uActive == IDC_RADIO1 )
					{
						hCopy = CopyHandle( hDIB );
						hBM = 0;
					}
					else
					{
						hBM = DIBToBitmap( hDIB2, hPal, lpDIBInfo );
					}
					if( hBM )
					{
						hCopy = BitmapToDIB2( hBM,
							hPal,
							iBPP );
						DeleteObject( hBM );
						hBM = 0;
					}
					if( hCopy )
					{
						lprd->rd_hDIB = hCopy;
                  // get the copy name
						//wsprintf( lprd->rd_pFName,
                  DVNextRDName( lprd,
							"TEMPG%d.BMP",
							(DWORD)Nxt );
						// Perform the actual window opening.
						//OpenDIBWindow( hDIB, szTitle, df_CLIPBRD );
						if( OpenDIBWindow2( lprd ) )
						{
							iCnt++;
						}
					}
				}
			}	// for COUNT
			DVGlobalFree( hDIB2 );	// toss the COPY memory
		}	// if various
	}	// If lpDIBInfo 

	if( iCnt )
		bRet = TRUE;

#ifdef   USEITHREAD
   DVGlobalUnlock(hDibInfo);
#endif   // ifdef   USEITHREAD

	return bRet;
}
#endif // #ifdef ADD_JPEG_SUPPORT // This also seems to include GIF support


BOOL	Do_Dupe_OK( HWND hDlg )
{
	BOOL		bRet;
	LPISTR	lpi;
	PRDIB		prd;
//	char		szTitle[64];
	HANDLE		hDIB, hCopy;
	int			iBPP;
	HBITMAP		hBitmap;	// May NOT exist for BIG DIBs!
	LPDIBINFO	lpDIBInfo;
#ifdef   USEITHREAD
   HGLOBAL     hDibInfo;
   if( ( hDibInfo = GET_PROP( hDlg, ATOM_HDI )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hDibInfo) ) )
   {
      lpi = &lpDIBInfo->di_sIS;
   }
   else
   {
      return FALSE;
   }
#else // !USEITHREAD
   lpi = lpDIStr;
   if(lpi)
      lpDIBInfo = lpi->lpDIBInfo;
#endif   // #ifdef   USEITHREAD y/n

	bRet = FALSE;
	prd = (PRDIB)MALLOC( sizeof(RDIB) );
   if(!prd)
      chkme( "C:ERROR: No memory!!!"MEOR );
	NULPRDIB( prd );
   prd->rd_pTitle = gszRPTit;
   prd->rd_pPath  = gszRPNam;
	//wsprintf( &gszRPTit[0], &gszRPFrm[0], gnPasteNum++ );
   DVNextRDName2( prd, gszRPFrm, &gnPasteNum, &gbChgPaste ); // = "TEMPB%03d.BMP"
	prd->rd_Caller = df_DUPE;

	hCopy = 0;		// No COPY yet
	iBPP = 0;		// No REQUESTED bitcount
	if( ( lpi ) &&
		 ( hDIB = lpi->hDIB ) &&
		 ( lpDIBInfo        ) )
	{
		// NOTE: May be ZERO for BIG DIBs
		//hBitmap = lpi->is_hBitmap;
		hBitmap = 0;
#ifdef ADD_JPEG_SUPPORT // This also seems to include GIF support
		if( ( lpi->bIsAnimGIF ) &&
			( lpi->dwGIFCnt ) )
		{
			if( IsDlgButtonChecked(	hDlg, IDC_CHECK1 ) == BST_CHECKED )
			{
				if( ( lpi->uActive == IDC_RADIO1 ) ||
					( lpi->uActive == IDC_RADIO2 ) ||
					( lpi->uActive == IDC_RADIO3 ) ||
					( lpi->uActive == IDC_RADIO4 ) ||
					( lpi->uActive == IDC_RADIO5 ) )
				{
					return( Do_GIF_Dupe( hDlg, prd, lpi ) );
				}
			}
		}
#endif // #ifdef ADD_JPEG_SUPPORT // This also seems to include GIF support
		// OK, so try to do the REQUEST
		switch( lpi->uActive )
		{

//	{ IDC_RADIO1, "Exactly the same" },
		case IDC_RADIO1:
			if( hCopy = CopyHandle( hDIB ) )
			{
				prd->rd_hDIB = hCopy;
			}
			else
			{
				DIBError( ERR_NO_COPY );
				return bRet;
			}
			break;

//	{ IDC_RADIO2, "As Monochrome" },
		case IDC_RADIO2:
			hBitmap = lpi->is_hBitmap;
			iBPP = 1;
			break;

//	{ IDC_RADIO3, "As 16-Color Bitmap" },
		case IDC_RADIO3:
			hBitmap = lpi->is_hBitmap;
			iBPP = 4;
			break;

//	{ IDC_RADIO4, "As 256-Color Bitmap" },
		case IDC_RADIO4:
			hBitmap = lpi->is_hBitmap;
			iBPP = 8;
			break;

//	{ IDC_RADIO5, "As 24-bit Bitmap" },
		case IDC_RADIO5:
			hBitmap = lpi->is_hBitmap;
			iBPP = 24;
			break;

		default:
			return bRet;	// Should NEVER HAPPEN
			break;

		}
		if( hBitmap )
		{
			hCopy = BitmapToDIB2( hBitmap,
				lpi->hPal,
				iBPP );
		}
		else if( iBPP )
		{
			// AWK: We have a REQUEST for CHANGE BPP,
			// but NO BITMAP (ie is BIG DIB)

		}
		if( hCopy )
		{
			DumpDIB( hCopy );
			prd->rd_hDIB = hCopy;
			// Perform the actual window opening.
			//OpenDIBWindow( hDIB, szTitle, df_CLIPBRD );
			if( OpenDIBWindow2( prd ) )
			{
				bRet = TRUE;
			}
		}
	}

#ifdef   USEITHREAD
   DVGlobalUnlock(hDibInfo);
#endif   // ifdef   USEITHREAD

   MFREE(prd);

	return bRet;
}

// WM_COMMAND 
BOOL	Do_Dupe_Command( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL	bRet, bEnd;
	DWORD	wNotifyCode, wID;
	HWND	hwndCtl;
	LPISTR	lpi;
#ifdef   USEITHREAD
   HGLOBAL     hdi;
   PDI         pdi;
   if( ( hdi = GET_PROP( hDlg, ATOM_HDI )   ) &&
       ( pdi = (PDI)DVGlobalLock(hdi) ) )
   {
      lpi = &pdi->di_sIS;
   }
   else
   {
      return FALSE;
   }
#else // !USEITHREAD
   lpi = lpDIStr;
//   if( !lpi )
//      return FALSE;
#endif   // #ifdef   USEITHREAD y/n

	bRet = TRUE;
	wNotifyCode = HIWORD(wParam);	// notification code 
	wID         = LOWORD(wParam);	// item, control
	hwndCtl     = (HWND)lParam;	// handle of control

	if( ( wID  == IDOK      ) ||
		 ( wID  == IDCANCEL  ) )
	{
		if( wID == IDOK )
		{
			Do_Dupe_OK( hDlg );
			bEnd = TRUE;
		}
		else
		{
			bEnd = FALSE;
		}
		EndDialog(hDlg, bEnd);
		// If an application processes this message, it should return zero. 
		bRet = FALSE;	// Processed message
	}
	else if( wID == IDC_BUTTON1 )
	{
		// The HELP Button
		Do_HELP( HELP_CONTEXT, (0x20000 + IDD_DIALOG5) );
	}
	else
	{
		if( ( wID >= IDC_RADIO1 ) &&
			 ( wID <= IDC_RADIO5 ) )
		{
			lpi->hDlg = hDlg;
			if( lpi )
			{
				switch( lpi->iCalcCols )
				{
				case 2:
					if( wID == IDC_RADIO2 )
						wID = IDC_RADIO1;	// To SAME
					break;
				case 16:
					if( wID == IDC_RADIO3 )
						wID = IDC_RADIO1;	// To SAME
					break;
				case 256:
					if( wID == IDC_RADIO4 )
						wID = IDC_RADIO1;	// To SAME
					break;
				case 0:
					if( wID == IDC_RADIO5 )
						wID = IDC_RADIO1;	// To SAME
					break;
				default:
					break;
				}
			}
			lpi->uActive = wID;
			SetDupStg( lpi );
			bRet = FALSE;	// processed
		}
		else if( wID == IDC_EDIT6 )
		{
			// Out of HERE - SetFocus

		}
		else if( wID == IDC_CHECK1 )
		{
			LPSTR	lps;
			if( IsDlgButtonChecked(	hDlg, IDC_CHECK1 ) == BST_CHECKED )
				lps = "Duplicate EACH GIF Image";
			else
				lps = "Duplicate ONLY current GIF";
			SetDlgItemText( hDlg,
				IDC_EDIT6,
				lps );
		}
	}

#ifdef   USEITHREAD
   DVGlobalUnlock(hdi);
#endif   // ifdef   USEITHREAD


	return bRet;
}

// ===============================================================
// BOOL CALLBACK DUPEDLGPROC( HWND, UINT, WPARAM, LPARAM )
//
// Purpose: Dialog box handler for IDM_DUPLICATE
//			Allows attributes to be CHANGED for duplicate
//       Uses IDD_DIALOG5 template
//
// ===============================================================
BOOL CALLBACK DUPEDLGPROC( HWND hDlg, UINT message,
						  WPARAM wParam, LPARAM lParam)
{
	BOOL	flg = FALSE;
	switch( message )
	{

	case WM_INITDIALOG:
		flg = Do_Dupe_Init( hDlg, wParam, lParam );
		break;
		
#ifdef	WIN32
	case WM_NOTIFY:
//		chknote();
		break;
#endif	// WIN32

	case WM_COMMAND:
		flg = Do_Dupe_Command( hDlg, wParam, lParam );
		break;
#ifdef   USEITHREAD
   case WM_DESTROY:
      REMOVE_PROP( hDlg, ATOM_HDI );
      break;
#endif   // ifdef   USEITHREAD

	}
	return( flg );
}

// ===========================================================
// void Dv_IDM_DUPLICATE( HWND hwnd )
//
// Purpose: To create a DUPLICATE window for the current DIB
//          From IDM_DUPLICATE menu item
//
// ===========================================================
void Dv_IDM_DUPLICATE( HWND hwnd )
{
	LPSTR	   lpb, lpbits, lpfn;
#ifndef	WIN32
	FARPROC	lpInfo;	// NOT required in WIN32!!!
#endif	/* WIN32 */
	DWORD	   dwSize;
   PDI      lpDIBInfo;
   HGLOBAL  hDIBInfo;
   HWND     hMDI;
   HANDLE   hDIB;
#ifndef   USEITHREAD
	ISTR	   IStr;
#endif   // ifndef   USEITHREAD
	PIS   	lpi;

	Hourglass( TRUE );
	hDIBInfo  = 0;
	lpDIBInfo = 0;
	// FIX980430 - Removed BITMAP bm member as NOT always right size
	if( ( hMDI      = GetCurrentMDIWnd()                   ) &&
		 ( hDIBInfo  = GetWindowExtra( hMDI, WW_DIB_HINFO ) ) &&
		 ( lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo ) ) )
   {
      if( !( hDIB = lpDIBInfo->hDIB ) )
         goto EXIT_DUPE;
#ifdef   USEITHREAD
      lpi = &lpDIBInfo->di_sIS;
#else // !USEITHREAD
   	lpi = &IStr;
	   NULPISTR(lpi);
		lpi->lpDIBInfo = lpDIBInfo;
#endif   // ifdef   USEITHREAD y/n
   	lpi->fChanged = FALSE;
	   lpi->dwFlag   = 0;
	   lpi->hMDI     = hMDI;
		lpi->hDIBInfo = hDIBInfo;
		lpi->hDIB     = hDIB;
		// FIX980504 - the BITMAP may not exist in case of BIG DIBs
		lpi->is_hBitmap = lpDIBInfo->hBitmap;
		lpfn = GetFNBuf();
		lstrcpy( lpfn, &lpDIBInfo->di_szDibFile[0] );
		lpi->lpFN = lpfn;

//		lpi->cxDIB = lpDIBInfo->di_dwDIBWidth;
//		lpi->cyDIB = lpDIBInfo->di_dwDIBHeight;
		lpi->hPal  = lpDIBInfo->hPal;
		if( lpb = DVGlobalLock( lpi->hDIB ) )  // LOCK DIB HANDLE
		{
			lpbits          = FindDIBBits( lpb );
			lpi->iColors    = DIBNumColors( lpb );
			lpi->iCalcCols  = CalcDIBColors( lpb );
			if( ( lpDIBInfo->di_dwDIBBits ) &&
				( dwSize = (DWORD)( (WIDTHBYTES( lpDIBInfo->di_dwDIBWidth * lpDIBInfo->di_dwDIBBits )) *
					lpDIBInfo->di_dwDIBHeight ) ) )
			{
//				lpi->iUsedCols = GetUCCnt( lpDIBInfo,
//						lpbits, lpi->iColors,
//						dwSize, lpi->cxDIB, lpi->cyDIB,
//						lpi->wBPP, lpb );
				lpi->iUsedCols = GetUCCnt( lpDIBInfo,
						lpbits, lpi->iColors,
						dwSize,
                  lpDIBInfo->di_dwDIBWidth,  // lpi->cxDIB
                  lpDIBInfo->di_dwDIBHeight, // lpi->cyDIB,
						lpDIBInfo->di_dwDIBBits, //lpi->wBPP
                  lpb );
			}
			DVGlobalUnlock( lpi->hDIB );  // UNLOCK DIB HANDLE
		}
		else
			lpi->iColors = 0;

		GetClientRect( lpi->hMDI, &lpi->rcClient );
		lpi->rcClip.left   = lpDIBInfo->rcClip.left;
		lpi->rcClip.top    = lpDIBInfo->rcClip.top;
		lpi->rcClip.right  = lpDIBInfo->rcClip.right;
		lpi->rcClip.bottom = lpDIBInfo->rcClip.bottom;
		if( IsRectEmpty( &lpi->rcClip ) )
		{
			lpi->rcClip.left   = 0;
			lpi->rcClip.top    = 0;
			lpi->rcClip.right  = lpDIBInfo->di_dwDIBWidth;  //lpi->cxDIB
			lpi->rcClip.bottom = lpDIBInfo->di_dwDIBHeight; //lpi->cyDIB
		}
#ifdef	WIN32
#ifdef   USEITHREAD
#ifdef WIN64
		DialogBoxParam(ghDvInst,
			MAKEINTRESOURCE(IDD_DIALOG5),
			lpi->hMDI,
			(DLGPROC)DUPEDLGPROC,	// lpInfo,
			(LPARAM)hDIBInfo);
#else // !WIN64
		DialogBoxParam( ghDvInst,
			MAKEINTRESOURCE(IDD_DIALOG5),
			lpi->hMDI,
			DUPEDLGPROC,	// lpInfo,
			(DWORD)hDIBInfo );
#endif // WIN64 y/n
#else // !USEITHREAD
		DialogBoxParam( ghDvInst,
			MAKEINTRESOURCE(IDD_DIALOG5),
			lpi->hMDI,
			DUPEDLGPROC,	// lpInfo,
			(DWORD)lpi );
#endif   // ifdef   USEITHREAD y/n

#else	/* !WIN32 */
		lpInfo = MakeProcInstance( DUPEDLGPROC, ghDvInst );
		DialogBoxParam( ghDvInst,
			MAKEINTRESOURCE(IDD_DIALOG5),
			lpi->hMDI,
			lpInfo,
			(DWORD)lpi );
		FreeProcInstance( lpInfo );
#endif	/* WIN32 y/n */
		if( lpi->fChanged )	// Something is changed
		{

		}

EXIT_DUPE:

      lpi = 0;

	}

	if( hDIBInfo && lpDIBInfo )
		DVGlobalUnlock( hDIBInfo );

	Hourglass( FALSE );

}

