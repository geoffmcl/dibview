
/* ****************************************************************
 * DvUtil2.c
 * General utilities
 ****************************************************************** */

#include "dv.h"	/* All incusive include ... */
//#include  "DvUtil2.h"

#define	MXWRT		8096
#ifndef	WIN32
#include	<io.h>		// For _lseek
#endif	// !WIN32
#include <stdio.h>   // for sprintf()

#define  GetNxtBuf   GetStgBuf
#define         MXSTGS          32     // 20
#define         MXONE           264    // 64
// ==========================================================
// _sGetSStg
// =========
static  LPTSTR  _sGetSStg( void )
{
        LPTSTR  lprs;
        static TCHAR szP2S[ (MXSTGS * MXONE) ];
        static LONG  iNP2S = 0;
        // NOTE: Can be called only MXSTGS times before repeating
        lprs = &szP2S[ (iNP2S * MXONE) ];       // Get 1 of ? buffers
        iNP2S++;
        if( iNP2S >= MXSTGS )
                iNP2S = 0;
        return lprs;
}

LPTSTR   GetStgBuf( VOID )
{
   return( _sGetSStg() );
}

LPTSTR	Rect2Stg( LPRECT lpr )
{
	LPTSTR	lps = _sGetSStg();
		wsprintf( lps,
			"(%d,%d,%d,%d)",
			lpr->left,
			lpr->top,
			lpr->right,
			lpr->bottom );
	return lps;
}

//typedef struct _RECTL {
//    LONG left;
//    LONG top;
//    LONG right;
//    LONG bottom;
//    }   RECTL;
LPTSTR	RectL2Stg( PRECTL lpr )
{
	LPTSTR	lps = _sGetSStg();
		wsprintf( lps,
			"(%d,%d,%d,%d)",
			lpr->left,
			lpr->top,
			lpr->right,
			lpr->bottom );
   return lps;
}


int	InStr( LPSTR src, LPSTR mat )
{
	int	i, j, k, l, m, n;
	char	c;

	i = 0;
	if( ( src ) &&
		( mat ) &&
		( j = lstrlen( src ) ) &&
		( k = lstrlen( mat ) ) &&
		( j >= k ) )
	{
		c = mat[0];		// Get first char
		for( l = 0; l < j; l++ )
		{
			if( k > (j - ( l + 1)))	// If 'match' len GREATER than remainder
			{
				break;	// No FIND
			}
			else if( src[l] == c )	// if we get the FIRST char
			{
				n = l;
				for( m = 0; m < k; m++ )
				{
					if( src[n] != mat[m] )
						break;
				}
				if( m == k )
				{
					i = l + 1;
					break;
				}
			}
		}
	}
	return( i );
}

#ifdef	ADDOPENALL

void SRUnlock( HGLOBAL hg )
{
	if( hg )
		DVGlobalUnlock( hg );
}

HGLOBAL	SRAlloc( UINT typ, DWORD siz )
{
	HGLOBAL	hg;
	hg = DVGlobalAlloc( typ, siz );
	return( hg );
}

LPSTR SRLock( HGLOBAL hg )
{
	LPSTR	lps;
	lps = 0;
	if( hg )
		lps = DVGlobalLock( hg );
	return( lps );
}

void SRClose( LPSTR lpf, HANDLE hf )
{
	if( hf && (hf != (HANDLE)-1) )
		DVlclose( hf );
}

HANDLE SRCreat( LPSTR lpf, DWORD typ )
{
	HANDLE	hf;
	HFILE   fh;
	hf = 0;
	if (lpf && lpf[0]) {
		fh = _lcreat(lpf, typ);
		hf = IntToPtr(fh);
	}
	return hf;
}

//================================
//The CreateFile function creates, opens, or truncates a file, pipe, mailslot, communications resource, //disk device, or console. It returns a handle that can be used to
//access the object. It can also open and return a handle to a
//directory.
//HANDLE CreateFile(
//    LPCTSTR lpFileName,	// pointer to name of the file 
//    DWORD dwDesiredAccess,	// access (read-write) mode 
//    DWORD dwShareMode,	// share mode 
//    LPSECURITY_ATTRIBUTES lpSecurityAttributes,	// pointer to
//security descriptor 
//    DWORD dwCreationDistribution,	// how to create 
//    DWORD dwFlagsAndAttributes,	// file attributes 
//    HANDLE hTemplateFile 	// handle to file with attributes to
//copy  
//   );	
//Parameters
//lpFileName
//Points to a null-terminated string that specifies the name of
//the file, pipe, mailslot, communications resource, disk device,
//or console to create, open, or truncate. 
//If *lpFileName is a path, there is a default string size limit
//of MAX_PATH characters. This limit is related to how the
//CreateFile function parses paths. 
//Windows NT: You can transcend this limit and send in paths
//longer than MAX_PATH characters by calling the wide (W) version
//of CreateFile and prepending "\\?\" to the path. The "\\?\"
//tells the function to turn off path parsing. This lets you use
//paths that are nearly 32k Unicode characters long. You must use
//fully-qualified paths with this technique. This also works with
//UNC names. The "\\?\" is ignored as part of the path. For
//example, "\\?\C:\myworld\private" is seen as
//"C:\myworld\private", and "\\?\UNC\tom_1\hotstuff\coolapps" is
//seen as "\\tom_1\hotstuff\coolapps". 

//dwDesiredAccess
//Specifies the type of access to the file or other object. An
//application can obtain read access, write access, read-write
//access, or device query access. You can use the following flag
//constants to build a value for this parameter. Both GENERIC_READ
//and GENERIC_WRITE must be set to obtain read-write access: 
//Value	Meaning
//0	Allows an application to query device attributes without
//actually accessing the device.
//GENERIC_READ	Specifies read access to the file. Data can be read
//from the file and the file pointer can be moved.
//GENERIC_WRITE	Specifies write access to the file. Data can be
//written to the file and the file pointer can be moved.

//dwShareMode
//Set of bit flags that specifies how the file can be shared. 
//If dwShareMode is 0, the file cannot be shared. No other open
//operations can be performed on the file.
//To share the file, use a combination of one or more of the
//following values:  
//Value	Meaning
//FILE_SHARE_DELETE	Windows NT only: Other open operations can be
//performed on the file for delete access. 
//FILE_SHARE_READ	Other open operations can be performed on the
//file for read access. If the CreateFile function is opening the
//client end of a mailslot, this flag is specified.
//FILE_SHARE_WRITE	Other open operations can be performed on the
//file for write access.

//lpSecurityAttributes
//Pointer to a SECURITY_ATTRIBUTES structure that determines
//whether the returned handle can be inherited by child processes.
//If lpSecurityAttributes is NULL, the handle cannot be inherited.
//Windows NT: The lpSecurityDescriptor member of the structure
//specifies a security descriptor for the new file. If
//lpSecurityAttributes is NULL, the file gets a default security
//descriptor. The target file system must support security on
//files and directories for this parameter to have an effect.
//Windows 95: The lpSecurityDescriptor member of the structure is
//ignored.

