
//
// wcommon.c
//
// These routines are COMMON to BOTH Compress and Decompress JPEG
//
// Paris, France            26 February, 1996     Geoff R. McLane
// To 32-Bit                28 Auguest,  1996     Geoff R. McLane
// Remove FILE i/f			15 April,    1997	Geoff R. McLane
//
//

#include "winclude.h"

#ifdef	_INC_WINDOWS

#include	"wcommon.h"

BOOL	bErrFlag;
extern	LPSTR	lpMBuf;
extern	DWORD	ddBufCnt;
extern	DWORD	ddOutCnt;
extern	DWORD	ddMaxCnt;
extern	struct Decompress_info_struct1 DSdinfo;
extern	PINT8 BMPcopy( PINT8, PINT8, DWORD );

// local
GLOBAL void set_file_size( DWORD );

char	szMsgBuf[MXERRBUF];
LPSTR	msgout;
#define	MXBUFF	512
HFILE	FilHnd = 0;
char	FilBuf[MXBUFF];
WORD	BufCnt = 0;
WORD	BufOut = 0;
typedef struct tagBADEXIT {	/* be */
	WORD	beVal;
	LPSTR	beStg;
} BADEXIT;
typedef BADEXIT MLPTR LPBADEXIT;
char	szRUnkE[] = "Unlisted ERROR number &d (%X)!";

