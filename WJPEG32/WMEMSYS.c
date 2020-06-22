
/*
 * wmemsys.c
 *
 * Copyright (C) 1992, Thomas G. Lane.
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

#include "winclude.h"
#include "wmemsys.h"
#undef	SERIOUSMS	/* ON to write massive memory DIAGNOSTICS */

//extern HGLOBAL DVGlobalAlloc(
//    UINT uFlags,	// object allocation attributes 
//    DWORD dwBytes); 	// number of bytes to allocate  
//extern LPSTR	DVLocalLock( HGLOBAL hl );
//extern void		DVLocalFree( HGLOBAL hl );
//extern void		DVGlobalFree( HGLOBAL hl );
//extern void		DVLocalUnlock( HGLOBAL hl );

#ifdef	SERIOUSMS
#pragma message( "WARNING: Memory Diagnostic File is ***ON*** ... " )
#endif

#ifndef	_INC_WINDOWS
#ifdef INCLUDES_ARE_ANSI
#include <stdlib.h>		/* to declare malloc(), free() */
#else
// Avoid using 16-bit holders of anything - allocate 32++
// ======================================================
extern void * malloc PP((size_t size));
extern void free PP((void *ptr));
#endif
#endif	/* !_INC_WINDOWS */

static external_methods_ptr methods; /* saved for access to error_exit */

DWORD	total_used;
DWORD	total_small;
DWORD	gtot_small;
DWORD	high_small;
/*
 * Memory allocation and freeing are controlled by the regular library
 * routines malloc() and free().
 */

#ifdef	_INC_WINDOWS

// ======================================
#define	MXSMLMEM	200		// Why more than this???
#define	MXFARMEM	200		// This could be many more
DWORD	NxtHand = 0;
// FIX16 - Always uses a reasonable sized HANDLE!!!
typedef struct tagLOCHANDS {
	DWORD	laHnd;		// Define 32-bit handle
	void MLPTR laMem;
#ifdef	SERIOUSMS
	size_t	laSiz;
#endif
} LOCHANDS;


DWORD	NxtFHand = 0;
typedef struct tagFARHANDS {
	HGLOBAL	faHnd;
	void MLPTR faMem;
} FARHANDS;

// And setup the list ...
// This should be sufficient ... for most things ...
LOCHANDS	HandList[MXSMLMEM];
FARHANDS	FHandList[MXFARMEM];

#endif	/* !_INC_WINDOWS */

#ifdef	SERIOUSMS
WORD	RunCnt = 0;
char	szFN[] = "TEMPMEM2.TXT";
void	Add2MFile( LPSTR lps )
{
UINT	i;
HFILE	hf;
LPSTR	lpf;
	if( lps && (i = lstrlen( lps )) )
	{
		lpf = &szFN[0];
		if( ( hf = _lopen( lpf, READ_WRITE ) ) &&
			( hf != HFILE_ERROR ) )
		{
			_llseek( hf, 0L, SEEK_END );
		}
		else
		{
			hf = _lcreat( lpf, 0 );
		}
		if( hf && (hf != HFILE_ERROR) )
		{
			_lwrite( hf, lps, i );
			_lclose( hf );
		}
	}
}
void	chk4000( void )
{
int i;
	i = 0;
}

void	Add2MList( size_t sz )
{
char	buf[80];
LPSTR	lps;
	if( sz >= 4000 )
		chk4000();
	lps = &buf[0];
	wsprintf( lps, "\r\nAllocating %5d bytes (Tot=%5lu) ", sz, total_small );
	Add2MFile( lps );
}

void	Add2HList( HLOCAL hl )
{
char	buf[80];
LPSTR	lps;
	lps = &buf[0];
	wsprintf( lps, "Handle %4x ", hl );
	Add2MFile( lps );
}

void	Add2PList( void MLPTR m )
{
char	buf[80];
LPSTR	lps;
	lps = &buf[0];
	wsprintf( lps, "Pointer %4X ", m );
	Add2MFile( lps );
}

void	Add2FList( HLOCAL hl )
{
char	buf[80];
LPSTR	lps;
	lps = &buf[0];
	wsprintf( lps, "\r\nFreed Handle %x (%lu) ", hl, total_small );
	Add2MFile( lps );
}

