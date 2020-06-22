

// WComCfg.c

#include	<windows.h>
#include	"Win32.h"

// This module will try to incorporate ALL the Config items
// of the DECOMPRESSION Module. So, like WJpg2Bmp.c
// this module will again be BASED on DJPEG.c :-

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

#include "cdjpeg.h"		/* Common decls for cjpeg/djpeg applications */
#include "jversion.h"		/* for version message */
#include	"WComCfg.h"

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

int _gwscanf( LPSTR lpd, LPSTR lpf, LPSTR lpv );

/* Create the add-on message string table. */
#ifdef	ADDERRDATA
// This is where it is DEFINED ONLY ONCE in the DLL
#define JMESSAGE(code,string)	string ,

const char MLPTR cdjpeg_message_table[] = {
#include "cderror.h"
  NULL
};
#endif	// ADDERRDATA

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

static IMAGE_FORMATS requested_fmt;


/*
 * Argument-parsing code.
 * The switch parser is designed to be useful with DOS-style command line
 * syntax, ie, intermixed switches and file names, where only the switches
 * to the left of a given file name affect processing of that file.
 * The main program in this file doesn't actually use this capability...
 */


/* Give AID for the command line */

#define		BUFFOK	( (i = lstrlen( lpb )) < ( iMax - 80 ) )
#define		WEOL	"\r\n"

char	szColors[] = "colors";
char	szColours[] = "colours";
char	szQuantize[] = "quantize";
char	szQuantise[] = "quantise";
//			cinfo->desired_number_of_colors = val;
//			cinfo->quantize_colors = TRUE;

char	szDct[] = "dct";
char	szDctSlow[] = "int";
char	szDctFast[] = "fast";
char	szDctFloat[] = "float";
//			/* Select IDCT algorithm. */
//			} else if (keymatch(argv[argn], "int", 2)) {
//				cinfo->dct_method = JDCT_ISLOW;
//			} else if (keymatch(argv[argn], "fast", 2)) {
//				cinfo->dct_method = JDCT_IFAST;
//			} else if (keymatch(argv[argn], "float", 2)) {
//				cinfo->dct_method = JDCT_FLOAT;
char	szDither[] = "dither";
			/* Select dithering algorithm. */
char	szDithfs[] = "fs";
//				cinfo->dither_mode = JDITHER_FS;
char	szDithNone[] = "none";
//				cinfo->dither_mode = JDITHER_NONE;
char	szDithOrd[] = "ordered";	// 2
//				cinfo->dither_mode = JDITHER_ORDERED;
char	szFast[] = "fast";
			/* Select recommended processing options for quick-and-dirty output. */
//			cinfo->two_pass_quantize = FALSE;
//			cinfo->dither_mode = JDITHER_ORDERED;
//			if (! cinfo->quantize_colors) /* don't override an earlier -colors */
//				cinfo->desired_number_of_colors = 216;
//			cinfo->dct_method = JDCT_FASTEST;
//			cinfo->do_fancy_upsampling = FALSE;
char	szGray[] = "grayscale";	// 2
char	szMono[] = "monochrome";
			/* Force monochrome output. */
//			cinfo->out_color_space = JCS_GRAYSCALE;
char	szNoSm[] = "nosmooth";	// 3
			/* Suppress fancy upsampling */
//			cinfo->do_fancy_upsampling = FALSE;
char	szOnePass[] = "onepass";	// 3
			/* Use fast one-pass quantization. */
//			cinfo->two_pass_quantize = FALSE;
char	szScale[] = "scale";	// 1
			/* Scale the output image by a fraction M/N. */
//			if (_gwscanf(argv[argn], "%d/%d", (LPSTR)&ii[0]) != 2 )
//				goto Exit_Parse;
//			cinfo->scale_num = ii[0];
//			cinfo->scale_denom = ii[1];