//dwCreationDistribution
//Specifies which action to take on files that exist, and which
//action to take when files do not exist. For more information
//about this parameter, see the following Remarks section. This
//parameter must be one of the following values: 
//Value	Meaning
//CREATE_NEW	Creates a new file. The function fails if the
//specified file already exists.
//CREATE_ALWAYS	Creates a new file. The function overwrites the
//file if it exists.
//OPEN_EXISTING	Opens the file. The function fails if the file
//does not exist.
//	See the "Remarks" section, following, for a discussion of why
//you should use the OPEN_EXISTING flag if you are using the
//CreateFile function for a device, including the console.
//OPEN_ALWAYS	Opens the file, if it exists. If the file does not
//exist, the function creates the file as if
//dwCreationDistribution were CREATE_NEW.
//TRUNCATE_EXISTING	Opens the file. Once opened, the file is
//truncated so that its size is zero bytes. The calling process
//must open the file with at least GENERIC_WRITE access. The
//function fails if the file does not exist.

//dwFlagsAndAttributes
//Specifies the file attributes and flags for the file. 
//Any combination of the following attributes is acceptable,
//except all other file attributes override FILE_ATTRIBUTE_NORMAL.
//Attribute	Meaning
//FILE_ATTRIBUTE_ARCHIVE	The file is an archive file. Applications
//use this attribute to mark files for backup or removal.
//FILE_ATTRIBUTE_COMPRESSED	The file or directory is compressed.
//For a file, this means that all of the data in the file is
//compressed. For a directory, this means that compression is the
//default for newly created files and subdirectories.
//FILE_ATTRIBUTE_HIDDEN	The file is hidden. It is not to be
//included in an ordinary directory listing.
//FILE_ATTRIBUTE_NORMAL	The file has no other attributes set. This
//attribute is valid only if used alone.
//FILE_ATTRIBUTE_OFFLINE	The data of the file is not immediately
//available. Indicates that the file data has been physically
//moved to offline storage.
//FILE_ATTRIBUTE_READONLY	The file is read only. Applications can
//read the file but cannot write to it or delete it.
//FILE_ATTRIBUTE_SYSTEM	The file is part of or is used exclusively
//by the operating system.
//FILE_ATTRIBUTE_TEMPORARY	The file is being used for temporary
//storage. File systems attempt to keep all of the data in memory
//for quicker access rather than flushing the data back to mass
//storage. A temporary file should be deleted by the application
//as soon as it is no longer needed.
//Any combination of the following flags is acceptable.
//Flag	Meaning
//FILE_FLAG_WRITE_THROUGH	
//	Instructs the operating system to write through any
//intermediate cache and go directly to the file. The operating
//system can still cache write operations, but cannot lazily flush
//them.
//FILE_FLAG_OVERLAPPED	
//	Instructs the operating system to initialize the file, so
//ReadFile, WriteFile, ConnectNamedPipe, and TransactNamedPipe
//operations that take a significant amount of time to process
//return ERROR_IO_PENDING. When the operation is finished, an
//event is set to the signaled state.
//	When you specify FILE_FLAG_OVERLAPPED, the ReadFile and
//WriteFile functions must specify an OVERLAPPED structure. That
//is, when FILE_FLAG_OVERLAPPED is specified, an application must
//perform overlapped reading and writing.
//	When FILE_FLAG_OVERLAPPED is specified, the operating system
//does not maintain the file pointer. The file position must be
//passed as part of the lpOverlapped parameter (pointing to an
//OVERLAPPED structure) to the ReadFile and WriteFile functions.
//	This flag also enables more than one operation to be performed
//simultaneously with the handle (a simultaneous read and write
//operation, for example).
//FILE_FLAG_NO_BUFFERING	
//	Instructs the operating system to open the file with no
//intermediate buffering or caching. This can provide performance
//gains in some situations. An application must meet certain
//requirements when working with files opened with
//FILE_FLAG_NO_BUFFERING:�	File access must begin at offsets
//within the file that are integer multiples of the volume's
//sector size. �	File access must be for numbers of bytes that are
//integer multiples of the volume's sector size. For example, if
//the sector size is 512 bytes, an application can request reads
//and writes of 512, 1024, or 2048 bytes, but not of 335, 981, or
//7171 bytes. �	Buffer addresses for read and write operations
//must be aligned on addresses in memory that are integer
//multiples of the volume's sector size. An application can
//determine a volume's sector size by calling the GetDiskFreeSpace
//function.
//FILE_FLAG_RANDOM_ACCESS	
//	Indicates that the file is accessed randomly. Windows uses this
//flag to optimize file caching.
//FILE_FLAG_SEQUENTIAL_SCAN	
//	Indicates that the file is to be accessed sequentially from
//beginning to end. Windows uses this flag to optimize file
//caching. If an application moves the file pointer for random
//access, optimum caching may not occur; however, correct
//operation is still guaranteed.
//	Specifying this flag can increase performance for applications
//that read large files using sequential access. Performance gains
//can be even more noticeable for applications that read large
//files mostly sequentially, but occasionally skip over small
//ranges of bytes.
//FILE_FLAG_DELETE_ON_CLOSE	
//	Indicates that the operating system is to delete the file
//immediately after all of its handles have been closed.If you use
//this flag when you call CreateFile, then open the file again,
//and then close the handle for which you specified
//FILE_FLAG_DELETE_ON_CLOSE, the file will not be deleted until
//after you have closed the second and any other handle to the
//file.
//FILE_FLAG_BACKUP_SEMANTICS	
//	Windows NT only: Indicates that the file is being opened or
//created for a backup or restore operation. The operating system
//ensures that the calling process overrides file security checks,
//provided it has the necessary permission to do so. The relevant
//permissions are SE_BACKUP_NAME and SE_RESTORE_NAME.A Windows NT
//application can also set this flag to obtain a handle to a
//directory. A directory handle can be passed to some Win32
//functions in place of a file handle.
//FILE_FLAG_POSIX_SEMANTICS	
//	Indicates that the file is to be accessed according to POSIX
//rules. This includes allowing multiple files with names,
//differing only in case, for file systems that support such
//naming. Use care when using this option because files created
//with this flag may not be accessible by applications written for
//MS-DOS, Windows 3.x, or Windows NT.
//If the CreateFile function opens the client side of a named
//pipe, the dwFlagsAndAttributes parameter can also contain
//Security Quality of Service information. When the calling
//application specifies the SECURITY_SQOS_PRESENT flag, the
//dwFlagsAndAttributes parameter can contain one or more of the
//following values: 
//Value	Meaning
//SECURITY_ANONYMOUS	Specifies to impersonate the client at the
//Anonymous impersonation level.
//SECURITY_IDENTIFICATION	Specifies to impersonate the client at
//the Identification impersonation level.
//SECURITY_IMPERSONATION	Specifies to impersonate the client at
//the Impersonation impersonation level.
//SECURITY_DELEGATION	Specifies to impersonate the client at the
//Delegation impersonation level.
//SECURITY_CONTEXT_TRACKING	Specifies that the security tracking
//mode is dynamic. If this flag is not specified, Security
//Tracking Mode is static.
//SECURITY_EFFECTIVE_ONLY	Specifies that only the enabled aspects
//of the client's security context are available to the server. If
//you do not specify this flag, all aspects of the client's
//security context are available.This flag allows the client to
//limit the groups and privileges that a server can use while
//impersonating the client.
//For more information, see Security. 

