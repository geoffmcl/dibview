
/*************************************************************************

      File:  DVOPT1.C

   Purpose:  Routines to implement the display/printing options dialog
             box.

 Functions:  ShowOptions
             STRETCHDLG
             OptionsInit
             EnableXYAxisWindows
             SetHelpText
             GetDlgItemIntOrDef

  Comments:

   History:   Date     Reason

             6/1/91    Created

*************************************************************************/
#include "dv.h"

// Magic numbers used within this modules.

#define MAX_HELP_LINE   80


// Locally used variables -- globals for this module only.

static char          szStretchDlgName[] = "OptionsStretch";
static BOOL          bTmpStretch;         // New stretch value.
static BOOL          bTmpUse31PrnAPIs;    // New 3.1 Print APIs value. 
static BOOL          bTmpUseBanding;      // New banding flag value.
static LPOPTIONSINFO lpInfo;
static int           idDispOption;        // Current display option.
static int           idPrintOption;       // Current print option.
static BOOL          bTmpDefault;
static BOOL          bTmpApply;
static BOOL          bTmpAnim;
static BOOL bTmpAspect; // Current keep aspect option

// Locally used function prototypes.

//BOOL MLIBCALL STRETCHDLG(HWND, unsigned, WORD, LONG);
INT_PTR CALLBACK STRETCHDLG(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);

void EnableXYAxisWindows (HWND hDlg, BOOL bEnable);
BOOL OptionsInit (HWND hDlg, LPOPTIONSINFO lpOptions);

// NOTE: Paired STRING values used. OFF and + 1 is ON string
void SetHelpText (HWND hDlg, DWORD CtrlID);

int  GetDlgItemIntOrDef( HWND hDlg, int nCtrlID, int nDefault );
void OptionsTerm( HWND );


//---------------------------------------------------------------------
//
// Function:   ShowOptions
//
// Purpose:    Brings up options dialog box which modifies the OPTIONSINFO
//             structure passed to this routine.
//             Handle IDM_OPTIONS from the MENU
//
// Parms:      hWnd    == Handle to options dialog box's parent.
//             lpInfo  == Pointer to OPTIONSINFO structure the options
//                         dialog box should edit.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------

void ShowOptions (HWND hWnd, LPOPTIONSINFO lpInfo)
{
	DialogBoxParam( ghDvInst,
		szStretchDlgName,    // = "OPTIONSSTRETCH"
		hWnd,
		STRETCHDLG,	// lpProcStretch,
		(LPARAM) (LPOPTIONSINFO) lpInfo );
}

//---------------------------------------------------------------------
// Function:   STRETCHDLG
//
// Purpose:    Window procedure for the options dialog box.
//             From the IDM_OPTIONS menu item
//             Uses "OPTIONSSTRETCH" dialog template
//
// Parms:      hWnd    == Handle to options dialog box's parent.
//             message == Message being sent to dialog box.
//             wParam  == Depends on message.
//             lParam  == Depends on message.
//
// History:   Date      Reason
//             6/01/91  Created
//             7/01/91  Major restructuring.
//---------------------------------------------------------------------

