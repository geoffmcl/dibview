

// DvDiag.c
#include	"dv.h"
//#include  "dvdiag.h"

#define		WRTPDIAG
//extern	char	gszDiag[];	// abt 1024 General diag buffer

//#define	MXSBUF		256
// MAKE SURE the standard DOS Cr/Lf is used
// ========================================
int	outit( HFILE hf, LPSTR lpb, int len )
{
	char	buf[MXSBUF+8];
	int		i, j, k, wtn;
	char	c, d;

	wtn = 0;
	if( hf && (hf != HFILE_ERROR) &&
		lpb && len &&
		(i = lstrlen( lpb )) )
	{
//		if( i > MXSBUF )
//		{
//			i = MXSBUF;
//		}
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
			if( k >= MXSBUF )
			{
				wtn += _lwrite( hf, &buf[0], k );
				k = 0;
			}
		}
		if( c != '\n' )
		{
			buf[k++] = '\r';
			buf[k++] = '\n';
		}
		if( k )
			wtn += _lwrite( hf, &buf[0], k );
	}
	return wtn;
}

#ifdef	WRTPDIAG

char	szTempPal[] = "TEMPPAL.TXT";
BOOL	fAddFileP = FALSE;

HFILE	hOutFilP = 0;
#define	closeit			CloseOutP()

HFILE OpenFileP( LPSTR lpFileName,	// pointer to filename
				LPOFSTRUCT lpO,	// pointer to buffer for file information
				UINT uStyle	) // action and attributes
{
	HFILE hf;
	hf = OpenFile( lpFileName, lpO, uStyle );
	return hf;
}

void	OpenOutP( void )
{
	DWORD		fo;
	OFSTRUCT	of;
	LPSTR		lpf;

	lpf = &szTempPal[0];
	if( fAddFileP )
	{
		if( ( hOutFilP = OpenFileP( lpf, &of, OF_READWRITE ) ) &&
			( hOutFilP != HFILE_ERROR ) )
		{
			fo = _llseek( hOutFilP, 0, 2 );
			if( fo > 1000000 )
			{
				fAddFileP = FALSE;
				_lclose( hOutFilP );
				hOutFilP = OpenFileP( lpf, &of, OF_CREATE | OF_WRITE );
			}
		}
		else
		{
			hOutFilP = OpenFileP( lpf, &of, OF_CREATE | OF_WRITE );
		}
	}
	else
	{
		hOutFilP = OpenFileP( lpf, &of, OF_CREATE | OF_WRITE );
	}
}

void	CloseOutP( void )
{
	if( hOutFilP && (hOutFilP != HFILE_ERROR) )
		_lclose( hOutFilP );
	hOutFilP = 0;
}

#define	pwrtit( a ) \
{\
	if( hOutFilP == 0 )\
		OpenOutP();\
	if( a && strlen(a) && hOutFilP && (hOutFilP != HFILE_ERROR) )\
		outit( hOutFilP, a, lstrlen( a ) );\
}


//typedef struct tagLOGPALETTE { // lgpl  
//    WORD         palVersion; 
//    WORD         palNumEntries; 
//    PALETTEENTRY palPalEntry[1]; 
//} LOGPALETTE; 

void	WrtPalDiag( LPLOGPALETTE lpPal, BOOL fAppd,
				   LPSTR lpInfo, LPDWORD lpDW )
{
	BOOL	cFlg;
	WORD	wc, ic;
	BYTE	c[8];
	LPSTR	lpb;
	int		i;
	DWORD	dwTCnt;

	cFlg = fAddFileP;
	fAddFileP = fAppd;
	dwTCnt = 0;

	if( lpPal &&
		( wc = lpPal->palNumEntries ) )
	{
		lpb = GetTmp2();
		sprintf( lpb, "Palette Dump - %u colours ..."MEOR,
			wc );
		if( lpInfo && ( i = (int) strlen( lpInfo ) ) )
		{
			if( ( strlen( lpb ) + i ) < 1024 ) // MXTMPBUF
			{
				strcat( lpb, lpInfo );
			}
		}
		pwrtit( lpb );
		if( lpDW )
		{
			for( ic = 0; ic < wc; ic++ )
			{
				c[0] = lpPal->palPalEntry[ic].peRed;
				c[1] = lpPal->palPalEntry[ic].peGreen;
				c[2] = lpPal->palPalEntry[ic].peBlue;
				c[3] = lpPal->palPalEntry[ic].peFlags;
				sprintf( lpb, "%3u,%3u,%3u %6u"MEOR,
					c[0], c[1], c[2], lpDW[ic] );
				pwrtit( lpb );
				dwTCnt += lpDW[ic];
			}
		}
		else
		{
			for( ic = 0; ic < wc; ic++ )
			{
				c[0] = lpPal->palPalEntry[ic].peRed;
				c[1] = lpPal->palPalEntry[ic].peGreen;
				c[2] = lpPal->palPalEntry[ic].peBlue;
				c[3] = lpPal->palPalEntry[ic].peFlags;
				sprintf( lpb, "%3u,%3u,%3u %3u"MEOR,
					c[0], c[1], c[2], c[3] );
				pwrtit( lpb );
			}
		}
		if( dwTCnt )
		{
			sprintf( lpb, "Total of Frequencies = %u"MEOR,
				dwTCnt );
			pwrtit( lpb );
		}
		closeit;
	}
	fAddFileP = cFlg;

}