//hTemplateFile
//Specifies a handle with GENERIC_READ access to a template file.
//The template file supplies file attributes and extended
//attributes for the file being created. 
//Windows 95: This value must be NULL. If you supply a handle
//under Windows 95, the call fails and GetLastError returns
//ERROR_NOT_SUPPORTED.
//Return Values
//If the function succeeds, the return value is an open handle to
//the specified file. If the specified file exists before the
//function call and dwCreationDistribution is CREATE_ALWAYS or
//OPEN_ALWAYS, a call to GetLastError returns ERROR_ALREADY_EXISTS
//(even though the function has succeeded). If the file does not
//exist before the call, GetLastError returns zero. 
//If the function fails, the return value is INVALID_HANDLE_VALUE.
//To get extended error information, call GetLastError. 
//Remarks
//If you are attempting to create a file on a floppy drive that
//does not have a floppy disk or a CD-ROM drive that does not have
//a CD, the system displays a message box asking the user to
//insert a disk or a CD, respectively. To prevent the system from
//displaying this message box, call the SetErrorMode function with
//SEM_FAILCRITICALERRORS.
//When creating a new file, the CreateFile function performs the
//following actions: 
//�	Combines the file attributes and flags specified by
//dwFlagsAndAttributes with FILE_ATTRIBUTE_ARCHIVE. 
//�	Sets the file length to zero. 
//�	Copies the extended attributes supplied by the template file
//to the new file if the hTemplateFile parameter is specified. 
//When opening an existing file, CreateFile performs the following
//actions: 
//�	Combines the file flags specified by dwFlagsAndAttributes with
//existing file attributes. CreateFile ignores the file attributes
//specified by dwFlagsAndAttributes. 
//�	Sets the file length according to the value of
//dwCreationDistribution. 
//�	Ignores the hTemplateFile parameter. 
//�	Ignores the lpSecurityDescriptor member of the
//SECURITY_ATTRIBUTES structure if the lpSecurityAttributes
//parameter is not NULL. The other structure members are valid.
//The bInheritHandle member is the only way to indicate whether
//the file handle can be inherited. 
//If CreateFile opens the client end of a named pipe, the function
//uses any instance of the named pipe that is in the listening
//state. The opening process can duplicate the handle as many
//times as required but, once opened, the named pipe instance
//cannot be opened by another client. The access specified when a
//pipe is opened must be compatible with the access specified in
//the dwOpenMode parameter of the CreateNamedPipe function. For
//more information about pipes, see Pipes. 
//If CreateFile opens the client end of a mailslot, the function
//returns INVALID_HANDLE_VALUE if the mailslot client attempts to
//open a local mailslot before the mailslot server has created it
//with the CreateMailSlot function. For more information about
//mailslots, see Mailslots. 
//CreateFile can create a handle to a communications resource,
//such as the serial port COM1. For communications resources, the
//dwCreationDistribution parameter must be OPEN_EXISTING, and the
//hTemplate parameter must be NULL. Read, write, or read-write
//access can be specified, and the handle can be opened for
//overlapped I/O. For more information about communications, see
//Communications. 
//CreateFile can create a handle to console input (CONIN$). If the
//process has an open handle to it as a result of inheritance or
//duplication, it can also create a handle to the active screen
//buffer (CONOUT$). 
//The calling process must be attached to an inherited console or
//one allocated by the AllocConsole function. For console handles,
//set the CreateFile parameters as follows: 
//Parameters	Value
//lpFileName	Use the CONIN$ value to specify console input and the
//CONOUT$ value to specify console output.
//	CONIN$ gets a handle to the console's input buffer, even if the
//SetStdHandle function redirected the standard input handle. To
//get the standard input handle, use the GetStdHandle function.
//	CONOUT$ gets a handle to the active screen buffer, even if
//SetStdHandle redirected the standard output handle. To get the
//standard output handle, use GetStdHandle.
//dwDesiredAccess	GENERIC_READ | GENERIC_WRITE is preferred, but
//either one can limit access.
//dwShareMode	If the calling process inherited the console or if a
//child process should be able to access the console, this
//parameter must be FILE_SHARE_READ | FILE_SHARE_WRITE.
//lpSecurityAttributes	If you want the console to be inherited,
//the bInheritHandle member of the SECURITY_ATTRIBUTES structure
//must be TRUE.
//dwCreationDistribution	The user should specify OPEN_EXISTING
//when using CreateFile to open the console.
//dwFlagsAndAttributes	Ignored.
//hTemplateFile	Ignored.
//The following list shows the effects of various settings of
//fwdAccess and lpFileName.
//lpFileName	fwdAccess	Result
//CON	GENERIC_READ	Opens console for input.
//CON	GENERIC_WRITE	Opens console for output.
//CON	GENERIC_READ
//GENERIC_WRITE	Windows 95: Causes CreateFile to fail;
//GetLastError returns ERROR_PATH_NOT_FOUND.Windows NT:  Causes
//CreateFile to fail; GetLastError returns ERROR_FILE_NOT_FOUND.
//You can use the CreateFile function to open a disk drive or a
//partition on a disk drive. The function returns a handle to the
//disk device; that handle can be used with the DeviceIOControl
//function. The following requirements must be met in order for
//such a call to succeed: 
//�	The caller must have administrative privileges for the
//operation to succeed on a hard disk drive. 
//�	The lpFileName string should be of the form \\.\PHYSICALDRIVEx
//to open the hard disk x. Hard disk numbers start at zero. For
//example: 
//String	Meaning
//\\.\PHYSICALDRIVE2	Obtains a handle to the third physical drive
//on the user's computer.
//�	The lpFileName string should be \\.\x: to open a floppy drive
//x or a partition x on a hard disk. For example: 
//String	Meaning
//\\.\A:	Obtains a handle to drive A on the user's computer.
//\\.\C:	Obtains a handle to drive C on the user's computer.
//Windows 95: This technique does not work for opening a logical
//drive. In Windows 95, specifying a string in this form causes
//CreateFile to return an error.
//�	The dwCreationDistribution parameter must have the
//OPEN_EXISTING value. 
//�	When opening a floppy disk or a partition on a hard disk, you
//must set the FILE_SHARE_WRITE flag in the dwShareMode parameter.
//The CloseHandle function is used to close a handle returned by
//CreateFile. 
//As noted above, specifying zero for dwDesiredAccess allows an
//application to query device attributes without actually
//accessing the device. This type of querying is useful, for
//example, if an application wants to determine the size of a
//floppy disk drive and the formats it supports without having a
//floppy in the drive. 
//As previously noted, if an application opens a file with
//FILE_FLAG_NO_BUFFERING set, buffer addresses for read and write
//operations must be aligned on memory addresses that are integer
//multiples of the volume's sector size. One way to do this is to
//use VirtualAlloc to allocate the buffer. The VirtualAlloc
//function allocates memory that is aligned on addresses that are
//integer multiples of the operating system's memory page size.
//Since both memory page and volume sector sizes are powers of 2,
//and memory pages are larger than volume sectors, this memory is
//also aligned on addresses that are integer multiples of a
//volume's sector size.
//An application cannot create a directory with CreateFile; it
//must call CreateDirectory or CreateDirectoryEx to create a
//directory.
//Windows NT:
//You can obtain a handle to a directory by setting the
//FILE_FLAG_BACKUP_SEMANTICS flag. A directory handle can be
//passed to some Win32 functions in place of a file handle.
//Some file systems, such as NTFS, support compression for
//individual files and directories. On volumes formatted for such
//a file system, a new directory inherits the compression
//attribute of its parent directory.
//See Also
//AllocConsole, CloseHandle, ConnectNamedPipe, CreateDirectory,
//CreateDirectoryEx, CreateNamedPipe, DeviceIOControl,
//GetDiskFreeSpace, GetOverlappedResult, GetStdHandle, OpenFile,
//OVERLAPPED, ReadFile, SECURITY_ATTRIBUTES, SetErrorMode,
////SetStdHandle TransactNamedPipe, VirtualAlloc, WriteFile 

//================================

