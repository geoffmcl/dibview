
// DvMagnify.c

#include	"dv.h"		// Single inclusive include
extern	BOOL	CenterWindow(HWND hwndChild, HWND hwndParent);
extern COLORREF	CustColors[16];
extern   BOOL  WriteBMPFile2( LPTSTR lpf, HGLOBAL hDIB, BOOL bShow );
extern PMWL CommonFileOpen( HWND hWnd, LPSTR lpf, DWORD Caller );

BOOL gbSwish = TRUE;
BOOL gbOval = FALSE;
BOOL gbChgSwi = FALSE;
BOOL gbChgOva = FALSE;
//HWND  hActWnd = NULL;

#define  dbg_sprtf
#define  dbg_oval    sprtf

#ifndef  NOTSRCAND
#pragma message( "Defining NOTSRCAND ..." )
#define  NOTSRCAND 0x220326
#endif

// [Magnify] sizes
TCHAR szMagOcx[] = "MagOutercx";
int gMagOCX = 120;
BOOL gChgOCX = FALSE;
TCHAR szMagOcy[] = "MagOutercy";
int gMagOCY = 120;
BOOL gChgOCY = FALSE;
TCHAR szMagIcx[] = "MagInnercx";
int gMagICX = 40;
BOOL gChgICX = FALSE;
TCHAR szMagIcy[] = "MagInnercy";
int gMagICY = 40;
BOOL gChgICY = FALSE;
TCHAR szMagTC[] = "MagTransColor";
COLORREF gMagTC = RGB(255, 0, 254);
BOOL gChgTC = FALSE;
TCHAR szMagState[] = "Magnify";
int   gi_MouseMagnify = 1;
BOOL  gChgMag = FALSE;
// DrawColoredEllipse( hdc, &rc, RGB(255,0,0) );
TCHAR szMagOL[] = "MagOutlineColor";
COLORREF gMagOL = RGB(255, 0, 0);
BOOL gChgOL = FALSE;
TCHAR szMagAO[] = "MagAddOutline";
BOOL bAddOutline = TRUE;
BOOL gChgAO = FALSE;
// SWISH STUFF
#define  SW_PEN_SZ   8
TCHAR szPenW[] = "SwPenWidth";
DWORD gPenW = SW_PEN_SZ;   // default SIZE
BOOL  gChgPen = FALSE;

TCHAR szPenC[] = "SwPenColor";
COLORREF gPenC = RGB(255,0,0);   // default COLOR
BOOL  gChgPC = FALSE;

static int addsystime = 0;
static int addstdout = 0;

int   add_std_out( int val )
{
   int i = addstdout;
   addstdout = val;
   return i;
}

int   add_sys_time( int val )
{
   int   i = addsystime;
   addsystime = val;
   return i;
}

static POINT   ptOrigin;

VOID  Reset_Scroll_Adjustment( HWND hWnd, HDC hdc, PRECT prc )
{
#ifdef	WIN32
	   SetWindowOrgEx( hdc, ptOrigin.x, ptOrigin.y, NULL );
#else	// !WIN32
	   SetWindowOrg( hdc, ptOrigin.x, ptOrigin.y );
#endif	// WIN32 y/n
}

VOID  Make_Scroll_Adjustment( HWND hWnd, HDC hdc, PRECT prc )
{
   POINT pt;
   //RECT  rc = *prc;
   pt.x = GetScrollPos( hWnd, SB_HORZ );
	pt.y = GetScrollPos( hWnd, SB_VERT );
   if( pt.x || pt.y ) {
      // set starting coordinates.
#ifdef	WIN32
	   SetWindowOrgEx( hdc, pt.x, pt.y, &ptOrigin );
#else	// !WIN32
	   SetWindowOrg( hdc, pt.x, pty );
#endif	// WIN32 y/n
      //OffsetRect( prc, pt.x, pt.y );
      dbg_oval( "DEBUGSCROLL: Due to scroll origin set to %d,%d \n",
         pt.x, pt.y );
   }
}

void Dv_DrawTransparent(HDC hdcDst, HBITMAP hBmp,
                         int x, int y,
                         COLORREF clrTransparency)
{
   BITMAP bm;
   POINT size;
   POINT org;
   HDC dcImage = NULL;
   // CBitmap* pOldBitmapImage = dcImage.SelectObject (this);
   HBITMAP pOldBitmapImage = NULL;
   HDC dcAnd = NULL;
   HBITMAP bitmapAnd = NULL;
   HBITMAP pOldBitmapAnd = NULL;
   HDC dcXor = NULL;
   // CBitmap bitmapXor;
   // bitmapXor.CreateCompatibleBitmap (&dcImage, bm.bmWidth, bm.bmHeight);
   // CBitmap* pOldBitmapXor = dcXor.SelectObject (&bitmapXor);
   HBITMAP bitmapXor = NULL;
   HBITMAP pOldBitmapXor = NULL;
   HDC dcTemp = NULL;
   // CBitmap bitmapTemp;
   // bitmapTemp.CreateCompatibleBitmap (&dcImage, bm.bmWidth, bm.bmHeight);
   // CBitmap* pOldBitmapTemp = dcTemp.SelectObject (&bitmapTemp);
   HBITMAP bitmapTemp = NULL;
   HBITMAP pOldBitmapTemp = NULL;

   //GetBitmap (&bm);
   GetObject( hBmp, sizeof(BITMAP), (LPSTR)&bm );
   //CPoint size (bm.bmWidth, bm.bmHeight);
   //pDC->DPtoLP (&size);
   //CPoint org (0, 0);
   //pDC->DPtoLP (&org);
   size.x = bm.bmWidth;
   size.y = bm.bmHeight;
   DPtoLP( hdcDst, &size, 1 );
   org.x = 0;
   org.y = 0;
   DPtoLP( hdcDst, &org, 1 );

   // Create a memory DC (dcImage) and select the bitmap into it.
   //CDC dcImage;
   //dcImage.CreateCompatibleDC (pDC);
   dcImage = CreateCompatibleDC(hdcDst);
   // CBitmap* pOldBitmapImage = dcImage.SelectObject (this);
   pOldBitmapImage = (HBITMAP)SelectObject(dcImage, hBmp);
   // dcImage.SetMapMode (pDC->GetMapMode ());
   SetMapMode( dcImage, GetMapMode(hdcDst) );

   // Create a second memory DC (dcAnd) and in it create an AND mask.
   // CDC dcAnd;
   // dcAnd.CreateCompatibleDC (pDC);
   // dcAnd.SetMapMode (pDC->GetMapMode ());
   dcAnd = CreateCompatibleDC (hdcDst);
   SetMapMode (dcAnd, GetMapMode (hdcDst));

   // CBitmap bitmapAnd;
   // bitmapAnd.CreateBitmap (bm.bmWidth, bm.bmHeight, 1, 1, NULL);
   // CBitmap* pOldBitmapAnd = dcAnd.SelectObject (&bitmapAnd);
   bitmapAnd = CreateBitmap (bm.bmWidth, bm.bmHeight, 1, 1, NULL);
   pOldBitmapAnd = (HBITMAP)SelectObject (dcAnd, bitmapAnd);

   // dcImage.SetBkColor (clrTransparency);
   // dcAnd.BitBlt (org.x, org.y, size.x, size.y, &dcImage, org.x, org.y,
   //     SRCCOPY);
   SetBkColor(dcImage, clrTransparency);
   BitBlt (dcAnd, org.x, org.y, size.x, size.y, dcImage, org.x, org.y,
        SRCCOPY);

   // Create a third memory DC (dcXor) and in it create an XOR mask.
   // CDC dcXor;
   // dcXor.CreateCompatibleDC (pDC);
   // dcXor.SetMapMode (pDC->GetMapMode ());
   dcXor = CreateCompatibleDC (hdcDst);
   SetMapMode ( dcXor, GetMapMode (hdcDst) );

   // CBitmap bitmapXor;
   // bitmapXor.CreateCompatibleBitmap (&dcImage, bm.bmWidth, bm.bmHeight);
   // CBitmap* pOldBitmapXor = dcXor.SelectObject (&bitmapXor);
   bitmapXor = CreateCompatibleBitmap (dcImage, bm.bmWidth, bm.bmHeight);
   pOldBitmapXor = (HBITMAP)SelectObject (dcXor, bitmapXor);

    //dcXor.BitBlt (org.x, org.y, size.x, size.y, &dcImage, org.x, org.y,
    //    SRCCOPY);
   BitBlt (dcXor, org.x, org.y, size.x, size.y, dcImage, org.x, org.y,
        SRCCOPY);

    //dcXor.BitBlt (org.x, org.y, size.x, size.y, &dcAnd, org.x, org.y,
    //    0x220326);
   BitBlt (dcXor, org.x, org.y, size.x, size.y, dcAnd, org.x, org.y,
        NOTSRCAND ); // = 0x220326

   // Copy the pixels in the destination rectangle to a temporary
   // memory DC (dcTemp).
   // CDC dcTemp;
   // dcTemp.CreateCompatibleDC (pDC);
   // dcTemp.SetMapMode (pDC->GetMapMode ());
   dcTemp = CreateCompatibleDC (hdcDst);
   SetMapMode ( dcTemp, GetMapMode (hdcDst) );

   // CBitmap bitmapTemp;
   // bitmapTemp.CreateCompatibleBitmap (&dcImage, bm.bmWidth, bm.bmHeight);
   // CBitmap* pOldBitmapTemp = dcTemp.SelectObject (&bitmapTemp);
   bitmapTemp = CreateCompatibleBitmap (dcImage, bm.bmWidth, bm.bmHeight);
   pOldBitmapTemp = (HBITMAP)SelectObject (dcTemp, bitmapTemp);

   // dcTemp.BitBlt (org.x, org.y, size.x, size.y, pDC, x, y, SRCCOPY);
   BitBlt (dcTemp, org.x, org.y, size.x, size.y, hdcDst, x, y, SRCCOPY);

   // Generate the final image by applying the AND and XOR masks to
   // the image in the temporary memory DC.
   // dcTemp.BitBlt (org.x, org.y, size.x, size.y, &dcAnd, org.x, org.y,
   //     SRCAND);
   // dcTemp.BitBlt (org.x, org.y, size.x, size.y, &dcXor, org.x, org.y,
   //     SRCINVERT);
   BitBlt ( dcTemp, org.x, org.y, size.x, size.y, dcAnd, org.x, org.y,
        SRCAND);
   BitBlt ( dcTemp, org.x, org.y, size.x, size.y, dcXor, org.x, org.y,
      SRCINVERT);

   // Blit the resulting image to the screen.
   // pDC->BitBlt (x, y, size.x, size.y, &dcTemp, org.x, org.y, SRCCOPY);
   BitBlt ( hdcDst, x, y, size.x, size.y, dcTemp, org.x, org.y, SRCCOPY);
	//pDC->StretchBlt(x, y, cx, cy, &dcTemp, 0, 0, w, h, SRCCOPY);

   // Restore the default bitmaps.
   SelectObject ( dcTemp, pOldBitmapTemp);
   SelectObject ( dcXor,  pOldBitmapXor);
   SelectObject ( dcAnd,  pOldBitmapAnd);
   SelectObject ( dcImage, pOldBitmapImage);
   // delete objects
   DeleteDC(dcImage);
   DeleteDC(dcAnd);
   DeleteObject(bitmapAnd);
   DeleteDC(dcXor);
   DeleteObject(bitmapXor);
   DeleteDC(dcTemp);
   DeleteObject(bitmapTemp);
}

