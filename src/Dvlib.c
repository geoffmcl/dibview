
//
// DvLib.c
//
// Interface to WJPEGxxx.DLL set of DLL's
// WJPGE32.DLL and WJPEG16.DLL *AND*
// WJPG32_2.DLL and WJPG16_2.DLL - Added June, 1997 - grm
//
//
#define		_DvLib_c
// === Above shuts off certain expresion in DvFunc.h below

#include	"dv.h"
#include	"DvFunc.h"

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
//////////////////////////////////////////////////////////
#include	"DvLib.h"

#define		FMEMDIAGS
#undef		ADDTMDIAGS
#define		USEIJG6
#define		USEIJG6C

extern	BOOL GetNewLib( LPSTR lpOld,	// Old name
			   LPSTR lpFn,		// Buffer for NEW
			   DWORD dwTitleID,	// "Find ...." Title
			   LPSTR lpFilter,	// File Mask / Template, like *.DLL
			   char MLPTR MLPTR lpRServs );	// Requested services
extern	LPSTR	GetDT4( int Typ );	// From DT3
extern	LPSTR	GetDT4s( int Typ );	// Add seconds

extern	void	DVGetCurrentTime( LPSYSTEMTIME lpt );

extern	int	DVGetCwd( LPSTR lpb, DWORD siz );

extern	char  szFStrJ[];	// =  "Bitmap (*.bmp)\0 PLUS WJPEGs
extern	char  szFStrS[];	// =  "Bitmap (*.bmp)\0 MINUS WJPEGS
extern	LPSTR pszFilter;	// File Filter String.

extern	BOOL	LoadLib6( void );	// Loads and returns fLoadLib6
extern	BOOL	fLoadLib6;	// TRUE if LOADED - Only ONE try
extern   BOOL  DVWJpg2Bmp6( HGLOBAL hg1, DWORD dw, HGLOBAL hg2, HGLOBAL hg3 );
extern	DWORD DVWJpgSize6( HGLOBAL hg, DWORD dw, LPSIZE lps );
extern	WORD  DVWBmpToJpg6( HGLOBAL hg1, DWORD dw, HGLOBAL hg2, LPSTR lps );
extern   VOID  SetLibInfo( LPTSTR lpl, BOOL flg );

// Forward references

BOOL	fNoFErr = TRUE;	// Default is to SHOW Load failed sys msg in W 3.1
BOOL	fShownNOLIB = FALSE;

#ifdef	WJPEG6
extern	void	FreeLib2( void );
#endif	// WJPEG6

void chkload( void )
{
	int i;
	i = 0;
}
// #define	COMBWLIB	// LINKED with LIBRARY CODE
// NOTE: This is ONLY supplied as a Preprocessor FLAG
// ==================================================
#ifdef	COMBWLIB	// COMMON CODE
// ***********************************************
#pragma message( "COMBINED Link with library code. NO WJPEGxxx.LIB" )

HINSTANCE	hLibInst = (HINSTANCE)1;
BOOL		fDnLibLoad = TRUE;
// These are just external functions
#ifdef	NEED_TWO_FILE
LPWRITEJPEG	WWriteJpeg = &WWRITEJPEG;
LPWDATTOJPG	WDatToJpg = &WDATTOJPG;
#endif	// NEED_TWO_FILE

LPWBMPTOJPG	WBmpToJpg = &WBMPTOJPG;


#ifdef	WJPEG4
// Extended support of ...
// NOT used by DV...
#ifdef	ADDOLDTO
LPWGIFTOBMP	WGifToBmp = &WGIFTOBMP;
LPWJPGTOBMP	WJpgToBmp = &WJPGTOBMP;
#endif	// ADDOLDTO

#else	// !WJPEG4
// ==========================================================
// NOTE: Older FILE Interfaces REMOVED from LIBRARY = RETIRED
// ==========================================================
LPREADJPEG	WReadJpeg = &WREADJPEG;
#ifdef	ADDRDGIF	// No longer used
LPGIFTOBMP	WGifToBmp = &WGIFTOBMP;
#endif	// ADDRDGIF - disbanded
LPJPGTOBMP	WJpgToBmp = &WJPGTOBMP;
// --- WJPEG4 = Turned off forever ...
// ==========================================================
#endif	// !WJPEG4

//#ifdef	USENEWAPI
// These are always there
LPWGIFSIZE	WGifSize = &WGIFSIZE;
LPWJPGSIZE	WJpgSize = &WJPGSIZE;
LPWGIF2BMP	WGif2Bmp = &WGIF2BMP;
LPWJPG2BMP	WJpg2Bmp = &WJPG2BMP;

//#endif	/* USENEWAPI */

#ifdef	CVLIBBUG
LPWGETLCONFIG	WGetLConfig = &WGETLCONFIG;
LPWSETLCONFIG	WSetLConfig = &WSETLCONFIG;
#endif

// rel. #ifdef	USEGFUNC5
LPWGIFSIZX	WGifSizX = &WGIFSIZX;
LPWGIFNBMP	WGifNBmp = &WGIFNBMP;
// rel #endif

#ifdef	USEGFUNC7
LPWGIFSIZXT WGifSizXT = &WGIFSIZXT;
LPWGIFNBMPX	WGifNBmpX = &WGIFNBMPX;
#endif	// USEGFUNC7

// #define	COMBWLIB	// LINKED with LIBRARY CODE
// NOTE: This is ONLY supplied as a Preprocessor FLAG
// ==================================================
// ***********************************************
#else	// !COMBWLIB
// ***********************************************

