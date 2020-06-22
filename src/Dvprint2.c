
/* **************************************************************
 *
 *
 *
 *     File:  DVPRINT2.C
 *
 *  Purpose:  Routines called to print a DIB.  Uses banding or lets GDI
 *            do the banding.  Works with 3.0 (i.e. via escapes) or 3.1
 *            (i.e. via printing APIs).
 *
 * Functions: DIBPrint
 *            BandDIBToPrinter
 *            PrintABand
 *            DeviceSupportsEscape
 *            TranslatePrintRect
 *            GetPrinterDC
 *            PRINTABORTPROC
 *            PRINTABORTDLG
 *            DoStartDoc
 *            DoSetAbortProc
 *            DoStartPage
 *            DoEndPage
 *            DoEndDoc
 *            FindGDIFunction
 *            ShowPrintError
 *
 * Comments:  
 *
 *  History:   Date      Reason
 *            6/ 1/91    Created
 *
 ***************************************************************** */
#include "dv.h"	/* Single inclusive include, except for 'specials' */
#include "DvPrint.h"	/* ONLY included here ... */

#ifndef  NDEBUG
extern   LPTSTR	RetWMStg( UINT uMsg );
#endif   // !NDEBUG

extern	BOOL CenterWindow(HWND hwndChild, HWND hwndParent);
// NOTE: Also see DvPrint.c
// ========================

// Globals for printing.
BOOL gbUseEscapes = TRUE;        // Use Escape() or 3.1 printer APIs?
char szPrintDlg[] = "PRINTDLG";  // Name of Print dialog from .RC

// Macros
#define ChangePP(nPct)	if(ghDlgAbort) SendMessage( ghDlgAbort, MYWM_CHANGEPCT, (WPARAM)nPct, 0L )

// Function prototypes.
#ifdef	WIN32
// #define EXPORT32 __declspec(dllexport)

//EXPORT32
//BOOL MLIBCALL PRINTABORTPROC (HDC hDC, int code);
BOOL CALLBACK PRINTABORTPROC( HDC hdc,        // handle to DC
                              int iError );   // error value
//EXPORT32
//int  MLIBCALL PRINTABORTDLG  (HWND hWnd, 
//                            UINT msg, 
//                                WPARAM wParam, 
//                               LPARAM lParam);

INT_PTR CALLBACK PRINTABORTDLG(
  HWND   hDlg,  // handle to dialog box
  UINT   uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam ); // second message parameter

#else	// !WIN#@

BOOL MLIBCALL PRINTABORTPROC (HDC hDC, int code);
int  MLIBCALL PRINTABORTDLG  (HWND hWnd, 
                            UINT msg, 
                                WPARAM wParam, 
                                LPARAM lParam);
#endif	// WIN#@

// *** NOT USED IN WIN32 ***
// *********************************************************************
#ifndef	WIN32
// *** FOLLOWING ===NOT=== USED IN WIN32 ***
// The following typedef's are for printing functions.  They are defined
//  in PRINT.H (!!!!!!!!!!!!!!!!!!!!!!!?????) included with the 3.1
//  SDK -- as this app is supposed to compile in 3.0, I define them
//  here instead.

typedef struct 
   {
   BOOL bGraphics;            // Band includes graphics
   BOOL bText;                // Band includes text.
   RECT GraphicsRect;         // Rectangle to output graphics into.
   }
BANDINFOSTRUCT;


// LPDOCINFO is now defined in 3.1's WINDOWS.H.  We're compiling under
//  both 3.0 and 3.1.  For now, we'll define our own LPDOCINFO here.
//  This is a LESS than adequate solution!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// *** NOT USED IN WIN32 ***
typedef struct
   {
   short cbSize;
   LPSTR lpszDocName;
   LPSTR lpszOutput;
   }
OURDOCINFO, MLPTR LPOURDOCINFO;



// The following typedef's and string variables are used to link to
//  printing functions on-the-fly in Windows 3.1.  These API are not
//  present in Windows 3.0!  As such, we must use GetModuleHandle() and
//  GetProcAddress() to find and call them.
// *** NOT USED IN WIN32 ***
typedef int (MLIBCONV *LPSTARTDOC)     (HDC, LPOURDOCINFO);
typedef int (MLIBCONV *LPSETABORTPROC) (HDC, FARPROC);
typedef int (MLIBCONV *LPSTARTPAGE)    (HDC);
typedef int (MLIBCONV *LPENDPAGE)      (HDC);
typedef int (MLIBCONV *LPENDDOC)       (HDC);


   // The following strings are used to link to function within
   //  GDI on-the-fly.  These functions were added in 3.1.  We
   //  can't call them directly, because that would not allow this
   //  application to run under Windows 3.0.  We, therefore, use
   //  GetModuleHandle()/GetProcAddress() to link to these functions.
   //  See FindGDIFunction() below.
// *** NOT USED IN WIN32 ***
char szGDIModule[]    = "GDI";         // Module name for GDI in Win31.
char szStartDoc[]     = "StartDoc";    // StartDoc() function in GDI.
char szSetAbortProc[] = "SetAbortProc";// SetAbortProc() function in GDI.
char szStartPage[]    = "StartPage";   // StartPage() function in GDI.
char szEndPage[]      = "EndPage";     // EndPage function in GDI.
char szEndDoc[]       = "EndDoc";      // EndDoc function in GDI.

// However for WIN32, the follwoing functions can be used ..
// ======================================================
//Following are the functions used to print. 
//AbortDoc, DeviceCapabilities, EndDoc, EndPage, Escape
//ExtEscape, SetAbortProc, StartDoc, StartPage
//Following are the functions used to access the print spooler. 
//AbortPrinter, AbortProc, AddForm, AddJob, AddMonitor,
//AddPort, AddPrinter, AddPrinterConnection, AddPrinterDriver,
//AddPrintProcessor, AddPrintProvidor, AdvancedDocumentProperties,
//ClosePrinter, ConfigurePort, ConnectToPrinterDlg, DeleteForm,
//DeleteMonitor, DeletePort, DeletePrinter, DeletePrinterConnection,
//DeletePrinterDriver, DeletePrintProcessor, DeletePrintProvidor,
//DocumentProperties, EndDocPrinter, EndPagePrinter, EnumForms,
//EnumJobs, EnumMonitors, EnumPorts, EnumPrinterDrivers, EnumPrinters,
//EnumPrintProcessorDataTypes, EnumPrintProcessors,
//FindClosePrinterChangeNotification
//FindFirstPrinterChangeNotification
//FindNextPrinterChangeNotification
//FreePrinterNotifyInfo
//GetForm, GetJob, GetPrinter, GetPrinterData, GetPrinterDriver,
//GetPrinterDriverDirectory, GetPrintProcessorDirectory
//OpenPrinter, PrinterMessageBox, PrinterProperties,
//ReadPrinter, ResetPrinter, ScheduleJob, SetForm, SetJob, SetPort,
//SetPrinter, SetPrinterData, StartDocPrinter, StartPagePrinter,
//WaitForPrinterChange, WritePrinter 


// Globals for this module.

//static char szPrintDlg[] = "PrintDLG";  // Name of Print dialog from .RC
//static BOOL gbUseEscapes = TRUE;        // Use Escape() or 3.1 printer APIs?

// Function prototypes.
// *** NOT USED IN WIN32 ***
DWORD    BandDIBToPrinter    (HDC hPrnDC, 
                            LPSTR lpDIBHdr, 
                            LPSTR lpBits, 
                           LPRECT lpPrintRect);
DWORD    PrintABand          (HDC hDC,
                           LPRECT lpRectOut,
                           LPRECT lpRectClip,
                             BOOL fDoText,
                             BOOL fDoGraphics,
                            LPSTR lpDIBHdr,
                            LPSTR lpDIBBits);
HDC     GetPrinterDC        (HWND);
void    TranslatePrintRect  (HDC hDC, 
                          LPRECT lpPrintRect, 
                            DWORD wUnits,
                            DWORD cxDIB,
                            DWORD cyDIB,
                            BOOL bCenter );
DWORD    DoStartDoc          (HDC hPrnDC, LPSTR lpszDocName);
DWORD    DoEndPage           (HDC hPrnDC);
DWORD    DoSetAbortProc      (HDC hPrnDC, 
                          FARPROC lpfnAbortProc);
DWORD    DoStartPage         (HDC hPrnDC);
DWORD    DoEndPage           (HDC hPrnDC);
DWORD    DoEndDoc            (HDC hPrnDC);
FARPROC  FindGDIFunction     (LPSTR lpszFnName);
BOOL     DeviceSupportsEscape(HDC hDC, 
                             int nEscapeCode);


#ifdef	WIN32
static DWORD	dwLastErr;
#endif	// WIN32


#ifdef		OLDPRINT
// =================================================

