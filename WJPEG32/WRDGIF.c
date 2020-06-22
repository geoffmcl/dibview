
/*
 * wrdgif.c
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains routines to read input images in GIF format.
 *
 * These routines may need modification for non-Unix environments or
 * specialized applications.  As they stand, they assume input from
 * an ordinary stdio stream.  They further assume that reading begins
 * at the start of the file; gif_input_init may need work if the
 * user interface has already read some data (e.g., to determine that
 * the file is indeed GIF format).
 *
 * These routines are invoked via the methods gif_get_input_row
 * and gif_input_init/term.
 */

/*
 * This code is loosely based on giftoppm from the PBMPLUS distribution
 * of Feb. 1991.  That file contains the following copyright notice:
 * +-------------------------------------------------------------------+
 * | Copyright 1990, David Koblas.                                     |
 * |   Permission to use, copy, modify, and distribute this software   |
 * |   and its documentation for any purpose and without fee is hereby |
 * |   granted, provided that the above copyright notice appear in all |
 * |   copies and that both that copyright notice and this permission  |
 * |   notice appear in supporting documentation.  This software is    |
 * |   provided "as is" without express or implied warranty.           |
 * +-------------------------------------------------------------------+
 *
 * We are also required to state that
 *    "The Graphics Interchange Format(c) is the Copyright property of
 *    CompuServe Incorporated. GIF(sm) is a Service Mark property of
 *    CompuServe Incorporated."
 */

#include "winclude.h"
#define	DIRGIFBMP	/* Apply a DIRECT conversion ... */
#ifdef	PROBGOBJ

//#pragma message( "Still to solve problem of returning GLOBAL MEMORY Object" )
// Solved by providing a SIZE, which return information about the
// GIF Image file, and a ???NBMP call. Add ???NBMPX on WJPEG5
#ifdef	WIN32
#pragma message( "Using 32-bit handles to HEAP memory. (MMI/O)" )
#else	// !WIN32
// else us far memory allocation all the time - LARGE like
#pragma message( "Using WORD handles to 20-bit DOS (real) mem." )
#endif	// WIN32 y/n

#endif
#ifdef GIF_SUPPORTED

#define	FORCE256

#define	MAXCOLORMAPSIZE	256	/* max # of colors in a GIF colormap */
#define GNUMCOLORS	3	/* # of colors */
#define CM_RED		0	/* color component numbers */
#define CM_GREEN	1
#define CM_BLUE		2

static JSAMPARRAY colormap;	/* the colormap to use */
/* colormap[i][j] = value of i'th color component for pixel value j */

#define	MAX_LZW_BITS	12	/* maximum LZW code size */
#define LZW_TABLE_SIZE	(1<<MAX_LZW_BITS) /* # of possible LZW symbols */

/* Macros for extracting header data --- note we assume chars may be signed */

#define LM_to_uint(a,b)		((((b)&0xFF) << 8) | ((a)&0xFF))

#define BitSet(byte, bit)	((byte) & (bit))
#define INTERLACE	0x40	/* mask for bit signifying interlaced image */
#define COLORMAPFLAG	0x80	/* mask for bit signifying colormap presence */

#define	ReadOK(file,buffer,len)	(JFREAD(file,buffer,len) == ((size_t) (len)))

extern	void jselwnul( compress_info_ptr );
/* Static vars for GetCode and LZWReadByte */

static char code_buf[256+4];	/* current input data block */
static int last_byte;		/* # of bytes in code_buf */
static int last_bit;		/* # of bits in code_buf */
static int cur_bit;		/* next bit index to read */
static boolean out_of_blocks;	/* TRUE if hit terminator data block */

static int input_code_size;	/* codesize given in GIF file */
static int clear_code,end_code; /* values for Clear and End codes */

static int code_size;		/* current actual code size */
static int limit_code;		/* 2^code_size */
static int max_code;		/* first unused code value */
static boolean first_time;	/* flags first call to LZWReadByte */

/* LZW decompression tables:
 *   symbol_head[K] = prefix symbol of any LZW symbol K (0..LZW_TABLE_SIZE-1)
 *   symbol_tail[K] = suffix byte   of any LZW symbol K (0..LZW_TABLE_SIZE-1)
 * Note that entries 0..end_code of the above tables are not used,
 * since those symbols represent raw bytes or special codes.
 *
 * The stack represents the not-yet-used expansion of the last LZW symbol.
 * In the worst case, a symbol could expand to as many bytes as there are
 * LZW symbols, so we allocate LZW_TABLE_SIZE bytes for the stack.
 * (This is conservative since that number includes the raw-byte symbols.)
 *
 * The tables are allocated from FAR heap space since they would use up
 * rather a lot of the near data space in a PC.
 */

static UINT16 MLPTR symbol_head; /* => table of prefix symbols */
static UINT8  MLPTR symbol_tail; /* => table of suffix bytes */
static UINT8  MLPTR symbol_stack; /* stack for symbol expansions */
static UINT8  MLPTR sp;		/* stack pointer */

/* Static state for interlaced image processing */

static boolean is_interlaced;	/* TRUE if have interlaced image */
static big_sarray_ptr interlaced_image;	/* full image in interlaced order */
static long cur_row_number;	/* need to know actual row number */
static long pass2_offset;	/* # of pixel rows in pass 1 */
static long pass3_offset;	/* # of pixel rows in passes 1&2 */
static long pass4_offset;	/* # of pixel rows in passes 1,2,3 */

#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))

extern	HGLOBAL	hgDIB;

#ifdef	WJPEG4
// Keep a copy of the GLOBAL GIF Colour map,
// now written as a Windows BITMAP RGBQUAD array.
// =============================================
static	LPRGBQUAD	lpRGBMap = 0;

#ifdef	ADDOLDTO
extern	LPSTR lpOutFile;
extern	FILETYPE gout_open( char MLPTR );
#endif	// ADDOLDTO

#else	// !WJPEG4	// NOTE: Older file interface RETIRED - April, 1997

// Should NOT be used
extern	FILETYPE out_open( char MLPTR, char MLPTR );
extern	LPSTR lpOutFile;

char	*pRTempFil = "TEMPGIF5.BMP";
char	*pRTempMod = "rw";

#endif	// !WJPEG4

BOOL	fWrtTemp = TRUE;	// MOVE data to supplied buffer
// NOTE: Old interface WROTE a temporary file!!!

LPRGBQUAD	rgbColors = 0;
static HGLOBAL	hgColors = 0;
static LPSTR		lpColors = 0;
static HGLOBAL	hgData = 0;
static LPSTR		lpData = 0;
static DWORD	BMPCols;
static WORD	BMPCCnt;
static WORD	BMPBits;
BOOL	fUseWin = TRUE;
BOOL	fInvertI = TRUE;
DWORD	dwRow32;	// BYTES in a ROW (Rounded to 32-bits)
DWORD	dwdds;
DWORD	ddDCnt;
DWORD	ddTCnt;
DWORD	ddRCnt;
WORD	wGetICnt = 0;	/* ZERO = Image Num. 1 ... */

#endif	// _INC_WINDOWS & DIRGIFBMP


UINT16 GIFwidth, GIFheight;		/* image dimensions */

/* Forward declarations */
METHODDEF void load_interlaced_image PP((compress_info_ptr cinfo, JSAMPARRAY pixel_row));
METHODDEF void get_interlaced_row PP((compress_info_ptr cinfo, JSAMPARRAY pixel_row));
int SetBMPSizes( int );
void	ResetBMPSizes( void );
void SkipColorMap( compress_info_ptr, int );

METHODDEF void
do_nothing (compress_info_ptr ci)
{
}
// Some CONSTANTS for WINDOWS BITMAPS
// ==================================
WORD	GetBitCount( DWORD clen )
{
	UINT16	bits;
	if( clen > MAXCOLORMAPSIZE )
	{
		bits = 8;
	}
#ifndef	FORCE256
	else if( clen <= 2 )
	{
		bits = 1;
	}
	else if( clen <= 16 )
	{
		bits = 4;
	}
#endif	// FORCE256
	else if( clen <= 256 )
	{
		bits = 8;
	}
	else
	{
		bits = 24;	// CAN NOT GET HERE
		// since MAXCOLORMAPSIZE = 256!
	}
	return bits;
}

DWORD	GetDataSize( DWORD Gwidth, DWORD Gheight, WORD BBits )
{
	DWORD	dw;
	switch( BBits )	// This has been set to
	{	// 1, 4, 8 or 24 - Establish the SIZE for EACH
	case 1:		// Monochrome
		dw = Gwidth / 8;	// 8-bits packed into BYTES
		if( Gwidth % 8 )
			dw++;
		break;
	case 4:
		dw = Gwidth / 2;	// 4-bits packed into BYTES
		if( Gwidth % 2 )
			dw++;
		break;
	case 8:
		dw = Gwidth;
		break;
	case 24:
		dw = Gwidth * 3;
		break;
	}
	// We have BYTE needed per ROW
	if( dw % 4 )	// Is it already on 32-bit boundary?
		dwRow32 = (((dw / 4) + 1) * 4);	// NO, round to 32-bits
	else
		dwRow32 = dw;

	return( dwRow32 * Gheight );
}

LOCAL int
ReadByte( compress_info_ptr cinfo )
{	/* Read next byte from GIF file */
	register FILETYPE infile = cinfo->input_file;
	int c;

	if( (c = getc(infile)) == EOF )
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
#else
    ERREXIT(cinfo->emethods, "Premature EOF in GIF file");
#endif
	}
return c;
}


/* Read a GIF data block, which has a leading count byte */
/* A zero-length block marks the end of a data block sequence */
LOCAL int
GetDataBlock( compress_info_ptr cinfo, char MLPTR buf )
{
	int count;
	count = ReadByte(cinfo);
	if( count > 0 )
	{
		if( !ReadOK(cinfo->input_file, buf, count) )
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
#else
			ERREXIT(cinfo->emethods, "Premature EOF in GIF file");
#endif
		}
  }
return( count );
}

/* Skip a series of data blocks, until a block
   terminator is found */
LOCAL void
SkipDataBlocks (compress_info_ptr cinfo)
{
	char buf[256];
	while( GetDataBlock( cinfo, buf ) > 0 )
	{	/* skip */
#ifdef	_INC_WINDOWS
		if( bErrFlag )
			break;
#endif
	}
}


/* (Re)initialize LZW state; shared code for startup
   and Clear processing */
LOCAL void
ReInitLZW (void)
{
	code_size = input_code_size+1;
	limit_code = clear_code << 1;	/* 2^code_size */
	max_code = clear_code + 2;	/* first unused code value */
	sp = symbol_stack;		/* init stack to empty */
}


/* Initialize for a series of LZWReadByte
   (and hence GetCode) calls */
LOCAL void
InitLZWCode (void)
{
	/* GetCode initialization */
	last_byte = 2;		/* make safe to "recopy last two bytes" */
	last_bit = 0;			/* nothing in the buffer */
	cur_bit = 0;			/* force buffer load on first call */
	out_of_blocks = FALSE;

	/* LZWReadByte initialization */
	clear_code = 1 << input_code_size; /* compute special code values */
	end_code = clear_code + 1;	/* note that these do not change */
	first_time = TRUE;
	ReInitLZW();
}


/* Fetch the next code_size bits from the GIF data */
/* We assume code_size is less than 16 */
LOCAL int
GetCode (compress_info_ptr cinfo)
{
  register INT32 accum;
  int offs, ret, count;

  if ( (cur_bit+code_size) > last_bit) {
    /* Time to reload the buffer */
    if( out_of_blocks )
	 {
#ifndef	_NOWARNMESS
      WARNMS(cinfo->emethods, "Ran out of GIF bits");
#endif
      return end_code;		/* fake something useful */
    }
    /* preserve last two bytes of what we have -- assume code_size <= 16 */
    code_buf[0] = code_buf[last_byte-2];
    code_buf[1] = code_buf[last_byte-1];
    /* Load more bytes; set flag if we reach the terminator block */
    if( (count = GetDataBlock(cinfo, &code_buf[2])) == 0 )
	 {
      out_of_blocks = TRUE;
#ifndef	_NOWARNMESS
      WARNMS(cinfo->emethods, "Ran out of GIF bits");
#endif
      return end_code;		/* fake something useful */
    }
    /* Reset counters */
    cur_bit = (cur_bit - last_bit) + 16;
    last_byte = 2 + count;
    last_bit = last_byte * 8;
  }

  /* Form up next 24 bits in accum */
  offs = cur_bit >> 3;		/* byte containing cur_bit */
#ifdef CHAR_IS_UNSIGNED
  accum = code_buf[offs+2];
  accum <<= 8;
  accum |= code_buf[offs+1];
  accum <<= 8;
  accum |= code_buf[offs];
#else
  accum = code_buf[offs+2] & 0xFF;
  accum <<= 8;
  accum |= code_buf[offs+1] & 0xFF;
  accum <<= 8;
  accum |= code_buf[offs] & 0xFF;
#endif

  /* Right-align cur_bit in accum, then mask off desired number of bits */
  accum >>= (cur_bit & 7);
  ret = ((int) accum) & ((1 << code_size) - 1);
  
  cur_bit += code_size;
  return ret;
}


LOCAL int
LZWReadByte (compress_info_ptr cinfo)
/* Read an LZW-compressed byte */
{
  static int oldcode;		/* previous LZW symbol */
  static int firstcode;		/* first byte of oldcode's expansion */
  register int code;		/* current working code */
  int incode;			/* saves actual input code */

  /* First time, just eat the expected Clear code(s) and return next code, */
  /* which is expected to be a raw byte. */
  if (first_time) {
    first_time = FALSE;
    code = clear_code;		/* enables sharing code with Clear case */
  } else {

    /* If any codes are stacked from a previously read symbol, return them */
    if (sp > symbol_stack)
      return (int) *(--sp);

    /* Time to read a new symbol */
    code = GetCode(cinfo);

  }

  if (code == clear_code) {
    /* Reinit static state, swallow any extra Clear codes, and */
    /* return next code, which is expected to be a raw byte. */
    ReInitLZW();
    do {
      code = GetCode(cinfo);
    } while (code == clear_code);
    if (code > clear_code) 
	 {	/* make sure it is a raw byte */
#ifndef	_NOWARNMESS
      WARNMS(cinfo->emethods, "Corrupt data in GIF file");
#endif
      code = 0;			/* use something valid */
    }
    firstcode = oldcode = code;	/* make firstcode, oldcode valid! */
    return code;
  }

	if( code == end_code )
	{
    /* Skip the rest of the image, unless GetCode already read terminator */
    if (! out_of_blocks) {
      SkipDataBlocks(cinfo);
      out_of_blocks = TRUE;
    }
    /* Complain that there's not enough data */
#ifndef	_NOWARNMESS
    WARNMS(cinfo->emethods, "Premature end of GIF image");
#endif
    /* Pad data with 0's */
    return 0;			/* fake something usable */
  }

  /* Got normal raw byte or LZW symbol */
  incode = code;		/* save for a moment */
  
  if (code >= max_code) {	/* special case for not-yet-defined symbol */
    /* code == max_code is OK; anything bigger is bad data */
    if (code > max_code) 
	 {
#ifndef	_NOWARNMESS
      WARNMS(cinfo->emethods, "Corrupt data in GIF file");
#endif
      incode = 0;		/* prevent creation of loops in symbol table */
    }
    *sp++ = (UINT8) firstcode;	/* it will be defined as oldcode/firstcode */
    code = oldcode;
  }

  /* If it's a symbol, expand it into the stack */
  while (code >= clear_code) {
    *sp++ = symbol_tail[code];	/* tail of symbol: a simple byte value */
    code = symbol_head[code];	/* head of symbol: another LZW symbol */
  }
  /* At this point code just represents a raw byte */
  firstcode = code;		/* save for possible future use */

  /* If there's room in table, */
  if ((code = max_code) < LZW_TABLE_SIZE) {
    /* Define a new symbol = prev sym + head of this sym's expansion */
    symbol_head[code] = oldcode;
    symbol_tail[code] = (UINT8) firstcode;
    max_code++;
    /* Is it time to increase code_size? */
    if ((max_code >= limit_code) && (code_size < MAX_LZW_BITS)) {
      code_size++;
      limit_code <<= 1;		/* keep equal to 2^code_size */
    }
  }
  
  oldcode = incode;		/* save last input symbol for future use */
  return firstcode;		/* return first byte of symbol's expansion */
}


