
// DvRead.c
#include	"Dv.h"
//#include	"DvInfo.h"

// ============================================
extern	DWORD	DVFileSize( HANDLE );
extern	BOOL	ChkMFRFile( LPSTR lpf );
extern	void	SetReadPath( LPSTR lpdf );
extern	void	AddToFileList( LPSTR );
// ============================================
// NEW
extern   PMWL	AddToFileList4( PRDIB prd );

HANDLE	DvOpen2( LPSTR lpf, OFSTRUCT * pofs, UINT uType )
{
	HANDLE	hFile;

	hFile = 0;
	if( (IS_WIN95) || (IS_NT) )
	{
#ifdef	USEOPN
		hFile = _open( lpf, _O_RDONLY, _S_IREAD );
#else	// !USEOPN
		hFile = CreateFile( lpf,	// pointer to name of the file
			GENERIC_READ,			// access (read-write) mode 
			0,						// share mode 
			NULL,					// pointer
			OPEN_EXISTING,			// how to create 
			FILE_ATTRIBUTE_NORMAL,	// file attributes 
			NULL ) ;	// handle to file with attributes to copy  
#endif	// USEOPN
	}
	else
	{
		hFile = DVOpenFile ( lpf, pofs, uType );
	}
	return hFile;
}


BOOL	DvRead2( HANDLE hFile, LPSTR lpb, DWORD dwReq, LPDWORD pdwRead )
{
	BOOL	rflg;
	DWORD	dwRead;

	rflg = FALSE;
	dwRead = 0;
	if(( hFile                ) &&
		( hFile != INVALID_HANDLE_VALUE ) &&
		( lpb                  ) &&
		( dwReq                ) )
	{
		if( (IS_WIN95) || (IS_NT) )
		{
//BOOL ReadFile(
//    HANDLE hFile,	// handle of file to read 
//    LPVOID lpBuffer,	// address of buffer that receives
//data  
//    DWORD nNumberOfBytesToRead,	// number of bytes to read 
//    LPDWORD lpNumberOfBytesRead,	// address of number of
//bytes read 
//    LPOVERLAPPED lpOverlapped 	// address of structure for
//data 
////   );	
			rflg = ReadFile( hFile,
				lpb,
				dwReq,
				&dwRead,
				NULL );
		}
		else
		{
			dwRead = _lread( PtrToInt(hFile),
				lpb,
				dwReq );
		}

		if( dwRead == dwReq )
			rflg = TRUE;

	}
	if( pdwRead )
	{
		*pdwRead = dwRead;
		rflg = TRUE;
	}

	return rflg;

}
	
/* ************************************************************

  Function:  ReadBMPFile( PRDIB )
			which contains such things as -
			LPINT rd_pfHand, LPSTR rd_pFName, DWORD rd_dwFlag, ...

   Purpose:  Reads in the specified DIB file into a global chunk of
             memory.

   Returns:  A handle to a dib (hDIB) if successful.
             NULL if an error occurs.

  Comments:  BITMAPFILEHEADER is stripped off of the DIB.  Everything
             from the end of the BITMAPFILEHEADER structure on is
             returned in the global memory handle.

   History:   Date      Author      Reason

             6/1/91    Created
             6/27/91   Removed PM bitmap conversion routines.
             6/31/91   Removed logic which overallocated memory
                       (to account for bad display drivers).
            11/08/91   Again removed logic which overallocated
                       memory (it had creeped back in!)
				3 March 96	Added read of GIF and JPG ... Geoff.
				16 June 97 Upgraded to IJG Rev 6a (61) - WJPG2BMP
					or WJPEG32_2.DLL ...
  ************************************************************* */