//The _lopen function opens an existing file and sets the file pointer to the beginning of the file. This //function is provided for compatibility with 16-bit versions of
//Windows. Win32-based applications should use the CreateFile
//function. 
//HFILE _lopen(
//    LPCSTR lpPathName,	// pointer to name of file to open  
//    int iReadWrite 	// file access mode 
//   );	
//Parameters
//lpPathName
//Pointer to a null-terminated string that names the file to open.
//The string must consist of characters from the Windows ANSI
//character set. 
//iReadWrite
//Specifies the modes in which to open the file. This parameter
//consists of one access mode and an optional share mode. The
//access mode must be one of the following values: 
//Value	Meaning
//OF_READ	Opens the file for reading only.
//OF_READWRITE	Opens the file for reading and writing.
//OF_WRITE	Opens the file for writing only.
//The share mode can be one of the following values: 
//Value	Meaning
//OF_SHARE_COMPAT	Opens the file in compatibility mode, enabling
//any process on a given computer to open the file any number of
//times. If the file has been opened by using any of the other
//share modes, _lopen fails.
//OF_SHARE_DENY_NONE	Opens the file without denying other
//processes read or write access to the file. If the file has been
//opened in compatibility mode by any other process, _lopen fails.
//OF_SHARE_DENY_READ	Opens the file and denies other processes
//read access to the file. If the file has been opened in
//compatibility mode or for read access by any other process,
//_lopen fails.
//OF_SHARE_DENY_WRITE	Opens the file and denies other processes
//write access to the file. If the file has been opened in
//compatibility mode or for write access by any other process,
//_lopen fails.
//OF_SHARE_EXCLUSIVE	Opens the file in exclusive mode, denying
//other processes both read and write access to the file. If the
//file has been opened in any other mode for read or write access,
//even by the current process, _lopen fails.
//Return Values
//If the function succeeds, the return value is a file handle.
//If the function fails, the return value is HFILE_ERROR. To get
//extended error information, call GetLastError. 
//See Also
////CreateFile 
//HFILE	hfIn, hfOut, hfErr;

HANDLE SROpen( LPSTR lpf, DWORD typ )
{
	HANDLE	hf;
#ifdef	WIN32
	DWORD	dwAcc, dwShare, dwCreat, dwAttr;
	DWORD	ct;
#endif	// WIN#@
	hf = 0;
	if( lpf && lpf[0] )
	{
#ifdef	WIN32
//		hfIn = SRCreat( "TEMP1E.ERR", 0 );
//		hfOut = SRCreat( "TEMP2E.ERR", 0 );
//		hfErr = SRCreat( "TEMP3E.ERR", 0 );
		ct = typ & (OF_READ	| OF_READWRITE | OF_WRITE);
		if( ct == 0 )
			dwAcc = GENERIC_READ | GENERIC_WRITE; // Specifies read/write
		else if( ct == OF_READ )
			dwAcc = GENERIC_READ | GENERIC_WRITE;
		else
			dwAcc = GENERIC_READ | GENERIC_WRITE;
		ct = typ & (OF_SHARE_COMPAT|OF_SHARE_DENY_NONE|\
			OF_SHARE_DENY_READ|OF_SHARE_DENY_WRITE|\
			OF_SHARE_EXCLUSIVE);
		if( ct == 0 )
			dwShare = 0;
		else
		{
			dwShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
			if( ct & OF_SHARE_DENY_READ )
				dwShare &= ~FILE_SHARE_READ;
			if( ct & OF_SHARE_DENY_WRITE )
				dwShare &= ~FILE_SHARE_WRITE;
		}
		dwCreat = OPEN_EXISTING;
		dwAttr  = FILE_ATTRIBUTE_NORMAL;
		hf = CreateFile( lpf,	// pointer to name of the file
			dwAcc,	// access (read-write) mode
			dwShare,	// share mode
			NULL,	// pointer to security descriptor
			dwCreat,	// how to create
			dwAttr,	// file attributes
			0 );	// handle to file with attributes to copy
//		if( hfIn && ( hfIn != HFILE_ERROR ) )
//			DVlclose( hfIn );
//		if( hfOut && ( hfOut != HFILE_ERROR ) )
//			DVlclose( hfOut );
//		if( hfErr && ( hfErr != HFILE_ERROR ) )
//			DVlclose( hfErr );
#else	// !WIN#@
		hf = _lopen( lpf, typ );
#endif	// WIN32 y/n
	}
	return( hf );
}

void SRFree( HGLOBAL hg )
{
	if( hg )
		DVGlobalFree( hg );
}

#ifndef	WIN32
// NOTE: Does NOT appear to work under Window 95 (WIN32)!!!!
LONG	SRSeek( HFILE hf, LONG pos, UINT typ )
{
	LONG	npos;
	int		ie;
	npos = 0;
	if( hf && (hf != HFILE_ERROR) )
	{
		npos = _lseek( hf, pos, typ );
		if( npos = (LONG)-1 )
		{
			ie = errno;
		}
	}
	return( npos );
}
#endif	// !WIN#@

//The GetFileInformationByHandle function retrieves information about a specified file. 
//BOOL GetFileInformationByHandle(
//    HANDLE hFile, 	// handle of file 
//    LPBY_HANDLE_FILE_INFORMATION lpFileInformation 	// address
//of structure 
//   );	
//Parameters
//hFile
//Handle to the file that you want to obtain information about. 
//This handle should not be a pipe handle. The
//GetFileInformationByHandle function does not work with pipe
//handles.
//lpFileInformation
//Points to a BY_HANDLE_FILE_INFORMATION structure that receives
//the file information. The structure can be used in subsequent
//calls to GetFileInformationByHandle to refer to the information
//about the file. 
//Return Values
//If the function succeeds, the return value is a nonzero value. 
//If the function fails, the return value is zero. To get extended
//error information, call GetLastError. 
//Remarks
//Depending on the underlying network components of the operating
//system and the type of server connected to, the
//GetFileInformationByHandle function may fail, return partial
//information, or full information for the given file. In general,
//you should not use GetFileInformationByHandle unless your
//application is intended to be run on a limited set of operating
//system configurations.
//See Also
////BY_HANDLE_FILE_INFORMATION 

//The BY_HANDLE_FILE_INFORMATION structure contains information retrieved by the //GetFileInformationByHandle function. 
//typedef struct _BY_HANDLE_FILE_INFORMATION { // bhfi  
//    DWORD    dwFileAttributes; 
//    FILETIME ftCreationTime; 
//    FILETIME ftLastAccessTime; 
//    FILETIME ftLastWriteTime; 
//    DWORD    dwVolumeSerialNumber; 
//    DWORD    nFileSizeHigh; 
//    DWORD    nFileSizeLow; 
//    DWORD    nNumberOfLinks; 
//    DWORD    nFileIndexHigh; 
//    DWORD    nFileIndexLow; 
//} BY_HANDLE_FILE_INFORMATION; 
//Members
//dwFileAttributes
//Specifies file attributes. This member can be one or more of the
//following values: 
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
//	The file has no other attributes. This value is valid only if
//used alone.
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
//Specifies the time the file was created. If the underlying file
//system does not support this time member, ftCreationTime is
//zero.  
//ftLastAccessTime
//Specifies the time the file was last accessed. If the underlying
//file system does not support this time member, ftLastAccessTime
//is zero.  
//ftLastWriteTime
//Specifies the last time the file was written to. 
//dwVolumeSerialNumber
//Specifies the serial number of the volume that contains the
//file. 
//nFileSizeHigh
//Specifies the high-order word of the file size. 
//nFileSizeLow
//Specifies the low-order word of the file size. 
//nNumberOfLinks
//Specifies the number of links to this file. For the FAT file
//system this member is always 1. For NTFS, it may be more than 1.
//nFileIndexHigh
//Specifies the high-order word of a unique identifier associated
//with the file. 
//nFileIndexLow
//Specifies the low-order word of a unique identifier associated
//with the file. This identifier and the volume serial number
//uniquely identify a file. This number may change when the system
//is restarted or when the file is opened. After a process opens a
//file, the identifier is constant until the file is closed. An
//application can use this identifier and the volume serial number
//to determine whether two handles refer to the same file. 
//See Also
////GetFileInformationByHandle 

