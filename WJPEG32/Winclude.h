
#ifndef  _WInclude_H
#define  _WInclude_H
//
// Personalised: 31Dec95 Geoff R. McLane - To inc. CODE in a Windows App.
// winclude.h
//
// Copyright (C) 1991, 1992, 1993, Thomas G. Lane.
// This file is part of the Independent JPEG Group's software.
// For conditions of distribution and use, see the accompanying README file.
//
// This is the central file that's #include'd by all the JPEG .c files.
// Its purpose is to provide a single place to fix any problems with
// including the wrong system include files.
// You can edit these declarations if you use a system with nonstandard
// system include files.
//

#define	USELOCBUF		/* INPUT BUFFER in DATA Seg. rather than HEAP */
#define	USEFARMEM		/* Switch some/most/all 'small/local' to global */
#define	DV32DLL			/* Special (private) LIBRARY for DV32 use. */
// Produces DV32JG.DLL and DV32JG.LIB with SUBSET of functions ...

#define	HAVE_STDC
#define	INCLUDES_ARE_ANSI
#define	ANSIINCS
/* #define	MSDOS */
#define  USE_FMEM
#define  NEED_FHEAPMIN
#define  SHORTxLCONST_32
#undef   PROGRESS_REPORT
#define	ADD_BMP			/* Add MS/IBM BMP support - Need W3.1 SDK */
#undef	ALLFARMEM		/* Switch to ALL FAR Memory pointers ... */
#define	MEM_STATS		/* Always KEEP memory statistics ... */

/*
 * Normally the __STDC__ macro can be taken as indicating that the system
 * include files conform to the ANSI C standard.  However, if you are running
 * GCC on a machine with non-ANSI system include files, that is not the case.
 * In that case change the following, or add -DNONANSI_INCLUDES to your CFLAGS.
 */

#ifdef __STDC__
#ifndef NONANSI_INCLUDES
#define INCLUDES_ARE_ANSI	/* this is what's tested before including */
#endif
#endif

#include	<windows.h>		// For MS Windows ...

#ifdef	Dv16_App			// Seems size_t missing from 16-bit windows.h???
#include	<string.h>		// For size_t
#endif	// Dv16_App

#define	USE_GETC_INPUT

// #define	COMBWLIB	// LINKED with LIBRARY CODE
// NOTE: This is ONLY supplied as a Preprocessor FLAG
// ==================================================
#ifdef	COMBWLIB

// Externalise these to check for leaks
extern	HGLOBAL	DVGlobalAlloc( UINT, DWORD );
extern	LPSTR	DVGlobalLock( HGLOBAL );
extern	void	DVGlobalUnlock( HGLOBAL );
extern	void	DVGlobalFree( HGLOBAL );

#define	JGlobalAlloc( a, b )	DVGlobalAlloc( a, b )
#define	JGlobalLock( a )		DVGlobalLock( a )
#define	JGlobalUnlock( a )		DVGlobalUnlock( a )
#define	JGlobalFree( a )		DVGlobalFree( a )

// FIX16 - 23
// And anything even named local can be 32-bit handle type
// ========================================================
#define	DVLocalFree( a )		DVGlobalFree( a )
#define	DVLocalLock( a )		DVGlobalLock( a )
#define	DVLocalUnlock( a )		DVGlobalUnlock( a )

// #define	COMBWLIB	// LINKED with LIBRARY CODE
// NOTE: This is ONLY supplied as a Preprocessor FLAG
// ==================================================
#else	// !COMBWLIB


// No problems - Go directly to the system call
#define	JGlobalAlloc( a, b )	GlobalAlloc( a, b )
#define	JGlobalLock( a )		GlobalLock( a )
#define	JGlobalUnlock( a )		GlobalUnlock( a )
#define	JGlobalFree( a )		GlobalFree( a )

// In many FORMS of the name(s)
#define	DVGlobalAlloc( a, b )		GlobalAlloc( a, b )
#define	DVGlobalFree( a )		GlobalFree( a )

