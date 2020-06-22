
//
// wcmainD.c
//
// NOTE: This module contains a Version String of the Form
//	*<Name>*<Version>*<Date>*
//
// Copyright (C) 1991, 1992, 1993, Thomas G. Lane.
// This file is part of the Independent JPEG Group's software.
// For conditions of distribution and use, see the accompanying README file.
//
// This file contains a command-line user interface for the JPEG compressor.
// It should work on any system with Unix- or MS-DOS-style command lines.
//
// Two different command line styles are permitted, depending on the
// compile-time switch TWO_FILE_COMMANDLINE:
//	cjpeg [options]  inputfile outputfile
//	cjpeg [options]  [inputfile]
// In the second style, output is always to standard output, which you'd
// normally redirect to a file or pipe to some other program.  Input is
// either from a named file or from standard input (typically redirected).
// The second style is convenient on Unix but is unhelpful on systems that
// don't support pipes.  Also, you MUST use the first style if your system
// doesn't do binary I/O to stdin/stdout.
// To simplify script writing, the "-outfile" switch is provided.  The syntax
//	cjpeg [options]  -outfile outputfile  inputfile
// works regardless of which command line style is used.
// 

#include "winclude.h"

#ifdef	_INC_WINDOWS
// =======================================

extern	void jmem_term( void );
#include "wcommon.h"
extern	LIBCONFIG	LibConfig;	// Global Configuration PAD
extern	BOOL	IsDIB;

#ifdef	Dv16_App
#include <ctype.h>		// to declare isupper(), tolower()
#endif	// Dv16_App

// =======================================
#else		// !_INC_WINDOWS
// =======================================
#pragma message( "WARNING: This NON-Windows code NOT checked!" )
#ifdef INCLUDES_ARE_ANSI
#include <stdlib.h>		// to declare exit()
#endif
#include <ctype.h>		// to declare isupper(), tolower()
#ifdef NEED_SIGNAL_CATCHER
#include <signal.h>		// to declare signal()
#endif
#ifdef USE_SETMODE
#include <fcntl.h>		// to declare setmode()
#endif

#ifdef THINK_C
#include <console.h>		// command-line reader for Macintosh
#endif

// =======================================
#endif	// _INC_WINDOWS y/n

#ifdef DONT_USE_B_MODE		// define mode parameters for fopen()
#define READ_BINARY	"r"
#define WRITE_BINARY	"w"
#else
#define READ_BINARY	"rb"
#define WRITE_BINARY	"wb"
#endif

#ifndef EXIT_FAILURE		// define exit() codes if not provided
#define EXIT_FAILURE  1
#endif
#ifndef EXIT_SUCCESS
#ifdef VMS
#define EXIT_SUCCESS  1		// VMS is very nonstandard
#else
#define EXIT_SUCCESS  0
#endif
#endif

#ifdef BMP_SUPPORTED

// BIMTAP SUPPORT
// ==============
#ifdef	_INC_WINDOWS
// These are defined elsewhere ...
#else	/* !_INC_WINDOWS */
extern	FILE *in_open( char *filename, char *mode );
extern	FILE *out_open( char *filename, char *mode );
#endif	/* _INC_WINDOWS y/n */

#define	inpopen	in_open
#define	outopen	out_open

#else	// !BMP_SUPPORT
#define	inpopen	fopen
#define	outopen	fopen
#endif	// BMP_SUPPORT y/n

#include "wversion.h"		// for version message

#ifdef	PROTO
#pragma message( "Assumes COMPILE can correctly handle PROTOTYPING ... " )
#endif

#ifdef	_INC_WINDOWS

#define	_DLLLIB
#include	"wjpeglib.h"	// Module Services (as WINAPI headers)

#ifdef	exit
#undef	exit
#endif

#endif	// _INC_WINDOWS - Error Exit messages ...

// Version String (in data)
char	szDllVers[] = "*<"VER_DLL1">*<"VER_STRING">*<"VER_DATE">*";
//
// This routine determines what format the input file is,
// and selects the appropriate input-reading module.
//
// To determine which family of input formats the file belongs to,
// we may look only at the first byte of the file, since C does not
// guarantee that more than one character can be pushed back with ungetc.
// Looking at additional bytes would require one of these approaches:
//     1) assume we can fseek() the input file (fails for piped input);
//     2) assume we can push back more than one character (works in
//        some C implementations, but unportable);
//     3) provide our own buffering as is done in djpeg (breaks input readers
//        that want to use stdio directly, such as the RLE library);
// or  4) don't put back the data, and modify the input_init methods to assume
//        they start reading after the start of file (also breaks RLE library).
// #1 is attractive for MS-DOS but is untenable on Unix.
//
// The most portable solution for file types that can't be identified by their
// first byte is to make the user tell us what they are.  This is also the
// only approach for "raw" file types that contain only arbitrary values.
// We presently apply this method for Targa files.  Most of the time Targa
// files start with 0x00, so we recognize that case.  Potentially, however,
// a Targa file could start with any byte value (byte 0 is the length of the
// seldom-used ID field), so we provide a switch to force Targa input mode.
//