// SEEK_SET   Beginning of file
// SEEK_CUR   Current position of file pointer
// SEEK_END   End of file
LONG DVFileSize( HANDLE  hFile)
{
	LONG lCur, lLen;
#ifdef	WIN32
	BY_HANDLE_FILE_INFORMATION FileInfo;
	LPVOID lpMsgBuf;
	lCur = lLen = 0;
	if( hFile && (hFile != (HANDLE)-1) )
	{
		if( GetFileInformationByHandle( hFile, 	// handle of file
			&FileInfo ) )
		{
			lLen = FileInfo.nFileSizeLow;
		}
		else
		{
			lCur = GetLastError();
//The FormatMessage function formats a message string. The function requires a message definition as //input. The message definition can come from a buffer passed into
//the function. It can come from a message table resource in an
//already-loaded module. Or the caller can ask the function to
//search the system's message table resource(s) for the message
//definition. The function finds the message definition in a
//message table resource based on a message identifier and a
//language identifier. The function copies the formatted message
//text to an output buffer, processing any embedded insert
//sequences if requested. 
//DWORD FormatMessage(
//    DWORD dwFlags,	// source and processing options 
//    LPCVOID lpSource,	// pointer to  message source 
//    DWORD dwMessageId,	// requested message identifier 
//    DWORD dwLanguageId,	// language identifier for requested
//message 
//    LPTSTR lpBuffer,	// pointer to message buffer 
//    DWORD nSize,	// maximum size of message buffer 
//    va_list *Arguments 	// address of array of message inserts 
//   );	
//Parameters
//dwFlags
//Contains a set of bit flags that specify aspects of the
//formatting process and how to interpret the lpSource parameter.
//The low-order byte of dwFlags specifies how the function handles
//line breaks in the output buffer. The low-order byte can also
//specify the maximum width of a formatted output line.
//You can specify a combination of the following bit flags:
//Value	Meaning
//FORMAT_MESSAGE_ALLOCATE_BUFFER	
//	Specifies that the lpBuffer parameter is a pointer to a PVOID
//pointer, and that the nSize parameter specifies the minimum
//number of bytes (ANSI version) or characters (Unicode version)
//to allocate for an output message buffer. The function allocates
//a buffer large enough to hold the formatted message, and places
//a pointer to the allocated buffer at the address specified by
//lpBuffer. The caller should use the LocalFree function to free
//the buffer when it is no longer needed.
//FORMAT_MESSAGE_IGNORE_INSERTS	
//	Specifies that insert sequences in the message definition are
//to be ignored and passed through to the output buffer unchanged.
//This flag is useful for fetching a message for later formatting.
//If this flag is set, the Arguments parameter is ignored.
//FORMAT_MESSAGE_FROM_STRING	
//	Specifies that lpSource is a pointer to a null-terminated
//message definition. The message definition may contain insert
//sequences, just as the message text in a message table resource
//may. Cannot be used with FORMAT_MESSAGE_FROM_HMODULE or
//FORMAT_MESSAGE_FROM_SYSTEM. 
//FORMAT_MESSAGE_FROM_HMODULE	
//	Specifies that lpSource is a module handle containing the
//message-table resource(s) to search. If this lpSource handle is
//NULL, the current process's application image file will be
//searched. Cannot be used with FORMAT_MESSAGE_FROM_STRING.
//FORMAT_MESSAGE_FROM_SYSTEM	
//	Specifies that the function should search the system
//message-table resource(s) for the requested message. If this
//flag is specified with FORMAT_MESSAGE_FROM_HMODULE, the function
//searches the system message table if the message is not found in
//the module specified by lpSource. Cannot be used with
//FORMAT_MESSAGE_FROM_STRING.If this flag is specified, an
//application can pass the result of the GetLastError function to
//retrieve the message text for a system-defined error.
//FORMAT_MESSAGE_ARGUMENT_ARRAY	
//	Specifies that the Arguments parameter is not a va_list
//structure, but instead is just a pointer to an array of 32-bit
//values that represent the arguments.
//The low-order byte of dwFlags can specify the maximum width of a
//formatted output line. Use the FORMAT_MESSAGE_MAX_WIDTH_MASK
//constant and bitwise Boolean operations to set and retrieve this
//maximum width value.
//The following table shows how FormatMessage interprets the value
//of the low-order byte.
//Value	Meaning
//0	There are no output line width restrictions. The function
//stores line breaks that are in the message definition text into
//the output buffer.
//a nonzero value other than FORMAT_MESSAGE_MAX_WIDTH_MASK	The
//nonzero value is the maximum number of characters in an output
//line. The function ignores regular line breaks in the message
//definition text. The function never splits a string delimited by
//white space across a line break. The function stores hard-coded
//line breaks in the message definition text into the output
//buffer. Hard-coded line breaks are coded with the %n escape
//sequence.
//FORMAT_MESSAGE_MAX_WIDTH_MASK	The function ignores regular line
//breaks in the message definition text. The function stores
//hard-coded line breaks in the message definition text into the
//output buffer. The function generates no new line breaks. 
//lpSource
//Specifies the location of the message definition. The type of
//this parameter depends upon the settings in the dwFlags
//parameter. 
//dwFlags Setting	Parameter Type
//FORMAT_MESSAGE_FROM_HMODULE	lpSource is an hModule of the module
//that contains the message table to search.
//FORMAT_MESSAGE_FROM_STRING	lpSource is an LPTSTR that points to
//unformatted message text. It will be scanned for inserts and
//formatted accordingly.
//If neither of these flags is set in dwFlags, then lpSource is
//ignored. 
//dwMessageId
//Specifies the 32-bit message identifier for the requested
//message. This parameter is ignored if dwFlags includes
//FORMAT_MESSAGE_FROM_STRING. 
//dwLanguageId
//Specifies the 32-bit language identifier for the requested
//message. This parameter is ignored if dwFlags includes
//FORMAT_MESSAGE_FROM_STRING. 
//lpBuffer
//Points to a buffer for the formatted (and null-terminated)
//message. If dwFlags includes FORMAT_MESSAGE_ALLOCATE_BUFFER, the
//function allocates a buffer using the LocalAlloc function, and
//places the address of the buffer at the address specified in
//lpBuffer. 
//nSize
//If the FORMAT_MESSAGE_ALLOCATE_BUFFER flag is not set, this
//parameter specifies the maximum number of bytes (ANSI version)
//or characters (Unicode version) that can be stored in the output
//buffer. If FORMAT_MESSAGE_ALLOCATE_BUFFER is set, this parameter
//specifies the minimum number of bytes or characters to allocate
//for an output buffer. 
//Arguments
//Points to an array of 32-bit values that are used as insert
//values in the formatted message. %1 in the format string
//indicates the first value in the Arguments array; %2 indicates
//the second argument; and so on. 
//The interpretation of each 32-bit value depends on the
//formatting information associated with the insert in the message
//definition. The default is to treat each value as a pointer to a
//null-terminated string. 
//By default, the Arguments parameter is of type va_list*, which
//is a language- and implementation-specific data type for
//describing a variable number of arguments. If you do not have a
//pointer of type va_list*, then specify the
//FORMAT_MESSAGE_ARGUMENT_ARRAY flag and pass a pointer to an
//array of 32-bit values; those values are input to the message
//formatted as the insert values. Each insert must have a
//corresponding element in the array. 
//Return Values
//If the function succeeds, the return value is the number of
//bytes (ANSI version) or characters (Unicode version) stored in
//the output buffer, excluding the terminating null character. 
//If the function fails, the return value is zero. To get extended
//error information, call GetLastError. 
//Remarks
//The FormatMessage function can be used to obtain error message
//strings for the system error codes returned by GetLastError, as
//shown in the following sample code. 
//LPVOID lpMsgBuf;
//FormatMessage( 
//    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
//    NULL,
//    GetLastError(),
//    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default
//language
//    (LPTSTR) &lpMsgBuf,
//    0,
//    NULL 
//);
//// Display the string.
//MessageBox( NULL, lpMsgBuf, "GetLastError",
//MB_OK|MB_ICONINFORMATION );
//// Free the buffer.
//LocalFree( lpMsgBuf );
//Within the message text, several escape sequences are supported
//for dynamically formatting the message. These escape sequences
//and their meanings are shown in the following table. All escape
//sequences start with the percent character (%). 
//Escape Sequence	Meaning
//%0	Terminates a message text line without a trailing newline
//character. This escape sequence can be used to build up long
//lines or to terminate the message itself without a trailing
//newline character. It is useful for prompt messages.
//%n!printf format string!	Identifies an insert. The value of n
//can be in the range 1 through 99. The printf format string
//(which must be bracketed by exclamation marks) is optional and
//defaults to !s! if not specified.
//	The printf format string can contain the * specifier for either
//the precision or the width component. If * is specified for one
//component, the FormatMessage function uses insert %n+1; it uses
//%n+2 if * is specified for both components.
//	Floating-point printf format specifiers  �  e, E, f, and g  � 
//are not supported. The workaround is to to use the sprintf
//function to format the floating-point number into a temporary
//buffer, then use that buffer as the insert string.
//Any other nondigit character following a percent character is
//formatted in the output message without the percent character.
//Following are some examples: 
//Format string	Resulting output
//%%	A single percent sign in the formatted message text.
//%n	A hard line break when the format string occurs at the end of
//a line. This format string is useful when FormatMessage is
//supplying regular line breaks so the message fits in a certain
//width.
//%space	A space in the formatted message text. This format string
//can be used to ensure the appropriate number of trailing spaces
//in a message text line.
//%.	A single period in the formatted message text. This format
//string can be used to include a single period at the beginning
//of a line without terminating the message text definition.
//%!	A single exclamation point in the formatted message text.
//This format string can be used to include an exclamation point
//immediately after an insert without its being mistaken for the
//beginning of a printf format string.
//See Also
////LoadString, LocalFree 
			lpMsgBuf = 0;
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				lCur,	// Results of GetLastError()
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL );
			if( lpMsgBuf )
			{
				// Display the string.
				MessageBox( NULL, lpMsgBuf, "GetLastError",
					MB_OK|MB_ICONINFORMATION );
				// Free the buffer.
				LocalFree( lpMsgBuf );
			}
		}
	}
