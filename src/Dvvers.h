//
// FILE: DvVers.h (was version.h)
// DIBView Application VERSION File
// Here ALL the primary decisions MUST be made
// ===========================================
// NOTE: Also no _INC_WINDOWS yet either ...
#ifndef	_version_h
#define	_version_h

// InternalName= < IN VERSION BLOCK >
#define	INT_NAME		"Dv32"		// Should AGREE with VersionInfo
#define	Dv32						// and FLAG this (single) product
// NOTE WELL: Remember to changed the VERSIO_INFO in resource file to agree!
#define	VER_MAJOR		5
#define	VER_MINOR		0
#define	VER_MINOR2		0
// ===== current INI date =====
#define  VER_BUILD		0	// FIX20200602 - 64-bit build in Dell03
#define  VER_DATE		"2 June, 2010"	// using MSVC 16 2019 x64

// Until FIXED
#undef ADD_JPEG_SUPPORT

// VER_MAJOR		1
// VER_BUILD		40	// FIX20081025 - various - clip list, magnify, swish, etc,
// VER_DATE		"25 October, 2008"	// MSVC8

// VER_BUILD		39	// FIX20081019 - add RGB color to mouse position
// VER_DATE		"19 October, 2008"	// MSVC8

// VER_BUILD		38	// FIX20080720 - avoid multiple error dialogs
// VER_DATE		"20 July, 2008"	// MSVC8
// VER_BUILD		37	// FIX20080316 - INI location, and window size
// VER_DATE		"16 March, 2008"	// MSVC8
// 1. Loading programmically, the DIBVIEW.INI get loaded, and written in the
// current active directory. Should be the EXE path. NO!!
// FIX20200602: DibView.ini should be in %APPDATA%\DibView/
// 2. Enlarging to full screen, break the alternate sizes. They sort of
// become full screen also.
#define  USENEWWINSIZE
// and there does seem some BUG now in the CLIP code???
// and the status bar 'competes' with the horizontal scroll!!!

//	VER_BUILD		36	// FIX20071201 - Add more CLIP manipulation
//	VER_DATE		"1 December, 2007"	// in Express Edition (.NET 2005)

//	VER_BUILD		35	// FIX20070223 - Add STATUS BAR
//	VER_DATE		"23 February, 2007"	// Express Edition (.NET 2005)
#define  ADD_STATUS_BAR

//	VER_BUILD		34	// FIX20061105 - In Dell01 using MSVC8
//	VER_DATE		"5 November, 2006"	// Express Edition (.NET 2005)
#pragma warning( disable:4996 )
// VER_BUILD		33	// FIX20051128 - Mouse modification of CLIP region
// VER_DATE		"28 November, 2005"	// also in MSVC7.1 (.NET 2003)
#define  ADD_CLIP_RESIZING // new code under this item

//	VER_BUILD		32	// FIX20050321 - INSTALL and USE keep aspect ration
//	VER_DATE		"21 March, 2005"	// fix 'temp???' file only to exe root
//	VER_BUILD		31	// FIX20041116 - add keep aspect ration
//	VER_DATE		"16 November, 2004"	// fix ???
//	VER_BUILD		30	// Into PRO-1 - FIX20041009
//	VER_DATE		"9 October, 2004"	// fix ???
//	VER_BUILD		29	// fixes to readrgb - FIX20030724
//	VER_DATE		"24 July, 2003"	// fix date - dbg:write bmp file