static boolean is_targa;	// records user -targa switch

#ifdef	_INC_WINDOWS

LPSTR	lpErrBuf;
extern int wsscanf( LPSTR, LPSTR, void MLPTR ); 

#define	MXSCN		10
WORD	AList[MXSCN];

#endif

GLOBAL void
select_file_type (compress_info_ptr cinfo)
{
	int c;

	if( is_targa )
	{
#ifdef TARGA_SUPPORTED
		jselrtarga(cinfo);
#else
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_TARSNC );	// "Targa support was not compiled"
#else
		ERREXIT(cinfo->emethods, "Targa support was not compiled");
#endif
#endif	// TARGA_SUPPORTED
		return;
	}

	if( (c = getc(cinfo->input_file)) == EOF )
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_EMPTY );
		return;
#else
		ERREXIT(cinfo->emethods, "Empty input file");
#endif
		return;
	}

	switch (c)
	{

#ifdef GIF_SUPPORTED
#pragma message( "GIF input supported." )
	case 'G':
		jselrgif(cinfo);
		break;
#endif

#ifdef PPM_SUPPORTED
#pragma message( "PPM input supported." )
	case 'P':
		jselrppm(cinfo);
		break;
#endif

#ifdef RLE_SUPPORTED
#pragma message( "RLE input supported." )
	case 'R':
		jselrrle(cinfo);
		break;
#endif

#ifdef BMP_SUPPORTED
#pragma message( "BMP input supported." )
	case 'B':
		WSELRBMP(cinfo);
		break;
#endif	// BMP_SUPPORT

#ifdef TARGA_SUPPORTED
#pragma message( "TARGA input supported." )
	case 0x00:
		jselrtarga(cinfo);
		break;
#endif

	default:
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_FORMAT );
#else

#ifdef TARGA_SUPPORTED
		ERREXIT(cinfo->emethods, "Unrecognized input file format --- perhaps you need -targa");
#else
		ERREXIT(cinfo->emethods, "Unrecognized input file format");
#endif

#endif
		break;
	}

	if( retgetc( c, cinfo->input_file ) == EOF)
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_UNGET );
#else
		ERREXIT(cinfo->emethods, "ungetc failed");
#endif
	}
}


//
// This routine gets control after the input file header has been read.
// It must determine what output JPEG file format is to be written,
// and make any other compression parameter changes that are desirable.
//

// If the input is gray scale, generate a monochrome JPEG file.
GLOBAL void
c_ui_method_selection (compress_info_ptr cinfo)
{
	if (cinfo->in_color_space == CS_GRAYSCALE)
		j_monochrome_default(cinfo);

	// For now, always select JFIF output format.
#ifdef JFIF_SUPPORTED
	jselwjfif(cinfo);
#else
	You shoulda defined JFIF_SUPPORTED.   // deliberate syntax error
#endif

}


//
// Signal catcher to ensure that temporary files are removed before aborting.
// NB: for Amiga Manx C this is actually a global routine named _abort();
// see -Dsignal_catcher=_abort in CFLAGS.  Talk about bogus...

#ifndef	_INC_WINDOWS	// No signal cather in WINDOWS!!!

#ifdef NEED_SIGNAL_CATCHER

static external_methods_ptr emethods; // for access to free_all

GLOBAL void
signal_catcher (int signum)
{
	if( emethods != NULL )
	{
		emethods->trace_level = 0;	// turn off trace output
		(*emethods->free_all) ();	// clean up memory allocation & temp files
	}
	exit(EXIT_FAILURE);
}

#endif

#endif	// !_INC_WINDOWS

//
// Optional routine to display a percent-done figure on msgout ...
// This can be stderr or stdout in DOS.
// See jcdeflts.c for explanation of the information used.

#ifdef PROGRESS_REPORT

METHODDEF void
Cprogress_monitor (compress_info_ptr cinfo, long loopcounter, long looplimit)
{
	if( cinfo->total_passes > 1 )
	{
		fprintf(msgout, "\rPass %d/%d: %3d%% ",
			cinfo->completed_passes+1, cinfo->total_passes,
			(int) (loopcounter*100L/looplimit));
	}
	else
	{
		fprintf(msgout, "\r %3d%% ",
			(int) (loopcounter*100L/looplimit));
	}
	fpaint(msgout);
}

#endif


