
/* =========================================================================
 * file2.c
 *
 * ========================================================================= */
#include	"dv.h"	/* Single, all inclusing include ... */
//#include "wjpeglib.h"	/* ONLY module to include this ... ALL calls from here */
//#include	"WJPEG32\WJPEGLIB.h"

#define LM_to_uint(a,b)		((((b)&0xFF) << 8) | ((a)&0xFF))
#define BitSet(byte, bit)	((byte) & (bit))
#define INTERLACE       0x40	/* mask for bit signifying interlaced image */
#define COLORMAPFLAG    0x80	/* mask for bit signifying colormap presence */
#define MAXCOLORMAPSIZE	 256	/* max # of colors in a GIF colormap */
#define GNUMCOLORS	      3	/* # of colors */
#define CM_RED		         0	/* color component numbers */
#define CM_GREEN	         1
#define CM_BLUE		      2
#define MAX_LZW_BITS	     12	/* maximum LZW code size */
#define LZW_TABLE_SIZE	(1<<MAX_LZW_BITS) /* # of possible LZW symbols */

//extern void	GetFPath( LPSTR, LPSTR, LPSTR, LPSTR, LPSTR );
//extern	char    gszDrive[_MAX_DRIVE];          // Drive
//extern	char    gszDir[_MAX_DIR];          // Directory
//extern	char    gszFname[_MAX_FNAME];         // Filename
//extern	char    gszExt[_MAX_EXT];            // Extension

BOOL	fExitLabelF9;
BOOL	fNetScape;
WORD	wLoopCnt;

void	chkgerr( void )
{
	int	i;
	i = 0;
}

