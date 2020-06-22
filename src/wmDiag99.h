
// WMDiag99.h
#ifndef	_WMDiag99_H
#define	_WMDiag99_H

#if defined(SHOWMSG) || defined(INCLWMCODE)

typedef	void (*DMSG)( char * ptr, ... );

typedef struct tagREGSTR {
	DWORD	rs_Size;
	LPSTR	rs_lpName;	// pointer to NAME to use at beginning
	DMSG	rs_vpOut;
	DWORD	rs_Flag;
}REGSTR;
#define	RSS		( sizeof(REGSTR) )

typedef REGSTR * LPREGSTR;

// like REGSTR	sRegFrame = { RSS, szFH, &DiagMsg, 0 };
// or   REGSTR	sRegChild = { RSS, szCH, &DiagMsg, (DWORD)-1 };
// or   REGSTR	sRegDialog= { RSS, szDH, &DiagMsg, 1 };

extern	HANDLE	RegWMDiagsEx( LPREGSTR lprs );
extern	void	RegWMWindow( HANDLE hand, HWND hwnd, LPSTR lps );

#endif	/* SHOWMSG == ( defined(DIAGWM)||defined(DIAGWM1 or 2 - 5 ) */

#endif	/* _WMDiag99_H */
// eof - WMDiag99.h
