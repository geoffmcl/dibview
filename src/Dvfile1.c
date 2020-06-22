
/* ************************************************************************

      File:  DVFILE1.C

   Purpose:  Contains the file I/O routines and the Dib Information
             dialog box.

 Functions:  HANDLE   OpenDIBFile       (lpFN);
			void	OpenDIBFile2       ( PRDIB );
             HANDLE   ReadDIBFile       ( PRDIB );
             BOOL     MyRead            (HANDLE, LPSTR, DWORD);
             int      CheckIfFileExists ( LPSTR );
             int      GetDibInfo        ( LPSTR, LPINFOSTRUCT, BOOL );
 Moved & renamed  BOOL SaveDIBFile(void); See Dv_IDM_SAVE in DvFile.c
             HANDLE   WinDibFromBitmap  (HBITMAP, DWORD, WORD, HPALETTE);
             HANDLE   PMDibFromBitmap   (HBITMAP, DWORD, WORD, HPALETTE);
             VOID     ParseCommandLine  (LPSTR);
             HANDLE   GetDIB            (LPSTR, DWORD);
             HANDLE   GetDIB2           ( PRDIB );
             DWORD    DVWrite           (int, VOID MLPTR, DWORD);

  Comments:

   History:   Date     Reason

             6/1/91    Created
             6/27/91   Added Dib Information support
             7/22/91   Added File Save support
             8/8/91    Added Command Line Support
            11/15/91   Added lpFN parm to OpenDIBFile.
				15Jan96	Added (a) Fill in default ".BMP" if NOT given, and
									(b) Show [NAME] of file NOT found ...
            1March96 Add JPEG encode and decode, plus the code for GIF,
                     RLE, PPM, Targa, but this is NOT completed.
            05March98 Moved WriteDIB(LPSTR, HANDLE) to DvWrite.c

************************************************************************ */
#include	"dv.h"	// Single, all inclusive include.
#include	"DvFunc.h"

// #define	COMBWLIB	// LINKED with LIBRARY CODE
// NOTE: This is ONLY supplied as a Preprocessor FLAG
// ==================================================
// The above has been moved into the BUILD as a precompiler flag
// =============================================================
#ifndef	WIN32
#include	<io.h>
#endif	// !WIN32

#include	"DvGif.h"

#ifdef	PROBGOBJ
// #pragma message( "Still to solve problem of returning GLOBAL MEMORY Object" )
// This is NOT required for the 32-bit library, which always uses
// discardable memory HGLOBAL passed to Library for conversion to data...

#ifdef	_WIN32
#ifdef _WIN64
#pragma message( "Using 64-bit handles to HEAP memory. (MMI/O)" )
#else
#pragma message( "Using 32-bit handles to HEAP memory. (MMI/O)" )
#endif // _WIN64 y/n
#else	// !_WIN32
// else us far memory allocation all the time - LARGE like
#pragma message( "Using WORD handles to 20-bit DOS (real) mem." )
#endif	// _WIN32 y/n

#endif	// PROBGOBJ

#undef	USEOPN		// Try _lopen( ... )

#ifdef	USEOPN
#ifndef _INC_FCNTL
#include	<fcntl.h>
#endif
#ifndef _INC_STAT
#include	<sys\stat.h>
#endif
#endif

/* And a list of ERROR message numbers (per messages in wcommon.c) ... */
extern	void  Hourglass           ( BOOL );
extern	void DDBPaint( LPDIBINFO, HDC, LPRECT, HBITMAP, LPRECT, HPALETTE, DWORD );
extern	void AddToFileList( LPSTR );
extern	BOOL	fReLoad;
extern	void	DVWait( BOOL );
extern   DWORD DVFileSize( HANDLE );
//extern	BOOL	fChgOpen;
extern	BOOL	FindNewName( LPSTR );
extern	DWORD	gdwfFlag;
extern	void	SetReadPath( LPSTR lpdf );
#ifdef	GIFDIAGS
#pragma message ( "Advice: GIFDIAGS ON, use del temp*.* ..." )
extern	void	DumpGIF( LPGIFHDREXT, LPSTR );
#endif	// GIFDIAGS
extern	DWORD	BitmapFileSize( LPSTR lpbi );	// Return SIZE
extern	DWORD	OffsetToBits( LPSTR lpbi );

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now
extern	BOOL DVWGif2Bmp( HGLOBAL, DWORD, HGLOBAL, HGLOBAL);
extern	BOOL DVWJpg2Bmp( HGLOBAL, DWORD, HGLOBAL, HGLOBAL);
extern	BOOL DVWJpg2Bmp6( HGLOBAL, DWORD, HGLOBAL, HGLOBAL);
extern	BOOL	LibLoaded2( void );	//	if( WJpgSize && WJpg2Bmp )
//extern	BOOL DVWJpg2Bmp( HGLOBAL, DWORD, HGLOBAL, HGLOBAL);
//extern	WORD DVWBmpToJpg( HGLOBAL, DWORD, HGLOBAL, LPSTR );
extern	DWORD DVWGifSize( HGLOBAL, DWORD, LPSIZE );
extern	DWORD DVWJpgSize( HGLOBAL, DWORD, LPSIZE );
extern	DWORD DVWJpgSize6( HGLOBAL, DWORD, LPSIZE );
extern	WORD DVWGifSizX( HGLOBAL, DWORD, HGLOBAL );
#ifndef	USENEWAPI
extern	HGLOBAL DVWGifToBmp( HGLOBAL, DWORD, HGLOBAL, LPSTR );
extern	HGLOBAL DVWJpgToBmp( HGLOBAL, DWORD, HGLOBAL, LPSTR );
#endif	// USENEWAPI
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now

extern	BOOL	ChkMFRFile( LPSTR lpf );

extern	HANDLE	DvOpen2( LPSTR lpf, OFSTRUCT * pos, UINT uType );
extern	DWORD	DWSIZE( DWORD siz );

extern	DWORD	PutInfo( LPSTR lpt );
extern	DWORD	KillInfo( void );
extern	LPSTR	GetNNumSStg( DWORD );
// NEW
extern   PMWL	AddToFileList4( PRDIB prd );

DWORD    DVWrite( HANDLE, VOID MLPTR, DWORD);
BOOL LIBGifNBmp( HGLOBAL hgG, DWORD ddSz, HGLOBAL hIBuf,
				HGLOBAL hDIB, WORD num );
LPSTR	GetInpFile( LPSTR lpf );

DWORD	FType = ft_Undef;
BOOL	fShwNoConv = TRUE;
DWORD	GetSizeMxI(void)
{
	return(1124);
}

#ifdef	ADDWJPEG

#ifdef	USEB2G
extern	BOOL B2G( HGLOBAL, DWORD, HGLOBAL, DWORD, LPSTR );
#endif

//extern	BOOL	fBe_Tidy;
BOOL	fNeedTwo = FALSE;
DWORD	ErrNumb = 0;
HGLOBAL	hgGIF = 0;
LPSTR		lpGIF = 0;
HGLOBAL	hgINF = 0;
LPSTR		lpINF = 0;

// Internal Output/Input FILE TYPE - See 
//#define	ft_Undef		0
//#define	ft_BMP		1
//#define	ft_GIF		2
//#define	ft_PPM		3
//#define	ft_RLE		4	/* NOT YET SUPPORTED */
//#define	ft_TARGA		5
//#define	ft_JPG		6
//#define	ft_TIF		7
//	VER_BUILD29	// fixes to readrgb - FIX20030724
//#define  ft_RGB      8  // RGB (SGI texture files)

#ifndef	WJPEG4	// NOTE: Older file and command line interface
// was RETIRED April, 1997
// ===========================================================
char	*pRSwPPM = "-ppm";
char	*pRSwGIF = "-gif";
char	*pRSwTARGA = "-targa";

/* Option switches for WWRITEJPEG (wcmain.c) ... */
/* ============================================= */

char	*pRGrSw = "-grayscale";		/* Create monochrome JPEG file */
char	*pROpSw = "-optimize";	/* IFF ENTROPY_OPT_SUPPORTED, then ... */
	/* Optimize Huffman table (smaller file, but slow compression) */
char	*pRQuSw = "-quality";	/* + N Compression quality followed by ... */
	/* N = 0-100. 5-95 is useful range */

	/* Switches for advanced users */
	/* =========================== */
char	*pRSmSw = "-smooth";	/* + N = Smooth dithered input */
	/* N = 1 - 100 is strength */
char	*pRReSw = "-restart";	/* + N = Set restart interval in rows, */
	/*  or in blocks with B */

	/* "Switches for wizards */
	/* ===================== */
char	*pRArSw = "-arithmetic";	/* Use arithmetic coding (if supported!) */
char	*pRNiSw = "-nointerleave";	/* Create noninterleaved JPEG file */
char	*pRQtSw = "-qtables";	/* + file  Use quantization tables given in file */
char	*pRSaSw = "-sample";	/* + HxV[,...]  Set JPEG sampling factors */

/* Usage switches for WREADJPEG followed by inputfile outputfile (wdmain.c) */
/* Switch names may be abbreviated */
char	*pRDCol = "-colors";	/* + N Reduce image to no more than N colors */
char	*pRDGif = "-gif";	/* Select GIF output format. */
char	*pRDPpm = "-ppm";	/* Select PBMPLUS (PPM/PGM) output format. */
char	*pRDBmp = "-bmp"; /* Select MS/IBM Windows Bitmap format (default) */
char	*pRDQua = "-quantize"; /* + N Same as -colors N! */
char	*pRDRle = "-rle";	/* Select Utah RLE output format */
char	*pRDTar = "-targa";	/* Select Targa output format */
/* Switches for advanced users ... */
char	*pRDBlk = "-blocksmooth";	/* Apply cross-block smoothing */
char	*pRDGry = "-grayscale";	/* Force grayscale output */
char	*pRDNod = "-nodither";	/* Don't use dithering in quantization */
char	*pRDOne = "-onepass";	/* Use 1-pass quantization (fast, low quality) */
#endif	// !WJPEG4

#endif	/* ADDWJPEG */

//      "Bitmaps (*.bmp,*.dib,*.rle)",	/* Index 1 */
//      "*.bmp;*.dib;*.rle",

/* Here we TRUSTED the compiler to make these strings contiguous in MEMORY */
/* and we thus supply the first POINTER. One day this should be FIXED!!! */
/* NOTE: If this is adjusted, then so must the pRDefLst[] below match */
/* BUT THIS PROVED IN ERROR IN THE NEW CL 32-BIT COMPILER */
#ifdef	ADDWJPEG    // this library works ok, so this is the active stream

char szFStrJ[] =  // filter string
   "Bitmap    (*.bmp)\0*.bmp;*.dib;*.rle\0"
   "JPEG      (*.jpg)\0*.jpg\0"
	"GIF       (*.gif)\0*.gif\0"
#ifdef   ADDTIFF6
   "TIFF      (*.tif)\0*.tif\0"
#endif   // ADDTIFF6
   "All Images Supp'd\0*.bmp;*.dib;*.rle;*.jpg;*.gif;*.tif\0"
	"ALL         (*.*)\0*.*\0"
	"\0";		/* WITH Double nul termination ... */

char szFStrJ_2[] =  // filter string
   "All Images Supported\0*.bmp;*.dib;*.rle;*.jpg;*gif;*.tif\0"
   "Bitmap (*.bmp)\0*.bmp;*.dib;*.rle\0"
   "JPEG   (*.jpg)\0*.jpg\0"
	"GIF    (*.gif)\0*.gif\0"
#ifdef   ADDTIFF6
   "TIFF   (*.tif)\0*.tif\0"
#endif   // ADDTIFF6
#ifdef   ADD_SGI_RGB_READ
   "SGIrgb (*.rgb)\0*.rgb\0"
#endif   // #ifdef   ADD_SGI_RGB_READ
	"ALL    (*.*)\0*.*\0"
	"\0";		/* WITH Double nul termination ... */

#endif	/* ADDWJPEG */

char szFStrS[] =  "Bitmap (*.bmp)\0"\
	"*.bmp;*.dib;*.rle\0"\
	"ALL (*.*)\0"\
	"*.*\0"\
	"\0";		/* WITH Double nul termination ... */

#ifdef	ADDWJPEG    // this library works ok, so this is the active stream
LPSTR pszFilter = &szFStrJ[0];	/* File Filter String */
#else // !#ifdef	ADDWJPEG    // this library works ok, so this is the active stream
LPSTR pszFilter = &szFStrS[0];	/* File Filter String */
#endif // #ifdef	ADDWJPEG    // this library works ok, so this is the active stream

char    szDFileName[_MAX_PATH];         // Filename of DIB
//char    szErrorMess[_MAX_PATH];         // Error Message Buffer
#define	MXIBUF		1024     // 1k information buffer - for read
HGLOBAL	hIBuf;
LPSTR		lpIBuf;
SIZE		dibSize;

// See Dv.h for values ...
//#define	MXELST	10
//#define	MXCLST	3	/* Currently only 3 entries ... */
char	szDib[] = ".dib";
char	szRle[] = ".rle";
char	szBmp[] = ".bmp";
char	szJpg[] = ".jpg";
char	szGif[] = ".gif";
char  szTif[] = ".tif";
char  szRgb[] = ".rgb";
char	szAll[] = "";
char	*pRDefExt = &szBmp[0];	/* BMP extent ... */
/* NOTE WELL: This MUST match the above list of pszFilter ... */
char	*pRDefLst[MXELST] = {
		{ &szBmp[0] },
		{ &szJpg[0] },
		{ &szGif[0] },
#ifdef   ADDTIFF6
      { szTif     },
#endif   // #ifdef   ADDTIFF6
#ifdef   ADD_SGI_RGB_READ
      { szRgb     },
#endif   // #ifdef   ADD_SGI_RGB_READ
		{ &szAll[0] },
		{ 0         },
		{ 0         }
};

// and *MATCHING* type list
UINT  g_uTypExt[] = {
   ft_BMP,  // NOTE: index is ONE(1) BASED = 1
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
   ft_JPG,
   ft_GIF,
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
#ifdef   ADDTIFF6
   ft_TIF,
#endif   // #ifdef   ADDTIFF6
#ifdef   ADD_SGI_RGB_READ
   ft_RGB,
#endif   // #ifdef   ADD_SGI_RGB_READ
   0  // terminator of list
};

extern	WORD	gwOutIndex;
WORD	InIndex = 0;
BOOL	fChgInInd = FALSE;
char	*pRTmpWork = "TEMP";
void DlgWndPaint( HWND );
//void	GetCurWDir( LPSTR );
void	AddFN( LPSTR, LPSTR );
#ifdef	USEGFUNC5
#ifdef	USEGFUNC6
WORD	GetGIFCountExt( HGLOBAL, DWORD, PRDIB );
#else	// !USEGFUNC6
WORD	GetGIFCount( HGLOBAL, DWORD, PRDIB );
#endif	// USEGFUNC6 y/n
#endif	// USEGFUNC5

//char   szDirName[MXDIR+1];              // Directory name
//char   szRDirName[MXDIR+1];              // READ/Open Directory name
//char   szWDirName[MXDIR+1];              // WRITE/Save Directory name
// WORD   wDibType;                    // Type of Dib
char	*pRAFNm = "\r\nFile name [%s].";
char	*pRFnGrp = "%s [%d:%d]";
char	*pRFilSiz2 = "File Size of %lu bytes less than %u!";
char	*pRFilSiz  = "File Size of %lu bytes.";

#ifdef	DIAGSAVEHOOK
extern	void	DiagSaveHook( HWND, WORD, WORD, LONG );
extern	void	WriteBgnDiag( void );
extern	void	WriteEndDiag( void );
#endif	/* DIAGSAVEHOOK */

BOOL	fChgFE  = FALSE;
BOOL	fShowNOLIB = FALSE;	// Only SHOW this failure ONCE!!!
// ==========================================================


// Some DEBUG breakpoints ...
void	chkpv( void )
{
int i;
	i = 0;
}

#define	MXONEREAD		1024

#ifdef	WIN32
DWORD	DVRead( HANDLE hf, LPSTR lpb, DWORD siz )
{
#ifdef	WIN32
	DWORD	dwrd;
	BOOL	flg = FALSE;
	HFILE   fh = PtrToInt(hf);

	// Let the OS do its BEST with the file size
	if( siz > 2000000 )
	{
   	//char	buf[128];
		//LPTSTR	lpt = buf;
		LPTSTR	lpt = GetStgBuf();   // get a buffer

		sprintf( lpt, "Moment loading file of %s bytes ...",
			GetNNumSStg(siz) );

		if( PutInfo( lpt ) )
			flg = TRUE;
	}

	dwrd = _lread( fh, lpb, siz );

	if( flg )
		KillInfo();

	return dwrd;
#else	// !WIN32
DWORD	rd;
PINT8	cp;
DWORD	rem;
WORD	crd, rdrq;
	cp = ( PINT8 ) lpb;
	rd = 0;
	rem = siz;
	while( rem )
	{
		if( rem > MXONEREAD )
			rdrq = MXONEREAD;
		else
			rdrq = LOWORD( rem );
		crd = _lread( hf, cp, rdrq );
		rd += (DWORD) crd;
		if( crd != rdrq )
		{
			break;
		}
		cp = cp + crd;
		rem -= (DWORD) crd;
	}
return( rd );
#endif	// WIN32 y/n
}
#endif	// WIN32 y/n

/* **********************************************************

  Function:  OpenDIBFile ( lpFN )

   Purpose:  Prompts user for a filename, opens it, and returns a handle
             to the DIB memory block.

   Returns:  NULL if no file is opened, or an error occurs reading the file
             A valid DIB handle if successful.

  Comments:

   History:   Date     Reason

             6/1/91    Created
            11/15/91   Added File name parm.

   Handles: 1. From Do_IDM_OPEN from IDM_OPEN from menu
            2. etc

   *********************************************************** */
HANDLE OpenDIBFile( LPSTR lpFN )
{
	HANDLE   hD;
	LPSTR	   lpdf;
   INT      typ;

	hD = 0;  // start with NO DIB handle
	lpdf = &szDFileName[0]; // pointer to file name buffer
	if( ( lpFN[0]                         ) ||	/* If we ALREADY have a file name, or */
		 ( GetFileName( lpFN, IDS_OPENDLG) ) )    /* If the USER picks one ... */
	{	/* Then we will assume it is a DIB File ... */
		if( lpdf != lpFN )
    		strcpy( lpdf, lpFN );
      typ = gmTypeOfFile(lpdf);     // try to determine the TYPE of file
		hD = GetDIB( lpdf, gd_OF );
#ifdef	WIN32
		if( hD )
		{
			SetReadPath( lpdf );
		}
#endif	// WIN32
   }
   return( hD );
}

// ===================================================
// void	OpenDIBFile2( PRDIB prd )
//
// Called from COMMON FILE OPEN
//
// Input: PRDIB structure
// Output: PRDIB->rd_hDIB has DIB ready to OPEN WINDOW
//
// ====================================================
void	OpenDIBFile2( PRDIB prd )
{
	HANDLE   hD;
	LPSTR	lpdf, lpFN;

	hD = 0;
	lpdf = &szDFileName[0];
	lpFN = prd->rd_pPath;
	if( ( lpFN && lpFN[0] ) ||	/* If we ALREADY have a file name, or */
		( GetFileName( lpFN, IDS_OPENDLG) ) )	/* If the USER picks one ... */
	{
		// Then we will assume it is a DIB File ...
		if( lpdf != lpFN )
			lstrcpy( lpdf, lpFN );
		prd->rd_Caller |= gd_OF;	// Add CALLER, and
		hD = GetDIB2( prd );	// Pass ON pointer
		//hD = GetDIB( lpdf, gd_OF );
#ifdef	WIN32
		if( hD )
		{
			SetReadPath( lpdf );
		}
#endif	// WIN32
	}
	if( hD )	// If returned
		prd->rd_hDIB = hD;

}

