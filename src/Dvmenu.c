
// DbMenu.c

#include	"dv.h"	/* All inclusive include ... */
#include <windowsx.h>

#define M_Fil           1
//#define M_Sep           0x80000000  /* Note: is set when 'Separator' added */

// external items
//extern	HWND  hFrameWnd;
//extern	HGLOBAL	hFileList;		// Global MEMORY handle for FILES
//extern	WORD	wFilCnt;	/* Count of files in File List ... */
extern	void	AddToFileList( LPSTR );
extern	PMWL  CommonFileOpen( HWND hWnd, LPSTR lpf, DWORD Caller );
// NEW
extern   PMWL	AddToFileList4( PRDIB prd );
extern	HINSTANCE hLibInst;
extern   BOOL	bPalDev;	// Display device can animate palettes?
extern	BOOL	fDnLibLoad;
extern   BOOL  gbHide;   // TRUE if MAIN window is to be hidden on capture
extern   BOOL  GotAChange( VOID );
// local items
//DWORD	dwMaxFiles = MXFILCNT;	/* Remember the last ?? files - See Dib.h */
//DWORDLONG	MenuFlag = 0;	// Max of 64-bits
//BOOL	fAddSep = FALSE;
//UINT	guiChecked = 0;

// Add FILENAMES to MENU ITEM
//#ifdef  USELLIST
DWORD SetFileMRU( HWND hwnd, PLIST_ENTRY pHead );

BOOL	AddMenuNames( PLIST_ENTRY pHead )
{
   BOOL  flg = FALSE;
   if( ( ghFrameWnd                      ) &&
       ( SetFileMRU( ghFrameWnd, pHead ) ) )
   {
      flg = TRUE;
   }
   return flg;
}


// ==========================================================
// long DoFileCommand( HWND hWnd, WPARAM wParam, LPARAM lParam )
//
// User has clicked on one of the File[n] entries in the MRU
// list presented. That is IDM_FILE??
//
// ==========================================================
// #ifdef   USELLIST

long DoFileCommand( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
   long     lret = 0;
	ENUSTR	es;
	HWND	   hChild;
   PLE      pHead, pNext;
   LPTSTR   lps, lpf;
   PMWL     pmwl;
   DWORD    dwf, dwc;

	es.es_code = ENU_FINDFILES;
	es.es_hwnd = 0;
	lps = &es.es_string[0];
   pHead = &gsFileList;
   dwf   = LOWORD(wParam);
   if( ( dwf >= IDM_FILE1   ) &&
       ( dwf <= IDM_FILEMax ) )
   {
      dwf -= IDM_FILE1;
      dwc  = 0;
      Traverse_List( pHead, pNext )
      {
         if( dwc == dwf )
            break;   // end this travers
         dwc++;
      }

      if( dwc == dwf )
      {
         // got the FILE from the MRU list
         pmwl = (PMWL)pNext;
         lpf  = &pmwl->wl_szFile[0];   // get pointer to file name
         strcpy( lps, lpf );
			EnumAllKids( &es );           // check if already LOADED
			if( hChild = es.es_hwnd )
			{
            // YES - only have to give it the focus
	PRDIB prd = (PRDIB)MALLOC( sizeof(RDIB) );
   if(!prd)
      chkme( "C:ERROR: No memory!!!"MEOR );
	NULPRDIB( prd );

            prd->rd_pTitle = gszRPTit;
            prd->rd_pPath  = gszRPNam;
            strcpy( gszRPTit, lpf );
            DVGetFullName2( gszRPNam, gszRPTit );
				prd->rd_hMDI   = hChild;
				prd->rd_Caller = df_COMMAND;	// This is a COMMAND
				// Got the CHILD
				// Bring CHILD to FRONT
				SetFocus( hChild );
#ifdef	CHGADDTO
				// And PUT file at TOP
				//AddToFileList( lps );
				//AddToFileList4( &rd );
            ADD2LIST(prd);    // and adjust the LIST position to the top
#endif	// CHGADDTO
            MFREE(prd);
			}
			else	// we can only try to OPEN the file
			{	// Pass the Parent, the FILE, and an ID
				if( CommonFileOpen( hWnd, lps, df_IDOPEN ) )
            {
               pmwl->wl_dwFlag |= flg_IsLoaded;
            }
			}
      }
   }

   return lret;
}