HBRUSH GetHollowBrush( void )
{
   LOGBRUSH lb;
   HBRUSH hbr = NULL;
   lb.lbStyle = BS_HOLLOW;
   lb.lbColor = RGB(0,0,0);   // ignored for HOLLOW
   lb.lbHatch = 0;            // ignored for HOLLOW
   hbr = CreateBrushIndirect( &lb );
   return hbr;
}

LPRGNDATA GetRgnDataBlock( HRGN rgn )
{
   DWORD sz = GetRegionData(rgn,0,NULL);
   BYTE * cp = MALLOC(sz);
   LPRGNDATA prd = (LPRGNDATA)cp;
   if(prd) {
      GetRegionData(rgn, sz, prd);
   }
   return prd;
}

void DrawEllipseOutline( HDC hdc, int centx, int centy, int radius )
{
   int diam = radius * 2;
   HRGN rgn = CreateEllipticRgn(0,0,diam,diam);
   if(rgn) {
      LPRGNDATA prd = GetRgnDataBlock(rgn);
      if(prd) {
         DWORD max = prd->rdh.nCount;
         DWORD maxps = sizeof(POINT) * ((max * 2) + 2);
         PPOINT pp = (PPOINT)MALLOC( maxps );
         PRECT prc;
         dbg_sprtf( "Rgn bounding rect = %s, count %d (Center %d,%d, radius %d)\n",
            GetRectStg( &prd->rdh.rcBound ),
            max, centx, centy, radius );
         prc = (PRECT)&prd->Buffer[0]; // point to FIRST rectangle
         if(pp) {
            DWORD i, k, wrap;
            int cx, cy, dt;
            int oldt = add_sys_time(0);
            POINT pt;
            DWORD hm = (max / 2);
            k = 0;
            wrap = 0;
            SecureZeroMemory(pp, maxps);
            dbg_sprtf( "First set of %d + 1 points...\n", max );
            dt = 0;
            for( i = 0; i < max; i++ ) {
               if( i == 0 ) {
                  cx = (prc->left + prc->right) / 2;
                  cy = prc->top - 1;
                  if(cy < 0)
                     cy++;
                  pp[k].x = cx;
                  pp[k].y = cy;
                  dbg_sprtf( "%s first point...\n", GetPointStg( &pp[k] ));
                  k++;
               }
               cx = prc->left;
               if( i < hm )
                  cy = prc->top;
               else {
                  if( !dt ) {
                     dbg_sprtf(" to lower half\n");
                     wrap = 0;
                     dt = 1;
                  }
                  cy = prc->bottom;
               }
               pp[k].x = cx;
               pp[k].y = cy;
               dbg_sprtf( "%s ", GetPointStg( &pp[k] ));
               k++;
               wrap++;
               if(wrap > 10) {
                  dbg_sprtf("\n");
                  wrap = 0;
               }
               prc++;
            }
            if(wrap) {
               dbg_sprtf("\n");
               wrap = 0;
            }
            dbg_sprtf( "Second set of %d + 1 points...\n", max );
            dt = 0;
            for( i = 0; i < max; i++ ) {
               prc--;
               cx = prc->right;
               if( i < hm )
                  cy = prc->bottom;
               else {
                  if( !dt ) {
                     dbg_sprtf(" to upper half\n");
                     wrap = 0;
                     dt = 1;
                  }
                  cy = prc->top;
               }
               pp[k].x = cx;
               pp[k].y = cy;
               dbg_sprtf( "%s ", GetPointStg( &pp[k] ));
               k++;
               wrap++;
               if(wrap > 10) {
                  dbg_sprtf("\n");
                  wrap = 0;
               }
            }

            if(wrap)
               dbg_sprtf( "\n" );
            // add closing POINT - back to FIRST
            pp[k].x = pp[0].x;
            pp[k].y = pp[0].y;
            dbg_sprtf( "%s last = first\n", GetPointStg( &pp[k] ));
            k++;
            // ========================================================
            dbg_sprtf( "PAINT %d points - just join lines ...(%d)\n",
               k,
               maxps / sizeof(POINT) );
            // set viewport origin to top, left ... CENTER - RADIUS
            SetViewportOrgEx(hdc, centx - radius, centy - radius, &pt); 
            Polyline(hdc, pp, k);   // paint the LINE, in the current PEN
            SetViewportOrgEx(hdc, pt.x, pt.y, NULL); // restore viewport origin 
            // ========================================================
            add_sys_time(oldt);
            MFREE(pp);
         }
         MFREE(prd);
      }
      DeleteObject(rgn);
   }
}