// FIX16 - 23
// And anything even named local can be 32-bit handle type
// ========================================================
#define	DVLocalFree( a )		GlobalFree( a )
#define	DVLocalLock( a )		GlobalLock( a )
#define	DVLocalUnlock( a )		GlobalUnlock( a )

// #define	COMBWLIB	// LINKED with LIBRARY CODE
// NOTE: This is ONLY supplied as a Preprocessor FLAG
// ==================================================
#endif	// COMBWLIB y/n


/*
 * <stdio.h> is included to get the FILE typedef and NULL macro.
 * Note that the core portable-JPEG files do not actually do any I/O
 * using the stdio library; only the user interface, error handler,
 * and file reading/writing modules invoke any stdio functions.
 * (Well, we did cheat a bit in jmemmgr.c, but only if MEM_STATS is defined.)
 */
#ifndef	_INC_WINDOWS
#pragma message( "WARNING: This code has NOT been checked if no _INC_WINDOWS" )

#include <stdio.h>
#ifdef	MSDOS
#define	msgout	stdout
#else
#define	msgout	stderr
#endif
.error
//================ This is RETIRED ================
#endif	/* !_INC_WINDOWS */

/*
 * We need the size_t typedef, which defines the parameter type of malloc().
 * In an ANSI-conforming implementation this is provided by <stdio.h>,
 * but on non-ANSI systems it's more likely to be in <sys/types.h>.
 * On some not-quite-ANSI systems you may find it in <stddef.h>.
 */

#ifndef INCLUDES_ARE_ANSI	/* shouldn't need this if ANSI C */
#include <sys/types.h>
#endif
#ifdef __SASC			/* Amiga SAS C provides it in stddef.h. */
#include <stddef.h>
#endif

/*
 * In ANSI C, and indeed any rational implementation, size_t is also the
 * type returned by sizeof().  However, it seems there are some irrational
 * implementations out there, in which sizeof() returns an int even though
 * size_t is defined as long or unsigned long.  To ensure consistent results
 * we always use this SIZEOF() macro in place of using sizeof() directly.
 */

#undef SIZEOF			/* in case you included X11/xmd.h */
#define SIZEOF(object)	((size_t) sizeof(object))

/*
 * fread() and fwrite() are always invoked through these macros.
 * On some systems you may need to twiddle the argument casts.
 * CAUTION: argument order is different from underlying functions!
 */
#ifdef	_INC_WINDOWS

#ifndef	WJPEG4
#define	WJPEG4
#endif	// !WJPEG4

extern	UINT	bmpread( HFILE, void _huge *, UINT );
extern	void	APIStart( void );

#define JFREAD(file,buf,sizeofbuf)  \
  ((size_t) bmpread(file, (void _huge*) (buf), sizeofbuf))

#ifdef	WJPEG4
// NOTE: The FILE interface has been RETIRED - April, 1997
extern	size_t	chkjfwrite( void );
#define JFWRITE( file, buf, sizeofbuf )  chkjfwrite()

#else	// !WJPEG4

#define JFWRITE( file, buf, sizeofbuf )  \
	( (size_t) _lwrite( file, (void _huge*) (buf), sizeofbuf) )
#endif	// WJPEG4 y/n

#else	/* !_INC_WINDOWS */

#define JFREAD(file,buf,sizeofbuf)  \
  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))

