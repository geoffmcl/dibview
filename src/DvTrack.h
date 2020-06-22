// DvTrack.h
#ifndef _DvTrack_h_
#define _DvTrack_h_

// SWAP bits
#define  SWAP_LR     1
#define  SWAP_TB     2

typedef enum {
   DV_ON_NONE = 0,
   DV_ON_LEFT,
   DV_ON_RIGHT,
   DV_ON_TOP,
   DV_ON_BOTTOM,
   DV_ON_TOPLEFT,
   DV_ON_TOPRIGHT,
   DV_ON_BOTTLEFT,
   DV_ON_BOTTRIGHT,
   DV_ON_ALL
}DV_CURPOS;

extern BOOL  InitCursors ( VOID );
extern VOID  SetCursorType( DV_CURPOS cp );

extern HCURSOR  hCurWE, hCurNS, hCurNESW;  //   / for TOPRIGHT, and BOTTLEFT
extern HCURSOR  hCurNWSE;  //   \ for TOPLEFT, and BOTTRIGHT
extern HCURSOR  hCurALL, hCurArrow;

extern DV_CURPOS Clippos;

#endif //_DvTrack_h_
// eof - DvTrack.h

