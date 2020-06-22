
/*
 * wwrppm.c
 *
 * Copyright (C) 1991, 1992, 1993, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains routines to write output images in PPM/PGM format.
 *
 * These routines may need modification for non-Unix environments or
 * specialized applications.  As they stand, they assume output to
 * an ordinary stdio stream.
 *
 * These routines are invoked via the methods ppm_put_pixel_rows, ppm_put_color_map,
 * and ppm_output_init/term.
 */

#include "winclude.h"

#ifdef PPM_SUPPORTED

#ifdef	_INC_WINDOWS
#define	WWRITE( a, b, c )	( ( ( _lwrite( a, b, c )) == (UINT) c ) ? 0 : 1 )
#endif	/* _INC_WINDOWS */

/*
 * Haven't yet got around to making this work with text-format output,
 * hence cannot handle pixels wider than 8 bits.
 */

#ifndef EIGHT_BIT_SAMPLES
  Sorry, this code only copes with 8-bit JSAMPLEs. /* deliberate syntax err */
#endif


/*
 * On most systems, writing individual bytes with putc() is drastically less
 * efficient than buffering a row at a time for fwrite().  But we must
 * allocate the row buffer in near data space on PCs, because we are assuming
 * small-data memory model, wherein fwrite() can't reach far memory.  If you
 * need to process very wide images on a PC, you may have to use the putc()
 * approach.  Also, there are still a few systems around wherein fwrite() is
 * actually implemented as a putc() loop, in which case this buffer is a waste
 * of space.  So the putc() method can be used by defining USE_PUTC_OUTPUT.
 */

#ifndef USE_PUTC_OUTPUT
static char * row_buffer;	/* holds 1 pixel row's worth of output */
#endif


/*
 * Write the file header.
 */

METHODDEF boolean
ppm_output_init( decompress_info_ptr cinfo )
{
#ifdef	_INC_WINDOWS
char	Buf[128];
#endif
	if( cinfo->out_color_space == CS_GRAYSCALE )
	{
    /* emit header for raw PGM format */
#ifdef	_INC_WINDOWS
		wsprintf( &Buf[0], "P5\n%ld %ld\n%d\n",
	    cinfo->image_width, cinfo->image_height, 255);
		if( WWRITE( cinfo->output_file, &Buf[0], lstrlen( &Buf[0] ) ) )
		{
			ERRFLAG( BAD_WWRITE );
			goto ppmoutiret;
		}
#else
    fprintf(cinfo->output_file, "P5\n%ld %ld\n%d\n",
	    cinfo->image_width, cinfo->image_height, 255);
#endif
#ifndef USE_PUTC_OUTPUT
    /* allocate space for row buffer: 1 byte/pixel */
    row_buffer = (char MLPTR) (*cinfo->emethods->alloc_small)
			((size_t) (SIZEOF(char) * cinfo->image_width));
#endif
	}
	else if( cinfo->out_color_space == CS_RGB )
	{
    /* emit header for raw PPM format */
#ifdef	_INC_WINDOWS
		wsprintf( &Buf[0], "P6\n%ld %ld\n%d\n",
			cinfo->image_width, cinfo->image_height, 255);
		if( WWRITE( cinfo->output_file, &Buf[0], lstrlen( &Buf[0] ) ) )
		{
			ERRFLAG( BAD_WWRITE );
			goto ppmoutiret;
		}
#else
    fprintf(cinfo->output_file, "P6\n%ld %ld\n%d\n",
	    cinfo->image_width, cinfo->image_height, 255);
#endif
#ifndef USE_PUTC_OUTPUT
    /* allocate space for row buffer: 3 bytes/pixel */
    row_buffer = (char MLPTR) (*cinfo->emethods->alloc_small)
			((size_t) (3 * SIZEOF(char) * cinfo->image_width));
#endif
	}
	else
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_PPMOUT );
#else
		ERREXIT(cinfo->emethods, "PPM output must be grayscale or RGB");
#endif
	}
#ifdef	_INC_WINDOWS
ppmoutiret:
#endif
return( bErrFlag );
}


/*
 * Write some pixel data.
 */

#ifdef USE_PUTC_OUTPUT