extern   DWORD	GetGType( LPTSTR lpf, LPTSTR lps, DWORD Siz );

DWORD	GetSizeMxIBuf( void )
{
	return( MXIBUF );
}

/*************************************************************************

  Function:  ReadDIBFile( PRDIB )
			which contains such things as -
			LPINT rd_pfHand, LPSTR rd_pFName, DWORD rd_dwFlag, ...

   Purpose:  Reads in the specified DIB file into a global chunk of
             memory.

   Returns:  A handle to a dib (hDIB) if successful.
             NULL if an error occurs.

  Comments:  BITMAPFILEHEADER is stripped off of the DIB.  Everything
             from the end of the BITMAPFILEHEADER structure on is
             returned in the global memory handle.

   History:   Date      Author      Reason

             6/1/91    Created
             6/27/91   Removed PM bitmap conversion routines.
             6/31/91   Removed logic which overallocated memory
                       (to account for bad display drivers).
            11/08/91   Again removed logic which overallocated
                       memory (it had creeped back in!)
				3 March 96	Added read of GIF and JPG ... Geoff.
				16 June 97 Upgraded to IJG Rev 6a (61) - WJPG2BMP
					or WJPEG32_2.DLL ...
*************************************************************************/
//#define  VF(h) 	( h && ( h != HFILE_ERROR ) )
#ifndef VFH
#define  VFH(h) ( h && ( h != INVALID_HANDLE_VALUE) )
#endif // VFH

BITMAPFILEHEADER	bmfHeader;
HANDLE ReadDIBFile( PRDIB prd )
{
	HANDLE *		lphFile;
	PSTR			lpf;
	//BITMAPFILEHEADER	bmfHeader;
	DWORD			dwBitsSize;
	HANDLE		hDIB;
	LPTSTR		lpDIB;
	HANDLE		hFile;
	BOOL			flg, flg2;
	DWORD			ddBmp;
	DWORD			ArgC;
#ifdef	ADDWJPEG
#ifndef	WJPEG4
	LPSTR			lpArgv[6];
	char			szTemp[260];
	char			szTemp2[260];
	LPSTR			lpo;
	LPSTR			lpi;
#endif	// !WJPEG4
#ifdef	Dv16_App
	int				ret = 0;
#else	// !Dv16_App
#ifdef	WJPEG5
	int				ret;
#endif	// WJPGE5
#endif	// Dv16_App y/n
	OFSTRUCT		ofs;
	LPSTR			lpe;
#endif	// ADDWJPEG
	DWORD			dwread;

	lphFile = prd->rd_pfHand;  // get POINTER to FILE HANDLE
	lpf     = prd->rd_pPath;   // get ptr to FULL FILE NAME

	flg = FALSE;
#if defined(ADDWJPEG) && !defined(WJPEG4)
	lpo = 0;
	lpi = 0;
#endif	// !WJPEG4
#ifdef	ADDWJPEG
	hgGIF = 0;
	lpGIF = 0;
	hgINF = 0;
	lpINF = 0;
	ddBmp = 0;
#endif // #ifdef	ADDWJPEG

	hDIB  = 0;	/* No HANDLE yet ... */
	flg2  = TRUE;

	hIBuf = DVGlobalAlloc( GHND, GetSizeMxIBuf() ); 
	if(hIBuf)
		lpIBuf = DVGlobalLock( hIBuf );
	else
		lpIBuf = 0;

   // get length of FILE in bytes for use when reading
	hFile = *lphFile;
	if( VFH(hFile) )
	{
//		dwBitsSize = _filelength (hFile);
//		if( dwBitsSize == (DWORD) -1 )
		dwBitsSize = DVFileSize( hFile );
      if( lpIBuf )
		{
		   sprintf( lpIBuf, pRFilSiz, dwBitsSize );
			AddFN( lpIBuf, lpf );
		}
		if(( dwBitsSize  < MINFILE    ) || // Is file length LT arbit. min.? 16
         ( dwBitsSize == (DWORD) -1 ) )  // or is more than 0xFFFFFFFE
		{
			DVlclose( hFile );
			if( dwBitsSize == 0 )
			{
				DIBError( ERR_NULSIZE );	// ERROR: File is ZERO Length!
				goto ReadDRet;
			}
			else
			{
				HFILE hf = _lopen(lpf, 0);
				hFile = IntToPtr(hf);
				if( VFH(hFile) )
				{
					dwBitsSize = DVFileSize( hFile );
				}
				if( (dwBitsSize < MINFILE) || (dwBitsSize == (DWORD) -1) )
				{
      			if( lpIBuf && (dwBitsSize < MINFILE))
		      	{
				      sprintf( lpIBuf, pRFilSiz2, dwBitsSize, (WORD) MINFILE );
				      AddFN( lpIBuf, lpf );
      				DIBError2( ERR_UNKNOWNF, lpIBuf );
               }
               else
               {
   					DIBError( ERR_UNKNOWNF );
			      }
					goto ReadDRet;
				}
            *lphFile = hFile; // maybe diff handle
			}
		}

   // Go read the DIB file header and check if it's valid.
		FType = ft_BMP;	/* Assume reading a BMP file ... */

#ifdef	WIN32

		if( (IS_WIN95) || (IS_NT) )
		{
//BOOL ReadFile(
//    HANDLE hFile,	// handle of file to read 
//    LPVOID lpBuffer,	// address of buffer that receives
//data  
//    DWORD nNumberOfBytesToRead,	// number of bytes to read 
//    LPDWORD lpNumberOfBytesRead,	// address of number of
//bytes read 
//    LPOVERLAPPED lpOverlapped 	// address of structure for
//data 
////   );	
			flg2 = ReadFile( (HANDLE)hFile, (LPVOID) &bmfHeader, sizeof( bmfHeader ),
				&ddBmp, NULL );
			ArgC = ddBmp;
		}
		else
		{
			HFILE hf = PtrToInt(hFile);
			ArgC = _lread( hf, (LPSTR) &bmfHeader, sizeof (bmfHeader) );
		}
#else	// !WIN#@
		ArgC = _lread( hFile, (LPSTR) &bmfHeader, sizeof (bmfHeader) );
#endif	// WIN32 y/n
		if(( ArgC != sizeof( bmfHeader )           ) ||
			( bmfHeader.bfType != DIB_HEADER_MARKER ) )
		{
			// OK, we have established it is NOT BMP input ...
#ifdef	ADDWJPEG
// jpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpeg
			DVlclose( hFile );	/* Close the file ... Physically and */
			*lphFile = 0;		/* logically ... */
         hFile = 0;  // and locally in this service
/* If NOT BMP, try to establish WHAT we are reading ... */
			lpe = (LPSTR) pRDefExt;
			lpDIB = (LPSTR) &bmfHeader;	/* Get pointer to HEADER */
//			FType = GetGType( lpf, lpDIB, sizeof(bmfHeader) );
			FType = GetGType( lpf, lpDIB, ArgC );

#ifndef	WJPEG4
			if( lpIBuf )
				lpIBuf[0] = 0;
			ArgC = 0;
			lpArgv[ArgC++] = lpIBuf;	/* Error message buffer ... */
#endif	// WJPEG4

			switch( FType )
			{
				case ft_PPM:
				{
//					lpe = &szJpg[0];	/* First ouptut is JPG ... */
//					lpArgv[ArgC++] = (LPSTR) pRSwPPM;
//					break;
				}
				case ft_GIF:
				{
					lpe = &szJpg[0];
//					lpArgv[ArgC++] = (LPSTR) pRSwGIF;
					break;
				}
				case ft_RLE:
				{
					DIBError( ERR_NO_RLE );
					goto ReadDRet;
					break;
				}
				case ft_TARGA:		/* ONLY for TARGA is a SWITCH required ... */
				{
					lpe = &szJpg[0];
#ifdef	WJPEG4
					DIBError( ERR_NO_RLE );	// NOT SUPPORTED
#else	// !WJPEG4
					lpArgv[ArgC++] = (LPSTR) pRSwTARGA;
#endif	// WJPEG4 y/n
					break;
				}
				case ft_JPG:
				{
//					lpe = (LPSTR) pRDefExt;	/* This has already been set ... */
					break;	/* Nothing to do here ... */
				}

				case ft_TIF:
               {
//	{ ERR_NO_RLE              ,"ERROR: Unsupported file format! "},
                  lpe = szTif;   // get default ending
#ifdef   ADDTIFF6
                  hDIB = LoadTIFImage( prd );
                  if( !hDIB )
#endif   // ADDTIFF6
                  {

                     if( lpIBuf )
                        DIBError2( ERR_NO_RLE, lpIBuf );
                     else
                        DIBError( ERR_NO_RLE );
                  }
	   				goto ReadDRet;
               }
					break;
#ifdef   ADD_SGI_RGB_READ
            case ft_RGB:
               {
#define  ERR_NO_RGB     ERR_NO_RLE  // unsupported format
                  extern HANDLE LoadRGBImage( PRDIB prd );
                  lpe = szRgb;
                  hDIB = LoadRGBImage( prd );
                  if(!hDIB) {
                     if(lpIBuf)
                        DIBError2( ERR_NO_RGB, lpIBuf );
                     else
                        DIBError( ERR_NO_RGB );
                  }
   				   goto ReadDRet;
               }
               break;
#endif   // #ifdef   ADD_SGI_RGB_READ

				case ft_Undef:
				{
					if( lpIBuf )
						AddFN( lpIBuf, lpf );
					if( lpIBuf && lpIBuf[0] )
  						DIBError2( ERR_UNKNOWNF, lpIBuf );
					else
						DIBError( ERR_UNKNOWNF );
					goto ReadDRet;
					break;
				}
				default:
				{
					if( lpIBuf )
						AddFN( lpIBuf, lpf );
					if( lpIBuf && lpIBuf[0] )
  						DIBError2( ERR_UNKNOWNF, lpIBuf );
					else
						DIBError( ERR_UNKNOWNF );
					goto ReadDRet;
					break;
				}
			}

#ifndef	WJPEG4
		/* NOTE: It is NOT a BMP type file! Thus we use the JPEG library */
		/* code to attempt to load it, and WRITE OUT a BMP type file IF */
		/* a JPEG file, else we have TWO STEPS ... */
		/* 1. Load supported file and write a JPEG */
		/* 2. Read the JPEG, and write a BMP ... */
		/* ============================================================= */
			lpArgv[ArgC++] = lpf;	/* INPUT file parameter ... */
// From WJPEG4 no output NAME could be passes to the Library,
// only GOBJECT handles ...

			lpo = &szTemp[0];
			DVGetFPath( lpf, gszDrive, gszDir, gszFname, gszExt ); /* Get components */
			lstrcpy( lpo, gszDrive );
			lstrcat( lpo, gszDir );
//			lstrcat( lpo, szFname );
			lstrcat( lpo, pRTmpWork );
			lstrcat( lpo, lpe );	/* Add BMP if JPG, else some OUTPUT extent... */
			if( !FindNewName( lpo ) )	/* Ensure UNIQUE name created ... */
			{
				DIBError( ERR_INTERROR );
				lpo = 0;
				goto ReadDRet;
			}
			lpArgv[ArgC++] = lpo;	/* OUTPUT file parameter ... */
			lpArgv[ArgC] = 0;
#endif	// !WJPEG4
			if( (FType == ft_PPM ) ||	/* Input is maybe PPM ... */
				( FType == ft_GIF ) ||
				( FType == ft_RLE ) ||	/* Not presently supported ... */
				( FType == ft_TARGA ) )	/* if one of these, then 2 steps ... */
			{
#ifdef	LDYNALIB
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= |
// #define	COMBWLIB	// LINKED with LIBRARY CODE
// NOTE: This is ONLY supplied as a Preprocessor FLAG
// ==================================================
#ifndef	COMBWLIB
				if( !GetJPEGLib(1) )
				{
					if( !fShownNOLIB )
					{
						UINT	ui;
						fShownNOLIB = TRUE;
						//DIBError( ERR_NO_LIB );
						ui = Err_No_Lib();
					}
					goto ReadDRet;
				}
#endif	// !COMBWLIB
				if( FType == ft_GIF )
				{
#ifndef	WJPEG4
					lpi = 0;
#ifndef	PROBGOBJ
					lpo = 0;
#endif	// PROBGOBJ
#endif	// !WJPEG4
// FIX980501 ( hgGIF = DVGlobalAlloc( GHND | GMEM_SHARE, dwBitsSize ) ) &&
					if( ( hFile = DVOpenFile( lpf, &ofs, OF_READ ) ) &&
						( hFile != (HANDLE)-1 ) &&
						( dwBitsSize = DVFileSize( hFile ) ) &&
						( hgGIF = DVGAlloc( lpf, (GHND | GMEM_SHARE), DWSIZE(dwBitsSize) ) ) &&
						( lpGIF = DVGlobalLock( hgGIF ) ) &&
						( DVRead( hFile, lpGIF, dwBitsSize ) == dwBitsSize ) )
					{
//#ifdef	PROBGOBJ
#ifdef	USENEWAPI
						DVlclose( hFile );	/* File LOADED - Physically CLOSE IT */
						hFile = 0;	/* And Logically as well ... */
#ifdef	USEGFUNC5
						DVGlobalUnlock( hgGIF );
						lpGIF = 0;
#ifdef	USEGFUNC6
						ArgC = GetGIFCountExt( hgGIF, dwBitsSize, prd );
#else	// !USEGFUNC6
						ArgC = GetGIFCount( hgGIF, dwBitsSize, prd );
#endif	// USEGFUNC6 y/n
						goto ReadDRet;
#endif	/* USEGFUNC5 */
						//if( ddBmp = (*WGifSize) ( hgGIF, dwBitsSize, &dibSize ) )
						if( ddBmp = DVWGifSize( hgGIF, dwBitsSize, &dibSize ) )
						{
							// Allocate memory for DIB
							hDIB = DVGlobalAlloc( GMEM_SHARE, ddBmp );
							if( hDIB == 0 )
							{
								DIBError( ERR_MEMORY );
								goto ReadDRet;
							}
							//flg = (*WGif2Bmp) ( hgGIF, dwBitsSize, hIBuf, hDIB );
							flg = DVWGif2Bmp( hgGIF, dwBitsSize, hIBuf, hDIB );

							DVGlobalUnlock( hgGIF );
							DVGlobalFree( hgGIF );
							lpGIF = 0;
							hgGIF = 0;
							if( flg )
							{	
								DVGlobalFree( hDIB );
								hDIB = 0;
								if( lpIBuf )
									AddFN( lpIBuf, lpf );
								if( lpIBuf && lpIBuf[0] )
									DIBError2( ERR_NOLCONV, lpIBuf );
								else
									DIBError( ERR_NOLCONV );
							}
							goto ReadDRet;
						}
						else
						{
			      			DIBError( ERR_NOLSIZEG );
      						goto ReadDRet;
						}
//#endif	// USENEWAPI
#else	// !USENEWAPI
//						hDIB = (*WGifToBmp) ( hgGIF, dwBitsSize, hIBuf, lpo );
						hDIB = DVWGifToBmp( hgGIF, dwBitsSize, hIBuf, lpo );
//#else	// !PROBGOBJ
//						hDIB = (*WGifToBmp) ( hgGIF, dwBitsSize, hIBuf, NULL );
//#endif	// PROBGOBJ y/n
						DVGlobalUnlock( hgGIF );
						DVGlobalFree( hgGIF );
						lpGIF = 0;
						hgGIF = 0;
						if( hDIB == 0 )
						{
							if( lpIBuf ) AddFN( lpIBuf, lpf );
							if( lpIBuf && lpIBuf[0] )
			      				DIBError2( ERR_NOLCONV, lpIBuf );
							else
								DIBError( ERR_NOLCONV );
      						goto ReadDRet;
						}
						else
						{
#ifdef	PROBGOBJ
							goto Opn_BMP;
#endif	// PROBGOBJ
						}	
#endif	// USENEWAPI
					}
					else
					{
						DIBError( ERR_NO_REOPEN );
					}
					goto ReadDRet;
				}	// end of if( FType == ftGIF
#ifdef	NEED_TWO_FILE
				if( ret = (*WWriteJpeg) (ArgC, &lpArgv[0] ) )
				{
					if( fShwNoConv )
					{
						if( lpIBuf && lpIBuf[0] )
							DIBError2( ERR_NO_CONV, lpIBuf );
						else
							DIBError( ERR_NO_CONV );
					}
					goto ReadDRet;
				}
#else	// !NEED_TWO_FILE
				if( fNeedTwo )
				{
					if( fShwNoConv )
					{
						if( lpIBuf && lpIBuf[0] )
							DIBError2( ERR_NO_CONV, lpIBuf );
						else
							DIBError( ERR_NO_CONV );
					}
					goto ReadDRet;
				}
#endif	// NEED_TWO_FILE

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= |
#else	// !LDYNALIB - ie STATIC link with LIBRARY
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= |
// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// Release of GIFSize and fetch functions
#ifdef	WJPEG4
				if( FType == ft_GIF )
				{
				   if( (hFile = DVOpenFile( lpf, &ofs, OF_READ)) &&
						(hFile != -1 ) &&
//						( dwBitsSize = _filelength (hFile) ) &&
						( dwBitsSize = DVFileSize( hFile ) ) &&
						( hgGIF = DVGlobalAlloc( GHND | GMEM_SHARE, dwBitsSize ) ) &&
						( lpGIF = DVGlobalLock( hgGIF ) ) &&
						( DVRead( hFile, lpGIF, dwBitsSize ) == dwBitsSize ) )
					{
						DVlclose( hFile );	/* File LOADED - Physically CLOSE IT */
						hFile = 0;	/* And Logically as well ... */
						DVGlobalUnlock( hgGIF );
						lpGIF = 0;
						ArgC = GetGIFCountExt( hgGIF, dwBitsSize, prd );
						goto ReadDRet;
				   }
				}
#endif	// WJPEG4
// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
#ifdef	NEED_TWO_FILE
				if( ret = WWRITEJPEG( ArgC, &lpArgv[0] ) )
				{
					if( fShwNoConv )
					{
						if( lpIBuf && lpIBuf[0] )
							DIBError2( ERR_NO_CONV, lpIBuf );
						else
							DIBError( ERR_NO_CONV );
					}
					goto ReadDRet;
				}
#else	// !NEED_TWO_FILE
				if( fNeedTwo )
				{
					if( fShwNoConv )
					{
						if( lpIBuf && lpIBuf[0] )
							DIBError2( ERR_NO_CONV, lpIBuf );
						else
							DIBError( ERR_NO_CONV );
					}
					goto ReadDRet;
				}

#endif	// NEED_TWO_FILE

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= |
#endif	// LDYNALIB y/n
#ifndef	WJPEG4	// Retire ALL this
// === retired =====================================
//				{
//					if( fShwNoConv )
//					{
//						if( lpIBuf && lpIBuf[0] )
//							DIBError2( ERR_NO_CONV, lpIBuf );
//						else
//							DIBError( ERR_NO_CONV );
//					}
//					goto ReadDRet;
//				}
				if( lpIBuf && lpIBuf[0] )
					DIBError2( ERR_WN_CONV, lpIBuf );
				/* OK, we assume we have WRITTEN a JPEG file ... */
				/* Now to convert JPG to BMP ... */
				ArgC = 0;
				lpArgv[ArgC++] = lpIBuf;	/* Error message buffer ... */
				lpi = lpo;
				lpArgv[ArgC++] = lpi;	/* JPG Output is now INPUT */
				lpe = (LPSTR) pRDefExt;	/* Ouput is BMP ... */
				lpo = &szTemp2[0];		/* Another buffer ... */
				lstrcpy( lpo, gszDrive );
				lstrcat( lpo, gszDir );
//				lstrcat( lpo, szFname );
				lstrcat( lpo, pRTmpWork );
				lstrcat( lpo, lpe );	/* Add BMP if JPG, else some OUTPUT extent... */
				if( !FindNewName( lpo ) )	/* Ensure UNIQUE name created ... */
				{
      			DIBError( ERR_INTERROR );
					lpi = 0;
					goto ReadDRet;
				}
				lpArgv[ArgC++] = lpo;	/* Set the BMP output ... */
				lpArgv[ArgC] = 0;
// === retired =====================================
#endif	// !WJPEG4
			}
#ifdef	LDYNALIB
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= |
			if( !GetJPEGLib(2) )
			{
				if( !fShownNOLIB )
				{
					UINT	ui;
					fShownNOLIB = TRUE;
					//DIBError( ERR_NO_LIB );
					ui = Err_No_Lib();
				}
				goto ReadDRet;
			}
#ifdef	USEGFUNC1
//					( dwBitsSize = _filelength (hFile) ) 
				   if( (hFile = DVOpenFile( lpf, &ofs, OF_READ)) &&
						(hFile != (HANDLE)-1 ) &&
						( dwBitsSize = DVFileSize( hFile ) ) &&
						( dwBitsSize != (DWORD)-1) &&
						( hgGIF = DVGlobalAlloc( GHND | GMEM_SHARE, dwBitsSize ) ) &&
						( lpGIF = DVGlobalLock( hgGIF ) ) &&
						( DVRead( hFile, lpGIF, dwBitsSize ) == dwBitsSize ) )
					{
//#ifdef	PROBGOBJ
#if	(defined( USENEWAPI ) && defined( USEGFUNC2 ))
//						if( ddBmp = (*WJpgSize) ( hgGIF, dwBitsSize, &dibSize ) )
						if( ddBmp = DVWJpgSize( hgGIF, dwBitsSize, &dibSize ) )
						{
//Got_SizeJ:
							// Allocate memory for DIB
							hDIB = DVGlobalAlloc( GMEM_SHARE, ddBmp );
							if( hDIB == 0 )
							{
								DIBError( ERR_MEMORY );
								goto ReadDRet;
							}
							//	flg = (*WJpg2Bmp) ( hgGIF, dwBitsSize, hIBuf, hDIB );
							flg = DVWJpg2Bmp( hgGIF, dwBitsSize, hIBuf, hDIB );
							DVGlobalUnlock( hgGIF );
							DVGlobalFree( hgGIF );
							lpGIF = 0;
							hgGIF = 0;
							if( flg )
							{
								DVGlobalFree( hDIB );
								hDIB = 0;
								if( lpIBuf )
									AddFN( lpIBuf, lpf );
								if( lpIBuf && lpIBuf[0] )
									DIBError2( ERR_NOLCONV, lpIBuf );
								else
									DIBError( ERR_NOLCONV );
							}
							goto ReadDRet;
						}
						else
						{
							// Use the ALTERNATIVE JPEG 6a (61)
							// ================================
							if( ddBmp = DVWJpgSize6( hgGIF, dwBitsSize, &dibSize ) )
							{
								// goto Got_SizeJ;
								// Allocate memory for DIB
								hDIB = DVGlobalAlloc( GMEM_SHARE, ddBmp );
								if( hDIB == 0 )
								{
									DIBError( ERR_MEMORY );
									goto ReadDRet;
								}
								//	flg = (*WJpg2Bmp) ( hgGIF, dwBitsSize, hIBuf, hDIB );
								// 	flg = DVWJpg2Bmp( hgGIF, dwBitsSize, hIBuf, hDIB );
								flg = DVWJpg2Bmp6( hgGIF, dwBitsSize, hIBuf, hDIB );
								DVGlobalUnlock( hgGIF );
								DVGlobalFree( hgGIF );
								lpGIF = 0;
								hgGIF = 0;
								if( flg )
								{
									DVGlobalFree( hDIB );
									hDIB = 0;
									if( lpIBuf ) AddFN( lpIBuf, lpf );
									if( lpIBuf && lpIBuf[0] )
										DIBError2( ERR_NOLCONV, lpIBuf );
									else
										DIBError( ERR_NOLCONV );
								}
								goto ReadDRet;
							}
							else
							{
								DIBError( ERR_NOLSIZEJ );
								goto ReadDRet;
							}
						}
#else	// !(USENEWAPI and USEGFUNC2)
//#endif	// USENEWAPI and USEGFUNC2
						//hDIB = (*WJpgToBmp) ( hgGIF, dwBitsSize, hIBuf, lpo );
						hDIB = DVWJpgToBmp( hgGIF, dwBitsSize, hIBuf, lpo );
//#else	// !PROBGOBJ
//						hDIB = (*WJpgToBmp) ( hgGIF, dwBitsSize, hIBuf, NULL );
//#endif	// PROBGOBJ y/n
						DVGlobalUnlock( hgGIF );
						DVGlobalFree( hgGIF );
						lpGIF = 0;
						hgGIF = 0;
						if( hDIB == 0 )
						{
							if( lpIBuf ) AddFN( lpIBuf, lpf );
							if( lpIBuf && lpIBuf[0] )
			      			DIBError2( ERR_NOLCONV, lpIBuf );
							else
								DIBError( ERR_NOLCONV );
      						goto ReadDRet;
						}
						else
						{
#ifdef	PROBGOBJ
							goto Opn_BMP;
#endif	/* PROBGOBJ */
						}	
#endif	// USENEWAPI and USEGFUNC2 y/n
					}
					else
					{
						DIBError( ERR_NO_REOPEN );
     					goto ReadDRet;
					}
//			ret = 0;
//			if( ret )
//			{
//				if( fShwNoConv )
//				{
//					if( lpIBuf && lpIBuf[0] )
//						DIBError2( ERR_NO_CONV, lpIBuf );
//					else
//						DIBError( ERR_NO_CONV );
//				}
//				goto ReadDRet;
//			}
			// Should have LEFT this mess by now ... ie ReadDRet
			// BUT if NOT, show and Error if a buffer...
			if( lpIBuf && lpIBuf[0] )
			{
	      		DIBError2( ERR_WN_CONV, lpIBuf );
			}
#else	// !USEGFUNC1
			if( ret = (*WReadJpeg) (ArgC, &lpArgv[0]) )
			{
				if( fShwNoConv )
				{
					if( lpIBuf && lpIBuf[0] )
						DIBError2( ERR_NO_CONV, lpIBuf );
					else
						DIBError( ERR_NO_CONV );
				}
				goto ReadDRet;
			}
			else if( lpIBuf && lpIBuf[0] )
			{
	      		DIBError2( ERR_WN_CONV, lpIBuf );
			}
#endif	// USEGFUNC1 y/n
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= |
#else	// !LDYNALIB
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= |
#ifndef	WJPEG4
			// Retired service ...
			if( ret = WREADJPEG( ArgC, &lpArgv[0] ) )
			{
				if( fShwNoConv )
				{
					if( lpIBuf && lpIBuf[0] )
						DIBError2( ERR_NO_CONV, lpIBuf );
					else
						DIBError( ERR_NO_CONV );
				}
				goto ReadDRet;
			}
#endif	// !WJPEG4	- April 97 release ...
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= |
#endif	// LDYNALIB y/n
		/* ============================================================= */
		/* We seem to have SUCCESSFULLY read this particular INPUT, and */
		/* WRITTEN the data to a BMP type file. Now we USE this (temporary) */
		/* BMP file ... so ... see if we can OPEN it ... */
#if	(defined( PROBGOBJ ) && defined( LDYNALIB ))
#ifdef	WJPEG4
#ifdef	ADDOLDTO
#ifndef	USENEWAPI
Opn_BMP:
#endif	// !USENEWAPI
#endif	// ADDOLDTO
#else	// !WJPEG4
Opn_BMP:
#endif	// WJPEG4 y/n
#endif	// PROBGOBJ & LDYNALIB
// **************************************************************
#ifdef	USEGFUNC1
			if( ( hFile = DVOpenFile( lpf, &ofs, OF_READ) ) &&
				( hFile != (HANDLE)-1 ) &&
				( dwBitsSize = DVFileSize( hFile ) ) &&
				( dwBitsSize != (DWORD)-1) &&
				( hgGIF = DVGlobalAlloc( GHND | GMEM_SHARE, dwBitsSize ) ) &&
				( lpGIF = DVGlobalLock( hgGIF ) ) &&
				( DVRead( hFile, lpGIF, dwBitsSize ) == dwBitsSize ) )
			{
#if	(defined( USENEWAPI ) && defined( USEGFUNC2 ))
				if( ddBmp = DVWJpgSize( hgGIF, dwBitsSize, &dibSize ) )
				{
// Got_Size - Got Size
					// Allocate memory for DIB
					hDIB = DVGlobalAlloc( GMEM_SHARE, ddBmp );
					if( hDIB == 0 )
					{
						DIBError( ERR_MEMORY );
						goto ReadDRet;
					}
					flg = DVWJpg2Bmp( hgGIF, dwBitsSize, hIBuf, hDIB );
					DVGlobalUnlock( hgGIF );
					DVGlobalFree( hgGIF );
					lpGIF = 0;
					hgGIF = 0;
					if( flg )
					{
						DVGlobalFree( hDIB );
						hDIB = 0;
						if( lpIBuf )
							AddFN( lpIBuf, lpf );
						if( lpIBuf && lpIBuf[0] )
							DIBError2( ERR_NOLCONV, lpIBuf );
						else
							DIBError( ERR_NOLCONV );
					}
					goto ReadDRet;
				}
				else
				{
					if( ddBmp = DVWJpgSize6( hgGIF, dwBitsSize, &dibSize ) )
					{
						// Allocate memory for DIB
						hDIB = DVGlobalAlloc( GMEM_SHARE, ddBmp );
						if( hDIB == 0 )
						{
							DIBError( ERR_MEMORY );
							goto ReadDRet;
						}
						flg = DVWJpg2Bmp6( hgGIF, dwBitsSize, hIBuf, hDIB );
						DVGlobalUnlock( hgGIF );
						DVGlobalFree( hgGIF );
						lpGIF = 0;
						hgGIF = 0;
						if( flg )
						{
							DVGlobalFree( hDIB );
							hDIB = 0;
							if( lpIBuf )
								AddFN( lpIBuf, lpf );
							if( lpIBuf && lpIBuf[0] )
								DIBError2( ERR_NOLCONV, lpIBuf );
							else
								DIBError( ERR_NOLCONV );
						}
						goto ReadDRet;
//						goto Got_Size;
					}
					else
					{
						DIBError( ERR_NOLSIZEJ );
						goto ReadDRet;
					}
				}
#endif	// (USENEWAPI and USEGFUNC2)
			}
			else
			{
				DIBError( ERR_NO_CONV );
			}
#else	// !USEGFUNC1
// **************************************************************
			if( (hFile = DVOpenFile( lpo, &ofs, OF_READ)) == -1 )
			{
	      		DIBError( ERR_NO_CONV );
		  		goto ReadDRet;
			}
			*lphFile = hFile;	/* Put in the (maybe) NEW handle ... */
//			dwBitsSize = _filelength( hFile ); /* Get this files size ... */
			dwBitsSize = DVFileSize( hFile ); /* Get this files size ... */
			if( (_lread (hFile, (LPSTR) &bmfHeader, sizeof (bmfHeader)) !=
				sizeof (bmfHeader)) ||
				(bmfHeader.bfType != DIB_HEADER_MARKER))
			{
		      DIBError( ERR_NO_CONV );
      		goto ReadDRet;
			}
			flg = TRUE;
		/* ============================================================= */
		/* Now we CONTINUE with the load of this NEW BMP type file ... */
// **************************************************************
#endif	// USEGFUNC1 y/n
// jpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpeg
#else	// !ADDWJPEG
// jpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpeg
      DIBError (ERR_NOT_DIB);
      return NULL;
// jpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpegjpeg
#endif	// ADDWJPEG y/n
		}	// Is a DIB Header file
		// ===========================================
		// IF we didn't enter the above MESS, then the
		// BITMAPFILEHEADER was VALID, so proceed.
		// Allocate memory for DIB (hDIB) and READ in
		// the file. Simple!
		// ===========================================
// FIX980501 hDIB = DVGlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, dwBitsSize - sizeof(BITMAPFILEHEADER));
		/* NOTE: Allocation size is FULL FILE SIZE */
		/*		 minus the BITMAPFILEHEADER        */
		dwread = dwBitsSize - sizeof(BITMAPFILEHEADER);
		hDIB = DVGAlloc( lpf, (GMEM_MOVEABLE | GMEM_ZEROINIT), DWSIZE(dwread) );
		if( hDIB == 0 )
		{
			DIBError( ERR_MEMORY );
			goto ReadDRet;
		}

		lpDIB = DVGlobalLock( hDIB ); // LOCK DIB HANDLE
		if( lpDIB == 0 )
		{
         DVGlobalFree(hDIB);     // LOCK DIB HANDLE FAILED, so free memory allocated
         hDIB = 0;
			DIBError( ERR_MEMORY );
			goto ReadDRet;
		}

		// Go read the bits.
//		if( !MyRead( hFile, lpDIB, dwBitsSize - sizeof(BITMAPFILEHEADER)) )
		if( !MyRead( hFile, lpDIB, dwread) )
		{
			DVGlobalUnlock (hDIB);  // UNLOCK DIB HANDLE - for read FAILED
			DVGlobalFree   (hDIB);
			DIBError (ERR_READ);
			hDIB = 0;
			goto ReadDRet;
		}

		/* successful read of Device Independent Bitmap (DIB) */
		DVGlobalUnlock (hDIB);  // UNLOCK DIB HANDLE - for successful read

	}
	else
	{
		hDIB = 0;
		DIBError( ERR_READ );
	}

ReadDRet:

	if( gfBe_Tidy )
	{
#if 0 // 00000 defined(ADDWJPEG) && !defined(WJPEG4) 000
		if( lpo && (lpo[0]) )
		{
			DVOpenFile( lpo, &ofs, OF_DELETE);
		}
		if( lpi && (lpi[0]) )
		{
			DVOpenFile( lpi, &ofs, OF_DELETE);
		}
#endif	// 000000 !WJPEG4 0000
#ifdef	LDYNALIB
#ifndef	CVLIBBUG
		CloseJPEGLib();	/* If CVLIBBUG then only called from FRAME.c at exit */
#endif	// !CVLIBBUG
#endif	// LDYNALIB
	}

#ifdef	ADDWJPEG
	if( hgGIF && lpGIF )
		DVGlobalUnlock( hgGIF );
	if( hgGIF )
		DVGlobalFree( hgGIF );
	if( hgINF && lpINF )
		DVGlobalUnlock( hgINF );
	if( hgINF )
		DVGlobalFree( hgINF );
	hgGIF = 0;
	lpGIF = 0;
	hgINF = 0;
	lpINF = 0;
#endif // #ifdef	ADDWJPEG

	if( VFH(hFile) )
	{
		DVlclose( hFile );	/* Close the file ... Physically and */
	}
	*lphFile = 0;		/* logically ... */
	if( hIBuf && lpIBuf )
		DVGlobalUnlock( hIBuf );
	if( hIBuf )
		DVGlobalFree( hIBuf );
	hIBuf = 0;
	lpIBuf = 0;
	// RETURN Handle or NULL if failed ...
	// but NOTE that some DIB Handles are returned in
	// the PRDIB structure, and not directly.
	return( hDIB );
}	// end - ReadDIBFile( PRDIB prd )

