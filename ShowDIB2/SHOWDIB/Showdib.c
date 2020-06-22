
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
 *  PROGRAM     : ShowDIB.c                                                    *
 *                                                                             *
 *  PURPOSE     : Application to illustrate the use of the GDI                 *
 *                DIB (Device Independent Bitmap) and Palette manager          *
 *                functions.                                                   *
 *                                                                             *
 *  FUNCTIONS   : WinMain ()             -  Creates the app. window and enters *
 *                                          the message loop.                  *
 *                                                                             *
 *                WndProc2()              -  Processes app. window messages.    *
 *                                                                             *
 *                MenuCommand()          -  Processes menu commands.           *
 *                                                                             *
 *                FreeDIB()              -  Frees currently active objects.    *
 *                                                                             *
 *                InitDIB()              -  Reads DIB from a file and loads it.*
 *                                                                             *
 *******************************************************************************/

#include "showdib.h"

extern   BOOL     gbAt100pc;

#define  ADDMAG2

DIBPARAMS      DIBParams;                  /* params for the SETSCALING escape */
TCHAR          achFileName[264];
DWORD          dwOffset;
NPLOGPALETTE   pLogPal;
HPALETTE       hpalSave = NULL;
HINSTANCE      hInst ;
RECT           rcClip;
static         HCURSOR hcurSave;
SIZE           g_wm_size;   // width and height of current window

BOOL    fPalColors  = FALSE;          /* TRUE if the current DIB's color table   */
                                      /* contains palette indexes not rgb values */
UINT_PTR    nAnimating  = 0;              /* Palette animation count                 */
WORD    UpdateCount = 0;

BOOL    bUpdateColors = TRUE;  /* Directly update screen colors                */
BOOL    bDIBToDevice  = FALSE; /* Use SetDIBitsToDevice() to BLT data.         */
BOOL    bNoUgly       = FALSE; /* Make window black on a WM_PALETTEISCHANGING  */
BOOL    bLegitDraw    = FALSE; /* We have a valid bitmap to draw               */

#ifdef   ADDRGB2
TCHAR    szBitmapExt[] = "*.BMP; *.DIB; *.RLE; *.RGB";     /* possible file extensions */
#else // !ADDRGB2
TCHAR    szBitmapExt[] = "*.BMP; *.DIB; *.RLE";     /* possible file extensions */
#endif   // ADDRGB2

WORD    wTransparent  = TRANSPARENT;               /* Mode of DC               */
CHAR    szAppName[]   = "ShowDIB" ;                /* App. name                */

HPALETTE hpalCurrent   = NULL;         /* Handle to current palette            */
HANDLE   hdibCurrent   = NULL;         /* Handle to current memory DIB         */
HBITMAP  hbmCurrent    = NULL;         /* Handle to current memory BITMAP      */
HANDLE   hbiCurrent    = NULL;         /* Handle to current bitmap info struct */
HWND     hWndApp;                      /* Handle to app. window                */

int   gxScreen, gyScreen;
BOOL  bPalSupport = FALSE;
BOOL  bMenuAnim;  // = ( bLegitDraw && bPalSupport );
int   iRasterCaps = 0;
DWORD iLogPixelsX = 0;  // GetDeviceCaps( hDC, LOGPIXELSX );
DWORD iLogPixelsY = 0;  // GetDeviceCaps( hDC, LOGPIXELSY );
RECT  rcWorkArea; // SystemParametersInfo(SPI_GETWORKAREA,...)
DWORD    iCyHScroll; // = GetSystemMetrics (SM_CYHSCROLL);
DWORD    iCxVScroll; // = GetSystemMetrics (SM_CXVSCROLL);
BOOL  bShiftDown = FALSE;
TCHAR gszBuffer[264];
DWORD   iCxBorder  = 0; // GetSystemMetrics (SM_CXEDGE);
DWORD   iCyBorder  = 0; // GetSystemMetrics (SM_CYEDGE);

/* Styles of app. window */
DWORD          dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                         WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;

VOID PrintDIB (HWND hWnd, HDC hDC, INT x, INT y, INT dx, INT dy);
/****************************************************************************
 *                                                                          *
 *  FUNCTION   : WinMain(HANDLE, HANDLE, LPSTR, int)                        *
 *                                                                          *
 *  PURPOSE    : Creates the app. window and enters the message loop.       *
 *                                                                          *
 ****************************************************************************/
int APIENTRY WinMain2(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
    )
{
     HWND        hWnd ;
     WNDCLASS    wndclass ;
     MSG         msg ;
//     int         xScreen, yScreen ;
//     CHAR        ach[40];

     hInst = hInstance ;

     /* Initialize clip rectangle */
     SetRectEmpty(&rcClip);

     if (!hPrevInstance) {
         wndclass.style         = CS_DBLCLKS;
         wndclass.lpfnWndProc   = (WNDPROC) WndProc2 ;
         wndclass.cbClsExtra    = 0 ;
         wndclass.cbWndExtra    = 0 ;
         wndclass.hInstance     = hInstance ;
         wndclass.hIcon         = LoadIcon(hInst, "SHOWICON");
         wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
         wndclass.hbrBackground = GetStockObject (BLACK_BRUSH) ;
         wndclass.lpszMenuName  = szAppName ;
         wndclass.lpszClassName = szAppName ;

         if (!RegisterClass (&wndclass))
                 return FALSE ;
     }

//     if (!GetProfileString("extensions", "bmp", "", ach, sizeof(ach)))
//             WriteProfileString("extensions", "bmp", "showdib.exe ^.bmp");
//     if (!GetProfileString("extensions", "dib", "", ach, sizeof(ach)))
//             WriteProfileString("extensions", "dib", "showdib.exe ^.dib");

     /* Save the pointer to the command line */
     achFileName[0] = 0;
     if( lpCmdLine && *lpCmdLine )
        strcpy(achFileName, lpCmdLine);

     gxScreen = GetSystemMetrics (SM_CXSCREEN) ;
     gyScreen = GetSystemMetrics (SM_CYSCREEN) ;

     /* Create the app. window */
     hWnd = CreateWindow( szAppName,
                          szAppName,
                          dwStyle,
                          CW_USEDEFAULT,
                          0,
                          gxScreen / 2,
                          gyScreen / 2,
                          NULL,
                          NULL,
                          hInstance,
                          NULL) ;

     hWndApp = hWnd;
     ShowWindow(hWndApp, nCmdShow);

     /* Enter message loop */
     while (GetMessage (&msg, NULL, 0, 0))
     {
         TranslateMessage (&msg) ;
         DispatchMessage (&msg) ;
     }

     return (int)msg.wParam ;
}