BADEXIT BadExit[] = {
	{ BAD_FACTOR,   "ERROR: if(sscanf(arg, %d%c%d%c, &val1, &ch1, &val2, &ch2) < 3)! " },
	{ BAD_SYNT,     "ERROR: if ((ch1 != 'x' && ch1 != 'X') || ch2 != ',')! " },
	{ BAD_FACTOR2,  "ERROR: JPEG sampling factors must be 1..4! " },
	{ BAD_FILENAME, "ERROR: Must name one input and one output file! " },
	{ BAD_EMPTY,    "ERROR: Empty input file! " },
	{ BAD_FORMAT,   "ERROR: Unrecognized input file format! " },
	{ BAD_UNGET,    "ERROR: Ungetc failed! " },
	{ BAD_FOPEN,    "ERROR: Can't open INPUT file! " },
	{ BAD_OOPEN,    "ERROR: Can't create OUTPUT file! " },
	{ BAD_MEMORY,   "ERROR: Insufficient memory! " },
	{ BAD_MEMORY2,  "ERROR: Bogus free_small request! " },
	{ BAD_MEMORY3,  "ERROR: Bogus free_medium request! " },
	{ BAD_MEMORY4,  "ERROR: Image too wide for this memory implementation! " },
	{ BAD_MEMORY5,  "ERROR: Bogus free_small_sarray request! " },
	{ BAD_MEMORY6,  "ERROR: Bogus free_small_barray request! " },
	{ BAD_MEMORY7,  "ERROR: Bogus access_big_sarray request! " },
	{ BAD_MEMORY8,  "ERROR: Virtual array controller messed up! " },
	{ BAD_MEMORY9,  "ERROR: Bogus access_big_barray request! " },
	{ BAD_MEMORY10, "ERROR: Bogus free_big_sarray request! " },
	{ BAD_MEMORY11, "ERROR: Bogus free_big_barray request! " },
	{ BAD_GIFOUT,   "ERROR: GIF output got confused! " },
	{ BAD_BMPEOF,   "ERROR: Premature EOF in BMP file! " },
	{ BAD_BMPSIZ,   "Error: File too small. Not a BMP file! " },
	{ BAD_BMPREAD,  "Error: Bad read of input file! " },
	{ BAD_BMPSIZ2,  "Error: Conflict in SIZE. Not a BMP file! " },
	{ BAD_BMPRES1,  "Error: Reserved items NOT zero. Not a BMP file! " },
	{ BAD_BMPBM,    "ERROR: No BM signature! Not a BMP file! " },
	{ BAD_BMPBC,    "Error: Unsupported bit count! Not 1,4,8,24! " },
	{ BAD_BMPCM,    "Error: Unsupported Color map! Only RGBQUAD! " },
	{ BAD_BMPCOLR,  "Error: Color table mistake. Not a BMP file! " },
	{ BAD_BMPWHP,   "ERROR: Width, Height or Planes! Not a BMP file! " },
	{ BAD_BMPNS,    "ERROR: Compressed BMP file not presently handled! " },
	{ BAD_WWRITE,   "ERROR: Output File write error! " },
	{ BAD_PPMOUT,   "ERROR: PPM output must be grayscale or RGB! " },
	{ BAD_BMPOUT,   "ERROR: BMP output is confused! Can not handle channels! " },
	{ BAD_BACKING,  "ERROR: Backing store not supported! " },
	{ BAD_GIFEOF,   "ERROR: Premature EOF in GIF file! " },
	{ BAD_GIFNOT,   "ERROR: Not a GIF file! " },
	{ BAD_GIFCNT,   "ERROR: Too few images in GIF file! " },
	{ BAD_GIFCS,    "ERROR: Bogus codesize (input_code_size)! " },
	{ BAD_PPMEOF,   "ERROR: Premature EOF in PPM file! " },
	{ BAD_PPMBD,    "ERROR: Bogus data in PPM file! " },
	{ BAD_PPMNOT,   "ERROR: Not a PPM file! " },
	{ BAD_HUFFTAB,  "ERROR: Huffman table not defined! " },
	{ BAD_JFIFMAX,  "ERROR: Maximum image dimension for JFIF is 65535 pixels! " },
	{ BAD_WRITOUT,  "ERROR: Output file write error! Out of disk space? " },
	{ BAD_TAREOF,   "ERROR: Unexpected end of Targa file! " },
	{ BAD_TARGA,    "ERROR: Invalid or unsupported Targa file! " },
 	{ BAD_TARSIZ,   "ERROR: Targa Colormap too large! " },
	{ BAD_HUFFDHT,  "ERROR: Bogus DHT counts! " },
	{ BAD_HUFFDAC,  "ERROR: Bogus DAC index! " },
	{ BAD_HUFFTN,   "ERROR: Bogus table number! " },
	{ BAD_HUFFDRI,  "ERROR: Bogus length in DRI! " },
	{ BAD_JFIFBN,   "ERROR: Unsupported JFIF revision number! " },
	{ BAD_JFIFDNL,  "ERROR: Empty JPEG image (DNL not supported)! " },
	{ BAD_JFIFDP,   "ERROR: Unsupported JPEG data precision! " },
	{ BAD_JFIFSOF,  "ERROR: Bogus SOF length! " },
	{ BAD_JFIFSOS,  "ERROR: Invalid component number in SOS! " },
	{ BAD_JFIFNOT,  "ERROR: Not a JPEG file! " },
	{ BAD_JFIFSMT,  "ERROR: Unsupported SOF marker type! " },
	{ BAD_JFIFUM,   "ERROR: Unexpected marker! " },
	{ BAD_TARHDR,   "ERROR: Could not write Targa header! " },
	{ BAD_TARRGB,   "ERROR: Targa output must be grayscale or RGB! " },
	{ BAD_COMMAND,  "ERROR: Bad command line arguments given! " },
	{ BAD_JPGEOF,   "ERROR: Premature EOF in JPG file! " },
	{ BAD_JPGNUL,   "ERROR: Empty JPEG file! " },
	{ BAD_JPGDATA,  "WARNING: Corrupt JPEG data. Premature end of data segment! " },
	{ BAD_TARCF,    "ERROR: Unsupported Targa colormap format! " },
	{ BAD_JPGSAMP,  "ERROR: Bogus sampling factors! " },
	{ BAD_JPGTMC,   "ERROR: Too many components for interleaved scan! " },
	{ BAD_JPGICA,   "ERROR: I'm confused about the image width! " },
	{ BAD_JPGSFT,   "ERROR: Sampling factors too large for interleaved scan! " },
	{ BAD_NOTIMP,   "ERROR: Not implemented yet! " },
	{ BAD_JPGMSS,   "ERROR: Multiple-scan support was not compiled! " },
	{ BAD_JPGBIC,   "ERROR: Bogus input colorspace! " },
	{ BAD_JPGBJC,   "ERROR: Bogus JPEG colorspace! " },
	{ BAD_JPGUCC,   "ERROR: Unsupported color conversion request! " },
	{ BAD_HUFFTE,   "ERROR: Missing Huffman code table entry! " },
	{ BAD_HUFFUHT,  "ERROR: Use of undefined Huffman table! " },
	{ BAD_HUFFCST,  "ERROR: Huffman code size table overflow! " },
	{ BAD_CCIR,     "ERROR: CCIR601 downsampling not implemented yet! " },
	{ BAD_FRACTD,   "ERROR: Fractional downsampling not implemented yet! " },
	{ BAD_NOOUT,    "ERROR: Unsupported output file format! " },
	{ BAD_BMPCE,    "ERROR: Cannot handle colormap entries for BMP! " },
	{ BAD_BMPNRGB,  "ERROR: BMP output must be grayscale or RGB! " },
	{ BAD_GIFNRGB,  "ERROR: GIF output must be grayscale or RGB! " },
	{ BAD_GIF256,   "ERROR: GIF can only handle 256 colors! " },
	{ BAD_TARCOLS,  "ERROR: Too many colors for Targa output! " },
	{ BAD_ARITH,    "ERROR: Arithmetic coding not supported! " },
	{ BAD_JPGQFC,   "ERROR: Cannot quantize fewer than 2 colors! " },
	{ BAD_JPGCQC,   "ERROR: Cannot quantize more color components! " },
	{ BAD_JPGCRM,   "ERROR: Cannot request more quantized colors" },
	{ BAD_NOIMP2,   "ERROR: Should not get here!" },
	{ BAD_JPGCRL,   "ERROR: Cannot request less than 8 quantized colors" },
	{ BAD_JPG2PQ,   "ERROR: 2-pass quantization only handles YCbCr input" },
	{ BAD_JPGHUFF,  "WARNING: Corrupt JPEG data: Bad Huffman code! " },
	{ BAD_JPGDAT2,  "WARNING: Corrupt JPEG data: Extraneous bytes before marker! " },
	{ BAD_MEMCNTS,  "ERROR: Increase small memory counter! " },
	{ BAD_MEMCNTS,  "ERROR: Increase large memory counter!" },
	{ BAD_MEMHEAP1,  "ERROR: Increase HEAP size for near memory! (1)" },
	{ BAD_MEMHEAP2,  "ERROR: Increase HEAP size for near memory! (2)" },
	{ BAD_JPGEOS,   "ERROR: Sorry, entropy optimization was not compiled! " },
	{ BAD_TARSNC,   "ERROR: Targa support was not compiled! " }, 
	{ BAD_QTABOPN,  "ERROR: Unable to open quantizing table file! " },
	{ BAD_QTABSIZ,  "ERROR: Too many tables in qtable file! " },
	{ BAD_QTABINC,  "ERROR: Incomplete table in qtable file! " },
	{ BAD_GIFPTR,   "ERROR: NULL Data pointer passed! " },
	{ BAD_G2B_ENTRY,"ERROR: Input parameter to LIBRARY errant! "},
	{ BAD_MEMORY12, "ERROR: Unable to ACCESS Global Memory! " },
	{ BAD_QFILE,    "ERROR: Bogus data in quantization file! " },
	{ BAD_PTRNULL,	"ERROR: Pointer NOT intialized! " },
	{ BAD_RETIRED,	"ERROR: Attempt to use a RETIRED interface! " },
	{ BAD_NOGCOLR,	"ERROR: Bad GIF Image. No Global Color Table! " },
	{ BAD_ZEROMEMH,  "ERROR: No more memory handles! " },
	{ 0, 0 } };