LOCAL( int )
usage ( LPSTR lpb, int iMax )
{
	int		i;

	i = 0;
	if( lpb )
	{
		*lpb = 0;	// Init to zero
//  wsprintf( ( lpb + i ), "usage: %s [switches] ", progname);
//#ifdef TWO_FILE_COMMANDLINE
//  wsprintf( ( lpb + i ), "inputfile outputfile\n");
//#else
//  wsprintf( ( lpb + i ), "[inputfile]\n");
//#endif

		if( BUFFOK )
  wsprintf( ( lpb + i ), "Switches (names may be abbreviated):" WEOL );

		// Colors in OUTPUT
		if( BUFFOK )
  wsprintf( ( lpb + i ), "  -%s N      Reduce image to no more than N colors. " WEOL, &szColors[0] );
		if( BUFFOK )
  wsprintf( ( lpb + i ), "  -%s          Fast, low-quality processing. " WEOL, &szFast[0] );
		if( BUFFOK )
  wsprintf( ( lpb + i ), "  -grayscale     Force grayscale output. " WEOL );
#ifdef IDCT_SCALING_SUPPORTED
		if( BUFFOK )
  wsprintf( ( lpb + i ), "  -scale M/N     Scale output image by fraction M/N, eg, 1/8. " WEOL );
#endif
		if( BUFFOK )
  wsprintf( ( lpb + i), "Switches for advanced users: " WEOL );
#ifdef DCT_ISLOW_SUPPORTED
		if( BUFFOK )
  wsprintf( ( lpb + i ), "  -dct int       Use integer DCT method%s." WEOL,
	  (JDCT_DEFAULT == JDCT_ISLOW ? " (default)" : ""));
#endif
#ifdef DCT_IFAST_SUPPORTED
		if( BUFFOK )
  wsprintf( ( lpb + i ), "  -dct fast      Use fast integer DCT (less accurate)%s " WEOL,
	  (JDCT_DEFAULT == JDCT_IFAST ? " (default)" : ""));
#endif
#ifdef DCT_FLOAT_SUPPORTED
		if( BUFFOK )
  wsprintf( ( lpb + i ), "  -dct float     Use floating-point DCT method%s " WEOL,
	  (JDCT_DEFAULT == JDCT_FLOAT ? " (default)" : ""));
#endif
		if( BUFFOK )
  wsprintf( ( lpb + i ), "  -dither fs     Use F-S dithering (default). " WEOL );
		if( BUFFOK )
  wsprintf( ( lpb + i ), "  -dither none   Don't use dithering in quantization. " WEOL );
		if( BUFFOK )
  wsprintf( ( lpb + i ), "  -dither ordered  Use ordered dither (medium speed, quality). " WEOL );
		if( BUFFOK )
//#ifdef QUANT_2PASS_SUPPORTED
//  wsprintf( ( lpb + i ), "  -map FILE      Map to colors used in named image file\n");
//#endif
		if( BUFFOK )
  wsprintf( ( lpb + i ), "  -nosmooth      Don't use high-quality upsampling. " WEOL );

#ifdef QUANT_1PASS_SUPPORTED
		if( BUFFOK )
  wsprintf( ( lpb + i ), "  -onepass       Use 1-pass quantization (fast, low quality). " WEOL );
#endif
//  wsprintf( ( lpb + i ), "  -maxmemory N   Maximum memory to use (in kbytes)\n");
//  wsprintf( ( lpb + i ), "  -outfile name  Specify name for output file\n");
//  wsprintf( ( lpb + i ), "  -verbose  or  -debug   Emit debug output\n");

  i = lstrlen( lpb );

	}
	return i;
}


LOCAL(int)
parse_switches (j_decompress_ptr cinfo, int argc, char MLPTR MLPTR argv,
		int last_file_arg_seen, boolean for_real)