//BOOL MLIBCALL STRETCHDLG( HWND hDlg,
//						 unsigned message,
//						 WORD wParam,
//						 LONG lParam)
INT_PTR CALLBACK STRETCHDLG(
  HWND hDlg,  // handle to dialog box
  UINT message,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
   DWORD cmd = LOWORD(wParam);
	switch( message )
	{

	case WM_INITDIALOG:
		return OptionsInit (hDlg, (LPOPTIONSINFO) lParam);

	case WM_COMMAND:
		if( ( cmd != IDRB_STRETCHWINDOW ) &&
          ( cmd != IDC_CHKCENTER      ) &&
          ( cmd != IDRB_ANIMATE       ) )
      {
			SetHelpText( hDlg, cmd );  // wParam );
      }
		switch( cmd )
		{
			// If the stretch checkbox is hit, toggle the current
			//  stretch mode, and update the help window.
		case IDRB_STRETCHWINDOW:
			bTmpStretch = !bTmpStretch;
			SetHelpText( hDlg, (DWORD)(cmd + bTmpStretch) );
			return TRUE;

      case IDC_KEEPASPECT:
         bTmpAspect = !bTmpAspect;
         return TRUE;

      case IDC_CHKCENTER:
         gbTmpCenter = !gbTmpCenter;
         SetHelpText( hDlg,
            ( gbTmpCenter ? IDS_STRING3485 : IDS_STRING3484 ) );
         return TRUE;

#ifndef  WIN32
			// If the banding checkbox is hit, toggle the current
			//  banding mode, and update the help window.
		case IDRB_USEBANDING:
			bTmpUseBanding = !bTmpUseBanding;
			SetHelpText( hDlg, (DWORD)(cmd + bTmpUseBanding) );
			return TRUE;

			// If the "Use 3.1 APIs" checkbox is hit, toggle the current
			//  mode, and update the help window.
		case IDRB_USE31APIS:
			bTmpUse31PrnAPIs = !bTmpUse31PrnAPIs;
			SetHelpText( hDlg, (DWORD)(cmd + bTmpUse31PrnAPIs) );
			return TRUE;
#endif   // #ifndef  WIN32

			// If one of the printer option buttons is pressed,
			//  we may want to enable or gray the X/Y Axis
			//  edit controls.  We only perform this action if
			//  it is necessary.  Also, remember which button
			//  is selected.
		case IDRB_SCALE:
		case IDRB_STRETCH:
		case IDRB_BESTFIT:
			if( (cmd == IDRB_SCALE) &&
				(idPrintOption != IDRB_SCALE) )
         {
				EnableXYAxisWindows( hDlg, TRUE );
         }
			else if( (cmd != IDRB_SCALE) &&
				(idPrintOption == IDRB_SCALE) )
         {
				EnableXYAxisWindows( hDlg, FALSE );
         }
			idPrintOption = cmd;
			return TRUE;

			// If one of the display option buttons is pressed,
			//  remember it.
		case IDRB_USEDIBS:
		case IDRB_USEDDBS:
		case IDRB_USESETDIBITS:
			idDispOption = cmd;
			return TRUE;

		case IDRB_ANIMATE:
			bTmpAnim = !bTmpAnim;
			SetHelpText( hDlg, (DWORD)(cmd + bTmpAnim) );
			return TRUE;

		//case IDEF_XAXIS:
			//return TRUE;

		//case IDEF_YAXIS:
			//return TRUE;

		case IDD_DEFAULT:
			bTmpDefault = !bTmpDefault;
			SetHelpText( hDlg, 
            (DWORD)(bTmpDefault ? IDS_HELP_SETDEFAULT : IDS_HELP_NODEFAULT) );
			return TRUE;

		case IDD_RESTORE:
			bTmpApply = !bTmpApply;
			SetHelpText( hDlg, 
            (DWORD)(bTmpApply ? IDS_HELP_APPLYALL : IDS_HELP_NOTAPPLY) );
			return TRUE;

		case IDD_OK:   // was IDOK!
			OptionsTerm( hDlg );
         EndDialog( hDlg, TRUE );
         break;

		case IDD_CANCEL:  // was IDCANCEL
		case IDCANCEL:  // and IDCANCEL
			EndDialog( hDlg, FALSE );
			return TRUE;

		default:
			return FALSE;

		}
	}

	return FALSE;

}