void DrawColoredEllipse( HDC hdc, PRECT prc, COLORREF cr )
{
   HPEN hp, old;
   HBRUSH hbr, hbrold;
   hp = CreatePen( PS_SOLID, 1, cr );
   if(hp) {
      old = (HPEN)SelectObject(hdc, hp);
      // this has the problem that it FILLS the circle
      // BUT MAYBE IF I USE A HOLLOW BRUSH
      hbr = GetHollowBrush();
      if(hbr) {
         hbrold = (HBRUSH)SelectObject(hdc, hbr);
         Ellipse( hdc, prc->left, prc->top, prc->right, prc->bottom );
         hbr = (HBRUSH)SelectObject(hdc,hbrold);
         DeleteObject(hbr);
      } else {
         // so have written my OWN outlining ...
         DrawEllipseOutline( hdc,
            (prc->left + prc->right) / 2,
            (prc->top + prc->bottom) / 2,
            RECTWIDTH(prc) / 2 );
      }

      hp = (HPEN)SelectObject(hdc, old);
      DeleteObject(hp);
   }
}


HBITMAP Dv_MagnifyPoint( HWND hWnd, PMAGNIF pm )
{
#ifdef   ADD_TIME_STAMP
   TmStamp tm;
#endif // ADD_TIME_STAMP
   HBITMAP bitmap3 = NULL;
   HRGN rgn1, rgn2, rgn3;
   HDC   hdc = GetDC(hWnd);
   HBITMAP bitmap = NULL;
   HBITMAP bitmap2 = NULL;
   HDC   hdcBmp = NULL;
   HBITMAP hold, hold2, hold3;
   HBRUSH   hbr = NULL;
   HDC   hdcMem = NULL;
   HDC   hdcKeep = NULL;
   COLORREF rgbTransparent = pm->trans;   // gMagTC;   // RGB(255, 0, 254);
   //SIZE szIN, szOUT;
   PSIZE pszin  = &pm->szin;
   PSIZE pszout = &pm->szout;
   POINT point = pm->pt;
   INT   xi2, yi2;
   INT   xo2, yo2;

   // INNER AND OUTER SIZES
   //pszin->cx  = gMagICX; // 40;
   //pszin->cy  = gMagICY; // 40;
   //pszout->cx = gMagOCX; // 120
   //pszout->cy = gMagOCY; // 120
   // half INNER and OUTER sizes
   xi2 = pszin->cx  / 2;
   yi2 = pszin->cy  / 2;
   xo2 = pszout->cx / 2;
   yo2 = pszout->cy / 2;

   // create regions
   rgn1 = CreateEllipticRgn(0,0,pszout->cx,pszout->cy);
   rgn2 = CreateRectRgn(0,0,pszout->cx,pszout->cy);
   rgn3 = CreateRectRgn(0,0,pszout->cx,pszout->cy);
   if( !rgn1 || !rgn2 || !rgn3 )
      goto Cleanup;

   // ==============================================================
   CombineRgn(rgn3,rgn2,rgn1, RGN_DIFF); // subtract center ellipse,
   // from outer rectangle - gets a complex region
   // Try outputRgn() for diagnostics only
   // OutputRgn( hdc, rgn1 );
   // OutputRgn( hdc, rgn3 ); // this PAINTS all the RECT to the HDC
   // ==============================================================

   if( !hdc )
      goto Cleanup;

   hbr = CreateSolidBrush( rgbTransparent );
   if( !hbr )
      goto Cleanup;

   hdcBmp = CreateCompatibleDC(hdc);
   if( !hdcBmp )
      goto Cleanup;
   hdcMem = CreateCompatibleDC(hdc);
   if( !hdcMem )
      goto Cleanup;
   hdcKeep = CreateCompatibleDC(hdc);
   if( !hdcKeep )
      goto Cleanup;

	bitmap  = CreateCompatibleBitmap( hdc, pszin->cx, pszin->cy );
   if( !bitmap )
      goto Cleanup;

   bitmap2 = CreateCompatibleBitmap( hdc, pszout->cx, pszout->cy );
   if( !bitmap2 )
      goto Cleanup;

   bitmap3 = CreateCompatibleBitmap( hdc, pszout->cx, pszout->cy );
   if( !bitmap3 )
      goto Cleanup;

   hold = (HBITMAP)SelectObject(hdcBmp,bitmap);
   hold2 = (HBITMAP)SelectObject(hdcMem,bitmap2);

   // get the OUTTER rectangle, for replacement
   // =========================================
   hold3 = (HBITMAP)SelectObject(hdcKeep,bitmap3);
   BitBlt( hdcKeep, 0, 0, pszout->cx, pszout->cy,
      hdc, point.x - xo2, point.y - yo2, SRCCOPY );
   bitmap3 = (HBITMAP)SelectObject(hdcKeep,hold3);
   DeleteDC(hdcKeep);
   // ==========================================

   // get the INNER rectangle
   BitBlt( hdcBmp, 0, 0, pszin->cx, pszin->cx,
      hdc, point.x - xi2, point.y - yi2, SRCCOPY );

   // expand this inner rectangle to fill the outer rectangle
   //StretchBlt(hdc, point.x, point.y, 120, 120, hdcBmp, 0, 0, 40, 40, SRCCOPY);
   StretchBlt(hdcMem, 0, 0, pszout->cx, pszout->cy,
      hdcBmp, 0, 0, pszin->cx, pszin->cy, SRCCOPY);

   // fill the outside the circle with the transparent color
   FillRgn( hdcMem, rgn3, hbr );


   // HAD VARIOUS TRIES AT COPYING THE EXPANDED RECTANGLE TO THE SCREEN,
   // excluding the surrounding transparent region ... until SUCCESS
   bitmap2 = (HBITMAP)SelectObject(hdcMem,hold2);

   Dv_DrawTransparent( hdc, bitmap2, point.x-xo2, point.y-yo2, rgbTransparent );

   // MAYBE OUTLINE THE CIRCLE!
   //	dc.SelectObject(&CPen(PS_SOLID, 1, RGB(0, 0, 0)));
	//	dc.SelectObject(&CBrush(NULL, RGB(0, 0, 0)));
	//	dc.Ellipse(point.x-(60*1)-1, point.y-(60*1)-1, point.x+(60*1), point.y+(60*1));
   if( bAddOutline )
   {
      RECT rc;
      rc.left   = point.x-xo2;
      rc.top    = point.y-yo2;
      rc.right  = rc.left + pszout->cx;
      rc.bottom = rc.top + pszout->cy;
      DrawColoredEllipse( hdc, &rc, gMagOL );
   }

   SelectObject(hdcBmp,hold);

   pm->hBitmap2 = bitmap2;
   bitmap2 = NULL;

Cleanup:
   if(hdc)
      ReleaseDC(hWnd, hdc);
   if(bitmap)
      DeleteObject(bitmap);
   if(bitmap2)
      DeleteObject(bitmap2);
   if(hdcBmp)
      DeleteDC(hdcBmp);
   if(hdcMem)
      DeleteDC(hdcMem);
   if(hbr)
      DeleteObject(hbr);
   if(rgn1)
      DeleteObject(rgn1);
   if(rgn2)
      DeleteObject(rgn2);
   if(rgn3)
      DeleteObject(rgn3);

#ifdef   ADD_TIME_STAMP
   {
      long elap = tm.elapsed();
      sprtf( "test_MagnifyPoint: Elapsed = %ld usecs (%0.5f secs)\n",
         elap,
         tm.get_secs_d(elap) );
   }
#endif // ADD_TIME_STAMP

   return bitmap3;
}

