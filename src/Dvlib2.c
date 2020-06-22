
// ============================================================
// DvLib2.c
//
// Interface to WJPEGxxx.DLL set of DLL's
// Specifically -
// WJPG32_2.DLL - Added June, 1997 - grm
// WDJPEG.DLL and WJPG2BMP.DLL are all the SAME
//
// This module has TWO main forms via the COMBWLIB switch.
// If defined( COMBWLIB ) then
//		DV is LINKED with the Library Source, thus all library
//		services are resolved by the LINK at MAKE time.
// Else (NOT COMBWLIB) then
//		DV will dynamically LOAD the DLL, and try to RESOLVE
//		the library service addresses at RUNTIME.
// ============================================================
#define		_DvLib_c
// === Above shuts off certain expression(s) in DvFunc.h below

#include	"dv.h"
#include	"DvFunc.h"

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now
//////////////////////////////////////////////////////////
#include	"DvLib.h"

#define		TRYB2J6
#undef		ADDTMDIAG2
#define		FMEMDIAGS
#undef		ADDTMDIAGS
#define		USEIJG6

// Include the COMMON CONFIG structure, but certain
// "enumeration" take place, now in JpgStru6.h ...
/* Known color spaces. */

typedef enum {
	JCS_UNKNOWN,		/* error/unspecified */
	JCS_GRAYSCALE,		/* monochrome */
	JCS_RGB,		/* red/green/blue */
	JCS_YCbCr,		/* Y/Cb/Cr (also known as YUV) */
	JCS_CMYK,		/* C/M/Y/K */
	JCS_YCCK		/* Y/Cb/Cr/K */
} J_COLOR_SPACE;

/* DCT/IDCT algorithm options. */

typedef enum {
	JDCT_ISLOW,		/* slow but accurate integer algorithm */
	JDCT_IFAST,		/* faster, less accurate integer method */
	JDCT_FLOAT		/* floating-point: accurate, fast on fast HW */
} J_DCT_METHOD;

#ifndef JDCT_DEFAULT		/* may be overridden in jconfig.h */
#define JDCT_DEFAULT  JDCT_ISLOW
#endif
#ifndef JDCT_FASTEST		/* may be overridden in jconfig.h */
#define JDCT_FASTEST  JDCT_IFAST
#endif

/* Dithering options for decompression. */

typedef enum {
	JDITHER_NONE,		/* no dithering */
	JDITHER_ORDERED,	/* simple ordered dither */
	JDITHER_FS		/* Floyd-Steinberg error diffusion dither */
} J_DITHER_MODE;

#if	(defined( _INC_WINDOWS ) || defined( WJPEG6 ))

// To build a 32-bit Library
#define EXPORT32 __declspec(dllexport)

// Make these LIBRARY headers
#define GLOBAL(type)		EXPORT32 type
#define EXTERN(type)		extern EXPORT32 type

#else	// !_INC_WIND

/* a function referenced thru EXTERNs: */
#define GLOBAL(type)		type
/* a reference to a GLOBAL function: */
#define EXTERN(type)		extern type

#endif	// _INC_WIND y/n
// Finally
#define JPP(arglist)	arglist

// NOW we can include this header... NOT GOOD, but how to SHARE
// ============================================================
//#include	"D:\DOWN\JPEGSRC\WComCfg.H"
// Feb 1998 - Moved DV32 to F:\GTOOLS32\DV32 (base), and
// the WJPG32_2.DLL source to (base)\WJPG32_2
#include	"../WJPG32_2/WComCfg.H"

// Forward reference
void	BuildLibFilter( LPSTR lpFilter );
// Present in ALL cases
// ====================
BOOL	fLoadLib6 = FALSE;
BOOL	bDnLib2 = FALSE;
// ====================
JCOMCFG	AppJComCfg = { 
	DEF_J2_CFG
};


#ifdef		ADDTMDIAG2
void Do_Time_Trial2( HGLOBAL hg1, DWORD dw, HGLOBAL hg2 );
extern	LPSTR	DVf2s( double source );
#endif	// ADDTMDIAG2

#ifdef	FMEMDIAGS
#include	"DvMemF.h"
extern	LPJPEG_EXTMM	GetMemMgr( void );
BOOL	PassMemMgr( void );
#endif	// FMEMDIAGS

#ifdef	COMBWLIB
// ==================================================

LPWJPGSIZE6	WJpgSize6 = &WJPGSIZE6;
LPWJPG2BMP6	WJpg2Bmp6 = &WJPG2BMP6;
LPWBMP2JPG6	WBmp2Jpg6 = &WBMP2JPG6;

LPGETCONFIG6	WGetConfig6 = &WGETCONFIG6;
LPSETCONFIG6	WSetConfig6 = &WSETCONFIG6;

// Then a SPECIALISED BMP to JPEG Service
//EXPORT32
//WORD MLIBCALL WBMPTOJPG6( HGLOBAL, DWORD, HGLOBAL, LPSTR );
// Where
// HGLOBAL hgIn - Handle to INPUT Data to be compressed
// DWORD   InSz - Size of INPUT Data
// HGLOBAL hgInf- Just in INFORMATION buffer for string errors
// LPSTR   lpFile - OUPUT File NAME.
// Returns:
// WORD  = 0 = Success. File written
// else 1++ ie sometimes an ERROR VALUE
//typedef WORD (MLIBCONV *LPWBMPTOJPG6) ( HGLOBAL, DWORD, HGLOBAL, LPSTR );
LPWBMPTOJPG6	WBmpToJpg6 = &WBMPTOJPG6;

//EXPORT32
//WORD MLIBCALL WBMP2JPG6( HGLOBAL, DWORD, HGLOBAL, HGLOBAL );
// Where
// HGLOBAL hgIn - Handle to INPUT Data to be compressed
// DWORD   InSz - Size of INPUT Data
// HGLOBAL hgInf- Just in INFORMATION buffer for string errors
// LPSTR   lpFile - OUPUT File NAME.
// Returns:
// WORD  = 0 = Success. JPEG data in buffer, where the FIRST
//		DWORD is the data's length.
// else 1++ ie sometimes an ERROR VALUE
//typedef WORD (MLIBCONV *LPWBMP2JPG6) ( HGLOBAL, DWORD, HGLOBAL, HGLOBAL );
LPWBMP2JPG6	WBmp2Jpg6 = &WBMP2JPG6;