HANDLE ReadBMPFile( PRDIB prd, BOOL bErrDlg )
{
	HANDLE *		lphFile;
	LPSTR			lpf;
	BITMAPFILEHEADER	bmfHeader;
	DWORD			dwBitsSize;
	HANDLE			hDIB;
	LPSTR			lpDIB;
	HANDLE			hFile;
	BOOL			flg, flg2;
	DWORD			dwRead;

	lphFile = prd->rd_pfHand;
	lpf = prd->rd_pPath;
	flg = FALSE;
	hDIB = 0;		// No (DIB memory) HANDLE yet
	flg2 = TRUE;
	dwRead = 0;		// No READ yet
   // get length of FILE in bytes for use when reading
	if( ( lphFile ) &&
		( hFile = *lphFile ) &&
		( hFile != INVALID_HANDLE_VALUE ) )
	{
//		dwBitsSize = _filelength (hFile);
//		if( dwBitsSize == (DWORD) -1 )
		dwBitsSize = DVFileSize( hFile );
		if(( dwBitsSize == 0          ) ||
			( dwBitsSize == (DWORD) -1 ) ||
			( dwBitsSize < MINFILE     ) )
		{
			if( bErrDlg )
			{
				if( dwBitsSize == 0 )
				{
					DIBError( ERR_NULSIZE );	// ERROR: File is ZERO Length!
				}
				else
				{
					DIBError( ERR_UNKNOWNF );
				}
			}
			goto ReadBRet;
		}

		// Go read the DIB file header and check if it's valid.
		if( !DvRead2( hFile, (LPSTR) &bmfHeader,
						sizeof( BITMAPFILEHEADER ), &dwRead ) ||
			( dwRead != sizeof( BITMAPFILEHEADER ) ) ||
			( bmfHeader.bfType != DIB_HEADER_MARKER ) )
		{
			// OK, we have established it is NOT BMP input ...
			DIBError (ERR_NOT_DIB);
			//return NULL;
			goto ReadBRet;

		}	// Is NOT a BMP file

		// ******* READ IN THE DEVICE INDEPENDENT BITMAP *******
		// =====================================================
		// Allocate memory for DIB (hDIB) and READ in
		// the file. Simple!
		// ===========================================
		hDIB = DVGlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, dwBitsSize - sizeof(BITMAPFILEHEADER));

		if( hDIB == 0 )
		{
			DIBError( ERR_MEMORY );
			goto ReadBRet;
		}

		lpDIB = DVGlobalLock( hDIB ); // LOCK DIB HANDLE
		if( lpDIB == 0 )
		{
			DIBError( ERR_MEMORY );
			goto ReadBRet;
		}

		// Go read the bits.
		if( !MyRead( hFile,
			lpDIB,
			dwBitsSize - sizeof(BITMAPFILEHEADER) ) )
		{
			DVGlobalUnlock (hDIB);
			DVGlobalFree   (hDIB);
			DIBError (ERR_READ);
			hDIB = 0;
			goto ReadBRet;
		}
		DVGlobalUnlock (hDIB);  // UNLOCK DIB HANDLE
	}
	else	// NO FILE HANDLE PASSED!!!!!!!!!!!!!!
	{
		// Internal CALLER Error
		// *********************
		hDIB = 0;
		DIBError( ERR_READ );
	}
ReadBRet:

	// NOTE: The EXIT always CLOSES THE FILE (if Open still)
	// *************************************
	if( VFH(hFile) )
	{
		DVlclose( hFile );	/* Close the file ... Physically and */
	}

	if( lphFile && *lphFile )
		*lphFile = 0;		/* logically ... */

	// RETURN Handle or NULL if failed ...
	// but NOTE that some DIB Handles are returned in
	// the PRDIB structure, and not directly.
	return( hDIB );
}	// end - ReadBMPFile( PRDIB prd )


