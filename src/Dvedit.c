

// DvEdit.c

#include	"Dv.h"
//#include	"DvInfo.h"

extern	BOOL  CommonOpenBitmapFile( HWND hWnd, LPSTR lpf, DWORD Caller );
extern	BOOL	FindNewName( LPSTR );
extern	BOOL	WriteBMPFile( LPDIBINFO lpdi, LPSTR lpf );

// Edit a BITMAP
// =============
// Handle for EDIT structure - pesistant data - we can have
// many edits outstanding.
HGLOBAL		hEditBMPs = 0;
DWORD		dwEditCnt = 0;
char		gszEditbmp[MAX_PATH+16] = {0};

#define		MXEDITCNT		64
#define		MXEDITSTR		((MXEDITCNT + 8) * sizeof(DIBINFO))

DWORD	GetSizeMxEdit( void )
{
	return( MXEDITSTR );
}

BOOL	Add2EditBMPs( PDI lpDIBInfo )
{
	BOOL	      bRet;
	HANDLE	   hDIB;
	LPVOID	   lpv;
	LPDIBINFO	lpi;
	PRDIB		   prd;	// stmrDInfo member

	bRet = FALSE;
	if( ( dwEditCnt < MXEDITCNT    ) &&
		 ( lpDIBInfo                ) &&
		 ( hDIB = lpDIBInfo->hDIB ) )
	{
      LPSTR lps = 0;
		if( hEditBMPs == 0 )
		{
			hEditBMPs = DVGlobalAlloc( GHND, MXEDITSTR );
		}
		if( ( dwEditCnt < MXEDITCNT           ) && // Bump EDIT BMP Count - for TIMER
			 ( hEditBMPs                       ) &&
			 ( lpv = DVGlobalLock( hEditBMPs ) ) )
		{
			DWORD	dwDIBSz;
			LPSTR	lpd;

			lpi = (LPDIBINFO) lpv;
			lpi += dwEditCnt;	// Up to NEXT
			// Get COPY of structure - lots of information
			memcpy( lpi, lpDIBInfo, sizeof(DIBINFO) );
			prd = &lpi->stmrDInfo;
			NULPRDIB( prd );	// Ensure TIMER section is ZERO on creation

			// hCopy = MAKE OUR OWN IMAGE DATA
			if( ( dwDIBSz = GlobalSize( hDIB )               ) &&
				 ( lps = DVGlobalLock( hDIB )                 ) &&   // LOCK DIB HANDLE
				 ( lpi->hDIB = DVGlobalAlloc( GHND, dwDIBSz ) ) )
			{
				if( lpd = DVGlobalLock( lpi->hDIB ) )
				{
					memcpy( lpd, lps, dwDIBSz );	// Copy the IMAGE
					// But some things can be deleted, and can NOT
					// be used after returning since a child can close
					lpi->di_szCGenBuff[0] = 0; // A general WORK buffer
					DVGlobalUnlock( lpi->hDIB );
					// SET Timer in motion
					// ===================
					dwEditCnt++;	// Bump EDIT BMP Count - In TIMER
				}

				if( ( lpi->hBitmap ) &&
					 ( lpi->hPal    ) )
				{
					prd->rd_hDIB = WinDibFromBitmap( lpi->hBitmap,
						BI_RGB,
						24,
						lpi->hPal );
				}
			}

			DVGlobalUnlock( hEditBMPs );
		}

      // FIX20001201 - This UNLOCK appears missing
      if(lps)
         DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE
	}
	return	bRet;
}


// ==============================================
// EDit BMP Timer uses some of ...
//	HWND	rd_hWnd;
//	LPINT	rd_pfHand;
//	LPSTR	rd_pFName;
//	DWORD	rd_Caller;
//	DWORD	rd_dwFlag;
//	HGLOBAL	rd_hDIB;	// Special case for Show_File & GIF
//	HGLOBAL	rd_hGIFInfo;	// A special special case for GIF
//	DWORD	rd_Res1;
#define		et_DnInit		0x00000001	// Passed to EDITOR?
#define		et_GotFile		0x00000002	// Got a NEW FN
#define		et_GotData		0x00000004	// Got a FILE Info
#define		et_GotDiff		0x80000000	// Changed OPEN Window

extern	BOOL	IsChildOpen( HWND hwnd );
extern	int		DVGetCwd( LPSTR lpb, DWORD siz );
TCHAR 	gszBgnEd[] = "TEMPE001.BMP";

STARTUPINFO          siPaint = { 0 };
PROCESS_INFORMATION  piPaint = { 0 };