INT_PTR  OpenMRUFile( HWND hWnd, PMWL pmwl )
{
   INT_PTR  lret = 0;
	ENUSTR	es;
	HWND	   hChild;
   LPTSTR   lps, lpf;

	es.es_code = ENU_FINDFILES;
	es.es_hwnd = 0;
	lps = &es.es_string[0];
   lpf = &pmwl->wl_szFile[0];   // get pointer to file name
   strcpy( lps, lpf );
	EnumAllKids( &es );           // check if already LOADED
	if( hChild = es.es_hwnd )
	{
      // YES - only have to give it the focus
	PRDIB prd = (PRDIB)MALLOC( sizeof(RDIB) );
   if(!prd)
      chkme( "C:ERROR: No memory!!!"MEOR );
	NULPRDIB( prd );
      prd->rd_pTitle = gszRPTit;
      prd->rd_pPath  = gszRPNam;
      strcpy( gszRPTit, lpf );
      DVGetFullName2( gszRPNam, gszRPTit );
		prd->rd_hMDI   = hChild;
		prd->rd_Caller = df_COMMAND;	// This is a COMMAND
		// Got the CHILD
		// Bring CHILD to FRONT
		SetFocus( hChild );
#ifdef	CHGADDTO
		//AddToFileList4( &rd );
      ADD2LIST(prd);    // and adjust the LIST position to the top
#endif	// CHGADDTO
      MFREE(prd);
	}
	else	// we can only try to OPEN the file
	{	// Pass the Parent, the FILE, and an ID
		if( CommonFileOpen( hWnd, lps, df_IDOPEN ) )
      {
         pmwl->wl_dwFlag |= flg_IsLoaded;
      }
	}

   return lret;
}



/* ===================================================================== */
/* -----------------------------------------------------------------------
	Remove File Name from the 'File' menu pop-up
	for the MAX_FILS configured
   ----------------------------------------------------------------------- */
void DeletePref( HMENU hSMenu )
{
	DWORD	cyc;
	DWORDLONG	bflg = M_Fil ;		/* Init BIT flag - Max 15 shifts left */
	UINT	fMsg = IDM_FILE1 ;	/* Start IDM_FILE[n] ... */
	for( cyc = 0; cyc < gdwMaxFiles; cyc++ )
	{
		if( gdwlMenuFlag & bflg )
		{
			DeleteMenu( hSMenu, fMsg, MF_BYCOMMAND ) ;
			gdwlMenuFlag &= ~bflg ;
		}
		fMsg += 1 ;
		bflg = bflg << 1 ;
	}
}
/* ===================================================================== */

/* ===================================================================== */
/* -----------------------------------------------------------------------
	Add the File Names from the User's preferences buffer -
	Loaded from the ...INI file - to the 'File' Pop-up
	Menu section
   ----------------------------------------------------------------------- */
void AppendPref( HMENU hSMenu, LPSTR lpf )
{
	int		i, cyc;
	DWORD	cnt;
	LPSTR	lps;
	DWORDLONG	bflg;
	UINT	fMsg;

	bflg = M_Fil;
	fMsg = IDM_FILE1 ;	// Init IDM_FILE[n] - Max 15 reserved
	// NOTE: SHould be max 64 reserved
	if( lps = lpf )
	{
		cyc = 0;
		cnt = 0;
		while( i = lstrlen( (lps + cyc) ) )
		{
			if( !gfAddSep )
			{
				AppendMenu( hSMenu, MF_SEPARATOR, 0, 0 ) ;
				gfAddSep = TRUE;
			}
			AppendMenu( hSMenu,
				MF_ENABLED | MF_STRING,
				fMsg,
				(lps + cyc) );
			gdwlMenuFlag |= bflg;
			bflg = bflg << 1;
			fMsg += 1;
			cyc += i + 1;
			cnt++;
			if( cnt >= gdwMaxFiles )
				break;
		}
		gdwLastMax = cnt;	// Keep the COUNT added
	}
}

/* ===================================================================== */

/* ===================================================================== */
BOOL	InFileRange( UINT ui )
{
	BOOL	flg = FALSE;
	if( ( ui >= IDM_FILE1 ) &&
		( ui <= IDM_FILEMax ) )
	{
		flg = TRUE;
	}
	return flg;
}

/* -----------------------------------------------------------------------
	Handle 'File' POP-UP changes
	(a) Remove any existing file names
	(b) Add any new list of names
	(c) Check (or Uncheck) the loaded (not loaded) file
   ----------------------------------------------------------------------- */
