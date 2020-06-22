

// DVToJPG.c

#include "winclude.h"

#ifdef	_INC_WINDOWS
/* ******************************************* */

#include "wcommon.h"
//#ifdef BMP_SUPPORTED
#define	inpopen	in_open
#define	outopen	out_open
//#endif	/* BMP_SUPPORT */
#include "wversion.h"		/* for version message */
//#define	_DLLLIB
#include	"wjpeglib.h"	/* Module Services (as WINAPI headers) */
//#ifdef	exit
//#undef	exit
//#endif

#ifdef	DV32DLL
//#include	"..\Dv32Lib.h"	// Get PARENTS header ...
//#include	"SetExit.h"
//#ifndef	WJPEG4 - This function NOT released...
// #pragma message( "Adding RETIRED Service... WBMPTOJPG!" )
// No direct disk i/o from library. App. must pass a
// HGLOBAL to sufficient memory to hold the RESULTS.
// In WIN32 this can be done by memory mapping a file,
// and getting into some (virtual) memory, 32-bit. Then
// this output can be any HANDLE passed by the applications.
// THe Lib. does NOT check this memory, but uses DVGlobalAlloc,
// DV..memory..Free, set free, or even delete could be used.
// or any User Defined handle given suitable replacements,
// Set could be provided as DvMem.h alternative ...
// Thus to convert to MEMORY/file mapping, an application
// must expose such "memory" attributes to achieve file i/o.
// ==========================================================
#define READ_BINARY	"rb"
#define WRITE_BINARY	"wb"

#ifdef	NEED_TWO_FILE	// Only IF required
GLOBAL int
c_parse_switches( compress_info_ptr, int, int, char FAR * FAR * );
extern	char MLPTR c_outfilename;	/* for -outfile switch */
#endif	// NEED_TWO_FILE

GLOBAL void
c_ui_method_selection( compress_info_ptr );
GLOBAL void
set_file_size( DWORD );
GLOBAL void
select_file_type( compress_info_ptr );

extern	DWORD	ddMaxCnt;	// GLOBAL: Character INPUT counter
extern	LPVOID	lpMBuf;		// GLOBAL: Input buffer pointer
extern	DWORD	ddBufCnt;	// GLOBAL: Remaining INPUT counter
extern	struct External_methods_struct e_methods;
extern	struct Compress_info_struct WCMcinfo;
extern	struct Compress_methods_struct c_methods;
extern	LIBCONFIG	LibConfig;	// Global CONFIG pad (in wddeflts.c)
extern	BOOL	IsDIB;
extern	void jmem_term( void );
extern	FILETYPE gout_open( char MLPTR );
extern	int gout_close( FILETYPE );

#if		( _MSC_VER > 700 )
//#pragma message( "Using _MSC_VER Version > 700!" )
#if		(defined( NDEBUG ) && defined( Dv16_App ))
// **************************************************************
#pragma warning( disable : 4704 )
// **************************************************************
#endif	// NDEBUG & Dv16_App
#endif	// _MSC_VER > 700

