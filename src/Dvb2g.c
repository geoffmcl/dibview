
/* =================================================================
 * DvB2g.c
 *
 * Conversion of a BITMAP into a GIF file
 *
 * ================================================================= */

// #include "winclude.h"
#include	"dv.h"
#include	"DvB2g.h"
#include	"DvGif2.h"

#undef	WRTFILE
#define	FIXONE
#define	FIXTWO

#define	DEF_RED			0	/* Used to fill out colors to 256 */
#define	DEF_GREEN		0
#define	DEF_BLUE		0	

#ifdef	ADDPROGM
extern	void PutProgM( void );
extern	void KillProgM( void );
extern	void SetProgM( DWORD, DWORD );
extern	void SetProgM1( void );
extern	void SetProgMInfo( LPSTR );
extern	BOOL bUsrAbort;
extern	DWORD wCurPCT;	/* Current active percentage ... */
#endif	/* ADDPROGM */

extern void DVselwgif( decompress_info_ptr );
extern DWORD DVWrite(HANDLE, VOID MLPTR, DWORD);
extern HPALETTE CreateDIBPal24( PDI, HANDLE, BOOL );

// NOTE: These are the OLD JPEG < 4 I think structures
// See the NEWER structures in WJPGLIB2.DLL - Source in WJPG32_2
// =============================================================

Decompress_info_struct1	DCinfo;	/* Define these HERE ... */
struct External_methods_struct1 e_methods;
struct Decompress_methods_struct1 c_methods;


BOOL	fInvColor = TRUE;
#ifdef	WRTFILE
#pragma message( "WARNING: File write DEBUG is ***ON***!" )
extern	DWORD MyDOSWrite( HFILE, LPSTR, DWORD );
char	szTmpPal[] = "TEMPPAL.FIL";
HFILE	hFile;
#endif	/* WRTFILE */
HANDLE SortPal256( LPLOGPALETTE, LPSTR );
HANDLE BuildNewDib( decompress_info_ptr, HANDLE, LPLOGPALETTE );
BOOL	fVeryVerb = TRUE;

void	DV_putc( int c, HFILE hf )
{
	PINT8 hp;

	if( DCinfo.lpOut && (DCinfo.ddOut < DCinfo.ddOSz) )
	{
		hp = (PINT8) DCinfo.lpOut + DCinfo.ddOut;
		*hp = c & 0xff;
		DCinfo.ddOut++;
	}
}

int	b_write( HFILE hf, LPSTR lpb, int Siz )
{
	int	i, dn;

	dn = 0;
	if( lpb && Siz )
	{
		for( i = 0; i < Siz; i++ )
		{
			DV_putc( lpb[i], hf );
			dn++;
		}
	}
	return( dn );
}

void	DV_fflush( HFILE hf )
{
	int i;
	i = 0;
}

BOOL	Dob2g( decompress_info_ptr cinfo )
{
   BOOL	flg = TRUE;
   HGLOBAL	hgCA;
   JSAMPARRAY lpS;
   DWORD	i, j;
   long	wid, hgt, li, nwid;
   DWORD	cnt;
   LPRGBQUAD lpC;
   LPSTR	lps;
   LPSTR *lpdw;
   PINT8	hp;
   HFILE	hf;

	hgCA = 0;
	(*cinfo->methods->output_init) (cinfo);
	i = cinfo->desired_number_of_colors;
	cnt = (i * 3) + (sizeof(LPSTR) * 3);
	if( ( hgCA = DVGlobalAlloc( GHND, cnt+2 ) ) &&
		( lps = DVGlobalLock( hgCA ) ) )    // lock handle
	{
		lpC = (LPRGBQUAD) (cinfo->lpDib + sizeof( BITMAPINFOHEADER ));
		lpdw = (LPSTR *)lps;
		lpdw[0] = (LPSTR) (lps + (sizeof(LPSTR) * 3) + (i * 0));
		lpdw[1] = (LPSTR) (lps + (sizeof(LPSTR) * 3) + (i * 1));
		lpdw[2] = (LPSTR) (lps + (sizeof(LPSTR) * 3) + (i * 2));
		lpS = (JSAMPARRAY) lps;
		if( fInvColor )
		{
			for( j = 0; j < i; j++ )
			{
				lpS[2][j] = lpC[j].rgbBlue;
				lpS[1][j] = lpC[j].rgbGreen;
				lpS[0][j] = lpC[j].rgbRed;
			}
		}
		else
		{
			for( j = 0; j < i; j++ )
			{
				lpS[0][j] = lpC[j].rgbBlue;
				lpS[1][j] = lpC[j].rgbGreen;
				lpS[2][j] = lpC[j].rgbRed;
			}
		}
		(*cinfo->methods->put_color_map) ( cinfo, i, lpS ); 
		DVGlobalUnlock( hgCA );    // unlock handle
		DVGlobalFree( hgCA );
		hgCA = 0;
		wid = cinfo->image_width;
		hgt = cinfo->image_height;
		if( wid % 4 )
			nwid = ((wid / 4) + 1) * 4;
		else
			nwid = wid;
		cnt = cinfo->image_height * sizeof( LPSTR );
		if( wid && LOWORD(hgt) && cnt && 
			(hgCA = DVGlobalAlloc( GHND, cnt + 2 ) ) &&
			( lps = DVGlobalLock( hgCA ) ) )    // lock handle
		{
			lpdw = (LPSTR *)lps;
			hp = ( PINT8 ) FindDIBBits( cinfo->lpDib );
			for( i = 0, li = hgt; li > 0; i++, li-- )
			{
				cnt = (li-1) * nwid;
				lpdw[i] = (hp + cnt);	/* Set POINTERS for each row ... */
			}
  			(*cinfo->methods->put_pixel_rows) (cinfo,
					LOWORD(hgt), /* Number of image ROWS ... */
					(JSAMPIMAGE) &lps);	/* Array of POINTERS to data ... */

			DVGlobalUnlock( hgCA ); // unlock handle
			DVGlobalFree( hgCA );
			hgCA = 0;
			(*cinfo->methods->output_term) (cinfo);
			if( cinfo->lpOut && cinfo->ddOut && (cinfo->ddOut <= cinfo->ddOSz) &&
				cinfo->lpFil )
			{
				if( ( hf = _lcreat( cinfo->lpFil, 0 ) ) &&
					( hf != HFILE_ERROR ) )
				{
					if( DVWrite( IntToPtr(hf), (VOID MLPTR) cinfo->lpOut, cinfo->ddOut ) ==
						cinfo->ddOut )
					{
						flg = FALSE;
					}
					DVlclose( IntToPtr(hf) );
				}
			}
		}
	}
	if( hgCA )
		DVGlobalFree( hgCA );
return( flg );
}