BOOL FileCheck( HWND hwnd, UINT Flag, LPSTR lpFiles, UINT fc )
{
	HMENU	hSMenu = 0;
	HMENU	hMenu = 0;
	UINT	Chk;
	BOOL	flg = FALSE;
	if( InFileRange( fc ) )
		Chk = fc;
	else
		Chk = (UINT)-1;
	if( hMenu = GetMenu ( hwnd ) )
	{
		if( hSMenu = GetSubMenu( hMenu, FILE_MENU) )
		{
			if( gdwlMenuFlag )	/* If there are previous items */
				DeletePref( hSMenu ) ;	/* Remove them all NOW */
			AppendPref( hSMenu, lpFiles );		/* Add any new items */
			DrawMenuBar( (HWND) hMenu ) ;	/* Repaint the MENU BAR */
			if( InFileRange( Chk ) )
			{
				CheckMenuItem( hSMenu, Chk, /* CHECK/UNCHECK THE FIRST! */
					Flag | MF_BYCOMMAND );
			}
			guiChecked = Chk;
			flg = TRUE;
		}
	}
	return( flg );
}

/* ===================================================================== */


// **********************************************************
// November, 2000 - NEW FUNCTIONS USING LINKED LISTS
// =================================================
/* -----------------------------------------------------------------------
	Remove File Name from the 'File' menu pop-up
	for the MAX_FILS configured
   ----------------------------------------------------------------------- */
DWORD DeleteMRU( HMENU hSMenu )
{
	DWORD	cyc, cnt;
	DWORDLONG	bflg = M_Fil ;		/* Init BIT flag - Max 15 shifts left */
	UINT	      fMsg = IDM_FILE1 ;	/* Start IDM_FILE[n] ... */

   cnt = 0;
	for( cyc = 0; cyc < gdwMaxFiles; cyc++ )
	{
		if( gdwlMenuFlag & bflg )
		{
			DeleteMenu( hSMenu, fMsg, MF_BYCOMMAND );
			gdwlMenuFlag &= ~bflg;
         cnt++;
		}
		fMsg++;  // bump to next
		bflg = bflg << 1 ;
	}
   return cnt;
}

/* -----------------------------------------------------------------------
	Add the File Names from the User's preferences buffer -
	Loaded from the ...INI file - to the 'File' Pop-up
	Menu section
   ----------------------------------------------------------------------- */
DWORD AppendMRU( HMENU hSMenu, PLIST_ENTRY pHead )
{
	DWORD	      cnt;
	LPTSTR      lps;
	DWORDLONG	bflg;
	UINT	      fMsg;
   PLIST_ENTRY pNext;
   PMWL        pmwl;

	cnt  = 0;

	// NOTE: Should be upper max of say 64 or 128 max!!!
	if( !IsListEmpty(pHead) )
	{
   	bflg = M_Fil;
	   fMsg = IDM_FILE1 ;	// Init IDM_FILE[n] - Max 15 reserved
      Traverse_List( pHead, pNext )
		{
         pmwl = (PMWL)pNext;
         if( pmwl->wl_dwFlag & flg_DELETE )  // if it has any DELETE flags
            continue;

         lps  = &pmwl->wl_szFile[0];
			if( !gfAddSep )
			{
				AppendMenu( hSMenu, MF_SEPARATOR, 0, 0 ) ;
				gfAddSep = TRUE;
			}
			AppendMenu( hSMenu,
				MF_ENABLED | MF_STRING,
				fMsg,
				lps );
         if( pmwl->wl_dwFlag & flg_IsLoaded )
         {
            CheckMenuItem( hSMenu, fMsg, /* CHECK/UNCHECK THE FILE! */
               ( MF_BYCOMMAND | MF_CHECKED ) ); // show as loaded
         }
			gdwlMenuFlag |= bflg;
			bflg = bflg << 1;
			fMsg += 1;
			cnt++;
			if( cnt >= gdwMaxFiles )
				break;
		}

      // set the count ADDED to the DROPDOWN MENU ITEM
		gdwLastMax = cnt;	// Keep the COUNT added
      // =============================================

	}
   return cnt;
}

/* -----------------------------------------------------------------------
   DWORD SetFileMRU( HWND hwnd, PLIST_ENTRY pHead )

	Handle 'File' POP-UP changes
	(a) Remove any existing file names
	(b) Add any new list of names
	(c) Check (or Uncheck) the loaded (not loaded) file
   ----------------------------------------------------------------------- */