HANDLE	StartProcess( LPSTR lpcmd,
					 STARTUPINFO * psi,
					 PROCESS_INFORMATION * ppi )
{
	BOOL	rflg;

	rflg = FALSE;

	// Close process and thread handles.
	if( ppi->hProcess )
		CloseHandle( ppi->hProcess );
	if( ppi->hThread )
		CloseHandle( ppi->hThread );

	ppi->hProcess = 0;
	ppi->hThread  = 0;

   ZeroMemory( psi, sizeof(STARTUPINFO) );
   psi->cb = sizeof(STARTUPINFO);

   // Start the child process. 
   if( CreateProcess( NULL, // No module name (use command line). 
        lpcmd,			// Command line. 
        NULL,             // Process handle not inheritable. 
        NULL,             // Thread handle not inheritable. 
        FALSE,            // Set handle inheritance to FALSE. 
        0,                // No creation flags. 
        NULL,             // Use parent�s environment block. 
        NULL,             // Use parent�s starting directory. 
        psi,              // Pointer to STARTUPINFO structure.
        ppi ) )           // Pointer to PROCESS_INFORMATION structure.
	{
		rflg = TRUE;
	}
	else
	{
		rflg = FALSE;
        //ErrorExit( �CreateProcess failed.� );
		ppi->hProcess = 0;
		ppi->hThread  = 0;
    }

    // Wait until child process exits.
    //WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    //CloseHandle( pi.hProcess );
    //CloseHandle( pi.hThread );
	return( ppi->hProcess );

	//return rflg;
}

HWND	RunMSPaint( LPDIBINFO lpi, LPSTR lpf,
				   STARTUPINFO * psi,
				   PROCESS_INFORMATION * ppi )
{
	// start - winexec - exec
	HWND	hWnd;
	LPSTR	lpexec;
	UINT	ui;

	hWnd = 0;
	lpexec = &lpi->di_szDibFile[0];	// DIB's filename (Ask hWnd if reqd)
	lstrcpy( lpexec, "MSPAINT " );
	lstrcat( lpexec, lpf );
	if( ( ui = WinExec( lpexec, SW_SHOW ) ) &&
		( ui > 31 ) )
	{
		hWnd = UIntToPtr(ui);
	}
	else
	{
		lstrcpy( lpexec, "C:\\PROGRA~1\\ACCESS~1\\MSPAINT.EXE " );
		lstrcat( lpexec, lpf );
		if( ( ui = PtrToUint(StartProcess( lpexec, psi, ppi )) ) ||
			( ( ui = WinExec( lpexec, SW_SHOW ) ) &&
			( ui > 31 ) ) )
		{
			hWnd = UIntToPtr(ui);
		}
	}
	return hWnd;
}

void	CloseEdit( LPDIBINFO lpi )
{
	PRDIB		prd;	// stmrDInfo member
	if( lpi->hDIB )
		DVGlobalFree( lpi->hDIB );
	lpi->hDIB = 0;

	prd = &lpi->stmrDInfo;	// Get POINTER to Timer info
	if( prd->rd_hDIB )
		DVGlobalFree( prd->rd_hDIB );
	prd->rd_hDIB = 0;

	if( dwEditCnt )
		dwEditCnt--;

}

void	KillEditBMPs( void )
{
	// Action the BMP Edit function
	LPVOID		lpv;	// Pointer to begin of a list of await edit
	DWORD		dwc;	// o/s count
	DWORD		dwi;	// loop counter
	LPDIBINFO	lpi;	// each instance waiting ...
	PRDIB		prd;	// stmrDInfo member
	LPSTR		lpf;	// File we are watching for change ...
	int			i;

	if( dwc = dwEditCnt )
	{
		if( ( hEditBMPs                       ) &&
			 ( lpv = DVGlobalLock( hEditBMPs ) ) )
		{
			lpi = (LPDIBINFO) lpv;
			for( dwi = 0; dwi < dwc; dwi++ )
			{
				prd = &lpi->stmrDInfo;	// Get POINTER to Timer info
				lpf = &lpi->di_szCGenBuff[0]; // The BMP written (Edit BMP)
				i = lstrlen( lpf );
				CloseEdit( lpi );	// Close this item

				lpi++;
			}
			DVGlobalUnlock( hEditBMPs );
		}

	}

   if( hEditBMPs )
      DVGlobalFree( hEditBMPs );

	hEditBMPs = 0;
	dwEditCnt = 0;

}

// FILETIME Compare
BOOL	CompareATime( FILETIME * pt1, FILETIME * pt2 )
{
	BOOL	rflg;
	rflg = FALSE;

//typedef struct _FILETIME { // ft 
	if( ( pt1->dwLowDateTime  != pt2->dwLowDateTime  ) ||
		 ( pt1->dwHighDateTime != pt2->dwHighDateTime ) )
	{
		rflg = TRUE;
	}
	return rflg;
}