#else	// !WIN32
	lLen = 0;
	if( (lCur = SRSeek( hFile, 0L, SEEK_CUR )) != (LONG)-1 ) /* Save current position */
	{
		lLen = SRSeek( hFile, 0L, SEEK_END );	/* Move to END OF FILE */
		SRSeek( hFile, lCur, SEEK_SET );	/* And back to Current */
	}
	else
	{
		lLen = _filelength( hFile );
		// Or _fstat( hFile );
	}
#endif	// WIN32 y/n
	return( lLen );
}


char	SRUpperCase( char c )
{
	char	d;
	if( (c >= 'a') && (c <= 'z' ) )
		d = c & 0x5f;
	else
		d = c;
	return( d );
}

BOOL	SRIsExeOrDll( LPSTR lps )
{
	BOOL	flg;
	int	i;
	char	b, c, d;
	flg = FALSE;

	if( (i = lstrlen( lps )) > 2 )
	{
		b = SRUpperCase( lps[i-1] ); 
		c = SRUpperCase( lps[i-2] );
		d = SRUpperCase( lps[i-3] );
		if( ( (d == 'E') && (c == 'X') && (b == 'E')  ) ||
			( (d == 'D') && (c == 'L') && (b == 'L') ) )
		{
			flg = TRUE;
		}
	}
	return( flg );
}

#ifdef	WIN32
// ===================================================
DWORD MyDOSWrite( HFILE hf, LPSTR lps, DWORD sz)
{
	LPSTR	lpOut;
	DWORD	rem, tsz;
	WORD	rsz, wrt;
	tsz = 0;
	if( hf &&
		(hf != HFILE_ERROR) &&
		(rem = sz) &&
		(lpOut = lps) )
	{
		while( rem )
		{
			if( rem > MXWRT )
				rsz = MXWRT;
			else
				rsz = LOWORD( rem );
			wrt = _lwrite( hf, lpOut, rsz );
			if( wrt != rsz )
				break;
			tsz += wrt;
			rem -= wrt;
			lpOut += wrt;		
		}
	}
	return( tsz );
}

// #define	BYTES_PER_READ		1024

DWORD	SRReadLong( HFILE hf, LPSTR lpb, DWORD lsiz )
{
	LPSTR	lpIn;
	int		nBytes;
	DWORD	dwSize, rsz;
	rsz = 0;
	if( (lpIn = lpb) &&
		(dwSize = lsiz) )
	{
		while( dwSize )
		{
			if( dwSize > BYTES_PER_READ )
				nBytes = BYTES_PER_READ;
			else
				nBytes = LOWORD( dwSize );
			if( _lread( hf, lpIn, nBytes ) != (WORD)nBytes )
			{
				break;
			}
			dwSize  -= nBytes;	/* Reduce remainder ... */
			lpIn += nBytes;	/* Bump buffer pointer ... */
			rsz += (DWORD) nBytes;		/* And keep count of READS ... */
		}
	}
	return( rsz );
}

// ===================================================
#else	// !WIN#@
// ===================================================

DWORD MyDOSWrite( HFILE hf, LPSTR lps, DWORD sz)
{
	PINT8	buf;
	DWORD	rem, tsz;
	WORD	rsz, boff, soff, wrt;

	tsz = 0;
	if( ( hf ) &&
		( hf != HFILE_ERROR ) &&
		( rem = sz ) &&
		( buf = (PINT8) lps ) )
	{
		while( rem )
		{
			if( rem > MXWRT )
				rsz = MXWRT;
			else
				rsz = LOWORD( rem );
			if( boff = LOWORD( buf ) )
			{
				soff = ~boff + 1;
				if( soff )
				{
					if( soff < rsz )
						rsz = soff;
				}
			}
			wrt = _lwrite( hf, buf, rsz );
			if( wrt != rsz )
				break;
			tsz += wrt;
			rem -= wrt;
			buf += wrt;		
		}
	}
	return( tsz );
}

// #define	BYTES_PER_READ		1024

