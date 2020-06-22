
// win32.h

#ifndef	_mwin32_h
#define	_mwin32_h

#ifndef RC_INVOKED

#ifdef	_INC_WINDOWS
// =====================================

// Makes it easier to determine appropriate code paths:
#if defined (WIN32)
        #define IS_WIN32 TRUE
#else	// !WIN32
        #define IS_WIN32 FALSE
#endif	// WIN32 y/n

#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) &&\
        (LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95 (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32

// Use UNIVERSAL/GLOBAL from G library ...
// Include WScanf.c in link, and GWScanf.h available
// =================================================
extern	int _gwscanf( LPSTR, LPSTR, LPSTR );
//int dv_wscanf( LPSTR lpd, LPSTR lpf, PINT8 lpv )
#define	dv_wscanf( a, b, c )	_gwscanf( a, b, c )
// =====================================
#endif	// _INC_WINDOWS

#ifdef	WIN32
// =============================================================

#define EXPORT32 __declspec(dllexport)

#define	dv_fmemcpy	memcpy
#define	dv_fmemset	memset
#define	PWORD	WORD *
#define	PINT8	BYTE *
#define	MEXPORTS	PASCAL
#define	MLPTR	*
#define	MLIBCONV	PASCAL		// Still pascal
#define	MLIBCALL	MLIBCONV
#define	MFILEPTR	void MLPTR

#ifdef	_INC_WINDOWS
// =======================================
#ifdef WIN64
#define	SetWindowExtra( a, b, c )	SetWindowLongPtr( a, b, c )
#define	GetWindowExtra( a, b )		(HANDLE)GetWindowLongPtr( a, b )
#else // !WIN64
#define	SetWindowExtra( a, b, c )	SetWindowLong( a, b, (LONG) c )
#define	GetWindowExtra( a, b )		(HANDLE)GetWindowLong( a, b )
#endif // WIN646 y/n

#ifndef	_WINGDI_
#include <wingdi.h>
#endif	// _WINGDI_
typedef	RGBTRIPLE MLPTR LPRGBTRIPLE;
#endif	// _INC_WINDOWS

// =============================================================
#else	// !WIN32
// =============================================================

#define EXPORT32

#define	dv_fmemcpy	_fmemcpy
#define	dv_fmemset	_fmemset

#define	MLPTR	FAR *			// Obsolete 16-bit Seg:Off - 240bit addr
#define	PWORD	WORD MLPTR
#define	PINT8	BYTE huge *
#define	MEXPORTS	FAR PASCAL _export
#define	MLIBCONV	FAR PASCAL
#define	MLIBCALL	MLIBCONV	__loadds
#define	MFILEPTR	FILE *

#ifdef	_INC_WINDOWS
#define	SetWindowExtra( a, b, c )	SetWindowWord( a, b, c )
#define	GetWindowExtra( a, b )		GetWindowWord( a, b )
#endif	// _INC_WINDOWS

// =============================================================
#endif	// WIN32 y/n

#endif	// !RC_INVOKED
#endif	// _mwin32_h

// eof Win32.h - Pointers and conventions ...