DWORD SetFileMRU( HWND hwnd, PLIST_ENTRY pHead )
{
   DWORD iRet   = 0;
	HMENU	hMenu, hSMenu;
	if( ( hMenu  = GetMenu( hwnd )                ) &&
       ( hSMenu = GetSubMenu( hMenu, FILE_MENU ) ) )
   {
      if( gdwlMenuFlag )	/* If there are previous items */
         DeleteMRU( hSMenu ) ;	/* Remove them all NOW */
      iRet = AppendMRU( hSMenu, pHead );
   }

   return iRet;
}

PMWL  UnloadMRU( HWND hMDI, LPTSTR lpFile )
{
   PMWL  pRet = NULL;
   PLIST_ENTRY pHead = &gsFileList;
   PLIST_ENTRY pNext;
   PMWL     pmwl;
   LPTSTR   lpf;

   if( ( hMDI ) ||
      ( lpFile && *lpFile ) )
   {
      Traverse_List( pHead, pNext )
      {
         pmwl = (PMWL)pNext;
         lpf  = &pmwl->wl_szFile[0];
         if( ( pmwl->wl_hMDI == hMDI ) ||
            ( lstrcmpi( lpFile, lpf ) == 0 ) )
         {
            // found it
            if( pmwl->wl_dwFlag & (flg_IsLoaded | flg_MDIOpen) )
            {
               pRet = pmwl;
               pmwl->wl_dwFlag &= ~(flg_IsLoaded | flg_MDIOpen);
            }
            break;
         }
      }
   }
   return pRet;
}

VOID	InitFileList2( HWND hMain, PLIST_ENTRY pHead )
{
   if( !IsListEmpty(pHead) )
   {
      SetFileMRU( hMain, pHead );
   }
}