/* ************************************************************************

  Function:  MyRead (HANDLE, LPSTR, DWORD)

   Purpose:  Routine to read files greater than 64K in size.

   Returns:  TRUE if successful.
             FALSE if an error occurs.

  Comments:

   History:   Date     Reason

             6/1/91    Created
			Nov 14, 1996 If WIN32 then use DVRead( ... )

************************************************************************ */

BOOL MyRead( HANDLE hFile, LPSTR lpBuffer, DWORD dwSize )
{
	if( DVRead( hFile, lpBuffer, dwSize ) != dwSize )
		return( FALSE );
	return( TRUE );
}

/* **************************************************************

  Function:  GetDibInfo ( LPSTR, LPINFOSTRUCT, BOOL)

   Purpose:  Retrieves the INFOSTRUCT specifications about the selected
             file when the "Dib Information..." button is selected.

   Returns:  TRUE if successful
             FALSE if an error occurs.

  Comments:  This routine will handle both Windows and PM bitmaps.
             If an PM bitmap is found, NULL is returned for the
             compression type.

   History:   Date     Reason

             6/27/91   Created
             26Nov96	Expand to CONVERT JPG or GIF to BMP QUICKLY!!!

***************************************************************** */
HANDLE DoJpgToBmp( LPSTR, LPINFOSTRUCT );

