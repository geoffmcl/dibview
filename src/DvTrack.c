// DvTrack.c

#include "dv.h"
extern VOID SetMouseMessage( HWND hWnd, INT xPos, INT yPos, DV_CURPOS cp, PRECT prcClip );

#define  dbg_scroll  sprtf

DV_CURPOS Clippos = DV_ON_NONE;

HCURSOR  hCurWE = 0;
HCURSOR  hCurNS = 0;
HCURSOR  hCurNESW = 0;  //   / for TOPRIGHT, and BOTTLEFT
HCURSOR  hCurNWSE = 0;  //   \ for TOPLEFT, and BOTTRIGHT
HCURSOR  hCurALL = 0;
HCURSOR  hCurArrow = 0;

VOID Dv_Set_Scroll_Origin( HWND hWnd, HDC hdc, PPOINT ppt, PPOINT psave )
{
   ppt->x = GetScrollPos( hWnd, SB_HORZ );
   ppt->y = GetScrollPos( hWnd, SB_VERT );
	SetWindowOrgEx( hdc, ppt->x, ppt->y, psave );
}
VOID Dv_Reset_Scroll_Origin( HDC hdc, PPOINT ppt )
{
	SetWindowOrgEx( hdc, ppt->x, ppt->y, NULL );
}

//---------------------------------------------------------------------
// Function:   NormalizeRect
// Purpose:    Insure that the upper/left corner of the rectangle is
//             kept in rect.top/rect.right.  Swaps around coordinates
//             in the rectangle to make sure this is true.
//             Code was stolen verbatim from ShowDIB.
// Parms:      lpRect == Pointer to RECT to normalize.
// History:   Date      Reason
//             ????     Created
//---------------------------------------------------------------------
int NormalizeRect (LPRECT lpRect)
{
   int   ret = 0;
   if (lpRect->right < lpRect->left) {
      SWAP (lpRect->right,lpRect->left);
      ret |= SWAP_LR;
   }

   if (lpRect->bottom < lpRect->top) {
      SWAP (lpRect->bottom,lpRect->top);
      ret |= SWAP_TB;
   }
   return ret;
}

//---------------------------------------------------------------------
// Function:   TrackMouse
// Purpose:    This routine is called when the left mouse button is
//             held down.  It will continuously draw a rectangle
//             showing where the user has selected for cutting to the
//             clipboard.  When this routine is called, lpClipRect's
//             top/left should point at the point in the client area
//             where the left mouse button was hit.  It will return the
//             full sized rectangle the user selected.  Never allow the
//             rubber band to extend beyond the DIB's margins.
//
//             Code was stolen almost verbatim from ShowDIB.
//
// Parms:      hWnd       == Handle to this MDI child window.
//             lpClipRect == Rectangle enclosed by tracking box.
//             cxDIB      == Width of DIB.  Won't allow tracking box
//                             to go beyond the width.
//             cyDIB      == Height of DIB.  Won't allow tracking box
//                             to go beyond the height.
//
// Caller:	void ChildWndLeftButton( HWND hWnd, int x, int y)
//			Caller: WM_LBUTTONDOWN case in CHILDWNDPROC
//
// History:   Date      Reason
//             ????     Created
//             9/1/91   Added cxDIB/cyDIB to not allow garbage
//                        to be pasted to the clipboard.
//---------------------------------------------------------------------