#ifdef	LDYNALIB
// ================================================
// Just Dynamic Library loading ...

BOOL	GetJPEGLib( UINT caller );
void  CloseJPEGLib( void );

char	szJpegLib[] = DEF_JPEG_LIB;   // 32-bit = WJPEG32.DLL

#ifdef	LIBLINK	/* If we are LINKING with WJPEG.LIB */
/* =============================================== */
#pragma message( "LIBLINK: Static library load. Linked with " DEF_JPEG_LIB )

HINSTANCE	hLibInst = 1;
BOOL		fDnLibLoad = TRUE;
LPWRITEJPEG	WWriteJpeg = &WWRITEJPEG;
LPWDATTOJPG	WDatToJpg = &WDATTOJPG;
LPWBMPTOJPG	WBmpToJpg = &WBMPTOJPG;
#ifndef	WJPEG4	// Old services REMOVED/RETIRED
LPREADJPEG	WReadJpeg = &WREADJPEG;
LPGIFTOBMP	WGifToBmp = &WGIFTOBMP;
LPJPGTOBMP	WJpgToBmp = &WJPGTOBMP;
#endif	// !WJPEG4
//#ifdef	USENEWAPI
// Alwasy avail.
LPWGIFSIZE	WGifSize = &WGIFSIZE;
LPWJPGSIZE	WJpgSize = &WJPGSIZE;
LPWGIF2BMP	WGif2Bmp = &WGIF2BMP;
LPWJPG2BMP	WJpg2Bmp = &WJPG2BMP;
//#endif	/* USENEWAPI */

#ifdef	CVLIBBUG
LPWGETLCONFIG	WGetLConfig = &WGETLCONFIG;
LPWSETLCONFIG	WSetLConfig = &WSETLCONFIG;
#endif

//#ifdef	USEGFUNC5
LPWGIFSIZX	WGifSizX = &WGIFSIZX;
LPWGIFNBMP	WGifNBmp = &WGIFNBMP;
//#endif

#if	(defined( USEGFUNC7 ) && defined( WJPEG5 ))
LPWGIFSIZXT WGifSizXT = &WGIFSIZXT;
LPWGIFNBMPX	WGifNBmpX = &WGIFNBMPX;
#endif	// USEGFUNC7 & WJPEG5

/* =============================================== */
#else	/* !LIBLINK = Manual (that is DYNAMIC) LOAD Library ... */
/* =============================================== */
#pragma message( "Dynamic Load library pointers from names ...LDYNALIB & !LIBLINK " )
#pragma message( "From WJPEG32.DLL and WJPG32_2.DLL " )

HINSTANCE	hLibInst = 0;
BOOL		fDnLibLoad = FALSE;
char	szJpegWrite[] = "WWRITEJPEG";
char	szWDatToJpg[] = "WDATTOJPG";
char	szWBmpToJpg[] = "WBMPTOJPG";
#ifndef	WJPEG4	// Older services REMOVED/RETIRED
char	szJpegRead[] = "WREADJPEG";
char	szJpegGIF[] = "WGIFTOBMP";
char	szJpegJPG[] = "WJPGTOBMP";
#endif	// !WJPEG4
#ifdef	USENEWAPI
char	szGIFSize[] = "WGIFSIZE";
char	szJPGSize[] = "WJPGSIZE";
char	szGIF2Bmp[] = "WGIF2BMP";
char	szJPG2Bmp[] = "WJPG2BMP";
#endif	/* USENEWAPI */
#ifdef	NEED_TWO_FILE
LPWRITEJPEG	WWriteJpeg = 0;
LPWDATTOJPG	WDatToJpg = 0;
#endif	// NEED_TWO_FILE
LPWBMPTOJPG	WBmpToJpg = 0;
#ifndef	WJPEG4	// Older services REMOVED
LPREADJPEG	WReadJpeg = 0;
#endif	// !WJPEG4

#ifdef	ADDOLDTO
char	szWGifToBmp[] = "WGIFTOBMP";
char	szWJpgToBmp[] = "WJPGTOBMP";
LPWGIFTOBMP	WGifToBmp = 0;
LPWJPGTOBMP	WJpgToBmp = 0;
#endif	// ADDOLDTO
//#endif	// !WJPEG4
//#ifdef	USENEWAPI
// Always
LPWGIFSIZE	WGifSize = 0;
char	szWGifSize[] = "WGIFSIZE";
LPWJPGSIZE	WJpgSize = 0;
LPWGIF2BMP	WGif2Bmp = 0;
LPWJPG2BMP	WJpg2Bmp = 0;
//#endif	/* USENEWAPI */


#ifdef	CVLIBBUG
char	szWGetLConfig[] = "WGETLCONFIG";	/* Names of 2 headers used */
char	szWSetLConfig[] = "WSETLCONFIG";	/* by this module ... */
LPWGETLCONFIG	WGetLConfig = 0;	/* These are filled in, when the Library */
LPWSETLCONFIG	WSetLConfig = 0;	/* has been successfully found and loaded */
#endif

//#ifdef	USEGFUNC5
char	szWGifSizX[] = "WGIFSIZX";
char	szGIFNBmp[] = "WGIFNBMP";
LPWGIFSIZX	WGifSizX = 0;
LPWGIFNBMP	WGifNBmp = 0;
//#endif

