// DvClipDlg.c

#include "dv.h"
extern PRECT Get_Clip_Rect( UINT cmd );

VOID Do_IDM_CLIPLISTMAX( HWND hWndFrame )
{


}

VOID Do_IDM_CLIPLIST( HWND hWndFrame, UINT cmd )
{
   HWND hWnd = GetCurrentMDIWnd();
   PRECT prc = Get_Clip_Rect( cmd - IDM_CLIPLIST );
   HDC   hdc;
   if(hWnd && prc)
   {
      HANDLE h;
      LPDIBINFO pdi;
      PRECT prcc;
      RECT  rc;
      if( Get_DIB_Info( hWnd, &h, &pdi ) )
      {
         prcc = &pdi->rcClip;
         rc = *prcc;
         hdc = GetDC(hWnd);
         if(hdc)
         {
            BOOL gs = Got_Swish( &pdi->sSwish );
            DC_ClearClip( hdc, &pdi->rcClip, pdi );
            *prcc = *prc;
            DrawSelect( hdc, prcc );
            if(gs)
               Draw_CLIP( hdc, prcc, pdi );
            ReleaseDC( hWnd, hdc );
         }
         Release_DIB_Info( hWnd, &h, &pdi );
      }
   }
}

// eof - DvClipDlg.c
