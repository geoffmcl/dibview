
//
// wdmainD.c
//
// Copyright (C) 1991, 1992, 1993, Thomas G. Lane.
// This file is part of the Independent JPEG Group's software.
// For conditions of distribution and use, see the accompanying README file.
//
// This file contains a command-line user interface for the JPEG decompressor.
// It should work on any system with Unix- or MS-DOS-style command lines.
//
// Two different command line styles are permitted, depending on the
// compile-time switch TWO_FILE_COMMANDLINE:
//	djpeg [options]  inputfile outputfile
//	djpeg [options]  [inputfile]
// In the second style, output is always to standard output, which you'd
// normally redirect to a file or pipe to some other program.  Input is
// either from a named file or from standard input (typically redirected).
// The second style is convenient on Unix but is unhelpful on systems that
// don't support pipes.  Also, you MUST use the first style if your system
// doesn't do binary I/O to stdin/stdout.
// To simplify script writing, the "-outfile" switch is provided.  The syntax
//	djpeg [options]  -outfile outputfile  inputfile
// works regardless of which command line style is used.
//

#include "winclude.h"

#ifdef	_INC_WINDOWS
// =============================================
#define	DLLLIB

#include	"wjpeglib.h"
extern	void jmem_term( void );
#include	"wcommon.h"

#ifdef	exit
#undef	exit
#endif

#ifdef	Dv16_App
#include <ctype.h>		// to declare isupper(), tolower()
#endif	// Dv16_App

// =============================================
#else	// !_INC_WINDOWS
// =============================================

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

// =============================================
#endif	// _INC_WINDOWS y/n


#ifdef TWO_FILE_COMMANDLINE
#ifdef DONT_USE_B_MODE		// define mode parameters for fopen()
#define READ_BINARY	"r"
#define WRITE_BINARY	"w"
#else
#define READ_BINARY	"rb"
#define WRITE_BINARY	"wb"
#endif
#endif	// TWO_FILE_COMMANDLINE

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

#include "wversion.h"		// for version message
//#include	"SetExit.h"

#ifdef	ADD_BMP
EXTERN short far jdos_open PP((short far * handle, char far * filename));
#endif	// ADD_BMP

#ifdef	BMP_SUPPORTED
#ifdef	WJPEG4
#ifdef	ADDOLDTO
extern	LPSTR	lpOutFile;	// GLOBAL pointer to OUTPUT FILE
extern	FILETYPE gout_open( char MLPTR );
extern	int gout_close( FILETYPE );
#endif	// ADDOLDTO
#else	// !WJPEG4
extern	LPSTR	lpOutFile;	// GLOBAL pointer to OUTPUT FILE
#endif	// WJPEG4 y/n
#endif	// BMP_SUPPORTED

//
// This list defines the known output image formats
// (not all of which need be supported by a given version).
// You can change the default output format by defining DEFAULT_FMT;
// indeed, you had better do so if you undefine PPM_SUPPORTED.
//

typedef enum {
	FMT_GIF,		// GIF format
	FMT_PPM,		// PPM/PGM (PBMPLUS formats)
	FMT_RLE,		// RLE format
	FMT_TARGA,		// Targa format
	FMT_TIFF,		// TIFF format
	FMT_BMP			// BMP format
} IMAGE_FORMATS;

#ifndef DEFAULT_FMT		// so can override from CFLAGS in Makefile
#ifdef	_INC_WINDOWS
#define DEFAULT_FMT	FMT_BMP	// We want the DEFAULT as a BITMAP
#else	// !_INC_WINDOWS
#define DEFAULT_FMT	FMT_PPM
#endif	// _INC_WINDOWS y/n
#endif	// !DEFAULT_FMT

static IMAGE_FORMATS requested_fmt;

//
// This routine gets control after the input file header has been read.
// It must determine what output file format is to be written,
// and make any other decompression parameter changes that are desirable.
//

