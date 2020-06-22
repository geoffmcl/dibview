
/* =========================================================================

	File:  DVOPT3.C

	Purpose:  Contains the handlers for the "General Options" Dialog box

	Comments:

   History:
	Date     Reason
	15 April 1996	Created                           Geoff R. McLane
 * ======================================================================== */
#include	"dv.h"	/* Single, all inclusing include ... except 'specials' */
//#include	"WJPEG32\WJPEGLIB.h"

//#define	ChkDlg( a, b )	CheckDlgButton( hDlg, a, b )
//#define	IsChk( a )		IsDlgButtonChecked( hDlg, a )
//	{ &fSetQual, &fSQChg, IDO_CQUAL, QP_Default, &fQualNum, QP_DNumb, IDO_CQPERC  },
//typedef	struct tagOPTLIST	{
//	WORD	op_Type;
//	LPINT	op_Flag;
//	LPINT	op_Chg;
//	WORD	op_ID;
//	BOOL	op_Def;
//	LPWORD	op_Num;
//	WORD	op_DNum;
//	WORD	op_NID;
//	LPINT	op_Chgn;
//	LPSTR	opi_Sect;
//	LPSTR	opi_Item;
//	WORD	opi_Type;
//	LPSTR	opi_Deft;
//}OPTLIST;
//typedef	OPTLIST MLPTR LPOPTLIST;

extern	BOOL	GetJPEGLib( UINT caller );
extern	LIBCONFIG DVLibConfig;
extern LPWGETLCONFIG	WGetLConfig; /* These are filled in, when the Library */
extern LPWSETLCONFIG	WSetLConfig;	/* has been successfully found and loaded */
extern BOOL	fNoFErr;
extern BOOL	fChgFE;
//extern	WORD	wMaxCols;
//extern	BOOL	fChgMC;
extern	BOOL	fDnAutoLoad;

/* General Parameters */
#define	BT_Default	TRUE
//BOOL	fBe_Tidy = BT_Default;
//BOOL	fChgBT = FALSE;
#define	SI_Default	TRUE
//BOOL	fSavINI = SI_Default;
//BOOL	fChgSI = FALSE;
#define	RL_Default	TRUE
BOOL	fReLoad = RL_Default;
BOOL	fChgRL = FALSE;
#define	SF_Default	TRUE
BOOL	fLSafety = SF_Default;
BOOL	fChgSF = FALSE;
#ifdef	TIMEDPUT
// For IDD_ADDTIME (Check Button) and IDD_BAUDCB (Speed string)
#define	BS_Default		8000
#define	AT_Default		FALSE
BOOL	fAddTime = AT_Default;
DWORD	wBitsPSec = BS_Default;	/* Default to 8,000 bits per second */
BOOL	fChgBS = FALSE;
BOOL	fChgAT = FALSE;

#endif	/* TIMEDPUT */

//BOOL	fAutoLoad = FALSE;
BOOL	fPreAutoLoad;
//BOOL	fChgAL = FALSE;
#define	AL_Default		FALSE
#define	SC_Default		20
//extern	DWORD	dwMaxFiles;	/* Remember the last ?? files - See Dib.h */
//BOOL	fChgSC = FALSE;

//BOOL MEXPORTS OPTION3DLG( HWND, unsigned, WORD, LONG );
INT_PTR CALLBACK OPTION3DLG(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);
BOOL	Option3Term( HWND );
BOOL	Option3Init( HWND, LPARAM );
BOOL	Option3Def( HWND );

