

// gmUtil.c
#ifndef  _CRT_SECURE_NO_WARNINGS
#define  _CRT_SECURE_NO_WARNINGS
#endif // #ifndef  _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

#include	<windows.h>
#include	<direct.h>
#include <stdio.h>
#include	"gmUtils.h"		// Exposes service of module

#ifndef	BYTES_PER_READ
#define	BYTES_PER_READ		1024
#endif	// BYTES_PER_READ

// This module contains
//int	DVGetCwd( LPSTR lpb, DWORD siz );
//BOOL    GetRootDir( LPSTR lpf );
//BOOL CenterWindow(HWND hwndChild, HWND hwndParent);
//void Hourglass (BOOL bDisplay);
//int	EnsureCrLf( LPSTR lpd, LPSTR lps );
//LPSTR	DVf2s( double source );
//#ifdef	WIN32
//DWORD	GetTextExtent( HDC hdc, LPSTR lpS, int len );
//#endif	// WIN32
// but see GmUtils.h for the complete list exposed
// ===============================================

char	szCWD[MAX_PATH+16] = { "\0" };

#ifdef   ADD_STGBUF

#define		MXSTGS		32
#define		MXONE		   256
#define		MXONEX	   (MXONE + 8)
//#define  GetNxtBuf   _sGetSStg
LPTSTR	GetStgBuf( VOID )
{
	LPTSTR	lprs;
	static TCHAR szP2S[ (MXSTGS * MXONEX) ];
	static LONG  iNP2S = 0;
	// NOTE: Can be called only MXSTGS times before repeating
	lprs = &szP2S[ (iNP2S * MXONEX) ];	// Get 1 of ? buffers
	iNP2S++;
	if( iNP2S >= MXSTGS )
		iNP2S = 0;
	return lprs;
}

#endif   // #ifdef   ADD_STGBUF

#ifdef   ADD_SPRTF

extern   void DiagMsg( LPTSTR lpd );
void _cdecl sprtf( LPTSTR lpf, ... )
{
   static TCHAR _s_sprtfbuf[1024];
   LPTSTR      lpb = &_s_sprtfbuf[0];
   int         i;
   va_list     arg_ptr;
   va_start(arg_ptr, lpf);
   i = vsprintf( lpb, lpf, arg_ptr );
   va_end(arg_ptr);
   //sprtf(lpb);
   //prt(lpb);
   DiagMsg(lpb);
}

#ifdef   ADD_CHKME
extern   DWORD InStr( LPTSTR lps, LPTSTR lpi );
#endif   // ADD_CHKME
void _cdecl chkme( LPTSTR lpf, ... )
{
   static TCHAR _s_chkmebuf[1024];
   static int _s_incheckme = 0;
   LPTSTR      lpb = &_s_chkmebuf[0];
   int         i;
   va_list     arg_ptr;
   va_start(arg_ptr, lpf);
   i = vsprintf( lpb, lpf, arg_ptr );
   va_end(arg_ptr);
   //sprtf(lpb);
   //prt(lpb);
#ifdef   ADD_CHKME
#ifndef  BVTEST
   // add some 'visible' noise if string small
   if( i < 200 )
   {
      LPTSTR   lps = GetStgBuf();
      *lps = 0;
      if( !InStr(lpb,"CHECKME") )
         strcpy(lps,"CHECKME: ");
      if( !InStr(lpb,"WARNING") )
         strcat(lps,"WARNING: ");
      strcat(lps,lpb);
      DiagMsg(lps);
   }
   else
#endif // #ifndef  BVTEST
#endif   // ADD_CHKME
   {
      DiagMsg(lpb);
   }
   if( !_s_incheckme )
   {
      _s_incheckme = 1;
      if(( i > 2         ) &&
         ( lpb[0] == 'C' ) &&
         ( lpb[1] == ':' ) )
      {
         i = MessageBox(NULL,
            lpb,
            "CRITICAL ERROR: CHECK ME",
            (MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_SYSTEMMODAL) );
         if( i == IDABORT )
         {
            //DestroyWindow(ghwndFrame);
            exit(-1);
         }
      }
      _s_incheckme = 0;
   }
}

#endif   // #ifdef   ADD_SPRTF

#ifdef   ADD_RECT2STG
LPTSTR	Rect2Stg( PRECT lpr )
{
	LPSTR	lps = GetStgBuf();
   sprintf( lps,
			"(%d,%d,%d,%d)",
			lpr->left,
			lpr->top,
			lpr->right,
			lpr->bottom );
	return lps;
}

