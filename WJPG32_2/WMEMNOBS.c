
/*
 * WJPG32_2.DLL Project
 *
 * MODULE: WMemNobs.c
 *
 * This is based on JMEMNOBS.C from the IJG 6a source.
 * The main changes are :-
 * 1. malloc and free replaced by GlobalAlloc/GlobalFree
 * 2. ALL memory is FAR allocations, thus get_small is the
 *		SAME as get_large
 * 3. An EXTERNAL allocation and free service can be used
 *		for memory diagnostics (through WMemDiag.h)
 *
 */

#include	<windows.h>
#include	"Win32.h"
//#include	"d:\work\t4\dv32\DvMemF.h"	// But a COPY made, so
#include	"WMemDiag.h"

/*
 * jmemnobs.c
 *
 * Copyright (C) 1992-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file provides a really simple implementation of the system-
 * dependent portion of the JPEG memory manager.  This implementation
 * assumes that no backing-store files are needed: all required space
 * can be obtained from malloc().
 * This is very portable in the sense that it'll compile on almost anything,
 * but you'd better have lots of main memory (or virtual memory) if you want
 * to process big images.
 * Note that the max_memory_to_use option is ignored by this implementation.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jmemsys.h"		/* import the system-dependent declarations */

//#ifndef HAVE_STDLIB_H		/* <stdlib.h> should declare malloc(),free() */
//extern void * malloc JPP((size_t size));
//extern void free JPP((void *ptr));
//#endif

extern	LPJPEG_EXTMM	lpExtMM;

/*
 * Memory allocation and freeing are controlled by the regular library
 * routines malloc() and free().
 */
#define	MXLIST	1024
void MLPTR	lpList[MXLIST];
DWORD	dwListCnt = 0;

void	Add2List( void MLPTR lp )
{
	if( dwListCnt < MXLIST )
		lpList[dwListCnt++] = lp;
}
void	RemFList( void MLPTR lp )
{
	DWORD	dw;
	for( dw = 0; dw < dwListCnt; dw++ )
	{
		if( lpList[dw] == lp )
		{
			lpList[dw] = 0;
			break;
		}
	}
}

// AND since we do keep a LIST,
// AND if the allocations are LESS than the LIST
// WE CAN VERIFY A POINTER
// =======================
BOOL	IsInList( void MLPTR lp )
{
	BOOL	flg = FALSE;
	DWORD	dw;
	for( dw = 0; dw < dwListCnt; dw++ )
	{
		if( lpList[dw] == lp )
		{
			flg = TRUE;
			break;
		}
	}
	if( !flg &&
		( dw >= MXLIST ) )
	{
		// We can't VERIFY if we went over the MAX. LIST count
		// BUT it could be valid, so ...
		flg = TRUE;
	}
	return flg;
}

//return (void MLPTR) malloc(sizeofobject);
GLOBAL(void MLPTR)
jpeg_get_small (j_common_ptr cinfo, size_t sizeofobject)
{
	void MLPTR lp = 0;
	if( lpExtMM &&
		( lpExtMM->EXTLOCALALLOC ) )
	{
		// Use DIAGNOSTIC FAR Allocation
		lp = (*lpExtMM->EXTLOCALALLOC) ( sizeofobject );
	}
	else
	{
		// Use system global FAR allocation
		lp = GlobalAlloc( GPTR, sizeofobject );
	}
	if( lp )	// If allocation
		Add2List( lp );	// Add to LIST
	else	// MEMORY FAILED - NEVER RETURN
		ERREXIT(cinfo, JERR_NO_BACKING_STORE);	// Might HELP!!!

	// and return POINTER to memory
	return lp;
}

//free(object);
GLOBAL(void)
jpeg_free_small (j_common_ptr cinfo, void MLPTR object,
				 size_t sizeofobject)
{
	if( lpExtMM &&
		( lpExtMM->EXTLOCALFREE ) )
	{
		(*lpExtMM->EXTLOCALFREE) ( object );
	}
	else
	{
		GlobalFree( (HGLOBAL)object );
	}
	if( object )
		RemFList( object );
}


/*
 * "Large" objects are treated the same as "small" ones.
 * NB: although we include FAR keywords in the routine declarations,
 * this file won't actually work in 80x86 small/medium model; at least,
 * you probably won't be able to process useful-size images in only 64KB.
 */

// HOWEVER, through the MACRO "MLPTR" the pointer can be *
// as in NEAR and 32-bits, but FAR far etc are NOT sufficient
// in Windows - the keyword huge must be used in the definition
// to ensure compiler does the correct 20-Bit addressing on
// buffer increments, etc.
// SINCE ALL DATA IS FAR REFERENCED, then it really does not
// matter which compiler option is used. It can be small or
// medium, but since this IS a Library, why not always use a
// Large MODEL?
//
// In this case ALL memory is truely treated equally. As can
// be seen, the get large call actually does a get small call!!!
//
// ==========================================================
GLOBAL(void MLPTR)
jpeg_get_large (j_common_ptr cinfo, size_t sizeofobject)
{
	//return (void MLPTR) malloc(sizeofobject);
	return( jpeg_get_small( cinfo, sizeofobject ) );
}

GLOBAL(void)
jpeg_free_large (j_common_ptr cinfo, void MLPTR object,
				 size_t sizeofobject)
{
	//free(object);
	jpeg_free_small( cinfo, object, sizeofobject );
}


/*
 * This routine computes the total memory space available for allocation.
 * Here we always say, "we got all you want bud!"
 */

GLOBAL(long)
jpeg_mem_available (j_common_ptr cinfo, long min_bytes_needed,
		    long max_bytes_needed, long already_allocated)
{
  return max_bytes_needed;
}


/*
 * Backing store (temporary file) management.
 * Since jpeg_mem_available always promised the moon,
 * this should never be called and we can just error out.
 */

GLOBAL(void)
jpeg_open_backing_store (j_common_ptr cinfo, backing_store_ptr info,
			 long total_bytes_needed)
{
  ERREXIT(cinfo, JERR_NO_BACKING_STORE);
}


/*
 * These routines take care of any system-dependent initialization and
 * cleanup required.  Here, there isn't any.
 */

GLOBAL(long)
jpeg_mem_init (j_common_ptr cinfo)
{
  return 0;			/* just set max_memory_to_use to 0 */
}

GLOBAL(void)
jpeg_mem_term (j_common_ptr cinfo)
{
  /* no work ?????????? Who said this? */
	// void MLPTR	lpList[MXLIST];
	DWORD	dw;
	void MLPTR lp;
	if( dwListCnt )
	{
		// This is a DOUBLE check of the memory
		// manager. The "destroy" call should have
		// released ALL memory held by the DLL
		for( dw = 0; dw < dwListCnt; dw++ )
		{
			if( lp = lpList[dw] )
			{
				jpeg_free_small ( cinfo, lp, 1 );
				lpList[dw] = 0;
			}
		}
	}
	dwListCnt = 0;

}

// eof - WmemNobs.c
