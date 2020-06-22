
/*
 * wrdbmp.c
 *
 * NO real Copyright - BMP Files - a Windows agreed format
 *
 * This file contains routines to read input images in BMP format.
 *
 * These routines are invoked via the methods bmp_get_input_row
 * and bmp_input_init/term. But since JPEG requires the BIT data
 * with the first ROW first, that is an Inversion of the BMP
 * where the "origin of the bitmap is the lower-left corner (V3 Pg237)
 * then the bmp_get_input_row is changed
 * to load_compressed_image, which then causes successive calls to
 * get_compressed_row. These WERE originally intended to handle
 * Run Length encoded BITMAPS, but that feature has not yet been
 * added.
 *
 * NOTE: Some work has been done to also include IBM OS/2 bitmaps,
 * but this work is NOT complete ...
 *
 * Paris, France            23 February, 1996     Geoff R. McLane
 *
 */

#include "winclude.h"

#ifdef BMP_SUPPORTED		/* Support for BOTH READ and WRITE BMP */

extern struct External_methods_struct e_methods;
extern BOOL	IsDIB;		/* We can use a DIB or BMP file ... */
#define	MAXCOLORMAPSIZE	256	/* max # of colors in a BMP colormap */

#define JNUMCOLORS	3	/* Only pass RGB colors to JPEG */
#define CM_RED		0	/* color component numbers */
#define CM_GREEN	1
#define CM_BLUE	2
#ifndef DV322
#define CM_NONE	3	/* Not used. Should be 0 ... */
#endif // #ifndef DV322

#define	MXCOLS	4	/* Presently only work with RGBQUAD, but ... */

/* *** FROM *** */
/* ============ */
/* BITMAP File is in RGBQUAD style (in MS Windows), like */
//LPRGBQUAD	aColorMap[MAXCOLORMAPSIZE];
//    BYTE    rgbBlue;
//    BYTE    rgbGreen;
//    BYTE    rgbRed;
//    BYTE    rgbReserved;
/* ========== */
/* *** TO *** */
/* ========== */
/* Near pointer to Array of Far pointer to pointing to each COLOR ... */
static JSAMPARRAY colormap;
/* colormap[i][j] = value of i'th color component for pixel value j */

#define	ReadOK(file,buffer,len)	(JFREAD(file,buffer,len) == ((size_t) (len)))

/* Static state for image processing */
static boolean is_compressed;	/* TRUE if have compressed image */
static big_sarray_ptr whole_image;	/* full image in compressed order */
static long cur_row_number;	/* need to know actual row number */
static DWORD	RowCnt;	/* A 2nd counter to the above READ and ACCESS */

/* ================================================================== */
BOOL	Inv_Image = TRUE;	/* JPEG desires the last row first ... so read */
/* whole file into an array of FAR pointers. Each containing a ROW */
/* from the file. Then they can be accessed in 'reverse' order ... */
/* ================================================================= */

BOOL	Inv_Color = FALSE;	/* Just a possible option items ... */

WORD wBMcolormaplen = 0;

/* Forward declarations */
METHODDEF void load_compressed_image PP((compress_info_ptr cinfo, JSAMPARRAY pixel_row));
METHODDEF void get_compressed_row PP((compress_info_ptr cinfo, JSAMPARRAY pixel_row));
extern DWORD get_file_size( void );

HWND	hwndMain = 0;

DWORD	ReadSize;

/* Just some DEBUG traps only ... */
void	chkei( void )
{
int i;
	i = 0;
}
void	chkie( void )
{
int i;
	i = 0;
}

#ifdef	_INC_WINDOWS

LOCAL int
ReadByte (compress_info_ptr cinfo)
/* Read next byte from BMP file */
{
  register HFILE infile = cinfo->input_file;
  int c;
	if( (c = getc(infile)) == EOF )
	{
		ERRFLAG( BAD_BMPEOF );	/* "Premature EOF in BMP file" */
	}
	ReadSize++;	/* Bump the file offset by this byte ... */
return c;
}

#else	/* !_INC_WINDOWS */

LOCAL int
ReadByte (compress_info_ptr cinfo)
/* Read next byte from BMP file */
{
  register FILE * infile = cinfo->input_file;
  int c;

  if ((c = getc(infile)) == EOF)
    ERREXIT(cinfo->emethods, "Premature EOF in BMP file");
	ReadSize++;	/* Bump the file offset by this byte ... */
  return c;
}
#endif	/* _INC_WINDOWS y /n */

