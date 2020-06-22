
/* ********************************************

      File:  DVERRORS.C

   Purpose:  Contains the error message box handler.

 Functions:  DIBError()

  Comments:  Should use a string table here...We're unnecessarily
             eating up DS, and make it harder to localize for international
             markets...  Maybe next time...

   History:   Date     Reason

             6/1/91    Created

*********************************************** */

#include	"dv.h"

#define	MB_INFO		( MB_OK | MB_ICONINFORMATION )

/* ================================================
	Original was enum, but changed to FIXED from own DvRes.H
   ================================================================ */
extern	DWORD	dwMissedC;
extern	char	szMissedS[];

char	MLPTR pRPInfo = "\r\nwith infomation message"MEOR;
char	szMszF[] = "\r\nFunction: %s MISSED!";	// Missed service
char	szPers[] = "\r\nPerhaps copy the DLL to here!";

LPSTR	GetErrStg( DWORD );
LPSTR	GetInfStg( DWORD );

typedef struct tagERRMSGS {	/* em */
	DWORD	emVal;
	LPSTR	emData;
}ERRMSGS;
typedef	ERRMSGS MLPTR LPERRMSGS;

//BOOL	bShowWarn = TRUE;
//BOOL	bChgWarn = FALSE;

char	*pRUnDefE = "Undefined Error!";
char	*pRUnDefI = "Undefined Information!";
char	szDvWrn[] = "DIBVIEW WARNING";

