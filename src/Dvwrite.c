

// DvWrite.c

#include	"dv.h"	// single inclusive include
//#include	"dvinfo.h"	// and another information structure

#define		MXIWBUF			1024	// Information buffer (for LIBRARY)

#ifdef	USEB2G
extern	BOOL	B2G( HGLOBAL, DWORD, HGLOBAL, DWORD, LPSTR );
#endif	// USEB2G
extern	WORD	DVWBmpToJpg( HGLOBAL, DWORD, HGLOBAL, LPSTR );
extern	void	AddFN( LPSTR, LPSTR );
extern	BOOL	fReLoad;
extern	void	AddToFileList( LPSTR );
extern	BOOL	FindNewName( LPSTR );
extern	DWORD	DVWrite(HANDLE, VOID MLPTR, DWORD);

extern	WORD	gwOutIndex;		// Current OUT index into filter
//extern	char    gszDrive[_MAX_DRIVE];          // Drive
//extern	char    gszDir[_MAX_DIR];          // Directory
//extern	char    gszFname[_MAX_FNAME];         // Filename
//extern	char    gszExt[_MAX_EXT];            // Extension
extern	LPSTR	pRDefExt;	// = &szBmp[0];	say, BMP extent ... 
//extern	BOOL	fBe_Tidy;

extern	PMWL  CommonFileOpen( HWND hWnd, LPSTR lpf, DWORD Caller );
extern	BOOL  CommonOpenBitmapFile( HWND hWnd, LPSTR lpf, DWORD Caller );
// NEW
extern   PMWL	AddToFileList4( PRDIB prd );

char	gszDefTemp[] = "temp";

/*	***********************************************************

 FUNCTION   : WriteDIB(LPSTR szFile,HANDLE hdib)

 PURPOSE    : Write a global handle in CF_DIB format to a file.

 RETURNS    : TRUE  - if successful.
		 FALSE - otherwise
 HISTORY    : In March 1996, add the JPEG library type code to WRITE GIF,
	etc ... whatever supported ...
	***********************************************************	*/