DWORD	Gif_Count( LPSTR lps, DWORD ddSz )
{
	PINT8	bhp;
	PINT8	bhp2;
	DWORD	ddOff;
	WORD	GIFwidth, GIFheight;
	WORD	colormaplen, aspectRatio;
	WORD	gmaplen, icnt;
	WORD	is_interlaced, input_code_size;
	BYTE	c;
	WORD	extlabel;
	DWORD	glen, gcnt;
	BOOL	fHdExt;

	icnt = 0;
	fNetScape = FALSE;
	fExitLabelF9 = FALSE;
	fHdExt = FALSE;
	wLoopCnt = 0;
	if( ddSz && (bhp = ( PINT8 ) lps) )
	{
		ddOff = 6;
		if( ddOff > ddSz )
		{
			chkgerr();
			return( icnt );
		}
		if( (bhp[0] != 'G') || (bhp[1] != 'I') || (bhp[2] != 'F') )
		{
			chkgerr();
			return( icnt );
		}
  /* Check for expected version numbers.
   * If unknown version, give warning and try to process anyway;
   * this is per recommendation in GIF89a standard.
   */
		if( (bhp[3] != '8' || bhp[4] != '7' || bhp[5] != 'a') &&
			 (bhp[3] != '8' || bhp[4] != '9' || bhp[5] != 'a') )
		{
			chkgerr();
			return( icnt );
		}
		if( (bhp[3] == '8') && (bhp[4] == '9') && (bhp[5] == 'a') )
			fNetScape++;
		bhp2 = bhp + ddOff;	/* Past the HEADER of 6 */
		ddOff += 7;				/* and bump offset by next header 7 */
		if( ddOff > ddSz )	/* If out of data ... */
		{
			chkgerr();
			return( icnt );
		}

		GIFwidth    = LM_to_uint( bhp2[0], bhp2[1] );	/* Get G WIDTH */
		GIFheight   = LM_to_uint( bhp2[2], bhp2[3] );	/* Get G HEIGHT */
		colormaplen = 2 << ( bhp2[4] & 0x07 );	/* Extract Color Map Lenght */
  /* we ignore the color resolution, sort flag, and background color index */
		aspectRatio = bhp2[6] & 0xFF;	/* Extract aspect ratio ... */
		if( aspectRatio != 0 && aspectRatio != 49 )
		{
			chkgerr();
			return( icnt );
		}

		gmaplen = 0;
	/* Read global colormap if header indicates it is present */
		if( BitSet( bhp2[4], COLORMAPFLAG ) )
		{
			if( colormaplen > MAXCOLORMAPSIZE )
				colormaplen = MAXCOLORMAPSIZE;
			gmaplen = colormaplen;
			ddOff += (colormaplen * GNUMCOLORS);	/* Bump the OFFSET */
			if( ddOff > ddSz )
			{
				chkgerr();
				return( icnt );
			}
		}

		bhp2 = bhp + ddOff;
	/* Scan until we reach start of end of file */
		for( ;; )
		{
			c = *bhp2++;	/* Get BYTE and bump HUGE pointer ... */
			ddOff++;			/* and BUMP the Offset ... */
			if( c == ';' )		/* GIF terminator?? */
			{
				break;	/* That's it folks ... */
			}
			if( c == '!' )
			{
            /* Extension */
				/* Read extension label byte */
				extlabel = *bhp2;
				if( !fHdExt )
				{
					fHdExt = TRUE;
					if( fNetScape && ( extlabel == 0xff ) )
					{
						if( ( bhp2[1] == 11 ) &&
							( bhp2[2] == 'N' ) &&
							( bhp2[3] == 'E' ) &&
							( bhp2[4] == 'T' ) &&
							( bhp2[5] == 'S' ) &&
							( bhp2[6] == 'C' ) &&
							( bhp2[7] == 'A' ) &&
							( bhp2[8] == 'P' ) &&
							( bhp2[9] == 'E' ) &&
							( bhp2[10] == '2' ) &&
							( bhp2[11] == '.' ) &&
							( bhp2[12] == '0' ) &&
							( bhp2[13] == 3 ) &&
							( bhp2[14] == 1 ) )
						{
							fNetScape++;
							wLoopCnt = (WORD) (bhp2[15] + (bhp2[16] << 8));
						}
					}
					else if( extlabel == 0xf9 )
					{
						fExitLabelF9 = TRUE;
					}
				}

				bhp2++;		// Bump PAST "Exit Label"
				ddOff++;	// and ADD this BYTE ...
				glen = 0;
				while( (gcnt = bhp2[glen]) > 0 )
				{
               /* skip */
					glen += gcnt + 1;	/* Bump by SKIP length PLUS Count byte ... */
					if( (ddOff + glen) > ddSz )
					{
						chkgerr();
						return( icnt );
					}
				}

				glen++;	/* Skipped LENGTH + final ZERO Count byte ... */
				ddOff += glen;	/* Skip the BLOCK(s) ... */
				if( ddOff > ddSz )
				{
					chkgerr();
					return( icnt );
				}

				bhp2 = bhp + ddOff;
				continue;
			}

			if( c != ',' )
			{
            /* Not an image separator? */
				continue;	/* Then get NEXT byte ... */
			}

			/* Read and decipher Local Image Descriptor */
			ddOff += 9;	/* Bump offset by 9 bytes ... */
			if( ddOff > ddSz )
			{
				chkgerr();
				return( icnt );
			}

			/* we ignore top/left position info, also sort flag */
			GIFwidth      = LM_to_uint( bhp2[4], bhp2[5] );
			GIFheight     = LM_to_uint( bhp2[6], bhp2[7]);
			is_interlaced = BitSet( bhp2[8], INTERLACE);

    /* Read local colormap if header indicates it is present */
    /* Note: if we wanted to support skipping images, */
    /* we'd need to skip rather than read colormap for ignored images */
			if( BitSet( bhp2[8], COLORMAPFLAG ) ) 
			{
				colormaplen = 2 << ( bhp2[8] & 0x07);
				if( colormaplen > MAXCOLORMAPSIZE )
					colormaplen = MAXCOLORMAPSIZE;
				ddOff += (colormaplen * GNUMCOLORS) ;
				if( ddOff > ddSz )
				{
					chkgerr();
					return( icnt );
				}
			}

			bhp2 = bhp + ddOff;
			input_code_size = *bhp2++; /* get minimum-code-size byte */
			ddOff++;

			if( input_code_size < 2 || input_code_size >= MAX_LZW_BITS )
			{
				chkgerr();
				return( icnt );
			}

			icnt++;	/* Bump to NEXT, and ... */
			glen = 0;

			while( (gcnt = bhp2[glen]) > 0 )
			{
            /* skip */
				glen += gcnt + 1;	/* Bump by SKIP length PLUS Count byte ... */
				if( (ddOff + glen) > ddSz )
				{
					chkgerr();
					return( icnt );
				}
			}

			glen++;	/* Skipped LENGTH + final ZERO Count byte ... */
			ddOff += glen;	/* Skip the BLOCK(s) ... */
			if( ddOff > ddSz )
			{
				chkgerr();
				return( icnt );
			}

			bhp2 = bhp + ddOff;

		}	/* Forever - until break or returned flag FALSE if error */
	}

   return( icnt );

}

// =====================================================
// The Is???? Series.
// Functions to only CHECK the file EXTENT
// =======================================