// Initialize pull down menus.  Gray the File/SaveAs+Print
//  if there is no current MDI child window.  Also, gray the
//  palette options when appropriate (i.e. gray when non-palette
//  device, gray animate if one is being animated or if there
//  are no MDI child windows, gray restore if none are being
//  animated).  The "Window" menu will also be grayed if there
//  are no MDI children
// WM_INITMENUPOPUP.
// Parameters
// wParam 
// Handle to the drop-down menu or submenu. 
// lParam 
// The low-order word specifies the zero-based relative position of the
//    menu item that opens the drop-down menu or submenu.
// The high-order word indicates whether the drop-down menu is the window menu.
//    If the menu is the window menu, this parameter is TRUE; otherwise, it is FALSE.
// See DvView.h for
// Menu Defines for main menu.
// NOTE: MUST agree with RC file
// MENU_FILE - Zero relative position
//#define	FILE_MENU			0
//#define	EDIT_MENU			1
// ... etc.
/* =========================================================
IDR_FRAMEMENU MENU 
BEGIN
    POPUP "&File"
    BEGIN
        MENUITEM "&Open...\tCtrl+O",            IDM_OPEN
        MENUITEM "Open &MRU ...",               IDM_OPENMRU
        MENUITEM "Op&en All ...",               IDM_OPENAS
        MENUITEM "&Save...\tCtrl+S",            IDM_SAVE
        MENUITEM "Sa&ve as BMP...\tCtrl+B",     IDM_SAVEAS
        MENUITEM SEPARATOR
        MENUITEM "Print S&creen",               IDM_PRINTSCRN
        MENUITEM "Prin&t Per Options",          IDM_PPOPTIONS
        MENUITEM "Setup and &Print...\tCtrl+P", IDM_PRINT
        MENUITEM SEPARATOR
        MENUITEM "S&ave on Exit",               IDO_SAVEINI
        MENUITEM SEPARATOR
        MENUITEM "Exit &No Save",               IDM_EXITNOSAVE
        MENUITEM "Exit &With Save",             IDM_EXITWSAVE
        MENUITEM "E&xit\tAlt+X",                IDM_EXIT
    END
    POPUP "&Edit"
    BEGIN
        MENUITEM "&Copy Clip Region\tCtrl+C",   IDM_COPY
        MENUITEM "&Paste\tCtrl+V",              IDM_PASTE
        MENUITEM SEPARATOR
        MENUITEM "&Attributes ...",             IDM_IMAGEATT
        MENUITEM SEPARATOR
        MENUITEM "C&olor ...",                  IDM_BKCOLOR
        MENUITEM "&Font ...",                   IDM_DEFFONT
        MENUITEM SEPARATOR
        MENUITEM "&Duplicate ...",              IDM_DUPLICATE
        MENUITEM "&Edit ...",                   IDM_EDITBMP
        MENUITEM "Copy and Pa&ste",             IDM_COPYSAVE
        MENUITEM "&Restore Size",               IDM_RESTORE2
        MENUITEM SEPARATOR
        MENUITEM "Edit Clip List..."            IDM_CLIPLISTMAX
        POPUP "Clip List"
        BEGIN
             MENUITEM "Clip1"                   IDM_CLIPLIST
        END
        MENUITEM "C&lear Clip",                 IDM_CLEARCLIP
        MENUITEM "&Magnify",                    IDM_MAGNIFY
        MENUITEM "Add Swish",                   IDM_ADDSWISH
        MENUITEM "Render New",                  IDM_RENDERNEW
    END
    POPUP "&Palette"
    BEGIN
        MENUITEM "&DIB's",                      IDM_PALDIB
        MENUITEM "&System's",                   IDM_PALSYS
        MENUITEM "&Animate",                    IDM_PALANIMATE
        MENUITEM "&Restore",                    IDM_PALRESTORE
    END
    POPUP "&Options"
    BEGIN
        MENUITEM "&Display Options...",         IDM_OPTIONS
        MENUITEM "&JPEG Options...",            IDM_OPTION2
        MENUITEM "&General Options...",         IDM_OPTION3
        MENUITEM "&Edit MRU List...",           IDM_OPTION4
        MENUITEM "Magnify &Options...",         IDM_OPTION6
    END
    POPUP "&Capture"
    BEGIN
        MENUITEM "&Window",                     IDM_CAPTWINDOW
        MENUITEM "&Client Area",                IDM_CAPTCLIENT
        MENUITEM "&Desktop",                    IDM_CAPTFULLSCREEN
        MENUITEM SEPARATOR
        MENUITEM "&Hide Window on Capture",     IDM_CAPTUREHIDE, CHECKED
    END
    POPUP "&Window"
    BEGIN
        MENUITEM "&Tile All",                   IDM_WINDOWTILE
        MENUITEM "&Cascade All",                IDM_WINDOWCASCADE
        MENUITEM "R&estore All",                IDM_WINDOWRESTALL
        MENUITEM "&Restore Active",             IDM_WINDOWRESTORE
        MENUITEM "Close &All",                  IDM_WINDOWCLOSEALL
        MENUITEM SEPARATOR
        MENUITEM "Arrange &Icons",              IDM_WINDOWICONS
    END
    POPUP "&Help"
    BEGIN
        MENUITEM "&About",                      IDM_ABOUT
        MENUITEM "&Help",                       IDM_HELP
    END
END
 ================================= */
// EMI( IDM_WINDOWTILE, gnDIBsOpen );

/* -----------------------------------------------------------------------
   DWORD SetFileMRU( HWND hwnd, PLIST_ENTRY pHead )

	Handle 'Clip' POP-UP changes
	(a) Remove any existing file names
	(b) Add any new list of names
	(c) Check (or Uncheck) if on display now
typedef struct tagMENUITEMINFO {
  UINT    cbSize; 
  UINT    fMask; 
  UINT    fType; 
  UINT    fState; 
  UINT    wID; 
  HMENU   hSubMenu; 
  HBITMAP hbmpChecked; 
  HBITMAP hbmpUnchecked; 
  ULONG_PTR dwItemData; 
  LPTSTR  dwTypeData; 
  UINT    cch; 
  HBITMAP hbmpItem;
} MENUITEMINFO, *LPMENUITEMINFO; 

   ----------------------------------------------------------------------- */
static HMENU hmenuBar  = NULL; 
static HMENU hmenuPop  = NULL; 
static HMENU hmenuClip = NULL;
//#define  dbg_clip_menu  sprtf
#define  dbg_clip_menu

