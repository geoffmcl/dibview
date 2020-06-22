
/*
 * wgmain.c
 *
 * Copyright (C) 1991, 1992, 1993, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains some WINDOWS entry points to the service provided 
 * by this WJPEG.DLL (library of functions)
 *
 */

#include "winclude.h"

//#ifndef	WJPEG4	// NOTE: Old FILE interface RETIRED April, 1997
//#ifdef	ADDOLDTO

EXPORT32
HGLOBAL MLIBCALL WGIFTOBMP( HGLOBAL, DWORD, HGLOBAL, LPSTR );

//#endif	// !WJPEG4

#ifdef	_INC_WINDOWS
extern	void jmem_term( void );
extern	void winp_term( void );
#ifdef	WIN32
extern	setexitjump( void MLPTR, DWORD );
#else	// !WIN32
extern	setexitjump( void MLPTR, WORD );
#endif	// WIN32 y/n
extern	GetErrorMsg( LPSTR, WORD );
extern	GetWarnMsg( LPSTR );
#else		/* !_INC_WINDOWS */
/* ======================================= */
#ifdef INCLUDES_ARE_ANSI
#include <stdlib.h>		/* to declare exit() */
#endif
#include <ctype.h>		/* to declare isupper(), tolower() */
#ifdef NEED_SIGNAL_CATCHER
#include <signal.h>		/* to declare signal() */
#endif
#ifdef USE_SETMODE
#include <fcntl.h>		/* to declare setmode() */
#endif

#ifdef THINK_C
#include <console.h>		/* command-line reader for Macintosh */
#endif

#endif	/* !_INC_WINDOWS */
/* ======================================= */

#include "wversion.h"		/* for version message */
//#include	"SetExit.h"

#ifdef	WJPEG2	/* We ARE NOT linking with wcmaind.c so ... */
#ifdef	WIN32
#pragma	message( "Building WJPEG2 32-bit DLL ..." )
#else	// !WIN32
#pragma message( "Building WJPEG2 DLL ... " )
#endif	// WIN32 y/n

struct Compress_info_struct WCMcinfo;	/* Define these HERE ... */
struct External_methods_struct e_methods;
struct Compress_methods_struct c_methods;

#else
extern	struct Compress_info_struct WCMcinfo;
extern	struct External_methods_struct e_methods;
extern	struct Compress_methods_struct c_methods;
#endif
#define	MINLibheap			4 * 1024
extern	BYTE	CompInf[MINLibheap];

EXPORT32
WORD MLIBCALL WGIFSIZX( HGLOBAL, DWORD, HGLOBAL );
EXPORT32
WORD MLIBCALL WGIFSIZXT( HGLOBAL, DWORD, HGLOBAL );

extern	struct Decompress_info_struct1 DSdinfo;
extern	struct Decompress_methods_struct DSdc_methods;
extern	struct External_methods_struct DSe_methods;
extern	void SetupJDef( decompress_info_ptr );
#ifdef	PROGRESS_REPORT
extern	void Dprogress_monitor( decompress_info_ptr,
	long, long );
#endif
extern	LIBCONFIG	LibConfig;
BOOL	IsDIB;
HGLOBAL	hgDIB;
LPSTR	lpMBuf = 0;
DWORD	ddBufCnt = 0;
DWORD	ddOutCnt = 0;
DWORD	ddMaxCnt = 0;
JSAMPARRAY pix_row;	// Workspace for a pixel row in input format

#ifdef	WJPEG4	// NEW interface

#ifdef	ADDOLDTO	// but retain some OLD i/f
LPSTR	lpOutFile;	// GLOBAL pointer to OUTPUT FILE ...
#endif	// ADDOLDTO

#else	// !WJPEG4	// NOTE: Older file interface RETIRED - April, 1997
LPSTR	lpOutFile;	// GLOBAL pointer to OUTPUT FILE ...
#endif	// !WJPEG4

#if		( _MSC_VER > 700 )
#if		(defined( NDEBUG ) && defined( Dv16_App ))
// **************************************************************
#pragma warning( disable : 4704 )
// **************************************************************
#endif	// NDEBUG & Dv16_App
#endif	// _MSC_VER > 700

GLOBAL	void
APIStart( void )
{
	lpMBuf = 0;
	ddBufCnt = 0;
	ddOutCnt = 0;
	ddMaxCnt = 0;
//#ifndef	WJPEG4	// NOTE: Older file interface RETIRED - April, 1997
#ifdef	ADDOLDTO
	lpOutFile = 0;	// Request for return of ...
#endif	// !ADDOLDTO ie eventually !WJPEG4
	WCMcinfo.hgOut = 0;	/* Output GLOBAL Object handle */
	WCMcinfo.lpOut = 0;	/* Output pointer ... */
	WCMcinfo.ddOut = 0;	/* Output size ... */
	DSdinfo.hgOut = 0;	/* Output GLOBAL Object handle */
	DSdinfo.lpOut = 0;	/* Output pointer ... */
	DSdinfo.ddOut = 0;	/* Output size ... */
	IsDIB = FALSE;			/* NOT a DIB ... */
}

/*
 * Compression pipeline controller used for single-scan files
 * with no optimization of entropy parameters.
 */
/* Work buffer for pre-downsampling data (see comments at head of file) */
JSAMPIMAGE fs_data[2];