// if grayscale or CMYK input, force similar output;
// else leave the output colorspace as set by options.
METHODDEF void
d_ui_method_selection( decompress_info_ptr cinfo )
{
	if (cinfo->jpeg_color_space == CS_GRAYSCALE)
		cinfo->out_color_space = CS_GRAYSCALE;
	else if (cinfo->jpeg_color_space == CS_CMYK)
		cinfo->out_color_space = CS_CMYK;

	// select output file format
	// Note: jselwxxx routine may make additional parameter changes,
	// such as forcing color quantization if it's a colormapped format.
	//
	switch( requested_fmt )
	{

#ifdef GIF_SUPPORTED
#pragma message( "GIF output supported." )
	case FMT_GIF:
		jselwgif(cinfo);
		break;
#endif	// GIF_SUPPORTED

#ifdef PPM_SUPPORTED
#pragma message( "PPM output supported." )
	case FMT_PPM:
		jselwppm(cinfo);
		break;
#endif	// PPM_SUPPORTED

#ifdef BMP_SUPPORTED
#pragma message( "BMP output supported." )
	case FMT_BMP:
		jselwbmp(cinfo);
		break;
#endif	// BMP_SUPPORTED

#ifdef RLE_SUPPORTED
#pragma message( "RLE output supported." )
	case FMT_RLE:
		jselwrle(cinfo);
		break;
#endif	// RLE_SUPPORTED

#ifdef TARGA_SUPPORTED
#pragma message( "TARGA output supported." )
	case FMT_TARGA:
		jselwtarga(cinfo);
		break;
#endif	// TARGA_SUPPORTED

	default:
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_NOOUT );	// "Unsupported output file format"
#else	// !_INC_WINDOWS
			ERREXIT(cinfo->emethods, "Unsupported output file format");
#endif	// _INC_WINDOWS y/n
		}
		break;
	}	// switch pre output file type
}


//
// Signal catcher to ensure that temporary files are removed before aborting.
// NB: for Amiga Manx C this is actually a global routine named _abort();
// see -Dsignal_catcher=_abort in CFLAGS.  Talk about bogus...
//
#ifndef	_INC_WINDOWS
// =========================================================
#ifdef NEED_SIGNAL_CATCHER

static external_methods_ptr emethods; // for access to free_all

GLOBAL void
signal_catcher (int signum)
{
	if( emethods != NULL )
	{
		emethods->trace_level = 0;	// turn off trace output
		if( emethods->free_all )
			(*emethods->free_all) ();	// clean up memory allocation & temp files
	}
	exit(EXIT_FAILURE);
}

#endif	// NEED_SIGNAL_CATHCER
#endif	// !_INC_WINDOWS

//
// Optional routine to display a percent-done figure on msgout.
// See jddeflts.c for explanation of the information used.
//

#ifdef PROGRESS_REPORT

GLOBAL void
Dprogress_monitor (decompress_info_ptr cinfo, long loopcounter, long looplimit)
{
	if( cinfo->total_passes > 1 )
	{
		fprintf(msgout, "\rPass %d/%d: %3d%% ",
			cinfo->completed_passes+1,
			cinfo->total_passes,
			(int) (loopcounter*100L/looplimit) );
	}
	else
	{
		fprintf(msgout, "\r %3d%% ",
			(int) (loopcounter*100L/looplimit));
	}
#ifdef	_INC_WINDOWS
	tpaint( msgout );
#else	// !_INC_WINDOWS
	fflush( msgout );
#endif	// INC_WINDOWS y/n

}

#endif	// PROGRESS_REPORT


//
// Argument-parsing code.
// The switch parser is designed to be useful with DOS-style command line
// syntax, ie, intermixed switches and file names, where only the switches
// to the left of a given file name affect processing of that file.
// The main program in this file doesn't actually use this capability...
//

static char MLPTR outfilename;	// for -outfile switch