//
// ============================================================
// Retired from LIBRARY - April, 1997
// This removes completely the FILE interface ...
// In FACT, direct file read/writes have been "Retired", from the
// Library. These have been fully replaced with a SIZE, followed
// by a CONVERT function.
// Use W???SIZX or W???SIXT, to get structures of GIF information
// first, then
// Use W???NBMP or W???NBMPX, to convert each image, by simple
// Numbering 1, 2, ..., n images.
// NOTE: BMP zero (0), from using W???NBMP (or X), on GIF, gives
// first a GIF Logical Screen image, FILLED with the
// background colour indicated as an Offset in a ???HDR/IMG
// structure(s). The first, 0, is a header on which n images
// should be logically placed.
// WJPEG Library ENTRY
// ###################
EXPORT32
WORD MLIBCALL 
WBMPTOJPG( HGLOBAL hDib, DWORD ddSiz, HGLOBAL hInf, LPSTR lpOFile )
{
	LPSTR	lpInf;

	APIStart();
	winp_term();	/* and all inputs cleared ... */
	bErrFlag = 0;	/* Global ERROR flag (if NON-ZERO) return ... */
	msgout = &szMsgBuf[0];	/* Setup a MESSAGE buffer ... */
	/* In a sense, OPEN the input file ... */
	lpInf = 0;
	if( hDib && ( ddMaxCnt = ddSiz ) &&
		( lpMBuf = JGlobalLock( hDib ) ) )
	{
		ddBufCnt = ddSiz;
		set_file_size( ddSiz );	/* Insert the FILE SIZE ... */
	}
	else
	{
		bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Errant Input parameter! " */
			goto bmpjpg_ret;
	}	
	/* Must have an output file name, and must be creatable ie valid handle */
	if( (lpOFile == 0) || (lpOFile[0] == 0) ||
		( (WCMcinfo.output_file = gout_open( lpOFile )) == 0 ) ||
		( WCMcinfo.output_file == HFILE_ERROR ) )
	{
		bErrFlag = BAD_OOPEN;
			goto bmpjpg_ret;
	}
#ifdef	WIN32
//	SETEXIT( BMPJPGRET );
#ifndef WIN64
	_asm 
	{
		mov  eax,offset BMPJPGRET ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump ;
		add  esp,2 * 4 ;
	}
#endif // 1WIN64    
#else	// !WIN32
	_asm
	{
		mov	ax,offset BMPJPGRET
		mov	dx,cs		; Return JUMP address ...
		mov	cx,sp		; and STACK frame ...
		push	cx
		push	dx
		push	ax
		call	setexitjump	; Save these items ...
		add	sp,6
	}
#endif	// WIN32 y/n
	/* Set up links to method structures. */
	WCMcinfo.methods = &c_methods;
	WCMcinfo.emethods = &e_methods;
	/* These were done in c_parse_switches ... */
	/* (Re-)initialize the system-dependent error and memory managers. */
	jselerror(WCMcinfo.emethods);	/* error/trace message routines */
	jselmemmgr(WCMcinfo.emethods);	/* memory allocation routines */
	WCMcinfo.methods->c_ui_method_selection = c_ui_method_selection;
	/* Set up default JPEG parameters. */
	j_c_defaults( &WCMcinfo, LibConfig.ccf_out_quality, FALSE );
  /* NO Figuring here - Assumed it is a DIB input ONLY  */
	IsDIB = TRUE;	/* And further assume it is a DIB only, NOT BMP File */
	WSELRBMP( &WCMcinfo );
	if( bErrFlag )
		goto bmpjpg_ret;

  /* Do it to it! */
  jpeg_compress(&WCMcinfo);

  /* All done. */
#ifdef	WIN32
#ifndef WIN64
	_asm
	{
BMPJPGRET:
		mov	eax,[bErrFlag]	;	Get the ERROR FLAG ...
	}
#endif //#ifndef WIN64
#else	// !WIN32

	_asm
	{
BMPJPGRET:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}

#endif	// WIN32 y/n

bmpjpg_ret:
	jmem_term();	/* Ensure ALL memory freed ... */
	winp_term();	/* and all inputs cleared ... */
	if( hDib && lpMBuf )
	{
		JGlobalUnlock( hDib );	/* Unlock the INPUT buffer ... */
		lpMBuf = 0;
	}
	if( hInf && ( lpInf = JGlobalLock( hInf ) ) )
	{
		lpInf[0] = 0;
		if( bErrFlag )
		{
			GetErrorMsg( lpInf, (WORD)bErrFlag );
		}
		GetWarnMsg( lpInf );
		JGlobalUnlock( hInf );	/* Unlock the INFORMATION buffer ... */
	}
	if( WCMcinfo.output_file &&
		( WCMcinfo.output_file != HFILE_ERROR ) )
	{
		gout_close( WCMcinfo.output_file );
		if( bErrFlag )
		{
			// We could DELETE if ERROR
			// ========================
		}
	}
	WCMcinfo.output_file = 0;
	return( bErrFlag );
}	// WJPEG Library EXIT
// ###################
//#endif	// !WJPEG4