/* Parse optional switches.
 * Returns argv[] index of first file-name argument (== argc if none).
 * Any file names with indexes <= last_file_arg_seen are ignored;
 * they have presumably been processed in a previous iteration.
 * (Pass 0 for last_file_arg_seen on the first or only iteration.)
 * for_real is FALSE on the first (dummy) pass; we may skip any expensive
 * processing.
 */
{
	int argn;
	char MLPTR arg;
	char	c;
	/* Set up default JPEG parameters. */
	requested_fmt = DEFAULT_FMT;	/* set default output file format */
	//  outfilename = NULL;

	cinfo->err->trace_level = 0;

	/* Scan command line options, adjust parameters */
	for( argn = 1; argn < argc; argn++ )
	{
		arg = argv[argn];
		c = *arg;
		if( ( c == '-' ) || ( c == '/' ) )
		{
			arg++;	// Advance
			c = toupper( *arg );
		}
		else
		{
			/* Not a switch, DIE */
			goto Exit_Parse;	/* ERRANT done parsing switches */
		}

		if (keymatch(arg, "colors", 1) || keymatch(arg, "colours", 1) ||
	       keymatch(arg, "quantize", 1) || keymatch(arg, "quantise", 1)) {

			/* Do color quantization. */
			int val;
			val = 0;
			if( ++argn >= argc )	/* advance to next argument */
				goto Exit_Parse;
			if( _gwscanf(argv[argn], "%d", (LPSTR)&val) != 1 )
				goto Exit_Parse;
			cinfo->desired_number_of_colors = val;
			cinfo->quantize_colors = TRUE;

		} else if (keymatch(arg, "dct", 2)) {

			/* Select IDCT algorithm. */
			if (++argn >= argc)	/* advance to next argument */
				goto Exit_Parse;
			if (keymatch(argv[argn], "int", 1)) {
				cinfo->dct_method = JDCT_ISLOW;
			} else if (keymatch(argv[argn], "fast", 2)) {
				cinfo->dct_method = JDCT_IFAST;
			} else if (keymatch(argv[argn], "float", 2)) {
				cinfo->dct_method = JDCT_FLOAT;
			} else
				goto Exit_Parse;

		} else if (keymatch(arg, "dither", 2)) {

			/* Select dithering algorithm. */
			if (++argn >= argc)	/* advance to next argument */
				goto Exit_Parse;
			if (keymatch(argv[argn], "fs", 2)) {
				cinfo->dither_mode = JDITHER_FS;
			} else if (keymatch(argv[argn], "none", 2)) {
				cinfo->dither_mode = JDITHER_NONE;
			} else if (keymatch(argv[argn], "ordered", 2)) {
				cinfo->dither_mode = JDITHER_ORDERED;
			} else
				goto Exit_Parse;

		} else if (keymatch(arg, "fast", 1)) {

			/* Select recommended processing options for quick-and-dirty output. */
			cinfo->two_pass_quantize = FALSE;
			cinfo->dither_mode = JDITHER_ORDERED;
			if (! cinfo->quantize_colors) /* don't override an earlier -colors */
				cinfo->desired_number_of_colors = 216;
			cinfo->dct_method = JDCT_FASTEST;
			cinfo->do_fancy_upsampling = FALSE;

		} else if (keymatch(arg, "grayscale", 2) || keymatch(arg, "greyscale",2)) {

			/* Force monochrome output. */
			cinfo->out_color_space = JCS_GRAYSCALE;

		} else if (keymatch(arg, "nosmooth", 3)) {

			/* Suppress fancy upsampling */
			cinfo->do_fancy_upsampling = FALSE;

		} else if (keymatch(arg, "onepass", 3)) {

			/* Use fast one-pass quantization. */
			cinfo->two_pass_quantize = FALSE;

//		} else if (keymatch(arg, "os2", 3)) {
//
//			/* BMP output format (OS/2 flavor). */
//			requested_fmt = FMT_OS2;
//
//		} else if (keymatch(arg, "pnm", 1) || keymatch(arg, "ppm", 1)) {
//
//			/* PPM/PGM o`utput format. */
//			requested_fmt = FMT_PPM;
//
//		} else if (keymatch(arg, "rle", 1)) {
//
//			/* RLE output format. */
//			requested_fmt = FMT_RLE;

		} else if (keymatch(arg, "scale", 1)) {

			int		ii[4];
			/* Scale the output image by a fraction M/N. */
			if (++argn >= argc)	/* advance to next argument */
				goto Exit_Parse;
			if (_gwscanf(argv[argn], "%d/%d", (LPSTR)&ii[0]) != 2 )
				goto Exit_Parse;

			cinfo->scale_num = ii[0];
			cinfo->scale_denom = ii[1];

//		} else if (keymatch(arg, "targa", 1)) {
//
//			/* Targa output format. */
//			requested_fmt = FMT_TARGA;
//
		} else {
			goto Exit_Parse;			/* bogus switch */
		}
	}	// For data list
	argn = 0;	// ALL Done
	goto End_Parse;
Exit_Parse:
	argn = 1;
End_Parse:
  return argn;			/* return index of next arg (file name) */
}