//IDC_SHOWDIB2 MENU DISCARDABLE 
//BEGIN
//    POPUP "&File"
//    BEGIN
//        MENUITEM "&Open...",                    IDM_OPEN
//        MENUITEM "&Save...",                    IDM_SAVE
//        MENUITEM "&Print...",                   IDM_PRINT
//        MENUITEM SEPARATOR
//        MENUITEM "E&xit",                       IDM_EXIT
//    END
//    POPUP "&Edit"
//    BEGIN
//        MENUITEM "&Paste DIB",                  IDM_PASTEDIB
//        MENUITEM "&Paste DDB",                  IDM_PASTEDDB
//        MENUITEM "&Paste Palette",              IDM_PASTEPAL
//        MENUITEM "&Copy",                       IDM_COPY
//    END
//    POPUP "&Options"
//    BEGIN
//        MENUITEM "&Update Colors",              IDM_UPDATECOL
//        MENUITEM "&Hide Changes",               IDM_NOUGLY
//        MENUITEM "&DIB to Screen",              IDM_DIBSCREEN
//        MENUITEM "&Memory DIBs",                IDM_MEMORYDIB
//        MENUITEM "&Transparent",                IDM_TRANSPARENT
//        MENUITEM "&Show at 100%",               IDM_SHOW100PC
//    END
//    POPUP "&Animate"
//    BEGIN
//        MENUITEM "Steal Colors",                IDM_STEALCOL
//        MENUITEM "Off",                         IDM_ANIMATE0
//        MENUITEM "5",                           IDM_ANIMATE5
//        MENUITEM "50",                          IDM_ANIMATE50
//        MENUITEM "100",                         IDM_ANIMATE100
//        MENUITEM "200",                         IDM_ANIMATE200
//        MENUITEM "LongTime",                    IDM_ANIMATE201
//    END
//    POPUP "Help"
//    BEGIN
//        MENUITEM "&About...",                   IDM_ABOUT
//    END
//END
#define  MENU_FILE      0
#define  MENU_EDIT      1
#define  MENU_OPTIONS   2
#define  MENU_ANIM      3
#define  MENU_HELP      4

VOID  Do_WM_INITMENU( HWND hWnd, WPARAM wParam, LPARAM lParam )
{

            /* check/uncheck menu items depending on state  of related
             * flags
             */

           // set OPTION items
            CheckMenuItem((HMENU)wParam, IDM_UPDATECOL,
                (bUpdateColors ? MF_CHECKED : MF_UNCHECKED));
            CheckMenuItem((HMENU)wParam, IDM_TRANSPARENT,
                (wTransparent == TRANSPARENT ? MF_CHECKED : MF_UNCHECKED));
            CheckMenuItem((HMENU)wParam, IDM_DIBSCREEN,
                (bDIBToDevice ? MF_CHECKED : MF_UNCHECKED));
            CheckMenuItem((HMENU)wParam, IDM_NOUGLY,
                (bNoUgly ? MF_CHECKED : MF_UNCHECKED));
            CheckMenuItem((HMENU)wParam, IDM_MEMORYDIB, MF_CHECKED);

#if   0  // moved to initpopup
            // set EDIT items
            EnableMenuItem((HMENU)wParam, IDM_PASTEDIB,
                IsClipboardFormatAvailable(CF_DIB)?MF_ENABLED:MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_PASTEDDB,
                IsClipboardFormatAvailable(CF_BITMAP)?MF_ENABLED:MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_PASTEPAL,
                IsClipboardFormatAvailable(CF_PALETTE)?MF_ENABLED:MF_GRAYED);

            // set FILE items
            EnableMenuItem((HMENU)wParam, IDM_PRINT,
                bLegitDraw ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_SAVE,
                bLegitDraw ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_COPY,
                bLegitDraw ? MF_ENABLED : MF_GRAYED);
#endif   // #if   0  // moved to initpopup

            // set ANIMATE options
            bMenuAnim = ( bLegitDraw && bPalSupport );
            EnableMenuItem((HMENU)wParam, IDM_ANIMATE0,
                bMenuAnim ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_ANIMATE5,
                bMenuAnim ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_ANIMATE50,
                bMenuAnim ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_ANIMATE100,
                bMenuAnim ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_ANIMATE200,
                bMenuAnim ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_ANIMATE201,
                bMenuAnim ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_STEALCOL,
                bMenuAnim ? MF_ENABLED : MF_GRAYED);
#if   0  // moved to initpopup
            {
               LPTSTR lps;
               if( gbAt100pc )
                  lps = "&Show in Window";
               else
                  lps = "&Show at 100pc";

               ModifyMenu((HMENU)wParam,           // handle to menu
                  IDM_SHOW100PC,    // menu item to modify
                  MF_BYCOMMAND | MF_STRING,     // options
                  IDM_SHOW100PC,    // UINT_PTR uIDNewItem,  // identifier, menu, or submenu
                  lps ); // menu item content
            }
#endif   // #if   0  // moved to initpopup

}