//---------------------------------------------------------------------
//
// Function:   DIBPrint
//
// Purpose:    This routine drives the printing operation.  It has the code
//             to handle both banding and non-banding printers.  A banding
//             printer can be distinguished by the GetDeviceCaps() API (see
//             the code below.  On banding devices, must repeatedly call the
//             NEXTBAND escape to get the next banding rectangle to print
//             into.  If the device supports the BANDINFO escape, it should
//             be used to determine whether the band "wants" text or
//             graphics (or both).  On non-banding devices, we can ignore
//             all this and call PrintPage() on the entire page!
//
// Parms:      hDIBWnd     == HWND of child owner of DIB (Not used yet!)
//             hDIB        == Handle to global memory with a DIB spec in it.
//                              can be either a Win30 DIB or an OS/2 PM DIB.
//             lpPrintRect == Rect to print (decoded based on next parm)
//             wUnits      == Units lpPrintRect is in (see 
//                              TranslatePrintRect()).
//             dwROP       == Raster operation to use.
//!!!!!!!!!!!!!!!!!!!!dwROP isn't used !!!!!!!!!!!!!!!!!!!!!!
//             fBanding    == TRUE when want to do banding (use NEXTBAND).
//
// Returns:   Encoded error value -- bitwise combination of ERR_PRN_*
//             in PRINT.H.  More than one error can be returned --
//             the application can parse the bits in the DWORD returned,
//             or call ShowPrintError() to display all the errors
//             that ocurred.
//
// History:   Date      Reason
//             6/01/91  Created
//            10/26/91  Added error return codes.
//                      Use DeviceSupportsEscape() instead
//                        of QUERYESCSUPPORT.
//            10/29/91  Added the fUse31APIs flag.
//            11/14/91  Added more error checking.
//                      Added lpDocName as a parameter.
//             6May96   Added HWND of DIB Owner window.
//---------------------------------------------------------------------
// *** NOT USED IN WIN32 ***
DWORD DIBPrint_NOT_USED( HWND   hDIBWnd,
                HANDLE hDIB,
                LPRECT lpPrintRect,
                  DWORD wUnits,
                 DWORD dwROP,
                  BOOL fBanding,
                  BOOL fUse31APIs,
                 LPSTR lpszDocName )
{
	HDC				hPrnDC;
	RECT			rect;
	static FARPROC	lpAbortProc;
	static FARPROC	lpAbortDlg;
	LPSTR			lpDIBHdr, lpBits;
	DWORD			dwErr = ERR_PRN_NONE;

	// Do some setup (like getting pointers to the DIB and its header,
	//  and a printer DC).  Also, set the global gbUseEscapes to force
	//  using printer escapes or the 3.1 printing API.

	if( !hDIB )
		return ERR_PRN_NODIB;
	lpDIBHdr     = DVGlobalLock (hDIB); // LOCK DIB HANDLE
   if( !lpDIBHdr )
		return ERR_PRN_NODIB;

	gbUseEscapes = !fUse31APIs;
	lpBits       = FindDIBBits( lpDIBHdr );

	if( hPrnDC = GetPrinterDC( hDIBWnd ) )
	{
		SetStretchBltMode( hPrnDC, COLORONCOLOR );
      // NOTE: NOT_USED
		TranslatePrintRect( hPrnDC,
			lpPrintRect,
			wUnits,
			DIBWidth (lpDIBHdr),
			DIBHeight (lpDIBHdr),
         FALSE );

		gbAbort      = FALSE;
		// Initialize the abort procedure.  Then STARTDOC.
		lpAbortProc = MakeProcInstance(PRINTABORTPROC, ghDvInst);
		lpAbortDlg  = MakeProcInstance(PRINTABORTDLG,  ghDvInst);
		ghDlgAbort   = CreateDialog( ghDvInst, szPrintDlg,
			GetFocus (), lpAbortDlg);

		if( dwErr |= DoSetAbortProc( hPrnDC, lpAbortProc ) )
			goto PRINTERRORCLEANUP;

		if (dwErr |= DoStartDoc( hPrnDC, lpszDocName ) )
			goto PRINTERRORCLEANUP;

		if( fBanding )
		{
			dwErr |= BandDIBToPrinter (hPrnDC, lpDIBHdr, lpBits, lpPrintRect);
		}
		else
		{
			// When not doing banding, call PrintABand() to dump the
			//  entire page to the printer in one shot (i.e. give it
			//  a band that covers the entire printing rectangle,
			//  and tell it to print graphics and text).

			rect = *lpPrintRect;
			dwErr |= PrintABand (hPrnDC,
				lpPrintRect,
				&rect,
				TRUE,
				TRUE,
				lpDIBHdr,
				lpBits );

			// Non-banding devices need the NEWFRAME or EndPage() call.
			dwErr |= DoEndPage( hPrnDC );
		}


         // End the print operation.  Only send the ENDDOC if
         //   we didn't abort or error.
		if( !gbAbort )
			dwErr |= DoEndDoc (hPrnDC);

		// All done, clean up.

PRINTERRORCLEANUP:

		if( ghDlgAbort )
			DestroyWindow( ghDlgAbort );
		ghDlgAbort = 0;

		FreeProcInstance(lpAbortProc);
		FreeProcInstance(lpAbortDlg);

		DeleteDC( hPrnDC );
	}
	else	/* NO DC returned - Could be just CANCEL by User ... */
	{
		dwErr |= ERR_PRN_NODC;
	}

	DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE

	return( dwErr );

}  // NOT_USED

// =================================================
#endif	// OLDPRINT


//The Escape function allows applications to access capabilities of a particular
// device not directly
//available through GDI. Escape calls made by an application are
//translated and sent to the driver. 
//int Escape(
//    HDC hdc,	// handle to device context 
//    int nEscape,	// escape function 
//    int cbInput,	// number of bytes in input structure 
//    LPCSTR lpvInData,	// pointer to input structure 
//    LPVOID lpvOutData 	// pointer to output structure 
//   );	
//Parameters

//hdc
//Identifies the device context. 

//nEscape
//Specifies the escape function to be performed. This parameter
//must be one of the predefined escape values. Use the ExtEscape
//function if your application defines a private escape value. 

//cbInput
//Specifies the number of bytes of data pointed to by the
//lpvInData parameter. 

//lpvInData
//Points to the input structure required for the specified escape.

//lpvOutData
//Points to the structure that receives output from this escape.
//This parameter should be NULL if no data is returned. 

//Return Values
//If the function succeeds, the return value is greater than zero,
//except with the QUERYESCSUPPORT printer escape, which checks for
//implementation only. If the escape is not implemented, the
//return value is zero. 
//If the function fails, the return value is an error. To get
//extended error information, call GetLastError. 

//Errors
//If the function fails, the return value is one of the following
//values. 
//Value	Meaning
//SP_ERROR	General error. If SP_ERROR is returned, Escape may set
//the last error code to:ERROR_INVALID_PARAMETER
//ERROR_DISK_FULL
//ERROR_NOT_ENOUGH_MEMORY
//ERROR_PRINT_CANCELLED
//SP_OUTOFDISK	Not enough disk space is currently available for
//spooling, and no more space will become available.
//SP_OUTOFMEMORY	Not enough memory is available for spooling.
//SP_USERABORT	The user terminated the job through Windows Print
//Manager.

//Remarks
//The Win32 API provides six new functions that supersede some
//printer escapes: 
//Function	Description
//AbortDoc	Terminates a print job. Supersedes the ABORTDOC escape.
//EndDoc	Ends a print job. Supersedes the ENDDOC escape.
//EndPage	Ends a page. Supersedes the NEWFRAME escape. Unlike
//NEWFRAME, this function is always called after printing a page.
//SetAbortProc	Sets the abort function for a print job. Supersedes
//the SETABORTPROC escape.
//StartDoc	Starts a print job. Supersedes the STARTDOC escape.
//StartPage	Prepares printer driver to receive data.
//The Win32 API provides six new indexes for the GetDeviceCaps
//function that supersede some printer escapes: 
//Index	Description
//PHYSICALWIDTH	For printing devices: the width of the physical
//page, in device units. For example, a printer set to print at
//600 dpi on 8.5"x11" paper has a physical width value of 5100
//device units. Note that the physical page is almost always
//greater than the printable area of the page, and never smaller. 
//PHYSICALHEIGHT	For printing devices: the height of the physical
//page, in device units. For example, a printer set to print at
//600 dpi on 8.5"x11" paper has a physical height value of 6600
//device units. Note that the physical page is almost always
//greater than the printable area of the page, and never smaller.
//PHYSICALOFFSETX	For printing devices: the distance  from the
//left edge of the physical page to the left edge of the printable
//area, in device units. For example, a printer set to print at
//600 dpi on 8.5"x11" paper, that cannot print on the leftmost
//0.25" of paper, has a horizontal physical offset of 150 device 
//units.
//PHYSICALOFFSETY	For printing devices: the distance from the top
//edge of the physical page to the top edge of the printable area,
//in device units. For example, a printer set to print at 600 dpi
//on 8.5"x11" paper, that cannot print on the topmost 0.5" of
//paper, has a vertical physical offset of 300 device units.
//Of the original printer escapes, only the following can be used
//by Win32-based application:
//Escape	Description
//QUERYYESCSUPPORT	Determines whether a particular escape is
//implemented by the device driver.
//Following is a list of the obsolete printer escapes that are
//supported only for compatibility with 16-bit versions of
//Windows: 
//Escape	Description
//ABORTDOC	Stops the current print job and erases everything the
//application has written to the device since the last ENDDOC
//escape.
//ENDDOC	Ends a print job started by the STARTDOC escape.
//GETPHYSPAGESIZE	Retrieves the physical page size and copies it
//to the specified location.
//GETPRINTINGOFFSET	Retrieves the offset from the upper-left
//corner of the physical page where the actual printing or drawing
//begins.
//GETSCALINGFACTOR	Retrieves the scaling factors for the x-axis
//and the y-axis of a printer.
//NEWFRAME	Informs the printer that the application has finished
//writing to a page.
//NEXTBAND	Informs the printer that the application has finished
//writing to a band.
//PASSTHROUGH	Allows the application to send data directly to a
//printer.
//SETABORTPROC	Sets the Abort function for a print job.
//STARTDOC	Informs a printer driver that a new print job is
//starting.
//See Also
//AbortDoc, EndDoc, EndPage, ExtEscape, SetAbortProc, StartDoc,
////StartPage, ResetDC 