LOCAL void
BMPReadColorMap (compress_info_ptr cinfo, int cmaplen, JSAMPARRAY cmap,
	DWORD Cols )
/* Read a BMP colormap */
{
	int i;
	if( Inv_Color )
	{
		for( i = 0; i < cmaplen; i++ )
		{
			cmap[CM_RED][i]  = (JSAMPLE) ReadByte(cinfo);
			cmap[CM_GREEN][i] = (JSAMPLE) ReadByte(cinfo);
			cmap[CM_BLUE][i]   = (JSAMPLE) ReadByte(cinfo);
			ReadByte(cinfo);
		}
	}
	else
	{
		for( i = 0; i < cmaplen; i++ )
		{	/* Note RGB (file) order is BLUE, GREEN, RED, Resv ... */
			cmap[CM_BLUE][i]  = (JSAMPLE) ReadByte(cinfo);
			cmap[CM_GREEN][i] = (JSAMPLE) ReadByte(cinfo);
			cmap[CM_RED][i]   = (JSAMPLE) ReadByte(cinfo);
			ReadByte(cinfo);
		}
	}
}



/* ====================================================================
 *
 * Read the file header; return image size and component count.
 * BITMAPFILEHEADER
 * BITMAPINFOHEADER
 * RGBQUAD[NUMCOLS]	If 1, 4, 8. If 24 colors then none ...
 * BYTE[Bits]
 * ===================================================================== */

#define	MXBMPBUF	(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))
char BMPhdrbuf[MXBMPBUF];	/* workspace for BITMAP Header */

UINT16 bmWidth, bmHeight;		/* image dimensions */
DWORD	bmDWidth;
WORD bmPlanes, bmCount;
DWORD	bmComp;

//---------------------------------------------------------------------
//
// Function:   DIBNumColors
//
// Purpose:    Given a pointer to a DIB, returns a number of colors in
//             the DIB's color table.
//
// Parms:      lpbi == pointer to DIB header (either BITMAPINFOHEADER
//                       or BITMAPCOREHEADER)
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------
METHODDEF WORD 
DIBNumColors( LPSTR lpbi )
{
   WORD wBitCount;
   WORD wNCols;

      // If this is a Windows style DIB, the number of colors in the
      //  color table can be less than the number of bits per pixel
      //  allows for (i.e. lpbi->biClrUsed can be set to some value).
      //  If this is the case, return the appropriate value.

   if (IS_WIN30_DIB (lpbi))
	{
      DWORD dwClrUsed;
      dwClrUsed = ((LPBITMAPINFOHEADER) lpbi)->biClrUsed;
		if( dwClrUsed )
		{
			wNCols = (WORD) dwClrUsed;
			goto Ret_Cols;
		}
	}


      // Calculate the number of colors in the color table based on
      //  the number of bits per pixel for the DIB.

   if (IS_WIN30_DIB (lpbi))
      wBitCount = ((LPBITMAPINFOHEADER) lpbi)->biBitCount;
   else
      wBitCount = ((LPBITMAPCOREHEADER) lpbi)->bcBitCount;

	switch( wBitCount )
	{
		case 1:	/* Monochrome - 2 entries - In BMP 0 = 1st 1 = 2nd */
			wNCols = 2;
			break;
		case 4:	/* 16 Colors - Each 4-bit in BMP is mapped to color */
			wNCols = 16;
			break;
		case 8:	/* 256 colors - Each BYTE in the BMP is mapped to a color */
			wNCols = 256;
			break;
		case 24: /* 2^24th colors. If bmiColors = 0 RGB BMP */
			wNCols =((WORD) -1);
			break;
		default:
			wNCols = 0;
			break;
      }

Ret_Cols:
return( wNCols );
}

//---------------------------------------------------------------------
//
// Function:   PaletteSize
//
// Purpose:    Given a pointer to a DIB, returns number of bytes
//             in the DIB's color table.
//
// Parms:      lpbi == pointer to DIB header (either BITMAPINFOHEADER
//                       or BITMAPCOREHEADER)
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

METHODDEF WORD
PaletteSize( LPSTR lpbi )
{
WORD	nc, rnc;
	nc = DIBNumColors( lpbi );
	if( nc == (WORD) -1 )
		nc = 0;
   if (IS_WIN30_DIB (lpbi))
      rnc = nc * sizeof( RGBQUAD );
   else
      rnc = nc * sizeof( RGBTRIPLE );
return( rnc );
}

