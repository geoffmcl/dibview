
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright 1993 - 1998 Microsoft Corporation.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
\******************************************************************************/

/*******************************************************************************
 *                                                                             *
 *  MODULE      : DLGOPEN.C                                                    *
 *                                                                             *
 *  DESCRIPTION : Routines to display a standard File/Open and File/Save       *
 *                dialog boxes.                                                *
 *                                                                             *
 *  FUNCTIONS   : DlgOpenFile() - Displays a dialog box for opening or saving a*
 *                                file.                                        *
 *                                                                             *
 *                DlgfnOpen()   - Dialog function for the above dialog.        *
 *                                                                             *
 *                AddExt()      - Adds an extension to a filename if not       *
 *                                already present.                             *
 *                                                                             *
 *                FSearchSpec() - Checks if given string contains a wildcard   *
 *                                character.                                   *
 *                                                                             *
 *                FillListBox() - Fills listbox with files that match specs.   *
 *                                                                             *
 *                DlgCheckOkEnable() - Enables <OK> button iff there's text in *
 *                                     the edit control.                       *
 *                                                                             *
 *                NOTE : These routines require that the app. be running       *
 *                       SS = DS since they use near pointers into the stack.  *
 *                                                                             *
 *******************************************************************************/
#include <windows.h>
#include <string.h>
#include "showdib.h"
#include "..\SD2Util.h"    // some services, ie SplitFN(), ...
#include "../../src/DVList.h"     // LINKED LIST MWL and PMWL
#ifndef  VFH
#define  VFH(a)   ( a && ( a != INVALID_HANDLE_VALUE ) )
#endif   // VFH

static PSTR         pszExt;
static PSTR         szFileName;
static PSTR         szTitle;
static DWORD        flags;
static WORD         fOpt;

/* Forward declarations of helper functions */

static VOID  NEAR DlgCheckOkEnable(HWND hwnd, INT idEdit, UINT message);
static CHAR *NEAR FillListBox (HWND,TCHAR*, UINT);
static BOOL  NEAR FSearchSpec (TCHAR*);
static VOID  NEAR AddExt (TCHAR*,TCHAR*);

#define DLGOPEN_UNUSED   0

/* Mask to eliminate bogus style and bitcount combinations ...
 * RLE formats, if chosen should be matched with the bitcounts:
 *   RLE4 scheme should be used only for 4 bitcount DIBs.
 *   RLE8   "      "     "   "   "    "  8   "       "
 *
 * BITCOUNTMASK is indexed by DLGOPEN_RLE4 >> 4, DLGOPEN_RLE8 >> 4
 * and DLGOPEN_RLE8 >> 4
 */

static WORD BITCOUNTMASK[] = { DLGOPEN_UNUSED,
                               DLGOPEN_1BPP | DLGOPEN_8BPP | DLGOPEN_24BPP,
                               DLGOPEN_1BPP | DLGOPEN_4BPP | DLGOPEN_24BPP,
                               DLGOPEN_UNUSED,
                               0 };

static TCHAR     achFile[264];
static TCHAR     achExt[264];

LPTSTR   gpOpen  = "Format >>";
LPTSTR   gpClose = "Format <<";

TCHAR szBuffer[264];

// === another try at open/browse - real simple
BOOL  g_bUseNew = TRUE; // switch to NEW dialog

TCHAR g_szFullPath[264];
TCHAR g_szTitle[264];
TCHAR g_szPath[264];
TCHAR g_szTit[264];
TCHAR gszOFile[264];
TCHAR gszOTitle[264];
TCHAR szFilterOpen[] =
   "Windows Bitmap (*.bmp)\0"
   "*.bmp\0"
   "All Files (*.*)\0"
   "*.*\0"
   "\0";

OPENFILENAME   gsOpen;
WIN32_FIND_DATA   g_FD;