METHODDEF void
ppm_put_pixel_rows (decompress_info_ptr cinfo, int num_rows,
		JSAMPIMAGE pixel_data)
{
  register FILETYPE outfile = cinfo->output_file;
  register JSAMPROW ptr0, ptr1, ptr2;
  register long col;
  long width = cinfo->image_width;
  int row;
  
  for (row = 0; row < num_rows; row++) {
    ptr0 = pixel_data[0][row];
    ptr1 = pixel_data[1][row];
    ptr2 = pixel_data[2][row];
    for (col = width; col > 0; col--) {
      putc(GETJSAMPLE(*ptr0), outfile);
      ptr0++;
      putc(GETJSAMPLE(*ptr1), outfile);
      ptr1++;
      putc(GETJSAMPLE(*ptr2), outfile);
      ptr2++;
    }
  }
}

METHODDEF void
put_gray_rows (decompress_info_ptr cinfo, int num_rows,
	       JSAMPIMAGE pixel_data)
{
  register FILETYPE outfile = cinfo->output_file;
  register JSAMPROW ptr0;
  register long col;
  long width = cinfo->image_width;
  int row;
  
  for (row = 0; row < num_rows; row++) {
    ptr0 = pixel_data[0][row];
    for (col = width; col > 0; col--) {
      putc(GETJSAMPLE(*ptr0), outfile);
      ptr0++;
    }
  }
}

#else /* use row buffering */

METHODDEF void
ppm_put_pixel_rows (decompress_info_ptr cinfo, int num_rows,
		JSAMPIMAGE pixel_data)
{
  FILETYPE outfile = cinfo->output_file;
  register JSAMPROW ptr0, ptr1, ptr2;
  register char MLPTR row_bufferptr;
  register long col;
  long width = cinfo->image_width;
  int row;
  
  for (row = 0; row < num_rows; row++) {
    ptr0 = pixel_data[0][row];
    ptr1 = pixel_data[1][row];
    ptr2 = pixel_data[2][row];
    row_bufferptr = row_buffer;
    for (col = width; col > 0; col--) {
      *row_bufferptr++ = (char) GETJSAMPLE(*ptr0++);
      *row_bufferptr++ = (char) GETJSAMPLE(*ptr1++);
      *row_bufferptr++ = (char) GETJSAMPLE(*ptr2++);
    }
#ifdef	_INC_WINDOWS
		_lwrite(
			outfile,
			row_buffer,
			(UINT) (3*width) );
#else
    (void) JFWRITE( outfile, row_buffer, 3*width);
#endif
  }
}

METHODDEF void
put_gray_rows (decompress_info_ptr cinfo, int num_rows,
	       JSAMPIMAGE pixel_data)
{
  FILETYPE outfile = cinfo->output_file;
  register JSAMPROW ptr0;
  register char MLPTR row_bufferptr;
  register long col;
  long width = cinfo->image_width;
  int row;
  
	for( row = 0; row < num_rows; row++ )
	{
		ptr0 = pixel_data[0][row];
		row_bufferptr = row_buffer;
		for( col = width; col > 0; col-- )
		{
			*row_bufferptr++ = (char) GETJSAMPLE(*ptr0++);
		}
#ifdef	_INC_WINDOWS
		_lwrite( outfile,
			row_buffer,
			(UINT) width);
#else
		(void) JFWRITE(outfile, row_buffer, width);
#endif
	}
}

#endif /* USE_PUTC_OUTPUT */


/*
 * Write some pixel data when color quantization is in effect.
 */

#ifdef USE_PUTC_OUTPUT

METHODDEF void
put_demapped_rgb (decompress_info_ptr cinfo, int num_rows,
		  JSAMPIMAGE pixel_data)
{
  register FILETYPE outfile = cinfo->output_file;
  register JSAMPROW ptr;
  register JSAMPROW color_map0 = cinfo->colormap[0];
  register JSAMPROW color_map1 = cinfo->colormap[1];
  register JSAMPROW color_map2 = cinfo->colormap[2];
  register int pixval;
  register long col;
  long width = cinfo->image_width;
  int row;
  
  for (row = 0; row < num_rows; row++) {
    ptr = pixel_data[0][row];
    for (col = width; col > 0; col--) {
      pixval = GETJSAMPLE(*ptr++);
      putc(GETJSAMPLE(color_map0[pixval]), outfile);
      putc(GETJSAMPLE(color_map1[pixval]), outfile);
      putc(GETJSAMPLE(color_map2[pixval]), outfile);
    }
  }
}

