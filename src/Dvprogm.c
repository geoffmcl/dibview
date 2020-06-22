

// ===========================================================
// DvProgM.c
//
// Progress Dialog Box, like
//
//PERCENTDLG DIALOG DISCARDABLE  158, 86, 157, 116
//STYLE DS_MODALFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU
//CAPTION "Progress Information"
//FONT 8, "MS Sans Serif"
//BEGIN
//    CTEXT           "",IDD_INFO,12,5,140,24
//    RTEXT           "Estimated:",IDM_FILE1,12,34,58,8
//    CTEXT           "0",IDM_FILE2,75,34,75,8
//    DEFPUSHBUTTON   "Attempt Abort",IDM_ABORT,10,50,140,14
//    CTEXT           "",IDM_FILE3,12,71,140,8
//    LTEXT           "",IDC_EDIT1,12,85,140,25
//END
//
// ===========================================================
#include	"dv.h"
#include	<time.h>

#ifdef	ADDPROGM
// =======================================================
extern	BOOL CenterWindow(HWND hwndChild, HWND hwndParent);

int		iProgCnt = 0;
HWND	hPercent = NULL;        // Handle to info dialog box.
char	szPercentDlg[] = "PERCENTDLG";  // Name of dialog in .RC
FARPROC lpPercentDlg = 0;
WORD	wCurPCT;
BOOL	bUsrAbort = FALSE;
char	szRPctDn[] = "%3u%% Done.";
int		giSetPCTime = 0;
char	gszPctTxt[256];

typedef struct {
	HWND		hFocus;
	UINT		uiBgnPct;
	UINT		uiCurPct;
	UINT		uiNxtPct;
	SYSTEMTIME	sSysTime;
	SYSTEMTIME	sBgnTime;
	time_t		sBT;
	time_t		sCT;

}PROGON;

typedef PROGON * LPPROGON;

PROGON	gsProgOn = { 0 };
LPPROGON	glpProgOn = &gsProgOn;

#define	P		(*glpProgOn)

BOOL	SetPercent( HWND hDlg )
{
	BOOL	b;
	LPSTR	lps;

	lps = &gszPctTxt[0];

	if( *lps == 0 )
	{
		wsprintf( lps, &szRPctDn[0], wCurPCT );
	}

	b = SetDlgItemText( hDlg, IDM_FILE2, lps );

	return b;

}

BOOL	Perc_WM_INITDIALOG( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL	ret;
	LPSTR	lps;

	// BOOL CenterWindow(HWND hwndChild, HWND hwndParent)
    CenterWindow( hDlg, GetWindow( hDlg, GW_OWNER ));
	bUsrAbort = FALSE;
	wCurPCT = 0;

	lps = &gszPctTxt[0];
	wsprintf( lps, &szRPctDn[0], wCurPCT );
	SetPercent( hDlg );	/* initial percent DISPLAY */

	SetFocus( hDlg );

	ret = TRUE;

	return ret;
}

BOOL MEXPORTS PERCENTDLG( HWND hWnd, UINT msg, WPARAM wP,
						 LPARAM lParam )
{
	BOOL	ret = FALSE;
   DWORD cmd = LOWORD(wP);
	switch( msg )
	{

	case WM_INITDIALOG:
		ret = Perc_WM_INITDIALOG( hWnd, cmd, lParam );
		break;

	case WM_COMMAND:
		{
			if( cmd == IDM_ABORT )
			{
				SetDlgItemText( hWnd, IDM_FILE3, "Abort Registered!" );
				bUsrAbort = TRUE;
				ret = TRUE;
			}
		}
		break;

	case MYWM_CHANGEPCT:
		{
			if( cmd != wCurPCT )
			{
				wCurPCT = (WORD)cmd;
				/* This is to CHANGE the percent display */
				SetPercent( hWnd );
			}
			ret = TRUE;
		}
		break;

	case MYWM_ADDINFO:
		{
			if( lParam )
			{
				SetDlgItemText( hWnd, IDD_INFO,
					(LPSTR)lParam );
			}
			ret = TRUE;

		}
		break;

	case MYWM_ADDINFO2:
		{
			if( lParam )
			{
				SetDlgItemText( hWnd, IDC_EDIT1,
					(LPSTR)lParam );
			}
			ret = TRUE;
		}
		break;

	default:
		ret = FALSE;
		break;

	}	// switch by UINT message

	return( ret  );
}