void Dv_ReplaceMagnifyDC( HDC hdc, PMAGNIF pm )
{
   HDC hdcMem = NULL;
   HBITMAP hold;
   POINT point = pm->pt;
   int xo2 = pm->szout.cx / 2;
   int yo2 = pm->szout.cy / 2;
   if(!hdc) // nothing to do!!!
      goto Cleanup;
   if( !pm->hBitmap1 )
      goto Cleanup;
   // create compatible device context
   hdcMem = CreateCompatibleDC(hdc);
   if(!hdcMem)
      goto Cleanup;
   // select bitmap into context
   hold = (HBITMAP)SelectObject(hdcMem, pm->hBitmap1);
   // bli it to the destination
   BitBlt( hdc, point.x - xo2, point.y - yo2, pm->szout.cx, pm->szout.cy,
      hdcMem, 0, 0, SRCCOPY );
   // select out the bitmap
   SelectObject(hdcMem, hold);
   
Cleanup:
   if( hdcMem )
      DeleteDC(hdcMem);
   if( pm->hBitmap1 )
      DeleteObject( pm->hBitmap1 );
   pm->hBitmap1 = NULL;
}

void Dv_ReplaceMagnify( HWND hWnd, PMAGNIF pm )
{
   HDC hdc = GetDC(hWnd);
   if( hdc ) // got it
   {
      Dv_ReplaceMagnifyDC( hdc, pm );
      ReleaseDC(hWnd,hdc);
   }
}

void Setup_Magnify( PMAGNIF pm )
{
   // INNER AND OUTER SIZES
   pm->szin.cx  = gMagICX; // 40;
   pm->szin.cy  = gMagICY; // 40;
   pm->szout.cx = gMagOCX; // 120
   pm->szout.cy = gMagOCY; // 120
   // and transparent color to use - can be any (I think)
   pm->trans = gMagTC;
}


void Toggle_Magnify( HWND hWnd, POINT pt, PMAGNIF pm )
{
   if( pm->hBitmap1 ) {
      Dv_ReplaceMagnify( hWnd, pm );
      pm->hBitmap1 = NULL;
   } else {
      Setup_Magnify( pm );
      // do it around THIS POINT
      pm->pt.x = pt.x;
      pm->pt.y = pt.y;
      pm->hBitmap1 = Dv_MagnifyPoint( hWnd, pm );
   }
}

void Magnify2OffDC( HDC hdc, PMAGNIF pm )
{
   if( pm->hBitmap1 ) {
      Dv_ReplaceMagnifyDC( hdc, pm );
      pm->hBitmap1 = NULL;
   }
   if( pm->hBitmap2 ) {
      DeleteObject(pm->hBitmap2);
      pm->hBitmap2 = NULL;
   }
}

void Magnify2Off( HWND hWnd, PMAGNIF pm )
{
   if( pm->hBitmap1 ) {
      Dv_ReplaceMagnify( hWnd, pm );
      pm->hBitmap1 = NULL;
   }
   if( pm->hBitmap2 ) {
      DeleteObject(pm->hBitmap2);
      pm->hBitmap2 = NULL;
   }
}

void MouseMagnify( HWND hWnd, LPARAM lParam, LPDIBINFO lpDIBInfo )
{
	MSG   msg;
   RECT  rcClient;
   POINT point;
   PMAGNIF pm = &lpDIBInfo->sMagnif;

   if( pm->hBitmap1 )
      Magnify2Off( hWnd, pm );  // replace any OLD

   Setup_Magnify( pm );  // set any NEW parameters

   point.x = LOWORD (lParam);
   point.y = HIWORD (lParam);

   SetCapture( hWnd );

   GetWindowRect( hWnd, &rcClient );

   Toggle_Magnify( hWnd, point, &lpDIBInfo->sMagnif );   // paint FIRST

   for( ; ; )
   {
		WaitMessage(); // suspended, until message available
		// PeekMessage Return Values
		// If a message is available, the return value is nonzero.
		if( PeekMessage( &msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) )
		{
         if( pm->hBitmap1 )
            Magnify2Off( hWnd, pm );  // remove OLD

         point.x = LOWORD (msg.lParam);
         point.y = HIWORD (msg.lParam);
         pm->pt.x = point.x;
         pm->pt.y = point.y;
         pm->hBitmap1 = Dv_MagnifyPoint( hWnd, pm );

			if( msg.message == WM_LBUTTONUP ) {
            break; // we have completed tracking
			}
      }
   }

   ReleaseCapture();
}

BOOL IsMagnifyOn( PMAGNIF pm )
{
   if( pm->hBitmap1 && pm->hBitmap2 ) {
      return TRUE;
   }
   return FALSE;
}


void RePaintMagnify( HDC hdc, LPDIBINFO lpDIBInfo )
{
   PMAGNIF pm = &lpDIBInfo->sMagnif;
   if( IsMagnifyOn(pm) ) {
      HBITMAP bitmap2 = pm->hBitmap2;
      COLORREF rgbTransparent = pm->trans;
      PSIZE pszin  = &pm->szin;
      PSIZE pszout = &pm->szout;
      POINT point = pm->pt;
      INT xo2 = pszout->cx / 2;
      INT yo2 = pszout->cy / 2;

      Dv_DrawTransparent( hdc, bitmap2, point.x-xo2, point.y-yo2, rgbTransparent );

      // MAYBE OUTLINE THE CIRCLE!
      //	dc.SelectObject(&CPen(PS_SOLID, 1, RGB(0, 0, 0)));
	   //	dc.SelectObject(&CBrush(NULL, RGB(0, 0, 0)));
	   //	dc.Ellipse(point.x-(60*1)-1, point.y-(60*1)-1, point.x+(60*1), point.y+(60*1));
      if( bAddOutline )
      {
         RECT rc;
         rc.left   = point.x-xo2;
         rc.top    = point.y-yo2;
         rc.right  = rc.left + pszout->cx;
         rc.bottom = rc.top + pszout->cy;
         DrawColoredEllipse( hdc, &rc, gMagOL );
      }
   }
}

// Other references viewed
// http://www.winehq.org/pipermail/wine-patches/2006-August/029286.html
// http://www.cs.cmu.edu/afs/cs.cmu.edu/project/gwydion/external-sources/tk4.1a2/win/tkWinDraw.c
// http://source.winehq.org/source/dlls/comctl32/imagelist.c

VOID Do_IDM_MAGNIFY( HWND hWnd )
{
   if( gi_MouseMagnify )
      gi_MouseMagnify = 0;
   else
      gi_MouseMagnify = 1;
   gChgMag = TRUE;
}