//#ifndef	NO_TWO_FILE		// Abandon this TWO file command line i/f
#ifdef	NEED_TWO_FILE	// Only IF required

// WJPEG Library ENTRY
// ###################
EXPORT32
WORD MLIBCALL 
WDATTOJPG( HGLOBAL hDib, DWORD ddSiz, HGLOBAL hInf, LPSTR lpOFile )
{
	LPSTR	lpInf;

	APIStart();
	winp_term();	/* and all inputs cleared ... */
	bErrFlag = 0;	/* Global ERROR flag (if NON-ZERO) return ... */
	msgout = &szMsgBuf[0];	/* Setup a MESSAGE buffer ... */
	/* In a sense, OPEN the input file ... */
	lpInf = 0;
	if( hDib && ( ddMaxCnt = ddSiz ) &&
		( lpMBuf = JGlobalLock( hDib ) ) )
	{
		ddBufCnt = ddSiz;
		set_file_size( ddSiz );	/* Insert the FILE SIZE ... */
	}
	else
	{
		bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Errant Input parameter! " */
			goto datjpg_ret;
	}	
	/* Must have an output file name, and must be creatable ie valid handle */
	if( (lpOFile == 0) || (lpOFile[0] == 0) ||
		((WCMcinfo.output_file = outopen( lpOFile, WRITE_BINARY)) == 0 ) &&
		( WCMcinfo.output_file == HFILE_ERROR ) )
	{
		bErrFlag = BAD_OOPEN;
			goto datjpg_ret;
	}

#ifdef	WIN32
//	SETEXIT( DATJPGRET );
#ifndef WIN64
	_asm 
	{
		mov  eax,offset DATJPGRET ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump ;
		add  esp,2 * 4 ;
	}
#endif // !#ifndef WIN64
#else	// WIN32
	_asm
	{
		mov	ax,offset DATJPGRET
		mov	dx,cs		; Return JUMP address ...
		mov	cx,sp		; and STACK frame ...
		push	cx
		push	dx
		push	ax
		call	setexitjump	; Save these items ...
		add	sp,6
	}
#endif	// WIN32 y/n

	/* Set up links to method structures. */
	WCMcinfo.methods = &c_methods;
	WCMcinfo.emethods = &e_methods;
	/* These were done in c_parse_switches ... */
	/* (Re-)initialize the system-dependent error and memory managers. */
	jselerror(WCMcinfo.emethods);	/* error/trace message routines */
	jselmemmgr(WCMcinfo.emethods);	/* memory allocation routines */
	WCMcinfo.methods->c_ui_method_selection = c_ui_method_selection;
	/* Set up default JPEG parameters. */
	j_c_defaults( &WCMcinfo, LibConfig.ccf_out_quality, FALSE );
  /* Figure out the input file format, and set up to read it. */
	select_file_type( &WCMcinfo );	/* The INPUT can be one of several types */
	if( bErrFlag )
		goto datjpg_ret;

  /* Do it to it! */
  jpeg_compress(&WCMcinfo);

  /* All done. */
#ifdef	WIN32
#ifndef WIN64
	_asm
	{
DATJPGRET:
		mov	eax,[bErrFlag]	;	Get the ERROR FLAG ...
	}
#endif // #ifndef WIN64
#else	// WIN32
	_asm
	{
DATJPGRET:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}
#endif	// WIN32 y/n
datjpg_ret:
	jmem_term();	/* Ensure ALL memory freed ... */
	winp_term();	/* and all inputs cleared ... */
	if( hDib && lpMBuf )
	{
		JGlobalUnlock( hDib );	/* Unlock INPUT buffer */
		lpMBuf = 0;
	}
	if( hInf && ( lpInf = JGlobalLock( hInf ) ) )
	{
		lpInf[0] = 0;
		if( bErrFlag )
		{
			GetErrorMsg( lpInf, (WORD)bErrFlag );
		}
		GetWarnMsg( lpInf );
		JGlobalUnlock( hInf );	/* Unlock Information Buffer */
	}
return( bErrFlag );
}	// WJPEG Library EXIT
// ###################



