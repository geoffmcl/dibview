


// DvInfo.c

#include	"Dv.h"

#undef		DIAGINFO
#define		TBCOLOR			RGB( 255, 255,   0 )
#define		DLGCOLOR		RGB( 255,   0,   0 )
#define		TXTCOLOR		RGB(   0,   0,   0 )

extern	BOOL CenterWindow(HWND hwndChild, HWND hwndParent);

void	InfoInit( void );
void	InfoDestroy( void );
DWORD	PutInfo( LPSTR lpt );
DWORD	KillInfo( void );

HWND	ghInfo = 0;
DWORD	dwInfoCnt = 0;
HBRUSH	ghBkBrush = 0;
HBRUSH	ghDlgBrush = 0;

BOOL	InitInfo( HWND hDlg, LPARAM lParam )
{
	BOOL	ret = TRUE;
	CenterWindow( hDlg, ghWndMDIClient );

	if( lParam )
		SetDlgItemText( hDlg, IDC_INFOTEXT, (LPSTR)lParam );

//	SetFocus( hDlg );
	return ret;
}

BOOL	TextInfo( HWND hDlg, LPARAM lParam )
{
	BOOL	ret = TRUE;
	if( lParam )
		SetDlgItemText( hDlg, IDC_INFOTEXT, (LPSTR)lParam );
	return ret;
}

BOOL MEXPORTS IDD_INFODLGPROC( HWND hDlg, UINT msg, WPARAM wParam,
						 LPARAM lParam )
{
	BOOL	ret = FALSE;

	switch( msg )
	{

	case WM_INITDIALOG:
		ret = InitInfo( hDlg, lParam );
		break;

	case MYWM_ADDINFO:
		ret = TextInfo( hDlg, lParam );
		break;

	case WM_CTLCOLORSTATIC:
		if( ghBkBrush == 0 )
			InfoInit();
		if( wParam )
		{
			SetBkColor( (HDC)wParam, TBCOLOR );
			SetTextColor( (HDC)wParam, TXTCOLOR );
		}
		ret = ghBkBrush ? TRUE : FALSE;
		break;

	case WM_CTLCOLORDLG:
		if( ghDlgBrush == 0 )
			InfoInit();
		ret = ghDlgBrush ? TRUE : FALSE;
		break;
	}

#ifdef	DIAGINFO
	{
		LPSTR lpd = GetTmp1();
		wsprintf( lpd,
			"Dlg0x%x: Msg 0x%x wP 0x%x lP 0x%x"MEOR,
			hDlg,
			msg,
			wParam,
			lParam );
		DO(lpd);
	}
#endif	// DIAGINFO
	return ret;
}


void	CheckInfoMsgs( void )
{
	MSG	msg;
	if( ghInfo )
	{
		while( PeekMessage( &msg, ghInfo,
			0, 0,	// Range
			PM_REMOVE | PM_NOYIELD ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}
}


BOOL	SetInfoText( LPSTR lpt )
{
	BOOL	flg = FALSE;
	if( lpt && *lpt && ghInfo && dwInfoCnt )
	{
		SendMessage( ghInfo, MYWM_ADDINFO, 0, (LPARAM)lpt );
		SetFocus( ghInfo );
		CheckInfoMsgs();
		flg = TRUE;
	}
	return flg;
}

DWORD	PutInfo( LPSTR lpt )
{
	BOOL	flg = FALSE;
	if( lpt && *lpt )
	{
		if( ghInfo && dwInfoCnt )
		{
			dwInfoCnt++;
			SetInfoText( lpt );
		}
		else
		{
			ghInfo = CreateDialogParam( ghDvInst,	// Out instance handle
				MAKEINTRESOURCE(IDD_INFODLG),
				GetFocus(),
				(DLGPROC)IDD_INFODLGPROC,
				(LPARAM)lpt );
			if(ghInfo)
			{
				dwInfoCnt++;
				ShowWindow( ghInfo, SW_SHOW );
				SetFocus( ghInfo );
				CheckInfoMsgs();
			}
		}
	}
	return dwInfoCnt;
}

DWORD	KillInfo( void )
{
	if( dwInfoCnt )
		dwInfoCnt--;
	if( dwInfoCnt == 0 )
	{
		if( ghInfo )
			DestroyWindow( ghInfo );
		ghInfo = 0;
	}
	return dwInfoCnt;
}

void	InfoInit( void )
{
	if( ghBkBrush == 0 )
		ghBkBrush = CreateSolidBrush( TBCOLOR );// brush color value
	if( ghDlgBrush == 0 )
		ghDlgBrush = CreateSolidBrush( DLGCOLOR );
}
void	InfoDestroy( void )
{
	while( dwInfoCnt && ghInfo )
		KillInfo();
	if( ghBkBrush )
		DeleteObject( ghBkBrush );
	ghBkBrush = 0;
	if( ghDlgBrush )
		DeleteObject( ghDlgBrush );
	ghDlgBrush = 0;
}

// eof - DvInfo.c