DWORD	SRReadLong( HFILE hf, LPSTR lpb, DWORD lsiz )
{
	char _huge *lpInBuf = (char _huge *) lpb;
	int       nBytes;
	DWORD	dwSize = lsiz;
	DWORD	rsz;
	WORD	boff, soff;

	rsz = 0;
	while( dwSize )
	{
		if( dwSize > BYTES_PER_READ )
			nBytes = BYTES_PER_READ;
		else
			nBytes = LOWORD( dwSize );
		if( boff = LOWORD( lpInBuf ) )
		{
			soff = ~boff + 1;
			if( soff )
			{
				if( soff < (WORD) nBytes )
					nBytes = soff;
			}
		}
		if( _lread( hf, (LPSTR) lpInBuf, nBytes ) != (WORD) nBytes )
		{
         break;
		}
      dwSize  -= nBytes;	/* Reduce remainder ... */
      lpInBuf += nBytes;	/* Bump buffer pointer ... */
		rsz += (DWORD) nBytes;		/* And keep count of READS ... */
	}
	return( rsz );
}

// ===================================================
#endif	// WIN32 y/n

#endif	/* ADDOPENALL */

#ifndef	_DvWait_H

void SRWait( BOOL flg, DWORD wd )
{
	if( flg )
	{
		if( wd == SRW_BGN )
			Hourglass( TRUE );
	}
	else
	{
		if( wd == SRW_END )
			Hourglass( FALSE );
	}
}

#endif	/* _DvWait_H */


// convert UTC FILETIME to local FILETIME and then to a date/time string
LPTSTR	GetFDTStg( FILETIME * pft )
{
   LPTSTR   lps = _sGetSStg();
   SYSTEMTIME  st;
   FILETIME    ft;
   if( ( FileTimeToLocalFileTime( pft, &ft ) ) && // UTC file time converted to local
       ( FileTimeToSystemTime( &ft, &st)     ) )
   {
      wsprintf(lps,
         "%02d/%02d/%02d %02d:%02d",
         (st.wDay & 0xffff),
         (st.wMonth & 0xffff),
         (st.wYear % 100),
         (st.wHour & 0xffff),
         (st.wMinute & 0xffff) );
   }
   else
   {
		lstrcpy( lps, "??/??/?? ??:??" );
   }
   return lps;
}

LPTSTR   GetI64Stg( PLARGE_INTEGER pli )
{
   LPTSTR   lps = _sGetSStg();
   TCHAR    buf[32];
   int      i,j,k;
   LARGE_INTEGER  li;
   __int64 i64;

   li = *pli;
   i64 = li.QuadPart;
   sprintf(buf,
      "%I64d",
      i64 );
   *lps = 0;   // clear any previous
   if( i = (int)strlen(buf) )  // get its length
   {
      k = 32;
      j = 0;
      lps[k+1] = 0;  // esure ZERO termination
      while( ( i > 0 ) && ( k >= 0 ) )
      {
         i--;     // back up one
         if( j == 3 )   // have we had a set of 3?
         {
            lps[k--] = ',';   // ok, add a comma
            j = 0;            // and restart count of digits
         }
         lps[k--] = buf[i];   // move the buffer digit
         j++;
      }
      k++;  // back to LAST position
      lps = &lps[k]; // pointer to beginning of 'nice" number
   }
   return lps;
}

// some timing functions
// ===============================================================================
#define  MXTMS    32    // NOTE this MAXIMUM at any one time, else unpredictable

typedef  struct { /* tm */
   BOOL  tm_bInUse;
   BOOL  tm_bt;
   LARGE_INTEGER  tm_lif, tm_lib, tm_lie;
   DWORD tm_dwtc;
}GTM, * PGTM;

GTM   sGtm[MXTMS];
int   iNxt = 0;

// single instance of the runtime of the application
static GTM   sInitTm;

VOID  SetInitTime( VOID )
{
   PGTM  ptm = &sInitTm;
   ptm->tm_bt = QueryPerformanceFrequency( &ptm->tm_lif );
   if( ptm->tm_bt )
      QueryPerformanceCounter( &ptm->tm_lib ); // counter value
   else
      ptm->tm_dwtc = GetTickCount(); // ms since system started
}

double GetElapTime( VOID )
{
   DWORD          dwd;
   double         db;
   LARGE_INTEGER  lid;
   PGTM           ptm = &sInitTm;
   if( ptm->tm_bt )
   {
      QueryPerformanceCounter( &ptm->tm_lie ); // counter value
      lid.QuadPart = ( ptm->tm_lie.QuadPart - ptm->tm_lib.QuadPart ); // get difference
      db  = (double)lid.QuadPart / (double)ptm->tm_lif.QuadPart;
   }
   else
   {
      dwd = (GetTickCount() - ptm->tm_dwtc);   // ms elapsed
      db = ((double)dwd / 1000.0);
   }

   return db;

}

LPTSTR   GetElapsedStg( VOID )
{
   static TCHAR _s_szelap[264];
   LPTSTR   lps = _s_szelap;
   double   db  = GetElapTime();

   if( db < 60.0 )
   {
      sprintf( lps, "%s secs.",
         Dbl2Str( db, 5 ) );
   }
   else
   {
      INT   imins = (INT)(db / 60.0);
      db -= (60.0 * (double)imins);
      sprintf( lps, "%d:%s m:s.",
         imins,
         Dbl2Str( db, 5 ) );
   }

   return lps;
}

VOID  InitTimers( VOID )
{
   int   i;
   PGTM  ptm = &sGtm[0];

   for( i = 0; i < MXTMS; i++ )
   {
      ptm->tm_bInUse = FALSE;
      ptm++;
   }
   SetInitTime();
}

int  GetTimer( PGTM * pptm )
{
   PGTM  ptm = &sGtm[0];
   int   i = (int)-1;
   int   j;
   for( j = 0; j < MXTMS; j++ )
   {
      if( !ptm->tm_bInUse )
      {
         ptm->tm_bInUse = TRUE;
         i = j;
         *pptm = ptm;
         break;
      }
      ptm++;
   }
   return i;
}

/* =================================
 * int  SetBTime( void )
 *
 * Purpose: Set the beginning timer, and return the INDEX of that timer
 *
 * Return: Index (offset) of timer SET
 *
 */
int  SetBTime( void )
{
   PGTM  ptm;
   int   i;
   i = GetTimer( &ptm );
   if( i != (int)-1 )
   {
      if( ptm->tm_bt = QueryPerformanceFrequency( &ptm->tm_lif ) )
         QueryPerformanceCounter( &ptm->tm_lib ); // counter value
      else
         ptm->tm_dwtc = GetTickCount(); // ms since system started
   }
   return i;
}

/* =================================
 * double GetETime( int i )
 *
 * Purpose: Return ELAPSED time as double (in seconds) of the index given
 *
 * Return: If index is with the range of 0 to (MXTMS - 1) then
 *          compute ELAPSED time as a double in SECONDS
 *
 *         Else results indeterminate
 *
 */
double GetETime( int i )
{
   DWORD          dwd;
   double         db;
   LARGE_INTEGER  lid;
   PGTM           ptm;
   if( ( i < 0     ) ||
       ( i >= MXTMS ) )
   {
      db = (double)100.0;  // return an idiot number!!!
   }
   else
   {
      ptm = &sGtm[i];
      if( ptm->tm_bt )
      {
         QueryPerformanceCounter( &ptm->tm_lie ); // counter value
         lid.QuadPart = ( ptm->tm_lie.QuadPart - ptm->tm_lib.QuadPart ); // get difference
         db  = (double)lid.QuadPart / (double)ptm->tm_lif.QuadPart;
      }
      else
      {
         dwd = (GetTickCount() - ptm->tm_dwtc);   // ms elapsed
         db = ((double)dwd / 1000.0);
      }
      // make this timer available
      ptm->tm_bInUse = FALSE;
   }

   return db;

}