HWND	CreateInstofProg( void )
{
		hPercent = CreateDialog( ghDvInst,	// Out instance handle
			szPercentDlg,
			P.hFocus,		// GetFocus (), 
			(DLGPROC)PERCENTDLG );
		if( hPercent )
		{
			iProgCnt++;
			CheckMessages();
			GetSystemTime( &P.sBgnTime );
			GetSystemTime( &P.sSysTime );
			time( &P.sCT );
			P.sBT = P.sCT;
		}
		return hPercent;
}

void	PutProgM( void )
{
		bUsrAbort = FALSE;
	if( iProgCnt )
	{
		iProgCnt++;
	}
	else
	{
		CreateInstofProg();
	}
}

void	StartProgM( int iPct )
{
	if( iProgCnt )
	{
		PutProgM();

	}
	else
	{
		//wsprintf( lps, &szRPctDn[0], wCurPCT );
		wCurPCT = (WORD)iPct;
		CreateInstofProg();
		SetProgMInfo2( " * Estimated Percent * " );
		P.uiBgnPct = iPct;

	}
}

void	KillProgM( void )
{
	if( iProgCnt )
		iProgCnt--;
	if( iProgCnt == 0 )
	{
		if( hPercent )
			DestroyWindow( hPercent );
		hPercent = 0;			
		if( lpPercentDlg )
			FreeProcInstance( lpPercentDlg );
		lpPercentDlg = 0;
	}
}