//
// Argument-parsing code.
// The switch parser is designed to be useful with DOS-style command line
// syntax, ie, intermixed switches and file names, where only the switches
// to the left of a given file name affect processing of that file.
// The main program in this file doesn't actually use this capability...
//

char MLPTR c_outfilename;	// for -outfile switch
static char MLPTR progname;	// Program Name if NOT Windows

#ifndef	_INC_WINDOWS


// complain about bad command line

// ==============================================================
LOCAL void
usage (void)
{
	fprintf(msgout, "usage: %s [switches] ", progname);
#ifdef TWO_FILE_COMMANDLINE
	fprintf(msgout, "inputfile outputfile\n");
#else
	fprintf(msgout, "[inputfile]\n");
#endif

  fprintf(msgout, "Switches (names may be abbreviated):\n");
  fprintf(msgout, "  -quality N     Compression quality (0..100; 5-95 is useful range)\n");
  fprintf(msgout, "  -grayscale     Create monochrome JPEG file\n");
#ifdef ENTROPY_OPT_SUPPORTED
  fprintf(msgout, "  -optimize      Optimize Huffman table (smaller file, but slow compression)\n");
#endif
#ifdef TARGA_SUPPORTED
  fprintf(msgout, "  -targa         Input file is Targa format (usually not needed)\n");
#endif
  fprintf(msgout, "Switches for advanced users:\n");
  fprintf(msgout, "  -restart N     Set restart interval in rows, or in blocks with B\n");
#ifdef INPUT_SMOOTHING_SUPPORTED
  fprintf(msgout, "  -smooth N      Smooth dithered input (N=1..100 is strength)\n");
#endif
  fprintf(msgout, "  -maxmemory N   Maximum memory to use (in kbytes)\n");
  fprintf(msgout, "  -verbose  or  -debug   Emit debug output\n");
  fprintf(msgout, "Switches for wizards:\n");
#ifdef C_ARITH_CODING_SUPPORTED
  fprintf(msgout, "  -arithmetic    Use arithmetic coding\n");
#endif
#ifdef C_MULTISCAN_FILES_SUPPORTED
  fprintf(msgout, "  -nointerleave  Create noninterleaved JPEG file\n");
#endif
  fprintf(msgout, "  -qtables file  Use quantization tables given in file\n");
  fprintf(msgout, "  -sample HxV[,...]  Set JPEG sampling factors\n");
  exit(EXIT_FAILURE);
}
// ==============================================================
#endif	// !_INC_WINDOWS

// Case-insensitive matching of (possibly abbreviated) keyword switches.
// keyword is the constant keyword (must be lower case already),
// minchars is length of minimum legal abbreviation.
LOCAL boolean
keymatch( char MLPTR arg, const char MLPTR keyword, int minchars)
{
	int ca, ck;
	int nmatched = 0;

	while( (ca = *arg++) != '\0' )
	{
		if( (ck = *keyword++) == '\0' )
			return FALSE;	//  arg longer than keyword, no good

		if( isupper(ca) )	// force arg to lcase (assume ck is already)
			ca = tolower(ca);

		if( ca != ck )
			return FALSE;	// no good

		nmatched++;		// count matched characters

	}

	// reached end of argument;
	// fail if it's too short for unique abbrev
	if( nmatched < minchars )
		return FALSE;

	return TRUE;	// A-OK

}


// Read next char, skipping over any comments (# to end of line)
// A comment/newline sequence is returned as a newline
LOCAL int
qt_getc( FILETYPE file)
{
	int ch;

	ch = getc(file);
	if( ch == '#' )
	{
		do
		{
			ch = getc(file);
		} while( (ch != '\n') && (ch != EOF) );
	}
	return ch;
}


// Read an unsigned decimal integer from a quantization-table file
// Swallows one trailing character after the integer
LOCAL long
read_qt_integer(FILETYPE file)
{
	int ch;
	long val;

	// Skip any leading whitespace, detect EOF
	do
	{
		ch = qt_getc(file);
		if( ch == EOF )
			return EOF;
	} while( isspace(ch) );

	if( !isdigit(ch) )
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_QFILE );
		return 0;
#else
		fprintf(msgout,
			"%s: bogus data in quantization file\n",
			progname );
		exit(EXIT_FAILURE);
#endif
	}

	val = ch - '0';
	while( ch = qt_getc(file), isdigit(ch) )
	{
		val *= 10;
		val += ch - '0';
	}
	return val;
}

QUANT_TBL table;	// Put this is DATA segment, not stack

// Read a set of quantization tables from the specified file.
// The file is plain ASCII text: decimal numbers with whitespace between.
// Comments preceded by '#' may be included in the file.
// There may be one to NUM_QUANT_TBLS tables in the file, each of 64 values.
// The tables are implicitly numbered 0,1,etc.