METHODDEF WORD
ColorSize( LPSTR lpbi )
{
   if (IS_WIN30_DIB (lpbi))
      return (sizeof (RGBQUAD));
   else
      return (sizeof (RGBTRIPLE));
}


//---------------------------------------------------------------------
//
// Function:   FindDIBBits
//
// Purpose:    Given a pointer to a DIB, returns a pointer to the
//             DIB's bitmap bits.
//
// Parms:      lpbi == pointer to DIB header (either BITMAPINFOHEADER
//                       or BITMAPCOREHEADER)
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------
METHODDEF LPSTR 
FindDIBBits (LPSTR lpbi)
{
   return (lpbi + *(LPDWORD)lpbi + PaletteSize (lpbi));
}


//---------------------------------------------------------------------
//
// Function:   DIBHeight
//
// Purpose:    Given a pointer to a DIB, returns its height.  Note
//             that it returns a DWORD (since a Win30 DIB can have
//             a DWORD in its height field), but under Win30, the
//             high order word isn't used!
//
// Parms:      lpDIB == pointer to DIB header (either BITMAPINFOHEADER
//                       or BITMAPCOREHEADER)
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

METHODDEF DWORD
DIBHeight (LPSTR lpDIB)
{
   LPBITMAPINFOHEADER lpbmi;
   LPBITMAPCOREHEADER lpbmc;

   lpbmi = (LPBITMAPINFOHEADER) lpDIB;
   lpbmc = (LPBITMAPCOREHEADER) lpDIB;

   if (lpbmi->biSize == sizeof (BITMAPINFOHEADER))
      return lpbmi->biHeight;
   else
      return (DWORD) lpbmc->bcHeight;
}



//---------------------------------------------------------------------
//
// Function:   DIBWidth
//
// Purpose:    Given a pointer to a DIB, returns its width.  Note
//             that it returns a DWORD (since a Win30 DIB can have
//             a DWORD in its width field), but under Win30, the
//             high order word isn't used!
//
// Parms:      lpDIB == pointer to DIB header (either BITMAPINFOHEADER
//                       or BITMAPCOREHEADER)
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

METHODDEF DWORD 
DIBWidth (LPSTR lpDIB)
{
   LPBITMAPINFOHEADER lpbmi;
   LPBITMAPCOREHEADER lpbmc;

   lpbmi = (LPBITMAPINFOHEADER) lpDIB;
   lpbmc = (LPBITMAPCOREHEADER) lpDIB;

   if (lpbmi->biSize == sizeof (BITMAPINFOHEADER))
      return lpbmi->biWidth;
   else
      return (DWORD) lpbmc->bcWidth;
}


METHODDEF boolean
bmp_input_init (compress_info_ptr cinfo)
{
	int	b;
  BYTE c;
  WORD	coff;
  LPBITMAPFILEHEADER lpbmp;
  LPBITMAPINFOHEADER	lpbmi;
	DWORD	bmOffset;
	DWORD szArr, szCols;
	is_compressed = FALSE;
	if( IsDIB )
		szArr = sizeof( BITMAPINFOHEADER );
	else
		szArr = MXBMPBUF;
  /* Read and verify BMP FILE Header and INFO Header */
	if( get_file_size() < szArr )
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_BMPSIZ );	/* "Error: File too small. Not a BMP file!" */
		goto bmpinpret;
#else
		ERREXIT(cinfo->emethods, "Error: File too small. Not a BMP file!");
#endif
	}

	if( !ReadOK(cinfo->input_file, BMPhdrbuf, LOWORD( szArr )) )
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_BMPREAD );	/* "Error: Bad read of input file!" */
		goto bmpinpret;
#else
		ERREXIT(cinfo->emethods, "Error: Bad read of input file!");
#endif
	}
	ReadSize = szArr;
	if( IsDIB )
	{
		lpbmi = (LPBITMAPINFOHEADER) &BMPhdrbuf[0];
		bmOffset = ((DWORD) FindDIBBits((LPSTR) lpbmi ) - 
			(DWORD) lpbmi) ;
	}
	else	/* !IsDIB - ie it is a BITMAP FILE */
	{
		lpbmp = (LPBITMAPFILEHEADER) &BMPhdrbuf[0];
		lpbmi = (LPBITMAPINFOHEADER) &BMPhdrbuf[sizeof(BITMAPFILEHEADER)];
		if( get_file_size() != lpbmp->bfSize )
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_BMPSIZ2 );	/* "Error: Conflict in SIZE. Not a BMP file!" */
			goto bmpinpret;