#ifdef	WJPEG4
GLOBAL void
chkretired( void )
{
	int i;
	i = 0;
}
GLOBAL	size_t
chkjfwrite( void )
{
	chkretired();
	return 0;
}
#endif	// WJPEG4

GLOBAL int
getc( HFILE hf )
{
	int c;
	char	d;
	PINT8 cp;
// UINT	i;
	c = 0;
	if( lpMBuf && ddMaxCnt && ddBufCnt )
	{
		if( ddOutCnt < ddMaxCnt )
		{
			cp = ( PINT8 ) lpMBuf;
			cp = cp + ddOutCnt;
			d = *cp;
			c = ((int) d & 0xff );
			ddOutCnt++;
			ddBufCnt--;
		}
		else
		{
			c = EOF;
		}
	}
	else
	{
#ifdef	WJPEG4
		chkretired();
		c = EOF;
		bErrFlag = BAD_RETIRED;	// Attempt to use RETIRED interface
#else	// !WJPEG4
		if( hf && (hf != HFILE_ERROR) && (BufCnt == 0) )
		{
			FilHnd = hf;
			BufCnt = _lread( hf, &FilBuf[0], MXBUFF );
			BufOut = 0;
		}
		if( BufCnt )
		{
			d = FilBuf[BufOut];
			c = ((int) d & 0xff );
			BufOut++;
			BufCnt--;
		}	
		else
		{
			c = EOF;
		}
#endif	// WJPEG4 y/n
	}
return( c );
}

GLOBAL int
retgetc( int c, HFILE hf )
{
	int	i;
	char	d;
	i = EOF;
	d = c & 0xff;
	if( lpMBuf && ddMaxCnt )
	{
		if( (ddBufCnt < ddMaxCnt) && ddOutCnt )
		{
			ddOutCnt--;
			ddBufCnt++;
			i = 1;
		}
	}
	else
	{
#ifdef	WJPEG4
		chkretired();
		bErrFlag = BAD_RETIRED;	// Attempt to use RETIRED interface
#else	// !WJPEG4
		if( hf && (hf != HFILE_ERROR) && (hf == FilHnd) && BufOut )
		{
			if( (d == FilBuf[BufOut - 1]) && ((BufCnt + 1) <= MXBUFF) )
			{
				BufOut--;
				BufCnt++;
				i = 1;
			}
		}
#endif	// WJPEG4 y/n
	}
return( i );
}

GLOBAL void
flushmsg( LPSTR lps )
{
	int	i;
	if( lps && (i = lstrlen( lps )) )
	{
		lps[0] = 0;
	}
}

DWORD SRFileSize( HFILE  hFile)
{
#ifdef	WJPEG4
	chkretired();
	return( (DWORD)0 );
#else	// !WJPEG4
	DWORD lCur, lLen;

	lCur = _llseek( hFile, 0L, SEEK_CUR );	// Save current position
	lLen = _llseek( hFile, 0L, SEEK_END );	// Move to END OF FILE
	_llseek( hFile, lCur, SEEK_SET );	// And back to Current

	return( lLen );
#endif	// WJPEG4 y/n
}