LOCAL void
read_quant_tables (compress_info_ptr cinfo,
				   char MLPTR filename,
				   int scale_factor)
{
	// ZIG[i] is the zigzag-order position
	// of the i'th element of a DCT block
	// read in natural order (left to right, top to bottom).
	static const short ZIG[DCTSIZE2] = {
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
    };
	FILETYPE fp;
	int tblno, i;
	long val;

	if( ((fp = outopen(filename, "r")) == 0) || (fp == HFILE_ERROR) )
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_QTABOPN );	// "ERROR: Unable to open quantizing table file! "
#else
		fprintf(msgout, "%s: can't open %s\n", progname, filename);
		exit(EXIT_FAILURE);
#endif
	}
	tblno = 0;

	while( (val = read_qt_integer(fp)) != EOF )
	{	// read 1st element of table
		if( tblno >= NUM_QUANT_TBLS )
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_QTABSIZ ); // "ERROR: Too many tables in qtable file! "
#else
			fprintf(msgout, "%s: too many tables in file %s\n", progname, filename);
			exit(EXIT_FAILURE);
#endif
		}
		table[0] = (QUANT_VAL) val;
		for( i = 1; i < DCTSIZE2; i++ )
		{
			if( (val = read_qt_integer(fp)) == EOF )
			{
#ifdef	_INC_WINDOWS
				ERRFLAG( BAD_QTABINC ); // "ERROR: Incomplete table in qtable file! "
#else
				fprintf(msgout, "%s: incomplete table in file %s\n", progname, filename);
				exit(EXIT_FAILURE);
#endif
			}
			table[ZIG[i]] = (QUANT_VAL) val;
		}
		j_add_quant_table( cinfo,
			tblno, 
			table, 
			scale_factor, 
			FALSE );
    	tblno++;
	}
	fclose(fp);
}


// Process a sample-factors parameter string, of the form
//     HxV[,HxV,...]

LOCAL void
set_sample_factors (compress_info_ptr cinfo, char MLPTR arg)
{
	int ci, val1, val2;
	char ch1, ch2;
#define MAX_COMPONENTS 4	// # of comp_info slots made by jcdeflts.c

	for( ci = 0; ci < MAX_COMPONENTS; ci++ )
	{
		if( *arg )
		{
			ch2 = ',';	// if not set by sscanf, will be ','
#ifdef	_INC_WINDOWS
			if( wsscanf(arg, "%d%c%d%c", &AList ) < 3 )
			{
				ERRFLAG( BAD_FACTOR );
				break;
			}
			val1 = AList[0];
			ch1  = (char) AList[1];
			val2 = AList[2];
			if( AList[3] )
				ch2  = (char) AList[3];
#else
      	if (sscanf(arg, "%d%c%d%c", &val1, &ch1, &val2, &ch2) < 3)
				usage();
#endif
      	if( (ch1 != 'x' && ch1 != 'X') || ch2 != ',' )
			{
#ifdef	_INC_WINDOWS
				ERRFLAG( BAD_SYNT );
				break;
#else
				usage();		// syntax check
#endif
			}
	      if (val1 <= 0 || val1 > 4 || val2 <= 0 || val2 > 4)
			{
#ifdef	_INC_WINDOWS
				ERRFLAG( BAD_FACTOR2 );
				break;
#else
				fprintf(msgout, "JPEG sampling factors must be 1..4\n");
				exit(EXIT_FAILURE);
#endif
			}
			cinfo->comp_info[ci].h_samp_factor = val1;
			cinfo->comp_info[ci].v_samp_factor = val2;
			while( *arg && ( *arg++ != ',') ) // adv to nxt seg of arg string
				{};
		}
		else
		{
			// reached end of parameter, set remaining components to 1x1 sampling
			cinfo->comp_info[ci].h_samp_factor = 1;
			cinfo->comp_info[ci].v_samp_factor = 1;
		}
	} // For loop
}

// Initialize cinfo with default switch settings, then parse option switches.
// Returns argv[] index of first file-name argument (== argc if none).
// Any file names with indexes <= last_file_arg_seen are ignored;
// they have presumably been processed in a previous iteration.
// (Pass 0 for last_file_arg_seen on the first or only iteration.)