#else
			ERREXIT(cinfo->emethods, "Error: Conflict in SIZE. Not a BMP file!");
#endif
		}
		if( lpbmp->bfReserved1 || lpbmp->bfReserved2 )
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_BMPRES1 );	/* "Error: Reserved items NOT zero. Not a BMP file!" */
			goto bmpinpret;
#else
			ERREXIT(cinfo->emethods, "Error: Reserved items NOT zero. Not a BMP file!");
#endif
//    UINT    bfType;
//    DWORD   bfSize;
//    UINT    bfReserved1;
//    UINT    bfReserved2;
//    DWORD   bfOffBits;
//} BITMAPFILEHEADER;
		}
		coff = 0;
		c = BMPhdrbuf[coff];
		if( BMPhdrbuf[0] != 'B' || BMPhdrbuf[1] != 'M' )
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_BMPBM );	/* "Error: No BM signature! Not a BMP file!" */
			goto bmpinpret;
#else
			ERREXIT(cinfo->emethods, "ERROR: No BM signature! Not a BMP file!");
#endif
		}
		bmOffset = ((DWORD) FindDIBBits((LPSTR) lpbmi ) - 
			(DWORD) lpbmi) + 
			sizeof(BITMAPFILEHEADER);
		bmOffset = lpbmp->bfOffBits;
	}	/* IsDIB y/n */
	szArr = MAXCOLORMAPSIZE;
//    DWORD   biSize;
//    LONG    biWidth;
//    LONG    biHeight;
//    WORD    biPlanes;
//    WORD    biBitCount;
//    DWORD   biCompression;
//    DWORD   biSizeImage;
//    LONG    biXPelsPerMeter;
//    LONG    biYPelsPerMeter;
//    DWORD   biClrUsed;
//    DWORD   biClrImportant;
//} BITMAPINFOHEADER;
  bmWidth = (UINT16) lpbmi->biWidth;
  bmHeight = (UINT16) lpbmi->biHeight;
  bmWidth = (UINT16) DIBWidth((LPSTR) lpbmi );
  bmHeight = (UINT16) DIBHeight((LPSTR) lpbmi);
	bmPlanes = lpbmi->biPlanes;
	bmCount = lpbmi->biBitCount;
	bmComp = lpbmi->biCompression;
	if( bmComp == BI_RGB )
	{
		is_compressed = FALSE;
	}
	else
	{
		is_compressed = TRUE;
	}
	wBMcolormaplen = DIBNumColors( (LPSTR) lpbmi );
//	if( wBMcolormaplen == 0 )
	if( !((wBMcolormaplen == 2) ||	/* Bit Count of 1 */
		(wBMcolormaplen == 16) ||	/* 4 */
		(wBMcolormaplen == 256) ||	/* 8 */
		(wBMcolormaplen == (WORD)-1) ) )	/* 24 */
	{	/* Yeek, NOT a supported Bit Count ... */
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_BMPBC );	/* "Error: Unsupported bit count! Not 1,4,8,24!" */
		goto bmpinpret;
#else
		ERREXIT(cinfo->emethods, "Error: Unsupported bit count! Not 1,4,8,24!");
#endif
	}
	szArr = (DWORD) wBMcolormaplen;
	szCols = ColorSize((LPSTR) lpbmi );
	if( szCols != 4 )
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_BMPCM );	/* "Error: Unsupported Color map! Only RGBQUAD!" */
		goto bmpinpret;
#else
		ERREXIT(cinfo->emethods, "Error: Unsupported Color map! Only RGBQUAD!");
#endif
	}
  /* we ignore the color resolution, sort flag, and background color index */
	if( bmHeight && bmWidth && (bmPlanes == 1) )
	{
		if( wBMcolormaplen != (WORD)-1 )
		{
	  		/* Allocate space to store the colormap */
//			if( aColorMap = (*cinfo->emethods->alloc_small_sarray)
//			( szArr, szCols) )
			if( colormap = (*cinfo->emethods->alloc_small_sarray)
			( szArr, szCols) )
			{
//    				BMPReadColorMap( cinfo, wBMcolormaplen, 
//					aColorMap, szArr, szCols );
    				BMPReadColorMap( cinfo, wBMcolormaplen, 
					colormap, szCols );
			}
			else
			{
#ifdef	_INC_WINDOWS
				ERRFLAG( BAD_MEMORY );
				goto bmpinpret;
#else
				ERREXIT(cinfo->emethods, "Error: Insufficient MEMORY!");
#endif
			}
		}
		if( ReadSize > bmOffset )
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_BMPCOLR );	/* "Error: Color table mistake. Not a BMP file ..." */
			goto bmpinpret;
