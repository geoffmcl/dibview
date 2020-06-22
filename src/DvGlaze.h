
// DvGlaze.h
#ifndef  _DvGlaze_h
#define  _DvGlaze_h

// ======= fun painting a splashy window ====== GLAZING I call it
#define   COLOUR_MINI       0
#define COLOR_SCALE_RED     COLOUR_MINI
#define COLOR_SCALE_GREEN   1
#define COLOR_SCALE_BLUE    2
#define COLOR_SCALE_GRAY    3
#define  COLOUR_MAXI        COLOR_SCALE_GRAY

// g_ giCurColor; // = COLOUR_MINI;
extern   VOID  GlazeWindow1( HDC hdc, PRECT lpr );
extern   VOID  PntRect1( HDC hDC, PRECT lpRect, int nColor );

#endif   // _DvGlaze_h
// eof - DvGlaze.h
