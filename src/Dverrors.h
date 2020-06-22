
/* errors.h */
#ifndef	_ERRORS_H
#define	_ERRORS_H

/* Convert to FIXED Number, rather than enum ... 16 March 96 - Geoff. */
/* Error Definition Table - See DvRes.H for earlier definitions */
#define ERR_MIN               5500    // All error #s >= this value
#define ERR_NOT_DIB           5501    // Tried to load a file, NOT a DIB!
#define ERR_MEMORY            5502    // Not enough memory!
#define ERR_READ              5503    // Error reading file!
#define ERR_LOCK              5504    // Error on a GlobalLock()!
#define ERR_OPEN              5505    // Error opening a file!
#define ERR_CREATEPAL         5506    // Error creating palette.
#define ERR_GETDC             5507    // Couldn't get a DC.
#define ERR_CREATECHILD       5508    // Error creating MDI child.
#define ERR_CREATEDDB         5509    // Error create a DDB.
#define ERR_STRETCHBLT        5510    // StretchBlt() returned failure.
#define ERR_STRETCHDIBITS     5511    // StretchDIBits() returned failure.
#define ERR_NODIBORDDB        5512    // Error painting -- need BOTH DDB & DIB.
#define ERR_SETDIBITSTODEVICE 5513    // SetDIBitsToDevice() failed.
#define ERR_STARTDOC          5514    // Error calling StartDoc().
#define ERR_NOGDIMODULE       5515    // Couldn't find GDI module in memory.
#define ERR_SETABORTPROC      5516    // Error calling SetAbortProc().
#define ERR_STARTPAGE         5517    // Error calling StartPage().
#define ERR_NEWFRAME          5518    // Error calling NEWFRAME escape.
#define ERR_ENDPAGE           5519    // Error calling EndPage().
#define ERR_ENDDOC            5520    // Error calling EndDoc().
#define ERR_ANIMATE           5521    // Only one DIB animated @ a time.
#define ERR_NOTIMERS          5522    // No timers avail for pal animation.
#define ERR_NOCLIPWINDOW      5523    // Now current window for clipboard.
#define ERR_CLIPBUSY          5524    // Clipboard is busy.
#define ERR_NOCLIPFORMATS     5525    // During paste can't find DIB or DDB.
#define ERR_SETDIBITS         5526    // Error calling SetDIBits().
#define ERR_FILENOTFOUND      5527    // Error opening file in GetDib()
#define ERR_WRITEDIB          5528    // Error writing DIB file.
#define ERR_NO_CONV           5529    // Can NOT convert file to Bitmap.
#define ERR_WN_CONV           5530    // WARNING: Error in file conversion!
#define ERR_NO_RLE            5531    // ERROR: Unsupported file format!
#define ERR_UNKNOWNF          5532    // ERROR: Unknown file format!
#define ERR_INTERROR          5533    // WARNING: Some internal failure
#define ERR_NO_LIB            5534    // ERROR: Unable to load WJPEGxxx.DLL
#define ERR_NO_REOPEN         5535    // ERROR: Unable to RE-OPEN GIF file 
#define ERR_MEMHANDLE         5536    // ERROR: Unable to ACCESS memory
#define ERR_NOLCONV           5537    // ERROR: Library FAILED in conversion of GIF to Bitmap.
#define ERR_NOLSIZE           5538    // ERROR: Library FAILED to give SIZE of BMP!
#define ERR_NOLCONF           5539    // ERROR: Library rejected Configuration parameters! 
#define ERR_NOGIF             5540    // ERROR: Output of GIF file failed!
#define ERR_COPYPAL           5541    // Error creating and copying a palette for a 24-Bit image.
#define	ERR_RETIRED           5542    // ERROR: This/these Interfaces have been RETIRED!
#define	ERR_PARAMS            5543    // ERROR: This is an INTERNAL Error\r\n ...
#define ERR_NULSIZE           5544    // ERROR: File is ZERO Length!
#define	ERR_NOLSIZEG		5545
// "ERROR: Library FAILED in WGIFSIZ? function to give SIZE of BMP! "},
#define ERR_NOLSIZEJ		5546
// "ERROR: Library FAILED to WJPGSIZE? get SIZE of BMP! "},
#define	ERR_NOBMP2JPG		5547
//"ERROR: Library FAILED in conversion of BMP to JPEG file! ";
#define	ERR_NO_MRF			5548	// No recent file list
#define	ERR_NO_COPY			5549	//"ERROR: Copying of DIB handle FAILED!\r\nPerhaps insufficient memory! "
#define	ERR_NOT_YET			5550	//"PARDON! This function has NOT yet been implemented! "
#define	ERR_NO_INFO			5551	//"ERROR: GetWindowExtra (WW_DIB_INFO) FAILED for this MDI Window! "
#define	ERR_NO_LOCK			5552	//"ERROR: Handle to DIB Info structure failed to convert to memory! "
#define	ERR_NO_PMDIB		5553	//"ERROR: Function PMDibFromBitmap FAILED to return a handle! "
#define	ERR_NO_WINDIB		5554	//"ERROR: Function WinDibFromBitmap FAILED to return a handle! "

//  *** Add NEW Error values here ***
// ===================================
#define ERR_MAX               5999    // All error #s < this value

#define INF_MIN               6000	// Commence of INFORMATION range
#define INF_LIBSET            6001	// Appears WJPEG DLL has accepted changed Configuration! "}
#define INF_LIBERR            6002  // Warning: WJPEG DLL rejected changed Configuration!

#define INF_MAX               6999

extern void DIBInfo( DWORD );	/* Display an Indexed INFORMATION string */
extern void DIBError( int );	/* Display INDEXED ERROR string */
extern void DIBError2( int, LPSTR );	/* Display INDEXED ERROR *PLUS* string */
extern void DIBErrorStg( LPSTR );	/* Display an ERROR string */
extern int DIBEString( LPSTR, int );	/* Get INDEXED ERROR string - ret Length */
void DvError( LPSTR, LPSTR  );	// Information 
void DvErrorN( WORD, WORD );

// Internal ERROR NUMBER *AND* system GetLastError()
extern	void	SysDIBError( int ErrNo, DWORD dwi );

#endif	/* _ERRORS_H */

/* eof */
