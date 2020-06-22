// DvMagnify.h
#ifndef _DvMagnify_h_
#define _DvMagnify_h_

typedef struct tagMAGNIF {
   POINT pt;            // mouse POINT
   SIZE  szin, szout;   // INNER and OUTER sizes
   COLORREF trans;      // transparent color
   HBITMAP hBitmap1;    // saved BITMAP - for RESTORE
   HBITMAP hBitmap2;    // masked BITMAP - for REPAINT
}MAGNIF, * PMAGNIF;

typedef struct tagCLIPSWISH {
   RECT     rc;      // base rectangle
   int   penwidth;   // width of swish PEN
   COLORREF   cr;    // color of swish
   RECT     src1, src2;     // swish rectangle
   POINT    ptB1, ptE1; // first half of swish
   POINT    ptB2, ptE2; // second half of swish
   HBITMAP  hBitmap;    // saved BITMAP - for RESTORE
} CLIPSWISH, * PCLIPSWISH;

#endif   // _DvMagnify_h_
// eof - DvMagnify.h