VOID  Do_WM_INITMENUPOPUP( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
   // Parameters
   HMENU hMenu = (HMENU)wParam;  // Handle to the drop-down menu or submenu.
   DWORD pos = LOWORD(lParam); // The low-order word specifies
   // the zero-based relative position of the menu item that opens the drop-down menu
   // or submenu.
   DWORD sys = HIWORD(lParam);   // The high-order word indicates whether the
   // drop-down menu is the window menu. If the menu is the window menu,
   // this parameter is TRUE; otherwise, it is FALSE.

   if(sys)
      return;  // all done here

   switch(pos)
   {
   case MENU_FILE:
            // set FILE items
            EnableMenuItem((HMENU)wParam, IDM_PRINT,
                bLegitDraw ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_SAVE,
                bLegitDraw ? MF_ENABLED : MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_COPY,
                bLegitDraw ? MF_ENABLED : MF_GRAYED);
      break;
   case MENU_EDIT:
            // set EDIT items
            EnableMenuItem((HMENU)wParam, IDM_PASTEDIB,
                IsClipboardFormatAvailable(CF_DIB)?MF_ENABLED:MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_PASTEDDB,
                IsClipboardFormatAvailable(CF_BITMAP)?MF_ENABLED:MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_PASTEPAL,
                IsClipboardFormatAvailable(CF_PALETTE)?MF_ENABLED:MF_GRAYED);
      break;
   case MENU_OPTIONS:
            {
               LPTSTR lps;
               if( gbAt100pc )
                  lps = "&Show in Window";
               else
                  lps = "&Show at 100pc";

               ModifyMenu(hMenu,    // handle to menu
                  IDM_SHOW100PC,    // menu item to modify
                  MF_BYCOMMAND | MF_STRING,     // options
                  IDM_SHOW100PC,    // UINT_PTR uIDNewItem,  // identifier, menu, or submenu
                  lps ); // menu item content
            }
      break;
   case MENU_ANIM:
      break;
   case MENU_HELP:
      break;
   }

}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : WndProc2 (hWnd, iMessage, wParam, lParam)                   *
 *                                                                          *
 *  PURPOSE    : Processes window messages.                                 *
 *                                                                          *
 ****************************************************************************/