extern	void    Dbl2Stg( LPSTR lps, double factor, int prec );

LPTSTR   Dbl2Str( double factor, int prec )
{
   LPTSTR   lpr = _sGetSStg();
   *lpr = 0;
   Dbl2Stg(lpr,factor,prec);
   return lpr;
}

DWORD  IsValidFile( LPTSTR lpf, PWIN32_FIND_DATA pfd )
{
   DWORD     flg = 0;
   HANDLE   hFind = FindFirstFile( lpf, pfd );
   if( VFH( hFind ) )   // ie != INVALID_HANDLE_VALUE
   {
      if( pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
         flg = IS_FOLDER;
      else
         flg = IS_FILE;
      FindClose(hFind);
   }
   return flg;
}

LPTSTR   GetDBStg( LPTSTR lpm, INT iln )
{
   LPTSTR   lpr = _sGetSStg();
   DVGetFileTitle(lpm,lpr);
   wsprintf(EndBuf(lpr),"LN%d", iln);
   return lpr;
}

#undef  USECLR2

#ifdef   WRTCLRFILE
//typedef struct tagCLRHDR {
//   char  ch_szHdr[8];, etc
//#ifdef   USECLR2

VOID  GetCLRFile2( LPTSTR lpt, PDI lpDIBInfo ) // #ifdef   WRTCLRFILE
{
   LPTSTR lpf = &lpDIBInfo->di_szDibFile[0];
   INT      i, j;
   WIN32_FIND_DATA   fd;
   gszNxtClr[0] = 0;
   if( gszRunTime[0] )
   {
      // split runtime module file into some work pads
      DVGetFPath( gszRunTime, gszDrive, gszDir, gszFname, gszExt );
      strcpy( gszNxtClr, gszDrive );
      strcat( gszNxtClr, gszDir   );   // set the base directory
   }
   j = strlen(lpf);
   if( ( j ) &&
      ( IsValidFile( lpf, &fd ) & IS_FILE ) )
   {
      DWORD dwi = 0;
      for( i = 0; i < j; i++ )
      {
         dwi += lpf[i];    // accumulate a SUM of name
      }
      sprintf( lpt, "DV%08X%016X.CLR", dwi, fd.ftLastWriteTime.dwHighDateTime,
         fd.ftLastWriteTime.dwLowDateTime );
      strcat( gszNxtClr, lpt );
      strcpy( lpt, gszNxtClr );
   }
   else
   {
      lstrcpy( lpt, "DVT00001.CLR" );
      DVGetNewName( lpt );
      strcpy( gszNxtClr, lpt );
   }

   sprtf( "NextCLR=[%s]"MEOR, lpt );

}

//#else // !USECLR2
VOID  GetCLRFile( LPTSTR lpt, PDI lpDIBInfo ) // #ifdef WRTCLRFILE
{
   int   i, j, k, c;
   LPTSTR lpf = &lpDIBInfo->di_szDibFile[0];
   GetCLRFile2( lpt, lpDIBInfo );
   if( *lpf )
   {
      wsprintf(lpt,
         "DVT%s",
         lpf);
      j = lstrlen(lpt);
      k = 0;
      for( i = 0; i < j; i++ )
      {
         c = lpt[i];
         if( c == ':' )       // change COLON
            lpt[i] = '_';     // to UNDERSCORE
         else if( c == '\\' ) // change BACKSLASH
            lpt[i] = '-';     // to HYPHEN
         else if( c == '.' )  // remember the DOT
            k = i;
      }
      if(k)
      {
         i = k;
      }
      else
      {
         lstrcat(lpt,".");
         i = j;
      }
      // change to ??????.CLR
      lstrcpy( &lpt[i], ".CLR" );
      // ====================
   }
   else
   {
      lstrcpy( lpt, "DVT00001.CLR" );
      DVGetNewName( lpt );
   }
}
//#endif   // USECLR2 y/n

#endif   // #ifdef   WRTCLRFILE

HANDLE   DVCreateFile( LPTSTR lpf )
{
   HANDLE   hRet = 0;
   HANDLE   h;
   h = CreateFile( lpf, // pointer to name of the file
			GENERIC_READ | GENERIC_WRITE,			// access (read-write) mode 
			0,						// share mode 
			NULL,					// pointer
			CREATE_ALWAYS,			// how to create 
			FILE_ATTRIBUTE_NORMAL,	// file attributes 
			NULL );	// handle to file with attributes to copy
   if( VFH(h) )
   {
      hRet = h;
   }
   return hRet;
}

HANDLE   DVOpenFile2( LPTSTR lpf )
{
   HANDLE   hRet = 0;
   HANDLE   h;
   h = CreateFile( lpf, // pointer to name of the file
			GENERIC_READ,  // access (read-write) mode 
			0,					// share mode 
			NULL,					// pointer
			OPEN_EXISTING,			// how to create 
			FILE_ATTRIBUTE_NORMAL,	// file attributes 
			NULL );	// handle to file with attributes to copy
   if( VFH(h) )
   {
      hRet = h;
   }
   return hRet;
}

static TCHAR _s_szline[264];
VOID  DumpRGBQUAD( LPTSTR lps, DWORD cc )
{
   LPTSTR    lpd = _s_szline;
	RGBQUAD * lpq = (RGBQUAD *)lps;
   DWORD       ui, uk;  //, uj;
//   PRGBQUAD    pq;

//   FixRGBQ( lps );   // set FIRST 12 Colours, then set NO 2 indexes the ***SAME***
//   AnalRGBQ( lps );
	*lpd = 0;
	uk = 0;
	for( ui = 0; ui < cc; ui++ )
	{
//  0(  0,  0,  0)   1(  0,  0,191)   2(  0,191,  0)   3(  0,191,191) 
      sprintf( EndBuf(lpd),
			"%3d(%3d,%3d,%3d) ",
			ui,
			(lpq->rgbRed & 0xff),
			(lpq->rgbGreen & 0xff),
			(lpq->rgbBlue & 0xff) );
//    BYTE    rgbBlue; 
//    BYTE    rgbGreen; 
//    BYTE    rgbRed; 
		uk++;
		if( uk == 4 )
		{
		   sprtf("%s"MEOR, lpd );
			*lpd = 0;
			uk = 0;
		}
//  4(191,  0,  0)   5(191,  0,191)   6(191,191,  0)   7(192,192,192) 
//248(128,128,128) 249(  0,  0,255) 250(  0,255,  0) 251(  0,255,255) 
//252(255,  0,  0) 253(255,  0,255) 254(255,255,  0) 255(255,255,255) 
		lpq++;
	}

	if( uk )
	   sprtf("%s"MEOR, lpd );
}

char * GetRectStg( PRECT prc ) // like &m_Clip
{
   char * ps = (char *)GetNxtBuf();
   sprintf(ps, "(%d,%d,%d,%d)", prc->left, prc->top, prc->right, prc->bottom );
   return ps;
}

char * GetPointStg( PPOINT pp )
{
   char * ps = (char *)GetNxtBuf();
   sprintf(ps, "(%d,%d)", pp->x, pp->y );
   return ps;
}

// eof - DvUtil2.c
