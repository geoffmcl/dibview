

// DvGlaze.c
#include "Dv.h"   // all inclusinve include
//#include  "DvGlaze.h"

// ======= fun painting a splashy window ====== GLAZING I call it
//#define   COLOUR_MINI       0
//#define COLOR_SCALE_RED     0
//#define COLOR_SCALE_GREEN   1
//#define COLOR_SCALE_BLUE    2
//#define COLOR_SCALE_GRAY    3
//#define  COLOUR_MAXI       3

// global DWORD/INT giCurColor = COLOUR_MINI;
VOID  GlazeWindow1( HDC hdc, PRECT lpr );
VOID  PntRect1( HDC hDC, PRECT lpRect, int nColor );

COLORREF _GetNxtColr( int index, int iMax, int nColor )
{
   COLORREF cr = 0;
   switch(nColor)
   {
   case COLOR_SCALE_RED:
     cr = RGB(index,0,0);
      break;
   case COLOR_SCALE_GREEN:
      cr = RGB(0,index,0);
      break;
   case COLOR_SCALE_BLUE:
      cr = RGB(0,0,index);
      break;
   case COLOR_SCALE_GRAY:
      cr = RGB(index,index,index);
      break;
   }
   return cr;
}

//MM_ANISOTROPIC   Logical units are converted to arbitrary units
//with arbitrarily scaled axes. Setting the mapping mode to
//MM_ANISOTROPIC does not change the current window or viewport
//settings. To change the units, orientation, and scaling, call
//the SetWindowExt and SetViewportExt member functions.

//VOID PntRect1( HDC hDC, LPRECT lpRect, int idx, int nColor, int i )
VOID PntRect( HDC hDC, LPRECT lpRect, int nColor )
{
	RECT     rc, rect;
	HBRUSH   hBrush;
   int      idx; 
   int      nMapMode;
   SIZE	   szw,szv;
   POINT	   px;

	rc = *lpRect;
//	rc.right  -= rc.left;
//	rc.bottom -= rc.top;
   nMapMode =	SetMapMode(	hDC,	MM_ANISOTROPIC );
   // sets the horizontal and vertical extents of the window 
   SetWindowExtEx(	hDC,	512,			512,			         &szw );
   // function sets the horizontal and vertical extents of the viewport 
   SetViewportExtEx(	hDC,	rc.right,	-rc.bottom + rc.top,	&szv );
   SetViewportOrgEx(	hDC,	rc.left,     rc.bottom,	         &px  );

	for( idx = 0; idx < 256; idx++ )
	{
  		SetRect( &rect, idx, idx, 512 - idx, 512 - idx );
		hBrush = CreateSolidBrush( _GetNxtColr( idx, 256, nColor ) ); //RGB(0,0,idx));
		if(hBrush) 
		{
			FillRect( hDC, &rect, hBrush );
			DeleteObject(hBrush);
		}
	}

   SetViewportOrgEx(	hDC,	px.x,				 px.y,	NULL );
   SetViewportExtEx(	hDC,	szv.cx,	      szv.cy,	NULL );
   SetWindowExtEx(	hDC,	szw.cx,			szw.cy,	NULL );
   SetMapMode( hDC, nMapMode );

}

VOID  GlazeWindow1( HDC hdc, LPRECT lpr )
{
   if( hdc )
   {
       PntRect( hdc, lpr, giCurColor );
       giCurColor++;    // bump to next colour
       if( giCurColor > COLOUR_MAXI )
           giCurColor = COLOUR_MINI;  // wrap back to RED
   }
}

#if   0  // previous code
//#define COLOR_SCALE_RED     1
//#define COLOR_SCALE_GREEN   2
//#define COLOR_SCALE_BLUE    3
//#define COLOR_SCALE_GRAY    4