int GetDibInfo ( LPSTR pszFile, LPINFOSTRUCT info, BOOL fErr )
{
	BITMAPFILEHEADER   bmfHeader;
	BITMAPINFOHEADER   DIBHeader;
	DWORD              dwHeaderSize;
	HANDLE             hFile;
	OFSTRUCT           ofstruct;
	BITMAPCOREHEADER   bmCore;
	long               lFilePos;
	char               szBuffer[20];
	LPSTR              lpb;
	DWORD			   ft = ft_Undef;
	HFILE              hf;

	if( !pszFile[0] )
		return (int)0;

	// fill in filename into structure.
	lstrcpy ((LPSTR) info->is_szName, (LPSTR) pszFile);
	hFile = DVOpenFile( pszFile, &ofstruct, OF_READ | OF_SHARE_DENY_WRITE);
	if( !VFH(hFile) )
		return (int)0;

	// read the BITMAPFILEHEADER structure and check for BM marker
	hf = PtrToInt(hFile);
	if( (_lread (hf, (LPSTR) &bmfHeader, sizeof (bmfHeader)) !=
			sizeof (bmfHeader)) ||
			( bmfHeader.bfType != DIB_HEADER_MARKER ) )
	{
		if( fErr )
		{
			DIBError( ERR_NOT_DIB );
		}
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
		else
		{
			lpb = (LPSTR) &bmfHeader;	// Cast to a BYTE inspector
			ft = GetGType( pszFile, lpb, sizeof( bmfHeader ) );
			info->is_dwType = ft;
			if( (ft == ft_GIF) || (ft == ft_JPG) )
			{
				DVlclose( hFile );
				hFile = 0;
				if( ft == ft_JPG )
				{
					if( hFile = DoJpgToBmp( pszFile, info ) )
					{
						return 1;
					}
				}
			}
		}
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

		if( hFile )
			DVlclose(hFile);
		hFile = 0;
		return (int)0;
	}

	hf = PtrToInt(hFile);
	// ALL THIS to avoid _lseek, which SEEMS TO FAIL!!!
	lFilePos = sizeof( bmfHeader );
	if( _lread( hf, (LPSTR) &dwHeaderSize, sizeof( dwHeaderSize ) )
        != sizeof (dwHeaderSize) )
	{
		DIBError (ERR_NOT_DIB);
		DVlclose(hFile);
		return (int)0;
	}
	DVlclose( hFile );
	hFile = DVOpenFile((LPSTR) pszFile, &ofstruct, OF_READ | OF_SHARE_DENY_WRITE);
	if( !VFH(hFile) )
		return (int)0;
	hf = PtrToInt(hFile);
	// read the BITMAPFILEHEADER structure and check for BM marker
	if( _lread( hf, (LPSTR) &bmfHeader, sizeof (bmfHeader) ) != sizeof (bmfHeader) )
		return (int)0;

	info->is_dwType = ft_BMP;
	if( dwHeaderSize == sizeof (BITMAPCOREHEADER) )      // PM dib
	{
		_lread( hf, (LPSTR) &bmCore, sizeof (bmCore));
		LoadString( ghDvInst, IDS_PMBMP, szBuffer, sizeof(szBuffer));
		lstrcpy ((LPSTR)info->is_szType, (LPSTR) szBuffer);
		info->is_cbWidth  = bmCore.bcWidth;
		info->is_cbHeight = bmCore.bcHeight;
		info->is_cbColors = (DWORD)1L << (bmCore.bcBitCount);
		szBuffer[0]=0;
		lstrcpy ((LPSTR) info->is_szCompress, (LPSTR) szBuffer);
	}
	else if( dwHeaderSize == sizeof (BITMAPINFOHEADER) )  // windows dib
	{
		_lread( hf, (LPSTR) &DIBHeader, sizeof (DIBHeader));
		LoadString( ghDvInst, IDS_WINBMP, szBuffer, sizeof(szBuffer));
		lstrcpy ((LPSTR)info->is_szType, (LPSTR) szBuffer);
		info->is_cbWidth  = DIBHeader.biWidth;
		info->is_cbHeight = DIBHeader.biHeight;
		info->is_cbColors = (DWORD)1L << DIBHeader.biBitCount;
		switch( DIBHeader.biCompression )
		{
			case BI_RGB:
				LoadString( ghDvInst, IDS_RGB, szBuffer, sizeof(szBuffer));
				break;

			case BI_RLE4:
				LoadString( ghDvInst, IDS_RLE4, szBuffer, sizeof(szBuffer));
				break;

			case BI_RLE8:
				LoadString( ghDvInst, IDS_RLE8, szBuffer, sizeof(szBuffer));
				break;

			default:
				szBuffer[0]=0;
				break;
		}
		lstrcpy ((LPSTR) info->is_szCompress, (LPSTR) szBuffer);
	}
	else
	{
		DIBError( ERR_NOT_DIB );
		DVlclose(hFile);
		return (int)0;
	}
	DVlclose(hFile);
return 1;
}

void SetDIBInfo( LPINFOSTRUCT info,
				BITMAPINFOHEADER MLPTR pDIBHeader )
{
	char	szBuffer[20];
	if( info && pDIBHeader )
	{
				LoadString( ghDvInst, IDS_WINBMP, szBuffer, sizeof(szBuffer));
				lstrcpy ((LPSTR)info->is_szType, (LPSTR) szBuffer);
				info->is_cbWidth  = pDIBHeader->biWidth;
				info->is_cbHeight = pDIBHeader->biHeight;
				info->is_cbColors = (DWORD)1L << pDIBHeader->biBitCount;
				switch( pDIBHeader->biCompression )
				{
				case BI_RGB:
					LoadString( ghDvInst, IDS_RGB, szBuffer, sizeof(szBuffer));
					break;

				case BI_RLE4:
					LoadString( ghDvInst, IDS_RLE4, szBuffer, sizeof(szBuffer));
					break;

				case BI_RLE8:
					LoadString( ghDvInst, IDS_RLE8, szBuffer, sizeof(szBuffer));
					break;

				default:
					szBuffer[0]=0;
					break;

				}
				lstrcpy ((LPSTR) info->is_szCompress, (LPSTR) szBuffer);
	}
}

int GetDibInfo2( LPINFOSTRUCT info )
{
	int	i;
	HGLOBAL	hDIB;
	BITMAPINFOHEADER MLPTR pDIBHeader;
	i = 0;
	if( ( info ) && ( hDIB = info->is_hdlDib                       ) &&
		( pDIBHeader = (BITMAPINFOHEADER MLPTR)DVGlobalLock( hDIB ) ) )   // LOCK DIB HANDLE
	{
		SetDIBInfo( info, pDIBHeader );
		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE
		i = 1;
	}
	return( i );
}

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now - includes GIF

// ================================================
// HANDLE DoJpgToBmp( LPSTR lpf, LPINFOSTRUCT info )
//
// Call into a POINTER, if they are VALID
// ie WJpgSize && WJpg2Bmp must contain ADDRESSES
// of the LIBRARY ENTRY POINT!!!
// STATIC LOAD or DYNAMIC LOAD - Use only
// the 32-bit FAR PASCAL POINTERS
// ================================================

HANDLE DoJpgToBmp( LPSTR lpf, LPINFOSTRUCT info )
{
	HANDLE	hF, hFile;
	DWORD	dwBitsSize, ddBmp;
	HGLOBAL	hgGIF, hDIB;
	LPSTR	lpGIF;
	OFSTRUCT	ofs;
	BOOL	flg;
	BITMAPINFOHEADER MLPTR pDIBHeader;

	hF = 0;
	hFile = 0;
	hDIB = 0;
	lpGIF = 0;
	hgGIF = 0;
	if( LibLoaded2() )
//	if( LibLoaded2() )
//	if( WJpgSize && WJpg2Bmp )
	{
		if( (hFile = DVOpenFile( lpf, &ofs, OF_READ)) &&
			(hFile != (HANDLE)-1 ) &&
			( dwBitsSize = DVFileSize( hFile ) ) &&
			( hgGIF = DVGlobalAlloc( GHND | GMEM_SHARE, dwBitsSize ) ) &&
			( lpGIF = DVGlobalLock( hgGIF ) ) &&
			( DVRead( hFile, lpGIF, dwBitsSize ) == dwBitsSize ) )
		{
//			if( ddBmp = (*WJpgSize) ( hgGIF, dwBitsSize, &dibSize ) )
			if( ( ddBmp = DVWJpgSize( hgGIF, dwBitsSize, &dibSize ) ) ||
				( ddBmp = DVWJpgSize6( hgGIF, dwBitsSize, &dibSize ) ) )
			{
				// Allocate memory for DIB
				hDIB = DVGlobalAlloc( GMEM_SHARE, ddBmp );
				if( hDIB == 0 )
				{
					//DIBError( ERR_MEMORY );
					goto ReadJRet;
				}
			}
			//flg = (*WJpg2Bmp) ( hgGIF, dwBitsSize, hIBuf, hDIB );
			flg = DVWJpg2Bmp( hgGIF, dwBitsSize, hIBuf, hDIB );
			DVGlobalUnlock( hgGIF );
			DVGlobalFree( hgGIF );
			lpGIF = 0;
			hgGIF = 0;
			if( flg )
			{
				if( hDIB )
					DVGlobalFree( hDIB );
				hDIB = 0;
			}
			if( hDIB &&
				( pDIBHeader = (BITMAPINFOHEADER MLPTR)DVGlobalLock( hDIB ) ) ) // LOCK DIB HANDLE
			{
				hF = hFile;
				SetDIBInfo( info, pDIBHeader );
	
			}
		}
		else
		{
			DIBError( ERR_NO_REOPEN );
		}
	}

ReadJRet:

	if( VFH(hFile) )
		DVlclose( hFile );
	if( hgGIF && lpGIF )
		DVGlobalUnlock( hgGIF );
	if( hgGIF )
		DVGlobalFree( hgGIF );
	if( hDIB && pDIBHeader )
		DVGlobalUnlock( hDIB );    // UNLOCK DIB HANDLE
	if( hDIB )
		DVGlobalFree( hDIB );

	return( hF );

}

#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now - includes GIF

/*************************************************************************

  Function:  INFODLGPROC (HWND, UINT, WPARAM, LPARAM)

   Purpose:  Window procedure for the Dib Information dialog box.

   Returns:  TRUE if the message was processed.
             FALSE if the system needs to process the message.

  Comments:

   History:   Date     Reason

             6/27/91   Created

*************************************************************************/
#ifdef	WIN32
BOOL MLIBCALL INFODLGPROC (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
#else
BOOL MLIBCALL INFODLGPROC (HWND hDlg, WORD message, WORD wParam, LONG lParam)
#endif
{
  LPINFOSTRUCT pInfo;
  char         szBuffer[20];

  switch (message)
    {
    case WM_INITDIALOG:
        pInfo = (LPINFOSTRUCT) lParam;

        // Set the strings into the dialog box.

        SetDlgItemText(hDlg, IDD_NAME, (LPSTR) pInfo->is_szName);
        SetDlgItemText(hDlg, IDD_FORMAT, (LPSTR) pInfo->is_szType);
        wvsprintf ((LPSTR)szBuffer, (LPSTR) "%lu", (LPSTR) &pInfo->is_cbWidth);
        SetDlgItemText(hDlg, IDD_WIDTH, (LPSTR) szBuffer);
        wvsprintf ((LPSTR)szBuffer, (LPSTR) "%lu", (LPSTR) &pInfo->is_cbHeight);
        SetDlgItemText(hDlg, IDD_HEIGHT, (LPSTR) szBuffer);
        wvsprintf ((LPSTR)szBuffer, (LPSTR) "%lu", (LPSTR) &pInfo->is_cbColors);
        SetDlgItemText(hDlg, IDD_COLORS, (LPSTR) szBuffer);

        if (pInfo->is_szCompress[0] == 0)
           ShowWindow(GetDlgItem (hDlg, IDD_COMPHEAD), SW_HIDE);
        else
           SetDlgItemText(hDlg, IDD_COMPRESS, (LPSTR) pInfo->is_szCompress);

        return (TRUE);

    case WM_COMMAND:
       {
          DWORD   cmd = LOWORD(wParam);
          if( cmd == IDOK || cmd == IDCANCEL)
          {
             EndDialog(hDlg, TRUE);
             return (TRUE);
          }
          break;
       }
    }
  return (FALSE);
}

/* ************************************************************************

  Function:  VIEWDLGPROC (HWND, UINT, WPARAM, LPARAM)

   Purpose:  Window procedure for the Dib View dialog box.

   Returns:  TRUE if the message was processed.
             FALSE if the system needs to process the message.

  Comments:

   History:   Date     Reason

             14/02/96  Created

************************************************************************ */
LPINFOSTRUCT lpInfo;
#ifdef	WIN32
BOOL MLIBCALL VIEWDLGPROC (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
#else
BOOL MLIBCALL VIEWDLGPROC (HWND hDlg, WORD message, WORD wParam, LONG lParam)
#endif
{
  char         szBuffer[20];

  switch (message)
    {
    case WM_INITDIALOG:
        lpInfo = (LPINFOSTRUCT) lParam;

        // Set the strings into the dialog box.

        SetDlgItemText(hDlg, IDD_NAME, (LPSTR) lpInfo->is_szName);
        SetDlgItemText(hDlg, IDD_FORMAT, (LPSTR) lpInfo->is_szType);
        wvsprintf ((LPSTR)szBuffer, (LPSTR) "%lu", (LPSTR) &lpInfo->is_cbWidth);
        SetDlgItemText(hDlg, IDD_WIDTH, (LPSTR) szBuffer);
        wvsprintf ((LPSTR)szBuffer, (LPSTR) "%lu", (LPSTR) &lpInfo->is_cbHeight);
        SetDlgItemText(hDlg, IDD_HEIGHT, (LPSTR) szBuffer);
        wvsprintf ((LPSTR)szBuffer, (LPSTR) "%lu", (LPSTR) &lpInfo->is_cbColors);
        SetDlgItemText(hDlg, IDD_COLORS, (LPSTR) szBuffer);

        if( lpInfo->is_szCompress[0] == 0 )
           ShowWindow(GetDlgItem (hDlg, IDD_COMPHEAD), SW_HIDE);
        else
           SetDlgItemText(hDlg, IDD_COMPRESS, (LPSTR) lpInfo->is_szCompress);

        return (TRUE);

    case WM_COMMAND:
       {
          DWORD   cmd = LOWORD(wParam);
          if( cmd == IDOK || cmd == IDCANCEL)
          {
            EndDialog(hDlg, TRUE);
            return (TRUE);
          }
       }
       break;

    case WM_PAINT:
			if( lpInfo && lpInfo->is_hdlDib )
			{
				chkpv();
				DlgWndPaint( hDlg );
			}
			break;
    }
  return (FALSE);
}


/****************************************************************************

  FUNCTION   : WinDibFromBitmap()

  PURPOSE    : Will create a global memory block in DIB format that
		 represents the Device-dependent bitmap (DDB) passed in.

		      dwStyle -> DIB format     ==  RGB, RLE
		      wBits  -> Bits per pixel ==  1,4,8,24

  RETURNS    : A handle to the DIB

 ****************************************************************************/
HANDLE WinDibFromBitmap( HBITMAP hBitmap,
						DWORD dwStyle,
						WORD wBits, 
                        HPALETTE hPal )
{
	BITMAP               bm;
	BITMAPINFOHEADER     bi;
	BITMAPINFOHEADER MLPTR lpbi;
	DWORD                dwLen;
	HANDLE               hDIB;
	HANDLE               h;
	HDC                  hDC;
	WORD                 wB;

	if( !hBitmap )
		return NULL;

	if( hPal == NULL )
		hPal = GetStockObject( DEFAULT_PALETTE );

	GetObject( hBitmap, sizeof (bm), (LPSTR)&bm );

	if( wBits == 0 )
	{
		wB =  bm.bmPlanes * bm.bmBitsPixel;
	}
	else
	{
		wB = wBits;
	}

	// Condition the BIT size ...
	if( wB <= 1 )
		wB = 1;
	else if( wB <= 4 )
		wB = 4;
	else if( wB <= 8 )
		wB = 8;
	else
		wB = 24;

	bi.biSize               = sizeof (BITMAPINFOHEADER);
	bi.biWidth              = bm.bmWidth;
	bi.biHeight             = bm.bmHeight;
	bi.biPlanes             = 1;
	bi.biBitCount           = wB;
	bi.biCompression        = dwStyle;
	bi.biSizeImage          = 0;
	bi.biXPelsPerMeter      = 0;
	bi.biYPelsPerMeter      = 0;
	bi.biClrUsed            = 0;
	bi.biClrImportant       = 0;

	dwLen  = bi.biSize + PaletteSize( (LPSTR)&bi );
	hDIB = DVGlobalAlloc( GHND, dwLen );

	if( !hDIB )
		return( NULL );

	lpbi   = (VOID MLPTR)DVGlobalLock(hDIB);  // LOCK DIB HANDLE
   if( !lpbi )
      return( NULL );

	*lpbi  = bi;
	hDC    = GetDC (NULL);
	hPal   = SelectPalette (hDC, hPal, FALSE);
	RealizePalette(hDC);
	// call GetDIBits with a NULL lpBits param, so it
	// will calculate the biSizeImage field for us.
	GetDIBits( hDC,
		hBitmap,
		0,
		(WORD) bi.biHeight,
		NULL,
		(LPBITMAPINFO) lpbi,
		DIB_RGB_COLORS );

	bi = *lpbi;
	DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE

	// If the driver did not fill in the biSizeImage field,
	// make one up
	if( bi.biSizeImage == 0 )
	{
		bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * wBits) *
			bm.bmHeight;
		if( dwStyle != BI_RGB )
			bi.biSizeImage = (bi.biSizeImage * 3) / 2;
	}

	// Realloc the buffer big enough to hold all the bits.
	dwLen = bi.biSize + PaletteSize((LPSTR)&bi) + bi.biSizeImage;
	if( h = GlobalReAlloc( hDIB, dwLen, 0 ) )
	{
		hDIB = h;	// Return the HANDLE
	}
	else
	{
		DVGlobalFree( hDIB );
		hDIB = NULL;
		SelectPalette( hDC, hPal, FALSE );
		ReleaseDC( NULL, hDC );
		return( hDIB );
	}

	// call GetDIBits with a NON-NULL lpBits param,
	// and actualy get the bits this time.
	lpbi = (VOID MLPTR)DVGlobalLock(hDIB); // LOCK DIB HANDLE
   if( !lpbi )
      return NULL;

	if( GetDIBits( hDC,
		hBitmap,
		0,
		(WORD) bi.biHeight,
		(LPSTR) lpbi + (WORD) lpbi->biSize + PaletteSize((LPSTR) lpbi),
		(LPBITMAPINFO) lpbi, DIB_RGB_COLORS) == 0 )
	{
		DVGlobalUnlock (hDIB);  // UNLOCK DIB HANDLE - for error exit
		hDIB = NULL;
		SelectPalette (hDC, hPal, FALSE);
		ReleaseDC (NULL, hDC);
		return NULL;
	}

	bi = *lpbi;
	DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE

	SelectPalette (hDC, hPal, FALSE);
	ReleaseDC (NULL, hDC);

	return hDIB;
}

#define	MXCOPY	0x4000

DWORD	DVCopy( LPSTR lpD, LPSTR lpS, DWORD Siz )
{
DWORD	cpy, bal;
PINT8	hd;
PINT8	hs;
WORD	cnt, boff, soff;
	cpy = 0;
	if( (hd = ( PINT8 ) lpD) &&
		(hs = ( PINT8 ) lpS) &&
		(bal = Siz) )
	{
	while( bal )
	 {
		if( bal > MXCOPY )
			cnt = MXCOPY;
		else
			cnt = LOWORD( bal );
		if( boff = LOWORD( hd ) )
		{
			soff = ~boff + 1;
			if( soff && (soff < cnt) )
				cnt = soff;
		}
		if( boff = LOWORD( hs ) )
		{
			soff = ~boff + 1;
			if( soff && (soff < cnt) )
				cnt = soff;
		}
		dv_fmemcpy( hd, hs, cnt );	/* Do the MOVE of data ... */
		hd += cnt;
		hs += cnt;
		cpy += cnt;
		bal -= cnt;
	 }	/* While data to move ... */
	}
return( cpy );
}

/****************************************************************************

 FUNCTION   : PMDibFromBitmap()

 PURPOSE    : Will create a global memory block in DIB format that
		 represents the Device-dependent bitmap (DDB) passed in.

		      biStyle -> DIB format     ==  PM
		      biBits  -> Bits per pixel ==  1,4,8,24

 RETURNS    : A handle to the DIB

 ****************************************************************************/
HANDLE PMDibFromBitmap( HBITMAP hbm, DWORD biS,
					   WORD biB, HPALETTE hpal )
{
	BITMAP					bm;
    BITMAPCOREINFO			bi;
    BITMAPCOREINFO MLPTR	lpbi;
    DWORD					dwLen;
    HANDLE					hdib;
    HDC						hdc;
    DWORD					SizeImage;
	WORD					biBits;

	hdib = 0;
	if( hbm )
	{
		if( hpal == NULL )
			hpal = GetStockObject( DEFAULT_PALETTE );

		if( GetObject( hbm, sizeof(bm), (LPSTR)&bm ) )
		{
			if( biB == 0 )
				biBits =  bm.bmPlanes * bm.bmBitsPixel;
			else
				biBits = biB;

			bi.bmciHeader.bcSize	= sizeof(BITMAPCOREHEADER);
			bi.bmciHeader.bcWidth	= (WORD)bm.bmWidth;
			bi.bmciHeader.bcHeight	= (WORD)bm.bmHeight;
			bi.bmciHeader.bcPlanes	= 1;
			bi.bmciHeader.bcBitCount= biBits;

			SizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) *
				bm.bmHeight;

			dwLen = bi.bmciHeader.bcSize +
				PaletteSize((LPSTR)&bi) +
				SizeImage;

			if( hdc = GetDC(NULL) )
			{
				if( hpal )
				{
					hpal = SelectPalette( hdc, hpal, FALSE );
					RealizePalette( hdc );
				}
				if( hdib = DVGlobalAlloc( GHND, dwLen ) )
				{
					if( lpbi = (VOID MLPTR)DVGlobalLock( hdib ) )   // LOCK DIB HANDLE - new alloc
					{
						*lpbi = bi;
						if( GetDIBits( hdc,
							hbm,
							0,
							(WORD)bi.bmciHeader.bcHeight,
							(LPSTR)lpbi + (WORD)bi.bmciHeader.bcSize + PaletteSize((LPSTR)&bi),
							(LPBITMAPINFO)(LPBITMAPCOREINFO) lpbi, DIB_RGB_COLORS) )
						{
							DVGlobalUnlock( hdib ); // UNLOCK DIB HANDLE - new get bits ok
						}
						else
						{
							DVGlobalUnlock( hdib ); // UNLOCK DIB HANDLE - new for free
							DVGlobalFree( hdib );
							hdib = 0;
						}	// if GetDIBits() y/n
					}	// if lock ok
				}	// if allocation

				if( hpal )
					SelectPalette( hdc, hpal, FALSE );

				ReleaseDC( NULL, hdc );
			}	// if GetDC()
		}	// if GetObject on bitmap handle
	}	// if bitmap handle

   return hdib;

}