// Input MODE
//             mode: "r", "w", "a", "r+", "w+", "a+", "c", "n"
//                   ("t" or "b" appended to <mode> indicates type)
// Flags when using _open ...
//             oflag: _O_APPEND, _O_BINARY, _O_CREAT, _O_EXCL, _O_RDONLY,
//                    _O_RDWR, _O_TEXT, _O_TRUNC, _O_WRONLY
//                    (may be joined by |)
//             pmode: _S_IWRITE, _S_IREAD, _S_IREAD | _S_IWRITE
//
//  Returns:   a handle if successful, or -1 if not.
//             errno:  EACCES, EEXIST, EINVAL, EMFILE, ENOENT
// See also:  _close, _sopen, _creat, _dup, fopen
// ==============================
// 	Dos structure - at DTA
//	----------------------
//dsrchstr	struc
// dsr_resv	db	21 dup(?)
// dsr_attr	db	?
// dsr_wtime	dw	?
// dsr_wdate	dw	?
// dsr_size	dd	?
// dsr_name	db	13 dup(?)
//dsrchstr	ends
// ==============================

typedef	struct tagFILEFIND {	/* ff */
	int	ffAttr;
	unsigned int	ffTime;
	unsigned int	ffDate;
	unsigned long	ffSize;
	unsigned int	ffFNLen;
	char	ffByte[260];
} FILEFIND;

typedef FILEFIND MLPTR LPFILEFIND;

typedef	struct tagDGETFILE {	/* gf */
	int	gfMxCnt;
	int	gfCount;
	int	gfAttr;
	char	gfMask[260];
	LPFILEFIND	gfName;	// For full file name, attribute, etc
} DGETFILE;
typedef DGETFILE MLPTR LPDGETFILE;

// DOS File System Attributes
#define	dfReadOnly		0x01		// Read ONLY
#define	dfHidden			0x02		// Hidden
#define	dfSystem			0x04		// System File
#define	dfVolume			0x08		// Volume label
#define	dfDirectory		0x10		// Directory
#define	dfArchive		0x20		// Archive attribute

DGETFILE	DGetFil;
FILEFIND	DFilFnd;

#define	DFHour( a )		(( a & 0xf800 ) >> 11 )
#define	DFMins( a )		(( a & 0x07e0 ) >> 5  )
#define	DFSecs( a )		(( a & 0x001f ) *  2  )
#define	DFYear( a )		((( a & 0xfe00 ) >> 9  ) + 80)
#define	DFMonth( a )	(( a & 0x01e0 ) >> 5  )
#define	DFDay( a )		( a & 0x001f )

char	szRFilEnt[] = "Input: %s %10ld %02d/%02d/%02d %02d:%02d\r\n";

LOCAL void
ShowFile( LPFILEFIND lpff )
{
	WORD	fhr, fmin, fsec, fyr, fmth, fdy;
	char	buf[260];

	fhr = (WORD) DFHour( lpff->ffTime );
	fmin = (WORD) DFMins( lpff->ffTime );
	fsec = (WORD) DFSecs( lpff->ffTime );
	fyr = (WORD) DFYear( lpff->ffDate );
	fmth = (WORD) DFMonth( lpff->ffDate );		
	fdy = (WORD) DFDay( lpff->ffDate );
	gw_fstrcpy( &buf[0], &lpff->ffByte[0] );
	fprintf( msgout, 
		(LPSTR)&szRFilEnt[0], 
		&buf[0], 
		lpff->ffSize, 
		fdy, fmth, fyr, fhr, fmin );
	flushmsg( msgout );
}

#ifdef	WIN32

#define	READ	OF_READ
#define	WRITE	OF_WRITE

#endif	// WIN32