#endif   // #ifdef   ADD_RECT2STG

LPTSTR	Pt2Stg( PPOINT ppt )
{
	LPSTR	lps = GetStgBuf();
   sprintf( lps,
			"(%d,%d)",
			ppt->x,
			ppt->y );
	return lps;
}

//
// Get current work directory
//
int	DVGetCwd( LPSTR lpb, DWORD siz )
{
	LPSTR	lpd;
	int		i;

	i = 0;
	lpd = &szCWD[0];
	if( *lpd == 0 )
	{
		_getcwd( lpd, MAX_PATH );
		if( i = lstrlen( lpd ) )
		{
			if( lpd[i-1] != '\\' )
				lstrcat( lpd, "\\" );
		}
	}
	if( lpb && siz )	// Check the CALLER
	{
		*lpb = 0;
		if( i = lstrlen( lpd ) )
		{
			if( (DWORD)i < siz )
			{
				lstrcpy( lpb, lpd );
			}
			else
			{
				for( i = 0; (DWORD)i < siz; i++ )
				{
					lpb[i] = lpd[i];
				}
				i--;
				lpb[i] = 0;
			}
		}
		i = lstrlen( lpb );
	}

	return i;
}

//
// BOOL    GetRootDir( LPSTR lpf )
//
// Purpose: Get the ROOT RUNTIME Directory
//			This could be the current work directory,
//			that is the directory "Windows" has as
//			the "current",
//			*** OR ***
//			The directory where this EXE was run
//			from.
// Input:	LPSTR lpf - Buffer for results
// Output:	BOOL - TRUE if OK
//
BOOL    GetRootDir( LPSTR lpf )
{
	BOOL    flg;
	DWORD   dwL, dwO, dwI;
	char	buf[260];
	char    c;
	int		i;
	LPSTR	lpb;

	flg    = FALSE;		// Assume FAILED
	if( lpf )	// If given a buffer
	{
		*lpf = 0;		// Start with NOTHING
		lpb = &buf[0];	// Set up a pointer
		buf[0] = 0;		// Init it to zero also
		//_getcwd( &buf[0], 256 );	// Get DIRECTORY
		if( i = DVGetCwd( lpb, 256 ) )	// Get DIRECTORY
		{
			// Simple - Copy it to CALLER
			lstrcpy( lpf, lpb );	// Copy it, and
			flg = TRUE;				// Set OK
		}
		else if( dwL = GetModuleFileName( NULL, lpf, MAX_PATH ) )
		{
			// This method has the BAD effect of returning
			// like D:\fff\fff\DEBUG\ or RELEASE,
			// or other MS Studio folders where the EXE
			// is written.
			// BUT, the _getcwd should NEVER fail!!!
			dwO = 0;
			// Move only the ROOT without module name
			for( dwI = 0; dwI < dwL; dwI++ )
			{
				c = lpf[dwI];
				if( (c == '\\') ||
					(c == '/') ||
					(c == ':') )
				{
					// Keep LAST of any of these
					dwO = dwI + 1;
				}
			}
			lpf[dwO] = 0;	// Zero terminate it
			if( dwO )	// if length
				flg = TRUE;
		}
	}
	return( flg );
}

//
//  FUNCTION: CenterWindow(HWND, HWND)
//
//  PURPOSE:  Center one window over another.
//
//  PARAMETERS:
//    hwndChild - The handle of the window to be centered.
//    hwndParent- The handle of the window to center on.
//
//  RETURN VALUE:
//
//    TRUE  - Success
//    FALSE - Failure
//
//  COMMENTS:
//
//    Dialog boxes take on the screen position that they were designed
//    at, which is not always appropriate. Centering the dialog over a
//    particular window usually results in a better position.
//