METHODDEF void
single_nul_controller (compress_info_ptr cinfo)
{
  int rows_in_mem;		/* # of sample rows in full-size buffers */
  long fullsize_width;		/* # of samples per row in full-size buffers */
  long cur_pixel_row;		/* counts # of pixel rows processed */
  long mcu_rows_output;		/* # of MCU rows actually emitted */
  int mcu_rows_per_loop;	/* # of MCU rows processed per outer loop */
  /* Work buffer for downsampled data */
//  JSAMPIMAGE sampled_data;
  int rows_this_time;
  short whichss;
  short ci;
  jpeg_component_info MLPTR compptr;
  //  short i;
	cinfo->num_components = 1;
  /* Prepare for single scan containing all components */
//  if( cinfo->num_components > MAX_COMPS_IN_SCAN )
//	{
//#ifdef	_INC_WINDOWS
//		ERRFLAG( BAD_JPGTMC ); /* "Too many components for interleaved scan" */
//		return;
//#else
//    ERREXIT(cinfo->emethods, "Too many components for interleaved scan");
//#endif
//	}
  cinfo->comps_in_scan = cinfo->num_components;
  cinfo->comp_info = (jpeg_component_info MLPTR) &CompInf[0];
  for( ci = 0; ci < cinfo->num_components; ci++ )
  {
	  cinfo->cur_comp_info[ci] = &cinfo->comp_info[ci];
	  compptr = &cinfo->comp_info[ci];
	  cinfo->cur_comp_info[ci] = compptr;
#ifdef	_INC_WINDOWS
	  if( cinfo->cur_comp_info[ci] == 0 )
	  {
		  ERRFLAG( 	BAD_PTRNULL );	//"ERROR: Pointer NOT intialized! "
		  return;
	  }
#endif	// _INC_WINDOWS
  }
  if (cinfo->comps_in_scan == 1) {
//    noninterleaved_scan_setup(cinfo);
    /* Vk block rows constitute the same number of MCU rows */
    mcu_rows_per_loop = cinfo->cur_comp_info[0]->v_samp_factor;
  } else {
//    interleaved_scan_setup(cinfo);
    /* in an interleaved scan, one MCU row contains Vk block rows */
    mcu_rows_per_loop = 1;
  }
  cinfo->total_passes++;

  /* Compute dimensions of full-size pixel buffers */
  /* Note these are the same whether interleaved or not. */
  rows_in_mem = cinfo->max_v_samp_factor * DCTSIZE;
  fullsize_width = jround_up(cinfo->image_width,
			     (long) (cinfo->max_h_samp_factor * DCTSIZE));

  /* Allocate working memory: */
  /* fs_data is sample data before downsampling */
//  alloc_sampling_buffer(cinfo, fs_data, fullsize_width);
  /* sampled_data is sample data after downsampling */
//  sampled_data = (JSAMPIMAGE) (*cinfo->emethods->alloc_small)
//				(cinfo->num_components * SIZEOF(JSAMPARRAY));
//  for (ci = 0; ci < cinfo->num_components; ci++) {
//   sampled_data[ci] = (*cinfo->emethods->alloc_small_sarray)
//			(cinfo->comp_info[ci].downsampled_width,
//			 (long) (cinfo->comp_info[ci].v_samp_factor * DCTSIZE));
//  }

  /* Tell the memory manager to instantiate big arrays.
   * We don't need any big arrays in this controller,
   * but some other module (like the input file reader) may need one.
   */
  (*cinfo->emethods->alloc_big_arrays)
	((long) 0,				/* no more small sarrays */
	 (long) 0,				/* no more small barrays */
	 (long) 0);				/* no more "medium" objects */

  /* Initialize output file & do per-scan object init */

  (*cinfo->methods->write_scan_header) (cinfo);
  cinfo->methods->entropy_output = cinfo->methods->write_jpeg_data;
//  (*cinfo->methods->entropy_encode_init) (cinfo);
//  (*cinfo->methods->downsample_init) (cinfo);
//  (*cinfo->methods->extract_init) (cinfo);

  /* Loop over input image: rows_in_mem pixel rows are processed per loop */

  mcu_rows_output = 0;
  whichss = 1;			/* arrange to start with fs_data[0] */

  for (cur_pixel_row = 0; cur_pixel_row < cinfo->image_height;
       cur_pixel_row += rows_in_mem) {
#ifdef	PROGRESS_REPORT
    (*cinfo->methods->progress_monitor) (cinfo, cur_pixel_row,
					 cinfo->image_height);
#endif
    whichss ^= 1;		/* switch to other fs_data buffer */
    
    /* Obtain rows_this_time pixel rows and expand to rows_in_mem rows. */
    /* Then we have exactly DCTSIZE row groups for downsampling. */   
    rows_this_time = (int) MIN((long) rows_in_mem,
			       cinfo->image_height - cur_pixel_row);
 
    (*cinfo->methods->get_sample_rows) (cinfo, rows_this_time,
					fs_data[whichss]);
//    (*cinfo->methods->edge_expand) (cinfo,
//				    cinfo->image_width, rows_this_time,
//				    fullsize_width, rows_in_mem,
//				    fs_data[whichss]);
    
    /* Downsample the data (all components) */
    /* First time through is a special case */
    
//    if (cur_pixel_row) {
      /* Downsample last row group of previous set */
//      downsample(cinfo, fs_data[whichss], sampled_data, fullsize_width,
//		 (short) DCTSIZE, (short) (DCTSIZE+1), (short) 0,
//		 (short) (DCTSIZE-1));
      /* and dump the previous set's downsampled data */
//      (*cinfo->methods->extract_MCUs) (cinfo, sampled_data, 
//				       mcu_rows_per_loop,
//				       cinfo->methods->entropy_encode);
//      mcu_rows_output += mcu_rows_per_loop;
      /* Downsample first row group of this set */
//      downsample(cinfo, fs_data[whichss], sampled_data, fullsize_width,
//		 (short) (DCTSIZE+1), (short) 0, (short) 1,
//		 (short) 0);
//    } else {
      /* Downsample first row group with dummy above-context */
//      downsample(cinfo, fs_data[whichss], sampled_data, fullsize_width,
//		 (short) (-1), (short) 0, (short) 1,
//		 (short) 0);
//    }
    /* Downsample second through next-to-last row groups of this set */
//    for (i = 1; i <= DCTSIZE-2; i++) {
//      downsample(cinfo, fs_data[whichss], sampled_data, fullsize_width,
//		 (short) (i-1), (short) i, (short) (i+1),
//		 (short) i);
//    }
  } /* end of outer loop */
  
  /* Downsample the last row group with dummy below-context */
  /* Note whichss points to last buffer side used */
//  downsample(cinfo, fs_data[whichss], sampled_data, fullsize_width,
//	     (short) (DCTSIZE-2), (short) (DCTSIZE-1), (short) (-1),
//	     (short) (DCTSIZE-1));
  /* Dump the remaining data (may be less than full height if uninterleaved) */
//  (*cinfo->methods->extract_MCUs) (cinfo, sampled_data, 
//		(int) (cinfo->MCU_rows_in_scan - mcu_rows_output),
//		cinfo->methods->entropy_encode);

  /* Finish output file */
//  (*cinfo->methods->extract_term) (cinfo);
//  (*cinfo->methods->downsample_term) (cinfo);
//  (*cinfo->methods->entropy_encode_term) (cinfo);
//  (*cinfo->methods->write_scan_trailer) (cinfo);
  cinfo->completed_passes++;

  /* Release working memory */
  /* (no work -- we let free_all release what's needful) */
}