#ifdef	FMEMDIAGS
LPWEXTMMGR		lpWExtMMgr	= &WEXTMMGR;
#endif	// FMEMDIAGS

BOOL	LoadLib6( void )
{
	if( !bDnLib2 )
	{
		fLoadLib6 = TRUE;
		bDnLib2 = TRUE;
#ifdef	FMEMDIAGS
		PassMemMgr();
#endif	// FMEMDIAGS
	}
	return fLoadLib6;
}

BOOL DVWJpg2Bmp6( HGLOBAL hg1, DWORD dw, HGLOBAL hg2, HGLOBAL hg3 )
{
	BOOL	flg;
	flg = FALSE;	// SUCCESS
	if( WJpg2Bmp6 )
	{
		flg = (*WJpg2Bmp6) ( hg1, dw, hg2, hg3 );
#ifdef		ADDTMDIAG2
		Do_Time_Trial2( hg1, dw, hg2 );
#endif	// ADDTMDIAG2
	}
	else
	{
		flg = TRUE;
	}
	return flg;
}

DWORD DVWJpgSize6( HGLOBAL hg, DWORD dw, LPSIZE lps )
{
	DWORD	dwSZ = 0;
	if( WJpgSize6 )
	{
		// Return NEW Version 6a (61) - SIZE
		dwSZ = ( ( *WJpgSize6 ) ( hg, dw, lps ) );
	}
	return( dwSZ );
}


#ifdef	TRYB2J6
extern  DWORD DVWrite(HANDLE fh, PINT8 pv, DWORD ul );
#endif	// TRYB2J6

BOOL DVWBmpToJpg6( HGLOBAL hg1, // INPUT global handle
				  DWORD dw,		// INPUT size
				  HGLOBAL hg2,	// Info global handle (can be NULL)
				  LPSTR lpf )	// Pointer to FILE NAME to CREATE
{
	BOOL	flg = FALSE;	// SUCCESS
#ifdef	TRYB2J6
	if( WBmp2Jpg6 )
	{
		HGLOBAL	hgJpg;
		LPSTR	lpJpg;
		if( hgJpg = DVGlobalAlloc( GHND, dw ) )
		{
			flg = (*WBmp2Jpg6) ( hg1, dw, hg2, hgJpg );
			if( flg )
			{
New_Failed:
				DVGlobalFree( hgJpg );
			}
			else
			{
				if( lpJpg = DVGlobalLock( hgJpg ) )
				{
					LPDWORD	lpdw;
					DWORD	dwjsz, dwwrtn;
					lpdw = (LPDWORD)lpJpg;
					if( dwjsz = *lpdw )
					{
						OFSTRUCT	of;
						HFILE		hf;
						lpJpg = (LPSTR)((LPDWORD)lpdw + 1);
						if( ( hf = OpenFile( lpf, &of, OF_CREATE ) ) &&
							( hf != HFILE_ERROR ) )
						{
							dwwrtn = DVWrite( hf, lpJpg, dwjsz );
							_lclose( hf );
							if( dwwrtn != dwjsz )
							{
								goto Failed_2;
							}
							DVGlobalUnlock( hgJpg );
							DVGlobalFree( hgJpg );
							return 0;
						}
						else
						{
							goto Failed_2;
						}
					}
					else
					{
Failed_2:
						DVGlobalUnlock( hgJpg );
						goto New_Failed;
					}
				}
				else
				{
					goto New_Failed;
				}
			}
		}
	}
#endif	// TRYB2J6
	if( WBmpToJpg6 )
	{
		flg = (*WBmpToJpg6) ( hg1, dw, hg2, lpf );
	}
	else
	{
		flg = TRUE;
	}
	return flg;
}

// ==================================================
#else	// !COMBWLIB
// ==================================================

#ifdef	WIN32
//#define	DEF_LIB2_NAME		"WJPG2BMP.DLL"
//#define	DEF_LIB2_NAME		"WDJPEG.DLL"
#define	DEF_LIB2_NAME		"WJPG32_2.DLL"
#else	// !WIN32
#define	DEF_LIB2_NAME		"WJPG2B16.DLL"
#endif	// WIN32 y/n

extern	BOOL GetNewLib( LPSTR lpOld,	// Old name
			   LPSTR lpFn,		// Buffer for NEW
			   DWORD dwTitleID,	// "Find ...." Title
			   LPSTR lpFilter,	// File Mask / Template, like *.DLL
			   char MLPTR MLPTR lpRServs );	// Requested services
extern	LPSTR	GetBigBuf( void );
extern	LPSTR	GetDT4( int Typ );	// From DT3
extern	LPSTR	GetDT4s( int Typ );	// Add seconds

extern	void	DVGetCurrentTime( LPSYSTEMTIME lpt );

extern	int	DVGetCwd( LPSTR lpb, DWORD siz );

HANDLE	hGotLib2 = 0;
BOOL	DnBat = FALSE;

// Introduce by WJPEG6, after adding the JPEG Group 6a (61) source
// ===================	June, 1997 ===============================
#ifdef	WJPEG6
// For static and dynamic library loading ...
//typedef DWORD (MLIBCONV *LPWJPGSIZE6) ( HGLOBAL, DWORD, LPSIZE );
//typedef BOOL  (MLIBCONV *LPWJPG2BMP6) ( HGLOBAL, DWORD, HGLOBAL, HGLOBAL);
#ifdef	LINKLIB6
#pragma message( "LIBLINK6: Static library load. Linked with " DEF_LIB2_NAME )
LPWJPGSIZE6		WJpgSize6	= &WJPGSIZE6;
LPWJPG2BMP6		WJpg2Bmp6	= &WJPG2BMP6;
LPGETCONFIG6	WGetConfig6	= &WGETCONFIG6;
LPSETCONFIG6	WSetConfig6	= &WSETCONFIG6;
LPWBMPTOJPG6	WBmpToJpg6	= &WBMPTOJPG6;
LPWBMP2JPG6		WBmp2Jpg6	= &WBMP2JPG6;
#ifdef	FMEMDIAGS
LPWEXTMMGR		lpWExtMMgr	= &WEXTMMGR;
#endif	// FMEMDIAGS