LRESULT  DVGetFileOpen( VOID )
{
   LRESULT  lRes = 0;
	OPENFILENAME * pofn = &gsOpen;
   LPTSTR   lpf, lpt;
   //HCURSOR  hcur;

   lpf = &gszOFile[0];
   lpt = &gszOTitle[0];
   if( pofn->lStructSize != sizeof(OPENFILENAME) )
   {
      pofn->lStructSize			= sizeof(OPENFILENAME);
	   pofn->hInstance			= 0;
	   pofn->lpstrFilter			= szFilterOpen;
	   pofn->lpstrCustomFilter	= NULL;
	   pofn->nMaxCustFilter		= 0;
	   pofn->nFilterIndex		= 0;
	   pofn->lpstrFileTitle		= lpt;
	   pofn->nMaxFileTitle		= 256;
	   pofn->lpstrInitialDir	= NULL;
	   pofn->lpstrTitle			= NULL;
	   pofn->nFileOffset			= 0;
	   pofn->nFileExtension		= 0;
 	   pofn->lpstrDefExt			= NULL;
	   pofn->lCustData			= 0L;
	   pofn->lpfnHook			   = NULL;
	   pofn->lpTemplateName		= NULL;
   	pofn->hwndOwner			= hWndApp;
	   pofn->lpstrFile			= lpf;
	   pofn->nMaxFile			   = 256;
	   pofn->Flags				   = OFN_CREATEPROMPT;
   	strcpy(lpf, g_szFullPath);
      strcpy(lpt, g_szTitle   );
   }

   if( GetOpenFileName(pofn) )
	{
      HANDLE hFind = FindFirstFile( lpf, &g_FD );
      if( VFH(hFind) )
      {

         FindClose(hFind);
      }
   }
   return lRes;
}

INT_PTR  nOpn_WM_INITDIALOG( HWND hDlg )
{
   HWND  hwnd = GetDlgItem(hDlg, IDC_COMBO1);
   if( !hwnd )
   {
      EndDialog(hDlg, -1);
      return -1;
   }




   CenterDialog( hDlg, hWndApp );
   return TRUE;
}
INT_PTR  nOpn_WM_COMMAND( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
   DWORD cmd = LOWORD(wParam);
   switch(cmd)
   {
   case IDCANCEL:
      EndDialog( hDlg, 0 );
      break;
   case IDOK:
      EndDialog( hDlg, 0 );
      break;
   case IDC_BROWSE:
      DVGetFileOpen();
      break;
   }

   return TRUE;
}

INT_PTR CALLBACK OPENBMPPROC(
  HWND hDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
   INT_PTR  iRet = TRUE;
   switch (uMsg)
   {
   case WM_INITDIALOG:
       nOpn_WM_INITDIALOG( hDlg );
       break;

   case WM_COMMAND:
       nOpn_WM_COMMAND( hDlg, wParam, lParam );
       break;

   default:
      return FALSE;
   }

   return iRet;

}

// NOTE: ghWnd is the MAIN window handle, hInst our current module, ...
HANDLE   NewDlgOpenFile( LPTSTR pcf )
{
   HANDLE   hFile;
   if( pcf )
   {
      if( !_fullpath( g_szFullPath, pcf, 256 ) )
         strcpy(g_szFullPath,pcf);
      GetFileTitle( g_szTitle, g_szFullPath, 256 );
      SplitFN( g_szPath, g_szTit, g_szFullPath );
   }

   hFile = (HANDLE)DialogBox(hInst,
      MAKEINTRESOURCE(IDD_OPENBMP), // "DlgOpenBox",
      hWndApp,      // hwndParent,
      OPENBMPPROC );   //(DLGPROC)lpProc);

   return hFile;
}
/*******************************************************************************
 *                                                                             *
 *  FUNCTION   :DlgOpen(LPSTR szFile)                                          *
 *                                                                             *
 *  PURPOSE    :Display dialog box for opening files. Allow user to interact   *
 *              with dialogbox, change directories as necessary, and try to    *
 *              open file if user selects one. Automatically append            *
 *              extension to filename if necessary.                            *
 *              This routine correctly parses filenames containing KANJI       *
 *              characters.                                                    *
 *                                                                             *
 *  RETURNS    :  - Handle to the opened file if legal filename.               *
 *                - 0 if user presses <cancel>                                 *
 *                - 1 if filename entered is illegal                           *
 *                                                                             *
 *******************************************************************************/