/* ======================================================
IDD_MAGNIFY DIALOGEX 0, 0, 204, 225
STYLE DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | 
    WS_SYSMENU
CAPTION "Magnify Options"
FONT 8, "MS Shell Dlg", 400, 0, 0x1
BEGIN
    CONTROL         "Magnify Mode On",IDC_CHECK1,"Button",BS_AUTOCHECKBOX | 
                    WS_TABSTOP,18,17,85,10
    LTEXT           "Inner",IDC_STATIC,65,31,39,8
    LTEXT           "Outer",IDC_STATIC,114,30,48,10
    LTEXT           "Width (x)",IDC_STATIC,7,44,50,8
    LTEXT           "Height (y)",IDC_STATIC,7,57,50,8
    EDITTEXT        IDC_EDIT1,63,42,34,12,ES_AUTOHSCROLL
    EDITTEXT        IDC_EDIT15,112,41,34,12,ES_AUTOHSCROLL
    EDITTEXT        IDC_EDIT2,63,55,34,12,ES_AUTOHSCROLL
    EDITTEXT        IDC_EDIT3,111,55,34,12,ES_AUTOHSCROLL
    GROUPBOX        "RGB Colors",IDC_STATIC,14,71,175,59
    LTEXT           "Trasparent",IDC_STATIC,18,82,54,9
    LTEXT           "Outline",IDC_STATIC,17,94,47,9
    EDITTEXT        IDC_EDIT4,79,81,20,12,ES_AUTOHSCROLL
    EDITTEXT        IDC_EDIT5,99,81,20,12,ES_AUTOHSCROLL
    EDITTEXT        IDC_EDIT6,119,81,20,12,ES_AUTOHSCROLL
    EDITTEXT        IDC_EDIT7,79,94,20,12,ES_AUTOHSCROLL
    EDITTEXT        IDC_EDIT8,99,94,20,12,ES_AUTOHSCROLL
    EDITTEXT        IDC_EDIT9,120,94,20,12,ES_AUTOHSCROLL
    CONTROL         "Add Outline",IDC_CHECK2,"Button",BS_AUTOCHECKBOX | 
                    WS_TABSTOP,17,108,62,13
    DEFPUSHBUTTON   "OK",IDOK,101,203,50,14
    PUSHBUTTON      "Cancel",IDCANCEL,38,203,50,14
    GROUPBOX        "Magnify Options",IDC_STATIC,7,7,190,128
    GROUPBOX        "Swish Options",IDC_STATIC,7,140,190,56
    LTEXT           "Pen Width",IDC_STATIC,19,159,67,11
    EDITTEXT        IDC_EDIT10,102,159,29,13,ES_AUTOHSCROLL
    LTEXT           "Pen Color (RGB)",IDC_STATIC,20,180,54,10
    EDITTEXT        IDC_EDIT11,86,177,20,12,ES_AUTOHSCROLL
    EDITTEXT        IDC_EDIT12,105,177,20,12,ES_AUTOHSCROLL
    EDITTEXT        IDC_EDIT13,123,177,20,12,ES_AUTOHSCROLL
    PUSHBUTTON      "Select",IDC_BUTTON1,143,81,36,13
    PUSHBUTTON      "Select",IDC_BUTTON2,144,95,36,13
    PUSHBUTTON      "Select",IDC_BUTTON3,145,177,36,13
END
    ===================================================== */

typedef enum {
   OPT_VALUE = 1,
   OPT_COLORR,
   OPT_COLORG,
   OPT_COLORB,
}DLGOP;


typedef struct tagDLGOPTIONS {
   UINT  id;
   DLGOP type;
   PTSTR popt;
   PBOOL pchg;
   UINT  idsel;
}DLGOPTIONS, * PDLGOPTIONS;

DLGOPTIONS dlg_options[] = {
   { IDC_EDIT15, OPT_VALUE, (PTSTR)&gMagOCX, &gChgOCX, 0 },
   { IDC_EDIT3,  OPT_VALUE, (PTSTR)&gMagOCY, &gChgOCY, 0 },
   { IDC_EDIT1, OPT_VALUE, (PTSTR)&gMagICX, &gChgICX, 0 },
   { IDC_EDIT2,  OPT_VALUE, (PTSTR)&gMagICY, &gChgICY, 0 },

   { IDC_EDIT4,  OPT_COLORR, (PTSTR)&gMagTC, &gChgTC, IDC_BUTTON1 }, 
   { IDC_EDIT5,  OPT_COLORG, (PTSTR)&gMagTC, &gChgTC, IDC_BUTTON1 }, 
   { IDC_EDIT6,  OPT_COLORB, (PTSTR)&gMagTC, &gChgTC, IDC_BUTTON1 },

   { IDC_EDIT7,  OPT_COLORR, (PTSTR)&gMagOL, &gChgOL, IDC_BUTTON2 }, 
   { IDC_EDIT8,  OPT_COLORG, (PTSTR)&gMagOL, &gChgOL, IDC_BUTTON2 }, 
   { IDC_EDIT9,  OPT_COLORB, (PTSTR)&gMagOL, &gChgOL, IDC_BUTTON2 },

   { IDC_EDIT10, OPT_VALUE,  (PTSTR)&gPenW, &gChgPen, 0 },
   { IDC_EDIT11,  OPT_COLORR, (PTSTR)&gPenC, &gChgPC, IDC_BUTTON3 }, 
   { IDC_EDIT12,  OPT_COLORG, (PTSTR)&gPenC, &gChgPC, IDC_BUTTON3 }, 
   { IDC_EDIT13,  OPT_COLORB, (PTSTR)&gPenC, &gChgPC, IDC_BUTTON3 },

   // LAST ENTRY
   { 0,  0, 0, 0 }
};

VOID Process_Table( PDLGOPTIONS pdo, HWND hDlg, BOOL set )
{
   COLORREF * pcr;
   UINT * pui;
   UINT  ui;
   if( set )
   {
      while( pdo->id )
      {
         switch( pdo->type )
         {
         case OPT_VALUE:
            pui = (UINT *)pdo->popt;
            SetDlgItemInt( hDlg, pdo->id, *pui, FALSE );
            break;
         case OPT_COLORR:
            pcr = (COLORREF *)pdo->popt;
            SetDlgItemInt( hDlg, pdo->id, GetRValue(*pcr), FALSE );
            break;
         case OPT_COLORG:
            pcr = (COLORREF *)pdo->popt;
            SetDlgItemInt( hDlg, pdo->id, GetGValue(*pcr), FALSE );
            break;
         case OPT_COLORB:
            pcr = (COLORREF *)pdo->popt;
            SetDlgItemInt( hDlg, pdo->id, GetBValue(*pcr), FALSE );
            break;
         }
         pdo++;
      }
   } else {
      // get values
      BOOL  b;
      COLORREF cr;
      while( pdo->id )
      {
         switch( pdo->type )
         {
         case OPT_VALUE:
            pui = (UINT *)pdo->popt;
            ui = GetDlgItemInt( hDlg, pdo->id, &b, FALSE );
            if( ui != *pui )
            {
               *pui = ui;
               *pdo->pchg = TRUE;
            }
            break;
         case OPT_COLORR:
            pcr = (COLORREF *)pdo->popt;
            ui = GetDlgItemInt( hDlg, pdo->id, &b, FALSE );
            cr = RGB( ui, GetGValue(*pcr), GetBValue(*pcr) );
            if( cr != *pcr )
            {
               *pcr = cr;
               *pdo->pchg = TRUE;
            }
            break;
         case OPT_COLORG:
            pcr = (COLORREF *)pdo->popt;
            ui = GetDlgItemInt( hDlg, pdo->id, &b, FALSE );
            cr = RGB( GetRValue(*pcr), ui, GetBValue(*pcr) );
            if( cr != *pcr )
            {
               *pcr = cr;
               *pdo->pchg = TRUE;
            }
            break;
         case OPT_COLORB:
            pcr = (COLORREF *)pdo->popt;
            ui = GetDlgItemInt( hDlg, pdo->id, &b, FALSE );
            cr = RGB( GetRValue(*pcr), GetGValue(*pcr), ui );
            if( cr != *pcr )
            {
               *pcr = cr;
               *pdo->pchg = TRUE;
            }
            break;
         }
         pdo++;
      }
   }
}

INT_PTR InitMagnifyDialog( HWND hDlg )
{
   Process_Table( &dlg_options[0], hDlg, TRUE );
   CheckRadioButton( hDlg, IDC_RADIO1, IDC_RADIO3,
      ( gbSwish ? IDC_RADIO1 : gbOval ? IDC_RADIO2 : IDC_RADIO3 ) );
	CenterWindow( hDlg, GetWindow( hDlg, GW_OWNER ) );
   return TRUE;
}

COLORREF Choose_New_Color( HWND hDlg, COLORREF * pcr )
{
	CHOOSECOLOR	cc;
   BYTE cr, cg, cb;
   BYTE ncr, ncg, ncb;
   COLORREF ncolr = *pcr;
	cr = GetRValue( *pcr );
	cg = GetGValue( *pcr );
   cb = GetBValue( *pcr );
	cc.lStructSize = sizeof( CHOOSECOLOR );
	cc.hwndOwner = hDlg;
	cc.hInstance = (HWND) ghDvInst;
	cc.rgbResult = RGB( cr, cg, cb );
	cc.lpCustColors = &CustColors[0]; 
	cc.Flags = CC_ANYCOLOR | CC_RGBINIT;
	cc.lCustData = 0;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;
	if( ChooseColor( &cc ) )
	{
		ncr = GetRValue( cc.rgbResult );
		ncg = GetGValue( cc.rgbResult );
		ncb = GetBValue( cc.rgbResult );
		if((cr != ncr ) ||
			(cg != ncg ) ||
			(cb != ncb ) )
		{
			ncolr = RGB( ncr, ncg, ncb );
      }
   }
   return ncolr;
}


