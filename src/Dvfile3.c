
// DvFile3.c - Based on srfile.c 

#include	"dv.h"
#include	"DvSrch.h"	// Was "srsearch.h"

#ifdef	WIN32
// ======================================================

#define	_fmemcpy	memcpy

//extern void	GetFPath( LPSTR, LPSTR, LPSTR, LPSTR, LPSTR );


BOOL	fPassAll = TRUE;
BOOL	fAddPath = TRUE;

//The FindFirstFile function searches a directory for a file whose name matches the specified filename. //FindFirstFile examines subdirectory names as well as filenames. 
//HANDLE FindFirstFile(
//    LPCTSTR lpFileName,	// pointer to name of file to search for
//LPWIN32_FIND_DATA lpFindFileData 	// pointer to returned
//information 
//   );	
//Parameters
//lpFileName
//Windows 95: Points to a null-terminated string that specifies a
//valid directory or path and filename, which can contain wildcard
//characters (* and ?). This string must not exceed MAX_PATH
//characters.
//Windows NT: Points to a null-terminated string that specifies a
//valid directory or path and filename, which can contain wildcard
//characters (* and ?). 
//There is a default string size limit for paths of MAX_PATH
//characters. This limit is related to how the FindFirstFile
//function parses paths. An application can transcend this limit
//and send in paths longer than MAX_PATH characters by calling the
//wide (W) version of FindFirstFile and prepending "\\?\" to the
//path. The "\\?\" tells the function to turn off path parsing; it
//lets paths longer than MAX_PATH be used with FindFirstFileW.
//This also works with UNC names. The "\\?\" is ignored as part of
//the path. For example, "\\?\C:\myworld\private" is seen as
//"C:\myworld\private", and "\\?\UNC\bill_g_1\hotstuff\coolapps"
//is seen as "\\bill_g_1\hotstuff\coolapps".
//lpFindFileData
//Points to the WIN32_FIND_DATA structure that receives
//information about the found file or subdirectory. The structure
//can be used in subsequent calls to the FindNextFile or FindClose
//function to refer to the file or subdirectory. 
//Return Values
//If the function succeeds, the return value is a search handle
//used in a subsequent call to FindNextFile or FindClose.
//If the function fails, the return value is INVALID_HANDLE_VALUE.
//To get extended error information, call GetLastError. 
//Remarks
//The FindFirstFile function opens a search handle and returns
//information about the first file whose name matches the
//specified pattern. Once the search handle is established, you
//can use the FindNextFile function to search for other files that
//match the same pattern. When the search handle is no longer
//needed, close it by using the FindClose function. 
//This function searches for files by name only; it cannot be used
//for attribute-based searches. 
//See Also
//FindClose, FindNextFile, GetFileAttributes, SetFileAttributes,
////WIN32_FIND_DATA 

