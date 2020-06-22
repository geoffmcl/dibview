
/*
 * MODULE: WJpg2Bmp.c
 *
 * COMMENCED: 13 June, 1997
 *
 * Began as an EXACT copy of djpeg.c, with the main() removed,
 * and the whole command line interface removed, and conversion
 * is replaced by two (2) far pascal headers -
 * WJPGSIZE6 - Return the BMP size
 * WJPG2BMP6 - Convert data to BITMAP
 * BOTH are all HANDLES (to GLOBAL shareable memory for data in/out)
 *
 * See WJpg2Bmp.h for details of the CALLS
 * 
 */

#define	WJPEG6		// In DV32 (DibView) this adds these functions

// First - The notices in djpeg.c ...

/*
 * djpeg.c
 *
 * Copyright (C) 1991-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains a command-line user interface for the JPEG decompressor.
 * It should work on any system with Unix- or MS-DOS-style command lines.
 *
 * Two different command line styles are permitted, depending on the
 * compile-time switch TWO_FILE_COMMANDLINE:
 *	djpeg [options]  inputfile outputfile
 *	djpeg [options]  [inputfile]
 * In the second style, output is always to standard output, which you'd
 * normally redirect to a file or pipe to some other program.  Input is
 * either from a named file or from standard input (typically redirected).
 * The second style is convenient on Unix but is unhelpful on systems that
 * don't support pipes.  Also, you MUST use the first style if your system
 * doesn't do binary I/O to stdin/stdout.
 * To simplify script writing, the "-outfile" switch is provided.  The syntax
 *	djpeg [options]  -outfile outputfile  inputfile
 * works regardless of which command line style is used.
 */
#include	<windows.h>
#include	"Win32.h"	// OK, some pointer defs, like MLPTR

#include "cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */
#include "jversion.h"		/* for version message */
// NOTE: The above list EXPANDS TO the follwoing SET of INCLUDES
// CDJPEG.H		-	JINCLUDE.H
//				-	JCONFIG.H
//				-	JPEGLIB.H
//				-	JMORECFG.H
//				-	JPGSTRU6.H
//				-	JERROR.H
//				-	CDERROR.H
// JVERSION.H

// NOTE2: The JPEG Library also adds
// JPEGINT.H

// Source Files
//	jpeg_destroy_decompress		-	JDAPIMIN.c
//	jpeg_read_scanlines			-	JDAPISTD.c
//	jpeg_destroy				-	JCOMAPI.c
//	jpeg_handle_src				-	WDataSrc.c
//	jinit_write_bmp				-	WWrBmp6.c
//	jpeg_std_error				-	JError.c

//	jinit_mem_mgr		-	WMemMgr6.c (needs jmemsys.h)
//	jpeg_get_small		-	WMemNobs.c
//	jpeg_free_small		-	WMemNobs.c
//	jpeg_get_large		-	WMemNobs.c
//	jpeg_free_large		-	WMemNobs.c
//	jpeg_mem_available	-	WMemNobs.c
//	jpeg_open_backing_store -	WMemNobs.c
//	jpeg_mem_init		-	WMemNobs.c
//	jpeg_mem_term		-	WMemNobs.c

//	jpeg_resync_to_restart		-	JDMarker.c
//	jpeg_natural_order[]		-	JUtils.c
//	jpeg_calc_output_dimensions	-	JDMaster.c

//	jinit_input_controller		-	JDInput.c
//	jinit_d_coef_controller		-	JDCoefct.c
//	jinit_color_deconverter		-	JDColor.c
//	jinit_inverse_dct			-	JDDctmgr.c (needs jdct.h)

//	jinit_huff_decoder			-	JDHuff.c (needs jdhuff.h)
//	jpeg_make_d_derived_tbl		-	JDHuff.c
//	jpeg_fill_bit_buffer		-	JDHuff.c
//	jpeg_huff_decode			-	JDHuff.c
//	jinit_phuff_decoder			-	JDPHhuff.c
//	jinit_d_main_controller		-	JDMainct.c
//	jinit_merged_upsampler		-	JDMerge.c
//	jinit_d_post_controller		-	JDPostct.c
//	jinit_upsampler				-	JDSample.c
//	jpeg_read_coefficients		-	JDTrans.c

//	jpeg_fdct_islow				-	JFDctint.c
//	jpeg_fdct_ifast				-	JFDctfst.c
//	jpeg_fdct_float				-	JFDctflt.c

//	jpeg_idct_islow				-	JIDctint.c
//	jpeg_idct_ifast				-	JIDctfst.c
//	jpeg_idct_float				-	JIDctflt.c

//	jpeg_idct_4x4				-	JIDctred.c
//	jpeg_idct_2x2				-	JIDctred.c
//	jpeg_idct_1x1				-	JIDctred.c

//	jinit_1pass_quantizer		-	JQuant1.c
//	jinit_2pass_quantizer		-	JQuant2.c

#include	"WWrBmp6.h"	// Get MANAGER structure