/* Read a GIF colormap */
LOCAL void
ReadColorMap (compress_info_ptr cinfo, int cmaplen, JSAMPARRAY cmap)
{
	int i;
#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
	BYTE	b1, b2, b3;
	LPRGBQUAD	lpq;

	if( fUseWin )
	{
		rgbColors = 0;
		if( hgColors )
		{
			rgbColors = (LPRGBQUAD) JGlobalLock( hgColors );
			lpq = rgbColors;
		}
	}
	for( i = 0; i < cmaplen; i ++ )
	{
		b1 = (JSAMPLE) ReadByte(cinfo);
		b2 = (JSAMPLE) ReadByte(cinfo);
		b3 = (JSAMPLE) ReadByte(cinfo);

		cmap[CM_RED][i]   = b1;
		cmap[CM_GREEN][i] = b2;
		cmap[CM_BLUE][i]  = b3;

		if( rgbColors )
		{
			lpq->rgbBlue = b3;
			lpq->rgbGreen = b2;
			lpq->rgbRed = b1;
			lpq->rgbReserved = 0;
			lpq++;
		}
	}

	if( fUseWin )
	{
		if( hgColors && rgbColors )
		{
			if( (WORD) cmaplen < BMPCCnt )
			{
				b1 = b2 = b3 = 0;
				for( ; (WORD) i < BMPCCnt; i++ )
				{	/* Fill in the balance of the 8-bit precision color table */
					lpq->rgbBlue = b3;
    				lpq->rgbGreen = b2;
					lpq->rgbRed = b1;
    				lpq->rgbReserved = 0;
					lpq++;
				}
			}
			JGlobalUnlock( hgColors );
			rgbColors = 0;
		}
	}
#else	// !( _INC_WINDOWS & DIRGIFBMP )

	for (i = 0; i < cmaplen; i++)
	{
		cmap[CM_RED][i]   = (JSAMPLE) ReadByte(cinfo);
		cmap[CM_GREEN][i] = (JSAMPLE) ReadByte(cinfo);
		cmap[CM_BLUE][i]  = (JSAMPLE) ReadByte(cinfo);
	}
#endif	// ( _INC_WINDOWS & DIRGIFBMP ) y/n

}

#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
BOOL	fNulFil = FALSE;

// Read GIF Global (or Local) Colour information,
// and convert it to a Windows BITMAP Colour QUADs
// RGB/COLOREF by BYTE index value.
// ie a 256-Colour MAP only
LOCAL void
CopyColorMap (compress_info_ptr cinfo, int cmaplen,
			  LPRGBQUAD lpRGB,
			  JSAMPARRAY cmap )
{
	int i;
	//JSAMPARRAY cmap;
	//cmap = (JSAMPARRAY) lpRGB;

	BYTE	b1, b2, b3;
	LPRGBQUAD	lpq;
	LPBYTE	lpC1, lpC2, lpC3;

	lpq = 0;
	if( lpRGB )
	{
		lpq = lpRGB;
	}

	lpC1 = cmap[CM_RED];
	lpC2 = cmap[CM_GREEN];
	lpC3 = cmap[CM_BLUE];

	for( i = 0; i < cmaplen; i ++ )
	{
		b1 = (JSAMPLE) ReadByte(cinfo);
		b2 = (JSAMPLE) ReadByte(cinfo);
		b3 = (JSAMPLE) ReadByte(cinfo);

		if( lpC1 )
			lpC1[i]   = b1;	// GIF Red
		if( lpC2 )
			lpC2[i]   = b2;	// GIF Grean
		if( lpC3 )
			lpC3[i]   = b3;	// GIF Blue
		if( lpq )
		{
			lpq->rgbBlue = b3;
			lpq->rgbGreen = b2;
			lpq->rgbRed = b1;
			lpq->rgbReserved = 0;
			lpq++;
		}
	}

	if( fNulFil )
	{
		if( (WORD) cmaplen < BMPCCnt )
		{
			b1 = b2 = b3 = 0;
			for( ; (WORD) i < BMPCCnt; i++ )
			{	/* Fill in the balance of the 8-bit precision color table */
				lpq->rgbBlue = b3;
   				lpq->rgbGreen = b2;
				lpq->rgbRed = b1;
   				lpq->rgbReserved = 0;
				lpq++;
			}
		}
	}
}

#endif	// ( _INC_WINDOWS & DIRGIFBMP )


/* Process an extension block */
/* Currently we ignore 'em all */
LOCAL int
DoExtension (compress_info_ptr cinfo)
{
	int extlabel;

	/* Read extension label byte */
	extlabel = ReadByte(cinfo);
#ifndef	_INC_WINDOWS
	TRACEMS1(cinfo->emethods, 1,
		"Ignoring GIF extension block of type 0x%02x",
		extlabel );
#endif	// _INC_WINDOWS
	/* Skip the data block(s) associated with the extension */
	SkipDataBlocks( cinfo );

	return extlabel;
}


/*
 * Read the file header; return image size and component count.
 */

METHODDEF boolean
gif_input_init (compress_info_ptr cinfo)
{
	char hdrbuf[10];		/* workspace for reading control blocks */
	int cmaplen, aspectRatio, gmaplen;
	int c;

#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
	ResetBMPSizes();
#endif
	gmaplen = 0;

	/* Allocate space to store the colormap */
	colormap = (*cinfo->emethods->alloc_small_sarray)
		((long) MAXCOLORMAPSIZE, (long) GNUMCOLORS);

	/* Read and verify GIF Header */
	if( !ReadOK(cinfo->input_file, hdrbuf, 6) )
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_GIFNOT );	/* "Not a GIF file" */
		return( bErrFlag );
#else
		ERREXIT(cinfo->emethods, "Not a GIF file");
#endif
	}

	if (hdrbuf[0] != 'G' || hdrbuf[1] != 'I' || hdrbuf[2] != 'F')
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_GIFNOT );
		return( bErrFlag );
#else
		ERREXIT(cinfo->emethods, "Not a GIF file");
#endif
	}
	/* Check for expected version numbers.
	 * If unknown version, give warning and try to process anyway;
	 * this is per recommendation in GIF89a standard.
	 */
	if( (hdrbuf[3] != '8' || hdrbuf[4] != '7' || hdrbuf[5] != 'a') &&
		(hdrbuf[3] != '8' || hdrbuf[4] != '9' || hdrbuf[5] != 'a') )
	{
#ifndef	_INC_WINDOWS
		TRACEMS3(cinfo->emethods, 1,
			"Warning: unexpected GIF version number '%c%c%c'",
			hdrbuf[3], hdrbuf[4], hdrbuf[5]);
#endif	// _INC_WINDOWS
	}

	/* Read and decipher Logical Screen Descriptor */
	if( !ReadOK(cinfo->input_file, hdrbuf, 7) )
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
		return( bErrFlag );
#else
		ERREXIT(cinfo->emethods, "Premature EOF in GIF file");
#endif
	}

	GIFwidth = LM_to_uint(hdrbuf[0],hdrbuf[1]);
	GIFheight = LM_to_uint(hdrbuf[2],hdrbuf[3]);
	cmaplen = 2 << (hdrbuf[4] & 0x07);

	/* we ignore the color resolution, sort flag,
	   and background color index */
	aspectRatio = hdrbuf[6] & 0xFF;
	if( aspectRatio != 0 && aspectRatio != 49 )
	{
#ifndef	_INC_WINDOWS
		TRACEMS(cinfo->emethods, 1,
			"Warning: nonsquare pixels in input");
#endif
	}

	/* Read global colormap if header indicates it is present */
	if( BitSet(hdrbuf[4], COLORMAPFLAG) )
	{
#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
		SetBMPSizes( cmaplen );
		gmaplen = cmaplen;
		if( BMPCols )
			hgColors = JGlobalAlloc( GHND, BMPCols );
		else
			hgColors = 0;
#endif
		ReadColorMap( cinfo, cmaplen, colormap );
	}

	/* Scan until we reach start of desired image.
	 * We don't currently support skipping images,
	 * but could add it easily.
	 */
	for( ;; )
	{
		c = ReadByte( cinfo );

		if( c == ';' )		/* GIF terminator?? */
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_GIFCNT ); /* "Too few images in GIF file" */
			return( bErrFlag );
#else
			ERREXIT(cinfo->emethods, "Too few images in GIF file");
#endif
		}
		if( c == '!' )
		{	/* Extension */
			DoExtension( cinfo );
			continue;
		}

		if( c != ',' )
		{	/* Not an image separator? */
#ifndef	_INC_WINDOWS
			TRACEMS1(cinfo->emethods, 1, "Bogus input char 0x%02x, ignoring", c);
#endif	// _INC_WINDOWS
			continue;
		}

		/* Read and decipher Local Image Descriptor */
		if( !ReadOK(cinfo->input_file, hdrbuf, 9) )
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
			return( bErrFlag );
#else	// !_INC_WINDOWS
			ERREXIT(cinfo->emethods, "Premature EOF in GIF file");
#endif	// _INC-WINDOWS y/n
		}

		/* we ignore top/left position info, also sort flag */
		GIFwidth = LM_to_uint(hdrbuf[4],hdrbuf[5]);
		GIFheight = LM_to_uint(hdrbuf[6],hdrbuf[7]);
		is_interlaced = BitSet(hdrbuf[8], INTERLACE);

		/* Read local colormap if header indicates it is present */
		/* Note: if we wanted to support skipping images, */
		/* we'd need to skip rather than read colormap for
		   ignored images */
		if( BitSet( hdrbuf[8], COLORMAPFLAG ) )
		{
			cmaplen = 2 << (hdrbuf[8] & 0x07);
#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
			SetBMPSizes( cmaplen );
			if( hgColors )	/* If already a GLOBAL map ... */
				JGlobalFree( hgColors );
			if( BMPCols )
				hgColors = JGlobalAlloc( GHND, BMPCols );
			else
				hgColors = 0;
#endif	// _INCWINDOWS & DIRGIFBMP

			ReadColorMap( cinfo, cmaplen, colormap );
		}

		/* get minimum-code-size byte */
		input_code_size = ReadByte( cinfo );
		if( ( input_code_size < 2 ) ||
			( input_code_size >= MAX_LZW_BITS ) )
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_GIFCS ); /* "Bogus codesize %d, input_code_size */
			return( bErrFlag );
#else	// !_INC_WINDOWS
			ERREXIT1(cinfo->emethods, "Bogus codesize %d", input_code_size);
#endif	// _INC_WINDOWS y/n
		}

		/* Reached desired image, so break out of loop */
		/* If we wanted to skip this image, */
		/* we'd call SkipDataBlocks and then continue the loop */

		break;
	}

	/* Prepare to read selected image: first 
		initialize LZW decompressor */
	symbol_head = (UINT16 FAR *) (*cinfo->emethods->alloc_medium)
		(LZW_TABLE_SIZE * SIZEOF(UINT16));

	symbol_tail = (UINT8 FAR *) (*cinfo->emethods->alloc_medium)
		(LZW_TABLE_SIZE * SIZEOF(UINT8));

	symbol_stack = (UINT8 FAR *) (*cinfo->emethods->alloc_medium)
		(LZW_TABLE_SIZE * SIZEOF(UINT8));

	InitLZWCode();

	/*
	 * If image is interlaced, we read it into a full-size
	 * sample array, decompressing as we go; then
	 * gif_get_input_row selects rows from the
	 * sample array in the proper order.
	 */

	if( is_interlaced )
	{
		/* We request the big array now, but can't access it until the pipeline
		 * controller causes all the big arrays to be allocated.  Hence, the
		 * actual work of reading the image is postponed until the first call
		 * of gif_get_input_row.
		 */
		interlaced_image = (*cinfo->emethods->request_big_sarray)
			((long) GIFwidth, (long) GIFheight, 1L);

		cinfo->methods->get_input_row = load_interlaced_image;

		cinfo->total_passes++;	/* count file reading as separate pass */

	}

	/* Return info about the image. */
	cinfo->input_components = GNUMCOLORS;
	cinfo->in_color_space = CS_RGB;
	cinfo->image_width = GIFwidth;
	cinfo->image_height = GIFheight;
	cinfo->data_precision = 8;	/* always, even if 12-bit JSAMPLEs */
#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
	ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
	hgData = JGlobalAlloc( GMEM_SHARE, /* Get a FIXED SHAREable memory handle */
		(sizeof( BITMAPINFOHEADER ) + BMPCols + ddRCnt) );
	ddDCnt = 0;
	ddTCnt = 0;
#endif	// _INC_WINDOWS & DIRGIFBMP

#ifndef	_INC_WINDOWS
	TRACEMS3(cinfo->emethods, 1, "%ux%ux%d GIF image",
		(unsigned int) GIFwidth,
		(unsigned int) GIFheight,
		wBMcolormaplen );
#endif	// !_INC_WINDOWS

	return( bErrFlag );

}

#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))