/* =============================================================== */
GLOBAL FILETYPE
in_open( char MLPTR fn, char MLPTR m )
{
	int	i, j;
	char	c;
	WORD	md;
	//int	hnd
	int	oflag, omode;
	LPDGETFILE	lpgf;
	HFILE	hf;

	hf = 0;
	md = 0;
	j = gw_fstrlen( fn );
	i = gw_fstrlen( m );
	oflag = 0;
	omode = 0;
//	ReadSize = 0;
	for( j = 0; j < i; j++ )
	{
		c = m[j];	/* Get char ... */
		switch( c )
		{
			case 'r':
				md |= READ;
//				oflag |= _O_BINARY;
//				omode |= _S_IREAD;
				break;
			case 'w':
//				oflag |= _O_BINARY;
//				omode |= _S_IWRITE;
				md |= WRITE;
				break;
		}	
	}
//	if( ( hnd = _open( fn, oflag, omode ) ) && (hnd != (-1)) )
	if( fn && fn[0] )
	{
//		_close( hnd );
		lpgf = &DGetFil;
		lpgf->gfMxCnt = 1;
		lpgf->gfCount = 0;
		lpgf->gfAttr =	dfArchive;	/* Archive attribute */
		gw_fstrcpy( &lpgf->gfMask[0], fn );
		lpgf->gfName = (LPFILEFIND) &DFilFnd;	/* For full name, attr, etc ... */
//		if( (i = (int) SRFindFirst( lpgf )) == 1 )
//		{
//			if( e_methods.trace_level > 0 )
//			{
//				ShowFile( &DFilFnd );	/* Found file information ... */
//			}
//		}
	}
//	return( fopen( fn, m ) );
#ifdef	WJPEG4
	chkretired();
	hf = HFILE_ERROR;
#else	// !WJPEG4
	hf = _lopen( (LPSTR) fn, md );
//	ReadSize = 0;	/* Nothing read yet ... */
	if( hf && (hf == HFILE_ERROR) )
	{
		hf = 0;
	}
	else if( hf )
	{
		set_file_size( (DWORD) SRFileSize( hf ) );
	}
#endif	// WJPEG4 y/n
	return( hf );
}

GLOBAL FILETYPE
out_open( char MLPTR fn, char MLPTR m )
{
	int	i, j;
	char	c;
	WORD	md;
	//int	hnd
	int	oflag, omode;
	HFILE	hf;
#ifndef	WJPEG4
	OFSTRUCT of;
#endif	// !WJPEG4

	hf = 0;
	md = 0;
	j = gw_fstrlen( fn );
	i = gw_fstrlen( m );
	oflag = 0;
	omode = 0;
//	ReadSize = 0;
	for( j = 0; j < i; j++ )
	{
		c = m[j];	/* Get char ... */
		switch( c )
		{
			case 'r':
				md |= READ;
//				oflag |= _O_BINARY;
//				omode |= _S_IREAD;
				break;
			case 'w':
//				oflag |= _O_BINARY;
//				omode |= _S_IWRITE;
				md |= WRITE;
				break;
		}	
	}
#ifdef	WJPEG4
	chkretired();
	hf = HFILE_ERROR;
#else	// !WJPEG4
	hf = OpenFile((LPSTR) fn, &of, OF_CREATE|OF_READWRITE);
#endif	// WJPEG4 y/n
	return( hf );
}

// ====================================================
#ifdef	ADDOLDTO
// This added back, at request ...

GLOBAL FILETYPE
gout_open( char MLPTR fn )
{
	FILETYPE	hf;
	OFSTRUCT of;
	hf = OpenFile((LPSTR) fn, &of, OF_CREATE|OF_READWRITE);
	return( hf );
}
// but an application should do its OWN file i/o
GLOBAL size_t
gout_write( HFILE hFile, char MLPTR lpData, size_t cnt )
{
	FILETYPE	hf;
	size_t			wtn;
	if( ( wtn = cnt ) &&
		( lpData ) &&
		( hf = hFile ) &&
		( hf != HFILE_ERROR ) )
	{
		wtn = _lwrite( hf, lpData, cnt );
	}
	return( wtn );
}

GLOBAL int
gout_close( HFILE hFile )
{
	HFILE	hf;
	int	i = 0;
	if(	( hf = hFile ) &&
		( hf != HFILE_ERROR ) )
	{
		_lclose( hf );
		i = 1;
	}
	return( i );
}

#endif	// ADDOLDTO


/* =============================================================== */
void	chkread( void )
{
	int	i;
	i = 0;
}

GLOBAL DWORD
get_file_size( void )
{
	return ((DWORD) DFilFnd.ffSize);
}

GLOBAL void
set_file_size( DWORD fs )
{
		DFilFnd.ffSize = fs;
}

GLOBAL	UINT	
bmpread( HFILE hf, PINT8 buf, UINT len )
{
	PINT8	ptr;
	UINT	cnt;
	UINT 	rd1, rd2;
	cnt = 0;
	rd2 = 0;
	if( lpMBuf && ddMaxCnt && ddBufCnt )
	{
			ptr = buf;
			while( ddBufCnt && (cnt < len) )
			{
					ptr[cnt++] = (BYTE) getc( hf );
			}
	}
	else
	{
		if( hf && (hf != HFILE_ERROR) )
		{
			ptr = buf;
			if( BufCnt && (hf == FilHnd) )
			{
				while( BufCnt && (cnt < len) )
				{
					ptr[cnt++] = (BYTE) getc( hf );
				}
			}
			if( cnt < len )
			{
				rd1 = len - cnt;
				ptr = &buf[cnt];
#ifdef	WJPEG4
				chkretired();
				chkread();
#else	// !WJPEG4
				if( rd2 = _lread( hf, ptr, rd1 ) )
					cnt = cnt + rd2;
				else
					chkread();
#endif	// WJPEG4 y/n
			}
		}
	}
return( cnt );
}