#else
			ERREXIT(cinfo->emethods, "Error: Color table mistake. Not a BMP file ...");
#endif
		}
		while( ReadSize < bmOffset )
		{
			b = ReadByte(cinfo);	/* Move onto BITMAP image */
			if( b == EOF )
				goto bmpinpret;
		}
	}
	else
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_BMPWHP ); /* "ERROR: Width, Height or Planes! Not a BMP file!" */
		goto bmpinpret;
#else
		ERREXIT(cinfo->emethods, "ERROR: Width, Height or Planes! Not a BMP file!");
#endif
	}
  /*
   * If image is compressed, we read it into a full-size sample array,
   * decompressing as we go; then bmp_get_input_row selects rows from the
   * sample array in the proper order.
   */
	if( is_compressed || Inv_Image )
	{
    /* We request the big array now, but can't access it until the pipeline
     * controller causes all the big arrays to be allocated.  Hence, the
     * actual work of reading the image is postponed until the first call
     * of bmp_get_input_row.
     */
		if( is_compressed )
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_BMPNS ); /* "Compressed BMP file not presently handled ..." */
			goto bmpinpret;
#else
			ERREXIT(cinfo->emethods, "Compressed BMP file not presently handled ...");
#endif
		}
		if( wBMcolormaplen == (WORD)-1 )
		{
			szArr = bmWidth * 3;
		}
		else
		{	/* NOTE: We could reduce the SIZE of a "scan line", but ... */
			if( bmWidth % 4 )	/* Ensure on LONG boundary regardless of WIDTH */
				szArr = (((bmWidth / 4) + 1) * 4);
			else
				szArr = bmWidth;
		}
		bmDWidth = szArr;
    	whole_image = (*cinfo->emethods->request_big_sarray)
				( bmDWidth, (long) bmHeight, 1L);
    cinfo->methods->get_input_row = load_compressed_image;
    cinfo->total_passes++;	/* count file reading as separate pass */
	}

  /* Return info about the image. */
  cinfo->input_components = JNUMCOLORS;	/* Controls the input arrays */
  cinfo->in_color_space = CS_RGB;
  cinfo->image_width = bmWidth;
  cinfo->image_height = bmHeight;
  cinfo->data_precision = 8;	/* always, even if 12-bit JSAMPLEs */

//  TRACEMS3(cinfo->emethods, 1, "Input BMP: %u x %u x %d colors...",
//	   (unsigned int) bmWidth, (unsigned int) bmHeight, wBMcolormaplen);
	RowCnt = 0;
	chkei();
#ifdef	_INC_WINDOWS
bmpinpret:
#endif
return( bErrFlag );
} /* end bmp_input_init( compress_info_ptr cinfo ) */


#ifdef	PROGRESS_REPORT
LOCAL void
ShowRow( compress_info_ptr cinfo )
{
	if( cinfo->emethods->trace_level > 0 )
	{
		fprintf(msgout, "\rRow %ld of %ld ...\r", RowCnt, cinfo->image_height);
    	flushmsg(msgout);
	}
	else
	{
  		(*cinfo->methods->progress_monitor) (cinfo, RowCnt, cinfo->image_height);
	}
}
#endif

/*
 * Read one row of pixels.
 * This could be used to get 'inverted' for JPEG BMP images:
 * We read directly from the BMP file, byte by byte, in file order.
 * This would produce an UPSIDE DOWN JPEG IMAGE, so this routine
 * is NOT used. Before this is called, it is changed to the
 * get_compressed_row routine, where in load_compressed_image will
 * load all the file into an array of FAR buffers, then allow
 * row by row access to the data bits from the BOTTOM up to the TOP row.
 * That is inverting the file image ... but keeping left to right per row ...
 *
 */