#ifdef	_INC_WINDOWS
char	*pRUsage = "Usage: [switches] inputfile outputfile\r\n"
	"Switches (names may be abbreviated):\r\n"
	"  -colors N      Reduce image to no more than N colors\r\n"
#ifdef GIF_SUPPORTED
   "  -gif           Select GIF output format\r\n"
#endif
#ifdef PPM_SUPPORTED
   "  -ppm           Select PBMPLUS (PPM/PGM) output format\r\n"
#endif
#ifdef BMP_SUPPORTED
   "  -bmp           Select MS/IBM Windows Bitmap format (default)\r\n"
#endif
   "  -quantize N    Same as -colors N\r\n"
#ifdef RLE_SUPPORTED
   "  -rle           Select Utah RLE output format\r\n"
#endif
#ifdef TARGA_SUPPORTED
   "  -targa         Select Targa output format\r\n"
#endif
	"Switches for advanced users:\r\n"
#ifdef BLOCK_SMOOTHING_SUPPORTED
	"  -blocksmooth   Apply cross-block smoothing\r\n"
#endif
	"  -grayscale     Force grayscale output\r\n"
	"  -nodither      Don't use dithering in quantization\r\n"
#ifdef QUANT_1PASS_SUPPORTED
	"  -onepass       Use 1-pass quantization (fast, low quality)\r\n"
#endif
	"\r\n";	// Usage string

UINT WINAPI
DECOMPUSE( LPSTR buf, int len )
{
UINT	i, j, k;
LPSTR	lps;
	lps = (LPSTR) pRUsage;
	j = 0;
	if( buf && len && (i = lstrlen( lps) ) )
	{
		k = len - 1;
		for( j = 0; j < i; j++ )
		{
			if( j >= k )
				break;
			buf[j] = lps[j];	// Move as much as the buffer allows
		}
		if( j )
			buf[j] = 0;
	}
return( j );
}

#else	// !_INC_WINDOWS

static char MLPTR progname;	// program name for error messages

// complain about bad command line
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
  fprintf(msgout, "  -colors N      Reduce image to no more than N colors\n");
#ifdef GIF_SUPPORTED
  fprintf(msgout, "  -gif           Select GIF output format\n");
#endif
#ifdef PPM_SUPPORTED
  fprintf(msgout, "  -pnm           Select PBMPLUS (PPM/PGM) output format (default)\n");
#endif
#ifdef BMP_SUPPORTED
  fprintf(msgout, "  -bmp           Select MS/IBM Windows Bitmap format\n");
#endif
  fprintf(msgout, "  -quantize N    Same as -colors N\n");
#ifdef RLE_SUPPORTED
  fprintf(msgout, "  -rle           Select Utah RLE output format\n");
#endif
#ifdef TARGA_SUPPORTED
  fprintf(msgout, "  -targa         Select Targa output format\n");
#endif
  fprintf(msgout, "Switches for advanced users:\n");
#ifdef BLOCK_SMOOTHING_SUPPORTED
  fprintf(msgout, "  -blocksmooth   Apply cross-block smoothing\n");
#endif
  fprintf(msgout, "  -grayscale     Force grayscale output\n");
  fprintf(msgout, "  -nodither      Don't use dithering in quantization\n");
#ifdef QUANT_1PASS_SUPPORTED
  fprintf(msgout, "  -onepass       Use 1-pass quantization (fast, low quality)\n");
#endif
  fprintf(msgout, "  -maxmemory N   Maximum memory to use (in kbytes)\n");
  fprintf(msgout, "  -verbose  or  -debug   Emit debug output\n");
  exit(EXIT_FAILURE);
}

#endif	// !_INC_WINDOWS