/*
 * This routine determines what format the input file is,
 * and selects the appropriate input-reading module.
 */
LOCAL void
select_GIF_type (compress_info_ptr cinfo)
{
    jselrgif(cinfo);
}

METHODDEF void
write_nul (compress_info_ptr ci)
{
}
#ifdef	PROGRESS_REPORT
METHODDEF void
Gprogress_monitor (compress_info_ptr cinfo, long loopcounter, long looplimit)
{
}
#endif

// FIX16 - 5
METHODDEF void
write_nul_data (compress_info_ptr cin, char MLPTR dptr, int dcnt)
{
}


METHODDEF void
rgb_nul_term (compress_info_ptr cinfo)
{
  /* no work (we let free_all release the workspace) */
}
METHODDEF void
rgb_nul_init (compress_info_ptr cinfo)
{
  /* Allocate a workspace for the result of get_input_row. */
  pix_row = (*cinfo->emethods->alloc_small_sarray)
		(cinfo->image_width, (long) cinfo->input_components);
}



METHODDEF void
get_rgb_nul_rows (compress_info_ptr cinfo,
		  int rows_to_read, JSAMPIMAGE image_data)
{
  long width = cinfo->image_width;
  int row;

	for (row = 0; row < rows_to_read; row++) 
	{
    /* Read one row from the source file */
    (*cinfo->methods->get_input_row) (cinfo, pix_row);
	}
}


GLOBAL void
jselwnul (compress_info_ptr cinfo)
{
  cinfo->methods->write_file_header = write_nul;
  cinfo->methods->write_scan_header = write_nul;
  cinfo->methods->write_jpeg_data = write_nul_data;
  cinfo->methods->write_scan_trailer = write_nul;
  cinfo->methods->write_file_trailer = write_nul;
}

/*
 * This routine gets control after the input file header has been read.
 * It must determine what output JPEG file format is to be written,
 * and make any other compression parameter changes that are desirable.
 */

METHODDEF void
c_NUL_method_selection (compress_info_ptr cinfo)
{
  cinfo->methods->c_pipeline_controller = single_nul_controller;
  jselwnul(cinfo);
}

/*
 * This is the main entry point to the JPEG compressor.
 */


GLOBAL void
jpeg_wnull (compress_info_ptr cinfo)
{
  /* Init pass counts to 0 --- total_passes is adjusted in method selection */
  cinfo->total_passes = 0;
  cinfo->completed_passes = 0;

	cinfo->methods->colorin_init = rgb_nul_init;
	cinfo->methods->colorin_term = rgb_nul_term;
	cinfo->methods->get_sample_rows = get_rgb_nul_rows;

  /* Read the input file header: determine image size & component count.
   * NOTE: the user interface must have initialized the input_init method
   * pointer (eg, by calling jselrppm) before calling me.
   * The other file reading methods (get_input_row etc.) were probably
   * set at the same time, but could be set up by input_init itself,
   * or by c_ui_method_selection.
   */
  (*cinfo->methods->input_init) (cinfo);
#ifdef	_INC_WINDOWS
	if( bErrFlag )
		goto Free_Em;
#endif		
  /* Give UI a chance to adjust compression parameters and select */
  /* output file format based on results of input_init. */
  (*cinfo->methods->c_ui_method_selection) (cinfo);

  /* Now select methods for compression steps. */
//  initial_setup(cinfo);
  cinfo->max_h_samp_factor = 1;
  cinfo->max_v_samp_factor = 1;
//  c_initial_method_selection(cinfo);

  /* Initialize the output file & other modules as needed */
  /* (entropy_encoder is inited by pipeline controller) */

  (*cinfo->methods->colorin_init) (cinfo);
  (*cinfo->methods->write_file_header) (cinfo);

  /* And let the pipeline controller do the rest. */
  (*cinfo->methods->c_pipeline_controller) (cinfo);

  /* Finish output file, release working storage, etc */
  (*cinfo->methods->write_file_trailer) (cinfo);
  (*cinfo->methods->colorin_term) (cinfo);
  (*cinfo->methods->input_term) (cinfo);
#ifdef	_INC_WINDOWS
Free_Em:
#endif
  (*cinfo->emethods->free_all) ();

  /* My, that was easy, wasn't it? */
}


/*
 * The main APIS.
 */
// NOTE: Old FILE Interface RETIRED - April, 1997
// =============================================
//#ifndef	WJPEG4	// Not RETIRED!
#ifdef	ADDOLDTO

// WJPEG Library ENTRY
// ###################
EXPORT32
HGLOBAL MLIBCALL
WGIFTOBMP( HGLOBAL hg, DWORD sz, HGLOBAL hIB, LPSTR lpof )
{
	LPSTR	lpInfoBuf;

	APIStart();
	lpOutFile = lpof;	/* Either a POINTER to a FILE or NULL ... */
	winp_term();	/* and all inputs cleared ... */
	bErrFlag = 0;	/* Global ERROR flag (if NON-ZERO) return ... */
	msgout = &szMsgBuf[0];	/* Setup a MESSAGE buffer ... */
    
#ifndef WIN64
#ifdef	WIN32
//	SETEXIT( GMAINRET );
	_asm 
	{
		mov  eax,offset GMAINRET ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump ;
		add  esp,2 * 4 ;
	}
#else	// !WIN32

	_asm
	{
		mov	ax,offset GMAINRET
		mov	dx,cs		; Return JUMP address ...
		mov	cx,sp		; and STACK frame ...
		push	cx
		push	dx
		push	ax
		call	setexitjump	; Save these items ...
		add	sp,6
	}
#endif	// WIN32 y/n
#endif // #ifndef WIN64

  /* Set up links to method structures. */
  WCMcinfo.methods = &c_methods;
  WCMcinfo.emethods = &e_methods;

  /* (Re-)initialize the system-dependent error and memory managers. */
  jselerror(WCMcinfo.emethods);	/* error/trace message routines */
  jselmemmgr(WCMcinfo.emethods);	/* memory allocation routines */
  WCMcinfo.methods->c_ui_method_selection = c_NUL_method_selection;
	if( hg &&
		( ddMaxCnt = sz ) &&
		( lpMBuf = JGlobalLock( hg ) ) )
	{
		ddBufCnt = sz;
	}
	else
	{
		ERRFLAG( BAD_G2B_ENTRY );	/* "ERROR: Input parameter to LIBRARY errant! " */
		goto gmain_ret;
	}	
  /* Figure out the input file format, and set up to read it. */
  select_GIF_type( &WCMcinfo );
	if( bErrFlag )
		goto gmain_ret;
#ifdef	PROGRESS_REPORT
	c_methods.progress_monitor = Gprogress_monitor;
#endif
  /* Do it to it! */
  jpeg_wnull( &WCMcinfo );

  /* All done. */
#ifndef WIN64
#ifdef	WIN32
	_asm
	{
GMAINRET:
		mov		eax, [bErrFlag]
	}
#else	// !WIN#@
	_asm
	{
GMAINRET:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}
#endif	// WIN#@ y/n
#endif // #ifndef WIN64

gmain_ret:
	jmem_term();	/* Ensure ALL memory freed ... */
	winp_term();	/* and all inputs cleared ... */
	if( hg && lpMBuf )
		JGlobalUnlock( hg );
	if( hIB && (lpInfoBuf = JGlobalLock( hIB )) )
	{
		lpInfoBuf[0] = 0;
		if( bErrFlag )
		{
			GetErrorMsg( lpInfoBuf, (WORD)bErrFlag );
		}
		GetWarnMsg( lpInfoBuf );
		JGlobalUnlock( hIB );
	}
	return( hgDIB );
}	// WJPEG Library EXIT
// ###################

