// DvContext.c

#include "dv.h"
#include <windowsx.h>

#define  mfEnable    ( MF_ENABLED )
#define  mfDisable   ( MF_DISABLED | MF_GRAYED )
#define  AM(a,b)  if(AppendMenu(hMenu,uFlags,a,b))iCnt++
#define  AMD(a,b)  if(AppendMenu(hMenu,(uFlags | MF_DISABLED | MF_GRAYED),a,b))iCnt++
#define  CAM(a,b) CheckMenuItem( hMenu, a, (mfEnable | (b ? MF_CHECKED : MF_UNCHECKED)) )
#define  EMI(a,b) EnableMenuItem( hMenu, a, MF_BYCOMMAND | (b ? mfEnable : mfDisable) )

#define  AM_SEP if(iCnt) {\
   AppendMenu(hMenu, MF_SEPARATOR, 0, 0    );\
   iCnt = 0;\
}


//VOID  Child_WM_RBUTTONDOWN - NOW DoContextMenu(...)
VOID  DoContextMenu( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
   POINT pnt;
   HMENU hMenu;
   BOOL  bCB = FALSE;
//   HWND  hWnd = ghwndFrame;   // get PARENT handle
   HWND  hWnd = hwnd;   // main handle
//   PREDOC   predoc = GetCurrentMDIStr();  // get currect active document (if any)
   HWND  hDIBWnd = GetCurrentMDIWnd();
   HANDLE hDIBInfo;
   LPDIBINFO lpDIBInfo;
   BOOL  bGotDI = FALSE;
   PTSTR       psms = "";
   BOOL bGotSM = FALSE;
   BOOL bGotRect = FALSE;

   pnt.x = LOWORD(lParam);
   pnt.y = HIWORD(lParam);
   // if( hDIBWnd ) {
      //ClientToScreen(hWnd, &pnt); // WHY THIS???
      //ClientToScreen(hDIBWnd, &pnt);   // why this???
   // }
   sprtf( "CONTEXT MENU: Right button down at %d,%d"MEOR,
         GET_X_LPARAM(lParam),
         GET_Y_LPARAM(lParam) );

   hMenu = CreatePopupMenu(); // create a POPUP Menu
   if(hMenu)  
   {
      UINT  uOpts, uFlags;
      INT   iCnt = 0;
      uOpts = TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON;  // allow L/R click/select
      // uOpts = TPM_CENTERALIGN | TPM_VCENTERALIGN; // options
      // we COULD fiddle with these position options, but it seems the
      // windows UI already does some of this so forget it for now ...
      // GetClientRect(hwnd, &rc);
     	// Add the cut, copy, paste verbs enabled or disable accordingly

      // note:
      uFlags = MF_ENABLED | MF_STRING | MF_BYCOMMAND;
      //     =          0 | MF_STRING | MF_BYCOMMAND;

      iCnt = 0;
      bCB = (IsClipboardFormatAvailable( CF_DIB ) ||
         IsClipboardFormatAvailable( CF_BITMAP ));
      if(hDIBWnd)
         bGotDI = Get_DIB_Info( hDIBWnd, &hDIBInfo, &lpDIBInfo );
      if( bGotDI )
      {
         bGotSM = Got_Swish_or_Magnify( &lpDIBInfo->sSwish, &lpDIBInfo->sMagnif );
         bGotRect = IsRectEmpty( &lpDIBInfo->rcClip ) ? FALSE : TRUE;
         psms = GetSwishMenu();
      }

      AM( IDM_OPEN, "Open...");
      if(hDIBWnd)
         AM( IDM_SAVE, "Save...");

      AM_SEP;

      AM( IDM_COPY,
         (bGotRect ? "Copy Clip" : "Copy ALL") );
      AM( IDM_PASTE,
         (bCB ? "Paste" : "Paste (Empty)") );
      EMI( IDM_PASTE, bCB );

      AM_SEP;

      //if( VALIDPD(predoc) )
      if( bGotDI )
      {
         if( bGotSM || bGotRect )
            AM( IDM_CLEARCLIP, "Clear Clip" );
         if( bGotRect ) {
            AM( IDM_ADDSWISH, psms );
            CAM( IDM_ADDSWISH, Got_Swish(&lpDIBInfo->sSwish) );
            if( strcmp(psms, Menu_Swish()) )
               AM( IDM_PAINTSWISH, Menu_Swish());
            if( strcmp(psms, Menu_Oval()) )
               AM( IDM_PAINTOVAL, Menu_Oval());
            if( strcmp(psms, Menu_Square()) )
               AM( IDM_PAINTSQUARE, Menu_Square());
         }

         AM( IDM_MAGNIFY, "Magnify" );
         CAM( IDM_MAGNIFY, gi_MouseMagnify );

         if( bGotSM )
            AM( IDM_RENDERNEW, "Render New" );

         AM( IDM_REPAINT, "Repaint" );

      }
      else
      {
         // no active MID child == no image on display, so
         AM(IDM_OPEN, "View an Image File (Open...)");
      }

      AM_SEP;
      // bottom item = EXIT
      AM( IDM_EXITNOSAVE, "Quit, NO save."  );
      AM( IDM_EXITWSAVE,  "Exit with Save"  );
      AM( IDM_EXIT,       "Exit (Alt+X)"    );

      if( bGotDI )
         Release_DIB_Info( hWnd, &hDIBInfo, &lpDIBInfo );

      TrackPopupMenu( hMenu,         // handle to shortcut menu
         uOpts,   // TPM_CENTERALIGN | TPM_VCENTERALIGN, // options
         pnt.x,   // horizontal position
         pnt.y,   // vertical position
         0,       // reserved, must be zero
         hWnd,    // handle to owner window
         0 );     // ignored

      DestroyMenu(hMenu);

   } else {
      chkme( "ERROR CONTEXT MENU: FAILED TO GET MENU HANDLE!!!"MEOR );
   }

}

// eof - DvContext.c