METHODDEF void
bmp_get_input_row (compress_info_ptr cinfo, JSAMPARRAY pixel_row)
{
  register JSAMPROW ptr0, ptr1, ptr2;
  register long col;
  register int c;
	int i, j;
  long CWid, RWid;
	chkie();
	RowCnt++;
#ifdef	PROGRESS_REPORT
	ShowRow( cinfo );
#endif
	CWid = cinfo->image_width;
	if( CWid % 4 )	/* Does it END on a 32 Bit boundary ... */
		RWid = ((((CWid / 4) + 1) * 4) - CWid);
	else
		RWid = 0;
  ptr0 = pixel_row[0];
  ptr1 = pixel_row[1];
  ptr2 = pixel_row[2];
	if( wBMcolormaplen == (WORD)-1 )
	{
		for( col = 0; col < CWid; col++ )
		{
			c = ReadByte(cinfo);	/* Get a FILE Byte ... */
			*ptr2++ = c;	/* BLUE ... */
			c = ReadByte(cinfo);	/* Get a FILE Byte ... */
			*ptr1++ = c;	/* GREEN component ... */
			c = ReadByte(cinfo);	/* Get a FILE Byte ... */
			*ptr0++ = c;	/* Get it RED value from RGB array */
		}
	}
	else if( wBMcolormaplen == 256 )
	{
		for( col = 0; col < CWid; col++ )
		{
			c = ReadByte(cinfo);	/* Get a FILE Byte ... */
			*ptr0++ = colormap[CM_RED][c];	/* Get it RED value from RGB array */
			*ptr1++ = colormap[CM_GREEN][c];	/* GREEN component ... */
			*ptr2++ = colormap[CM_BLUE][c];	/* and BLUE ... */
		}
	}
	else if( wBMcolormaplen == 16 )
	{
		i = 0;
		for( col = 0; col < CWid; col++ )
		{
			if( i == 0 )
			{
				c = ReadByte(cinfo);	/* Get a FILE Byte ... */
				j = (c & 0xf0) >> 4;
				i = 1;
			}
			else
			{
				j = c & 0x0f;
				i = 0;
			}
			*ptr0++ = colormap[CM_RED][j];	/* Get it RED value from RGB array */
			*ptr1++ = colormap[CM_GREEN][j];	/* GREEN component ... */
			*ptr2++ = colormap[CM_BLUE][j];	/* and BLUE ... */
		}
	}
	else if( wBMcolormaplen == 2 )
	{
		i = 0;
		for( col = 0; col < CWid; col++ )
		{
			if( i == 0 )
			{
				c = ReadByte(cinfo);	/* Get a FILE Byte ... */
				i = 0x80;
			}
			if( c & i )
			{
				*ptr0++ = colormap[CM_RED][1];
				*ptr1++ = colormap[CM_GREEN][1];
				*ptr2++ = colormap[CM_BLUE][1];
			}
			else
			{
				*ptr0++ = colormap[CM_RED][1];
				*ptr1++ = colormap[CM_GREEN][1];
				*ptr2++ = colormap[CM_BLUE][1];
			}
			i = i >> 1;
		}
	}
	else
	{
		RWid = 0;
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_BMPBC );	/* "Error: Unsupported bit count! Not 1,4,8,24!" */
		goto DnGet;
#else
		ERREXIT(cinfo->emethods, "Error: Unsupported bit count! Not 1,4,8,24!");
#endif
	}
	if( RWid ) /* Get PADDING from file buffer ... ever forward ... */
	{
		for( col = 0; col < RWid; col++ )
		{
			c = ReadByte(cinfo);
		}
	}
#ifdef	_INC_WINDOWS
DnGet:
#endif
	if( ( RowCnt == (DWORD) cinfo->image_height ) &&
		( cinfo->emethods->trace_level > 0 ) )
	{
		fprintf(msgout, "\r\nDone last Row (%ld) ...\r\n", RowCnt );
    	flushmsg(msgout);
	}
}


/*
 * Read one row of pixels.
 * This version is used for the first call on bmp_get_input_row when
 * reading a BMP file: we read the whole image into memory so it can
 * be passed row by row to JPEG service from the end back up to the top.
 * 
 */