// ========================================================
//	CONFIGURATION of WDJPEG Library
//
// As extracted from JDApiMin.c
//switch (cinfo->num_components) {
//  case 1:
// -   cinfo->jpeg_color_space = JCS_GRAYSCALE;
// -   cinfo->out_color_space = JCS_GRAYSCALE;
//    break;
//case 3:
//    if (cinfo->saw_JFIF_marker) {
// -     cinfo->jpeg_color_space = JCS_YCbCr; /* JFIF implies YCbCr */
//    } else if (cinfo->saw_Adobe_marker) {
// -     switch (cinfo->Adobe_transform) {
//      case 0:
// -	cinfo->jpeg_color_space = JCS_RGB;
//	break;
//      case 1:
// -	cinfo->jpeg_color_space = JCS_YCbCr;
//	break;
//      default:
//	WARNMS1(cinfo, JWRN_ADOBE_XFORM, cinfo->Adobe_transform);
// -	cinfo->jpeg_color_space = JCS_YCbCr; /* assume it's YCbCr */
//	break;
//      }
//    } else {
//      /* Saw no special markers, try to guess from the component IDs */
// -     int cid0 = cinfo->comp_info[0].component_id;
// -     int cid1 = cinfo->comp_info[1].component_id;
// -     int cid2 = cinfo->comp_info[2].component_id;
//if (cid0 == 1 && cid1 == 2 && cid2 == 3)
// -	cinfo->jpeg_color_space = JCS_YCbCr; /* assume JFIF w/out marker */
//      else if (cid0 == 82 && cid1 == 71 && cid2 == 66)
// -	cinfo->jpeg_color_space = JCS_RGB; /* ASCII 'R', 'G', 'B' */
//      else {
//	TRACEMS3(cinfo, 1, JTRC_UNKNOWN_IDS, cid0, cid1, cid2);
// -	cinfo->jpeg_color_space = JCS_YCbCr; /* assume it's YCbCr */
//      }
//    }
//    /* Always guess RGB is proper output colorspace. */
// -    cinfo->out_color_space = JCS_RGB;
//    break;
//case 4:
// -   if (cinfo->saw_Adobe_marker) {
//      switch (cinfo->Adobe_transform) {
//      case 0:
// -	cinfo->jpeg_color_space = JCS_CMYK;
//	break;
//      case 2:
// -	cinfo->jpeg_color_space = JCS_YCCK;
//	break;
//      default:
//	WARNMS1(cinfo, JWRN_ADOBE_XFORM, cinfo->Adobe_transform);
// -	cinfo->jpeg_color_space = JCS_YCCK; /* assume it's YCCK */
//	break;
//      }
//    } else {
//      /* No special markers, assume straight CMYK. */
// -     cinfo->jpeg_color_space = JCS_CMYK;
//    }
// -   cinfo->out_color_space = JCS_CMYK;
//    break;
//default:
// -   cinfo->jpeg_color_space = JCS_UNKNOWN;
// -   cinfo->out_color_space = JCS_UNKNOWN;
//    break;
//  }
///* Set defaults for other decompression parameters. */
//  cinfo->scale_num = 1;		/* 1:1 scaling */
//  cinfo->scale_denom = 1;
//  cinfo->output_gamma = 1.0;
//  cinfo->buffered_image = FALSE;
//  cinfo->raw_data_out = FALSE;
//==  cinfo->dct_method = JDCT_DEFAULT;
//==  cinfo->do_fancy_upsampling = TRUE;
//==  cinfo->do_block_smoothing = TRUE;
//==  cinfo->quantize_colors = FALSE;
//  /* We set these in case application only sets quantize_colors. */
//==  cinfo->dither_mode = JDITHER_FS;
//==  cinfo->two_pass_quantize = TRUE;
//==  cinfo->desired_number_of_colors = 256;
// - cinfo->colormap = NULL;
//  /* Initialize for no mode change in buffered-image mode. */
// - cinfo->enable_1pass_quant = FALSE;
// - cinfo->enable_external_quant = FALSE;
// - cinfo->enable_2pass_quant = FALSE;
//}