#if	(defined( USEGFUNC7 ) && defined( WJPEG5 ))
// Net release, ADD "Plain Text fetches
char	szWGifSizXT[] = "WGIFSIZXT";
char	szWGifNBmpX[] = "WGIFNBMPX";
LPWGIFSIZXT WGifSizXT = 0;	// Use Get Proc Address( LPSTR(Service) )
LPWGIFNBMPX	WGifNBmpX = 0;	// and then don't need names ...


// except, of course, as COMBWLIB shown above, where LINKER
// resolves all external ...
// #define	COMBWLIB	// LINKED with LIBRARY CODE
// NOTE: This is ONLY supplied as a Preprocessor FLAG
// ==================================================
#endif	// USEGFUNC7 & WJPEG5

/* =============================================== */
#endif	/* LIBLINK y/n */

// ====================
#else	/* !LDYNALIB */
// ====================

// STATIC LOAD THESE POINTER -
// ie FORCE A LINK With the LIBRARY WJPEG lib
// Make sure the 16-16 32-32 are MATCHED exactly
// ===================================================
// #include "c:temp\t1\gw\wjpeglib.h"	// OLD location!!!
//#include "wjpeglib.h"	= Access (WINAPIs) in JPEG Code ...
// One version ...
#pragma message( "STATIC library load. ie !LDYNALIB Linked with " DEF_JPEG_LIB )

LPWGETLCONFIG	WGetLConfig = &WGETLCONFIG;
LPWSETLCONFIG	WSetLConfig = &WSETLCONFIG;

LPWGIFSIZE	WGifSize = &WGIFSIZE;
LPWJPGSIZE	WJpgSize = &WJPGSIZE;
LPWGIF2BMP	WGif2Bmp = &WGIF2BMP;
LPWJPG2BMP	WJpg2Bmp = &WJPG2BMP;

//#ifdef	USEGFUNC5 - Released Extended Information
LPWGIFSIZX	WGifSizX = &WGIFSIZX;
// and get bitmaps for GIF Images 1, 2, 3,
LPWGIFNBMP	WGifNBmp = &WGIFNBMP;
//#endif

// Non-released function - BUT writes JPG
LPWBMPTOJPG	WBmpToJpg = &WBMPTOJPG;

//#ifdef	ADDOLDTO
//LPWGIFTOBMP	WGifToBmp = &WGIFTOBMP;
//LPWJPGTOBMP	WJpgToBmp = &WJPGTOBMP;
//#endif	// ADDOLDTO
HINSTANCE	hLibInst = (HINSTANCE)1;
BOOL		fDnLibLoad = TRUE;

#endif	// LDYNALIB y/n
// #define	COMBWLIB	// LINKED with LIBRARY CODE
// NOTE: This is ONLY supplied as a Preprocessor FLAG
// ==================================================
// ***********************************************
#endif	// COMBWLIB y/n

// ========================================================
// Now the CODE
// ========================================================
#ifdef	USEGFUNC5

BOOL LIBGifNBmp( HGLOBAL hgG, DWORD ddSz, HGLOBAL hIBuf,
				HGLOBAL hDIB, WORD num )
{
	BOOL	flg;

	flg = TRUE;		// Setup ERROR
#ifdef	LDYNALIB
//	if( *WGifNBmp )
	if( WGifNBmp )
	{
		flg = (*WGifNBmp) ( hgG, ddSz, hIBuf, hDIB, num );
	}
#else	// !LDYNALIB
	flg = WGIFNBMP( hgG, ddSz, hIBuf, hDIB, num );
#endif	// LDYNALIB
	return( flg );

}

#endif	// USEGFUNC5


BOOL DVWGif2Bmp( HGLOBAL hg1, DWORD dw, HGLOBAL hg2, HGLOBAL hg3 )
{
	if( WGif2Bmp )
		return( (*WGif2Bmp) ( hg1, dw, hg2, hg3 ) );
	else
		return TRUE;
}

#ifdef	ADDTMDIAGS
#define	MXREPT		20
char	szBGN[] = "Beginning %u tests ... File abt. %uK ..."MEOR;
char	szBT1[] = "Beginning Test 1 [%s]"MEOR;
char	szET1[] = "End 1: WJPG2BMP  [%s]"MEOR;
char	szDif[] = "Elapsed %u: %u ms. or %u ms per test."MEOR;
char	szBT2[] = "Beginning Test 2 [%s]"MEOR;
char	szET2[] = "End 2: WJPG2BMP6 [%s]"MEOR;
char	szAve[] = "Average of %u ms per test."MEOR;

char	szTimeMsg[2048];
#ifndef	MXSTRBUF
#define	MXSTRBUF		2048
#endif	// MXSTRBUF

