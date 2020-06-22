
// DvData.c

#include	"dv.h"	// common include
#include <commctrl.h>
#include "DvGlaze.h"

//extern   VOID  GetGlobPrinter( VOID );

#ifndef	NoCrLf

#ifdef	Dv16_App
// Fill in for the WIN32 SYSTEMTIME structure
// ==========================================
#include	<time.h>	// Get the localtime( &tm ) funtion
// =================

typedef struct  _SYSTEMTIME
    {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
    }	SYSTEMTIME;

typedef struct _SYSTEMTIME FAR * LPSYSTEMTIME;

#endif	// Dv16_App

#define	COPYTIME( d, s ) \
{\
	d.wYear = s.wYear;\
	d.wMonth = s.wMonth;\
	d.wDayOfWeek = s.wDayOfWeek;\
	d.wDay = s.wDay;\
	d.wHour = s.wHour;\
	d.wMinute = s.wMinute;\
	d.wSecond = s.wSecond;\
	d.wMilliseconds = s.wMilliseconds;\
}


// TYPES of Date Time string ADDITIONS
// ===================================
#define	NoCrLf			0	// = No Cr/Lf
#define	TrailingCrLf	1	// = Trailing Cr/Lf
#define	TwoCrLf			2	// = Leading AND Trailing Cr/Lf
#define	LeadingCrLf		3	// = Leading Cr/Lf

#endif	// !NoCrLf


#ifdef	WIN32
#ifndef _TM_DEFINED
struct tm {
	int tm_sec;	// seconds after the minute - [0,59]
	int tm_min;	// minutes after the hour - [0,59]
	int tm_hour;	// hours since midnight - [0,23]
	int tm_mday;	// day of the month - [1,31]
	int tm_mon;	// months since January - [0,11]
	int tm_year;	// years since 1900
	int tm_wday;	// days since Sunday - [0,6]
	int tm_yday;	// days since January 1 - [0,365]
	int tm_isdst;	// daylight savings time flag
	};
#define _TM_DEFINED
#endif

#endif	// WIN32