//The SetAbortProc function sets the application-defined 
//abort function that allows a print job to be
//canceled during spooling. This function replaces the
//SETABORTPROC printer escape. 
//int SetAbortProc(
//    HDC hdc,	// handle of device context 
//    ABORTPROC lpAbortProc 	// address of abort function  
//   );	
//Parameters
//hdc
//Identifies the device context for the print job. 
//lpAbortProc
//Points to the application-defined abort function. For more
//information about the callback function, see the AbortProc
//callback function. 
//Return Values
//If the function succeeds, the return value is greater than zero.
//If the function fails, the return value is SP_ERROR. To get
//extended error information, call GetLastError. 
//See Also
////AbortDoc, AbortProc 

//#pragma	warning( disable : 4047 )
//#pragma warning( disable : 4024 )


//---------------------------------------------------------------------
//
// Function:   DeviceSupportsEscape
//
// Purpose:    Uses QUERYESCSUPPORT to see if the given device
//             supports the given escape code.
//
// Parms:      hDC         == Device to check if escape is supported on.
//             nEscapeCode == Escape code to check for.
//
// History:   Date      Reason
//            10/26/91  Created
//             
//---------------------------------------------------------------------
// *** NOT USED IN WIN32 ***
BOOL DeviceSupportsEscape (HDC hDC, int nEscapeCode)
{
   return Escape(hDC, QUERYESCSUPPORT, sizeof(int), (LPSTR) &nEscapeCode, NULL);
}


//---------------------------------------------------------------------
//
// Function:   GetPrinterDC
//
// Purpose:    Returns a DC to the currently selected printer.  Returns
//             NULL on error.
//
// Parms:      None
//
// History:   Date      Reason
//             6/01/91  Created
//             6May96   Add passing of HWND of DIB Child Window             
//---------------------------------------------------------------------
// *** NOT USED IN WIN32 ***
HDC GetPrinterDC( HWND hDIBWnd )
{
   PRINTDLG pd;

   pd.lStructSize          = sizeof (pd);
   pd.hwndOwner            = NULL;
   pd.hDevMode             = NULL;
   pd.hDevNames            = NULL;
   pd.hDC                  = NULL;
//   pd.Flags                = PD_RETURNDC | PD_RETURNDEFAULT; // Auto-return
//   pd.Flags                = PD_RETURNDC;
   pd.Flags                = 
	               PD_RETURNDC | PD_HIDEPRINTTOFILE | PD_NOPAGENUMS | PD_NOSELECTION;
   pd.nFromPage            = 0;
   pd.nToPage              = 0;
   pd.nMinPage             = 0;
   pd.nMaxPage             = 0;
   pd.nCopies              = 0;
   pd.hInstance            = NULL;
   pd.lCustData            = (LPARAM)NULL;
   pd.lpfnPrintHook        = NULL;
   pd.lpfnSetupHook        = NULL;
   pd.lpPrintTemplateName  = NULL;
   pd.lpSetupTemplateName  = NULL;
   pd.hPrintTemplate       = NULL;
   pd.hSetupTemplate       = NULL;

   if (PrintDlg (&pd))
      return pd.hDC;
   else
      return NULL;

}	/* GetPrinterDC( HWND ) */


//-------------------- Abort Routines ----------------------------