void TrackMouse( HWND hWnd, LPRECT lpClipRect, int cxDIB, int cyDIB)
{

	HDC   hDC;
	MSG   msg;
	POINT ptOrigin, ptStart, ptSave;
	RECT  rcClient, rcLast;

	hDC = GetDC (hWnd);
	if( !hDC )
		return;

	SetCapture( hWnd );

	GetClientRect( hWnd, &rcClient );
   rcLast = *lpClipRect;

	// Get mouse coordinates relative to origin of DIB.  Then
	//  setup the clip rectangle accordingly (it already should
	//  contain the starting point in its top/left).
   Dv_Set_Scroll_Origin( hWnd, hDC, &ptOrigin, &ptSave );
	// FIX981228 - This looks WRONG
	//lpClipRect->top   += ptOrigin.x;
	//lpClipRect->left  += ptOrigin.y;
	// Lets SWITCH IT and SEE
	lpClipRect->left  += ptOrigin.x;
	lpClipRect->top   += ptOrigin.y;
	lpClipRect->right  = lpClipRect->left;
	lpClipRect->bottom = lpClipRect->top;

	ptStart.x          = lpClipRect->left;    // Need to remember the
	ptStart.y          = lpClipRect->top;     //  starting point.

   dbg_scroll( "DEBUGSCROLL: Due to scroll RECT %s adjusted to %s ...(%d,%d)\n",
      Rect2Stg( &rcLast ), Rect2Stg(lpClipRect), ptOrigin.x, ptOrigin.y );

	// ADD THE FIRST NEW CLIP, but will do nothing, since no width/height yet
   DrawSelect( hDC, lpClipRect );

#ifdef	SHOWCLIP
	ShowBegin( lpClipRect, &ptOrigin, &ptStart, &rcClient, cxDIB, cyDIB );
#endif	// SHOWCLIP
	rcLast = *lpClipRect;
	// Eat mouse messages until a WM_LBUTTONUP is encountered.
	// Meanwhile continue to draw a rubberbanding rectangle
	// and display it's dimensions
	for( ;; )
	{

		// The WaitMessage function yields control to other threads
		// when a thread has no other messages in its message queue.
		// The WaitMessage function suspends the thread and does not
		// return until a new message is placed in the thread's
		// message queue. 
		WaitMessage();

		// PeekMessage Return Values
		// If a message is available, the return value is nonzero.
		if( PeekMessage( &msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) )
		{
			// Erase the OLD rectangle
         DrawSelect( hDC, lpClipRect );

			rcLast = *lpClipRect;

			// Determine new coordinates.
			lpClipRect->left   = ptStart.x;
			lpClipRect->top    = ptStart.y;

			lpClipRect->right  = LOWORD (msg.lParam) + ptOrigin.x;
			lpClipRect->bottom = HIWORD (msg.lParam) + ptOrigin.y;

			NormalizeRect( lpClipRect );

			// Keep the rectangle within the bounds of the DIB.
			lpClipRect->left   = MAX(lpClipRect->left,   0);
			lpClipRect->top    = MAX(lpClipRect->top,    0);
			lpClipRect->right  = MAX(lpClipRect->right,  0);
			lpClipRect->bottom = MAX(lpClipRect->bottom, 0);

			lpClipRect->left   = MIN(lpClipRect->left,   cxDIB);
			lpClipRect->top    = MIN(lpClipRect->top,    cyDIB);
			lpClipRect->right  = MIN(lpClipRect->right,  cxDIB);
			lpClipRect->bottom = MIN(lpClipRect->bottom, cyDIB);

			// Draw the NEW rectangle.
			DrawSelect( hDC, lpClipRect );

#if	(defined( SHOWWM ) || defined( DIAGWM2 ))
//typedef struct tagMSG {     // msg
//	HWND   hwnd;
//	UINT   message; 
//  WPARAM wParam;
//	LPARAM lParam;
//	DWORD  time;
//	POINT  pt;
//} MSG;
			DiagChildMsg( msg.hwnd,		// HWND hWnd,
				msg.message,			// UINT message,
				msg.wParam,				// WPARAM wParam,
				msg.lParam );			// LPARAM lParam
#endif	// SHOWWM or DIAGWM2

			// If the button is released, quit.
			if( msg.message == WM_LBUTTONUP )
			{
#ifdef	SHOWCLIP
				ShowFinal( lpClipRect,
					&ptOrigin, &ptStart,
					&rcClient, cxDIB, cyDIB );
#endif	// SHOWCLIP
				// we have completed tracking
				break;
			}
			else
			{
#ifdef	SHOWCLIP
//				if( ( rcLast.left != lpClipRect->left ) ||
				DiagDrawSel( lpClipRect, &ptStart );
#endif	// SHOWCLIP
			}
		}
		else
			continue;

	}	// FOREVER

	// Clean up.
	ReleaseCapture();

   Dv_Reset_Scroll_Origin( hDC, &ptSave );

	ReleaseDC( hWnd, hDC );

} // end - void TrackMouse (HWND hWnd, LPRECT lpClipRect, int cxDIB, int cyDIB)