ERRMSGS	ErrList[] = {
	{ ERR_NOT_DIB             ,"ERROR: ERR_NOT_DIB (5501)\r\nNot a DIB file! "},
	{ ERR_MEMORY              ,"ERROR: ERR_MEMORY (5502)\r\nCouldn't allocate memory! "},
	{ ERR_READ                ,"Error reading file! (3) "},
	{ ERR_LOCK                ,"Error locking memory! (4) "},
	{ ERR_OPEN                ,"Error opening file! (5) "},
	{ ERR_CREATEPAL           ,"Error creating palette! (6) "},
	{ ERR_GETDC               ,"Error getting a DC! (7) "},
	{ ERR_CREATECHILD         ,"Error creating MDI Child! (8) "},
	{ ERR_CREATEDDB           ,"ERROR: ERR_CREATEDDB (5509)\r\nError creating Device Dependent Bitmap! "},
	{ ERR_STRETCHBLT          ,"StretchBlt() failed! "},
	{ ERR_STRETCHDIBITS       ,"StretchDIBits() failed! "},
	{ ERR_NODIBORDDB          ,"Paint requires both DDB and DIB! "},
	{ ERR_SETDIBITSTODEVICE   ,"SetDIBitsToDevice() failed! "},
	{ ERR_STARTDOC            ,"Printer: StartDoc failed! "},
	{ ERR_NOGDIMODULE         ,"Printing: GetModuleHandle() couldn't find GDI! "},
	{ ERR_SETABORTPROC        ,"Printer: SetAbortProc failed! "},
	{ ERR_STARTPAGE           ,"Printer: StartPage failed! "},
	{ ERR_NEWFRAME            ,"Printer: NEWFRAME failed! "},
	{ ERR_ENDPAGE             ,"Printer: EndPage failed! "},
	{ ERR_ENDDOC              ,"Printer: EndDoc failed! "},
	{ ERR_ANIMATE             ,"Only one DIB can be animated at a time! "},
	{ ERR_NOTIMERS            ,"No timers available for animation! "},
	{ ERR_NOCLIPWINDOW        ,"Can't copy to clipboard -- no current DIB! "},
	{ ERR_CLIPBUSY            ,"Clipboard is busy -- operation aborted! "},
	{ ERR_NOCLIPFORMATS       ,"Can't paste -- no DIBs or DDBs in clipboard! "},
	{ ERR_SETDIBITS           ,"SetDIBits() failed! "},
	{ ERR_FILENOTFOUND        ,"File Not Found! "},
	{ ERR_WRITEDIB            ,"Error writing DIB to a file! "},
	{ ERR_NO_CONV             ,"ERROR: Can NOT convert file to Bitmap! "},
	{ ERR_WN_CONV             ,"WARNING: Error in file conversion! "},
	{ ERR_NO_RLE              ,"ERROR: Unsupported file format! "},
	{ ERR_UNKNOWNF            ,"ERROR: Unknown file format! "},
	{ ERR_INTERROR            ,"WARNING: Some internal failure! "},
	{ ERR_NO_LIB              ,"ERROR: Unable to load WJPEGxxx.DLL,"MEOR
		"and/or get PROC addresses! "},
	{ ERR_NO_REOPEN           ,"ERROR: Unable to RE-OPEN file! "}, /* ERR_NO_REOPEN */
	{ ERR_MEMHANDLE           ,"ERROR: Unable to ACCESS memory! "},	/* ERR_MEMHANDLE */
	{ ERR_NOLCONV             ,"ERROR: Library FAILED in conversion of file to Bitmap! "}, /* ERR_NOLCONV */
	{ ERR_NOLSIZE             ,"ERROR: Library FAILED to give SIZE of BMP! "},
	{ ERR_NOLSIZEG            ,"ERROR: Library FAILED in WGIFSIZ? function to give SIZE of BMP! "},
	{ ERR_NOLSIZEJ            ,"ERROR: Library FAILED in WJPGSIZE?"MEOR\
			"NO SIZE of BMP returned!!!"MEOR\
			"Usually means NOT JPEG format or contains an unsupported marker."MEOR\
			"NOTE: WJPGSIZE6 in WJPG2BMP.dll adds progressive decoding? "},
	{ ERR_NOLCONF             ,"ERROR: Library rejected Configuration parameters! "},
	{ ERR_NOGIF               ,"ERROR: Output of GIF file failed!" },
	{ ERR_COPYPAL             ,"Error creating a palette for a 24-Bit image!"MEOR\
			"If NOT already tried,"MEOR\
			"perhaps increase max. colors in General Options."MEOR\
			"Else CopyPalette failed!" },
	{ ERR_RETIRED             ,"ERROR: This/these Interfaces have been RETIRED! "},
	{ ERR_PARAMS              ,"ERROR: This is an INTERNAL Error"MEOR\
		"Incorrect PARAMETERS were passed to ChildWndCreat!"MEOR\
		"Aborting window creation! "},
	{ ERR_NULSIZE             ,"ERROR: File is ZERO Length!" },
	{ ERR_NOBMP2JPG           ,"ERROR: Library FAILED in conversion of BMP to JPEG file! "},
	{ ERR_NO_MRF, "ERROR: No most recent file list to manage!" },
	{ ERR_NO_COPY,"ERROR: Copying of DIB handle FAILED!\r\nPerhaps insufficient memory! " },
	{ ERR_NOT_YET,"PARDON! This function has NOT yet been implemented! " },
	{ ERR_NO_INFO,"ERROR: GetWindowExtra (WW_DIB_INFO) FAILED for this MDI Window! " },
	{ ERR_NO_LOCK,"ERROR: Handle to DIB Info structure failed to convert to memory! " },
	{ ERR_NO_PMDIB,"ERROR: Function PMDibFromBitmap FAILED to return a handle! " },
	{ ERR_NO_WINDIB,"ERROR: Function WinDibFromBitmap FAILED to return a handle! " },
	{ 0                       , 0 } };

ERRMSGS	InfList[] = {
	{ INF_LIBSET              ,"Appears WJPEG DLL has accepted changed Configuration! "},
	{ INF_LIBERR              ,"Warning: WJPEG DLL rejected changed Configuration! " },
	{ 0                       , 0 } };

int	DvMsgBox( HWND hWnd,
				 LPSTR lpText,
				 LPSTR lpTitle,
				 UINT  uType )
{
	int	i = -1;
	LPSTR	lpCap;
   PLASTERR pla = &g_lasterr; // FIX20080720 - avoid multiple error dialogs

	lpCap = lpTitle;
	if( lpCap == NULL )
	{
		lpCap = &szDvWrn[0];
	}

   // FIX20080720 - avoid multiple error dialogs
   if ((lstrcmp(pla->text, lpText) == 0) &&
       (lstrcmp(pla->title, lpCap) == 0) &&
       ( pla->type == uType )) {
       // already SHOWN this error
       sprtf( "Repeat of ERROR: %s AVOIDED ..."MEOR, lpText );
   } else {
      lstrcpy(pla->text, lpText);
      lstrcpy(pla->title, lpCap);
      pla->type = uType;

#ifdef	DIAGFILE2
	   if( lpText && *lpText )
		   DiagString( lpText );
#endif	// DIAGFILE2
	   i = MessageBox( hWnd, lpText, lpCap, uType );
   }
	return i;
}

