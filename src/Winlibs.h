

// WinLibs.h
// Establish BEFORE here, whether CODE:32, Data 16 ...
// ie Entry into REAL MODE Libraries, etc ...

#ifndef	__WINLIBS_H_
#define	__WINLIBS_H_

#ifdef	Dv16_App
// If we can have - ALL FAR DATA, and
//					ALL FAR CALLS
//#pragma message ( "Loading 16-bit WINDOWS includes ..." )
// ======================================================

#undef	WIN32
#define	WIN16

//#include	<win16gi.h>		// ONLY avail. in 16-bit C700 INC/LIB
#include	<windows.h>
#include	<commdlg.h>
#include	<dlgs.h>
#include	<stdlib.h>
#include	<string.h>

#ifndef	MAX_PATH
#define	MAX_PATH		260
#endif	// MAX_PATH

#else	// !Dv16_App

// ELSE we are 32-BIT - ALL FAR DATA and ALL FAR CALL
// ==================================================
#if	(defined( WIN32 ) && defined( MS32_X86 ))
// ===========================================

#include <windows.h>

#ifndef _WINUSER_
#include <winuser.h>
#endif	// _WINUSER_

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <io.h>
#include <direct.h>
#include <stdlib.h>
#include <commdlg.h>
#include <dlgs.h>

#else	// !WIN32 & MS32_X86

/****** Common handle types *************************************************/

//#ifdef STRICT
//typedef const void NEAR*        HANDLE;
//#define DECLARE_HANDLE(name)    struct name##__ { int unused; };
//typedef const struct name##__ NEAR* name
//#define DECLARE_HANDLE32(name)  struct name##__ { int unused; };
//typedef const struct name##__ FAR* name
//#else   /* STRICT */
//typedef UINT                    HANDLE;
//#define DECLARE_HANDLE(name)    typedef UINT name
//#define DECLARE_HANDLE32(name)  typedef DWORD name
//#endif  /* !STRICT */
#include <windows.h>
//#include <direct.h>		// For _getcwd - Could use 32-bit COMPAT
#include <stdio.h>   // FIX20010130 - for sprintf() as opposed to wsprintf()!


#endif	// WIN32 && MS32_X86 

// ==================================================
#endif	// Dv!6_App

#endif	// __WINLIBS_H_

// eof - WinLibs.h = CHOICE of LIBRARIES!!!!!!!