//The WIN32_FIND_DATA structure describes a file found by the FindFirstFile or FindNextFile //function. 
//typedef struct _WIN32_FIND_DATA { // wfd  
//    DWORD dwFileAttributes; 
//    FILETIME ftCreationTime; 
//    FILETIME ftLastAccessTime; 
//    FILETIME ftLastWriteTime; 
//    DWORD    nFileSizeHigh; 
//    DWORD    nFileSizeLow; 
//    DWORD    dwReserved0; 
//    DWORD    dwReserved1; 
//    TCHAR    cFileName[ MAX_PATH ]; 
//    TCHAR    cAlternateFileName[ 14 ]; 
//} WIN32_FIND_DATA; 
//Members
//dwFileAttributes
//Specifies the file attributes of the file found. This member can
//be one or more of the following values: 
//Value	Meaning
//FILE_ATTRIBUTE_ARCHIVE	
//	The file is an archive file. Applications use this value to
//mark files for backup or removal.
//FILE_ATTRIBUTE_COMPRESSED	
//	The file or directory is compressed. For a file, this means
//that all of the data in the file is compressed. For a directory,
//this means that compression is the default for newly created
//files and subdirectories.
//FILE_ATTRIBUTE_DIRECTORY	
//	The file is a directory.
//FILE_ATTRIBUTE_HIDDEN	
//	The file is hidden. It is not included in an ordinary directory
//listing.
//FILE_ATTRIBUTE_NORMAL	
//	The file has no other attributes set. This value is valid only
//if used alone.
//FILE_ATTRIBUTE_OFFLINE	
//	The data of the file is not immediately available. Indicates
//that the file data has been physically moved to offline storage.
//FILE_ATTRIBUTE_READONLY	
//	The file is read-only. Applications can read the file but
//cannot write to it or delete it.
//FILE_ATTRIBUTE_SYSTEM	
//	The file is part of the operating system or is used exclusively
//by it.
//FILE_ATTRIBUTE_TEMPORARY	
//	The file is being used for temporary storage. Applications
//should write to the file only if absolutely necessary. Most of
//the file's data remains in memory without being flushed to the
//media because the file will soon be deleted.
//ftCreationTime
//Specifies a FILETIME structure containing the time the file was
//created. FindFirstFile and FindNextFile report file times in
//Coordinated Universal Time (UTC) format. These functions set the
//FILETIME members to zero if the file system containing the file
//does not support this time member. You can use the
//FileTimeToLocalFileTime function to convert from UTC to local
//time, and then use the FileTimeToSystemTime function to convert
//the local time to a SYSTEMTIME structure containing individual
//members for the month, day, year, weekday, hour, minute, second,
//and millisecond. 
//ftLastAccessTime
//Specifies a FILETIME structure containing the time that the file
//was last accessed. The time is in UTC format; the FILETIME
//members are zero if the file system does not support this time
//member.
//ftLastWriteTime
//Specifies a FILETIME structure containing the time that the file
//was last written to. The time is in UTC format; the FILETIME
//members are zero if the file system does not support this time
//member.
//nFileSizeHigh
//Specifies the high-order DWORD value of the file size, in bytes.
//This value is zero unless the file size is greater than
//MAXDWORD. The size of the file is equal to (nFileSizeHigh *
//MAXDWORD) + nFileSizeLow.
//nFileSizeLow
//Specifies the low-order DWORD value of the file size, in bytes.
//dwReserved0
//Reserved for future use. 
//dwReserved1
//Reserved for future use. 
//cFileName
//A null-terminated string that is the name of the file. 
//cAlternateFileName
//A null-terminated string that is an alternative name for the
//file. This name is in the classic 8.3 (filename.ext) filename
//format. 
//Remarks
//If a file has a long filename, the complete name appears in the
//cFileName field, and the 8.3 format truncated version of the
//name appears in the cAlternateFileName field. Otherwise,
//cAlternateFileName is empty. As an alternative, you can use the
//GetShortPathName function to find the 8.3 format version of a
//filename.
//See Also
//FindFirstFile, FindNextFile, FILETIME, FileTimeToLocalFileTime,
////FileTimeToSystemTime, GetShortPathName 

//The FILETIME structure is a 64-bit value representing the number of 100-nanosecond intervals since //January 1, 1601. 
//typedef struct _FILETIME { // ft  
//    DWORD dwLowDateTime; 
//    DWORD dwHighDateTime; 
//} FILETIME; 
//Members
//dwLowDateTime
//Specifies the low-order 32 bits of the file time. 
//dwHighDateTime
//Specifies the high-order 32 bits of the file time. 
//See Also
//CompareFileTime, DosDateTimeToFileTime, FileTimeToDosDateTime,
//FileTimeToLocalFileTime, FileTimeToSystemTime, GetFileTime,
////LocalFileTimeToFileTime, SetFileTime, SystemTimeToFileTime 

//typedef struct _WIN32_FIND_DATA { // wfd  
//    DWORD dwFileAttributes; 
//    FILETIME ftCreationTime; 
//    FILETIME ftLastAccessTime; 
//    FILETIME ftLastWriteTime; 
//    DWORD    nFileSizeHigh; 
//    DWORD    nFileSizeLow; 
//    DWORD    dwReserved0; 
//    DWORD    dwReserved1; 
//    TCHAR    cFileName[ MAX_PATH ]; 
//    TCHAR    cAlternateFileName[ 14 ]; 
////} WIN32_FIND_DATA; 
WIN32_FIND_DATA	FFData;
HANDLE	FFHnd = 0;