VOID GetClipMRU( HWND hWnd, HMENU hmenuPopup, UINT uPos )  // get sub-menu handles
{
   MENUITEMINFO mii;
   if( hmenuBar == NULL ) {
      // only need to do this ONCE
      hmenuBar = GetMenu(hWnd); 
      ZeroMemory( &mii, sizeof(MENUITEMINFO) );
      mii.cbSize = sizeof(MENUITEMINFO);
      mii.fMask = MIIM_SUBMENU;     // information to get 
      // BOOL GetMenuItemInfo( HMENU hMenu, UINT uItem,
      //                       BOOL fByPosition, LPMENUITEMINFO lpmii );
      GetMenuItemInfo(hmenuBar, EDIT_MENU, TRUE, &mii);  // byPosition = T
      hmenuPop = mii.hSubMenu;
      GetMenuItemInfo(hmenuPop, 14, TRUE, &mii);  // byPosition = T
      hmenuClip = mii.hSubMenu;
      dbg_clip_menu( "GetClipMRU: Recent %p is popup of %p sub, of main %p ...\n",
         hmenuClip, hmenuPop, hmenuBar );
   }
}

extern PRECT Get_Clip_Rect( UINT cmd );

VOID DeleteClipMRU( HMENU hmenu )
{
   MENUITEMINFO mii;
   UINT  ui;
   for( ui = IDM_CLIPLIST; ui < IDM_CLIPLISTMAX; ui++ )
   {
      ZeroMemory( &mii, sizeof(MENUITEMINFO) );
      mii.cbSize = sizeof(MENUITEMINFO);
      mii.fMask = MIIM_ID; //MIIM_TYPE;
      mii.wID = ui;
      if( GetMenuItemInfo(hmenu, ui, FALSE, &mii) )
      {
         // this is to be DELETED
         DeleteMenu( hmenu, ui, MF_BYCOMMAND );
      }
      else
         break;
   }
}
VOID AppendClipMRU( HMENU hmenu, PRECT prc, UINT ui, UINT num )
{
   PTSTR pstg = GetStgBuf();
   sprintf(pstg, "Clip %d = %d,%d,%d,%d", num, prc->left, prc->top,
      RECTWIDTH(prc), RECTHEIGHT(prc) );
   AppendMenu( hmenu, MF_STRING, ui, pstg ); 
}

VOID SetClipMRU( HWND hWndFrame, HMENU hmenuPopup, UINT uPos,
                LPDIBINFO	lpDIBInfo )  // get sub-menu handles
{
   UINT  cnt = 0;
   PRECT prc = Get_Clip_Rect(cnt);
   UINT  ui = IDM_CLIPLIST;   // start of LIST

   DeleteClipMRU( hmenuPopup ) ;	/* Remove them all NOW */

   if(prc && lpDIBInfo)
   {
      LONG cxDIB, cyDIB;
      if( lpDIBInfo->Options.bStretch2W ) {
	      RECT rcClient;
	      GetClientRect( lpDIBInfo->di_hwnd, &rcClient );
	      cxDIB = rcClient.right;
	      cyDIB = rcClient.bottom;
      } else {
	      cxDIB = lpDIBInfo->di_dwDIBWidth;
	      cyDIB = lpDIBInfo->di_dwDIBHeight;
      }

      do
      {
         cnt++;
         if(( prc->right < cxDIB )&&( prc->bottom < cyDIB ))
         {
            AppendClipMRU( hmenuPopup, prc, ui, cnt );
         }
         ui++;
      } while(( ui < IDM_CLIPLISTMAX) && ( prc = Get_Clip_Rect(cnt) ));
   }
   dbg_clip_menu( "SetClipMRU: popup is %p ...done %d...\n", hmenuPopup, cnt );
}


#define  mfEnable    ( MF_ENABLED )
#define  mfDisable   ( MF_DISABLED | MF_GRAYED )
#define  EMI(a,b) EnableMenuItem( hmenuPopup, a, MF_BYCOMMAND | (b ? mfEnable : mfDisable) )
#define  EMIH(a)  EMI(a,hCurWnd)
#define  CMI(a,b) CheckMenuItem( hmenuPopup, a, (mfEnable | (b ? MF_CHECKED : MF_UNCHECKED)) )