BOOL WriteDIB( LPSTR lpOutReq, HANDLE hOutDIB )
{
	BITMAPFILEHEADER	hdr;
	LPBITMAPINFOHEADER  lpbi;
	HANDLE              fh;
	OFSTRUCT            of;
	BOOL				flg, rflg;
	char				OutName[260];
	char				OutName2[260];
	LPSTR				lpo, lpo2;
	LPSTR				lpf;
	LPSTR				lpsw;
#ifndef	WJPEG4
	LPSTR				JFiles[6];
#endif	// !WJPEG4
	WORD				ArgC;
	WORD				FType;
#ifdef	USEGFUNC3
	HGLOBAL				hBMP, hNDIB;
	DWORD				ddSz;
	LPBITMAPFILEHEADER	lphdr;
#endif
	HGLOBAL				hIBuf;
	LPSTR				lpIBuf;
	DWORD				dwDIBSz;
	PRDIB prd = (PRDIB)MALLOC( sizeof(RDIB) );
   if(!prd)
      chkme( "C:ERROR: No memory!!!"MEOR );
	NULPRDIB( prd );

   prd->rd_pTitle = gszRPTit;
   prd->rd_pPath  = gszRPNam;
	hIBuf = 0;
	lpIBuf = 0;

	rflg = FALSE;	// NOT done yet
	lpf = lpOutReq;
	lpsw = 0;	// No SWITCH initially

	if( ( lpf == NULL ) ||
		( lpf[0] == 0 ) ||
		( hOutDIB == NULL ) ||
		( (dwDIBSz = GlobalSize( hOutDIB )) == 0 ) )
	{
		DIBError( ERR_WRITEDIB );
		//goto WriteDRet;
		return rflg;
	}
	lpo = &OutName[0];
	lpo2 = &OutName2[0];
	lstrcpy( lpo2, lpf );	// Assume for a moment this is JPG output
	switch( gwOutIndex )	// First TRY per the global INDEX
	{

		case 0:	// Index is 1, BMP
			FType = ft_BMP;	// Ouput is BITMAP
			lstrcpy( lpo, lpf );
			lpsw = 0;		// No SWITCH required
			flg = FALSE;
			break;
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
		case 1:	// Index is 2, JPG
			FType = ft_JPG;
			lpsw = 0;	// No SWITCH required
			flg = TRUE;
			break;
		case 2:	// Index is 3, which is GIF
			FType = ft_GIF;
#ifndef	WJPEG4
			lpsw = (LPSTR) pRDGif;	/* Set a SWITCH to GIF */
#endif	// !WJPEG4
			flg = TRUE;
			break;
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

			// ================================================
			// All the rest can ONLY depend on the EXTENT given
			// ================================================
		default:
			if( IsBMP( lpf ) )
			{
				FType = ft_BMP;
				lpsw = 0;	// No SWITCH required
				flg = FALSE;
			}
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
			else if( IsJPEG( lpf ) )
			{
				FType = ft_JPG;
				lpsw = 0;	// No SWITCH required
				flg = TRUE;
			}
			else if( IsGIF( lpf ) )
			{
				FType = ft_GIF;
#ifndef	WJPEG4
				lpsw = (LPSTR) pRDGif;	/* Set a SWITCH to GIF */
#endif	// !WJPEG4
				flg = TRUE;
			}
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

			else if( IsRLE( lpf ) )
			{
				FType = ft_RLE;
#if defined(ADDWJPEG) && !defined(WJPEG4)
				lpsw = (LPSTR) pRDRle;	/* Set a SWITCH to RLE */
#endif	// !WJPEG4
				DIBError( ERR_NO_RLE );	/* Advise UNSUPPORTED */
				goto WriteDRet;
			}
			else if( IsPPM( lpf ) )
			{
				FType = ft_PPM;
#if defined(ADDWJPEG) && !defined(WJPEG4)
				lpsw = (LPSTR) pRDPpm;	/* Set a SWITCH to PPM */
#endif	// !WJPEG4
				flg = TRUE;
			}
			else if( IsTARGA( lpf ) )
			{
				FType = ft_TARGA;
#if defined(ADDWJPEG) && !defined(WJPEG4)
				lpsw = (LPSTR) pRDTar;	/* Set a SWITCH to TARGA */
#endif	// !WJPEG4
				flg = TRUE;
				DIBError( ERR_NO_RLE );	// Advise UNSUPPORTED
				goto WriteDRet;
			}
			else
			{
				FType = ft_Undef;
				DIBError( ERR_NO_RLE );	// Advise UNSUPPORTED
				goto WriteDRet;
			}
			break;
			// ================================================
	}	// Switch per gwOutIndex

#ifdef	USEB2G
	// Take away some TYPES now ...
	if( FType == ft_GIF )
	{
		//ddSz = GlobalSize( hdib ) + sizeof( BITMAPFILEHEADER );
		ddSz = dwDIBSz + sizeof( BITMAPFILEHEADER );
		if( hBMP = DVGlobalAlloc( GMEM_SHARE, (ddSz+2) ) )
		{
			flg = B2G( hOutDIB, dwDIBSz, hBMP, ddSz, lpf );
			DVGlobalFree( hBMP );
		}
		if( flg )
			DIBError( ERR_NOGIF );	/* Advise OUTPUT Failed */
		else
			rflg = TRUE;
		goto WriteDRet;
	}
#endif	// USEB2G

#ifdef	USEGFUNC3

	hBMP = 0;
	lphdr = 0;
	//if( hIBuf == 0 )
	//{
	hIBuf = DVGlobalAlloc( GHND, MXIWBUF );
	//}
	// If to OUTPUT a JPEG FIle
	// ========================

	if( FType == ft_JPG )
	{	/* If outputting a JPEG file, use NEW function ... */
#ifdef	USEGFUNC4
#ifdef	LDYNALIB
		if( !GetJPEGLib(3) )
		{
			if( !fShownNOLIB )
			{
				UINT	ui;
				fShownNOLIB = TRUE;
				//DIBError( ERR_NO_LIB );
				ui = Err_No_Lib();
			}
			goto WriteDRet;
		}
//		ArgC = (*WBmpToJpg) ( hdib, GlobalSize( hdib ), hIBuf, lpf );
		ArgC = DVWBmpToJpg( hOutDIB, dwDIBSz, hIBuf, lpf );
#else	// !LDYNALIB
//		if( WBmpToJpg )
//		{	// Load with Wjpeg32.Lib linkage ...
//			ArgC = (*WBmpToJpg) ( hdib, GlobalSize( hdib ), hIBuf, lpf );
//		}
		ArgC = DVWBmpToJpg( hOutDIB, dwDIBSz, hIBuf, lpf );
		//ArgC = WBMPTOJPG( hdib, GlobalSize( hdib ), hIBuf, lpf );
#endif	// LDYNALIB y/n
		if( ArgC )
		{
			if( hIBuf )
				lpIBuf = DVGlobalLock( hIBuf );
			else
				lpIBuf = 0;
			if( lpIBuf )
			{
				// If this version did NOT return an ERROR string
				if( ( *lpIBuf == 0 ) &&
					( ArgC != TRUE ) )
				{
					wsprintf( lpIBuf, "WJPEG Library error %d!\r\n", ArgC );
				}
			}
			if( lpIBuf ) AddFN( lpIBuf, lpf );
			if( lpIBuf && lpIBuf[0] )
				DIBError2( ERR_NOBMP2JPG, lpIBuf );
			else
				DIBError( ERR_NOBMP2JPG );
			ArgC = 0;
		}
		else	/* Return is ZERO = SUCCESS */
		{	/* So signal SUCCESSFUL input of DATA and OUTPUT of JPEG */
			if( fReLoad )
			{
            strcpy( gszRPTit, lpf );
            DVGetFullName2( gszRPNam, gszRPTit );
				// If file to be RELOAD (in a NEW Window of course)
	         //if( hNDIB = OpenDIBFile( lpf ) )
	         if( hNDIB = OpenDIBFile( gszRPNam ) )
				{	/* We have a NEW DIB ... */
					prd->rd_hDIB = hNDIB;
					//rd.rd_pFName = lpf;
					prd->rd_Caller = df_CONVJPEG;
					if( OpenDIBWindow2( prd ) )
               {
		        	//OpenDIBWindow( hNDIB, lpf, df_CONVJPEG );
#ifdef	CHGADDTO
					   //AddToFileList( lpf );
					   //AddToFileList( gszRPNam );
                  //AddToFileList4( &rd );
                  ADD2LIST(prd);
#endif	// CHGADDTO
               }
					// EVRYTHING HAS BEEN DONE HERE
					// so the RETURN FLAG is left FALSE!!!!!!!!!!!!!
					// =============================================
				}
				else
				{
					rflg = TRUE;
				}
			}
			else
			{
				// NO new Window option ON, so ...
				rflg = TRUE;	// This will cause the TITLE to be CHANGED
			}
		}
		goto WriteDRet;	// All done here ... whether error or NOT
#endif	/* USEGFUNC4 - Specialised DIB to JPEG directly from memory */
		//ddSz = GlobalSize( hdib ) + sizeof( BITMAPFILEHEADER );
		ddSz = dwDIBSz + sizeof( BITMAPFILEHEADER );

#ifdef	NEED_TWO_FILE

		if( ( hBMP = DVGlobalAlloc( GMEM_SHARE, (ddSz+2) ) ) &&
			( lphdr = (LPBITMAPFILEHEADER) DVGlobalLock( hBMP ) ) &&
			( lpbi = (VOID MLPTR) DVGlobalLock( hdib ) ) )  // LOCK DIB HANDLE
		{
			/* Fill in the fields of the file header */
			lphdr->bfType		= DIB_HEADER_MARKER;
			lphdr->bfSize		= ddSz;
			lphdr->bfReserved1     = 0;
			lphdr->bfReserved2     = 0;
			lphdr->bfOffBits       = (DWORD)sizeof(BITMAPFILEHEADER) + lpbi->biSize +
                          PaletteSize((LPSTR)lpbi);
			lphdr++;
			DVCopy( (LPSTR)lphdr, (LPSTR)lpbi, 
				(ddSz - sizeof( BITMAPFILEHEADER )));
			DVGlobalUnlock( hdib ); // UNLOCK DIB HANDLE
			DVGlobalUnlock( hBMP );
			lphdr = 0;
			if( !GetJPEGLib(4) )
			{
				if( !fShownNOLIB )
				{
					UINT	ui;
					fShownNOLIB = TRUE;
					//DIBError( ERR_NO_LIB );
					ui = Err_No_Lib();
				}
				goto WriteDRet;
			}
			ArgC = (*WDatToJpg) ( hBMP, ddSz, hIBuf, lpf );
			DVGlobalFree( hBMP );
			hBMP = 0;
			if( ArgC )
			{
				if( hIBuf )
					lpIBuf = DVGlobalLock( hIBuf );
				else
					lpIBuf = 0;
				if( lpIBuf ) AddFN( lpIBuf, lpf );
				if( lpIBuf && lpIBuf[0] )
  					DIBError2( ERR_NOLCONV, lpIBuf );
				else
					DIBError( ERR_NOLCONV );
				ArgC = 0;
			}
			else	/* Return is ZERO = SUCCESS */
			{	/* So signal SUCCESSFUL input of DATA and OUTPUT of JPEG */
				rflg = TRUE;	/* This will cause the TITLE to be CHANGED */
			}
		}
		else
		{
			DIBError( ERR_MEMORY );	/* Some memory error ... */
		}
#endif	// NEED_TWO_FILE

		goto WriteDRet;	/* All done here ... whether error or NOT ... */
	}	// If FType == ft_JPG

#endif	/* USEGFUNC3 */

	if( flg )	// On if FINAL OUPUT is NOT to be BMP.
	{
		lstrcpy( lpo, gszDrive );
		lstrcat( lpo, gszDir );
//		lstrcat( lpo, szFname );
		lstrcat( lpo, gszDefTemp );
		lstrcat( lpo, pRDefExt );	/* Add BMP extent ... */
		if( !FindNewName( lpo ) )	/* and get a unique name ... */
		{
      	DIBError( ERR_INTERROR );
			lpo = 0;
			goto WriteDRet;
		}
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
		if( !((FType == ft_JPG) || (FType == ft_BMP)) )
		{	/* If FINAL output is NOT JPG or BMP, then we must build a */
			/* TEMORARY JPG output file ... */
			lstrcpy( lpo2, gszDrive );
			lstrcat( lpo2, gszDir );
//			lstrcat( lpo2, szFname );
			lstrcat( lpo2, gszDefTemp );
			lstrcat( lpo2, ".jpg" );	// &szJpg[0] Add JPG extent
		}
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

	}

	// CREATE the OUTPUT FILE
	fh = DVOpenFile (lpo, &of, OF_CREATE|OF_READWRITE);
	// If we GOT a file,
	// Lock down the DIB
	if( ( fh == 0 ) ||
		( fh == (HANDLE)-1 ) ||
		( (lpbi = (VOID MLPTR)DVGlobalLock( hOutDIB )) == 0 ) )  // LOCK DIB HANDLE
	{
		if( fh && (fh != INVALID_HANDLE_VALUE) ) // HFILE_ERROR
			DVlclose (fh);
		if( gfBe_Tidy )
			DVOpenFile( lpo, &of, OF_DELETE );
		DIBError( ERR_WRITEDIB );	// FAILED to write BMP file!!!
		goto WriteDRet;
	}

	// Lock down the DIB
	//lpbi = (VOID MLPTR)DVGlobalLock( hOutDIB );

    // Fill in the fields of the file header
    hdr.bfType		= DIB_HEADER_MARKER;
//    hdr.bfSize		= GlobalSize (hdib) + sizeof (BITMAPFILEHEADER);
    hdr.bfSize		= dwDIBSz + sizeof (BITMAPFILEHEADER);
    hdr.bfReserved1     = 0;
    hdr.bfReserved2     = 0;
    hdr.bfOffBits       = (DWORD)sizeof(BITMAPFILEHEADER) + lpbi->biSize +
                          PaletteSize((LPSTR)lpbi);

    // Write the file header.
	if( DVWrite( fh, (VOID MLPTR)&hdr, sizeof (BITMAPFILEHEADER)) !=
		sizeof( BITMAPFILEHEADER ) )
	{
		DVGlobalUnlock( hOutDIB ); // UNLOCK DIB HANDLE - failed write return
		DVlclose (fh);
		if( gfBe_Tidy )
			DVOpenFile( lpo, &of, OF_DELETE );
		goto WriteDRet;
	}

    // Write the DIB header and the bits
//	if( DVWrite( fh, (VOID MLPTR)lpbi, GlobalSize (hdib)) !=
//		GlobalSize( hdib )  )
	if( DVWrite( fh, (VOID MLPTR)lpbi, dwDIBSz) !=
		dwDIBSz  )
	{
		DVGlobalUnlock( hOutDIB ); // UNLOCK DIB HANDLE - failed write return 2
		DVlclose (fh);
		if( gfBe_Tidy )
			DVOpenFile( lpo, &of, OF_DELETE );
		goto WriteDRet;
	}
	else if( FType == ft_BMP )
	{
		rflg = TRUE;	// Signal *** SUCCESS *** for BITMAP FILE
	}

	// Finished with the DIB
	DVGlobalUnlock( hOutDIB ); // UNLOCK DIB HANDLE - write success
	// And FILE IS WRITTEN
	// ===================
	DVlclose (fh);
#ifdef	WJPEG4
	if( flg )
	{
		DIBError( ERR_RETIRED );	// Interface RETIRED
	}
#else	// !WJPEG4
	if( flg )	// Written BMP. NOW to write JPG, then ...
	{
		ArgC = 0;
		JFiles[ArgC++] = lpIBuf;  // Error Message Buffer
		JFiles[ArgC++] = lpo;	// BMP INPUT File ...
		JFiles[ArgC++] = lpo2;	// JPG Output File ...
		JFiles[ArgC] = NULL;
#ifdef	ADDWJPEG
#ifdef	LDYNALIB
// =========================================
				if( !GetJPEGLib(5) )
				{
					if( !fShownNOLIB )
					{
						UINT	ui;
						fShownNOLIB = TRUE;
						//DIBError( ERR_NO_LIB );
						ui = Err_No_Lib();
					}
					goto WriteDRet;
				}
#ifdef	NEED_TWO_FILE
		if( fh = (*WWriteJpeg) (ArgC, &JFiles[0] ) )
#else	// !NEED_TWO_FILE
		if( fNeedTwo )
#endif	// NEED_TWO_FILE
// =========================================
#else	// !LDYNALIB
// =========================================
#ifdef	NEED_TWO_FILE
		if( fh = WWRITEJPEG( ArgC, &JFiles[0] ) )	/* Go DO IT ... */
#else	// !NEED_TWO_FILE
		if( fNeedTwo )
#endif	// NEED_TWO_FILE y/n
// =========================================
#endif	// LDYNALIB y/n
		{	/* Yikes, we FAILED */
			if( gfBe_Tidy )
				DVOpenFile( lpo, &of, OF_DELETE );
			if( lpIBuf && lpIBuf[0] )
      		DIBError2( ERR_NO_CONV, lpIBuf );
			else
				DIBError( ERR_NO_CONV );
			goto WriteDRet;
		}
		else
		{
			if( !((FType == ft_JPG) || (FType == ft_BMP)) )
			{
				if( gfBe_Tidy )
					DVOpenFile( lpo, &of, OF_DELETE );	/* Finished with BMP */
				lpo = lpo2;
				ArgC = 0;
				JFiles[ArgC++] = lpIBuf;  /* Error Message Buffer */
				if( lpsw )
					JFiles[ArgC++] = lpsw;
				JFiles[ArgC++] = lpo;	/* JPG INPUT file ... */
				JFiles[ArgC++] = lpf;		/* Final OUTPUT file ... */
				JFiles[ArgC] = 0;
#ifdef	LDYNALIB
			if( !GetJPEGLib(6) )
			{
				if( !fShownNOLIB )
				{
					UINT	ui;
					fShownNOLIB = TRUE;
					//DIBError( ERR_NO_LIB );
					ui = Err_No_Lib();
				}
				goto WriteDRet;
			}
			if( fh = (*WReadJpeg) (ArgC, &JFiles[0] ) )
#else	// !LDYNALIB
				if( fh = WREADJPEG( ArgC, &JFiles[0] ) )
#endif	// LDYNALIB y/n
				{	/* READ JPG and OUTPUT say GIF failed ... */
					if( gfBe_Tidy )
					{
						DVOpenFile( lpo, &of, OF_DELETE );
						DVOpenFile( lpf, &of, OF_DELETE );
					}
					if( lpIBuf && lpIBuf[0] )
      				DIBError2( ERR_NO_CONV, lpIBuf );
					else
						DIBError( ERR_NO_CONV );
					goto WriteDRet;
				}
			}
			rflg = TRUE;
		}
		if( gfBe_Tidy && lpo )
			DVOpenFile (lpo, &of, OF_DELETE);
#endif	/* ADDWJPEG */
	}
	else
	{
		rflg = TRUE;
	}
#endif	// WJPEG4 y/n
WriteDRet:

#ifdef	LDYNALIB
#ifndef	CVLIBBUG
		CloseJPEGLib();	/* If CVLIBBUG then only called from FRAME.c at exit */
#endif	// !CVLIBBUG
#endif	// LDYNALIB

#ifdef	USEGFUNC3
	if( hBMP && lphdr )
		DVGlobalUnlock( hBMP );
	if( hBMP )
		DVGlobalFree( hBMP );
#endif	// USEGFUNC3

	if( hIBuf && lpIBuf )
		DVGlobalUnlock( hIBuf );
	lpIBuf = 0;

	if( hIBuf )
		DVGlobalFree( hIBuf );
	hIBuf = 0;

   MFREE(prd);

	return( rflg );
}	// WriteDIB()



