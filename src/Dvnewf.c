

// DvNewF.c

#include	"dv.h"

#define		MXDELLIST		60
#define		MXDELBUFF		(MXDELLIST * MAX_PATH)

// Local
int		CheckIfFileExists( LPSTR );
BOOL	FindNewName( LPSTR );
void	DVGetFPath( LPTSTR, LPTSTR, LPTSTR, LPTSTR, LPTSTR );
void	NewFileIni( LPSTR );

char	szDefNF[] = "TEMPDV32.FIL";
//char	szNewFile[MAX_PATH+16];
HGLOBAL	hgDelList = 0;
DWORD	dwDelCnt  = 0;
DWORD	dwDelOff  = 0;

/* *************************************************************

   Function:  CheckIfFileExists ( LPSTR )

   Purpose:  Checks to see if the user selected file exists.

   Returns:  TRUE if the file is opened.
             FALSE if the file does not exists.

   Comments: After checking the file, the file is immediately closed
               again (using the OF_EXIST flag).

   History:  Date      Reason
             6/27/91   Created

   ***************************************************************** */

int CheckIfFileExists( LPSTR pszFilename )
{
   HANDLE      hFile;
   //OFSTRUCT ofstruct;
   int	   ret = FALSE;

	if( ( pszFilename    ) &&
       ( pszFilename[0] ) )
   {
	   /* We appear to have a NAME, so ... */
		hFile = DVOpenFile( pszFilename, &gsOFS, OF_EXIST );  // &ofstruct, OF_EXIST );
		if( ( hFile                ) &&
          ( hFile != (HANDLE)-1 )) // HFILE_ERROR 
		{
			ret = TRUE;
		}
	}
   return( ret );
}