//---------------------------------------------------------------------
//
// Function:   OptionsInit
//
// Purpose:    Called by options dialog for WM_INITDIALOG.  Keeps a pointer
//             to the OPTIONSINFO structure we're modifying.  Sets up
//             globals for current state of certain buttons.  Sets correct
//             buttons.  Grays controls that should be gray.  Initializes
//             edit fields.
//
//             Sets focus to radio button who is "selected" in the
//             first group of radio buttons (i.e. the display options
//             radio button group).  Strange side-effects can happen
//             if this isn't done (e.g. tabbing through the controls
//             will stop on the first control in the dialog box -- even
//             if it isn't checked).  This leads to some subtle bugs...
//
// Parms:      hDlg      == Handle to options dialog box's window.
//             lpOptions == Pointer to OPTIONSINFO structure passed
//                          for our dialog box to change.  All starting
//                          info is saved from this.
//
// History:   Date      Reason
//             9/01/91  Cut out of dialog procedure.
//            11/08/91  Added SetFocus, and return FALSE.
//             26DEC2000 - Remove BANDING and USE W3.1 API
//
//---------------------------------------------------------------------

BOOL OptionsInit (HWND hDlg, LPOPTIONSINFO lpOptions)
{
	char	tbuf1[128];
	char	tbuf2[128];
	char	tbuf3[128];
	HWND	hMDI;
	LPSTR	lpb1, lpb2, lpb3;

	lpb1             = &tbuf1[0];
	lpb2             = &tbuf2[0];
	lpb3             = &tbuf3[0];
	lpInfo           = lpOptions;	/* Establish a LOCAL pointer ... */
	bTmpStretch      = lpOptions->bStretch2W;
   bTmpAspect       = lpOptions->bAspect;
#ifdef  WIN32
   gbTmpCenter      = lpOptions->bCenter;
#else // !WIN32
	bTmpUseBanding   = lpOptions->bPrinterBand;
	bTmpUse31PrnAPIs = lpOptions->bUse31PrintAPIs;
#endif   // !WIN32
	bTmpAnim         = lpOptions->bAnimate;
	// Set these two items always OFF - ie unchecked.
	// FIX980429 - No, use per INI
	bTmpDefault = lpInfo->bSetDefault;
	bTmpApply   = lpInfo->bApplyAll;

	if( lpOptions->bIsAGIF )
	{
		SendDlgItemMessage(hDlg,
			IDRB_ANIMATE,
			BM_SETCHECK,
			bTmpAnim,
			(LPARAM)NULL);
//		SetDlgItemInt( hDlg,
//			IDEF_MSECS,
//			(int)lpOptions->dwMilSecs,
//			FALSE );
	}
	else
	{
		ShowWindow( GetDlgItem( hDlg, IDRB_ANIMATE ), SW_HIDE ) ;
//		ShowWindow( GetDlgItem( hDlg, IDC_MLABEL ), SW_HIDE ) ;
//		ShowWindow( GetDlgItem( hDlg, IDEF_MSECS ), SW_HIDE ) ;
	}

   // always show ANIMATION ms counter
	SetDlgItemInt( hDlg,
			IDEF_MSECS,
			(int)lpOptions->dwMilSecs,
			FALSE );


	if( gnDIBsOpen )
	{

		ShowWindow( GetDlgItem( hDlg, IDD_MESSAGE ), SW_HIDE ) ;

		SendDlgItemMessage( hDlg,
			IDD_DEFAULT,
			BM_SETCHECK,
			bTmpDefault,
			(LPARAM)NULL );
		if( gnDIBsOpen > 1 )
		{
			SendDlgItemMessage( hDlg,
				IDD_RESTORE,
				BM_SETCHECK,
				bTmpApply,
				(LPARAM)NULL );
		}
		else
		{
			// If ONLY 1, hide this since it will be APPLIED
			ShowWindow( GetDlgItem( hDlg, IDD_RESTORE ), SW_HIDE ) ;
		}

	}
	else
	{
		// if NONE open, HIDE the two check buttons
		ShowWindow( GetDlgItem( hDlg, IDD_RESTORE ), SW_HIDE ) ;
		ShowWindow( GetDlgItem( hDlg, IDD_DEFAULT ), SW_HIDE ) ;
		SetWindowText( GetDlgItem( hDlg, IDD_MESSAGE ),
			"Will set default, and will be applied to next window(s) openned" ) ;

	}

	// Set up the correct buttons in the box.  Disable controls
	//  which should be disabled.
	SendDlgItemMessage( hDlg,
		IDRB_STRETCHWINDOW,
		BM_SETCHECK,
		bTmpStretch,
		(LPARAM)NULL );

   //CDI( IDC_KEEPASPECT, gbKeepAspect );
   CDI( IDC_KEEPASPECT, bTmpAspect );

   // CheckDlgButton() - IsDlgButtonChecked()
   CheckDlgButton(hDlg,      // handle to dialog box
                  IDC_CHKCENTER, // button identifier
                  ( gbTmpCenter ? BST_CHECKED : BST_UNCHECKED ) );   // check state

#ifdef   WIN32

   EnableWindow( GetDlgItem( hDlg, IDRB_USEBANDING), FALSE );
   EnableWindow( GetDlgItem( hDlg, IDRB_USE31APIS ), FALSE );
   ShowWindow( GetDlgItem( hDlg, IDRB_USEBANDING), SW_HIDE );
   ShowWindow( GetDlgItem( hDlg, IDRB_USE31APIS ), SW_HIDE );

#else // !#ifdef   WIN32
	SendDlgItemMessage( hDlg,
		IDRB_USEBANDING,
		BM_SETCHECK,
		bTmpUseBanding,
		(LPARAM)NULL );
	
	SendDlgItemMessage( hDlg,
		IDRB_USE31APIS,
		BM_SETCHECK,
		bTmpUse31PrnAPIs,
		(LPARAM)NULL );

#endif   // #ifdef   WIN32 y/n

	idDispOption  = lpOptions->wDispOption;
	idPrintOption = lpOptions->wPrintOption;
	
	// Note the use of CheckRadioButton (as opposed to
	//  SendDlgItemMessage (... BM_SETCHECK...) -- this is used so
	//  that tabbing occurs between _groups_ correctly.  When
	//  a radio button is pressed, tabbing to the group of
	//  radio buttons should set the focus to *that* button.
	CheckRadioButton( hDlg, IDRB_USEDIBS, IDRB_USESETDIBITS,
		idDispOption );

	CheckRadioButton( hDlg, IDRB_BESTFIT, IDRB_SCALE,
		idPrintOption );

	SetDlgItemInt( hDlg, IDEF_XAXIS,
		lpOptions->wXScale,
		FALSE );

	SetDlgItemInt( hDlg, IDEF_YAXIS,
		lpOptions->wYScale,
		FALSE );

	EnableXYAxisWindows( hDlg,
		lpOptions->wPrintOption == PRINT_SCALE );

	lpb3[0] = 0;
	if( ( hMDI = GetCurrentMDIWnd() ) &&
		( GetWindowText( hMDI, lpb2, sizeof( tbuf2 ) ) ) &&
		( GetDlgItemText( hDlg, IDD_TITLELINE, lpb1, sizeof( tbuf1 ) ) ) )
	{
		wsprintf( lpb3, lpb1, lpb2 );
	}
	SetDlgItemText( hDlg, IDD_TITLELINE, lpb3 );

	return( TRUE );	/* T = Set FOCUS  F = Do NOT set focus -1 = Kill?? */

}