VOID SetNewMenuString( HMENU hmenu, UINT item, PTSTR pstg )
{
   MENUITEMINFO _mii;
   ZeroMemory(&_mii, sizeof(MENUITEMINFO));
   _mii.cbSize = sizeof(MENUITEMINFO);
   _mii.fMask = MIIM_STRING;
   if( GetMenuItemInfo( hmenu, item, FALSE, &_mii ) ) {
      _mii.dwTypeData = pstg;
      SetMenuItemInfo( hmenu, item, FALSE, &_mii );
   }
}
// SMIS( IDM_ADDSWISH, "New String" );
#define  SMIS(a,b) SetNewMenuString(hmenuPopup,a,b)

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : Frm_WM_INITMENUPOPUP
// Return type: long 
// Arguments  : HWND hWnd
//            : UINT message
//            : WPARAM wParam
//            : LPARAM lParam
// Description: Initialize the particular menu on WM_INITMENUPOPUP
//              
///////////////////////////////////////////////////////////////////////////////
long Frm_WM_INITMENUPOPUP(HWND hWnd, UINT message,
						  WPARAM wParam, LPARAM lParam )
{
	long	   pret = 1;
	HWND	   hCurWnd = NULL;
	HMENU	   hmenuPopup;
	UINT	   uPos;
	BOOL	   fSystemMenu;
   HGLOBAL  hDIBInfo;
   PDI      lpDIBInfo = NULL;
   BOOL     bEnab, bEna2, bEnaClip;
   RECT     rcWindow, rc;
   BOOL     bGotMDI = FALSE;
   BOOL     bIsZoom = FALSE;
//   POINT    pt;

   bEnaClip = FALSE;
	fSystemMenu = (BOOL) HIWORD(lParam); // window menu flag 
   if( !fSystemMenu )
   {
      // only if NOT the "windows" menu
   	pret = 0;
   	hmenuPopup  = (HMENU) wParam;         // handle of submenu
	   uPos        = (UINT) LOWORD(lParam);        // submenu item position
	   hCurWnd     = GetCurrentMDIWnd ();   // get active MDI child window

      if( hCurWnd )
      {
         bIsZoom = IsZoomed(hCurWnd);
         if(bIsZoom)
         {
            if(uPos)
               uPos--;  // zero becomes system memu
         }
         bGotMDI = Get_DIB_Info( hCurWnd, &hDIBInfo, &lpDIBInfo );
         if(bGotMDI)
         {
            if( !IsRectEmpty( &lpDIBInfo->rcClip ) )
               bEnaClip = TRUE;
         }
      }

      dbg_clip_menu( "INITMENUPOPUP: %p is popup at position %d ...\n",
         hmenuPopup, uPos );

//         sprtf( "WM_INITMENUPOPUP at adjusted position %d."MEOR, uPos );
//      else
//      {
//         sprtf( "WM_INITMENUPOPUP at position %d."MEOR, uPos );
//      }

      switch( uPos )
      {
      case FILE_MENU:
         {
            EMIH( IDM_SAVE   );
            EMIH( IDM_SAVEAS );
            EMIH( IDM_PRINT  );
#ifdef	TICKINI
            bEnab = (BOOL)(gfChgAll | GotAChange());
            EMI( IDM_EXITNOSAVE, bEnab );    // enabled only *IFF* got-an-INI-change
            CMI( IDM_EXITWSAVE,  gfChgAll );
            //	{ &fSavINI, &fChgSI, IDO_SAVEINI, SI_Default, 0, 0, 0 },
	         CMI( IDO_SAVEINI,    gfSavINI );
#endif	/* TICKINI */
         }
         break;

      case EDIT_MENU:
         {
            // populate Recent Files sub-menu
            GetClipMRU( hWnd, hmenuPopup, uPos );  // get sub-menu handles

            // ==========================================================
            // MENUITEM "&Copy\tCtrl+C",   IDM_COPY
            // MENUITEM "&Paste\tCtrl+V",  IDM_PASTE
            if(bGotMDI)
            {
               EMI( IDM_COPY, TRUE ); // if there is a MDI child open
               SMIS( IDM_COPY,
                  (bEnaClip ? "Copy Clip\tCtrl+C" : "Copy All\tCtrl+C" ) );
            }
            else
            {
               EMI( IDM_COPY, FALSE ); // there is NO MDI child!!!
            }
            bEnab = IsClipboardFormatAvailable( CF_DIB ) ||
		         IsClipboardFormatAvailable( CF_BITMAP );
         	EMI( IDM_PASTE, bEnab );
            // ==========================================================

            bEnab = FALSE;
            bEna2 = FALSE;
            if( bGotMDI )
            {
               rcWindow.left = rcWindow.top = 0;
               rcWindow.right  = (int) lpDIBInfo->di_dwDIBWidth;
	            rcWindow.bottom = (int) lpDIBInfo->di_dwDIBHeight;
               //AdjustWindowRect( &rcWindow,
               //   ( WS_CHILD | WS_SYSMENU | WS_CAPTION |
			      //      WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX ),
               //      FALSE );
               GetClientRect(hCurWnd,&rc);
               if( ( rc.right  == rcWindow.right  ) &&
                   ( rc.bottom == rcWindow.bottom ) )
                  bEna2 = FALSE;
               else
                  bEna2 = TRUE;

               // Seek flag fg_DnThread, or better
               if( lpDIBInfo->di_dwDnCount & fg_DnMyWM )   // when thread count completes
                  bEnab = TRUE;

            }

            // only if the thread has completed its job
            EMI( IDM_IMAGEATT,  bEnab );
            EMI( IDM_DUPLICATE, bEnab );

            EMIH( IDM_EDITBMP   );

            EMI( IDM_COPYSAVE, bEnaClip ); // if there is a CLIP region
            EMI( IDM_RESTORE2, bEna2  );
            SMIS( IDM_ADDSWISH, GetSwishMenu() );
            CMI( IDM_ADDSWISH,
               (lpDIBInfo ? Got_Swish( &lpDIBInfo->sSwish ) : FALSE) ); // if there is a CLIP region
            EMI( IDM_RENDERNEW,
               (lpDIBInfo ? Got_Swish_or_Magnify( &lpDIBInfo->sSwish, &lpDIBInfo->sMagnif ) : FALSE ));
	         CMI( IDM_MAGNIFY, gi_MouseMagnify );
            EMI( IDM_CLIPLISTMAX, FALSE );   // until dialog done

         }
         break;


      case PALETTE_MENU:
         {
         	//EMIH( IDM_PALDIB );
            bEnab = FALSE;
            if( bGotMDI )
            {
               // Seek flag fg_DnThread, or better
               if( lpDIBInfo->di_dwDnCount & fg_DnMyWM )   // when thread count completes
                  bEnab = TRUE;
            }
         	EMI( IDM_PALDIB, bEnab   );
         	EMI( IDM_PALSYS, bPalDev );   // if PALETTE device, not 24-bit windows
            bEna2 = ((bPalDev && !hWndAnimate && bEnab) ? TRUE : FALSE);
	         EMI( IDM_PALANIMATE, bEna2 );
	         EMI( IDM_PALRESTORE, bEna2 );
         }
         break;

      case OPTION_MENU: // position of option menu
         {
            // FIX980430 - Leave this ALWAYS enabled
            //	EnableMenuItem( ghFrameMenu,
            //		IDM_OPTIONS,
            //		MF_BYCOMMAND |
            //		(hCurWnd ? MF_ENABLED : MF_GRAYED) );
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
         	if( fDnLibLoad )
		         EMI( IDM_OPTION2, hLibInst );
	         else
		         EMI( IDM_OPTION2, 1        );
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

         }
         break;

      case CAPTURE_MENU:
         {
#ifndef   ADDSELWIN2
            // ***TBD*** These two NEED FIXING - they do not function correctly
//        MENUITEM "&Window",                     IDM_CAPTWINDOW = SelWin
//        MENUITEM "&Client Area",                IDM_CAPTCLIENT
            EMI( IDM_CAPTWINDOW, FALSE );
            EMI( IDM_CAPTCLIENT, FALSE );
#endif   // #ifndef   ADDSELWIN2

//        MENUITEM "&Hide Window on Capture",     IDM_CAPTUREHIDE, CHECKED
            CMI( IDM_CAPTUREHIDE, gbHide );
         }
         break;

      case WINDOW_MENU: // position of window menu
         {
            EMI( IDM_WINDOWTILE,     gnDIBsOpen );
            EMI( IDM_WINDOWCASCADE,  gnDIBsOpen );
            EMI( IDM_WINDOWRESTALL,  gnDIBsOpen );
            EMI( IDM_WINDOWRESTORE,  gnDIBsOpen );
            EMI( IDM_WINDOWCLOSEALL, gnDIBsOpen );
            EMI( IDM_WINDOWICONS,    gnDIBsOpen );
         }
         break;

      case HELP_MENU:
         {

         }
         break;

      case 13: // if it is 'full size', ie IsZoomed ...
      case 14: // else the POPUP is the 14th item in the LIST
         // This the CLIP LIST popup
         SetClipMRU( hWnd, hmenuPopup, uPos, lpDIBInfo );  // get sub-menu handles
         break;

      }

   }

   if( bGotMDI )
      Release_DIB_Info( hCurWnd, &hDIBInfo, &lpDIBInfo );

	return( pret );

}

// EOF - DvMenu,c

