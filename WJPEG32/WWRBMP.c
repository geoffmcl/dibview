
/*
 * wwrbmp.c
 *
 * Copyright (C) 1996, Geoff R. McLane.
 * This file is NOT part of the Independent JPEG Group's software.
 *
 * This file contains routines to write output images in BMP format.
 * The MS W 3.1 SDK is required.
 *
 * These routines are invoked via the methods bd_put_pixel_rows, bd_put_color_map,
 * and bd_output_init/term.
 *
 * Based on code in the various jwr????.c JPEG source
 */

#include "winclude.h"

#ifdef BMP_SUPPORTED

/* jbmp.h is taken from the MS C W 3.1 SDK. */
#ifdef	_INC_WINDOWS

typedef struct tagcol_map 
{
	RGBQUAD	aColors[MAXJSAMPLE];
} col_map;

#ifdef	WJPEG4
extern	void	chkretired( void );
#else	// !WJPEG4
//#define	jdos_write(a,b,c)		_lwrite(a,b,c)
#define	jdos_close(a)			_lclose(a)
#endif	// WJPEG4 y/n

#else	// !_INC_WINDOWS

#include "jbmp.h"

#endif	// _INC_WINDOWS y/n

/*
 * bd_output_term assumes that JSAMPLE has the same representation as BMP pixel,
 * to wit, "unsigned char".  Hence we can't cope with 12- or 16-bit samples.
 */

#ifndef EIGHT_BIT_SAMPLES
  Sorry, this code only copes with 8-bit JSAMPLEs. /* deliberate syntax err */
#endif


/*
 * Since BMP, like RLE 
 * stores scanlines bottom-to-top, we have to invert the image
 * from JPEG's top-to-bottom order.  To do this, we save the outgoing data
 * in virtual array(s) during put_pixel_row calls, then actually emit the
 * BMP/RLE file during bd_output_term.  We use one virtual array if the output is
 * grayscale or colormapped, more if it is full color.
 */

#define MAX_CHANS	4	/* allow up to four color components */
static big_sarray_ptr channels[MAX_CHANS]; /* Virtual arrays for saved data */

static long cur_output_row;	/* next row# to write to virtual array(s) */


/*
 * For now, if we emit an BMP color map then it is always 256 entries long,
 * though not all of the entries need be used.
 */

#define CMAPBITS	8
#define CMAPLENGTH	(1<<(CMAPBITS))
#if	(defined( _INC_WINDOWS ) && defined( USELOCBUF ))
RGBQUAD	ColMapBuf[256];	/* 256 x 4 byte buffer */
#endif	/* USELOCBUF */

// FIX16 - 10
//static col_map *BMP_colormap; /* RLE-style color map, or NULL if none */
static col_map MLPTR BMP_colormap; /* RLE-style color map, or NULL if none */
static int number_colors;	/* Number of colors actually used */

/*
 * File Handling Routines
 */
static decompress_info_ptr dcinfo; /* to avoid passing to all functions */

/*
 * Write the file header.
 *
 * In this module it's easier to wait till bd_output_term to actually write
 * anything; here we just request the big arrays we'll need.
 */

METHODDEF boolean
bd_output_init (decompress_info_ptr cinfo)
{
  short ci;
	dcinfo = cinfo;
  if ((ci = cinfo->final_out_comps) != 1) /* safety check */
  {
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_BMPOUT ); /* "BMP output is confused! Can not handle %d channels!" */
		goto boutret;
#else
    ERREXIT1(cinfo->emethods, "BMP output is confused! Can not handle %d channels!",
	cinfo->final_out_comps);
#endif
  }  

	for( ci = 0; ci < cinfo->final_out_comps; ci++ )
	{
		channels[ci] = (*cinfo->emethods->request_big_sarray)
			(cinfo->image_width, cinfo->image_height, 1L);
	}
  
#if	(defined( _INC_WINDOWS ) && defined( USELOCBUF ))
	BMP_colormap = (col_map MLPTR) &ColMapBuf[0];
#else
  BMP_colormap = NULL;	/* No output colormap as yet */
#endif
  number_colors = 0;
  cur_output_row = 0;		/* Start filling virtual arrays at row 0 */

  cinfo->total_passes++;	/* count file writing as separate pass */
#ifdef	_INC_WINDOWS
boutret:
#endif
return( bErrFlag );
}