BOOL CenterWindow(HWND hwndChild, HWND hwndParent)
{
    RECT    rcChild, rcParent;
    int     cxChild, cyChild, cxParent, cyParent;
    int     cxScreen, cyScreen, xNew, yNew;
    HDC     hdc;

    // Get the Height and Width of the child window
    GetWindowRect(hwndChild, &rcChild);
    cxChild = rcChild.right - rcChild.left;
    cyChild = rcChild.bottom - rcChild.top;

    // Get the Height and Width of the parent window
    GetWindowRect(hwndParent, &rcParent);
    cxParent = rcParent.right - rcParent.left;
    cyParent = rcParent.bottom - rcParent.top;

    // Get the display limits
    hdc = GetDC(hwndChild);
    cxScreen = GetDeviceCaps(hdc, HORZRES);
    cyScreen = GetDeviceCaps(hdc, VERTRES);
    ReleaseDC(hwndChild, hdc);

    // Calculate new X position, then adjust for screen
    xNew = rcParent.left + ((cxParent - cxChild) / 2);
    if (xNew < 0)
    {
         xNew = 0;
    }
    else if ((xNew + cxChild) > cxScreen)
    {
        xNew = cxScreen - cxChild;
    }

    // Calculate new Y position, then adjust for screen
    yNew = rcParent.top  + ((cyParent - cyChild) / 2);
    if (yNew < 0)
    {
        yNew = 0;
    }
    else if ((yNew + cyChild) > cyScreen)
    {
        yNew = cyScreen - cyChild;
    }

    // Set it, and return
    return SetWindowPos(hwndChild,
                        NULL,
                        xNew, yNew,
                        0, 0,
                        SWP_NOSIZE | SWP_NOZORDER);
}

//---------------------------------------------------------------------
//
// Function:   Hourglass
//
// Purpose:    Displays or hides the hourglass during lengthy operations.
//
// Parms:      bDisplay == TRUE to display, false to put it away.
//
// History:	Date			Reason
//
//			6/1/91			Created.
//			17 June, 1997	Moved to DvUtil.c             
//---------------------------------------------------------------------

void Hourglass (BOOL bDisplay)
{
   static HCURSOR hOldCursor = NULL;
   static int     nCount     = 0;

	if( bDisplay )
	{
		// Check if we already have the hourglass up and increment
		//  the number of times Hourglass (TRUE) has been called.
		if( nCount++ )
			return;

		hOldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );

		// If this machine doesn't have a mouse, display the
		//  hourglass by calling ShowCursor(TRUE) (if it does
		//  have a mouse this doesn't do anything much).
		ShowCursor( TRUE );
	}
	else
	{
		// If we haven't changed the cursor, return to caller.
		if( !nCount )
			return;
		// If our usage count drops to zero put back the cursor
		//  we originally replaced.
		if( !(--nCount) )
		{
			SetCursor( hOldCursor );
			hOldCursor = NULL;
			ShowCursor( FALSE );
		}
	}
}

// Include the service TransparentBlt( HDC, HBITMAP,
//	DestX, DestY, COLORREF )
// ===============================================

//#include	"DvTrans.h"

// MAKE SURE the standard DOS Cr/Lf is used
// ========================================
int	EnsureCrLf( LPSTR lpd, LPSTR lps )
{
	int		i, j, k;
	char	c, d;

	k = 0;
	if( lpd && lps &&
		(i = lstrlen( lps )) )
	{
		d = 0;
		for( j = 0; j < i; j++ )
		{
			c = lps[j];
			if( c == '\r' )
			{	// If a CR
				if( (j+1) < i )
				{	// and there is more
					if( lps[j+1] != '\n' )
					{	// but NOT a LF
						lpd[k++] = c;	// Add the CR
						c = '\n';		// and set LF
					}
				}
				else
				{	// no more
					lpd[k++] = c;	// Add the CR
					c = '\n';		// and set LF
				}
			}
			else if( c == '\n' )
			{	// If a LF
				if( d != '\r' )
				{	// If no previous CR
					lpd[k++] = '\r';	// Add a CR
				}
			}
			lpd[k++] = c;	// Add char
			d = c;			// keep last put
		}
		if( c != '\n' )
		{	// If it does NOT end in a Cr/Lf
			lpd[k++] = '\r';	// then ADD THEM NOW
			lpd[k++] = '\n';
		}
	}
	return k;	// Return the NEW length
}

//#define	MXFSB	32
//#define	MXFSBS	8
#define	MXFSB	   264
#define	MXFSBS	32
TCHAR    szFSBufs[MXFSB*MXFSBS];
INT		iFSBuf = 0;
BOOL	   bFilNPlace = FALSE; //TRUE;	// Put a SPACE for the NEG sign
BOOL	   bAddSig3 = FALSE; //TRUE;	// Always min. 3 digits before decimal
BOOL	   bFilNeg10 = FALSE; //TRUE;
BOOL	   bFilPos10 = FALSE; //TRUE;