BOOL	CompareTimes( LPWIN32_FIND_DATA lpOld,
					 LPWIN32_FIND_DATA lpNew )
{
	BOOL	rflg;
	FILETIME * pt1;
	FILETIME * pt2;

	rflg = FALSE;	// Start with *** NO CHANGE ***

	pt1 = &lpOld->ftCreationTime; 
	pt2 = &lpNew->ftCreationTime; 
	if( ( !rflg ) &&
		( CompareATime( pt1, pt2 ) ) )
	{
		rflg = TRUE;
	}

	pt1 = &lpOld->ftLastAccessTime;
	pt2 = &lpNew->ftLastAccessTime;
	if( ( !rflg ) &&
		( CompareATime( pt1, pt2 ) ) )
	{
		rflg = TRUE;
	}

	pt1 = &lpOld->ftLastWriteTime;
	pt2 = &lpNew->ftLastWriteTime;
	if( ( !rflg ) &&
		( CompareATime( pt1, pt2 ) ) )
	{
		rflg = TRUE;
	}

	return rflg;
	
}


BOOL	CompareEdits( LPWIN32_FIND_DATA lpOld,
					 LPWIN32_FIND_DATA lpNew )
{
	BOOL	rflg;
	rflg = FALSE;
	// typedef struct _WIN32_FIND_DATA { // wfd 
	if( ( lpOld->dwFileAttributes != lpNew->dwFileAttributes ) ||
		 ( lpOld->nFileSizeHigh    != lpNew->nFileSizeHigh ) ||
		 ( lpOld->nFileSizeLow     != lpNew->nFileSizeLow ) )
	{
		rflg = TRUE;

		//    FILETIME ftCreationTime; 
		//     FILETIME ftLastAccessTime; 
		//     FILETIME ftLastWriteTime; 
//    TCHAR    cFileName[ MAX_PATH ]; 
//    TCHAR    cAlternateFileName[ 14 ]; 
	}
	else
	{
		rflg = CompareTimes( lpOld, lpNew );
	}
	return rflg;
}

