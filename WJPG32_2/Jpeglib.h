
#ifndef JPEGLIB_H
#define JPEGLIB_H
/*
 * jpeglib.h
 *
 * Copyright (C) 1991-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file defines the application interface for the JPEG library.
 * Most applications using the library need only include this file,
 * and perhaps jerror.h if they want to know the exact error codes.
 */

#define	JPEGLIB_ORG

#if	(defined( WIN32 ) || defined( WJPEG6 ))

// WIN32 is a NORMAL define in MS Studio 4.++
// ==========================================
// BUT can be over-ridden with WJPEG6 moniker passed in ...
// ====================================================

#ifndef WJPEG6
#define WJPEG6
#endif  // !WJPEG6

#ifndef _INC_WINDOWS
#include        <windows.h>
#endif  // _INC_WINDOWS

#ifndef	_mwin32_h
#include	"win32.h"	// For MLPTR, and MFILEPTR especially
// These ADD portability back to 16-bit code using 20-bit
// addressing, with on a modern CPU cost no more time really.
// Sometime 32-bit OS's have rather kluggy translations, but
// this should run fast on all good CPU's
#endif	// _mwin32_h

#endif	// WIN32 (or 16 if needed)

/*
 * First we include the configuration files that record how this
 * installation of the JPEG library is set up.  jconfig.h can be
 * generated automatically for many systems.  jmorecfg.h contains
 * manual configuration options that most people need not worry about.
 */

#ifndef JCONFIG_INCLUDED	/* in case jinclude.h already did */
#include "jconfig.h"		/* widely used configuration options */
#endif
#include "jmorecfg.h"		/* seldom changed options */


/* Version ID for the JPEG library.
 * Might be useful for tests like "#if JPEG_LIB_VERSION >= 60".
 */

#define JPEG_LIB_VERSION  61	/* Version 6a */

/* Various constants determining the sizes of things.
 * All of these are specified by the JPEG standard, so don't change them
 * if you want to be compatible.
 */

#define DCTSIZE		    8	/* The basic DCT block is 8x8 samples */
#define DCTSIZE2	    64	/* DCTSIZE squared; # of elements in a block */
#define NUM_QUANT_TBLS      4	/* Quantization tables are numbered 0..3 */
#define NUM_HUFF_TBLS       4	/* Huffman tables are numbered 0..3 */
#define NUM_ARITH_TBLS      16	/* Arith-coding tables are numbered 0..15 */
#define MAX_COMPS_IN_SCAN   4	/* JPEG limit on # of components in one scan */
#define MAX_SAMP_FACTOR     4	/* JPEG limit on sampling factors */
/* Unfortunately, some bozo at Adobe saw no reason to be bound by the standard;
 * the PostScript DCT filter can emit files with many more than 10 blocks/MCU.
 * If you happen to run across such a file, you can up D_MAX_BLOCKS_IN_MCU
 * to handle it.  We even let you do this from the jconfig.h file.  However,
 * we strongly discourage changing C_MAX_BLOCKS_IN_MCU; just because Adobe
 * sometimes emits noncompliant files doesn't mean you should too.
 */
#define C_MAX_BLOCKS_IN_MCU   10 /* compressor's limit on blocks per MCU */
#ifndef D_MAX_BLOCKS_IN_MCU
#define D_MAX_BLOCKS_IN_MCU   10 /* decompressor's limit on blocks per MCU */
#endif


/* Data structures for images (arrays of samples and of DCT coefficients).
 * On 80x86 machines, the image arrays are too big for near pointers,
 * but the pointer arrays can fit in near memory.
 */

typedef JSAMPLE MLPTR JSAMPROW;	/* ptr to one image row of pixel samples. */
typedef JSAMPROW MLPTR JSAMPARRAY;	/* ptr to some rows (a 2-D sample array) */
typedef JSAMPARRAY MLPTR JSAMPIMAGE;	/* a 3-D sample array: top index is color */