// Case-insensitive matching of (possibly abbreviated) keyword switches.
// keyword is the constant keyword (must be lower case already),
// minchars is length of minimum legal abbreviation.
LOCAL boolean
keymatch (char MLPTR arg, const char MLPTR keyword, int minchars)
{
	register int ca, ck;
	register int nmatched = 0;

	while( (ca = *arg++ ) != '\0' )
	{
		if( (ck = *keyword++) == '\0' )
			return FALSE;	// arg longer than keyword, no good.

		if( isupper(ca) )	// force arg to lcase (assume ck is already)
			ca = tolower(ca);

		if (ca != ck)
			return FALSE;	// no good.

		nmatched++;		// count matched characters
	}
	
	// reached end of argument; fail if it's too short for unique abbrev
	if( nmatched < minchars )
		return FALSE;

	return( TRUE );	// A-OK
}

int DSval;	// DS Values ...
long DSlval;
char DSch;

#ifdef	_INC_WINDOWS
int wsscanf( LPSTR, LPSTR, void MLPTR ); 
#define	MXSCN		5
WORD	ScanList[MXSCN];
#endif	// _INC_WINDOWS

// Initialize cinfo with default switch settings, then parse option switches.
// Returns argv[] index of first file-name argument (== argc if none).
// Any file names with indexes <= last_file_arg_seen are ignored;
// they have presumably been processed in a previous iteration.
// (Pass 0 for last_file_arg_seen on the first or only iteration.)
//
LOCAL int
parse_switches (decompress_info_ptr cinfo, int last_file_arg_seen,
		int argc, char MLPTR MLPTR argv)
{
	int argn;
	char MLPTR arg;

	// (Re-)initialize the system-dependent error and memory managers.
	jselerror(cinfo->emethods);	// error/trace message routines
	jselmemmgr(cinfo->emethods);	// memory allocation routines
	cinfo->methods->d_ui_method_selection = d_ui_method_selection;

	// Now OK to enable signal catcher
#ifdef NEED_SIGNAL_CATCHER
	emethods = cinfo->emethods;
#endif

	// Set up default JPEG parameters.
	j_d_defaults(cinfo, TRUE);
	requested_fmt = DEFAULT_FMT;	// set default output file format
	outfilename = NULL;

	argn = 0;
	arg = argv[argn];
	argn = 1;

// =====================================================
	// Scan command line options, adjust parameters
	for( argn = 1; argn < argc; argn++ )
	{
		arg = argv[argn];
		if( *arg != '-' )
		{
			// Not a switch, must be a file name argument
			if( argn <= last_file_arg_seen )
			{
				outfilename = NULL;	// -outfile applies to just one input file
				continue;		// ignore this name if previously processed
			}
			break;			// else done parsing switches
		}
		arg++;			// advance past switch marker character

		if( keymatch( arg, "blocksmooth", 1) )
		{
			// Enable cross-block smoothing.
			cinfo->do_block_smoothing = TRUE;

		}
		else if( keymatch(arg, "colors", 1) || keymatch(arg, "colours", 1) ||
	       keymatch(arg, "quantize", 1) || keymatch(arg, "quantise", 1))
		{
			// Do color quantization.
#ifdef	_INC_WINDOWS
	      if( ++argn >= argc )	// advance to next argument
			{
				ERRFLAG( BAD_COMMAND );
			}
			if( wsscanf( argv[argn], "%d", &ScanList[0] ) != 1 )
			{
				ERRFLAG( BAD_COMMAND );
			}
			DSval = ScanList[0];
#else
	      if( ++argn >= argc )	// advance to next argument
				usage();
      	if( sscanf(argv[argn], "%d", &DSval ) != 1 )
				usage();
#endif
			cinfo->desired_number_of_colors = DSval;
			cinfo->quantize_colors = TRUE;

		}
#ifndef	_INC_WINDOWS
		else if( keymatch(arg, "debug", 1) || keymatch(arg, "verbose", 1))
		{
			// Enable debug printouts.
			// On first -d, print version identification
			if( last_file_arg_seen == 0 && cinfo->emethods->trace_level == 0 )
				fprintf(msgout, "Independent JPEG Group's DJPEG, version %s\n%s\n",
				JVERSION, JCOPYRIGHT);
			cinfo->emethods->trace_level++;

		}
#endif
		else if( keymatch(arg, "gif", 1) )
		{
			// GIF output format.
			requested_fmt = FMT_GIF;

		}
		else if( keymatch(arg, "bmp", 1) )
		{
			// BMP output format.
			requested_fmt = FMT_BMP;

		}
		else if( keymatch(arg, "grayscale", 2) || keymatch(arg, "greyscale",2))
		{
			// Force monochrome output.
			cinfo->out_color_space = CS_GRAYSCALE;

		}
#ifndef	_INC_WINDOWS
		else if( keymatch(arg, "maxmemory", 1))
		{
			// Maximum memory in Kb (or Mb with 'm').
			DSch = 'x';
			if( ++argn >= argc )	// advance to next argument
				usage();
			if( sscanf(argv[argn], "%ld%c", &DSlval, &DSch) < 1 )
				usage();
			if (DSch == 'm' || DSch == 'M')
				DSlval *= 1000L;
			cinfo->emethods->max_memory_to_use = DSlval * 1000L;

		}
#endif
		else if( keymatch(arg, "nodither", 3))
		{
			// Suppress dithering in color quantization.
			cinfo->use_dithering = FALSE;

		}
		else if( keymatch(arg, "onepass", 1))
		{
			// Use fast one-pass quantization.
			cinfo->two_pass_quantize = FALSE;

		}
#ifndef	_INC_WINDOWS
		else if( keymatch(arg, "outfile", 3))
		{
			// Set output file name.
			if( ++argn >= argc )	// advance to next argument
				usage();
			outfilename = argv[argn];	// save it away for later use

		}
#endif
		else if( keymatch(arg, "pnm", 1) || keymatch(arg, "ppm", 1))
		{
			// PPM/PGM output format.
			requested_fmt = FMT_PPM;

		}
		else if( keymatch(arg, "rle", 1) )
		{
			// RLE output format.
			requested_fmt = FMT_RLE;

		}
		else if( keymatch(arg, "targa", 1) )
		{
			// Targa output format.
			requested_fmt = FMT_TARGA;

		}
		else
		{
#ifdef	_INC_WINDOWS
			ERRFLAG( BAD_COMMAND );
#else
			usage();			// bogus switch
#endif
		}
	}	// For argument list ...
// =====================================================

	return( argn );	// return index of next arg (file name)
}

