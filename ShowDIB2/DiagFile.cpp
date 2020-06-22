
// DiagFile.cpp
// based on DiagFile.c in Dump4 ...
//#include	<windows.h>
#include "stdafx.h"
#include "resource.h"


#ifdef   NDEBUG
// *** NO DEBUG OUTPUT ***
int   _cdecl sprtf( LPTSTR lpf, ... )
{
   return 0;
}
// *** =============== ***
#else /* !NDEBUG = *** DEBUG OUTPUT *** */

// file handle
#define  VH(a)    ( a && ( a != (HANDLE)-1 ) )
HANDLE   ghDiagFile = 0;
TCHAR    szDiagFile[264] = { "TEMPDS2.TXT" };

BOOL  gWriteFile( HANDLE * ph, LPTSTR lpb, DWORD dwl )
{
   HANDLE   h;
   DWORD    dwi, dww;

      dww = 0;
   if( ph && lpb && (dwi = dwl) &&
      ( h = *ph ) &&
      ( VH(h) ) )
   {
      if( ( WriteFile(h,lpb,dwi,&dww,NULL) ) &&
         ( dwi == dww ) )
      {
         // success
      }
      else
      {
         CloseHandle(h);
         h = (HANDLE)-1;
         *ph = h;
         dww = 0;
      }
   }
   return( (BOOL)dww );
}

BOOL  gCloseFile( HANDLE * ph )
{
   HANDLE   h;
   BOOL  bRet = FALSE;
   if( ph &&
      ( h = *ph ) &&
      ( VH(h) ) )
   {
      if( CloseHandle(h) )
         bRet = TRUE;

      h = 0;
      *ph = h;

   }
   return bRet;
}


void	gOpenFile( LPSTR lpf, HANDLE * lpHF, BOOL bApd )
{
	HFILE		hf;
	OFSTRUCT	of;
	DWORD		fs;

	hf = HFILE_ERROR;
	if( lpf && *lpf )
	{
		if( bApd )
		{
			if( ( hf = OpenFile( lpf, &of, OF_READWRITE ) ) &&
				( hf != HFILE_ERROR ) )
			{
				fs = _llseek( hf, 0, 2 );
				if( fs > 1000000 )
				{
					_lclose( hf );
					hf = OpenFile( lpf, &of, OF_CREATE | OF_READWRITE );
				}
			}
			else
			{
				hf = OpenFile( lpf, &of, OF_CREATE | OF_READWRITE );
			}
		}
		else
		{
			hf = OpenFile( lpf, &of, OF_CREATE | OF_READWRITE );
		}
	}
	if( lpHF )
		*lpHF = (HANDLE)hf;
}

void	gCloseFile( HFILE * lpHF )
{
	HFILE	hf;
	if( lpHF )
	{
		if( ( hf = *lpHF ) &&
			( hf != HFILE_ERROR ) )
		{
			_lclose( hf );
		}
		*lpHF = 0;
	}
}

BOOL  oi( LPTSTR lpb, DWORD dwl )
{
   BOOL  bRet = FALSE;
   if( ghDiagFile == 0 )
   {
      gOpenFile( szDiagFile,  // [264] = { "TEMPDS2.TXT" };
         &ghDiagFile,
         FALSE );
   }
   if( VH(ghDiagFile) )
   {
      if( gWriteFile( &ghDiagFile, lpb, dwl ) )
      {
         // success
         bRet = TRUE;
      }
   }
   return bRet;
}

#define	MXIOB		1024
static char	prtbuf[MXIOB+4];
static DWORD dwTtl = 0;
void	prt( LPTSTR lps )
{
	char	c, d;
   DWORD dwLen;
   DWORD dwi, dwk;
	LPTSTR	lpout;

	{
		if( dwLen = lstrlen( lps ) )
		{
			lpout = &prtbuf[0];
			dwk = 0;
			d = 0;
			for( dwi = 0; dwi < dwLen; dwi++ )
			{
				c = lps[dwi];
				if( c == 0x0d )
				{
					if( (dwi+1) < dwLen )
					{
						if( lps[dwi+1] != 0x0a )
						{
							lpout[dwk++] = c;
							c = 0x0a;
						}
					}
					else
					{
						lpout[dwk++] = c;
						c = 0x0a;
					}
				}
				else if( c == 0x0a )
				{
					if( d != 0x0d )
					{
						lpout[dwk++] = 0x0d;
					}
				}
				lpout[dwk++] = c;
				d = c;
				if( dwk >= MXIOB )
				{
					if( oi( lpout, dwk ) )
   					dwTtl += dwk;
					dwk = 0;
				}
			}
			if( dwk )
			{
					if( oi( lpout, dwk ) )
   					dwTtl += dwk;
					dwk = 0;
			}
		}
	}
}


int   _cdecl sprtf( LPTSTR lpf, ... )
{
   static TCHAR _s_sprtfbuf[1024];
   LPTSTR   lpb = &_s_sprtfbuf[0];
   int   i;
   va_list arglist;
   va_start(arglist, lpf);
   i = wvsprintf( lpb, lpf, arglist );
   va_end(arglist);
   prt(lpb);
   return i;
}

#endif   /* NDEBUG y/n */
// eof - DiagFile.cpp