COLORREF	_GetNxtColr( int index, int iMax, int nColor )
{
	COLORREF	cr = 0;
	switch(nColor)
	{
	case COLOR_SCALE_RED:
		cr = RGB(index,0,0);
		break;
	case COLOR_SCALE_GREEN:
		cr = RGB(0,index,0);
		break;
	case COLOR_SCALE_BLUE:
		cr = RGB(0,0,index);
		break;
	case COLOR_SCALE_GRAY:
		cr = RGB(index,index,index);
		break;
	}
	return cr;
}

//MM_ANISOTROPIC   Logical units are converted to arbitrary units
//with arbitrarily scaled axes. Setting the mapping mode to
//MM_ANISOTROPIC does not change the current window or viewport
//settings. To change the units, orientation, and scaling, call
//the SetWindowExt and SetViewportExt member functions. 

VOID PntRect( HDC hDC, LPRECT lpRect, int nColor )
{
	int         nMapMode,idx;
	RECT        rc, rect;
	HBRUSH      hBrush;
	SIZE			szw,szv;
	POINT			px;

	rc = *lpRect;
//	rc.right  -= rc.left;
//	rc.bottom -= rc.top;
   // MM_ANISOTROPIC   Logical units are converted to arbitrary units
   // with arbitrarily scaled axes.
	nMapMode =	SetMapMode(	hDC,	MM_ANISOTROPIC );

	//        set the horizontal and vertical extents of the window 
	SetWindowExtEx(	hDC,	512,			512,		  &szw );

	// function sets the horizontal and vertical extents of the viewport 
	SetViewportExtEx(	hDC,	rc.right,	-rc.bottom,	&szv );
	SetViewportOrgEx(	hDC,	rc.left,     rc.bottom, &px  );

	for( idx = 0; idx < 256; idx++ )
	{
		SetRect( &rect,
         idx, idx,
         512 - idx, 512 - idx );

		if( hBrush = CreateSolidBrush( _GetNxtColr( idx, 256, nColor ) ) ) //RGB(0,0,idx));
		{
			FillRect( hDC, &rect, hBrush );
			DeleteObject(hBrush);
		}
	}

   SetViewportOrgEx(	hDC,	px.x,				 px.y,	NULL );
   SetViewportExtEx(	hDC,	szv.cx,	      szv.cy,	NULL );
   SetWindowExtEx(	hDC,	szw.cx,			szw.cy,	NULL );
	SetMapMode( hDC, nMapMode );

}

VOID PntRect1( HDC hDC, LPRECT lpRect, int idx, int nColor, int i )
{
	int         nMapMode;
	RECT        rc, rect;
	HBRUSH      hBrush;
	static SIZE			szw,szv;
	static POINT			px;

	rc = *lpRect;
//	rc.right  -= rc.left;
//	rc.bottom -= rc.top;
   if( i == 0 )
   {
	   nMapMode =	SetMapMode(	hDC,	MM_ANISOTROPIC );
	   // sets the horizontal and vertical extents of the window 
	   SetWindowExtEx(	hDC,	512,			512,			         &szw );

	   // function sets the horizontal and vertical extents of the viewport 
	   SetViewportExtEx(	hDC,	rc.right,	-rc.bottom + rc.top,	&szv );
	   SetViewportOrgEx(	hDC,	rc.left,     rc.bottom,	         &px  );
   }
	//for( idx = 0; idx < 256; idx++ )
	{
		SetRect( &rect,
         idx, idx,
         512 - idx, 512 - idx );

		if( hBrush = CreateSolidBrush( _GetNxtColr( idx, 256, nColor ) ) ) //RGB(0,0,idx));
		{
			FillRect( hDC, &rect, hBrush );
			DeleteObject(hBrush);
		}
	}

   if( i == 9 )
   {
      SetViewportOrgEx(	hDC,	px.x,				 px.y,	NULL );
      SetViewportExtEx(	hDC,	szv.cx,	      szv.cy,	NULL );
      SetWindowExtEx(	hDC,	szw.cx,			szw.cy,	NULL );

	   SetMapMode( hDC, nMapMode );
   }
}

#endif   // 0

// eof - DvGlaze.c