VOID Process_Color_Select( PDLGOPTIONS pdo, HWND hDlg, UINT cmd )
{
   while( pdo->id )
   {
      if( pdo->idsel == cmd )
      {
         COLORREF * pcr = (COLORREF *)pdo->popt;
         COLORREF ncolr = Choose_New_Color( hDlg, pcr );
         if( ncolr != *pcr )
         {
            *pcr = ncolr;
            *pdo->pchg = TRUE;
            Process_Table( &dlg_options[0], hDlg, TRUE );
         }
         break;
      }
      pdo++;
   }
}

VOID  Set_Swish_ON(VOID)
{
   if( !gbSwish ) {
      gbSwish = TRUE;
      gbChgSwi = TRUE;
   }
   if( gbOval ) {
      gbOval = FALSE;
      gbChgOva = TRUE;
   }
}
VOID Set_Oval_ON(VOID)
{
   if( gbSwish ) {
      gbSwish = FALSE;
      gbChgSwi = TRUE;
   }
   if( !gbOval ) {
      gbOval = TRUE;
      gbChgOva = TRUE;
   }
}
VOID Set_Square_ON(VOID)
{
   if( gbSwish ) {
      gbSwish = FALSE;
      gbChgSwi = TRUE;
   }
   if( gbOval ) {
      gbOval = FALSE;
      gbChgOva = TRUE;
   }
}

VOID GetRadioButtons( HWND hDlg )
{
   if( IsDlgButtonChecked( hDlg, IDC_RADIO1 ) == BST_CHECKED ) {
      Set_Swish_ON();   // turn ON Swish, off Oval
   } else if( IsDlgButtonChecked( hDlg, IDC_RADIO2 ) == BST_CHECKED ) {
      Set_Oval_ON();    // turn ON Oval, off Swish
   } else { // if( IsDlgButtonChecked( hDlg, IDC_RADIO3 ) == BST_CHECKED ) {
      Set_Square_ON();  // turn OFF Swish AND Oval
   }
}

INT_PTR CALLBACK MAGNIFYDLG(
  HWND hDlg,  // handle to dialog box
  UINT message,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
   switch (message)
   {
      case WM_INITDIALOG:
         return InitMagnifyDialog( hDlg );

      case WM_COMMAND:
         {
            DWORD cmd = LOWORD(wParam);
            if( ( cmd == IDOK     ) ||  // "OK" box selected?        
                ( cmd == IDCANCEL ) )   // System menu close command?
            {
               if( cmd == IDOK ) {
                  Process_Table( &dlg_options[0], hDlg, FALSE );
                  GetRadioButtons( hDlg );
               }
               EndDialog(hDlg, cmd);     // Exits the dialog box 
               return TRUE;
            } else {
               Process_Color_Select( &dlg_options[0], hDlg, cmd );
            }
         }
         break;
    }
   return FALSE;                    // Didn't process a message
}


VOID Do_IDM_OPTION6( HWND hWnd )
{
	DialogBox( ghDvInst,             // current instance
		MAKEINTRESOURCE(IDD_MAGNIFY),        // resource to use 
		hWnd,              // parent handle
		MAGNIFYDLG );      // instance address

}

// ========================================
// SWISH STUFF

BOOL bUseBitBlt = TRUE;

HBITMAP Get_DCBitmap( HDC hdc, PRECT prc )
{
   HDC hdcKeep = NULL;
   HBITMAP  bitmap3 = NULL;
   HBITMAP  bitmap = NULL;
   HBITMAP  hold3 = NULL;

   if(( RECTWIDTH(prc) <= 0 ) ||
      ( RECTHEIGHT(prc) <= 0 ))
      return NULL;
   
   hdcKeep = CreateCompatibleDC(hdc);
   if( !hdcKeep )
      goto Cleanup;

   bitmap3 = CreateCompatibleBitmap( hdc, RECTWIDTH(prc), RECTHEIGHT(prc) );
   if( !bitmap3 )
      goto Cleanup;

   hold3 = (HBITMAP)SelectObject(hdcKeep,bitmap3);

   BitBlt( hdcKeep, 0, 0, RECTWIDTH(prc), RECTHEIGHT(prc),
      hdc, prc->left, prc->top, SRCCOPY );

   bitmap = (HBITMAP)SelectObject(hdcKeep,hold3);
   bitmap3 = NULL;

Cleanup:

   if( hdcKeep )
      DeleteDC( hdcKeep );

   if(bitmap3)
      DeleteObject(bitmap3);

   return bitmap;
}

// Copy the BITMAP saved, back to the screen
// The problem is that the image can have been SCROLLED,
// and thus this REPLACEMENT is in the incorrect position!!!
// HOW TO ENSURE IT IS PAINTED TO THE CORRECT PLACE???
BOOL ReplaceClip( HDC hdc, HBITMAP hbm, PRECT prcClip )
{
   HDC hMemDCsrc;
	HBITMAP hOldBitmap;

   if( !hbm )
      return FALSE;

	hMemDCsrc = CreateCompatibleDC( NULL );
   if( !hMemDCsrc )
      return FALSE;

	hOldBitmap = (HBITMAP)SelectObject( hMemDCsrc, hbm );   // current is SOURCE

   if( bUseBitBlt ) {
      BitBlt( hdc,
         prcClip->left,
         prcClip->top,
         RECTWIDTH(prcClip),
         RECTHEIGHT(prcClip),
         hMemDCsrc,
         0,
         0,
         SRCCOPY );
   } else {
      StretchBlt( hdc,                      // handle to destination DC
         prcClip->left,             // x-coord of destination upper-left corner
         prcClip->top,             // y-coord of destination upper-left corner
         RECTWIDTH(prcClip),        // width of destination rectangle
         RECTHEIGHT(prcClip),       // height of destination rectangle
         hMemDCsrc,                 // handle to source DC
         0,                         // x-coord of source upper-left corner
         0,                         // y-coord of source upper-left corner
         RECTWIDTH(prcClip),        // width of source rectangle
         RECTHEIGHT(prcClip),       // height of source rectangle
         SRCCOPY );                 // raster operation code
   }

	SelectObject( hMemDCsrc, hOldBitmap );   // swap out current is SOURCE

   DeleteDC(hMemDCsrc);

   return TRUE;
}

void Replace_Swish( HDC hDC, PRECT prc, PCLIPSWISH pcs )
{
   if( pcs->hBitmap ) {
      ReplaceClip( hDC, pcs->hBitmap, prc );
      DeleteObject(pcs->hBitmap);
      pcs->hBitmap = NULL;
   }
}

BOOL Do_Arc( HDC hdc, PRECT prc, PPOINT pptB, PPOINT pptE )
{
   return( Arc(hdc,
       prc->left,
       prc->top,
       prc->right,
       prc->bottom,
       pptB->x,
       pptB->y,
       pptE->x,
       pptE->y ) );
}

void DrawLine( HDC hdc, PPOINT pS, PPOINT pE )
{
   MoveToEx( hdc, pS->x, pS->y, NULL );
   LineTo( hdc, pE->x, pE->y );
}