METHODDEF boolean
gif_input_initn( compress_info_ptr cinfo )
{
	char hdrbuf[10];		/* workspace for reading control blocks */
	int cmaplen, aspectRatio, gmaplen;
	int c;
	WORD	icnt;

	ResetBMPSizes();

	/* Allocate space to store the colormap */

	colormap = (*cinfo->emethods->alloc_small_sarray)
		((long) MAXCOLORMAPSIZE,
		(long) GNUMCOLORS);

	cinfo->pcolormap = colormap;

	/* Read and verify GIF Header */
	if( !ReadOK(cinfo->input_file, hdrbuf, 6) )
	{
		ERRFLAG( BAD_GIFNOT );	/* "Not a GIF file" */
		return( bErrFlag );
	}
	if (hdrbuf[0] != 'G' || hdrbuf[1] != 'I' || hdrbuf[2] != 'F')
	{
		ERRFLAG( BAD_GIFNOT );
		return( bErrFlag );
	}

	/* Check for expected version numbers.
	 * If unknown version, give warning and try to process anyway;
	 * this is per recommendation in GIF89a standard.
	 */
	if( (hdrbuf[3] != '8' || hdrbuf[4] != '7' || hdrbuf[5] != 'a') &&
		(hdrbuf[3] != '8' || hdrbuf[4] != '9' || hdrbuf[5] != 'a') )
	{
#ifndef	_INC_WINDOWS
		TRACEMS3(cinfo->emethods, 1,
			"Warning: unexpected GIF version number '%c%c%c'",
			hdrbuf[3], hdrbuf[4], hdrbuf[5]);
#endif	// !_INC_WINDOWS
	}

	/* Read and decipher Logical Screen Descriptor */
	if( !ReadOK(cinfo->input_file, hdrbuf, 7) )
	{
		ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
		return( bErrFlag );
	}

	GIFwidth = LM_to_uint(hdrbuf[0],hdrbuf[1]);
	GIFheight = LM_to_uint(hdrbuf[2],hdrbuf[3]);

	cmaplen = 2 << (hdrbuf[4] & 0x07);

	/* we ignore the color resolution, sort flag,
	   and background color index */

	aspectRatio = hdrbuf[6] & 0xFF;
//	if( aspectRatio != 0 && aspectRatio != 49 )
//	{
//	}

	gmaplen = 0;
	/* Read global colormap if header indicates it is present */
	if( BitSet(hdrbuf[4], COLORMAPFLAG) )
	{
		SetBMPSizes( cmaplen );
		gmaplen = cmaplen;
		if( BMPCols )
			hgColors = JGlobalAlloc( GHND, BMPCols );
		else
			hgColors = 0;
		ReadColorMap(cinfo, cmaplen, colormap);
	}

	// Scan until we reach start of desired image.
	icnt = 0;

	for( ; ; )
	{

		c = ReadByte( cinfo );

		if( c == ';' )		/* GIF terminator?? */
		{
			ERRFLAG( BAD_GIFCNT ); /* "Too few images in GIF file" */
			return( bErrFlag );
		}

		if( c == '!' )
		{	/* Extension */
			DoExtension( cinfo );
			continue;
		}
    
		if( c != ',' )
		{		/* Not an image separator? */
			continue;
		}

		/* Read and decipher Local Image Descriptor */
		if( !ReadOK(cinfo->input_file, hdrbuf, 9) )
		{
			ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
			return( bErrFlag );
		}

		/* we ignore top/left position info, also sort flag */
		GIFwidth = LM_to_uint(hdrbuf[4],hdrbuf[5]);
		GIFheight = LM_to_uint(hdrbuf[6],hdrbuf[7]);

		is_interlaced = BitSet(hdrbuf[8], INTERLACE);

		/* Read local colormap if header indicates it is present */
		/* Note: if we skip it if not ours */
		if( BitSet(hdrbuf[8], COLORMAPFLAG)) 
		{

			cmaplen = 2 << (hdrbuf[8] & 0x07);
			SetBMPSizes( cmaplen );
			if( icnt < wGetICnt )
			{	/* Not reached the DESIRED colormap yet ... */
				SkipColorMap( cinfo, cmaplen );
			}
			else
			{
				if( hgColors )	/* If already GOT a GLOBAL, then discard it ... */
					JGlobalFree( hgColors );
				if( BMPCols )
					hgColors = JGlobalAlloc( GHND, BMPCols );
				else
					hgColors = 0;

				ReadColorMap( cinfo, cmaplen, colormap );
			}
		}


		input_code_size = ReadByte(cinfo); /* get minimum-code-size byte */
		if( ( input_code_size < 2 ) ||
			( input_code_size >= MAX_LZW_BITS ) )
		{
			ERRFLAG( BAD_GIFCS ); /* "Bogus codesize %d, input_code_size */
			return( bErrFlag );
		}

		/* If we reached desired image, break out of loop */
		if( icnt >= wGetICnt )	/* If we reached the desired image ... */
			break;	/* FOUND IT - Do the conversion ... */

		icnt++;	/* Bump to NEXT, and ... */
		SkipDataBlocks( cinfo );	/* Skip this data ... */

	}	// Forever!!!

	/* Prepare to read selected image:
	   first initialize LZW decompressor */

	symbol_head = (UINT16 FAR *) (*cinfo->emethods->alloc_medium)
		(LZW_TABLE_SIZE * SIZEOF(UINT16));
	symbol_tail = (UINT8 FAR *) (*cinfo->emethods->alloc_medium)
		(LZW_TABLE_SIZE * SIZEOF(UINT8));
	symbol_stack = (UINT8 FAR *) (*cinfo->emethods->alloc_medium)
		(LZW_TABLE_SIZE * SIZEOF(UINT8));

	InitLZWCode();

	/*
	 * If image is interlaced, we read it into a full-size sample array,
	 * decompressing as we go; then gif_get_input_row selects rows from the
	 * sample array in the proper order.
	 */

	if( is_interlaced )
	{
		/* We request the big array now, but can't access it until the pipeline
		 * controller causes all the big arrays to be allocated.  Hence, the
		 * actual work of reading the image is postponed until the first call
		 * of gif_get_input_row.
		 */

		interlaced_image = (*cinfo->emethods->request_big_sarray)
			((long) GIFwidth, (long) GIFheight, 1L);

		cinfo->methods->get_input_row = load_interlaced_image;

		cinfo->total_passes++;	/* count file reading as separate pass */

	}


	/* Return info about the image. */
	cinfo->input_components = GNUMCOLORS;
	cinfo->in_color_space = CS_RGB;
	cinfo->image_width = GIFwidth;
	cinfo->image_height = GIFheight;
	cinfo->data_precision = 8;	/* always, even if 12-bit JSAMPLEs */
	ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
	hgData = JGlobalAlloc( GMEM_SHARE, /* Get a FIXED SHAREable memory handle */
		(sizeof( BITMAPINFOHEADER ) + BMPCols + ddRCnt) );

	ddDCnt = 0;
	ddTCnt = 0;

	return( bErrFlag );

}

METHODDEF void
gif_background_index(compress_info_ptr cinfo)
{
	PINT8	lps, lpb;	// NOTE: In 16-Bit this is "huge"
	BYTE	index;
	DWORD	dd;
	// Fill in the hgData with the BACKGROUND Index
	if( hgData )
	{
		if( lps = (PINT8) JGlobalLock( hgData ) )
		{
			lpb = lps + (sizeof( BITMAPINFOHEADER ) + BMPCols);
			index =	cinfo->GHE.gheIndex;	// Background Colour Index
			for( dd = 0; dd < ddRCnt ; dd++ )
			{
				lpb[dd] = index;
			}
			JGlobalUnlock( hgData );
		}
	}
}


METHODDEF boolean
gif_input_initx( compress_info_ptr cinfo )
{
	char hdrbuf[10];		/* workspace for reading control blocks */
	int cmaplen, aspectRatio, gmaplen;
	int c;
	WORD	icnt;
	UINT16	logwidth, logheight;		/* Logical Screen dimensions */
	DWORD	ddRC;

	ResetBMPSizes();

	/* Allocate space to store the colormap */

	colormap = (*cinfo->emethods->alloc_small_sarray)
		((long) MAXCOLORMAPSIZE,
		(long) GNUMCOLORS);

	cinfo->pcolormap = colormap;

	/* Read and verify GIF Header */
	if( !ReadOK(cinfo->input_file, hdrbuf, 6) )
	{
		ERRFLAG( BAD_GIFNOT );	/* "Not a GIF file" */
		return( bErrFlag );
	}
	if (hdrbuf[0] != 'G' || hdrbuf[1] != 'I' || hdrbuf[2] != 'F')
	{
		ERRFLAG( BAD_GIFNOT );
		return( bErrFlag );
	}

	/* Check for expected version numbers.
	 * If unknown version, give warning and try to process anyway;
	 * this is per recommendation in GIF89a standard.
	 */
	if( (hdrbuf[3] != '8' || hdrbuf[4] != '7' || hdrbuf[5] != 'a') &&
		(hdrbuf[3] != '8' || hdrbuf[4] != '9' || hdrbuf[5] != 'a') )
	{
#ifndef	_INC_WINDOWS
		TRACEMS3(cinfo->emethods, 1,
			"Warning: unexpected GIF version number '%c%c%c'",
			hdrbuf[3], hdrbuf[4], hdrbuf[5]);
#endif	// !_INC_WINDOWS
	}

	/* Read and decipher Logical Screen Descriptor */
	if( !ReadOK(cinfo->input_file, hdrbuf, 7) )
	{
		ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
		return( bErrFlag );
	}

	cinfo->GHE.gheWidth = GIFwidth = LM_to_uint(hdrbuf[0],hdrbuf[1]);
	cinfo->GHE.gheHeight = GIFheight = LM_to_uint(hdrbuf[2],hdrbuf[3]);
	// Add the EXTENDED Information
	cinfo->GHE.gheBits = hdrbuf[4];		// packed field
	cinfo->GHE.gheIndex = hdrbuf[5];	// Background Colour Index
	cinfo->GHE.ghePAR   = hdrbuf[6];	// Pixel Aspect Ration

	logwidth = GIFwidth = LM_to_uint(hdrbuf[0],hdrbuf[1]);
	logheight = GIFheight = LM_to_uint(hdrbuf[2],hdrbuf[3]);

	cmaplen = 2 << (hdrbuf[4] & 0x07);

	/* we ignore the color resolution, sort flag,
	   and background color index */

	aspectRatio = hdrbuf[6] & 0xFF;
//	if( aspectRatio != 0 && aspectRatio != 49 )
//	{
//	}

	gmaplen = 0;
	ddRC = 0;
	/* Read global colormap if header indicates it is present */
	if( BitSet(hdrbuf[4], COLORMAPFLAG) )
	{
		SetBMPSizes( cmaplen );
		ddRC = GetDataSize( GIFwidth, GIFheight, BMPBits );
		gmaplen = cmaplen;
		if( BMPCols )
			hgColors = JGlobalAlloc( GHND, BMPCols );
		else
			hgColors = 0;
		ReadColorMap(cinfo, cmaplen, colormap);
	}
	else
	{
		ERRFLAG( BAD_NOGCOLR );	// This "Plain Text" GIF MUST
		return( bErrFlag );		// have a GLOBAL Color Table.
	}

	// Scan until we reach start of desired image.
	icnt = 0;
	if( icnt == wGetICnt )
	{
		// We have the LOGICAL Screen Size and Color Table
		cinfo->methods->c_pipeline_controller = do_nothing;
		cinfo->methods->c_ui_method_selection = jselwnul;
		cinfo->methods->colorin_init = gif_background_index;
		goto exit_initx;
	}
	else
	{
		wGetICnt--;		// Back up to LOGICAL Image Number
		ddRC = 0;
	}
	for( ; ; )
	{

		c = ReadByte( cinfo );

		if( c == ';' )		/* GIF terminator?? */
		{
			ERRFLAG( BAD_GIFCNT ); /* "Too few images in GIF file" */
			return( bErrFlag );
		}

		if( c == '!' )
		{	/* Extension */
			//DoExtension( cinfo );
			int extlabel;
			/* Read extension label byte */
			extlabel = ReadByte( cinfo );
			if( extlabel == GIF_PTxtExt )
			{
				if( icnt >= wGetICnt )
				{	// If we reached the desired text block
					if( cinfo->hgOut )
					{
//     +---------------+
//  1  |               | 0 Text Grid Left Position Unsigned
//     +-             -+
//  2  |               | 1
//     +---------------+
//  3  |               | 2 Text Grid Top Position        Unsigned
//     +-             -+
//  4  |               | 3
//     +---------------+
//  5  |               | 4 Text Grid Width               Unsigned
//     +-             -+
//  6  |               | 5
//     +---------------+
//  7  |               | 6 Text Grid Height              Unsigned
//     +-             -+
//  8  |               | 7
//     +---------------+
//  9  |               | 8 Character Cell Width          Byte
//     +---------------+
// 10  |               | 9 Character Cell Height         Byte
//     +---------------+
// 11  |               | 10 Text Foreground Color Index   Byte
//     +---------------+
// 12  |               | 11 Text Background Color Index   Byte
//     +---------------+
//			lpGIE->gceFlag |=	gie_PTE;	// Plain Text Extension
//			lpGIE->giLeft = LM_to_uint(lpb[0],lpb[1]);// Left (logical) column of TEXT
//			lpGIE->giTop  = LM_to_uint(lpb[2],lpb[3]);// Top (logical) row
//			lpGIE->giGI.giWidth = LM_to_uint(lpb[4],lpb[5]);
//			lpGIE->giGI.giHeight = LM_to_uint(lpb[6],lpb[7]);
//			lpGIE->gceRes1 = (DWORD)LM_to_uint(lpb[8],lpb[9]); // Char CELL
//			lpGIE->gceRes2 = (DWORD)lpb[10];	// Foreground index
//			lpGIE->gceIndex = lpb[11];	// Backgound INDEX
//	DWORD	gceColr;	// COLORREF (if SET)
						DWORD	csz;
						LPSTR	lout;
						int		count;
						DWORD	tcnt, terr;
						LPPTEHDR	ppte;

						// Nothing more to do
						jselwnul( cinfo );	// DO UI SELECTION NOW
						cinfo->methods->colorin_init = do_nothing;
						cinfo->methods->c_pipeline_controller = do_nothing;
						cinfo->methods->c_ui_method_selection = do_nothing;
						cinfo->methods->input_term = do_nothing;
						csz = GlobalSize( cinfo->hgOut );
						if( (csz >= sizeof(PTEHDR)) &&
							( lout = JGlobalLock( cinfo->hgOut ) ) )
						{
							cinfo->lpOut = lout;	// Establish value to unlock on exit.
							ppte = (LPPTEHDR)lout;	// Keep beginning
							ppte->pt_Total = 0;
							ppte->pt_Missed = 0;
							lout += sizeof(PTEHDR);				// Bump by HEADER
							csz -= sizeof(PTEHDR);	// Reduce available
							tcnt = 0;	// Start COUNT
							terr = 0;	// Start MISSED
							while( count = ReadByte(cinfo) )
							{
								if( csz > (DWORD)count )
								{
									tcnt += (count + 1);	// Add to TOTAL
									*lout++ = (BYTE)count;	// Insert the count
									csz--;					// Reduce available
									if( !ReadOK(cinfo->input_file, lout, count) )
									{	// Failed of READ
										ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
									}
									csz -= count;	// Remove this count
									lout += count;	// Bump the POINTER
								}
								else
								{	// Discard DATA
									terr += (count + 1); // Keep DISCARD count
									if( !ReadOK(cinfo->input_file, code_buf, count) )
									{
										ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
									}
								}
							}
							if( csz )	// If there is BUFFER left
							{
								*lout = 0;	// Zero TERMINATION
								tcnt++;		// Plus 1
								tcnt += sizeof(PTEHDR); // plus HEADER
							}
							ppte->pt_Total  = tcnt;
							ppte->pt_Missed = terr;
						}
						else
						{
							bErrFlag = BAD_MEMORY12;	/* Set an error, but fall through to FREE */
						}
					}
					goto exit_initx;
				}
				else
				{
					icnt++;	/* Bump to NEXT, and ... */
				}
			}
			// Skip the data block(s) associated with
			// the extension
			SkipDataBlocks( cinfo );
			continue;
		}
    
		if( c != ',' )
		{		/* Not an image separator? */
			continue;
		}

		/* Read and decipher Local Image Descriptor */
		if( !ReadOK(cinfo->input_file, hdrbuf, 9) )
		{
			ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
			return( bErrFlag );
		}

		/* we ignore top/left position info, also sort flag */
		GIFwidth = LM_to_uint(hdrbuf[4],hdrbuf[5]);
		GIFheight = LM_to_uint(hdrbuf[6],hdrbuf[7]);

		is_interlaced = BitSet(hdrbuf[8], INTERLACE);

		/* Read local colormap if header indicates it is present */
		/* Note: if we skip it if not ours */
		if( BitSet(hdrbuf[8], COLORMAPFLAG)) 
		{

			cmaplen = 2 << (hdrbuf[8] & 0x07);
			SetBMPSizes( cmaplen );
			if( icnt < wGetICnt )
			{	/* Not reached the DESIRED colormap yet ... */
				SkipColorMap( cinfo, cmaplen );
			}
			else
			{
				if( hgColors )	/* If already GOT a GLOBAL, then discard it ... */
					JGlobalFree( hgColors );
				if( BMPCols )
					hgColors = JGlobalAlloc( GHND, BMPCols );
				else
					hgColors = 0;

				ReadColorMap( cinfo, cmaplen, colormap );
			}
		}


		input_code_size = ReadByte(cinfo); /* get minimum-code-size byte */
		if( ( input_code_size < 2 ) ||
			( input_code_size >= MAX_LZW_BITS ) )
		{
			ERRFLAG( BAD_GIFCS ); /* "Bogus codesize %d, input_code_size */
			return( bErrFlag );
		}

		/* If we reached desired image, break out of loop */
		if( icnt >= wGetICnt )	/* If we reached the desired image ... */
			break;	/* FOUND IT - Do the conversion ... */

		icnt++;	/* Bump to NEXT, and ... */
		SkipDataBlocks( cinfo );	/* Skip this data ... */

	}	// Forever!!!

	/* Prepare to read selected image:
	   first initialize LZW decompressor */

	symbol_head = (UINT16 FAR *) (*cinfo->emethods->alloc_medium)
		(LZW_TABLE_SIZE * SIZEOF(UINT16));
	symbol_tail = (UINT8 FAR *) (*cinfo->emethods->alloc_medium)
		(LZW_TABLE_SIZE * SIZEOF(UINT8));
	symbol_stack = (UINT8 FAR *) (*cinfo->emethods->alloc_medium)
		(LZW_TABLE_SIZE * SIZEOF(UINT8));

	InitLZWCode();

	/*
	 * If image is interlaced, we read it into a full-size sample array,
	 * decompressing as we go; then gif_get_input_row selects rows from the
	 * sample array in the proper order.
	 */

	if( is_interlaced )
	{
		/* We request the big array now, but can't access it until the pipeline
		 * controller causes all the big arrays to be allocated.  Hence, the
		 * actual work of reading the image is postponed until the first call
		 * of gif_get_input_row.
		 */

		interlaced_image = (*cinfo->emethods->request_big_sarray)
			((long) GIFwidth, (long) GIFheight, 1L);

		cinfo->methods->get_input_row = load_interlaced_image;

		cinfo->total_passes++;	/* count file reading as separate pass */

	}

exit_initx:
	/* Return info about the image. */
	cinfo->input_components = GNUMCOLORS;
	cinfo->in_color_space = CS_RGB;
	cinfo->image_width = GIFwidth;
	cinfo->image_height = GIFheight;
	cinfo->data_precision = 8;	/* always, even if 12-bit JSAMPLEs */
	ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
	hgData = JGlobalAlloc( GMEM_SHARE, /* Get a FIXED SHAREable memory handle */
		(sizeof( BITMAPINFOHEADER ) + BMPCols + ddRCnt) );

	ddDCnt = 0;
	ddTCnt = ddRC;	// This will be ZERO EXCEPT for first LOG.SCREEN

	return( bErrFlag );

}