#else	// !LINKLIB6

#pragma message( "!LIBLINK6: Dynamic library load of " DEF_LIB2_NAME )
// Source was in D:\DOWN\JPEGSRC directory.
char	szJpgLib2[MAX_PATH+4] = { DEF_LIB2_NAME }; // 32-bit = "WJPG32_2.DLL"
// just function names
char	szWJpgSize6[] = "WJPGSIZE6";
char	szWJpg2Bmp6[] = "WJPG2BMP6";
char	szWBmp2Jpg6[] = "WBMP2JPG6";
char	szWGetConfig6[] = "WGETCONFIG6";
char	szWSetConfig6[] = "WSETCONFIG6";
#ifdef	FMEMDIAGS
// Non-essential LOAD
char		szWExtMMgr[] = "WEXTMMGR";
#endif	// FMEMDIAGS

LPWJPGSIZE6		WJpgSize6	= 0;
LPWJPG2BMP6		WJpg2Bmp6	= 0;
LPGETCONFIG6	WGetConfig6 = 0;
LPSETCONFIG6	WSetConfig6 = 0;
LPWBMPTOJPG6	WBmpToJpg6	= 0;
LPWBMP2JPG6		WBmp2Jpg6	= 0;
#ifdef	FMEMDIAGS
LPWEXTMMGR		lpWExtMMgr	= 0;
#endif	// FMEMDIAGS

#endif	// LINKLIB6
#endif	// WJPEG6

// ========================================================
// Now the CODE
// ========================================================
#ifdef	WJPEG6
void	FreeLib2( void )
{
#ifdef	LINKLIB6
	// Nothing to do here
#else	// !LINKLIB6
	// Physically close library handle
	// and remove references
			WJpgSize6 = 0;
			WJpg2Bmp6 = 0;
			WBmp2Jpg6 = 0;
#ifdef	FMEMDIAGS
			lpWExtMMgr = 0;
#endif	FMEMDIAGS
			if( hGotLib2 > (HANDLE)(HINSTANCE)MYHERROR )    // was HINSTANCE_ERROR
			{
				FreeLibrary( hGotLib2 );
			}
			hGotLib2 = 0;
#endif	// LINILIB6 y/n
}
#endif	// WJPEG6

BOOL DVWJpg2Bmp6( HGLOBAL hg1, DWORD dw, HGLOBAL hg2, HGLOBAL hg3 )
{
	BOOL	flg;
	flg = FALSE;	// SUCCESS
	if( WJpg2Bmp6 )
	{
		flg = (*WJpg2Bmp6) ( hg1, dw, hg2, hg3 );
	}
	else
	{
		flg = TRUE;
	}
	return flg;
}



DWORD DVWJpgSize6( HGLOBAL hg, DWORD dw, LPSIZE lps )
{
	DWORD	dwSZ = 0;
	if( WJpgSize6 )
	{
		// Return NEW Version 6a (61) - SIZE
		dwSZ = ( ( *WJpgSize6 ) ( hg, dw, lps ) );
	}
	return( dwSZ );
}

#ifdef	TRYB2J6
extern  DWORD DVWrite(HANDLE fh, PINT8 pv, DWORD ul );
#endif	// TRYB2J6

BOOL DVWBmpToJpg6( HGLOBAL hg1, // INPUT global handle
				  DWORD dw,		// INPUT size
				  HGLOBAL hg2,	// Info global handle (can be NULL)
				  LPSTR lpf )	// Pointer to FILE NAME to CREATE
{
	BOOL	flg = FALSE;	// SUCCESS
#ifdef	TRYB2J6
	if( WBmp2Jpg6 )
	{
		HGLOBAL	hgJpg;
		LPSTR	lpJpg;
		if( hgJpg = DVGlobalAlloc( GHND, dw ) )
		{
			flg = (*WBmp2Jpg6) ( hg1, dw, hg2, hgJpg );
			if( flg )
			{
New_Failed:
				DVGlobalFree( hgJpg );
			}
			else
			{
				if( lpJpg = DVGlobalLock( hgJpg ) )
				{
					LPDWORD	lpdw;
					DWORD	dwjsz, dwwrtn;
					lpdw = (LPDWORD)lpJpg;
					if( dwjsz = *lpdw )
					{
						OFSTRUCT	of;
						HFILE		hf;
						lpJpg = (LPSTR)((LPDWORD)lpdw + 1);
						if( ( hf = OpenFile( lpf, &of, OF_CREATE ) ) &&
							( hf != HFILE_ERROR ) )
						{
							HANDLE fh = IntToPtr(hf);
							dwwrtn = DVWrite( fh, lpJpg, dwjsz );
							_lclose( hf );
							if( dwwrtn != dwjsz )
							{
								goto Failed_2;
							}
							DVGlobalUnlock( hgJpg );
							DVGlobalFree( hgJpg );
							return 0;
						}
						else
						{
							goto Failed_2;
						}
					}
					else
					{
Failed_2:
						DVGlobalUnlock( hgJpg );
						goto New_Failed;
					}
				}
				else
				{
					goto New_Failed;
				}
			}
		}
	}
#else	// !TRYB2J6
	if( WBmpToJpg6 )
	{
		flg = (*WBmpToJpg6) ( hg1, dw, hg2, lpf );
	}
	else
	{
		if( !flg )
			flg = TRUE;
	}
#endif	// TRYB2J6 y/n
	return flg;
}


//#define	MXSERVS		260		// Maximum service name(s) length
// Allocate failed service name buffer ...
extern	char	szMissedS[MXSERVS];	// Buffer for MISSED service list...
extern	DWORD	dwMissedC; // = 0;	// and COUNT of missed services ...
extern	char	szMsdOne[]; // = "GROUP_Service(s)";	// General NAME