//  A config structure
//typedef	struct	tagJCOMCFG {
//	long	jc_size;	// = sizeof( JCOMCFG )
//	int		jc_desired_number_of_colors;	//  256;
//	BOOL	jc_quantize_colors;	// = FALSE;
//	int		jc_dct_method;	// = JDCT_IFAST;
//	int		jc_dither_mode;	// JDITHER_FS;
//	int		jc_fast;		// 0
//	// Select recommended processing options for quick-and-dirty output
//	BOOL	jc_two_pass_quantize;	// = TRUE;
//	BOOL	jc_do_fancy_upsampling;	// = TRUE;
//	BOOL	jc_do_block_smoothing;	// = TRUE;
//	int		jc_scale_num;			// = 1;		/* 1:1 scaling */
//	int		jc_scale_denom;			// = 1;
//}JCOMCFG;
//
//typedef JCOMCFG MLPTR LPJCOMCFG;
//
//#define		DEF_J2_CFG	\
//	(int)256, \
//	FALSE,	\
//	JDCT_ISLOW, \
//	JDITHER_FS, \
//	(int)0, \
//	TRUE, \
//	TRUE, \
//	TRUE, \
//	(int)1, \
//	(int)1

JCOMCFG	DefJComCfg = { 
	DEF_J2_CFG
};

void	SetFastDecomp( j_decompress_ptr cinfo, LPJCOMCFG ci  )
{
	cinfo->two_pass_quantize = FALSE;
	cinfo->dither_mode = JDITHER_ORDERED;
	// don't override an earlier -colors
	if( !cinfo->quantize_colors && !ci->jc_quantize_colors )
		cinfo->desired_number_of_colors = 216;
	cinfo->dct_method = JDCT_FASTEST;
	cinfo->do_fancy_upsampling = FALSE;
}

LOCAL(void)
SetConfig6Def( LPJCOMCFG cinfo )
{
	cinfo->jc_size = sizeof( JCOMCFG );
	cinfo->jc_desired_number_of_colors = 256;
	cinfo->jc_quantize_colors = FALSE;
	cinfo->jc_dct_method = JDCT_IFAST;
	cinfo->jc_dither_mode = JDITHER_FS;

	cinfo->jc_two_pass_quantize = TRUE;
	cinfo->jc_do_fancy_upsampling = TRUE;
	cinfo->jc_do_block_smoothing = TRUE;

	cinfo->jc_scale_num = 1;		// 1:1 scaling
	cinfo->jc_scale_denom = 1;	// 1

	cinfo->jc_fast = 0;
}

LOCAL(int)
Config6Copy( j_decompress_ptr cinfo, LPJCOMCFG ci )
{
	if( ci->jc_size != sizeof( JCOMCFG ) )
		return 0;

	cinfo->desired_number_of_colors = ci->jc_desired_number_of_colors;
	cinfo->quantize_colors = ci->jc_quantize_colors;
	cinfo->dct_method = ci->jc_dct_method;	// JDCT_IFAST;
	cinfo->dither_mode = ci->jc_dither_mode; // JDITHER_FS;

	cinfo->two_pass_quantize = ci->jc_two_pass_quantize;	// = TRUE;
	cinfo->do_fancy_upsampling = ci->jc_do_fancy_upsampling;// = TRUE;
	cinfo->do_block_smoothing = ci->jc_do_block_smoothing;	// = TRUE;

	cinfo->scale_num = ci->jc_scale_num;		// 1:1 scaling
	cinfo->scale_denom = ci->jc_scale_denom;	// 1

// else if (keymatch(arg, "fast", 1))
	if( ci->jc_fast == 1 )		//
		SetFastDecomp( cinfo, ci );

	return 1;
}