GLOBAL int
c_parse_switches (compress_info_ptr cinfo, int last_file_arg_seen,
		int argc, char MLPTR MLPTR argv)
{
	int argn;
	char MLPTR arg;
	char MLPTR qtablefile = NULL;	// saves -qtables filename if any
	int q_scale_factor = 100;	// default to no scaling for -qtables

	// (Re-)initialize the system-dependent error and memory managers.
	jselerror(cinfo->emethods);	// error/trace message routines
	jselmemmgr(cinfo->emethods);// memory allocation routines
	cinfo->methods->c_ui_method_selection = c_ui_method_selection;

	// Now OK to enable signal catcher.
#ifdef NEED_SIGNAL_CATCHER
	emethods = cinfo->emethods;
#endif

	// Set up default JPEG parameters.
	// Note that default -quality level here need not, and does not
	// match the default scaling for an explicit -qtables argument.
	
#ifdef	_INC_WINDOWS
	j_c_defaults( cinfo, LibConfig.ccf_out_quality, FALSE );
#else
	j_c_defaults(cinfo, 75, FALSE); // default quality level = 75
#endif
  is_targa = FALSE;
  c_outfilename = NULL;

#ifndef	_INC_WINDOWS
	argn = 0;
	arg = argv[argn];
	lpErrBuf = arg;
	argn++;	// Past message return buffer ...
#endif

// At present NO switches passed per this command line ... 
// ========================================================
  // Scan command line options, adjust parameters

	for( argn = 1; argn < argc; argn++ )
	{
		arg = argv[argn];
		if( *arg != '-' )
		{
			// Not a switch, must be a file name argument
			if( argn <= last_file_arg_seen )
			{
				c_outfilename = NULL;	// -outfile applies to just one input file
				continue;		// ignore this name if previously processed
			}
			break;			// else done parsing switches
		}
		arg++;			// advance past switch marker character

		if( keymatch(arg, "arithmetic", 1) )
		{
			// Use arithmetic coding.
#ifdef C_ARITH_CODING_SUPPORTED
			cinfo->arith_code = TRUE;
#else
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_ARITH );
#else
			fprintf(msgout, "%s: sorry, arithmetic coding not supported\n",
				progname);
			exit(EXIT_FAILURE);
#endif
#endif

		}
#ifndef	_INC_WINDOWS
		else if( keymatch(arg, "debug", 1) || keymatch(arg, "verbose", 1))
		{
			// Enable debug printouts.
			// On first -d, print version identification
			if( last_file_arg_seen == 0 && cinfo->emethods->trace_level == 0 )
				fprintf(msgout, "Independent JPEG Group's CJPEG, version %s\n%s\n",
				JVERSION, JCOPYRIGHT);
			cinfo->emethods->trace_level++;

		}
#endif
		else if( keymatch(arg, "grayscale", 2) || keymatch(arg, "greyscale",2) )
		{
			// Force a monochrome JPEG file to be generated.
			j_monochrome_default(cinfo);

		}
#ifndef	_INC_WINDOWS	// Forget max mem in WINDOWS ...
		else if( keymatch(arg, "maxmemory", 1) )
		{
			// Maximum memory in Kb (or Mb with 'm')
			long lval;
			char ch = 'x';

			if( ++argn >= argc )	// advance to next argument
			{
				usage();
			}
			if( sscanf(argv[argn], "%ld%c", &lval, &ch) < 1 )
				usage();
			if (ch == 'm' || ch == 'M')
				lval *= 1000L;
			cinfo->emethods->max_memory_to_use = lval * 1000L;

		}
#endif
		else if( keymatch(arg, "nointerleave", 3) )
		{
			// Create noninterleaved file.
#ifdef C_MULTISCAN_FILES_SUPPORTED
#pragma message( "Multiscan files supported ..." )
			cinfo->interleave = FALSE;
#else
#ifdef	_INC_WINDOWS
#pragma message( "Multiscan files NOT supported ..." )
			ERRFLAG( BAD_JPGMSS );
#else
			fprintf(msgout, "%s: sorry, multiple-scan support was not compiled\n",
				progname);
			exit(EXIT_FAILURE);
#endif
#endif

		}
		else if( keymatch(arg, "optimize", 1) || keymatch(arg, "optimise", 1))
		{
			// Enable entropy parm optimization.
#ifdef ENTROPY_OPT_SUPPORTED
#pragma message( "Optimize coding supported ..." )
			cinfo->optimize_coding = TRUE;
#else
#ifdef	_INC_WINDOWS
#pragma message( "Optimize coding NOT supported ..." )
			ERRFLAG( BAD_JPGEOS );	// sorry, entropy optimization was not compiled\n"
#else
      fprintf(msgout, "%s: sorry, entropy optimization was not compiled\n",
	      progname);
      exit(EXIT_FAILURE);
#endif
#endif

		}
#ifndef	_INC_WINDOWS
		else if (keymatch(arg, "outfile", 3))
		{
			// Set output file name.
			if (++argn >= argc)	// advance to next argument
				usage();
			c_outfilename = argv[argn];	// save it away for later use

		}
#endif
		else if (keymatch(arg, "quality", 1))
		{
			// Quality factor (quantization table scaling factor).
			int val;

#ifdef	_INC_WINDOWS
			if( ++argn >= argc )	// advance to next argument
				ERRFLAG( BAD_COMMAND );
			if( wsscanf( argv[argn], "%d", &AList) != 1 )
				ERRFLAG( BAD_COMMAND );
			val = AList[0];
#else
			if( ++argn >= argc )	// advance to next argument
				usage();
			if( sscanf(argv[argn], "%d", &val) != 1 )
				usage();
#endif

      // Set quantization tables (will be overridden if -qtables also given).
      // Note: we make force_baseline FALSE.
      // This means non-baseline JPEG files can be created with low Q values.
      // To ensure only baseline files are generated, pass TRUE instead.

			j_set_quality(cinfo, val, FALSE);

			// Change scale factor in case -qtables is present.
			q_scale_factor = j_quality_scaling(val);

		}
		else if( keymatch(arg, "qtables", 2) )
		{
			// Quantization tables fetched from file.
			if( ++argn >= argc )	// advance to next argument
			{
#ifdef	_INC_WINDOWS
				ERRFLAG( BAD_COMMAND );
#else
				usage();
#endif
			}
			qtablefile = argv[argn];
			// we postpone actually reading the file in case -quality comes later

		}
		else if( keymatch(arg, "restart", 1) )
		{
			// Restart interval in MCU rows (or in MCUs with 'b').
			long lval;
			char ch = 'x';
#ifdef	_INC_WINDOWS
			if( ++argn >= argc )	// advance to next argument
				ERRFLAG( BAD_COMMAND );
			if( wsscanf(argv[argn], "%ld%c", &AList) < 1 )
				ERRFLAG( BAD_COMMAND );
			lval = AList[0] + ( AList[1] << 16 );
			if( lval < 0 || lval > 65535L )
				ERRFLAG( BAD_COMMAND );
#else
			if( ++argn >= argc )	// advance to next argument
				usage();
			if( sscanf(argv[argn], "%ld%c", &lval, &ch) < 1 )
				usage();
			if( lval < 0 || lval > 65535L )
				usage();
#endif
			if( ch == 'b' || ch == 'B' )
				cinfo->restart_interval = (UINT16) lval;
			else
				cinfo->restart_in_rows = (int) lval;

		}
		else if( keymatch(arg, "sample", 2) )
		{
			// Set sampling factors.
			if( ++argn >= argc )	// advance to next argument
			{
#ifdef	_INC_WINDOWS
				ERRFLAG( BAD_COMMAND );
#else
				usage();
#endif
			}
			set_sample_factors(cinfo, argv[argn]);

		}
		else if( keymatch(arg, "smooth", 2) )
		{
			// Set input smoothing factor.
			int val;
#ifdef	_INC_WINDOWS
			if( ++argn >= argc )	// advance to next argument
				ERRFLAG( BAD_COMMAND );
			if( wsscanf(argv[argn], "%d", &AList) != 1 )
				ERRFLAG( BAD_COMMAND );
			val = AList[0];
			if( val < 0 || val > 100 )
				ERRFLAG( BAD_COMMAND );
#else
			if( ++argn >= argc )	// advance to next argument
				usage();
			if( sscanf(argv[argn], "%d", &val) != 1 )
				usage();
			if( val < 0 || val > 100 )
				usage();
#endif
			cinfo->smoothing_factor = val;

		}
		else if( keymatch(arg, "targa", 1) )
		{
			// Input file is Targa format.
			is_targa = TRUE;

		}
		else
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_COMMAND );
#else
			usage();			// bogus switch