// Just Dynamic Library loading ...
#if	(defined( ADDWJPEG ) && defined( LDYNALIB ))	
//#define	MXDLGMSG		1024	// 1K max. output ... message.
// return char	szJpegLib[] = DEF_JPEG_LIB;   // 32-bit = WJPEG32.DLL
extern	LPSTR	GetLibPtr( void );
extern	BOOL	GetRootDir( LPSTR );

UINT	Err_No_Lib( void )
{
	UINT	ui;
	int	i;
	HGLOBAL hg;
	LPSTR	lps, lpm;
	int		iData;
	char	buf[260];
#ifdef	_DvWarn_h
	WARNSTR	ws;
#endif	// _DvWarn_h

	hg = 0;
	lps = 0;
	iData = 0;
	ui = 0;
   // return char	szJpegLib[] = DEF_JPEG_LIB;   // 32-bit = WJPEG32.DLL
	if( (lpm = GetLibPtr() ) && // Get NAME
		(i = lstrlen( lpm )) )	// which HAS length ...
	{
		if( (hg = DVGlobalAlloc( GHND, (MXDLGMSG + i) ) ) &&
			(lps = DVGlobalLock( hg )) )
		{
			// BEGIN Error string. This should EXPLAIN
			// exactly WHY NO LIBRARY LOAD!!! In details.
			// =========================================

			// 1 - Get the current short ERROR string.
			lstrcpy( lps, GetErrStg( (DWORD)ERR_NO_LIB ) );

			// 2. Add the Current Working Directory
			wsprintf( (lps + lstrlen(lps)),
				"\r\nLibrary Name is: [%s]\r\nCurrent Folder is [",
				lpm );

			// But be prepared for a BLANK - And SHOW IT
			if( !GetRootDir( (lps + lstrlen(lps)) ) )
			{
				lstrcat( lps, "***No directory available!***" );
			}

			lstrcat( lps, "]!" );
			iData = lstrlen( lps );
			// If we KNOW the Services missed, then ADD THESE
			// ==============================================
			if( dwMissedC )
			{
				LPSTR lpnm, lpform;
				int		inm, inmt, imcnt;
				inm = 0;
				inmt = 0;
				imcnt = 0;
				lpnm = &szMissedS[0];	// Get the LIST
				lpform = &szMszF[0];
				if( ( inm = lstrlen( lpnm ) ) &&
					( (inmt + inm) < MXSERVS ) )
				{
					// This is the FIRST. This could be a FAILED LoadLibary
					if( (inm > 8) &&
						( lpnm[0] == 'L' ) &&
						( lpnm[1] == 'o' ) &&
						( lpnm[2] == 'a' ) &&
						( lpnm[3] == 'd' ) )
					{
						wsprintf( &buf[0],
							"ERROR: %s - Failed LOAD!",
							lpnm );
						lpnm += inm + 1;	// Bump NAME list
						inm = lstrlen( &buf[0] );
						inmt += inm;		// bump the total
						iData += inm;
					}
				}

				while( inm = lstrlen( lpnm ) )
				{
					if( (inmt + inm) < MXSERVS )
					{
						imcnt++;
						wsprintf( (lps + lstrlen(lps)),
							lpform,
							lpnm );
					}
					inmt += inm;		// bump the total
					lpnm += inm + 1;	// bump the ptr
					iData += inm + lstrlen( lpform );
				}
				if( imcnt )
				{
						wsprintf( (lps + lstrlen(lps)),
							"\r\nAdvice: Missed %u Functions ...",
							imcnt );
				}
			}
			else
			{
				if( ((iData = lstrlen( lps )) + lstrlen( &szPers[0] )) < MXDLGMSG )
				{
					lstrcat( lps, &szPers[0] );
				}
			}
#ifdef	_DvWarn_h
			if( gfShowWarn )
			{
				NULWARN( ws );
				lstrcat( lps,
					"\r\nThis disables the JPEG and GIF options."MEOR
					"Do you wish to SEARCH for the DLL?" );
				ws.lpText = lps;
				ws.bJustOK = FALSE;
				ws.bAddCheck = TRUE;    // add the CHECK button
				ws.bCheck = gfShowWarn; // and set flag appropriately
				ws.bChgCheck = FALSE;
				ui = ShowWarning( GetFocus(), &ws );   // up the dialog
				if( ws.bChgCheck )
				{
					gfShowWarn = ws.bCheck;
					gfChgShowWarn  = TRUE;
				}
				if( ( ui == IDOK ) ||
					( ui == IDYES ) )
				{
					ui = IDYES;
					lstrcpy( lps,
						"Regrettably,\r\n*** NOT YET IMPLEMENTED! ***" );
					ws.lpText = lps;
					ws.bJustOK = TRUE;
					ws.bAddCheck = FALSE;
					ShowWarning( GetFocus(), &ws );
				}
			}
#else	// !_DvWarn_h
			DvMsgBox( GetFocus (),
				lps,
				NULL,
				MB_INFO );
#endif	// _DvWarn_h y/n
		}
		else
		{
			if( !fShownNOLIB )
			{
				UINT	ui;
				fShownNOLIB = TRUE;
				//DIBError( ERR_NO_LIB );
				ui = Err_No_Lib();
			}
		}
	}
	else
	{
			if( !fShownNOLIB )
			{
				UINT	ui;
				fShownNOLIB = TRUE;
				//DIBError( ERR_NO_LIB );
				ui = Err_No_Lib();
			}
	}
	if( hg && lps )
		DVGlobalUnlock( hg );
	if( hg )
		DVGlobalFree( hg );
	return ui;
}