LPSTR SkipCharacter(LPSTR lpSearch,
                    BYTE  cTarget)
{
  BYTE cLook;

  while(cLook=*lpSearch++)
    if(cLook!=cTarget)
      return --lpSearch;

  return NULL;
}


LPSTR FindCharacter(LPSTR lpSearch,
                    BYTE  cTarget)
{
  BYTE cLook;

  while(cLook=*lpSearch++)
    if(cLook==cTarget)
      return --lpSearch;

  return NULL;
}


/*************************************************************************

  Function:  GetSectionName (lpCmdLine)

   Purpose:  Check for command line switch and save it.

   Return:   Pointer to first non-space, non-section name character

  Comments:

   History:   Date     Reason

            11/13/91   Created
            11/15/91   Added code to skip leading spaces.
                       Otherwise would enter infinite loop
                       if >1 file specified on command line.

*************************************************************************/
LPSTR GetSectionName(LPSTR lpCmdLine)
{
  LPSTR lpWork;
  BOOL  bEndOfString = FALSE;
//  LPSTR lpStartHere;


  // Skip any leading spaces.
  while (*lpCmdLine == ' ')
      *lpCmdLine++;
  lpWork = lpCmdLine;

  // Use a switch instead of if, so we can easily add other switches in
  // the future.

//  switch(*lpWork)
//  {
//    case '/':
//    case '-':
//      switch(*(lpWork+1))
//      {  
//         default:
//          break;
//      }
//      break;
//  }
  
  return lpCmdLine;
}


LPSTR RemLeadingBlanks(LPSTR lpIn)
{
  LPSTR lpWork;
  BOOL  bEndOfString = FALSE;
	if( lpWork = lpIn )
	{
		// Skip any leading spaces.
		while (*lpWork == ' ')
			*lpWork++;
	}
	return lpWork;
}



char	UpIt( char c )
{
	char	d;
	if( (c >= 'a') && (c <= 'z') )
		d = c & 0x5f;
	else
		d = c;
	return( d);
}

LPSTR LoadIFile( LPSTR lpFile )
{
	int		i, j;
	LPSTR	   lps = lpFile;
   LPSTR lpb;
//	char	Buff[MAX_PATH+4];
	HANDLE hDIB;
	char	c;
	PRDIB		   prd = (PRDIB)MALLOC( sizeof(RDIB) );

   if(!prd)
      chkme( "C:ERROR: No memory!!!"MEOR );

	NULPRDIB( prd );
   i = 0;
   if(lps)
      i = strlen(lps);
	if(i)
	{
		//lpb = &Buff[0];
      lpb = gszRPTit;
		for( j = 0; j < i; j++ )
		{
			c = *lps++;
			if( c && ((c != '-') || (c != '/')) )
			{
				lpb[j] = c;
			}
			else
			{
				break;
			}
		}
		if( j )
		{
         lpb[j] = '\0';
         j--;
         while(j)
         {
            if( lpb[j] > ' ' )
               break;
            lpb[j] = 0;
         }
         DVGetFullName2( gszRPNam, gszRPTit );
			//if( CheckIfFileExists( lpb ) )
			if(( CheckIfFileExists( gszRPNam ) ) &&
            ( IsValidFile( gszRPNam, &prd->rd_fd ) == IS_FILE ) )
         {
				//lstrcpy ((LPSTR)szDFileName, lpb);
				//hDIB = GetDIB ((LPSTR) szDFileName, gd_LI );
				hDIB = GetDIB( gszRPNam, gd_LI );
				if( hDIB )
				{
					prd->rd_hDIB = hDIB;
					//rd.rd_pPName = szDFileName;
					prd->rd_pPath  = gszRPNam;
               prd->rd_pTitle = gszRPTit;
					prd->rd_Caller = df_COMMAND;
					if( OpenDIBWindow2( prd ) )   // note: file info at prd->rd_fd
					{
					//OpenDIBWindow(hDIB, szDFileName, df_COMMAND );
#ifdef	CHGADDTO
						//AddToFileList( szDFileName );
						//AddToFileList( gszRPNam );
                  //AddToFileList4( &rd );
                  ADD2LIST( prd );
#endif	// CHGADDTO
					}
				}
			}
		}
	}

   MFREE(prd);

   // NOTE: Returns updated pointer to a command line
	return( lps );
}


// *************************************************************
//
//  Function:  ParseCommandLine (lpCmdLine)
//
//   Purpose:  Parse the command line arguments for filenames, and then
//             open the DIB'S
//
//   Return:   void
//
//  Comments:
//
//   History:   Date     Reason
//
//             8/08/91   Created
//            11/13/91   Added a switch for the section name
//                       (Used only for the CT stuff)
//
//   *************************************************************
void ParseCommandLine( LPSTR lpCL )
{
	LPSTR	lpCmdLine;
	char	c, d;
	BOOL	swt;

	swt = FALSE;
	lpCmdLine = lpCL; 
	if(lpCmdLine)
	{
		c = *lpCmdLine;	// Get first character
		while( c != 0 )
		{
			// Remove leading spaces
			lpCmdLine = RemLeadingBlanks(lpCmdLine);

			c = *lpCmdLine;	// Get NEXT character
			if( c )	// If we HAVE a character ...
			{
				// Is it a SWITCH beginning
				if( ( c == '-' ) ||
					 ( c == '/' ) )
				{
					lpCmdLine++;	// to next
					c = *lpCmdLine;	// Get NEXT character ...
					d = UpIt( c );	// all to upper case
					switch( d )
					{

					case 'I':
						lpCmdLine++;
						c = *lpCmdLine;
						if( c )
							lpCmdLine = LoadIFile( lpCmdLine );
						break;

					default:
						while( c > ' ' )
						{
							lpCmdLine++;
							c = *lpCmdLine;	// Get NEXT character ...
						}
					} // switch by COMMAND
				}
				else if( c == '@' )
				{
					lpCmdLine++;
					lpCmdLine = GetInpFile( lpCmdLine );
				}
				else
				{
					lpCmdLine = LoadIFile( lpCmdLine );
				}
			}
			c = *lpCmdLine;	// Get NEXT character ...
		} // while NOT a 0
    }
	return;
}

//The CreateFile function creates, opens, or truncates a file, pipe, mailslot, communications resource, disk //device, or console. It returns a handle that can be used to
//access the object. It can also open and return a handle to
//a directory.
//HANDLE CreateFile(
//    LPCTSTR lpFileName,	// pointer to name of the file 
//    DWORD dwDesiredAccess,	// access (read-write) mode 
//    DWORD dwShareMode,	// share mode 
//    LPSECURITY_ATTRIBUTES lpSecurityAttributes,	// pointer
//to security descriptor 
//    DWORD dwCreationDistribution,	// how to create 
//    DWORD dwFlagsAndAttributes,	// file attributes 
//    HANDLE hTemplateFile 	// handle to file with attributes
//to copy  
//   );	
//Parameters

//lpFileName
//Points to a null-terminated string that specifies the name
//of the file, pipe, mailslot, communications resource, disk
//device, or console to create, open, or truncate. 
//If *lpFileName is a path, there is a default string size
//limit of MAX_PATH characters. This limit is related to how
//the CreateFile function parses paths. 
//Windows NT: You can transcend this limit and send in paths
//longer than MAX_PATH characters by calling the wide (W)
//version of CreateFile and prepending "\\?\" to the path.
//The "\\?\" tells the function to turn off path parsing.
//This lets you use paths that are nearly 32k Unicode
//characters long. You must use fully-qualified paths with
//this technique. This also works with UNC names. The "\\?\"
//is ignored as part of the path. For example,
//"\\?\C:\myworld\private" is seen as "C:\myworld\private",
//and "\\?\UNC\tom_1\hotstuff\coolapps" is seen as
//"\\tom_1\hotstuff\coolapps". 

//dwDesiredAccess
//Specifies the type of access to the file or other object.
//An application can obtain read access, write access,
//read-write access, or device query access. You can use the
//following flag constants to build a value for this
//parameter. Both GENERIC_READ and GENERIC_WRITE must be set
//to obtain read-write access: 
//Value	Meaning
//0	Allows an application to query device attributes without
//actually accessing the device.
//GENERIC_READ	Specifies read access to the file. Data can be
//read from the file and the file pointer can be moved.
//GENERIC_WRITE	Specifies write access to the file. Data can
//be written to the file and the file pointer can be moved.

//dwShareMode
//Set of bit flags that specifies how the file can be shared.
//If dwShareMode is 0, the file cannot be shared. No other
//open operations can be performed on the file.
//To share the file, use a combination of one or more of the
//following values:  
//Value	Meaning
//FILE_SHARE_DELETE	Windows NT only: Other open operations
//can be performed on the file for delete access. 
//FILE_SHARE_READ	Other open operations can be performed on
//the file for read access. If the CreateFile function is
//opening the client end of a mailslot, this flag is
//specified.
//FILE_SHARE_WRITE	Other open operations can be performed on
//the file for write access.

//lpSecurityAttributes
//Pointer to a SECURITY_ATTRIBUTES structure that determines
//whether the returned handle can be inherited by child
//processes. If lpSecurityAttributes is NULL, the handle
//cannot be inherited. 
//Windows NT: The lpSecurityDescriptor member of the
//structure specifies a security descriptor for the new file.
//If lpSecurityAttributes is NULL, the file gets a default
//security descriptor. The target file system must support
//security on files and directories for this parameter to
//have an effect.
//Windows 95: The lpSecurityDescriptor member of the
//structure is ignored.

//dwCreationDistribution
//Specifies which action to take on files that exist, and
//which action to take when files do not exist. For more
//information about this parameter, see the following Remarks
//section. This parameter must be one of the following
//values: 
//Value	Meaning
//CREATE_NEW	Creates a new file. The function fails if the
//specified file already exists.
//CREATE_ALWAYS	Creates a new file. The function overwrites
//the file if it exists.
//OPEN_EXISTING	Opens the file. The function fails if the
//file does not exist.
//	See the "Remarks" section, following, for a discussion of
//why you should use the OPEN_EXISTING flag if you are using
//the CreateFile function for a device, including the
//console.
//OPEN_ALWAYS	Opens the file, if it exists. If the file does
//not exist, the function creates the file as if
//dwCreationDistribution were CREATE_NEW.
//TRUNCATE_EXISTING	Opens the file. Once opened, the file is
//truncated so that its size is zero bytes. The calling
//process must open the file with at least GENERIC_WRITE
//access. The function fails if the file does not exist.

//dwFlagsAndAttributes
//Specifies the file attributes and flags for the file. 
//Any combination of the following attributes is acceptable,
//except all other file attributes override
//FILE_ATTRIBUTE_NORMAL. 
//Attribute	Meaning
//FILE_ATTRIBUTE_ARCHIVE	The file is an archive file.
//Applications use this attribute to mark files for backup or
//removal.
//FILE_ATTRIBUTE_COMPRESSED	The file or directory is
//compressed. For a file, this means that all of the data in
//the file is compressed. For a directory, this means that
//compression is the default for newly created files and
//subdirectories.
//FILE_ATTRIBUTE_HIDDEN	The file is hidden. It is not to be
//included in an ordinary directory listing.
//FILE_ATTRIBUTE_NORMAL	The file has no other attributes set.
//This attribute is valid only if used alone.
//FILE_ATTRIBUTE_OFFLINE	The data of the file is not
//immediately available. Indicates that the file data has
//been physically moved to offline storage.
//FILE_ATTRIBUTE_READONLY	The file is read only. Applications
//can read the file but cannot write to it or delete it.
//FILE_ATTRIBUTE_SYSTEM	The file is part of or is used
//exclusively by the operating system.
//FILE_ATTRIBUTE_TEMPORARY	The file is being used for
//temporary storage. File systems attempt to keep all of the
//data in memory for quicker access rather than flushing the
//data back to mass storage. A temporary file should be
//deleted by the application as soon as it is no longer
//needed.
//Any combination of the following flags is acceptable.
//Flag	Meaning
//FILE_FLAG_WRITE_THROUGH	
//	Instructs the operating system to write through any
//intermediate cache and go directly to the file. The
//operating system can still cache write operations, but
//cannot lazily flush them.
//FILE_FLAG_OVERLAPPED	
//	Instructs the operating system to initialize the file, so
//ReadFile, WriteFile, ConnectNamedPipe, and
//TransactNamedPipe operations that take a significant amount
//of time to process return ERROR_IO_PENDING. When the
//operation is finished, an event is set to the signaled
//state.
//	When you specify FILE_FLAG_OVERLAPPED, the ReadFile and
//WriteFile functions must specify an OVERLAPPED structure.
//That is, when FILE_FLAG_OVERLAPPED is specified, an
//application must perform overlapped reading and writing.
//	When FILE_FLAG_OVERLAPPED is specified, the operating
//system does not maintain the file pointer. The file
//position must be passed as part of the lpOverlapped
//parameter (pointing to an OVERLAPPED structure) to the
//ReadFile and WriteFile functions.
//	This flag also enables more than one operation to be
//performed simultaneously with the handle (a simultaneous
//read and write operation, for example).
//FILE_FLAG_NO_BUFFERING	
//	Instructs the operating system to open the file with no
//intermediate buffering or caching. This can provide
//performance gains in some situations. An application must
//meet certain requirements when working with files opened
//with FILE_FLAG_NO_BUFFERING:�	File access must begin at
//offsets within the file that are integer multiples of the
//volume's sector size. �	File access must be for numbers of
//bytes that are integer multiples of the volume's sector
//size. For example, if the sector size is 512 bytes, an
//application can request reads and writes of 512, 1024, or
//2048 bytes, but not of 335, 981, or 7171 bytes. �	Buffer
//addresses for read and write operations must be aligned on
//addresses in memory that are integer multiples of the
//volume's sector size. An application can determine a
//volume's sector size by calling the GetDiskFreeSpace
//function.
//FILE_FLAG_RANDOM_ACCESS	
//	Indicates that the file is accessed randomly. Windows uses
//this flag to optimize file caching.
//FILE_FLAG_SEQUENTIAL_SCAN	
//	Indicates that the file is to be accessed sequentially
//from beginning to end. Windows uses this flag to optimize
//file caching. If an application moves the file pointer for
//random access, optimum caching may not occur; however,
//correct operation is still guaranteed.
//	Specifying this flag can increase performance for
//applications that read large files using sequential access.
//Performance gains can be even more noticeable for
//applications that read large files mostly sequentially, but
//occasionally skip over small ranges of bytes.
//FILE_FLAG_DELETE_ON_CLOSE	
//	Indicates that the operating system is to delete the file
//immediately after all of its handles have been closed.If
//you use this flag when you call CreateFile, then open the
//file again, and then close the handle for which you
//specified FILE_FLAG_DELETE_ON_CLOSE, the file will not be
//deleted until after you have closed the second and any
//other handle to the file.
//FILE_FLAG_BACKUP_SEMANTICS	
//	Windows NT only: Indicates that the file is being opened
//or created for a backup or restore operation. The operating
//system ensures that the calling process overrides file
//security checks, provided it has the necessary permission
//to do so. The relevant permissions are SE_BACKUP_NAME and
//SE_RESTORE_NAME.A Windows NT application can also set this
//flag to obtain a handle to a directory. A directory handle
//can be passed to some Win32 functions in place of a file
//handle.
//FILE_FLAG_POSIX_SEMANTICS	
//	Indicates that the file is to be accessed according to
//POSIX rules. This includes allowing multiple files with
//names, differing only in case, for file systems that
//support such naming. Use care when using this option
//because files created with this flag may not be accessible
//by applications written for MS-DOS, Windows 3.x, or Windows
//NT.
//If the CreateFile function opens the client side of a named
//pipe, the dwFlagsAndAttributes parameter can also contain
//Security Quality of Service information. When the calling
//application specifies the SECURITY_SQOS_PRESENT flag, the
//dwFlagsAndAttributes parameter can contain one or more of
//the following values: 
//Value	Meaning
//SECURITY_ANONYMOUS	Specifies to impersonate the client at
//the Anonymous impersonation level.
//SECURITY_IDENTIFICATION	Specifies to impersonate the client
//at the Identification impersonation level.
//SECURITY_IMPERSONATION	Specifies to impersonate the client
//at the Impersonation impersonation level.
//SECURITY_DELEGATION	Specifies to impersonate the client at
//the Delegation impersonation level.
//SECURITY_CONTEXT_TRACKING	Specifies that the security
//tracking mode is dynamic. If this flag is not specified,
//Security Tracking Mode is static.
//SECURITY_EFFECTIVE_ONLY	Specifies that only the enabled
//aspects of the client's security context are available to
//the server. If you do not specify this flag, all aspects of
//the client's security context are available.This flag
//allows the client to limit the groups and privileges that a
//server can use while impersonating the client.
//For more information, see Security. 