LPTSTR	GetABuf( VOID )
{
	LPTSTR	lpb;
	lpb = &szFSBufs[ (iFSBuf * MXFSB) ];   // select NEXT buffer
	iFSBuf++;      // bump buffer index
	if( iFSBuf >= MXFSBS )  // check against max
		iFSBuf = 0; // roll back to zero
	return lpb;
}

//
// Convert "double" to string
// with a little bit of conditioning
// 
LPSTR	DVf2s( double source )
{
	int     decimal,   sign;
	char    *buffer;
	char	wbuf[MAX_PATH+8];
	int     precision = 10;
	LPSTR	lpb, lpwb;
	int		i, j, k, l, sd, fd;
	char	c, d;

	// Do the CONVERSION to ASCII at precision 10
	// Returns buffer pointer to digits, and
	// a decimal position counter, and
	// the sign.
	buffer = _ecvt( source, precision, &decimal, &sign );
	lpb = GetABuf();	// Get a (next) text buffer
	lpwb = &wbuf[0];	// and a work buffer (if reqd)
	i = lstrlen( buffer );	// Get length of digits.
	k = 0;
	if( sign )
	{
		lpb[k++] = '-';		// Add in the Minus
	}
	else
	{
		if( bFilNPlace )
		{
			lpb[k++] = ' ';		// Else a SPACE
		}
	}

	if( decimal < 0 )
	{
		// We have just a DECIMAL
		sd = 0;
		if( bAddSig3 )
		{
			lpb[k++] = ' ';
			lpb[k++] = ' ';
		}
		// Now start the DECIMAL
		lpb[k++] = '0';
		lpb[k++] = '.';
		while( decimal < 0 )
		{
			if( decimal < (int)-10 )
			{
				wsprintf( &lpb[k], "0+%d+0s", -(decimal) );
				k = lstrlen( lpb );
			}
			else
			{
				lpb[k++] = '0';
				sd++;		// Counted as significant
				decimal++;
			}
		}
		d = 0;
		for( j = 0; j < i; j++ )
		{
			c = buffer[j];
			if( j && ( c == '0' ) )
			{
				if( d == 0 )
				{
					d = c;
					l = j;
				}
			}
			else
			{
				if( d )
				{
					for( ; l < j; l++ )
					{
						lpb[k++] = d;
						sd++;
					}
				}
				d = 0;
				lpb[k++] = c;
				sd++;
			}
		}
		if( bFilNeg10 &&
			( sd < 10 ) )
		{
			fd = 10 - sd;
			while( fd-- )
			{
				lpb[k++] = ' ';
			}
		}
	}
	else	// Decimal is positive
	{
		if( decimal == 0 )
		{
			lpb[k++] = ' ';
			lpb[k++] = ' ';
			lpb[k++] = '0';
			lpb[k++] = '.';
			decimal--;
		}
		else if( ( decimal < 3 ) &&
			bAddSig3 )
		{
			int	tdec;
			tdec = 3 - decimal;
			while( tdec-- )
			{
				lpb[k++] = ' ';
			}
		}
		d = 0;
		sd = 0;
		for( j = 0; j < i; j++ )
		{
			c = buffer[j];
			if( j == decimal )
			{
				if( d )
				{
					for( ; l < j; l++ )
					{
						lpb[k++] = d;
						sd++;
					}
				}
				d = 0;
				lpb[k++] = '.';
			}
			if( j && ( c == '0' ) )
			{
				if( d == 0 )
				{
					d = c;
					l = j;
				}
			}
			else
			{
				if( d )
				{
					for( ; l < j; l++ )
					{
						lpb[k++] = d;
						sd++;
					}
				}
				d = 0;
				lpb[k++] = c;
				sd++;
			}
		}
		if( bFilPos10 &&
			( sd < 10 ) )
		{
			fd = 10 - sd;
			while( fd-- )
			{
				lpb[k++] = ' ';
			}
		}
	}
	if( k )
	{
		if( lpb[k-1] == '.' )
			lpb[k++] = '0';
	}
	lpb[k] = 0;
	return lpb;
}