static BOOL	fInEBTimer = FALSE;
void	EditBMPTimer( void )
{
	// Action the BMP Edit function
	LPVOID		lpv;	// Pointer to begin of a list of await edit
	DWORD		dwc;	// o/s count
	DWORD		dwi, dwj;	// loop counters
	LPDIBINFO	lpi;	// each instance waiting ...
	PRDIB		prd;	// stmrDInfo member
	LPSTR		lpf;	// File we are watching for change ...
	int			i;
	WIN32_FIND_DATA nfd;	// file data
	LPWIN32_FIND_DATA lpnfd;	// file data
	HANDLE		hFind, hDIB;
	HWND		hDIBWin;

	if( fInEBTimer )
		return;

	fInEBTimer = TRUE;
	if( dwc = dwEditCnt )
	{
		if( hEditBMPs &&
			( lpv = DVGlobalLock( hEditBMPs ) ) )
		{
//			DWORD	dwDIBSz;
//			LPSTR	lps, lpd;
			lpnfd = &nfd;
			lpi = (LPDIBINFO) lpv;
			for( dwi = 0; dwi < dwc; dwi++ )
			{
				prd = &lpi->stmrDInfo;	// Get POINTER to Timer info
				lpf = &lpi->di_szCGenBuff[0]; // The BMP written (Edit BMP)
				hDIBWin = lpi->di_hwnd;
//	HWND	rd_hWnd;
//	LPINT	rd_pfHand;
//	LPSTR	rd_pFName;
//	DWORD	rd_Caller;
//	DWORD	rd_dwFlag;
//	HGLOBAL	rd_hDIB;	// Special case for Show_File & GIF
//	HGLOBAL	rd_hGIFInfo;	// A special special case for GIF
//	DWORD	rd_Res1;
//#define		et_DnInit		0x00000001	// Passed to EDITOR?
				if( prd->rd_dwFlag & et_DnInit )
				{
					if( ( IsChildOpen( hDIBWin ) ) &&
						( prd->rd_dwFlag & et_GotData ) )
					{
						// Ok, we can WAIT
						// Check if FILE CHANGED ...
						if( prd->rd_dwFlag & et_GotDiff )
						{
							//CommonFileOpen( NULL, lpf, df_IDOPEN );
							CommonOpenBitmapFile( NULL, lpf, df_IDOPEN );
							prd->rd_dwFlag &= ~(et_GotDiff);
						}
						hFind = FindFirstFile( lpf, // pointer to name of file to search for
							lpnfd );
						if( VFH(hFind) )
						{
							if( prd->rd_dwFlag & et_GotData )
							{
								// Compare current with OLD.
								// Anything change???
								if( CompareEdits( &lpi->fdEditFile,
									lpnfd ) )
								{
									// LPWIN32_FIND_DATA lpOld,
									// LPWIN32_FIND_DATA lpNew )
									// are DIFFERENT - Open DIB Window
									prd->rd_dwFlag |= et_GotDiff;
								}
							}
							memcpy( &lpi->fdEditFile,
								lpnfd,
								sizeof(	WIN32_FIND_DATA ) );
							// get initial file data
							FindClose( hFind );
						}
					}
					else
					{
						LPDIBINFO	lpicur, lpinxt;	// each instance waiting ...
						// Child CLOSED - Close Edit WAIT
						//if( lpi->hDIB )
						//	DVGlobalFree( lpi->hDIB );
						//lpi->hDIB = 0;
						//dwEditCnt--;
						CloseEdit( lpi );	// Close this item

						lpicur = lpi;
						lpinxt = lpi + 1;
						for( dwj = dwi; dwj < dwc; dwj++ )
						{
							memcpy( lpicur, lpinxt, sizeof(DIBINFO) );
							lpicur++;
							lpinxt++;
						}
						dwi++;	// We have DONE a record - DELETED
					}
				}
				else
				{
					prd->rd_dwFlag |= et_DnInit;
					// This is FIRST TIME
					// Get current work directory (cwd)
					//lstrcpy( lpf, &gszCurWrk[0] );
					//EditBMPInit( lpi, prd, lpf, lpnfd );
					if( *lpf == 0 )
					{
						LPSTR	lpbmfn;
						lpbmfn = &gszEditbmp[0];
						if( *lpbmfn == 0 )
						{
							DVGetCwd( lpbmfn, MAX_PATH );
							if( i = lstrlen( lpbmfn ) )
							{
								if( lpbmfn[i-1] != '\\' )
									lstrcat( lpbmfn, "\\" );
							}
							lstrcat( lpbmfn, &gszBgnEd[0] );
							i = FindNewName( lpbmfn );
						}
						if( ( i = lstrlen( lpbmfn ) ) &&
							( FindNewName( lpbmfn ) ) )
						{
							// Copy over NEW file name to use
							lstrcpy( lpf, lpbmfn );
							// wait! prd->rd_dwFlag |= et_GotFile;
							if( hDIB = prd->rd_hDIB )
							{
								// If we have a 24-bit image, use that
								prd->rd_hDIB = lpi->hDIB;
								lpi->hDIB = hDIB;	// *** SCREEN fm bmp ***
							}
							if( WriteBMPFile( lpi, lpf ) )
							{
								// start - winexec - exec
								// LOAD THE IMAGE EDITOR
								// =====================
								hFind = FindFirstFile( lpf, // pointer to name of file to search for
									lpnfd );
								if( VFH(hFind) )
								{
									// get initial file data
									memcpy( &lpi->fdEditFile,
										lpnfd,
										sizeof(	WIN32_FIND_DATA ) );
									// get initial file data
									FindClose( hFind );
									prd->rd_dwFlag |= et_GotData;
								}
								prd->rd_hWnd = RunMSPaint( lpi, lpf,
									&siPaint, &piPaint );
								if( prd->rd_hWnd )   // = RunMSPaint( lpi, lpf, &siPaint, &piPaint ) )
								{
									if( dwEditCnt < MXEDITCNT )
									{
										prd->rd_dwFlag |= et_GotFile;
									}
								}
							}
						}
						else
						{
							// Inc. to some number should work ...
						}
					}

				}


				lpi++;
			}
			DVGlobalUnlock( hEditBMPs );
		}
	}
	fInEBTimer = FALSE;

}

void Dv_IDM_EDITBMP( HWND hwnd )
{
	PIS	   lpi;
   HWND     hMDI;
   HGLOBAL  hDIBInfo;
   PDI      lpDIBInfo;
   HANDLE   hDIB;
#ifndef   USEITHREAD
	ISTR	IStr;
#endif   // ifndef   USEITHREAD

	Hourglass( TRUE );
	hDIBInfo = 0;
	lpDIBInfo = 0;
	if( ( hMDI     = GetCurrentMDIWnd()                    ) &&
		 ( hDIBInfo = GetWindowExtra( hMDI, WW_DIB_HINFO )  ) &&
		 ( lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo ) ) &&
		 ( hDIB = lpDIBInfo->hDIB                           ) )
	{
#ifdef   USEITHREAD
      lpi = &lpDIBInfo->di_sIS;
#else    // !USEITHREAD
   	lpi = &IStr;
	   NULPISTR( lpi );
		lpi->lpDIBInfo = lphDIBInfo;
#endif   // ifdef   USEITHREAD y/n
	   lpi->fChanged  = FALSE;
   	lpi->dwFlag    = 0;
      lpi->hMDI      = hMDI;
      lpi->hDIBInfo  = hDIBInfo;
      lpi->hDIB      = hDIB;
		Add2EditBMPs( lpDIBInfo );
	}

	if( hDIBInfo && lpDIBInfo )
		DVGlobalUnlock( hDIBInfo );

	Hourglass( FALSE );
}

// eof - DvEdit.c
