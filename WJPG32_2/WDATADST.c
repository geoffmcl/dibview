
// WDataDst.c

/*
 * jdatadst.c
 *
 * Copyright (C) 1994-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains compression data destination routines for the case of
 * emitting JPEG data to a file (or any stdio stream).  While these routines
 * are sufficient for most applications, some will want to use a different
 * destination manager.
 * IMPORTANT: we assume that fwrite() will correctly transcribe an array of
 * JOCTETs into 8-bit-wide elements on external storage.  If char is wider
 * than 8 bits on your machine, you may need to do some tweaking.
 */

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"


/* Expanded data destination object for stdio output */

typedef struct {
  struct jpeg_destination_mgr pub; /* public fields */

  HFILE outfile;		/* target stream */
  JOCTET MLPTR buffer;		/* start of buffer */
  HGLOBAL	hgJpgOut;
  DWORD		dwMaxOut;
  DWORD		dwCurOut;

} my_destination_mgr;

typedef my_destination_mgr MLPTR my_dest_ptr;

#define OUTPUT_BUF_SIZE  4096	/* choose an efficiently fwrite'able size */


/*
 * Initialize destination --- called by jpeg_start_compress
 * before any data is actually written.
 */

METHODDEF(void)
init_destination (j_compress_ptr cinfo)
{
  my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

  /* Allocate the output buffer --- it will be released when done with image */
  dest->buffer = (JOCTET MLPTR )
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  OUTPUT_BUF_SIZE * SIZEOF(JOCTET));

  dest->pub.next_output_byte = dest->buffer;
  dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}


/*
 * Empty the output buffer --- called whenever buffer fills up.
 *
 * In typical applications, this should write the entire output buffer
 * (ignoring the current state of next_output_byte & free_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been dumped.
 *
 * In applications that need to be able to suspend compression due to output
 * overrun, a FALSE return indicates that the buffer cannot be emptied now.
 * In this situation, the compressor will return to its caller (possibly with
 * an indication that it has not accepted all the supplied scanlines).  The
 * application should resume compression after it has made more room in the
 * output buffer.  Note that there are substantial restrictions on the use of
 * suspension --- see the documentation.
 *
 * When suspending, the compressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_output_byte & free_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point will be regenerated after resumption, so do not
 * write it out when emptying the buffer externally.
 */
#define		MXFIO		1024

BOOL	WriteToFile( HFILE hf, LPSTR lpb, DWORD len )
{
	BOOL	flg = FALSE;
	DWORD	rem;
	int		wr,ww;
	BOOL	ferr;

	ferr = FALSE;
	if( lpb && hf &&
		( hf != HFILE_ERROR ) )
	{
		if( rem = len )
		{
			while( rem )
			{
				if( rem > MXFIO )
					wr = MXFIO;
				else
					wr = (int)rem;
				ww = _lwrite( hf, lpb, wr );
				if( ww != wr )
				{
					ferr = TRUE;
					break;
				}
				rem -= (DWORD)ww;
			}
		}
		if( !ferr )
			flg = TRUE;
	}
	return flg;
}

METHODDEF(boolean)
empty_output_buffer (j_compress_ptr cinfo)
{
	my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

	if( !WriteToFile( dest->outfile, dest->buffer, OUTPUT_BUF_SIZE ) )
		ERREXIT(cinfo, JERR_FILE_WRITE);

	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

	return TRUE;
}


/*
 * Terminate destination --- called by jpeg_finish_compress
 * after all data has been written.  Usually needs to flush buffer.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF(void)
term_destination (j_compress_ptr cinfo)
{
  my_dest_ptr dest = (my_dest_ptr) cinfo->dest;
  size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

  /* Write any data remaining in the buffer */
  if (datacount > 0) {
	  if( !WriteToFile( dest->outfile, dest->buffer, datacount ) )
		  ERREXIT(cinfo, JERR_FILE_WRITE);
  }
  //fflush(dest->outfile);
  /* Make sure we wrote the output file OK */
  //if (ferror(dest->outfile))
  //  ERREXIT(cinfo, JERR_FILE_WRITE);
}