typedef JCOEF JBLOCK[DCTSIZE2];	/* one block of coefficients */
typedef JBLOCK MLPTR JBLOCKROW;	/* pointer to one row of coefficient blocks */
typedef JBLOCKROW MLPTR JBLOCKARRAY;		/* a 2-D array of coefficient blocks */
typedef JBLOCKARRAY MLPTR JBLOCKIMAGE;	/* a 3-D array of coefficient blocks */

typedef JCOEF MLPTR JCOEFPTR;	/* useful in a couple of places */


#include	"JpgStru6.h"

/* Declarations for routines called by application.
 * The JPP macro hides prototype parameters from compilers that can't cope.
 * Note JPP requires double parentheses.
 */

#ifdef HAVE_PROTOTYPES
#define JPP(arglist)	arglist
#else
#define JPP(arglist)	()
#endif


/* Short forms of external names for systems with brain-damaged linkers.
 * We shorten external names to be unique in the first six letters, which
 * is good enough for all known systems.
 * (If your compiler itself needs names to be unique in less than 15 
 * characters, you are out of luck.  Get a better compiler.)
 */

#ifdef NEED_SHORT_EXTERNAL_NAMES
#define jpeg_std_error		jStdError
#define jpeg_CreateCompress	jCreaCompress
#define jpeg_CreateDecompress	jCreaDecompress
#define jpeg_destroy_compress	jDestCompress
#define jpeg_destroy_decompress	jDestDecompress
#define jpeg_stdio_dest		jStdDest
#define jpeg_stdio_src		jStdSrc
#define jpeg_set_defaults	jSetDefaults
#define jpeg_set_colorspace	jSetColorspace
#define jpeg_default_colorspace	jDefColorspace
#define jpeg_set_quality	jSetQuality
#define jpeg_set_linear_quality	jSetLQuality
#define jpeg_add_quant_table	jAddQuantTable
#define jpeg_quality_scaling	jQualityScaling
#define jpeg_simple_progression	jSimProgress
#define jpeg_suppress_tables	jSuppressTables
#define jpeg_alloc_quant_table	jAlcQTable
#define jpeg_alloc_huff_table	jAlcHTable
#define jpeg_start_compress	jStrtCompress
#define jpeg_write_scanlines	jWrtScanlines
#define jpeg_finish_compress	jFinCompress
#define jpeg_write_raw_data	jWrtRawData
#define jpeg_write_marker	jWrtMarker
#define jpeg_write_tables	jWrtTables
#define jpeg_read_header	jReadHeader
#define jpeg_start_decompress	jStrtDecompress
#define jpeg_read_scanlines	jReadScanlines
#define jpeg_finish_decompress	jFinDecompress
#define jpeg_read_raw_data	jReadRawData
#define jpeg_has_multiple_scans	jHasMultScn
#define jpeg_start_output	jStrtOutput
#define jpeg_finish_output	jFinOutput
#define jpeg_input_complete	jInComplete
#define jpeg_new_colormap	jNewCMap
#define jpeg_consume_input	jConsumeInput
#define jpeg_calc_output_dimensions	jCalcDimensions
#define jpeg_set_marker_processor	jSetMarker
#define jpeg_read_coefficients	jReadCoefs
#define jpeg_write_coefficients	jWrtCoefs
#define jpeg_copy_critical_parameters	jCopyCrit
#define jpeg_abort_compress	jAbrtCompress
#define jpeg_abort_decompress	jAbrtDecompress
#define jpeg_abort		jAbort
#define jpeg_destroy		jDestroy
#define jpeg_resync_to_restart	jResyncRestart
#endif /* NEED_SHORT_EXTERNAL_NAMES */


/* Default error-management setup */
EXTERN(struct jpeg_error_mgr MLPTR) jpeg_std_error
	JPP((struct jpeg_error_mgr MLPTR err));