#endif	// ADDWJPEG & LDYNALIB

void	Err_No_Conv( LPSTR lpm )
{
	int	i;
	HGLOBAL hg;
	LPSTR	lps;
	hg = 0;
	lps = 0;
	if( lpm && 		/* If passed an addition message ... */
		(i = lstrlen( lpm )) )	/* which HAS length ... */
	{
		if( (hg = DVGlobalAlloc( GHND, (1024 + i) ) ) &&
			(lps = DVGlobalLock( hg )) )
		{
			lstrcpy( lps, GetErrStg( (DWORD)ERR_NO_CONV ) );
			wsprintf( (lps + lstrlen(lps)),
				"\r\nFile is:\r\n[%s]!",
				lpm );
			DvMsgBox( GetFocus (),
				lps,
				NULL,
				MB_INFO );
		}
		else
		{
			DIBError( ERR_NO_CONV );
		}
	}
	else
	{
		DIBError( ERR_NO_CONV );
	}
	if( hg && lps )
		DVGlobalUnlock( hg );
	if( hg )
		DVGlobalFree( hg );
}

void DIBInfo( DWORD InfNo )
{
   if( (InfNo > INF_MIN) && (InfNo < INF_MAX) )
   {
      DvMsgBox( GetFocus(), GetInfStg(InfNo), "DibView Information",
			MB_OK | MB_ICONINFORMATION);
   }
}

void DIBError( int ErrNo )
{
	UINT	ui = 0;
#if	(defined( ADDWJPEG ) && defined( LDYNALIB ))
	// Perhaps show "better" message ONCE ONLY
	if( ErrNo == ERR_NO_LIB )
	{
		ui = Err_No_Lib();
		return;
	}
#endif	// ADDWJEPG and LDYNALIB

   if ((ErrNo < ERR_MIN) || (ErrNo >= ERR_MAX))
   {
      DvMsgBox( GetFocus(),
		  pRUnDefE,
		  NULL,
		  MB_OK | MB_ICONHAND);
   }
   else
   {
	   // int DvMsgBox(
	   //    HWND hWnd,	// handle of owner window
	   //	LPCTSTR lpText,	// address of text in message box
	   // LPCTSTR lpCaption,	// address of title of message box  
	   // UINT uType 	// style of message box
	   // );

	   DvMsgBox( GetFocus(),
		   GetErrStg((DWORD)ErrNo),
		   NULL,
		   MB_INFO );
   }
}

void DvError( LPSTR lpMsg, LPSTR lpTit )
{
	LPSTR lpm, lpt;
	if( lpMsg && lpMsg[0] )
		lpm = lpMsg;
	else
		lpm = pRUnDefE;
	if( lpTit && lpTit[0] )
		lpt = lpTit;
	else
		lpt = &szDvWrn[0];
	DvMsgBox( GetFocus (),
		lpm,
		lpt,
		MB_OK | MB_ICONINFORMATION );
}