#endif	// _INC_WINDOWS & DIRGIFBMP


/*
 * Read one row of pixels.
 * This version is used for noninterlaced GIF images:
 * we read directly from the GIF file.
 */

METHODDEF void
gif_get_input_row (compress_info_ptr cinfo, JSAMPARRAY pixel_row)
{
	register JSAMPROW ptr0, ptr1, ptr2;
	register long col;
	register int c;
#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
	PINT8	lpb;
	PINT8	lpd;
	PINT8	lps;
	DWORD	bgncnt;
	DWORD	dds, ddd;

	lpb = 0;
	lps = 0;
	bgncnt = ddDCnt;
	if( hgData )
	{
		if( lps = ( PINT8 ) JGlobalLock( hgData ) )
			lpb = lps + (sizeof( BITMAPINFOHEADER ) + BMPCols );
	}
#endif	// _INC_WINDOWS & DIRGIFBMP

	ptr0 = pixel_row[0];
	ptr1 = pixel_row[1];
	ptr2 = pixel_row[2];

	for( col = cinfo->image_width; col > 0; col-- )
	{
		c = LZWReadByte(cinfo);
#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
		if( lpb )
		{
			lpb[ddDCnt++] = c;
		}
#endif	// _INC_WINDOWS & DIRGIFBMP

		*ptr0++ = colormap[CM_RED][c];
		*ptr1++ = colormap[CM_GREEN][c];
		*ptr2++ = colormap[CM_BLUE][c];
	}	/* For ... */


#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
	if( lpb &&
		(dwRow32 > GIFwidth) &&
		(ddd = dwRow32 - GIFwidth ) )
	{
		for( dds = 0; dds < ddd; dds++ )
		{
			lpb[ddDCnt++] = 0;
		}
	}

	ddTCnt += ddDCnt - bgncnt;
	if( fInvertI )
	{
		if( (ddDCnt - bgncnt) == dwRow32 )	/* If one LINE filled in ... */
		{
			if( ddRCnt >= dwRow32 )
			{
				ddRCnt -= dwRow32;
			}
			else
			{
				ddRCnt = 0;
			}
			if( ddRCnt )	/* Then this line has to be moved ... */
			{
				lpd = lpb + ddRCnt;	/* Get location, other end of buffer dest. */
				lps = lpb + bgncnt;	/* Get where this line started ... */
					gw_fmemcpy( lpd, 
					lps, 
					LOWORD(dwRow32) );
			}
			ddDCnt = 0;	/* Reset this offset back to BEGINNING of buffer */
		}
	}
	if( hgData && lps )
		JGlobalUnlock( hgData );

#endif	// _INC_WINDOWS & DIRGIFBMP
}


/*
 * Read one row of pixels.
 * This version is used for the first call on gif_get_input_row when
 * reading an interlaced GIF file: we read the whole image into memory.
 */

METHODDEF void
load_interlaced_image (compress_info_ptr cinfo, JSAMPARRAY pixel_row)
{
	JSAMPARRAY image_ptr;
	register JSAMPROW sptr;
	register long col;
	long row;

	/* Read the interlaced image into the big array we've created. */
	for( row = 0; row < cinfo->image_height; row++ )
	{
#ifdef	PROGRESS_REPORT
		(*cinfo->methods->progress_monitor) (cinfo, row, cinfo->image_height);
#endif	// PROGRESS_REPORT

		image_ptr = (*cinfo->emethods->access_big_sarray)
			(interlaced_image, row, TRUE);

		sptr = image_ptr[0];
		for( col = cinfo->image_width; col > 0; col-- )
		{
			*sptr++ = (JSAMPLE) LZWReadByte(cinfo);
		}
	}

	cinfo->completed_passes++;

	/* Replace method pointer so subsequent calls don't come here. */
	cinfo->methods->get_input_row = get_interlaced_row;

	/* Initialize for get_interlaced_row, and perform first call on it. */
	cur_row_number = 0;
	pass2_offset = (cinfo->image_height + 7L) / 8L;
	pass3_offset = pass2_offset + (cinfo->image_height + 3L) / 8L;
	pass4_offset = pass3_offset + (cinfo->image_height + 1L) / 4L;
	
	get_interlaced_row( cinfo, pixel_row );

}


/*
 * Read one row of pixels.
 * This version is used for interlaced GIF images:
 * we read from the big in-memory image.
 */

METHODDEF void
get_interlaced_row (compress_info_ptr cinfo, JSAMPARRAY pixel_row)
{
	JSAMPARRAY image_ptr;
	register JSAMPROW sptr, ptr0, ptr1, ptr2;
	register long col;
	register int c;
	long irow;
#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
	PINT8	lpb;
	PINT8	lpd;
	PINT8	lps;
	DWORD	bgncnt;
	DWORD	dds;

	lpb = 0;
	lps = 0;
	bgncnt = ddDCnt;
	if( hgData )
	{
		if( lps = ( PINT8 ) JGlobalLock( hgData ) )
			lpb = lps + (sizeof( BITMAPINFOHEADER ) + BMPCols );
	}
#endif	// _INC_WINDOWS & DIRGIFBMP

	/* Figure out which row of interlaced image is needed,
	   and access it. */
	switch( (int)( cur_row_number & 7L ) )
	{

	case 0:			/* first-pass row */
		irow = cur_row_number >> 3;
		break;

	case 4:			/* second-pass row */
		irow = (cur_row_number >> 3) + pass2_offset;
		break;

	case 2:			/* third-pass row */
	case 6:
		irow = (cur_row_number >> 2) + pass3_offset;
		break;

	default:			/* fourth-pass row */

		irow = (cur_row_number >> 1) + pass4_offset;
		break;

	}

	image_ptr = (*cinfo->emethods->access_big_sarray)
		(interlaced_image, irow, FALSE);

	/* Scan the row, expand colormap, and output */
	sptr = image_ptr[0];
	ptr0 = pixel_row[0];
	ptr1 = pixel_row[1];
	ptr2 = pixel_row[2];

	for( col = cinfo->image_width; col > 0; col-- )
	{
		c = GETJSAMPLE(*sptr++);
#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
		if( lpb )
		{
			lpb[ddDCnt++] = c;
		}
#endif
		*ptr0++ = colormap[CM_RED][c];
		*ptr1++ = colormap[CM_GREEN][c];
		*ptr2++ = colormap[CM_BLUE][c];
	}
	cur_row_number++;		/* for next time */
#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
	if( lpb && (dwRow32 > GIFwidth) )
	{
		for( dds = 0; dds < (dwRow32 - GIFwidth); dds++ )
		{
			lpb[ddDCnt++] = 0;	/* ZERO padding to LONG boundary ... */
		}
	}
	ddTCnt += ddDCnt - bgncnt;
	if( fInvertI )
	{
		if( (ddDCnt - bgncnt) == dwRow32 )	/* If one LINE filled in ... */
		{
			if( ddRCnt >= dwRow32 )
			{
				ddRCnt -= dwRow32;
			}
			else
			{
				ddRCnt = 0;
			}
			if( ddRCnt )	/* Then this line has to be moved ... */
			{
				lpd = lpb + ddRCnt;	/* Get location, other end of buffer dest. */
				lps = lpb + bgncnt;	/* Get where this line started ... */
				gw_fmemcpy( lpd, 
					lps, 
					LOWORD(dwRow32) );
			}
			ddDCnt = 0;	/* Reset this offset back to BEGINNING of buffer */
		}
	}
	if( hgData && lps )
		JGlobalUnlock( hgData );

#endif	// _INC_WINDOWS & DIRGIFBMP

}


/*
 * Finish up at the end of the file.
 */
#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))

#ifdef	ADDOLDTO
#define	MAXWRITE		1024
BOOL	BMPwrite( HFILE hf, LPSTR lpb, DWORD siz )
{
PINT8	cp;
DWORD	rem;
WORD	wcnt, dcnt, lwd, wb;
BOOL	flg;
	flg = FALSE;
	rem = siz;
	cp = ( PINT8 ) lpb;
	while( rem && hf && (hf != HFILE_ERROR) )
	{
		if( rem > MAXWRITE )
			wcnt = MAXWRITE;
		else
			wcnt = (WORD) rem;
		if( lwd = LOWORD( cp ) )
		{
			wb = ~lwd + 1;
			if( wb )
			{
				if( wb < wcnt )
					wcnt = wb;
			}
		}
		dcnt = _lwrite( hf, cp, wcnt );
		if( dcnt != wcnt )
		{
			flg = TRUE;
			break;
		}
		cp += dcnt;
		rem = rem - dcnt;
	}
return( flg );
}
// NOT yet retired
//#ifndef	WJPEG4	// NOTE: Old FILE interface RETIRED
//#endif	// !WJPEG4
#endif	// ADDOLDTO

#define	MAXCOPYSZ		32768

PINT8 BMPcopy( PINT8 bhd, PINT8 bhs, DWORD len )
{
PINT8	dest;
PINT8	src;
WORD	csz, soff, sbal;
DWORD	rem;
	dest = bhd;
	if( ( dest = bhd ) &&
		( src = bhs ) &&
		( rem = len ) )
	{
		while( rem )
		{
			if( rem > MAXCOPYSZ )
				csz = MAXCOPYSZ;
			else
				csz = LOWORD( rem );
			if( soff = LOWORD( dest ) )
			{
				sbal = ~soff + 1;
				if( sbal )
				{
					if( sbal < csz )
						csz = sbal;
				}
			}
			gw_fmemcpy( dest, src, csz );
			rem -= (DWORD) csz;
			dest = dest + csz;
			src = src + csz;
		}
	}
return( dest );
}

#endif	// _INC_WINDOWS & DIRGIFBMP


METHODDEF void
gif_input_term (compress_info_ptr cinfo)
{
	/* no work (we let free_all release the workspace) */
	// BUT in Windows this is where the BMP is CREATED!!!
#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
	LPSTR	lpd;
	PINT8	bhh;
	PINT8	bhd;
	PINT8	bhc;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	LPBITMAPINFOHEADER lpbmih;
	PINT8	lout;
	DWORD	csz;
#ifndef	WJPEG4
	HFILE	hf;
	LPSTR	lpo;
#endif	// !WJPEG4

	lpd = 0;
	if( hgData &&
		( lpData = JGlobalLock( hgData ) ) )
	{
		lpbmih = (LPBITMAPINFOHEADER) lpData;
		bhh = ( PINT8 ) lpData;	/* This is actually INFO Header */
		bhc = bhh + sizeof( BITMAPINFOHEADER );
		bhd = bhc + BMPCols;
	}
	else
	{
		bhc = bhd = 0;
		lpbmih = &bmih;
	}
	/* Fill in bm File Header */
	bmfh.bfType = 'MB';	/* Add the BITMAP signature ... "BM" actually ... */
	bmfh.bfSize = sizeof( BITMAPINFOHEADER ) +	/* Header + */
		BMPCols +	/* Color MAP + */
		(dwRow32 * GIFheight);	/* Pixel Data */
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof( BITMAPFILEHEADER ) +
		sizeof( BITMAPINFOHEADER ) +
		BMPCols;

	/* Fill in bm Information Header */
	lpbmih->biSize = sizeof( BITMAPINFOHEADER );
	lpbmih->biWidth = GIFwidth;
	lpbmih->biHeight = GIFheight;
	lpbmih->biPlanes = 1;
//	lpbmih->biBitCount = 8;
	lpbmih->biBitCount = BMPBits;	/* Set the BIT COUNT calculated earlier */
	lpbmih->biCompression = BI_RGB;
	lpbmih->biSizeImage = (dwRow32 * GIFheight);
	lpbmih->biXPelsPerMeter = 0;
	lpbmih->biYPelsPerMeter = 0;
	lpbmih->biClrUsed = 0;
	lpbmih->biClrImportant = 0;
	if( hgColors )	/* If we have a Color Table */
	{
		if( ( lpColors = JGlobalLock( hgColors ) ) &&
			bhc )
		{
			BMPcopy( bhc, lpColors, BMPCols );
		}
	}
	else
	{
		lpColors = 0;
	}
	if( fWrtTemp &&
		bhd &&
		ddTCnt )
	{	/* If we have the data and data count, then */
		if( cinfo->hgOut )
		{
			csz = sizeof( BITMAPINFOHEADER ) +
				BMPCols +
				(dwRow32 * GIFheight);
			if( ( GlobalSize( cinfo->hgOut ) >= csz ) &&
				( lout = ( PINT8 ) JGlobalLock( cinfo->hgOut )) )
			{
				cinfo->lpOut = lout;	/* Establish value to unlock on exit */
				BMPcopy( lout, ( PINT8 ) lpbmih, sizeof( BITMAPINFOHEADER ));
				lout += sizeof( BITMAPINFOHEADER );	/* Bump pointer ... */
				if( BMPCols && lpColors && bhc )	/* IFF there is a color table ... */
				{
					BMPcopy( lout, bhc, BMPCols );
					lout += BMPCols;	/* Bump by size of the color table */
				}
				BMPcopy( lout, bhd, (dwRow32 * GIFheight) );
			}
			else
			{
				bErrFlag = BAD_MEMORY12;	/* Set an error, but fall through to FREE */
			}
		}
		else
		{
#ifdef	ADDOLDTO
			HFILE	hf;
			LPSTR	lpo;
			lpo = 0;
			hf = 0;
			if( lpOutFile )
			{
				lpo = lpOutFile;
				if( ( hf = gout_open( lpo ) ) &&
					( hf != (int) -1 ) )
				{	// We can WRITE a file
					BMPwrite( hf, (LPSTR) &bmfh, sizeof( BITMAPFILEHEADER ) );
					BMPwrite( hf, (LPSTR) lpbmih, sizeof( BITMAPINFOHEADER ) );
					if( BMPCols && lpColors && bhc )	// IFF there is a color table ...
					{
						//BMPwrite( hf, lpColors, BMPCols );
						BMPwrite( hf, (LPSTR) bhc, BMPCols );
					}
					BMPwrite( hf, (LPSTR) bhd, (dwRow32 * GIFheight) );
					_lclose( hf );
				}
			}
#else	// !ADDOLDTO
			bErrFlag = BAD_RETIRED;	// Set ERROR
#endif	// ADDOLDTO y/n
		}
	}
	if( hgData && bhd )
	{
		JGlobalUnlock( hgData );
		lpData = 0;
		bhd = 0;
/* ======================================================= */
		hgDIB = hgData;	/* Return the GLOBAL MEMORY OBJECT */
/* ======================================================= */
#ifndef	PROBGOBJ
		hgData = 0;
#endif	/* PROBGOBJ */
	}
	ResetBMPSizes();
#endif	// _INC_WINDOWS & DIRGIFBMP

}