/* Initialization of JPEG compression objects.
 * jpeg_create_compress() and jpeg_create_decompress() are the exported
 * names that applications should call.  These expand to calls on
 * jpeg_CreateCompress and jpeg_CreateDecompress with additional information
 * passed for version mismatch checking.
 * NB: you must set up the error-manager BEFORE calling jpeg_create_xxx.
 */
#define jpeg_create_compress(cinfo) \
    jpeg_CreateCompress((cinfo), JPEG_LIB_VERSION, \
			(size_t) sizeof(struct jpeg_compress_struct))

#define jpeg_create_decompress(cinfo) \
    jpeg_CreateDecompress((cinfo), JPEG_LIB_VERSION, \
			  (size_t) sizeof(struct jpeg_decompress_struct))

EXTERN(void) jpeg_CreateCompress JPP((j_compress_ptr cinfo,
				      int version, size_t structsize));
EXTERN(void) jpeg_CreateDecompress JPP((j_decompress_ptr cinfo,
					int version, size_t structsize));

/* Destruction of JPEG compression objects */
EXTERN(void) jpeg_destroy_compress JPP((j_compress_ptr cinfo));
EXTERN(void) jpeg_destroy_decompress JPP((j_decompress_ptr cinfo));

/* Standard data source and destination managers: stdio streams. */
/* Caller is responsible for opening the file before and closing after. */
EXTERN(void) jpeg_stdio_dest JPP((j_compress_ptr cinfo,
								 void MLPTR outfile));
EXTERN(void) jpeg_stdio_src JPP((j_decompress_ptr cinfo,
								void MLPTR infile));

/*
 * Standard data source and destination managers:
 * Using Windows HANDLES. Mostly HGLOBAL objects
 */
#if	(defined( _INC_WINDOWS ) || defined( WJPEG6 ))
// June 1997 - Added to IJG 6a source - Geoff McLane
// EMail: 70651.3447@compuserve.com

// For DECOMPRESSION - JPG to BMP
// Use GLOBAL MEMORY HANDLES as input and output
// Services WJPG2BMP6 (and WJPGSIZE6)
EXTERN(void) jpeg_handle_src JPP((j_decompress_ptr cinfo,
								 HGLOBAL infile,
								 DWORD	dwLength ));

// For COMPRESSION
// Uses GLOBAL MEMORY HANDLE as input, but
// here OUPUT is directly to the DISK file.
// Services WBMPTOJPG6, and
EXTERN(void) jpeg_winio_dest JPP((j_compress_ptr cinfo,
								 HFILE outfile));
// WBMP2JPG6
EXTERN(void) jpeg_handle_dest JPP((j_compress_ptr cinfo,
								  HGLOBAL outfile ));

#endif	// _INC_WIND

/* Default parameter setup for compression */
EXTERN(void) jpeg_set_defaults JPP((j_compress_ptr cinfo));
/* Compression parameter setup aids */
EXTERN(void) jpeg_set_colorspace JPP((j_compress_ptr cinfo,
				      J_COLOR_SPACE colorspace));
EXTERN(void) jpeg_default_colorspace JPP((j_compress_ptr cinfo));
EXTERN(void) jpeg_set_quality JPP((j_compress_ptr cinfo, int quality,
				   boolean force_baseline));
EXTERN(void) jpeg_set_linear_quality JPP((j_compress_ptr cinfo,
					  int scale_factor,
					  boolean force_baseline));
EXTERN(void) jpeg_add_quant_table JPP((j_compress_ptr cinfo, int which_tbl,
				       const unsigned int MLPTR basic_table,
				       int scale_factor,
				       boolean force_baseline));
EXTERN(int) jpeg_quality_scaling JPP((int quality));
EXTERN(void) jpeg_simple_progression JPP((j_compress_ptr cinfo));
EXTERN(void) jpeg_suppress_tables JPP((j_compress_ptr cinfo,
				       boolean suppress));
EXTERN(JQUANT_TBL MLPTR) jpeg_alloc_quant_table JPP((j_common_ptr cinfo));
EXTERN(JHUFF_TBL MLPTR) jpeg_alloc_huff_table JPP((j_common_ptr cinfo));