//typedef struct tagFILEFIND {    /* ff */
//        int     ffAttr;
//        unsigned int    ffTime;
//        unsigned int    ffDate;
//        unsigned long   ffSize;
//        unsigned int    ffFNLen;
//        char    ffByte[260];
//} FILEFIND;
//typedef FILEFIND MLPTR LPFILEFIND;
//typedef struct tagDGETFILE {    /* gf */
//        int     gfMxCnt;
//        int     gfFCount;
//        int     gfDCount;
//        int     gfAttr;
//        char    gfMask[260];	// Actual FILE MASK
//        char    gfDire[260];	// And to GET DIRECTORIES
//        LPFILEFIND      gfName;
//} DGETFILE;
//typedef DGETFILE MLPTR LPDGETFILE;
DGETFILE	DGetFile = { 0 };

BOOL IsAFile( WIN32_FIND_DATA * lpd )
{
	BOOL flg = FALSE;
	if( lpd )
	{
		if( lpd->dwFileAttributes == FILE_ATTRIBUTE_NORMAL )
			flg = TRUE;
		else if( lpd->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE )
			flg = TRUE;
	}
	return( flg );
}

BOOL IsADir( WIN32_FIND_DATA * pFF )
{
	BOOL flg = FALSE;
	if( pFF->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
	{
		if( fPassAll )
		{
			flg = TRUE;
		}
		else if( pFF->cFileName[0] != '.' )
		{
			flg = TRUE;
		}
	}
	return( flg );
}

//#define _MAX_PATH   260 /* max. length of full pathname */
//#define _MAX_DRIVE  3   /* max. length of drive component */
//#define _MAX_DIR    256 /* max. length of path component */
//#define _MAX_FNAME  256 /* max. length of file name component */
//#define _MAX_EXT    256 /* max. length of extension component */
void CopyEntry( WIN32_FIND_DATA * pFF, LPFILEFIND lpff, LPSTR lpPath )
{ 
	LPSTR		lprf, lpce;
	FILETIME	ft;
	char	bufd[_MAX_DRIVE+1];
	char	bufp[_MAX_DIR+1];
	if( pFF->cFileName[0] )
		lpce = &pFF->cFileName[0];
	else if( pFF->cAlternateFileName[0] )
		lpce = &pFF->cAlternateFileName[0];
	else
		lpce = 0;
	lprf = &lpff->ffByte[0];
	if( FileTimeToLocalFileTime( &pFF->ftLastWriteTime, &ft ) )
	{
		lpff->ffDate = 0;
		lpff->ffTime = 0;
		FileTimeToDosDateTime( &ft,
			(LPWORD)&lpff->ffDate,
			(LPWORD)&lpff->ffTime );
	}
	lpff->ffAttr = 0;
	if( pFF->dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE )
		lpff->ffAttr = dfArchive;
	else if( pFF->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		lpff->ffAttr = dfDirectory;
	lpff->ffSize = FFData.nFileSizeLow;
	lprf[0] = 0;
	if( lpce )
	{
		if( fAddPath && lpPath && lpPath[0] )
		{
			DVGetFPath( lpce, &bufd[0], &bufp[0], NULL, NULL );
			if( (bufd[0] == 0) && (bufp[0] == 0) )
			{
				DVGetFPath( lpPath, &bufd[0], &bufp[0], NULL, NULL );
				lstrcpy( lprf, &bufd[0] );
				lstrcat( lprf, &bufp[0] );
				lstrcat( lprf, lpce );
			}
			else
			{
				lstrcpy( lprf, lpce );
			}
		}
		else
		{
			lstrcpy( lprf, lpce );
		}
	}
}

DWORD SRFindFirst( LPDGETFILE lpg )
{
	DWORD	fcnt;
	LPSTR	lpFN, lpDN;
	int		i, gfa;
	LPFILEFIND lpff;
	char *	lprf;
	BOOL	fgf;
	LPDGETFILE	lplgf;
	fcnt = 0;
	fgf = FALSE;

	if( FFHnd && (FFHnd != INVALID_HANDLE_VALUE) )
		FindClose( FFHnd );
	FFHnd = 0;
	lplgf = &DGetFile;
	if( lpg )
		_fmemcpy( (void *)lplgf, (void *)lpg, sizeof(DGETFILE) );

	if( lpg && 
		(lpff = lpg->gfName) &&
		lpg->gfMxCnt && (i = strlen( &lpg->gfMask[0] )) )
	{
		lpg->gfFCount = 0;
		lpg->gfDCount = 0;
		gfa = lpg->gfAttr;
		FFData.dwFileAttributes = (DWORD) gfa;
		lprf = &lpff->ffByte[0];
		lprf[0] = 0;
		lpFN = &lpg->gfMask[0];
		// Directories to be INCLUDED, find ALL directories ...
		if( gfa & dfDirectory )
		{
			lpDN = &lpg->gfDire[0];
			FFHnd = FindFirstFile( lpDN, &FFData );
			//if( ( FFHnd = FindFirstFile( lpDN, &FFData ) ) &&
			//	( FFHnd != INVALID_HANDLE_VALUE ) )
         if( VFH(FFHnd) )
			{
				if( IsADir( &FFData ) )
				{
					lpg->gfDCount++;
					fcnt++;
				}
				while( FindNextFile( FFHnd, &FFData ) )
				{
					if( IsADir( &FFData ) )
					{
						lpg->gfDCount++;
						fcnt++;
					}
				}
				FindClose( FFHnd );	// ALL the DIRECTORIES Found
				FFHnd = 0;
			}
		}

		// Find ALL Files matching MASK
		//if( ( FFHnd = FindFirstFile( lpFN, &FFData ) ) &&
		//	( FFHnd != INVALID_HANDLE_VALUE ) )
		FFHnd = FindFirstFile( lpFN, &FFData );
		if( VFH( FFHnd ) )
		{
			if( IsAFile( &FFData ) )
			{
				lpg->gfFCount++;
				fcnt++;
			}
			while( FindNextFile( FFHnd, &FFData ) )
			{
				if( IsAFile( &FFData ) )
				{
					lpg->gfFCount++;
					fcnt++;
				}
			}
			FindClose( FFHnd );
			FFHnd = 0;
		}
		// Now, return FIRST ENTRY and leave set for NEXT
		//if( lpg->gfFCount &&
		//	( FFHnd = FindFirstFile( lpFN, &FFData ) ) &&
		//	( FFHnd != INVALID_HANDLE_VALUE ) )
      FFHnd    = 0;
      if( lpg->gfFCount )
			FFHnd = FindFirstFile( lpFN, &FFData );

		if( VFH(FFHnd) )
		{
			if( IsAFile( &FFData ) )
			{
				CopyEntry( &FFData, lpff, lpFN );
				fgf = TRUE;
			}
			else
			{
				while( FindNextFile( FFHnd, &FFData ) )
				{
					if( IsAFile( &FFData ) )
					{
						CopyEntry( &FFData, lpff, lpFN );
						fgf = TRUE;
						break;
					}
				}
			}
		}
		else if( ( gfa & dfDirectory ) &&
			( lpg->gfDCount ) &&
			( FFHnd = FindFirstFile( lpDN, &FFData ) ) &&
			( FFHnd != INVALID_HANDLE_VALUE ) )
		{
			if( IsADir( &FFData ) )
			{
				CopyEntry( &FFData, lpff, lpFN );
				fgf = TRUE;
			}
			else
			{
				while( FindNextFile( FFHnd, &FFData ) )
				{
					if( IsADir( &FFData ) )
					{
						CopyEntry( &FFData, lpff, lpFN );
						fgf = TRUE;
						break;
					}
				}
			}
		}
	}
	if( fcnt )
	{
		WORD a, b;
		a = (WORD)fcnt;
		b = (WORD)gfa;
		fcnt = MAKELONG( a, b );
		_fmemcpy( (void *)lplgf, (void *)lpg, sizeof(DGETFILE) );
		if( FFData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			if( lplgf->gfDCount )
				lplgf->gfDCount--;
			if( lplgf->gfDCount == 0 )
			{
				FindClose( FFHnd );
				FFHnd = 0;
			}
		}
		else
		{
			if( lplgf->gfFCount )
				lplgf->gfFCount--;
			if( lplgf->gfFCount == 0 )
			{
				FindClose( FFHnd );
				FFHnd = 0;
			}
		}
	}
	if( FFHnd == INVALID_HANDLE_VALUE )
		FFHnd = 0;
	return( fcnt );
}

BOOL SRFindNext( LPDGETFILE lpg )
{
	BOOL flg, fgf;
	LPDGETFILE	lplgf;
	LPSTR	lpFN, lpDN;
	LPFILEFIND lpff;
	lplgf = &DGetFile;
	fgf = FALSE;
	flg = FALSE;
	lpff = lpg->gfName;
	if( lplgf->gfFCount || lplgf->gfDCount )
	{
		lpFN = &lplgf->gfMask[0];
		if( lplgf->gfFCount )
		{
			if( FFHnd && (FFHnd != INVALID_HANDLE_VALUE) )
			{
				while( FindNextFile( FFHnd, &FFData ) )
				{
					if( IsAFile( &FFData ) )
					{
						CopyEntry( &FFData, lpff, lpFN );
						fgf = TRUE;
						flg = TRUE;
						lplgf->gfFCount--;
						if( lplgf->gfFCount == 0 )
						{
							FindClose( FFHnd );
							FFHnd = 0;
						}
						break;
					}
				}
			}
		}
		if( !fgf && lplgf->gfDCount )
		{
			lpDN = &lplgf->gfDire[0];
			if( FFHnd == 0 )
			{
				if( ( FFHnd = FindFirstFile( lpDN, &FFData ) ) &&
					(FFHnd != INVALID_HANDLE_VALUE) )
				{
					if( IsADir( &FFData ) )
					{
						CopyEntry( &FFData, lpff, lpFN );
						fgf = TRUE;
						flg = TRUE;
						lplgf->gfDCount--;
						if( lplgf->gfDCount == 0 )
						{
							FindClose( FFHnd );
							FFHnd = 0;
						}
					}
					else
					{
						while( FindNextFile( FFHnd, &FFData ) )
						{
							if( IsADir( &FFData ) )
							{
								CopyEntry( &FFData, lpff, lpFN );
								fgf = TRUE;
								flg = TRUE;
								lplgf->gfDCount--;
								if( lplgf->gfDCount == 0 )
								{
									FindClose( FFHnd );
									FFHnd = 0;
								}
							}
						}
					}
				}
			}
			if( !fgf &&
				FFHnd &&
				( FFHnd != INVALID_HANDLE_VALUE ) )
			{
						while( FindNextFile( FFHnd, &FFData ) )
						{
							if( IsADir( &FFData ) )
							{
								CopyEntry( &FFData, lpff, lpFN );
								fgf = TRUE;
								flg = TRUE;
								lplgf->gfDCount--;
								if( lplgf->gfDCount == 0 )
								{
									FindClose( FFHnd );
									FFHnd = 0;
								}
								break;
							}
						}
			}
		}
	}
	return( flg );
}
// ======================================================
#else	// !WIN32
// ======================================================
// HOW TO DO SRFindFirt and SRFindNext in Windows 3.1???
// Can I use OpenFile?
DWORD SRFindFirst( LPDGETFILE lpg )
{
	DWORD	tcnt;

	tcnt = 0;

	return( tcnt );
}

BOOL SRFindNext( LPDGETFILE lpg )
{
	BOOL	fflg;

	fflg = 0;

	return( fflg );
}

// ======================================================
#endif	// WIN32 y/n

// eof - DvFile3.c (Was srfile.c)