METHODDEF void
load_compressed_image (compress_info_ptr cinfo, JSAMPARRAY pixel_row)
{
  JSAMPARRAY image_ptr;
  register JSAMPROW sptr;
  register long col;
  long row;
	long col1;
	chkie();
  /* Read the total image into the big array we've created. */
	/* Set SCAN Width ... */
	if( wBMcolormaplen == (WORD)-1 )	
	{ /* If TRUECOLOR ... Each BYTE = R,G,B ... */
		col1 = ( cinfo->image_width * 3 );
	}
	else if( wBMcolormaplen == 256 )
	{	/* If each BYTE is index into color */
		col1 = cinfo->image_width;
	}
	else if( wBMcolormaplen == 16 )
	{	/* Each NIBBLE is index into color */
		col1 = cinfo->image_width / 2;
		if( cinfo->image_width % 2 )
			col1++;
	}
	else if( wBMcolormaplen == 2 )
	{	/* Each BIT is index into color */
		col1 = cinfo->image_width / 8;	/*
		if( cinfo->image_width % 8 )	/* If overrun ... */
			col1++;
	}
	else
	{
		col1 = 0;
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_BMPBC );	/* "Error: Unsupported bit count! Not 1,4,8,24!" */
		goto DnLoad;
#else
		ERREXIT(cinfo->emethods, "Error: Unsupported bit count! Not 1,4,8,24!");
#endif
	}
	for( row = 0; row < cinfo->image_height; row++ )
	{
#ifdef	PROGRESS_REPORT
		if( cinfo->emethods->trace_level > 0)
		{
			fprintf(msgout, "\rRead %ld of %ld ...\r", (row+1), cinfo->image_height);
    		flushmsg(msgout);
		}
		else
		{
  			(*cinfo->methods->progress_monitor) (cinfo, row, cinfo->image_height);
		}
#endif
		image_ptr = (*cinfo->emethods->access_big_sarray)
			(whole_image, row, TRUE);
		sptr = image_ptr[0];
		/* This could perhaps be sped up a little by BLOCK reads ... */
		for( col = 0; col < col1; col++)
		{
			*sptr++ = (JSAMPLE) ReadByte(cinfo);
		}
		if( col1 % 4 )
		{
			col = ((((col1 / 4) + 1) * 4) - col1);
			while( col-- )
			{
				ReadByte(cinfo);	/* Read and discard padding to 32 bits ... */
			}
		}
  }
	if( get_file_size() < ReadSize )
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_BMPSIZ2 ); /* "Error: Bitmap LENGTH error. Not a BMP file ..." */
		goto DnLoad;
#else
		ERREXIT(cinfo->emethods, "Error: Bitmap LENGTH error. Not a BMP file ...");
#endif
	}
	if( cinfo->emethods->trace_level > 0 )
	{
		fprintf(msgout, "\r\nDone Read %ld of %ld ... All data in ...\r\n", row, cinfo->image_height);
   	flushmsg(msgout);
	}
  cinfo->completed_passes++;

  /* Replace method pointer so subsequent calls don't come here. */
  cinfo->methods->get_input_row = get_compressed_row;
  /* Initialize for get_compressed_row, and perform first call on it. */
  cur_row_number = 0;
  get_compressed_row(cinfo, pixel_row);
#ifdef	_INC_WINDOWS
DnLoad:
#endif
return;
}


/*
 *
 * Read one row of pixels.
 * We read from the big in-memory image, from the bottom of the image
 * back towards the top. That is INVERTING it as from the BMP file.
 *	At present the routine ONLY handle bitmaps where each Byte is an
 * index into the color array, thus producing an R, G, B, stream
 * which is stored into the large array provided by the caller ...
 * The array provided, actually a near pointer to an array of FAR
 * pointers, is already set to 3 colors deep ...
 *
 */