#endif	// ADDOLDTO = unretired

// WJPEG Library ENTRY
// ###################
EXPORT32
BOOL MLIBCALL
WGIF2BMP( HGLOBAL hg, DWORD sz, HGLOBAL hIB, HGLOBAL hOut )
{
	DWORD	ddSz;
	SIZE	cxcy;
	LPSTR	lpInfoBuf;
	if( hg && sz && hOut )
	{
		if( LibConfig.cf_Safety_Check )
		{
			if( ( ddSz = WGIFSIZE( hg, sz, &cxcy ) ) &&
				( GlobalSize( hOut ) >= ddSz ) )
			{
				WCMcinfo.ddOut = 0;	/* No output yet ... */
			}
			else
			{
				bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Input parameter to LIBRARY errant! " */
					goto g2main_ret;
			}
		}
	}
	else
	{
		bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Input parameter to LIBRARY errant! " */
		goto g2main_ret;
	}
	APIStart();
	winp_term();	/* and all inputs cleared ... */
	bErrFlag = 0;	/* Global ERROR flag (if NON-ZERO) return ... */
	msgout = &szMsgBuf[0];	/* Setup a MESSAGE buffer ... */

#ifndef WIN64
#ifdef	WIN32
//	SETEXIT( G2MAINRET );
	_asm 
	{
		mov  eax,offset G2MAINRET ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump ;
		add  esp,2 * 4 ;
	}
#else	// !WIN32

	_asm
	{
		mov	ax,offset G2MAINRET
		mov	dx,cs		; Return JUMP address ...
		mov	cx,sp		; and STACK frame ...
		push	cx
		push	dx
		push	ax
		call	setexitjump	; Save these items ...
		add	sp,6
	}

#endif	// WIN32 y/n
#endif // #ifndef WIN64

	/* Set up links to method structures. */
  WCMcinfo.methods = &c_methods;
  WCMcinfo.emethods = &e_methods;
  /* (Re-)initialize the system-dependent error and memory managers. */
  jselerror(WCMcinfo.emethods);	/* error/trace message routines */
  jselmemmgr(WCMcinfo.emethods);	/* memory allocation routines */
  WCMcinfo.methods->c_ui_method_selection = c_NUL_method_selection;
	/* Setup INPUT data ... */
	if( hg &&
		( ddMaxCnt = sz ) &&
		( lpMBuf = JGlobalLock( hg ) ) )
	{
		ddBufCnt = sz;
	}
	else
	{
		bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Input parameter to LIBRARY errant! " */
		goto g2main_ret;
	}	
	/* Setup OUTPUT data ... */
	WCMcinfo.hgOut = hOut;
  /* Figure out the input file format, and set up to read it. */
  select_GIF_type( &WCMcinfo );
	if( bErrFlag )
		goto g2main_ret;
#ifdef	PROGRESS_REPORT
	c_methods.progress_monitor = Gprogress_monitor;
#endif
  /* Do it to it! */
  jpeg_wnull( &WCMcinfo );
  /* All done. */
#ifndef WIN64
#ifdef	WIN32
	_asm
	{
G2MAINRET:
		mov		eax, [bErrFlag]
	}
#else	// !WIN#@
	_asm
	{
G2MAINRET:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}
#endif	// WIN#@ y/n
#endif // #ifndef WIN64

g2main_ret:
	jmem_term();	/* Ensure ALL memory freed ... */
	winp_term();	/* and all inputs cleared ... */
	if( hIB && (lpInfoBuf = JGlobalLock( hIB ) ) )
	{
		lpInfoBuf[0] = 0;
		if( bErrFlag )
		{
			GetErrorMsg( lpInfoBuf, (WORD)bErrFlag );
		}
		GetWarnMsg( lpInfoBuf );
		JGlobalUnlock( hIB );
	}
	if( hg && lpMBuf )
	{
		JGlobalUnlock( hg );
		lpMBuf = 0;
	}
	if( WCMcinfo.hgOut && WCMcinfo.lpOut )
	{
		JGlobalUnlock( WCMcinfo.hgOut );
		WCMcinfo.lpOut = 0;
	}
	return( bErrFlag );
}	// WJPEG Library EXIT
// ###################


extern	void jselrgifn( compress_info_ptr, WORD );
extern	void jselrgifx( compress_info_ptr, WORD );