void Draw_Colored_Swish( HDC hdc, PCLIPSWISH pcs )
{
   HPEN  hp;
   HPEN  hpOld = 0;
   int   pw2;

   hp = CreatePen( PS_SOLID,  // int fnPenStyle,    // pen style
      pcs->penwidth, // int nWidth,        // pen width
      pcs->cr ); //  COLORREF crColor   // pen color
   if(hp)
      hpOld = (HPEN)SelectObject(hdc, hp);

   Make_Scroll_Adjustment( ghActMDIWnd, hdc, &pcs->rc );

   if( pcs->hBitmap == NULL )
      pcs->hBitmap = Get_DCBitmap( hdc, &pcs->rc );

   pw2 = pcs->penwidth / 2;
   if( pcs->penwidth % 2 )
      pw2++;

   if( gbSwish ) {
      Do_Arc( hdc, &pcs->src1, &pcs->ptB1, &pcs->ptE1 );
      Do_Arc( hdc, &pcs->src2, &pcs->ptB2, &pcs->ptE2 );
   } else if( gbOval ) {
      dbg_oval("DEBUG:Do_Swish_Toggle:Draw_Colored_Swish: Arc to RECT %s...\n",
         GetRectStg(&pcs->rc) );
      Arc(hdc,
         pcs->rc.left + pw2,
         pcs->rc.top + pw2,
         pcs->rc.right - pw2,
         pcs->rc.bottom - pw2,
         pcs->rc.left,
         pcs->rc.top,
         pcs->rc.left,
         pcs->rc.top );
   } else {
      // NOT Swish NOR Oval, to DO SQUARE
      RECT  rc = pcs->rc;
      POINT pb, pe;
      // top line
      pb.x = rc.left + pw2;
      pb.y = rc.top  + pw2;
      pe.x = rc.right - pw2;
      pe.y = rc.top  + pw2;
      DrawLine( hdc, &pb, &pe );
      // right side
      pb.x = rc.right - pw2;
      pb.y = rc.top  + pw2;
      pe.x = rc.right - pw2;
      pe.y = rc.bottom - pw2;
      DrawLine( hdc, &pb, &pe );
      // bottom
      pb.x = rc.left + pw2;
      pb.y = rc.bottom - pw2;
      pe.x = rc.right - pw2;
      pe.y = rc.bottom - pw2;
      DrawLine( hdc, &pb, &pe );
      // left side
      pb.x = rc.left + pw2;
      pb.y = rc.top  + pw2;
      pe.x = rc.left + pw2;
      pe.y = rc.bottom - pw2;
      DrawLine( hdc, &pb, &pe );
   }

   if(hpOld)
      hpOld = (HPEN)SelectObject(hdc, hpOld);

   Reset_Scroll_Adjustment( ghActMDIWnd, hdc, &pcs->rc );
}

void Set_Colored_Swish( HDC hdc, PRECT prc, int pw, COLORREF cr, PCLIPSWISH pcs )
{
   RECT rc = *prc;
   //if(( !EqualRect( &pcs->rc, &rc ) ) ||
   //   ( pcs->penwidth != pw ) ||
   //   ( pcs->cr != cr ) ) {
      int   halfp = pw / 2;
      pcs->rc = rc;
      pcs->penwidth = pw;
      pcs->cr = cr;
      rc.left   += halfp;
      rc.right  -= halfp;
      rc.top    += halfp;
      rc.bottom -= halfp;

      // full rectangle
      pcs->src1 = rc;
      // start on the 3/4 X, top Y radial
      pcs->ptB1.x = rc.left + ((RECTWIDTH(&rc) * 3) / 4);
      pcs->ptB1.y = rc.top;
      // end on the 1/2 X, bottom Y radial
      pcs->ptE1.x = (rc.left + rc.right) / 2;
      pcs->ptE1.y = rc.bottom;

      // inner rectangle (smaller, to wrap inside larger)
      rc.top += pw + 2; // adjust rectangle
      pcs->src2 = rc;
      // start on 1/2 X, Y bottom radial
      pcs->ptB2.x = (rc.left + rc.right) / 2;
      pcs->ptB2.y = rc.bottom;
      // end on the 1/4 X, Y top radial
      pcs->ptE2.x = rc.left + (RECTWIDTH(&rc) / 4);
      pcs->ptE2.y = rc.top;

   //}

   if((RECTWIDTH(prc) > ( 4 * pw )) &&
      (RECTHEIGHT(prc) > ( 4 * pw ))) {
         dbg_oval("DEBUG:Do_Swish_Toggle: Doing Draw_Colored swish ...\n");
         Draw_Colored_Swish( hdc, pcs );
   }
}

void Draw_CLIP( HDC hdc, PRECT pclip, LPDIBINFO lpDIBInfo )
{
   DrawSelect( hdc, pclip );   // remove SELECT
   Set_Colored_Swish( hdc, pclip, gPenW, gPenC, &lpDIBInfo->sSwish );
   DrawSelect( hdc, pclip );   // add SELECT back
}


VOID  DC_ClearClip( HDC hdc, PRECT prc, LPDIBINFO lpDIBInfo )
{

	DrawSelect( hdc, prc );    // remove RECTANGLE around

   Replace_Swish( hdc, prc, &lpDIBInfo->sSwish ); // put back any SWISH bitmap

   Magnify2OffDC( hdc, &lpDIBInfo->sMagnif );      // put bach any Magnify bitmap

}

BOOL Do_IDM_CLEARCLIP( HWND hWndFrame )
{
	HANDLE		hDIBInfo;
	LPDIBINFO	lpDIBInfo;
   HDC         hdc;
   HWND        hWnd = GetCurrentMDIWnd();

   if( !hWnd )
      return FALSE;

   if( !Get_DIB_Info( hWnd, &hDIBInfo, &lpDIBInfo ) )
      return FALSE;

   hdc = GetDC( hWnd );
   if( !hdc ) {
      Release_DIB_Info( hWnd, &hDIBInfo, &lpDIBInfo );
      return FALSE;
   }

	DC_ClearClip( hdc, &lpDIBInfo->rcClip, lpDIBInfo );

   ReleaseDC( hWnd, hdc );

   SetRect( &lpDIBInfo->rcClip, 0, 0, 0, 0 );

   Release_DIB_Info( hWnd, &hDIBInfo, &lpDIBInfo );

   return TRUE;
}


BOOL Do_Swish_Toggle( HWND hWndFrame, BOOL toggle )
{
	HANDLE		hDIBInfo;
	LPDIBINFO	lpDIBInfo;
   HDC         hdc;
   PRECT       pclip;
   PCLIPSWISH  pcs;
   HWND        hWnd = GetCurrentMDIWnd();

   if( !hWnd )
      return FALSE;

   if( !Get_DIB_Info( hWnd, &hDIBInfo, &lpDIBInfo ) )
   {
      dbg_oval("DEBUG:Do_Swish_Toggle: FAILED to get DIB Info ...\n");
      return FALSE;
   }
   hdc = GetDC( hWnd );
   if( !hdc ) {
      dbg_oval("DEBUG:Do_Swish_Toggle: FAILED to get hdc ...\n");
      Release_DIB_Info( hWnd, &hDIBInfo, &lpDIBInfo );
      return FALSE;
   }

   pclip = &lpDIBInfo->rcClip;
   pcs   = &lpDIBInfo->sSwish;

   if( toggle )
   {  // TOGGLE = If ON, take OFF
      // ELSE = put ON
      if( Got_Swish( pcs ) ) {
         Replace_Swish( hdc, pclip, pcs ); // put back any SWISH bitmap
         DrawSelect( hdc, pclip );   // add SELECT back
      } else {
         Draw_CLIP( hdc, pclip, lpDIBInfo );
      }
   } else {
      // NOT TOGGLE = if ON, take it OFF
      if( Got_Swish( pcs ) ) {
         Replace_Swish( hdc, pclip, pcs ); // put back any SWISH bitmap
         DrawSelect( hdc, pclip );   // add SELECT back
      }
      // but ALWAYS put it ON
      dbg_oval("DEBUG:Do_Swish_Toggle: Painting SWISH ...\n");
      Draw_CLIP( hdc, pclip, lpDIBInfo );
   }

   ReleaseDC( hWnd, hdc );

   Release_DIB_Info( hWnd, &hDIBInfo, &lpDIBInfo );

   return TRUE;
}