// ======================================================
// BOOL	FindNewName( LPSTR lpf )	// get a unique name
// ======================================================
//#define	gszDrive		W.w_szDrive
//#define	gszDir			W.w_szDir
//#define	gszFname		W.w_szFname
//#define	gszExt			W.w_szExt
BOOL	FindNewName( LPSTR lpf )
{
   BOOL      flg;
   //char	FilNam[9];
   //char	FilExt[4];
   //char	FilDrv[3];
   //char	NewName[260];
   DWORD	   i, j, k;
   HFILE	   hf;
   TCHAR    c;
   DWORD	   nl, el, dl;
   BOOL	   fExt;
   DWORD	   wOff;
   LPTSTR	lpn, lpfn, lpfe, lple, lpd;

	lpn  = &gszNewName[0];
	lpfn = &gszFname[0]; // FilNam[0];
	lpfe = &gszExt[0];   // FilExt[0];
	lpd  = &gszDrive[0]; // FilDrv[0];
	flg = FALSE;

Try_Nxt:
	lple = lpf;
	if( lpf && (i = lstrlen( lpf )) )
	{
		hf = _lopen( lpf, (OF_READ | OF_SHARE_COMPAT) );
		if( hf && (hf != HFILE_ERROR) ) // 
		{
			DVlclose( IntToPtr(hf) );
			wOff = 0;
			*lpfn = 0;  // FilNam[0] = 0;
			*lpfe = 0;  // FilExt[0] = 0;
			*lpn  = 0;  // NewName[0] = 0;
			*lpfe = 0;  // FilDrv[0] = 0;
			nl = 0;
			el = 0;
			dl = 0;
			fExt = FALSE;
			for( j = 0; j < i; j++ )
			{
				c = lpf[j];
				if( j && (c == ':') )
				{
					lpd[0] = lpf[j-1];
					lpd[1] = c;
					lpd[2] = 0;
					lstrcpy( lpn, lpd );
					dl = 2;
					lple = lpf + j + 1;		/* From this location ... */
				}
				else if( c == '\\' )
				{
					k = lstrlen( lpn );
					nl = 0;
					el = 0;
					lpf[j] = 0;
					lstrcat( lpn, lple );	/* Take from the last ... */
					lstrcat( lpn, "\\" );	/* plus the directory mark ... */
					lple = lpf + j + 1;	/* Next copy from loaction ... */
					wOff = j;
					*lpfn = 0;  // FilNam[0] = 0;
					*lpfe = 0;  // FilExt[0] = 0;
				}
				else if( c == '.' )
				{
					fExt = TRUE;
					el = 0;
				}
				else
				{
					if( fExt )
					{
						lpfe[el] = c;
						el++;
						if( el > 3 )
							el = 3;
					}
					else
					{
						lpfn[nl] = c;
						nl++;
						if( nl > 8 )
							nl = 8;
					}
				}
			}	/* For length of given name ... */
			lpfn[nl] = 0;	/* The end of the name ... */
			lpfe[el] = 0;	/* and extent ... */
			k = lstrlen( lpfe );
			k = lstrlen( lpfn );	/* Get LENGTH of name ... */
			if( nl == 8 )
			{
				for( j = 8; j > 0; j-- )
				{
					c = lpfn[j-1];	/* Get last, 2nd last, etc,  character ... */
					if( ( c >= '0' ) && ( c <= '9' ) )	/* if already a NUM...*/
					{
						if( c < '9' )
						{
							c++;	/* Bump number, ... */
							break;	/* and ALL FINISHED this modification */
						}						
						else
						{
							c = '0';	/* Zero this number 9 ... and backup */
							lpfn[j-1] = c;	/* Insert NEW char ... */
						}
					}
					else
					{
						c = '0';	/* Convert to a NUMBER! ie Set to ZERO ... */
						break;	/* This is a mod. to check ... */
					}
				}
				lpfn[j-1] = c;	/* Insert NEW char ... */
			}
			else	/* Currently name is NOT full length, so add to it ... */
			{
				for( j = nl; j < 8; j++ )
					lpfn[j] = '0';	/* Start with a added ZEROS ... */
				lpfn[j] = 0;	/* and terminate ... */
				nl = 8;
			}
			lstrcat( lpn, lpfn );	/* Add NAME portion ... */
			if( el )	/* If we have an EXTENT ... */
			{
				lstrcat( lpn, "." );	/* At full stop ... */
				lstrcat( lpn, lpfe );	/* and move back extent ... */
			}
			lstrcpy( lpf, lpn );	/* and back to User's buffer ... */
			goto Try_Nxt;	/* see if this exists ... */
		}
		else
		{
			flg = TRUE;	/* We have FOUND a NEW file name ... DOES NOT EXIST */
		}
	}
   return( flg );
}

/* =====================================================================
 *
 * DVGetFPath( lpFullPath, lpDrive, lpPath, lpFile, lpExtent )
 *
 * Purpose: Split the supplied FULL PATH into four components if
 * the buffers are supplied. Those components NOT required may be
 * NULL. This is a FAR PTR implementation of _splitpath in STDLIB.H.
 * The supplied buffers must be at least equal to the manifest contants
 * of _MAX_DRIVE, _MAX_DIR, _MAX_FNAME, _MAX_EXT in stdlib.h ...
 * The Drive will include the ':'; The Directory will include both the
 * leading and trailing '\' or '/'; The file name will be exactly that;
 * the Extent will include the '.' character; Such that a recombination
 * by say lstrcat will put it all back together ...
 * Any component requested that does not exist in the FULL PATH will 
 * contain a nul string.
 *
 * ===================================================================== */