//hTemplateFile
//Specifies a handle with GENERIC_READ access to a template
//file. The template file supplies file attributes and
//extended attributes for the file being created. 
//Windows 95: This value must be NULL. If you supply a handle
//under Windows 95, the call fails and GetLastError returns
//ERROR_NOT_SUPPORTED.
//Return Values
//If the function succeeds, the return value is an open
//handle to the specified file. If the specified file exists
//before the function call and dwCreationDistribution is
//CREATE_ALWAYS or OPEN_ALWAYS, a call to GetLastError
//returns ERROR_ALREADY_EXISTS (even though the function has
//succeeded). If the file does not exist before the call,
//GetLastError returns zero. 
//If the function fails, the return value is
//INVALID_HANDLE_VALUE. To get extended error information,
//call GetLastError. 
//Remarks
//If you are attempting to create a file on a floppy drive
//that does not have a floppy disk or a CD-ROM drive that
//does not have a CD, the system displays a message box
//asking the user to insert a disk or a CD, respectively. To
//prevent the system from displaying this message box, call
//the SetErrorMode function with SEM_FAILCRITICALERRORS.
//When creating a new file, the CreateFile function performs
//the following actions: 
//�	Combines the file attributes and flags specified by
//dwFlagsAndAttributes with FILE_ATTRIBUTE_ARCHIVE. 
//�	Sets the file length to zero. 
//�	Copies the extended attributes supplied by the template
//file to the new file if the hTemplateFile parameter is
//specified. 
//When opening an existing file, CreateFile performs the
//following actions: 
//�	Combines the file flags specified by dwFlagsAndAttributes
//with existing file attributes. CreateFile ignores the file
//attributes specified by dwFlagsAndAttributes. 
//�	Sets the file length according to the value of
//dwCreationDistribution. 
//�	Ignores the hTemplateFile parameter. 
//�	Ignores the lpSecurityDescriptor member of the
//SECURITY_ATTRIBUTES structure if the lpSecurityAttributes
//parameter is not NULL. The other structure members are
//valid. The bInheritHandle member is the only way to
//indicate whether the file handle can be inherited. 
//If CreateFile opens the client end of a named pipe, the
//function uses any instance of the named pipe that is in the
//listening state. The opening process can duplicate the
//handle as many times as required but, once opened, the
//named pipe instance cannot be opened by another client. The
//access specified when a pipe is opened must be compatible
//with the access specified in the dwOpenMode parameter of
//the CreateNamedPipe function. For more information about
//pipes, see Pipes. 
//If CreateFile opens the client end of a mailslot, the
//function returns INVALID_HANDLE_VALUE if the mailslot
//client attempts to open a local mailslot before the
//mailslot server has created it with the CreateMailSlot
//function. For more information about mailslots, see
//Mailslots. 
//CreateFile can create a handle to a communications
//resource, such as the serial port COM1. For communications
//resources, the dwCreationDistribution parameter must be
//OPEN_EXISTING, and the hTemplate parameter must be NULL.
//Read, write, or read-write access can be specified, and the
//handle can be opened for overlapped I/O. For more
//information about communications, see Communications. 
//CreateFile can create a handle to console input (CONIN$).
//If the process has an open handle to it as a result of
//inheritance or duplication, it can also create a handle to
//the active screen buffer (CONOUT$). 
//The calling process must be attached to an inherited
//console or one allocated by the AllocConsole function. For
//console handles, set the CreateFile parameters as follows: 
//Parameters	Value
//lpFileName	Use the CONIN$ value to specify console input
//and the CONOUT$ value to specify console output.
//	CONIN$ gets a handle to the console's input buffer, even
//if the SetStdHandle function redirected the standard input
//handle. To get the standard input handle, use the
//GetStdHandle function.
//	CONOUT$ gets a handle to the active screen buffer, even if
//SetStdHandle redirected the standard output handle. To get
//the standard output handle, use GetStdHandle.
//dwDesiredAccess	GENERIC_READ | GENERIC_WRITE is preferred,
//but either one can limit access.
//dwShareMode	If the calling process inherited the console or
//if a child process should be able to access the console,
//this parameter must be FILE_SHARE_READ | FILE_SHARE_WRITE.
//lpSecurityAttributes	If you want the console to be
//inherited, the bInheritHandle member of the
//SECURITY_ATTRIBUTES structure must be TRUE.
//dwCreationDistribution	The user should specify
//OPEN_EXISTING when using CreateFile to open the console.
//dwFlagsAndAttributes	Ignored.
//hTemplateFile	Ignored.
//The following list shows the effects of various settings of
//fwdAccess and lpFileName.
//lpFileName	fwdAccess	Result
//CON	GENERIC_READ	Opens console for input.
//CON	GENERIC_WRITE	Opens console for output.
//CON	GENERIC_READ
//GENERIC_WRITE	Windows 95: Causes CreateFile to fail
//GetLastError returns ERROR_PATH_NOT_FOUND.Windows NT: 
//Causes CreateFile to fail; GetLastError returns
//ERROR_FILE_NOT_FOUND.
//You can use the CreateFile function to open a disk drive or
//a partition on a disk drive. The function returns a handle
//to the disk device; that handle can be used with the
//DeviceIOControl function. The following requirements must
//be met in order for such a call to succeed: 
//�	The caller must have administrative privileges for the
//operation to succeed on a hard disk drive. 
//�	The lpFileName string should be of the form
//\\.\PHYSICALDRIVEx to open the hard disk x. Hard disk
//numbers start at zero. For example: 
//String	Meaning
//\\.\PHYSICALDRIVE2	Obtains a handle to the third physical
//drive on the user's computer.
//�	The lpFileName string should be \\.\x: to open a floppy
//drive x or a partition x on a hard disk. For example: 
//String	Meaning
//\\.\A:	Obtains a handle to drive A on the user's computer.
//\\.\C:	Obtains a handle to drive C on the user's computer.
//Windows 95: This technique does not work for opening a
//logical drive. In Windows 95, specifying a string in this
//form causes CreateFile to return an error.
//�	The dwCreationDistribution parameter must have the
//OPEN_EXISTING value. 
//�	When opening a floppy disk or a partition on a hard disk,
//you must set the FILE_SHARE_WRITE flag in the dwShareMode
//parameter.
//The CloseHandle function is used to close a handle returned
//by CreateFile. 
//As noted above, specifying zero for dwDesiredAccess allows
//an application to query device attributes without actually
//accessing the device. This type of querying is useful, for
//example, if an application wants to determine the size of a
//floppy disk drive and the formats it supports without
//having a floppy in the drive. 
//As previously noted, if an application opens a file with
//FILE_FLAG_NO_BUFFERING set, buffer addresses for read and
//write operations must be aligned on memory addresses that
//are integer multiples of the volume's sector size. One way
//to do this is to use VirtualAlloc to allocate the buffer.
//The VirtualAlloc function allocates memory that is aligned
//on addresses that are integer multiples of the operating
//system's memory page size. Since both memory page and
//volume sector sizes are powers of 2, and memory pages are
//larger than volume sectors, this memory is also aligned on
//addresses that are integer multiples of a volume's sector
//size.
//An application cannot create a directory with CreateFile;
//it must call CreateDirectory or CreateDirectoryEx to create
//a directory.
//Windows NT:
//You can obtain a handle to a directory by setting the
//FILE_FLAG_BACKUP_SEMANTICS flag. A directory handle can be
//passed to some Win32 functions in place of a file handle.
//Some file systems, such as NTFS, support compression for
//individual files and directories. On volumes formatted for
//such a file system, a new directory inherits the
//compression attribute of its parent directory.
//See Also
//AllocConsole, CloseHandle, ConnectNamedPipe,
//CreateDirectory, CreateDirectoryEx, CreateNamedPipe,
//DeviceIOControl, GetDiskFreeSpace, GetOverlappedResult,
//GetStdHandle, OpenFile, OVERLAPPED, ReadFile,
//SECURITY_ATTRIBUTES, SetErrorMode, SetStdHandle
////TransactNamedPipe, VirtualAlloc, WriteFile 

/* ************************************************************

  Function:  GetDIB ( LPSTR lpFile, DWORD Caller )

   Purpose:  Opens dib file and reads into memory.

   Return:   HDIB if successful.
             NULL if an error occurs

  Comments:

   History:   Date     Reason

             8/8/91    Created

   ************************************************************ */


HANDLE GetDIB( LPSTR lpf, DWORD Caller )
{

	int      i;
	HANDLE hFile;
   OFSTRUCT ofs;
   HANDLE   hDIB;
	//char	ebuf[MXTMPSTR];
	LPTSTR	lps;
	PRDIB prd = (PRDIB)MALLOC( sizeof(RDIB) );
   if(!prd)
      chkme( "C:ERROR: No memory!!!"MEOR );
	NULPRDIB( prd );

//   SetCursor(LoadCursor(NULL, IDC_WAIT));
	Hourglass( TRUE );
	hDIB = 0;
//typedef struct _OFSTRUCT { // of  
//    BYTE cBytes; 
//    BYTE fFixedDisk; 
//    WORD nErrCode; 
//    WORD Reserved1; 
//    WORD Reserved2; 
//    CHAR szPathName[OFS_MAXPATHNAME]; 
////} OFSTRUCT; 
   if( !( IsValidFile( lpf, &prd->rd_fd ) == IS_FILE ) )
      goto No_File;

//	ofs.cBytes = sizeof( OFSTRUCT );
#ifdef	WIN32
	if( (IS_WIN95) || (IS_NT) )
	{
#ifdef	USEOPN
		hFile = _open( lpf, _O_RDONLY, _S_IREAD );
#else	// !USEOPN
		hFile = CreateFile( lpf,	// pointer to name of the file
			GENERIC_READ,			// access (read-write) mode 
			0,						// share mode 
			NULL,					// pointer
			OPEN_EXISTING,			// how to create 
			FILE_ATTRIBUTE_NORMAL,	// file attributes 
			NULL ) ;	// handle to file with attributes to copy  
#endif	// USEOPN
	}
	else
	{
   	ofs.cBytes = sizeof( OFSTRUCT );
		hFile = DVOpenFile ( lpf, &ofs, OF_READ);
	}
#else	// !WIN32
   	ofs.cBytes = sizeof( OFSTRUCT );
		hFile = DVOpenFile ( lpf, &ofs, OF_READ);
#endif	// WIN32 y/n

	if(( hFile                ) &&
	   ( hFile != INVALID_HANDLE_VALUE ) ) // HFILE_ERROR
	{
		prd->rd_pfHand   = &hFile;	// Ptr to File Handle
      prd->rd_pTitle   = gszRPTit;  // set buffers
      prd->rd_pPath    = gszRPNam;
      strcpy( gszRPTit, lpf );
      DVGetFullName2( gszRPNam, gszRPTit );
		//rd.rd_pFName   = lpf;		// Ptr to File Name
		prd->rd_Caller   = Caller;	// Caller ID
		prd->rd_dwFlag   = 0;
		prd->rd_hDIB     = 0;		// No DIB
		prd->rd_hGIFInfo = 0;		// and NO GIF Info
		// NOTE: Difference for GIF Read!!!
		hDIB = ReadDIBFile( prd );	// Will return NUL for GIF
		if( hDIB == 0 )				// And if gd_SF caller hDIB
			hDIB = prd->rd_hDIB;		// will be here!!!
		if( VFH(hFile) )
			DVlclose( hFile );
      hFile = 0;
	}
   else
	{

No_File:

		//lps = &ebuf[0];
      lps = gszTmpBuf3;    // just a temp buffer
//		wsprintf( lps, "File: [%s]"MEOR, (LPSTR) szDFileName );
		sprintf( lps, "File: [%s]"MEOR, lpf );
		i = strlen( lps );
		if( DIBEString( (lps+i), ERR_FILENOTFOUND ) )
		{
			if( !ChkMFRFile( lpf ) )
				DIBErrorStg( lps );
		}
		else
		{
			DIBError( ERR_FILENOTFOUND );
		}
	}

   MFREE(prd);

//	SetCursor(LoadCursor(NULL, IDC_ARROW));
	Hourglass( FALSE );
	return( hDIB );
}

// =========================================================
// HANDLE GetDIB2( PRDIB prd )
//
// Called from CommonFileOpen() through OpenDIBFile2()
//
//typedef struct _OFSTRUCT { // of  
//    BYTE cBytes; 
//    BYTE fFixedDisk; 
//    WORD nErrCode; 
//    WORD Reserved1; 
//    WORD Reserved2; 
//    CHAR szPathName[OFS_MAXPATHNAME]; 
////} OFSTRUCT; 
//
// =========================================================
HANDLE GetDIB2( PRDIB prd )
{
	int		i;
	HANDLE  hFile;
	OFSTRUCT ofs;
	HANDLE   hDIB;
	char	ebuf[MXTMPSTR];
	LPSTR	lps, lpf;

//   SetCursor(LoadCursor(NULL, IDC_WAIT));
	Hourglass( TRUE );
	hDIB = 0;
	ofs.cBytes = sizeof( OFSTRUCT );
	lpf = prd->rd_pPath;	// Extract the FILE NAME pointer

	hFile = DvOpen2( lpf, &ofs, OF_READ );	// DvRead.c
	if( VFH(hFile) )
	{
		prd->rd_pfHand   = &hFile;	// Ptr to File Handle
		// NOTE: Difference for GIF Read!!!
		hDIB = ReadDIBFile( prd );	// Will return NUL for GIF
		if( hDIB == 0 )				// And if gd_SF caller hDIB
			hDIB = prd->rd_hDIB;		// will be here!!!
		if( hFile )
			DVlclose( hFile );
	}
   else
	{
		lps = &ebuf[0];
//		wsprintf( lps, "File: [%s]"MEOR, (LPSTR) szDFileName );
		wsprintf( lps, "File: [%s]"MEOR, lpf );
		i = lstrlen( lps );
		if( DIBEString( (lps+i), ERR_FILENOTFOUND ) )
		{
			if( !ChkMFRFile( lpf ) )
				DIBErrorStg( lps );
		}
		else
		{
			DIBError( ERR_FILENOTFOUND );
		}
	}

//	SetCursor(LoadCursor(NULL, IDC_ARROW));
	Hourglass( FALSE );
	return( hDIB );
}


//---------------------------------------------------------------------
//
// Function:   DlgWndPaint
//
// Purpose:    Called by VIEWDLGPROC() on WM_PAINT.  Does all paints
//             for this DIALOG window.
//
//             Calls the appropriate paint routine depending on the
//             option set for this window (i.e. DIB, DDB, or SetDIBits).
//
// Parms:      hWnd == Handle to window being painted.
//
// History:   Date      Reason
//				14/02/96    Created
//             
//---------------------------------------------------------------------

void DlgWndPaint( HWND hWnd )
{
   HDC         hDC;
   PAINTSTRUCT ps;
   HPALETTE    hOldPal = NULL;
   RECT        rectClient, rectDDB;
   BOOL        bStretch;
   HANDLE      hDIBInfo;
   LPDIBINFO   lpDIBInfo;
   BITMAP      Bitmap;

   Hourglass (TRUE);

   hDC      = BeginPaint (hWnd, &ps);
   hDIBInfo = lpInfo->is_hdlDib;

   if (!hDIBInfo)
      goto ABORTDPAINT;

   lpDIBInfo = (LPDIBINFO) DVGlobalLock (hDIBInfo);

   if (!lpDIBInfo->hDIB || !lpDIBInfo->hBitmap)
      {
      DVGlobalUnlock (hDIBInfo);
      goto ABORTDPAINT;
      }

   bStretch = TRUE;
      
  // Set up the necessary rectangles -- i.e. the rectangle
  //  we're rendering into, and the rectangle in the DIB.

   GetClientRect (hWnd, &rectClient);
   GetObject (lpDIBInfo->hBitmap, sizeof (Bitmap), (LPSTR) &Bitmap);
   
   rectDDB.left   = 0;
   rectDDB.top    = 0;
   rectDDB.right  = Bitmap.bmWidth;
   rectDDB.bottom = Bitmap.bmHeight;
//	rectClient.left = rectClient.right / 2;
//	rectClient.right = rectClient.right / 2;
	if( rectClient.right > rectDDB.right )
	{
//		rectClient.left = ( rectClient.right - rectDDB.right ) / 2
//		rectClient.right = rectDDB.right;
	}
	if( rectClient.bottom > rectDDB.bottom )
	{
				
	}
 // Setup the palette.

   if( lpDIBInfo->hPal )
	{
      hOldPal = SelectPalette( hDC, lpDIBInfo->hPal, TRUE );
	   RealizePalette( hDC );
	}
      // Go do the actual painting.
   DDBPaint( lpDIBInfo, hDC, &rectClient, lpDIBInfo->hBitmap, &rectDDB, 
               lpDIBInfo->hPal, SRCCOPY );

   if( hOldPal )
      SelectPalette( hDC, hOldPal, FALSE );

   DVGlobalUnlock (hDIBInfo);

ABORTDPAINT:

   EndPaint( hWnd, &ps );

   Hourglass (FALSE);
}

BOOL	SetInIndex( LPSTR lpe )
{
   /* Given a EXTENT, try to SET the InIndex value ... */
   BOOL	flg;
   WORD	i, k;
   LPSTR	lps;

	flg = FALSE;   // assume can not
   if(lpe)
      i = strlen(lpe);
   else
      i = 0;
   
   lps = pRDefLst[0];

	//if( lpe && (i = lstrlen( lpe )) && (lps = pRDefLst[0]) )
	if( i && lps )
	{
		k = 0;
		if((strcmpi( lpe, &szDib[0] ) == 0) ||
			(strcmpi( lpe, &szRle[0] ) == 0) )
		{	/* We have one of the two specials ... */
			flg = TRUE;	
		}
		else
		{	/* Go through the LIST ... */
			while( ( lps = pRDefLst[k] ) != NULL )
			{
				if( strcmpi( lps, lpe ) == 0 )
				{
					flg = TRUE;	
					break;
				}
				else
				{
					k++;	/* Bump the INDEX ... */
				}
			}	/* While ... */
		}
		if( flg )
		{
			if( InIndex != k )
			{
				InIndex = k;
				fChgInInd = TRUE;
			}
		}
	}
   return flg;
}

void	AddFN( LPSTR lpb, LPSTR lpf )
{
   DWORD	i, j;
	if( lpb && lpf )
	{
		i = strlen( lpb );
		j = strlen( pRAFNm );
		if( (i + j) < (MXIBUF-1) )
			wsprintf( (lpb+i), pRAFNm, lpf );
	}
}

#ifdef	USEGFUNC5

//typedef struct tagGIFIMG {
//	WORD	giWidth;
//	WORD	giHeight;
//	WORD	giCMSize;
//	DWORD	giBMPSize;
//}GIFIMG;
//typedef GIFIMG MLPTR LPGIFIMG;
//typedef struct tagGIFHDR {
//	WORD	ghMaxCount;
//	WORD	ghImgCount;
//	WORD	ghWidth;
//	WORD	ghHeight;
//	WORD	ghCMSize;
//	DWORD	ghBMPSize;
//	GIFIMG	ghGifImg[1];
//}GIFHDR;
//typedef GIFHDR MLPTR LPGIFHDR;
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now - includes GIF

// This is just a LOCAL service - Only does COUNT
extern	DWORD	Gif_Count( LPSTR, DWORD );