char	szOption3Dlg[] = "GENOPTIONS";
// General other parameters ... 
//typedef	struct tagOPTLIST
//	dw op_Type ,PINT op_Flag, PINT op_Chg, dw op_ID, f op_Def, PW op_Num
//                                                  dw op_DNum, dw op_NID, PINT	op_Chgn
OPTLIST	Opt3List[] = {
	{ it_General, &gfBe_Tidy, &gfChgBT, IDO_GTIDY, BT_Default, 0, 0, 0, 0, 0, 0, 0, 0 },
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
	{ it_General, &fNoFErr, &fChgFE, IDD_SYSMSG, TRUE, 0, 0, 0, 0, 0, 0, 0, 0 },
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
	{ it_General, &fReLoad, &fChgRL, IDO_RELOAD, RL_Default, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ it_General, &fLSafety,&fChgSF, IDO_SAFETY, SF_Default, 0, 0, 0, 0, 0, 0, 0, 0 },
#ifdef	TIMEDPUT
	{ it_Size2,   &fAddTime,&fChgAT, IDD_ADDTIME, AT_Default, 
		(LPINT) &wBitsPSec, BS_Default, IDD_BAUDCB, &fChgBS, 0, 0, 0, 0 },
#endif	/* TIMEDPUT */
	{ it_Size2,           0,      0,          0,           0,
		(PWORD) &gdwMaxCols, DEF_MAX_COLS, IDD_COLORS, &gfChgMC, 0, 0, 0, 0 },
	{ it_Size2,   &gfAutoLoad, &gfChgAL, IDC_CHECK1, AL_Default,
		(PWORD) &gdwMaxFiles, DEF_MXFILCNT, IDC_EDIT1, &gfChgSC, 0, 0, 0, 0 },
	{ it_Size1,           0,      0,          0,           0,
		(PWORD) &gnPasteNum, 1, IDC_EDIT2, &gbChgPaste, 0, 0, 0, 0 },
//	{ &szGens[0], &szAutoRLLast[0],it_BoolT,  (LPSTR)&gfAutoRLLast, &gfChgRLL, 0, 0 },
	{ it_General, &gfAutoRLLast, &gfChgRLL, IDC_CHECK2, TRUE, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ it_General, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

/* ================================================================
 *
 * Function:   ShowOption3
 * Purpose:    Brings up option3 dialog box
 *		case IDM_OPTION3:
 *    uses szOption3Dlg "GENOPTIONS", proc OPTION3DLG in DvOpt3.c
 * Parms:      hWnd    == Handle to options dialog box's parent.
 * History:
 *   Date      Reason
 * 15 April 96 Created
 *
 * ================================================================ */
#ifdef	WIN32
void ShowOption3( HWND hWnd )
{
   DialogBoxParam( ghDvInst,
                  szOption3Dlg,  // = "GENOPTIONS"
                  hWnd,
                  OPTION3DLG,
                  (LPARAM)NULL);
}

#else	/* !WIN32 */
	
void ShowOption3( HWND hWnd )
{
FARPROC lpProcOpt;
	lpProcOpt = MakeProcInstance( OPTION3DLG, ghDvInst );
   DialogBoxParam( ghDvInst,
                  szOption3Dlg,
                  hWnd,
                  lpProcOpt,
                  (LPARAM)NULL);
   FreeProcInstance( lpProcOpt );
}
#endif	/* WIN32 y/n */

/* =====================================================================
 * OPTION3DLG( ... )
 *		case IDM_OPTION3:
 *    uses szOption3Dlg "GENOPTIONS", proc OPTION3DLG in DvOpt3.c
 * ===================================================================== */
//BOOL MEXPORTS OPTION3DLG (HWND hDlg, unsigned message, 
//	WORD wParam, LONG lParam)
INT_PTR CALLBACK OPTION3DLG(
  HWND hDlg,  // handle to dialog box
  UINT message,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	switch( message )
	{
	case WM_INITDIALOG:
		return Option3Init( hDlg, lParam );

	case WM_COMMAND:
      {
         DWORD cmd = LOWORD(wParam);
			switch( cmd )
			{
			case IDDEFAULT:
				Option3Def( hDlg );
				return TRUE;

         case IDC_RESTORE:
            PutCurOpt( hDlg, &Opt3List[0] );
            return TRUE;

         case IDD_OK:
			   Option3Term( hDlg );	/* Get values, and fall through CANCEL */
				EndDialog( hDlg, TRUE );
				return TRUE;

			case IDD_CANCEL:
			case IDCANCEL:
				EndDialog(hDlg, FALSE);
				return TRUE;
			}
      }
	}
   return FALSE;
}

//	LPINT	op_Flag;
//	LPINT	op_Chg;
//	WORD	op_ID;
//	BOOL	op_Def;
//	PWORD	op_Num;
//	WORD	op_DNum;
//	WORD	op_NID;
BOOL	Option3Def( HWND hDlg )
{
   BOOL	      flg = TRUE;
   LPOPTLIST	lpo = &Opt3List[0];
	PutDefOpt( hDlg, lpo );
   return( flg );
}

/* =====================================================================
 * Option3Init( ... ) = "GENOPTIONS"
 * ===================================================================== */
BOOL	Option3Init( HWND hDlg, LPARAM lParam )
{
   BOOL	      flg;
   LPOPTLIST	lpo;
   HWND        hWnd;    // handle to some controls

	flg = TRUE;	/* Default to all OK ... */
	lpo = &Opt3List[0];	/* OK, now put up all the parameters ... */
	fPreAutoLoad = gfAutoLoad;

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
	if( GetJPEGLib(13) && WGetLConfig )
	{
		if( (*WGetLConfig) ( &DVLibConfig ) )
		{
			goto NoLib;
		}
		else
		{
			fLSafety = DVLibConfig.cf_Safety_Check;
		}			
	}
	else
	{
NoLib:
		if( !fShownNOLIB )
		{
			UINT	ui;
			fShownNOLIB = TRUE;
			//DIBError( ERR_NO_LIB );
			ui = Err_No_Lib();
		}
      /* Should ADD a DISABLE here ... */
	}
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

   {
      // disable as yet UNUSED items
      if( hWnd = GetDlgItem( hDlg, IDD_ADDTIME ) )
      {
	      EnableWindow( hWnd, FALSE );
   		ShowWindow( hWnd, SW_HIDE );
      }
      if( hWnd = GetDlgItem( hDlg, IDD_BAUDCB ) )
      {
	      EnableWindow( hWnd, FALSE );
   		ShowWindow( hWnd, SW_HIDE );
      }
      if( hWnd = GetDlgItem( hDlg, IDC_BPSLABEL ) )
      {
	      EnableWindow( hWnd, FALSE );
   		ShowWindow( hWnd, SW_HIDE );
      }
   }

	if( flg )
	{
		PutCurOpt( hDlg, &Opt3List[0] );
	}

   return( flg );

}

BOOL	Option3Term( HWND hDlg )
{
	BOOL	flg = FALSE;
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
	BOOL    achg;
   LPOPTLIST	lpo;
   BOOL	fSaf;
   LPLIBCONFIG	lpCfg;
	achg = FALSE;
	lpo = &Opt3List[0];
	fSaf = fLSafety;	/* Get Current FLAG */
	lpCfg = &DVLibConfig;
	achg = GetCurOpt( hDlg, lpo );
	if( achg )
	{
      /* If there has been a change ... */
		if( ( fSaf && !fLSafety ) || ( !fSaf && fLSafety ) )
		{	/* And the change is in the LIBRARY item ... */
			lpCfg->cf_Safety_Check = fLSafety;
			if( GetJPEGLib(14) && WSetLConfig )
			{	/* and can load the library .. */
				if( (*WSetLConfig) ( lpCfg ) )
				{	/* Oops, refused by Library ... */
					DIBInfo( INF_LIBERR );
				}
				else
				{
					DIBInfo( INF_LIBSET );
				}			
			}
			else
			{
				DIBError( ERR_NOLCONF );
			}
		}
	}

	if( gfAutoLoad != fPreAutoLoad )
	{
		if( gfAutoLoad )
			fDnAutoLoad = FALSE;
	}

	flg = TRUE;
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

   return( flg );

}

// eof - DvOpt3.c