/*
 * Write some pixel data.
 *
 * This routine just saves the data away in virtual arrays.
 */
JSAMPROW outprow[1];	/* a pseudo JSAMPARRAY structure */

METHODDEF void
bd_put_pixel_rows (decompress_info_ptr cinfo, int num_rows,
		JSAMPIMAGE pixel_data)
{
  int row;
  short ci;
  
	for( row = 0; row < num_rows; row++ )
	{
		for( ci = 0; ci < cinfo->final_out_comps; ci++ )
		{
			outprow[0] = *((*cinfo->emethods->access_big_sarray)
				(channels[ci], cur_output_row, TRUE));
			jcopy_sample_rows(pixel_data[ci], 
				row,
				outprow,
				0,
				1, 
				cinfo->image_width );
		}
		cur_output_row++;
 	}
}


/*
 * Write the color map.
 *  For BMP output we just save the colormap for the output stage.
 */
void chkcol( void )
{
int i;
	i = 0;
}

METHODDEF void
bd_put_color_map (decompress_info_ptr cinfo, int num_colors, JSAMPARRAY colormap)
{
  size_t cmapsize;
  short ci;
  int i;
	chkcol();
	if( num_colors > CMAPLENGTH )
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_BMPCE );	/* "Cannot handle colormap entries for BMP" */
		return;
#else
		ERREXIT1(cinfo->emethods, "Cannot handle %d colormap entries for BMP",
	     num_colors);
#endif
	}
  /* Allocate storage for BMP/RLE-style cmap, zero any extra entries */
//  cmapsize = cinfo->color_out_comps * CMAPLENGTH * SIZEOF(col_map);
//  cmapsize = cinfo->color_out_comps * SIZEOF(col_map);
  cmapsize = SIZEOF(col_map);
#if	(defined( _INC_WINDOWS ) && defined( USELOCBUF ))
  BMP_colormap = (col_map MLPTR) &ColMapBuf[0];
#else
  BMP_colormap = (col_map MLPTR) (*cinfo->emethods->alloc_small) (cmapsize);
#endif
  MEMZERO(BMP_colormap, cmapsize);

  /* Save away color data in BMP format */
  if (cinfo->out_color_space == CS_RGB) 
  {
    /* Normal case: RGB color map */
    if( (ci = cinfo->color_out_comps) > 0 )
    {
      for( i = 0; i < num_colors; i++ )
      {
	BMP_colormap->aColors[i].rgbRed  = GETJSAMPLE(colormap[0][i]);
	BMP_colormap->aColors[i].rgbGreen = GETJSAMPLE(colormap[1][i]);
	BMP_colormap->aColors[i].rgbBlue   = GETJSAMPLE(colormap[2][i]);
	/* Note: We have already ZEROED this memory, so additional byte is 0 */
      }
    }
  }
  number_colors = num_colors;
}


/*
 * Finish up at the end of the file.
 *
 * Here is where we really output the BMP/RLE file.
 */

BITMAPFILEHEADER bmfh;
BITMAPINFOHEADER bmih;
void	chkterm( void )
{
int i;
	i = 0;
}

METHODDEF void
bd_output_term (decompress_info_ptr cinfo)
{
short ci, cc, bc, cnt;
long row, wid, ochr;
BYTE MLPTR bptr;
BYTE MLPTR oput_row;
short ofile;
int	c, d, chr;
	chkterm();
  /* Initialize the header info */
	ofile = (short) cinfo->output_file;
  MEMZERO(&bmfh, sizeof(BITMAPFILEHEADER)); /* make sure all bits are 0 */
  MEMZERO(&bmih, sizeof(BITMAPINFOHEADER)); /* make sure all bits are 0 */
/* Initialise FILE HEADER structure */
/* First get the width - 32-bit boundary */
	if( number_colors <= 2 )
	{
		cc = 2;
		bc = 1;
		wid = cinfo->image_width / 8;
		if( cinfo->image_width % 8 )
			wid++;
	}
	else if( number_colors <= 16 )
	{
		cc = 16;
		bc = 4;
		wid = cinfo->image_width / 2;
		if( cinfo->image_width % 2 )
			wid++;
	}
	else if( number_colors <= 256 )
	{
		cc = 256;
		bc = 8;
		wid = cinfo->image_width;
	}
	else
	{
		cc = 0;
		bc = 0;
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_BMPCE );	/* "Cannot handle colormap entries for BMP" */
		return;
#else
		ERREXIT1(cinfo->emethods, "Cannot handle %d colormap entries for BMP",
	     num_colors);
#endif
	}
	if( wid % 4 )	/* If NOT a multiple of 4 bytes - 32-bit */
	{
		ci = (short) wid / 4;
		ci++;
		wid = (long) ci * 4;
	}
	bmfh.bfType = 19778;		/* Add the all important 'BM' signature ... */
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) +	/* File HEADER + */
		sizeof(BITMAPINFOHEADER) +		/* Info HEADER + */
		cc * sizeof(RGBQUAD);	/* Color MAP. */
	bmfh.bfSize = bmfh.bfOffBits +	/* And this offset to BITS, + */
		(wid * cinfo->image_height);	/* FULL SIZE of the bits is FILE SIZE */