void	chku2( void )
{
int	i;
	i = 0;
}

BOOL	B2G( HGLOBAL hDib, DWORD ddSz, HGLOBAL hOut, DWORD ddOut, LPSTR lpf )
{
   BOOL	flg;
   HPALETTE hP, hP2;
   LPLOGPALETTE lpP, lpP2, lpPal, lpPal2;
   DWORD	wCC, wTCC, wCCols, ccc, cc2, MPos;
   DWORD	ddSiz, ddCnt, lCnt, MSiz, TSiz;
   BYTE	c[4];
   HANDLE	hP256;
   HGLOBAL	hDIB2;

	flg = TRUE;	/* Start as error */
	DCinfo.hgDib = 0;
	DCinfo.lpDib = 0;
	DCinfo.ddDSz = 0;
	DCinfo.hgOut = 0;
	DCinfo.lpOut = 0;
	DCinfo.ddOSz = 0;
	DCinfo.ddOut = 0;
	ddSiz = ddCnt = 0;
	hP256 = 0;
	if( hDib && ddSz && hOut && ddOut && lpf )
	{
		DCinfo.hgDib = hDib;
		DCinfo.lpDib = DVGlobalLock( hDib );   // LOCK DIB HANDLE
		DCinfo.ddDSz = ddSz;
		DCinfo.hgOut = hOut;
		DCinfo.lpOut = DVGlobalLock( hOut );
		DCinfo.ddOSz = ddOut;
		DCinfo.ddOut = 0;
		DCinfo.lpFil = lpf;
		DCinfo.output_file = 0;
		DCinfo.desired_number_of_colors = 256;
		if( DCinfo.lpDib && DCinfo.lpOut )
		{
			if( DCinfo.desired_number_of_colors = DIBNumColors( DCinfo.lpDib) )
			{
      		DCinfo.image_height = DIBHeight( DCinfo.lpDib );
      		DCinfo.image_width  = DIBWidth( DCinfo.lpDib );
				DCinfo.data_precision = DIBBitCount( DCinfo.lpDib );
				DCinfo.out_color_space = CS_RGB;
   			DCinfo.quantize_colors = TRUE;
				DCinfo.methods = &c_methods;
				DCinfo.emethods = &e_methods;
				DCinfo.final_out_comps = 1;
				DVselwgif( &DCinfo );
				flg = Dob2g( &DCinfo );
			}
			else	/* NO COLORS = 24-bit BITMAP ... */
			{
#ifdef	ADDPROGM
				PutProgM();
#endif
      		if( (DCinfo.image_height = DIBHeight( DCinfo.lpDib )) &&
      			(DCinfo.image_width  = DIBWidth( DCinfo.lpDib )) &&
					( hP = CreateDIBPal24( 0, hDib, TRUE ) ) )
				{
					if( lpP = (LPLOGPALETTE) DVGlobalLock( hP ) )
					{
						if( wCC = lpP->palNumEntries )
						{
#ifdef	ADDPROGM
							wsprintf( DCinfo.lpOut, "Sorting %u Colors!", wCC );
							SetProgMInfo( DCinfo.lpOut );
							if( bUsrAbort )
								goto DnTry;
#endif
							wTCC = wCC;
							hP2 = 0;
							lpP2 = 0;
							ddSiz = sizeof( LOGPALETTE ) + 
								( sizeof( PALETTEENTRY ) * wTCC );
							if( (hP2 = (HPALETTE) DVGlobalAlloc( GHND, ddSiz ) ) &&
								(lpP2 = (LPLOGPALETTE) DVGlobalLock( hP2 ) ) )
							{
/* ============================================= */
				lpPal  = lpP;
				lpPal2 = lpP2;
				wCCols = wCC;
				flg = FALSE;
				lpPal2->palVersion    = PALVERSION;
				lpPal2->palNumEntries = (WORD)wCCols;
				lCnt = wCCols * 2;
				for( cc2 = 0; cc2 < wCCols; cc2++ )
				{
					MSiz = RGB( 255, 255, 255 );
					for( ccc = 0; ccc < wCCols; ccc++ )
					{
						if( lpPal->palPalEntry[ccc].peFlags )
						{
							c[0] = lpPal->palPalEntry[ccc].peBlue;
							c[1] = lpPal->palPalEntry[ccc].peGreen;
							c[2] = lpPal->palPalEntry[ccc].peRed;
							TSiz = RGB( c[0], c[1], c[2] );
							if( TSiz <= MSiz )
							{
								MSiz = TSiz;
								MPos = ccc;
							}
						}
					}
					/* Get this MINIMUM Entry ... */
					c[0] = lpPal->palPalEntry[MPos].peBlue;
					c[1] = lpPal->palPalEntry[MPos].peGreen;
					c[2] = lpPal->palPalEntry[MPos].peRed;
					c[3] = lpPal->palPalEntry[MPos].peFlags;
					/* Ensure entry NOT found again ... */
					lpPal->palPalEntry[MPos].peFlags = 0;
					/* Add this entry to lpPal2 ... */
					lpPal2->palPalEntry[cc2].peBlue = c[0];
					lpPal2->palPalEntry[cc2].peGreen = c[1];
					lpPal2->palPalEntry[cc2].peRed = c[2];
					lpPal2->palPalEntry[cc2].peFlags = c[3];
#ifdef	ADDPROGM
					SetProgM( (DWORD)((cc2 / 2) + wCCols), lCnt );
					if( bUsrAbort )
						flg = TRUE;
#endif
					if( flg )
						break;
				}	/* For each of the FOUND colors ... */
#ifdef	WRTFILE
				if( hFile = _lcreat( (LPSTR)szTmpPal, 0 ) )
				{
					MyDOSWrite( hFile, (LPSTR) lpPal2, ddSiz );
					DVlclose( hFile );
				}
#endif	/* WRTFILE */
				chku2();
				hP256 = SortPal256( lpPal2, DCinfo.lpOut );
				flg = TRUE;	/* SET ERROR since not finished here ... */
				if( hP256 )
				{	/* If a LOGICAL 256 type palette returned ... */
					if( hDIB2 = BuildNewDib( &DCinfo, hP256, lpPal2 ) )
					{
						if( hP256 )
							DVGlobalFree( hP256 );
						hP256 = 0;
						if( hP2 && lpP2 )
							DVGlobalUnlock( hP2 );
						lpP2 = 0;
						if( hP2 )
							DVGlobalFree( hP2 );
						hP2 = 0;
						if( DCinfo.hgDib && DCinfo.lpDib )
							DVGlobalUnlock( hDib );
						DCinfo.lpDib = 0;
						if( DCinfo.hgOut && DCinfo.lpOut )
							 DVGlobalUnlock( hOut );
						DCinfo.lpOut = 0;
//						if( OpenDIBWindow( hDIB2, lpf, 0x02000000 ) )
//						{	/* All appears OK - Call this routine with NEW DIB */
#ifdef	ADDPROGM
							SetProgMInfo( "Converting NEW DIB to GIF ..." );
							SetProgM( 99, 100 );
#endif	/* ADDPROGM */
							flg = B2G( hDIB2, ddSz, hOut, ddOut, lpf );
//						}	/* =============================================== */
						DVGlobalFree( hDIB2 );
						hDIB2 = 0;
					}
					if( hP256 )
						DVGlobalFree( hP256 );
				}
/* ============================================= */
							}
							chku2();
							if( hP2 && lpP2 )
								DVGlobalUnlock( hP2 );
							lpP2 = 0;
							if( hP2 )
								DVGlobalFree( hP2 );
							hP2 = 0;
						}
DnTry:
						DVGlobalUnlock( hP );
						lpP = 0;
					}					
					DVGlobalFree( hP );
					hP = 0;
				}
#ifdef	ADDPROGM
				KillProgM();
#endif
			}
		}
	}

	if( DCinfo.hgDib && DCinfo.lpDib )
		DVGlobalUnlock( hDib ); // UNLOCK DIB HANDLE
	DCinfo.lpDib = 0;
	if( DCinfo.hgOut && DCinfo.lpOut )
		 DVGlobalUnlock( hOut );
	DCinfo.lpOut = 0;
return( flg );
}