BOOL WriteBMP( LPSTR lpOutReq, HANDLE hOutDIB, BOOL fErrDlg )
{
	BITMAPFILEHEADER	hdr;
	LPBITMAPINFOHEADER  lpbi;
	HANDLE               fh;
	OFSTRUCT            of;
	BOOL				rflg;
	char				boutf[MAX_PATH+16];	//OutName[MAX_PATH+16];
	LPSTR				lpo;
	LPSTR				lpf;
	WORD				FType;
	DWORD				dwDIBSz;


	rflg = FALSE;	// NOT done yet
	lpf = lpOutReq;

	if( ( lpf == NULL ) ||
		( lpf[0] == 0 ) ||
		( hOutDIB == NULL ) ||
		( (dwDIBSz = GlobalSize( hOutDIB )) == 0 ) )
	{
		if( fErrDlg )
			DIBError( ERR_WRITEDIB );
		return rflg;
	}
	lpo = &boutf[0];
//#define	ft_BMP		1
	FType = ft_BMP;	// Ouput is BITMAP
	lstrcpy( lpo, lpf );	// COPY the NAME of the out file

	// CREATE the OUTPUT FILE
	fh = DVOpenFile (lpo, &of, OF_CREATE|OF_READWRITE);
	// If we GOT a file,
	// Lock down the DIB
	if( ( fh == 0 ) ||
		( fh == INVALID_HANDLE_VALUE) ||
		( (lpbi = (VOID MLPTR)DVGlobalLock( hOutDIB )) == 0 ) )  // LOCK DIB HANDLE
	{
		if( fh && (fh != INVALID_HANDLE_VALUE) )
			DVlclose (fh);
		if( gfBe_Tidy )
			DVOpenFile( lpo, &of, OF_DELETE );
		if( fErrDlg )
			DIBError( ERR_WRITEDIB );	// FAILED to write BMP file!!!
		return rflg;
	}

	// got locked down the DIB

    // Fill in the fields of the file header
    hdr.bfType		= DIB_HEADER_MARKER;
//    hdr.bfSize		= GlobalSize (hdib) + sizeof (BITMAPFILEHEADER);
    hdr.bfSize		= dwDIBSz + sizeof (BITMAPFILEHEADER);
    hdr.bfReserved1     = 0;
    hdr.bfReserved2     = 0;
    hdr.bfOffBits       = (DWORD)sizeof(BITMAPFILEHEADER) + lpbi->biSize +
                          PaletteSize((LPSTR)lpbi);

    // Write the file header.
	if( DVWrite( fh, (VOID MLPTR)&hdr, sizeof (BITMAPFILEHEADER)) !=
		sizeof( BITMAPFILEHEADER ) )
	{
		DVGlobalUnlock( hOutDIB ); // UNLOCK DIB HANDLE - for error write return
		DVlclose (fh);
		if( gfBe_Tidy )
			DVOpenFile( lpo, &of, OF_DELETE );
		if( fErrDlg )
			DIBError( ERR_WRITEDIB );
		return	rflg;
	}

    // Write the DIB header and the bits
//	if( DVWrite( fh, (VOID MLPTR)lpbi, GlobalSize (hdib)) !=
//		GlobalSize( hdib )  )
	if( DVWrite( fh, (VOID MLPTR)lpbi, dwDIBSz) !=
		dwDIBSz  )
	{
		DVGlobalUnlock( hOutDIB ); // UNLOCK DIB HANDLE - for another error write
		DVlclose (fh);
		if( gfBe_Tidy )
			DVOpenFile( lpo, &of, OF_DELETE );
		if( fErrDlg )
			DIBError( ERR_WRITEDIB );
		return rflg;
	}

	// Finished with the DIB
	DVGlobalUnlock( hOutDIB ); // UNLOCK DIB HANDLE - success write

	// And FILE IS WRITTEN
	// ===================
	DVlclose (fh);

	rflg = TRUE;

	return( rflg );

}	// WriteBMP() - Silently or with Modal ERROR DIALOG