//	VER_BUILD		28	// add readrgb - remove CLR file
//	VER_DATE		"10 April, 2003"	// fix begin date
//	VER_BUILD		27	// Add load / review folder
//	VER_DATE		"9 October, 2002"	// fix begin date
//	VER_BUILD		26	// Add TIFF6/GEOTIFF TIFF/IT
//	VER_DATE		"17 September, 2002"	// Release date
//	VER_BUILD		25	// Lot of small tidying, but better ...
// VER_DATE		"16 September, 2002"	// Release date
//	VER_BUILD		24	// Use thread to get DIB Info - see USEITHREAD
//	VER_DATE		"2 December, 2000"	// Release date
//	VER_BUILD		2	// After a few compile tries ...
//	VER_BUILD		3	// Add JPEG library ...
//	VER_DATE			"6 March, 1996"
//	VER_BUILD		4	// Add direct GIF to BMP JPEG library ...
//	VER_DATE			"16 March, 1996"
//	VER_BUILD		5	// Add direct JPG to BMP from WJPEG library ...
//	VER_DATE			"28 March, 1996"
//	VER_BUILD		6	// Separate READ/WRITE Directories and store in INI
//	VER_DATE			"2 April, 1996"
//	VER_BUILD		7	// Separate READ/WRITE Directories and store in INI
// VER_DATE			"4 April, 1996"
//	VER_BUILD		8	// Separate READ/WRITE Directories and store in INI
//	VER_DATE			"10 April, 1996"
//	VER_BUILD		9	// +Config and GIF(n)
//	VER_DATE			"22 April, 1996"
//	VER_BUILD		10	// +Timer and Restore ALL (Enum Chilren)
//	VER_DATE			"24 April, 1996"
//	VER_BUILD		11	// Many fixes/changes in palette.c (and print)
//	VER_DATE			"6 May, 1996"
//	VER_BUILD		12	// 32-Bit version
//	VER_DATE		"31 August, 1996"
//	VER_BUILD		13	// 32-Bit version
// Started refinements	"22 October, 1996"
//	VER_DATE		"30 November, 1996"	// Private and Release
//	VER_BUILD		14	// 16-Bit AND 32-Bit versions
// Started refinements	"10 April, 1997"
//	VER_DATE		"10 April, 1997"	// Private
//	VER_BUILD		15	// Rel. 16-Bit version of WJPEG16.DLL
// Started refinements	"10 April, 1997", but considerable NEW
// service stuff, to Malo ...
//	VER_DATE		"29 April, 1997"	// Release
//	VER_BUILD		16	// 2nd Rel.16-Bit WJPEG16.DLL
// Final Release to Malo...
//	VER_DATE		"10 May, 1997"	// Release
//  VER_BUILD		17	// Addition.16/32-Bit DLL WJPG2BMP source
//	VER_DATE		"11 June, 1997"	// Release
//	VER_BUILD		18	// Rework on drive F:
//	VER_DATE		"25 November, 1997"	// Release
//	VER_BUILD		19	// BIG DIB Fixes and more ...
//	VER_DATE		" 7 May, 1998"	// Release
//	VER_BUILD		20	// Clip rectangle Fixes
//	VER_DATE		"28 December, 1998"	// Release
// VER_BUILD		21	// Improve PALETTE/Color show
// VER_DATE		"17 October, 1999"	// Release
// added F20000825 - Show RUNTIME MODULE and CURRENT WORK DIRECTORY
// VER_BUILD		22	// Improve About information dialog as above
//	VER_DATE		"25 August, 2000"	// Release
// F20001126 - Appears LoadLibary() returns ZERO for ERROR, so change
// HINSTANCE_ERROR to MYHERROR in WIN32 == ZERO
//	VER_BUILD		23	// Above correction
//	VER_DATE		"26 November, 2000"	// Release date
// go back to TOP - it now grow UPWARDS

// PS: DON'T FORGET THE DATE IN DIB VIEW RTF HELP FILE
// (if included - only with Dv releases, not Libs ONLY).

// FEATURES SUPPORTED
// VER_BUILD29	// fixes to readrgb - FIX20030724
#define  ADD_SGI_RGB_READ