/* eof */

#define	b2gMXBUF			4096
#define	b2gMXBAL			256

DWORD	BCnt = 0;
BOOL	fOvr = FALSE;

/* Logical 256 color Palette */
typedef struct tagLOGPAL256
{
    WORD    palVersion;
    WORD    palNumEntries;
    PALETTEENTRY palPalEntry[256];
} LOGPAL256;
typedef LOGPAL256 * PLOGPAL256;
typedef LOGPAL256 MLPTR LPLOGPAL256;

// LOGPAL256	LogPal256;

void	ClrDelta( LPWORD lpDB, LPWORD lpDC, LPWORD lpIC )
{
   DWORD	i;
	for( i = 0; i < b2gMXBUF; i++ )
	{
		lpDB[i] = 0;
		lpDC[i] = 0;
		lpIC[i] = 0;
	}
	fOvr = FALSE;
	BCnt = 0;
}
void	InsertDelta( WORD d, BYTE cnt, LPWORD lpDB, LPWORD lpDC, LPWORD lpIC )
{
   DWORD	i;
   DWORD	wDB, wDC, wIC;
   DWORD	wDB2, wDC2, wIC2;
   BOOL	flg;
	flg = TRUE;
	for( i = 0; i < BCnt; i++ )
	{
		if( lpDB[i] > d )
		{
			wDB = lpDB[i];
			wDC = lpDC[i];	/* Extract current ... */
			wIC = lpIC[i];
			lpDB[i] = d;	/* INSERT smaller ... */
			lpDC[i] = 1;
			lpIC[i] = (WORD) cnt;
			i++;				/* Bump to NEXT ... */
			flg = FALSE;
			for( ; i <= BCnt; i++ )
			{
				wDB2 = lpDB[i];
				wDC2 = lpDC[i];
				wIC2 = lpIC[i];
				lpDB[i] = (WORD)wDB;
				lpDC[i] = (WORD)wDC;
				lpIC[i] = (WORD)wIC;
				wDB = wDB2;
				wDC = wDC2;
				wIC = wIC2;
			}
		}
	}
	if( flg )
	{
		lpDB[BCnt] = d;
		lpDC[BCnt] = 1;
		lpIC[BCnt] = (WORD) cnt;
	}
}