METHODDEF void
put_demapped_gray (decompress_info_ptr cinfo, int num_rows,
		   JSAMPIMAGE pixel_data)
{
  register FILETYPE outfile = cinfo->output_file;
  register JSAMPROW ptr;
  register JSAMPROW color_map0 = cinfo->colormap[0];
  register int pixval;
  register long col;
  long width = cinfo->image_width;
  int row;
  
  for (row = 0; row < num_rows; row++) {
    ptr = pixel_data[0][row];
    for (col = width; col > 0; col--) {
      pixval = GETJSAMPLE(*ptr++);
      putc(GETJSAMPLE(color_map0[pixval]), outfile);
    }
  }
}

#else /* use row buffering */

METHODDEF void
put_demapped_rgb (decompress_info_ptr cinfo, int num_rows,
		  JSAMPIMAGE pixel_data)
{
  FILETYPE outfile = cinfo->output_file;
  register JSAMPROW ptr;
  register char MLPTR row_bufferptr;
  register JSAMPROW color_map0 = cinfo->colormap[0];
  register JSAMPROW color_map1 = cinfo->colormap[1];
  register JSAMPROW color_map2 = cinfo->colormap[2];
  register int pixval;
  register long col;
  long width = cinfo->image_width;
  int row;
  
  for (row = 0; row < num_rows; row++) {
    ptr = pixel_data[0][row];
    row_bufferptr = row_buffer;
    for (col = width; col > 0; col--) {
      pixval = GETJSAMPLE(*ptr++);
      *row_bufferptr++ = (char) GETJSAMPLE(color_map0[pixval]);
      *row_bufferptr++ = (char) GETJSAMPLE(color_map1[pixval]);
      *row_bufferptr++ = (char) GETJSAMPLE(color_map2[pixval]);
    }
#ifdef	_INC_WINDOWS
		_lwrite(
			outfile,
			row_buffer,
			(UINT) (3*width) );
#else
    (void) JFWRITE(outfile, row_buffer, 3*width);
#endif
  }
}

METHODDEF void
put_demapped_gray (decompress_info_ptr cinfo, int num_rows,
		   JSAMPIMAGE pixel_data)
{
  FILETYPE outfile = cinfo->output_file;
  register JSAMPROW ptr;
  register char MLPTR row_bufferptr;
  register JSAMPROW color_map0 = cinfo->colormap[0];
  register int pixval;
  register long col;
  long width = cinfo->image_width;
  int row;
  
  for (row = 0; row < num_rows; row++) {
    ptr = pixel_data[0][row];
    row_bufferptr = row_buffer;
    for (col = width; col > 0; col--) {
      pixval = GETJSAMPLE(*ptr++);
      *row_bufferptr++ = (char) GETJSAMPLE(color_map0[pixval]);
    }
#ifdef	_INC_WINDOWS
		_lwrite( outfile,
			row_buffer,
			(UINT) width);
#else
    (void) JFWRITE(outfile, row_buffer, width);
#endif
  }
}

#endif /* USE_PUTC_OUTPUT */


/*
 * Write the color map.
 * For PPM output, we just remember to demap the output data!
 */

METHODDEF void
ppm_put_color_map (decompress_info_ptr cinfo, int num_colors, JSAMPARRAY colormap)
{
  if (cinfo->out_color_space == CS_RGB)
    cinfo->methods->put_pixel_rows = put_demapped_rgb;
  else
    cinfo->methods->put_pixel_rows = put_demapped_gray;
}


/*
 * Finish up at the end of the file.
 */

METHODDEF void
output_term (decompress_info_ptr cinfo)
{
  /* No work except to make sure we wrote the output file OK; */
  /* we let free_all release any workspace */
#ifdef	_INC_WINDOWS
	if( cinfo->output_file )
	{
		b_fflush( cinfo->output_file );
		_lclose( cinfo->output_file );
	}
	cinfo->output_file = 0;
#else
  fflush(cinfo->output_file);
  if (ferror(cinfo->output_file))
    ERREXIT(cinfo->emethods, "Output file write error --- out of disk space?");
#endif
}


/*
 * The method selection routine for PPM format output.
 * This should be called from d_ui_method_selection if PPM output is wanted.
 */

GLOBAL void
jselwppm (decompress_info_ptr cinfo)
{
  cinfo->methods->output_init = ppm_output_init;
  cinfo->methods->put_color_map = ppm_put_color_map;
  if (cinfo->out_color_space == CS_RGB)
    cinfo->methods->put_pixel_rows = ppm_put_pixel_rows;
  else
    cinfo->methods->put_pixel_rows = put_gray_rows;
  cinfo->methods->output_term = output_term;
}

#endif /* PPM_SUPPORTED */

/* eof - wwrppm.c */