// **************************************************************
//
// FUNCTION   : DVWrite(HANDLE fh, VOID MLPTR pv, DWORD ul)
//
// PURPOSE    : Writes data in steps of 32k
//				till all the data is written.
//
// RETURNS    : 0 - If write did not proceed correctly.
//		 number of bytes written otherwise.
//
// **************************************************************
DWORD DVWrite(HANDLE fh, LPSTR pv, DWORD ul )
{
	DWORD	ulT = ul;
	LPSTR	hp = pv;
	HFILE   hf = (HFILE)PtrToLong(fh);

	while( ul > BYTES_PER_READ )
	{
		if( _lwrite(hf, (LPSTR)hp, (WORD)BYTES_PER_READ) != BYTES_PER_READ )
			return 0;
		ul -= BYTES_PER_READ;
		hp += BYTES_PER_READ;
	}
	if( ul )
	{
		if( _lwrite(hf, (LPSTR)hp, (WORD)ul) != (WORD)ul )
			return 0;
	}

	return ulT;
}

//HFILE OpenFile(
//    LPCSTR lpFileName,	// pointer to filename 
//    LPOFSTRUCT lpReOpenBuff,	// pointer to buffer for file information  
//    UINT uStyle	// action and attributes 
HANDLE	DVOpenFile( LPSTR lpf, LPOFSTRUCT pof, UINT uStyle )
{
	HANDLE	hf;
	HFILE fh;
	fh = OpenFile( lpf, pof, uStyle );
	hf = IntToPtr(fh);
	return hf;
}

// The _llseek function repositions the file pointer in a
// previously opened file. This function is provided for
// compatibility with 16-bit versions of Windows.
// Win32-based applications should use the SetFilePointer
// function. 
// LONG _llseek(
//    HFILE hFile,	// handle to file 
//    LONG lOffset,	// number of bytes to move  
//    int iOrigin ); 	// position to move from 
//	
//The SetFilePointer function moves the file pointer of an open
//file. 
//DWORD SetFilePointer(
//    HANDLE hFile,	// handle of file 
//    LONG lDistanceToMove,	// number of bytes to move file
//pointer 
//    PLONG lpDistanceToMoveHigh,	// address of high-order word of
//distance to move  
//    DWORD dwMoveMethod 	// how to move 
//   );	
//Parameters
//hFile
//Identifies the file whose file pointer is to be moved. The file
//handle must have been created with GENERIC_READ or GENERIC_WRITE
//access to the file. 
//lDistanceToMove
//Specifies the number of bytes to move the file pointer. A
//positive value moves the pointer forward in the file and a
//negative value moves it backward. 
//lpDistanceToMoveHigh
//Points to the high-order word of the 64-bit distance to move. If
//the value of this parameter is NULL, SetFilePointer can operate
//only on files whose maximum size is 2^32 - 2. If this parameter
//is specified, the maximum file size is 2^64 - 2. This parameter
//also receives the high-order word of the new value of the file
//pointer. 
//dwMoveMethod
//Specifies the starting point for the file pointer move. This
//parameter can be one of the following values: 
//Value	Meaning
//FILE_BEGIN	The starting point is zero or the beginning of the
//file. If FILE_BEGIN is specified, DistanceToMove is interpreted
//as an unsigned location for the new file pointer.
//FILE_CURRENT	The current value of the file pointer is the
//starting point.
//FILE_END	The current end-of-file position is the starting point.
//Return Values
//If the SetFilePointer function succeeds, the return value is the
//low-order doubleword of the new file pointer, and if
//lpDistanceToMoveHigh is not NULL, the function puts the
//high-order doubleword of the new file pointer into the LONG
//pointed to by that parameter. 
//If the function fails and lpDistanceToMoveHigh is NULL, the
//return value is 0xFFFFFFFF. To get extended error information,
//call GetLastError. 
//If the function fails, and lpDistanceToMoveHigh is non-NULL, the
//return value is 0xFFFFFFFF and GetLastError will return a value
//other than NO_ERROR. 
//Remarks
//You cannot use the SetFilePointer function with a handle to a
//nonseeking device, such as a pipe or a communications device. To
//determine the file type for hFile, use the GetFileType function.
//You should be careful when setting the file pointer in a
//multithreaded application. For example, an application whose
//threads share a file handle, update the file pointer, and read
//from the file must protect this sequence by using a critical
//section object or mutex object. For more information about these
//objects, see Mutex Objects and Critical Section Objects.
//If the hFile file handle was opened with the
//FILE_FLAG_NO_BUFFERING flag set, an application can move the
//file pointer only to sector-aligned positions. A sector-aligned
//position is a position that is a whole number multiple of the
//volume's sector size. An application can obtain a volume's
//sector size by calling the GetDiskFreeSpace function. If an
//application calls SetFilePointer with distance-to-move values
//that result in a position that is not sector-aligned and a
//handle that was opened with FILE_FLAG_NO_BUFFERING, the function
//fails, and GetLastError returns ERROR_INVALID_PARAMETER.
//Note that if the return value is 0xFFFFFFFF and if
//lpDistanceToMoveHigh is non-NULL, an application must call
//GetLastError to determine whether the function has succeeded or
//failed. The following sample code illustrates this point: 
////  
//// Case One: calling the function with 
////           lpDistanceToMoveHigh == NULL 
//// try to move hFile's file pointer some distance 
//dwPointer = SetFilePointer (hFile, lDistance, 
//                            NULL, FILE_BEGIN) ; 
//// if we failed ... 
//if (dwPointer == 0xFFFFFFFF) { 
//// obtain the error code 
//    dwError = GetLastError() ; 
//// deal with that failure 
//    . 
//    . 
//    . 
//} // end of error handler 
//// 
//// Case Two: calling the function with 
////           lpDistanceToMoveHigh != NULL 
//// try to move hFile's file pointer some huge distance 
//dwPointerLow = SetFilePointer (hFile, lDistanceLow, 
//                               & lDistanceHigh, FILE_BEGIN) ; 
//// if we failed ... 
//if (dwPointerLow == 0xFFFFFFFF 
//    && 
//    (dwError = GetLastError()) != NO_ERROR ){ 
//// deal with that failure 
//    . 
//    . 
//    . 
//} // end of error handler 
//See Also
//GetDiskFreeSpace, GetFileType, ReadFile, ReadFileEx, WriteFile,
////WriteFileEx 
 
