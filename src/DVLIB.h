
// DvLib.h
#ifndef	_DvLib_h
#define	_DvLib_h

// Set the NAME of the Dynaically Linked Library
// =============================================
//#ifdef	LDYNALIB
// Since we are DYNAMICALLY loading the LIBRARY, we MUST supply
// the NAME of the LIBRARY to LOAD...
// This in turn depends upon the basic 16/32 support.
// =================================================
#ifdef	Dv16_App
// ================================================
// 16-Bit SUPPORT
// ==============
#ifndef	NDEBUG
// **************************************************************
// Very EXPLICIT Name
//#define	DEF_JPEG_LIB	"D:\\TEMP\\DV\\DV16\\WJPEG16.DLL"
#define	DEF_JPEG_LIB	"WJPEG16.DLL"
// **************************************************************
#else	// NDEBUG
// **************************************************************
#define	DEF_JPEG_LIB	"WJPEG16.DLL"
// **************************************************************
#endif	// NDEBUG y/n

// ================================================
#else	// !Dv16_App
// ================================================

#ifdef	WIN32
// ==================================

#ifndef	NDEBUG
// **************************************************************
//#define	DEF_JPEG_LIB	"D:\\WORK\\T4\\DV32\\WJPEG32\\Debug\\WJPEG32.DLL"
#define	DEF_JPEG_LIB	"WJPEG32.DLL"


// **************************************************************
#else	// NDEBUG
// **************************************************************

#define	DEF_JPEG_LIB	"WJPEG32.DLL"

// **************************************************************
#endif	// NDEBUG y/n

// ==================================
#else	// !WIN32
// We must load a GENERIC DLL - ie a 16-bit one in this case
//#define	DEF_JPEG_LIB	"WJPEG.DLL"
// ==================================
#define	DEF_JPEG_LIB	"WJPEG16.DLL"
// ==================================
#endif	// WIN32 y/n

// ================================================
#endif	// Dv16_App y/n

//#endif	// LDYNALIB

#endif	// _DvLib_h
// eof - DvLib.h