// min. size of buffers1234567890123456789012345678901234567890123456789012345
#define  MINMLBUF    64

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
#ifdef	COMBWLIB	// COMMON CODE WITH LIBRARY 1
TCHAR    gszMLib1[MINMLBUF] = {"Combined code with Library.\r\n"That is no WJPEG32.DLL!"};
TCHAR    gszMLib2[MINMLBUF] = {"And no WJPG32_2.DLL!"};
#else    // !COMBWLIB	// COMMON CODE WITH LIBRARY 1
#  ifdef	LDYNALIB
// program dynamically LOADS library and service offsets
TCHAR    gszMLib1[MINMLBUF] = {"Program dynamic load of WJPEG32.DLL"};
#  else  // !LDYNALIB
// system statically LOADS library and service offsets
TCHAR    gszMLib1[MINMLBUF] = {"System static load of WJPEG32.DLL"};
#  endif // ifdef	LDYNALIB y/n
#  ifdef	LINKLIB6
TCHAR    gszMLib2[MINMLBUF] = {"System static load of WJPG32_2.DLL"};
#  else  // !LINKLIB6
TCHAR    gszMLib2[MINMLBUF] = {"Program dynamic load of WJPG32_2.DLL"};
#  endif // ifdef	LINKLIB6 y/n
#endif   // ifdef	COMBWLIB	y/n // COMMON CODE WITH LIBRARY 1

VOID  SetLibInfo(LPTSTR lpl, BOOL flg)
{
   if (!flg)
   {
	  lstrcpy(gszMLib1, "Dynamic load of WJPEG32.DLL FAILED!");
	  lstrcpy(gszMLib2, "Thus load of WJPG32_.DLL ABORTED!");
   }
}

#else // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
TCHAR    gszMLib1[MINMLBUF] = {"Presently no JPEG support!"};
TCHAR    gszMLib2[MINMLBUF] = {"And no GIF support!"};
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF


//typedef	struct	{
//	char	szBigBuf[2048];
//	char	szTmpBuf[1024+4];
//	char	szTmpBuf1[1024+4];
//	char	szTmpBuf2[1024+4];
//}ONEBUF;

//ONEBUF	gszOneBuf;	// Declare as ONE

// Just some work pads
//char    gszDrive[_MAX_DRIVE+4];	// Drive
//char    gszDir[_MAX_DIR+4];		// Directory
//char    gszFname[_MAX_FNAME+4];	// Filename
//char    gszExt[_MAX_EXT+4];		// Extension
//char	gszDIBFile[MAX_PATH+8];

// Done first RUN
BOOL	fDnOneTime = FALSE;

FIXEDWORK   sFixedWork; // global FIXED data structure

//WRKSTR	WrkStr;		   // Global (eventually ALLOCATED) Work Structure
//PWS      pWS = &WrkStr;
PWS      pWS = 0;

//typedef	struct	tagMYRGBQUAD {
//	RGBQUAD	m_rgbQuad;
//}

RGBQUAD DefaultPal[20] = { 
    { 0,   0,   0,    0 }, 
    { 0x80,0,   0,    0 }, 
    { 0,   0x80,0,    0 }, 
    { 0x80,0x80,0,    0 }, 
    { 0,   0,   0x80, 0 }, 
    { 0x80,0,   0x80, 0 }, 
    { 0,   0x80,0x80, 0 }, 
    { 0xC0,0xC0,0xC0, 0 }, 
 
    { 192, 220, 192,  0 }, 
    { 166, 202, 240,  0 }, 
    { 255, 251, 240,  0 }, 
    { 160, 160, 164,  0 }, 
 
    { 0x80,0x80,0x80, 0 }, 
    { 0xFF,0,   0,    0 }, 
    { 0,   0xFF,0,    0 }, 
    { 0xFF,0xFF,0,    0 }, 
    { 0,   0,   0xFF, 0 }, 
    { 0xFF,0,   0xFF, 0 }, 
    { 0,   0xFF,0xFF, 0 }, 
    { 0xFF,0xFF,0xFF, 0 } 
}; 
 
LPSTR	GetBigBuf( void )
{
	return( &gszOneBuf.szBigBuf[0] );
}

LPSTR	GetTmp1( void )
{
	return( &gszOneBuf.szTmpBuf1[0] );
}

LPSTR	GetTmp2( void )
{
	return( &gszOneBuf.szTmpBuf2[0] );
}

LPSTR	GetTmp3( void )
{
	return( &gszOneBuf.szTmpBuf3[0] );
}

LPSTR	GetFNBuf( void )
{
	return( &gszDIBFile[0] );
}

SYSTEMTIME SysTime; 	// system time structure  
//typedef struct _SYSTEMTIME {  // st  
//    WORD wYear; 
//    WORD wMonth; 
//    WORD wDayOfWeek; 
//    WORD wDay; 
//    WORD wHour; 
//    WORD wMinute; 
//    WORD wSecond; 
//    WORD wMilliseconds; 
////} SYSTEMTIME; 
// FROM EdTime.c

//char	szDateTime[48];
char	szTime[16];	// Time buffer. HH:MM:ss
char	szHrMin[16];	// Just HOUR:MIN
char	szDate[32];	// Date buffer
char	szCrLf[] = MEOR;	// = "\r\n";
char	szSp1[] = " ";
// Current DAY ascii
char	szADay[24];
// Current MONTH ascii
char	szAMonth[24];
BOOL	fAddSecs = FALSE;

SYSTEMTIME	LastTime, BeginTime;
//typedef struct _SYSTEMTIME {  // st  
//    WORD wYear; 
//    WORD wMonth; 
//    WORD wDayOfWeek; 
//    WORD wDay; 
//    WORD wHour; 
//    WORD wMinute; 
//    WORD wSecond; 
//    WORD wMilliseconds; 
//} SYSTEMTIME; 
//Members
//wYear
//Specifies the current year. 
//wMonth
//Specifies the current month; January = 1, February = 2, and so
char szCurMth[] = "UnkJanFebMarAprMayJunJulAugSepOctNovDec";
//on. 
//wDayOfWeek
//Specifies the current day of the week; Sunday = 0, Monday = 1,
//and so on. 
char szCurDay[] = "SunMonTueWedThuFriSatUnk";
//wDay
//Specifies the current day of the month. 
//wHour
//Specifies the current hour. 
//wMinute
//Specifies the current minute. 
//wSecond
//Specifies the current second. 
//wMilliseconds
//Specifies the current millisecond. 
//Remarks
//It is not recommended that you add and subtract values from the
//SYSTEMTIME structure to obtain relative times. Instead, you
//should 
//�	Convert the SYSTEMTIME structure to a FILETIME structure.
//�	Copy the resulting FILETIME structure to a LARGE_INTEGER
//structure.
//�	Use normal 64-bit arithmetic on the LARGE_INTEGER value.
//See Also
//FILETIME, GetSystemTime, LARGE_INTEGER, SetSystemTime
#ifndef	WIN32
void	GetSystemTime( LPSYSTEMTIME lpst )
{
	time_t	ttime;
	struct tm *	ptm;

	if( ptm = localtime( &ttime ) )
	{
		lpst->wYear = (WORD) ptm->tm_year;
		//int tm_year;	// years since 1900
		lpst->wMonth = (WORD) ptm->tm_mon; 
		//int tm_mon;	// months since January - [0,11]
		lpst->wDayOfWeek = (WORD) ptm->tm_wday; 
		//int tm_wday;	// days since Sunday - [0,6]
		lpst->wDay = (WORD) ptm->tm_mday; 
		//int tm_mday;	// day of the month - [1,31]
		lpst->wHour = (WORD) ptm->tm_hour; 
		//int tm_hour;	// hours since midnight - [0,23]
		lpst->wMinute = (WORD) ptm->tm_min; 
		//int tm_min;	// minutes after the hour - [0,59]
		lpst->wSecond = (WORD) ptm->tm_sec; 
		//int tm_sec;	// seconds after the minute - [0,59]
		lpst->wMilliseconds = 0;
		
		//int tm_yday;	// days since January 1 - [0,365]
		//int tm_isdst;	// daylight savings time flag

	}
}
#endif	// !WIN32

#ifdef	WIN32
void	DVGetCurrentTime( LPSYSTEMTIME lpt )
{
	GetLocalTime( lpt );	// UTC or GMT adjusted per PC W95 setting
}
#else	// !WIN32
void	DVGetCurrentTime( LPSYSTEMTIME lpt )
{
	GetSystemTime( lpt );	// DOS is per CMOS MACHINE SETTING
//	lpt->wHour += 2;
//	if( lpt->wHour >= 24 )
//	{
//		lpt->wHour -= 24;
//		lpt->wDay++;
//	}
}
#endif	// WIN32

void FormatTime( LPSTR lpt, LPSYSTEMTIME lpst )
{
	WORD	wCurHr;
	LPSTR	lpampm;
	LPSTR	lptime;

	lptime = lpt;
	if( ((wCurHr = lpst->wHour) == 0 ) ||
		( wCurHr < 12 ) )
	{
		lpampm = "AM";
	}
	else
	{
		if( lpst->wHour > 12 )
			wCurHr = (lpst->wHour - 12);
		else
			wCurHr = lpst->wHour;	// Keep the 12
		lpampm = "PM";
	}

	sprintf( lptime,
		"%2d:%02d:%02d %s",
		(wCurHr & 0xffff),
		lpst->wMinute,
		lpst->wSecond,
		lpampm );

	// And setup 24-Hour time
	lptime = &szHrMin[0];
	sprintf( lptime,
			"%2d:%02d",
			lpst->wHour,
			lpst->wMinute );

}

LPSTR SetszHrMin( void )
{
	LPSTR	lphm = 0;
	SYSTEMTIME  sysTime;

	lphm = &szHrMin[0];
	//GetLocalTime(&sysTime);
	//GetSystemTime( &sysTime );
	DVGetCurrentTime( &sysTime );

	if(( fAddSecs                                  ) ||
		( sysTime.wMinute    != LastTime.wMinute    ) ||
		( sysTime.wHour      != LastTime.wHour      ) ||
		( sysTime.wDay       != LastTime.wDay       ) ||
		( sysTime.wMonth     != LastTime.wMonth     ) ||
		( sysTime.wYear      != LastTime.wYear      ) ||
		( sysTime.wDayOfWeek != LastTime.wDayOfWeek ) )
	{
		sprintf( lphm,
			"%2d:%02d",
			sysTime.wHour,
			sysTime.wMinute );
		LastTime = sysTime;
	}
	return( lphm );
}

BOOL SetszTime( void )
{
	BOOL	flg = FALSE;
	SYSTEMTIME  sysTime;
	LPSTR	lpt;

	//GetLocalTime(&sysTime);
	// Get SYSTEM TIME
	//GetSystemTime( &sysTime );
	DVGetCurrentTime( &sysTime );
	lpt = &szTime[0];
	FormatTime( lpt, &sysTime );

	if(( fAddSecs                                  ) ||
		( !fDnOneTime                               ) ||
		( sysTime.wSecond    != LastTime.wSecond    ) ||
		( sysTime.wMinute    != LastTime.wMinute    ) ||
		( sysTime.wHour      != LastTime.wHour      ) ||
		( sysTime.wDay       != LastTime.wDay       ) ||
		( sysTime.wMonth     != LastTime.wMonth     ) ||
		( sysTime.wYear      != LastTime.wYear      ) ||
		( sysTime.wDayOfWeek != LastTime.wDayOfWeek ) )
	{
		LastTime = sysTime;
		flg = TRUE;
		fDnOneTime = TRUE;
	}
	return( flg );
}

LPSTR	PtrDay( int i )
{
	int		j, k;
	LPSTR	lpDays, lpRet;

	k = i;
	lpDays = &szCurDay[0];
	j = (int)strlen( lpDays ) / 3;
	if( k > 7 )
		k = 7;	// Unknown ends list

	lpDays = &szCurDay[k*3];	// Point to string of 3's
	lpRet = &szADay[0];
	for( j = 0; j < 3; j++ )
	{
		lpRet[j] = lpDays[j];
	}
	lpRet[j] = 0;
	return lpRet;
}

LPSTR	PtrMonth( int i )
{
	int		j, k;
	LPSTR	lpMths, lpRet;

	k = i;
	lpMths = &szCurMth[0];
	j = lstrlen( lpMths ) / 3;
	if( k > 12 )  // FIX20001201 - Changed to > 12, NOT >= 12
		k = 0;		// Unknown begins list

	lpMths = &szCurMth[k*3];	// Point to string of 3's
	lpRet = &szAMonth[0];
	for( j = 0; j < 3; j++ )
	{
		lpRet[j] = lpMths[j];
	}
	lpRet[j] = 0;
	return lpRet;
}

BOOL SetszDate( void )
{
	BOOL	flg = FALSE;
	SYSTEMTIME  sysTime;
	LPSTR	lpd;
	char	buf[64+4];

	lpd = &szDate[0];
	//GetSystemTime( &sysTime );
	DVGetCurrentTime( &sysTime );
	SetszTime();

	// Copy the TIME
	COPYTIME( sysTime, LastTime );
	//sysTime = LastTime;
	//GetLocalTime(&sysTime);

	//GetSystemTime( &sysTime );
	DVGetCurrentTime( &sysTime );

	if( sysTime.wDayOfWeek > 7 )
		sysTime.wDayOfWeek = 7;
	if( sysTime.wMonth > 12 )
		sysTime.wMonth = 0;

	sprintf( &buf[0],
		"%s., %s %2d, %4d",
		PtrDay(sysTime.wDayOfWeek),	//	Cur.Day. MonTue...etc
		PtrMonth(sysTime.wMonth),	//  Cur.Mth. JanFebMar...etc
		sysTime.wDay,
		sysTime.wYear );

	if( strcmp( &buf[0], lpd ) )
	{
		flg = TRUE;
		strcpy( lpd, &buf[0] );
	}
	return( flg );
}

// Types
// NoCrLf		=	0 = No Cr/Lf
// TrailingCrLf	=	1 = Trailing Cr/Lf
// TwoCrLf		=	2 = Leading AND Trailing Cr/Lf
// LeadingCrLf	=	3 = Leading Cr/Lf
#define		MXDT	40
LPSTR GetTimeStr( int Typ )
{
	LPSTR	lpt;
	static	char	_szdt[MXDT*2];
	static	int		_idt = 0;

//	lpt = &gszDateTime[0];
	if( _idt )
	{
		_idt = 0;
		lpt = &_szdt[MXDT];
	}
	else
	{
		_idt++;
		lpt = &_szdt[0];
	}

	SetszDate();

	*lpt = 0;
	// Maybe START with Cr/Lf???
	if(( Typ == TwoCrLf     ) ||
		( Typ == LeadingCrLf ) )
		strcpy( lpt, &szCrLf[0] );

	// Put in SHORT Day, Date
	strcat( lpt, &szDate[0] );
	// Add a space
	strcat( lpt, &szSp1[0] );

	// Decided ADD seconds
	if( fAddSecs )
		strcat( lpt, &szTime[0] );
	else
		strcat( lpt, &szHrMin[0] );

	// Maybe ADD Cr/Lf after
	if(( Typ == TrailingCrLf ) ||
		( Typ == TwoCrLf      ) )
		strcat( lpt, &szCrLf[0] );

	// And return POINTER to (static) STRING
	return( lpt );

}

// This is the EVOLUTION of DT3, I used in DOS for a long time
// ============================
//#define	NoCrLf			0	// = No Cr/Lf
//#define	TrailingCrLf	1	// = Trailing Cr/Lf
//#define	TwoCrLf			2	// = Leading AND Trailing Cr/Lf
//#define	LeadingCrLf		3	// = Leading Cr/Lf
LPSTR	GetDT4( int Typ )
{
	GetSystemTime( &SysTime );
	// For FILE Use - Usually ADD (terminator or 2)
	// ============================================
	return( GetTimeStr( Typ ) );
}

LPSTR	GetDT4s( int Typ )
{
	LPSTR	lpt;
	BOOL	fSecs;

	fSecs = fAddSecs;
	fAddSecs = TRUE;	// Set to ADD seconds
	GetSystemTime( &SysTime );
	// For FILE Use - Usually ADD (terminator or 2)
	// ============================================
	lpt = GetTimeStr( Typ );	// return 1 of 2 static strings
	fAddSecs = fSecs;

	return lpt;
}

BOOL  InitWrkStr( HINSTANCE hInst )
{
	PTSTR ptmp;
	pWS = (PWS)DVGAlloc( "WrkStr", GPTR, sizeof(WRKSTR) );
	if(!pWS)
		return FALSE;

	ZeroMemory(pWS, sizeof(WRKSTR)); // or memset( &WrkStr, 0, sizeof(WRKSTR) );

	CoInitialize(NULL);

	//required to use the common controls
	InitCommonControls();

	InitTimers();

	GetModuleFileName( hInst, &gszRunTime[0], 256 ); // Runtime directory (for *.CLR files)

	gfBe_Tidy   = BT_Default;
	gfSavINI    = SI_Default;

	giTwipsPerInch = DEF_TPI;	//  = 1440;

	giCurColor  = COLOR_SCALE_RED;

	gnPasteNum = 1;		// Init numbering
	//lstrcpy( &gszRPTit[0], "TEMPB%03d.BMP" ); // now see gsaRPFmr[]

	// Set initial options
	gfStretch        = OPT_DEF_STRETCH;
//#ifdef  WIN32
	gfPrtCenter      = OPT_DEF_CENTER;
//#else // !WIN32
#ifndef  WIN32
	gfPrinterBand    = OPT_DEF_BANDING;
	gfUse31PrintAPIs = OPT_DEF_USE31PRNAPIS;
#endif   // !WIN32
	gwDispOption     = OPT_DEF_DISP;
	gwPrintOption    = OPT_DEF_PRNSIZE;
	gwXScale         = OPT_DEF_PRNSCALEX;
	gwYScale         = OPT_DEF_PRNSCALEY;
	gdwMilSecs       = OPT_DEF_MILSECS;

	ptmp = gszTpBuf1;
	GetAppData(ptmp); // GetModulePath(ptmp);
	lstrcpy( gszDbgBmp, ptmp );
	lstrcpy( gszDefDiag, ptmp );	//"TEMPDIAG.TXT", now TEMPDV32.TXT
	lstrcat( gszDbgBmp, "TEMPD001.BMP" );
	lstrcat( gszDefDiag, DEFDIAGFILE );	//"TEMPDIAG.TXT", now TEMPDV32.TXT

	gdwMaxFiles       = DEF_MXFILCNT;	// Remember the last ?? files - See Dib.h
	gdwHisFiles       = DEF_MXHISCNT;   // max. files in HISTORY list
	gdwFndFiles       = DEF_MXFNDCNT;   // max. files in FIND list
	gdwMskFiles       = DEF_MXMSKCNT;   // max. files in MASK list
	gdwAutFiles       = DEF_MXAUTCNT;   // max. files in AUOT RELOAD list

	gdwMaxCols        = DEF_MAX_COLS;

	giVerbal          = DEF_DBGLEVEL;	// 9;	// Set MAXIMUM Verbosity for DEBUG
	gfDoIter          = DEF_ITER;       // interate into folders

	// TOO EARLY   GetGlobPrinter();
	//   LIST_ENTRY  w_sWL1;     // work list
	InitLList( &gsFileList );
	InitLList( &gsHistList );
	InitLList( &gsFindList );
	InitLList( &gsMaskList );
	InitLList( &gsAutoList );
	InitLList( &gsLoadedList );   // loaded files, after WM_CREATE

	return TRUE;
}
void	FreeWrkStr( void )
{
   PLIST_ENTRY   pHead, pNext;

   pHead = &gsFileList;
   FreeLList( pHead, pNext );

   pHead = &gsHistList;
   FreeLList( pHead, pNext );

   pHead = &gsFindList;
   FreeLList( pHead, pNext );

   pHead = &gsMaskList;
   FreeLList( pHead, pNext );

   pHead = &gsAutoList;
   FreeLList( pHead, pNext );

   pHead = &gsLoadedList;   // loaded files, after WM_CREATE
   FreeLList( pHead, pNext );

   CoUninitialize();

   DVGFree(pWS);
   pWS = 0;

}

// eof - DvData.c
