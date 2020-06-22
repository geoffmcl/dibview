

// preview.cpp
#include "showdib.h" // single 'C' include

typedef struct tagPRINTPREVIEW {
   BOOL fDone;
   HWND hwndDlg;
//   REDOC *predoc;
   LONG cchText;
   LONG dxPage;
   LONG dyPage;
   RECT rc;
//   FORMATRANGE fr;
#ifndef NOBLIT
   HBITMAP hbmp;
   HBITMAP hbmpOld;
   INT dxBmp;
   INT dyBmp;
#endif   // !NOBLIT
   INT ipage;
   INT cpage;
   INT cpageMost;
   LONG *rgcpPages;

} PRINTPREVIEW;

//LRESULT  sd_IDM_PPREVIEW( HWND hWnd )

LRESULT  sd_IDM_PRINT( HWND hWnd )
{
   LRESULT  lRes = 0;
   HDC      hDC;
   INT      xSize, ySize, xRes, yRes, dx, dy;
   RECT     Rect;
   BOOL     bSave = FALSE;
   static CHAR Name[80];
   static BITMAPINFOHEADER bi;

   GetWindowText(hWnd, Name, sizeof(Name));

   DibInfo(hbiCurrent, &bi);

   if (!IsRectEmpty(&rcClip))
   {
       bi.biWidth  = rcClip.right  - rcClip.left;
       bi.biHeight = rcClip.bottom - rcClip.top;
   }

   /* Initialise printer stuff */
   if( !(hDC = GetPrinterDC()) )
           return lRes;

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
   if( InitPrinting( hDC, hWnd, hInst, Name ) )
   {

           StartPage(hDC);
           PrintDIB(hWnd, hDC, xRes/2, yRes, dx, dy);

           /* Signal to the driver to begin translating the drawing
            * commands to printer output...
            */
           EndPage(hDC);
           TermPrinting(hDC);
   }

   DeleteDC(hDC);

   return lRes;
}


// eof - preview.cpp
