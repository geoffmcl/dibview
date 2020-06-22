
// DvFunc.h
// Used in just for File.c and DvLib.c
// but is included in DV.H! (for ALL to see)
// =========================================
#ifndef	_DvFunc_h
#define	_DvFunc_h

#define	MINFILE		16	// Rather arbitrary ***MINIMUM*** File Size ...

#define	USEGFUNC1	// Switch to return HGLOBAL eventually ...

#define	USENEWAPI	// Switch to using get SIZE, then pass GLOBAL handle
//#undef	USENEWAPI	// Switch to using get SIZE, then pass GLOBAL handle

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
#define	USEGFUNC2	// Switch to new JPG functions ...
#define	USEGFUNC3	// Switch to new WDATTOJPG function ...
#define	USEGFUNC4	// Now WDATTOJPG works OK, goto the SPECIALISED BMPTOJPG
#define	USEGFUNC5	// WGIFSIZX Function ...
#define	USEGFUNC6	// EXTENDED GIFSIZE ... 
#define	USEGFUNC7	// TWO NEW FUNCTIONS
#define	USEB2G		// Try a LOCAL FAST BMP to GIF convertion ...

#define	CHKSIZE2	// Do a SIZE check on GIF
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

#undef	NEED_TWO_FILE	// This abandons the WriteJpeg command line i/f

extern	BOOL	GetJPEGLib( UINT caller );
extern	void CloseJPEGLib( void );

extern	BOOL	fShownNOLIB;

#ifdef	ADDWJPEG
#ifdef	LDYNALIB
extern	UINT	Err_No_Lib( void );
#endif	// LDYNALIB
#endif	// ADDWJPEG

#endif	// _DvFunc_h
// eof - DvFunc.h