void	DVGetFPath( LPTSTR lpc, LPTSTR lpd, LPTSTR lpp, LPTSTR lpf, LPTSTR lpe )
{
	int	i, j, k;
	LPTSTR	lps;
	char	c;

	lps = lpc;
	if( lpd )
		lpd[0] = 0;
	if( lpp )
		lpp[0] = 0;
	if( lpf )
		lpf[0] = 0;
	if( lpe )
		lpe[0] = 0;

	if( ( lps                ) &&
		 ( i = lstrlen( lps ) ) )
	{
		if( lps[1] == ':' )	/* If a DRIVE ... */
		{	
			k = 2;
			if( lpd )	/* If a DRIVE requested ... */
			{
				lpd[0] = lps[0];
				lpd[1] = lps[1];
				lpd[2] = 0;
			}
			lps += 2;
			if( k <= i )
				i = i - k;
			else
				i = 0;
		}
		if( i )
		{
			c = lps[0];
			k = 0;
			if( (c == '\\') || (c == '/') )
			{
				for( j = 0; j < i; j++ )
				{
					c = lps[j];
					if( (c == '\\') || (c == '/') )
						k = j + 1;	/* include this char in move ... */
					if( lpp )
					{
						if( j < _MAX_DIR )
							lpp[j] = c;
					}
				}
				if( lpp )
				{
					if( k < _MAX_DIR )
						lpp[k] = 0;	/* Put the NUL at the correct place ... */
					else
						lpp[_MAX_DIR - 1] = 0;
				}
			}
			lps += k;
			if( k <= i )
				i = i - k;
			else
				i = 0;
		}
		if( i )
		{
			k = 0;
			for( j = 0; j < i; j++ )
			{
				c = lps[j];
				if( c == '.' )
				{
					k = j;
					break;
				}
				if( lpf )
				{
					if( j < _MAX_FNAME )
						lpf[j] = c;
				}
			}
			lps += k;
			if( k <= i )
				i = i - k;
			else
				i = 0;
			if( lpf )
			{
				if( j < _MAX_FNAME )
					lpf[j] = 0;
				else
					lpf[_MAX_FNAME - 1] = 0;
			}
		}
		if( i && (c == '.') )
		{
			for( j = 0; j < i; j++ )
			{
				c = lps[j];
				if( lpe )
				{	
					if( j < _MAX_EXT )
						lpe[j] = c;
				}
			}
			if( lpe )
			{
				if( j < _MAX_EXT )
					lpe[j] = 0;
				else
					lpe[_MAX_EXT - 1] = 0;
			}
		}
	}
}

// Purpose: Return ONLY the file "title"
// That is exclude any drive and path
// =====================================
//#define	gszDrive		W.w_szDrive
//#define	gszDir			W.w_szDir
//#define	gszFname		W.w_szFname
//#define	gszExt			W.w_szExt
int	DVGetFileTitle( LPTSTR lpfull, LPTSTR lptitle )
{
	int		i = 0;
//	TCHAR	nm[_MAX_FNAME];
//	TCHAR	ext[_MAX_EXT];
	if( ( lpfull  ) &&
		 ( lptitle ) )
	{
      // given FULL PATH return only Name and Extent
		DVGetFPath( lpfull, 0, 0, lptitle, &gszExt[0] );
		//lstrcpy( lptitle, &nm[0] );
		lstrcat( lptitle, &gszExt[0] );
		i = lstrlen( lptitle );
	}
	return i;
}

int	DVGetFilePath( LPTSTR lpfull, LPTSTR lppath )
{
   int		i;
	//TCHAR	   pth[_MAX_FNAME];
   // given FULL PATH FILE NAME return only FOLDER
	DVGetFPath( lpfull, lppath, &gszDir[0], 0, 0 );
   strcat( lppath, &gszDir[0] );
   i = (int)strlen(lppath);
   return i;
}

// specialised bump of JUST the file NAME
// NO PATH and NO EXTENT passed
void	BumpName8( LPSTR lps )
{
	int		i;
	char	c;
	if( lps && (i = lstrlen( lps ) ) )
	{
		if( i < 8 )
		{
			lps[i++] = '0';
			lps[i] = 0;
		}
		else
		{
			while( i-- )
			{
				c = lps[i];
				if( (c >= '0') && (c <= '9') )
				{
					if( c == '9' )
					{
						lps[i] = '0';
					}
					else
					{
						c++;
						lps[i] = c;
						break;
					}
				}
				else
				{
					lps[i] = '0';
					break;
				}
			}
		}
	}
}