#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
/*
 * Read the file header; return image size and component count.
 */
LOCAL void
SkipColorMap( compress_info_ptr cinfo, int cmaplen )
{	/* Skip a GIF colormap */
	int i;
	for( i = 0; i < cmaplen; i++ )
	{
		ReadByte(cinfo);
		ReadByte(cinfo);
		ReadByte(cinfo);
	}
}



GLOBAL	DWORD
get_gif_size( compress_info_ptr cinfo )
{
  char hdrbuf[10];		/* workspace for reading control blocks */
  int gifcmaplen, aspectRatio;
  int c;
	ResetBMPSizes();
  /* Read and verify GIF Header */
	if( !ReadOK( cinfo->input_file, hdrbuf, 6 ) )
	{
		ERRFLAG( BAD_GIFNOT );	/* "Not a GIF file" */
		return( 0 );
	}
	if (hdrbuf[0] != 'G' || hdrbuf[1] != 'I' || hdrbuf[2] != 'F')
	{
		ERRFLAG( BAD_GIFNOT );
		return( 0 );
	}
  /* Check for expected version numbers.
   * If unknown version, give warning and try to process anyway;
   * this is per recommendation in GIF89a standard.
   */
//  if ((hdrbuf[3] != '8' || hdrbuf[4] != '7' || hdrbuf[5] != 'a') &&
//      (hdrbuf[3] != '8' || hdrbuf[4] != '9' || hdrbuf[5] != 'a'))
//	{
//	}
  /* Read and decipher Logical Screen Descriptor */
	if( !ReadOK(cinfo->input_file, hdrbuf, 7) )
	{
		ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
		return( 0 );
	}
  GIFwidth = LM_to_uint(hdrbuf[0],hdrbuf[1]);
  GIFheight = LM_to_uint(hdrbuf[2],hdrbuf[3]);
  gifcmaplen = 2 << (hdrbuf[4] & 0x07);
  /* we ignore the color resolution, sort flag, and background color index */
  aspectRatio = hdrbuf[6] & 0xFF;
//	if( aspectRatio != 0 && aspectRatio != 49 )
//	{
//	}
  /* Read global colormap if header indicates it is present */
	if( BitSet(hdrbuf[4], COLORMAPFLAG) )
	{
		SetBMPSizes( gifcmaplen );
    	SkipColorMap( cinfo, gifcmaplen );
	}
  /* Scan until we reach start of desired image.
   * We don't currently support skipping images, but could add it easily.
   */
	for( ;; )
	{
		c = ReadByte(cinfo);
		if( c == ';' )		/* GIF terminator?? */
		{
			ERRFLAG( BAD_GIFCNT ); /* "Too few images in GIF file" */
			return( 0 );
		}
		if (c == '!')
		{		/* Extension */
      	DoExtension(cinfo);
      	continue;
    	}
		if( c != ',' )
		{		/* Not an image separator? */
			continue;
		}
		/* Read and decipher Local Image Descriptor */
		if( !ReadOK(cinfo->input_file, hdrbuf, 9) )
		{
			ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
			return( 0 );
		}
		/* we ignore top/left position info, also sort flag */
		GIFwidth = LM_to_uint(hdrbuf[4],hdrbuf[5]);
		GIFheight = LM_to_uint(hdrbuf[6],hdrbuf[7]);
		is_interlaced = BitSet(hdrbuf[8], INTERLACE);
		/* Read local colormap if header indicates it is present */
		/* Note: if we wanted to support skipping images, */
		/* we'd need to skip rather than read colormap for ignored images */
		if( BitSet(hdrbuf[8], COLORMAPFLAG) ) 
		{
			gifcmaplen = 2 << (hdrbuf[8] & 0x07);
			SetBMPSizes( gifcmaplen );
			SkipColorMap( cinfo, gifcmaplen );
		}
		input_code_size = ReadByte(cinfo); /* get minimum-code-size byte */
		if(input_code_size < 2 || input_code_size >= MAX_LZW_BITS)
		{
	      ERRFLAG( BAD_GIFCS ); /* "Bogus codesize %d, input_code_size */
			return( 0 );
		}
	/* Reached desired image, so break out of loop */
	/* If we wanted to skip this image, */
	/* we'd call SkipDataBlocks and then continue the loop */
		break;
	}	/* Forever ... */

  /* Return info about the image. */
  cinfo->input_components = GNUMCOLORS;
  cinfo->in_color_space = CS_RGB;
  cinfo->image_width = GIFwidth;
  cinfo->image_height = GIFheight;
  cinfo->data_precision = 8;	/* always, even if 12-bit JSAMPLEs */
	ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
	return( sizeof( BITMAPINFOHEADER ) + BMPCols + ddRCnt );
}

//typedef struct tagGIFIMG {
//	WORD	giWidth;
//	WORD	giHeight;
//	WORD	giCMSize;
//	DWORD	giBMPSize;
//}GIFIMG;
//typedef GIFIMG FAR *LPGIFIMG;
//typedef struct tagGIFHDR {
//	WORD	ghMaxCount;
//	WORD	ghImgCount;
//	WORD	ghWidth;
//	WORD	ghHeight;
//	WORD	ghCMSize;
//	DWORD	ghBMPSize;
//	GIFIMG ghGifImg[1];
//}GIFHDR;
//typedef GIFHDR FAR *LPGIFHDR;

///* Multiple and Transparent GIF Image Support */
///* ========================================== */
//#define		gie_Flag		0x8000	// This is in the ghMaxCount
//// if the application expect an EXTENDED description!
//typedef	struct	tagGIFIMGEXT {
//	GIFIMG	giGI;	// Width/Height/ColSize and BMP Size as above
//// Image Descriptor - Wdith and Height in above
//	WORD	giLeft;		// Left (logical) column of image
//	WORD	giTop;		// Top (logical) row
//	BYTE	giBits;		// See below (packed field)
//// Graphic Control Extension
//	BYTE	gceExt;		// Extension into - Value = 0x21
//	BYTE	gceLabel;	// Graphic Control Extension = 0xf9
//	DWORD	gceSize;	// Block Size (4 for GCE, BIG for TEXT)
//	BYTE	gceBits;	// See below (packed field)
//	WORD	gceDelay;	// 1/100 secs to wait
//	BYTE	gceIndex;	// Transparency Index (if Bit set)
//	DWORD	gceColr;	// COLORREF (if SET)
//	DWORD	gceFlag;	// IN/OUT Options Flag - See Below
//	RGBQUAD	gceBkGrnd;	// Background Colour
//	DWORD	gceRes1;	// Reserved
//	DWORD	gceRes2;	// ditto
//}GIFIMGEXT;
//typedef GIFIMGEXT	FAR * LPGIFIMGEXT;
//typedef struct tagGIFHDREXT {
//	WORD	gheMaxCount;	// gie_Flag + MAX. Count
//	WORD	gheImgCount;	// Images in GIF
//	WORD	gheWidth;		// Logical Screen Width
//	WORD	gheHeight;		// Logical Screen Height
//	WORD	gheCMSize;		// BMP Colour map size (byte count)
//	DWORD	gheBMPSize;		// Estimated final BMP size
//	BYTE	gheBits;		// See below (packed field)
//	BYTE	gheIndex;		// Background Colour Index
//	BYTE	ghePAR;			// Pixel Aspect Ration
//	DWORD	gheFlag;		// IN/OUT Options Flag - See Below
//	RGBQUAD	gheBkGrnd;		// Background Colour
//	DWORD	gheRes1;		// Reserved
//	DWORD	gheRes2;		// ditto
//	GIFIMGEXT	gheGIE[1];	// 1 for Each Image follows
//}GIFHDREXT;
////typedef GIFHDREXT FAR * LPGIFHDREXT;
// GIFHDREXT Flag
//#define		ghf_Netscape	0x00000001	// Contains "Netscape" Extension
// That is, it is an "ANIMATED" GIF
//#define		ghf_Incomplete	0x80000000	// Incomplete GIF data
//#define		ghf_AppExt		0x00000002	// Had App Extension
//#define		ghf_CommExt		0x00000004	// Had Comment Extension
//#define		ghf_CtrlExt		0x00000008	// Had Graphic Control Extension
//#define		ghf_PTxtExt		0x00000010	// Had plain text extension
//#define		ghf_UnknExt		0x40000000	// Had an UNKNONW extension

//#define	gie_Netscape		0x00000001	// Found NETSCAPE extension
//#define	gie_GCE				0x00000002	// Had Graphic Control Extension
//#define	gie_PTE				0x00000004	// Plain Text Extentsion
//#define	gie_GID				0x00000008	// Image Descriptor
//#define		gie_APP			0x00000010	// Application Extension
//#define		gie_COM			0x00000020	// Comment Extension
//#define		gie_UNK			0x80000000	// Unknown Extension

//#define GIF_AppExt              0xFF
//#define GIF_CommExt             0xFE
//#define GIF_CtrlExt             0xF9
//#define GIF_ImgDesc             0x2C
//#define GIF_PTxtExt             0x01
//#define GIF_Trailer             0x3B

BOOL	IsNetscape( LPSTR lpb, int i )
{
	BOOL	flg = FALSE;
	if( ( lpb[i+0] == 'N' ) &&
		( lpb[i+1] == 'E' ) &&
		( lpb[i+2] == 'T' ) &&
		( lpb[i+3] == 'S' ) &&
		( lpb[i+4] == 'C' ) &&
		( lpb[i+5] == 'A' ) &&
		( lpb[i+6] == 'P' ) &&
		( lpb[i+7] == 'E' ) &&
		( lpb[i+8] == '2' ) &&
		( lpb[i+9] == '.' ) &&
		( lpb[i+10] == '0' ) &&
		( lpb[i+11] == 3 ) &&
		( lpb[i+12] == 1 ) )
	{
		flg = TRUE;
	}
	return flg;
}

int	DoExtensionExt( compress_info_ptr cinfo, LPGIFIMGEXT lpGIE,
				   LPGIFHDREXT lpGHE, LPRGBQUAD lprgb )
{
	int extlabel;
	char buf[256];
	int count;
	int	i;
	LPSTR	lps, lpb;

	/* Read extension label byte */
	extlabel = ReadByte( cinfo );
	lpGIE->gceLabel = (BYTE)extlabel;	// Graphic Control Extension = 0xf9
	//lpGIE->gceSize  = 0;	// Block Size (4 for TEXT, Bit for TEXT)
	lpb = &code_buf[0];		// Use STATIC buffer for FIRST
	/* Process the FIRST data block associated with the extension */
	if( count = ReadByte( cinfo ) )
	{
		lpGIE->gceSize += (count+1);	// Accumlate DATA SIZE
		if( !ReadOK(cinfo->input_file, lpb, count) )
		{
			ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
		}
		if( ( count    == 4 ) &&
			( extlabel == GIF_CtrlExt ) )
		{
			// NOTE: Offsets a MINUS 1, since we have read the count
			lpGIE->gceFlag |=	gie_GCE;	// Graphic Control Extension
			lpGIE->gceBits = lpb[0];	// packed field
//	DWORD	gceColr;	// COLORREF (if SET)
			lpGIE->gceDelay = LM_to_uint(lpb[1],lpb[2]);	// 1/100 secs to wait
			lpGIE->gceIndex = lpb[3];	// Transparency Index (if Bit set)
			lpGIE->gceColr = 0;	// Just ZERO will do.
			if( ( lpGIE->gceBits & gce_TransColr ) &&
				lprgb )
			{
				int	idx;
				idx = lpGIE->gceIndex & 0xff;
				lpGIE->gceColr = RGB( lprgb[idx].rgbRed,
										lprgb[idx].rgbGreen,
										lprgb[idx].rgbBlue );
				if( lpGIE->gceColr )
				{
					lpGIE->gceColr = RGB( lprgb[idx].rgbRed,
										lprgb[idx].rgbGreen,
										lprgb[idx].rgbBlue );
				}
			}
		}
		else if( ( count == 11 ) &&
			( extlabel == GIF_AppExt ) )
		{
			lpGIE->gceFlag |= gie_APP;	// Application Extension
			if( IsNetscape( lpb, 0 ) )
			{
				lpGIE->gceRes1 = (DWORD)LM_to_uint(lpb[13],lpb[14]);	// Loop count
				lpGIE->gceFlag |= gie_Netscape;	// FLAG Netscape
			}
		}
		else if( ( count == 12 ) &&
			( extlabel == GIF_PTxtExt ) )
		{
//     +---------------+
//  1  |               | 0 Text Grid Left Position Unsigned
//     +-             -+
//  2  |               | 1
//     +---------------+
//  3  |               | 2 Text Grid Top Position        Unsigned
//     +-             -+
//  4  |               | 3
//     +---------------+
//  5  |               | 4 Text Grid Width               Unsigned
//     +-             -+
//  6  |               | 5
//     +---------------+
//  7  |               | 6 Text Grid Height              Unsigned
//     +-             -+
//  8  |               | 7
//     +---------------+
//  9  |               | 8 Character Cell Width          Byte
//     +---------------+
// 10  |               | 9 Character Cell Height         Byte
//     +---------------+
// 11  |               | 10 Text Foreground Color Index   Byte
//     +---------------+
// 12  |               | 11 Text Background Color Index   Byte
//     +---------------+
			lpGIE->gceFlag |=	gie_PTE;	// Plain Text Extension
			lpGIE->giLeft = LM_to_uint(lpb[0],lpb[1]);// Left (logical) column of TEXT
			lpGIE->giTop  = LM_to_uint(lpb[2],lpb[3]);// Top (logical) row
			lpGIE->giGI.giWidth = LM_to_uint(lpb[4],lpb[5]);
			lpGIE->giGI.giHeight = LM_to_uint(lpb[6],lpb[7]);
			lpGIE->gceRes1 = (DWORD)(LM_to_uint(lpb[8],lpb[9]) & 0xffff); // Char CELL
			lpGIE->gceRes2 = (DWORD)(lpb[10] & 0xff);	// Foreground index
			lpGIE->gceIndex = lpb[11];	// Backgound INDEX
			lpGIE->gceSize += sizeof( PTEHDR );	// Room for 2 x DWORD header
		}
		else if( extlabel == GIF_CommExt )
		{
			lpGIE->gceFlag |=	gie_COM;	// Had Comment
			if( count > 8 )
				count = 8;
			lps = (LPSTR) &lpGIE->gceRes1;
			for( i = 0; i < count; i++ )
			{
				lps[i] = lpb[i];
			}
		}
		else
		{
			lpGIE->gceFlag |= gie_UNK;	// Undefined Extension
			if( count > 8 )
				count = 8;
			lps = (LPSTR) &lpGIE->gceRes1;
			for( i = 0; i < count; i++ )
			{
				lps[i] = lpb[i];
			}
		}
	}
	// Now process any subsequent BLOCKS
	if( count )	// IFF there WAS a FIRST BLOCK
	{	// Probably an ERROR if NO FIRST BLOCK!!!
		// DISCARD into BUFFER (on stack)
		while( count = ReadByte( cinfo ) )
		{
			lpGIE->gceSize += ( count + 1 );	// Keep accumulating SIZE if say TEXT
			if( !ReadOK(cinfo->input_file, buf, count) )
			{
				ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
			}
		}
	}
	lpGIE->gceSize++;	// Bump 1 for NUL termination
	return	extlabel;
}