HANDLE APIENTRY DlgOpenFile (
    HWND          hwndParent,
    CHAR          *szTitleIn,
    DWORD         flagsIn,
    CHAR          *szExtIn,
    CHAR          *szFileNameIn,
    WORD          *pfOpt)
{
    INT_PTR      fh;
    FARPROC  lpProc;
//    static TCHAR     achFile[264];
//    static TCHAR     achExt[264];
    HANDLE   hInstance;
    WORD     w;

    if(pfOpt == NULL)
        pfOpt = &w;

    flags    = flagsIn;
    fOpt     = *pfOpt;

    szFileName = achFile;
    pszExt     = achExt;
    strcpy( szFileName, szFileNameIn );
    strcpy( pszExt,     szExtIn );
    szTitle = szTitleIn;
    if( g_bUseNew )
       return NewDlgOpenFile( szFileName );


    hInstance = (HANDLE)GetWindowLongPtr (hwndParent, GWLP_HINSTANCE);

    /* Show the dialog box */
    lpProc = MakeProcInstance ((FARPROC)DlgfnOpen, hInstance);
    fh = DialogBox (hInstance, "DlgOpenBox", hwndParent, (DLGPROC)lpProc);
    FreeProcInstance (lpProc);

    if(fh != 0)
    {
        strcpy( szFileNameIn, szFileName );  // copy the NAME
        *pfOpt = fOpt;  // and OPTIONS
    }
    return fh;
}

BOOL  Opn_WM_INITDIALOG( HWND hwnd )
{
   LPTSTR   lpB = szBuffer;
   HWND     hwndT;
   INT      f;

   if( szTitle && *szTitle )
      SetWindowText(hwnd, szTitle);

   /* Set text on <OK> button according to mode (File/Open or File/Save) */
   if( flags & OF_SAVE )
   {
                LoadString(hInst, IDS_SAVESTR, lpB, 256);
                SetDlgItemText(hwnd, IDOK, lpB);
   }
   if( flags & OF_OPEN )
   {
                LoadString(hInst, IDS_OPENSTR, lpB, 256);
                SetDlgItemText(hwnd, IDOK, lpB);
   }

   hwndT = GetDlgItem(hwnd, DLGOPEN_FOLDOUT);
   if((flags & OF_NOOPTIONS) &&
      (hwndT               ) )
   {
      EnableWindow( hwndT, FALSE );
      ShowWindow( hwndT, SW_HIDE );
   }

   hwndT = GetDlgItem( hwnd, DLGOPEN_SMALL );
   if(hwndT)
   {
      RECT  rc, rcCtl;
      GetWindowRect( hwnd,  &rc   );
      GetWindowRect( hwndT, &rcCtl);

      SetWindowPos( hwnd,
         NULL,
         0,
         0,
         rcCtl.left - rc.left,
         rc.bottom - rc.top,
         SWP_NOZORDER | SWP_NOMOVE );

      SetDlgItemText( hwnd, DLGOPEN_FOLDOUT, gpOpen );
   }

   /* fill list box with filenames that match specifications, and
    * fill static field with path name.
    */

   FillListBox(hwnd, pszExt, WM_INITDIALOG);

   /* If in Save mode, set the edit control with default (current)
    * file name,and select the corresponding entry in the listbox.
    */
   if((flags & OF_SAVE) && *szFileName)
   {
                        SetDlgItemText (hwnd, DLGOPEN_EDIT, szFileName);
                        SendDlgItemMessage (hwnd,
                                    DLGOPEN_FILE_LISTBOX,
                                    LB_SELECTSTRING,
                                    0,
                                    (LONG_PTR)(LPSTR)szFileName);
   }
   else
   {
                /*  Set the edit field with the default extensions... */
                if (flags & OF_NOSHOWSPEC)
                    SetDlgItemText (hwnd, DLGOPEN_EDIT, "");
                else
                    SetDlgItemText (hwnd, DLGOPEN_EDIT, pszExt);
   }

   /*  ...and select all text in the edit field */
   /* JAP added HWND cast*/
   SendMessage((HWND)GetDlgItem(hwnd, DLGOPEN_EDIT), EM_SETSEL, GET_EM_SETSEL_MPS(0, 0x7fff));

   /*  check all options that are set */
   for ( f = DLGOPEN_1BPP; f; f<<=1)
      CheckDlgButton(hwnd, (INT)FID(f), (WORD) (fOpt & f));

   return TRUE;
}