#endif
		}
	}	// For argument list
  // Post-switch-scanning cleanup

	if( qtablefile != NULL )	// process -qtables if it was present
		read_quant_tables(cinfo, qtablefile, q_scale_factor);
// =========================================================

// parse_ret:
	return( argn );	// return index of next arg (file name)
}


//
// The main program.
//
struct External_methods_struct e_methods;
struct Compress_info_struct WCMcinfo;	// A DS variable
struct Compress_methods_struct c_methods;

#ifndef	DV32DLL

// DV32 uses its OWN headers ... See DVToJPG.c for these ...
// =====================================================

#ifdef	_INC_WINDOWS
extern	LPSTR	lpMBuf;
extern	DWORD	ddBufCnt;
// DWORD	ddOutCnt = 0;
extern	DWORD	ddMaxCnt;
extern	void set_file_size( DWORD );

#if		( _MSC_VER > 700 )
#if		(defined( NDEBUG ) && defined( Dv16_App ))
// **************************************************************
#pragma warning( disable : 4704 )
// **************************************************************
#endif	// NDEBUG & Dv16_App
#endif	// _MSC_VER

// WJPEG Library ENTRY
// ###################
EXPORT32
WORD MLIBCALL 
WBMPTOJPG( HGLOBAL hDib, DWORD ddSiz, HGLOBAL hInf, LPSTR lpOFile )
{
	LPSTR	lpInf;

	APIStart();
	winp_term();	// and all inputs cleared ...
	bErrFlag = 0;	// Global ERROR flag (if NON-ZERO) return ...
	msgout = &szMsgBuf[0];	// Setup a MESSAGE buffer ...
	// In a sense, OPEN the input file ...
	lpInf = 0;
	if( hDib && ( ddMaxCnt = ddSiz ) &&
		( lpMBuf = JGlobalLock( hDib ) ) )
	{
		ddBufCnt = ddSiz;
		set_file_size( ddSiz );	// Insert the FILE SIZE ...
	}
	else
	{
		bErrFlag = BAD_G2B_ENTRY;	// "ERROR: Input parameter to LIBRARY errant! "
			goto bmpjpg_ret;
	}	
	// Must have an output file name, and must be creatable ie valid handle
	if( (lpOFile == 0) || (lpOFile[0] == 0) ||
		( (WCMcinfo.output_file = outopen( lpOFile, WRITE_BINARY)) == 0 ) ||
		( WCMcinfo.output_file == HFILE_ERROR ) )
	{
		bErrFlag = BAD_OOPEN;
			goto bmpjpg_ret;
	}
#ifdef	WIN32
	_asm
	{
		mov		eax,offset BMPJPGRET
		mov		edx, esp
		push	edx
		push	eax
		call	setexitjump
		add		esp,2 * 4
	}
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
	// Set up links to method structures.
	WCMcinfo.methods = &c_methods;
	WCMcinfo.emethods = &e_methods;
	// These were done in c_parse_switches ...
	// (Re-)initialize the system-dependent error and memory managers.
	jselerror(WCMcinfo.emethods);	// error/trace message routines
	jselmemmgr(WCMcinfo.emethods);	// memory allocation routines
	WCMcinfo.methods->c_ui_method_selection = c_ui_method_selection;
	// Set up default JPEG parameters.
	j_c_defaults( &WCMcinfo, LibConfig.ccf_out_quality, FALSE );
  // NO Figuring here - Assumed it is a DIB input ONLY
	IsDIB = TRUE;	// And further assume it is a DIB only, NOT BMP File
	WSELRBMP( &WCMcinfo );
	if( bErrFlag )
		goto bmpjpg_ret;

  // Do it to it!
  jpeg_compress(&WCMcinfo);

  // All done.
#ifdef	WIN32

	_asm
	{
BMPJPGRET:
		mov	eax,[bErrFlag]	;	Get the ERROR FLAG ...
	}

#else	// !WIN32

	_asm
	{
BMPJPGRET:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}

#endif	// WIN32 y/n

bmpjpg_ret:
	jmem_term();	// Ensure ALL memory freed ...
	winp_term();	// and all inputs cleared ...
	if( hDib && lpMBuf )
	{
		JGlobalUnlock( hDib );	// Unlock the INPUT buffer ...
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
		JGlobalUnlock( hInf );	// Unlock the INFORMATION buffer ...
	}
	return( bErrFlag );
}	// WJPEG Library EXIT
// ###################


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
		bErrFlag = BAD_G2B_ENTRY;	/* "ERROR: Input parameter to LIBRARY errant! " */
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
	_asm 
	{
		mov  eax,offset DATJPGRET ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump ;
		add  esp,2 * 4 ;
	}
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
	_asm
	{
DATJPGRET:
		mov	eax,[bErrFlag]	;	Get the ERROR FLAG ...
	}
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