BOOL WriteBMPFile( LPDIBINFO lpdi, LPSTR lpf )
{
	BOOL	rflg;
	HANDLE	hDIB;

	rflg = FALSE;
	if( lpdi &&
		( hDIB = lpdi->hDIB ) )
	{
		rflg = WriteBMP( lpf, hDIB, FALSE );
	}
	return rflg;
}

BOOL  WriteBMPFile2( LPTSTR lpf, HGLOBAL hDIB, BOOL bShow )
{
   BOOL                 flg = FALSE;
   OFSTRUCT             of;
   HANDLE                fh;
	BITMAPFILEHEADER	   hdr;
	LPBITMAPINFOHEADER   lpbi;
   DWORD                dws;

   if( ( lpf && *lpf && hDIB    ) &&
       ( dws = GlobalSize(hDIB) ) )
   {
	   // CREATE the OUTPUT FILE
	   fh = DVOpenFile( lpf, &of, OF_CREATE|OF_READWRITE );
	   // If we GOT a file,
      if( ( fh                ) &&
          ( fh != (HANDLE)-1 ) )
      {
         // Lock down the DIB
         if( lpbi = (LPBITMAPINFOHEADER)DVGlobalLock( hDIB ) )  // LOCK DIB HANDLE
         {
            // Fill in the fields of the file header
            ZeroMemory( &hdr, sizeof(BITMAPFILEHEADER) );
            hdr.bfType		 = DIB_HEADER_MARKER;
            hdr.bfSize		 = dws + sizeof (BITMAPFILEHEADER);
            hdr.bfReserved1 = 0;
            hdr.bfReserved2 = 0;
            hdr.bfOffBits   = (DWORD)sizeof(BITMAPFILEHEADER) +   // file header plus
                              lpbi->biSize +    // size of 	LPBITMAPINFOHEADER
                              PaletteSize((LPSTR)lpbi ); // plus PALETTE (if any)
            // write header to file
            if( DVWrite( fh, (LPSTR)&hdr, sizeof(BITMAPFILEHEADER)) == sizeof( BITMAPFILEHEADER ) )
            {
               if( DVWrite( fh, (LPSTR)lpbi, dws) == dws  )
               {
                  // write DIB to file
                  flg = TRUE;
               }
            }
      		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE
         }
      }

		if( fh && ( fh != (HANDLE)-1 ) )
			DVlclose (fh);

      if( !flg )
      {
		   if( gfBe_Tidy )
         {
			   DVOpenFile( lpf, &of, OF_DELETE );
         }
         if( bShow )
         {
            DIBError( ERR_WRITEDIB );	// FAILED to write BMP file!!!
         }
      }
	}

   return flg;

}

// eof - DvWrite.c