void	OptionsTerm( HWND hDlg )
{
	WORD	res;
	int		i;

	if( lpInfo->bIsAGIF )
	{
		lpInfo->bAnimate = bTmpAnim;
		//res = GetDlgItemInt( hDlg, IDEF_MSECS, &i, FALSE );
		//if( i && (res >= 5) && (res <= 5000) )
		//{
		//	lpInfo->dwMilSecs = res;
		//}
	}

   // always get the MS animation timer
	res = GetDlgItemInt( hDlg, IDEF_MSECS, &i, FALSE );
	if( i && (res >= 5) && (res <= 5000) )
	{
      // return a possible NEW ms animation timer
		lpInfo->dwMilSecs = res;
	}

   lpInfo->bStretch2W      = bTmpStretch;
   lpInfo->bAspect         = bTmpAspect;               
#ifdef  WIN32
   lpInfo->bCenter         = gbTmpCenter;
#else // !WIN32
   lpInfo->bPrinterBand    = bTmpUseBanding;
   lpInfo->bUse31PrintAPIs = bTmpUse31PrnAPIs;
#endif   // !WIN32
   lpInfo->wDispOption     = idDispOption;
   lpInfo->wPrintOption    = idPrintOption;
   lpInfo->wXScale         = GetDlgItemIntOrDef (hDlg, 
                                                 IDEF_XAXIS, 
                                                 lpInfo->wXScale);
   lpInfo->wYScale         = GetDlgItemIntOrDef (hDlg,
                                                 IDEF_YAXIS,
                                                 lpInfo->wYScale);
//	lpInfo->bSetDefault  = IsDlgButtonChecked( hDlg, IDD_DEFAULT );
//	lpInfo->bApplyAll    = IsDlgButtonChecked( hDlg, IDD_RESTORE );
	lpInfo->bSetDefault  = bTmpDefault;
	lpInfo->bApplyAll    = bTmpApply;;

}