#endif	/* _INC_WINDOWS */

/* The original Write JPEG from the INPUT File data ... */
// WJPEG Library ENTRY
// ###################
EXPORT32
int MLIBCALL
WWRITEJPEG( int argc, char FAR * FAR *argv )
{
	int file_index;

#ifdef	_INC_WINDOWS
	LPSTR lpInf;

	APIStart();
	winp_term();	/* and all inputs cleared ... */
	bErrFlag = 0;	/* Global ERROR flag (if NON-ZERO) return ... */
	msgout = &szMsgBuf[0];	/* Setup a MESSAGE buffer ... */

#ifdef	WIN32
//	SETEXIT( BMAINRET );
	_asm 
	{
		mov  eax,offset BMAINRET ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump ;
		add  esp,2 * 4 ;
	}
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

#else	/* !_INC_WINDOWS */

  /* On Mac, fetch a command line. */
#ifdef THINK_C
  argc = ccommand(&argv);
#endif

  progname = argv[0];

#endif

  /* Set up links to method structures. */
  WCMcinfo.methods = &c_methods;
  WCMcinfo.emethods = &e_methods;

  /* Install, but don't yet enable signal catcher. */
#ifdef NEED_SIGNAL_CATCHER
  emethods = NULL;
  signal(SIGINT, signal_catcher);
#ifdef SIGTERM			/* not all systems have SIGTERM */
  signal(SIGTERM, signal_catcher);
#endif
#endif

  /* Scan command line: set up compression parameters, find file names. */

  file_index = c_parse_switches(&WCMcinfo, 0, argc, argv);

#ifdef TWO_FILE_COMMANDLINE
  /* Must have either -outfile switch or explicit output file name */
	if( c_outfilename == NULL )
	{
		if( file_index != argc - 2 )
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_FILENAME );
			goto mainret;
#else
			fprintf(msgout, "%s: must name one input and one output file\n",
				progname);
			usage();
#endif
		}
    c_outfilename = argv[file_index+1];
  } else {
		if( file_index != argc-1 ) 
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_FILENAME );
			goto mainret;