#define	MXMSGL		512
#define	MXTITL		64

void DvErrorN( WORD wMsg, WORD wTit )
{
	LPSTR	lpm, lpt;
	char	mbuf[MXMSGL+4];
	char	tbuf[MXTITL+4];
	int		i;
	if( ghDvInst )
	{
		lpm = &mbuf[0];
		lpt = &tbuf[0];
		if( wMsg )
		{
			if( ( i = LoadString( ghDvInst,	// handle of module
				wMsg,	// resource identifier
				lpm,	// address of buffer for resource
				MXMSGL ) ) == 0 ) // size of buffer
				lpm = pRUnDefE;
		}
		else
		{
			lpm = pRUnDefE;
		}
		if( wTit )
		{
			if( ( i = LoadString( ghDvInst,	// handle of module
				wTit,	// resource identifier
				lpt,	// address of buffer for resource
				MXTITL ) ) == 0 ) // size of buffer
				lpt = &szDvWrn[0];
		}
		else
		{
			lpt = &szDvWrn[0];
		}
	}
	else
	{
		lpm = pRUnDefE;
		lpt = &szDvWrn[0];
	}
	DvError( lpm, lpt );
}

void DIBError2( int ErrNo, LPSTR lpm )
{
int	i;
HGLOBAL hg;
LPSTR	lps;
	hg = 0;
	lps = 0;
	if( lpm && 		/* If passed an addition message ... */
		(i = lstrlen( lpm )) &&	/* which HAS length ... */
		(ErrNo >= ERR_MIN) &&	/* and number in range ... */
		(ErrNo < ERR_MAX) )
	{
		if( (hg = DVGlobalAlloc( GHND, (1024 + i) ) ) &&
			(lps = DVGlobalLock( hg )) )
		{
			lstrcpy( lps, GetErrStg((DWORD)ErrNo) );
			lstrcat( lps, pRPInfo );
			lstrcat( lps, lpm );
			DvMsgBox( GetFocus (),
				lps,
				NULL,
				MB_OK | MB_ICONHAND);
		}
		else
		{
			DIBError( ErrNo );
		}
	}
	else
	{
		DIBError( ErrNo );
	}
	if( hg && lps )
		DVGlobalUnlock( hg );
	if( hg )
		DVGlobalFree( hg );
}

void SysDIBError( int ErrNo, DWORD dwi )
{
	int		i;
	HGLOBAL hg;
	LPSTR	lps, lperr;
	LPVOID	lpMsgBuf;
	DWORD	dwl;

	hg = 0;
	lps = 0;
	lpMsgBuf = 0;
	dwl = 0;
	if( ( dwi ) && 		// If passed an addition message
		( ErrNo >= ERR_MIN ) &&	// and number in range
		( ErrNo < ERR_MAX ) )
	{
		lperr = GetErrStg( (DWORD)ErrNo );
		i = lstrlen( lperr );
		dwl = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwi,		// GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL );
		if( dwl && lpMsgBuf )
		{
			if( (hg = DVGlobalAlloc( GHND, (1024 + i) ) ) &&
				(lps = DVGlobalLock( hg )) )
			{
				lstrcpy( lps, lperr );
				lstrcat( lps, "\r\nSystem Error Message"MEOR );
				lstrcat( lps, lpMsgBuf );
 				DvMsgBox( GetFocus (),
					lps,
					NULL,
					MB_INFO );
			}
			else
			{
				DIBError( ErrNo );
			}
		}
		// Free the buffer.
		if( lpMsgBuf )
			LocalFree( lpMsgBuf );
	}
	else
	{
		DIBError( ErrNo );
	}
	if( hg && lps )
		DVGlobalUnlock( hg );
	if( hg )
		DVGlobalFree( hg );
}

void DIBErrorStg( LPSTR lpm )
{
	DvMsgBox( GetFocus(),
		lpm,
		NULL,
		MB_INFO );
}

/* ****************************************
 * DIBEString() - Get the indexed error string
 * **************************************** */