/* Initialise INFO HEADER structure */
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = cinfo->image_width;
	bmih.biHeight = cinfo->image_height;
	bmih.biPlanes = 1;
//	bmih.biBitCount = 8;
	bmih.biBitCount = bc;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;
  /* Emit the BMP header and color map (if any) */
	bptr = (BYTE *) &bmfh;
	if( cinfo->hgOut == 0 )	/* Only IF WRITING a file ... */
	{	/* do we WRITE the BITMAPFILEHEADER ... */
		for( ci = 0; ci < sizeof(BITMAPFILEHEADER); ci++ )
		{
			b_putc( bptr[ci], ofile );
#ifdef	_INC_WINDOWS
			if( bErrFlag )
				goto CloseOut;
#endif
		}
	}
	bptr = (BYTE *) &bmih;
	for( ci = 0; ci < sizeof(BITMAPINFOHEADER); ci++ )
	{
		b_putc( bptr[ci], ofile );
#ifdef	_INC_WINDOWS
		if( bErrFlag )
			goto CloseOut;
#endif
	}
  /* Now add the color map - from [BMP_colormap] allocated */
  if( BMP_colormap && number_colors )
  {
	for( ci = 0; ci < number_colors; ci++ )
	{
		b_putc( BMP_colormap->aColors[ci].rgbBlue, ofile );
		b_putc( BMP_colormap->aColors[ci].rgbGreen, ofile );
		b_putc( BMP_colormap->aColors[ci].rgbRed, ofile );
		b_putc( 0, ofile );	/* Complete RGBQUAD byte */
#ifdef	_INC_WINDOWS
		if( bErrFlag )
			goto CloseOut;
#endif
	}
	if( ci < cc )
	{
		for( ; ci < cc; ci++ )
		{
			b_putc( 0, ofile );
			b_putc( 0, ofile );
			b_putc( 0, ofile );
			b_putc( 0, ofile );	/* Complete RGBQUAD byte */
#ifdef	_INC_WINDOWS
			if( bErrFlag )
				goto CloseOut;
#endif
		}
	}
  }
  /* Now output the BMP data from our virtual array(s).
   * We assume here that (a) rle_pixel is represented the same as JSAMPLE,
   * and (b) we are not on a machine where FAR pointers differ from regular.
   */
	ci = 0;
  b_fflush( ofile );
  for (row = cinfo->image_height-1; row >= 0; row--) {
#ifdef	PROGRESS_REPORT
    (*cinfo->methods->progress_monitor) (cinfo, cinfo->image_height-row-1,
					 cinfo->image_height);
#endif
	ci = 0;
      oput_row = (BYTE MLPTR) *((*cinfo->emethods->access_big_sarray)
					(channels[ci], row, FALSE));
		switch( bc )
		{
			case 1:
			{
				cnt = 0;
				d = 0x80;
				chr = 0;
				ochr = 0;
				for( ci = 0; ci < cinfo->image_width; ci++ )
				{
					c = oput_row[ci];	/* Extract value */
					if( c )
						chr |= d;
					d = d >> 1;
					cnt++;
					if( cnt == 8 )	/* If 8 BITS done ... */
					{
						b_putc( chr, ofile );	/* Output the BYTE ... */
#ifdef	_INC_WINDOWS
						if( bErrFlag )
							goto CloseOut;
#endif
						d = 0x80;
						cnt = 0;
						ochr++;
						chr = 0;	/* Restart OUT BYTE ... */
					}
				}
				if( cnt )	/* If some remainder ... */
				{
						b_putc( chr, ofile );	/* Output the BYTE ... */
#ifdef	_INC_WINDOWS
						if( bErrFlag )
							goto CloseOut;
#endif
						d = 0x80;
						cnt = 0;
						ochr++;
				}
				while( ochr < wid )
				{
					b_putc( 0, ofile );
					ochr++;
#ifdef	_INC_WINDOWS
					if( bErrFlag )
						goto CloseOut;
#endif
				}
				break;
			}
			case 4:
			{
				cnt = 0;
				d = 0x80;
				chr = 0;
				ochr = 0;
				for( ci = 0; ci < cinfo->image_width; ci++ )
				{
					c = oput_row[ci];	/* Extract DATA BITS ... */
					if( cnt == 0 )
					{
						chr |= (c & 0x0f) << 4;
					}
					else
					{
						chr |= (c & 0x0f);
					}
					cnt++;
					if( cnt == 2 )
					{
						b_putc( chr, ofile );
#ifdef	_INC_WINDOWS
						if( bErrFlag )
							goto CloseOut;
#endif
						chr = 0;
						cnt = 0;
						ochr++;
					}
				}	/* For the DATA width ... packing 2 into 1 */
				if( cnt )
				{
						b_putc( chr, ofile );
#ifdef	_INC_WINDOWS
						if( bErrFlag )
							goto CloseOut;
#endif
						chr = 0;
						cnt = 0;
						ochr++;
				}
				while( ochr < wid )
				{
					b_putc( 0, ofile );
					ochr++;
#ifdef	_INC_WINDOWS
					if( bErrFlag )
						goto CloseOut;
#endif
				}
				break;
			}
			case 8:	/* If outputting 8 bit BYTE, then ... */
			{
				for( ci = 0; ci < cinfo->image_width; ci++ )
				{
					b_putc( oput_row[ci], ofile );
#ifdef	_INC_WINDOWS
					if( bErrFlag )
						goto CloseOut;
#endif
				}
				while( (long) ci < wid )
				{
					b_putc( 0, ofile );
					ci++;
#ifdef	_INC_WINDOWS
					if( bErrFlag )
						goto CloseOut;
#endif
				}
				break;
			}
		} /* Switch case ... */
  }	/* For each row ... */
	cinfo->completed_passes++;
	b_fflush( ofile );