// Reduce FIXUPS
// =============
void	GetFPath2( LPSTR lpf )
{
	DVGetFPath( lpf, gszDrive, gszDir, gszFname, gszExt );
}

LPSTR	GetGExt( void )
{
	return( &gszExt[0] );
}

BOOL	IsJPEG( LPSTR lpf )
{
	int	i;
	BOOL	flg = FALSE;
	LPSTR	lpe = GetGExt();
	if( lpf && ( i = lstrlen( lpf ) ) && (i >= 4) )
	{
//		_splitpath( pf, szDrive, szDir, szFname, szExt );
		GetFPath2( lpf );
		if( (lpe[0] == '.') &&
			((lpe[1] == 'J') || (lpe[1] == 'j')) &&
			((lpe[2] == 'P') || (lpe[2] == 'p')) &&
			((lpe[3] == 'G') || (lpe[3] == 'g')) )
		{
			flg = TRUE;
		}
	}
	return( flg );
}

BOOL	IsBMP( LPSTR lpf )
{
	int	i;
	BOOL	flg = FALSE;
	LPSTR	lpe = GetGExt();
	if( lpf && ( i = lstrlen( lpf ) ) && (i >= 4) )
	{
//		_splitpath( pf, szDrive, szDir, szFname, szExt );
		GetFPath2( lpf );
		if( (lpe[0] == '.') &&
			((lpe[1] == 'B') || (lpe[1] == 'b')) &&
			((lpe[2] == 'M') || (lpe[2] == 'm')) &&
			((lpe[3] == 'P') || (lpe[3] == 'p')) )
		{
			flg = TRUE;
		}
	}
	return( flg );
}

BOOL	IsGIF( LPSTR lpf )
{
	int	i;
	BOOL	flg = FALSE;
	LPSTR	lpe = GetGExt();
	if( lpf && ( i = lstrlen( lpf ) ) && (i >= 4) )
	{
//		_splitpath( pf, szDrive, szDir, szFname, szExt );
		GetFPath2( lpf );
		if( (lpe[0] == '.') &&
			((lpe[1] == 'G') || (lpe[1] == 'g')) &&
			((lpe[2] == 'I') || (lpe[2] == 'i')) &&
			((lpe[3] == 'F') || (lpe[3] == 'f')) )
		{
			flg = TRUE;
		}
	}
	return( flg );
}

BOOL	IsRLE( LPSTR lpf )
{
	int	i;
	BOOL	flg = FALSE;
	LPSTR	lpe = GetGExt();
	if( lpf && ( i = lstrlen( lpf ) ) && (i >= 4) )
	{
//		_splitpath( pf, szDrive, szDir, szFname, szExt );
		GetFPath2( lpf );
		if( (lpe[0] == '.') &&
			((lpe[1] == 'R') || (lpe[1] == 'r')) &&
			((lpe[2] == 'L') || (lpe[2] == 'l')) &&
			((lpe[3] == 'E') || (lpe[3] == 'e')) )
		{
			flg = TRUE;
		}
	}
	return( flg );
}

BOOL	IsPPM( LPSTR lpf )
{
	int	i;
	BOOL	flg = FALSE;
	LPSTR	lpe = GetGExt();
	if( lpf && ( i = lstrlen( lpf ) ) && (i >= 4) )
	{
//		_splitpath( pf, szDrive, szDir, szFname, szExt );
		GetFPath2( lpf );
		if( (lpe[0] == '.') &&
			((lpe[1] == 'P') || (lpe[1] == 'p')) &&
			((lpe[2] == 'P') || (lpe[2] == 'p')) &&
			((lpe[3] == 'M') || (lpe[3] == 'm')) )
		{
			flg = TRUE;
		}
	}
	return( flg );
}

BOOL	IsTARGA( LPSTR lpf )
{
	int	i;
	BOOL	flg = FALSE;
	LPSTR	lpe = GetGExt();
	if( lpf && ( i = lstrlen( lpf ) ) && (i >= 4) )
	{
//		_splitpath( pf, szDrive, szDir, szFname, szExt );
		GetFPath2( lpf );
		if( (lpe[0] == '.') &&
			((lpe[1] == 'T') || (lpe[1] == 't')) &&
			((lpe[2] == 'G') || (lpe[2] == 'g')) &&
			((lpe[3] == 'A') || (lpe[3] == 'a')) )
		{
			flg = TRUE;
		}
	}
	return( flg );
}


// =====================================================
// eof - File2.c