int DIBEString( LPSTR lpb, int ErrNo )
{
int i;
	i = 0;
   if( lpb && !((ErrNo < ERR_MIN) || (ErrNo >= ERR_MAX)) )
   {
      lstrcpy( lpb, GetErrStg((DWORD)ErrNo) );
		i = lstrlen( lpb );
   }
return( i );
}	/* end DIBEString() */

LPSTR	GetErrStg( DWORD er )
{
   LPERRMSGS	lpe;
   LPSTR	lps;
   DWORD	ce;
	lps = (LPSTR) pRUnDefE;	/* = "Undefined Error!"; */
	lpe = &ErrList[0];
	while( ce = lpe->emVal )
	{
		if( ce == er )
		{
			if( lpe->emData )
				lps = (LPSTR) lpe->emData;
			break;
		}
		lpe++;
	}
   return( lps );
}

LPSTR	GetInfStg( DWORD er )
{
   LPERRMSGS	lpe;
   LPSTR	lps;
   DWORD	ce;
	lps = (LPSTR) pRUnDefI;	/* = "Undefined Information!"; */
	lpe = &InfList[0];
	while( ce = lpe->emVal )
	{
		if( ce == er )
		{
			if( lpe->emData )
				lps = (LPSTR) lpe->emData;
			break;
		}
		lpe++;
	}
   return( lps );
}


#ifdef	ADDOPENALL
//#define	MXSBUF	260

/* ------------------------  global variables  ------------------------- */
char	*pRETit = "ERROR" ;
char	*pRQTit = "QUERY" ;
char	*pRSUnkE = "ERROR: An error but actual Error string resource failed to load!";
char	szStgBuf[MXSBUF];

BOOL	fStopOnErr = TRUE;
BOOL	fErrStop = TRUE;

/* ----------------------- external variables -------------------------- */

void	SRShwErr( LPSTR lpm, LPSTR lpt )
{
//LPSTR	lps;
//	lps = &szSBuf[0];
//	wsprintf( lps, "%s - %s", lpt, lpm );
//	SRSend( lps );
}

/* --------------------- SRError ------------------------------------ */
/*
 *
 *  FUNCTION   : SRError( LPSTR lpDisp, LPSTR lpTitle )
 *
 *  PURPOSE    : To display an ERROR DIALOG
 *
 *  RETURNS    : NONE
 *
 */
void SRError( LPSTR lpDsp, LPSTR lpTit )
{
LPSTR	lpT;
	if( lpTit )
		lpT = lpTit;
	else
		lpT = pRETit;
	SRShwErr( lpDsp, lpT );
	if( fErrStop )
	{
		DvMsgBox( NULL, lpDsp, lpT,
			MB_OK | MB_ICONSTOP | MB_TASKMODAL ) ;
	}
}

/* --------------------- SRErrorN ------------------------------------ */
/*
 *
 *  FUNCTION   : SRError( UINT rID, LPSTR lpTitle )
 *
 *  PURPOSE    : To display an ERROR DIALOG using a RESOURCE ID
 *
 *  RETURNS    : NONE
 *
 */
LPSTR	SRGetEStg( UINT rID )
{
LPSTR	lpD;
int	slen;
	lpD = &szStgBuf[0];
	if( !(slen = LoadString( ghDvInst, rID, lpD, sizeof( szStgBuf ) ) ) )
 		lpD = pRSUnkE;	/* Use a canned string ... */
return( lpD );
}

void	SRErrorN( UINT rID, LPSTR lpTit )
{
	LPSTR	lpD;
	if( lpD = SRGetEStg( rID ) )
	{
		SRError( lpD, lpTit );
	}
}

/* --------------------- SRQuery ------------------------------------ */
/*
 *
 *  FUNCTION   : SRQuery( LPSTR )
 *
 *  PURPOSE    : To display a QUERY DIALOG asking YES or NO
 *
 *  RETURNS    : TRUE  = Yes
 *               FALSE = No
 *
 */
BOOL	SRQuery( LPSTR lpMsg )
{
BOOL	flg;
int	rep;
	SRShwErr( lpMsg, (LPSTR) pRQTit );
	rep = DvMsgBox( NULL, lpMsg, pRQTit,
		MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL | MB_DEFBUTTON1) ;
	if( rep == IDYES )
		flg = TRUE;
	else
		flg = FALSE;
return( flg );
}

#endif	/* ADDOPENALL */

/* eof - End DvErrors.c */