BOOL	fAppdFile = TRUE;
char	sztmpbuf[MXSTRBUF + 16];
char	szTmpFile[] = "TEMPTIME.TXT";
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
int	OutFile( HFILE hf, LPSTR lpb, int len )
{
	char	buf[MXSTRBUF+8];
	int		i, j, k, wtn;
	char	c, d;

	wtn = 0;
	if( hf && (hf != HFILE_ERROR) &&
		lpb && len &&
		(i = lstrlen( lpb )) )
	{
		if( i > MXSTRBUF )
		{
			i = MXSTRBUF;
		}
		k = 0;
		d = 0;

		for( j = 0; j < i; j++ )
		{
			c = lpb[j];
			if( c == '\r' )
			{
				if( (j+1) < i )
				{
					if( lpb[j+1] != '\n' )
					{
						buf[k++] = c;
						c = '\n';
					}
				}
				else
				{
					buf[k++] = c;
					c = '\n';
				}
			}
			else if( c == '\n' )
			{
				if( d != '\r' )
					buf[k++] = '\r';
			}
			buf[k++] = c;
			d = c;
		}
		if( c != '\n' )
		{
			buf[k++] = '\r';
			buf[k++] = '\n';
		}
		if( k )
			wtn = _lwrite( hf, &buf[0], k );
	}
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

void Do_Time_Trials( HGLOBAL hg1, DWORD dw, HGLOBAL hg2, HGLOBAL hg3 )
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

	if( WJpgSize && WJpg2Bmp && WJpgSize6 && WJpg2Bmp6 )
	{
		flg = (*WJpg2Bmp) ( hg1, dw, hg2, hg3 );
		if( ( flg == 0 ) &&
			( WJpg2Bmp6 ) &&
			( WJpgSize6 ) )
		{
			pbt1 = &bt1;
			pet1 = &et1;
			pbt2 = &bt2;
			pet2 = &et2;
			dwPS = (*WJpgSize) ( hg1, dw, &psz );
			lpb = &szTimeMsg[0];
			wsprintf( lpb,	// Start buffer
				&szBGN[0],
				MXREPT,
				(dw / 1024) );
			wsprintf( ( lpb + lstrlen( lpb )),
				&szBT1[0],
				GetDT4s( 0 ) );
			DVGetCurrentTime( pbt1 );
			dbt1 = GetTickCount();
			for( i = 0; i < MXREPT; i++ )
			{
				flg2 = (*WJpg2Bmp) ( hg1, dw, hg2, hg3 );
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
				wsprintf( (lpb + lstrlen( lpb )),
					&szDif[0],
					1,
					ddif,
					(ddif / MXREPT) );
				dwPS = (*WJpgSize6) ( hg1, dw, &psz );
				wsprintf( (lpb + lstrlen( lpb )),
					&szBT2[0],
					GetDT4s( 0 ) );
				dbt2 = GetTickCount();
				for( i = 0; i < MXREPT; i++ )
				{
					dwNS = (*WJpgSize6) ( hg1, dw, &nsz );
					flg2 = (*WJpg2Bmp6) ( hg1, dw, hg2, hg3 ); // Use 6a

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
					wsprintf( (lpb + lstrlen( lpb )),
						&szDif[0],
						2,
						ddif,
						(ddif / MXREPT) );
//char	szAve[] = "Average of %u ms per test."MEOR;
					wsprintf( (lpb + lstrlen( lpb )),
						&szAve[0],
						(((det1 - dbt1)+(det2 - dbt2)) / (2 * MXREPT)) );
					
					flg2 = 1;
					i = lstrlen( lpb );
					wrtit( lpb );
					closeit;
					// Back to original
					flg = (*WJpg2Bmp) ( hg1, dw, hg2, hg3 );
				}
			}
		}
}
#endif	// ADDTMDIAGS

BOOL DVWJpg2Bmp( HGLOBAL hg1, DWORD dw, HGLOBAL hg2, HGLOBAL hg3 )
{
	BOOL	flg;
	flg = FALSE;	// SUCCESS
#ifdef	USEIJG6
	if( fLoadLib6 )
		flg = DVWJpg2Bmp6( hg1, dw, hg2, hg3 );
	else
	{
		if( WJpg2Bmp )
		{
			flg = (*WJpg2Bmp) ( hg1, dw, hg2, hg3 );
		}
		else
		{
			flg = TRUE;
		}
	}
#else	// !USEIJG6
	if( WJpg2Bmp )
	{
#ifdef	ADDTMDIAGS
		Do_Time_Trial( hg1, dw, hg2, hg3 );
#endif	// ADDTMDIAGS
		flg = (*WJpg2Bmp) ( hg1, dw, hg2, hg3 );
	}
	else
	{
		flg = TRUE;
	}

#endif	// USEIJG6 y/n

	return flg;
}


BOOL	LibLoaded2( void )
{
	BOOL	flg = FALSE;
	if( WJpgSize && WJpg2Bmp )
		flg = TRUE;
	return flg;
}

WORD DVWBmpToJpg( HGLOBAL hg1, DWORD dw, HGLOBAL hg2, LPSTR lps )
{
#ifdef	USEIJG6C
	if( fLoadLib6 )
		return( DVWBmpToJpg6( hg1, dw, hg2, lps ) );
	else
	{
		if( WBmpToJpg )
			return( (*WBmpToJpg) ( hg1, dw, hg2, lps ) );
		else
			return TRUE;
	}
#else	// !USEIJG6C
	if( WBmpToJpg )
		return( (*WBmpToJpg) ( hg1, dw, hg2, lps ) );
	else
		return TRUE;
#endif	// USEIJG6C
}

DWORD DVWGifSize( HGLOBAL hg, DWORD dw, LPSIZE lps )
{
	if( WGifSize )
		return( (*WGifSize) ( hg, dw, lps ) );
	else
		return( (DWORD) 0 );
}


//	if( ddBmp = (*WJpgSize) ( hgGIF, dwBitsSize, &dibSize ) )
DWORD DVWJpgSize( HGLOBAL hg, DWORD dw, LPSIZE lps )
{
#ifdef	USEIJG6
	if( fLoadLib6 )
		return( DVWJpgSize6( hg, dw, lps ) );
	else
	{
		if( WJpgSize )
			return( (*WJpgSize) ( hg, dw, lps ) );
		else
			return( (DWORD) 0 );
	}
#else	// !USEIJG6
	if( WJpgSize )
		return( (*WJpgSize) ( hg, dw, lps ) );
	else
		return( (DWORD) 0 );
#endif	// USEIJG6 y/n
}


WORD DVWGifSizX( HGLOBAL hg1, DWORD dw, HGLOBAL hg2 )
{
	if( WGifSizX )
		return( (*WGifSizX) ( hg1, dw, hg2 ) );
	else
		return( (WORD) 1 );
}

#ifndef	USENEWAPI

HGLOBAL DVWGifToBmp( HGLOBAL hg1, DWORD dw, HGLOBAL hg2, LPSTR lps )
{
	if( WGifToBmp )
		return( (*WGifToBmp) ( hg1, dw, hg2, lps ) );
	else
		return( (HGLOBAL)1 );
}

HGLOBAL DVWJpgToBmp( HGLOBAL hg1, DWORD dw, HGLOBAL hg2, LPSTR lps )
{
	if( WJpgToBmp )
		return( (*WJpgToBmp) ( hg1, dw, hg2, lps ) );
	else
		return( (HGLOBAL)1 );
}

#endif	// !USENEWAPI

#ifdef	ADDWJPEG
//#define	MXSERVS		260		// Maximum service name(s) length
// Allocate failed service name buffer ...
char	szMissedS[MXSERVS];	// Buffer for MISSED service list...
DWORD	dwMissedC = 0;	// and COUNT of missed services ...
char	szMsdOne[] = "GROUP_Service(s)";	// General NAME

#define	LoadService( a, b, c )\
{\
	if( !( a = (b) GetProcAddress( hLibInst, c ) ) )\
	{\
		if( !fPrevs && lpMiss &&\
			( dwMissedC < MXSERVS ) &&\
			( iSLen = lstrlen( lpServ ) ) &&\
			( (dwMissedC + (DWORD)iSLen) < MXSERVS ) )\
		{\
			lstrcpy( &lpMiss[dwMissedC], lpServ );\
			dwMissedC += (DWORD)iSLen + 1;\
			lstrcat( lpMiss, " " );\
		}\
		flg = FALSE;\
	}\
}

// #define	COMBWLIB	// LINKED with LIBRARY CODE
// ***NOTE***: This is ONLY supplied
// as a Preprocessor FLAG to the Compiler!
// ==================================================
#ifdef	COMBWLIB
// *****************************************************

// These DO NOTHING when LINKED with LIBRARY CODE
BOOL	GetJPEGLib( UINT caller )
{
	return( LoadLib6() )		// Still have to LOAD this baby
}
LPSTR	GetLibPtr( void )
{
	return( (LPSTR)0 );
}
void CloseJPEGLib( void )
{
}

// #define	COMBWLIB	// LINKED with LIBRARY CODE
// NOTE: This is ONLY supplied as a Preprocessor FLAG
// ==================================================
// *****************************************************
#else	// !COMBWLIB
// *****************************************************

#ifdef	LDYNALIB	// Dynamic library loading ...
// =.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.=.
LPSTR	GetLibPtr( void )
{
// = DEF_JPEG_LIB;   // 32-bit = WJPEG32.DLL
	return( &szJpegLib[0] );
}

// LOAD LIBRARY 2 - if possible
extern	HANDLE	LoadDJPEGLib2( void );

BOOL	GetJPEGLib( UINT caller )
{
	BOOL		flg;
	UINT		uPreErr;
	HINSTANCE	hLI;
	LPSTR		lpl;
	LPSTR		lpMiss;
	LPSTR		lpServ;
	int			iSLen;
	BOOL		fPrevs;
	char		buf[24];

	uPreErr = 0;
	iSLen = 0;
	lpMiss = 0;
	lpServ = 0;

#ifdef	LIBLINK
	flg = TRUE;	// LIBRARY was STATICALLY LOADED by the SYSTEM!!!
#else	// !LIBLINK
	flg = FALSE;
	fPrevs = fDnLibLoad;	// Get PREVIOUS
	if( !fPrevs )
	{
		// Very FIRST load attempt
		szMissedS[0] = 0;
		dwMissedC = 0;
	}
   // F20001126 - Appears LoadLibary() returns ZERO for ERROR, so change
   // HINSTANCE_ERROR to MYHERROR in WIN32 == ZERO
   if( hLibInst > (HINSTANCE)MYHERROR )   // was HINSTANCE_ERROR
	{
		flg = TRUE;
	}
	else if( !fDnLibLoad )   // was if( hLibInst == 0 )
	{
      // return char	szJpegLib[] = DEF_JPEG_LIB;   // 32-bit = WJPEG32.DLL
		lpl = GetLibPtr();
		// Keep actual service NAME, that FAILED to LOAD...
		// That is first, 2nd, ... list of no address achieved ...
		lpMiss = &szMissedS[0];
		if( !fNoFErr )	// SET the ERROR MODE to NOT give a SYSTEM MSG
		{
			uPreErr = SetErrorMode(SEM_NOOPENFILEERRORBOX);
		}

		fDnLibLoad = TRUE;	// We have TRIED to LOAD library!!!
		chkload();
      // F20001126 - Appears LoadLibary() returns ZERO for ERROR, so change
      // HINSTANCE_ERROR to MYHERROR in WIN32 == ZERO
      // LOAD LIBRARY 1 = DEF_JPEG_LIB;   // 32-bit = WJPEG32.DLL
		if( (hLI = LoadLibrary( lpl )) > (HINSTANCE)MYHERROR )   // was HINSTANCE_ERROR
		{
#ifndef	CVLIBBUG	// Can NOT uload and re-load library, so as a fix
			hLibInst = 0;
			CloseJPEGLib();
#endif   // ifndef	CVLIBBUG	// Can NOT uload and re-load library, so as a fix
			hLibInst = hLI;
//#ifdef	NEED_TWO_FILE
//			if( WWriteJpeg = (LPWRITEJPEG) GetProcAddress( hLibInst, &szJpegWrite[0] ) )
//			{
//#endif	// NEED_TWO_FILE
#ifndef	WJPEG4	// Note: Older interfaces RETIRED
				if( WReadJpeg = (LPREADJPEG) GetProcAddress( hLibInst, &szJpegRead[0] ) )
				{
					if( WGifToBmp = (LPGIFTOBMP) GetProcAddress( hLibInst,
						&szJpegGIF[0] ) )
					{
						if( WJpgToBmp = (LPJPGTOBMP) GetProcAddress( hLibInst,
							&szJpegJPG[0] ) )
						{
#endif	// !WJPEG4
#ifdef	USENEWAPI
	if( WGifSize = (LPWGIFSIZE) GetProcAddress( hLibInst, &szWGifSize[0] ) )
	{		
		if( WJpgSize = (LPWJPGSIZE) GetProcAddress( hLibInst, &szJPGSize[0] ) )
		{
			if( WGif2Bmp = (LPWGIF2BMP) GetProcAddress( hLibInst, &szGIF2Bmp[0]) )
			{
				if( WJpg2Bmp = (LPWJPG2BMP) GetProcAddress( hLibInst,
						&szJPG2Bmp[0] ) )
				{
#ifdef	CVLIBBUG
		if( WGetLConfig = (LPWGETLCONFIG) GetProcAddress( hLibInst, &szWGetLConfig[0] ) )
		{
			if( WSetLConfig = (LPWSETLCONFIG) GetProcAddress( hLibInst, &szWSetLConfig[0] ) )
			{
#ifdef	USEGFUNC3
#ifdef	NEED_TWO_FILE
				if( WDatToJpg = (LPWDATTOJPG) GetProcAddress( hLibInst,
						&szWDatToJpg[0] ) )
				{
#endif	// NEED_TWO_FILE
//#ifdef	USEGFUNC4
//					if( WBmpToJpg = (LPWBMPTOJPG) GetProcAddress( hLibInst,
//						&szWBmpToJpg[0] ) )
//					{
//						flg = TRUE;
//					}
//#else	// !USEFUNC4
					flg = TRUE;
//#endif	// USEGFUNC4 y/n
#ifdef	NEED_TWO_FILE
				}
#endif	// NEED_TWO_FILE
#else	// !USEGFUNC3
				flg = TRUE;
#endif	// USEGFUNC3 y/n
			}
		}
#else	// !CVLIBBUG
							flg = TRUE;
#endif	// CVLIBBUG y/n
				}
			}
		}
	}
#else	// !USENEWAPI
							flg = TRUE;
#endif	// USENEWAPI y/n
#ifndef	WJPEG4
						}
					}
				}
#endif	// !WJPEG4
//#ifdef	NEED_TWO_FILE
//			} // WWriteJpeg = (LPWRITEJPEG) GetProcAddress(...)
//#endif	// NEED_TWO_FILE
#ifdef	USEGFUNC7
			// If we have HAD a good LOAD of addresses ...
				// Set Service ptr ...
				if( !flg )
				{
					lpServ = &szMsdOne[0];
					if( lpMiss &&
						( dwMissedC == 0 ) &&
						( iSLen = lstrlen( lpServ ) ) &&
						( iSLen < MXSERVS ) &&
						!fPrevs )
					{
						lstrcpy( &lpMiss[dwMissedC], lpServ );
						dwMissedC += (DWORD)iSLen + 1;	// Bump to END
						lpMiss[dwMissedC] = 0;	// Add 2nd = end list.
					}
				}
#ifdef	WJPEG5
			lpServ = &szWGifSizXT[0];
			LoadService( WGifSizXT, LPWGIFSIZXT, lpServ );
#endif	// WJPEG5
				// Set Service ptr ...
//				lpServ = &szGIFNBmp[0]; 
//			if( !( WGifNBmp =
//					(LPWGIFNBMP) GetProcAddress( hLibInst,
//						lpServ ) ) )
//			{
//				if( !fPrevs &&
//					lpMiss &&
//					( dwMissedC < MXSERVS ) &&
//					( iSLen = lstrlen( lpServ ) ) &&
//					( (dwMissedC + (DWORD)iSLen) < MXSERVS ) )
//				{
//					lstrcpy( &lpMiss[dwMissedC], lpServ );
//					dwMissedC += (DWORD)iSLen + 1;	// Bump to END
//					lpMiss[dwMissedC] = 0;	// Add 2nd = end list.
//				}
//				flg = FALSE;
//			}

#endif	// USEGFUNC7
			// =====================
#ifdef	NEED_TWO_FILE
			// Set SERVICE required ...
			lpServ = &szJpegWrite[0];
			LoadService( WWRiteJpeg, LPWRITEJPEG, lpServ );

#endif	// NEED_TWO_FILE

//#ifdef	USEGFUNC5
			// Set SERVICE required ...
			lpServ = &szWGifSizX[0];
			LoadService( WGifSizX, LPWGIFSIZX, lpServ );

			// Set SERVICE required ...
			lpServ = &szGIFNBmp[0];
			LoadService( WGifNBmp, LPWGIFNBMP, lpServ );
//#endif	// !USEFUNC5

#ifdef	USEGFUNC4
			// Set SERVICE required ...
			// This MUST first be turned ON in the Library
//			lpServ = &szWBmpToJpg[0];
//			LoadService( WBmpToJpg, LPWBMPTOJPG, lpServ );

//			lpServ = &szGIFNBmp[0];
//				if( !( WBmpToJpg =
//						(LPWBMPTOJPG) GetProcAddress( hLibInst,
//							lpServ ) ) )
//				{
//					if( !fPrevs &&
//						lpMiss &&
//						( dwMissedC < MXSERVS ) &&
//						( iSLen = lstrlen( lpServ ) ) &&
//						( (dwMissedC + (DWORD)iSLen) < MXSERVS ) )
//					{
//						lstrcpy( &lpMiss[dwMissedC], lpServ );
//						dwMissedC += (DWORD)iSLen + 1;	// Bump to END
//						lpMiss[dwMissedC] = 0;	// Add 2nd = end list.
//					}
//					flg = FALSE;
//				}
#endif	// USEFUNC4

#ifdef	ADDOLDTO
				lpServ = &szWGifToBmp[0];
				LoadService( WGifToBmp, LPWGIFTOBMP, lpServ );
				lpServ = &szWJpgToBmp[0];
				LoadService( WJpgToBmp, LPWJPGTOBMP, lpServ );
#endif	// ADDOLDTO

			// =====================
#ifdef	WJPEG6
            if( flg )
            {
               flg = LoadLib6();
            }
#endif	// WJPEG6
			if( !flg )
			{	// If ANY function FAILED, then FAIL ALL (for now)
				CloseJPEGLib();
				pszFilter = &szFStrS[0];
			}
		}	// if LoadLibrary
		else	// Load library FAILED
		{
			// Remove the FULL mutiple file support filter
			pszFilter = &szFStrS[0];
			// Little ( < 24 ) message
			wsprintf( &buf[0], "LoadLibrary(%u)%#04x ", hLI, hLI );
			// ERROR
			if( !fPrevs &&
				lpMiss &&
				( dwMissedC < MXSERVS ) &&
				( iSLen = lstrlen( &buf[0] ) ) &&
						( (dwMissedC + (DWORD)iSLen) < MXSERVS ) )
			{
				lstrcpy( &lpMiss[dwMissedC], &buf[0] );
				dwMissedC += (DWORD)iSLen + 1;	// Bump to END
				lpMiss[dwMissedC] = 0;	// Add 2nd = end list.
			}
		}
		if( !fNoFErr )
		{
			uPreErr = uPreErr & ~(SEM_FAILCRITICALERRORS |
				SEM_NOGPFAULTERRORBOX |
				SEM_NOOPENFILEERRORBOX );
			uPreErr = SetErrorMode( uPreErr );
		}

      SetLibInfo( lpl, flg );

	}	// hLibInst was 0 n/y
#endif	// LIBLINK y/n

	return( flg );

}

/* ========================= CloseJPEGLib ==========================
 * Reduce usage count by 1 ...
 * NOTE: If CVLIBBUG is ON, then ONLY called at EXIT TIME ...
 * ================================================================= */
void CloseJPEGLib( void )
{
#ifndef	LIBLINK
	if( hLibInst > (HINSTANCE)MYHERROR )   // was HINSTANCE_ERROR
	{
		FreeLibrary( hLibInst );
	}
	hLibInst = 0;
#endif
// ===============================================================
#ifdef	NEED_TWO_FILE
			WWriteJpeg = 0;
#endif	// NEED_TWO_FILE
#ifndef	WJPEG4
			WReadJpeg = 0;
			WGifToBmp = 0;
			WJpgToBmp = 0;
#endif	// !WJPEG4
#ifdef	USENEWAPI
			WGifSize = 0;		
			WJpgSize = 0;
			WGif2Bmp = 0;
			WJpg2Bmp = 0;
#ifdef	CVLIBBUG
			WGetLConfig = 0;
			WSetLConfig = 0;
#ifdef	USEGFUNC3
#ifdef	NEED_TWO_FILE
			WDatToJpg = 0;
#endif	// NEED_TWO_FILE
#ifdef	USEGFUNC4
			WBmpToJpg = 0;
//#ifdef	USEGFUNC5
			WGifSizX = 0;
			WGifNBmp = 0;
//#endif	// USEGFUNC5
#endif	// USEGFUNC4
#endif	// USEGFUNC3
#endif	// CVLIBBUG
#endif	// USENEWAPI
#ifdef	WJPEG6
			FreeLib2();
#endif	// WJPEG6
// ===============================================================
}	/* End CloseJPEGLib() */

#else	// !LDYNALIB

// Here the SYSTEM has LOADED the DLL into memory,
// and increaded its instance count, So we just
// just return TRUE = Library LOADED.
// BUT we MUST still statically load Lib 6a - June 1997
// ========================================
BOOL	GetJPEGLib( UINT caller )
{
	BOOL flg = LoadLib6();
	return( flg );
}

#endif 	// LDYNALIB y/n

#endif	// COMBWLIB y/n
// #define	COMBWLIB	// LINKED with LIBRARY CODE
// NOTE: This is ONLY supplied as a Preprocessor FLAG
// ==================================================

#ifdef	ADDRDGIF	// No longer USED!!!
// ===================================================
char	szTmpFile[] = "TEMPFILE.BMP";

HANDLE ReadGIFFileOLD( LPSTR lpf )
{
HANDLE	hDIB;
int		hFile;
OFSTRUCT	ofs;
DWORD	dwBitsSize;
HGLOBAL	hgGIF, hgINF;
LPSTR		lpo, lpGIF, lpINF, lpDIB;
BITMAPFILEHEADER bmfHeader;
	hDIB = 0;	/* No HANDLE yet ... */
	hgGIF = 0;
	lpGIF = 0;
	lpo = &szTmpFile[0];
	hgINF = 0;
	lpINF = 0;
#ifdef	LDYNALIB
	if( !GetJPEGLib(11) )
	{
		if( !fShownNOLIB )
		{
			UINT	ui;
			fShownNOLIB = TRUE;
			//DIBError( ERR_NO_LIB );
			ui = Err_No_Lib();
		}
		goto ReadGRet;
	}
#endif	// LDYNALIB
	if( (hFile = DVOpenFile( lpf, &ofs, OF_READ)) &&
		(hFile != -1 ) &&
//		( dwBitsSize = _filelength (hFile) ) &&
		( dwBitsSize = DVFileSize( hFile ) ) &&
		( hgGIF = DVGlobalAlloc( GHND | GMEM_SHARE, dwBitsSize ) ) &&
		( lpGIF = DVGlobalLock( hgGIF ) ) &&
		( DVRead( hFile, lpGIF, dwBitsSize ) == dwBitsSize ) )
	{
#ifdef	LDYNALIB
#ifdef	PROBGOBJ
			hDIB = (*WGifToBmp) ( hgGIF, dwBitsSize, hgINF, lpo );
#else
			hDIB = (*WGifToBmp) ( hgGIF, dwBitsSize, hgINF, NULL );
#endif
#else	// !LDYNALIB
#ifdef	PROBGOBJ
			hDIB = WGIFTOBMP( hgGIF, dwBitsSize, hgINF, lpo );
#else	// !PROBGOBJ
			hDIB = WGIFTOBMP( hgGIF, dwBitsSize, hgINF, NULL );
#endif	// PROBGOBJ y/n
#endif	// LDYNALIB y/n
			DVGlobalUnlock( hgGIF );
			DVGlobalFree( hgGIF );
			lpGIF = 0;
			hgGIF = 0;
			if( hDIB == 0 )
			{
				if( lpINF && lpINF[0] )
					DIBError2( ERR_NOLCONV, lpINF );
				else
					DIBError( ERR_NOLCONV );
				goto ReadGRet;
			}
			if( (hFile = DVOpenFile( lpo, &ofs, OF_READ)) == -1 )
			{
      		DIBError( ERR_NO_CONV );
      		goto ReadGRet;
			}
//			dwBitsSize = _filelength( hFile ); /* Get this files size ... */
			dwBitsSize = DVFileSize( hFile ); /* Get this files size ... */
			if( (DVRead (hFile, (LPSTR) &bmfHeader, sizeof (bmfHeader)) !=
				sizeof (bmfHeader)) ||
				(bmfHeader.bfType != DIB_HEADER_MARKER))
			{
		      DIBError( ERR_NO_CONV );
      		goto ReadGRet;
			}
			// Allocate memory for DIB
		   hDIB = DVGlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, dwBitsSize - sizeof(BITMAPFILEHEADER));
			if( hDIB == 0 )
			{
				DIBError( ERR_MEMORY );
				goto ReadGRet;
			}
			lpDIB = DVGlobalLock( hDIB ); // LOCK DIB HANDLE
			if( lpDIB == 0 )
			{
				DIBError( ERR_MEMORY );
				goto ReadGRet;
			}
			// Go read the bits.
			if( DVRead( hFile, lpDIB, dwBitsSize - sizeof(BITMAPFILEHEADER)) !=
				(dwBitsSize - sizeof(BITMAPFILEHEADER)) )
			{
				DVGlobalUnlock (hDIB);  // UNLOCK DIB HANDLE - fore free and error exit
				DVGlobalFree   (hDIB);
				DIBError (ERR_READ);
				hDIB = 0;
				goto ReadGRet;
			}
	   DVGlobalUnlock (hDIB);  // UNLOCK DIB HANDLE
	}
	else
	{
		hDIB = 0;
		DIBError( ERR_READ );
	}
ReadGRet:
	if( lpo && (lpo[0]) )
	{
		DVOpenFile( lpo, &ofs, OF_DELETE);
	}

#ifndef	CVLIBBUG
	CloseJPEGLib();	/* If CVLIBBUG then only called from FRAME.c at exit */
#endif   // CVLIBBUG

	if( hgGIF && lpGIF )
		DVGlobalUnlock( hgGIF );
	if( hgGIF )
		DVGlobalFree( hgGIF );
	if( hgINF && lpINF )
		DVGlobalUnlock( hgINF );
	if( hgINF )
		DVGlobalFree( hgINF );
	hgGIF = 0;
	lpGIF = 0;
	hgINF = 0;
	lpINF = 0;
return( hDIB );	/* RETURN Handle or NULL if failed ... */
}
#endif	//	ADDRDGIF	// No longer USED!!!

#endif	/* ADDWJPEG */

////////////////////////////////////////////////////////////
#endif // #ifdef ADD_JPEG_SUPPORT


// eof - DvLib.c