void	MLPTR RetAddr = 0;
void	MLPTR mra;
#ifdef	WIN32
DWORD	RetBP = 0;
DWORD	RetESP = 0;
DWORD	mBp;
DWORD	mSp;
#else
WORD	RetBP = 0;
WORD	RetSP = 0;
WORD	mBp;
WORD	mSp;
#endif	// WIN32 y.n

/* ====================================================================
 * SetError( int )
 * Until such time that the WINDOWS Library can EXIT-ON-ERROR we
 * will keep the list of up the last 20 errors ...
 * ==================================================================== */
#define	MXERRS		20		/* Keep list of up to 20 errors */
int	ErrList[MXERRS];
int	ErrCount = 0;

/* ====================================================================
 * SetWarning( int )
 * Keep a list of up the last 20 WARNINGS ...
 * ==================================================================== */
#define	MXWARNS		20		/* Keep list of up to 20 errors */
int	WarnList[MXWARNS];
int	WarnCount = 0;
int	SetWarning( int i )
{
int	j;
	if( WarnCount )
	{
		for( j = 0; j < WarnCount; j++ )
		{
			if( WarnList[j] == i )
				break;
		}
		if( (j == WarnCount) && ((WarnCount + 1) < MXWARNS) )
		{
			WarnList[WarnCount] = i;
			WarnCount++;
		}
	}
	else
	{
		WarnList[WarnCount] = i;
		WarnCount++;
	}
return( i );
}


GLOBAL	int
GetErrorMsg( LPSTR lpb, WORD val )
{
LPBADEXIT lpB;
int	i, j, k, l;
	i = 0;
	lpB = &BadExit[0];	/* Point to FIRST ... */
	if( lpb && val )
	{
		while( lpB->beVal )
		{
			if( lpB->beVal == val )
				break;
			lpB++;
		}
		if( (lpB->beVal == val) && lpB->beStg )
		{
			lstrcpy( lpb, lpB->beStg );
		}
		else
		{
			wsprintf( lpb, (LPSTR)&szRUnkE[0], val, val );
		}
		if( ErrCount && ((i = lstrlen( lpb )) < MXERRBUF) )
		{
			for( j = 0; j < ErrCount; j++ )
			{
				k = ErrList[j];
				if( k )
				{
					lpB = &BadExit[0];	/* Point to FIRST ... */
					while( lpB->beVal )
					{
						if( lpB->beVal == (WORD) k )
						break;
						lpB++;
					}
					if( (lpB->beVal == (WORD) k) && lpB->beStg )
					{
						i = lstrlen( lpb );
						l = lstrlen( lpB->beStg );
						if( (i + l) < MXERRBUF )
							lstrcat( lpb, lpB->beStg );
						else
							break;
					}
				}
			}
		}
		i = lstrlen( lpb );
	}
return( i );
}

GLOBAL	int
GetWarnMsg( LPSTR lpb )
{
LPBADEXIT lpB;
int	i, j, k, l;
	i = 0;
	lpB = &BadExit[0];	/* Point to FIRST ... */
	if( lpb )
	{
		if( WarnCount && ((i = lstrlen( lpb )) < MXERRBUF) )
		{
			for( j = 0; j < WarnCount; j++ )
			{
				k = WarnList[j];
				if( k )
				{
					lpB = &BadExit[0];	/* Point to FIRST ... */
					while( lpB->beVal )
					{
						if( lpB->beVal == (WORD) k )
						break;
						lpB++;
					}
					if( (lpB->beVal == (WORD) k) && lpB->beStg )
					{
						i = lstrlen( lpb );
						l = lstrlen( lpB->beStg );
						if( (i + l) < MXERRBUF )
							lstrcat( lpb, lpB->beStg );
						else
							break;
					}
				}
			}
		}
		i = lstrlen( lpb );
	}
return( i );
}

/* ===============================================================
 * wsscanf( lpData, lpFormat, lpList )
 * A replacement of sscanf(...) which appears missing in WINDOWS
 * Presently ONLY handles %d, %ld, %c ..
 * =============================================================== */