// WJPEG Library ENTRY
// ###################
EXPORT32
BOOL MLIBCALL
WGIFNBMP( HGLOBAL hg, DWORD sz, HGLOBAL hIB, HGLOBAL hOut, WORD num )
{
	DWORD		ddSz;
	LPGIFHDR	lpgh;
	LPGIFIMG	lpgi;
	LPSTR		lpInfoBuf;
	HGLOBAL		hGH;

	hGH = 0;
	lpgh = 0;
	if( hg && sz && hOut && num )
	{
		if( ( LibConfig.cf_Safety_Check ) &&
			( hGH = JGlobalAlloc( GHND, 
			(sizeof(GIFHDR) + (sizeof(GIFIMG) * (num+1))) ) ) &&
			( lpgh = (LPGIFHDR) JGlobalLock( hGH ) ) )
		{
			lpgh->ghMaxCount = (num+1);
			JGlobalUnlock( hGH );
			if( WGIFSIZX( hg, sz, hGH ) )
				goto Bad_Entry;
			if( ( lpgh = (LPGIFHDR) JGlobalLock( hGH ) ) &&
				( num <= lpgh->ghImgCount ) )
			{
				ddSz = lpgh->ghBMPSize;
				// Correction: 20Apr97 - This should be [num-1]!
				// num has already been checked as NOT 0
				// thus MUST be reduced by 1 to the logical
				// count of the image!!!
				//lpgi = &lpgh->ghGifImg[num];
				lpgi = &lpgh->ghGifImg[num-1];
				ddSz = lpgi->giBMPSize;
				JGlobalUnlock( hGH );
				JGlobalFree( hGH );
				lpgh = 0;
				hGH = 0;
				if( GlobalSize( hOut ) < ddSz )
					goto Bad_Entry;
			}
			else
				goto Bad_Entry;
		}	/* Safety = LibConfig.cf_Safety_Check */
	}
	else
	{
Bad_Entry:
		bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Input parameter to LIBRARY errant! " */
		goto gnmain_ret;
	}
	APIStart();
	winp_term();	/* and all inputs cleared ... */
	bErrFlag = 0;	/* Global ERROR flag (if NON-ZERO) return ... */
	msgout = &szMsgBuf[0];	/* Setup a MESSAGE buffer ... */
    
#ifndef WIN64
#ifdef	WIN32
//	SETEXIT( GNMAINRET );
	_asm 
	{
		mov  eax,offset GNMAINRET ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump ;
		add  esp,2 * 4 ;
	}
#else	// !WIN32

	_asm
	{
		mov	ax,offset GNMAINRET
		mov	dx,cs		; Return JUMP address ...
		mov	cx,sp		; and STACK frame ...
		push	cx
		push	dx
		push	ax
		call	setexitjump	; Save these items ...
		add	sp,6
	}

#endif	// WIN32 y/n
#endif // #ifndef WIN64

	/* Set up links to method structures. */
	WCMcinfo.methods = &c_methods;
	WCMcinfo.emethods = &e_methods;

	/* (Re-)initialize the system-dependent error and memory managers. */
	jselerror(WCMcinfo.emethods);	/* error/trace message routines */
	jselmemmgr(WCMcinfo.emethods);	/* memory allocation routines */

	WCMcinfo.methods->c_ui_method_selection = c_NUL_method_selection;

	/* Setup INPUT data ... */
	if( hg &&
		( ddMaxCnt = sz ) &&
		( lpMBuf = JGlobalLock( hg ) ) )
	{
		ddBufCnt = sz;
	}
	else
	{
		bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Input parameter to LIBRARY errant! " */
		goto gnmain_ret;
	}	
	/* Setup OUTPUT data ... */
	WCMcinfo.hgOut = hOut;
	/* Figure out the input file format, and set up to read it. */
	jselrgifn( &WCMcinfo, num );
	if( bErrFlag )
		goto gnmain_ret;
#ifdef	PROGRESS_REPORT
	c_methods.progress_monitor = Gprogress_monitor;
#endif
	/* Do it to it! */
	jpeg_wnull( &WCMcinfo );
    
  /* All done. */
#ifndef WIN64
#ifdef	WIN32
	_asm
	{
GNMAINRET:
		mov		eax, [bErrFlag]
	}
#else	// !WIN#@
	_asm
	{
GNMAINRET:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}
#endif	// WIN32 y/n
#endif // #ifndef WIN64

gnmain_ret:
	jmem_term();	/* Ensure ALL memory freed ... */
	winp_term();	/* and all inputs cleared ... */
	if( hIB && (lpInfoBuf = JGlobalLock( hIB ) ) )
	{
		lpInfoBuf[0] = 0;
		if( bErrFlag )
		{
			GetErrorMsg( lpInfoBuf, (WORD)bErrFlag );
		}
		GetWarnMsg( lpInfoBuf );
		JGlobalUnlock( hIB );
	}
	if( hg && lpMBuf )
	{
		JGlobalUnlock( hg );
		lpMBuf = 0;
	}
	if( WCMcinfo.hgOut && WCMcinfo.lpOut )
	{
		JGlobalUnlock( WCMcinfo.hgOut );
		WCMcinfo.lpOut = 0;
	}
	if( hGH && lpgh )
		JGlobalUnlock( hGH );
	if( hGH )
		JGlobalFree( hGH );
	return( bErrFlag );
}	// WJPEG Library EXIT
// ###################


/* Where -
 * HGLOBAL hInputData;	// Handle to INPUT DATA Buffer
 * DWORD   ddInputSize;	// Size   of INPUT DATA
 * HGLOBAL hInfo;			// Handle to Information buffer (1K) May be NULL
 * HGLOBAL hOutputData;	// Handle to OUTPUT DATA Buffer
 * WORD    wImageNum;	// Image NUMBER to convert ...
 * If SUCCESS function will return ZERO. Otherwise an ERROR VALUE per
 * the above list will be returned.
 * Naturally OUTPUT Data is only valid if ZERO is returned.
 * NOTE 1: When conversion of Image 1 is requested, this function
 * returns a BITMAP exactly equivalent to the Logical Screen size,
 * with the first Image, (or Plain Text Extension) correctly placed
 * into the BITMAP.
 * NOTE 2: However, each successive call will only return a BITMAP
 * equivalent to the next Image, or Plain Text Extension, thus the
 * application must correctly place this image onto the Logical
 * Screen.
 * ------------------------------------------------------------------ */
