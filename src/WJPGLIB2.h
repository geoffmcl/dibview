

// WjpgLib2.h
// ======================================================
// June 1997 - Build WJPG2BMP.DLL
// Nov  1997 - Moved DV32 to F:\GTOOLS32\DV32 and
//		renamed this DLL to WJPG32_2.DLL
// Feb  1998 - Moved DLL source to DV32\WJPG32_2  
// =================================================
#ifndef	_WJPGLIB2_H
#define	_WJPGLIB2_H

#ifdef	__cplusplus
extern	"C"
{
#endif	// __cplusplus


// 2 NEW Functions for JPEG decompression
// ===============
// One SIZE Functions
EXPORT32
DWORD MLIBCALL WJPGSIZE6( HGLOBAL, DWORD, LPSIZE );

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

EXPORT32
BOOL MLIBCALL WJPG2BMP6( HGLOBAL, DWORD, HGLOBAL, HGLOBAL);

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

// And for static library loading ... as in DibView
typedef DWORD (MLIBCONV *LPWJPGSIZE6) ( HGLOBAL, DWORD, LPSIZE );
typedef BOOL  (MLIBCONV *LPWJPG2BMP6) ( HGLOBAL, DWORD, HGLOBAL, HGLOBAL);

// Then a SPECIALISED BMP to JPEG Service
// from the IJG 6a source - Added June, 1997
EXPORT32
WORD MLIBCALL WBMPTOJPG6( HGLOBAL, DWORD, HGLOBAL, LPSTR );
// Where
// HGLOBAL hgIn - Handle to INPUT Data to be compressed
// DWORD   InSz - Size of INPUT Data
// HGLOBAL hgInf- Just in INFORMATION buffer for string errors
// LPSTR   lpFile - OUPUT File NAME.
// Returns:
// WORD  = 0 = Success. File written
// else 1++ ie sometimes an ERROR VALUE
typedef WORD (MLIBCONV *LPWBMPTOJPG6) ( HGLOBAL, DWORD, HGLOBAL, LPSTR );

// And this is an IN MEMORY version. ie NO File I/O is done.
// NOTE: The returned data has a DWORD at its head,
//		advising the SIZE of the JPEG data block, if it needs
//		to, say written to disk AFTER conversion.
EXPORT32
WORD MLIBCALL WBMP2JPG6( HGLOBAL, DWORD, HGLOBAL, HGLOBAL );
// Where
// HGLOBAL hgIn - Handle to INPUT Data to be compressed
// DWORD   InSz - Size of INPUT Data
// HGLOBAL hgInf- Just in INFORMATION buffer for string errors
// LPSTR   lpFile - OUPUT File NAME.
// Returns:
// WORD  = 0 = Success. JPEG data in buffer, where the FIRST
//		DWORD is the data's length.
// else 1++ ie sometimes an ERROR VALUE
typedef WORD (MLIBCONV *LPWBMP2JPG6) ( HGLOBAL, DWORD, HGLOBAL, HGLOBAL );

// Close the CPP Block
#ifdef	__cplusplus
}
#endif	// __cplusplus

#endif	//	_WJPGLIB2_H
// eof - WjpgLib2.h
// ================