GLOBAL int
wsscanf( LPSTR lpd, LPSTR lpf, void FAR * lpv )
{
int	fcnt;
int	i, j, k, l, v;
char	c;
BOOL	lng, gtf, bn;
LPWORD	lpw;
DWORD	dd;
	lpw = lpv;
	gtf = lng = FALSE;
	fcnt = 0;
	v = l = k = 0;
	if( lpd && lpf && (i = lstrlen( lpd )) && (j = lstrlen( lpf )) )
	{
NxtForm:
		for( ; k < j; k++ )
		{
			c = lpf[k];	/* Get format char ... */
			if( c == '%' )	/* Start of format? */
			{
				k++;
				if( k < j )
				{
					c = lpf[k];
					if( c == 'l' )
					{
						lng = TRUE;
						k++;
						c = lpf[k];
						if( c == 'd' )
						{
							gtf = TRUE;
							break;
						}
					}
					else if( c == 'd' )
					{
						gtf = TRUE;
						break; 
					}
					else if( c == 'c' )
					{
						gtf = TRUE;
						break;
					}
				}	/* and there is length ... */
			}	/* Got format char ... */
		}	/* For the format length ... */
		if( gtf )
		{
			lpw[v] = 0;
			if( lng )
				lpw[v+1] = 0;
			switch( c )
			{
				case 'd':
					if( l < j )
					{
						bn = FALSE;
						for( ; l < j; l++ )
						{
							c = lpd[l];
							if( (c >= '0') && (c <= '9') )	/* If in decimal */
							{
								if( bn )
								{
									dd = (dd * 10) + ( c - '0' );
								}
								else
								{
									bn = TRUE;
									dd = ( c - '0' );
								}
							}
							else if( bn )
							{
								lpw[v] = LOWORD( dd );
								v++;
								if( lng )
								{
									lpw[v] = HIWORD( dd );
									v++;
								}
								fcnt++;
								break;
							}
						}
					}
					break;
				case 'c':
					if( l < j )
					{
						for( ; l < j; l++ )
						{
							c = lpd[l];
							if( c > ' ' )
							{
								l++;
								lpw[v] = (WORD) c;
								v++;
								fcnt++;
							}
						}						
					}
					break;
			}	/* Switch case ... */
			gtf = FALSE;
			goto NxtForm;
		}	/* We have a format type ... */
	}
return( fcnt );
} 

#define	MXIO	256
BYTE	fbuf[MXIO];
WORD	fcnt = 0;

int b_fflush( HFILE fHand )
{
	int	i;
	PINT8	lout;
	PINT8	lpo;
	if( i = fcnt )
	{
		fcnt = 0;	/* Clear the BUFFER counter ... regardless */
		if( DSdinfo.hgOut )
		{
			if( lout = JGlobalLock( DSdinfo.hgOut ) )
			{
				lpo = lout + DSdinfo.ddOut;
				BMPcopy( lpo, ( PINT8 ) &fbuf[0], (DWORD) i );
				DSdinfo.ddOut += i;
				JGlobalUnlock( DSdinfo.hgOut );
			}
			else
			{
				ERRFLAG( BAD_MEMORY12 );
			}
		}
		else if( fHand && i &&
			( fHand != HFILE_ERROR ) )
		{
// My PRIVATE Write JPG Functions - TO A DISK FILE
// ===============================================
//#ifdef	WJPEG4
//			ERRFLAG( BAD_RETIRED );
//#else	// !WJPEG4
			if( _lwrite( fHand, ( PINT8 ) &fbuf[0], i ) != (UINT) i )
			{
#ifdef	_INC_WINDOWS
				ERRFLAG( BAD_WRITOUT );
#else
		    	ERREXIT(dcinfo->emethods, "Output file write error --- out of disk space?");
#endif
			}
//#endif	// WJPEG4 y/n
		}
		else if( i && fHand )
		{
				ERRFLAG( BAD_WRITOUT );
		}
	}
	return( i );	// Return COUNT written
}

int b_putc( int c, HFILE Hand )
{
	fbuf[fcnt] = (BYTE) c;	/* Just slam it into a BUFFER ... */
	fcnt++;		/* And bump the input counter ... */
	if( fcnt >= MXIO )	/* If reached maximum (or in error more!), then ... */
	{
		b_fflush( Hand );	/* Put this block to DISK ... */
	} 
return( 0 );
}

GLOBAL void
winp_term( void )
{
	FilHnd = 0;
	BufCnt = 0;
	BufOut = 0;
	fcnt = 0;
}

#ifdef	WJPEG4
GLOBAL int
get_jpeg_data (decompress_info_ptr cinfo)
{
	int	i, j, c;
	HFILE infile = cinfo->input_file;
	cinfo->next_input_byte = cinfo->input_buffer + MIN_UNGET;
	if( ddBufCnt && lpMBuf )
	{
		if( ddBufCnt < JPEG_BUF_SIZE )
			j = LOWORD(ddBufCnt);
		else
			j = JPEG_BUF_SIZE;
		for( i = 0; i < j; i++ )
		{
			if( (c = getc(infile)) == EOF )
				break;
			cinfo->next_input_byte[i] = c;
		}
		if( i < j )
		{
			cinfo->next_input_byte[i++] = c;
			if( i < j )
			{
				j = 0xd9;
				cinfo->next_input_byte[i++] = (char)j;
			}
		}
		cinfo->bytes_in_buffer = i;
	}
	else
	{
#ifndef	_INC_WINDOWS
		WARNMS(cinfo->emethods, "Premature EOF in JPEG file");
#endif	// !_INC_WINDOWS
		i = 0xff;
		cinfo->next_input_byte[0] = (char) i;
		i = 0xd9;
		cinfo->next_input_byte[1] = (char) i; /* EOI marker */
		cinfo->bytes_in_buffer = 2;
	}
	return JGETC(cinfo);
}