void	Add2FList2( HLOCAL hl )
{
char	buf[80];
LPSTR	lps;
	lps = &buf[0];
	wsprintf( lps, "\r\n*** Freed Handle %x (%lu) ", hl, total_small );
	Add2MFile( lps );
}
void	chk3( void )
{
int i;
	i = 0;
}
void	chk39( void )
{
int i;
	i = 0;
}
void	WriteBgn( void )
{
char	buf[80];
LPSTR	lps;
	lps = &buf[0];
	RunCnt++;
	if( RunCnt == 3 )
		chk3();
	if( RunCnt == 39 )
		chk39();
	wsprintf( lps, "\r\nBegin run %d ", RunCnt );
	Add2MFile( lps );
}

void	WriteNoMem( void )
{
char	buf[80];
LPSTR	lps;
	lps = &buf[0];
	wsprintf( lps, "\r\nERROR00: No MEMORY (LocalAlloc returned 0)! \r\n" );
	Add2MFile( lps );
}

void	WriteNoMem3( void )
{
char	buf[80];
LPSTR	lps;
	lps = &buf[0];
	wsprintf( lps, "\r\nERRORM03: No MEMORY (LocalLock returned 0)! \r\n" );
	Add2MFile( lps );
}

void	WriteNoMem2( void )
{
char	buf[80];
LPSTR	lps;
	lps = &buf[0];
	wsprintf( lps, "\r\nERRORM02: No internal MEMORY storage area! " );
	Add2MFile( lps );
}

void	WriteEnd( void )
{
char	buf[80];
LPSTR	lps;
	lps = &buf[0];
	wsprintf( lps, "\r\nHigh Small = %lu (%lu)\r\nGTot_Small = %lu\r\nEnd run %d\r\n",
		high_small, total_small, gtot_small, RunCnt );
	Add2MFile( lps );
}
void	MissedFree( void * ob )
{
char	buf[80];
LPSTR	lps;
	lps = &buf[0];
	wsprintf( lps, "\r\nERROR: Missed free! (%x)\r\n", ob );
	Add2MFile( lps );
}
#endif	/* SERIOUSMS */

#ifndef	ALLFARMEM
//#ifdef	WIN32
// Use LocalAlloc, instead of GlobalAlloc
// ======================================
// Using GlobalAlloc for return of ES:BX pointer.