void	CheckMessages( void )
{
	MSG	msg;
	if( hPercent )
	{
		while( !bUsrAbort &&
			( PeekMessage( &msg, hPercent,
				WM_MOUSEFIRST, WM_MOUSELAST,	// Range
				PM_REMOVE | PM_NOYIELD ) ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}
}

void	SetChangeMsg( WORD wpct )
{
	LPSTR	lps;

	lps = &gszPctTxt[0];
	wsprintf( lps, &szRPctDn[0], wpct );

}

void	SendChangeMsg( WORD wpct )
{
	SendMessage( hPercent, (UINT)MYWM_CHANGEPCT, wpct, 0L );
}

// ===================================================
// void	SetProgM( DWORD Done, DWORD Total )
//
// Set the PERCENTAGE control to -
//		dn of tot %
//
// ===================================================
void	SetProgM( DWORD dn, DWORD tot )
{
	DWORD	pct;
	WORD	wpct;
//	LPSTR	lps;
	if( hPercent )
	{
		if( tot )
			pct = (dn * 100) / tot;
		else
			pct = 100;
		wpct = LOWORD( pct );
		if( wpct != wCurPCT )
		{
			SetChangeMsg( wpct );
//			lps = &gszPctTxt[0];
//			wsprintf( lps, &szRPctDn[0], wpct );
			SendChangeMsg( wpct );
//			SendMessage( hPercent, (UINT)MYWM_CHANGEPCT, wpct, 0L );
		}
		CheckMessages();
	}
}

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
#define		SECS_IN_MIN		60
#define		SECS_IN_HOUR	SECS_IN_MIN * 60
#define		SECS_IN_DAY		SECS_IN_HOUR * 24
#define		SECS_IN_MTH		SECS_IN_DAY  * 30.417
#define		SECS_IN_YEAR	SECS_IN_DAY  * 365

int	SecsDiff( SYSTEMTIME * lpB, SYSTEMTIME * lpC )
{
	int		s;
	s = (int)(P.sCT - P.sBT);
	return s;
}

int	SecsDiff2( SYSTEMTIME * lpB, SYSTEMTIME * lpC )
{
	int	s = 0;
	double	ds, dsiy;

	ds = (double)0;
	if( lpB->wYear <= lpC->wYear )
	{
		dsiy = (double)SECS_IN_YEAR;
		ds = dsiy * ( lpC->wYear - lpB->wYear );
	}
	else
	{
		return s;
	}

	if( lpB->wMonth < lpC->wMonth )
	{
		dsiy = (double)SECS_IN_YEAR;
		ds = dsiy * ( lpC->wMonth - lpB->wMonth );
	}



	return s;
}

//Members
//wYear
//Specifies the current year. 
//wMonth
//Specifies the current month; January = 1, February = 2, and so
char * pCurMth[] = {
	{ "Unk" },
	{ "Jan" },
	{ "Feb" },
	{ "Mar" },
	{ "Apr" },
	{ "May" },
	{ "Jun" },
	{ "Jul" },
	{ "Aug" },
	{ "Sep" },
	{ "Oct" },
	{ "Nov" },
	{ "Dec" }
};
//on. 
//wDayOfWeek
//Specifies the current day of the week; Sunday = 0, Monday = 1,
//and so on. 
char * pCurDay[] = {
	{ "Sun" },
	{ "Mon" },
	{ "Tue" },
	{ "Wed" },
	{ "Thu" },
	{ "Fri" },
	{ "Sat" },
	{ "Unk" }
};
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
void	SetCurTime( void )
{
	GetSystemTime( &P.sSysTime );
	time( &P.sCT );
}

void	SetProgP( int iPct )
{
	WORD	wpct;
	WORD	ch, cm, cs, bh, bm, bs;
	LPSTR	lpm2;
	if( hPercent )
	{
		wpct = LOWORD( iPct );
		if( wpct != wCurPCT )
		{
			int		iD, iS, iRem, iTot;	/* percentage change in secs */
			lpm2 = &gszMsg[0];
			SetCurTime();
			SetChangeMsg( wpct );
//			GetSystemTime( &P.sSysTime );
//			time( &P.sCT );
//    WORD wMinute; 
//    WORD wSecond; 
//    WORD wMilliseconds; 
			P.uiCurPct = wpct;
//	UINT		uiNxtPct;
			cs = P.sSysTime.wSecond;
			bs = P.sBgnTime.wSecond;
			cm = P.sSysTime.wMinute;
			bm = P.sBgnTime.wMinute;
			ch = P.sSysTime.wHour;
			bh = P.sBgnTime.wHour;

			if( ( ( cs != bs   ) ||
				  ( cm != bm   ) ||
				  ( ch != bh   )        ) &&
				( wpct > wCurPCT        ) &&
				( iD = (wpct - wCurPCT) ) &&
				( iD < 100              ) &&
				( iS = SecsDiff( &P.sBgnTime, &P.sSysTime ) ) &&
				( iS > 0                           ) &&
				( P.uiCurPct > P.uiBgnPct          ) &&
				( iD = ( P.uiCurPct - P.uiBgnPct ) ) &&
				( iD <= 100 ) )
			{

//		SetProgMInfo2( " * Estimated Percent * " );
				iRem = ( iS * 100 ) / iD;
				if( iS < iRem )
				{
					iTot = iS + ( iRem - iS );
					iTot = iRem;
					iRem -= iS;
					wsprintf( lpm2,
						"At %3d s. est. %ds 2go of %d ts",
						iS,
						iRem,
						iTot );
				}
				else
				{
					wsprintf( lpm2,
						" Done %d %% in %d secs.",
						iD,
						iS,
						iRem );
				}
				SetProgMInfo2(lpm2);
			}
			else
			{
				// SetChangeMsg( wpct );
			}

			// SendMessage( hPercent, (UINT)MYWM_CHANGEPCT, wpct, 0L );
			SendChangeMsg( wpct );

		}
		CheckMessages();
	}
}

void	SetProgM1( void )
{
	WORD	wpct;
	if( hPercent )
	{
		if( wCurPCT < 100 )
		{
			wpct = wCurPCT + 1;
			SendMessage( hPercent, (UINT)MYWM_CHANGEPCT, wpct, 0L );
		}
		CheckMessages();
	}
}

void	SetProgMInfo( LPSTR lpi )
{
	if( hPercent )
	{
		if( lpi )
			SendMessage( hPercent, MYWM_ADDINFO, 0, (LPARAM)lpi );
		CheckMessages();
	}
}

void	SetProgMInfo2( LPSTR lpi )
{
	if( hPercent )
	{
		if( lpi )
			SendMessage( hPercent, MYWM_ADDINFO2, 0, (LPARAM)lpi );
		CheckMessages();
	}
}

#endif	/* ADDPROGM */

// eof - DvProgM.c