#define	SETRGB(a,r,g,b)		\
{\
	a.rgbBlue     = b;\
	a.rgbGreen    = g;\
	a.rgbRed      = r;\
	a.rgbReserved = 0;\
}

WORD	Get_GIF_Ext( compress_info_ptr cinfo,
					LPGIFHDREXT lpGHE,
					WORD maxcnt )
{

	char hdrbuf[10];	// workspace for reading control blocks
	int gifcxmaplen, aspectRatio, gmaplen;
	int c;
	WORD icnt;
	LPGIFIMGEXT lpGIE;
	HGLOBAL		hgQuadColr;
	LPRGBQUAD	lpQuadColr;

	hgQuadColr = 0;
	lpQuadColr = 0;
	icnt = 0;
	lpGIE = &lpGHE->gheGIE[0];	// Point to FIRST
	lpGHE->gheImgCount = 0;
	lpGHE->gheBMPSize  = 0;
	lpGHE->gheCMSize   = 0;
	lpGHE->gheFlag     = 0;		// IN/OUT Options Flag - See Below
	SETRGB(lpGHE->gheBkGrnd,0,0,0);		// Background Colour
	lpGHE->gheRes1     = 0;		// Reserved
	lpGHE->gheRes2     = 0;		// ditto

	if( maxcnt )
	{
		lpGIE->giGI.giCMSize = 0;	// Always SET per the current COLOR MAP
		lpGIE->giGI.giWidth  = 0;
		lpGIE->giGI.giHeight = 0;
		lpGIE->giGI.giBMPSize = 0;
		lpGIE->gceFlag     = 0;	// IN/OUT Options Flag - See Below
		SETRGB(lpGIE->gceBkGrnd,0,0,0);	// Background Colour
		lpGIE->gceSize     = 0;	// Block Sizes (4 for TEXT, Bit for TEXT)
		lpGIE->gceRes1     = 0;	// Reserved
		lpGIE->gceRes2     = 0;	// ditto
	}

	ResetBMPSizes();
	gmaplen = 0;

	// Read and verify GIF Header.
	if( !ReadOK( cinfo->input_file, hdrbuf, 6 ) )
	{
		ERRFLAG( BAD_GIFNOT );	// "Not a GIF file"
		return( bErrFlag );
	}
	if (hdrbuf[0] != 'G' || hdrbuf[1] != 'I' || hdrbuf[2] != 'F')
	{
		ERRFLAG( BAD_GIFNOT );
		return( bErrFlag );
	}

	// Check for expected version numbers.
	// If unknown version, give warning and try to process anyway;
	// this is per recommendation in GIF89a standard.
	//
	//if ((hdrbuf[3] != '8' || hdrbuf[4] != '7' || hdrbuf[5] != 'a') &&
	//	(hdrbuf[3] != '8' || hdrbuf[4] != '9' || hdrbuf[5] != 'a'))
	//{
	//}
	// Read and decipher Logical Screen Descriptor.
	if( !ReadOK(cinfo->input_file, hdrbuf, 7) )
	{
		ERRFLAG( BAD_GIFEOF );	// "Premature EOF in GIF file"
		return( bErrFlag );
	}

	lpGHE->gheWidth = GIFwidth = LM_to_uint(hdrbuf[0],hdrbuf[1]);
	lpGHE->gheHeight = GIFheight = LM_to_uint(hdrbuf[2],hdrbuf[3]);
	// Add the EXTENDED Information
	lpGHE->gheBits = hdrbuf[4];		// packed field
	lpGHE->gheIndex = hdrbuf[5];	// Background Colour Index
	lpGHE->ghePAR   = hdrbuf[6];	// Pixel Aspect Ration

	gifcxmaplen = 2 << (hdrbuf[4] & 0x07);
	// We have copied the color resolution, sort flag,
	// and background color index
	aspectRatio = hdrbuf[6] & 0xFF;
//	if( aspectRatio != 0 && aspectRatio != 49 )
//	{
//	}
	// 24 May 1997 - We SET THE SIZE now.
	lpGHE->gheCMSize = SetBMPSizes( gifcxmaplen );
	ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
	lpGHE->gheBMPSize = ( sizeof( BITMAPINFOHEADER ) +
		BMPCols +	// RGBQUAD size times COLOUR COUNT
		ddRCnt );	// plus calculate DATA BLOCK size

	if( maxcnt == 0 )
	{
		goto	ExtGIFRet;
	}

	// Read GLOBAL colormap if header indicates it is present.
	if( BitSet(hdrbuf[4], COLORMAPFLAG) )
	{
		// HUH! Why possibly ALTER the gifcxmaplen
		//lpGH->ghCMSize = gifcxmaplen = SetBMPSizes( gifcxmaplen );
		// Sure, only set OUR returned SIZE, NOT the GIF length
		DWORD	dwCOLMAP, dwSA;
		//lpGHE->gheCMSize = SetBMPSizes( gifcxmaplen );
		// For GLOBAL Maps we must realise NOW
		// for later COLORREF to be obtained.
		dwSA = 0;
		if( dwCOLMAP = gifcxmaplen * sizeof( RGBQUAD ) )
			dwSA = (3 * 4);	// Actually 3 * 256 bytes color
		// BUT is an ARRAY of pointers, so ...

		if( ( dwCOLMAP ) &&
			( hgQuadColr = JGlobalAlloc( GHND, (dwCOLMAP+dwSA+(3*256)) ) ) &&
			( lpQuadColr = (LPRGBQUAD)JGlobalLock( hgQuadColr ) ) )
		{
			LPSTR	lpcmap, lpm;
			//JSAMPARRAY	cmap;
			LPDWORD	lpdw;
			lpm = (LPSTR)lpQuadColr;
			// Bump to CMAP header = to Pointers
			lpcmap = lpm + dwCOLMAP;
			lpdw = (LPDWORD)lpcmap;
			//cmap = (JSAMPARRAY)lpcmap;
			lpm = lpcmap + dwSA; 
			//cmap[CM_RED] = lpm;
			//lpdw[0] = (DWORD)lpm;
			lpdw[0] = 0;
			lpm += 256;
			//cmap[CM_GREEN] = lpm;
			//lpdw[1] = (DWORD)lpm;
			lpdw[1] = 0;
			lpm += 256;
			//cmap[CM_BLUE] = lpm;
			//lpdw[2] = (DWORD)lpm;
			lpdw[2] = 0;
			//ReadColorMap( cinfo, gifcxmaplen, colrmap );
			CopyColorMap( cinfo, gifcxmaplen,
				lpQuadColr,
				(JSAMPARRAY) lpcmap );
			// Keep this pointer - lpQuadColr[0] = any INDEX
			lpRGBMap = lpQuadColr;
		}
		else
		{
			SkipColorMap( cinfo, gifcxmaplen );
		}
		gmaplen = gifcxmaplen;	// Save GLOBAL MAP length
		//ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
		//lpGHE->gheBMPSize = ( sizeof( BITMAPINFOHEADER ) +
		//	BMPCols +	// RGBQUAD size times COLOUR COUNT
		//	ddRCnt );	// plus calculate DATA BLOCK size
	}
	ResetBMPSizes();
	/* Scan until we reach the END of the GIF! */
	for( ;; )
	{
		c = ReadByte(cinfo);
		if( c == ';' )		/* GIF terminator?? */
		{
			if( icnt == 0 )
			{
				ERRFLAG( BAD_GIFCNT ); /* "Too few images in GIF file" */
				return( bErrFlag );
			}
			else
			{
				break;	/* We have reached the END OF FILE ... */
			}
		}
		if (c == '!')
		{		/* Extension */
			if( maxcnt && 	/* If given multiple info blocks */
				( icnt < maxcnt ) )
			{
				lpGIE->gceExt = (BYTE)c;
				c = DoExtensionExt( cinfo, lpGIE, lpGHE, lpQuadColr );
			}
			else
			{
				c = DoExtension( cinfo );
			}
			switch( c )
			{
			case GIF_AppExt:	//              0xFF
				lpGHE->gheFlag |= ghf_AppExt;	// Had App Extension
				break;
			case GIF_CommExt:	//             0xFE
				lpGHE->gheFlag |= ghf_CommExt;	// Had Comment Extension
				break;
			case GIF_CtrlExt:	//             0xF9
				lpGHE->gheFlag |= ghf_CtrlExt;	// Had Graphic Control Extension
				break;
			case GIF_PTxtExt:	//             0x01
				lpGHE->gheFlag |= ghf_PTxtExt;	// Had plain text extension
				break;
			default:
				lpGHE->gheFlag |= ghf_UnknExt;	// Had an UNKNONW extension
				break;
			}
			continue;
    	}
		if( c != ',' )
		{		/* Not an image separator? */
			continue;
		}
		/* Read and decipher Local Image Descriptor */
		if( !ReadOK(cinfo->input_file, hdrbuf, 9) )
		{
			ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
			return( bErrFlag );
		}
		/* we ignore top/left position info, also sort flag */
		GIFwidth = LM_to_uint(hdrbuf[4],hdrbuf[5]);
		GIFheight = LM_to_uint(hdrbuf[6],hdrbuf[7]);
		is_interlaced = BitSet(hdrbuf[8], INTERLACE);
		/* Read local colormap if header indicates it is present */
		/* Note: if we wanted to support skipping images, */
		/* we'd need to skip rather than read colormap for ignored images */
		gifcxmaplen = 0;
		if( BitSet(hdrbuf[8], COLORMAPFLAG) ) 
		{
			gifcxmaplen = 2 << (hdrbuf[8] & 0x07);
			SetBMPSizes( gifcxmaplen );
			SkipColorMap( cinfo, gifcxmaplen );
		}
		if( maxcnt && 	/* If given multiple info blocks */
			( icnt < maxcnt ) )
		{
			lpGIE->giGI.giCMSize = SetBMPSizes( gmaplen );	/* Always SET per the current COLOR MAP */
			/* This can be the GLOBAL Color map, or the local colormap */
			lpGIE->giGI.giWidth = GIFwidth;
			lpGIE->giGI.giHeight = GIFheight;
			ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
			lpGIE->giGI.giBMPSize = ( sizeof( BITMAPINFOHEADER ) + BMPCols + ddRCnt );
			if( lpGIE->giGI.giBMPSize > lpGHE->gheBMPSize )
				lpGHE->gheBMPSize = lpGIE->giGI.giBMPSize; /* RETURN LARGEST in here */
			lpGIE++;
			if( (icnt + 1) < maxcnt )
			{	// Initialise the NEXT
				lpGIE->giGI.giCMSize = 0;	/* Always SET per the current COLOR MAP */
				lpGIE->giGI.giWidth  = 0;
				lpGIE->giGI.giHeight = 0;
				lpGIE->giGI.giBMPSize = 0;
				lpGIE->gceFlag     = 0;	// IN/OUT Options Flag - See Below
				SETRGB(lpGIE->gceBkGrnd,0,0,0);	// Background Colour
				lpGIE->gceSize     = 0;	// Block Size (4 for TEXT, Bit for TEXT)
				lpGIE->gceRes1     = 0;	// Reserved
				lpGIE->gceRes2     = 0;	// ditto
			}
		}

		input_code_size = ReadByte(cinfo); /* get minimum-code-size byte */
		if( (input_code_size < 2) ||
			(input_code_size >= MAX_LZW_BITS) )
		{
			ERRFLAG( BAD_GIFCS ); /* "Bogus codesize %d, input_code_size */
			return( bErrFlag );
		}
		/* Reached an image */
		/* call SkipDataBlocks and continue the loop */
		icnt++;
		lpGHE->gheImgCount++;	/* Bump return count ... */
		SkipDataBlocks(cinfo);
		ResetBMPSizes();
	}	/* Forever ... */

	if( hgQuadColr && lpQuadColr )
		JGlobalUnlock( hgQuadColr );
	if( hgQuadColr )
		JGlobalFree( hgQuadColr );
	lpRGBMap = 0;	// Now NO Global Color MAP Available

ExtGIFRet:

	/* Return info about the image. */
	cinfo->input_components = GNUMCOLORS;
	cinfo->in_color_space = CS_RGB;
	cinfo->image_width = GIFwidth;
	cinfo->image_height = GIFheight;
	cinfo->data_precision = 8;	/* always, even if 12-bit JSAMPLEs */

	return( bErrFlag );

}