#else	// !WJPEG4
GLOBAL int
get_jpeg_data (decompress_info_ptr cinfo)
{
	int	i, c;
	register HFILE infile = cinfo->input_file;
	cinfo->next_input_byte = cinfo->input_buffer + MIN_UNGET;
// Original BLOCK read ...
//		cinfo->bytes_in_buffer = (int) JFREAD(cinfo->input_file,
//					cinfo->next_input_byte,
//					JPEG_BUF_SIZE);
// replaced with BYTE read/get from memory ...
#ifdef	WIN32
	i = 0;
	c = 0;
	if( ddBufCnt < JPEG_BUF_SIZE )
	{
		if( ddBufCnt )
		{
			i = ddBufCnt;
			c = EOF;
		}
	}
	else
	{
		i = JPEG_BUF_SIZE;
		c = 0;
	}
	if( i )
		memmove( &cinfo->next_input_byte[0], (lpMBuf + ddOutCnt), i );
	ddOutCnt += i;
	ddBufCnt -= i;
	if( c == EOF )
	{
		cinfo->next_input_byte[i++] = c;
	}
#else	// !WIN32
	for( i = 0; i < JPEG_BUF_SIZE; i++ )
	{
		if( (c = getc(infile)) == EOF )
		{
			break;
		}
		cinfo->next_input_byte[i] = c;
	}
#endif	// WIN32 y/n
	cinfo->bytes_in_buffer = i;
	if( cinfo->bytes_in_buffer <= 0 )	/* IF EOF!!! */ 
	{
#ifndef	_INC_WINDOWS
		WARNMS(cinfo->emethods, "Premature EOF in JPEG file");
#endif	// !_INC_WINDOWS
		i = 0xff;
		cinfo->next_input_byte[0] = (char) i;
		i = 0xd9;
		cinfo->next_input_byte[1] = (char) i; /* EOI marker */
		cinfo->bytes_in_buffer = 2;
	}
	return JGETC(cinfo);
}

#endif	// WJPEG4 y/n

// =========== The following contain ASSEMBLER =============
// For the 16-BIT compiler, TURN OFF the 
// GLOBAL OPTIMIZATION WARNING with INLINE Assembler!!!

#if		( _MSC_VER > 700 )
#if		(defined( NDEBUG ) && defined( Dv16_App ))
// **************************************************************
#pragma warning( disable : 4704 )
// **************************************************************
#endif	// NDEBUG & Dv16_App
#endif	// _MSC_VER > 700

int	SetError( int i )
{
int	j;
	if( bErrFlag )
	{
		if( bErrFlag != i )
		{
			j = 0;
			if( ErrCount )
			{
				for( j = 0; j < ErrCount; j++ )
				{
					if( ErrList[j] == i )
						break;
				}
			}
			if( (j == ErrCount) && ((ErrCount + 1) < MXERRS) )
			{
				ErrList[ErrCount] = i;
				ErrCount++;
			}
		}	/* Not the first error ... */
	}
	else
	{
		bErrFlag = i;
	}
#ifndef WIN64
#ifdef	WIN32
	if( (mra = RetAddr) && 
		(mBp = RetBP) && 
		(mSp = RetESP) )
	{
		RetAddr = 0;
		RetBP = 0;
		RetESP = 0;
		_asm
		{
			mov		ebp,[mBp]
			mov		esp,[mSp]
			jmp		[mra]
		}
#else	// !WIN32
	if( (mra = RetAddr) && 
		(mBp = RetBP) && 
		(mSp = RetSP) )
	{
		RetAddr = 0;
		RetBP = 0;
		RetSP = 0;
		_asm
		{
			mov	bp,[mBp]				; Get original BP back ...
			mov	sp,[mSp]
			jmp	dword ptr [mra]	; Out of here ...
		}
#endif	// WIN32 y/n
	}
#endif // #ifndef WIN64
return( i );
}

#ifndef WIN64
#ifdef	WIN32
GLOBAL	void
setexitjump( void MLPTR Ex, DWORD eSp )
{
	RetAddr = Ex;
	_asm
	{
		mov	eax,[ebp]
		mov	[RetBP],eax
	}
	RetESP = eSp;
}
#else	// !WIN32
GLOBAL	void
setexitjump( void MLPTR Ex, WORD Sp )
{
	RetAddr = Ex;
	_asm
	{
		mov	ax,[bp]
		mov	RetBP,ax
	}
	RetSP = Sp;
}
#endif	// WIN32 y/n
#endif // #ifndef WIN64

#if		( _MSC_VER > 700 )
#if		(defined( NDEBUG ) && defined( Dv16_App ))
// **************************************************************
#pragma warning( default : 4704 )
// **************************************************************
#endif	// NDEBUG & Dv16_App
#endif	// _MSC_VER > 700

// =========== The ABOVE contain ASSEMBLER ===========
//====================================================
#endif	// _INC_WINDOWS


// eof - WCommon.c