/* Main entry points for compression */
EXTERN(void) jpeg_start_compress JPP((j_compress_ptr cinfo,
				      boolean write_all_tables));
EXTERN(JDIMENSION) jpeg_write_scanlines JPP((j_compress_ptr cinfo,
					     JSAMPARRAY scanlines,
					     JDIMENSION num_lines));
EXTERN(void) jpeg_finish_compress JPP((j_compress_ptr cinfo));

/* Replaces jpeg_write_scanlines when writing raw downsampled data. */
EXTERN(JDIMENSION) jpeg_write_raw_data JPP((j_compress_ptr cinfo,
					    JSAMPIMAGE data,
					    JDIMENSION num_lines));

/* Write a special marker.  See libjpeg.doc concerning safe usage. */
EXTERN(void) jpeg_write_marker
	JPP((j_compress_ptr cinfo, int marker,
	     const JOCTET MLPTR dataptr, unsigned int datalen));

/* Alternate compression function: just write an abbreviated table file */
EXTERN(void) jpeg_write_tables JPP((j_compress_ptr cinfo));

/* Decompression startup: read start of JPEG datastream to see what's there */
EXTERN(int) jpeg_read_header JPP((j_decompress_ptr cinfo,
				  boolean require_image));
/* Return value is one of: */
#define JPEG_SUSPENDED		0 /* Suspended due to lack of input data */
#define JPEG_HEADER_OK		1 /* Found valid image datastream */
#define JPEG_HEADER_TABLES_ONLY	2 /* Found valid table-specs-only datastream */
/* If you pass require_image = TRUE (normal case), you need not check for
 * a TABLES_ONLY return code; an abbreviated file will cause an error exit.
 * JPEG_SUSPENDED is only possible if you use a data source module that can
 * give a suspension return (the stdio source module doesn't).
 */

/* Main entry points for decompression */
EXTERN(boolean) jpeg_start_decompress JPP((j_decompress_ptr cinfo));
EXTERN(JDIMENSION) jpeg_read_scanlines JPP((j_decompress_ptr cinfo,
					    JSAMPARRAY scanlines,
					    JDIMENSION max_lines));
EXTERN(boolean) jpeg_finish_decompress JPP((j_decompress_ptr cinfo));

/* Replaces jpeg_read_scanlines when reading raw downsampled data. */
EXTERN(JDIMENSION) jpeg_read_raw_data JPP((j_decompress_ptr cinfo,
					   JSAMPIMAGE data,
					   JDIMENSION max_lines));

/* Additional entry points for buffered-image mode. */
EXTERN(boolean) jpeg_has_multiple_scans JPP((j_decompress_ptr cinfo));
EXTERN(boolean) jpeg_start_output JPP((j_decompress_ptr cinfo,
				       int scan_number));
EXTERN(boolean) jpeg_finish_output JPP((j_decompress_ptr cinfo));
EXTERN(boolean) jpeg_input_complete JPP((j_decompress_ptr cinfo));
EXTERN(void) jpeg_new_colormap JPP((j_decompress_ptr cinfo));
EXTERN(int) jpeg_consume_input JPP((j_decompress_ptr cinfo));
/* Return value is one of: */
/* #define JPEG_SUSPENDED	0    Suspended due to lack of input data */
#define JPEG_REACHED_SOS	1 /* Reached start of new scan */
#define JPEG_REACHED_EOI	2 /* Reached end of image */
#define JPEG_ROW_COMPLETED	3 /* Completed one iMCU row */
#define JPEG_SCAN_COMPLETED	4 /* Completed last iMCU row of a scan */

/* Precalculate output dimensions for current decompression parameters. */
EXTERN(void) jpeg_calc_output_dimensions JPP((j_decompress_ptr cinfo));