/* ====================================================
 * BOOL	DVGetNewName( LPTSTR lpnf )
 *
 * Purpose: Get next file name by bumping given name,
 *       and check for existance of that name
 *
 */
BOOL	DVGetNewName( LPTSTR lpnf )
{
	BOOL	flg = FALSE;
	static TCHAR   sszDrive[_MAX_DRIVE+2];
	static TCHAR   sszDir[_MAX_DIR+2];
	static TCHAR	sszFName[_MAX_FNAME+2];
	static TCHAR	sszExt[_MAX_EXT+2];
	static TCHAR	sszNew[MAX_PATH+16];
	LPTSTR	lpd, lpf, lpn, lpe, lpnew;
	if( lpnf && lpnf[0] )
	{
      // setup some static buffers
   	lpd   = &sszDrive[0];
	   lpf   = &sszDir[0];
	   lpn   = &sszFName[0];
	   lpe   = &sszExt[0];
	   lpnew = &sszNew[0];
		lstrcpy( lpnew, lpnf );
		if( CheckIfFileExists( lpnew ) )
		{
			do
			{
				DVGetFPath( lpnew, lpd, lpf, lpn, lpe );
				BumpName8( lpn );
				lstrcpy( lpnew, lpd );
				lstrcat( lpnew, lpf );
				lstrcat( lpnew, lpn );
				lstrcat( lpnew, lpe );
			} while( CheckIfFileExists( lpnew ) );
			if( CheckIfFileExists( lpnew ) == 0 )
				flg = TRUE;
		}
		else
		{
			flg = TRUE;
		}
		if( flg )
			lstrcpy( lpnf, lpnew );
	}
	return( flg );
}

LPSTR	GetNewFile( void )
{
	LPSTR	lpf, lpl;
	int		i;
	lpf = &gszNewFile[0];
	if( DVGetNewName( lpf ) &&
		( i = lstrlen( lpf ) ) )
	{
		if( hgDelList == 0 )
		{
			hgDelList = DVGlobalAlloc( GHND, MXDELBUFF );
		}
		if( hgDelList &&
			( lpl = DVGlobalLock( hgDelList ) ) )
		{
			if( dwDelCnt )
			{
				lstrcpy( (lpl+dwDelOff), lpf );
				dwDelOff += i + 1;
			}
			else
			{
				lstrcpy( lpl, lpf );
				dwDelOff = i + 1;
			}
			dwDelCnt++;
			lpl[dwDelOff] = 0;
			DVGlobalUnlock( hgDelList );
		}
	}
	else
	{
		lpf = 0;
	}
	return( lpf );
}

void	NewFileTerm( void )
{
	HGLOBAL	hg;
	DWORD	dc;
	LPSTR	lpb;
	int		i, j;
	//OFSTRUCT ofstruct;

	lpb = 0;
	if( ( dc = dwDelCnt ) &&
		( hg = hgDelList ) &&
		( lpb = DVGlobalLock( hg ) ) )
	{
		j = 0;
		while( i = lstrlen( (lpb+j) ) )
		{
			DVOpenFile( (lpb + j), &gsOFS, OF_DELETE );  // &ofstruct, OF_DELETE);
			j += i + 1;
		}
	}
	if( hgDelList && lpb )
		DVGlobalUnlock( hgDelList );
	if( hgDelList )
		DVGlobalFree( hgDelList );
	hgDelList = 0;
	dwDelCnt  = 0;
	dwDelOff  = 0;
}

void	NewFileIni( LPSTR lpd )
{
	LPSTR	lpf;
	int		i;
	char	c;
	lpf = &gszNewFile[0];
	lpf[0] = 0;
	if( lpd && lpd[0] )
	{
		lstrcpy( lpf, lpd );
		if( i = lstrlen( lpf ) )
		{
			c = lpf[i - 1];
			if( !((c == '\\') || (c == '/')) )
				lstrcat( lpf, "\\" );
		}
	}
	lstrcat( lpf, &szDefNF[0] );
}