void	AddDelta( WORD d, BYTE cnt, LPWORD lpDB, LPWORD lpDC, LPWORD lpIC )
{
   DWORD	i;
	for( i = 0; i < BCnt; i++ )
	{
		if( d == lpDB[i] )
			break;
	}
	if( i == BCnt )
	{
		if( BCnt )
		{
			InsertDelta( d, cnt, lpDB, lpDC, lpIC );
		}
		else
		{
			lpDB[i] = d;
			lpDC[i] = 1;
			lpIC[i] = (WORD) cnt;
		}
		if( BCnt < b2gMXBUF )
			BCnt++;
		else
			fOvr = TRUE;			
	}
	else
	{
		lpDC[i]++;
		lpIC[i] += (WORD) cnt;
	}
}

DWORD	GetSizeSLP256( void )
{
	return( sizeof( LOGPAL256 ) );
}

/* ======================= SortPal256(...) =======================
 * Extract a 256 Color palette from the BIG COLOR MAP
 * Passed a LOGICAL PALETTE of colors, if sucessful will return
 * a (less than or equal to) 256 color palette ...
 * Uses (deltaR^2 + deltaG^2 + deltaB^2) when matching colors
 * together. 
 * =============================================================== */
HANDLE SortPal256( LPLOGPALETTE lpLogPal, LPSTR lpb )
{
	HANDLE	i;
	LPLOGPALETTE	plp;
	LPLOGPAL256  plp2;
	LPPALETTEENTRY ppe, ppe2;
	WORD	ver, ccnt, delta, wlp, blp, mlp, wa, bal, mdelt, mwlp, lp;
	BYTE	c[4], d[4], diff[4];
	WORD	dif2[4];
	BOOL	fdn;
	//DWORD	fSiz;
	DWORD ddRd;
	//WORD	wSiz;
	//void MLPTR fptr;
	WORD	wBgn, wCnt, wBLp, wCol;
	BYTE	fc[4];
	LPWORD	lpDB, lpDC, lpIC;
	HGLOBAL	hWBuf, hP256;
	LPSTR		lpWBuf;

	i = 0;
//	plp2 = &LogPal256;
//	ppe2 = &plp2->palPalEntry[0];	
	plp2 = 0;
	ppe2 = 0;	
	ddRd = (b2gMXBUF+2) * 3	* 2;
	hWBuf = 0;
	lpWBuf = 0;
	wBLp = 0;
	if( ( plp = lpLogPal ) &&
		( hWBuf = DVGlobalAlloc( GHND, ddRd ) ) &&
		( hP256 = DVGlobalAlloc( GHND, GetSizeSLP256() ) ) && 
		( lpWBuf = DVGlobalLock( hWBuf )) &&
		( plp2 = (LPLOGPAL256) DVGlobalLock( hP256 ) ) )
	{
	lpDB = (LPWORD) lpWBuf;
	lpDC = lpDB + b2gMXBUF;
	lpIC = lpDC + b2gMXBUF;
	plp2->palVersion = 0x300;
	ppe2 = &plp2->palPalEntry[0];
	ppe = &plp->palPalEntry[0];
	ver = plp->palVersion;
	ccnt = plp->palNumEntries;
#ifdef	ADDPROGM
	SetProgM1();	/* Bump to 75% ... */
	if( bUsrAbort )
		goto DnSort;
#endif	/* ADDPROGM */
		if( ccnt <= 256 )
		{	/* OK, very little to do. We have the REQUIRED number of colors */
			plp2->palNumEntries = ccnt;
			for( lp = 0; lp < ccnt; lp++ )
			{
				c[0] = ppe[lp].peRed;
				c[1] = ppe[lp].peGreen;
				c[2] = ppe[lp].peBlue;
				c[3] = ppe[lp].peFlags;		/* Count of instances ... */
				ppe2[lp].peRed = c[0];	
				ppe2[lp].peGreen = c[1];	
				ppe2[lp].peBlue = c[2];	
				ppe2[lp].peFlags = 0;
				/* Set INDEX for this COLOR */
				/* ======================== */
				ppe[lp].peFlags = LOBYTE( lp );	/* Count no longer required! */
				/* Used in BuildNewDib to match with 24-bit source */
				/* =============================================== */
			}
			for( ; lp < 256; lp++ )
			{
				ppe2[lp].peRed = DEF_RED;
				ppe2[lp].peGreen = DEF_GREEN;	
				ppe2[lp].peBlue = DEF_BLUE;	
				ppe2[lp].peFlags = LOBYTE( lp );
			}
			i = hP256;
			goto DnSort;
		}
//		printf( "Palette Version: %X\n", ver );
//		printf( "Color Count    : %d\n", ccnt );
//		if( ver == 0x300 )
//		{
//			printf( "     Red  Grn  Blu   Count  Delta\n" );
#ifdef	ADDPROGM
				SetProgMInfo( "Calculating color deltas ..." );
#endif
			ClrDelta( lpDB, lpDC, lpIC );
			d[0] = 0;
			d[1] = 0;
			d[2] = 0;
			d[3] = 0;
// NxtSet:
			for( lp = 0; lp < ccnt; lp++ )
			{
				c[0] = ppe->peRed;
				c[1] = ppe->peGreen;
				c[2] = ppe->peBlue;
				c[3] = ppe->peFlags;		/* Count of instances ... */
				if( c[0] >= d[0] ) 
					diff[0] = c[0] - d[0];
				else
					diff[0] = d[0] - c[0];
				if( c[1] >= d[1] )
					diff[1] = c[1] - d[1];
				else
					diff[1] = d[1] - c[1];
				if( c[2] >= d[2] )
					diff[2] = c[2] - d[2];
				else
					diff[2] = d[2] - c[2];
				dif2[0] = diff[0] * diff[0];
				dif2[1] = diff[1] * diff[1];
				dif2[2] = diff[2] * diff[2];
				delta = dif2[0] + dif2[1] + dif2[2];
//				printf( "RGB( %3d, %3d, %3d ) = %3d  (%5u)\n",
//					c[0], c[1], c[2], c[3], delta );
				/* Note: First DELTA is always ZERO, but no problems with this */
				AddDelta( delta, c[3], lpDB, lpDC, lpIC );
				ppe++;
				d[0] = c[0];
				d[1] = c[1];
				d[2] = c[2];
			} 
//				printf( "Processed %d colors ...\n", ccnt );
#ifdef	ADDPROGM
				wsprintf( lpb, "Processed %d colors ...", ccnt );
				SetProgMInfo( lpb );
				SetProgM1();	/* Bump to 76% ... */
				if( bUsrAbort )
					goto DnSort;
#endif	/* ADDPROGM */
				if( fOvr )
				{
//					printf( "WARNING: Not all delta's accounted!\n\tMXBUF needs to be increased\n" );
#ifdef	ADDPROGM
				SetProgMInfo( "WARNING: Not all delta's accounted!\nMXBUF needs to be increased!" );
#endif
				}
				if( BCnt )
				{
//					printf( "List of Delta's ...\n" );
					mlp = (WORD) -1;
					mdelt = 0;
					wa = 0;
					mwlp = 0;
					fdn = FALSE;
						for( wlp = 0; wlp < BCnt; wlp++ )
						{
								wa += lpDC[wlp];
								bal = (ccnt - wa);
//								printf( "Delta %5u Count %5u Accum %5u Bal %5u Used %5u\n",
//									lpDB[wlp], lpDC[wlp], wa, bal, lpIC[wlp] );
								if( !fdn && (bal <= b2gMXBAL) )
								{
									fdn = TRUE;
									mdelt = lpDB[wlp];
//									printf( "Delta's up to %5u for just %u colors!\n",
//											mdelt, b2gMXBAL );
									mwlp = wlp;
								}
						}
//					printf( "Done." );
#ifdef	ADDPROGM
				wsprintf( lpb, "Done. Delta at %u for %u colors.", mdelt, b2gMXBAL );
				SetProgMInfo( lpb );
#endif
					if( mdelt && !fOvr )
					{
/* ============================================================== */
//						printf( " Now to merge colors up to delta %u ...\n",
//							mdelt );
						ppe = &plp->palPalEntry[0];	/* Reset back to start */
			d[0] = 0;
			d[1] = 0;
			d[2] = 0;
			d[3] = 0;
ColList:
			d[0] = ppe->peRed;
			d[1] = ppe->peGreen;
			d[2] = ppe->peBlue;
			d[3] = ppe->peFlags;		/* Count of instances ... */
			blp = 0;
			wa = 0;
			for( lp = 0; lp < ccnt; lp++ )
			{
				wa++;
				c[0] = ppe[lp].peRed;
				c[1] = ppe[lp].peGreen;
				c[2] = ppe[lp].peBlue;
				c[3] = ppe[lp].peFlags;		/* Count of instances ... */
				if( c[0] >= d[0] ) 
					diff[0] = c[0] - d[0];
				else
					diff[0] = d[0] - c[0];
				if( c[1] >= d[1] )
					diff[1] = c[1] - d[1];
				else
					diff[1] = d[1] - c[1];
				if( c[2] >= d[2] )
					diff[2] = c[2] - d[2];
				else
					diff[2] = d[2] - c[2];
				dif2[0] = diff[0] * diff[0];
				dif2[1] = diff[1] * diff[1];
				dif2[2] = diff[2] * diff[2];
				delta = dif2[0] + dif2[1] + dif2[2];
				if( delta > mdelt )
				{
					blp++;
//					printf( "RGB( %3d, %3d, %3d ) (d=%5u) %5u\n",
//						d[0], d[1], d[2], delta, blp );
#ifndef	FIXONE
					lp++;
#endif	/* !FIXONE */
					if( lp < ccnt )
					{
						d[0] = ppe[lp].peRed;
						d[1] = ppe[lp].peGreen;
						d[2] = ppe[lp].peBlue;
					}
					wa = 0;
				}
			} /* For the color list ... */
			if( wa )
				blp++;
			if( blp > 256 )
			{
//				printf( "Color count %u greater than 256 ...\n", blp );
				if( mwlp < BCnt )
				{
					mwlp++;
					mdelt = lpDB[mwlp];
//					printf( "Re-trying with delta of %u ...\n", mdelt );
					goto ColList;
				}
				else
				{
//					printf( "ERROR: Unable to choose ...\n" );
				}
			}
			else
			{
//				printf( "Got color count of %u ...\n", blp );
#ifdef	ADDPROGM
				wsprintf( lpb, "Fixed. Delta at %u for %u colors.", mdelt, blp );
				SetProgMInfo( lpb );
				SetProgM1();	/* Bump to 77% ... */
				if( bUsrAbort )
					goto DnSort;
#endif	/* ADDPROGM */
				d[0] = ppe->peRed;
				d[1] = ppe->peGreen;
				d[2] = ppe->peBlue;
				d[3] = ppe->peFlags;		/* Count of instances ... */
				plp2->palVersion = 0x300;
				plp2->palNumEntries = blp;
				wa = 0;
				blp = 0;
				wBgn = 0;
				wCnt = 0;
				for( lp = 0; lp < ccnt; lp++ )
				{
					wa++;
					c[0] = ppe[lp].peRed;
					c[1] = ppe[lp].peGreen;
					c[2] = ppe[lp].peBlue;
					c[3] = ppe[lp].peFlags;		/* Count of instances ... */
					wCnt += (WORD) c[3];
#ifndef	FIXTWO
					ppe[lp].peFlags = LOBYTE( blp );	/* Set INDEX for this */
#endif	/* !FIXTWO */
				if( c[0] >= d[0] ) 
					diff[0] = c[0] - d[0];
				else
					diff[0] = d[0] - c[0];
				if( c[1] >= d[1] )
					diff[1] = c[1] - d[1];
				else
					diff[1] = d[1] - c[1];
				if( c[2] >= d[2] )
					diff[2] = c[2] - d[2];
				else
					diff[2] = d[2] - c[2];
				dif2[0] = diff[0] * diff[0];
				dif2[1] = diff[1] * diff[1];
				dif2[2] = diff[2] * diff[2];
				delta = dif2[0] + dif2[1] + dif2[2];
					if( delta > mdelt )
					{
						wa++;
						wCol = wBgn;	/* Start using FIRST color ... */
						if( wBgn && ((lp - wBgn) > 2 ) )
						{
							wCol += ((lp - wBgn) / 2);	/* Choose MIDDLE color */
						}
#ifdef	FIXTWO
						for( wBLp = wBgn; wBLp <= lp; wBLp++ )
						{
							if( wBgn && /* If NOT the FIRST color, and USE Greater */
								(ppe[wBLp].peFlags > ppe[wCol].peFlags) )	/* USE > */
							{
								wCol = wBLp;	/* Use COLOR of higher frequency */
							}
							/* Set INDEX for this COLOR */
							/* ======================== */
							ppe[wBLp].peFlags = LOBYTE( blp );
							/* Used in BuildNewDib to match with 24-bit source */
							/* =============================================== */
						}
#endif	/* FIXTWO */
						if( (lp + 1) == ccnt )	/* Override if LAST Color ... */
							wCol = lp;	/* Get LAST color ... */
						fc[0] = ppe[wCol].peRed;
						fc[1] = ppe[wCol].peGreen;
						fc[2] = ppe[wCol].peBlue;
						if( blp < 256 )
						{
							ppe2[blp].peRed = fc[0];	
							ppe2[blp].peGreen = fc[1];	
							ppe2[blp].peBlue = fc[2];	
							ppe2[blp].peFlags = 0;
						}
						blp++;
//		printf( "%3u RGB(%3d,%3d,%3d)to(%3d,%3d,%3d)[%3d] (d=%5u) C=%5u [%3d,%3d,%3d]\n",
//							blp,
//							d[0], d[1], d[2],
//							c[0], c[1], c[2],
//							wa, delta, wCnt,
//							fc[0], fc[1], fc[2] );
#ifndef	FIXONE
						lp++;
#endif	/* !FIXONE */
						if( lp < ccnt )
						{
							d[0] = ppe[lp].peRed;
							d[1] = ppe[lp].peGreen;
							d[2] = ppe[lp].peBlue;
							d[3] = ppe[lp].peFlags;
#ifndef	FIXONE
							ppe[lp].peFlags = LOBYTE( blp );
#endif	/* !FIXONE */
						}
						wa = 0;
#ifdef	FIXONE
						wBgn = lp + 1;
						wCnt = 0;
#else	/*!FIXONE */
						wBgn = lp;
						wCnt = (WORD) d[3];
#endif	/* FIXONE y/n */
					}
				} /* For the color list ... */
				if( wa )	/* If more at end ... */
				{
#ifdef	FIXTWO
						for( wBLp = wBgn; wBLp < lp; wBLp++ )
						{
							ppe[wBLp].peFlags = LOBYTE( blp );	/* Set INDEX for this */
						}
#endif	/* FIXTWO */
						wCol = ccnt - 1;	/* Use the LAST color ... */
						fc[0] = ppe[wCol].peRed;
						fc[1] = ppe[wCol].peGreen;
						fc[2] = ppe[wCol].peBlue;
						if( blp < 256 )
						{
							ppe2[blp].peRed = fc[0];	
							ppe2[blp].peGreen = fc[1];	
							ppe2[blp].peBlue = fc[2];	
							ppe2[blp].peFlags = 0;
						}
						blp++;
//		printf( "%3u RGB(%3d,%3d,%3d)to(%3d,%3d,%3d)[%3d] (d=%5u) C=%5u [%3d,%3d,%3d]\n",
//							blp,
//							d[0], d[1], d[2],
//							c[0], c[1], c[2],
//							wa, delta, wCnt,
//							fc[0], fc[1], fc[2] );
				}
				chku2();
				i = hP256;
			}
/* ================================================== */
					}
				}
//			}
//			else
//			{
//				goto NxtSet;
//			}
//		}
//		else
//		{
//			printf( "ERROR: Not palette version 300h ..." );
//			i = 1;
//		}
	}
	else
	{
		DIBError( ERR_MEMORY );
	}
DnSort:
	if( hWBuf && lpWBuf )
		DVGlobalUnlock( hWBuf );
	if( hWBuf )
		DVGlobalFree( hWBuf );
	if( hP256 && plp2 )
		DVGlobalUnlock( hP256 );
	if( i == 0 )
	{
		if( hP256 )
			DVGlobalFree( hP256 );
	}
return( i );
}