#define	LoadService2( hi, a, b, c )\
{\
	if( !( a = (b) GetProcAddress( hi, c ) ) )\
	{\
		if( lpMiss &&\
			( dwMissedC < MXSERVS ) &&\
			( iSLen = lstrlen( lpServ ) ) &&\
			( (dwMissedC + (DWORD)iSLen) < MXSERVS ) )\
		{\
			wsprintf( (lpMiss + lstrlen( lpMiss )),\
				"%s ", lpServ );\
			dwMissedC += (DWORD)lstrlen( lpMiss );\
		}\
		flg = FALSE;\
	}\
}

// #define	COMBWLIB	// LINKED with LIBRARY CODE
// NOTE: This is ONLY supplied as a Preprocessor FLAG
// ==================================================
#ifdef	LINKLIB6	// Link with LIBARAY

#else	// !LINKLIB6

#define	LDYNALIB6
#ifdef	LDYNALIB6	// Dynamic library loading ...
// =.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.

LPSTR	GetMsgBuf( void )
{
	return( GetBigBuf() );
}

char	szTmpBat[] = "TEMPUPD.BAT";
BOOL	bPrevBat = FALSE;

HFILE	OpenBatFile( LPSTR lpBat )
{
	OFSTRUCT	of;
	HFILE	hf = 0;
	if( lpBat && *lpBat )
	{
		if( bPrevBat )
		{
			hf = OpenFile( lpBat, &of, OF_READWRITE );
			if( hf &&
				( hf != HFILE_ERROR ) )
			{
				_llseek( hf, 0, FILE_END );
			}
		}
		else
		{
			hf = OpenFile( lpBat, &of, (OF_CREATE | OF_READWRITE) );
			if( hf &&
				( hf != HFILE_ERROR ) )
			{
				bPrevBat = TRUE;
			}
		}
	}
	return hf;
}

void	GiveLib2Help( LPSTR lpb,	// BATCH file message
					 LPSTR lpDest,	// Where BATCH file to be put
					 LPSTR lpOldLib,// What we could not load
					 LPSTR lpNL )	// and the NEW Library
{
	HFILE		hf;
	char		bbuf[MAX_PATH];
	LPSTR		lpBat, lpMsg;
	int	i, j;
	if( lpb && (i = lstrlen( lpb )) )
	{
		hf = 0;
		lpMsg = GetTmp3();
		lpBat = &bbuf[0];
		lstrcpy( lpBat, lpDest );
		lstrcat( lpBat, &szTmpBat[0] );
		if( ( hf = OpenBatFile( lpBat ) ) &&
			( hf != HFILE_ERROR ) )
		{
			if( (j = _lwrite( hf, lpb, i )) == i )
			{
				wsprintf( lpMsg,
					"Check and Run %s"MEOR
					"in [%s] folder,"MEOR
					"to avoid this SEARCH next time!",
					&szTmpBat[0],
					lpDest );
				if( (lstrlen( lpDest ) + lstrlen( lpb ) + 16) < 1024 )
				{
					wsprintf( (lpMsg + lstrlen( lpMsg )),
						"\r\nBatch commands are -\r\n%s",
						lpb );
				}
				_lclose( hf );	// Physically, and
				hf = 0;			// Logically
				// ==================================
				// One answer is a SIMPLE message box
				// A better one would be an OFFER to 
				// do the INSTALL now.
				MessageBox( GetFocus(),
					lpMsg,
					"HELPFUL BATCH FILE",
					MB_OK | MB_ICONINFORMATION );
			}
		}
		if( hf && (hf != HFILE_ERROR) )
			_lclose( hf );
	}
}

/* =====================================
 * HANDLE	LoadDJPEGLib2( void )
 *
 * Purpose: LOAD JPEG LIBRARY 2 - WJPG32_2.DLL
 *
 */