GLOBAL void MLPTR
jget_small (size_t sizeofobject)
{
	void MLPTR mem = 0;
	//HLOCAL	hl;
	DWORD	hl;
	HGLOBAL	ghMem;
	LPSTR	lpm;

	lpm = 0;
  /* near data space is NOT counted in total_used */
#ifndef NEED_FAR_POINTERS
	total_used += sizeofobject;
#endif

#ifdef	_INC_WINDOWS
	// =======================================================
	gtot_small += sizeofobject;
	total_small += sizeofobject;	/* accumulate a GRAND total ALL small */

#ifdef	SERIOUSMS
	HandList[NxtHand].laSiz = sizeofobject;
	Add2MList( HandList[NxtHand].laSiz );
#endif	/* SERIOUSMS */
//	HandList[NxtHand].laHnd = LocalAlloc( LHND, sizeofobject );
	// ==================== ????????
	// As per GlobalAlloc
	ghMem = DVGlobalAlloc( GHND, sizeofobject );
	if( !ghMem )
		ERRFLAG( BAD_ZEROMEMH );	// ERROR: No MEMORY Handles!!!
	// This is an immediate libary exit	cause

//	HandList[NxtHand].laHnd = LocalAlloc( LPTR, sizeofobject );
	HandList[NxtHand].laHnd = (DWORD)ghMem;

	if( hl = (DWORD) HandList[NxtHand].laHnd )
	{
#ifdef	SERIOUSMS
		Add2HList( hl );
#endif	/* SERIOUSMS */
		// Always USE/STORE a 32-bit handle, regardless of what is
		// being used as handles ...
		// and ALWAYS return 32-bit for locking address ...
		lpm = (LPSTR)DVLocalLock( ghMem );
		HandList[NxtHand].laMem = lpm;
		mem = HandList[NxtHand].laMem;
		if( mem )
		{
			if( NxtHand + 1 < MXSMLMEM )
			{
				NxtHand++;
#ifdef	SERIOUSMS
				Add2PList( mem );
#endif	/* SERIOUSMS */
			}
			else 
			{
				gtot_small -= sizeofobject;
				total_small -= sizeofobject;	/* remove this request. */
				DVLocalFree( (HGLOBAL)HandList[NxtHand].laHnd );
				HandList[NxtHand].laHnd = 0;
#ifdef	SERIOUSMS
				WriteNoMem2();
#endif	/* SERIOUSMS */
				ERRFLAG( BAD_MEMCNTS );	/* ERROR: Increase small memory counter! */
			}
		}
		else	// AWK! - Memory pointer is ZERO
		{
			gtot_small -= sizeofobject;
			total_small -= sizeofobject;	/* remove this request. */
			DVLocalFree( (HGLOBAL) HandList[NxtHand].laHnd );
			HandList[NxtHand].laHnd = 0;
#ifdef	SERIOUSMS
			WriteNoMem3();	/* Show FAILED to get POINTER ... */
#endif	/* SERIOUSMS */
			ERRFLAG( BAD_MEMHEAP1 );	/* ERROR: Increase HEAP size for near memory! */
		}
	}
	else
	{
		gtot_small -= sizeofobject;
		total_small -= sizeofobject;	/* remove this request. */
#ifdef	SERIOUSMS
		WriteNoMem();	/* Show FAILED to get HANDLE */
#endif	/* SERIOUSMS */
		ERRFLAG( BAD_MEMHEAP2 );	/* ERROR: Increase HEAP size for near memory! */
	}
	// =======================================================
#else	// !_INC_WINDOWS
	// ===================== Just using near heap + DS = SS ======
	// ===========================================================
#pragma message( "NEAR malloc, converted to FAR (20-bit) Pointer" )
// This means allocate near in 16-bit, and convert it by the
// DS becoming the UPPER Segment component.
// =======================================
	mem = (void MLPTR) malloc(sizeofobject);
	// =====================================
	// ===========================================================
#endif	// _INC_WINDOWS y/n

	if( total_small > high_small )
		high_small = total_small;
	return( mem );

}

GLOBAL void
jfree_small (void MLPTR object)
{
	WORD	i;
	BOOL	flg;
	DWORD	hl;
	flg = FALSE;
	if( object && NxtHand )
	{
		for( i = 0; i < NxtHand; i++ )
		{
			if( flg )
			{
				HandList[i - 1].laHnd = HandList[i].laHnd;
				HandList[i - 1].laMem = HandList[i].laMem;
				HandList[i].laHnd = 0;
				HandList[i].laMem = 0;
#ifdef	SERIOUSMS
				HandList[i - 1].laSiz = HandList[i].laSiz;
				HandList[i].laSiz = 0;
#endif
			}
			else
			{
				if( HandList[i].laMem == object )
				{
					if( hl = (DWORD)HandList[i].laHnd )
					{
						DVLocalUnlock( (HGLOBAL)hl );
						DVLocalFree( (HGLOBAL)hl );
#ifdef	SERIOUSMS
						total_small -= (DWORD) HandList[i].laSiz;	/* Reduce total */
						HandList[i].laSiz = 0;
						Add2FList( hl );
#endif
						flg = TRUE;
					}
					HandList[i].laHnd = 0;
					HandList[i].laMem = 0;
				}
			}
		}
		if( flg && NxtHand )
			NxtHand--;
	}
#ifdef	SERIOUSMS
	if( !flg )
	{
		MissedFree( object );
	}
#endif
///////  free(object);
}

//==================
//#endif	// WIN32

#endif	/* !ALLFARMEM */

/*
 * Far-memory allocation and freeing
 */

#ifdef NEED_FAR_POINTERS

GLOBAL void FAR *
jget_large (size_t sizeofobject)
{
void FAR *mem;
  total_used += sizeofobject;
#ifdef	_INC_WINDOWS
	FHandList[NxtFHand].faHnd = JGlobalAlloc( GHND, sizeofobject );
	if( FHandList[NxtFHand].faHnd )
	{
		FHandList[NxtFHand].faMem = JGlobalLock( FHandList[NxtFHand].faHnd );
		mem = FHandList[NxtFHand].faMem;
		if( mem )
		{
			if( NxtFHand + 1 < MXFARMEM )
			{
				NxtFHand++;
			}
			else 
			{
				JGlobalFree( FHandList[NxtFHand].faHnd );
				FHandList[NxtFHand].faHnd = 0;
				ERRFLAG( BAD_MEMCNTF );	/* ERROR: Increase far memory counter! */
			}
		}
		else
		{
			JGlobalFree( FHandList[NxtFHand].faHnd );
			FHandList[NxtFHand].faHnd = 0;
		}
	}
#else
  mem = (void FAR *) far_malloc(sizeofobject);
#endif
return( mem );
}