//#include <ctype.h>		/* to declare isprint() */
//
//#ifdef USE_CCOMMAND		/* command-line reader for Macintosh */
//#ifdef __MWERKS__
//#include <SIOUX.h>              /* Metrowerks needs this */
//#include <console.h>		/* ... and this */
//#endif
//#ifdef THINK_C
//#include <console.h>		/* Think declares it here */
//#endif
//#endif

// AFTER ALL THE INCLUDES we can TURN OFF SOME SUPPORT
// and ENSURE certain support is *** ON ***
#ifndef BMP_SUPPORTED
#define	BMP_SUPPORTED
#endif	// !BMP_SUPPORT

#undef GIF_SUPPORTED
#undef PPM_SUPPORTED
#undef RLE_SUPPORTED
#undef TARGA_SUPPORTED

/* Create the add-on message string table. */
#ifdef	ADDERRDATA	// Only define ONCE in WComCfg.c
extern	const char MLPTR cdjpeg_message_table[];
//#define JMESSAGE(code,string)	string ,
//static const char MLPTR cdjpeg_message_table[] = {
//#include "cderror.h"
//  NULL
//};
#endif	// ADDERRDATA

// For EXTERNAL MEMORY Diagnostics
//#include	"D:\Work\t4\Dv32\DvMemF.h"	// But a local COPY made
#include	"WMemDiag.h"

/*
 * This list defines the known output image formats
 * (not all of which need be supported by a given version).
 * You can change the default output format by defining DEFAULT_FMT;
 * indeed, you had better do so if you undefine PPM_SUPPORTED.
 */

typedef enum {
	FMT_BMP,		/* BMP format (Windows flavor) */
	FMT_GIF,		/* GIF format */
	FMT_OS2,		/* BMP format (OS/2 flavor) */
	FMT_PPM,		/* PPM/PGM (PBMPLUS formats) */
	FMT_RLE,		/* RLE format */
	FMT_TARGA,		/* Targa format */
	FMT_TIFF		/* TIFF format */
} IMAGE_FORMATS;

#ifndef DEFAULT_FMT		/* so can override from CFLAGS in Makefile */
#define DEFAULT_FMT	FMT_BMP
#endif

// Library declarations
EXTERN(int)	CopyConfig6 JPP(( j_decompress_ptr cinfo ));

static IMAGE_FORMATS requested_fmt;

LPSTR	lpCommBuf = 0;
DWORD	dwMaxComm = 1024;
DWORD	dwInComm = 0;

#ifndef	WJPEG6
// Completely REMOVE the Command Line interface.
// =============================================

// Later there can be a CONFIG, like in WJPEG32.DLL
// ===========================
  /* Set up default JPEG parameters. */
  cinfo->err->trace_level = 0;
  requested_fmt = FMT_BMP;
  cinfo->desired_number_of_colors = val;
  cinfo->quantize_colors = TRUE;

	cinfo->dct_method = JDCT_ISLOW;
	cinfo->dct_method = JDCT_IFAST;
	cinfo->dct_method = JDCT_FLOAT;
	cinfo->dither_mode = JDITHER_FS;
	cinfo->dither_mode = JDITHER_NONE;
	cinfo->dither_mode = JDITHER_ORDERED;
	cinfo->err->trace_level++;

    // "fast"
	// Select recommended processing options for quick-and-dirty output.
	cinfo->two_pass_quantize = FALSE;
	cinfo->dither_mode = JDITHER_ORDERED;
	if (! cinfo->quantize_colors) /* don't override an earlier -colors */
		cinfo->desired_number_of_colors = 216;
	cinfo->dct_method = JDCT_FASTEST;
	cinfo->do_fancy_upsampling = FALSE;
    /* Force monochrome output. */
    cinfo->out_color_space = JCS_GRAYSCALE;
	// "nosmooth"
	/* Suppress fancy upsampling */
	cinfo->do_fancy_upsampling = FALSE;
	// "onepass"
	/* Use fast one-pass quantization. */
	cinfo->two_pass_quantize = FALSE;
	// "os2"
	/* BMP output format (OS/2 flavor). */
	requested_fmt = FMT_OS2;

// ==== Above removes the COMMAND LINE completely ====
// ===================================================
#endif	// !WJPEG6

/*
 * Marker processor for COM markers.
 * This replaces the library's built-in processor, which just skips the marker.
 * We want to print out the marker as text, if possible.
 * Note this code relies on a non-suspending data source.
 */

/* Read next byte */
LOCAL(unsigned int)
jpeg_getc (j_decompress_ptr cinfo)
{
	struct jpeg_source_mgr MLPTR datasrc = cinfo->src;

	if (datasrc->bytes_in_buffer == 0)
	{
		if( !(*datasrc->fill_input_buffer) (cinfo) )
			ERREXIT(cinfo, JERR_CANT_SUSPEND);
	}
	datasrc->bytes_in_buffer--;
	return GETJOCTET(*datasrc->next_input_byte++);
}