/* The original Write JPEG from the INPUT File data ... */
// WJPEG Library ENTRY
// ###################
EXPORT32
int MLIBCALL
WWRITEJPEG( int argc, char FAR * FAR *argv )
{
	int file_index;
	LPSTR lpInf;

	APIStart();
	winp_term();	/* and all inputs cleared ... */
	bErrFlag = 0;	/* Global ERROR flag (if NON-ZERO) return ... */
	msgout = &szMsgBuf[0];	/* Setup a MESSAGE buffer ... */

#ifdef	WIN32
//	SETEXIT( BMAINRET );
#ifndef WIN64
	_asm 
	{
		mov  eax,offset BMAINRET ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump ;
		add  esp,2 * 4 ;
	}
#endif // #ifndef WIN64
#else	// WIN32

	_asm
	{
		mov	ax,offset BMAINRET
		mov	dx,cs		; Return JUMP address ...
		mov	cx,sp		; and STACK frame ...
		push	cx
		push	dx
		push	ax
		call	setexitjump	; Save these items ...
		add	sp,6
	}

#endif	// WIN32

  /* Set up links to method structures. */
  WCMcinfo.methods = &c_methods;
  WCMcinfo.emethods = &e_methods;

  /* Scan command line: set up compression parameters, find file names. */

  file_index = c_parse_switches(&WCMcinfo, 0, argc, argv);

  /* Open the input file. */
	if( file_index < argc )
	{
		if( ((WCMcinfo.input_file = inpopen(argv[file_index], READ_BINARY)) == 0) ||
			( WCMcinfo.input_file == HFILE_ERROR ) )
		{
			ERRFLAG( BAD_FOPEN );
			goto mainret;
		}
	}
	else
	{
		ERRFLAG( BAD_FILENAME );
		goto mainret;
  }

  /* Open the output file. */
	if( c_outfilename != NULL ) 
	{
		if( ( (WCMcinfo.output_file = outopen(c_outfilename, WRITE_BINARY)) == 0 ) ||
			( WCMcinfo.output_file == HFILE_ERROR ) )
		{
			ERRFLAG( BAD_OOPEN );
			goto mainret;
    }
  } 
  else
  {
		ERRFLAG( BAD_FILENAME );
		goto mainret;
  }

  /* Figure out the input file format, and set up to read it. */
  select_file_type(&WCMcinfo);
	if( bErrFlag )
		goto mainret;
#ifdef PROGRESS_REPORT
  /* Start up progress display, unless trace output is on */
  if (e_methods.trace_level == 0)
    c_methods.progress_monitor = Cprogress_monitor;
#endif

  /* Do it to it! */
  jpeg_compress(&WCMcinfo);

#ifdef PROGRESS_REPORT
  /* Clear away progress display */
  if (e_methods.trace_level == 0) {
    fprintf(msgout, "\r                \r");
    fpaint(msgout);
  }
#endif

  /* All done. */
#ifdef	WIN32
#ifndef WIN64
	_asm
	{
BMAINRET:
		mov	eax,[bErrFlag]	;	Get the ERROR FLAG ...
	}
#endif // #ifndef WIN64
   
#else	// !WIN32
	_asm
	{
BMAINRET:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}
#endif	// WIN32 y/n
mainret:
	jmem_term();	/* Ensure ALL memory freed ... */
	winp_term();	/* and all inputs cleared ... */
	if( lpInf = argv[0] )	/* In WINDOWS, this is an INFORMATION BUFFER */
	{	/* But in can be NULL ... */
		lpInf[0] = 0;
		if( bErrFlag )
		{
			GetErrorMsg( lpInf, (WORD)bErrFlag );
		}
		GetWarnMsg( lpInf );
	}
	return( bErrFlag );
}	// WJPEG Library EXIT
// ###################

#endif	// NEED_TWO_FILE

#endif	// DV32DLL

/* ******************************************* */
#endif	/* !_INC_WINDOWS */
/* ======================================= */

// EOF - DVToJPG.c