void TrackMouse2 (HWND hWnd, LPRECT lpClipRect, int cxDIB, int cyDIB,
                  LPDIBINFO lpDIBInfo )
{

	HDC   hDC;
	MSG   msg;
	POINT ptOrigin, ptStart, ptBgn;
	RECT  rcClient, rcLast;
   INT   x, y, swap;
   DV_CURPOS  cp = Clippos;
   INT   xd,yd;
   BOOL  bGotSw;

	hDC = GetDC (hWnd);
	if( !hDC )
		return;

   // extract current mouse position,
   // and this is also where the FINAL Clip is returned
   x = lpClipRect->left;
   y = lpClipRect->top;
   ptBgn.x = x;
   ptBgn.y = y;

   rcLast = *lpClipRect; // extract the ORIGINAL x,y mouse position

	SetCapture( hWnd );
   SetCursorType( cp );

	GetClientRect( hWnd, &rcClient );

	// Get mouse coordinates relative to origin of DIB.  Then
	//  setup the clip rectangle accordingly
	ptOrigin.x         = GetScrollPos( hWnd, SB_HORZ );
	ptOrigin.y         = GetScrollPos( hWnd, SB_VERT );
	//lpClipRect->left  += ptOrigin.x;
	//lpClipRect->top   += ptOrigin.y;
	//lpClipRect->right  = lpClipRect->left;
	//lpClipRect->bottom = lpClipRect->top;
   *lpClipRect = lpDIBInfo->rcClip; // establish the ORIGINAL clip

	ptStart.x          = lpClipRect->left;    // Need to remember the
	ptStart.y          = lpClipRect->top;     //  starting point.

   // Display the starting coordinates.
	SetWindowOrgEx( hDC, ptOrigin.x, ptOrigin.y, NULL );

   // REMOVE the OLD clip region
   // DrawSelect( hDC, lpClipRect );
   bGotSw = Got_Swish( &lpDIBInfo->sSwish );
   DC_ClearClip( hDC, &lpDIBInfo->rcClip, lpDIBInfo ); // includes DrawSelect( ... );

#ifdef	SHOWCLIP
	ShowBegin( lpClipRect, &ptOrigin, &ptStart, &rcClient, cxDIB, cyDIB );
#endif	// SHOWCLIP

// #if   0     // this now appears incorrect
   // make NEW clip region ... will probably be the same +/- 1 pixel
   switch( cp ) // = Clippos
   {
   case DV_ON_LEFT:
      lpClipRect->left = x + ptOrigin.x;
      break;
   case DV_ON_RIGHT:
      lpClipRect->right = x + ptOrigin.x;
      break;
   case DV_ON_TOP:
      lpClipRect->top = y + ptOrigin.y;
      break;
   case DV_ON_BOTTOM:
      lpClipRect->bottom = y + ptOrigin.y;
      break;
   case DV_ON_TOPLEFT:
      lpClipRect->left = x + ptOrigin.x;
      lpClipRect->top = y + ptOrigin.y;
      break;
   case DV_ON_TOPRIGHT:
      lpClipRect->top = y + ptOrigin.y;
      lpClipRect->right = x + ptOrigin.x;
      break;
   case DV_ON_BOTTLEFT:
      lpClipRect->bottom = y + ptOrigin.y;
      lpClipRect->left = x + ptOrigin.x;
      break;
   case DV_ON_BOTTRIGHT:
      lpClipRect->bottom = y + ptOrigin.y;
      lpClipRect->right = x + ptOrigin.x;
      break;
   case DV_ON_ALL:
      break;
   }

   // DRAW the NEW clip region
   DrawSelect( hDC, lpClipRect );
// #endif   // 0 ===============================================

	rcLast = *lpClipRect;
	// Eat mouse messages until a WM_LBUTTONUP is encountered.
	// Meanwhile continue to draw a rubberbanding rectangle
	// and display it's dimensions
	for( ;; )
	{
		// The WaitMessage function yields control to other threads
		// when a thread has no other messages in its message queue.
		// The WaitMessage function suspends the thread and does not
		// return until a new message is placed in the thread's
		// message queue. 
		WaitMessage();

		// PeekMessage Return Values
		// If a message is available, the return value is nonzero.
		if( PeekMessage( &msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) )
		{
			// Erase the OLD rectangle
         DrawSelect( hDC, lpClipRect );

         x = LOWORD (msg.lParam);
         y = HIWORD (msg.lParam);
         xd = ptBgn.x - x;
         yd = ptBgn.y - y;

			rcLast = *lpClipRect;

			// Determine new coordinates.
         // there is a PROBLEM when crossing over
         // fix = cancel NormalizeRect( lpClipRect );
         // and do normailisation case by case
         switch( cp ) // = Clippos
         {
         case DV_ON_LEFT:
            lpClipRect->left = x + ptOrigin.x;
            if( lpClipRect->right < lpClipRect->left ) {
               SWAP (lpClipRect->right,lpClipRect->left);
               cp = DV_ON_RIGHT; // switch sides
            }
            break;
         case DV_ON_RIGHT:
            lpClipRect->right = x + ptOrigin.x;
            if( lpClipRect->right < lpClipRect->left ) {
               SWAP (lpClipRect->right,lpClipRect->left);
               cp = DV_ON_LEFT;
            }
            break;
         case DV_ON_TOP:
            lpClipRect->top = y + ptOrigin.y;
            if (lpClipRect->bottom < lpClipRect->top) {
               SWAP (lpClipRect->bottom,lpClipRect->top);
               cp = DV_ON_BOTTOM;
            }
            break;
         case DV_ON_BOTTOM:
            lpClipRect->bottom = y + ptOrigin.y;
            if (lpClipRect->bottom < lpClipRect->top) {
               SWAP (lpClipRect->bottom,lpClipRect->top);
               cp = DV_ON_TOP;
            }
            break;
         case DV_ON_TOPLEFT:
            lpClipRect->left = x + ptOrigin.x;
            lpClipRect->top = y + ptOrigin.y;
            swap = NormalizeRect(lpClipRect);
            if( swap ) {
               if( swap == (SWAP_LR & SWAP_TB) ) {
                  cp = DV_ON_BOTTRIGHT; // swapped BOTH
               } else if ( swap & SWAP_LR ) {
                  cp = DV_ON_TOPRIGHT;
               } else if ( swap & SWAP_TB ) {
                  cp = DV_ON_BOTTLEFT;
               }
            }
            break;
         case DV_ON_TOPRIGHT:
            lpClipRect->top = y + ptOrigin.y;
            lpClipRect->right = x + ptOrigin.x;
            swap = NormalizeRect(lpClipRect);
            if( swap ) {
               if(swap == (SWAP_LR & SWAP_TB)) {
                  cp = DV_ON_BOTTLEFT; // swapped BOTH
               } else if( swap & SWAP_LR ) {
                  cp = DV_ON_TOPLEFT;
               } else if( swap & SWAP_TB ) {
                  cp = DV_ON_BOTTRIGHT;
               }
            }
            break;
         case DV_ON_BOTTLEFT:
            lpClipRect->bottom = y + ptOrigin.y;
            lpClipRect->left = x + ptOrigin.x;
            swap = NormalizeRect(lpClipRect);
            if( swap ) {
               if(swap == (SWAP_LR & SWAP_TB)) {
                  cp = DV_ON_TOPRIGHT; // swapped BOTH
               } else if( swap & SWAP_LR ) {
                  cp = DV_ON_BOTTRIGHT;
               } else if( swap & SWAP_TB ) {
                  cp = DV_ON_TOPLEFT;
               }
            }
            break;
         case DV_ON_BOTTRIGHT:
            lpClipRect->bottom = y + ptOrigin.y;
            lpClipRect->right = x + ptOrigin.x;
            swap = NormalizeRect(lpClipRect);
            if( swap ) {
               if(swap == (SWAP_LR & SWAP_TB)) {
                  cp = DV_ON_TOPLEFT; // swapped BOTH
               } else if( swap & SWAP_LR ) {
                  cp = DV_ON_BOTTLEFT;
               } else if( swap & SWAP_TB ) {
                  cp = DV_ON_TOPRIGHT;
               }
            }
            break;
         case DV_ON_ALL:
            // move the whole thing, potentially
            {
               RECT rcNew, rcCopy;
               rcNew = *lpClipRect; // get current
               rcNew.left   -= xd;
               rcNew.right  -= xd;
               rcNew.top    -= yd;
               rcNew.bottom -= yd;
               rcCopy = rcNew;   // get COPY
               // max limit fixes
			      rcNew.left   = MAX(rcNew.left,   0);
               rcNew.top    = MAX(rcNew.top,    0);
			      rcNew.right  = MAX(rcNew.right,  0);
			      rcNew.bottom = MAX(rcNew.bottom, 0);
               rcNew.left   = MIN(rcNew.left,   cxDIB);
               rcNew.top    = MIN(rcNew.top,    cyDIB);
               rcNew.right  = MIN(rcNew.right,  cxDIB);
               rcNew.bottom = MIN(rcNew.bottom, cyDIB);
               if(( rcCopy.left   == rcNew.left   ) &&
                  ( rcCopy.bottom == rcNew.bottom ) &&
                  ( rcCopy.top    == rcNew.top    ) &&
                  ( rcCopy.right  == rcNew.right  ) ) {
                     // only UPDATE if NOT going OUT-OF-LIMITS
                     lpClipRect->left   -= xd;
                     lpClipRect->right  -= xd;
                     lpClipRect->top    -= yd;
                     lpClipRect->bottom -= yd;
               }
            }
            break;
         }

			// Keep the rectangle within the bounds of the DIB.
			lpClipRect->left   = MAX(lpClipRect->left,   0);
			lpClipRect->top    = MAX(lpClipRect->top,    0);
			lpClipRect->right  = MAX(lpClipRect->right,  0);
			lpClipRect->bottom = MAX(lpClipRect->bottom, 0);

			lpClipRect->left   = MIN(lpClipRect->left,   cxDIB);
			lpClipRect->top    = MIN(lpClipRect->top,    cyDIB);
			lpClipRect->right  = MIN(lpClipRect->right,  cxDIB);
			lpClipRect->bottom = MIN(lpClipRect->bottom, cyDIB);

			// Draw the NEW rectangle.
			DrawSelect( hDC, lpClipRect );
         // update start of move point
         ptBgn.x = x;
         ptBgn.y = y;
         SetMouseMessage( hWnd, x, y, cp, lpClipRect );
			// If the button is released, quit.
			if( msg.message == WM_LBUTTONUP ) {
#ifdef	SHOWCLIP
				ShowFinal( lpClipRect,
					&ptOrigin, &ptStart,
					&rcClient, cxDIB, cyDIB );
#endif	// SHOWCLIP
				// we have completed tracking
            SetCursorType( DV_ON_NONE );
				break;   // EXIT forever
			}
			else
			{
#ifdef	SHOWCLIP
//				if( ( rcLast.left != lpClipRect->left ) ||
				DiagDrawSel( lpClipRect, &ptStart );
#endif	// SHOWCLIP
            SetCursorType( cp );
			}
		}
		else
			continue;

	}	// FOREVER

	// Clean up.
	ReleaseCapture();

	ReleaseDC( hWnd, hDC );

} // end - void TrackMouse2 (HWND hWnd, LPRECT lpClipRect, int cxDIB, int cyDIB,
  //                        LPDIBINFO	lpDIBInfo)