HANDLE	LoadDJPEGLib2( void )
{
	HINSTANCE	hLI, hLI2;
	LPSTR		lpl, lpb, lpServ;
	LPSTR		lpMiss;
	int			iSLen, iMLen;
	BOOL		flg, flg2;
	BOOL		fDoMB;
	HGLOBAL		hgNL;
	LPSTR		lpNL, lpFilt, lpf;
	LPSTR		lpm;
	char MLPTR MLPTR lpRServs;
	BOOL		fNL;	// We GOT a NEW Library
	LPSTR	lpDest;
	int		dlen;
   DWORD    dwError;

	lpMiss = &szMissedS[0];
	iMLen = lstrlen( lpMiss );

	flg = TRUE;		// Start this TRUE - False if SERVICE missed
	fDoMB = TRUE;
	hLI2 = 0;
	hgNL = 0;
	lpNL = 0;
	lpb = GetMsgBuf();
	lpDest = GetTmp2();
	*lpDest = 0;
	DVGetCwd( lpDest, MAX_PATH );	// BEFORE any changes
	fNL = FALSE;
//	if( !fNoFErr )	// SET the ERROR MODE to NOT give a SYSTEM MSG
//	{
		//uPreErr = SetErrorMode(SEM_NOOPENFILEERRORBOX);
//	}
   // = { DEF_LIB2_NAME }; // 32-bit = "WJPG32_2.DLL"
	lpl = &szJpgLib2[0];
   // LOAD LIBRARY 2 = { DEF_LIB2_NAME }; // 32-bit = "WJPG32_2.DLL"
	hLI = LoadLibrary( lpl );	// ) > (HINSTANCE)HINSTANCE_ERROR )
	if( hGotLib2 == 0 )
	{
		if( hLI <= (HINSTANCE)MYHERROR ) // was HINSTANCE_ERROR
		{
#ifdef   WIN32
         dwError = GetLastError();
			wsprintf( lpb, "ERROR: Unable to LOAD [%s] DLL!!!\r\n"
            "Error value is [** %u **](%#x).\r\n"
            "Unsually means DLL is NOT installed!", lpl,
            dwError, dwError );
#else // !WIN32
         dwError = (DWORD)hLi;
			wsprintf( lpb, "ERROR: Unable to LOAD [%s] DLL!!!\r\n"
            "Error value (HINSTANCE) is [** %04u **]\r\n"
            "Unsually means DLL is NOT installed!", lpl,
            dwError );
#endif   // WIN32

			if( ( hgNL = DVGlobalAlloc( GHND, 1024 ) ) &&
				( lpNL = DVGlobalLock( hgNL ) ) )
			{
				*lpNL = 0;
				lpFilt = lpNL + 512;
				lpRServs = (char MLPTR MLPTR)lpFilt;
				// services
				lpRServs -= 4;	// Backup space for 4 pointers
				lpRServs[0] = &szWJpgSize6[0];
				lpRServs[1] = &szWJpg2Bmp6[0];
				lpRServs[2] = &szWBmp2Jpg6[0];
				lpRServs[3] = 0;

				// Build a file template string
				lpf = lpFilt;
				BuildLibFilter( lpf );
            // FIX20001202 - The code in GetNewLib() apppeared VERY ERRANT
            // Will now return FALSE always
				if( GetNewLib( lpl,	// current
					lpNL,	// new buffer
					0,		// no title
					lpFilt,	// setup *.dll and service list
					lpRServs ) )	// If the USER picks one
				{
					if( *lpNL )
					{
                  // RETRY LOAD LIBRARY 2 = { DEF_LIB2_NAME }; // 32-bit = "WJPG32_2.DLL"
						// Try a NEW load
						hLI2 = LoadLibrary( lpNL );	// ) > (HINSTANCE)HINSTANCE_ERROR )
						if( hLI2 > (HINSTANCE)MYHERROR ) // was HINSTANCE_ERROR
						{
							// If success
							hLI = hLI2;
							fDoMB = FALSE;
							fNL = TRUE;
						}
					}
				}
			}
			if( fDoMB )
			{
				MessageBox( NULL, lpb, "LOAD ERROR",
					MB_OK | MB_ICONHAND );
				hLI = (HINSTANCE)MYHERROR; // was HINSTANCE_ERROR;
			}
			else
			{
				lpServ = &szWJpgSize6[0];
				LoadService2( hLI, WJpgSize6, LPWJPGSIZE6, lpServ );
				lpServ = &szWJpg2Bmp6[0];
				LoadService2( hLI, WJpg2Bmp6, LPWJPG2BMP6, lpServ );
				lpServ = &szWBmp2Jpg6[0];
				LoadService2( hLI, WBmp2Jpg6, LPWBMP2JPG6, lpServ );
#ifdef	FMEMDIAGS
// #include	"DvMemF.h"
				lpWExtMMgr = 0;
				if( flg )
				{
					// This can be skipped.
					// NOT ESSENTIAL FOR LIBARARY TO RUN
					flg2 = flg;
					lpServ = &szWExtMMgr[0];	// = "WEXTMMGR";
					LoadService2( hLI, lpWExtMMgr, LPWEXTMMGR, lpServ );
					if( flg2 && flg && lpWExtMMgr )
					{
						PassMemMgr();
					}
					else
					{
						flg = flg2;
					}
					WGetConfig6 = (LPGETCONFIG6) GetProcAddress( hLI, &szWGetConfig6[0] );
					WSetConfig6 = (LPSETCONFIG6) GetProcAddress( hLI, &szWSetConfig6[0] );
				}
#endif	// FMEMDIAGS
				if( !flg ||
					( WJpgSize6 == 0 ) ||
					( WJpg2Bmp6 == 0 ) ||
					( WBmp2Jpg6 == 0 ) )
				{
					// Could also ( lstrlen( lpMiss ) != iMLen )
					FreeLibrary( hLI2 );
					WJpgSize6 = 0;
					WJpg2Bmp6 = 0;
					WBmp2Jpg6 = 0;
					if( lpMiss && *lpMiss )
						lpm = lpMiss;
					else
						lpm = lpServ;
					wsprintf( lpb,
						"ERROR: Unable to locate [%s] in this file"MEOR\
						"[%s]\r\nIs this the correct DLL???"MEOR\
						"Will continue without it for now.",
						lpm, lpNL );
					MessageBox( NULL, lpb, "LOAD ERROR",
						MB_OK | MB_ICONHAND );
					hLI = (HINSTANCE)MYHERROR; // was HINSTANCE_ERROR;
				}
				else
				{
					// Successful SERVICE Load
					if( !DnBat && fNL && lpNL && *lpNL )
					{
						DnBat = TRUE;
						//LPSTR	lpDest;
						//int		dlen;
						//lpDest = GetTmp2();
						//*lpDest = 0;
						//DVGetCwd( lpDest, MAX_PATH );
						if( dlen = lstrlen( lpDest ) )
						{
							if( lpDest[dlen-1] != '\\' )
							{
								dlen++;
								lstrcat( lpDest, "\\" );
							}
							lstrcat( lpDest, lpl );
							wsprintf( lpb, "@echo	Upating %s ..."MEOR, lpl );
							lstrcat( lpb, "@pause"MEOR );
							wsprintf( ( lpb + lstrlen( lpb )),
								"Copy %s %s > nul"MEOR,
								lpNL,
								lpDest );
							lpDest[dlen] = 0;	// Shorten DEST again
							GiveLib2Help( lpb, lpDest, lpl, lpNL );
						}
					}
				}
			}
		}
		else
		{
			lpServ = &szWJpgSize6[0];
			LoadService2( hLI, WJpgSize6, LPWJPGSIZE6, lpServ );
			lpServ = &szWJpg2Bmp6[0];
			LoadService2( hLI, WJpg2Bmp6, LPWJPG2BMP6, lpServ );
			lpServ = &szWBmp2Jpg6[0];
			LoadService2( hLI, WBmp2Jpg6, LPWBMP2JPG6, lpServ );
#ifdef	FMEMDIAGS
// #include	"DvMemF.h"
			// This can be skipped.
			// NOT ESSENTIAL FOR LIBARARY TO RUN
			lpWExtMMgr = 0;	// Kill pointer to service
			if( flg )
			{
				flg2 = flg;		// Save current flag
				lpServ = &szWExtMMgr[0];	// = "WEXTMMGR";
				LoadService2( hLI, lpWExtMMgr, LPWEXTMMGR, lpServ );
				if( flg2 && flg && lpWExtMMgr )	// Did we GET IT
				{
					// It was LOADED - Pass on the MEMORY MANAGER
					PassMemMgr();
				}
				else
				{
					// It was NOT loaded.
					// Just RESTORE the "success" flag
					flg = flg2;
				}
					WGetConfig6 = (LPGETCONFIG6) GetProcAddress( hLI, &szWGetConfig6[0] );
					WSetConfig6 = (LPSETCONFIG6) GetProcAddress( hLI, &szWSetConfig6[0] );
			}
#endif	// FMEMDIAGS
//				( lstrlen( lpMiss ) != iMLen )
			if( !flg ||
				( WJpgSize6 == 0 ) ||
				( WJpg2Bmp6 == 0 ) ||
				( WBmp2Jpg6 == 0 ) )
			{
					FreeLibrary( hLI );
					WJpgSize6 = 0;
					WJpg2Bmp6 = 0;
					WBmp2Jpg6 = 0;
					if( lpMiss && *lpMiss )
						lpm = lpMiss;
					else
						lpm = lpServ;
					wsprintf( lpb,
						"ERROR: Unable to locate [%s] in"MEOR\
						"[%s] DLL\r\nIs this the correct DLL???"MEOR\
						"Will continue without it for now.",
						lpm, lpl );
					MessageBox( NULL, lpb, "LOAD ERROR",
						MB_OK | MB_ICONHAND );
					hLI = (HINSTANCE)MYHERROR;  // was HINSTANCE_ERROR
			}
		}
	}
	if( hgNL && lpNL )
		DVGlobalUnlock( hgNL );
	if( hgNL )
		DVGlobalFree( hgNL );
	return( hLI );
}