GLOBAL(int)
CopyConfig6( j_decompress_ptr cinfo )
{
	LPJCOMCFG ci;
	ci = &DefJComCfg;
	ci->jc_size = sizeof( JCOMCFG );	// Set HEADER
	return( Config6Copy( cinfo, ci ) );
}

EXPORT32
int MLIBCALL
WGETCONFIG6( LPJCOMCFG cinfo )
{
	LPJCOMCFG	ci;
	if( cinfo->jc_size != sizeof( JCOMCFG ) )
		return 0;
	ci = &DefJComCfg;
	ci->jc_size = sizeof( JCOMCFG );	// Set HEADER

	cinfo->jc_desired_number_of_colors = ci->jc_desired_number_of_colors;
	cinfo->jc_quantize_colors = ci->jc_quantize_colors;
	cinfo->jc_dct_method = ci->jc_dct_method;	// JDCT_IFAST;
	cinfo->jc_dither_mode = ci->jc_dither_mode; // JDITHER_FS;

	cinfo->jc_two_pass_quantize = ci->jc_two_pass_quantize;	// = TRUE;
	cinfo->jc_do_fancy_upsampling = ci->jc_do_fancy_upsampling;// = TRUE;
	cinfo->jc_do_block_smoothing = ci->jc_do_block_smoothing;	// = TRUE;

	cinfo->jc_scale_num = ci->jc_scale_num;		// 1:1 scaling
	cinfo->jc_scale_denom = ci->jc_scale_denom;	// 1

	cinfo->jc_fast = ci->jc_fast;

	return 1;
}

EXPORT32
int MLIBCALL
WSETCONFIG6( LPJCOMCFG ci )
{
	int		i;
	LPJCOMCFG	cinfo;

	cinfo = &DefJComCfg;
	cinfo->jc_size = sizeof( JCOMCFG );	// Set HEADER
	i = 0;
	if( ci->jc_size != cinfo->jc_size )
		return( (int)-1 );	// Show displeasure

	if( cinfo->jc_desired_number_of_colors != ci->jc_desired_number_of_colors )
	{
		i++;
		cinfo->jc_desired_number_of_colors = ci->jc_desired_number_of_colors;
	}
	if( cinfo->jc_quantize_colors != ci->jc_quantize_colors )
	{
		i++;
		cinfo->jc_quantize_colors = ci->jc_quantize_colors;
	}
	if( cinfo->jc_dct_method != ci->jc_dct_method )	// JDCT_IFAST;
	{
		i++;
		cinfo->jc_dct_method = ci->jc_dct_method;	// JDCT_IFAST;
	}
	if( cinfo->jc_dither_mode != ci->jc_dither_mode )
	{
		i++; // JDITHER_FS;
		cinfo->jc_dither_mode = ci->jc_dither_mode; // JDITHER_FS;
	}
	if( cinfo->jc_two_pass_quantize != ci->jc_two_pass_quantize )
	{
		i++;	// = TRUE;
		cinfo->jc_two_pass_quantize = ci->jc_two_pass_quantize;	// = TRUE;
	}
	if( cinfo->jc_do_fancy_upsampling != ci->jc_do_fancy_upsampling )
	{
		i++;// = TRUE;
		cinfo->jc_do_fancy_upsampling = ci->jc_do_fancy_upsampling;// = TRUE;
	}
	if( cinfo->jc_do_block_smoothing != ci->jc_do_block_smoothing )
	{
		i++;	// = TRUE;
		cinfo->jc_do_block_smoothing = ci->jc_do_block_smoothing;	// = TRUE;
	}

	if( cinfo->jc_scale_num != ci->jc_scale_num )
	{
		i++;		// 1:1 scaling
		cinfo->jc_scale_num = ci->jc_scale_num;		// 1:1 scaling
	}
	if( cinfo->jc_scale_denom != ci->jc_scale_denom )
	{
		i++;	// 1
		cinfo->jc_scale_denom = ci->jc_scale_denom;	// 1
	}

	if( cinfo->jc_fast = ci->jc_fast )
	{
		i++;	// NO WAY JOSE!!!
		cinfo->jc_fast = ci->jc_fast;
	}

	return i;
}

// eof - WComCfg.c