HANDLE BuildNewDib( decompress_info_ptr cinfo, HANDLE hP256, 
	LPLOGPALETTE lpPal2 )
{
   DWORD	wWid, wHgt, wNWid, ddSiz, i, j, wSWid, k;
   HGLOBAL	hDIB2, hRet;
   LPSTR	lpDIB2, lpDIB1;
   LPBITMAPINFOHEADER lpbi;
   LPLOGPALETTE lpPal3;
   PINT8	fpSrc;
   PINT8	fpDest;
   BYTE	c[4];
   BYTE	ind;
   LPPALETTEENTRY ppe1, ppe2;
   DWORD	ccnt, wk;
   char	buf[40];
   LPSTR	lpb;
#ifdef	ADDPROGM
   DWORD	bpct, cpct;
	cpct = wCurPCT;
	bpct = ((100 - cpct) * 100) / 98;
#endif	/* ADDPROGM */
	lpb = &buf[0];
	hDIB2 = 0;
	hRet = 0;
	wHgt = cinfo->image_height;
	wWid = cinfo->image_width;
	if( wWid % 4 )
		wNWid = ((wWid / 4) + 1) * 4;
	else
		wNWid = wWid;
	wSWid = 0;
	if( (wWid * 3) % 4 )
		wSWid = ((((wWid * 3) / 4) + 1) * 4) - (wWid * 3);
	ddSiz = sizeof( BITMAPINFOHEADER ) +
		(sizeof( RGBQUAD ) * 256) +
		( wNWid * wHgt );
	if( (lpPal3 = (LPLOGPALETTE) DVGlobalLock( hP256 )) &&
		( hDIB2 = DVGlobalAlloc( GHND, ddSiz ) ) &&
		( lpDIB2 = DVGlobalLock( hDIB2 )) )
	{
#ifdef	ADDPROGM
		wsprintf( lpDIB2, "Building NEW DIB with %u colors",
			lpPal3->palNumEntries );
		SetProgMInfo( lpDIB2 );			
#endif	/* ADDPROGM */
		lpbi = (LPBITMAPINFOHEADER) lpDIB2;
		lpbi->biSize = sizeof( BITMAPINFOHEADER );
		lpbi->biWidth = wWid;
		lpbi->biHeight = wHgt;
		lpbi->biPlanes = 1;
		lpbi->biBitCount = 8;
		lpbi->biCompression = BI_RGB;
		lpbi->biSizeImage = 0;
		lpbi->biXPelsPerMeter = 0;
		lpbi->biYPelsPerMeter = 0;
		lpbi->biClrUsed = 0;
		lpbi->biClrImportant = 0;
		lpbi++;
		dv_fmemcpy( lpbi, &lpPal3->palPalEntry[0],
			(sizeof( PALETTEENTRY ) * 256) );
		lpbi = (LPBITMAPINFOHEADER) lpDIB2;
		lpDIB2 = FindDIBBits( lpDIB2 );	/* Target BITMAP site */
		lpDIB1 = FindDIBBits( cinfo->lpDib ); /* Source BITMAP */
		fpSrc = ( PINT8 ) lpDIB1;
		fpDest = ( PINT8 ) lpDIB2;
		lpDIB2 = (LPSTR) lpbi;
		ppe1 = &lpPal2->palPalEntry[0];
		ppe2 = &lpPal3->palPalEntry[0];
		ccnt = lpPal2->palNumEntries;
		for( i = 0; i < wHgt; i++ )
		{
			for( j = 0; j < wWid; j++ )
			{
				c[0] = *fpSrc++;	/* Extract the COLORS ... */
				c[1] = *fpSrc++;
				c[2] = *fpSrc++;
				ind = 0;
				for( wk = 0; wk < ccnt; wk++ )
				{
					if( (c[0] == ppe1[wk].peRed) &&
						 (c[1] == ppe1[wk].peGreen) &&
						 (c[2] == ppe1[wk].peBlue) )
					{
						ind = ppe1[wk].peFlags;
						break;
					}
				}
				*fpDest++ = ind;
			}	/* For WIDTH */
			ind = 0;
			for( ; j < wNWid; j++ )
			{	/* Fill in to END OF DESTINATION SCAN -> LONG Boundary ... */
				*fpDest++ = ind;
			}
			for( k = 0; k < wSWid; k++ )
			{	/* And SKIP SCAN fill -> to LONG Boundary ... */
				fpSrc++;
			}
#ifdef	ADDPROGM
			if( fVeryVerb )
			{
				wsprintf( lpb, "Done row %lu of %lu ...", (i+1), wHgt );
				SetProgMInfo( lpb );
			}
			if( bpct )
				SetProgM( (DWORD) cpct + ((i*bpct) / wHgt), 100 );			
			if( bUsrAbort )
			{
				DVGlobalUnlock( hDIB2 );
				DVGlobalFree( hDIB2 );
				hDIB2 = 0;
				break;
			}
#endif	/* ADDPROGM */
		}	/* For HEIGHT */
		if( hDIB2 )
			DVGlobalUnlock( hDIB2 );
		hRet = hDIB2;
	}
return( hRet );
}

// eof - DvB2g.c