/* ========================= CloseJPEGLib6 ==========================
 * Reduce usage count by 1 ...
 * NOTE: If CVLIBBUG is ON, then ONLY called at EXIT TIME ...
 * ================================================================= */
void CloseJPEGLib6( void )
{
#ifdef	WJPEG6
			WJpgSize6 = 0;
			WJpg2Bmp6 = 0;
			WBmp2Jpg6 = 0;
#ifdef	FMEMDIAGS
			lpWExtMMgr = 0;
#endif	FMEMDIAGS
			if( hGotLib2 > (HANDLE)(HINSTANCE)MYHERROR ) // was HINSTANCE_ERROR
			{
				FreeLibrary( hGotLib2 );
			}
			hGotLib2 = 0;
#endif	// WJPEG6
// ===============================================================
}	// End CloseJPEGLib6() */

#else	// !LDYNALIB6

#endif 	// LDYNALIB6 y/n
#endif	// LINKLIB6 y/n

// #define	COMBWLIB	// LINKED with LIBRARY CODE
// NOTE: This is ONLY supplied as a Preprocessor FLAG
// ==================================================

BOOL	LoadLib6( void )
{
	if( !bDnLib2 )
	{
      // only ONE try
		bDnLib2 = TRUE;

		hGotLib2 = LoadDJPEGLib2();
		if( ( hGotLib2 > (HANDLE)(HINSTANCE)MYHERROR ) &&  // was HINSTANCE_ERROR
			( WJpgSize6 && WJpg2Bmp6 && WBmp2Jpg6 ) )
		{
			// SUCCESS
			fLoadLib6 = TRUE;
		}
		else
		{
			// FAILED to LOAD LIB
			// flg = FALSE;	// We have an ERROR
		}
	}

	return fLoadLib6;

}

// ==================================================
#endif	// COMBWLIB y/n

#ifdef	ADDTMDIAG2
#define	MXREPT		20
extern	LPSTR	GetDT4s( int Typ );	// Add seconds
extern	void	DVGetCurrentTime( LPSYSTEMTIME lpt );

char	szBGN[] = "\r\nBeginning %u tests ... File abt. %u.%uK ..."MEOR;
char	szBT1[] = "Beginning Test 1 [%s] (Out=%u)"MEOR;
char	szET1[] = "End 1: Defaults   [%s]"MEOR;
char	szDif[] = "Elapsed %u: %u ms. ie %u ms per test. %s"MEOR;
char	szBT2[] = "Beginning Test 2 [%s] (Out=%u)"MEOR;
char	szET2[] = "End 2: Fast=TRUE [%s]"MEOR;
char	szBT3[] = "Beginning Test 3 [%s] (Out=%u)"MEOR;
char	szET3[] = "End 3: QuantTRUE [%s]"MEOR;
char	szAve[] = "Average of %u ms per test."MEOR;

//char	szTimeMsg[2048];
#ifndef	MXSTRBUF
#define	MXSTRBUF		2048
#endif	// MXSTRBUF

BOOL	fAppdFile = TRUE;
char	sztmpbuf[MXSTRBUF + 16];
char	szTmpFile[] = "TEMPTIM2.TXT";
HFILE	hOutFil = 0;
#define	closeit			CloseOut()

HFILE TOpenFile( LPSTR lpFileName,	// pointer to filename
				LPOFSTRUCT lpO,	// pointer to buffer for file information
				UINT uStyle	) // action and attributes
{
	HFILE hf;
	hf = OpenFile( lpFileName, lpO, uStyle );
	return hf;
}

void	OpenOut( void )
{
	DWORD		fo;
	OFSTRUCT	of;
	if( fAppdFile )
	{
		if( ( hOutFil = TOpenFile( szTmpFile, &of, OF_READWRITE ) ) &&
			( hOutFil != HFILE_ERROR ) )
		{
			fo = _llseek( hOutFil, 0, 2 );
			if( fo > 1000000 )
			{
				fAppdFile = FALSE;
				_lclose( hOutFil );
				hOutFil = TOpenFile( szTmpFile, &of, OF_CREATE | OF_WRITE );
			}
		}
		else
		{
			hOutFil = TOpenFile( szTmpFile, &of, OF_CREATE | OF_WRITE );
		}
	}
	else
	{
		hOutFil = TOpenFile( szTmpFile, &of, OF_CREATE | OF_WRITE );
	}
}