// WJPEG Library ENTRY
// ###################
EXPORT32
BOOL MLIBCALL
WGIFNBMPX( HGLOBAL hg, DWORD sz, HGLOBAL hIB, HGLOBAL hOut, WORD num )
{
	DWORD	ddSz;
	LPGIFHDREXT	lpgh;
	LPGIFIMGEXT	lpgi;
	LPSTR	lpInfoBuf;
	HGLOBAL	hGH;

	hGH = 0;
	lpgh = 0;
	if( hg && sz && hOut )
	{
		if( ( LibConfig.cf_Safety_Check ) &&
			( hGH = JGlobalAlloc( GHND, 
				(sizeof(GIFHDREXT) + (sizeof(GIFIMGEXT) * (num+1))) ) ) &&
			( lpgh = (LPGIFHDREXT) JGlobalLock( hGH ) ) )
		{
			lpgh->gheMaxCount = (gie_Flag + (num+1));
			JGlobalUnlock( hGH );
			if( WGIFSIZXT( hg, sz, hGH ) )
				goto Bad_Entryx;
			if( ( lpgh = (LPGIFHDREXT) JGlobalLock( hGH ) ) &&
				( num <= lpgh->gheImgCount ) )
			{
				if( num )
				{
					lpgi = &lpgh->gheGIE[num-1];
					ddSz = lpgi->giGI.giBMPSize;
				}
				else
				{
					ddSz = lpgh->gheBMPSize;
				}
				JGlobalUnlock( hGH );
				JGlobalFree( hGH );
				lpgh = 0;
				hGH = 0;
				if( GlobalSize( hOut ) < ddSz )
					goto Bad_Entryx;
			}
			else
				goto Bad_Entryx;
		}	// Safety = LibConfig.cf_Safety_Check
	}
	else
	{
Bad_Entryx:
		bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Input parameter to LIBRARY errant! " */
		goto gnmain_retx;
	}
	APIStart();
	winp_term();	/* and all inputs cleared ... */
	bErrFlag = 0;	/* Global ERROR flag (if NON-ZERO) return ... */
	msgout = &szMsgBuf[0];	/* Setup a MESSAGE buffer ... */
    
#ifndef WIN64
#ifdef	WIN32
//	SETEXIT( GNMAINRET );
	_asm 
	{
		mov  eax,offset GNMAINRETX ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump ;
		add  esp,2 * 4 ;
	}
#else	// !WIN32

	_asm
	{
		mov	ax,offset GNMAINRETX
		mov	dx,cs		; Return JUMP address ...
		mov	cx,sp		; and STACK frame ...
		push	cx
		push	dx
		push	ax
		call	setexitjump	; Save these items ...
		add	sp,6
	}

#endif	// WIN32 y/n
#endif // #ifndef WIN64

	/* Set up links to method structures. */
	WCMcinfo.methods = &c_methods;
	WCMcinfo.emethods = &e_methods;

	/* (Re-)initialize the system-dependent error and memory managers. */
	jselerror(WCMcinfo.emethods);	/* error/trace message routines */
	jselmemmgr(WCMcinfo.emethods);	/* memory allocation routines */

	WCMcinfo.methods->c_ui_method_selection = c_NUL_method_selection;

	/* Setup INPUT data ... */
	if( hg &&
		( ddMaxCnt = sz ) &&
		( lpMBuf = JGlobalLock( hg ) ) )
	{
		ddBufCnt = sz;
	}
	else
	{
		bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Input parameter to LIBRARY errant! " */
		goto gnmain_retx;
	}

	/* Setup OUTPUT data ... */
	WCMcinfo.hgOut = hOut;

	/* Figure out the input file format, and set up to read it. */

	jselrgifx( &WCMcinfo, num );

	if( bErrFlag )
		goto gnmain_retx;
#ifdef	PROGRESS_REPORT
	c_methods.progress_monitor = Gprogress_monitor;
#endif

	/* Do it to it! */
	jpeg_wnull( &WCMcinfo );
	// bErrFlag = BAD_NOTIMP;
    
	/* All done. */
#ifndef WIN64
#ifdef	WIN32
	_asm
	{
GNMAINRETX:
		mov		eax, [bErrFlag]
	}
#else	// !WIN#@
	_asm
	{
GNMAINRETX:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}
#endif	// WIN32 y/n
#endif // #ifndef WIN64

gnmain_retx:
	jmem_term();	/* Ensure ALL memory freed ... */
	winp_term();	/* and all inputs cleared ... */
	if( hIB && (lpInfoBuf = JGlobalLock( hIB ) ) )
	{
		lpInfoBuf[0] = 0;
		if( bErrFlag )
		{
			GetErrorMsg( lpInfoBuf, (WORD)bErrFlag );
		}
		GetWarnMsg( lpInfoBuf );
		JGlobalUnlock( hIB );
	}
	if( hg && lpMBuf )
	{
		JGlobalUnlock( hg );
		lpMBuf = 0;
	}
	if( WCMcinfo.hgOut && WCMcinfo.lpOut )
	{
		JGlobalUnlock( WCMcinfo.hgOut );
		WCMcinfo.lpOut = 0;
	}
	if( hGH && lpgh )
		JGlobalUnlock( hGH );
	if( hGH )
		JGlobalFree( hGH );
	return( bErrFlag );
}	// WJPEG Library EXIT
// ###################