METHODDEF void
get_compressed_row (compress_info_ptr cinfo, JSAMPARRAY pixel_row)
{
  JSAMPARRAY image_ptr;
  register JSAMPROW sptr, ptr0, ptr1, ptr2;
  register long col;
  register int c;
  int hn, ln;
  long irow;
	chkie();
	RowCnt++;
#ifdef	PROGRESS_REPORT
	ShowRow( cinfo );
#endif
	/* Figure out which row of image is needed, and access it. */
	irow = cinfo->image_height - (cur_row_number + 1);
	image_ptr = (*cinfo->emethods->access_big_sarray)
		(whole_image, irow, FALSE);
	/* Scan the row, expand colormap, and output */
	sptr = image_ptr[0];
	ptr0 = pixel_row[0];
	ptr1 = pixel_row[1];
	ptr2 = pixel_row[2];
	if( wBMcolormaplen == (WORD)-1 )
	{
		for( col = cinfo->image_width; col > 0; col--)
		{	/* TRUECOLOR - 24-Bit - Each BYTE in BITMAP is a color ... */
			c = GETJSAMPLE(*sptr++);
			*ptr2++ = c;
			c = GETJSAMPLE(*sptr++);
			*ptr1++ = c;
			c = GETJSAMPLE(*sptr++);
			*ptr0++ = c;
		}
	}
	else if( wBMcolormaplen == 256 )
	{	/* Each BYTE indexes into the COLOR TABLE */
		for( col = cinfo->image_width; col > 0; col--)
		{
			c = GETJSAMPLE(*sptr++);
			*ptr0++ = colormap[CM_RED][c];
			*ptr1++ = colormap[CM_GREEN][c];
			*ptr2++ = colormap[CM_BLUE][c];
		}
	}
	else if( wBMcolormaplen == 16 )
	{	/* Each NIBBLE indexes into the COLOR TABLE */
		ln = 0;	/* Start as get sample high nibble */
		for( col = cinfo->image_width; col > 0; col--)
		{
			if( ln == 0 )
			{	/* Start by getting BYTE ... */
				c = GETJSAMPLE(*sptr++);
				hn = (c & 0xf0) >> 4;	/* But only upper nibble */
				ln = 1;	/* Set to 1 for next nibble ... */
			}
			else
			{	/* We have the byte ... */
				hn = (c & 0x0f);	/* Get lower nibble ... */
				ln = 0;	/* and set to fetch next BYTE ... */
			}
			*ptr0++ = colormap[CM_RED][hn];	/* Convert NIBBLE to COLOR */
			*ptr1++ = colormap[CM_GREEN][hn];
			*ptr2++ = colormap[CM_BLUE][hn];
		}
	}
	else if( wBMcolormaplen == 2 )
	{	/* Each BIT indexes into the COLOR TABLE */
		hn = 0;
		for( col = cinfo->image_width; col > 0; col--)
		{
			if( hn == 0 )
			{
				c = GETJSAMPLE(*sptr++);
				hn = 0x80;
			}
			if( c & hn )
			{
				*ptr0++ = colormap[CM_RED][1];
				*ptr1++ = colormap[CM_GREEN][1];
				*ptr2++ = colormap[CM_BLUE][1];
			}
			else
			{
				*ptr0++ = colormap[CM_RED][0];
				*ptr1++ = colormap[CM_GREEN][0];
				*ptr2++ = colormap[CM_BLUE][0];
			}
			hn = hn >> 1;
		}
	}
	cur_row_number++;		/* for next time */
	if( ( cur_row_number == cinfo->image_height ) &&
		( cinfo->emethods->trace_level > 0 ) )
	{
		fprintf(msgout, "\r\nAccessed last Row (%ld) ...\r\n", cur_row_number );
    	flushmsg(msgout);
	}
}	/* end get_compressed_row() ... */


/*
 * Finish up at the end of the file.
 */

METHODDEF void
bmp_input_term (compress_info_ptr cinfo)
{
  /* no work (we let free_all release the workspace) */
}


/*
 * The method selection routine for GIF format input.
 * Note that this must be called by the user interface before calling
 * jpeg_compress.  If multiple input formats are supported, the
 * user interface is responsible for discovering the file format and
 * calling the appropriate method selection routine.
 */
#ifdef	_INC_WINDOWS
GLOBAL int FAR PASCAL
WSELRBMP (compress_info_ptr cinfo)
{
#ifdef	_INC_WINDOWS
	msgout = &szMsgBuf[0];
#endif	/* _INC_WINDOWS */
  cinfo->methods->input_init = bmp_input_init;
  cinfo->methods->get_input_row = bmp_get_input_row; /* assume uncompressed */
  cinfo->methods->input_term = bmp_input_term;
return( 1 );
}
#else	/* !_INC_WINDOWS */
GLOBAL void
jselrbmp (compress_info_ptr cinfo)
{
#ifdef	_INC_WINDOWS
	msgout = &szMsgBuf[0];
#endif	/* _INC_WINDOWS */
  cinfo->methods->input_init = bmp_input_init;
  cinfo->methods->get_input_row = bmp_get_input_row; /* assume uncompressed */
  cinfo->methods->input_term = bmp_input_term;
}
#endif	/* _INC_WINDOWS y/n */

#endif /* BMP_SUPPORTED */

/* eof */