#define JFWRITE(file,buf,sizeofbuf)  \
  ((size_t) fwrite((const void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))

#endif	/* _INC_WINDOWS */

/*
 * We need the memcpy() and strcmp() functions, plus memory zeroing.
 * ANSI and System V implementations declare these in <string.h>.
 * BSD doesn't have the mem() functions, but it does have bcopy()/bzero().
 * Some systems may declare memset and memcpy in <memory.h>.
 *
 * NOTE: we assume the size parameters to these functions are of type size_t.
 * Change the casts in these macros if not!
 */
#if	(defined( ANSIINCS ) || defined( INCLUDES_ARE_ANSI ))
//#ifdef INCLUDES_ARE_ANSI

#include <string.h>

#ifdef	_INC_WINDOWS

#ifdef	WIN32
// In 32-bit environment.
#define	gw_fmemcpy( a, b, c )	memmove( a, b, c )		
#define	gw_fstricmp( a, b )		_stricmp( a, b )
#define	_fmemset			memset
#define	_fmemcpy			memcpy

#else	// !WIN32

// Yeek, still in 16-bit environment
#define	gw_fstricmp( a, b )		_fstricmp( a, b )		
#define	gw_fmemcpy( a, b, c )	_fmemmove( a, b, c )		
#endif	// WIN32 y/n

// Common for Windows ...
#define MEMZERO(target,size)	_fmemset((void MLPTR )(target), 0, (size_t)(size))
#define MEMCOPY(dest,src,size)	_fmemcpy((void MLPTR)(dest), (const void MLPTR)(src), (size_t)(size))

#else	// _!INC_WINDOWS

//========================================================
#pragma message( "NOTE: NON-Windows code NOT checked!" )
#define MEMZERO(target,size)	memset((void *)(target), 0, (size_t)(size))
#define MEMCOPY(dest,src,size)	memcpy((void *)(dest), (const void *)(src), (size_t)(size))
//========================================================
#endif

#else /* not ANSI */

//========================================================
#pragma message( "NOTE: NON-ANSI code NOT checked!" )
//========================================================
#ifdef BSD
#include <strings.h>
#define MEMZERO(target,size)	bzero((void *)(target), (size_t)(size))
#define MEMCOPY(dest,src,size)	bcopy((const void *)(src), (void *)(dest), (size_t)(size))
#else /* not BSD, assume Sys V or compatible */
#include <string.h>
#define MEMZERO(target,size)	memset((void *)(target), 0, (size_t)(size))
#define MEMCOPY(dest,src,size)	memcpy((void *)(dest), (const void *)(src), (size_t)(size))
#else	/* !BSD */
#pragma message( "ERROR: UNKNOWN target OS! No MEMZERO or MEMCOPY defined! " )
#endif /* BSD */

#endif /* ANSI */


/* Now include the portable JPEG definition files. */

#include "wconfig.h"

//#include "wjpegdat.h"
#ifdef	WIN32
typedef int				BOOL;
#endif	// WIN32


#ifdef	_INC_WINDOWS
/* Added for WINDOWS ... */
#include	"wjpeglib.h"	/* Module Services (as WINAPI headers) */
#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD) (lpbi)) == sizeof (BITMAPINFOHEADER))
#define	EOF		(-1)
#define	fprintf	wsprintf
extern	char	szMsgBuf[260];
extern	LPSTR	msgout;
extern	int getc( HFILE );
extern	int retgetc( int, HFILE );
#ifndef	WIN32
extern	void exit( int );
extern	BOOL isupper( int );
extern	BOOL isspace( int );
extern	BOOL isdigit( int );
#endif	// !WIN32

// Main WJPEG structures
#include "wjpegdat.h"

//extern	int tolower( int );
extern	HFILE fclose( HFILE );
extern	void flushmsg( LPSTR );
extern	int sscanf( LPSTR, LPSTR, ... );
extern	int FAR PASCAL WSELRBMP( compress_info_ptr );
extern	int b_putc( int, HFILE );
extern	int b_fflush( HFILE );
extern	BOOL bErrFlag;
extern	int SetError( int );
extern	int SetWarning( int );
#define	ERRFLAG( i )	SetError( i )
#define	WARNFLAG( i )	SetWarning( i )
#ifndef	stdout
#define	stdout	1
#endif	/* stdout */
#ifndef	stdin
#define	stdin	2
#endif	/* stdin */

#ifdef	WIN32

typedef	HFILE	FILETYPE;
#else	// !WIN32
#define	FILETYPE	HFILE
#endif	// WIN32 y/n

#define	fpaint( x )	flushmsg( x )
#define	NO_GETENV	/* No getting anything from environment ... */
#ifdef	putc
#undef	putc	/* Remove 'C' putc ... */
#endif
#else		/* !_INC_WINDOWS */
#define	fpaint	fflush
#define	FILETYPE		(FILE *)
#endif	/* _INC_WINDOWS */

#endif   // #ifndef  _WInclude_H
/* eof - WInclude.h */