// WJPEG Library ENTRY
// ###################
EXPORT32
BOOL MLIBCALL
WJPG2BMP( HGLOBAL hg, DWORD sz, HGLOBAL hIB, HGLOBAL hOut )
{
	DWORD	ddSz;
	SIZE	cxcy;
	LPSTR	lpI;

	bErrFlag = 0;	/* No error yet ... */
	if( hg && sz && hOut )
	{
		if( LibConfig.cf_Safety_Check )
		{
			if( ( ddSz = WJPGSIZE( hg, sz, &cxcy ) ) &&
				( GlobalSize( hOut ) >= ddSz ) )
			{
				if( bErrFlag )	/* Any error? */
					goto j2_Bad;
			}
			else
			{
				goto j2_Bad;
			}
		}	/* Safety ON */
	}
	else
	{
j2_Bad:
		bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Input parameter to LIBRARY errant! " */
		goto j2main_ret;
	}
	APIStart();
	winp_term();	/* and ensure any pending input cleared ... */
	msgout = &szMsgBuf[0];  /* Setup a MESSAGE buffer ... */ 

#ifndef WIN64
#ifdef	WIN32
	_asm
	{
		mov		eax,offset J2MAINRET
		mov		edx, esp
		push	edx
		push	eax
		call	setexitjump
		add		esp,2 * 4
	}

#else	// !WIN32

	_asm
	{
		mov	ax,offset J2MAINRET
		mov	dx,cs		; Return JUMP address ...
		mov	cx,sp		; and STACK frame ...
		push	cx
		push	dx
		push	ax
		call	setexitjump	; Save these items ...
		add	sp,6
	}

#endif	// WIN32 y/n
#endif // #ifndef WIN64

  /* Set up links to method structures. */
  DSdinfo.methods = &DSdc_methods;
  DSdinfo.emethods = &DSe_methods;

	/* NOTE: In original HEADERS these were ALL done in parse_switches!!! */
	SetupJDef( &DSdinfo );

  /* Open the input file. */
	if( hg &&
		( ddMaxCnt = sz ) &&
		( lpMBuf = JGlobalLock( hg ) ) )
	{
		ddBufCnt = sz;
	}
	else
	{
		ERRFLAG( BAD_G2B_ENTRY );	/* "ERROR: Input parameter to LIBRARY errant! " */
		goto j2main_ret;
	}	

  /* Open the output file. */
	DSdinfo.hgOut = hOut;
  
  /* Set up to read a JFIF or baseline-JPEG file. */
  /* A smarter UI would inspect the first few bytes of the input file */
  /* to determine its type. */
#ifdef JFIF_SUPPORTED
  jselrjfif(&DSdinfo);
#else
  You shoulda defined JFIF_SUPPORTED.   /* deliberate syntax error */
#endif

#ifdef PROGRESS_REPORT
  /* Start up progress display, unless trace output is on */
  if (DSe_methods.trace_level == 0)
    DSdc_methods.progress_monitor = Dprogress_monitor;
#endif

  // Do it to it! ie READ JPG file and OUPUT in requested format
  // Called from WJPG2BMP
  jpeg_decompress(&DSdinfo);

  /* All done. */
#ifndef WIN64
#ifdef	WIN32
	_asm
	{
J2MAINRET:
		mov		eax, [bErrFlag]
	}
#else	// !WIN#@
	_asm
	{
J2MAINRET:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}
#endif	// WIN32 y/n
#endif // #ifndef WIN64

j2main_ret:
	jmem_term();	/* Ensure ALL memory freed ... */
	winp_term();	/* and ensure any pending input cleared ... */
	if( hg && lpMBuf )
	{
		JGlobalUnlock( hg );
		lpMBuf = 0;
	}
	if( hIB && (lpI = JGlobalLock( hIB )) )
	{
		lpI[0] = 0;
		if( bErrFlag )
			GetErrorMsg( lpI, (WORD)bErrFlag );
		GetWarnMsg( lpI );
		JGlobalUnlock( hIB );
	}
#ifdef	PROBGOBJ
	if( bErrFlag == 0 )
	{
		if( hgDIB == 0 )
		{
			hgDIB = (HGLOBAL) -10;	/* Set NZ, but set neg 10 until fixed ... */
		}
	}
#endif	/* PROBGOBJ */
	if( DSdinfo.hgOut && DSdinfo.lpOut )
	{
		JGlobalUnlock( DSdinfo.hgOut );
		DSdinfo.lpOut = 0;
	}
	return( bErrFlag );
}	// WJPEG Library EXIT
// ###################


extern	DWORD get_gif_size( compress_info_ptr );

// WJPEG Library ENTRY
// ###################
EXPORT32
DWORD MLIBCALL
WGIFSIZE( HGLOBAL hIn, DWORD ddSz, LPSIZE lpSz )
{
	DWORD	gsize;
	gsize = 0;
	lpMBuf = 0;
	bErrFlag = 0;	/* No error yet ... */
	APIStart();
	winp_term();	/* and ensure any pending input cleared ... */
  /* Open the input file. */
	if( ddSz && hIn && (lpMBuf = JGlobalLock( hIn )) )
	{
		ddMaxCnt = ddBufCnt = ddSz;
	}
	else
		goto wg_ret;

#ifndef WIN64
#ifdef	WIN32
//	SETEXIT( WGRET );
	_asm 
	{
		mov  eax,offset WGRET ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump ;
		add  esp,2 * 4 ;
	}
#else	// !WIN32

	_asm
	{
		mov	ax,offset WGRET
		mov	dx,cs		; Return JUMP address ...
		mov	cx,sp		; and STACK frame ...
		push	cx
		push	dx
		push	ax
		call	setexitjump	; Save these items ...
		add	sp,6
	}
#endif	// WIN32 y/n
#endif // #ifndef WIN64

	gsize = get_gif_size( &WCMcinfo );
    
#ifndef WIN64
#ifdef	WIN32
	_asm
	{
WGRET:
		mov		eax, [bErrFlag]
	}
#else	// !WIN#@
	_asm
	{
WGRET:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}
#endif	// WIN32 y/n
#endif // #ifndef WIN64

	if( lpSz && (bErrFlag == 0) )
	{
		lpSz->cx = (int) WCMcinfo.image_width;
		lpSz->cy = (int) WCMcinfo.image_height;
	}
wg_ret:
	if( hIn && lpMBuf )
	{
		JGlobalUnlock( hIn );
		lpMBuf = 0;
	}
	return( gsize );
}	// WJPEG Library EXIT
// ###################


extern	WORD get_gif_sizeX( compress_info_ptr, LPGIFHDR );
extern	WORD get_gif_sizeXT( compress_info_ptr, LPGIFHDREXT );
// NOTE: Because this can be an EXTENDED size request
// we can not make ANY assumptions about the structure
// EXCEPT the SHARED first MEMBER.

// WJPEG Library ENTRY
// ###################
EXPORT32
WORD MLIBCALL
WGIFSIZX( HGLOBAL hIn, DWORD ddSz, HGLOBAL hgI )
{
	LPGIFHDR lpgh;	// NOTE: This can be a LPGIFHDREXT!!!

	bErrFlag = 0;	/* No error yet ... */
	APIStart();
	winp_term();	/* and ensure any pending input cleared ... */

	lpgh = 0;
	// Get INPUT DATA buffer
	if( ddSz &&
		hIn &&
		(lpMBuf = JGlobalLock( hIn )) )
	{
		ddMaxCnt = ddBufCnt = ddSz;
	}
	else
	{
		bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Input parameter to LIBRARY errant! " */
		goto wgx_ret;
	}
	if( !( hgI && (lpgh = (LPGIFHDR) JGlobalLock( hgI ) ) ) )
	{
		bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Input parameter to LIBRARY errant! " */
		goto wgx_ret;
	}

#ifndef WIN64
#ifdef	WIN32
//	SETEXIT( WGXRET );
	_asm 
	{
		mov  eax,offset WGXRET ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump ;
		add  esp,2 * 4 ;
	}
#else	// !WIN32

	_asm
	{
		mov	ax,offset WGXRET
		mov	dx,cs		; Return JUMP address ...
		mov	cx,sp		; and STACK frame ...
		push	cx
		push	dx
		push	ax
		call	setexitjump	; Save these items ...
		add	sp,6
	}
#endif	// WIN32 y/n
#endif //#ifndef WIN64

	bErrFlag = get_gif_sizeX( &WCMcinfo, lpgh );

#ifndef WIN64
#ifdef	WIN32
	_asm
	{
WGXRET:
		mov		eax, [bErrFlag]
	}
#else	// !WIN#@
	_asm
	{
WGXRET:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}
#endif	// WIN32 y/n
#endif // #ifndef WIN64

wgx_ret:
	if( hIn && lpMBuf )
	{
		JGlobalUnlock( hIn );
		lpMBuf = 0;
	}
	if( hgI && lpgh )
		JGlobalUnlock( hgI );

	return( bErrFlag );

}	/* WGIFSIZX */
// WJPEG Library EXIT
// ###################