DWORD	DVSeekEnd( HANDLE hf )
{
#ifdef	WIN32
	DWORD	dw;
	HANDLE	hFile;
	dw = (DWORD)-1;
	if( ( hFile = hf ) &&
		( hf != INVALID_HANDLE_VALUE ) ) //  HFILE_ERROR
	{
		dw = SetFilePointer( hFile,	// handle of file 
			0,		// number of bytes to move file pointer
			NULL,	// address of high-order word of distance to move
			FILE_END );	// // how to move
	}
	return( dw );
#else	// !WIN32
	return( _llseek( hf, 0, 2 ) );	// Ensure at EOF ... 
#endif	// WIN32 y/n
}

HANDLE DVlclose( HANDLE hf )
{
	HANDLE hFile = (HANDLE)-1;	// HFILE_ERROR;
	if( hf && (hf != (HANDLE)-1) )
	{
		HFILE fh = PtrToInt(hf);
		fh = _lclose( fh );
		hFile = IntToPtr(fh);
	}
	return( hFile );
}

/* Given a BUFFER, the Current Direcory and a File name */
/* Build a complete PATH into the Buffer */
void	SRSetupPath( LPSTR lpb, LPSTR lpd, LPSTR lpf )
{
WORD i;
char	c;
	i = 0;
	if( lpb && lpf && lpf[0] )
	{
		if( lpd && lpd[0] )
		{
			lstrcpy( lpb, lpd );
			if( i = lstrlen( lpb ) )
			{
				c = lpb[i-1];
				if( !(( c == ':') || ( c == '\\')) )
					lstrcat( lpb, "\\" );
			}
		}
		lstrcat( lpb, lpf );
	}
}

#ifdef	WIN32
void	chkte( void )
{
	int	i;
	i = 0;
}

DWORD	GetTextExtent( HDC hdc, LPSTR lpS, int len )
{
	TEXTMETRIC	tm;
	SIZE		sz, szt;
	DWORD		dwS;
	int			i;

	GetTextExtentPoint32( hdc,	// handle of device context
		lpS,	// address of text string
		len,	// number of characters in string
		&sz ); 	// address of structure for string size
	dwS = (DWORD)MAKELONG( sz.cx, sz.cy );
	//dwS =  (((WORD)sz.cy << 16) | ((WORD)sz.cx) );
	if( GetTextMetrics( hdc, &tm ) )
	{
		// FIX981227 Can NOT now understand WHY I have ADDED this
		// tmInternalLeading to the HEIGHT. If ANYTHING is added
		// it SHOULD be tmExternalLeading, as any additional space
		// between ROWS of text.
		//szt.cy = (tm.tmHeight + tm.tmInternalLeading);
		// BUT actually WHY NOT JUST USE HEIGHT!!!!
		szt.cy = tm.tmHeight;	// FIX981227 - Just use HEIGHT
		szt.cx = (tm.tmAveCharWidth * len);
		i = 0;
		if( szt.cx > sz.cx )
		{
			i++;
			sz.cx = szt.cx;
		}
		if( szt.cy > sz.cy )
		{
			i++;
			sz.cy = szt.cy;
		}
		if( i )
		{
			chkte();	// we are ALTERING the SIZE!!!
			// ***************************************
			dwS = (DWORD)MAKELONG( sz.cx, sz.cy );
			// ***************************************
		}
	}
	return( dwS );
}