LRESULT    APIENTRY WndProc2( HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT      ps;
    HDC              hDC;
    HANDLE           h;
    INT              i;
    INT              iMax;
    INT              iMin;
    INT              iPos;
    INT              dn;
    RECT             rc,Rect;
    HPALETTE         hOldPal;
    HMENU            hMenu;
    CHAR             lpBuffer[128];

    switch (iMessage)
    {
        case WM_DESTROY:
                /* Clean up and quit */
                FreeDib();
                PostQuitMessage(0);
                if(pLogPal)
                   LocalFree(pLogPal);
                break ;

        case WM_CREATE:
                /* Allocate space for our logical palette */
                pLogPal = (NPLOGPALETTE) LocalAlloc( LMEM_FIXED,
                                                     (sizeof(LOGPALETTE) +
                                                     (sizeof(PALETTEENTRY)*(MAXPALETTE))));
        if(!pLogPal)
        {
            LoadString(hInst, IDS_MEMLOW, lpBuffer, sizeof(lpBuffer));
            MessageBox(hWnd, lpBuffer, NULL, MB_OK | MB_ICONHAND);
            PostQuitMessage(0);
            break;
        }

// Temporary workaround.  lpCmdLine points to exe name, not first parameter
                /* If DIB initialization fails, quit */
//              if (achFileName[0] && !InitDIB(hWnd))
//                      PostQuitMessage (3) ;

                /* fall through */
        case WM_WININICHANGE:
              hMenu = GetMenu(hWnd);
              hDC = GetPrinterDC1(); 
              if (hDC)
              {
                    EnableMenuItem( hMenu,
                                    IDM_PRINT,
                                    (RC_DIBTODEV &
                                     GetDeviceCaps(hDC, RASTERCAPS)) ?
                                     MF_ENABLED :
                                     MF_GRAYED | MF_DISABLED);
                  DeleteDC(hDC);
              }
                break;

        case WM_PALETTEISCHANGING:
                /* if SHOWDIB was not responsible for palette change and if
                 * ok to hide changes, paint app. window black.
                 */
                if (wParam != (UINT)(hWnd && bNoUgly))
                {
                    GetClientRect(hWnd, &Rect);

                    hDC = GetDC(hWnd);
                    FillRect( hDC, (LPRECT) &Rect, GetStockObject(BLACK_BRUSH));
                    ReleaseDC(hWnd, hDC);
                }
                break;

        case WM_ACTIVATE:
                if (!GET_WM_ACTIVATE_STATE(wParam, lParam))  /* app. is being de-activated */
                   break;
                /* If the app. is moving to the foreground, fall through and
                 * redraw full client area with the newly realized palette,
                 * if the palette has changed.
                 */

        case WM_QUERYNEWPALETTE:
                /* If palette realization causes a palette change,
                 * we need to do a full redraw.
                 */
                if (bLegitDraw)
                {
                    hDC = GetDC (hWnd);
                    hOldPal = SelectPalette (hDC, hpalCurrent, 0);

                    i = RealizePalette(hDC);

                    SelectPalette (hDC, hOldPal, 0);
                    ReleaseDC (hWnd, hDC);

                    if (i)
                    {
                        InvalidateRect (hWnd, (LPRECT) (NULL), 1);
                        UpdateCount = 0;
                        return 1;
                    } else
                        return FALSE;
                }
                else
                    return FALSE;
                break;

        case WM_PALETTECHANGED:
                /* if SHOWDIB was not responsible for palette change and if
                 * palette realization causes a palette change, do a redraw.
                 */
                 if ((HWND)wParam != hWnd)
                 {
                    if (bLegitDraw)
                    {
                        hDC = GetDC (hWnd);
                        hOldPal = SelectPalette (hDC, hpalCurrent, 0);

                        i = RealizePalette (hDC);

                        if (i)
                        {
                            if (bUpdateColors)
                            {
                                UpdateColors (hDC);
                                UpdateCount++;
                            }
                            else
                                InvalidateRect (hWnd, (LPRECT) (NULL), 1);
                        }

                        SelectPalette (hDC, hOldPal, 0);
                        ReleaseDC (hWnd, hDC);
                    }
                }
                break;

        case WM_RENDERALLFORMATS:
                /* Ensure that clipboard data can be rendered even tho'
                 * app. is being destroyed.
                 */
                SendMessage(hWnd,WM_RENDERFORMAT,CF_DIB,0L);
                SendMessage(hWnd,WM_RENDERFORMAT,CF_BITMAP,0L);
                SendMessage(hWnd,WM_RENDERFORMAT,CF_PALETTE,0L);
                break;

        case WM_RENDERFORMAT:
                /* Format data in manner specified and pass the data
                 * handle to clipboard.
                 */
                h = RenderFormat(wParam); 
                if(h)
                    SetClipboardData((WORD)wParam,h);
                break;

        case WM_COMMAND:
                /* Process menu commands */
                return MenuCommand(hWnd, LOWORD(wParam));
                break;

        case WM_TIMER:

                /* Signal for palette animation */
                hDC = GetDC(hWnd);
                hOldPal = SelectPalette(hDC, hpalCurrent, 0);
                {
                    PALETTEENTRY peTemp;

                    /* Shift all palette entries left by one position and wrap
                     * around the first entry
                     */
                    peTemp = pLogPal->palPalEntry[0];
                    for (i = 0; i < (pLogPal->palNumEntries - 1); i++)
                         pLogPal->palPalEntry[i] = pLogPal->palPalEntry[i+1];
                    pLogPal->palPalEntry[i] = peTemp;
                }

                /* Replace entries in logical palette with new entries*/
                AnimatePalette(hpalCurrent, 0, pLogPal->palNumEntries, pLogPal->palPalEntry);

                SelectPalette(hDC, hOldPal, 0);
                ReleaseDC(hWnd, hDC);

                /* Decrement animation count and terminate animation
                 * if it reaches zero
                 */
                if (!(--nAnimating))
                    PostMessage(hWnd,WM_COMMAND,IDM_ANIMATE0,0L);
                break;

        case WM_PAINT:
                /* If we have updated more than once, the rest of our
                 * window is not in some level of degradation worse than
                 * our redraw...  we need to redraw the whole area
                 */
                if( UpdateCount > 1 )
                {
                    BeginPaint(hWnd, &ps);
                    EndPaint(hWnd, &ps);
                    sprtf( "WM_PAINT: Done w UpdateCount reset from %d"MEOR, UpdateCount );
                    UpdateCount = 0;
                    InvalidateRect(hWnd, (LPRECT) (NULL), TRUE);
                    break;
                }

                hDC = BeginPaint(hWnd, &ps);
                AppPaint(hWnd,
                         hDC,
                         GetScrollPos(hWnd,SB_HORZ),
                         GetScrollPos(hWnd,SB_VERT) );
                EndPaint(hWnd, &ps);
                break ;

        case WM_SIZE:
            g_wm_size.cx = LOWORD(lParam);
            g_wm_size.cy = HIWORD(lParam);   // width and height
            SetScrollRanges(hWnd, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_KEYDOWN:
            /* Translate keyboard messages to scroll commands */
            switch (wParam)
            {
                case VK_UP:
                    PostMessage (hWnd, WM_VSCROLL, SB_LINEUP,   0L);
                    break;

                case VK_DOWN:
                    PostMessage (hWnd, WM_VSCROLL, SB_LINEDOWN, 0L);
                    break;

                case VK_PRIOR:
                    PostMessage (hWnd, WM_VSCROLL, SB_PAGEUP,   0L);
                    break;

                case VK_NEXT:
                    PostMessage (hWnd, WM_VSCROLL, SB_PAGEDOWN, 0L);
                    break;

                case VK_HOME:
                    PostMessage (hWnd, WM_HSCROLL, SB_PAGEUP,   0L);
                    break;

                case VK_END:
                    PostMessage (hWnd, WM_HSCROLL, SB_PAGEDOWN, 0L);
                    break;

                case VK_LEFT:
                    PostMessage (hWnd, WM_HSCROLL, SB_LINEUP,   0L);
                    break;

                case VK_RIGHT:
                    PostMessage (hWnd, WM_HSCROLL, SB_LINEDOWN, 0L);
                    break;
#ifdef   ADDMAG2
                case VK_SHIFT:
                   bShiftDown = TRUE;
                   break;
#endif   // ADDMAG2

            }
            break;

        case WM_KEYUP:
            switch (wParam)
            {
               case VK_UP:
               case VK_DOWN:
               case VK_PRIOR:
               case VK_NEXT:
                  PostMessage (hWnd, WM_VSCROLL, SB_ENDSCROLL, 0L);
                  break;

               case VK_HOME:
               case VK_END:
               case VK_LEFT:
               case VK_RIGHT:
                  PostMessage (hWnd, WM_HSCROLL, SB_ENDSCROLL, 0L);
                  break;
#ifdef   ADDMAG2
               case VK_SHIFT:
                   bShiftDown = FALSE;
                   break;
#endif   // ADDMAG2
            }
            break;

        case WM_VSCROLL:
            /* Calculate new vertical scroll position */
            GetScrollRange (hWnd, SB_VERT, &iMin, &iMax);
            iPos = GetScrollPos (hWnd, SB_VERT);
            GetClientRect (hWnd, &rc);

            switch (GET_WM_VSCROLL_CODE(wParam, lParam)) {
                case SB_LINEDOWN:
                    dn =  rc.bottom / 16 + 1;
                    break;

                case SB_LINEUP:
                    dn = -rc.bottom / 16 + 1;
                    break;

                case SB_PAGEDOWN:
                    dn =  rc.bottom / 2  + 1;
                    break;

                case SB_PAGEUP:
                    dn = -rc.bottom / 2  + 1;
                    break;

                case SB_THUMBTRACK:
                case SB_THUMBPOSITION:
                    dn = GET_WM_VSCROLL_POS(wParam, lParam)-iPos;
                    break;

                default:
                    dn = 0;
                    break;
            }
            /* Limit scrolling to current scroll range */
            if (dn = BOUND (iPos + dn, iMin, iMax) - iPos) {
                ScrollWindow (hWnd, 0, -dn, NULL, NULL);
                SetScrollPos (hWnd, SB_VERT, iPos + dn, TRUE);
            }
            break;

        case WM_HSCROLL:
            /* Calculate new horizontal scroll position */
            GetScrollRange (hWnd, SB_HORZ, &iMin, &iMax);
            iPos = GetScrollPos (hWnd, SB_HORZ);
            GetClientRect (hWnd, &rc);

            switch (GET_WM_HSCROLL_CODE(wParam, lParam)) {
                case SB_LINEDOWN:
                    dn =  rc.right / 16 + 1;
                    break;

                case SB_LINEUP:
                    dn = -rc.right / 16 + 1;
                    break;

                case SB_PAGEDOWN:
                    dn =  rc.right / 2  + 1;
                    break;

                case SB_PAGEUP:
                    dn = -rc.right / 2  + 1;
                    break;

                case SB_THUMBTRACK:
                case SB_THUMBPOSITION:
                    dn = GET_WM_HSCROLL_POS(wParam, lParam)-iPos;
                    break;

                default:
                    dn = 0;
                    break;
            }
            /* Limit scrolling to current scroll range */
            if (dn = BOUND (iPos + dn, iMin, iMax) - iPos) {
                ScrollWindow (hWnd, -dn, 0, NULL, NULL);
                SetScrollPos (hWnd, SB_HORZ, iPos + dn, TRUE);
            }
            break;

        case WM_LBUTTONDOWN:
            /* Start rubberbanding a rect. and track it's dimensions.
             * set the clip rectangle to it's dimensions.
             */
            TrackMouse (hWnd, MAKEMPOINT(lParam));
            break;

        case WM_LBUTTONDBLCLK:
            break;

        case WM_INITMENUPOPUP:
           Do_WM_INITMENUPOPUP( hWnd, wParam, lParam );
           break;

        case WM_INITMENU:
           Do_WM_INITMENU( hWnd, wParam, lParam );
           break;

        default:
            return DefWindowProc (hWnd, iMessage, wParam, lParam) ;

    }
    return 0L ;

}
/****************************************************************************
 *                                                                          *
 *  FUNCTION   : MenuCommand ( HWND hWnd, WPARAM wParam)                            *
 *                                                                          *
 *  PURPOSE    : Processes menu commands.                                   *
 *                                                                          *
 *  RETURNS    : TRUE  - if command could be processed.                     *
 *               FALSE - otherwise                                          *
 *                                                                          *
 ****************************************************************************/
BOOL MenuCommand ( HWND hWnd, WPARAM wParam )
{
    static BITMAPINFOHEADER bi;
    HDC              hDC;
    HANDLE           h;
    HBITMAP          hbm;
    HPALETTE         hpal;
    DWORD            i, id;
//    CHAR             Name[40];
//    BOOL             bSave;
//    INT              xSize, ySize, xRes, yRes, dx, dy;
//    RECT             Rect;
    HANDLE           fh;
    WORD             fFileOptions;
//    CHAR             lpBuffer[128];
    LPTSTR           lpB = gszBuffer;

    id = LOWORD(wParam);
    switch(id)
    {
    case IDM_ABOUT:
                /* Show About .. box */
                fDialog ((INT)ABOUTBOX, hWnd,(FARPROC)AppAbout);
        break;

    case IDM_COPY:
        if( !bLegitDraw )
            return 0L;

        /* Clean clipboard of contents */
        if( OpenClipboard(hWnd) )
        {
                    EmptyClipboard ();
                    SetClipboardData (CF_DIB     ,NULL);
                    SetClipboardData (CF_BITMAP  ,NULL);
                    SetClipboardData (CF_PALETTE ,NULL);
                    CloseClipboard ();
        }
        break;

     case IDM_PASTEPAL:
        if( OpenClipboard (hWnd) )
        {
           h = GetClipboardData (CF_PALETTE); 
           if (h)
           {
              /* Delete current palette and get the CF_PALETTE data
               * from the clipboard
               */
              if( hpalCurrent )
                 DeleteObject (hpalCurrent);

              hpalCurrent = CopyPalette (h);

              /*
               * If we have a bitmap realized against the old palette
               * delete the bitmap and rebuild it using the new palette.
               */
              if(hbmCurrent)
              {
                 DeleteObject (hbmCurrent);
                 hbmCurrent = NULL;

                 if( hdibCurrent )
                    hbmCurrent = BitmapFromDib (hdibCurrent, hpalCurrent);
                    
              }
              
           }
           CloseClipboard();
           
        }
        
        break;

     case IDM_PASTEDIB:
        if( OpenClipboard (hWnd) )
        {
           h = GetClipboardData (CF_DIB); 
           if(h)
           {
              /* Delete current DIB and get CF_DIB and
               * CF_PALETTE format data from the clipboard
               */
              hpal = GetClipboardData (CF_PALETTE);

              FreeDib();

              hdibCurrent = CopyHandle (h);
              if( hdibCurrent )
              {
                 bLegitDraw = TRUE;
                 strcpy(achFileName,"<Clipboard>");
                 hbiCurrent = hdibCurrent;

                 /* If there is a CF_PALETTE object in the
                  * clipboard, this is the palette to assume
                  * the DIB should be realized against, otherwise
                  * create a palette for it.
                  */
                 if(hpal)
                    hpalCurrent = CopyPalette (hpal);
                 else
                    hpalCurrent = CreateDibPalette (hdibCurrent);

                 SizeWindow(hWnd);
                 
              }
              else
              {
                            bLegitDraw = FALSE;
                            LoadString(hInst, IDS_NOMEM, lpB, 256);
                            ErrMsg(lpB);
                  
              }
              
           }
           
           CloseClipboard();
           
        }
        
        break;

     case IDM_PASTEDDB:
        if( OpenClipboard (hWnd) )
        {
           hbm = GetClipboardData(CF_BITMAP); 
           if (hbm)
           {
                        hpal = GetClipboardData(CF_PALETTE);
                        FreeDib();

                        /*
                         * If there is a CF_PALETTE object in the
                         * clipboard, this is the palette to assume
                         * the bitmap is realized against.
                         */
                        if (hpal)
                            hpalCurrent = CopyPalette(hpal);
                        else
                            hpalCurrent = GetStockObject(DEFAULT_PALETTE);

                        hdibCurrent = DibFromBitmap(hbm,BI_RGB,0,hpalCurrent);

                        if (hdibCurrent)
                        {
                            bLegitDraw = TRUE;
                            strcpy(achFileName,"<Clipboard>");
                            hbiCurrent = hdibCurrent;

                            hbmCurrent = BitmapFromDib(hdibCurrent,hpalCurrent);

                            SizeWindow(hWnd);
                        }
                        else
                        {
                            bLegitDraw = FALSE;
                            LoadString(hInst, IDS_NOMEM, lpB, 256);
                            ErrMsg(lpB);
                        }
                    }
                    CloseClipboard ();
                
        }
        
        break;

     case IDM_PRINT:
#ifdef   SDOLD1
                GetWindowText(hWnd, Name, sizeof(Name));
                DibInfo(hbiCurrent, &bi);
                if (!IsRectEmpty(&rcClip))
                {
                    bi.biWidth  = rcClip.right  - rcClip.left;
                    bi.biHeight = rcClip.bottom - rcClip.top;
                }

                /* Initialise printer stuff */
                if (!(hDC = GetPrinterDC()))
                        break;

                xSize = GetDeviceCaps(hDC, HORZRES);
                ySize = GetDeviceCaps(hDC, VERTRES);
                xRes  = GetDeviceCaps(hDC, LOGPIXELSX);
                yRes  = GetDeviceCaps(hDC, LOGPIXELSY);

                /* Use half inch margins on left and right
                 * and one inch on top. Maintain the same aspect ratio.
                 */

                dx = xSize - xRes;
                dy = (INT)((LONG)dx * bi.biHeight/bi.biWidth);

                /* Fix bounding rectangle for the picture .. */
                Rect.top    = yRes;
                Rect.left   = xRes / 2;
                Rect.bottom = yRes + dy;
                Rect.right  = xRes / 2 + dx;

                /* ... and inform the driver */
                Escape(hDC, SET_BOUNDS, sizeof(RECT), (LPSTR)&Rect, NULL);

                bSave = TRUE;

                //
                // Use new Windows NT printing APIs...Petrus Wong 12-May-1993
                //
                if (InitPrinting(hDC, hWnd, hInst, Name)) {

                        StartPage(hDC);
                        PrintDIB(hWnd, hDC, xRes/2, yRes, dx, dy);

                        /* Signal to the driver to begin translating the drawing
                         * commands to printer output...
                         */
                        EndPage(hDC);
                        TermPrinting(hDC);
                }

                DeleteDC(hDC);

#else // !SDOLD1
                sd_IDM_PRINT( hWnd );
#endif   // SDOLD1
                break;

        case IDM_OPEN:    
           {
                /* Bring up File/Open ... dialog */
                LoadString(hInst, IDS_OPENDIBPROMPT, lpB, 256);
                fh = DlgOpenFile (hWnd,
                                  lpB,
                                  (LONG)OF_EXIST | OF_MUSTEXIST | OF_NOOPTIONS,
                                  szBitmapExt,
                                  achFileName,
                                  NULL
                                  );
                /*  Load up the DIB if the user did not press cancel */
                if (fh > 0)
                {
                   StartWait();
                   if (InitDIB (hWnd))
                       InvalidateRect (hWnd, NULL, FALSE);
                   else
                       bLegitDraw = FALSE;
                   EndWait();
                }
           }
           break;

        case IDM_SAVE:
                DibInfo(hbiCurrent,&bi);
                fFileOptions = 0;

                /* Depending on compression type for current DIB,
                 * set the appropriate bit in the fFileOptions flag
                 */
                if (bi.biCompression == BI_RGB)
                    fFileOptions |= F_RGB;
                else if (bi.biCompression == BI_RLE4)
                    fFileOptions |= F_RLE4;
                else if (bi.biCompression == BI_RLE8)
                    fFileOptions |= F_RLE8;

                /* Depending on bits/pixel type for current DIB,
                 * set the appropriate bit in the fFileOptions flag
                 */
                switch (bi.biBitCount)
                {
                    case  1:
                        fFileOptions |= F_1BPP;
                        break;

                    case  4:
                        fFileOptions |= F_4BPP;
                        break;

                    case  8:
                        fFileOptions |= F_8BPP;
                        break;

                    case 24:
                        fFileOptions |= F_24BPP;
                }

                /* Bring up File/Save... dialog and get info. about filename,
                 * compression, and bits/pix. of DIB to be written.
                 */
                LoadString(hInst, IDS_SAVEDIBPROMPT, lpB, 256);
                fh = DlgOpenFile( hWnd,
                                  lpB,
                                  (LONG)OF_EXIST | OF_SAVE | OF_NOSHOWSPEC,
                                  szBitmapExt,
                                  achFileName,
                                  &fFileOptions);

                /* Extract DIB specs. if the user did not press cancel */
                if( fh != 0 )
                {
                    if (fFileOptions & F_RGB)
                        bi.biCompression = BI_RGB;

                    if (fFileOptions & F_RLE4)
                        bi.biCompression = BI_RLE4;

                    if (fFileOptions & F_RLE8)
                        bi.biCompression = BI_RLE8;

                    if (fFileOptions & F_1BPP)
                        bi.biBitCount = 1;

                    if (fFileOptions & F_4BPP)
                        bi.biBitCount = 4;

                    if (fFileOptions & F_8BPP)
                        bi.biBitCount = 8;

                    if (fFileOptions & F_24BPP)
                        bi.biBitCount = 24;

                    /* Realize a DIB in the specified format and obtain a
                     * handle to it.
                     */
                    hdibCurrent = RealizeDibFormat(bi.biCompression,bi.biBitCount);
                    if( !hdibCurrent )
                    {
                        LoadString(hInst, IDS_CANTSAVEFILE, lpB, 256);
                        ErrMsg(lpB);
                        return 0L;
                    }

                    /* Write the DIB */
                    StartWait();
                    if( !WriteDIB( achFileName, hdibCurrent ) )
                    {
                        LoadString(hInst, IDS_CANTSAVEFILE, lpB, 256);
                        ErrMsg(lpB);
                    }
                    EndWait();
                }
                break;

        case IDM_EXIT:
                PostMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
                break;

        case IDM_UPDATECOL:
                /* Toggle state of "update screen colors" flag. If it is
                 * off, clear the "hide changes" flag
                 */
                bUpdateColors = !bUpdateColors;
                if (bUpdateColors)
                    bNoUgly = 0;
                break;

        case IDM_DIBSCREEN:
                bDIBToDevice = !bDIBToDevice;
                InvalidateRect(hWnd, (LPRECT) (NULL), 1);
                break;

        case IDM_MEMORYDIB:
                break;

        case IDM_NOUGLY:
                /* Toggle state of "hide changes" flag. If it is off, clear
                 * the "update screen colors" flag. This will tell SHOWDIB
                 * to paint itself black while the palette is changing.
                 */
                bNoUgly = !bNoUgly;
                if (bNoUgly)
                    bUpdateColors = 0;
                break;

        case IDM_TRANSPARENT:
                /* Toggle DC mode */
                wTransparent = (WORD) (wTransparent == TRANSPARENT ?
                    OPAQUE : TRANSPARENT);
                break;

        case IDM_ANIMATE0:
                if (!hpalSave)
                    break;

                /* Reset animation count and stop timer */
                KillTimer(hWnd, 1);
                nAnimating = 0;

                /* Restore palette which existed before animation started */
                DeleteObject(hpalCurrent);
                hpalCurrent = hpalSave;

                /* Rebuild bitmap based on newly realized information */
                hDC = GetDC (hWnd);
                SelectPalette (hDC, hpalCurrent, 0);
                RealizePalette (hDC);
                ReleaseDC (hWnd, hDC);

                if (hbmCurrent){
                    DeleteObject (hbmCurrent);
                    hbmCurrent = NULL;

                    if (hdibCurrent)
                       hbmCurrent = BitmapFromDib (hdibCurrent, hpalCurrent);
                }
                hpalSave = NULL;

                /* Force redraw with new palette for everyone */
                InvalidateRect(hWnd, NULL, TRUE);
                break;

        case IDM_STEALCOL:
        case IDM_ANIMATE5:
        case IDM_ANIMATE20:
        case IDM_ANIMATE50:
        case IDM_ANIMATE100:
        case IDM_ANIMATE200:
        case IDM_ANIMATE201:
           {
                /* Set animation count i.e number of times animation is to
                 * take place. REMOVE DEPENDENCE ON COMMAND VALUE
                 */
                //nAnimating = id;
                //if (id == IDM_STEALCOL)
                //        nAnimating = 0;
					switch(id)
					{
        			case IDM_STEALCOL:
						nAnimating = 0;
						break;
        			case IDM_ANIMATE5:
						nAnimating = 5;
						break;
        			case IDM_ANIMATE20:
						nAnimating = 20;
						break;
        			case IDM_ANIMATE50:
						nAnimating = 50;
						break;
        			case IDM_ANIMATE100:
						nAnimating = 100;
						break;
        			case IDM_ANIMATE200:
						nAnimating = 200;
						break;
       			case IDM_ANIMATE201:
						nAnimating = 32000;
						break;
					}

                /* Save current palette */
                hpalSave = CopyPalette(hpalCurrent);

                GetObject(hpalCurrent, sizeof(INT), (LPSTR)&pLogPal->palNumEntries);
                GetPaletteEntries(hpalCurrent, 0, pLogPal->palNumEntries, pLogPal->palPalEntry);

                /* Reserve all entries in the palette otherwise AnimatePalette()
                 * will not modify them
                 */
                for (i = 0; i < pLogPal->palNumEntries; i++)
                {
                     pLogPal->palPalEntry[i].peFlags = (BYTE)PC_RESERVED;
                }

                SetPaletteEntries( hpalCurrent,
                   0,
                   pLogPal->palNumEntries,
                   pLogPal->palPalEntry );

                /* Rebuild bitmap based on newly realized information */
                if( hbmCurrent )
                {
                    DeleteObject (hbmCurrent);
                    hbmCurrent = NULL;

                    if (hdibCurrent)
                       hbmCurrent = BitmapFromDib( hdibCurrent, hpalCurrent );
                }

                /* Force redraw with new palette for everyone */
                InvalidateRect(hWnd, NULL, TRUE);

                /* Initiate the timer so that palette can be animated in
                 * response to a WM_TIMER message
                 */
                if( nAnimating )
                {
                   if( !SetTimer(hWnd, 1, 250, (TIMERPROC)(LPSTR) NULL))
                   {
                      nAnimating = 0;
                   }
                }
           }

           break;

        case IDM_SHOW100PC:
            gbAt100pc = !gbAt100pc;
            sprtf( "Show image %s - full repaint ..."MEOR,
               ( gbAt100pc ? "at 100pc" : "in Window" ) );

            SetScrollRanges( hWnd,
               (WORD)g_wm_size.cx,
               (WORD)g_wm_size.cy );   // width and height
            InvalidateRect( hWnd, NULL, TRUE );
           break;

        default:
                break;
    }

    return TRUE;
}

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : InitDIB(hWnd)                                              *
 *                                                                          *
 *  PURPOSE    : Reads a DIB from a file, obtains a handle to it's          *
 *               BITMAPINFO struct., sets up the palette and loads the DIB. *
 *                                                                          *
 *  RETURNS    : TRUE  - DIB loads ok                                       *
 *               FALSE - otherwise                                          *
 *                                                                          *
 ****************************************************************************/
INT InitDIB(HWND hWnd)
{
    HFILE              fh;
    LPBITMAPINFOHEADER lpbi;
    WORD FAR *         pw;
    INT                i;
    BITMAPINFOHEADER   bi;
    OFSTRUCT           of;
    TCHAR *            lpB = gszBuffer;

    FreeDib();

    /* Open the file and get a handle to it's BITMAPINFO */

    fh = OpenFile(achFileName, (LPOFSTRUCT)&of, (UINT)OF_READ);
    if( fh == -1 )
    {
        LoadString(hInst, IDS_CANTOPENFILE, lpB, 256);
        ErrMsg(lpB, (LPSTR)achFileName);
        return FALSE;
    }
    hbiCurrent = ReadDibBitmapInfo(fh);
#ifdef   ADDRGB2
    if( !hbiCurrent )
    {
       int width, height, components;
       hbiCurrent = ReadRGBInfo(achFileName, &fh,
                                 &width, &height, &components );
    }
#endif   // ADDRGB2

    dwOffset = _llseek(fh, 0L, (UINT)FILE_BEGIN);  // was SEEK_CUR???
    _lclose(fh);

    if( hbiCurrent == NULL )
    {
        LoadString(hInst, IDS_BADDIBFILE, lpB, 256);
        ErrMsg(lpB, (LPTSTR)achFileName);
        return FALSE;
    }

    DibInfo(hbiCurrent,&bi);

    /* Set up the palette */
    hpalCurrent = CreateDibPalette(hbiCurrent);
#ifndef  WIN32
    if( hpalCurrent == NULL )
    {
        LoadString(hInst, IDS_CREATEPALFAIL, lpB, 256);
        ErrMsg(lpB);
        return FALSE;
    }
#endif   // !WIN32

    /*  Convert the DIB color table to palette relative indexes, so
     *  SetDIBits() and SetDIBitsToDevice() can avoid color matching.
     *  We can do this because the palette we realize is identical
     *  to the color table of the bitmap, ie the indexes match 1 to 1
     *
     *  Now that the DIB color table is palette indexes not RGB values
     *  we must use DIB_PAL_COLORS as the wUsage parameter to SetDIBits()
     */
    lpbi = (VOID FAR *)GlobalLock(hbiCurrent);
    if (lpbi->biBitCount != 24)
    {
        fPalColors = TRUE;

        pw = (WORD FAR *)((LPSTR)lpbi + lpbi->biSize);

        for (i=0; i<(INT)lpbi->biClrUsed; i++)
            *pw++ = (WORD)i;
    }

    GlobalUnlock(hbiCurrent);

    bLegitDraw = TRUE;

    /*  If the input bitmap is not in RGB FORMAT the banding code will
     *  not work!  we need to load the DIB bits into memory.
     *  if memory DIB, load it all NOW!  This will avoid calling the
     *  banding code.
     */
    hdibCurrent = OpenDIB(achFileName);

    /*  If the RLE could not be loaded all at once, exit gracefully NOW,
     *  to avoid calling the banding code
     */
    if ((bi.biCompression != BI_RGB) && !hdibCurrent){
        LoadString(hInst, IDS_CANTLOADRLE, lpB, 256);
        ErrMsg (lpB);
        FreeDib();
        return FALSE;
    }

    if (hdibCurrent && !bDIBToDevice){
       // try again to get a bitmap handle
        hbmCurrent = BitmapFromDib(hdibCurrent,hpalCurrent);
        if (!hbmCurrent){
           if( !bLegitDraw )
           {
               LoadString(hInst, IDS_CANTCREATEBMP, lpB, 256);
               ErrMsg (lpB);
               FreeDib();
               return FALSE;
           }
        }
    }

    SizeWindow(hWnd);

    return TRUE;
}
/****************************************************************************
 *                                                                          *
 *  FUNCTION   : FreeDib(void)                                              *
 *                                                                          *
 *  PURPOSE    : Frees all currently active bitmap, DIB and palette objects *
 *               and initializes their handles.                             *
 *                                                                          *
 ****************************************************************************/
VOID FreeDib()
{
    if (hpalCurrent)
        DeleteObject(hpalCurrent);

    if (hbmCurrent)
        DeleteObject(hbmCurrent);

    if (hdibCurrent)
        GlobalFree(hdibCurrent);

    if (hbiCurrent && hbiCurrent != hdibCurrent)
        GlobalFree(hbiCurrent);

    fPalColors  = FALSE;
    bLegitDraw  = FALSE;
    hpalCurrent = NULL;
    hdibCurrent = NULL;
    hbmCurrent  = NULL;
    hbiCurrent  = NULL;
    SetRectEmpty (&rcClip);
}


void  ShowMemErr( HWND hWnd )
{
   TCHAR lpBuffer[128];
   LoadString(hInst, IDS_MEMLOW, lpBuffer, sizeof(lpBuffer));
   MessageBox(hWnd, lpBuffer, NULL, MB_OK | MB_ICONHAND);
}


// eof - Showdib.c