#else
			fprintf(msgout, "%s: must name one input and one output file\n",
			progname);
			usage();
#endif
		}
  }
#else
#ifndef	_INC_WINDOWS
  /* Unix style: expect zero or one file name */
  if (file_index < argc-1) {
    fprintf(msgout, "%s: only one input file\n", progname);
    usage();
  }
#endif	// !_INC_WINDOWS
#endif /* TWO_FILE_COMMANDLINE */

  /* Open the input file. */
	if( file_index < argc )
	{
		if( ((WCMcinfo.input_file = inpopen(argv[file_index], READ_BINARY)) == 0) ||
			( WCMcinfo.input_file == HFILE_ERROR ) )
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_FOPEN );
			goto mainret;
#else
			fprintf(msgout, "%s: can't open %s\n", progname, argv[file_index]);
			exit(EXIT_FAILURE);
#endif
		}
	}
	else
	{
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_FILENAME );
		goto mainret;
#else	/* !_INC_WINDOWS */
    /* default input file is stdin */
#ifdef USE_SETMODE		/* need to hack file mode? */
    setmode(fileno(stdin), O_BINARY);
#endif
#ifdef USE_FDOPEN		/* need to re-open in binary mode? */
    if ((WCMcinfo.input_file = fdopen(fileno(stdin), READ_BINARY)) == NULL) {
      fprintf(msgout, "%s: can't open stdin\n", progname);
      exit(EXIT_FAILURE);
    }
#else
    WCMcinfo.input_file = stdin;
#endif
#endif	/* _INC_WINDOWS y/n */
  }

  /* Open the output file. */
	if( c_outfilename != NULL ) 
	{
		if( ( (WCMcinfo.output_file = outopen(c_outfilename, WRITE_BINARY)) == 0 ) ||
			( WCMcinfo.output_file == HFILE_ERROR ) )
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_OOPEN );
			goto mainret;
#else
			fprintf(msgout, "%s: can't open %s\n", progname, c_outfilename);
			exit(EXIT_FAILURE);
#endif
    }
  } 
  else
  {
#ifdef	_INC_WINDOWS
		ERRFLAG( BAD_FILENAME );
		goto mainret;
#else	/* !_INC_WINDOWS */
    /* default output file is stdout */
#ifdef USE_SETMODE		/* need to hack file mode? */
    setmode(fileno(stdout), O_BINARY);
#endif
#ifdef USE_FDOPEN		/* need to re-open in binary mode? */
    if ((WCMcinfo.output_file = fdopen(fileno(stdout), WRITE_BINARY)) == NULL) {
      fprintf(msgout, "%s: can't open stdout\n", progname);
      exit(EXIT_FAILURE);
    }
#else
    WCMcinfo.output_file = stdout;
#endif
#endif	/* _INC_WINDOWS */
  }

  /* Figure out the input file format, and set up to read it. */
  select_file_type(&WCMcinfo);
#ifdef	_INC_WINDOWS
	if( bErrFlag )
		goto mainret;
#endif
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
#ifdef	_INC_WINDOWS

#ifdef	WIN32
	_asm
	{
BMAINRET:
		mov	eax,[bErrFlag]	;	Get the ERROR FLAG ...
	}
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

#else
  exit(EXIT_SUCCESS);
  return 0;			/* suppress no-return-value warnings */
#endif
}	// WJPEG Library EXIT
// ###################


#endif		// !DV32DLL

#ifndef	_INC_WINDOWS
// ================================
int	pgm_ret = 0;

int
main(argc,argv)
int argc;
char *argv[];
{
	

	return( pgm_ret );
}

#endif	// !_INC_WINDOWS

// eof wcmaind.c