BOOL Do_IDM_ADDSWISH( HWND hWndFrame )
{
   return Do_Swish_Toggle( hWndFrame, TRUE );
}

PTSTR Menu_Swish(VOID)  { return "Paint Swish";  }
PTSTR Menu_Oval(VOID)   { return "Paint Oval";   }
PTSTR Menu_Square(VOID) { return "Paint Square"; }

PTSTR GetSwishMenu( VOID )
{
   PTSTR pstr = (gbSwish ? Menu_Swish() : gbOval ? Menu_Oval() : Menu_Square());
   return pstr;
}

// ========================================
#define  ISNUM(a) (( a >= '0' )&&( a <= '9' ))

BOOL BumpFileName( PTSTR pt )
{
   size_t len = strlen(pt);
   size_t i;
   int   val;
   PTSTR pf = gszNewFile;
   strcpy(pf,pt);
   for( i = (len - 1); i >= 0; i-- )
   {
      val = pt[i];
      if( ISNUM(val) )
      {
         if( val < '9' ) {
            val++;
            pt[i] = (TCHAR)val;
            return TRUE;
         } else {
            pt[i] = '0';   // set to ZERO an back up one
         }
      } else {
         pt[i] = '0';   // convert to '0', and return
         return TRUE;
      }
   }
   // we could EXTEND the size
   if( len < (MAX_PATH - 4) )
   {
      strcpy(pt,pf);
      strcat(pt,"0");   // append a ZERO
      return TRUE;      // and try that ...
   }

   return FALSE;
}

//TCHAR w_szDrive[_MAX_DRIVE+8];	// gszDrive - Drive
//TCHAR w_szDir[_MAX_DIR+8];		   // gszDir   - Directory
//TCHAR w_szFname[_MAX_FNAME+8];	// gszFname - Filename
//TCHAR	w_szExt[_MAX_EXT+8];		   // gszExt   - Extension
//TCHAR w_szNewName[264];          // gszNewName
PTSTR GetNextFileName( PTSTR pfile )
{
   PTSTR lpf = gszNewName;
   PTSTR lpt = gszFname;
   strcpy(lpf,pfile);
   gszDrive[0] = 0;
   gszDir[0] = 0;
   gszFname[0] = 0;
   gszExt[0] = 0;
   _splitpath( pfile, gszDrive, gszDir, lpt, gszExt );

   strcpy(lpf, gszDrive);
   strcat(lpf, gszDir);
   strcat(lpf, lpt);
   strcat(lpf, gszExt);
   while( CheckIfFileExists(lpf) )
   {
      if( !BumpFileName(lpt) )
         break;
      strcpy(lpf, gszDrive);
      strcat(lpf, gszDir);
      strcat(lpf, lpt);
      strcat(lpf, gszExt);
   }
   return lpf;
}

BOOL Write2File( HWND hWnd, HBITMAP bitmap, PTSTR pfile )
{
   HANDLE hDIB;
   BITMAP bmp;
   BOOL  bret = FALSE;
   if( GetObject( bitmap, sizeof(BITMAP), &bmp ) )
   {
      sprtf( "IDM_RENDERNEW: bitmap %d x %d x %d ... \n", bmp.bmWidth, bmp.bmHeight, bmp.bmBitsPixel );
      hDIB = WinDibFromBitmap( bitmap,
         BI_RGB,     // DWORD dwStyle,
         0,          // WORD wBits, 
         NULL );     // HPALETTE hPal )
      if(hDIB) {
         PTSTR pnew = GetNextFileName(pfile);
         if( WriteBMPFile2( pnew, hDIB, FALSE ) ) {
            PMWL pmwl = CommonFileOpen( hWnd, pnew, df_RENDERN );
				if(pmwl) {
               pmwl->wl_dwFlag |= flg_IsLoaded;
               bret = TRUE;
            }
         }
         DVGlobalFree( hDIB );
      }
   }
   return bret;
}

BOOL Got_Magnify( PMAGNIF pm )
{
   BOOL  bret = FALSE;
   if( pm->hBitmap1 )
      bret = TRUE;
   return bret;
}

BOOL Got_Swish( PCLIPSWISH pcs )
{
   BOOL  bret = FALSE;
   if( pcs->hBitmap )
      bret = TRUE;
   return bret;
}

BOOL Got_Swish_or_Magnify( PCLIPSWISH pcs, PMAGNIF pm )
{
   BOOL  bret = FALSE;
   if( Got_Swish( pcs ) || Got_Magnify( pm ) )
      bret = TRUE;
   return bret;
}


// ========================================
// Render current child image to a BITMAP
// BOOL ChildImage2BM( HWND hWndFrame )
BOOL Do_IDM_RENDERNEW( HWND hWndFrame )
{
	HANDLE		hDIBInfo;
	LPDIBINFO	lpDIBInfo;
   HDC         hdc;
   POINT       pt;
   RECT        rcClient;
   HDC         hdcBmp = NULL;
   HBITMAP     bitmap = NULL;
   HGDIOBJ     hold = NULL;
   PTSTR       pfile = "TEMPB000.BMP";
   HWND        hWnd = GetCurrentMDIWnd();

   if( !hWnd )
      return FALSE;

	GetClientRect (hWnd, &rcClient);

   if( !Get_DIB_Info( hWnd, &hDIBInfo, &lpDIBInfo ) )
      return FALSE;

   hdc = GetDC( hWnd );
   if( !hdc ) {
      Release_DIB_Info( hWnd, &hDIBInfo, &lpDIBInfo );
      return FALSE;
   }

   if( lpDIBInfo->Options.bStretch2W )
	{
		pt.x = rcClient.right;
		pt.y = rcClient.bottom;
      //sprtf( "IDM_RENDERNEW: Using client %d x %d ...(%x)\n", pt.x, pt.y, hWnd );
	}
	else
	{
		pt.x = lpDIBInfo->di_dwDIBWidth;
		pt.y = lpDIBInfo->di_dwDIBHeight;
      //sprtf( "IDM_RENDERNEW: Using DIB %d x %d ...(%x)\n", pt.x, pt.y, hWnd );
	}

   // we have the SIZE of the DIB
   hdcBmp = CreateCompatibleDC(hdc);
   if( !hdcBmp )
      goto Cleanup;
	bitmap  = CreateCompatibleBitmap( hdc, pt.x, pt.y );
   if( !bitmap )
      goto Cleanup;

   hold = SelectObject(hdcBmp,bitmap);

   DrawSelect( hdc, &lpDIBInfo->rcClip ); // remove outline
   BitBlt( hdcBmp, 0, 0, pt.x, pt.y, hdc, 0, 0, SRCCOPY );
   bitmap = (HBITMAP)SelectObject(hdcBmp,hold);
   DrawSelect( hdc, &lpDIBInfo->rcClip ); // put outline back

   Write2File( hWnd, bitmap, pfile );

Cleanup:

   if(hdcBmp)
      DeleteDC(hdcBmp);

   if(bitmap)
      DeleteObject(bitmap);

   ReleaseDC( hWnd, hdc );

   Release_DIB_Info( hWnd, &hDIBInfo, &lpDIBInfo );

   return TRUE;

}

// =========================================

VOID Do_IDM_PAINTSWISH( HWND hWndFrame )
{
   Set_Swish_ON();
   Do_Swish_Toggle( hWndFrame, FALSE );
}
VOID Do_IDM_PAINTOVAL( HWND hWndFrame )
{
   dbg_oval("DEBUG: Setting OVAL on ...\n");
   Set_Oval_ON();
   Do_Swish_Toggle( hWndFrame, FALSE );
}
VOID Do_IDM_PAINTSQUARE( HWND hWndFrame )
{
   Set_Square_ON();
   Do_Swish_Toggle( hWndFrame, FALSE );
}

// eof - DvMagnify.c
