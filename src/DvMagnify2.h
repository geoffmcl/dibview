// DvMagnify2.h
#ifndef _DvMangify2_h_
#define _DvMangify2_h_

extern void MouseMagnify( HWND hWnd, LPARAM lParam, LPDIBINFO lpDIBInfo );
extern int   gi_MouseMagnify;
extern void Magnify2Off( HWND hWnd, PMAGNIF pm );
extern BOOL IsMagnifyOn( PMAGNIF pm );
extern void RePaintMagnify( HDC hdc, LPDIBINFO lpDIBInfo );
extern VOID  DC_ClearClip( HDC hdc, PRECT prc, LPDIBINFO lpDIBInfo );
extern PTSTR GetSwishMenu( VOID );
extern PTSTR Menu_Swish(VOID);   // { return "Paint Swish";  }
extern PTSTR Menu_Oval(VOID);    // { return "Paint Oval";   }
extern PTSTR Menu_Square(VOID);  // { return "Paint Square"; }
extern BOOL gbSwish, gbOval;
extern void Draw_CLIP( HDC hdc, PRECT pclip, LPDIBINFO lpDIBInfo );
extern BOOL Got_Swish_or_Magnify( PCLIPSWISH pcs, PMAGNIF pm );
extern BOOL Got_Magnify( PMAGNIF pm );
extern BOOL Got_Swish( PCLIPSWISH pcs );


#endif   // _DvMangify2_h_
// eof - DvMagnify2.h