WORD	GetGIFCount( HGLOBAL hgG, DWORD ddSz, PRDIB prd )
{
	LPSTR		lpf;
	DWORD		gerr;
	WORD		ret;
   DWORD    nxt, bgn, max;
	DWORD		dwFlg;
	HGLOBAL		hGH;
	HGLOBAL		hDIB;
	LPGIFHDR	lpGH;
	LPGIFIMG	lpGI;
	DWORD		ddSiz, ddSz1;
	BOOL		flg;
	HGLOBAL		hIBuf;
	LPSTR		lpIBuf;
//	char		buf[MAX_FILENAME];
	LPSTR		lpGF;
	DWORD		dwMem;	// Size of WGIFSIZX Memory
//	GIFSTR		GifStr;
   PGIFSTR  pgs = &gsGif;  // = WrkStr.w_sGifStr  // global GIF structure

	Hourglass( TRUE );
	lpf = prd->rd_pPath;
	//lpGF = &buf[0];
	lpGF = &pgs->szFile[0];
	hGH = 0;
	lpGH = 0;
	ret = 0;		/* Assume FAILED */
	lpIBuf = 0;
	lstrcpy( lpGF, lpf );	/* Setup the DEFAULT */
	if( lpIBuf = DVGlobalLock( hgG ) )
	{
		max = Gif_Count( lpIBuf, ddSz );
		DVGlobalUnlock( hgG );
	}

	lpIBuf = 0;
	// Could use the above "max" information
	dwMem = (sizeof( GIFHDR ) + (sizeof( GIFIMG ) * MXGIFIMG ));
	if( ( hGH = DVGlobalAlloc( GHND | GMEM_SHARE, dwMem ) ) &&
		( lpGH = (LPGIFHDR) DVGlobalLock( hGH ) ) &&
		( hIBuf = DVGAlloc( "INFOBUF", GHND, 1024 ) ) )
	{
		//( hIBuf = DVGlobalAlloc( GHND, 1024 ) ) )
		lpGH->ghMaxCount = MXGIFIMG;
		lpGH->ghImgCount = 0;
		DVGlobalUnlock( hGH );
//		if( gerr = (*WGifSizX) ( hgG, ddSz, hGH ) )
		if( gerr = DVWGifSizX( hgG, ddSz, hGH ) )
		{
			DIBError( ERR_NOLSIZEG );
		}
		else
		{
			if( lpGH = (LPGIFHDR) DVGlobalLock( hGH ) )
			{
				if( max = lpGH->ghImgCount )
				{	/* If there is a Count ... */
#ifdef	CHKSIZE2
					ddSiz = lpGH->ghBMPSize;	// Get the MAX. size
					ddSz1 = 0;
					for( nxt = 0; nxt < max; nxt++ )
					{
						lpGI = &lpGH->ghGifImg[nxt];
						if( lpGI->giBMPSize > ddSz1 )
							ddSz1 = lpGI->giBMPSize;
					}
					if( ddSz1 == 0 )
					{
						DIBError( ERR_NOLSIZEG );
					}
					else
					{
						ddSz1 += 15;
					}
#endif	// CHKSIZE2
					pgs->hgGifs = hGH;
					pgs->lpGFile = lpf;
					pgs->fGAll = FALSE;						
					pgs->fGNone = FALSE;
					pgs->fGComb = TRUE;
					pgs->wGNum = 1;	/* Setup for image no 1 ... */
					pgs->hgFile = hgG;	// Handle to GIF File
					pgs->dwFSize = ddSz;	// Size of FILE
					if( max > 1 )
					{	/* More than 1 - Make a choice ... */
						DVGlobalUnlock( hGH );
						lpGH = 0;
						if( prd->rd_Caller & gd_AL )
						{
                     // set ANIMATE ie ALL *AND* NONE set!!!
							pgs->fGAll = TRUE;
							pgs->fGNone = TRUE;
						}  // FIX20001208 - Also except the PREVIEW caller
						else if( ( prd->rd_Caller & (gd_SF|gd_DP) ) == 0 )
						{
							// From GetGIFCount()!
							//ChooseGIF( &GifStr );
							ChooseGIF( pgs );
						}
					}

					if( pgs->fGNone && !pgs->fGAll )
					{
                  /* NOT Special NONSENCE case = ANIMATE GIF ... */
						ret = 0;	/* All done ... */
					}
					else
					{
						if( lpGH = (LPGIFHDR) DVGlobalLock( hGH ) )
						{
#ifndef	CHKSIZE2
							ddSiz = lpGH->ghBMPSize;	/* Get the MAX. size */
							if( ddSiz == 0 )
							{
								// What is happening ...???
								ddSiz = 0;
							}
#endif	// CHKSIZE2
							if( pgs->fGAll )
							{
                        /* Doing ALL ... */
								bgn = 0;
								ret = 0;
								if( pgs->fGNone )
									dwFlg = df_GIFANIM;
								else
									dwFlg = df_GIFGRP;
								gdwfFlag |= dwFlg;
								for( nxt = bgn; nxt < max; nxt++ )
								{
									lpGI = &lpGH->ghGifImg[nxt];
#ifdef	CHKSIZE2
									if( hDIB = DVGlobalAlloc( GMEM_SHARE, ddSz1 ) )
#else	// !CHKSIZE2
									if( ( ddSz1 = lpGI->giBMPSize ) &&
										( hDIB = DVGlobalAlloc( GMEM_SHARE, ddSz1 ) ) )
#endif	// CHKSIZE2
									{	/* Got SIZE and HANDLE ... */
										flg = LIBGifNBmp( hgG,
											ddSz,
											hIBuf,
											hDIB,
											(WORD)(nxt+1) );
	// =================================
	if( flg )
	{	/* Returned an ERROR */
		DVGlobalFree( hDIB );
		hDIB = 0;
		lpIBuf = 0;
		if( hIBuf )
			lpIBuf = DVGlobalLock( hIBuf );
		if( lpIBuf ) AddFN( lpIBuf, lpf );
		if( lpIBuf && lpIBuf[0] )
  			DIBError2( ERR_NOLCONV, lpIBuf );
		else
			DIBError( ERR_NOLCONV );
		if( lpIBuf )
			DVGlobalUnlock( hIBuf );
		lpIBuf = 0;
	}
	else
	{
		/* Success - Open a Window ... (IF NOT gd_AL = Autoload) */
      // FIX20001208 - also EXCEPT the preview caller
		if( prd->rd_Caller & (gd_AL|gd_DP) )
		{
			// Doing AUTOLOAD of a GIF
			prd->rd_hDIB = hDIB;	// is all that is required
		}
		else
		{
			wsprintf( lpGF, "%s [%d:%d]", lpf, (nxt+1), max );
			prd->rd_hDIB = hDIB;
			prd->rd_pTitle = lpGF;
			prd->rd_Caller |= dwFlg;
			if( OpenDIBWindow2( prd ) )
			{
			//OpenDIBWindow( hDIB, lpGF, dwFlg );
#ifdef	CHGADDTO
				//AddToFileList( lpf );
            //AddToFileList4( prd );
            ADD2LIST(prd);
#endif	// CHGADDTO
			}
		}
		ret++;
	}
	// =================================
									}	/* If a SIZE and an allocation */
									else
									{
										DIBError( ERR_MEMORY );
										break;
									}
									if( pgs->fGNone )	/* ALL *AND* NONE = ANIMATE */
										break;
								}	/* For Full SET ... */
							}
							else
							{
                        /* Doing ONLY 1 */
								if( nxt = pgs->wGNum )
								{	/* If a value ... */
									nxt--;
									lpGI = &lpGH->ghGifImg[nxt];
									if( lpGI->giBMPSize == 0 )
									{
										// What is happening????
										ddSz1 = 0;
									}
									if( ( ddSz1 = lpGI->giBMPSize ) &&
										( hDIB = DVGlobalAlloc( GMEM_SHARE, ddSz1 ) ) )
									{	/* Got SIZE and HANDLE ... */
										flg = LIBGifNBmp( hgG,
											ddSz,
											hIBuf,
											hDIB,
											(WORD)(nxt+1) );
	// =================================>
	if( flg )
	{	/* Returned an ERROR */
		DVGlobalFree( hDIB );
		hDIB = 0;
		lpIBuf = 0;
		if( hIBuf )
			lpIBuf = DVGlobalLock( hIBuf );
		if( lpIBuf ) AddFN( lpIBuf, lpf );
		if( lpIBuf && lpIBuf[0] )
  			DIBError2( ERR_NOLCONV, lpIBuf );
		else
			DIBError( ERR_NOLCONV );
		if( lpIBuf )
			DVGlobalUnlock( hIBuf );
		lpIBuf = 0;
	}
	else
	{
      /* Success - Open a Window ... IF NOT gd_SF */
		// Special MESSY case of GIF files
		// For some reason I put OpenDIBWindow HERE!!! BAD!!!
		// BUT the Show_File in OPEN FILE only want the hDIB
		// to display the image ... so ...
      // FIX20001208 - alse except the PREVIEW caller
		if( prd->rd_Caller & (gd_SF|gd_DP) )	// IF caller is Show_File
		{
			prd->rd_hDIB = hDIB;	// is all that is required
		}
		else
		{
			prd->rd_hDIB   = hDIB;
			prd->rd_pTitle = lpGF;
			if( max > 1 )	/* If MORE THAN ONLY one ... */
			{	/* Show the [number:total] ... */
				wsprintf( lpGF, pRFnGrp, lpf, (nxt+1), max );
				prd->rd_Caller |= df_GIFONEOF;
				//OpenDIBWindow( hDIB, lpGF, df_GIFONEOF );
			}
			else
			{	/* Just [1:1] - Use just default name ... */
				prd->rd_Caller |= df_GIFONE;
				//OpenDIBWindow( hDIB, lpGF, df_GIFONE );
			}

			if( OpenDIBWindow2( prd ) )
			{
#ifdef	CHGADDTO
				//AddToFileList( lpf );
            //AddToFileList4( prd );
            ADD2LIST(prd);
#endif	// CHGADDTO
			}
		}

		ret = 1;

	}
	// =================================>
									}	/* If a SIZE and an allocation */
								}
							}
						}
						else
						{
							goto Err_Mem;
						}

					}  // animate yes/no
				}
				else
				{
					DIBError( ERR_NOLSIZEG );
				}
			}
			else
			{
				goto Err_Mem;
			}
		}
	}
	else
	{

Err_Mem:
		DIBError( ERR_MEMORY );
	}

	if( hGH && lpGH )
		DVGlobalUnlock( hGH );
	if( hGH )
		DVGlobalFree( hGH );

	if( hIBuf )
		DVGlobalFree( hIBuf );

	Hourglass( FALSE );

   return( ret );

}

#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now - includes GIF


///* Multiple and Transparent GIF Image Support */
///* ========================================== */
//#define		gie_Flag		0x8000	// This is in the ghMaxCount
//// if the application expect an EXTENDED description!
//typedef	struct	tagGIFIMGEXT {
//	GIFIMG	giGI;	// Width/Height/ColSize and BMP Size as above
//// Image Descriptor - Wdith and Height in above
//	WORD	giLeft;		// Left (logical) column of image
//	WORD	giTop;		// Top (logical) row
//	BYTE	giBits;		// See below (packed field)
//// Graphic Control Extension
//	BYTE	gceExt;		// Extension into - Value = 0x21
//	BYTE	gceLabel;	// Graphic Control Extension = 0xf9
//	DWORD	gceSize;	// Block Size (0x04 for GCE, Big for TEXT)
//	BYTE	gceBits;	// See below (packed field)
//	WORD	gceDelay;	// 1/100 secs to wait
//	BYTE	gceIndex;	// Transparency Index (if Bit set)
//	DWORD	gceColr;	// COLORREF (if SET)
//	DWORD	gceFlag;	// IN/OUT Options Flag - See Below
//	RGBQUAD	gceBkGrnd;	// Background Colour
//	DWORD	gceRes1;	// Reserved
//	DWORD	gceRes2;	// ditto
//}GIFIMGEXT;
//typedef GIFIMGEXT	FAR * LPGIFIMGEXT;
//typedef struct tagGIFHDREXT {
//	WORD	gheMaxCount;	// gie_Flag + MAX. Count
//	WORD	gheImgCount;	// Images in GIF
//	WORD	gheWidth;		// Logical Screen Width
//	WORD	gheHeight;		// Logical Screen Height
//	WORD	gheCMSize;		// BMP Colour map size (byte count)
//	DWORD	gheBMPSize;		// Estimated final BMP size
//	BYTE	gheBits;		// See below (packed field)
//	BYTE	gheIndex;		// Background Colour Index
//	BYTE	ghePAR;			// Pixel Aspect Ration
//	DWORD	gheFlag;		// IN/OUT Options Flag - See Below
//	RGBQUAD	gheBkGrnd;		// Background Colour
//	DWORD	gheRes1;		// Reserved
//	DWORD	gheRes2;		// ditto
//	GIFIMGEXT	gheGIE[1];	// 1 for Each Image follows
//}GIFHDREXT;
////typedef GIFHDREXT FAR * LPGIFHDREXT;
#ifdef	GIFDIAGS
char	szTmpFil[MAX_PATH] = { "TEMP0000.BMP" };
#endif	// GIFDIAGS

#ifdef	WJPEG5		// For NEXT release ...

BOOL DoAllGifs( HGLOBAL hgG,	// GIF Data
			   DWORD ddSz,	// GIF Data Size
			   HGLOBAL hIBuf,	// Information buffer
			   LPGIFHDREXT lpGHE,
			   BOOL fDoTxt,		// If there are TEXT blocks
			   WORD max )		// Max count
{
	BOOL	flg = FALSE;
	WORD	cnt;
	LPGIFIMGEXT	lpGIE;
	DWORD	ddSz1;
	HGLOBAL	hDIB;
	BOOL	rflg;
	for( cnt = 0; cnt <= max; cnt++ )
	{
		if( cnt == 0 )
		{
			ddSz1 = lpGHE->gheBMPSize;
			lpGHE->hDIB    = 0;
			lpGHE->hPal    = 0;
			lpGHE->hBitmap = 0;
			lpGHE->hFont   = 0;
		}
		else
		{
			lpGIE = &lpGHE->gheGIE[cnt-1];
			ddSz1 = lpGIE->giGI.giBMPSize;
			lpGIE->hDIB    = 0;
			lpGIE->hPal    = 0;
			lpGIE->hBitmap = 0;
			lpGIE->hFont   = 0;
		}
		if( hDIB = DVGlobalAlloc( GMEM_SHARE, ddSz1 ) )
		{
			// Got SIZE and HANDLE ...
			//if( cnt == 0 )
			//{
				rflg = (*WGifNBmpX) ( hgG, ddSz, hIBuf, hDIB, cnt );
			//}
			//else if( lpGIE->gceFlag & gie_PTE )
			//{
			//	rflg = (*WGifNBmpX) ( hgG, ddSz, hIBuf, hDIB, cnt );
			//}
			//else
			//{
			//	rflg = (*WGifNBmp) ( hgG, ddSz, hIBuf, hDIB,
			//		(WORD)(cnt-1) );
			//}
			if( rflg )
			{
				DVGlobalFree( hDIB );
				if( cnt == 0 )
				{
					flg = TRUE;
					break;
				}
			}
			else
			{
				// We have a CONVERSION
				if( cnt )
				{
					lpGIE->hDIB = hDIB;
				}
				else
				{
					lpGHE->hDIB = hDIB;
				}
#ifdef	GIFDIAGS
				if( ( hDIB ) &&
					(  ( cnt == 0 ) ||
					( !( lpGIE->gceFlag & gie_PTE ) ) ) )
				{
					BITMAPFILEHEADER bmfh;
					LPBITMAPINFOHEADER lpbmih;
					HFILE	hFile;
					OFSTRUCT	ofs;
					DWORD		dwr;
					if( lpbmih = (LPBITMAPINFOHEADER)DVGlobalLock( hDIB ) )  // LOCK DIB HANDLE
					{
						if( ( lpbmih->biSize == sizeof(BITMAPINFOHEADER) ) &&
							( FindNewName( &szTmpFil[0] ) )	)// Ensure UNIQUE name
						{
							hFile = DVOpenFile( &szTmpFil[0], &ofs, (OF_CREATE | OF_WRITE) );
							if( hFile && (hFile != HFILE_ERROR) )
							{
								bmfh.bfType = 'MB';
								bmfh.bfSize = BitmapFileSize((LPSTR)lpbmih);
								bmfh.bfReserved1 = 0;
								bmfh.bfReserved2 = 0;
								bmfh.bfOffBits = (OffsetToBits((LPSTR)lpbmih) + sizeof(BITMAPFILEHEADER)); 
								dwr = DVWrite( hFile, (LPSTR)&bmfh, sizeof(BITMAPFILEHEADER) );
								if( dwr == sizeof(BITMAPFILEHEADER) )
								{
									dwr += DVWrite( hFile, (LPSTR)lpbmih,
										(bmfh.bfSize - dwr));
									DVlclose( hFile );
									if( dwr != bmfh.bfSize )
										DVOpenFile( &szTmpFil[0], &ofs, OF_DELETE);
								}
								else
								{
									DVlclose( hFile );
									DVOpenFile( &szTmpFil[0], &ofs, OF_DELETE);
								}
							}
						}
						DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE
					}
				}
#endif	// GIFDIAGS
			}
		}
	}
	return flg;
}

#endif	// WJPEG5	- Multiple GIF support

// Get a COUNT with even Plain Text Extensions
// ===========================================
BOOL	GIFCountExt( HGLOBAL hgG, DWORD ddSz, HGLOBAL hGHE )
{
	BOOL	flg;
	flg = ERR_NO_LIB;
#if	(defined( USEGFUNC7 ) && defined( WJPEG5 ))
	if( WGifSizXT )
		flg = (*WGifSizXT) ( hgG, ddSz, hGHE );
#else	// !( USEFUNC7 & WJPEG5 )

	// Release in April 97, next maybe May,June,July ...
	// ==============================================
#ifdef	USEGFUNC7
#ifdef	WJPEG4
	// This does the SAME as SizXT, but does NOT include
	// the "Plain Text Extension, actual TEXT Block
	// ============================================
//	if( WGifSizX )
//		flg = (*WGifSizX) ( hgG, ddSz, hGHE );
//	if( WGifSizX )
		flg = DVWGifSizX( hgG, ddSz, hGHE );
#endif	// WJPEG4 - Release
#endif	// USEGFUNC7 - Release

#endif	// USEGFUNC7 & WJPEG5 y/n
	return flg;
}

DWORD	GetSizeMGI( void )
{
	return(	sizeof( GIFHDREXT ) + (sizeof( GIFIMGEXT ) * MXGIFIMG) );
}