BOOL Opn_WM_COMMAND( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
   DWORD cmd = LOWORD(wParam);
   LPTSTR   lpsz;
   INT      result = -1;
   OFSTRUCT of;
   INT      f;
   BOOL     fEnable;
   INT      w = 0;				/* Is 0 OK for initialization? */
   RECT     rc, rcCtl;

   switch(cmd)
   {
   case IDOK:
      if( IsWindowEnabled (GetDlgItem(hwnd, IDOK)) )
      {
         /* Get contents of edit field and add search spec. if it
          * does not contain one.
          */
         GetDlgItemText( hwnd, DLGOPEN_EDIT, (LPSTR)szFileName, 256 );

         lpsz = CharPrev(szFileName, (szFileName + strlen(szFileName)) );
         switch(*lpsz)
         {
            case '\\':
            case '/':
               *lpsz = 0;
               break;
               
         }

         if(SetCurrentDirectory( (LPSTR)szFileName ) )
            strcpy( szFileName, pszExt);

         /*  Try to open path.  If successful, fill listbox with
          *  contents of new directory.  Otherwise, open datafile.
          */
         if( FSearchSpec(szFileName) )
         {
            strcpy( pszExt, FillListBox (hwnd, szFileName, WM_COMMAND) );
            if(flags & OF_NOSHOWSPEC)
               SetDlgItemText (hwnd, DLGOPEN_EDIT, "");
            else
               SetDlgItemText (hwnd, DLGOPEN_EDIT, pszExt);
            break;

         }

         /*  Make filename upper case and if it's a legal DOS
          *  name, try to open the file.
          */

         CharUpper(szFileName);
         AddExt(szFileName, pszExt);

         result = (INT)OpenFile(szFileName, &of, (WORD)flags);

         if( result && (result != -1) )
         {
            strcpy(szFileName,of.szPathName);
                        
         }
         else if (flags & OF_MUSTEXIST)
         {
            MessageBeep(0);
            return 0L;
         }

         /*  Get the state of all checked options */
         for( f = DLGOPEN_1BPP; f; f <<= 1)
         {
            if( IsDlgButtonChecked (hwnd, FID (f)) )
               fOpt |= f;
            else
               fOpt &= ~f;
         }

         EndDialog (hwnd, result);
                    
      }
      
      break;

   case DLGOPEN_OPTION + DLGOPEN_RLE4:
   case DLGOPEN_OPTION + DLGOPEN_RLE8:
   case DLGOPEN_OPTION + DLGOPEN_RGB:
      /* Mask out incompatible bitcount options and gray the
       * appropriate radiobuttons.
       */
      for( f = DLGOPEN_1BPP; f <= DLGOPEN_24BPP; f <<= 1) 
      {
         fEnable = !(f & BITCOUNTMASK [IDF(w) >> 4 ]);
         EnableWindow (GetDlgItem (hwnd, FID(f)), fEnable);

         /* If the radiobutton is being grayed, uncheck it and
          * and check an "allowed" option so the bitcount group
          * is still accessible via the keyboard
          */
         if( !fEnable && IsDlgButtonChecked (hwnd, FID (f)))
         {
            CheckDlgButton(hwnd, FID(f), FALSE);
            CheckDlgButton(hwnd, FID(IDF(w) >> 3), TRUE);
            
         }
         
      }
      
      break;

   case IDCANCEL:
      /* User pressed cancel.  Just take down dialog box. */
      EndDialog (hwnd, 0);
      break;

   /*  User single clicked or doubled clicked in listbox -
    *  Single click means fill edit box with selection.
    *  Double click means go ahead and open the selection.
    */
   case DLGOPEN_FILE_LISTBOX:
   case DLGOPEN_DIR_LISTBOX:
      switch( GET_WM_COMMAND_CMD(wParam, lParam) )
      {
         /* Single click case */
      case LBN_SELCHANGE:
         /* Get selection, which may be either a prefix to a
          * new search path or a filename. DlgDirSelectEx parses
          * selection, and appends a backslash if selection
          * is a prefix
          */
          DlgDirSelectEx(hwnd, szFileName, 128, LOWORD(wParam));
          lpsz = CharPrev(szFileName, szFileName + lstrlen(szFileName));
          switch(*lpsz)
          {
             case ':':
                strcat(szFileName,".");
                break;
             case '\\':
             case '/':
                *lpsz = 0;
                break;
                
          }

          SetDlgItemText(hwnd, DLGOPEN_EDIT, szFileName);
          break;

          /* Double click case - first click has already been
           * processed as single click
           */
          case LBN_DBLCLK:
             PostMessage(hwnd,WM_COMMAND,IDOK,0L);
             break;
                   
      }
      break;

   case DLGOPEN_EDIT:
      DlgCheckOkEnable(hwnd, DLGOPEN_EDIT, HIWORD(lParam));
      break;

   case DLGOPEN_FOLDOUT:

      GetWindowRect(hwnd,&rc);
      GetWindowRect(GetDlgItem(hwnd,DLGOPEN_BIG),&rcCtl);
      lpsz = gpOpen;

      if( (rcCtl.left <= rc.right) && (rcCtl.top <= rc.bottom) )
      {
         GetWindowRect( GetDlgItem(hwnd, DLGOPEN_SMALL), &rcCtl);
         lpsz = gpClose;
      }

      SetWindowPos( hwnd,
         NULL,
         0,
         0,
         rcCtl.left - rc.left,
         rc.bottom - rc.top,
         SWP_NOZORDER | SWP_NOMOVE);

      SetDlgItemText( hwnd, DLGOPEN_FOLDOUT, lpsz );

      break;
       
   }

   CenterDialog( hwnd, hWndApp );

   return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   :DlgfnOpen (hwnd, msg, wParam, lParam)                       *
 *                                                                          *
 *  PURPOSE    :Dialog function for File/Open dialog.                       *
 *                                                                          *
 ****************************************************************************/
INT_PTR APIENTRY DlgfnOpen (
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
       Opn_WM_INITDIALOG( hwnd );
       break;

    case WM_COMMAND:
       Opn_WM_COMMAND( hwnd, wParam, lParam );
       break;

    default:
       return FALSE;
    }

    return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : static void NEAR DlgCheckOkEnable(hwnd, idEdit, message)   *
 *                                                                          *
 *  PURPOSE    : Enables the <OK> button in a dialog box iff the edit item  *
 *               contains text.                                             *
 *                                                                          *
 ****************************************************************************/
static VOID NEAR DlgCheckOkEnable(
    HWND        hwnd,
    INT idEdit,
    UINT message)
{
    if (message == EN_CHANGE) {
        EnableWindow ( GetDlgItem (hwnd, IDOK),
                       (BOOL)SendMessage (GetDlgItem (hwnd, idEdit),
                                          WM_GETTEXTLENGTH,
                                          0, 0L));
    }
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : AddExt (pch, ext)                                          *
 *                                                                          *
 *  PURPOSE    : Add an extension to a filename if none is already specified*
 *                                                                          *
 ****************************************************************************/
static VOID NEAR AddExt (
    CHAR *pch,    /* File name    */
    CHAR *ext)    /* Extension to add */
{
    CHAR acExt[20];
    CHAR *pext = acExt;

    while (*ext && *ext != '.') {
        ext = CharNext(ext);
    }
    while (*ext && *ext != ';') {
        if (IsDBCSLeadByte(*ext)) {
            *pext++ = *ext++;
        }
        *pext++ = *ext++;
    }
    *pext = 0;
    pext = acExt;

    while (*pch == '.') {
        pch++;
        if ((*pch == '.') && pch[1] == '\\')
            pch += 2;                       /* ..\ */
        if (*pch == '\\')
            pch++;                         /* .\ */
    }
    while (*pch != '\0') {
	if (*pch == '.')
	    return;
	pch = CharNext(pch);
    }

    // *pch++ = '.';
    do
        *pch++ = *pext;
    while (*pext++ != '\0');
}
/****************************************************************************
 *                                                                          *
 *  FUNCTION   : FSearchSpec (sz)                                           *
 *                                                                          *
 *  PURPOSE    : Checks to see if NULL-terminated strings contains a "*" or *
 *               a "?".                                                     *
 *                                                                          *
 *  RETURNS    : TRUE  - if the above characters are found in the string    *
 *               FALSE - otherwise.                                         *
 *                                                                          *
 ****************************************************************************/
static BOOL NEAR FSearchSpec(TCHAR * psz)
{
   INT   c;
   for(; *psz ;psz = CharNext(psz))
   {
      c = *psz;
       if( (c == '*') || (c == '?') )
          return TRUE;
    }
    return FALSE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : static char * NEAR FillListBox (hDlg,pFile, cmd)                   *
 *                                                                          *
 *  PURPOSE    : Fill list box with filenames that match specifications, and*
 *               fills the static field with the path name.                 *
 *                                                                          *
 *  RETURNS    : A pointer to the pathname.                                                           *
 *                                                                          *
 ****************************************************************************/
static CHAR * NEAR FillListBox (
    HWND   hDlg,
    TCHAR  * pFile,  /* [path]={list of file wild cards, separated by ';'} */
    UINT  cmd )    /* if initdialog, or WM_COMMAND*/
{
    static TCHAR  _s_ach[264];
    static TCHAR  _s_cCurDir[264];

    TCHAR  * pch;
    TCHAR  * pDir;   /* Directory name or path */
    TCHAR  * pcwd;

    pch  = pFile;
    pDir = _s_ach;
    pcwd = _s_cCurDir;

    if( cmd == WM_INITDIALOG )
    {
       while( *pch && (*pch != ';') )
          pch = CharNext(pch);

       while( (pch > pFile) && (*pch != '/') && (*pch != '\\') )
          pch = CharPrev(pFile, pch);

       if( pch > pFile )
       {
          *pch = 0;
          strcpy (pDir, pFile);
          pFile = pch + 1;
       }
       else
       {
          strcpy (pDir,".");
       }
    }
    else
    {
        /* since SetCurrentDirectory was called already, I'll use GetCurrentDirectory*/
        /* to get pDir*/
        GetCurrentDirectory(256, pcwd);
        strcpy(pDir, pcwd);
    }

    // DlgDirList
    // The DlgDirList function replaces the contents of a list box
    // with the names of the subdirectories and files in a specified
    // directory. You can filter the list of names by specifying a
    // set of file attributes. The list can optionally include mapped drives.

    DlgDirList (hDlg, pDir, (INT)DLGOPEN_DIR_LISTBOX, (INT)DLGOPEN_PATH,(WORD)ATTRDIRLIST);

    SendDlgItemMessage (hDlg, DLGOPEN_FILE_LISTBOX, LB_RESETCONTENT, 0, 0L);
    SendDlgItemMessage (hDlg, DLGOPEN_FILE_LISTBOX, WM_SETREDRAW, FALSE, 0L);

    pDir = pFile;            /* save pFile to return */

    while (*pFile)
    {
        pch = _s_ach;   // set pointer

        while( *pFile == ' ' )
           pFile++;

        while(*pFile && (*pFile != ';'))
        {
           if( IsDBCSLeadByte(*pFile) )
           {
		        *pch++ = *pFile++;
	        }
	        *pch++ = *pFile++;
        }

        *pch = 0;

        if(*pFile)
           pFile++;

        SendDlgItemMessage( hDlg,
                            DLGOPEN_FILE_LISTBOX,
                            LB_DIR,ATTRFILELIST,
                            (LONG_PTR)(LPTSTR)_s_ach);
    }

    SendDlgItemMessage (hDlg, DLGOPEN_FILE_LISTBOX, WM_SETREDRAW, TRUE, 0L);

    InvalidateRect (GetDlgItem (hDlg, DLGOPEN_FILE_LISTBOX), NULL, TRUE);

    return pDir;

}

// **************** some utility functions **********
///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : CenterDialog
// Return type: BOOL 
// Arguments  : HWND hChild
//            : HWND hParent
// Description: 
// Added from grmLib.c - April, 2001 (FROM GMUtils.c - December, 1999)
// ===================================================================
//  FUNCTION: CenterDialog(HWND, HWND)
//	(was CenterWindow in GMUtils)
//  PURPOSE:  Center one window over another.
//  PARAMETERS:
//    hwndChild - The handle of the window to be centered.
//    hwndParent- The handle of the window to center on.
//  RETURN VALUE:
//    TRUE  - Success
//    FALSE - Failure
//  COMMENTS:
//    Dialog boxes take on the screen position that they were designed
//    at, which is not always appropriate. Centering the dialog over a
//    particular window usually results in a better position.
///////////////////////////////////////////////////////////////////////////////
BOOL CenterDialog(HWND hChild, HWND hParent)
{
    BOOL	bret = FALSE;
    RECT    rcChild, rcParent;
    int     cxChild, cyChild, cxParent, cyParent;
    int     cxScreen, cyScreen, xNew, yNew;
    HDC     hdc;
    HWND	hwndChild, hwndParent;

    hwndChild = hChild;
    hwndParent = hParent;
    if (!hParent && hChild)
        hwndParent = GetParent(hChild);

    if (hwndChild && hwndParent)
    {

        // Get the Height and Width of the child window
        if (GetWindowRect(hwndChild, &rcChild))
        {
            cxChild = rcChild.right - rcChild.left;
            cyChild = rcChild.bottom - rcChild.top;

            // Get the Height and Width of the parent window
            if (GetWindowRect(hwndParent, &rcParent))
            {
                cxParent = rcParent.right - rcParent.left;
                cyParent = rcParent.bottom - rcParent.top;

                // Get the display limits
                if (hdc = GetDC(hwndChild))
                {
                    cxScreen = GetDeviceCaps(hdc, HORZRES);
                    cyScreen = GetDeviceCaps(hdc, VERTRES);
                    ReleaseDC(hwndChild, hdc);

                    // Calculate new X position,
                    // then adjust for screen
                    xNew = rcParent.left +
                        ((cxParent - cxChild) / 2);
                    if (xNew < 0)
                    {
                        xNew = 0;
                    }
                    else if ((xNew + cxChild) > cxScreen)
                    {
                        xNew = cxScreen - cxChild;
                    }
                    // Calculate new Y position,
                    // then adjust for screen
                    yNew = rcParent.top +
                        ((cyParent - cyChild) / 2);
                    if (yNew < 0)
                    {
                        yNew = 0;
                    }
                    else if ((yNew + cyChild) > cyScreen)
                    {
                        yNew = cyScreen - cyChild;
                    }

                    // Set it, and return
                    bret = SetWindowPos(hwndChild,
                        NULL,
                        xNew, yNew,
                        0, 0,
                        SWP_NOSIZE | SWP_NOZORDER);
                }
            }
        }
    }
    return bret;
}
// END CenterDialog(HWND,HWND) ADDED FROM GMUtils.c
// December, 1999
// =====================================

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : SplitFN
// Return type: void 
// Arguments  : LPTSTR pPath
//            : LPTSTR pFile
//            : LPTSTR pFullName
// Description: Split the pFullName into a PATH, including the final \, and
//              a clean FILE NAME ONLY
///////////////////////////////////////////////////////////////////////////////
VOID  SplitFN(LPTSTR pPath, LPTSTR pFile, LPTSTR pFullName)  // 23Aug2002 - add last chk.
{
    int      i, j, k;
    TCHAR    c;

    j = 0;
    if (pFullName)
        j = strlen(pFullName);
    if (j)
    {
        c = pFullName[j - 1]; // get LAST character
        if ((c == ':') || (c == '\\'))
        {
            if (pPath) // if pPath given
                strcpy(pPath, pFullName);

            if (pFile)
                *pFile = 0;

            return;  // out of here
        }

        k = 0;
        for (i = 0; i < j; i++)
        {
            c = pFullName[i];
            if ((c == ':') || (c == '\\'))
            {
                k = i;
            }
        }
        if (k)  // get LAST ':' or '\'
        {
            if (k < j)
                k++;
            if (pPath) // if pPath given
            {
                strncpy(pPath, pFullName, k);
                pPath[k] = 0;
            }
            if (pFile) // if pFile given
            {
                strcpy(pFile, &pFullName[k]);
            }
        }
        else
        {
            if (pFile) // if pFile given
                strcpy(pFile, pFullName);  // then there is NO PATH
            if (pPath)
                *pPath = 0;
        }
    }
}

// eof - DlgOpen.c