#ifdef	_INC_WINDOWS
CloseOut:
#endif
	if( ofile && (ofile != HFILE_ERROR) )
	{
#ifdef	WJPEG4
		chkretired();
		ERRFLAG( BAD_RETIRED );
#else	// !WJPEG4
		if( jdos_close( ofile ) )	/* Physical CLOSE of output file ... */
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_WRITOUT );
#else
			ERREXIT(cinfo->emethods, "Output file write error --- out of disk space?");
#endif
		}
#endif	// WJPEG4 y/n
	}
	cinfo->output_file = 0;	/* and LOGICAL close ... */
  /* Release memory */
  /* no work (we let free_all release the workspace) */
}


/*
 * The method selection routine for BMP format output.
 * This should be called from d_ui_method_selection if BMP output is wanted.
 */

GLOBAL void
jselwbmp (decompress_info_ptr cinfo)
{
  cinfo->methods->output_init    = bd_output_init;
  cinfo->methods->put_color_map  = bd_put_color_map;
  cinfo->methods->put_pixel_rows = bd_put_pixel_rows;
  cinfo->methods->output_term    = bd_output_term;
	if( cinfo->out_color_space != CS_GRAYSCALE &&
      cinfo->out_color_space != CS_RGB )
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_BMPNRGB );	/* "BMP output must be grayscale or RGB" */
		return;
#else
    ERREXIT(cinfo->emethods, "BMP output must be grayscale or RGB");
#endif
	}
  /* Force quantization if color or if > 8 bits input */
  if (cinfo->out_color_space == CS_RGB || cinfo->data_precision > 8) {
    /* Force quantization to at most 256 colors */
    cinfo->quantize_colors = TRUE;
    if (cinfo->desired_number_of_colors > 256)
      cinfo->desired_number_of_colors = 256;
  }
}

#endif /* BMP_SUPPORTED */

/* eof - wwrbmp.c */