#endif	// WIN32


char	GMUpper( char c )
{
	char	d;
	if( (c >= 'a') && (c <= 'z') )
		d = c & 0x5f;
	else
		d = c;
	return( d );
}

int		GMInStr( LPSTR lpsrc, LPSTR lpfind )
{
	int		iAt, islen, iflen, i, j, k;
	char	c, d, b;

	iAt = 0;	// NOT FOOUND yet
	if( lpsrc && lpfind &&
		( islen = lstrlen( lpsrc ) ) &&
		( iflen = lstrlen( lpfind ) ) )
	{
		d = GMUpper( lpfind[0] );
		for( i = 0; i < islen; i++ )
		{
			c = GMUpper( lpsrc[i] );
			if( c == d )
			{
				if( iflen == 1 )
				{
					iAt = i+1;
					break;
				}
				else
				{
					if( (islen - i) >= iflen )
					{
						// ok, we have the length
						k = i + 1;	// Get to NEXT char
						for( j = 1; j < iflen; j++ )
						{
							c = GMUpper( lpsrc[k] );	// Get next
							b = GMUpper( lpfind[j] );
							if( c != b )
								break;
							k++;
						}
						if( j == iflen )
						{
							iAt = k + 1;
							break;
						}
					}
					else
					{
						// not enough length left
						break;
					}
				}
			}
		}
	}
	return iAt;
}

BOOL	GMIsExt( LPSTR lpsrc, LPSTR lpfind )
{
	BOOL	flg;
	int		i, j, k, l;

	flg = FALSE;
	if( ( lpsrc ) &&
		( i = lstrlen(lpsrc) ) &&
		( lpfind ) &&
		( j = lstrlen(lpfind) ) &&
		( l = GMInStr( lpsrc, "." ) ) )
	{
//		if( ( l + 1 ) == k )
		if( k = GMInStr( &lpsrc[l], lpfind ) )
		{
			flg = TRUE;
		}
	}
	return flg;
}

// **********************************************************
// Added 1998 May 17
// =================
/* =====================================================================
 *
 * GetFPath( lpFullPath, lpDrive, lpPath, lpFile, lpExtent )
 *
 * Purpose: Split the supplied FULL PATH into four components if
 * the buffers are supplied. Those components NOT required may be
 * NULL. This is a FAR PTR implementation of _splitpath in STDLIB.H.
 * The supplied buffers must be at least equal to the manifest contants
 * of _MAX_DRIVE, _MAX_DIR, _MAX_FNAME, _MAX_EXT in stdlib.h ...
 * The Drive will include the ':'; The Directory will include both the
 * leading and trailing '\' or '/'; The file name will be exactly that;
 * the Extent will include the '.' character; Such that a recombination
 * by say lstrcat will put it all back together ...
 * Any component requested that does not exist in the FULL PATH will 
 * contain a nul string.
 * (see grmlib.c for same/similar SplitFN - splitfilename into components)
 * ===================================================================== */
