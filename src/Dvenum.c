
// DvEnum.c
#include	"dv.h"
#include	"DvEnum.h"

extern	void	RestWin( HWND, BOOL );

// Defined function codes
// ======================
//#define	ENU_RESTORE			1
//#define	ENU_COUNT			2
//#define	ENU_FINDFILES		3
//#define	ENU_FINDHWND		4	// Check if a CHILD is still alive

//typedef	struct tagENUSTR {  /* es */
//	DWORD	es_code;
//	HGLOBAL	es_hand;
//	char	es_string[MAX_PATH];
//	HWND	es_hwnd;
//	DWORD	es_res1;
//	DWORD	es_res2;
//}ENUSTR;
//typedef ENUSTR FAR * PENUSTR;
//extern	void	EnumAllKids( PENUSTR );


BOOL	CALLBACK ENUMCPROC( HWND hwnd, LPARAM lP )
{
	BOOL	flg;
	DWORD	Act;
	HGLOBAL hg;
	LPWORD lpw;
	PENUSTR	pes;
	HGLOBAL    hDIBInfo;
	LPDIBINFO lpDIBInfo;
	LPSTR	lpf;

	flg = TRUE;
	if( hwnd && (pes = (PENUSTR)lP) )
	{
		Act = pes->es_code;
		if( Act == ENU_RESTORE )
		{
			RestWin( hwnd, FALSE );
		}
		else if( Act == ENU_COUNT )
		{
			if( hg = pes->es_hand )
			{
				if( lpw = (LPWORD) DVGlobalLock( hg ) )
				{
					*lpw += 1;
					DVGlobalUnlock( hg );
				}
			}
		}
		else if( ( Act == ENU_FINDFILES ) ||
			      ( Act == ENU_FINDHWND  ) )
		{
			if( ( hDIBInfo = (HGLOBAL)GetWindowExtra( hwnd, WW_DIB_HINFO ) ) &&
				 ( lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo )         ) )
			{
				lpf = &lpDIBInfo->di_szDibFile[0];  // DIB's filename
				if( Act == ENU_FINDFILES )
				{
					if( lstrcmpi( lpf, &pes->es_string[0] ) == 0 )
					{
						flg = FALSE;	      // STOP enumeration
						pes->es_hwnd = hwnd;	// and RETURN HANDLE
					}
				}
				else if( Act == ENU_FINDHWND )
				{
					if( pes->es_hwnd == hwnd )
					{
						flg = FALSE;
						pes->es_hwnd = 0;	// Signal WINDOW(child) alive

					}
				}

				DVGlobalUnlock( hDIBInfo );
			}
		}
	}

   return( flg );    // return TRUE to continue enumeration, else FALSE to STOP

}

void	EnumAllKids( PENUSTR lP )
{
	// Enumerate ALL the CHILD Windows with the given FUNCTION
	WNDENUMPROC wEnum;
	if( wEnum = (WNDENUMPROC) MakeProcInstance( (FARPROC) ENUMCPROC,
			ghDvInst ) )
	{
		EnumChildWindows( ghWndMDIClient,
			wEnum,
			(LPARAM)lP );
		FreeProcInstance( (FARPROC) wEnum );
	}
}


// End - DvEnum.c