#undef  ADDSELWIN2  // still not working - bah ***TBD***
// use a global 'selection' process, that commences when
// the left button goes DOWN - grab CURSOR, change to SELECTCUR, and
// wait for left button UP   - release cursor, translate spot into a WINDOW,
// if window is us, bleep, if window is desktop - not sure of this restriction,
// else capture the FRAME, or the CLIENT portion. ***TBD*** would be to not
// only accept UP-on-title-bar as the ONLY solution - enumerate wins before
// this event, and clicking on 'children' of parent windows would also be ENABLED!!!
// Also, while in FULL CAPTURE mode, show only a dialog of self.
// then add
#define  ADDTIFF6    // Add Tag Info File Format - v6 = TIFF/IT  FIX20020917
#define  ADDGEOTIFF  // and the 'world location' of the block    FIX20020917

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
#define	ADDWJPEG		// Use the JPEG library for reading and writing
	// JPG, GIF ... and others (like PPM, RLE, ...) may be added ...
#define  WJPEG4	// NOTE: Older file and command line interface - April 1997 - if ADD_JPEG_SUPPORT
#define	ADDLIBCFG	// Add the DLL Configuration items
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

#define		FRMENU	/* use french menu - in PAL only */
#ifdef	DV32_EXE	// Building 32-bit EXE, static is quick...
//=========================================================
//#undef	LDYNALIB	// MUST Build 32-bit DLL to MATCH
// AND include WJpeg32.Lib in the link. STATIC LOAD DLL
// AND WJPG2BMP.Lib
//=========================================================
// Use LoadLibrary - This allows DibView to RUN
// even without the DLL for the JPEG conversions.
// This is the BEST option, and should be ON!!!
#define	LDYNALIB	// For 32-bit DYNAMIC LOAD LIBRARY

#else	// !DV32_EXE
//=========================================================
// 16-bit SUPPORT maintained as best possible
// ==========================================
#ifdef	Dv16_App
// Some erros with 16-bit dynamic load of library
// but these need to be fixed!
//===============================================
#undef	LDYNALIB	// Cause a STATIC Link to LIB
//===============================================
#else	// !Dv16_App
// ==================================================
// This 32-bit world - Simpler memory references
//=========================================================
//#undef	LDYNALIB		// MUST Build 32-bit DLL to MATCH
// AND include WJpeg32.Lib in the link. STATIC LOAD DLL
// AND WJPG2BMP.Lib for JPEG 6a (61) inclusion - June 1997
//=========================================================
//#pragma message( "No problem with 32-bit library load" )
// 20200528 - try with this OFF - BUT FAILED
#define	LDYNALIB	// NO STATIC LINK with LIBRARY
// Use Dynamic LoadLibrary - And resolve each ADDRESS!
// This is the BEST option, and should be ON!!!
// since it allows DV32 to run WITHOUT the Library!
// ================================================
#endif	// Dv16_App y/n
//=========================================================
#endif	// DV32_EXE y/n

#undef	ADDWININI	// FIX20080316 - REMOVE - Put Version and INI File name in WIN.INI
#undef	ADDROT		// Develop a ROTATE function!
#undef	ADDREVIEW	// Add to the standard OPEN dialog
#define	ADDCOLRF	// Add Bk Color and Def. Font choices

#define	BANDWGIF	// Black and White GIF files.
#define	TRANSGIF	// TRANSPARENT GIF to Bitmap.
#define	ADDOLDTO	// Add back WGIFTOBMP and WJPGTOBMP
#define	WJPEG6		// Incorporate the JPEG Version 6a (61)
//#define LIBLINK		// static link to JPEG sources
//#define LINKLIB6	// static link to JPEG-2 sources
// WAS in D:\DOWN\JPEGSRC ... many files
// Nov 1997 - Moved DV32 to F:\GTOOLS32\DV32 (as base), and
// Feb 1998 - the WJPG32_2.DLL source to (base)\WJPG32_2
// ========================================================
// Added WJPGSIZE6 and WJPG2BMP6 for SOF2 and 10 support.
#define  USELLIST // 29NOV2000 - switch to using linked list for MRU files
#define  USEITHREAD  // FIX20001202 - Use thread to get COLOR count
//#define   WRTCLRFILE // FIX20001217 - Write a CLR file
#undef   WRTCLRFILE // FIX20030410 - do *NOT* write a CLR file