VOID	GMGetFPath( LPTSTR lpc, LPTSTR lpd, LPTSTR lpp, LPTSTR lpf, LPTSTR lpe )
{
	int	i, j, k;
	LPTSTR	lps;
	char	c;

	lps = lpc;
	if( lpd )
		lpd[0] = 0;
	if( lpp )
		lpp[0] = 0;
	if( lpf )
		lpf[0] = 0;
	if( lpe )
		lpe[0] = 0;

	if( ( lps ) &&
		( i = lstrlen( lps ) ) )
	{
		if( lps[1] == ':' )	/* If a DRIVE ... */
		{	
			k = 2;
			if( lpd )	/* If a DRIVE requested ... */
			{
				lpd[0] = lps[0];
				lpd[1] = lps[1];
				lpd[2] = 0;
			}
			lps += 2;
			if( k <= i )
				i = i - k;
			else
				i = 0;
		}
		if( i )
		{
			c = lps[0];
			k = 0;
			if( (c == '\\') || (c == '/') )
			{
				for( j = 0; j < i; j++ )
				{
					c = lps[j];
					if( (c == '\\') || (c == '/') )
						k = j + 1;	/* include this char in move ... */
					if( lpp )
					{
						if( j < _MAX_DIR )
							lpp[j] = c;
					}
				}
				if( lpp )
				{
					if( k < _MAX_DIR )
						lpp[k] = 0;	/* Put the NUL at the correct place ... */
					else
						lpp[_MAX_DIR - 1] = 0;
				}
			}
			lps += k;
			if( k <= i )
				i = i - k;
			else
				i = 0;
		}
		if( i )
		{
			k = 0;
			for( j = 0; j < i; j++ )
			{
				c = lps[j];
				if( c == '.' )
				{
					k = j;
					break;
				}
				if( lpf )
				{
					if( j < _MAX_FNAME )
						lpf[j] = c;
				}
			}
			lps += k;
			if( k <= i )
				i = i - k;
			else
				i = 0;
			if( lpf )
			{
				if( j < _MAX_FNAME )
					lpf[j] = 0;
				else
					lpf[_MAX_FNAME - 1] = 0;
			}
		}
		if( i && (c == '.') )
		{
			for( j = 0; j < i; j++ )
			{
				c = lps[j];
				if( lpe )
				{	
					if( j < _MAX_EXT )
						lpe[j] = c;
				}
			}
			if( lpe )
			{
				if( j < _MAX_EXT )
					lpe[j] = 0;
				else
					lpe[_MAX_EXT - 1] = 0;
			}
		}
	}
}

int	GMGetFName( LPSTR lpfull, LPSTR lpname )
{
	int		i = 0;
	char	nm[_MAX_FNAME];
	char	ext[_MAX_EXT];
	if( ( lpfull ) &&
		( lpname ) )
	{
		GMGetFPath( lpfull, 0, 0, &nm[0], &ext[0] );
		lstrcpy( lpname, &nm[0] );
		lstrcat( lpname, &ext[0] );
		i = lstrlen( lpname );
	}
	return i;
}


int	GMRTrimStg( LPSTR lps )
{
	int		i, j, k;

	k = 0;
	if( ( lps ) &&
		( i = lstrlen(lps) ) )
	{
		for( j = ( i - 1 ); j > 0; j-- )
		{
			if( lps[j] <= ' ' )
			{
				lps[j] = 0;
				k++;
			}
			else
			{
				break;
			}
		}
	}
	return k;
}


#define		MXNCNUM		128

// ===================================================
// void	GetNiceNum( LPSTR lpdest, DWORD num )
//
// Return NICE LOOKING large number
// ie 8123456 comes back as 8,123,456
// in the supplied buffer
//
// ===================================================
void	GetNiceNum( LPSTR lpdest, DWORD num )
{
	LPSTR	lpn, lpr;
	char	buf[MXNCNUM];
	int		i, j, k;

	lpn = &buf[0];
	lpr = lpdest;
	wsprintf( lpn, "%u", num );
	if( i = lstrlen( lpn ) )
	{
		if( i > 3 )
		{
			k = 0;
			for( j = 0; j < i; j++ )
			{
				if( ( (i - j) % 3 ) == 0 )
					lpr[k++] = ',';
				lpr[k++] = lpn[j];
			}
			lpr[k] = 0;
		}
		else
		{
			lstrcpy( lpr, lpn );
		}
	}
}

// =====================================================
// LPSTR	GetNNumSStg( DWORD num )
//
// Return a static buffer containing the nice number
//
// Note use of just TWO buffers before over-write
//
// =====================================================
LPSTR	GetNNumSStg( DWORD num )
{
	LPSTR	lpr;
	static	char	_numbuf[MXNCNUM*2];
	static	int		_innum;
	if( _innum )
	{
		lpr = &_numbuf[0];
		_innum = 0;
	}
	else
	{
		lpr = &_numbuf[MXNCNUM];
		_innum++;
	}
	GetNiceNum( lpr, num );
	return lpr;
}


// **********************************************************
// eof - gmUtil.c