/* ==============================================================
 * BOOL  DVGetFullName( LPTSTR lpd, LPTSTR lpf )
 *
 * Purpose: Convert a relative path to FULL name, including drive
 *
 * Library: <stdlib.h>
 *
 */
BOOL  DVGetFullName( LPTSTR lpd, LPTSTR lpf )
{
   BOOL  bRet = FALSE;
   if( _fullpath( lpd, lpf, 256 ) )
      bRet = TRUE;
   return bRet;
}

VOID  DVGetFullName2( LPTSTR lpd, LPTSTR lpf )
{
   DVGetFullName( lpd, lpf ); // relative to FULL PATH NAME
   DVGetFileTitle( lpd, lpf ); // and only get file TITLE
   return;
}


VOID  DVNextRDName( PRDIB prd, LPTSTR pfm, DWORD dwn )
{
   LPTSTR   lpn;
   LPTSTR   lpt;
   if( ( prd && pfm ) &&
       ( lpn = prd->rd_pPath  ) &&
       ( lpt = prd->rd_pTitle ) )
   {
		wsprintf( lpt, pfm, dwn );
      DVGetFullName( lpn, lpt );
      DVGetNewName(  lpn );
      DVGetFileTitle( lpn, lpt );
   }
}




///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : DVNextRDName2
// Return type: VOID 
// Arguments  : PRDIB prd - when READING/ACQUIRING a DIB/BMP some variables
//            : LPTSTR pfm - TEMPB???.BMP base name/title
//            : PDWORD pdwn
//            : PBOOL pChg
// Description: get the NEXT ***TEMPORARY*** file name
//              
///////////////////////////////////////////////////////////////////////////////
VOID  DVNextRDName2( PRDIB prd, LPTSTR pfm, PDWORD pdwn, PBOOL pChg )
{
   LPTSTR   lpn;
   LPTSTR   lpt;
   DWORD    dwb, dwn;

   if( ( prd && pfm && pdwn   ) &&  // if passed at least these three parameters
       ( lpn = prd->rd_pPath  ) &&  // and given a pointer to a full path
       ( lpt = prd->rd_pTitle ) )   // and a pointer to a title file name
   {
      dwb = dwn = *pdwn;   // extract number
      sprintf( lpt, pfm, dwn ); // prepare title file name
	  if (gszCacheData[0]) {
		  strcpy(lpn, gszCacheData);
		  strcat(lpn, lpt);
	  } else if( gszRunTime[0] ) {
         // split runtime module file into some work pads
         DVGetFPath( gszRunTime, gszDrive, gszDir, gszFname, gszExt );
         strcpy( lpn, gszDrive );
         strcat( lpn, gszDir   );   // set the base directory
         strcat( lpn, lpt      );
      } else {
         // if we want it created LOCAL, then ...
         DVGetFullName( lpn, lpt ); // get the FULL PATH name
      }

      while( CheckIfFileExists(lpn) )
      {
         dwn++;
         dwn = (dwn % 1000);
         sprintf( lpt, pfm, dwn ); // prepare title file name
         //DVGetFullName( lpn, lpt ); // get the FULL PATH name
		 if (gszCacheData[0]) {
			 strcpy(lpn, gszCacheData);
			 strcat(lpn, lpt);
		 }
		 else if (gszRunTime[0]) {
			 // split runtime module file into some work pads
			 DVGetFPath(gszRunTime, gszDrive, gszDir, gszFname, gszExt);
			 strcpy(lpn, gszDrive);
			 strcat(lpn, gszDir);   // set the base directory
			 strcat(lpn, lpt);
		 }
		 else {
			 // if we want it created LOCAL, then ...
			 DVGetFullName(lpn, lpt); // get the FULL PATH name
		 }
		 if( dwn == dwb )
         {
            // if full cycle
            break;
         }
      }
      dwn++;   // bump to NEXT
      *pdwn = dwn; // pass back NEXT number
      if( pChg ) {  // only if PASSED a change flag pointer
         *pChg = TRUE;  // set the change
      }
   }
}


// Eof - DvNewF.c