//---------------------------------------------------------------------
//
// Function:   PRINTABORTPROC
//
// Purpose:    Abort procedure while printing is occurring.  Registered
//             with Windows via the SETABORTPROC escape.  This routine
//             is called regularly by the sytem during a print operation.
//
//             By putting a PeekMessage loop here, multitasking can occur.
//             PeekMessage will yield to other apps if they have messages
//             waiting for them.
//
//             Doesn't bother if the global, gbAbort, is set.  This var
//             is set by PRINTABORTDLG() when a user cancels a print
//             operation.
//
// Parms:      hDC  == DC printing is being done to
//             code == Error code (see docs for SETABORTPROC printer
//                      escape).
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------
//The AbortProc function is an application-defined callback function that is called when a print job is to //be canceled during spooling. 
//BOOL CALLBACK AbortProc(
//    HDC hdc,	// handle to device context  
//    int iError 	// error value 
//   );	
//Parameters
//hdc
//Identifies the device context for the print job. 
//iError
//Specifies whether an error has occurred. This parameter is zero
//if no error has occurred; it is SP_OUTOFDISK if Windows Print
//Manager is currently out of disk space and more disk space will
//become available if the application waits. 
//Return Values
//The callback function should return TRUE to continue the print
//job or FALSE to cancel the print job. 
//Remarks
//An application installs this callback function by calling the
//SetAbortProc function. The AbortProc function is a placeholder
//for the application-defined function name. 
//If the iError parameter is SP_OUTOFDISK, the application need
//not cancel the print job. If it does not cancel the job, it must
//yield to Print Manager by calling the PeekMessage or GetMessage
//function. 
//See Also
////GetMessage, PeekMessage, SetAbortProc 
// *** NOT USED IN WIN32 ***
//BOOL MLIBCALL PRINTABORTPROC(HDC hDC, int code)
BOOL CALLBACK PRINTABORTPROC( HDC hdc,        // handle to DC
                              int iError );   // error value
{
   MSG msg;

   gbAbort |= (iError != 0);

   while( !gbAbort && PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
   {
      if( !IsDialogMessage( ghDlgAbort, &msg ) )
      {
         TranslateMessage( &msg );
         DispatchMessage(  &msg );
      }
   }
   return( !gbAbort );
}



//---------------------------------------------------------------------
//
// Function:   PRINTABORTDLG
//
// Purpose:    Dialog box window procedure for the "cancel" dialog
//             box put up while DIBView is printing.
//
//             Functions sets gbAbort (a global variable) to true
//             if the user aborts the print operation.  Other functions
//             in this module then "do the right thing."
//
//             Also handles MYWM_CHANGEPCT to change % done displayed
//             in dialog box.
//
// Parms:      hWnd    == Handle to this dialog box.
//             message == Message for window.
//             wParam  == Depends on message.
//             lParam  == Depends on message.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------
// *** NOT USED IN WIN32 ***

//int MLIBCALL PRINTABORTDLG(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
INT_PTR CALLBACK PRINTABORTDLG(
  HWND   hWnd,  // handle to dialog box
  UINT   msg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam ) // second message parameter
{
   LPDOCINFO pdi;
#ifndef  NDEBUG
   sprtf( "PADLG: hWnd %#x msg=%#x wP=%#x lP=%#x."MEOR,
      hWnd, msg, wParam, lParam );
#endif   // !NDEBUG
   switch(msg)
      {
      case WM_INITDIALOG:
         {
            CenterWindow( hWnd, ghMainWnd );
            if( ( pdi = (LPDOCINFO)lParam ) &&
                ( pdi->lpszDocName        ) )
            {
               SetDlgItemText( hWnd, IDC_PRTDOC, pdi->lpszDocName );
            }
         }
         SetFocus(hWnd);
         return TRUE;


      case WM_COMMAND:
         gbAbort = TRUE;
         return TRUE;


      case MYWM_CHANGEPCT:
         {
            char szBuf[20];
            wsprintf( szBuf, "%3d%% done", wParam );
            SetDlgItemText( hWnd, IDD_PRNPCT, szBuf );
         }
         return TRUE;
      }

   return FALSE;
}



//---------------------------------------------------------------------
//
// Function:   DoStartDoc
//
// Purpose:    Called at the beginning of printing a document.  Does
//             the "right thing," depending on whether we're using
//             3.0 style printer escapes, or the 3.1 printing API
//             (i.e. either does an Escape(STARTDOC) or StartDoc()).
//
//             Note that it uses FindGDIFunction() to find the address
//             of StartDoc() we can't just put a call to StartDoc()
//             here because we want this .EXE file to be compatible with
//             Windows 3.0 as well as 3.1.  3.0 didn't have a function
//             "StartDoc()!"
//
// Parms:      hPrnDC == DC to printer
//
// Returns:    An error code defined as ERR_PRN_* in PRINT.H:
//                ERR_PRN_NONE (0) if no error.
//
// History:   Date      Reason
//             6/01/91  Created
//            11/14/91  Added error return code.
//             
//---------------------------------------------------------------------
// *** NOT USED IN WIN32 ***
#ifdef	WIN32
//The StartDoc function starts a print job. This function replaces the STARTDOC printer escape. 
//int StartDoc(
//    HDC hdc,	// handle of device context 
//    CONST DOCINFO *lpdi 	// address of structure with file names
//);	
//Parameters
//hdc
//Identifies the device context for the print job. 
//lpdi
//Points to a DOCINFO structure containing the name of the
//document file and the name of the output file.
//Return Values
//If the function succeeds, the return value is greater than zero.
//This value is the print job identifier for the document. 
//If the function fails, the return value is less than or equal to
//zero. To get extended error information, call GetLastError. 
//Remarks
//Applications should call the StartDoc function immediately
//before beginning a print job. Using this function ensures that
//multipage documents are not interspersed with other print jobs. 
//Applications can use the value returned by StartDoc to retrieve
//or set the priority of a print job. Call the GetJob or SetJob
//function and supply this value as one of the required arguments.
//See Also
////DOCINFO, EndDoc, GetJob, SetJob 
//The DOCINFO structure contains the input and output filenames and other information used by the //StartDoc function. 
//typedef struct {    // di  
//    int     cbSize; 
//    LPCTSTR lpszDocName; 
//    LPCTSTR lpszOutput; 
//    LPCTSTR lpszDatatype;   // Windows 95 only; ignored on
//Windows NT 
//    DWORD   fwType;         // Windows 95 only; ignored on
//Windows NT 
//} DOCINFO; 
//Members
//cbSize
//Specifies the size, in bytes, of the structure. 
//lpszDocName
//Points to a null-terminated string that specifies the name of
//the document. 
//lpszOutput
//Points to a null-terminated string that specifies the name of an
//output file. If this pointer is NULL, the output will be sent to
//the device identified by the device context handle that was
//passed to the StartDoc function. 
//lpszDatatype
//Windows 95: Points to a null-terminated string that specifies
//the type of data used to record the print job.
//Windows NT: This member is ignored.
//fwType
//Windows 95: Specifies additional information about the print
//job. Can be zero or DI_APPBANDING if the application will use
//banding. For optimal performance during printing, banding
//applications should specify DI_APPBANDING. 
//Windows NT: This member is ignored.
//See Also
////StartDoc 
// *** NOT USED IN WIN32 ***

DWORD DoStartDoc (HDC hPrnDC, LPSTR lpszDocName)
{
	DWORD	dwret;
	int		i;
	DOCINFO DocInfo;
	DocInfo.cbSize      = sizeof (DocInfo);
	DocInfo.lpszDocName = lpszDocName;
	DocInfo.lpszOutput  = NULL;
	DocInfo.lpszDatatype= NULL;   // Windows 95 only 
	DocInfo.fwType      = 0;      // Windows 95 only
	i = StartDoc( hPrnDC,	// handle of device context 
		&DocInfo ); // address of structure	
	if( i <= 0 )
		dwret = ERR_PRN_STARTDOC;
	else
		dwret = ERR_PRN_NONE;
	return( dwret );
}

#else	// !WIN#@
DWORD DoStartDoc (HDC hPrnDC, LPSTR lpszDocName)
{
   if (gbUseEscapes)
   {
      if (Escape (hPrnDC, STARTDOC, lstrlen (lpszDocName), 
                  lpszDocName, NULL) < 0)
         return ERR_PRN_STARTDOC;
   }
   else
   {
      LPSTARTDOC Win31StartDoc;
      OURDOCINFO DocInfo;

      Win31StartDoc       = (LPSTARTDOC) FindGDIFunction (szStartDoc);
      DocInfo.cbSize      = sizeof (DocInfo);
      DocInfo.lpszDocName = lpszDocName;
      DocInfo.lpszOutput  = NULL;

      if (Win31StartDoc)
         {
         if (Win31StartDoc (hPrnDC, &DocInfo) < 0)
            return ERR_PRN_STARTDOC;
         }
      else
         return ERR_PRN_NOFNSTARTDOC;
   }

   return ERR_PRN_NONE;
}

#endif	// WIN32 y/n


// *** NOT USED IN WIN32 ***
//---------------------------------------------------------------------
//
// Function:   DoStartPage
//
// Purpose:    Called at the beginning of printing a page.  Does
//             the "right thing," depending on whether we're using
//             3.0 style printer escapes, or the 3.1 printing API.
//             Routine does nothing under 3.0 or when using the 3.0
//             Escapes, as there was no equivalent to StartPage()
//             in 3.0.
//
//             Note that it uses FindGDIFunction() to find the address
//             of StartPage() we can't just put a call to StartPage()
//             here because we want this .EXE file to be compatible with
//             Windows 3.0 as well as 3.1.  3.0 didn't have a function
//             "StartPage()!"
//
// Parms:      hPrnDC == DC to printer
//
// Returns:    An error code defined as ERR_PRN_* in PRINT.H:
//                ERR_PRN_NONE (0) if no error.
//
// History:   Date      Reason
//             6/01/91  Created
//            11/14/91  Added error return code.
//             
//---------------------------------------------------------------------
// *** NOT USED IN WIN32 ***
DWORD DoStartPage (HDC hPrnDC)
{
   LPSTARTPAGE Win31StartPage;

   if (!gbUseEscapes)
   {
      Win31StartPage = (LPSTARTPAGE) FindGDIFunction (szStartPage);
      if (Win31StartPage)
      {
         if (!Win31StartPage (hPrnDC))
            return ERR_PRN_STARTPAGE;
      }
      else
         return ERR_PRN_NOFNSTARTPAGE;
   }

   return ERR_PRN_NONE;
}


//---------------------------------------------------------------------
//
// Function:   DoEndPage
//
// Purpose:    Called at the end of printing a page.  Does the
//             "right thing," depending on whether we're using
//             3.0 style printer escapes, or the 3.1 printing API
//             (i.e. either does an Escape(NEWFRAME) or EndPage()).
//
//             Note that it uses FindGDIFunction() to find the address
//             of EndPage() we can't just put a call to EndPage()
//             here because we want this .EXE file to be compatible with
//             Windows 3.0 as well as 3.1.  3.0 didn't have a function
//             "EndPage()!"
//
// Parms:      hPrnDC == DC to printer
//
// Returns:    An error code defined as ERR_PRN_* in PRINT.H:
//                ERR_PRN_NONE (0) if no error.
//
// History:   Date      Reason
//             6/01/91  Created
//            11/14/91  Added error return code.
//             
//---------------------------------------------------------------------
// *** NOT USED IN WIN32 ***
DWORD DoEndPage (HDC hPrnDC)
{
   LPENDPAGE Win31EndPage;

   if (gbUseEscapes)
   {
      if (Escape (hPrnDC, NEWFRAME, NULL, NULL, NULL) < 0)
         return ERR_PRN_NEWFRAME;
   }
   else
   {
      Win31EndPage = (LPENDPAGE) FindGDIFunction (szEndPage);
      if (Win31EndPage)
      {
         if (Win31EndPage (hPrnDC) < 0)
            return ERR_PRN_NEWFRAME;
      }
      else
         return ERR_PRN_NOFNENDPAGE;
   }

   return ERR_PRN_NONE;
}

//---------------------------------------------------------------------
//
// Function:   DoEndDoc
//
// Purpose:    Called at the end of printing a document.  Does
//             the "right thing," depending on whether we're using
//             3.0 style printer escapes, or the 3.1 printing API
//             (i.e. either does an Escape(ENDDOC) or EndDoc()).
//
//             Note that it uses FindGDIFunction() to find the address
//             of EndDoc() we can't just put a call to EndDoc()
//             here because we want this .EXE file to be compatible with
//             Windows 3.0 as well as 3.1.  3.0 didn't have a function
//             "EndDoc()!"
//
// Parms:      hPrnDC == DC to printer
//
// Returns:    An error code defined as ERR_PRN_* in PRINT.H:
//                ERR_PRN_NONE (0) if no error.
//
// History:   Date      Reason
//             6/01/91  Created
//            11/14/91  Added error return code.
//             
//---------------------------------------------------------------------
// *** NOT USED IN WIN32 ***
DWORD DoEndDoc (HDC hPrnDC)
{
   LPENDDOC Win31EndDoc;

   if (gbUseEscapes)
   {
      if (Escape(hPrnDC, ENDDOC, NULL, NULL, NULL) < 0)
         return ERR_PRN_ENDDOC;
   }
   else
   {
      Win31EndDoc = (LPENDDOC) FindGDIFunction (szEndDoc);
      if (Win31EndDoc)
      {
         if (Win31EndDoc (hPrnDC) < 0)
            return ERR_PRN_ENDDOC;
      }
      else
         return ERR_PRN_NOFNENDDOC;
   }

   return ERR_PRN_NONE;
}

//---------------------------------------------------------------------
//
// Function:   FindGDIFunction
//
// Purpose:    Uses GetModuleHandle() and GetProcAddress() to find
//             a given function inside GDI itself.  This is useful
//             to "link" to functions on-the-fly that are only
//             present in Windows 3.1.  If we were to call these
//             functions directly from this EXE file, Windows 3.0 would
//             complain that we are not calling valid functions.
//
// Parms:      lpszFnName == Name of function within GDI.EXE to find
//                            an address for.
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------
// *** NOT USED IN WIN32 ***
FARPROC FindGDIFunction (LPSTR lpszFnName)
{
   HANDLE hGDI;

   hGDI = GetModuleHandle( szGDIModule );
   if( !hGDI )
      return NULL;

   return GetProcAddress( hGDI, lpszFnName );

}

//---------------------------------------------------------------------
//
// Function:   BandDIBToPrinter
//
// Purpose:    Repeatedly call the NEXTBAND escape to get the next
//             banding rectangle to print into.  If the device supports
//             the BANDINFO escape, use it to determine whether the band
//             wants text or graphics (or both).  For each band, call
//             PrintABand() to do the actual output.
//
// Parms:      hPrnDC   == DC to printer.
//             lpDIBHdr == Ptr to DIB header (BITMAPINFOHEADER or 
//                         BITMAPCOREHEADER)
//             lpBits   == Ptr to DIB's bitmap bits.
//
// Returns:    WORD -- One (or more) of the printer errors defined as
//             ERR_PRN_* in PRINT.H.
//
//             ERR_PRN_NONE (0) if no error.
//
// History:   Date      Reason
//            10/26/91  Chopped out of DIBPrint().
//                      Use DeviceSupportsEscape() instead of
//                        QUERYESCSUPPORT.
//            11/14/91  Added Error return codes ERR_PRN_BANDINFO
//                        and errors from PrintABand.
//            01/22/91  Fixed NEXTBAND error return check (was checking
//                        if != 0, now check if > 0).
//            12/26/2000 Try to bring this service into the 21 Centruy - grm.
//
//NEXTBAND Informs the printer that the application has finished
//writing to a band. 
//Band information is not used in Win32 applications
//---------------------------------------------------------------------
// *** NOT USED IN WIN32 ***
DWORD BandDIBToPrinter( HDC    hPrnDC, 
                        LPSTR  lpDIBHdr, 
                        LPSTR  lpBits, 
                        LPRECT lpPrintRect)
{
   BANDINFOSTRUCT bi;
   BOOL           bBandInfoDevice;
   RECT           rect;
   DWORD          dwError = ERR_PRN_NONE;
   int            nEscRet;


      // All printers should support the NEXTBAND escape -- we'll
      //  check here, just in case, though!
   if( !DeviceSupportsEscape (hPrnDC, NEXTBAND) )
      return ERR_PRN_CANTBAND;


      // Check if device supports the BANDINFO escape.  Then setup
      //  the BANDINFOSTRUCT (we'll use the values we put into it
      //  here later even if the device doesn't support BANDINFO).

   bBandInfoDevice = DeviceSupportsEscape( hPrnDC, BANDINFO );
   bi.bGraphics    = TRUE;
   bi.bText        = TRUE;
   bi.GraphicsRect = *lpPrintRect;


      // Enter the banding loop.  For each band, call BANDINFO if
      //  appropriate.  Then call PrintABand() to do the actual
      //  output.  Terminate loop when NEXTBAND returns an empty rect.
   while (((nEscRet = Escape (hPrnDC, NEXTBAND, NULL, NULL, (LPSTR) &rect)) > 0) &&
         !IsRectEmpty (&rect))
   {
      if (bBandInfoDevice)
         if (!Escape (hPrnDC, 
                      BANDINFO, 
                      sizeof (BANDINFOSTRUCT), 
                      (LPSTR) &bi, 
                      (LPSTR) &bi))
            dwError |= ERR_PRN_BANDINFO;

      dwError |= PrintABand (hPrnDC, 
                             lpPrintRect, 
                             &rect,
                             bi.bText,
                             bi.bGraphics,
                             lpDIBHdr,
                             lpBits);
   }

   if (nEscRet <= 0)
      dwError |= ERR_PRN_NEXTBAND;

   return dwError;
}


// *********************************************************************
// *** ABOVE ===NOT=== USED IN WIN32 ***
#endif	// !WIN32

//---------------------------------------------------------------------
//
// Function:   DoSetAbortProc
//
// Purpose:    Called at the beginning of printing a document.  Does
//             the "right thing," depending on whether we're using
//             3.0 style printer escapes, or the 3.1 printing API
//             (i.e. either does an Escape(SETABORTPROC) or SetAbortProc()).
//
//             Note that it uses FindGDIFunction() to find the address
//             of SetAbortProc() we can't just put a call to SetAbortProc()
//             here because we want this .EXE file to be compatible with
//             Windows 3.0 as well as 3.1.  3.0 didn't have a function
//             "SetAbortProc()!"
//
// Parms:      hPrnDC == DC to printer
//
// Returns:    An error code defined as ERR_PRN_* in PRINT.H:
//                ERR_PRN_NONE (0) if no error.
//
// History:   Date      Reason
//             6/01/91  Created
//            11/14/91  Added error return code.
//             
//---------------------------------------------------------------------
#ifdef	WIN32
//static DWORD	dwLastErr;
DWORD DoSetAbortProc( HDC hPrnDC, FARPROC lpfnAbortProc )
{
	DWORD	dwret;
	int		i;
	dwret = ERR_PRN_NONE;
	i = SetAbortProc( hPrnDC, (ABORTPROC)lpfnAbortProc );
	if( (i <= 0) || (i == SP_ERROR) )
	{
		gdwError = GetLastError();
		dwret = ERR_PRN_SETABORTPROC;
	}
	return( dwret );
}

#else	// !WIN32
DWORD DoSetAbortProc( HDC hPrnDC, FARPROC lpfnAbortProc )
{
	LPSETABORTPROC Win31SetAbortProc;

	if( gbUseEscapes )
	{
		if( Escape( hPrnDC, SETABORTPROC, NULL,
			(LPSTR) lpfnAbortProc, NULL) < 0 )
			return ERR_PRN_SETABORTPROC;
	}
	else
	{
		Win31SetAbortProc = (LPSETABORTPROC) FindGDIFunction( szSetAbortProc );
		if( Win31SetAbortProc )
		{
			if( Win31SetAbortProc( hPrnDC, lpfnAbortProc ) < 0 )
				return ERR_PRN_SETABORTPROC;
		}
		else
			return ERR_PRN_NOFNSETABORTPROC;
	}
	return ERR_PRN_NONE;
}

#endif	// WIN32 y/n


#ifdef	WIN32
//-------------------- Abort Routines ----------------------------


//---------------------------------------------------------------------
//
// Function:   PRINTABORTPROC
//
// Purpose:    Abort procedure while printing is occurring.  Registered
//             with Windows via the SETABORTPROC escape.  This routine
//             is called regularly by the sytem during a print operation.
//
//             By putting a PeekMessage loop here, multitasking can occur.
//             PeekMessage will yield to other apps if they have messages
//             waiting for them.
//
//             Doesn't bother if the global, gbAbort, is set.  This var
//             is set by PRINTABORTDLG() when a user cancels a print
//             operation.
//
// Parms:      hDC  == DC printing is being done to
//             code == Error code (see docs for SETABORTPROC printer
//                      escape).
//
// History:   Date      Reason
//             6/01/91  Created
//             
//---------------------------------------------------------------------
//The AbortProc function is an application-defined callback function that is called when a print job is to //be canceled during spooling. 
//BOOL CALLBACK AbortProc(
//    HDC hdc,	// handle to device context  
//    int iError 	// error value 
//   );	
//Parameters
//hdc
//Identifies the device context for the print job. 
//iError
//Specifies whether an error has occurred. This parameter is zero
//if no error has occurred; it is SP_OUTOFDISK if Windows Print
//Manager is currently out of disk space and more disk space will
//become available if the application waits. 
//Return Values
//The callback function should return TRUE to continue the print
//job or FALSE to cancel the print job. 
//Remarks
//An application installs this callback function by calling the
//SetAbortProc function. The AbortProc function is a placeholder
//for the application-defined function name. 
//If the iError parameter is SP_OUTOFDISK, the application need
//not cancel the print job. If it does not cancel the job, it must
//yield to Print Manager by calling the PeekMessage or GetMessage
//function. 
//See Also
////GetMessage, PeekMessage, SetAbortProc 

BOOL CALLBACK PRINTABORTPROC( HDC hdc,        // handle to DC
                              int iError )   // error value
{
   MSG   msg;
   INT   iCnt1, iCnt2;

   iCnt1 = iCnt2 = 0;
//#ifndef  NDEBUG
//   sprtf( "PRINTABORTPROC: Entered with hdc %#x iError=%d."MEOR,
//      hdc, iError );
//#endif   // !NDEBUG

   gbAbort |= (iError != 0);

   while( !gbAbort && PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
   {
      iCnt1++;
#ifndef  NDEBUG
      {
         LPTSTR   lps;
         if( lps = RetWMStg( msg.message ) )
         {
            sprtf( "PAPROC: msg=%s wP=%#x lP=%#x."MEOR,
               lps,
               msg.wParam,
               msg.lParam );
         }
         else
         {
            sprtf( "PAPROC: msg=%#x wP=%#x lP=%#x."MEOR,
               msg.message,
               msg.wParam,
               msg.lParam );
         }
      }
#endif   // !NDEBUG
      if( !IsDialogMessage( ghDlgAbort, &msg ) )
      {
         iCnt2++;
         TranslateMessage (&msg);
         DispatchMessage (&msg);
      }
   }

#ifndef  NDEBUG
   if( iCnt1 )
   {
      sprtf( "PRINTABORTPROC: Exit hdc %#x iError=%d Count 1=%d 2=%d."MEOR,
         hdc, iError, iCnt1, iCnt2 );
   }
#endif   // !NDEBUG

   return( !gbAbort );

}



//---------------------------------------------------------------------
//
// Function:   PRINTABORTDLG
//
// Purpose:    Dialog box window procedure for the "cancel" dialog
//             box put up while DIBView is printing.
//
//             Functions sets gbAbort (a global variable) to true
//             if the user aborts the print operation.  Other functions
//             in this module then "do the right thing."
//
//             Also handles MYWM_CHANGEPCT to change % done displayed
//             in dialog box.
//
// Parms:      hWnd    == Handle to this dialog box.
//             message == Message for window.
//             wParam  == Depends on message.
//             lParam  == Depends on message.
//
// History:   Date      Reason
//             6/01/91  Created
//             Jan 2001 Added centering
//
//---------------------------------------------------------------------

INT_PTR CALLBACK PRINTABORTDLG(
  HWND   hWnd,  // handle to dialog box
  UINT   msg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam ) // second message parameter
{
   LPDOCINFO   pdi;
   TCHAR       szBuf[64];

#ifndef  NDEBUG
//extern   LPTSTR	RetWMStg( UINT uMsg );
   {
      LPTSTR   lps;
      if( lps = RetWMStg( msg ) )
      {
         sprtf( "PADLG: hWnd %#x msg=%s wP=%#x lP=%#x."MEOR,
            hWnd, lps, wParam, lParam );
      }
      else
      {
         sprtf( "PADLG: hWnd %#x msg=%#x wP=%#x lP=%#x."MEOR,
            hWnd, msg, wParam, lParam );
      }
   }
#endif   // !NDEBUG
   switch (msg)
      {
      case WM_INITDIALOG:
         {
            CenterWindow( hWnd, ghMainWnd );
            if( ( pdi = (LPDOCINFO)lParam ) &&
                ( pdi->lpszDocName        ) )
            {
               SetDlgItemText( hWnd, IDC_PRTDOC, pdi->lpszDocName );
            }
         }
         SetFocus(hWnd);
         return TRUE;

	   case WM_CLOSE:	/* 0x0010 */
         EndDialog( hWnd, FALSE );
         return TRUE;

      case WM_COMMAND:
         if( LOWORD(wParam) == IDCANCEL )
         {
            gbAbort = TRUE;            // set an abort
            EndDialog( hWnd, FALSE );  // and remove dialog
            return TRUE;
         }
         return FALSE;

	   //case WM_DESTROY:
      //   ghDlgAbort = 0;
      //   return TRUE;

      case MYWM_CHANGEPCT:
         {
            wsprintf (szBuf, "%3d%% done", wParam);
            SetDlgItemText (hWnd, IDD_PRNPCT, szBuf);
         }
         return TRUE;

      case MYWM_CHANGEJOBS:
         {
            wsprintf(szBuf, "Spooler: %d Jobs", wParam);
            SetDlgItemText(hWnd, IDC_PRTJOBS, szBuf);
         }
         return TRUE;
      }

   return FALSE;
}

#endif	// WIN32

//---------------------------------------------------------------------
//
// Function:   PrintABand
//
// Purpose:    This routine does ALL output to the printer.  It is driven by
//             BandDIBToPrinter().  It is called for both banding and non-
//             banding printing devices.  lpClipRect contains the rectangular
//             area we should do our output into (i.e. we should clip our
//             output to this area).  The flags fDoText and fDoGraphics
//             should be set appropriately (if we want any text output to
//             the rectangle, set fDoText to true).  Normally these flags
//             are returned on banding devices which support the BANDINFO
//             escape.  On non-banding devices, all output goes to the
//             entire page, so this routine is passes a rectangle for
//             the entire page, and fDoText = fDoGraphics = TRUE.
//
//             This routine is also responsible for doing stretching of
//             the DIB.  As such, the lpRectOut parameter points to a
//             rectangle on the printed page where the entire DIB will
//             fit -- the DIB is stretched appropriately to fit in this
//             rectangle.
//
//             After printing a band, updates the print % shown in the
//             abort dialog box.
//
// Parms:      hDC         == DC to do output into.
//             lpRectOut   == Rectangle on DC DIB should fit in.
//             lpRectClip  == Rectangle to output during THIS call.
//             fDoText     == Output text into this rectangle (unused by DIBView)?
//             fDoGraphics == Output graphics into this rectangle?
//             lpDIBHdr    == Pointer to DIB's header (either a
//                              BITMAPINFOHEADER or a BITMAPCOREHEADER)
//             lpDIBBits   == Pointer to the DIB's bitmap bits.
//
// Returns:    One or more of the ERR_PRN_* errors in PRINT.H (or'd
//             together. ERR_PRN_NONE (0) if no error.
//
// History:   Date      Reason
//             6/01/91  Created
//             2000DEC28 NOTE: Banding is NOT used in WIN32 so the
//             lpRectOut = lpRectClip!
//
//---------------------------------------------------------------------

DWORD PrintABand( HDC    hDC,
                  LPRECT lpRectOut,
                  LPRECT lpRectClip,
                  BOOL   fDoText,
                  BOOL   fDoGraphics,
                  LPSTR  lpDIBHdr,
                  LPSTR  lpDIBBits )
{
	int      nxLogPix, nyLogPix;
	RECT     rect;
	double   dblXScaling, dblYScaling;
	DWORD    dwError = ERR_PRN_NONE;
   int      ir;
	if( fDoGraphics )
	{

		nxLogPix = GetDeviceCaps( hDC, LOGPIXELSX );
		nyLogPix = GetDeviceCaps( hDC, LOGPIXELSY );

		dblXScaling = ((double) lpRectOut->right - lpRectOut->left) / 
                     DIBWidth (lpDIBHdr);

		dblYScaling = ((double) lpRectOut->bottom - lpRectOut->top) /
                     DIBHeight (lpDIBHdr);

		// Now we set up a temporary rectangle -- this rectangle
		//  holds the coordinates on the paper where our bitmap
		//  WILL be output.  We can intersect this rectangle with
		//  the lpClipRect to see what we NEED to output to this
		//  band.  Then, we determine the coordinates in the DIB
		//  to which this rectangle corresponds (using dbl?Scaling).
		IntersectRect( &rect, lpRectOut, lpRectClip );

		if( !IsRectEmpty( &rect ) )
		{
			RECT rectIn;
			int  nPct;

			rectIn.left   = (int) ((rect.left - lpRectOut->left) / 
                                 dblXScaling + 0.5);
			rectIn.top    = (int) ((rect.top  - lpRectOut->top) / 
                                 dblYScaling + 0.5);
			rectIn.right  = (int) (rectIn.left + (rect.right  - rect.left) / 
                                 dblXScaling + 0.5);
			rectIn.bottom = (int) (rectIn.top  +  (rect.bottom - rect.top) / 
                                 dblYScaling + 0.5);

         // Could just always call StretchDIBits() below, but
         //  we want to give SetDIBitsToDevice() a work out, too!

			if( ( (rect.right - rect.left) == (rectIn.right - rectIn.left) ) &&
				 ( (rect.bottom - rect.top) == (rectIn.bottom - rectIn.top) ) )
			{
            // If the function succeeds, the return value is the number of scan lines set.
            // If the function fails, the return value is zero. 
				ir = SetDIBitsToDevice( hDC,                       // DestDC
                                    rect.left,                 // DestX
                                    rect.top,                  // DestY
                                    rect.right - rect.left,    // DestWidth
                                    rect.bottom - rect.top,    // DestHeight
                                    rectIn.left,               // SrcX
                                    (int) DIBHeight (lpDIBHdr)-// SrcY
                                       rectIn.top - 
                                       (rectIn.bottom - rectIn.top),
                                    0,                         // nStartScan
                                    (int) DIBHeight (lpDIBHdr),// nNumScans
                                    lpDIBBits,                 // lpBits
                                    (LPBITMAPINFO) lpDIBHdr,   // lpBitInfo
                                    DIB_RGB_COLORS );           // wUsage
            if( ir )
            {
               // success
               sprtf( "SetDIBits: to (%d,%d,%d,%d) from (%d,%d,%d,%d)."MEOR,
                                    rect.left,                 // DestX
                                    rect.top,                  // DestY
                                    rect.right - rect.left,    // DestWidth
                                    rect.bottom - rect.top,    // DestHeight
                                    rectIn.left,               // SrcX
                                    (int) DIBHeight (lpDIBHdr)-// SrcY
                                       rectIn.top - 
                                       (rectIn.bottom - rectIn.top),
                                    0,                         // nStartScan
                                    (int) DIBHeight (lpDIBHdr) ); // nNumScans
            }
            else
            {
               // SET error type
               gdwError = GetLastError();
					dwError |= ERR_PRN_SETDIBITSTODEV;
            }
			}
			else
			{
            // If the function succeeds,
            // the return value is the number of scan lines copied
            // If the function fails, the return value is GDI_ERROR
				ir = StretchDIBits( hDC,                           // DestDC
                                rect.left,                     // DestX
                                rect.top,                      // DestY
                                rect.right - rect.left,        // DestWidth
                                rect.bottom - rect.top,        // DestHeight
                                rectIn.left,                   // SrcX
                                (int) DIBHeight (lpDIBHdr) -   // SrcY
                                   rectIn.top - 
                                   (rectIn.bottom - rectIn.top),
                                rectIn.right - rectIn.left,    // SrcWidth
                                rectIn.bottom - rectIn.top,    // SrcHeight
                                lpDIBBits,                     // lpBits
                                (LPBITMAPINFO) lpDIBHdr,       // lpBitInfo
                                DIB_RGB_COLORS,                // wUsage
                                SRCCOPY );                     // dwROP
            if( ( ir ) &&
                ( ir != GDI_ERROR ) )
            {
               // success
               sprtf( "Stretch to (%d,%d,%d,%d) from (%d,%d,%d,%d) on %dx%d."MEOR,
                                rect.left,                     // DestX
                                rect.top,                      // DestY
                                rect.right - rect.left,        // DestWidth
                                rect.bottom - rect.top,        // DestHeight
                                rectIn.left,                   // SrcX
                                (int) DIBHeight (lpDIBHdr) -   // SrcY
                                   rectIn.top - 
                                   (rectIn.bottom - rectIn.top),
                                rectIn.right - rectIn.left,    // SrcWidth
                                rectIn.bottom - rectIn.top,  // SrcHeight
                                nxLogPix, // = GetDeviceCaps( hDC, LOGPIXELSX );
                                nyLogPix ); // = GetDeviceCaps( hDC, LOGPIXELSY );
            }
            else
            {
               // set error type
               gdwError = GetLastError();
					dwError |= ERR_PRN_STRETCHDIBITS;
            }
			}

			// Change percentage of print shown in abort dialog.
			nPct = MulDiv( rect.bottom, 100, lpRectOut->bottom );
         ChangePP( nPct );

		}
	}

	return dwError;

}


//---------------------------------------------------------------------
//
// Function:   TranslatePrintRect
//
// Purpose:    Given a rectangle and what units that rectangle is in,
//             translate the rectangle to the appropriate value in
//             device units.
//             Only called from DIBPrint()
//
// Parms:      hDC         == DC translation is relative to.
//             lpPrintRect == Pointer to rectangle to SET with translation.
//             wUnits      == Units lpPrintRect is in:
//                              UNITS_INCHES == Units are in inches, stretch
//                                                DIB to this size on page.
//                              UNITS_STRETCHTOPAGE == lpPrintRect doesn't
//                                                matter, stretch DIB to
//                                                fill the entire page.
//                              UNITS_BESTFIT == lpPrintRect doesn't matter,
//                                                stretch DIB as much as
//                                                possible horizontally,
//                                                and preserve its aspect
//                                                ratio vertically.
//                              UNITS_SCALE == lpPrintRect->top is factor to
//                                                stretch vertically.
//                                                lpPrintRect->left is
//                                                factor to stretch horiz.
//                              UNITS_PIXELS == lpPrintRect is in pixels.
//             cxDIB       == DIB's width.
//             cyDIB       == DIB's height.
//
// History:   Date      Reason
//             6/01/91  Created
//          30DEC2000   Changed the BEST FIT and added CENTERING
//
//---------------------------------------------------------------------

void TranslatePrintRect( HDC     hDC, 
                         LPRECT  lpPrintRect, 
                         DWORD   wUnits,    // internal UNIT type
                         DWORD   cxDIB,
                         DWORD   cyDIB,
                         BOOL    bCenter )
{
   int      cxPage, cyPage, cxInch, cyInch;
   RECT     rc;
   LPTSTR   lpt;

   if( !hDC )
      return;

   rc = *lpPrintRect;   // copy the given rectangle (ONLY FOR DEBUG)

   cxPage = GetDeviceCaps( hDC, HORZRES    );   // Width, in pixels, of the device
   cyPage = GetDeviceCaps( hDC, VERTRES    );   // Height, in raster lines, of the device
   cxInch = GetDeviceCaps( hDC, LOGPIXELSX );   // pixels per inch along the device width
   cyInch = GetDeviceCaps( hDC, LOGPIXELSY );   // pixels per inch along the height
   switch (wUnits)
   {

      // lpPrintRect contains units in inches.  Convert to pixels.
      case UNITS_INCHES:
         lpPrintRect->top    *= cyInch;
         lpPrintRect->left   *= cxInch;
         lpPrintRect->bottom *= cyInch;
         lpPrintRect->right  *= cxInch;
         lpt = "INCHES";
         break;


      // lpPrintRect contains no pertinent info -- create a rectangle
      //  which covers the entire printing page.
      case UNITS_STRETCHTOPAGE:
         lpPrintRect->top    = 0;
         lpPrintRect->left   = 0;
         lpPrintRect->bottom = cyPage;
         lpPrintRect->right  = cxPage;
         lpt = "STRETCH";
         break;


      // lpPrintRect contains no pertinent info -- create a rectangle
      //  which preserves the DIB's aspect ratio, and fills the page
      //  horizontally.  NOTE:  Assumes DIB is 1 to 1 aspect ratio,
      //  could use biXPelsPerMeter in a DIB to munge these values
      //  for non 1 to 1 aspect ratio DIBs (I've never seen such
      //  a beast, though)!
      case UNITS_BESTFIT:
         // try expand the x to full width and calculate the new y
         // if the new y does NOT fit the page, expand the y to full
         // height and calculate the new x
         rc.left   = cxPage;
         rc.top    = cyPage;
         // if use cyPage get appropriate cx using cy
         rc.right  = ( cxDIB * cyPage ) / cyDIB;
         // if use cxPage get appropriate cy using cx
         rc.bottom = ( cyDIB * cxPage ) / cxDIB;

         lpPrintRect->top    = 0;
         lpPrintRect->left   = 0;
         //lpPrintRect->bottom = (int)(((double) cyDIB * cyPage * cyInch) /
         //                            ((double) cxDIB * cxInch));
         //lpPrintRect->right  = cxPage;
         if( rc.right <= cxPage )
         {
            lpPrintRect->right  = rc.right;
            lpPrintRect->bottom = cyPage;
         }
         else
         {
            lpPrintRect->right  = cxPage;
            lpPrintRect->bottom = rc.bottom;
         }
         lpt = "BESTFIT";
         break;


      // lpPrintRect's top/left contain multipliers to multiply the
      //  DIB's height/width by.
      case UNITS_SCALE:
         {
            int cxMult, cyMult;

            cxMult              = lpPrintRect->left;
            cyMult              = lpPrintRect->top;
            lpPrintRect->top    = 0;
            lpPrintRect->left   = 0;
            lpPrintRect->bottom = cyDIB * cyMult;
            lpPrintRect->right  = cxDIB * cxMult;
            lpt = "SCALE";
         }
         break;


      // lpPrintRect already contains device units, don't touch it.
      case UNITS_PIXELS:
      default:
         // Don't touch the units...
         lpt = "PIXELS(DEF)";
         break;

   }

   sprtf( "Translated: Rect %s to %s using %s Units."MEOR,
      Rect2Stg( &rc ),
      Rect2Stg( lpPrintRect ),
      lpt );

   if( bCenter )
   {
      int   iW, iH;
      iW = iH = 0;
      if( RECTWIDTH(lpPrintRect) < cxPage )
         iW = (cxPage - RECTWIDTH(lpPrintRect)) / 2;
      if( RECTHEIGHT(lpPrintRect) < cyPage )
         iH = (cyPage - RECTHEIGHT(lpPrintRect)) / 2;
      OffsetRect( lpPrintRect, iW, iH );

      sprtf( "Centered: Using iX=%d and iY=%d to %s"MEOR,
         iW, iH,
         Rect2Stg(lpPrintRect) );

   }
}



//---------------------------------------------------------------------
//
// Function:   ShowPrintError
//
// Purpose:    Decode a printing error and display a message box
//             which describes what the error is.
//
//             Errors are stored in a bitwise fashion, so we 
//             check all the valid error bits in the error, and
//             display a messagebox for each error.
//
// Parms:      hWnd   == Parent for message box which shows error.
//             wError == Error bitfield (see ERR_PRN_* in PRINT.H).
//
// History:   Date      Reason
//            11/14/91  Created
//             
//---------------------------------------------------------------------

void ShowPrintError (HWND hWnd, DWORD dwError)
{
   char szError[100];
   int  i = 0;

   if (dwError == ERR_PRN_NONE)
      {
      if (LoadString( ghDvInst, IDS_PRN_NONE, szError, 100))
         MessageBox (hWnd, szError, NULL, MB_OK);
      return;
      }

   while (dwError)
      {
      i++;

      if (dwError & 1)
         {
         if (LoadString( ghDvInst, i + IDS_PRN_NONE, szError, 100))
            MessageBox (hWnd, szError, NULL, MB_OK);
         else
            MessageBeep (0);
         }

      dwError >>= 1;
      }
}

//---------------------------------------------------------------------
//
// Function:   DoPrintSetup
//
// Purpose:    Allow the PRINT SETUP to be done.
//
// Parms:      None
//
// History:   Date      Reason
//             6May96   Created.
//---------------------------------------------------------------------
//void	DoPrintSetup( void )
void	Do_IDM_PRTSETUP( HWND hWnd )
{
   PRINTDLG pd;

   ZeroMemory( &pd, sizeof(PRINTDLG) );
   pd.lStructSize = sizeof(PRINTDLG);
   pd.Flags       = PD_PRINTSETUP | PD_HIDEPRINTTOFILE | PD_NOPAGENUMS;

   PrintDlg( &pd );

}	/* Do_IDM_PRTSETUP */


// #ifdef	SAMPPRT
// =================================================

#define ChangePrintPercent(nPct)    if(ghDlgAbort) SendMessage( ghDlgAbort, MYWM_CHANGEPCT, nPct, NULL)

extern   BOOL  CenterWindow( HWND hWnd, HWND hParent );

extern BOOL gbUseEscapes;   // Use Escape() or 3.1 printer APIs?
extern char szPrintDlg[]; // = "PRINTDLG";  // Name of Print dialog from .RC

extern   void  TranslatePrintRect( HDC, LPRECT, DWORD, DWORD, DWORD, BOOL );
extern   DWORD DoSetAbortProc( HDC hPrnDC, FARPROC lpfnAbortProc );

//extern BOOL MLIBCALL PRINTABORTPROC( HDC, int );
BOOL CALLBACK PRINTABORTPROC( HDC hdc,        // handle to DC
                              int iError );   // error value

//extern int MLIBCALL PRINTABORTDLG( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK PRINTABORTDLG(
  HWND   hDlg,  // handle to dialog box
  UINT   uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam ); // second message parameter

extern DWORD PrintABand( HDC, LPRECT, LPRECT, BOOL, BOOL, LPSTR,
						LPSTR );

void PrintStuff( HWND );
HDC  DVGetPrinterDC(void);
BOOL CALLBACK DVPrintAbortProc( HDC, int );
void InitDocStruct( DOCINFO MLPTR, LPSTR );
void DrawStuff( HDC );
VOID  ChkDefPrtr( VOID );
 
//PRINTDLG pdlg;

// From the Print Per Options of the Frame Window ... 
void Do_IDM_PRINT2( HWND hWnd )
{
	HWND        hDIBWnd;
	HANDLE      hDIB;
	RECT        rect;       // only used in SOME case
	HANDLE      hDIBInfo;
	LPDIBINFO   lpDIBInfo;
	DWORD       cxDIB, cyDIB;
	DWORD		wUnits, cxScale, cyScale;
	BOOL        bUseBanding, bUse31APIs;
	char        lpszDocName [MAX_FILENAME];

	if( hDIBWnd = GetCurrentMDIWnd() )
	{
		if( hDIBInfo = GetWindowExtra(hDIBWnd, WW_DIB_HINFO) )
		{

			if( lpDIBInfo   = (LPDIBINFO) DVGlobalLock (hDIBInfo) )
			{
				hDIB        = lpDIBInfo->hDIB;
				cxDIB       = lpDIBInfo->di_dwDIBWidth;
				cyDIB       = lpDIBInfo->di_dwDIBWidth;
				wUnits      = lpDIBInfo->Options.wPrintOption;
				cxScale     = lpDIBInfo->Options.wXScale;
				cyScale     = lpDIBInfo->Options.wYScale;
#ifdef   WIN32
            // NOTE: In WIN32 bUseBanding is actually CENTERING
				bUseBanding = lpDIBInfo->Options.bCenter;
				bUse31APIs  = FALSE;
#else // !#ifdef   WIN32
				bUseBanding = lpDIBInfo->Options.bPrinterBand;
				bUse31APIs  = lpDIBInfo->Options.bUse31PrintAPIs;
#endif   // #ifdef   WIN32 y/n

				lstrcpy( lpszDocName, &lpDIBInfo->di_szDibFile[0] );


				DVGlobalUnlock (hDIBInfo);
				switch (wUnits)
				{
				case PRINT_BESTFIT:
					wUnits = UNITS_BESTFIT;
					// rect is filled in by DIBPrint().
					break;

				case PRINT_STRETCH:
					wUnits = UNITS_STRETCHTOPAGE;
					// rect is filled in by DIBPrint().
					break;

				case PRINT_SCALE:
					wUnits      = UNITS_SCALE;
					rect.left   = cxScale;
					rect.top    = cyScale;
					// rect is modified to reflect actual width in DIBPrint.
					break;

				default:
					wUnits      = UNITS_PIXELS;
					rect.left   = 0;
					rect.top    = 0;
					rect.right  = cxDIB;
					rect.bottom = cyDIB;
					break;
				}	// switch

				if( hDIB )
				{
					DWORD dwError;
					if( dwError = DIBPrint( hDIBWnd,
                                    hDIB,
                                    &rect,      // IN = vary per units OUT = Result
                                    wUnits,
                                    SRCPAINT,
                                    bUseBanding,   // IN WIN32 used to indicate CENTERING
                                    bUse31APIs,    // NOT USED IN WIN32
                                    lpszDocName ) )
               {
						ShowPrintError (hWnd, dwError);
               }
				}
			}
		}
	}
}

void PrintStuff( HWND hWndParent )
{
    HDC        hDC;
    DOCINFO    di;
 
    // Need a printer DC to print to
    hDC = DVGetPrinterDC();
 
    // Did you get a good DC?
    if( !hDC )
    {
        MessageBox(NULL, "Error creating DC", "Error",
                                    MB_APPLMODAL | MB_OK );
        return;
    }
 
    // You always have to use an DVPrintAbortProc()
    if( SetAbortProc( hDC, DVPrintAbortProc ) == SP_ERROR )
    {
        MessageBox( NULL, "Error setting up AbortProc",
                                    "Error", MB_APPLMODAL | MB_OK);
        return;
    }
 
    // Init the DOCINFO and start the document
    InitDocStruct( &di, "MyDoc");
    StartDoc( hDC, &di );
 
    // Print one page
    StartPage( hDC );
    DrawStuff( hDC );
    EndPage( hDC );
 
    // Indicate end of document
    EndDoc( hDC );
 
    // Clean up
    DeleteDC( hDC );
}
 
/*===============================*/
/* Obtain printer device context */
/* ==============================*/
HDC DVGetPrinterDC(void)
{
   PRINTDLG pdlg;

	// Initialize the PRINTDLG structure
	dv_fmemset( &pdlg, 0, sizeof( PRINTDLG ) );
	pdlg.lStructSize = sizeof( PRINTDLG );
	// Set the flag to return printer DC
	pdlg.Flags = PD_RETURNDEFAULT | PD_RETURNDC;

	// Invoke the printer dialog box
	PrintDlg( &pdlg );

	// hDC member of the PRINTDLG structure contains
	// the printer DC
	return pdlg.hDC;

}
 
/*===============================*/
/* The Abort Procudure           */
/* ==============================*/
BOOL CALLBACK DVPrintAbortProc( HDC hDC, int Error )
{
    MSG   msg;
    while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
    return TRUE;
}
 
/*===============================*/
/* Initialize DOCINFO structure  */
/* ==============================*/
void InitDocStruct( DOCINFO MLPTR di, LPSTR docname)
{
	// Always zero it before using it
	dv_fmemset( di, 0, sizeof( DOCINFO ) );

	// Fill in the required members
	di->cbSize = sizeof( DOCINFO );
	di->lpszDocName = docname;

}
 
/*===============================*/
/* Drawing on the DC             */
/* ==============================*/
void DrawStuff( HDC hdc)
{
    // This is the function that does draws on a given DC
    // Here, you are printing text
    TextOut(hdc, 0,0, "Test Printing", lstrlen( "Test Printing" ) );
}
 
DWORD DIBPrint( HWND   hDIBWnd,
                HANDLE hDIB,
                LPRECT lpPrintRect, // IN varies per UNITS - will be set in Translate
                DWORD wUnits,       // internal SCALE value
                DWORD dwROP,
                BOOL fBanding,      // in WIN32 this is CENTERING
                BOOL fUse31APIs,
                LPSTR lpszDocName )
{
	DWORD          dwret;
	HDC            hPrnDC;
	RECT           rect;
	static FARPROC lpAbortProc;
	LPSTR          lpDIBHdr, lpBits;
    DOCINFO        di;

	dwret = ERR_PRN_NODIB;
	if( hDIB )
	{
		gbUseEscapes = FALSE;
		if( lpDIBHdr = DVGlobalLock (hDIB) )  // LOCK DIB HANDLE
		{
			dwret = ERR_PRN_NONE;
			if( ( lpBits = FindDIBBits(lpDIBHdr) ) &&
				 ( hPrnDC = DVGetPrinterDC()      ) )
			{
                gbAbort      = FALSE;

				// Init the DOCINFO
				if( lpszDocName && lpszDocName[0] )
					InitDocStruct( &di, lpszDocName );
				else
					InitDocStruct( &di, "MyDoc" );

                SetStretchBltMode ( hPrnDC, COLORONCOLOR);

                TranslatePrintRect( hPrnDC,
                    lpPrintRect,      // results depend on UNITS selected
                    wUnits,           // internal SCALE value
                    DIBWidth (lpDIBHdr),
                    DIBHeight (lpDIBHdr),
                    fBanding );

                // Initialize the abort procedure.  Then STARTDOC.
                lpAbortProc = (FARPROC)MakeProcInstance( PRINTABORTPROC, ghDvInst);
                ghDlgAbort = CreateDialogParam( ghDvInst,
                    szPrintDlg, // = "PRINTDLG";  // Name of Print dialog from .RC
                    GetFocus(),
                    (DLGPROC)PRINTABORTDLG,
                    (LPARAM) &di );

                if( !ghDlgAbort )
                    goto PRINT_END;

                if( dwret |= DoSetAbortProc( hPrnDC, lpAbortProc ) )
                    goto PRINT_END;

                // The StartDoc function starts a print job. 
                StartDoc( hPrnDC, &di );

				// Print one page
				StartPage( hPrnDC );

				//DrawStuff( hPrnDC );
				rect = *lpPrintRect;    // copy the PRINT rectange

            //For example, a Win32-based application should not call the
            //NEXTBAND and BANDINFO escapes. Banding is no longer needed in
            //Windows 95+.
      		//if( fBanding )
			   //   dwret |= BandDIBToPrinter (hPrnDC, lpDIBHdr, lpBits, lpPrintRect);
		      //else
		      {
				   dwret |= PrintABand( hPrnDC,  // Device context
                              lpPrintRect,   // output print rectangle of object
                              &rect,         // in this case, a copy of same
                              TRUE,          // N/A in WIN32
                              TRUE,          // N/A in WIN32
                              lpDIBHdr,      // DIB header
                              lpBits );      // pointer at bits
            }

				EndPage( hPrnDC );

				// Indicate end of document
				EndDoc( hPrnDC );

PRINT_END:

				// Clean up
				if( ghDlgAbort )
					DestroyWindow( ghDlgAbort );

				ghDlgAbort = 0;

				FreeProcInstance(lpAbortProc);

                if( hPrnDC )
					DeleteDC( hPrnDC );
			}
			else
			{
		      dwret |= ERR_PRN_NODC;
			}

			DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE

		}
	}

	return( dwret );

}

// =================================================
//#endif	// SAMPPRT

// eof - DvPrint2.c