#endif	// WRTPDIAG

#ifdef	DIAGFILE2
#define		ADDDTIME

// **************************************************
// All moved to WORK STRUCTURE
// HFILE	hDiag = 0;
// char	szDefDiag[] = "TEMPDV32.TXT";
// Diag. file moved to gszDefDiag, which is actually
// char	w_szDefDiag[MAX_PATH];	// = "TEMPDV32.TXT";
// in extern	WRKSTR	WrkStr;
// **************************************************

#ifdef	ADDDTIME
extern	LPSTR	GetDT4s( int Typ );
void	OutBTime( void )
{
	char	buf[128];
	LPSTR	lps;
	lps = &buf[0];
	sprintf( lps,
		"Begin  at %s"MEOR,
		GetDT4s(0) );
	DO( lps );
}
void	OutETime( void )
{
	char	buf[128];
	LPSTR	lps;
	lps = &buf[0];
	sprintf( lps,
		"Ending at %s"MEOR,
		GetDT4s(0) );
	DO( lps );
}
#endif	// ADDDTIME


void	DiagOpen( void )
{
	OFSTRUCT	of;
	ghDiag = OpenFile( gszDefDiag, &of, OF_CREATE );
#ifdef	ADDDTIME
	OutBTime();
#endif	// ADDDTIME
}

void	DiagString( LPSTR lps )
{
	int	i;
	i = 0;
	if( ( lps ) &&
		( i = lstrlen( lps ) ) &&
		( ghDiag ) &&
		( ghDiag != HFILE_ERROR ) )
	{
		outit( ghDiag, lps, i );
	}
}

void	DiagClose( void )
{
	if( ( ghDiag ) &&
		( ghDiag != HFILE_ERROR ) )
	{
#ifdef	ADDDTIME
		OutETime();
#endif	// ADDDTIME
		_lclose( ghDiag );
	}
	ghDiag = 0;
}

#else	// !DIAGFILE2
void	DiagOpen( void )
{
}
void	DiagString( LPSTR lps )
{
}
void	DiagClose( void )
{
}
#endif	// DIAGFILE2

BOOL  IsDiagOpen( VOID )
{
	if( ghDiag &&
      ( ghDiag != HFILE_ERROR ) )
      return TRUE;

   return FALSE;
}

void	chkit( char * cp )
{
	char * lpd = &gszDiag[0];
	sprintf( lpd,
		"ERROR: chkit=[%s].",
		cp );
	DO(lpd);
}

VOID  _cdecl chkme( LPTSTR lpf, ... )
{
	LPTSTR   lpd = &gszChkme[0];
   va_list arglist;
   va_start(arglist, lpf);
   vsprintf( lpd, lpf, arglist );
   va_end(arglist);
   DO(lpd);
   return;
}

#ifndef  NDEBUG

VOID  chkdi( LPTSTR lpm, INT iln )
{
   chkme( "WARNING: In module %s, line %d FAILED to get DIB INFO POINTER!",
      lpm, iln );
}

VOID  _cdecl sprtf( LPTSTR lpf, ... )
{
	LPTSTR   lpd = &gszSprtf[0];
   va_list arglist;
   va_start(arglist, lpf);
   vsprintf( lpd, lpf, arglist );
   va_end(arglist);
   DO(lpd);
   return;
}

#endif   // !NDEBUG

// eof - DvDiag.c