//
// The main program.
//
// To DECOMPRESS a JPEG File
// =========================
struct Decompress_info_struct1 DSdinfo;
struct Decompress_methods_struct DSdc_methods;
struct External_methods_struct DSe_methods;

#ifndef	WJPEG4	// NOTE:Older Interfaces RETIRED

// WJPEG Library ENTRY
// ###################
EXPORT32
int MLIBCALL
WREADJPEG( int argc, char MLPTR MLPTR argv )
{
	int file_index;
#ifdef	_INC_WINDOWS
  LPSTR lpInf;

	bErrFlag = 0;	// No error yet
#endif
  // On Mac, fetch a command line.
#ifdef THINK_C
  argc = ccommand(&argv);
#endif
#ifdef	_INC_WINDOWS
	APIStart();
	winp_term();	// and ensure any pending input cleared
	msgout = &szMsgBuf[0];  // Setup a MESSAGE buffer
    
#ifndef WIN64
#ifdef	WIN32
//	SETEXIT( AMAINRET );
	_asm 
	{
		mov  eax,offset AMAINRET ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump ;
		add  esp,2 * 4 ;
	}
#else	// !WIN32
	_asm
	{
		mov	ax,offset AMAINRET
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

//	ERRFLAG( 1 );	// Test the JUMP

#else		// !_INC_WINDOWS

  progname = argv[0];

#endif	// INC_WINDOWS y/n

  // Set up links to method structures.
  DSdinfo.methods = &DSdc_methods;
  DSdinfo.emethods = &DSe_methods;

#ifndef	_INC_WINDOWS
  // Install, but don't yet enable signal catcher.
#ifdef NEED_SIGNAL_CATCHER
  emethods = NULL;
  signal(SIGINT, signal_catcher);
#ifdef SIGTERM			// not all systems have SIGTERM
  signal(SIGTERM, signal_catcher);
#endif
#endif
#endif	// !_INC_WINDOWS

  // Scan command line: set up compression parameters, find file names.

  file_index = parse_switches(&DSdinfo, 0, argc, argv);

#ifdef TWO_FILE_COMMANDLINE	// NOTE: This is used in _INC_WINDOWS
  // Must have either -outfile switch or explicit output file name
	if( outfilename == NULL )
	{
		if( file_index != argc-2 )
		{
#ifdef   _INC_WINDOWS
         ERRFLAG( BAD_FILENAME );
         goto dmainret;
#else
      	fprintf(msgout, "%s: must name one input and one output file\n",
	      	progname);
      	usage();
#endif
    	}
    	outfilename = argv[file_index+1];
	}
	else
	{
		if( file_index != argc-1 )
		{
#ifdef   _INC_WINDOWS
         ERRFLAG( BAD_FILENAME );
         goto dmainret;
#else
			fprintf(msgout, "%s: must name one input and one output file\n",
				progname);
			usage();
#endif
		}
  }
#else	// !USE_TWO_COMMANDLINE = !_INC_WINDOWS
// ========================================================
  // Unix style: expect zero or one file name
  if (file_index < argc-1) {
    fprintf(msgout, "%s: only one input file\n", progname);
    usage();
  }
// ========================================================
#endif // TWO_FILE_COMMANDLINE

  // Open the input file.
	if( file_index < argc )
	{
		if( ((DSdinfo.input_file = in_open(argv[file_index], READ_BINARY)) == 0 ) ||
			( DSdinfo.input_file == HFILE_ERROR ) )
		{
#ifdef	_INC_WINDOWS
			bErrFlag = BAD_FOPEN;
			goto dmainret;
#else
	      fprintf(msgout, "%s: can't open %s\n", progname, argv[file_index]);
   	   exit(EXIT_FAILURE);
#endif
		}
	}
	else
	{
#ifdef	_INC_WINDOWS
			bErrFlag = BAD_FOPEN;
			goto dmainret;
#else	// !_INC_WINDOWS
    // default input file is stdin
#ifdef USE_SETMODE		// need to hack file mode?
		setmode(fileno(stdin), O_BINARY);
#endif
#ifdef USE_FDOPEN		// need to re-open in binary mode?
		if((DSdinfo.input_file = fdopen(fileno(stdin), READ_BINARY)) == NULL)
		{
      	fprintf(msgout, "%s: can't open stdin\n", progname);
      	exit(EXIT_FAILURE);
		}
#else
		DSdinfo.input_file = stdin;
#endif
#endif	// _INC_WINDOWS y/n
  }

  // Open the output file.
	if( outfilename != NULL )
	{
		if( ((DSdinfo.output_file = out_open(outfilename, WRITE_BINARY)) == 0) ||
			( DSdinfo.output_file == HFILE_ERROR ) ) 
		{
#ifdef	_INC_WINDOWS
			bErrFlag = BAD_OOPEN;
			goto dmainret;
#else	// !_INC_WINDOWS
			fprintf(msgout, "%s: can't open %s\n", progname, outfilename);
			exit(EXIT_FAILURE);
#endif	// _INC_WINDOWS y/n
		}
	}
	else
	{
#ifdef	_INC_WINDOWS
			bErrFlag = BAD_OOPEN;
			goto dmainret;
#else	// !_INC_WINDWS
    // default output file is stdout
#ifdef USE_SETMODE		// need to hack file mode?
    setmode(fileno(stdout), O_BINARY);
#endif
#ifdef USE_FDOPEN		// need to re-open in binary mode?
    if ((DSdinfo.output_file = fdopen(fileno(stdout), WRITE_BINARY)) == NULL) {
      fprintf(msgout, "%s: can't open stdout\n", progname);
      exit(EXIT_FAILURE);
    }
#else
    DSdinfo.output_file = stdout;
#endif
#endif	// _INC_WINDOWS y/n
  }
  
  // Set up to read a JFIF or baseline-JPEG file.
  // A smarter UI would inspect the first few bytes of the input file
  // to determine its type.
#ifdef JFIF_SUPPORTED
  jselrjfif(&DSdinfo);
#else
  You shoulda defined JFIF_SUPPORTED.   // deliberate syntax error
#endif

#ifdef PROGRESS_REPORT
  // Start up progress display, unless trace output is on
  if (DSe_methods.trace_level == 0)
    DSdc_methods.progress_monitor = Dprogress_monitor;
#endif

  // Do it to it! ie READ JPG file and OUPUT in requested format.
  // called from WREADJPEG
  jpeg_decompress(&DSdinfo);

#ifndef	_INC_WINDOWS
#ifdef PROGRESS_REPORT
  // Clear away progress display
  if (DSe_methods.trace_level == 0) {
    fprintf(msgout, "\r                \r");
#ifdef	_INC_WINDOWS
	fpaint( msgout );
#else
	fflush( msgout );
#endif
  }
#endif
#endif	// !_INC_WINDOWS
  // All done.
#ifdef	_INC_WINDOWS
#ifndef WIN64
#ifdef	WIN32
	_asm
	{
AMAINRET:
		mov	eax, [bErrFlag]
	}
#else	// !WIN32
	_asm
	{
AMAINRET:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}
#endif	// WIN32 y/n
#endif // #ifndef WIN64

dmainret:
	jmem_term();	// Ensure ALL memory freed
	winp_term();	// and ensure any pending input cleared
	if( lpInf = argv[0] )	// In Windows this is an INFORMATION pointer
	{
		lpInf[0] = 0;
		if( bErrFlag )
			GetErrorMsg( lpInf, (WORD)bErrFlag );
		GetWarnMsg( lpInf );
	}
#else
  exit(EXIT_SUCCESS);
#endif
	return( bErrFlag );	// suppress no-return-value warnings
}	// WJPEG Library EXIT
// ###################

#endif	// !WJPEG4

#ifdef	_INC_WINDOWS
// ======================================================
extern	DWORD ddMaxCnt;
extern	LPSTR lpMBuf;
extern	DWORD ddBufCnt;
extern	HGLOBAL	hgDIB;
extern	int get_jpeg_data( decompress_info_ptr );

// NOTE: In original HEADERS these were ALL done in parse_switches!!!
GLOBAL	void
SetupJDef( decompress_info_ptr cinfo )
{
	// (Re-)initialize the system-dependent error and memory managers.
	jselerror(cinfo->emethods);	// error/trace message routines
	jselmemmgr(cinfo->emethods);// memory allocation routines
	cinfo->methods->d_ui_method_selection = d_ui_method_selection;
	// Set up default JPEG parameters.
	j_d_defaults(cinfo, TRUE);
	// Install standard buffer-reloading method (outer code over-riding)
	cinfo->methods->read_jpeg_data = get_jpeg_data;
	requested_fmt = FMT_BMP;	// SET output file format
	outfilename = NULL;
}

//#ifndef	WJPEG4	// NOTE: Older file interface RETIRED - April, 1997
#ifdef	ADDOLDTO	// Add back UNUSED i/f
// If we WANT Optimisation, ie NDEBUG, and as 16-BIT
// =================================================
#if		( _MSC_VER > 700 )
#if		(defined( NDEBUG ) && defined( Dv16_App ))
// **************************************************************
#pragma warning( disable : 4704 )
// **************************************************************
#endif	// NDEBUG & Dv16_App
#endif	// _MSC_VER > 700

// Use NEWER WJPG2BMP service ...
// ==========================
// WJPEG Library ENTRY
// ###################
EXPORT32
HGLOBAL MLIBCALL
WJPGTOBMP( HGLOBAL hg, DWORD sz, HGLOBAL hIB, LPSTR lpof )
{
	LPSTR	lpInf;

	bErrFlag = 0;	// No error yet ...
	hgDIB = 0;
	APIStart();
	lpOutFile = lpof;	/* Either a POINTER to a FILE or NULL ... */
	winp_term();	/* and ensure any pending input cleared ... */
	msgout = &szMsgBuf[0];  /* Setup a MESSAGE buffer ... */ 
	lpInf = 0;
#ifndef WIN64
#ifdef	WIN32
//	SETEXIT( JMAINRET );
	_asm 
	{
		mov  eax,offset JMAINRET ;
		mov  edx, esp ;
		push edx ;
		push eax ;
		call setexitjump ;
		add  esp,2 * 4 ;
	}
#else	// !WIN32
	_asm
	{
		mov	ax,offset JMAINRET
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
		goto jmain_ret;
	}	

  /* Open the output file. */
	if( lpof && lpof[0] )
	{
//		if( ((DSdinfo.output_file = out_open( lpof, WRITE_BINARY )) == 0 ) ||
//			( DSdinfo.output_file == HFILE_ERROR ) )
		if( ( (DSdinfo.output_file = gout_open( lpof )) == 0 ) ||
			( DSdinfo.output_file == HFILE_ERROR ) )
		{
			ERRFLAG( BAD_FILENAME );
			goto jmain_ret;
		}
	}
	else
	{
         ERRFLAG( BAD_FILENAME );
         goto jmain_ret;
	}
  
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

  // Do it to it! ie READ JPG file and OUPUT in requested format.
  // Called from WJPGTOBMP
  jpeg_decompress(&DSdinfo);

  /* All done. */
#ifndef WIN64
#ifdef	WIN32
	_asm
	{
JMAINRET:
		mov		eax, [bErrFlag]
	}
#else	// !WIN32
	_asm
	{
JMAINRET:
		mov	ax,bErrFlag	;	Get the ERROR FLAG ...
	}
#endif	// WIN32 y/n
#endif // #ifndef WIN64

jmain_ret:
	jmem_term();	/* Ensure ALL memory freed ... */
	winp_term();	/* and ensure any pending input cleared ... */
	if( hg && lpMBuf )
	{
		JGlobalUnlock( hg );
		lpMBuf = 0;
	}
	if( hIB && ( lpInf = JGlobalLock( hIB ) ) )
	{
		lpInf[0] = 0;
		if( bErrFlag )
			GetErrorMsg( lpInf, (WORD)bErrFlag );
		GetWarnMsg( lpInf );
		JGlobalUnlock( hIB );	/* Unlock the information buffer */
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
	if( DSdinfo.output_file &&
		( DSdinfo.output_file != HFILE_ERROR ) )
	{
		gout_close( DSdinfo.output_file );
	}
	DSdinfo.output_file = 0;
	return( hgDIB );
}	// WJPEG Library EXIT
// ###################
#if		( _MSC_VER > 700 )
#if		(defined( NDEBUG ) && defined( Dv16_App ))
// **************************************************************
#pragma warning( default : 4704 )
// **************************************************************
#endif	// NDEBUG & Dv16_App
#endif	// _MSC_VER > 700

#endif	// ADDOLDTO - After WJPEG4 put back for Malo

/* ====================================================== */
#endif	/* INC_WINDOWS */

/* eof */