METHODDEF(boolean)
COM_handler (j_decompress_ptr cinfo)
{
	boolean traceit = (cinfo->err->trace_level >= 1);
	INT32 length;
	unsigned int ch;
	unsigned int lastch = 0;

	length = jpeg_getc(cinfo) << 8;
	length += jpeg_getc(cinfo);
	length -= 2;			/* discount the length word itself */

	if( traceit && lpCommBuf && ((dwInComm + length) < dwMaxComm) )
	{
		wsprintf( lpCommBuf, "Comment, length %ld:\n", (long) length);
		dwInComm = lstrlen( lpCommBuf );
	}

	while (--length >= 0)
	{
		ch = jpeg_getc(cinfo);
		if( traceit && lpCommBuf && ((dwInComm + 1 ) < dwMaxComm) )
		{
			/* Emit the character in a readable form.
			 * Nonprintables are converted to \nnn form,
			 * while \ is converted to \\.
			 * Newlines in CR, CR/LF, or LF form will be printed as one newline.
			 */
			if( ch == '\r' )
			{
				lstrcat( lpCommBuf, "\n" );
				dwInComm++;
			}
			else if( ch == '\n' )
			{
				if( lastch != '\r' )
				{
					lstrcat( lpCommBuf, "\n" );
					dwInComm++;
				}
			}
			else if( ch == '\\' )
			{
				lstrcat( lpCommBuf, "\\\\" );
				dwInComm++;
			}
			else if( (ch == 9) ||
				( ( ch >= ' ' ) && ( ch < 0x7f ) ) )
			{
				wsprintf( (lpCommBuf + lstrlen( lpCommBuf )),
					"%c",
					ch );
				dwInComm++;
			}
			else
			{
				lstrcat( lpCommBuf, "." );
				dwInComm++;
			}
			lastch = ch;
		}
	}

	if( traceit && lpCommBuf && ((dwInComm + 1 ) < dwMaxComm) )
	{
		lstrcat( lpCommBuf, "\r\n" );
		dwInComm = lstrlen( lpCommBuf );
	}
	return TRUE;
}

// =========================================================
// eof - DJpeg.c

// Beginning of WJpg2Bmp.c
// =======================

// 2 NEW Functions for JPEG decompression
// ===============
int		bErrFlag2 = 0;
void    MLPTR RetAddr6 = 0;
void    MLPTR mra6;
#ifdef  WIN32
DWORD   RetBP6 = 0;
DWORD   RetESP6 = 0;
DWORD   mBp6;
DWORD   mSp6;
#else
WORD    RetBP6 = 0;
WORD    RetSP6 = 0;
WORD    mBp6;
WORD    mSp6;
#endif  // WIN32 y.n

#if		( _MSC_VER > 700 )
#if		(defined( NDEBUG ) && defined( Dv16_App ))
// **************************************************************
#pragma warning( disable : 4704 )
// **************************************************************
#endif	// NDEBUG & Dv16_App
#endif	// _MSC_VER > 700

void	KillError2( void )
{
	RetAddr6 = 0;
	RetBP6 = 0;
	RetESP6 = 0;
}