GLOBAL void
jfree_large (void FAR * object)
{
WORD	i;
BOOL	flg;
	flg = FALSE;
	if( object && NxtFHand )
	{
		for( i = 0; i < NxtFHand; i++ )
		{
			if( flg )
			{
				FHandList[i - 1].faHnd = FHandList[i].faHnd;
				FHandList[i - 1].faMem = FHandList[i].faMem;
				FHandList[i].faHnd = 0;
				FHandList[i].faMem = 0;
			}
			else
			{
				if( FHandList[i].faMem == object )
				{
					if( FHandList[i].faHnd )
					{
						JGlobalUnlock( FHandList[i].faHnd );
						JGlobalFree( FHandList[i].faHnd );
					}
					FHandList[i].faHnd = 0;
					FHandList[i].faMem = 0;
					flg = TRUE;
				}
			}
		}
		if( flg && NxtFHand )
			NxtFHand--;
	}
//  far_free(object);
}

#endif


/*
 * This routine computes the total memory space available for allocation.
 * Here we always say, "we got all you want bud!"
 */

GLOBAL long
jmem_available (long min_bytes_needed, long max_bytes_needed)
{
  return max_bytes_needed;
}


/*
 * Backing store (temporary file) management.
 * This should never be called and we just error out.
 */

GLOBAL void
jopen_backing_store (backing_store_ptr info, long total_bytes_needed)
{
#ifdef	_INC_WINDOWS
	ERRFLAG( BAD_BACKING );	/* "Backing store not supported" */
#else
  ERREXIT(methods, "Backing store not supported");
#endif
}

void
ClearStats( void )
{
	total_used = 0;
	total_small = 0;	/* clear total ... */
	gtot_small= 0;
	high_small = 0;
}

/*
 * These routines take care of any system-dependent initialization and
 * cleanup required.  Keep in mind that jmem_term may be called more than
 * once.
 */
extern	void ResetBMPSizes( void );

GLOBAL void
jmem_init (external_methods_ptr emethods)
{
  methods = emethods;		/* save struct addr for error exit access */
  emethods->max_memory_to_use = 0;
	ClearStats();
#ifdef	SERIOUSMS
	WriteBgn();
#endif
}

GLOBAL void
jmem_term (void)
{
#ifdef	_INC_WINDOWS
WORD	i;
DWORD	hl;
	if( NxtHand )	/* If there is still LOCAL allocated memory ... */
	{
		for( i = 0; i < NxtHand; i++ )
		{
			if( hl = HandList[i].laHnd )
			{
				if( HandList[i].laMem )
				{
					DVLocalUnlock( (HGLOBAL)hl );
				}
				DVLocalFree( (HGLOBAL)hl );
				HandList[i].laHnd = 0;
#ifdef	SERIOUSMS
				total_small -= HandList[i].laSiz;	/* Reduce total ... */
				HandList[i].laSiz = 0;
				Add2FList2( hl );
#endif
			}
			HandList[i].laMem = 0;
		}
	}
	if( NxtFHand )	/* If there is still GLOBAL allocated memory ... */
	{
		for( i = 0; i < NxtFHand; i++ )
		{
			if( FHandList[i].faHnd )
			{
				if( FHandList[i].faMem )
				{
					JGlobalUnlock( FHandList[i].faHnd );
				}
				DVGlobalFree( (HGLOBAL)FHandList[i].faHnd );
				FHandList[i].faHnd = 0;
			}
			FHandList[i].faMem = 0;
		}
	}
	NxtHand = 0;
	NxtFHand = 0;
#ifdef	SERIOUSMS
	WriteEnd();
#endif
#endif
	ClearStats();	/* and remove all the numbers ... */
	ResetBMPSizes();
}

/* eof - wmemsys.c */