// MAKE SURE the standard DOS Cr/Lf is used
// ========================================
extern	int	EnsureCrLf( LPSTR lpd, LPSTR lps );

int	OutFile( HFILE hf, LPSTR lpb, int len )
{
	char	buf[MXSTRBUF+8];
	int		k, wtn;

	wtn = 0;
	if( hf && (hf != HFILE_ERROR) &&
		lpb && len && *lpb )
	{
		k = EnsureCrLf( &buf[0], lpb );
		if( k )
			wtn = _lwrite( hf, &buf[0], k );
	}
	if( lpb && *lpb )
		*lpb = 0;	// Is WRITTEN

	return wtn;
}

void	CloseOut( void )
{
	if( hOutFil && (hOutFil != HFILE_ERROR) )
		_lclose( hOutFil );
	hOutFil = 0;
}

#define	wrtit( a ) \
{\
	if( hOutFil == 0 )\
		OpenOut();\
	if( a && lstrlen(a) && hOutFil && (hOutFil != HFILE_ERROR) )\
		OutFile( hOutFil, a, lstrlen( a ) );\
}

void	Getbps( LPSTR lpbps, DWORD bytes, DWORD ms )
{
	//				dwbps = (((bytes * 8) /  ( ddif / 1000 );
	//Getbps( lpbps, (dw * MXREPT), ddif );
	LPSTR	lps;
	float	fms, fdiv, fsecs;
	float	fbyt, fbit, fmul;
	float	fbps, fkbps;

	fms = (float)ms;
	fdiv = (float)1000;
	fsecs = fms / fdiv;
	fbyt = (float)bytes;
	fmul = (float)8;
	fbit = fbyt * fmul;
	fbps = fbit / fsecs;
	fdiv = (float)1024;

	if( fbps < fdiv )
	{
		lps = DVf2s( fbps );
		wsprintf( lpbps, "(%s bps)", lps );
	}
	else
	{
		fkbps = fbps / fdiv;
		fdiv = (float)10;
		if( fkbps < fdiv )
		{
			lps = DVf2s( fkbps );
			wsprintf( lpbps, "(%sK bps)", lps );
		}
		else
		{
			DWORD	dwk;
			dwk = (DWORD)fkbps;
			if( dwk < 1024 )
			{
				wsprintf( lpbps, "(%uK bps)", dwk );
			}
			else
			{
				wsprintf( lpbps, "(%u.%uM bps)",
					( dwk / 1024 ),
					(((dwk % 1024) * 10) / 1024) );
			}
		}
	}
}