int	SetError2( int i )
{
int	j;
	if( bErrFlag2 )
	{
		if( bErrFlag2 != i )
		{
			j = 0;
			//if( ErrCount )
			//{
			//	for( j = 0; j < ErrCount; j++ )
			//	{
			//		if( ErrList[j] == i )
			//			break;
			//	}
			//}
			//if( (j == ErrCount) && ((ErrCount + 1) < MXERRS) )
			//{
			//	ErrList[ErrCount] = i;
			//	ErrCount++;
			//}
		}	/* Not the first error ... */
	}
	else
	{
		bErrFlag2 = i;
	}
#ifdef	WIN32
	if( (mra6 = RetAddr6) && 
		(mBp6 = RetBP6) && 
		(mSp6 = RetESP6) )
	{
		RetAddr6 = 0;
		RetBP6 = 0;
		RetESP6 = 0;
		_asm
		{
			mov		ebp,[mBp6]
			mov		esp,[mSp6]
			jmp		[mra6]
		}
#else	// !WIN32
	if( (mra6 = RetAddr6) && 
		(mBp6 = RetBP6) && 
		(mSp6 = RetSP6) )
	{
		RetAddr6 = 0;
		RetBP6 = 0;
		RetSP6 = 0;
		_asm
		{
			mov	bp,[mBp6]				; Get original BP back ...
			mov	sp,[mSp6]
			jmp	dword ptr [mra6]	; Out of here ...
		}
#endif	// WIN32 y/n
	}
return( i );
}

#ifdef	WIN32
void
setexitjump2( void MLPTR Ex, DWORD eSp )
{
	RetAddr6 = Ex;
	_asm
	{
		mov	eax,[ebp]
		mov	[RetBP6],eax
	}
	RetESP6 = eSp;
}
#else	// !WIN32
void
setexitjump2( void MLPTR Ex, WORD Sp )
{
	RetAddr6 = Ex;
	_asm
	{
		mov	ax,[bp]
		mov	RetBP6,ax
	}
	RetSP6 = Sp;
}
#endif	// WIN32 y/n

// One SIZE Functions
//EXPORT32
//DWORD MLIBCALL WJPGSIZE6( HGLOBAL, DWORD, LPSIZE );

// Where :-
// HGLOBAL	hgInData;		// GIF or JPG INPUT data.
// DWORD		ddDataSize;	// Size of the INPUT data.
// LPSIZE		lpCxCy;		// OUPUT Width and Height. (May be NULL).
// Description :-
// In these functions, the HGLOBAL and DWORD pass (some or all) the 
// DATA of the GIF and JPG respectively, and the functions return 
// the DWORD size of the Buffer required to HOLD the completed DIB.
// If some error occurs, like NOT valid GIF or JPG data, or the 
// size is in error or insufficient, etc, the functions would return
// ZERO. No error information is supplied. 
//
// If the function is successful, and if LPSIZE was not a NULL then 
// the int cx; and int cy; members of the tagSIZE structure would be 
// filled with the WIDTH and HEIGHT of the final DIB.
//
// ==========================================================

// One Conversion Functions

//EXPORT32
//BOOL MLIBCALL WJPG2BMP6( HGLOBAL, DWORD, HGLOBAL, HGLOBAL);

// Where :-
// HGLOBAL	hgInData;		// The full GIF or JPG data.
// DWORD		ddDataSize;	// Size of the INPUT data.
// HGLOBAL	hgInfo;		// INFORMATION Buffer (if not NULL).
// HGLOBAL	hgOutData;	// Handle of OUPUT memory.
// Description :-
// The hgInData contains the FULL GIF or JPG data, and ddDataSzie is the
// length of this data.
// hgInfo may be NULL, else if an error occurs ASCII Information will be
// placed in this buffer up to a maximum length of 1024 bytes.
// hgOutData must realise an OUPUT Buffer sufficient in SIZE to completely
// hold the resultant BMP. The structure of this output buffer will be -
// BITMAPINFOHEADER bmih;
// RGBQUAD aColors[];
// BYTE    aBitmapBits[];
// The data in this Ouput buffer will only be valid if the function returns
// FALSE (0). Otherwise an ERROR Number as shown below will be returned.
//
// ============================================================

BOOL	bBeQuick = TRUE;
BOOL	fDnSetup = FALSE;

struct	jpeg_decompress_struct cinfo6;
struct	jpeg_error_mgr jerr6;
struct	djpeg_dest_struct MLPTR dest_mgr6;


void	Zero_Decomp( void )
{
	dest_mgr6 = 0;
	memset( &jerr6, 0, sizeof( jerr6 ) );
	memset( &cinfo6, 0, sizeof( cinfo6 ) );
}

void	SetBmpSize( j_decompress_ptr cinfo,
				   djpeg_dest_ptr dinfo )
{
	bmp_dest_ptr dest;
	INT32 headersize, bfSize;
	int bits_per_pixel, cmap_entries;

	dest = (bmp_dest_ptr) dinfo;
	/* Compute colormap size and total file size */
	if( cinfo->out_color_space == JCS_RGB )
	{
		if( cinfo->quantize_colors )
		{
			/* Colormapped RGB */
			bits_per_pixel = 8;
			cmap_entries = 256;
		}
		else
		{
			/* Unquantized, full color RGB */
			bits_per_pixel = 24;
			cmap_entries = 0;
		}
	}
	else
	{
		/* Grayscale output.  We need to fake a 256-entry colormap. */
		bits_per_pixel = 8;
		cmap_entries = 256;
	}
	/* File size */
	headersize = 40 + cmap_entries * 4; /* Header and colormap */
	bfSize = headersize + (INT32) dest->row_width *
		(INT32) cinfo->output_height;

	dinfo->dwBmpSize = bfSize;
	// *** OR ***
	dest->pub.dwBmpSize = bfSize;
}


DWORD	GetOutSize( j_decompress_ptr cinfo,
				   djpeg_dest_ptr dinfo )
{
	DWORD	dwSize = 0;
	DWORD	dwW, dwH;
	if( (dwH = cinfo->output_height) &&
		(dwW = cinfo6.output_width) )	/* scaled image width */
	{
		// OK, at least establish a 256 colour map BITMAP (DIB)
		// ===================================
		if( dwW % 4 )
			dwW = ((dwW / 4) + 1) * 4;
		dwSize = dwW * dwH;
		dwSize += sizeof( BITMAPINFOHEADER );
		dwSize += 256 * sizeof( RGBQUAD );
	}

	if( dinfo->dwBmpSize == 0 )
		SetBmpSize( cinfo, dinfo );

	if( dwSize < dinfo->dwBmpSize )
		dwSize = dinfo->dwBmpSize;

	return dwSize;
}

EXPORT32
DWORD MLIBCALL WJPGSIZE6( HGLOBAL hgInBuf, DWORD InSize, LPSIZE lpSz )
{
	DWORD	dwSize;
	BOOL	fDnCreate, fDnStart;
	int		retcode;
	int		stsz, stSZ;

	//djpeg_dest_ptr dest_mgr;
	HGLOBAL input_file;
	JDIMENSION num_scanlines;

	dwSize = 0;
	fDnCreate = FALSE;
	fDnStart = FALSE;
	input_file = 0;
	num_scanlines = 0;
	if( ( hgInBuf == 0 ) ||
		( InSize  == 0 ) ||
		( lpSz    == 0 ) )
	{
		goto WJpgSize_Ret;
	}
	input_file = hgInBuf;	// Set the HANDLE

	dest_mgr6 = NULL;
	if( !fDnSetup )
	{
		fDnSetup = TRUE;
		Zero_Decomp();
	}

#ifdef	WIN32
//	SETEXIT( GMAINRET );
	_asm 
	{
		mov  eax,offset WJPGSIZERET ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump2 ;
		add  esp,2 * 4 ;
	}
#else	// !WIN32

	_asm
	{
		mov	ax,offset WJPGSIZERET
		mov	dx,cs		; Return JUMP address ...
		mov	cx,sp		; and STACK frame ...
		push	cx
		push	dx
		push	ax
		call	setexitjump2	; Save these items ...
		add	sp,6
	}
#endif	// WIN32 y/n

  stsz = sizeof( cinfo6 );
  stSZ = (int) SIZEOF(struct jpeg_decompress_struct);
//  stSz = jpeg_GetDecompSize( (int) 1 );

  /* Initialize the JPEG decompression object with default error handling. */
  cinfo6.err = jpeg_std_error( &jerr6 );

//  cinfo6.ci_DecompSize = stsz;

//    jpeg_CreateDecompress( (&cinfo6), JPEG_LIB_VERSION,
//		((size_t) jpeg_GetDecompSize( 1 )) );
  jpeg_create_decompress( &cinfo6 );
  fDnCreate = TRUE;

#ifdef	ADDERRDATA
  /* Add some application-specific error messages (from cderror.h) */
  jerr6.addon_message_table = cdjpeg_message_table;
  jerr6.first_addon_message = JMSG_FIRSTADDONCODE;
  jerr6.last_addon_message = JMSG_LASTADDONCODE;
#else	// !ADDERRDATA
#if	(defined( _INC_WINDOWS ) || defined( WJPEG6 ))
  jerr6.addon_message_table = 0;
  jerr6.first_addon_message = 0;
  jerr6.last_addon_message = 0;
#else	// !
  /* Add some application-specific error messages (from cderror.h) */
  jerr6.addon_message_table = cdjpeg_message_table;
  jerr6.first_addon_message = JMSG_FIRSTADDONCODE;
  jerr6.last_addon_message = JMSG_LASTADDONCODE;
#endif	// !_INC_WINDOWS or WJPEG6
#endif	// ADDERRDATA y/n
	TRACEMS3( &cinfo6, 2, JTRC_WJPGSIZE6,
		(int)hgInBuf, (int)InSize, (int)lpSz );

  /* Insert custom COM marker processor. */
  jpeg_set_marker_processor( &cinfo6, JPEG_COM, COM_handler );

  /* Scan command line to find file names. */
  /* It is convenient to use just one switch-parsing routine, but the switch
   * values read here are ignored; we will rescan the switches after opening
   * the input file.
   * (Exception: tracing level set here controls verbosity for COM markers
   * found during jpeg_read_header...)
   */

  /* Specify data source for decompression */
  //jpeg_stdio_src( &cinfo6, input_file );
  jpeg_handle_src( &cinfo6, input_file, InSize );

  /* Read file header, set default decompression parameters */
  retcode = jpeg_read_header( &cinfo6, FALSE );

  if( retcode == JPEG_HEADER_OK )
  {

  /* Adjust default decompression parameters by re-parsing the options */
  // file_index = parse_switches(&cinfo, argc, argv, 0, TRUE);
	  // Library declarations
	// In this case, COPY the DLL Config structured items to the
	// BIG STRUCTURE POINTER.
	  CopyConfig6( &cinfo6 );

  /* Initialize the output module now to let it override any crucial
   * option settings (for instance, GIF wants to force color quantization).
   */
  dest_mgr6 = jinit_write_bmp( &cinfo6, FALSE );

  //dest_mgr6->output_file = output_file;
  dest_mgr6->output_file = 0;

  /* Start decompressor */
  (void) jpeg_start_decompress( &cinfo6 );
  fDnStart = TRUE;

  // We have to WAIT until after HERE
  // to get ALL the SIZE info!!!!!!!!
  // OR DO WE?????????????
  /* Write output file header */
  (*dest_mgr6->start_output) ( &cinfo6, dest_mgr6 );

  dwSize = GetOutSize( &cinfo6, dest_mgr6 );
  lpSz->cx = cinfo6.output_width;
  lpSz->cy = cinfo6.output_height;

	if( !bBeQuick )
	{

		// ===============================

  /* Process data */
  while ( cinfo6.output_scanline < cinfo6.output_height) {
    num_scanlines = jpeg_read_scanlines( &cinfo6, dest_mgr6->buffer,
					dest_mgr6->buffer_height);
    (*dest_mgr6->put_pixel_rows) ( &cinfo6, dest_mgr6,
		num_scanlines);
  }

  /* Finish decompression and release memory.
   * I must do it in this order because output module has allocated memory
   * of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
   */
  (*dest_mgr6->finish_output) ( &cinfo6, dest_mgr6 );

		if( fDnStart )
		{
			(void) jpeg_finish_decompress( &cinfo6 );
		}
		// ===============================
	}	// not if bBeQuick
  }	// retcode = JPEG_HEADER_OK;

WJpgSize_Ret:
#ifdef  WIN32
        _asm
        {
WJPGSIZERET:
                mov             eax, [bErrFlag2]
        }
#else   // !WIN#@
        _asm
        {
WJPGSIZERET:
                mov     ax,bErrFlag2     ;       Get the ERROR FLAG ...
        }
#endif  // WIN#@ y/n
		if( fDnCreate )
		{
			jpeg_destroy_decompress( &cinfo6 );
		}
		KillError2();

		if( bErrFlag2 )
			TRACEMS1( &cinfo6, 2, JTRC_WJPGSIZE6_ERR, bErrFlag2 );
		else
			TRACEMS1( &cinfo6, 2, JTRC_WJPGSIZE6_RET, dwSize );

		return dwSize;	// Return any SIZE found
}

// One Conversion Functions

//EXPORT32
//BOOL MLIBCALL WJPG2BMP6( HGLOBAL, DWORD, HGLOBAL, HGLOBAL);

// Where :-
// HGLOBAL	hgInData;		// The full GIF or JPG data.
// DWORD		ddDataSize;	// Size of the INPUT data.
// HGLOBAL	hgInfo;		// INFORMATION Buffer (if not NULL).
// HGLOBAL	hgOutData;	// Handle of OUPUT memory.
// Description :-
// The hgInData contains the FULL GIF or JPG data, and ddDataSzie is the
// length of this data.
// hgInfo may be NULL, else if an error occurs ASCII Information will be
// placed in this buffer up to a maximum length of 1024 bytes.
// hgOutData must realise an OUPUT Buffer sufficient in SIZE to completely
// hold the resultant BMP. The structure of this output buffer will be -
// BITMAPINFOHEADER bmih;
// RGBQUAD aColors[];
// BYTE    aBitmapBits[];
// The data in this Ouput buffer will only be valid if the function returns
// FALSE (0). Otherwise an ERROR Number as shown below will be returned.
//
EXPORT32
BOOL MLIBCALL WJPG2BMP6( HGLOBAL hgInBuf, DWORD hgInLen,
						HGLOBAL hgInfo, HGLOBAL hgOutput )
{
	BOOL	fDnCreate, fDnStart;
	BOOL	rFlg;
	DWORD	dwSize;
	SIZE	sz;
	LPSIZE	lpSz;
//  struct jpeg_decompress_struct cinfo;
//  struct jpeg_error_mgr jerr;
	//djpeg_dest_ptr dest_mgr = NULL;
	HGLOBAL	input_file;
	HGLOBAL	hg;
	JDIMENSION num_scanlines;
	int		retcode;

	dwSize = 0;
	rFlg = FALSE;
	lpSz = &sz;
	fDnCreate = FALSE;
	fDnStart = FALSE;
	bErrFlag2 = 5007;
	if( ((input_file = hgInBuf) == 0 ) ||
		( hgInLen == 0 ) ||
		( (hg = hgOutput) == 0 ) )
	{
		goto WJpg2Bmp_Ret;
	}
	bErrFlag2 = 0;

#ifdef	WIN32
//	SETEXIT( GMAINRET );
	_asm 
	{
		mov  eax,offset WJPG2BMP6RET ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump2 ;
		add  esp,2 * 4 ;
	}
#else	// !WIN32

	_asm
	{
		mov	ax,offset WJPG2BMP6RET
		mov	dx,cs		; Return JUMP address ...
		mov	cx,sp		; and STACK frame ...
		push	cx
		push	dx
		push	ax
		call	setexitjump2	; Save these items ...
		add	sp,6
	}
#endif	// WIN32 y/n

	if( hgInfo )
	{
		if( lpCommBuf = GlobalLock( hgInfo ) )
		{
			dwMaxComm = GlobalSize( hgInfo );
			if( dwMaxComm > 20 )
			{
				dwMaxComm -= 20;
			}
			else
			{
				GlobalUnlock( hgInfo );
				lpCommBuf = 0;
			}
		}
	}

	/* Initialize the JPEG decompression object with default error handling. */
	cinfo6.err = jpeg_std_error( &jerr6 );

//    jpeg_CreateDecompress( (&cinfo6), JPEG_LIB_VERSION,
//		((size_t) jpeg_GetDecompSize( 1 )) );
	jpeg_create_decompress( &cinfo6 );
	fDnCreate = TRUE;

#ifdef	ADDERRDATA
	/* Add some application-specific error messages (from cderror.h) */
	jerr6.addon_message_table = cdjpeg_message_table;
	jerr6.first_addon_message = JMSG_FIRSTADDONCODE;
	jerr6.last_addon_message = JMSG_LASTADDONCODE;
#else	// !ADDERRDATA
#if	(defined( _INC_WINDOWS ) || defined( WJPEG6 ))
	jerr6.addon_message_table = 0;
	jerr6.first_addon_message = 0;
	jerr6.last_addon_message = 0;
#else	// !_INC_WINDOWS
	/* Add some application-specific error messages (from cderror.h) */
	jerr6.addon_message_table = cdjpeg_message_table;
	jerr6.first_addon_message = JMSG_FIRSTADDONCODE;
	jerr6.last_addon_message = JMSG_LASTADDONCODE;
#endif	// _INC_WINDOWS y/n
#endif	// ADDERRDATA

	/* Insert custom COM marker processor. */
	jpeg_set_marker_processor( &cinfo6, JPEG_COM, COM_handler);

	/* Specify data source for decompression */
	jpeg_handle_src( &cinfo6, input_file, hgInLen );
	//jpeg_stdio_src( &cinfo6, input_file);

	/* Read file header, set default decompression parameters */
	retcode = jpeg_read_header( &cinfo6, TRUE );

	/* Adjust default decompression parameters by re-parsing the options */
	// file_index = parse_switches(&cinfo, argc, argv, 0, TRUE);
	// In this case, COPY the DLL Config structured items to the
	// BIG STRUCTURE POINTER.
	CopyConfig6( &cinfo6 );

	/* Initialize the output module now to let it override any crucial
	 * option settings (for instance, GIF wants to force color quantization).
	 */
	dest_mgr6 = jinit_write_bmp( &cinfo6, FALSE);

	dest_mgr6->output_file = hg;

  /* Start decompressor */
  (void) jpeg_start_decompress( &cinfo6 );
  fDnStart = TRUE;

  /* Write output file header */
  (*dest_mgr6->start_output) ( &cinfo6, dest_mgr6 );

  dwSize = GetOutSize( &cinfo6, dest_mgr6 );
  //dwSize = cinfo6.output_height * cinfo6.output_width;	/* scaled image width */
  lpSz->cx = cinfo6.output_width;
  lpSz->cy = cinfo6.output_height;

  /* Process data */
  while (cinfo6.output_scanline < cinfo6.output_height) {
    num_scanlines = jpeg_read_scanlines( &cinfo6, dest_mgr6->buffer,
					dest_mgr6->buffer_height);
    (*dest_mgr6->put_pixel_rows) (&cinfo6, dest_mgr6, num_scanlines);
  }

  // This is where the DIB is built
  // ==============================
  (*dest_mgr6->finish_output) ( &cinfo6, dest_mgr6 );


  /* Finish decompression and release memory.
   * I must do it in this order because output module has allocated memory
   * of lifespan JPOOL_IMAGE; it needs to finish before releasing memory.
   */
		if( fDnStart )
		{
			(void) jpeg_finish_decompress( &cinfo6);
		}
	rFlg = TRUE;

WJpg2Bmp_Ret:
#ifdef  WIN32
        _asm
        {
WJPG2BMP6RET:
                mov             eax, [bErrFlag2]
        }
#else   // !WIN#@
        _asm
        {
WJPG2BMP6RET:
                mov     ax,bErrFlag2     ;       Get the ERROR FLAG ...
        }
#endif  // WIN#@ y/n
		if( fDnCreate )
		{
			jpeg_destroy_decompress(&cinfo6);
		}

		if( hgInfo && lpCommBuf )
			GlobalUnlock( hgInfo );

		KillError2();
		return( (BOOL) bErrFlag2 );
}

//==========================================================
//
// AN EXTERNAL MEMORY MANAGER - For DIAGNOSTIC purposes
//
// This allows an Application to supply the following
// structure (originally in DvMemF.h) as follows,
// but now also in a local COPY, WMemDiag.h
//
//typedef struct tagJPEG_EXTMM {
//	DWORD		dwMM_Size;
//	void MLPTR	(*EXTLOCALALLOC)	( DWORD dwSz );
//	HGLOBAL		(*EXTLOCALFREE)		( void MLPTR );
//	HGLOBAL		(*EXTGLOBALALLOC)	( DWORD dwSz );
//	void MLPTR	(*EXTGLOBALLOCK)	( HGLOBAL hg );
//	void MLPTR	(*EXTGLOBALUNLOCK)	( HGLOBAL hg );
//	HGLOBAL		(*EXTGLOBALFREE)	( HGLOBAL hg );
//	void		(*EXTDEBUGSTOP)		( int code );
//	void		(*EXTTRACEOUT)		( LPSTR lpt );
//}JPEG_EXTMM;
//typedef JPEG_EXTMM MLPTR LPJPEG_EXTMM;

//HGLOBAL	FDv_GlobalAlloc( DWORD dwSz )
//{
//	HGLOBAL hg;
//	hg = GlobalAlloc( GHND, dwSz );
//	return hg;
//}

JPEG_EXTMM	ExtMM = { 0 };
LPJPEG_EXTMM	lpExtMM = 0;

//void	SetExtMM( void )
//{
//	ExtMM.EXTGLOBALALLOC = &FDv_GlobalAlloc;
//}

//HGLOBAL	Dv_GlobalAlloc2( DWORD dws )
//{
//	HGLOBAL	hg = 0;
//	if( ExtMM.EXTGLOBALALLOC )
//		hg = (*ExtMM.EXTGLOBALALLOC) ( dws );
//	else
//		hg = GlobalAlloc( GHND, dws );
//	return hg;
//}

//EXPORT32
//BOOL MLIBCALL WEXTMEMMGR( LPJPEG_EXTMM lpMM )
//{
//	BOOL	flg = FALSE;
//	if( lpMM &&
//		( lpMM->dwMM_Size = sizeof( JPEG_EXTMM ) ) &&
//		( lpMM->EXTGLOBALALLOC ) &&
//		( lpMM->EXTGLOBALLOCK ) &&
//		( lpMM->EXTGLOBALUNLOCK ) &&
//		( lpMM->EXTGLOBALFREE ) )
//	{
//		flg = TRUE;
//		ExtMM.dwMM_Size = lpMM->dwMM_Size;
//		ExtMM.EXTGLOBALALLOC = lpMM->EXTGLOBALALLOC;
//		ExtMM.EXTGLOBALLOCK = lpMM->EXTGLOBALLOCK;
//		ExtMM.EXTGLOBALUNLOCK = lpMM->EXTGLOBALUNLOCK;
//		ExtMM.EXTGLOBALFREE = lpMM->EXTGLOBALFREE;
//	}
//	return flg;
//}

//HGLOBAL	Dv_GlobalAlloc( DWORD dws )
//{
//	HGLOBAL	hg = 0;
//	if( lpExtMM &&
//		( lpExtMM->EXTGLOBALALLOC ) )
//		hg = (*lpExtMM->EXTGLOBALALLOC) ( dws );
//	else
//		hg = GlobalAlloc( GHND, dws );
//	return hg;
//}


void MLPTR Dv_LocalAlloc( DWORD dws )
{
	void MLPTR lp = 0;
	if( lpExtMM &&
		( lpExtMM->EXTLOCALALLOC ) )
		lp = (*lpExtMM->EXTLOCALALLOC) ( dws );
	return lp;
}

void MLPTR Dv_LocalFree( void MLPTR lpi )
{
	void MLPTR lp;
	if( ( lp = lpi ) &&
		lpExtMM &&
		( lpExtMM->EXTLOCALFREE ) )
		lp = (*lpExtMM->EXTLOCALFREE) ( lp );
	return lp;
}

void	ExtDebugStop( int code )
{
	if( ( lpExtMM ) &&
		( lpExtMM->EXTDEBUGSTOP ) )
	{
		(*lpExtMM->EXTDEBUGSTOP) ( code );
	}
}

BOOL	GotTraceOut( void )
{
	BOOL	flg = FALSE;
	if( ( lpExtMM ) &&
		( lpExtMM->EXTTRACEOUT ) )
	{
		flg = TRUE;
	}
	return flg;
}
void	ExtTraceOut( LPSTR lpt )
{
	if( GotTraceOut() )
	{
		(*lpExtMM->EXTTRACEOUT) ( lpt );
	}
}

// #define		mf_TurnOn		0x10000000	// ON, else OFF
EXPORT32
BOOL MLIBCALL WEXTMMGR( LPJPEG_EXTMM lpMM, DWORD dwFlag )
{
	BOOL	flg = FALSE;
	if( lpMM &&
		( lpMM->dwMM_Size = sizeof( JPEG_EXTMM ) ) &&
		( lpMM->EXTLOCALALLOC ) &&
		( lpMM->EXTLOCALFREE ) &&
		( lpMM->EXTGLOBALALLOC ) &&
		( lpMM->EXTGLOBALLOCK ) &&
		( lpMM->EXTGLOBALUNLOCK ) &&
		( lpMM->EXTGLOBALFREE ) )
	{
		flg = TRUE;
		if( dwFlag & mf_TurnOn )
			lpExtMM = lpMM;
		else
			lpExtMM = 0;	// Turn it OFF
	}
	else
	{
		lpExtMM = 0;	// Turn it OFF
	}
	return flg;
}

// End of EXTERNAL MEMORY MANAGER
//==========================================================

#if		( _MSC_VER > 700 )
#if		(defined( NDEBUG ) && defined( Dv16_App ))
// **************************************************************
#pragma warning( default : 4704 )
// **************************************************************
#endif	// NDEBUG & Dv16_App
#endif	// _MSC_VER > 700


// eof - WJpg2Bmp.c