// FIX20001208 - If CALLER is gd_DP DO NOT PUT UP ANY DIALOG
// since this is a call from a dialog that is only showing a REVIEW image
// so make choices and return a hDIB if possible
// ======================================================================
WORD	GetGIFCountExt( HGLOBAL hgG, DWORD ddSz, PRDIB prd )
{
	LPSTR		lpf;
	WORD		gerr;
	WORD		ret, nxt, bgn, max;
	DWORD		dwFlg;
	HGLOBAL		hGHE;
	HGLOBAL		hDIB;
	LPGIFHDREXT	lpGHE;
	LPGIFIMGEXT	lpGIE;
	DWORD		ddSiz, ddSz1;
	BOOL		flg;
	HGLOBAL		hIBuf;
	LPSTR		lpIBuf;
//	char		buf[MAX_FILENAME];
	LPSTR		lpGF;
	DWORD		dwMem;	// Size of WGIFSIZX Memory
	BOOL		fDoTxt;
	HGLOBAL		hGInf;
	LPSTR		lpg;
//	GIFSTR		GifStr;
   PGIFSTR  pgs = &gsGif;  // WrkStr.w_sGifStr  // global GIF structure

	Hourglass( TRUE );
	lpf = prd->rd_pPath;
	//lpGF = &buf[0];
	lpGF = &pgs->szFile[0];    // GIF file name
	hGHE = 0;
	lpGHE = 0;
	ret = 0;		/* Assume FAILED */
	lpIBuf = 0;
	lstrcpy( lpGF, lpf );	/* Setup the DEFAULT */
	if( lpIBuf = DVGlobalLock( hgG ) )
	{
		max = (WORD)Gif_Count( lpIBuf, ddSz );
		DVGlobalUnlock( hgG );
	}
	lpIBuf = 0;
	// Could use the above "max" information
	dwMem = GetSizeMGI();
	if( ( hGHE = DVGlobalAlloc( GHND | GMEM_SHARE, dwMem ) ) &&
		( lpGHE = (LPGIFHDREXT) DVGlobalLock( hGHE ) ) &&
		( hIBuf = DVGlobalAlloc( GHND, GetSizeMxI() ) ) )
	{
		lpGHE->gheMaxCount = (gie_Flag | MXGIFIMG);
		lpGHE->gheImgCount = 0;
		DVGlobalUnlock( hGHE );
//		if( gerr = (*WGifSizX) ( hgG, ddSz, hGHE ) )
		if( gerr = DVWGifSizX( hgG, ddSz, hGHE ) )
		{
			DIBError( ERR_NOLSIZEG );
		}
		else
		{
			if( lpGHE = (LPGIFHDREXT) DVGlobalLock( hGHE ) )
			{
				if( max = lpGHE->gheImgCount )
				{	/* If there is a Count ... */
#ifdef	CHKSIZE2
					ddSiz = lpGHE->gheBMPSize;	// Get the MAX. size
					ddSz1 = 0;
					for( nxt = 0; nxt < max; nxt++ )
					{
						lpGIE = &lpGHE->gheGIE[nxt];
						if( lpGIE->giGI.giBMPSize > ddSz1 )
							ddSz1 = lpGIE->giGI.giBMPSize;
					}
					if( ddSz1 == 0 )
					{
						DIBError( ERR_NOLSIZEG );
					}
					else
					{
						ddSz1 += 15;
					}
#endif	// CHKSIZE2
					pgs->hgGifs  = hGHE;
					pgs->lpGFile = lpf;
					pgs->fGAll   = FALSE;						
					pgs->fGNone  = FALSE;
					pgs->fGComb  = FALSE;
					pgs->wGNum   = 1;	/* Setup for image no 1 ... */
					pgs->hgFile  = hgG;	// Handle to GIF File
					pgs->dwFSize = ddSz;	// Size of FILE
					DVGlobalUnlock( hGHE );
					lpGHE = 0;
					if( max > 1 )
					{	/* More than 1 - Make a choice ... */
						if( prd->rd_Caller & gd_AL )
						{	// set ANIMATE ie ALL *AND* NONE set!!!
							pgs->fGAll = TRUE;
							pgs->fGNone = TRUE;
						}  // FIX20001208 - Also except gd_DP caller
						else if( ( prd->rd_Caller & (gd_SF | gd_DP) ) == 0 )
						{
							// From GetGIFCountExt
							//ChooseGIF( &GifStr );
							ChooseGIF( pgs );
						}
					}
					if( pgs->fGNone && !pgs->fGAll )
					{
                  /* NOT Special NONSENCE case = ANIMATE GIF ... */
						ret = 0;	/* All done ... */

					}
					else
					{
						if( lpGHE = (LPGIFHDREXT) DVGlobalLock( hGHE ) )
						{
#ifndef	CHKSIZE2
							ddSiz = lpGHE->gheBMPSize;	/* Get the MAX. size */
							if( ddSiz == 0 )
							{
								// What is happening ...???
								ddSiz = 0;
							}
#endif	// CHKSIZE2
							if( pgs->fGAll )
							{	/* Doing ALL ... */
								bgn = 0;
								ret = 0;
								if( pgs->fGNone )
									dwFlg = df_GIFANIM;
								else
									dwFlg = df_GIFGRP;
								gdwfFlag |= dwFlg;
#if	(defined( TRANSGIF ) && defined( WJPEG5 ))
								// ==============================
								// NEW capability - Combine mult GIF
								if( pgs->fGComb )
								{
									fDoTxt = FALSE;
#ifdef	USEGFUNC7
									if( lpGHE->gheFlag & ghf_PTxtExt )	// Had plain text extension
									{
										if( gerr = GIFCountExt( hgG, ddSz, hGHE ) )
										{
											DIBError( ERR_NOGLSIZE );
										}
										else
										{
											if( lpGHE->gheImgCount > max )
											{
												max = lpGHE->gheImgCount;
												fDoTxt = TRUE;
											}
										}
									}
#endif	// USEGFUNC7
									dwMem = (sizeof( GIFHDREXT ) +
										(sizeof( GIFIMGEXT ) * max ));
									if( hGInf = DVGlobalAlloc( GHND, dwMem ) )
									{
										if( lpg = DVGlobalLock( hGInf ) )
										{
											dv_fmemcpy( lpg,
												lpGHE,
												(size_t)dwMem );
											flg = DoAllGifs( hgG,	// GIF Data
												ddSz,	// GIF Data Size
												hIBuf,	// Information buffer
												(LPGIFHDREXT)lpg,
												fDoTxt,
												max ); 
#ifdef	GIFDIAGS
   // FIX20001208 - Avoid DIAGS if just PREVIEW displayer
   if( ( prd->rd_Caller & gd_DP ) == 0 )
   {
	   DumpGIF( (LPGIFHDREXT)lpg, lpf );
   }
#endif	// GIFDIAGS
											DVGlobalUnlock( hGInf );
											prd->rd_hGIFInfo = hGInf;
											prd->rd_Caller |= (dwFlg | df_GIFGRPC);
											prd->rd_dwFlag |= gd_GIFInfo;	// We are returning GIFHDREXT[n]
											goto gg_exit;
										}
										else
										{
											DVGlobalFree( hGInf );
										}
									}
								}
								else	// Not a COMBINED, many images
								{	// But STILL keep GIF Info
									fDoTxt = FALSE;
//#ifdef	USEGFUNC7
//									if( lpGHE->gheFlag & ghf_PTxtExt )	// Had plain text extension
//									{
//										if( gerr = GIFCountExt( hgG, ddSz, hGHE ) )
//										{
//											DIBError( ERR_NOGLSIZE );
//										}
//										else
//										{
//											if( lpGHE->gheImgCount > max )
//											{
//												max = lpGHE->gheImgCount;
//												fDoTxt = TRUE;
//											}
//										}
//									}
//#endif	// USEGFUNC7
									dwMem = (sizeof( GIFHDREXT ) +
										(sizeof( GIFIMGEXT ) * max ));
									if( hGInf = DVGlobalAlloc( GHND, dwMem ) )
									{
										if( lpg = DVGlobalLock( hGInf ) )
										{
											dv_fmemcpy( lpg,
												lpGHE,
												(size_t)dwMem );
											//flg = DoAllGifs( hgG,	// GIF Data
											//	ddSz,	// GIF Data Size
											//	hIBuf,	// Information buffer
											//	(LPGIFHDREXT)lpg,
											//	fDoTxt,
											//	max ); 
#ifdef	GIFDIAGS
   // FIX20001208 - Avoid DIAGS if just PREVIEW displayer
   if( ( prd->rd_Caller & gd_DP ) == 0 )
   {
   	DumpGIF( (LPGIFHDREXT)lpg, lpf );
   }
#endif	// GIFDIAGS
											DVGlobalUnlock( hGInf );
											prd->rd_hGIFInfo = hGInf;
											//prd->rd_Caller |= (dwFlg | df_GIFGRPC);
											prd->rd_dwFlag |= gd_GIFInfo2;	// We are returning GIFHDREXT[n]
										}
										else
										{
											DVGlobalFree( hGInf );
										}
									}
								}
#endif	// TRANSGIF & WJPEG5 - Next rel. July 97???
								// ==============================

								for( nxt = bgn; nxt < max; nxt++ )
								{
									lpGIE = &lpGHE->gheGIE[nxt];
#ifdef	CHKSIZE2
// FIX980501 						if( hDIB = DVGlobalAlloc( GMEM_SHARE, ddSz1 ) )
									if( hDIB = DVGAlloc( lpf, GMEM_SHARE, DWSIZE(ddSz1) ) )
#else	// !CHKSIZE2
									if( ( ddSz1 = lpGIE->giGI.giBMPSize ) &&
										( hDIB = DVGlobalAlloc( GMEM_SHARE, ddSz1 ) ) )
#endif	// CHKSIZE2
									{	/* Got SIZE and HANDLE ... */
										flg = LIBGifNBmp( hgG,
											ddSz,
											hIBuf,
											hDIB,
											(WORD)(nxt+1) );
	// =================================
	if( flg )
	{
      /* Returned an ERROR */
		DVGlobalFree( hDIB );
		hDIB = 0;
		lpIBuf = 0;
		if( hIBuf )
			lpIBuf = DVGlobalLock( hIBuf );
		if( lpIBuf ) AddFN( lpIBuf, lpf );
		if( lpIBuf && lpIBuf[0] )
  			DIBError2( ERR_NOLCONV, lpIBuf );
		else
			DIBError( ERR_NOLCONV );
		if( lpIBuf )
			DVGlobalUnlock( hIBuf );
		lpIBuf = 0;
	}
	else
	{
		/* Success - Open a Window ... (IF NOT gd_AL = Autoload) */
		if( prd->rd_Caller & gd_AL )
		{
			// Doing AUTOLOAD of a GIF
			prd->rd_hDIB = hDIB;	// is all that is required
		}
		else
		{
			wsprintf( lpGF, "%s [%d:%d]", lpf, (nxt+1), max );
			prd->rd_hDIB    = hDIB;
			prd->rd_pTitle  = lpGF;
			prd->rd_Caller |= dwFlg;
			if( OpenDIBWindow2( prd ) )
			{
			//OpenDIBWindow( hDIB, lpGF, dwFlg );
#ifdef	CHGADDTO
            //AddToFileList4( prd );
			   //AddToFileList( lpf );
            ADD2LIST(prd)
#endif	// CHGADDTO
			}
			prd->rd_hDIB = 0;	// MUST ZERO this later
		}
		ret++;
	}
	// =================================
									}	/* If a SIZE and an allocation */
									else
									{
										DIBError( ERR_MEMORY );
										break;
									}
									if( pgs->fGNone )	/* ALL *AND* NONE = ANIMATE */
										break;
								}	/* For Full SET ... */
							}
							else
							{
                        /* Doing ONLY 1 */
//==============================
									fDoTxt = FALSE;
//#ifdef	USEGFUNC7
//									if( lpGHE->gheFlag & ghf_PTxtExt )	// Had plain text extension
//									{
//										if( gerr = GIFCountExt( hgG, ddSz, hGHE ) )
//										{
//											DIBError( ERR_NOGLSIZE );
//										}
//										else
//										{
//											if( lpGHE->gheImgCount > max )
//											{
//												max = lpGHE->gheImgCount;
//												fDoTxt = TRUE;
//											}
//										}
//									}
//#endif	// USEGFUNC7
									dwMem = sizeof( GIFHDREXT );
									if( hGInf = DVGlobalAlloc( GHND, dwMem ) )
									{
										if( lpg = DVGlobalLock( hGInf ) )
										{
											LPGIFHDREXT lpNGHE;
											LPGIFIMGEXT lpNGIE;
											lpNGHE = (LPGIFHDREXT)lpg;
											dv_fmemcpy( lpg,
												lpGHE,
												(size_t)dwMem );
											if( nxt = pgs->wGNum )
												nxt--;

											if( nxt )
											{
                                    // If NOT the FIRST Image
												// Source
												lpGIE = &lpGHE->gheGIE[nxt];
												// Destination
												lpNGIE = &lpNGHE->gheGIE[0];
												dv_fmemcpy( lpNGIE,
													lpGIE,
													sizeof(GIFIMGEXT));
											}

											lpNGHE->gheImgCount = 1; // ONLY ONE(1)
											DVGlobalUnlock( hGInf ); // Unlock
											prd->rd_hGIFInfo = hGInf; // Store
											prd->rd_dwFlag |= gd_GIFInfo2;	// We are returning GIFHDREXT[1]

										}
										else
										{
											DVGlobalFree( hGInf );
										}
									}
//==============================

								if( nxt = pgs->wGNum )
								{
                           /* If a value ... */
									nxt--;
									lpGIE = &lpGHE->gheGIE[nxt];
									if( lpGIE->giGI.giBMPSize == 0 )
									{
										// What is happening????
										ddSz1 = 0;
									}
									if( ( ddSz1 = lpGIE->giGI.giBMPSize              ) &&
										 ( hDIB  = DVGlobalAlloc( GMEM_SHARE, ddSz1 ) ) )
									{
                              /* Got SIZE and HANDLE ... */
										flg = LIBGifNBmp( hgG,
											ddSz,
											hIBuf,
											hDIB,
											(WORD)(nxt+1) );

#ifdef	GIFDIAGS
	if( !flg && hDIB &&
		( prd->rd_dwFlag & gd_GIFInfo2 ) &&	// We are returning GIFHDREXT[1]
		( prd->rd_hGIFInfo ) )
	{
		LPGIFHDREXT lpghe2;
		LPGIFIMGEXT	lpgie2;
		if( lpghe2 = (LPGIFHDREXT)DVGlobalLock( prd->rd_hGIFInfo ) )
		{
			lpgie2 = &lpghe2->gheGIE[0];
			if( lpghe2->hDIB )
			{
            // FIX20001208 - Avoid DIAGS if just PREVIEW displayer
            if( ( prd->rd_Caller & gd_DP ) == 0 )
            {
				   DumpGIF( lpghe2, lpf );
            }
			}
			else
			{
				lpghe2->hDIB = hDIB;
            // FIX20001208 - Avoid DIAGS if just PREVIEW displayer
            if( ( prd->rd_Caller & gd_DP ) == 0 )
            {
				   DumpGIF( lpghe2, lpf );
            }
				lpghe2->hDIB = 0;
			}
			DVGlobalUnlock( prd->rd_hGIFInfo );
		}
	}
#endif	// GIFDIAGS
	// =================================>
	if( flg )
	{
      /* Returned an ERROR */
		DVGlobalFree( hDIB );
		hDIB = 0;
		lpIBuf = 0;
		if( hIBuf )
			lpIBuf = DVGlobalLock( hIBuf );
		if( lpIBuf ) AddFN( lpIBuf, lpf );
		if( lpIBuf && lpIBuf[0] )
  			DIBError2( ERR_NOLCONV, lpIBuf );
		else
			DIBError( ERR_NOLCONV );
		if( lpIBuf )
			DVGlobalUnlock( hIBuf );
		lpIBuf = 0;
            if( prd->rd_hGIFInfo )
               DVGlobalFree(prd->rd_hGIFInfo);
            prd->rd_hGIFInfo = 0;
	}
	else
	{
      /* Success - Open a Window ... IF NOT gd_SF */
		// Special MESSY case of GIF files
		// For some reason I put OpenDIBWindow HERE!!! BAD!!!
		// BUT the Show_File in OPEN FILE only want the hDIB
		// to display the image ... so ...
      // FIX20001208 - Also EXCEPT gd_DP
		if( prd->rd_Caller & (gd_SF|gd_DP) )	// IF caller is Show_File
		{
			prd->rd_hDIB = hDIB;	// is all that is required
         if( prd->rd_Caller & gd_DP )  // IF the caller is simply display preview
         {  // we must alos TOSS the GIF information header memory
            // or remember to do it LATER
            if( prd->rd_hGIFInfo )
               DVGlobalFree(prd->rd_hGIFInfo);
            prd->rd_hGIFInfo = 0;
         }
		}
		else
		{
			prd->rd_hDIB   = hDIB;
			prd->rd_pTitle = lpGF;
			if( max > 1 )	/* If MORE THAN ONLY one ... */
			{	/* Show the [number:total] ... */
				wsprintf( lpGF, pRFnGrp, lpf, (nxt+1), max );
				//OpenDIBWindow( hDIB, lpGF, df_GIFONEOF );
				prd->rd_Caller |= df_GIFONEOF;
			}
			else
			{	/* Just [1:1] - Use just default name ... */
				//OpenDIBWindow( hDIB, lpGF, df_GIFONE );
				prd->rd_Caller |= df_GIFONE;
			}

			if( OpenDIBWindow2( prd ) )
			{
#ifdef	CHGADDTO
            //AddToFileList4( prd );
				//AddToFileList( lpf );
            ADD2LIST(prd);
#endif	// CHGADDTO
			}
			prd->rd_hDIB = 0;	// But MUST ZERO this after
		}
		ret = 1;
	}
	// =================================>
									}	/* If a SIZE and an allocation */
								}
							}
						}
						else
						{
							goto Err_Mem;
						}
					}  // animate GIF y/n
				}
				else
				{
					DIBError( ERR_NOLSIZEG );
				}
			}
			else
			{
				goto Err_Mem;
			}
		}
	}
	else
	{
Err_Mem:
		DIBError( ERR_MEMORY );
	}
#if	(defined( TRANSGIF ) && defined( WJPEG5 ))
	// Support for many facet GIF - Only load image 0, and
	// let the timer process each GIFIMGEXT as its TIMER
	// expires. Check say every 200 ms ...
gg_exit:
	// ONLY the Logical GIF Screen has been completed, with the
	// background already filled in with the BkGnd colour.
	// Uses Index in lpGHE/lpGIE structures ....
	// ========================================================
#endif	// TRANSGIF & WJPEG5

	if( hGHE && lpGHE )
		DVGlobalUnlock( hGHE );
	if( hGHE )
		DVGlobalFree( hGHE );
	if( hIBuf )
		DVGlobalFree( hIBuf );

	Hourglass( FALSE );

	return( ret );

}

#endif	/* USEGFUNC5 */

DWORD  FilterInpFile( LPSTR lpb, DWORD dws )
{
   DWORD i, k;
   INT   c;

   k = 0;
   for( i = 0; i < dws; i++ )
   {
      c = lpb[i]; // get char from buffer
      if( c > ' ' )
      {
         if( c == ';' )
         {
            i++;
            for( ; i < dws; i++ )
            {
               c = lpb[i]; // get char from buffer
               if(( c == '\r' )||( c == '\n' ))
                  break;
            }
         }
         else
         {
            lpb[k++] = (char)c;
         }
      }
      else if(k)
      {
         if( lpb[ (k - 1) ] != ' ' )
            lpb[k++] = ' ';
      }
   }
   lpb[k] = 0;
   while(k)
   {
      k--;
      if(lpb[k] > ' ')
      {
         k++;
         break;
      }
      lpb[k] = 0;
   }
   return k;
}
// ==============================================
// LPSTR	GetInpFile( LPSTR lpf )
//
// Part of the COMMAND LINE processing, namely
// @FileName gives the name of an INPUT FILE
// which should contain commands.
//
// ==============================================
LPSTR	GetInpFile( LPSTR lpinp )
{
	LPSTR		lps, lpf, lpb;
	char		buf[MAX_PATH+8];
	int			i;
	HFILE		hf;
	OFSTRUCT	of;
	DWORD		dws;

	if( lps = lpinp )
	{
		i = 0;
		lpf = &buf[0];
		while( *lps > ' ' )
		{
			lpf[i++] = *lps;
			lps++;
			if( i > MAX_PATH )
				break;
		}

		// OK, see if we can OPEN this file
		lpf[i] = 0;
		if( ( hf = OpenFile( lpf, &of, OF_READ ) ) &&
			( hf != HFILE_ERROR ) )
		{
			if( ( dws = GetFileSize( IntToPtr(hf), 0 ) ) &&
				( dws != (DWORD)-1 ) )
			{
				if( lpb = LocalAlloc( LPTR, (dws+16) ) )
				{
					if( _lread( hf, lpb, dws ) == dws )
					{
						_lclose(hf);
						hf = 0;
                  dws = FilterInpFile( lpb, dws );
						while(dws)
						{
							lpb[dws] = 0;
							dws--;
							if( lpb[dws] > ' ' )
							{
								dws++;
								break;
							}
						}
						if( dws )
							ParseCommandLine( lpb );
					}
					LocalFree( lpb );
				}
			}
			if( hf && ( hf != HFILE_ERROR ) )
				_lclose( hf );
		}
		while( ( *lps <= ' ' ) &&
			( *lps != 0 ) )
		{
			lps++;
		}
	}
	return lps;
}

// eof - DvFile1.c