void Do_Time_Trial2( HGLOBAL hg1, DWORD dw, HGLOBAL hg2 )
{
	BOOL	flg;
	LPSTR	lpb;
	SYSTEMTIME	bt1, et1, bt2, et2;
	LPSYSTEMTIME pbt1, pet1, pbt2, pet2;
	DWORD	dbt1, det1, dbt2, det2, ddif;
	BOOL	flg2;
	int		i;
	SIZE	nsz, psz;
	DWORD	dwNS, dwPS;
	LPJCOMCFG	lpc;
	HGLOBAL		hgOut;
	DWORD		dbt3, det3;
	DWORD		dwbps;
	char		szBps[32];
	LPSTR		lpbps;

	flg = 0;
//EXPORT32
//BOOL MLIBCALL WJPG2BMP6( HGLOBAL, DWORD, HGLOBAL, HGLOBAL);
// Where :-
// HGLOBAL	hgInData;		// The full GIF or JPG data.
// DWORD		ddDataSize;	// Size of the INPUT data.
// HGLOBAL	hgInfo;		// INFORMATION Buffer (if not NULL).
// HGLOBAL	hgOutData;	// Handle of OUPUT memory.
	hgOut = 0;
	if( WJpgSize6 && WJpg2Bmp6 && WGetConfig6 && WSetConfig6 )
	{
			pbt1 = &bt1;
			pet1 = &et1;
			pbt2 = &bt2;
			pet2 = &et2;
			lpb = GetTmp1();
			lpbps = &szBps[0];
			lpc = &AppJComCfg;
			lpc->jc_size = sizeof( JCOMCFG );
			(*WGetConfig6) ( lpc );
			if( lpc->jc_fast != 0 )
			{
				lpc->jc_fast = 0;
				(*WSetConfig6) ( lpc );
			}
			dwPS = (*WJpgSize6) ( hg1, dw, &psz );
			// ALLOCATE MEMORY
			if( ( dwPS == 0 ) ||
				( (hgOut = DVGlobalAlloc( GHND, dwPS )) == 0 ) )
			{
				goto Dn_Trial;
			}
			wsprintf( lpb,	// Start buffer
				&szBGN[0],
				MXREPT,
				(dw / 1024),
				( ((dw % 1024) * 10) / 1024 ) );
			wsprintf( ( lpb + lstrlen( lpb )),
				&szBT1[0],
				GetDT4s( 0 ),
				dwPS );
			DVGetCurrentTime( pbt1 );
			dbt1 = GetTickCount();
			for( i = 0; i < MXREPT; i++ )
			{
				dwPS = (*WJpgSize6) ( hg1, dw, &psz );
				flg2 = (*WJpg2Bmp6) ( hg1, dw, hg2, hgOut );
				if( flg2 )
					break;
			}
			if( flg2 == 0 )
			{
				det1 = GetTickCount();
				wsprintf( (lpb + lstrlen( lpb )),
					&szET1[0],
					GetDT4s( 0 ) );
//char	szDif[] = "Elapsed %u: %u ms. or %u ms per test."MEOR;
				ddif = det1 - dbt1;
				dwbps = (((dw * 8 * MXREPT) / ddif ) / 1000 );
				Getbps( lpbps, (dw * MXREPT), ddif );
				//wsprintf( lpbps, "(%u bps)", dwbps );
				wsprintf( (lpb + lstrlen( lpb )),
					&szDif[0],
					1,
					ddif,
					(ddif / MXREPT),
					lpbps );
				if( i = lstrlen( lpb ) )
				{
					wrtit( lpb );
				}
				// FREE MEMORY
				DVGlobalFree( hgOut );
				hgOut = 0;
				lpc->jc_fast = 1;
				(*WSetConfig6) ( lpc );
				dwPS = (*WJpgSize6) ( hg1, dw, &psz );
				// ALLOCATE GLOBAL MEMORY
				if( ( dwPS == 0 ) ||
					( (hgOut = GlobalAlloc( GHND, dwPS )) == 0 ) )
				{
					goto Dn_Trial;
				}
				wsprintf( (lpb + lstrlen( lpb )),
					&szBT2[0],
					GetDT4s( 0 ),
					dwPS );
				dbt2 = GetTickCount();
				for( i = 0; i < MXREPT; i++ )
				{
					dwNS = (*WJpgSize6) ( hg1, dw, &nsz );
					flg2 = (*WJpg2Bmp6) ( hg1, dw, hg2, hgOut ); // Use 6a

					if( flg2 )
					{
						break;
					}
				}

				det2 = GetTickCount();
				if( flg2 == 0 )
				{
					wsprintf( (lpb + lstrlen( lpb )),
						&szET2[0],
						GetDT4s( 0 ) );
					ddif = det2 - dbt2;
					dwbps = (((dw * 8 * MXREPT) / ddif ) / 1000 );
					//wsprintf( lpbps, "(%u bps)", dwbps );
					Getbps( lpbps, (dw * MXREPT), ddif );
					wsprintf( (lpb + lstrlen( lpb )),
						&szDif[0],
						2,
						ddif,
						(ddif / MXREPT),
						lpbps );
//char	szAve[] = "Average of %u ms per test."MEOR;
					wsprintf( (lpb + lstrlen( lpb )),
						&szAve[0],
						(((det1 - dbt1)+(det2 - dbt2)) / (2 * MXREPT)) );
					if( i = lstrlen( lpb ) )
					{
						wrtit( lpb );
					}
					// FREE GLOBAL MEMORY
					DVGlobalFree( hgOut );
					hgOut = 0;
					// OK, test 3
					lpc->jc_quantize_colors = 1;
					(*WSetConfig6) ( lpc );
					// ============================
					// ALLOCATE GLOBAL MEMORY
				dwPS = (*WJpgSize6) ( hg1, dw, &psz );
				if( ( dwPS == 0 ) ||
					( (hgOut = GlobalAlloc( GHND, dwPS )) == 0 ) )
				{
					goto Dn_Trial;
				}
				wsprintf( (lpb + lstrlen( lpb )),
					&szBT3[0],
					GetDT4s( 0 ),
					dwPS );
				dbt3 = GetTickCount();
				for( i = 0; i < MXREPT; i++ )
				{
					dwNS = (*WJpgSize6) ( hg1, dw, &nsz );
					flg2 = (*WJpg2Bmp6) ( hg1, dw, hg2, hgOut ); // Use 6a

					if( flg2 )
					{
						break;
					}
				}

				det3 = GetTickCount();
				if( flg2 == 0 )
				{
					wsprintf( (lpb + lstrlen( lpb )),
						&szET3[0],
						GetDT4s( 0 ) );
					ddif = det3 - dbt3;
					dwbps = (((dw * 8 * MXREPT) / ddif ) / 1000 );
					//wsprintf( lpbps, "(%u bps)", dwbps );
					Getbps( lpbps, (dw * MXREPT), ddif );
					wsprintf( (lpb + lstrlen( lpb )),
						&szDif[0],
						3,
						ddif,
						(ddif / MXREPT),
						lpbps );
//char	szAve[] = "Average of %u ms per test."MEOR;
					wsprintf( (lpb + lstrlen( lpb )),
						&szAve[0],
						(((det1 - dbt1)+(det2 - dbt2)+(det3 - dbt3)) / (3 * MXREPT)) );
					if( i = lstrlen( lpb ) )
					{
						wrtit( lpb );
					}
				}
				// FREE GLOBAL MEMORY
					DVGlobalFree( hgOut );
					hgOut = 0;
					// ============================

				}
				lpc->jc_quantize_colors = 0;
				lpc->jc_fast = 0;
				(*WSetConfig6) ( lpc );
			}
Dn_Trial:
			if( i = lstrlen( lpb ) )
			{
				wrtit( lpb );
			}
			closeit;
			if( hgOut )	// If some memory still hanging ...
				DVGlobalFree( hgOut );
	}	// Got services
}

#endif	// ADDTMDIAG2

#ifdef	FMEMDIAGS
// =====================================================
//
// BOOL PassMemMgr( void )
//
// Pass a set of MEMORY SERVICES to the DLL
// These services are like - malloc & free, and
// GlobalAlloc() GlobalLock() GlobalUnlock() GlobalFree()
//
// If used, and if DEBUG is ON, then TEMPMEM.TXT is 
// written, giving memory statisics.
//
// ======================================================
BOOL	PassMemMgr( void )
{
	BOOL	flg;
	LPJPEG_EXTMM lpExtMM;

	lpExtMM = GetMemMgr();
	if( lpExtMM && lpWExtMMgr )
		flg = (*lpWExtMMgr) ( lpExtMM, mf_TurnOn );

	return flg;
}

#endif	// FMEMDIAGS

void	BuildLibFilter( LPSTR lpFilter )
{
	LPSTR	lpf;
	if( lpf = lpFilter )
	{
		lstrcpy( lpf, "Libraries (*.DLL)" );
		lpf = lpf + lstrlen( lpf ) + 1;
		lstrcpy( lpf, "*.DLL" );
		lpf = lpf + lstrlen( lpf ) + 1;
		lstrcpy( lpf, "All File (*.*)" );
		lpf = lpf + lstrlen( lpf ) + 1;
		lstrcpy( lpf, "*.*" );
		lpf = lpf + lstrlen( lpf ) + 1;
		*lpf = 0;
	}
}

//////////////////////////////////////////////////////////
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now

// eof - DvLib2.c