// Open and READ a BITMAP FILE
// ===========================
HANDLE GetBMP2( PRDIB prd, BOOL bErrDlg )
{
	int		i;
	HANDLE hFile;
	OFSTRUCT ofs;
	HANDLE   hDIB;
	char	ebuf[MXTMPSTR];
	LPSTR	lps, lpf;

//   SetCursor(LoadCursor(NULL, IDC_WAIT));
	Hourglass( TRUE );
	hDIB = 0;
	ofs.cBytes = sizeof( OFSTRUCT );
	lpf = prd->rd_pPath;	// Extract the FILE NAME pointer

	hFile = DvOpen2( lpf, &ofs, OF_READ );	// DvRead.c
	if( VFH( hFile ) )
	{
		prd->rd_pfHand   = &hFile;	// Ptr to File Handle
		// NOTE: Difference for GIF Read!!!
		hDIB = ReadBMPFile( prd, bErrDlg );	// Will return NUL for GIF
		if( hFile )
			DVlclose( hFile );
	}
   else
	{
	   if( bErrDlg )
	   {
		   lps = &ebuf[0];
		   //wsprintf( lps, "File: [%s]\r\n", (LPSTR) szDFileName );
		   wsprintf( lps, "File: [%s]\r\n", lpf );
		   i = lstrlen( lps );
		   if( DIBEString( (lps+i), ERR_FILENOTFOUND ) )
		   {
			   if( !ChkMFRFile( lpf ) )
				   DIBErrorStg( lps );
		   }
		   else
		   {
			   DIBError( ERR_FILENOTFOUND );
		   }
	   }
	}

	Hourglass( FALSE );

	return( hDIB );
}

static char	szPrevOpen[MAX_PATH+16];

void	OpenBMPFile2( PRDIB prd, BOOL bErrDlg )
{
	HANDLE   hD;
	LPSTR	lpdf, lpFN;

	hD = 0;
	lpdf = &szPrevOpen[0];
	lpFN = prd->rd_pPath;
	if( ( lpFN && lpFN[0] ) ||	/* If we ALREADY have a file name, or */
		( GetFileName( lpFN, IDS_OPENDLG) ) )	/* If the USER picks one ... */
	{
		// Then we will assume it is a DIB File ...
		if( lpdf != lpFN )
			lstrcpy( lpdf, lpFN );	// Update Open file Buffer
		prd->rd_Caller |= gd_OF;	// Add CALLER, and
		hD = GetBMP2( prd, bErrDlg );	// Pass ON pointer
		//hD = GetDIB2( lpdf, gd_OF )
		//hD = GetDIB( lpdf, gd_OF );
#ifdef	WIN32
		if( hD )
		{
			SetReadPath( lpFN );
		}
#endif	// WIN32
	}
	if( hD )	// If returned
		prd->rd_hDIB = hD;

}

BOOL  CommonOpenBitmapFile( HWND hWnd, LPSTR lpf, DWORD Caller )
{
   BOOL     bRet = FALSE;
	HANDLE	hDIB;
	int	   npDIBs = gnDIBsOpen;	// If already HAVE CHILDREN
	PRDIB prd = (PRDIB)MALLOC( sizeof(RDIB) );
   if(!prd)
      chkme( "C:ERROR: No memory!!!"MEOR );
	NULPRDIB( prd );

   prd->rd_pTitle = gszRPTit;
   prd->rd_pPath  = gszRPNam;
	prd->rd_hWnd   = hWnd;
	prd->rd_Caller = Caller;
   strcpy( gszRPTit, lpf );
   DVGetFullName2( gszRPNam, gszRPTit );
	OpenBMPFile2( prd, FALSE );
	//OpenDIBFile2( &rd );
	hDIB = prd->rd_hDIB; 
	if(hDIB)
	{
		prd->rd_Caller |= Caller;
		if( OpenDIBWindow2( prd ) )
      {
#ifdef	CHGADDTO
		   //AddToFileList( lpf );
         //AddToFileList4( &rd );
         ADD2LIST(prd);
#endif	// CHGADDTO
         bRet = TRUE;
      }
	}
	else	// We FAILED to LOAD FILE
	{	// BUT remember say GIF has already
		// established a WINDOW by now ...
		if( npDIBs == gnDIBsOpen )
		{
			// NOTHING GOT OPENNED
			SetReadPath( lpf );
		}
	}

   MFREE(prd);

   return bRet;
}

// eof - DvRead.c