// This MUST count "Plain Text Extension" as if they were
// IMAGES, and return the TOTAL DISPLAY COMPONENT COUNT.
// ======================================================
WORD	Get_GIF_ExtT( compress_info_ptr cinfo,
					LPGIFHDREXT lpGHE,
					WORD maxcnt )
{

	char hdrbuf[10];		/* workspace for reading control blocks */
	int gifcxmaplen, aspectRatio, gmaplen;
	int c;
	WORD icnt;
	LPGIFIMGEXT lpGIE;
	LPSTR	lpb;

	icnt = 0;
	lpGIE = &lpGHE->gheGIE[0];	// Point to FIRST
	lpGHE->gheImgCount = 0;
	lpGHE->gheBMPSize  = 0;
	lpGHE->gheCMSize   = 0;
	lpGHE->gheFlag     = 0;		// IN/OUT Options Flag - See Below
	SETRGB(lpGHE->gheBkGrnd,0,0,0);		// Background Colour
	lpGHE->gheRes1     = 0;		// Reserved
	lpGHE->gheRes2     = 0;		// ditto
	lpb = &code_buf[0];
	if( maxcnt )
	{
		lpGIE->giGI.giCMSize = 0;	/* Always SET per the current COLOR MAP */
		lpGIE->giGI.giWidth  = 0;
		lpGIE->giGI.giHeight = 0;
		lpGIE->giGI.giBMPSize = 0;
		lpGIE->gceFlag     = 0;	// IN/OUT Options Flag - See Below
		SETRGB(lpGIE->gceBkGrnd,0,0,0);	// Background Colour
		lpGIE->gceSize     = 0;	// Block Size (4 for TEXT, Bit for TEXT)
		lpGIE->gceRes1     = 0;	// Reserved
		lpGIE->gceRes2     = 0;	// ditto
	}
	ResetBMPSizes();
	gmaplen = 0;
  /* Read and verify GIF Header */
	if( !ReadOK( cinfo->input_file, hdrbuf, 6 ) )
	{
		ERRFLAG( BAD_GIFNOT );	/* "Not a GIF file" */
		return( bErrFlag );
	}
	if (hdrbuf[0] != 'G' || hdrbuf[1] != 'I' || hdrbuf[2] != 'F')
	{
		ERRFLAG( BAD_GIFNOT );
		return( bErrFlag );
	}
	/* Check for expected version numbers.
	 * If unknown version, give warning and try to process anyway;
	 * this is per recommendation in GIF89a standard.
	 */
	//if ((hdrbuf[3] != '8' || hdrbuf[4] != '7' || hdrbuf[5] != 'a') &&
	//	(hdrbuf[3] != '8' || hdrbuf[4] != '9' || hdrbuf[5] != 'a'))
	//{
	//}
	/* Read and decipher Logical Screen Descriptor */
	if( !ReadOK(cinfo->input_file, hdrbuf, 7) )
	{
		ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
		return( bErrFlag );
	}
	lpGHE->gheWidth = GIFwidth = LM_to_uint(hdrbuf[0],hdrbuf[1]);
	lpGHE->gheHeight = GIFheight = LM_to_uint(hdrbuf[2],hdrbuf[3]);
	// Add the EXTENDED Information
	lpGHE->gheBits = hdrbuf[4];		// packed field
	lpGHE->gheIndex = hdrbuf[5];	// Background Colour Index
	lpGHE->ghePAR   = hdrbuf[6];	// Pixel Aspect Ration

	gifcxmaplen = 2 << (hdrbuf[4] & 0x07);
	/* we ignore the color resolution, sort flag, and background color index */
	aspectRatio = hdrbuf[6] & 0xFF;
//	if( aspectRatio != 0 && aspectRatio != 49 )
//	{
//	}
	lpGHE->gheCMSize = SetBMPSizes( gifcxmaplen );
	ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
	lpGHE->gheBMPSize = ( sizeof( BITMAPINFOHEADER ) +
		BMPCols +	// RGBQUAD size times COLOUR COUNT
		ddRCnt );	// plus calculated DATA BLOCK size

	if( maxcnt == 0 )
	{
		goto ExTGIFRet;
	}
	/* Read GLOBAL colormap if header indicates it is present */
	if( BitSet(hdrbuf[4], COLORMAPFLAG) )
	{
		// HUH! Why possibly ALTER the gifcxmaplen
		//lpGH->ghCMSize = gifcxmaplen = SetBMPSizes( gifcxmaplen );
		// Sure, only set OUR returned SIZE, NOT the GIF length
		//lpGHE->gheCMSize = SetBMPSizes( gifcxmaplen );
    	SkipColorMap( cinfo, gifcxmaplen );
		gmaplen = gifcxmaplen;	// Save GLOBAL MAP length
		//ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
		//lpGHE->gheBMPSize = ( sizeof( BITMAPINFOHEADER ) +
		//	BMPCols +	// RGBQUAD size times COLOUR COUNT
		//	ddRCnt );	// plus calculated DATA BLOCK size
	}
	ResetBMPSizes();
	/* Scan until we reach the END of the GIF! */
	for( ;; )
	{
		c = ReadByte(cinfo);
		if( c == ';' )		/* GIF terminator?? */
		{
			if( icnt == 0 )
			{
				ERRFLAG( BAD_GIFCNT ); /* "Too few images in GIF file" */
				return( bErrFlag );
			}
			else
			{
				break;	/* We have reached the END OF FILE ... */
			}
		}
		if (c == '!')
		{		/* Extension */
			if( maxcnt && 	/* If given multiple info blocks */
				( icnt < maxcnt ) )
			{
				lpGIE->gceExt = (BYTE)c;
				c = DoExtensionExt( cinfo, lpGIE, lpGHE, 0 );
			}
			else
			{
				c = DoExtension( cinfo );
			}
			switch( c )
			{
			case GIF_AppExt:	//              0xFF
				lpGHE->gheFlag |= ghf_AppExt;	// Had App Extension
				break;
			case GIF_CommExt:	//             0xFE
				lpGHE->gheFlag |= ghf_CommExt;	// Had Comment Extension
				break;
			case GIF_CtrlExt:	//             0xF9
				lpGHE->gheFlag |= ghf_CtrlExt;	// Had Graphic Control Extension
				break;
			case GIF_PTxtExt:	//             0x01
				lpGHE->gheFlag |= ghf_PTxtExt;	// Had plain text extension
				if( maxcnt && 	/* If given multiple info blocks */
					( icnt < maxcnt ) )
				{
//     +---------------+
//  1  |               | 0 Text Grid Left Position Unsigned
//     +-             -+
//  2  |               | 1
//     +---------------+
//  3  |               | 2 Text Grid Top Position        Unsigned
//     +-             -+
//  4  |               | 3
//     +---------------+
//  5  |               | 4 Text Grid Width               Unsigned
//     +-             -+
//  6  |               | 5
//     +---------------+
//  7  |               | 6 Text Grid Height              Unsigned
//     +-             -+
//  8  |               | 7
//     +---------------+
//  9  |               | 8 Character Cell Width          Byte
//     +---------------+
// 10  |               | 9 Character Cell Height         Byte
//     +---------------+
// 11  |               | 10 Text Foreground Color Index   Byte
//     +---------------+
// 12  |               | 11 Text Background Color Index   Byte
//     +---------------+
//			lpGIE->gceFlag |=	gie_PTE;	// Plain Text Extension
//			lpGIE->giLeft = LM_to_uint(lpb[0],lpb[1]);// Left (logical) column of TEXT
//			lpGIE->giTop  = LM_to_uint(lpb[2],lpb[3]);// Top (logical) row
//			lpGIE->giGI.giWidth = LM_to_uint(lpb[4],lpb[5]);
//			lpGIE->giGI.giHeight = LM_to_uint(lpb[6],lpb[7]);
//			lpGIE->gceRes1 = (DWORD)LM_to_uint(lpb[8],lpb[9]); // Char CELL
//			lpGIE->gceRes2 = (DWORD)lpb[10];	// Foreground index
//			lpGIE->gceIndex = lpb[11];	// Backgound INDEX
//	DWORD	gceColr;	// COLORREF (if SET)
					//lpGIE->giGI.giCMSize = SetBMPSizes( gmaplen );	/* Always SET per the current COLOR MAP */
					/* This can be the GLOBAL Color map, or the local colormap */
					//lpGIE->giGI.giWidth = GIFwidth;
					//lpGIE->giGI.giHeight = GIFheight;
					//ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
					lpGIE->giGI.giBMPSize = lpGIE->gceSize;
					//if( lpGIE->giGI.giBMPSize > lpGHE->gheBMPSize )
						//lpGHE->gheBMPSize = lpGIE->giGI.giBMPSize; /* RETURN LARGEST in here */
					lpGIE++;
					if( (icnt + 1) < maxcnt )
					{	// Initialise the NEXT
						lpGIE->giGI.giCMSize = 0;	/* Always SET per the current COLOR MAP */
						lpGIE->giGI.giWidth  = 0;
						lpGIE->giGI.giHeight = 0;
						lpGIE->giGI.giBMPSize = 0;
						lpGIE->gceFlag     = 0;	// IN/OUT Options Flag - See Below
						SETRGB(lpGIE->gceBkGrnd,0,0,0);	// Background Colour
						lpGIE->gceSize     = 0;	// Block Size (4 for TEXT, Bit for TEXT)
						lpGIE->gceRes1     = 0;	// Reserved
						lpGIE->gceRes2     = 0;	// ditto
					}
				}
				icnt++;
				lpGHE->gheImgCount++;	/* Bump return count ... */
				break;
			default:
				lpGHE->gheFlag |= ghf_UnknExt;	// Had an UNKNONW extension
				break;
			}
			continue;
    	}
		if( c != ',' )
		{		/* Not an image separator? */
			continue;
		}
		/* Read and decipher Local Image Descriptor */
		if( !ReadOK(cinfo->input_file, hdrbuf, 9) )
		{
			ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
			return( bErrFlag );
		}
		/* we ignore top/left position info, also sort flag */
		// NO, we DO NOT IGNORE ANYTHING
		// =============================
		if( maxcnt && 	/* If given multiple info blocks */
			( icnt < maxcnt ) )
		{
// ===============
			//lpGIE->gceLabel = (BYTE)c;	// Image Descriptor = 0x2c
			//lpGIE->gceSize  = ??;	// Accumlate DATA SIZE
			//lpGIE->gceBits = lpb[0];	// packed field
			lpGIE->gceFlag |= gie_GID;	// Is Image Descriptor
			lpGIE->giLeft = LM_to_uint(hdrbuf[0],hdrbuf[1]);// Left (logical) column of TEXT
			lpGIE->giTop  = LM_to_uint(hdrbuf[2],hdrbuf[3]);// Top (logical) row
			lpGIE->giGI.giWidth = LM_to_uint(hdrbuf[4],hdrbuf[5]);
			lpGIE->giGI.giHeight = LM_to_uint(hdrbuf[6],hdrbuf[7]);
			lpGIE->giBits = hdrbuf[8];	// packed field
// ================
		}
		GIFwidth = LM_to_uint(hdrbuf[4],hdrbuf[5]);
		GIFheight = LM_to_uint(hdrbuf[6],hdrbuf[7]);
		is_interlaced = BitSet(hdrbuf[8], INTERLACE);
		/* Read local colormap if header indicates it is present */
		/* Note: if we wanted to support skipping images, */
		/* we'd need to skip rather than read colormap for ignored images */
		gifcxmaplen = 0;
		if( BitSet(hdrbuf[8], COLORMAPFLAG) ) 
		{
			gifcxmaplen = 2 << (hdrbuf[8] & 0x07);
			SetBMPSizes( gifcxmaplen );
			SkipColorMap( cinfo, gifcxmaplen );
		}
		if( maxcnt && 	/* If given multiple info blocks */
			( icnt < maxcnt ) )
		{
			lpGIE->giGI.giCMSize = SetBMPSizes( gmaplen );	/* Always SET per the current COLOR MAP */
			/* This can be the GLOBAL Color map, or the local colormap */
			lpGIE->giGI.giWidth = GIFwidth;
			lpGIE->giGI.giHeight = GIFheight;
			ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
			lpGIE->giGI.giBMPSize = ( sizeof( BITMAPINFOHEADER ) + BMPCols + ddRCnt );
			if( lpGIE->giGI.giBMPSize > lpGHE->gheBMPSize )
				lpGHE->gheBMPSize = lpGIE->giGI.giBMPSize; /* RETURN LARGEST in here */
			lpGIE++;
			if( (icnt + 1) < maxcnt )
			{	// Initialise the NEXT
				lpGIE->giGI.giCMSize = 0;	/* Always SET per the current COLOR MAP */
				lpGIE->giGI.giWidth  = 0;
				lpGIE->giGI.giHeight = 0;
				lpGIE->giGI.giBMPSize = 0;
				lpGIE->gceFlag     = 0;	// IN/OUT Options Flag - See Below
				SETRGB(lpGIE->gceBkGrnd,0,0,0);	// Background Colour
				lpGIE->gceSize     = 0;	// Block Size (4 for TEXT, Bit for TEXT)
				lpGIE->gceRes1     = 0;	// Reserved
				lpGIE->gceRes2     = 0;	// ditto
			}
		}

		input_code_size = ReadByte(cinfo); /* get minimum-code-size byte */
		if( (input_code_size < 2) ||
			(input_code_size >= MAX_LZW_BITS) )
		{
			ERRFLAG( BAD_GIFCS ); /* "Bogus codesize %d, input_code_size */
			return( bErrFlag );
		}
		/* Reached an image */
		/* call SkipDataBlocks and continue the loop */
		icnt++;
		lpGHE->gheImgCount++;	/* Bump return count ... */
		SkipDataBlocks(cinfo);
		ResetBMPSizes();
	}	/* Forever ... */

ExTGIFRet:
	// Return info about the image.
	cinfo->input_components = GNUMCOLORS;
	cinfo->in_color_space = CS_RGB;
	cinfo->image_width = GIFwidth;
	cinfo->image_height = GIFheight;
	cinfo->data_precision = 8;	/* always, even if 12-bit JSAMPLEs */
	//bErrFlag = BAD_NOTIMP;

	return( bErrFlag );

}