// ==============================================================
// changing cursor style

// initialize the cursor from resources
BOOL  InitCursors ( VOID )
{
   hCurWE = LoadCursor( NULL, IDC_SIZEWE ); // = <=> type
   hCurNS = LoadCursor( NULL, IDC_SIZENS );
   hCurNESW = LoadCursor( NULL, IDC_SIZENESW ); //   / for TOPRIGHT, and BOTTLEFT
   hCurNWSE = LoadCursor( NULL, IDC_SIZENWSE ); //   \ for TOPLEFT, and BOTTRIGHT
   hCurALL = LoadCursor( NULL, IDC_SIZEALL );
   hCurArrow = LoadCursor( NULL, IDC_ARROW );
   Clippos = DV_ON_NONE;

   if( !hCurWE || !hCurNS || !hCurArrow || !hCurNESW || !hCurNWSE || !hCurALL ) {
		MessageBox( NULL,
			"RESOURCE LOAD ERROR",
			"ERROR: Unable to load the one or more\r\n"
         "of the standard system cursors\r\n"
			"On OK, will abort application!",
			( MB_ICONINFORMATION | MB_OK ) );
      return FALSE;
   }

   return TRUE;
}

// set cursor type according to position
VOID  SetCursorType( DV_CURPOS cp )
{
   switch( cp )
   {
   case DV_ON_LEFT:
   case DV_ON_RIGHT:
      SetCursor( hCurWE );
      break;
   case DV_ON_TOP:
   case DV_ON_BOTTOM:
      SetCursor( hCurNS );
      break;
   case DV_ON_TOPLEFT:
   case DV_ON_BOTTRIGHT:
      SetCursor( hCurNWSE );
      break;
   case DV_ON_TOPRIGHT:
   case DV_ON_BOTTLEFT:
      SetCursor( hCurNESW );
      break;
   case DV_ON_ALL:
      SetCursor( hCurALL );
      break;
   default:
      SetCursor( hCurArrow );
      break;
   }
}

// ==============================================================

// DvTrack.c