/* Install a special processing method for COM or APPn markers. */
EXTERN(void) jpeg_set_marker_processor
	JPP((j_decompress_ptr cinfo, int marker_code,
	     jpeg_marker_parser_method routine));

/* Read or write raw DCT coefficients --- useful for lossless transcoding. */
EXTERN(jvirt_barray_ptr MLPTR) jpeg_read_coefficients JPP((j_decompress_ptr cinfo));
EXTERN(void) jpeg_write_coefficients JPP((j_compress_ptr cinfo,
					  jvirt_barray_ptr MLPTR coef_arrays));
EXTERN(void) jpeg_copy_critical_parameters JPP((j_decompress_ptr srcinfo,
						j_compress_ptr dstinfo));

/* If you choose to abort compression or decompression before completing
 * jpeg_finish_(de)compress, then you need to clean up to release memory,
 * temporary files, etc.  You can just call jpeg_destroy_(de)compress
 * if you're done with the JPEG object, but if you want to clean it up and
 * reuse it, call this:
 */
EXTERN(void) jpeg_abort_compress JPP((j_compress_ptr cinfo));
EXTERN(void) jpeg_abort_decompress JPP((j_decompress_ptr cinfo));

/* Generic versions of jpeg_abort and jpeg_destroy that work on either
 * flavor of JPEG object.  These may be more convenient in some places.
 */
EXTERN(void) jpeg_abort JPP((j_common_ptr cinfo));
EXTERN(void) jpeg_destroy JPP((j_common_ptr cinfo));

/* Default restart-marker-resync procedure for use by data source modules */
EXTERN(boolean) jpeg_resync_to_restart JPP((j_decompress_ptr cinfo,
					    int desired));

/* These marker codes are exported since applications and data source modules
 * are likely to want to use them.
 */

#define JPEG_RST0	0xD0	/* RST0 marker code */
#define JPEG_EOI	0xD9	/* EOI marker code */
#define JPEG_APP0	0xE0	/* APP0 marker code */
#define JPEG_COM	0xFE	/* COM marker code */


/* If we have a brain-damaged compiler that emits warnings (or worse, errors)
 * for structure definitions that are never filled in, keep it quiet by
 * supplying dummy definitions for the various substructures.
 */

#ifdef INCOMPLETE_TYPES_BROKEN
#ifndef JPEG_INTERNALS		/* will be defined in jpegint.h */
struct jvirt_sarray_control { long dummy; };
struct jvirt_barray_control { long dummy; };
struct jpeg_comp_master { long dummy; };
struct jpeg_c_main_controller { long dummy; };
struct jpeg_c_prep_controller { long dummy; };
struct jpeg_c_coef_controller { long dummy; };
struct jpeg_marker_writer { long dummy; };
struct jpeg_color_converter { long dummy; };
struct jpeg_downsampler { long dummy; };
struct jpeg_forward_dct { long dummy; };
struct jpeg_entropy_encoder { long dummy; };
struct jpeg_decomp_master { long dummy; };
struct jpeg_d_main_controller { long dummy; };
struct jpeg_d_coef_controller { long dummy; };
struct jpeg_d_post_controller { long dummy; };
struct jpeg_input_controller { long dummy; };
struct jpeg_marker_reader { long dummy; };
struct jpeg_entropy_decoder { long dummy; };
struct jpeg_inverse_dct { long dummy; };
struct jpeg_upsampler { long dummy; };
struct jpeg_color_deconverter { long dummy; };
struct jpeg_color_quantizer { long dummy; };
#endif /* JPEG_INTERNALS */
#endif /* INCOMPLETE_TYPES_BROKEN */


/*
 * The JPEG library modules define JPEG_INTERNALS before including this file.
 * The internal structure declarations are read only when that is true.
 * Applications using the library should not include jpegint.h, but may wish
 * to include jerror.h.
 */

#ifdef JPEG_INTERNALS
#include "jpegint.h"		/* fetch private declarations */
#include "jerror.h"		/* fetch error codes too */
#endif

#endif /* JPEGLIB_H */