#define	DIAGFILE2   // FIX20080316 - change to ALWAYS write DIAG file

// Some PURELY DIAGNOSTIC SWITCHES
// ###############################
// MUST be off for RELEASE
// =======================
#if	(defined( NDEBUG ) || defined( MRELEASE ))
// **************************************************************
// ==========================================
// ------------------------------------------
// Write out a GIF Info file. In File.c & GetGif.c
#undef		GIFDIAGS
// The following is(was) ONLY in DvMem.c
#undef		ADDMEMSTATS
// #undef	DIAGFILE2
// ------------------------------------------
// **************************************************************
#else	// !( NDEBUG | MRELEASE )
// **************************************************************
// ------------------------------------------

#define	GIFDIAGS	// Just some GIF diagnostic information
#define	ADDMEMSTATS	// Check memory allocation, lock and free
// Above SHOULD be off for RELEASE
// ###############################

// ------------------------------------------
// **************************************************************
#endif	// (NDEBUG | MRELEASE) y/n

// A buffer for collecting missed Service names ...
#define	MXSERVS		260		// Maximum service name(s) length
#define	MXTBUF		128

// NOTES ON DIAGNOSTICS OF WINDOWS MESSAGES
// ****************************************
// Since this requires WMDiag.c to be COMPILED with the diag. code
// this MUST be a switch passed in through the MAKE since WMDiag.h
// ONLY includes some standard WINDOWS files, and NOTHING from this
// particular project.
// Thus the COMPILE of the DIAGNOSTIC CODE can be achieved by
// defining any one of DIAGWM, or DIAGWM1 to DIAGWM5
// in the Settings. ie Through the MAKE utility
// This will define SHOWMSG, which will enable to CODE
//
// And ADDDBGW will also turn ON WMDiag, ifndef NDEBUG
#ifndef		NDEBUG
// **************************************************************

#	ifdef		ADDDBGW
#define		SHOWWM		// OUPUT each WINDOW message
#	else		// !ADDDBGW
#undef		SHOWWM
#	endif		// ADDDBGW y/n

// **************************************************************
#else	// NDEBUG is ON
// **************************************************************

#undef		SHOWWM		// OUPUT each WINDOW message

// **************************************************************
#endif	// NDEBUG n/y

// Defaults for WORK STRUCTURE
#define	BT_Default	TRUE
#define	SI_Default	TRUE
#define	DEF_TPI		1440	// 	giTwipsPerInch

//#define	DEFDIAGFILE		"TEMPDIAG.TXT"
#define	DEFDIAGFILE		"TEMPDV32.TXT"
#define	DEF_DBGLEVEL		9	// Set MAXIMUM Verbosity for DEBUG

#define		DT3		// Some Get Date and Time functions

#define	MXSBUF			264		// max. stack char buffer
#define	MXDLGMSG		1024	// 1K max. output ... message.

//#define	DVHELPFILE		"DIBVIEW.HLP"
#define	DVHELPFILE		"DV32.HLP"

#define		MEOR		"\r\n"

#define  DEF_FM_STG     "TEMPB%03d.BMP"  // TCHAR gszRPFrm[] = DEF_FM_STG;
#define	DEF_MAX_COLS	4096	// For 24-Bit Palette building in child.c
#define	DEF_MXFILCNT	20		/* Remember the last 20 files */
#define  DEF_MXHISCNT   20
#define  DEF_MXFNDCNT   20
#define  DEF_MXMSKCNT   10
#define  DEF_MXAUTCNT   64    // gdwAutFiles

#define  DEF_ITER       TRUE

#endif	// _version_h
// eof - DvVers.h (was version.h)