//---------------------------------------------------------------------
//
// Function:   EnableXYAxisWindows
//
// Purpose:    Enables or gray's the X/Y axis edit controls and their
//             labes.
//
// Parms:      hDlg    == Handle to options dialog box's parent.
//             bEnable == TRUE = Enable them, FALSE = Gray them.
//
// History:   Date      Reason
//             7/01/91  Cut out of options dialog.
//             
//---------------------------------------------------------------------

void EnableXYAxisWindows (HWND hDlg, BOOL bEnable)
{

   EnableWindow (GetDlgItem (hDlg, IDEF_XAXIS),  bEnable);
   EnableWindow (GetDlgItem (hDlg, IDEF_YAXIS),  bEnable);
   EnableWindow (GetDlgItem (hDlg, IDC_XLABEL),  bEnable);
   EnableWindow (GetDlgItem (hDlg, IDC_YLABEL),  bEnable);

}




//---------------------------------------------------------------------
//
// Function:   SetHelpText        
//
// Purpose:    Sets the help static edit control to the appropriate
//             string.  Strings are kept in a string table in the
//             resources (see DIBVIEW.RC).
//
// Parms:      hDlg    == Handle to options dialog box's parent.
//             wCtrlID == ID of control we're setting help for.
//
// History:   Date      Reason
//             7/01/91  Cut out of options dialog.
//             
//---------------------------------------------------------------------
   static char  _s_szHlpBuf[ MAX_HELP_LINE ];

void SetHelpText (HWND hDlg, DWORD dwCtrlID)
{
   int   i;
   PTSTR ps = _s_szHlpBuf;
   *ps = 0;
   i = LoadString( ghDvInst, dwCtrlID, ps, MAX_HELP_LINE );
   //if( i )
   {
      SetDlgItemText( hDlg, IDEF_HELP, ps );
   }

}

//---------------------------------------------------------------------
//
// Function:   GetDlgItemIntOrDef
//
// Purpose:    Returns an integer stored in an edit control of a
//             dialog box.  If the edit control doesn't contain
//             a valid integer value, returns a default value.
//
// Parms:      hDlg     == Handle to dialog box.
//             wCtrlID  == ID of control we're getting the int from.
//             nDefault == Value to return if edit control doesn't have
//                          a valid integer in it.
//
// History:   Date      Reason
//            11/13/91  Created
//            11/14/91  Use string table for error string.
//             
//---------------------------------------------------------------------

int GetDlgItemIntOrDef (HWND hDlg, int nCtrlID, int nDefault)
{
   int  nVal;
   BOOL bTrans;
   char szErr[MAX_HELP_LINE];

   nVal = GetDlgItemInt (hDlg, nCtrlID, &bTrans, FALSE);

   if (bTrans)
      return nVal;
   else
      if (LoadString( ghDvInst, IDS_ERRXYSCALE, szErr, MAX_HELP_LINE))
         MessageBox (hDlg, szErr, NULL, MB_OK);
      else
         MessageBeep (0);

   return nDefault;
}

// eof - DvOpt1.c