WORD	Get_GIF_Norm( compress_info_ptr cinfo, LPGIFHDR lpGH, WORD maxcnt )
{
	char hdrbuf[10];		/* workspace for reading control blocks */
	int gifcxmaplen, aspectRatio, gmaplen;
	int c;
	WORD icnt;
	LPGIFIMG lpGI;

	icnt = 0;
	lpGI = &lpGH->ghGifImg[0];	// Point to FIRST
	lpGH->ghImgCount = 0;
	lpGH->ghBMPSize  = 0;
	lpGH->ghCMSize   = 0;
	ResetBMPSizes();
	gmaplen = 0;
  /* Read and verify GIF Header */
	if( !ReadOK( cinfo->input_file, hdrbuf, 6 ) )
	{
		ERRFLAG( BAD_GIFNOT );	/* "Not a GIF file" */
		return( bErrFlag );
	}
	if (hdrbuf[0] != 'G' || hdrbuf[1] != 'I' || hdrbuf[2] != 'F')
	{
		ERRFLAG( BAD_GIFNOT );
		return( bErrFlag );
	}
	/* Check for expected version numbers.
	 * If unknown version, give warning and try to process anyway;
	 * this is per recommendation in GIF89a standard.
	 */
	//if ((hdrbuf[3] != '8' || hdrbuf[4] != '7' || hdrbuf[5] != 'a') &&
	//	(hdrbuf[3] != '8' || hdrbuf[4] != '9' || hdrbuf[5] != 'a'))
	//{
	//}
	/* Read and decipher Logical Screen Descriptor */
	if( !ReadOK(cinfo->input_file, hdrbuf, 7) )
	{
		ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
		return( bErrFlag );
	}
	lpGH->ghWidth = GIFwidth = LM_to_uint(hdrbuf[0],hdrbuf[1]);
	lpGH->ghHeight = GIFheight = LM_to_uint(hdrbuf[2],hdrbuf[3]);
	gifcxmaplen = 2 << (hdrbuf[4] & 0x07);
	/* we ignore the color resolution, sort flag, and background color index */
	aspectRatio = hdrbuf[6] & 0xFF;
//	if( aspectRatio != 0 && aspectRatio != 49 )
//	{
//	}
	/* Read GLOBAL colormap if header indicates it is present */
	if( BitSet(hdrbuf[4], COLORMAPFLAG) )
	{
		// HUH! Why possibly ALTER the gifcxmaplen
		//lpGH->ghCMSize = gifcxmaplen = SetBMPSizes( gifcxmaplen );
		// Sure, only set OUR returned SIZE, NOT the GIF length
		lpGH->ghCMSize = SetBMPSizes( gifcxmaplen );
    	SkipColorMap( cinfo, gifcxmaplen );
		gmaplen = gifcxmaplen;	// Save GLOBAL MAP length
		ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
		lpGH->ghBMPSize = ( sizeof( BITMAPINFOHEADER ) +
			BMPCols +	// RGBQUAD size times COLOUR COUNT
			ddRCnt );	// plus calculate DATA BLOCK size
	}
	ResetBMPSizes();
	/* Scan until we reach start of desired image. */
	for( ;; )
	{
		c = ReadByte(cinfo);
		if( c == ';' )		/* GIF terminator?? */
		{
			if( icnt == 0 )
			{
				ERRFLAG( BAD_GIFCNT ); /* "Too few images in GIF file" */
				return( bErrFlag );
			}
			else
			{
				break;	/* We have reached the END OF FILE ... */
			}
		}
		if (c == '!')
		{		/* Extension */
      		DoExtension(cinfo);
      		continue;
    	}
		if( c != ',' )
		{		/* Not an image separator? */
			continue;
		}
		/* Read and decipher Local Image Descriptor */
		if( !ReadOK(cinfo->input_file, hdrbuf, 9) )
		{
			ERRFLAG( BAD_GIFEOF );	/* "Premature EOF in GIF file" */
			return( bErrFlag );
		}
		/* we ignore top/left position info, also sort flag */
		GIFwidth = LM_to_uint(hdrbuf[4],hdrbuf[5]);
		GIFheight = LM_to_uint(hdrbuf[6],hdrbuf[7]);
		is_interlaced = BitSet(hdrbuf[8], INTERLACE);
		/* Read local colormap if header indicates it is present */
		/* Note: if we wanted to support skipping images, */
		/* we'd need to skip rather than read colormap for ignored images */
		gifcxmaplen = 0;
		if( BitSet(hdrbuf[8], COLORMAPFLAG) ) 
		{
			gifcxmaplen = 2 << (hdrbuf[8] & 0x07);
			SetBMPSizes( gifcxmaplen );
			gmaplen = gifcxmaplen;
			SkipColorMap( cinfo, gifcxmaplen );
		}
		if( maxcnt && 	/* If given multiple info blocks */
			( icnt < maxcnt ) )
		{
			lpGI->giCMSize = SetBMPSizes( gmaplen );	/* Always SET per the current COLOR MAP */
			/* This can be the GLOBAL Color map, or the local colormap */
			lpGI->giWidth = GIFwidth;
			lpGI->giHeight = GIFheight;
			ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
			lpGI->giBMPSize = ( sizeof( BITMAPINFOHEADER ) + BMPCols + ddRCnt );
			if( lpGI->giBMPSize > lpGH->ghBMPSize )
				lpGH->ghBMPSize = lpGI->giBMPSize; /* RETURN LARGEST in here */
			lpGI++;
		}

		input_code_size = ReadByte(cinfo); /* get minimum-code-size byte */
		if( (input_code_size < 2) ||
			(input_code_size >= MAX_LZW_BITS) )
		{
			ERRFLAG( BAD_GIFCS ); /* "Bogus codesize %d, input_code_size */
			return( bErrFlag );
		}
		/* Reached desired image, so break out of loop */
		/* If we wanted to skip this image, */
		/* we'd call SkipDataBlocks and then continue the loop */
		icnt++;
		lpGH->ghImgCount++;	/* Bump return count ... */
		SkipDataBlocks(cinfo);
		ResetBMPSizes();
		gmaplen = lpGH->ghCMSize;	/* Aways RESET to GLOBAL Size ... */
	}	/* Forever ... */

	/* Return info about the image. */
	cinfo->input_components = GNUMCOLORS;
	cinfo->in_color_space = CS_RGB;
	cinfo->image_width = GIFwidth;
	cinfo->image_height = GIFheight;
	cinfo->data_precision = 8;	/* always, even if 12-bit JSAMPLEs */

	return( bErrFlag );

}


GLOBAL	WORD
get_gif_sizeX( compress_info_ptr cinfo, LPGIFHDR lpGH )
{
	WORD		maxcnt;
	LPGIFHDREXT	lpGHE;

	maxcnt = lpGH->ghMaxCount;	// Extract the MAX COUNT ... Can be 0!
	if( maxcnt & gie_Flag )
	{
		lpGHE = (LPGIFHDREXT)lpGH;
		maxcnt = (maxcnt & ~(gie_Flag));
		lpGHE->gheImgCount = 0;
		lpGHE->gheBMPSize  = 0;
		lpGHE->gheCMSize   = 0;
		Get_GIF_Ext( cinfo, lpGHE, maxcnt );
	}
	else
	{
		lpGH->ghImgCount = 0;
		lpGH->ghBMPSize  = 0;
		lpGH->ghCMSize   = 0;
		Get_GIF_Norm( cinfo, lpGH, maxcnt );
	}
	return( bErrFlag );
}

GLOBAL	WORD
get_gif_sizeXT( compress_info_ptr cinfo, LPGIFHDREXT lpGHE )
{
	WORD		maxcnt;

	maxcnt = lpGHE->gheMaxCount;	/* Extract the MAX COUNT ... */
	if( maxcnt & gie_Flag )
	{
		maxcnt = (maxcnt & ~(gie_Flag));
		lpGHE->gheImgCount = 0;
		lpGHE->gheBMPSize  = 0;
		lpGHE->gheCMSize   = 0;
		Get_GIF_ExtT( cinfo, lpGHE, maxcnt );
	}
	else
	{
		bErrFlag = BAD_NOTIMP;
	}
	//bErrFlag = BAD_NOTIMP;
	return( bErrFlag );
}

/* ==========================================================
Just for information of an ERROR in the first Library release
GLOBAL	WORD
get_gif_sizeXOld( compress_info_ptr cinfo, LPGIFHDR lpGH )
{
	char hdrbuf[10];	// workspace for reading control blocks
	int gifcxmaplen, aspectRatio, gmaplen;
	int c;
	WORD icnt;
	LPGIFIMG lpGI;
	LPGIFHDREXT	lpGHE;
	LPGIFIMGEXT	lpGIE;

	WORD maxcnt;

	icnt = 0;

	maxcnt = lpGH->ghMaxCount;	// Extract the MAX COUNT

	if( maxcnt & gie_Flag )
	{
		lpGHE = (LPGIFHDREXT)lpGH;
		lpGIE = &lpGHE->gheGIE[0];
		maxcnt = (maxcnt & ~(gie_Flag));
		lpGHE->gheImgCount = 0;
		lpGHE->gheBMPSize  = 0;
		lpGHE->gheCMSize   = 0;
	}
	else
	{
		lpGI = &lpGH->ghGifImg[0];
		lpGH->ghImgCount = 0;
		lpGH->ghBMPSize  = 0;
		lpGH->ghCMSize   = 0;
	}
	ResetBMPSizes();
	gmaplen = 0;
	// Read and verify GIF Header
	if( !ReadOK( cinfo->input_file, hdrbuf, 6 ) )
	{
		ERRFLAG( BAD_GIFNOT );	// "Not a GIF file"
		return( bErrFlag );
	}
	if (hdrbuf[0] != 'G' || hdrbuf[1] != 'I' || hdrbuf[2] != 'F')
	{
		ERRFLAG( BAD_GIFNOT );
		return( bErrFlag );
	}
	// Check for expected version numbers.
	// If unknown version, give warning and try to process anyway;
	// this is per recommendation in GIF89a standard.
	// 
//  if ((hdrbuf[3] != '8' || hdrbuf[4] != '7' || hdrbuf[5] != 'a') &&
//      (hdrbuf[3] != '8' || hdrbuf[4] != '9' || hdrbuf[5] != 'a'))
//	{
//	}
	// Read and decipher Logical Screen Descriptor
	if( !ReadOK(cinfo->input_file, hdrbuf, 7) )
	{
		ERRFLAG( BAD_GIFEOF );	// "Premature EOF in GIF file"
		return( bErrFlag );
	}
	lpGH->ghWidth = GIFwidth = LM_to_uint(hdrbuf[0],hdrbuf[1]);
	lpGH->ghHeight = GIFheight = LM_to_uint(hdrbuf[2],hdrbuf[3]);
	gifcxmaplen = 2 << (hdrbuf[4] & 0x07);

	// we ignore the color resolution, sort flag,
	// and background color index
	aspectRatio = hdrbuf[6] & 0xFF;
//	if( aspectRatio != 0 && aspectRatio != 49 )
//	{
//	}
	// Read GLOBAL colormap if header indicates it is present
	if( BitSet(hdrbuf[4], COLORMAPFLAG) )
	{
		// HUH! Why possibly ALTER the gifcxmaplen
		//lpGH->ghCMSize = gifcxmaplen = SetBMPSizes( gifcxmaplen );
		// Sure, only set OUR returned SIZE, NOT the GIF length
		lpGH->ghCMSize = SetBMPSizes( gifcxmaplen );
    	SkipColorMap( cinfo, gifcxmaplen );
		gmaplen = gifcxmaplen;	// Save GLOBAL MAP length
		ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
		lpGH->ghBMPSize = ( sizeof( BITMAPINFOHEADER ) +
			BMPCols +	// RGBQUAD size times COLOUR COUNT
			ddRCnt );	// plus calculate DATA BLOCK size
	}
	ResetBMPSizes();
	// Scan until we reach start of desired image
	for( ;; )
	{
		c = ReadByte(cinfo);
		if( c == ';' )		// GIF terminator??
		{
			if( icnt == 0 )
			{
				ERRFLAG( BAD_GIFCNT ); // "Too few images in GIF file"
				return( bErrFlag );
			}
			else
			{
				break;	// We have reached the END OF FILE ...
			}
		}
		if (c == '!')
		{		// Extension
      		DoExtension(cinfo);
      		continue;
    	}
		if( c != ',' )
		{		// Not an image separator?
			continue;
		}
		// Read and decipher Local Image Descriptor.
		if( !ReadOK(cinfo->input_file, hdrbuf, 9) )
		{
			ERRFLAG( BAD_GIFEOF );	// "Premature EOF in GIF file"
			return( bErrFlag );
		}
		// we ignore top/left position info, also sort flag
		GIFwidth = LM_to_uint(hdrbuf[4],hdrbuf[5]);
		GIFheight = LM_to_uint(hdrbuf[6],hdrbuf[7]);
		is_interlaced = BitSet(hdrbuf[8], INTERLACE);

		// Read local colormap if header indicates it is present
		// Note: if we wanted to support skipping images,
		// we'd need to skip rather than read colormap for ignored images
		gifcxmaplen = 0;
		if( BitSet(hdrbuf[8], COLORMAPFLAG) ) 
		{
			gifcxmaplen = 2 << (hdrbuf[8] & 0x07);
			SetBMPSizes( gifcxmaplen );
			gmaplen = gifcxmaplen;
			SkipColorMap( cinfo, gifcxmaplen );
		}
		if( maxcnt && 	// If given multiple info blocks
			( icnt < maxcnt ) )
		{
			lpGI->giCMSize = SetBMPSizes( gmaplen );	// Always SET per the current COLOR MAP
			// This can be the GLOBAL Color map, or the local colormap
			lpGI->giWidth = GIFwidth;
			lpGI->giHeight = GIFheight;
			ddRCnt = GetDataSize( GIFwidth, GIFheight, BMPBits );
			lpGI->giBMPSize = ( sizeof( BITMAPINFOHEADER ) + BMPCols + ddRCnt );
			if( lpGI->giBMPSize > lpGH->ghBMPSize )
				lpGH->ghBMPSize = lpGI->giBMPSize; // RETURN LARGEST in here
			lpGI++;
		}
		input_code_size = ReadByte(cinfo); // get minimum-code-size byte
		if(input_code_size < 2 || input_code_size >= MAX_LZW_BITS)
		{
	      ERRFLAG( BAD_GIFCS ); // "Bogus codesize %d, input_code_size
			return( bErrFlag );
		}
		// Reached desired image, so break out of loop
		// If we wanted to skip this image,
		// we'd call SkipDataBlocks and then continue the loop
		icnt++;
		lpGH->ghImgCount++;	// Bump return count ...
		SkipDataBlocks(cinfo);
		ResetBMPSizes();
		gmaplen = lpGH->ghCMSize;	// Aways RESET to GLOBAL Size ...
	}	// Forever ...

	// Return info about the image.
	cinfo->input_components = GNUMCOLORS;
	cinfo->in_color_space = CS_RGB;
	cinfo->image_width = GIFwidth;
	cinfo->image_height = GIFheight;
	cinfo->data_precision = 8;	// always, even if 12-bit JSAMPLEs
	
	return( bErrFlag );
}
Above is just for information of an error!!!
================================================================ */

#endif	// _INC_WINDOWS & DIRGIFBMP


/*
 * The method selection routine for GIF format input.
 * Note that this must be called by the user interface before calling
 * jpeg_compress.  If multiple input formats are supported, the
 * user interface is responsible for discovering the file format and
 * calling the appropriate method selection routine.
 */

GLOBAL void
jselrgif (compress_info_ptr cinfo)
{
  cinfo->methods->input_init = gif_input_init;
  cinfo->methods->get_input_row = gif_get_input_row; /* assume uninterlaced */
  cinfo->methods->input_term = gif_input_term;
}

#if	(defined( _INC_WINDOWS ) && defined( DIRGIFBMP ))
GLOBAL void
jselrgifn( compress_info_ptr cinfo, WORD num )
{
  cinfo->methods->input_init = gif_input_initn;
  cinfo->methods->get_input_row = gif_get_input_row; /* assume uninterlaced */
  cinfo->methods->input_term = gif_input_term;
	if( num )
		wGetICnt = num - 1;	/* Set the LOGICAL GET number... */
	else
		wGetICnt = 0;			/* Default to FIRST image ... */
}

GLOBAL void
jselrgifx( compress_info_ptr cinfo, WORD num )
{
	cinfo->methods->input_init = gif_input_initx;
	cinfo->methods->get_input_row = gif_get_input_row; /* assume uninterlaced */
	cinfo->methods->input_term = gif_input_term;
	wGetICnt = num;
}

int SetBMPSizes( int cmlen )
{
	int	cmret;
	cmret = cmlen;
	switch( BMPBits = GetBitCount( cmlen ) )
	{
	case 1:
		BMPCols = 2 * sizeof(RGBQUAD);
		cmret = 2;
		break;
	case 4:
		BMPCols = 16 * sizeof(RGBQUAD);
		cmret = 16;
		break;
	case 8:
		BMPCols = 256 * sizeof(RGBQUAD);
		cmret = 256;
		break;
	case 24:
		BMPCols = 0;	/* 24-Bit TRUE Color = NO COLOR TABLE */
		cmret = 0;
		break;
	default:
		BMPCols = 256 * sizeof(RGBQUAD);
		cmret = 256;
		break;
	}
	BMPCCnt = cmret;
	return( cmret );
}

void	ResetBMPSizes( void )
{
	if( hgColors && lpColors )
		JGlobalUnlock( hgColors );
	lpColors = 0;
	if( hgColors )
		JGlobalFree( hgColors );
	hgColors = 0;
	if( hgData && lpData )
		JGlobalUnlock( hgData );
	lpData = 0;
	if( hgData )
		JGlobalFree( hgData );
	hgData = 0;
	rgbColors = 0;
	hgColors = 0;
	hgData = 0;
	BMPCols = 0;
	BMPCCnt = 0;
//	BMPBits = 0;
	ddDCnt = 0;
	ddTCnt = 0;
	ddRCnt = 0;
}
#endif	/* _INC_WINDOWS and DIRGIFMAP */

#endif /* GIF_SUPPORTED */

/* eof wrdgif.c - Part of PD code */