/* Where -
 * HGLOBAL hInputData;	// Handle to INPUT DATA Buffer
 * DWORD   ddInputSize;	// Size   of INPUT DATA
 * HGLOBAL hOutInfo;	// Handle to structure GIFHDREXT, with gheMaxCount
 *                      // member set to count of GIFIMGEXT structures
 *						// plus the gie_Flag.
 * If SUCCESS then a ZERO return, otherwise and ERROR VALUE per the above.
 * NOTE: If successful, the gheImgCount member of the structure
 * will contain the total number of IMAGES and Plain Text Extensions
 * found.
 * ----------------------------------------------------------------- */
// WJPEG Library ENTRY
// ###################
EXPORT32
WORD MLIBCALL
WGIFSIZXT( HGLOBAL hIn, DWORD ddSz, HGLOBAL hgI )
{
	LPGIFHDREXT lpgh;	// This IS LPGIFHDREXT!!!

	bErrFlag = 0;	/* No error yet ... */
	APIStart();
	winp_term();	/* and ensure any pending input cleared ... */

	lpgh = 0;
	// Get INPUT DATA buffer
	if( ddSz &&
		hIn &&
		(lpMBuf = JGlobalLock( hIn )) )
	{
		ddMaxCnt = ddBufCnt = ddSz;
	}
	else
	{
		bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Input parameter to LIBRARY errant! " */
		goto wgx_rett;
	}
	if( !( hgI && (lpgh = (LPGIFHDREXT) JGlobalLock( hgI ) ) ) )
	{
		bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Input parameter to LIBRARY errant! " */
		goto wgx_rett;
	}
    
#ifndef WIN64
#ifdef	WIN32
//	SETEXIT( WGXRET );
	_asm 
	{
		mov  eax,offset WGXRETT ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump ;
		add  esp,2 * 4 ;
	}
#else	// !WIN32

	_asm
	{
		mov	ax,offset WGXRETT
		mov	dx,cs		; Return JUMP address ...
		mov	cx,sp		; and STACK frame ...
		push	cx
		push	dx
		push	ax
		call	setexitjump	; Save these items ...
		add	sp,6
	}
#endif	// WIN32 y/n
#endif // #ifndef WIN64

	bErrFlag = get_gif_sizeXT( &WCMcinfo, lpgh );

#ifndef WIN64
#ifdef	WIN32
	_asm
	{
WGXRETT:
		mov		eax, [bErrFlag]
	}
#else	// !WIN#@
	_asm
	{
WGXRETT:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}
#endif	// WIN32 y/n
#endif // #ifndef WIN64

wgx_rett:
	if( hIn && lpMBuf )
	{
		JGlobalUnlock( hIn );
		lpMBuf = 0;
	}
	if( hgI && lpgh )
		JGlobalUnlock( hgI );

	return( bErrFlag );

}	// WJPEG Library EXIT
// ###################



extern	void jselrjfif2( decompress_info_ptr );
extern	void jpeg_decompsize( decompress_info_ptr );

// WJPEG Library ENTRY
// ###################
EXPORT32
DWORD MLIBCALL
WJPGSIZE( HGLOBAL hIn, DWORD ddSz, LPSIZE lpSz )
{
	DWORD	jsize, ddw;
	int	i;
	jsize = 0;
	lpMBuf = 0;
	bErrFlag = 0;	/* No error yet ... */
	APIStart();
	winp_term();	/* and ensure any pending input cleared ... */
  /* Open the input file. */
	if( ddSz && hIn && (lpMBuf = JGlobalLock( hIn )) )
	{
		ddMaxCnt = ddBufCnt = ddSz;
	}
	else
		goto wj_ret;

#ifndef WIN64
#ifdef	WIN32
	_asm
	{
		mov		eax,offset WJRET
		mov		edx, esp
		push	edx
		push	eax
		call	setexitjump
		add	esp,2 * 4
	}

#else	// !WIN32

	_asm
	{
		mov	ax,offset WJRET
		mov	dx,cs		; Return JUMP address ...
		mov	cx,sp		; and STACK frame ...
		push	cx
		push	dx
		push	ax
		call	setexitjump	; Save these items ...
		add	sp,6
	}
#endif	// WIN32 y/n
#endif // #ifndef WIN64

  // Set up links to method structures.
	DSdinfo.methods = &DSdc_methods;
	DSdinfo.emethods = &DSe_methods;
	// NOTE: In original HEADERS these were ALL done in parse_switches!!!
	SetupJDef( &DSdinfo );
	jselrjfif2( &DSdinfo );
	jpeg_decompsize( &DSdinfo );
	// If all ok, set SIZE
	if( (bErrFlag == 0) &&
		DSdinfo.image_width &&
		DSdinfo.image_height )
	{
		if( lpSz )
		{
			lpSz->cx = (int) DSdinfo.image_width;
			lpSz->cy = (int) DSdinfo.image_height;
		}
		if( DSdinfo.image_width % 4 )
			ddw = ((DSdinfo.image_width / 4) + 1) * 4;
		else
			ddw = DSdinfo.image_width;
		i = DSdinfo.desired_number_of_colors;
		if( i )
		{
			if( i <= 2 )
				i = 2;
			else if( i <= 16 )
				i = 16;
			else if( i <= 256 )
				i = 256;
			else
				i = 0;
		}
		jsize = sizeof( BITMAPINFOHEADER ) +
			( i * sizeof( RGBQUAD ) ) +
			( ddw * DSdinfo.image_height );
	}

#ifndef WIN64
#ifdef	WIN32
	_asm
	{
WJRET:
		mov		eax, [bErrFlag]
	}
#else	// !WIN#@
	_asm
	{
WJRET:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}
#endif	// WIN32 y/n
#endif // #ifndef WIN64

wj_ret:
	if( hIn && lpMBuf )
	{
		JGlobalUnlock( hIn );
		lpMBuf = 0;
	}
	return( jsize );
}	// WJPEG Library EXIT
// ###################


/* eof wgmain.c */