/*
 * Prepare for output to a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing compression.
 */

GLOBAL(void)
jpeg_winio_dest (j_compress_ptr cinfo, HFILE outfile)
{
  my_dest_ptr dest;

  /* The destination object is made permanent so that multiple JPEG images
   * can be written to the same file without re-executing jpeg_stdio_dest.
   * This makes it dangerous to use this manager and a different destination
   * manager serially with the same JPEG object, because their private object
   * sizes may be different.  Caveat programmer.
   */
  if (cinfo->dest == NULL) {	/* first time for this JPEG object? */
    cinfo->dest = (struct jpeg_destination_mgr MLPTR)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  SIZEOF(my_destination_mgr));
  }

  dest = (my_dest_ptr) cinfo->dest;
  dest->pub.init_destination = init_destination;
  dest->pub.empty_output_buffer = empty_output_buffer;
  dest->pub.term_destination = term_destination;
  dest->outfile = outfile;	// Set HFILE
  dest->hgJpgOut = 0;		// HGLOBAL is zero

}


BOOL	MoveHandData( my_dest_ptr dest, DWORD datacount )
{
	BOOL	flg;
	HGLOBAL	hgOut;
	LPSTR	lpb;
	LPDWORD	lpdw;
	
	flg = FALSE;
	if( (hgOut = dest->hgJpgOut) &&
		(lpb = GlobalLock( hgOut )) )
	{
		lpdw = (LPDWORD)lpb;
		if( dest->dwCurOut == 0 )
			*lpdw = 0;
		lpb = (LPSTR)((LPDWORD)lpdw + 1);
		MEMCOPY( &lpb[dest->dwCurOut], dest->buffer, datacount );
		dest->dwCurOut += datacount;	// To NEXT
		*lpdw += datacount;				// and RETURN COUNT
		GlobalUnlock( hgOut );			// Unlock DESTINATION
		flg = TRUE;						// ALL OK...
	}
	return flg;
}

METHODDEF(boolean)
empty_hand_output (j_compress_ptr cinfo)
{
	my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

	if( !MoveHandData( dest, OUTPUT_BUF_SIZE ) )
		ERREXIT(cinfo, JERR_FILE_WRITE);

	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

	return TRUE;
}


METHODDEF(void)
term_hand_dest (j_compress_ptr cinfo)
{
	my_dest_ptr dest = (my_dest_ptr) cinfo->dest;
	size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

	/* Write any data remaining in the buffer */
	if( datacount > 0 )
	{
		if( !MoveHandData( dest, datacount ) )
			ERREXIT(cinfo, JERR_FILE_WRITE);
	}

	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

}


GLOBAL(void)
jpeg_handle_dest (j_compress_ptr cinfo, HGLOBAL outfile)
{
	LPSTR	lpb;
	my_dest_ptr dest;

  /* The destination object is made permanent so that multiple JPEG images
   * can be written to the same file without re-executing jpeg_stdio_dest.
   * This makes it dangerous to use this manager and a different destination
   * manager serially with the same JPEG object, because their private object
   * sizes may be different.  Caveat programmer.
   */
  if (cinfo->dest == NULL) {	/* first time for this JPEG object? */
    cinfo->dest = (struct jpeg_destination_mgr MLPTR )
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
				  SIZEOF(my_destination_mgr));
  }

  dest = (my_dest_ptr) cinfo->dest;
  dest->pub.init_destination = init_destination;
  dest->pub.empty_output_buffer = empty_hand_output;
  dest->pub.term_destination = term_hand_dest;
  dest->outfile = 0;	// NO HFILE
  dest->hgJpgOut = outfile;
  dest->dwMaxOut = 0;
  dest->dwCurOut = 0;
  if( outfile &&
	  (lpb = GlobalLock( outfile )) )
  {
	  dest->dwMaxOut = GlobalSize( outfile );
	  GlobalUnlock( outfile );
  }

}

// eof - WDataDst.c
