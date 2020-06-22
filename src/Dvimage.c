

// DvImage.c

#include "dv.h"
//#include	"DvInfo.h"	// And information BLOCK (about object)
#include	"DvPal2.h"	/* some PAL specials */
extern	char *	Float2Str( float fNum );
extern	BOOL CenterWindow(HWND hwndChild, HWND hwndParent);   // in Utils\gmUtilc


// local
void     DVPaintClip2( HDC hDC, PRECT prcClip, PRECT prcSize, PRECT prcFrame );

#undef   TRYVIRTMEM     // does NOT seems to be FASTER in any way!!!
#undef   SHWDIAGCC
#define  TRYONEMEM      // only ONE allocation of the BIG BUFFER at a time

static   BOOL  bBigBuf = FALSE; // another idea - ONLY ONE BIG BUFFER AT A TIME

// extract from DVChild.h
///* kept in lpDIBInfo->di_hCOLR */
///* =========================== */
//typedef struct tagCOLR {
//	COLORREF	cr_Color;
//	DWORD		cr_Freq;
//}COLR, * PCR;
//typedef COLR MLPTR LPCOLR;
//typedef struct tagPALEX {
//	/* ======================================================= */
//	DWORD	px_Size;		/* SIZE of this instance of struct */
//	DWORD	px_Count;		/* COUNT of color listed in array  */
//	COLR	px_Colrs[1];	/* array of COLOR and FREQUENCY    */
//	/* ======================================================= */
//}PALEX, * PPX;
//typedef PALEX MLPTR LPPALEX;
// NOTE: The separation of the COLOR and the FREQUENCY has been done - no RANK

#define		DLPPALEX	   LPPALEX
#define		DLPCOLR		LPCOLR

#define		PALEXSIZE(a)	( sizeof(PALEX) + ( a * sizeof(COLR) ) )
// For other than 24-bit DIBs contruct a work block as follows
// BYTE PAD for COLORS found = max.colors time sizeof(BYTE)
// |---------------------------|
// |                           |
// |                           |
// |---------------------------|
// A PALEX structure
// DWORD	px_Size;		  -- SIZE of this instance of struct
//	DWORD	px_Count;	  -- COUNT of color listed in array
// Then an array of COLORREF and DWORD frequency
//	COLR	px_Colrs[??]; -- array of COLOR and FREQUENCY
// ============================================================
// And the FINAL structure is ONLY the PALEX structure where the
// array of COLR(COLOR and FREQ) is equal to the actual number
// of colors used in the DIB
// =============================================================

#ifdef	Dv16_App
// Appears these are NOT part of 3.1!!!
#define	BST_CHECKED			1
#define	BST_UNCHECKED		0
#define	CC_ANYCOLOR			0
#endif	// Dv16_App

extern   void  DrawSelect( HDC, LPRECT );
extern	DWORD	CalcDIBColors( LPSTR lpbi );
extern	void	SRErrorN( UINT rID, LPSTR lpTit );
extern	DWORD	OffsetToColor( LPSTR lpbi );

void	   OutColours( LPRGBQUAD lpq, int iCnt );


// Called from IDM_IMAGEATT menu item (uses IDD_IMAGEATT template)
BOOL CALLBACK INFO2DLGPROC( HWND, UINT, WPARAM, LPARAM );

// In DvInfo.h - are
//#define		fc_1		0x00000001	// Changed WIDTH
//#define		fc_2		0x00000002	// Changed HEIGHT
//#define		fc_3		0x00000004	// Changed bm WIDTH
//#define		fc_4		0x00000008	// Changed bm HEIGHT
//#define		fc_5		0x00000010	// Changed Clip WIDTH
//#define		fc_6		0x00000020	// Changed Clip HEIGHT
// CHANGED CLIP RECTANGLE
//#define		fc_7		0x00000040	// Changed Clip left
//#define		fc_8		0x00000080	// Changed Clip top
//#define		fc_9		0x00000100	// Changed Clip right
//#define		fc_10		0x00000200	// Changed Clip bottom
//#define		fc_11		0x00000400	// Changed bm Colors
//#define		fc_12		0x00000800	// Changed bm Colors
#define		fc_ChgClip	(fc_5|fc_6|fc_7|fc_8|fc_9|fc_10)

#ifdef	ADDCOLRF

//	case WM_ERASEBKGND:
extern	DIBX	gDIBx;
extern	BOOL	fUdtgDIBx;
extern	BOOL	fChgUDIBx;
extern	LPSTR	GetFNBuf( void );

#endif	// ADDCOLRF

// LPISTR	lpIStr;
int		giUseSRE = 1;
#define	uc		unsigned char
#define	puc		uc *
HANDLE	ghCOLR = 0;
int		gfBmpOrder = 1;		/* use from bottom to top */

//typedef struct tagCLRSET {
//	COLORREF	cs_Colr;
//	LPSTR		cs_RGB;
//	LPSTR		cs_Name;
//}CLRSET, * PCS;
CLRSET	crClrSet[] = {
	{ CR_RED,		"RGB(255,  0,  0)", "Red"    },
	{ CR_ORANGE,	"RGB(255,128,  0)", "Orange" },
	{ CR_YELLOW,	"RGB(255,255,  0)", "Yellow" },
	{ CR_YEL2GR,	"RGB(128,255,  0)", 0		  },
	{ CR_GREEN,		"RGB(  0,255,  0)", "Green"  },
	{ CR_GR2BL,		"RGB(  0,255,128)", 0		  },
	{ CR_GR2BL2,	"RGB(  0,255,255)", 0		  },
	{ CR_GR2BL3,	"RGB(  0,128,255)", 0		  },
	{ CR_BLUE,		"RGB(  0,  0,255)", "Blue"   },
	{ CR_BL2RD,		"RBG(128,  0,255)", 0		  },
	{ CR_BL2RD2,	"RGB(255,  0,255)", 0        },
	{ CR_BL2RD3,	"RGB(255,  0,128)", 0        },
	{ 0,            0,                  0       }
};

COLORREF crRange[24] = {
   { RGB(192,  0,  0) }, { RGB(255, 63, 63) },  // red
   { RGB(192, 64,  0) }, { RGB(255,191, 63) },  // orange
   { RGB(193,193,  0) }, { RGB(255,255, 63) },
   { RGB( 64,193,  0) }, { RGB(191,255, 63) },
   { 0 }
};

void	chkuc( LPSTR lpd )
{
	sprtf("CHECK ERROR: %s"MEOR,
		lpd );
}

#define		pP			lpPalInfo

typedef	struct	{
	COLORREF	ci_Colr;
	double		ci_Lumina;
}CLRI;
typedef CLRI * LPCLRI;

/* kept in lpDIBInfo->di_hCOLR */
/* =========================== */
//typedef struct tagCOLR {
//	COLORREF	cr_Color;
//	DWORD		cr_Freq;
//}COLR, * PCR;
//typedef COLR MLPTR LPCOLR;

//typedef struct tagPALEX {
//	DWORD	ps_Size;
//	DWORD	px_Count;
//	COLR	px_Colrs[1];
//}PALEX, * PPX;
//typedef PALEX MLPTR LPPALEX;

// purpose - sort into a sort of rainbow order
// basically first into 12 ranges, then try to
// order within the 12 ranges
//Describing Color
//Color can be described in terms of components. The components
//most often used by imaging professionals are hue, saturation,
//and brightness. A hue is what we normally think of as a color. A
//color's hue fixes its place in the visible spectrum of light.
//The saturation of a color is how "pure" or "strong" a color is.
//Neutral gray is said to have zero saturation. The brightness of
//a color refers to the intensity of light that is reflected or
//transmitted by an image. 
//The terms tint, tone, and shade are also well-used in color
//imaging literature. A tint of a color is obtained by mixing its
//hue with white. A tone of a color is created by mixing a hue
//with gray. A shade of a color is made by adding black to its
//hue.
//        Green   Yellow
//   Cyan     White    Red
//        Blue    Magenta
//
//             Black
VOID  SetRainRange( VOID )
{
   DWORD dwi;
   DWORD dwc;
   BYTE  r1,r2,g1,g2,b1,b2;
   LPCOLORREF  pclr;
   PCS   pcs = &crClrSet[0];
      for( dwi = 0; dwi < 12; dwi++ )
      {
         pclr = &crRange[ (dwi * 2) ];
         dwc = GetRValue( pcs->cs_Colr );
         if( dwc >= 192 )
         {
            r1 = 192;
            r2 = 255;
         }
         else if( dwc >= 128 )
         {
            r1 = 64;
            r2 = 191;
         }
         else
         {
            r1 = 0;
            r2 = 63;
         }
         dwc = GetGValue( pcs->cs_Colr );
         if( dwc >= 192 )
         {
            g1 = 192;
            g2 = 255;
         }
         else if( dwc >= 128 )
         {
            g1 = 64;
            g2 = 191;
         }
         else
         {
            g1 = 0;
            g2 = 63;
         }
         dwc = GetBValue( pcs->cs_Colr );
         if( dwc >= 192 )
         {
            b1 = 192;
            b2 = 255;
         }
         else if( dwc >= 128 )
         {
            b1 = 64;
            b2 = 191;
         }
         else
         {
            b1 = 0;
            b2 = 63;
         }
         pclr[0] = RGB(r1,g1,b1);
         pclr[1] = RGB(r2,g2,b2);
         pcs++;
      }

}

// Try to put all the Reds together, then Blue, then Greens
// ========================================================
#define  MYD         8
#define  MGetR(a)    GetRValue(a)
#define  MGetG(a)    GetGValue(a)
#define  MGetB(a)    GetBValue(a)
typedef struct tagPALEXS {
   COLORREF ps_crColor;
   DWORD    ps_dwFreq;
   int      ps_iIntensity;
   int      ps_iRn;
   int      ps_iGn;
   int      ps_iBn;
   DWORD    ps_dwType;
   DWORD    ps_dwRank;
}PALEXS, * PPXS;

VOID  DoRainbowSort( PPX ps, PPX pc, DWORD dwcnt )
{
   HGLOBAL  hg = 0;
   PPXS     px = 0;
   DWORD    dwi;
   PCR      pcr1;
   PPXS     px1, px2, pxe;
   DWORD    r1,g1,b1;
   LPCOLORREF  pc1;
   int      Rn, Gn, Bn;
   int      dwint;
   DWORD    dwk;
   DWORD    dws, dwc;
   DWORD    dwc2;
   DWORD    dwt;
   BOOL     swap;

   dws = dwcnt * sizeof(PALEXS) * 2;
   if( ( dwcnt ) &&
       ( hg = DVGAlloc( "sPALEXS", GHND, (dws + sizeof(PALEXS)) ) ) &&
       ( px = (PPXS)DVGlobalLock( hg )         ) )
   {
      ZeroMemory( px, dws );
      pxe  = px + (dwcnt * 2);   // get an END work block

      px1  = px;  // get start
      pcr1 = &ps->px_Colrs[0]; // get pointer into array
      for( dwi = 0; dwi < dwcnt; dwi++ )
      {
         pc1 = &pcr1->cr_Color;
         px1->ps_crColor = pcr1->cr_Color;
         px1->ps_dwFreq  = pcr1->cr_Freq;
         r1 = GetRValue( *pc1 );    // extract colors
         g1 = GetGValue( *pc1 );    // extract colors
         b1 = GetBValue( *pc1 );    // extract colors
         px1->ps_iIntensity = dwint = (r1 + g1 + b1);
         px1->ps_iRn = Rn = ( r1 * 2 ) - g1 - b1;
         px1->ps_iGn = Gn = ( g1 * 2 ) - r1 - b1;
         px1->ps_iBn = Bn = ( b1 * 2 ) - g1 - r1;

         if( ( Rn > Gn ) && ( Rn > Bn ) )
            px1->ps_dwType = 1;  // RED type
         else if( ( Gn > Rn ) && ( Gn > Bn ) )
            px1->ps_dwType = 2;  // GREEN type
         else if( ( Bn > Rn ) && ( Bn > Gn ) )
            px1->ps_dwType = 3;  // BLUE type
         else
            px1->ps_dwType = 4;  // GRAY (even) type

         // bump both structures
         pcr1++;
         px1++;
      }

      // we have it all layed out - now to set the ORDER or RANK
      dwc2 = 0;
      for( dwt = 1; dwt < 5; dwt++ )
      {
         px1  = px;  // start at base
         px2  = px + ( dwcnt + dwc2 );
         dwc  = 0;
         for( dwi = 0; dwi < dwcnt; dwi++ )
         {
            if( px1->ps_dwType == dwt )  // RED/GREEN/BLUE/GRAY type
            {
               memcpy( px2, px1, sizeof(PALEXS) ); // copy it
               px2++;   // bump to next structure block
               dwc++;   // count another RED/GREEN/BLUE/GRAY
            }
            px1++;
         }
         if( dwc )
         {
            dwk = dwc - 1; // get less ONE
            dws = 1;
            while(dws)
            {
               dws = 0;    // start with NO SWAPS
               px2 = px + ( dwcnt + dwc2 );  // beginning of THIS set
               for( dwi = 0; dwi < dwk; dwi++ ) // for count minus one
               {
                  px1 = px2 + 1; // get next
                  // if next intensity GT current intensity
                  //if( px1->ps_dwIntensity > px2->ps_dwIntensity )
                  swap = FALSE;
                  switch( dwt )
                  {

                  case 1:  // REDNESS
                     if( px1->ps_iRn > px2->ps_iRn )
                        swap = TRUE;
                     break;

                  case 2:  // GREENNESS
                     if( px1->ps_iGn > px2->ps_iGn )
                        swap = TRUE;
                     break;

                  case 3:  // BLUENESS
                     if( px1->ps_iBn > px2->ps_iBn )
                        swap = TRUE;
                     break;

                  case 4:  // GRAYNESS
                     if( px1->ps_iIntensity > px2->ps_iIntensity )
                        swap = TRUE;
                     break;

                  }

                  if( swap )
                  {
                     // swap the positions
                     memcpy( pxe, px1, sizeof(PALEXS) );
                     memcpy( px1, px2, sizeof(PALEXS) );
                     memcpy( px2, pxe, sizeof(PALEXS) );
                     dws++;   // around again
                  }

                  px2++;   // bump to NEXT structure

               }
            }
         }

         dwc2 += dwc;   // update the cumulative counter

      }  // for red-type green-type blue-type and gray-type by intensity

      if( dwc2 != dwcnt )
         chkme( "WAIT! We miss somebody in the swapping!!!" );

      px1  = px + dwcnt;  // get start
      pcr1 = &ps->px_Colrs[0]; // get pointer into array
      for( dwi = 0; dwi < dwcnt; dwi++ )
      {
         pcr1->cr_Color = px1->ps_crColor;
         pcr1->cr_Freq  = px1->ps_dwFreq;
         // bump both structures
         pcr1++;
         px1++;
      }

   }

   if( hg && px )
      DVGlobalUnlock(hg);
   if( hg )
      DVGlobalFree(hg);
}
VOID  DoRainbowSort3( PPX ps, PPX pc, DWORD dwcnt )
{
   DWORD       r1,r2;
   DWORD       g1,g2;
   DWORD       b1,b2;
   PCR         pcr1, pcr2;
   LPCOLORREF  pc1;
   //LPCOLORREF  pc2;
   DWORD       dwi, dwc, dws;
   DWORD       dwr, dwf, dwk;

   if( dwcnt == 0 )
      return;

   dwc = dwcnt;
   pcr1 = &ps->px_Colrs[0]; // get pointer into array
   for( dwi = 0; dwi < dwc; dwi++ )
   {
      pcr1->cr_Freq = 0;
      pcr1++;
   }

   dwk = 0;

   dwc = dwcnt;
   // 1. get all the EQUAL colors
   r2 = g2 = b2 = 255;
   dws = 256;
   while( dws )
   {
      pcr1 = &ps->px_Colrs[0]; // get pointer into array
      for( dwi = 0; dwi < dwc; dwi++ )
      {
         if( pcr1->cr_Freq == 0 )
         {
            pc1 = &pcr1->cr_Color;     // get pointers to COLORREF
            r1 = GetRValue( *pc1 );    // extract colors
            g1 = GetGValue( *pc1 );    // extract colors
            b1 = GetBValue( *pc1 );    // extract colors
            if( ( r1 == r2 ) &&
                ( g1 == g2 ) &&
                ( b1 == b2 ) )
            {
               dwk++;
               pcr1->cr_Freq = dwk;
               break;
            }
         }
         pcr1++;
      }
      r2--;
      g2--;
      b2--;
      dws--;
   }

   r2 = 255;
   g2 = 0;
   b2 = 0;
   dwc = dwcnt;
   dws = 0;
   while( dwk < dwcnt )
   {
      pcr1 = &ps->px_Colrs[0]; // get pointer into array
      for( dwi = 0; dwi < dwc; dwi++ )
      {
         if( pcr1->cr_Freq == 0 )
         {
            pc1 = &pcr1->cr_Color;     // get pointers to COLORREF
            r1 = GetRValue( *pc1 );    // extract colors
            g1 = GetGValue( *pc1 );    // extract colors
            b1 = GetBValue( *pc1 );    // extract colors
            if( ( r1 == r2 ) &&
                ( g1 == g2 ) &&
                ( b1 == b2 ) )
            {
               dwk++;
               pcr1->cr_Freq = dwk;
               break;
            }
         }
         pcr1++;
      }
      switch(dws)
      {
      case 0:
         // 1. RED with GREEN increasing - BLUE = 0
         g2++;
         if( g2 > 255 )
         {
            g2 = 255;
            r2 = 254;
            b2 = 0;
            dws++;
         }
         break;
      case 1:
         // 2. GREEN with RED decreasing - BLUE = 0
         if( r2 == 0 )
         {
            g2 = 255;
            b2 = 1;  // start BLUE
            dws++;
         }
         else
         {
            r2--;
         }
         break;
      case 2:
         // 3. GREEN with BLUE increasing - RED = 0
         b2++;
         if( b2 > 255 )
         {
            b2 = 255;
            g2 = 254;   // reduced GREEN
            r2 = 0;
            dws++;
         }
         break;

      case 3:
         // 4. BLUE with GREEN decreasing - RED = 0
         if( g2 == 0 )
         {
            r2 = 1;  // start RED
            dws++;
         }
         else
         {
            g2--; // decrease GREEN
         }
         break;

      case 4:
         // 5. BLUE with RED increasing - GREEN = 0
         r2++;
         if( r2 > 255 )
         {
            r2 = 255;
            b2 = 254;   // reduced BLUE
            dws++;
         }
         break;

      case 5:
         // 6. RED with BLUE decreasing - GREEN = 0
         if( b2 == 0 )
         {
            dws++;
            if( dwk < dwcnt )
            {
               pcr1 = &ps->px_Colrs[0]; // get pointer into array
               for( dwi = 0; dwi < dwc; dwi++ )
               {
                  if( pcr1->cr_Freq == 0 )
                  {
                     dwk++;
                     pcr1->cr_Freq = dwk;
                  }
                  pcr1++;
               }
            }
         }
         else
         {
            b2--;
         }
         break;

      default:
         // 6. should NOT happen
         chkme( "HEY THERE: Should NEVER be here!!!" );
         break;
      }
   }


   dws = 1;
   dwc = dwcnt - 1;
   while(dws)
   {
      dws = 0;
      for( dwi = 0; dwi < dwc; dwi++ )
      {
         pcr1 = &ps->px_Colrs[dwi]; // get pointer into array
         pcr2 = pcr1 + 1;           // and the NEXT
         dwf  = pcr1->cr_Freq;       // get the RANK
         if( dwf > pcr2->cr_Freq )
         {
            dwr = pcr1->cr_Color;
            //dwf = pcr1->cr_Freq;
            pcr1->cr_Color = pcr2->cr_Color;
            pcr1->cr_Freq  = pcr2->cr_Freq;
            pcr2->cr_Color = dwr;
            pcr2->cr_Freq  = dwf;
            dws++;
         }
      }
   }

   // now we have them sorted per their RANK
   // get back the correct FREQUENCY with its color
   for( dwi = 0; dwi < dwcnt; dwi++ )
   {
      pcr1  = &ps->px_Colrs[dwi];
      for( dwc = 0; dwc < dwcnt; dwc++ )
      {
         pcr2 = &pc->px_Colrs[dwc];
         if( pcr1->cr_Color == pcr2->cr_Color )
         {
            pcr1->cr_Freq = pcr2->cr_Freq; // return to current FREQ
            break;
         }
      }
      if( dwc == dwcnt )
         chkme( "WHOA: Color (%#x) NOT FOUND.", pcr1->cr_Color );
   }

}

VOID  DoRainbowSort2( PPX ps, PPX pc, DWORD dwcnt )
{
   static BOOL sbDnRng = FALSE;
   DWORD       r1,r2;
   DWORD       g1,g2;
   DWORD       b1,b2;
   PCR         pcr1, pcr2;
   LPCOLORREF  pc1, pc2;
   DWORD       dwi, dwc, dws;
   DWORD       dwr, dwf;
   if( !sbDnRng )
   {
      SetRainRange();
      sbDnRng = TRUE;
   }
   if(dwcnt)
      dwc = dwcnt - 1;
   else
      dwc = 0;

   dws = 1;
   while(dws)
   {
      dws = 0;
      for( dwi = 0; dwi < dwc; dwi++ )
      {
         pcr1 = &ps->px_Colrs[dwi]; // get pointer into array
         pcr2 = pcr1 + 1;           // and the NEXT
         pc1 = &pcr1->cr_Color;     // get pointers to COLORREF
         pc2 = &pcr2->cr_Color;

         r1 = MGetR( *pc1 );    // extract colors
         r2 = MGetR( *pc2 );
         if( r2 > r1 )
         {
            dwr = pcr1->cr_Color;
            dwf = pcr1->cr_Freq;
            pcr1->cr_Color = pcr2->cr_Color;
            pcr1->cr_Freq  = pcr2->cr_Freq;
            pcr2->cr_Color = dwr;
            pcr2->cr_Freq  = dwf;
            dws++;
         }  // else if( r1 == r2 )
         else if( r1 == r2 )
         {
            g1 = MGetG( *pc1 );    // extract colors
            g2 = MGetG( *pc2 );
            if( g2 > g1 )
            {
               dwr = pcr1->cr_Color;
               dwf = pcr1->cr_Freq;
               pcr1->cr_Color = pcr2->cr_Color;
               pcr1->cr_Freq  = pcr2->cr_Freq;
               pcr2->cr_Color = dwr;
               pcr2->cr_Freq  = dwf;
               dws++;
            }
            else if( g1 == g2 )
            {
               b1 = MGetB( *pc1 );    // extract colors
               b2 = MGetB( *pc2 );
               if( b2 > b1 )
               {
                  dwr = pcr1->cr_Color;
                  dwf = pcr1->cr_Freq;
                  pcr1->cr_Color = pcr2->cr_Color;
                  pcr1->cr_Freq  = pcr2->cr_Freq;
                  pcr2->cr_Color = dwr;
                  pcr2->cr_Freq  = dwf;
                  dws++;
               }
            }
         }
      }
   }
}

// just sort by the TOTAL RGB value
VOID  DoGraySort( PPX ps, PPX pc, DWORD dwcnt )
{
   PCR   pcr, pcr1;
   DWORD dwi;
   DWORD dwc;
   DWORD dws;
   DWORD dwf, dwr;
   BYTE  r1,g1,b1;
//   BYTE  r2,g2,b2;
   LPCOLORREF  pc1;

   pcr = &ps->px_Colrs[0];
   for( dwi = 0; dwi < dwcnt; dwi++ )
   {
      pcr = &ps->px_Colrs[dwi];  // get pointer into array
      pc1 = &pcr->cr_Color;      // get pointer to COLORREF
      r1 = GetRValue( *pc1 );    // extract colors
      g1 = GetGValue( *pc1 );
      b1 = GetBValue( *pc1 );
      dwc = ( r1 + g1 + b1 ); // total / 3;   // average of the three
      pcr->cr_Freq = dwc;
   }

   // use a bubble sort per TOTAL of the three colors
   dws = 1;
   dwc = dwcnt - 1;
   while(dws)
   {
      dws = 0;
      for( dwi = 0; dwi < dwc; dwi++ )
      {
         pcr  = &ps->px_Colrs[dwi];
         pcr1 = pcr + 1;
         if( pcr->cr_Freq > pcr1->cr_Freq )
         {
            dwr = pcr->cr_Color;
            dwf = pcr->cr_Freq;
            pcr->cr_Color = pcr1->cr_Color;
            pcr->cr_Freq  = pcr1->cr_Freq;
            pcr1->cr_Color = dwr;
            pcr1->cr_Freq  = dwf;
            dws++;
         }
      }
   }

   // now we have them sorted per their TOTAL
   // get back the correct FREQUENCY with its color
   for( dwi = 0; dwi < dwcnt; dwi++ )
   {
      pcr  = &ps->px_Colrs[dwi];
      for( dwc = 0; dwc < dwcnt; dwc++ )
      {
         pcr1 = &pc->px_Colrs[dwc];
         if( pcr->cr_Color == pcr1->cr_Color )
         {
            pcr->cr_Freq = pcr1->cr_Freq; // return to current FREQ
            break;
         }
      }
      if( dwc == dwcnt )
         chkme( "WHOA: Color (%#x) NOT FOUND.", pcr->cr_Color );
   }
}

VOID  DoFreqSort( PPX ps, PPX pc, DWORD dwcnt )
{
   PCR   pcr1, pcr2;
   DWORD dws, dwc, dwi;
   DWORD dwr, dwf;

   if( dwcnt )
   {
      dws = 1;
      dwc = dwcnt - 1;
      while(dws)
      {
         dws = 0;
         pcr1  = &ps->px_Colrs[0];
         for( dwi = 0; dwi < dwc; dwi++ )
         {
            pcr2 = pcr1 + 1;
            if( pcr2->cr_Freq > pcr1->cr_Freq )
            {
               // get values from one
               dwr = pcr1->cr_Color;
               dwf = pcr1->cr_Freq;
               // copy two to one
               pcr1->cr_Color = pcr2->cr_Color;
               pcr1->cr_Freq  = pcr2->cr_Freq;
               // set extracted values in two
               pcr2->cr_Color = dwr;
               pcr2->cr_Freq  = dwf;
               dws++;   // bump a switch
            }
            pcr1++;  // bump to the next structure
         }
      }  // while bubble sorting
   }
}

DWORD	CountRGB( DWORD Height, DWORD Wid, DWORD Width,
				 puc lpIn, LPDWORD lpdw, DWORD crmx )
{
	DWORD	i,j,k,m;
	uc		r,g,b;
	COLORREF	cr;
	LPSTR	lps = lpIn;

	k = 0;
	if( gfBmpOrder )
	{
		i = Height;
		while( i-- )		/* for EACH row, from bottom UP */
		{
			m = i * Width;	/* get offset to this ROW       */
			lps = &lpIn[m];	/* and get POINTER              */

				for( j = 0; j < Wid; j++ )
				{
					// For each COLUMN
					r = *lps++;
					g = *lps++;
					b = *lps++;
					cr = RGB(r,g,b);
					if( (DWORD)cr < (DWORD)crmx )
					{
						if( lpdw[cr] == 0 )
						{
							k++;	// Count a NEW color
						}
						lpdw[cr]++;
					}
					else
					{
						/* *** FAILED *** */
						chkuc( "Memory FAILED!!!" );
						DIBError( ERR_MEMORY );
						j = Wid;
						break;
					}
				}
//				m = j * 3;
//				while( m++ < Width )
//					lps++;
				m = Height - ( i + 1 );
				if( m )
				{
					// add percentage after each ROW
					//#define	ProgP(p)			SetProgP(p)
					ProgP( (int)(( m * 100 ) / Height ) );
				}
		}
	}
	else
	{
		/* ok, for each row, march across the columns */
		/* remember to roll past any padding at the end of a row */
		/* ===================================================== */
			for( i = 0; i < Height; i++ )
			{
				// For each ROW
				for( j = 0; j < Wid; j++ )
				{
					// For each COLUMN
					r = *lps++;
					g = *lps++;
					b = *lps++;
					cr = RGB(r,g,b);
					if( (DWORD)cr < (DWORD)crmx )
					{
						if( lpdw[cr] == 0 )
							k++;	// Count a NEW color
						lpdw[cr]++;
					}
					else
					{
						// FAILED
						chkuc( "Memory FAILED!!!" );
						DIBError( ERR_MEMORY );
						j = Wid;
						break;
					}
				}
				m = j * 3;
				while( m++ < Width )
					lps++;
				if( i )
				{
					// add percentage after each ROW
					//#define	ProgP(p)			SetProgP(p)
					ProgP( (int)(( i * 100 ) / Height ) );
				}
			}
	}
	return k;
}

DWORD  DVIsValidPalEx( PPX ppx )
{
   DWORD dwRet = 0;
   DWORD dwSize, dwCnt;
   if( ( ppx                        ) &&
       ( dwSize = ppx->px_Size      ) &&
       ( dwCnt  = ppx->px_Count     ) &&
       ( dwSize == PALEXSIZE(dwCnt) ) )
   {
      dwRet = dwSize;
   }
   return dwRet;
}

HGLOBAL	CopyhCOLR( HGLOBAL hg )
{
	LPSTR	lpb, lpbn;
	HGLOBAL	hgn, hgr;
	DWORD	dwSize;

	hgn = hgr = 0;
	lpb = lpbn = 0;
	if( ( hg                     ) &&
		( lpb = DVGlobalLock(hg) ) )
	{
		LPPALEX		lppx;
//		LPCOLR		lpcr;
		lppx = (LPPALEX)lpb;
//		lpcr = &lppx->px_Colrs[0];
//			( hgn = DVGlobalAlloc( GHND, dwSize ) ) &&
		if( ( dwSize = lppx->px_Size ) &&
			( dwSize == PALEXSIZE(lppx->px_Count) ) &&
			( hgn = DVGAlloc( "CopyhCOLR", GHND, dwSize ) ) &&
			( lpbn = DVGlobalLock(hgn) ) )
		{
			memcpy( lpbn, lpb, dwSize );
			DVGlobalUnlock(hgn);
			hgr = hgn;
			hgn = 0;
		}
		else
		{
			chkit( "No way Hose! Size, Count or MEMORY!" );
			if( hgn )
				DVGlobalFree(hgn);
		}
		DVGlobalUnlock(hg);
	}
	else
	{
		chkit( "BAD! Parameter or MEMORY?? !!!" );
	}
	return hgr;
}

// return the COUNT OF COLORS from the PALEX structure
// (after checking it is a TRUE PALEX!) else ZERO
DWORD	GetCOLRCnt( LPSTR lpb )
{
	DWORD		dwi = 0;
	DWORD		dwSize;
	LPPALEX		lppx;
	if( ( lppx = (LPPALEX)lpb    ) &&
		( dwSize = lppx->px_Size ) &&
		( dwSize == PALEXSIZE(lppx->px_Count) ) )
	{
		dwi = lppx->px_Count;
	}
	else
	{
		chkit( "Zheee! What happened??? !!!" );
	}
	return dwi;
}

// return a COLORREF (DWORD) of a particular INDEX
// of a PALEX structure,
// after checking it IS a valid PALEX structure
// and the INDEX passed is less than the COUNT
// ===============================================
COLORREF	GetCOLR( LPSTR lpb, DWORD index )
{
	COLORREF	dwi = 0;
	DWORD		dwSize, dwCnt;
	DLPPALEX	lppx;
	DLPCOLR		lpcr;
	if( ( lppx = (DLPPALEX)lpb        ) &&
		( dwSize = lppx->px_Size     ) &&
		( dwCnt  = lppx->px_Count    ) &&
		( dwSize == PALEXSIZE(dwCnt) ) &&
		( index  < dwCnt             ) )
	{
		/* in this array the COLOUR and FREQUENCY are in */
		/* a structured array                            */
		/* 1 - Index to this item                        */
		/* 2 - extract the colour (or freq) from here    */
		lpcr = &lppx->px_Colrs[index];
		dwi = lpcr->cr_Color;
		/* ============================================= */

	}
	else
	{
		chkit( "No DLPPALEX, dwSize, PALEXSIZE(dwCnt) or ..." );
	}

	return dwi;
}

// retunr the FREQUENCY (DWORD) of a particular INDEX
// from a PALEX structure after checking it is a VALID PALEX
// =========================================================
DWORD	GetFREQ( LPSTR lpb, DWORD index )
{
	COLORREF	dwi = 0;
	DWORD		dwSize, dwCnt;
	DLPPALEX	lppx;
	DLPCOLR		lpcr;
	if( ( lppx = (DLPPALEX)lpb        ) &&
		 ( dwSize = lppx->px_Size     ) &&
		 ( dwCnt  = lppx->px_Count    ) &&
		 ( dwSize == PALEXSIZE(dwCnt) ) &&
		 ( index  < dwCnt             ) )
	{
		/* in this array the COLOUR and FREQUENCY are in */
		/* a structured array                            */
		/* 1 - Index to this item                        */
		/* 2 - extract the frequency (or *) from here    */
		lpcr = &lppx->px_Colrs[index];
		dwi = lpcr->cr_Freq;
	}
	else
	{
		chkit( "No DLPPALEX, dwSize, PALEXSIZE(dwCnt) or ..." );
	}

	return dwi;

}


// FIX20001129 - Do NOT return ZERO, under any circumstances!!!
DWORD	GetFREQMax( LPSTR lpb )
{
	DWORD	dwMax = 1;  // was ZERO, but DIV BY ZERO is a PROBLEM
	DWORD	dwi, dwf, dwCnt, dwiMx;
	if( ( lpb                     ) &&
		 ( dwCnt = GetCOLRCnt(lpb) ) )
	{
		for( dwi = 0; dwi < dwCnt; dwi++ )
		{
			dwf = GetFREQ( lpb, dwi );
			if( dwf > dwMax )
			{
				dwMax = dwf;
				dwiMx = dwi;   // offset to THIS maximum
			}
		}
	}
	return dwMax;  // FIX20001129 - return a MINIMUM of ONE(1) - avoid div par ZERO!
}

DWORD	GetFREQMin( LPSTR lpb )
{
	DWORD	dwMin = (DWORD)-1;  // presume INVALID
	DWORD	dwi, dwf, dwCnt, dwiMx;
	if( ( lpb                     ) &&
		 ( dwCnt = GetCOLRCnt(lpb) ) )
	{
		for( dwi = 0; dwi < dwCnt; dwi++ )
		{
			dwf = GetFREQ( lpb, dwi );
			if( dwf < dwMin )
			{
				dwMin = dwf;
				dwiMx = dwi;   // offset to THIS maximum
			}
		}
	}
	return dwMin;  // return -1 if error!
}


DWORD GetUCCnt2( LPSTR lpbits,	// 32-bit alligned array of 24-bit colors
			  int Cols, DWORD Size, int iWid, int iHeight,
			 DWORD wBPP, LPSTR lpDib )
{
	DWORD		rcol;
	HGLOBAL		hBuf;
//	int		i, j, Width, k, l;
	DWORD	i, j, Width, k, l;
	//int		m;
//	int		Wi;
	DWORD		Wi;
	BYTE		r, g, b;
	BYTE FAR *	lpBuf;
	BYTE FAR *	lps;
	DWORD		bsize, dwMax, csize;
	BOOL		flg;
	LPTSTR		lpd;
	LPTSTR		lpfm;
	DWORD		Wid, Height;
	COLORREF	cr, crmx;
	LPDWORD		lpdw;
	HGLOBAL		hg;
	LPSTR		lpb;
	LPPALEX		lppx;
	LPCOLR		lpcr;
	DWORD		dwi;
	LPRGBQUAD	lpq;
	LPPALEX		lppxs;
	LPCOLR		lpcrs;
	BYTE		br, bg, bb;
	LPSTR		lpCOLR;

	flg = FALSE;
	rcol = Cols;
	hBuf = 0;
	lpBuf = 0;
	lpd = &gszTmp[0];
//	dwMax = ( Wid * Height );
	Wid    = (DWORD)iWid;
	Height = (DWORD)iHeight;
	lpCOLR = ( lpDib + sizeof( BITMAPINFOHEADER ) );
	lpCOLR = ( lpDib + OffsetToColor(lpDib) );
	dwMax = ( Wid * Height );

	r = 255;
	g = 255;
	b = 255;
	cr = RGB(r,g,b);
	crmx = ((DWORD)cr + 1);
	csize = 0;
	if( wBPP == 24 )
	{
//		bsize = (Wid * Height * 3);
		//bsize = ( dwMax * sizeof(DWORD) );
		flg = TRUE;
		lpfm = "24-bit";
//		r = 255;
//		g = 255;
//		b = 255;
//		cr = RGB(r,g,b);
//		crmx = ((DWORD)cr + 1);
		/* ok, allocate a DWORD count for every colour */
		/* 0x00ffffff =  16,777,215 + 1 = 16MB x 4 = 64MB contig */
		bsize = ( crmx * 4 );
	}
	else if( wBPP == 8 )
	{
		crmx  = 256;
		bsize = crmx * 10;	/* Colour byte/index, indexed, RGB & freq */
		bsize = ( crmx + sizeof(PALEX) + ( crmx * sizeof(COLR) ) );
		// bsize = dwMax;
		flg = TRUE;
		lpfm = "8-bit(256)";
	}
	else if( wBPP == 4 )
	{
		bsize = (dwMax / 2);
		crmx  = 16;
		bsize = crmx * 10;	/* Colour byte/index, indexed, RGB & freq */
		bsize = ( crmx + sizeof(PALEX) + ( crmx * sizeof(COLR) ) );
		flg = TRUE;
		lpfm = "4-bit(16)";
	}
	else if( wBPP == 1 )
	{
		bsize = (dwMax / 8);
		rcol = 2;
		lpfm = "1-bit(2)";
	}

	Width = WIDTHBYTES( (Wid * wBPP) );
	wsprintf( lpd,
		"Doing %s colour count ..."MEOR
		"Size %d x %d (%d pixels) ..."MEOR
		"Moment ... (w=%d, s=%d)",
		lpfm,
		Wid,
		Height,
		( Wid * Height ),
		Width,
		bsize );
	DO(lpd);
	if( ( flg                                 ) &&
		( lps = lpbits                        ) &&
		( hBuf = DVGlobalAlloc( GHND, bsize ) ) &&
		( lpBuf = DVGlobalLock( hBuf )        ) )
	{
		k = 0;
		if( wBPP == 24 )
		{
			/* 3 bytes or 16 million colours */
			Width = WIDTHBYTES( (Wid * wBPP) );
			Wi = Wid * 3;
			BeginProgM(0);	/* setup the progress meter */
			//ProgMsg( "Moment, doing colour count ..." );
			ProgMsg(lpd);
			lpdw = (LPDWORD)lpBuf;
			k = CountRGB( Height, Wid, Width, lps, lpdw, crmx );
//		bsize = ( crmx * 4 );
//			if( ( k * 4 ) < bsize )
//typedef struct tagPALEX {
//	DWORD	ps_Size;
//	DWORD	px_Count;
//	COLR	px_Colrs[n] DWORD cr_Color and cr_Freq;
//}PALEX;
			if( k )
			{
//				HGLOBAL	hg;
//				LPSTR	lpb;
//				LPPALEX	lppx;
//				LPCOLR	lpcr;
//				DWORD	dwi;
				csize = PALEXSIZE(k);
				//csize = ( sizeof(PALEX) + ( a * sizeof(COLR) ) )

				if( ( hg  = DVGlobalAlloc( GHND, csize ) ) &&
					( lpb = DVGlobalLock( hg )           ) )
				{
					j = 0;
					lppx = (LPPALEX)lpb;
					lpcr = &lppx->px_Colrs[0];
					for( i = 0; i < crmx; i++ )
					{
						/* if there is a COUNT at this color */
						if( dwi = lpdw[i] )
						{
							/* and we are still counting */
							if( j < k )
							{
								/* store offset as the COLOR */
								lpcr[j].cr_Color = (COLORREF)i;
								/* and its frequency         */
								lpcr[j].cr_Freq  = dwi;
								j++;
								/* and bump our progress */
								ProgP( (int)(( j * 100 ) / k ) );
							}
							else
							{
								chkuc( "Count error 1!" );
								break;
							}
						}
					}
					if( ( i == crmx ) &&
						( j == k  ) )
					{
						/* good count collected into PALEX/COLR array */
						lppx->px_Size  = csize;
						lppx->px_Count = k;
						ghCOLR = hg;
						DVGlobalUnlock(hg);
						hg = 0;
					}
					else
					{
						chkuc( "Count error 2!" );
						if( ghCOLR )
							DVGlobalFree(ghCOLR);
						ghCOLR = 0;
						lppx->px_Count = 0;
						DVGlobalUnlock(hg);
					}

					if( hg )
					{
						DVGlobalFree(hg);
						hg = 0;
					}
				}
				else
				{
					DIBError( ERR_MEMORY );
				}
			}
			KillProg();
		}
		else if( wBPP == 8 )
		{
//			LPRGBQUAD	lpq;
//			LPPALEX	lppxs;
//			LPCOLR	lpcrs;
			//DWORD	dwi;
			/* 1 byte or 256 colours */
			Width = WIDTHBYTES( Wid );
			Width = WIDTHBYTES( Wid * wBPP );
			Wi = Wid;
			lpfm = lpBuf + crmx;
			lppxs = (LPPALEX)lpfm;
			lpcrs = &lppxs->px_Colrs[0];
			lpq   = (LPRGBQUAD)lpCOLR;
			for( i = 0; i < Height; i++ )
			{	// For each ROW
				for( j = 0; j < Wid; j++ )
				{
					/* extract byte */
					r = *lps++;
					/* do not repeat a byte */
					for( l = 0; l < k; l++ )
					{
						if( lpBuf[l] == r )
						{
							break;
						}
					}
					if( l < k )
					{
						lpcrs[l].cr_Freq += 1;
					}
					else
					{
						/* keep the byte */
						lpBuf[k] = r;
						lpdw = (LPDWORD) (LPRGBQUAD) &lpq[r];
						lpcrs[k].cr_Color = RGB( lpq[r].rgbRed,
							lpq[r].rgbGreen, lpq[r].rgbBlue );
						lpcrs[k].cr_Freq  = 1;
						k++;
					}
				}
				while( j++ < Width )
					lps++;
			}
			if( k )
			{
				csize = PALEXSIZE(k);
//				csize = ( sizeof(PALEX) + ( k * sizeof(COLR) ) );
				if( ( hg  = DVGlobalAlloc( GHND, csize ) ) &&
					( lpb = DVGlobalLock( hg )           ) )
				{
					j = 0;
					lppx = (LPPALEX)lpb;
					lpcr = &lppx->px_Colrs[0];
					for( i = 0; i < k; i++ )
					{
						lpcr[j].cr_Color = lpcrs[i].cr_Color;
						lpcr[j].cr_Freq  = lpcrs[i].cr_Freq;
						j++;
					}
					if( ( i == k  ) &&
						( j == k  ) )
					{
						/* good count collected into PALEX/COLR array */
						lppx->px_Size  = csize;
						lppx->px_Count = k;
						ghCOLR = hg;
						DVGlobalUnlock(hg);
						hg = 0;
					}
					else
					{
						if( ghCOLR )
							DVGlobalFree(ghCOLR);
						ghCOLR = 0;
						lppx->px_Count = 0;
						DVGlobalUnlock(hg);
					}

					if( hg )
					{
						DVGlobalFree(hg);
						hg = 0;
					}
				}
				else
				{
					DIBError( ERR_MEMORY );
				}
			}
		}
		else if( wBPP == 4 )
		{
			Width = WIDTHBYTES( Wid );
			Width = WIDTHBYTES( Wid * wBPP );
			Wi = Wid / 2;	// Each FETCH = 2 COLORS!!!
			lpfm = lpBuf + crmx;
			lppxs = (LPPALEX)lpfm;
			lpcrs = &lppxs->px_Colrs[0];
			/* are we pointing at the bitmap COLOR TABLE? */
			lpq   = (LPRGBQUAD)lpCOLR;
			OutColours( lpq, 16 );
			for( i = 0; i < Height; i++ )
			{	// For each ROW
				for( j = 0; j < Wi; j++ )
				{	// For each COLUMN
					r = *lps++;		// Get TWO nibbles
					b = r & 0x0f;	// Split it
					g = (r & 0xf0) >> 4;
					for( l = 0; l < k; l++ )
					{	// Search first nibble
						if( lpBuf[l] == b )
						{
							break;
						}
					}
					if( l < k )
					{
						lpcrs[l].cr_Freq += 1;
					}
					else
					{
						/* keep the byte */
						lpBuf[k] = b;
						lpdw = (LPDWORD) (LPRGBQUAD) &lpq[b];
						br = lpq[b].rgbRed;
						bg = lpq[b].rgbGreen;
						bb = lpq[b].rgbBlue;
//						lpcrs[k].cr_Color = RGB( lpq[b].rgbRed,
//							lpq[b].rgbGreen, lpq[b].rgbBlue );
						lpcrs[k].cr_Color = RGB( br, bg, bb );
						lpcrs[k].cr_Freq  = 1;
						k++;
					}

					for( l = 0; l < k; l++ )
					{	// Search 2nd nibble
						if( lpBuf[l] == g )
							break;
					}
					//if( l == k )
					//{
					//	lpBuf[k++] = g;
					//}
					if( l < k )
					{
						lpcrs[l].cr_Freq += 1;
					}
					else
					{
						/* keep the byte */
						lpBuf[k] = g;
						lpdw = (LPDWORD) (LPRGBQUAD) &lpq[g];
						lpcrs[k].cr_Color = RGB( lpq[g].rgbRed,
							lpq[g].rgbGreen, lpq[g].rgbBlue );
						lpcrs[k].cr_Freq  = 1;
						k++;
					}
				}
				while( j++ < Width )
					lps++;
			}
			if( k )
			{
				csize = PALEXSIZE(k);
//				csize = ( sizeof(PALEX) + ( k * sizeof(COLR) ) );
				if( ( hg  = DVGlobalAlloc( GHND, csize ) ) &&
					( lpb = DVGlobalLock( hg )           ) )
				{
					j = 0;
					lppx = (LPPALEX)lpb;
					lpcr = &lppx->px_Colrs[0];
					for( i = 0; i < k; i++ )
					{
						lpcr[j].cr_Color = lpcrs[i].cr_Color;
						lpcr[j].cr_Freq  = lpcrs[i].cr_Freq;
						j++;
					}
					if( ( i == k  ) &&
						( j == k  ) )
					{
						/* good count collected into PALEX/COLR array */
						lppx->px_Size  = csize;
						lppx->px_Count = k;
						ghCOLR = hg;
						DVGlobalUnlock(hg);
						hg = 0;
					}
					else
					{
						if( ghCOLR )
							DVGlobalFree(ghCOLR);
						ghCOLR = 0;
						lppx->px_Count = 0;
						DVGlobalUnlock(hg);
					}

					if( hg )
					{
						DVGlobalFree(hg);
						hg = 0;
					}
				}
				else
				{
					DIBError( ERR_MEMORY );
				}
			}
		}
		rcol = k;
	}
	else
	{
		// MEMORY FAILED
		DO( "Get memory FAILED!"MEOR );
		if( giUseSRE )
		{
			SRErrorN( IDS_MEMFAILED, NULL );
		}
		else
		{
			DIBError( ERR_MEMORY );
		}
		rcol = 0;
	}

	if( hBuf && lpBuf )
		DVGlobalUnlock( hBuf );
	if( hBuf )
		DVGlobalFree( hBuf );

	return( rcol );
}

int GetUCCnt( LPDIBINFO lpDIBInfo, LPSTR lpbits, int Cols,
			 DWORD Size, int Wid, int Height, DWORD wBPP,
			 LPSTR lpDIB )
{
	int		k = 0;
	HGLOBAL	hg;
	LPSTR	lpb;
	DWORD		dwi, dwCnt, dwMax, dwf;
	COLORREF	cr;
	LPSTR		lpd;
	BYTE		r,g,b;

	lpd = &gszTmp[0];
	if( hg = lpDIBInfo->di_hCOLR )
	{
		LPPALEX	lppx;
		if( lppx = (LPPALEX)DVGlobalLock(hg) )
		{
			k = lppx->px_Count;
			DVGlobalUnlock(hg);
		}	// got GetUCCnt( )
		else
		{
			DIBError( ERR_MEMORY );
		}
		return k;
	}

	if( ghCOLR )
		DVGlobalFree(ghCOLR);
	ghCOLR = 0;

//	k = GetUCCnt2( lpbits, lpi->iColors,
//						dwSize, lpi->cxDIB, lpi->cyDIB,
//						lpi->wBPP );
	k = GetUCCnt2( lpbits, Cols,
						Size, Wid, Height,
						wBPP, lpDIB );

	if( hg = ghCOLR )
	{
		if( ( lpb = DVGlobalLock(hg)  ) &&
			( dwCnt = GetCOLRCnt(lpb) ) )
		{
			wsprintf(lpd,
				"Found %d colours ..."MEOR,
				dwCnt );
			DO(lpd);
			dwMax = 1;
			for( dwi = 0; dwi < dwCnt; dwi++ )
			{
				cr = GetCOLR(lpb,dwi);
				dwf= GetFREQ(lpb,dwi);
				r = GetRValue(cr);
				g = GetGValue(cr);
				b = GetBValue(cr);
				wsprintf( lpd,
					"Col:0x%06X (r%3d,g%3d,b%3d) F=%d."MEOR,
					cr,
					(r & 0xff),
					(g & 0xff),
					(b & 0xff),
					dwf );
				DO(lpd);
				if( dwf > dwMax )
					dwMax = dwf;
			}
			wsprintf(lpd,
				"Shown %d colors ... Max. freq = %d"MEOR,
				dwCnt,
				dwMax );
			DO(lpd);

			DVGlobalUnlock(hg);
		}
		else
		{
			wsprintf(lpd,
				"Get colours FAILED???"MEOR );
			DO(lpd);
		}
		lpDIBInfo->di_hCOLR = hg;
		ghCOLR = 0;
	}
	else
	{
			wsprintf(lpd,
				"NO colours ???"MEOR );
			DO(lpd);
	}
	return k;

}

int GetUCCnt_OK( LPSTR lpb, int Cols, DWORD Size, int Wid, int Height,
			 WORD wBPP )
{
	int		rcol;
	HGLOBAL	hBuf;
	int		i, j, Width, k, l, m, Wi;
	BYTE	r, g, b;
	BYTE FAR *lpBuf;
	BYTE FAR *lps;
	DWORD	bsize;
	BOOL	flg;
	LPTSTR	lpd;

	flg = FALSE;
	rcol = Cols;
	hBuf = 0;
	lpBuf = 0;
	lpd = &gszTmp[0];

	if( wBPP == 24 )
	{
		bsize = (Wid * Height * 3);
		flg = TRUE;
	}
	else if( wBPP == 8 )
	{
		bsize = (Wid * Height);
		flg = TRUE;
	}
	else if( wBPP == 4 )
	{
		bsize = (Wid * Height * 2);
		flg = TRUE;
	}
	else if( wBPP == 1 )
	{
		rcol = 2;
	}
	if( flg &&
		( lps = lpb ) &&
		( hBuf = DVGlobalAlloc( GHND, bsize ) ) &&
		( lpBuf = DVGlobalLock( hBuf ) ) )
	{
		k = 0;
		if( wBPP == 24 )
		{
			Width = WIDTHBYTES( (Wid * wBPP) );
			Wi = Wid * 3;

			BeginProgM(0);	/* setup the progress meter */
			wsprintf( lpd,
				"Doing 24-bit colour count ..."MEOR
				"Size %dx%d (%d pixels) ..."MEOR
				"Moment ...",
				Wid,
				Height,
				( Wid * Height ) );
			//ProgMsg( "Moment, doing colour count ..." );
			ProgMsg(lpd);
			for( i = 0; i < Height; i++ )
			{
				// For each ROW
				for( j = 0; j < Wid; j++ )
				{	// For each COLUMN
					r = *lps++;
					g = *lps++;
					b = *lps++;
					m = k * 3;
					for( l = 0; l < m; l+=3 )
					{
						if( (lpBuf[l+0] == r) &&
							(lpBuf[l+1] == g) &&
							(lpBuf[l+2] == b) )
							break;
					}
					if( l == m )
					{
						k++;	// Count a NEW color
						lpBuf[l+0] = r;
						lpBuf[l+1] = g;
						lpBuf[l+2] = b;
					}
				}
				m = j * 3;
				while( m++ < Width )
					lps++;
				if( i )
				{
					// add percentage after each ROW
					//#define	ProgP(p)			SetProgP(p)
					ProgP( (int)(( i * 100 ) / Height ) );
				}
			}
			KillProg();
		}
		else if( wBPP == 8 )
		{
			Width = WIDTHBYTES( Wid );
			Width = WIDTHBYTES( Wid * wBPP );
			Wi = Wid;
			for( i = 0; i < Height; i++ )
			{	// For each ROW
				for( j = 0; j < Wid; j++ )
				{
					r = *lps++;
					for( l = 0; l < k; l++ )
					{
						if( lpBuf[l] == r )
							break;
					}
					if( l == k )
					{
						lpBuf[k++] = r;
					}
				}
				while( j++ < Width )
					lps++;
			}
		}
		else if( wBPP == 4 )
		{
			Width = WIDTHBYTES( Wid );
			Width = WIDTHBYTES( Wid * wBPP );
			Wi = Wid / 2;	// Each FETCH = 2 COLORS!!!
			for( i = 0; i < Height; i++ )
			{	// For each ROW
				for( j = 0; j < Wi; j++ )
				{	// For each COLUMN
					r = *lps++;		// Get TWO nibbles
					b = r & 0x0f;	// Split it
					g = (r & 0xf0) >> 4;
					for( l = 0; l < k; l++ )
					{	// Search first nibble
						if( lpBuf[l] == b )
							break;
					}
					if( l == k )
					{
						lpBuf[k++] = b;
					}
					for( l = 0; l < k; l++ )
					{	// Search 2nd nibble
						if( lpBuf[l] == g )
							break;
					}
					if( l == k )
					{
						lpBuf[k++] = g;
					}
				}
				while( j++ < Width )
					lps++;
			}
		}
		rcol = k;
	}
	if( hBuf && lpBuf )
		DVGlobalUnlock( hBuf );
	if( hBuf )
		DVGlobalFree( hBuf );
	return( rcol );
}


//void DvImageAtt( HWND hwnd )
void Dv_IDM_IMAGEATT( HWND hwnd )	// (uses IDD_IMAGEATT template)
{
	LPISTR	lpi;
	LPSTR	   lpb, lpbits, lpfn;
#ifndef	WIN32
	FARPROC	lpInfo;	// NOT required in WIN32!!!
#endif
	HDC		hDC;
	DWORD	   dwSize;
   HWND     hMDI;
   HGLOBAL  hDIBInfo;
   PDI      lpDIBInfo;
   HANDLE   hDIB;
#ifndef   USEITHREAD
	ISTR	   IStr;
#endif   // #ifndef   USEITHREAD

	hDIBInfo  = 0;
	lpDIBInfo = 0;
   hDIB      = 0;
	hMDI = GetCurrentMDIWnd();
   if(hMDI)
      hDIBInfo = GetWindowExtra( hMDI, WW_DIB_HINFO );
   if(hDIBInfo)
      lpDIBInfo = (PDI) DVGlobalLock( hDIBInfo );
   if(lpDIBInfo)
      hDIB = lpDIBInfo->hDIB;

	if( hDIB )
	{
   	Hourglass( TRUE );		// setup WAIT
#ifdef   USEITHREAD
      lpi = &lpDIBInfo->di_sIS;
#else !USEITHREAD
   	lpi = &IStr;
	   NULPISTR(lpi);
      lpi->lpDIBInfo = lpDIBInfo;
#endif   // ifdef USEITHREAD y/n
// FIX980504 Maybe NO BITMAP( lpi->hBitmap = lpi->lpDIBInfo->hBitmap )!
// FIX980430 Removed( GetObject( lpi->hBitmap, sizeof(BITMAP), &lpi->bm ) ) )
   	lpi->fChanged  = FALSE;	// set no change yet
	   lpi->dwFlag    = 0;
      lpi->hMDI      = hMDI;
      lpi->hDIBInfo  = hDIBInfo;
		lpi->hDIB      = hDIB;
		lpfn = GetFNBuf();
		lstrcpy( lpfn, &lpDIBInfo->di_szDibFile[0] );
		lpi->lpFN = lpfn;

//		lpi->cxDIB     = lpDIBInfo->di_dwDIBWidth;
//		lpi->cyDIB     = lpDIBInfo->di_dwDIBHeight;
		lpb = DVGlobalLock( hDIB ); // LOCK DIB HANDLE
		if(lpb)  
		{
			/* FIX990925 - Add WAIT cursor, and maybe PROGRESS
				thu -
				DBWaitON	= SRWait( TRUE, SRW_BGN )
				DBWaitINC	= SRWait( TRUE, SRW_INC ), AND
				DBWaitOFF	= SRWait( FALSE, SRW_END )
			   ============================================	*/
			DBWaitON;
			lpbits =       FindDIBBits( lpb );
			lpi->iColors = DIBNumColors( lpb );
			/* ====== this is based on color depth only ====== */
			lpi->iCalcCols = CalcDIBColors( lpb );
			/* =================================== */
			DBWaitINC;
			if( ( lpDIBInfo->di_dwDIBBits ) &&
				( dwSize = 
				(DWORD)( (WIDTHBYTES( lpDIBInfo->di_dwDIBWidth * lpDIBInfo->di_dwDIBBits )) *
				lpDIBInfo->di_dwDIBHeight ) ) )
			{
				/* ====== this can take longer ====== */
				lpi->iUsedCols = GetUCCnt( lpDIBInfo,
					lpbits, lpi->iColors,
					dwSize,
               lpDIBInfo->di_dwDIBWidth,  //lpi->cxDIB,
               lpDIBInfo->di_dwDIBHeight, //lpi->cyDIB,
					lpDIBInfo->di_dwDIBBits, //lpi->wBPP,
               lpb );
				/* =================================== */
				DBWaitINC;
			}

			DVGlobalUnlock( hDIB );  // UNLOCK DIB HANDLE
			DBWaitOFF;	/* end = SRWait( FALSE, SRW_END ) */

		}
		else
			lpi->iColors = 0;

		GetClientRect( hMDI, &lpi->rcClient );
		lpi->rcClip.left   = lpDIBInfo->rcClip.left;
		lpi->rcClip.top    = lpDIBInfo->rcClip.top;
		lpi->rcClip.right  = lpDIBInfo->rcClip.right;
		lpi->rcClip.bottom = lpDIBInfo->rcClip.bottom;
		if( IsRectEmpty( &lpi->rcClip ) )
		{
			/* set CLIP to maximum of image */
			lpi->rcClip.left   = 0;
			lpi->rcClip.top    = 0;
			lpi->rcClip.right  = lpDIBInfo->di_dwDIBWidth;  //lpi->cxDIB;
			lpi->rcClip.bottom = lpDIBInfo->di_dwDIBHeight; //lpi->cyDIB;
		}

		/* === INTO DIALOG BOX === */
#ifdef	WIN32
#ifdef   USEITHREAD
		DialogBoxParam( ghDvInst,
			MAKEINTRESOURCE(IDD_IMAGEATT),	// IDM_IMAGEATT
			lpi->hMDI,
			(DLGPROC)INFO2DLGPROC, // lpInfo,
			(LPARAM) hDIBInfo );
#else // !USEITHREAD
		DialogBoxParam( ghDvInst,
			MAKEINTRESOURCE(IDD_IMAGEATT),	// IDM_IMAGEATT
			lpi->hMDI,
			INFO2DLGPROC, // lpInfo,
			(DWORD) lpi );
#endif   // ifdef   USEITHREAD y/n
#else	/* WIN32 */
		lpInfo = MakeProcInstance( INFO2DLGPROC, ghDvInst );
		DialogBoxParam( ghDvInst,
			MAKEINTRESOURCE(IDD_IMAGEATT),	// IDM_IMAGEATT
			lpi->hMDI,
			lpInfo,
			(DWORD) lpi );
		FreeProcInstance( lpInfo );
#endif	/* WIN32 y/n */
		/* === OUT  DIALOG BOX === */

		if( lpi->fChanged )	// Something is changed
		{
			if( lpi->dwFlag & (fc_1 | fc_2) )
			{	
				// Changed WINDOW size

			}
			if( lpi->dwFlag & (fc_3 | fc_4) )
			{	
				// Changed BITMAT size!!!
			}

			// === Clip Rectangle Changes ===
			// ************************************************
//			if( lpi->dwFlag & ( fc_5 | fc_6 | fc_7 | fc_8 | fc_9 | fc_10 ) )
			if( lpi->dwFlag & fc_ChgClip )
			{
				// Changed some CLIP size
				if( hDC = GetDC( hMDI ) )
				{
#ifdef	WIN32
					SetWindowOrgEx( hDC,
						GetScrollPos( lpi->hMDI, SB_HORZ),
						GetScrollPos( lpi->hMDI, SB_VERT),
						NULL );
#else	// !WIN32

					SetWindowOrg( hDC,
						GetScrollPos( lpi->hMDI, SB_HORZ),
						GetScrollPos( lpi->hMDI, SB_VERT));
#endif	// WIN32 y/n

					// Remove the OLD CLIP
					DrawSelect( hDC, &lpDIBInfo->rcClip );
					// Copy the NEW CLIP into
					// the Child windows's info structure
					lpDIBInfo->rcClip = lpi->rcClip;
					// Draw the NEW CLIP
					DrawSelect( hDC, &lpDIBInfo->rcClip );
					// and pop the HDC
					ReleaseDC( hMDI, hDC);

				}
			}
			// ************************************************
		}
	}

	if( hDIBInfo && lpDIBInfo )
		DVGlobalUnlock( hDIBInfo );

	Hourglass( FALSE );
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ***************************************************************
// The following is the code for the INFO dialog box
// -------------------------------------------------

BOOL Do_Info2_Paint( HWND hDlg, BOOL bTP )
{
   BOOL     flg = FALSE;
   HANDLE   hd, hDIB;
   PDI      pdi;
   HWND     hWnd;
   PAINTSTRUCT ps;

   if( bTP )
      BeginPaint(hDlg,&ps);

	if( ( hd = GET_PROP(hDlg, ATOM_INFO) ) &&
       ( pdi = (PDI)DVGlobalLock(hd)    ) )
	{
      // and put up a small image, with the CLIP region marked
      if( ( hDIB = pdi->hDIB ) &&
          ( hWnd = GetDlgItem(hDlg, IDC_FRAME ) ) )
      {
         LPSTR lpDIB;
         HDC   hdc;
         RECT  rcFrame;
         INT   ir;
         if( ( GetClientRect(hWnd,&rcFrame) ) &&
             ( lpDIB = DVGlobalLock(hDIB)   ) ) 
         {
            pdi->hwndIFrame = hWnd;    // handle to FRAME on IMAGEAT dialog
            pdi->rcIFrame   = rcFrame; // and its size
            if( hdc = GetDC(hWnd) )
            {
               SetStretchBltMode( hdc, COLORONCOLOR );
               ir = StretchDIBits( hdc,		// hDC
                  0,		// DestX
                  0,    // DestY
                  rcFrame.right,    // nDestWidth
                  rcFrame.bottom,   // nDestHeight
                  0,    // SrcX
                  0,    // SrcY
                  pdi->di_dwDIBWidth,   // wSrcWidth
                  pdi->di_dwDIBHeight,  // wSrcHeight
                  FindDIBBits(lpDIB),  
                  (LPBITMAPINFO)lpDIB, // lpBitsInfo
                  DIB_RGB_COLORS,		// wUsage
                  SRCCOPY );
               if( ir != GDI_ERROR )
               {
                  RECT  rcClip;
                  rcClip = pdi->rcClip;
                  rcClip.left   = ((pdi->di_sIS.rcClip.left   * rcFrame.right)
                     / pdi->di_dwDIBWidth);  // get CLIP
                  rcClip.right  = ((pdi->di_sIS.rcClip.right  * rcFrame.right)
                     / pdi->di_dwDIBWidth);
                  rcClip.top    = ((pdi->di_sIS.rcClip.top    * rcFrame.bottom)
                     / pdi->di_dwDIBHeight);
                  rcClip.bottom = ((pdi->di_sIS.rcClip.bottom * rcFrame.bottom)
                     / pdi->di_dwDIBHeight);
                  // Paint size adj clip,   add text of actual
                  DVPaintClip2(hdc, &rcClip, &pdi->di_sIS.rcClip, &rcFrame );
                  pdi->rcClip2 = pdi->di_sIS.rcClip;  // keep WORK clip                
                  pdi->rcClip3 = rcClip;  // keep PAINTED clip                
                  flg = TRUE;

               }
               ReleaseDC(hWnd,hdc);
            }
            DVGlobalUnlock(hDIB);
         }
      }

      DVGlobalUnlock(hd);
	}

   if( bTP )
      EndPaint(hDlg,&ps);

	return( flg );
}

VOID  Put_Info2_Clip( HWND hDlg, PRECT prcClip )
{
		// === Clip size ===
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT5,		// identifier of control
			(prcClip->right - prcClip->left), // value to set
			FALSE );
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT6,		// identifier of control
			(prcClip->bottom - prcClip->top), // value to set
			FALSE );

		// === Clip rectangle ===
		// **********************
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT7,		// identifier of control
			prcClip->left, // value to set
			FALSE );
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT8,		// identifier of control
			prcClip->top, // value to set
			FALSE );
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT9,		// identifier of control
			prcClip->right, // value to set
			FALSE );
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT10,		// identifier of control
			prcClip->bottom, // value to set
			FALSE );
		// **********************

}
// init dialog for IDM_IMAGEATT menu item (use IDD_IMAGEATT template)
BOOL Do_Info2_Init( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL	flg = FALSE;
	LPISTR	lpi;
   HANDLE   hd;
   PDI      pdi;
//   HWND     hWnd;
//   HGLOBAL  hDIB;

//	char	szBuffer[20];
//	if( lpi = (LPISTR)lParam )
   SET_PROP( hDlg, ATOM_INFO, lParam );
   CenterWindow(hDlg, ghMainWnd);
	if( ( hd = (HANDLE)lParam         ) &&
       ( pdi = (PDI)DVGlobalLock(hd) ) )
   {
//		lpIStr = lpi;
      lpi = &pdi->di_sIS;
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT1,		// identifier of control
			lpi->rcClient.right, // value to set
			FALSE );
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT2,		// identifier of control
			lpi->rcClient.bottom, // value to set
			FALSE );

      // FIX20001209 - These are now READ-ONLY information displays
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT3,		// identifier of control
			pdi->di_dwDIBWidth,  //lpi->cxDIB,	// NOT bm.bmWidth, // value to set
			FALSE );
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT4,		// identifier of control
			pdi->di_dwDIBHeight, //lpi->cyDIB,	// NOT bm.bmHeight, // value to set
			FALSE );

      Put_Info2_Clip( hDlg, &lpi->rcClip );

      // some more READ ONLY information edit boxes
      // ******************************************
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT11,		// identifier of control
			lpi->iColors, // value to set
			FALSE );
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT12,		// identifier of control
			lpi->iUsedCols, // value to set
			FALSE );
		SetDlgItemInt( hDlg,	// handle of dialog box
			IDC_EDIT13,		// identifier of control
			pdi->di_dwDIBBits, // lpi->wBPP, // value to set
			FALSE );
		SetDlgItemText( hDlg,
			IDC_EDIT14,
			lpi->lpFN );

		flg = TRUE;

      DVGlobalUnlock(hd);

	}

	return( flg );
}


// Close IDM_IMAGEATT menu item dialog (uses IDD_IMAGEATT template)
BOOL Do_Info2_Close( HWND hDlg )
{
	BOOL	flg = FALSE;
	UINT	ui;
	BOOL	fOK;
	LPISTR	lpi;
	RECT	rc;
   HANDLE   hd;
   PDI      pdi;

//	if( lpi = lpIStr )
//#define  lpi->cxDIB     pdi->di_dwDIBWidth
//#define  lpi->cyDIB     pdi->di_dwDIBHeight
	if( ( hd = GET_PROP(hDlg, ATOM_INFO) ) &&
       ( pdi = (PDI)DVGlobalLock(hd)    ) )
	{
      lpi = &pdi->di_sIS;
		ui = GetDlgItemInt( hDlg,	// handle to dialog box
			IDC_EDIT1,	// control identifier
			&fOK,		// points to variable to receive success/failure indicator
			FALSE );	// specifies whether value is signed or unsigned
		if( fOK )
		{
			if( lpi->rcClient.right != (int)ui ) // value
			{
				lpi->rcClient.right = ui;
				lpi->dwFlag |= fc_1;
				flg = TRUE;
			}
		}
		ui = GetDlgItemInt( hDlg,	// handle to dialog box
			IDC_EDIT2,	// control identifier
			&fOK,		// points to variable to receive success/failure indicator
			FALSE );	// specifies whether value is signed or unsigned
		if( fOK )
		{
			if( lpi->rcClient.bottom != (int)ui ) // value
			{
				lpi->rcClient.bottom = ui;
				lpi->dwFlag |= fc_2;
				flg = TRUE;
			}
		}

      // FIX20001209 - CAN NOT ALTER ACTUAL DIB WIDTH or HEIGHT
//		ui = GetDlgItemInt( hDlg,	// handle to dialog box
//			IDC_EDIT3,	// control identifier
//			&fOK,		// points to variable to receive success/failure indicator
//			FALSE );	// specifies whether value is signed or unsigned
//		if( fOK )
//		{
// FIX9804030 if( lpi->bm.bmWidth != (LONG)ui ) // value
//			if( lpi->cxDIB != (int)ui ) // value
//			{
//				lpi->bm.bmWidth = ui;
//				lpi->cxDIB = ui;
//				lpi->dwFlag |= fc_3;
//				flg = TRUE;
//			}
//		}
//		ui = GetDlgItemInt( hDlg,	// handle to dialog box
//			IDC_EDIT4,	// control identifier
//			&fOK,		// points to variable to receive success/failure indicator
//			FALSE );	// specifies whether value is signed or unsigned
//		if( fOK )
//		{
// FIX980430 if( lpi->bm.bmHeight != (LONG)ui ) // value
//			if( lpi->cyDIB != (int)ui ) // value
//			{
				//lpi->bm.bmHeight = ui;
//				lpi->cyDIB = (int)ui;
//				lpi->dwFlag |= fc_4;
//				flg = TRUE;
//			}
//		}

		// CLIP SIZE CHANGES
		// =================
		rc = lpi->rcClip;	// COPY the Rectangle
		// ======================================
		// Setting WIDTH
		// -------------
		ui = GetDlgItemInt( hDlg,	// handle to dialog box
			IDC_EDIT5,	// control identifier
			&fOK,		// points to variable to receive success/failure indicator
			FALSE );	// specifies whether value is signed or unsigned
		if( fOK )
		{
			if( (lpi->rcClip.right - lpi->rcClip.left) != (int)ui ) // value
			{
				if( (int)ui <= pdi->di_dwDIBWidth ) // lpi->cxDIB
				{
               // OK, this width will fit
					if( (ui - lpi->rcClip.left) <= pdi->di_dwDIBWidth )   //lpi->cxDIB )
					{
                  // OK, we stretch the right to match
						rc.right = lpi->rcClip.left + (int)ui;
					}
					else
					{
						rc.right = pdi->di_dwDIBWidth;   // lpi->cxDIB;
						rc.left  = pdi->di_dwDIBWidth - (int)ui;
					}
					lpi->dwFlag |= fc_5;
					flg = TRUE;
				}
				// Else can NOT match request!
			}
		}

		// Setting HEIGHT
		// --------------
		ui = GetDlgItemInt( hDlg,	// handle to dialog box
			IDC_EDIT6,	// control identifier
			&fOK,		// points to variable to receive success/failure indicator
			FALSE );	// specifies whether value is signed or unsigned
		if( fOK )
		{
			if( (lpi->rcClip.bottom - lpi->rcClip.top) != (int)ui ) // value
			{
				if( (int)ui <= pdi->di_dwDIBHeight )   // lpi->cyDIB
				{	// OK, height will fit
					if( (ui - lpi->rcClip.top) <= pdi->di_dwDIBHeight )
					{
						rc.bottom = lpi->rcClip.top + (int)ui;
					}
					else
					{
						rc.bottom = pdi->di_dwDIBHeight;
						rc.top = pdi->di_dwDIBHeight - (int)ui;
					}
					lpi->dwFlag |= fc_6;
					flg = TRUE;
				}
				// Else can NOT match request!
			}
		}

		// === Clip rectangle ===
		// **********************
		// Setting SPECIFIC left.top,right,bottom
		ui = GetDlgItemInt( hDlg,	// handle to dialog box
			IDC_EDIT7,	// control identifier
			&fOK,		// points to variable to receive success/failure indicator
			FALSE );	// specifies whether value is signed or unsigned
		if( fOK )
		{
			if( lpi->rcClip.left != (int)ui ) // value
			{
				if( (int)ui < pdi->di_dwDIBWidth )
				{
					lpi->dwFlag |= fc_7;
					flg = TRUE;
					rc.left = (int)ui;
				}
			}
		}

		ui = GetDlgItemInt( hDlg,	// handle to dialog box
			IDC_EDIT8,	// control identifier
			&fOK,		// points to variable to receive success/failure indicator
			FALSE );	// specifies whether value is signed or unsigned
		if( fOK )
		{
			if( lpi->rcClip.top != (int)ui ) // value
			{
				if( (int)ui < pdi->di_dwDIBHeight )
				{
					lpi->dwFlag |= fc_8;
					flg = TRUE;
					rc.top = (int)ui;
				}
			}
		}
		ui = GetDlgItemInt( hDlg,	// handle to dialog box
			IDC_EDIT9,	// control identifier
			&fOK,		// points to variable to receive success/failure indicator
			FALSE );	// specifies whether value is signed or unsigned
		if( fOK )
		{
			if( lpi->rcClip.right != (int)ui ) // value
			{
				if( ( (int)ui <= pdi->di_dwDIBWidth ) &&
					((int)ui > rc.left) )
				{
					lpi->dwFlag |= fc_9;
					flg = TRUE;
					rc.right = (int)ui;
				}
			}
		}

		ui = GetDlgItemInt( hDlg,	// handle to dialog box
			IDC_EDIT10,	// control identifier
			&fOK,		// points to variable to receive success/failure indicator
			FALSE );	// specifies whether value is signed or unsigned
		if( fOK )
		{
			if( lpi->rcClip.bottom != (int)ui ) // value
			{
				if( ( (int)ui <= pdi->di_dwDIBHeight ) &&
					( (int)ui > rc.top ) )
				{
					lpi->dwFlag |= fc_10;
					flg = TRUE;
					rc.bottom = (int)ui;
				}
			}
		}
		// **********************

		lpi->rcClip = rc;
		lpi->fChanged = flg;	// set a global change (if any)
      DVGlobalUnlock(hd);

	}

	return( flg );
}

void chknote( void )
{
	int i;
	i = 0;
}

BOOL  Do_Info2_Note( HWND hDlg, WPARAM wParam, LPARAM lParam )
{



   return TRUE;
}

// OLD STUFF SAID
// Send the EM_GETSEL message to the edit control.
//   // The low-order word of the return value is the character
//   // position of the caret relative to the first character in
//the
//   // edit control.
//   dwGetSel = (WORD)SendMessage(GetDlgItem(hWndDlg,101),
//EM_GETSEL,
//               0, 0L);
//   wStart = LOWORD(dwGetSel);
// NEW OCT 2000 MSDN SAYS
//EM_GETSEL
//The EM_GETSEL message retrieves the starting and ending
//character positions of the current selection in an edit control.
//You can send this message to either an edit control or a rich
//edit control.
//To send this message, call the SendMessage function with the
//following parameters. 
//SendMessage( 
//  (HWND) hWnd,              // handle to destination window 
//  EM_GETSEL,                // message to send
//  (WPARAM) wParam,          // starting position (LPDWORD)
//  (LPARAM) lParam          // ending position (LPDWORD)
//);
//Parameters
//wParam 
//Pointer to a buffer that receives the starting position of the
//selection. This parameter can be NULL. 
//lParam 
//Pointer to a buffer that receives the position of the first
//nonselected character after the end of the selection. This
//parameter can be NULL.
//Return Values
//The return value is a zero-based value with the starting
//position of the selection in the low-order word and the position
//of the first character after the last selected character in the
//high-order word. If either of these values exceeds 65,535, the
//return value is -1.
//It is better to use the values returned in wParam and lParam
//because they are full 32-bit values.

BOOL  Do_Info2_Cmd( HWND hDlg, DWORD cmd, DWORD code )
{
   BOOL  flg = FALSE;
   if( code == EN_CHANGE )
   {
      INT  ui;
      BOOL  fOK;
		ui = GetDlgItemInt( hDlg,	// handle to dialog box
			cmd,	   // control identifier
			&fOK,		// points to variable to receive success/failure indicator
			FALSE );	// specifies whether value is signed or unsigned
		if( fOK )
      {
         HGLOBAL  hd;
         PDI      pdi;
      	if( ( hd = GET_PROP(hDlg, ATOM_INFO) ) &&
             ( pdi = (PDI)DVGlobalLock(hd)    ) )
	      {
            RECT  rcClip;
            //  pdi->rcClip2 = pdi->di_sIS.rcClip;  // keep WORK clip                
            //  pdi->rcClip3 = rcClip;  // keep PAINTED clip
            rcClip = pdi->rcClip2;  // get last work clip
            fOK = FALSE;   // use for change only
            switch(cmd)
            {
		// === Clip size ===
//		SetDlgItemInt( hDlg,	// handle of dialog box
            case IDC_EDIT5:		// identifier of control
//			(lpi->rcClip.right - lpi->rcClip.left), // value to set
//			FALSE );
               if( ui != (rcClip.right - rcClip.left) )
               {
                  // change in WIDTH - keep current left and adjust right
                  rcClip.right = rcClip.left + ui;
                  fOK = TRUE;
               }
               break;
//		SetDlgItemInt( hDlg,	// handle of dialog box
            case IDC_EDIT6:		// identifier of control
//			(lpi->rcClip.bottom - lpi->rcClip.top), // value to set
//			FALSE );
               if( ui != (rcClip.bottom - rcClip.top) )
               {
                  rcClip.bottom = rcClip.top + ui;
                  fOK = TRUE;
               }
               break;
		// === Clip rectangle ===
		// **********************
//		SetDlgItemInt( hDlg,	// handle of dialog box
            case	IDC_EDIT7:		// identifier of control
//			lpi->rcClip.left, // value to set
//			FALSE );
               if( ui != rcClip.left )
               {
                  rcClip.left = ui;
                  fOK = TRUE;
               }
               break;
//		SetDlgItemInt( hDlg,	// handle of dialog box
            case	IDC_EDIT8:		// identifier of control
//			lpi->rcClip.top, // value to set
//			FALSE );
               if( ui != rcClip.top )
               {
                  rcClip.top = ui;
                  fOK = TRUE;
               }
               break;
//		SetDlgItemInt( hDlg,	// handle of dialog box
            case IDC_EDIT9:		// identifier of control
//			lpi->rcClip.right, // value to set
//			FALSE );
               if( ui != rcClip.right )
               {
                  rcClip.right = ui;
                  fOK = TRUE;
               }
               break;
//		SetDlgItemInt( hDlg,	// handle of dialog box
            case	IDC_EDIT10:		// identifier of control
//			lpi->rcClip.bottom, // value to set
//			FALSE );
               if( ui != rcClip.bottom )
               {
                  rcClip.bottom = ui;
                  fOK = TRUE;
               }
               break;

            }
            if( fOK )
            {
               RECT  rcNew, rcOld;
               RECT  rcFrame;
               HDC   hdc;

               rcFrame = pdi->rcIFrame;
               rcOld   = pdi->rcClip3;  // get PAINTED clip
            //pdi->hwndIFrame = hWnd;    // handle to FRAME on IMAGEAT dialog
            //pdi->rcIFrame   = rcFrame; // and its size
               rcNew.left   = ((rcClip.left    * rcFrame.right)
                     / pdi->di_dwDIBWidth);  // get CLIP
               rcNew.right  = ((rcClip.right   * rcFrame.right)
                     / pdi->di_dwDIBWidth);
               rcNew.top    = ((rcClip.top    * rcFrame.bottom)
                     / pdi->di_dwDIBHeight);
               rcNew.bottom = ((rcClip.bottom * rcFrame.bottom)
                     / pdi->di_dwDIBHeight);
               // make sure the NEW values FIT within the FRAME
               // and are differenct to previous
               if( ( rcNew.left >= rcFrame.left       ) &&
                   ( rcNew.right <= rcFrame.right     ) &&
                   ( rcNew.top   >= rcFrame.top       ) &&
                   ( rcNew.bottom <= rcFrame.bottom   ) &&
                   ( EqualRect( &rcNew, &rcOld ) == 0 ) )
               {
                  if( hdc = GetDC( pdi->hwndIFrame ) )
                  {
                     //LPSTR lpDIB;
                     //HGLOBAL  hDIB;
                     //if( ( hDIB = pdi->hDIB ) &&
                     //    ( lpDIB = DVGlobalLock(hDIB)   ) )
                     DWORD dwi;
                     {
                        DVPaintClip2(hdc, &rcOld, &pdi->rcClip2, &rcFrame );
                        DVPaintClip2(hdc, &rcNew, &rcClip, &rcFrame );
                        pdi->rcClip2 = rcClip;  // update CLIP size
                        pdi->rcClip3 = rcNew;   // and PAINT rect
                        SendMessage( GetDlgItem(hDlg,cmd),  // handle to destination window
                           EM_GETSEL,  // message to send
                           (WPARAM)&dwi,       // starting position (LPDWORD)
                           0 );        // ending position (LPDWORD)
                        Put_Info2_Clip(hDlg, &rcClip);   // fill in the NEW values
                        SendMessage( GetDlgItem(hDlg,cmd),  // handle to destination window
                           EM_SETSEL,                // message to send
                           (WPARAM)dwi,        // starting position
                           (LPARAM)dwi );      // ending position
                     //   DVGlobalUnlock(hDIB);
                     }
                     ReleaseDC( pdi->hwndIFrame, hdc );
                  }
               }
            }
            DVGlobalUnlock(hd);
         }
      }
   }
   return flg;
}

#ifndef  NDEBUG
extern   LPTSTR   DBGGetEdNote( UINT uiNote );

//IDD_IMAGEATT DIALOG DISCARDABLE  0, 0, 357, 141
//STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
//CAPTION "IMAGE ATTRIBUTES"
//FONT 8, "MS Sans Serif"
//BEGIN
typedef struct tagDBSTC {
   LPTSTR   d_pszType;
   LPTSTR   d_pszID;
   UINT     d_uiID;
   HWND     d_hWnd;
   LPTSTR   d_pszStg;
}DBSTC, * PDBSTC;

DBSTC sDbStc[] = {
   { "PUSHBUTTONDEF", "IDOK",      IDOK, 0, "OK" }, // ,IDOK,161,120,50,14
   { "PUSHBUTTON",    "IDCANCEL",  IDCANCEL, 0, "Cancel" },  //,IDCANCEL,98,120,50,14
   { "EDITTEXT",      "IDC_EDIT1", IDC_EDIT1, 0, "Edit1" },  //29,17,24,14,ES_AUTOHSCROLL
//    RTEXT           "W:",IDC_STATIC,14,19,10,8
//    RTEXT           "H: ",IDC_STATIC,61,20,10,8
   { "EDITTEXT",      "IDC_EDIT2", IDC_EDIT2, 0, "Edit2" },  //79,17,24,14,ES_AUTOHSCROLL
//    RTEXT           "W:",IDC_STATIC,128,20,10,8
//    RTEXT           "H: ",IDC_STATIC,177,20,10,8
   { "EDITTEXT",      "IDC_EDIT3", IDC_EDIT3, 0, "Edit3" },  //141,17,24,14,ES_AUTOHSCROLL | ES_READONLY | 
//                    NOT WS_TABSTOP
   { "EDITTEXT",      "IDC_EDIT4", IDC_EDIT4, 0, "Edit4" },  //191,17,24,14,ES_AUTOHSCROLL | ES_READONLY
//    RTEXT           "W:",IDC_STATIC,13,47,10,8
//    RTEXT           "H: ",IDC_STATIC,61,47,10,8
   { "EDITTEXT",      "IDC_EDIT5", IDC_EDIT5, 0, "Edit5" },  //30,44,24,14,ES_AUTOHSCROLL
   { "EDITTEXT",      "IDC_EDIT6", IDC_EDIT6, 0, "Edit6" },  //78,44,24,14,ES_AUTOHSCROLL
//    RTEXT           "x:",IDC_STATIC,129,47,8,8
   { "EDITTEXT",      "IDC_EDIT7", IDC_EDIT7, 0, "Edit7" },  //141,44,24,14,ES_AUTOHSCROLL
//    RTEXT           "y:",IDC_STATIC,177,47,9,8
   { "EDITTEXT",      "IDC_EDIT8", IDC_EDIT8, 0, "Edit8" },  // 191,44,24,14,ES_AUTOHSCROLL
//    RTEXT           "cx:",IDC_STATIC,126,66,13,8
   { "EDITTEXT",      "IDC_EDIT9", IDC_EDIT9, 0, "Edit9" },  //141,63,24,14,ES_AUTOHSCROLL
//    RTEXT           "cy:",IDC_STATIC,175,66,12,8
   { "EDITTEXT",      "IDC_EDIT10", IDC_EDIT10,0, "Edit10" }, //191,63,24,14,ES_AUTOHSCROLL
//    LTEXT           "Colors:",IDC_STATIC,7,81,22,8
//    RTEXT           "Maximum:",IDC_STATIC,26,66,35,8
   { "EDITTEXT",      "IDC_EDIT11", IDC_EDIT11,0, "Edit11" }, //32,78,36,14,ES_RIGHT | ES_AUTOHSCROLL | 
//                    ES_READONLY | NOT WS_BORDER | NOT WS_TABSTOP
//    RTEXT           "Count:",IDC_STATIC,75,66,25,8
   { "EDITTEXT",      "IDC_EDIT12", IDC_EDIT12,0, "Edit12" }, //77,78,36,14,ES_AUTOHSCROLL | ES_READONLY | 
//                    NOT WS_BORDER | NOT WS_TABSTOP
//    LTEXT           "Bits per Pixel:",IDC_STATIC,7,120,47,8
   { "EDITTEXT",      "IDC_EDIT13", IDC_EDIT13,0, "Edit13" }, //59,120,25,14,ES_READONLY | NOT WS_BORDER | 
//                    NOT WS_TABSTOP
//    GROUPBOX        "MDI Client",IDC_STATIC,7,7,107,27
//    GROUPBOX        "DIB Size",IDC_STATIC,122,7,105,27
//    GROUPBOX        "Clip Region Size",IDC_STATIC,7,35,107,27
//    GROUPBOX        "Current Clip Rectangle",IDC_STATIC,122,35,106,51
   { "EDITTEXT",      "IDC_EDIT14", IDC_EDIT14,0, "Edit14" }, //26,96,324,14,ES_AUTOHSCROLL | ES_READONLY //| 
//                    NOT WS_BORDER
//    LTEXT           "File:",IDC_STATIC,7,98,14,8
   { "CONTROL",      "IDC_FRAME", IDC_FRAME, 0, "Frame" },  //,SS_GRAYFRAME,234,7,116,84
//END
   { 0,               0,         0, 0       }
};

LPTSTR   GetCtlType( DWORD cmd )
{
   LPTSTR   lpc = 0;
   LPTSTR   lps;
   PDBSTC    pdb = &sDbStc[0];
   while( lps = pdb->d_pszType )
   {
      if( pdb->d_uiID == cmd )
      {
         lpc = lps;
         break;
      }
      pdb++;
   }
   return lpc;
}

LPTSTR   GetCmdStg( DWORD cmd )
{
   LPTSTR   lpc = 0;
   LPTSTR   lps;
   PDBSTC    pdb = &sDbStc[0];
   while( lps = pdb->d_pszType )
   {
      if( pdb->d_uiID == cmd )
      {
         lpc = pdb->d_pszID;
         break;
      }
      pdb++;
   }
   return lpc;
}

#endif   // #ifndef  NDEBUG

// Parameters
// wParam 
// The high-order word specifies the notification code,
//    if the message is from a control.
//    If the message is from an accelerator, this value is 1.
//    If the message is from a menu, this value is zero.
// The low-order word specifies the identifier (ID) of the menu item,
//    control, or accelerator.
// lParam
// Handle to the control sending the message
//    if the message is from a control.
//    Otherwise, this parameter is NULL.
// Item of interest
		// === Clip size ===
//		SetDlgItemInt( hDlg,	// handle of dialog box
//			IDC_EDIT5,		// identifier of control
//			(lpi->rcClip.right - lpi->rcClip.left), // value to set
//			FALSE );
//		SetDlgItemInt( hDlg,	// handle of dialog box
//			IDC_EDIT6,		// identifier of control
//			(lpi->rcClip.bottom - lpi->rcClip.top), // value to set
//			FALSE );
		// === Clip rectangle ===
		// **********************
//		SetDlgItemInt( hDlg,	// handle of dialog box
//			IDC_EDIT7,		// identifier of control
//			lpi->rcClip.left, // value to set
//			FALSE );
//		SetDlgItemInt( hDlg,	// handle of dialog box
//			IDC_EDIT8,		// identifier of control
//			lpi->rcClip.top, // value to set
//			FALSE );
//		SetDlgItemInt( hDlg,	// handle of dialog box
//			IDC_EDIT9,		// identifier of control
//			lpi->rcClip.right, // value to set
//			FALSE );
//		SetDlgItemInt( hDlg,	// handle of dialog box
//			IDC_EDIT10,		// identifier of control
//			lpi->rcClip.bottom, // value to set
//			FALSE );

BOOL  Do_Info2_Command( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
   BOOL  flg  = FALSE;
   DWORD cmd  = LOWORD(wParam);  // item ID (from RC or ...)
   DWORD code = HIWORD(wParam);  // notification code
   HWND  hWnd = (HWND)lParam;    // handle
#ifndef  NDEBUG
   LPTSTR   lps;
   if( lps = GetCtlType(cmd) )
   {
      if( *lps == 'E' )
      {
         sprtf( "EDIT-NOTE from h=%#x (%s) saying %s."MEOR,
            hWnd,
            GetCmdStg(cmd),
            DBGGetEdNote(code) );
      }
      else
      {
         sprtf( "NOTIFY from %s of %d."MEOR,
            GetCmdStg(cmd) );
      }
   }
   else
   {
      sprtf( "WM_COMMAND: cmd=%d code=%d hwnd=%#x."MEOR, cmd, code, hWnd);
   }
#endif   // !NDEBUG
   switch(cmd)
   {
   case IDOK:
	   Do_Info2_Close( hDlg );	// Close IDM_IMAGEATT (IDD_IMAGEATT)
	   EndDialog(hDlg, TRUE);
      flg = TRUE;
      break;

   case IDCANCEL:
	   EndDialog(hDlg, FALSE);
      flg = TRUE;
      break;

		// === Clip size ===
//		SetDlgItemInt( hDlg,	// handle of dialog box
   case IDC_EDIT5:		// identifier of control
//			(lpi->rcClip.right - lpi->rcClip.left), // value to set
//			FALSE );
//		SetDlgItemInt( hDlg,	// handle of dialog box
   case IDC_EDIT6:		// identifier of control
//			(lpi->rcClip.bottom - lpi->rcClip.top), // value to set
//			FALSE );
		// === Clip rectangle ===
		// **********************
//		SetDlgItemInt( hDlg,	// handle of dialog box
   case	IDC_EDIT7:		// identifier of control
//			lpi->rcClip.left, // value to set
//			FALSE );
//		SetDlgItemInt( hDlg,	// handle of dialog box
   case	IDC_EDIT8:		// identifier of control
//			lpi->rcClip.top, // value to set
//			FALSE );
//		SetDlgItemInt( hDlg,	// handle of dialog box
   case IDC_EDIT9:		// identifier of control
//			lpi->rcClip.right, // value to set
//			FALSE );
//		SetDlgItemInt( hDlg,	// handle of dialog box
   case	IDC_EDIT10:		// identifier of control
//			lpi->rcClip.bottom, // value to set
//			FALSE );
      flg = Do_Info2_Cmd( hDlg, cmd, code );
      break;
   }

   return flg;

}

//
// Called from IDM_IMAGEATT menu item (uses IDD_IMAGEATT template)
// INFO2DLGPROC, with (DWORD) hDIBInfo );
BOOL CALLBACK INFO2DLGPROC( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL	flg = FALSE;
#ifndef  NDEBUG
   if( ( message != WM_CTLCOLORSTATIC ) &&
       ( message != WM_NCHITTEST      ) &&
       ( message != WM_SETCURSOR      ) &&
       ( message != WM_MOUSEMOVE      ) &&
       ( message != WM_COMMAND        ) )
   {
      extern LPSTR GetWMStg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
      sprtf( "INFO2: h=%#x %s wP=%#x lP=%#x."MEOR, hDlg,
         GetWMStg( hDlg, message, wParam, lParam ),
         wParam, lParam );
   }
#endif   // #ifndef  NDEBUG
	switch( message )
	{

	case WM_INITDIALOG:	// init IDM_IMAGEATT (using IDD_IMAGEATT template)
		flg = Do_Info2_Init( hDlg, wParam, lParam );
      if( !flg )
         EndDialog(hDlg,-1);  // out of here
		break;
		
//#ifdef	WIN32
	case WM_NOTIFY:
		flg = Do_Info2_Note( hDlg, wParam, lParam );
		break;
//#endif	// WIN32

	case WM_COMMAND:
      flg = Do_Info2_Command( hDlg, wParam, lParam );
		break;

   case WM_PAINT:
      flg = Do_Info2_Paint( hDlg, TRUE );
      break;

   case WM_DESTROY:
      REMOVE_PROP(hDlg, ATOM_INFO);
      flg = TRUE;
      break;


	}

	return( flg );
}


#ifdef	ADDCOLRF

BOOL	fChgGBack = FALSE;
BOOL	fChgGShad = FALSE;
BOOL	fChgGHigh = FALSE;
BOOL	fChgGText = FALSE;

COLORREF	CustColors[16] = {0};

LPISTR	lpGCIStr;

void Set3Edits( HWND hDlg, COLORREF colr, UINT i1, UINT i2, UINT i3 )
{
	UINT	col;

	col = GetRValue( colr ) & 0xff;
	SetDlgItemInt( hDlg,	// handle of dialog box
		i1,			// identifier of control
		col,		// value to set
		FALSE );	// signed or unsigned indicator
	col = GetGValue( colr ) & 0xff;
	SetDlgItemInt( hDlg,	// handle of dialog box
		i2,			// identifier of control
		col,		// value to set
		FALSE );	// signed or unsigned indicator
	col = GetBValue( colr ) & 0xff;
	SetDlgItemInt( hDlg,	// handle of dialog box
		i3,			// identifier of control
		col,		// value to set
		FALSE );	// signed or unsigned indicator
}

COLORREF Get3Edits( HWND hDlg, COLORREF colr, UINT i1, UINT i2, UINT i3 )
{
	BOOL	flg;
	COLORREF	ncolr;
	UINT	col;
	BYTE	br, bg, bb;

	ncolr = 0;
	col = GetDlgItemInt( hDlg,	// handle to dialog box
		i1,			// control identifier
		&flg,		// points to variable to receive success/failure indicator
		FALSE );	// specifies whether value is signed or unsigned
	if( flg && (col < 256) )
		br = (BYTE)col;
	else
		br = GetRValue( colr ) & 0xff;

	col = GetDlgItemInt( hDlg,	// handle to dialog box
		i2,			// control identifier
		&flg,		// points to variable to receive success/failure indicator
		FALSE );	// specifies whether value is signed or unsigned
	if( flg && (col < 256) )
		bg = (BYTE)col;
	else
		bg = GetRValue( colr ) & 0xff;

	col = GetDlgItemInt( hDlg,	// handle to dialog box
		i3,			// control identifier
		&flg,		// points to variable to receive success/failure indicator
		FALSE );	// specifies whether value is signed or unsigned
	if( flg && (col < 256) )
		bb = (BYTE)col;
	else
		bb = GetRValue( colr ) & 0xff;

	ncolr = RGB( br, bg, bb );

	return ncolr;

}

void SetFace( HWND hDlg, COLORREF colr )
{
	Set3Edits( hDlg, colr, IDC_EDIT1, IDC_EDIT2, IDC_EDIT3 );
}
void SetShadow( HWND hDlg, COLORREF colr )
{
	Set3Edits( hDlg, colr, IDC_EDIT4, IDC_EDIT5, IDC_EDIT6 );
}
void SetHiLite( HWND hDlg, COLORREF colr )
{
	Set3Edits( hDlg, colr, IDC_EDIT7, IDC_EDIT8, IDC_EDIT9 );
}
void SetText( HWND hDlg, COLORREF colr )
{
	Set3Edits( hDlg, colr, IDC_EDIT10, IDC_EDIT11, IDC_EDIT12 );
}

COLORREF GetFace( HWND hDlg, COLORREF colr )
{
	return( Get3Edits( hDlg, colr, IDC_EDIT1, IDC_EDIT2, IDC_EDIT3 ) );
}
COLORREF GetShadow( HWND hDlg, COLORREF colr )
{
	return( Get3Edits( hDlg, colr, IDC_EDIT4, IDC_EDIT5, IDC_EDIT6 ) );
}
COLORREF GetHiLite( HWND hDlg, COLORREF colr )
{
	return( Get3Edits( hDlg, colr, IDC_EDIT7, IDC_EDIT8, IDC_EDIT9 ) );
}
COLORREF GetText( HWND hDlg, COLORREF colr )
{
	return( Get3Edits( hDlg, colr, IDC_EDIT10, IDC_EDIT11, IDC_EDIT12 ) );
}

BOOL Do_GC_Init( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL	flg = FALSE;
	LPISTR	lpi;
	LPDIBX	lpDIBx;
//	char	szBuffer[20];

	lpGCIStr = 0;
	if( lpi = (LPISTR)lParam )
	{
		if( ( lpi->hDIBx ) &&	// = GetWindowExtra( lpi->hMDI, WW_DIB_EX2 )
			( lpDIBx = (LPDIBX)DVGlobalLock( lpi->hDIBx ) ) )
		{
			lpGCIStr = lpi;
			SetFace( hDlg, lpDIBx->dx_Face );
			SetShadow( hDlg, lpDIBx->dx_Shadow );
			SetHiLite( hDlg, lpDIBx->dx_HiLite );
			SetText( hDlg, lpDIBx->dx_Text );
			CheckDlgButton( hDlg, IDC_CHECK1,
				(fUdtgDIBx ? BST_CHECKED : BST_UNCHECKED) );

			DVGlobalUnlock( lpi->hDIBx );
		}
		else
		{
			flg = -1;
		}
	}
	else
	{
		flg = -1;
	}
	return flg;
}

BOOL Do_GC_Close( HWND hDlg )
{
	BOOL	flg = FALSE;
	LPISTR	lpi;
	LPDIBX	lpDIBx;
	COLORREF	nface, nshad, nhi, ntxt;
	UINT	ui;

	nface = nshad = nhi = ntxt = 0;
	if( ( lpi = lpGCIStr ) &&
		( lpi->hDIBx ) &&	// = GetWindowExtra( lpi->hMDI, WW_DIB_EX2 )
		( lpDIBx = (LPDIBX)DVGlobalLock( lpi->hDIBx ) ) )
	{
		nface = GetFace( hDlg, lpDIBx->dx_Face );
		nshad = GetShadow( hDlg, lpDIBx->dx_Shadow );
		nhi   = GetHiLite( hDlg, lpDIBx->dx_HiLite );
		ntxt  = GetText( hDlg, lpDIBx->dx_Text );
		if( ( nface != lpDIBx->dx_Face ) ||
			( nshad != lpDIBx->dx_Shadow ) ||
			( nhi   != lpDIBx->dx_HiLite ) ||
			( ntxt  != lpDIBx->dx_Text ) )
		{
			lpDIBx->dx_Face = nface;
			lpDIBx->dx_Shadow = nshad;
			lpDIBx->dx_HiLite = nhi;
			lpDIBx->dx_Text = ntxt;
			flg = TRUE;
			lpi->fChanged = TRUE;
		}

		ui = IsDlgButtonChecked( hDlg,	// handle of dialog box
			IDC_CHECK1 );	// button identifier
		if( ui == BST_CHECKED )
			fUdtgDIBx = TRUE;
		else
			fUdtgDIBx = FALSE;
 
		DVGlobalUnlock( lpi->hDIBx );
	}
	return flg;
}

//typedef struct {   // cc  
//    DWORD        lStructSize; 
//    HWND         hwndOwner; 
//    HWND         hInstance; 
//    COLORREF     rgbResult; 
//    COLORREF*    lpCustColors; 
//    DWORD        Flags; 
//    LPARAM       lCustData; 
//    LPCCHOOKPROC lpfnHook; 
//    LPCTSTR      lpTemplateName; 
//} CHOOSECOLOR; 

BOOL	Do_GC_Button( HWND hDlg, WPARAM wParam )
{
	BOOL	flg = FALSE;
	LPISTR	lpi;
	LPDIBX	lpDIBx;
	CHOOSECOLOR	cc;
	COLORREF	colr, ncolr;
	BYTE	cr, cg, cb;
	BYTE	ncr, ncg, ncb;
   DWORD cmd = LOWORD(wParam);

	if( ( lpi = lpGCIStr ) &&
		( lpi->hDIBx ) &&	// = GetWindowExtra( lpi->hMDI, WW_DIB_EX2 )
		( lpDIBx = (LPDIBX)DVGlobalLock( lpi->hDIBx ) ) )
	{
		if( cmd == IDC_BUTTON1 )
			colr = lpDIBx->dx_Face;
		else if( cmd == IDC_BUTTON2 )
			colr = lpDIBx->dx_Shadow;
		else if( cmd == IDC_BUTTON3 )
			colr = lpDIBx->dx_HiLite;
		else if( cmd == IDC_BUTTON4 )
			colr = lpDIBx->dx_Text;
		cr = GetRValue( colr );
		cg = GetGValue( colr );
		cb = GetBValue( colr );

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
			if( (cr != ncr ) ||
				(cg != ncg ) ||
				(cb != ncb ) )
			{
				ncolr = RGB( ncr, ncg, ncb );
				if( cmd == IDC_BUTTON1 )
					SetFace( hDlg, ncolr );
				else if( cmd == IDC_BUTTON2 )
					SetShadow( hDlg, ncolr );
				else if( cmd == IDC_BUTTON3 )
					SetHiLite( hDlg, ncolr );
				else if( cmd == IDC_BUTTON4 )
					SetText( hDlg, ncolr );
				flg = TRUE;
			}
		}
		DVGlobalUnlock( lpi->hDIBx );
	}
	return flg;
}

BOOL CALLBACK GETCOLORDLGPROC( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL	flg = FALSE;
   DWORD cmd = LOWORD(wParam);
	switch( message )
	{

	case WM_INITDIALOG:
		flg = Do_GC_Init( hDlg, wParam, lParam );
		break;
		
	case WM_COMMAND:
		if( (cmd == IDOK) || (cmd == IDCANCEL) )
		{
			if( cmd == IDOK )
				flg = Do_GC_Close( hDlg );
			EndDialog(hDlg, flg );
		}
		else if( (cmd == IDC_BUTTON1) ||
			(cmd == IDC_BUTTON2) ||
			(cmd == IDC_BUTTON3) ||
			(cmd == IDC_BUTTON4) )
		{
			flg = Do_GC_Button( hDlg, wParam );
		}
		break;
	}
	return( flg );
}


// case IDM_BKCOLOR:
void DvBkColor( HWND hWnd )
{
	PIS      lpi;
	LPSTR	   lpb, lpbits;
#ifndef	WIN32
	FARPROC	lpInfo;	// NOT required in WIN32!!!
#endif
	HDC		hDC;
	DWORD	   dwSize;
	int		i;
	LPDIBX	lpDIBx;
   HWND     hMDI;
   HGLOBAL  hDIBInfo;
   PDI      lpDIBInfo;
   HANDLE   hDIB;
#ifndef   USEITHREAD
	ISTR	   IStr;
#endif   // ifndef   USEITHREAD

	hDIBInfo = 0;
	lpDIBInfo = 0;
	hDC = 0;
	if( ( hMDI = GetCurrentMDIWnd() ) &&
		( hDIBInfo = GetWindowExtra( hMDI, WW_DIB_HINFO ) ) &&
		( lpDIBInfo = (LPDIBINFO) DVGlobalLock( hDIBInfo ) ) &&
		( hDIB = lpDIBInfo->hDIB ) )
	{
#ifdef   USEITHREAD
      lpi = &lpDIBInfo->di_sIS;
#else // !USEITHREAD
   	lpi = &IStr;
	   NULPISTR( lpi );
      lpi->lpDIBInfo = lpDIBInfo;
#endif   // ifdef   USEITHREAD y/n
   	lpi->fChanged  = FALSE;
	   lpi->dwFlag    = 0;
      lpi->hMDI      = hMDI;
      lpi->hDIBInfo  = hDIBInfo;
      lpi->hDIB      = hDIB;
// FIX980430 ( GetObject( lpi->hBitmap, sizeof(BITMAP), &lpi->bm ) ) )
// FIX980504 - the BITMAP may not exist in case of BIG DIBs
		lpi->is_hBitmap = lpDIBInfo->hBitmap;
		lpi->hDIBx = GetWindowExtra( lpi->hMDI, WW_DIB_EX2 );
//		lpi->cxDIB = lpDIBInfo->di_dwDIBWidth;
//		lpi->cyDIB = lpDIBInfo->di_dwDIBHeight;
		if( lpb = DVGlobalLock( hDIB ) )  // LOCK DIB HANDLE
		{
			lpbits          = FindDIBBits( lpb );
			lpi->iColors    = DIBNumColors( lpb );
			lpi->iCalcCols  = CalcDIBColors( lpb );
			if( ( lpDIBInfo->di_dwDIBBits ) &&
				( dwSize = 
				(DWORD)( (WIDTHBYTES( lpDIBInfo->di_dwDIBWidth * lpDIBInfo->di_dwDIBBits )) *
				lpDIBInfo->di_dwDIBHeight ) ) )
			{
				lpi->iUsedCols = GetUCCnt( lpDIBInfo,
					lpbits, lpi->iColors,
					dwSize,
               lpDIBInfo->di_dwDIBWidth,
               lpDIBInfo->di_dwDIBHeight,
					lpDIBInfo->di_dwDIBBits,
               lpb );
			}
			DVGlobalUnlock( hDIB );  // UNLOCK DIB HANDLE
		}
		else
		{
			lpi->iColors = 0;
		}
		GetClientRect( lpi->hMDI, &lpi->rcClient );
		lpi->rcClip.left   = lpDIBInfo->rcClip.left;
		lpi->rcClip.top    = lpDIBInfo->rcClip.top;
		lpi->rcClip.right  = lpDIBInfo->rcClip.right;
		lpi->rcClip.bottom = lpDIBInfo->rcClip.bottom;
		if( IsRectEmpty( &lpi->rcClip ) )
		{
			lpi->rcClip.left   = 0;
			lpi->rcClip.top    = 0;
			lpi->rcClip.right  = lpDIBInfo->di_dwDIBWidth;
			lpi->rcClip.bottom = lpDIBInfo->di_dwDIBHeight;
		}
#ifdef	WIN32
#ifdef   USEITHREAD
		i = DialogBoxParam( ghDvInst,
			MAKEINTRESOURCE( IDD_GETCOLORS ),
			lpi->hMDI,
			(DLGPROC)GETCOLORDLGPROC,	// lpInfo,
			(LPARAM) hDIBInfo );
#else // !USEITHREAD
		i = DialogBoxParam( ghDvInst,
			MAKEINTRESOURCE( IDD_GETCOLORS ),
			lpi->hMDI,
			GETCOLORDLGPROC,	// lpInfo,
			(DWORD) lpi );
#endif   // ifdef   USEITHREAD y/n
#else	/* !WIN32 */
		lpInfo = MakeProcInstance( GETCOLORDLGPROC, ghDvInst );
		i = DialogBoxParam( ghDvInst,
			MAKEINTRESOURCE( IDD_GETCOLORS ),
			lpi->hMDI,
			lpInfo,
			(DWORD) lpi );
		FreeProcInstance( lpInfo );
#endif	/* WIn32 y/n */

		if( lpi->fChanged )	// Something is changed
		{
			InvalidateRect( hMDI, NULL, TRUE );
//extern	BOOL	fUdtgDIBx;
			if( fUdtgDIBx )
			{
				if( lpi->hDIBx &&	// = GetWindowExtra( lpi->hMDI, WW_DIB_EX2 );
					( lpDIBx = (LPDIBX)DVGlobalLock( lpi->hDIBx ) ) )
				{
					LPDIBX	lpg;
//extern	DIBX	gDIBx;
					lpg = &gDIBx;
					if( ( lpg->dx_Face != lpDIBx->dx_Face  ) ||
						( lpg->dx_Shadow != lpDIBx->dx_Shadow  ) ||
						( lpg->dx_HiLite != lpDIBx->dx_HiLite ) ||
						( lpg->dx_Text != lpDIBx->dx_Text ) )
					{
						if( lpg->dx_Face != lpDIBx->dx_Face )
						{
							fChgGBack = TRUE;
							lpg->dx_Face = lpDIBx->dx_Face;
						}
						if( lpg->dx_Shadow != lpDIBx->dx_Shadow )
						{
							fChgGShad = TRUE;
							lpg->dx_Shadow = lpDIBx->dx_Shadow;
						}
						if( lpg->dx_HiLite != lpDIBx->dx_HiLite )
						{
							fChgGHigh = TRUE;
							lpg->dx_HiLite = lpDIBx->dx_HiLite;
						}
						if( lpg->dx_Text != lpDIBx->dx_Text )
						{
							fChgGText = TRUE;
							lpg->dx_Text = lpDIBx->dx_Text;
						}
//extern	BOOL	fChgUDIBx;
						fChgUDIBx = TRUE;
					}
					DVGlobalUnlock( lpi->hDIBx );
				}
			}
		}
	}

	if( hDIBInfo && lpDIBInfo )
		DVGlobalUnlock( hDIBInfo );

}

// case IDM_DEFFONT:
void DvDefFont( HWND hWnd )
{
	MessageBox( hWnd, "Function not yet implemented!",
		"NOT IMPLEMENTED", MB_OK | MB_ICONINFORMATION );
}

#endif	// ADDCOLRF

//extern	char	gszDiag[];
void	OutColours( LPRGBQUAD lpq, int iCnt )
{
	int		i, j, k;
	BYTE	r,g,b;
	LPRGBQUAD lpqd;
	LPSTR	lpd = &gszDiag[0];
	//OutColours( lpq, 16 );
	j = k = 0;
	wsprintf( lpd,
		"COLOUR TABLE - Count of RGBQUADS = %d (%d Bytes)"MEOR,
		iCnt,
		(iCnt * sizeof(RGBQUAD)) );
	DO(lpd);
	lstrcpy( lpd,
		"Ind.  Colour     Ind.  Colour     Ind.  Colour     Ind.  Colour"MEOR );
	DO(lpd);

	*lpd = 0;
	for( i = 0; i < iCnt; i++ )
	{
		lpqd = &lpq[i];
		r = lpq[i].rgbRed;
		g = lpq[i].rgbGreen;
		b = lpq[i].rgbBlue;
		wsprintf( EndBuf(lpd),
			"%3d(%3d,%3d,%3d) ",
			i,
			r,g,b );
		j++;
		if( j >= 4 )
		{
			lstrcat(lpd, MEOR);
			DO(lpd);
			*lpd = 0;
			j = 0;
		}
	}
	if( *lpd )
	{
			lstrcat(lpd, MEOR);
			DO(lpd);
			*lpd = 0;
	}
	lstrcpy( lpd,
		"Ind.  Colour     Ind.  Colour     Ind.  Colour     Ind.  Colour"MEOR );
	DO(lpd);

}

//int		gfSetRank = 1;
// =====================================================
// Manage the COLOR array
//void	DoSORT( _P )
void		DoSORT( PPI lpPalInfo )
{
	HGLOBAL		hgc, hgs;
	LPSTR		   lpbc, lpbs;
	DWORD		   dwi, dwk, dwCnt, sort;
	LPSTR		   lpd = &gszDiag[0];

	hgc = hgs = 0;
	lpbc = lpbs = 0;
	sort = 0;
	if( ( hgc = lpPalInfo->pi_hCopyCOLR ) &&
		 ( hgs = lpPalInfo->pi_hSortCOLR ) &&
		 ( lpbc= DVGlobalLock(hgc)       ) &&
		 ( lpbs= DVGlobalLock(hgs)       ) &&
		 ( dwCnt = GetCOLRCnt(lpbc)      ) &&
		 ( dwCnt == GetCOLRCnt(lpbs)     ) )
	{
		DWORD		   dwSize, dwf, dwcr, dwr;
		BYTE		   r,g,b;
		DLPPALEX	   lppx;
		DLPCOLR		lpcr;
		LPSTR		   lpstg;
      LPCOLR      lpc2;
		if( ( lppx = (DLPPALEX)lpbs       ) &&
			 ( dwSize = lppx->px_Size      ) &&
			 ( dwCnt  == lppx->px_Count    ) &&
			 ( dwSize == PALEXSIZE(dwCnt)  ) )
		{
			HGLOBAL	hg = 0;
			LPSTR	   lpb = 0;
			float	   lum, lmx, lmi;
			DWORD	   dws = 0;
			DWORD	   dwj;

			dwk = dwCnt;   // fix dwk to CONSTANT size of array

         // NOTE: COPY the "copy" eaxctly to the "sort"
			memcpy( lpbs, lpbc, dwSize ); // if NO flags, then all is done

//			SetgszStatus( lpPalInfo->pi_dwCnt );
			lpstg = SetgszStatus( dwCnt );

			/* done sort of none = NORMAL per BITMAP Order */
			if( GotFlags( pP ) )	/* any ORDER FLAGS? */
			{
				LPCLRI	lpci;
				/* ok, set sort type */
				if( IsFlagIntensity( pP ) )
				{
					float	    plum;
					COLORREF	pcr;

					// sort per "intensity"
					sort = 1;
               strcat(lpstg, "per Intensity ");
					dws = ( sizeof(CLRI) * ( dwCnt + 1 ) );
//					if( ( hg = DVGlobalAlloc( GHND, dws ) ) &&
					if( ( hg = DVGAlloc( "DoSORT", GHND, dws ) ) &&
						 ( lpb = DVGlobalLock(hg)          ) )
					{
						// luminance=30% red,60% green,10% blue.
						lpci = (LPCLRI)lpb;  // set this NEW array of "intensity"
						lmx = 0.0f;
						lmi = ( 256 * 0.3f ) + ( 256 * 0.6f ) +
							( 256 * 0.1f );

                  // lppx is pointing to the "sort" list which
                  // at this time is an exactly COPY of the "copy" structure
                  // =======================================================
						for( dwi = 0; dwi < dwCnt; dwi++ )
						{
                     // get the intensity
//						double *	lpd;
							lpcr = &lppx->px_Colrs[dwi];  // get pointer to color array
							//dwf  = (DWORD)LOWORD(lpcr->cr_Freq);
							//dwr  = (DWORD)HIWORD(lpcr->cr_Freq);
							dwcr = lpcr->cr_Color;  // extract color
							r = GetRValue(dwcr);
							g = GetGValue(dwcr);
							b = GetBValue(dwcr);
							lum = ( r * 0.3f ) +
								   ( g * 0.6f ) +
								   ( b * 0.1f );
							lpci[dwi].ci_Colr   = dwcr;   // put it into this array
							lpci[dwi].ci_Lumina = (double)lum;  // with its intensisy

							if( lum > lmx )
								lmx = lum;
							if( lum < lmi )
								lmi = lum;
							if( ( dwi        ) &&
								 ( lum < plum ) )
							{
                        // and insert the "intensity" in its correct position
                        // ==================================================
								for( dwj = 0; dwj < dwi; dwj++ )
								{
									if( lum < lpci[dwj].ci_Lumina )
									{
										// extract the current greater
										pcr  = lpci[dwj].ci_Colr;
										plum = (float)lpci[dwj].ci_Lumina;

                              // inser the lesser here
										lpci[dwj].ci_Colr   = dwcr;
										lpci[dwj].ci_Lumina = (double)lum;

                              // and keep the extract color and luminance
										dwcr = pcr;
										lum  = plum;
									}
								}
							}

							lpci[dwi].ci_Colr   = dwcr;
							lpci[dwi].ci_Lumina = (double)lum;

//							wsprintf( lpd,
//								"Color (%3d,%3d,%3d) has luminance of %s.",
//								r, g, b,
//								Float2Str(lum) );
//							DO(lpd);

							plum = lum;
							pcr  = dwcr;

						}	/* cycle getting luminance */

                  // NOW order the "sort" array per the "lum" array, already "ordered"
						for( dwi = 0; dwi < dwCnt; dwi++ )
						{
							dwcr = lpci[dwi].ci_Colr;  // extract color from LUM array
#ifdef   DBGLUM2
							lum  = (float)lpci[dwi].ci_Lumina;
							r = GetRValue(dwcr);
							g = GetGValue(dwcr);
							b = GetBValue(dwcr);
							wsprintf( lpd,
								"Color (%3d,%3d,%3d) has luminance of %s.",
								r, g, b,
								Float2Str(lum) );
							DO(lpd);
#endif   // #ifdef   DBGLUM2
      					lpcr = &lppx->px_Colrs[dwi];           // get this pointer
                     dwf  = lpcr->cr_Freq;    // extract low values
                     dwr  = lpcr->cr_Color;
                     for( dwj = 0; dwj < dwCnt; dwj++ )
                     {
                        lpc2 = &lppx->px_Colrs[dwj]; // and this pointer
                        if( lpc2->cr_Color == dwcr )
                           break;
                     }
                     if( dwj < dwCnt )
                     {
                        if( lpcr != lpc2 )
                        {
                           lpcr->cr_Freq  = lpc2->cr_Freq;  // put high values into
                           lpcr->cr_Color = lpc2->cr_Color; // the low
                           lpc2->cr_Freq  = dwf;   // put low values into 
                           lpc2->cr_Color = dwr;   // the high
                        }
                     }
                     else
                     {
      						chkme( "YUK! Color %#x NOT FOUND in sort array!!!", dwcr );
                     }
						}
					}
					else
					{
						chkit( "YEEK! This MEMORY QUESTION IS A BUG!" );
					}
				}
				else if( IsFlagRainbow( pP ) )
				{
					// sort per rainbow colours
					sort = 2;
               strcat(lpstg, "per Rainbow ");
               DoRainbowSort( (PPX)lpbs, (PPX)lpbc, dwCnt ); // if NO flags, then all is done
				}
				else if( IsFlagNumeric( pP ) )
				{
					// just numeric order
					sort = 3;
               strcat(lpstg, "per Numeric ");
               dwCnt = dwk - 1;
               while( sort )
               {
                  sort = 0;   // kill the "sort" flag
                  for( dwi = 0; dwi < dwCnt; dwi++ )
                  {
                     lpcr = &lppx->px_Colrs[dwi];           // get this pointer
                     dwj  = dwi + 1;
                     lpc2 = &lppx->px_Colrs[dwj];
                     dwf  = lpcr->cr_Freq;    // extract low values
                     dwr  = lpcr->cr_Color;
                     if( dwr > lpc2->cr_Color ) // if this GREATER than next
                     {
                        // SWAP EM
                        lpcr->cr_Freq  = lpc2->cr_Freq;  // put high values into
                        lpcr->cr_Color = lpc2->cr_Color; // the low
                        lpc2->cr_Freq  = dwf;   // put low values into 
                        lpc2->cr_Color = dwr;   // the high
                        sort++;  // we have had a change
                     }
                  }
               }
               sort = 3;
				}
            else if( IsFlagGray( pP ) )
            {
               sort = 4;
               strcat(lpstg, "per Grayness ");
               DoGraySort( (PPX)lpbs, (PPX)lpbc, dwCnt ); // if NO flags, then all is done
            }
            else if( IsFlagFreq( pP ) )
            {
               sort = 5;
               strcat(lpstg, "per Frequency ");
               DoFreqSort( (PPX)lpbs, (PPX)lpbc, dwCnt );
            }
				else
				{
					chkit( "YEEK! No flag designed YET!!!" );
					dwCnt = 0;
				}
			}
			else
			{
				lstrcat( lpstg, "per BITMAP Order " );
			}

			if( lpPalInfo->pi_iInvert )
			{
				dwCnt = dwk / 2;  // get HALF the array
				for( dwi = 0; dwi < dwCnt; dwi++ )
				{
		/* in this array the COLOUR and FREQUENCY are in */
		/* a structured array                            */
		/* 1 - Index to this item                        */
		/* 2 - extract the frequency (or *) from here    */
					lpcr = &lppx->px_Colrs[dwi];           // set low pointer
               lpc2 = &lppx->px_Colrs[dwk - dwi - 1]; // and high pointer
               dwf = lpcr->cr_Freq;    // extract low values
               dwr = lpcr->cr_Color;
               lpcr->cr_Freq  = lpc2->cr_Freq;  // put high values into
               lpcr->cr_Color = lpc2->cr_Color; // the low
               lpc2->cr_Freq  = dwf;   // put low values into 
               lpc2->cr_Color = dwr;   // the high
				}  // for HALF the array

				lstrcat( lpstg, "(inverted)" );
			}

			if( hg && lpb )
				DVGlobalUnlock(hg);
			if( hg )
				DVGlobalFree(hg);

		}
		else
		{
			chkit( "Yick! This was just a SANITY CHECK!!!" );
		}

	}
	else
	{
		chkit( "Whey! We should be able to ENTER the sorter???" );
	}

	if( hgc && lpbc )
		DVGlobalUnlock(hgc);
	if( hgs && lpbs )
		DVGlobalUnlock(hgs);

}


COLORREF	GetRYBColr( DWORD index )
{
	DWORD	dwi;
	dwi = index;
	if( dwi >= 12 )
		dwi = 11;
	return( crClrSet[dwi].cs_Colr );
}

// =====================================================

#ifdef   USEITHREAD
// CreateThread
// like int GetUCCnt( LPDIBINFO lpDIBInfo,
//					 LPSTR lpb, int Cols, DWORD Size,
//					 int Wid, int Height,
//					 WORD wBPP, LPSTR lpDIB );
#define  bThdExit lpDIBInfo->di_bThdExit
#define  bEndThrd ( gbInClose || bThdExit )


DWORD	ThdCount24RGB( DWORD Height, DWORD Wid, DWORD Width,
				 PBYTE lpIn, LPDWORD lpdw, DWORD crmx, PDI lpDIBInfo )
{
	DWORD	   i,j,k,m, kr;
	BYTE	   r,g,b;
	COLORREF	cr;
	PBYTE	   lps = lpIn;
   DWORD    dwc, dwc1, dwc2;
   DWORD    dwor;

	kr = k = 0;
   dwc1 = dwc2 = dwc = 0;
   dwor = 0;
	if( gfBmpOrder )
	{
		i = Height;
		while( i-- )		/* for EACH row, from bottom UP */
		{
         if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
            goto Thd_Done;

			m = i * Width;	/* get offset to this ROW       */
			lps = &lpIn[m];	/* and get POINTER              */
         // for the PIXEL width, get three byte RGB values
				for( j = 0; j < Wid; j++ )
				{
               if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                  goto Thd_Done;

					// For each COLUMN get the THREE(3) RGB bytes
					r = *lps++;
					g = *lps++;
					b = *lps++;
					cr = RGB(r,g,b);  // form a COLORREF (DWORD)
					if( (DWORD)cr < (DWORD)crmx )
					{
                  dwc = lpdw[cr];   // extract current COUNT
						//if( lpdw[cr] == 0 )  // if the FIRST count
						if( dwc == 0 )  // if the FIRST count
						{
							k++;	// Count a NEW color
						}
                  //lpdw[cr]++;
                  //dwc = lpdw[cr];   // extract current COUNT
                  dwc++;   // bump the COUNT of this color
                  if( dwc == 0 ) // IF IT ROLLS OVER!!!
                  {
                     dwc--;      // stay at MAX
                     dwor++;     // and count an OVERRUN
                  }

                  lpdw[cr] = dwc;   // put the NEW count back
					}
					else
					{
						/* *** FAILED *** */
						chkme( "Memory not large enough! FAILED!!!" );
//						DIBError( ERR_MEMORY );
						j = Wid;
                  i = 0;
						break;
					}

				}  // for EACH pixel get THREE(3) BYTES

//				m = j * 3;
//				while( m++ < Width )
//					lps++;
//				m = Height - ( i + 1 );
//				if( m )
//				{
//					// add percentage after each ROW
//					//#define	ProgP(p)			SetProgP(p)
//					ProgP( (int)(( m * 100 ) / Height ) );
//				}
		}
	}
	else
	{
		/* ok, for each row, march across the columns */
		/* remember to roll past any padding at the end of a row */
		/* ===================================================== */
			for( i = 0; i < Height; i++ )
			{
            if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
               goto Thd_Done;
				// For each ROW
				for( j = 0; j < Wid; j++ )
				{
               if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                  goto Thd_Done;
					// For each COLUMN get the THREE bytes
					r = *lps++;
					g = *lps++;
					b = *lps++;
					cr = RGB(r,g,b);  // form a COLORREF (DWORD)
					if( (DWORD)cr < (DWORD)crmx )
					{
						if( lpdw[cr] == 0 )  // if this OFFSET is ZERO
							k++;	// Count a NEW color
						//lpdw[cr]++;
                  dwc = lpdw[cr];
                  dwc++;
                  if( dwc == 0 )
                  {
                     dwc--;
                     dwor++;
                  }
                  lpdw[cr] = dwc;
					}
					else
					{
						// FAILED
						chkme( "Memory not large enough! FAILED!!!" );
						//DIBError( ERR_MEMORY );
						j = Wid;
                  i = Height;
						break;
					}
				}
				m = j * 3;
				while( m++ < Width )
					lps++;
//				if( i )
//				{
//					// add percentage after each ROW
//					//#define	ProgP(p)			SetProgP(p)
//					ProgP( (int)(( i * 100 ) / Height ) );
//				}
			}

	}

   kr = k;
   if( dwor )
   {
      sprtf( "WARNING: Had %u overruns of WORD counter!"MEOR, dwor );
   }

Thd_Done:

   k = 0;

	return kr;

}  // end ThdCount24RGB

DWORD ThdCount256( PDI lpDIBInfo,  DWORD Height, DWORD Width, DWORD Wid,
                     PBYTE pbits, PBYTE lpBuf, DWORD crmx, PBYTE lpCOLR )
{
   DWORD       dwcnt = 0;
	LPRGBQUAD	lpq;
	LPPALEX	   lppxs;
	LPCOLR	   lpcrs;
   PBYTE       lps;
   DWORD       i, j, k, l, w;
   BYTE        r;
   LPDWORD     lpdw;

	//DWORD	dwi;
	/* 1 byte or 256 colours */
   //	Width = WIDTHBYTES( Wid * wBPP );
   // use lps to get to end of color blcok
	lps   = lpBuf + crmx;
	lppxs = (LPPALEX)lps;
   // but return lps to point at the actual DIB bits
   lps   = pbits;
	lpcrs = &lppxs->px_Colrs[0];
	lpq   = (LPRGBQUAD)lpCOLR;
   k     = 0;
	for( i = 0; i < Height; i++ )
	{
      // For each ROW, extract each index byte
      w = 0;
		for( j = 0; j < Width; j++ )
		{
         if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
            goto C256_Done;
         if( w < Wid )
         {
			   /* extract byte - actually index into 256 byte color table */
			   r = *lps++;    // extract this INDEX into the color table
			   /* do not repeat an index byte */
			   for( l = 0; l < k; l++ )
			   {
				   if( lpBuf[l] == r )
					   break;
			   }
			   if( l < k )
			   {
               // already have this index
				   lpcrs[l].cr_Freq += 1;  // bump count
			   }
			   else
			   {
				   /* else a NEW index keep the byte */
				   lpBuf[k] = r;
               // index into the COLOR table
				   lpdw = (LPDWORD) (LPRGBQUAD) &lpq[r];
               // extract the particular COLORREF
				   lpcrs[k].cr_Color = RGB( lpq[r].rgbRed,
					   lpq[r].rgbGreen, lpq[r].rgbBlue );
               // and start the FREQUENCY of this COLOR
				   lpcrs[k].cr_Freq  = 1;  // start frequency count for this color
				   k++;  // new color found
			   }
         }
         else
         {
            // got all the pixels in this row
            break;
         }
		}  // for each BYTE in a ROW

      // skip any padding
		while( j++ < Width )
			lps++;

	}  // for each ROW

   dwcnt = k;

C256_Done:

   lps = 0;

   return dwcnt;

}  // end ThdCount256



DWORD ThdCount4BPP(  PDI lpDIBInfo,  DWORD Height, DWORD Width, DWORD Wid,
                     PBYTE pbits, PBYTE lpBuf, DWORD crmx, PBYTE lpCOLR )
{
   DWORD    Wi;
   PBYTE    lps;
   LPPALEX  lppxs;
	LPCOLR	lpcrs;
   DWORD    i;
   DWORD    j, k, l;
   BYTE     r, b, g;
   BYTE     br,bg,bb;
   LPRGBQUAD   lpq;
   LPDWORD     lpdw;

   k = 0;
//		else if( wBPP == 4 )
		{
         if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
            goto Cnt_Done;
//			Width = WIDTHBYTES( Wid * wBPP );
         // use lps to get pointer after the COLOR block
			lps = lpBuf + crmx;
			lppxs = (LPPALEX)lps;
			lpcrs = &lppxs->px_Colrs[0];
         // but return lps to the actual BITS
         lps = pbits;
			/* are we pointing at the bitmap COLOR TABLE? */
			lpq   = (LPRGBQUAD)lpCOLR;
			OutColours( lpq, 16 );
			for( i = 0; i < Height; i++ )
			{
            if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
               goto Cnt_Done;
            // For each ROW plough the BYTE width
            Wi = 0;  // but do NOT overrun the PIXEL count
				for( j = 0; j < Width; j++ )
				{
               if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                  goto Cnt_Done;
               // For each COLUMN
               if( Wi < Wid ) // if we still have pixels to fetch
               {
   					r = *lps++;		// Get TWO nibbles
	   				b = r & 0x0f;	// Split it
		   			g = (r & 0xf0) >> 4;
					   for( l = 0; l < k; l++ )
					   {	// Search first nibble
						   if( lpBuf[l] == b )
							   break;
					   }
   					if( l < k )
	   				{
		   				lpcrs[l].cr_Freq += 1;
			   		}
				   	else
					   {
						   /* keep the NEW byte index into 16 color table */
						   lpBuf[k] = b;
                     // index to the color table
						   lpdw = (LPDWORD) (LPRGBQUAD) &lpq[b];
						   br = lpq[b].rgbRed;
						   bg = lpq[b].rgbGreen;
						   bb = lpq[b].rgbBlue;
//						lpcrs[k].cr_Color = RGB( lpq[b].rgbRed,
//							lpq[b].rgbGreen, lpq[b].rgbBlue );
						   lpcrs[k].cr_Color = RGB( br, bg, bb );
						   lpcrs[k].cr_Freq  = 1;
						   k++;  // count this NEW color
					   }
                  Wi++;    // count a NEW pixel of this ROW
                  if( Wi < Wid )
                  {
                     // and the second nibble is valid
                     for( l = 0; l < k; l++ )
					      {
                        // Search 2nd nibble
						      if( lpBuf[l] == g )
							   break;
					      }
      					if( l < k )
		      			{
				      		lpcrs[l].cr_Freq += 1;
					      }
					      else
					      {
						      /* keep the NEW byte index into the table */
						      lpBuf[k] = g;
                        // index into the table
						      lpdw = (LPDWORD) (LPRGBQUAD) &lpq[g];
                        // and extract the color there
						      lpcrs[k].cr_Color = RGB( lpq[g].rgbRed,
							      lpq[g].rgbGreen, lpq[g].rgbBlue );
						      lpcrs[k].cr_Freq  = 1;
						      k++;  // count a NEW color
					      }
                     Wi++;
				      }
                  else
                  {
                     break;   // got all the pixels in this row
                  }
               }
               else
               {
                  break;   // got all the pixels in this row
               }
            }  // for the BYTE width

            // skip any pad bytes to end of row
				while( j++ < Width )
					lps++;
			}  // for each ROW

      }

Cnt_Done:

   return k;

}  // end ThdCount4BPP

DWORD ThdCountMono( PDI lpDIBInfo, DWORD Height, DWORD Width, DWORD Wid,
                     PBYTE pbits, PBYTE lpBuf, DWORD crmx, PBYTE lpCOLR )
{
   PBYTE    lps;
   LPPALEX  lppxs;
	LPCOLR	lpcrs;
   DWORD    i;
   DWORD    j, k, l, w;
   BYTE     r;
   DWORD    b, g;
   BYTE     br,bg,bb;
   LPRGBQUAD   lpq;
   LPDWORD     lpdw;

   k = 0;
//		else if( wBPP == 1 )
		{
//			Width = WIDTHBYTES( Wid * wBPP );
         if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
            goto CntM_Done;

         // Use lps to get to the PALEX, which is after the color area
			lps   = lpBuf + crmx;
			lppxs = (LPPALEX)lps;
			lpcrs = &lppxs->px_Colrs[0];
         // but lps must point to the BITS of the DIB
         lps = pbits;
			/* are we pointing at the bitmap COLOR TABLE? */
			lpq   = (LPRGBQUAD)lpCOLR;
			OutColours( lpq, 2 );
         for( k = 0; k < 2; k++ )
         {
            lpdw = (LPDWORD) (LPRGBQUAD) &lpq[k];
			   br = lpq[k].rgbRed;
			   bg = lpq[k].rgbGreen;
			   bb = lpq[k].rgbBlue;
			   lpcrs[k].cr_Color = RGB( br, bg, bb );
			   lpcrs[k].cr_Freq  = 0;
         }

			for( i = 0; i < Height; i++ )
			{
            if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
               goto CntM_Done;
            // For each ROW
            w = 0;   // count pixel
				for( j = 0; j < Width; j++ )
				{
               if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                  goto CntM_Done;
               // For each COLUMN, extract a BYTE
               if( w < Wid )
               {
   					r = *lps++;		// Get 8 bits
	   				b = r;	      // copy it
		   			g = 1;         // set bit mask
			   		for( l = 0; l < 8; l++ )
				   	{
                     if( w < Wid )
                     {
                        if( g & b ) // pixel ON?
   						      lpcrs[1].cr_Freq++;
                        else
   						      lpcrs[0].cr_Freq += 1;
                        g = g << 1;
                        w++;  // bump pixel count
                     }
                     else
                     {
                        // got all the pixels in this row
                        break;
                     }
					   }
               }
               else
               {
                  // got all the pixels in this row
                  break;
               }
				}

				while( j++ < Width )
					lps++;
			}
      }

CntM_Done:

   return k;   // return 2

}  // end ThdCountMono


//	The luminance (brightness) signal is produced
//	by adding together, electronically,
//	the three signals from the color camera,
//	in the approximate ratios
//	30% red,
//	60% green, and
//	10% blue.
//	This combination produces white light on the screen of
//	the color receiver.  During a color telecast, this is
//	what is seen when the color-intensity control of the
//	receiver is turned off.

VOID  ShowSetArray( PDI lpDIBInfo )
{
   HGLOBAL  hg;
   PBYTE    lpb;
   DWORD    dwCnt;
   DWORD    dwi, dwf;
   DWORD    dwMax;
   BYTE     r,g,b;
   COLORREF cr;
	DLPPALEX	   lppx;
	DLPCOLR		lpcr;

//	if( hg = ghCOLR )
   dwMax  = 0;
	if( hg = lpDIBInfo->di_hCOLR )
	{
		if( ( lpb = DVGlobalLock(hg)  ) &&
			 ( dwCnt = GetCOLRCnt(lpb) ) )
		{
         lppx = (DLPPALEX)lpb;   // cast to structure
			sprtf( "Found %d colours ..."MEOR, dwCnt );
			dwMax = 1;
			for( dwi = 0; dwi < dwCnt; dwi++ )
			{
            if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
               goto Shw_Done;

      		lpcr = &lppx->px_Colrs[dwi];
      		cr   = lpcr->cr_Color;
            dwf  = lpcr->cr_Freq;
				r = GetRValue(cr);
				g = GetGValue(cr);
				b = GetBValue(cr);
				sprtf( "Col:0x%06X (r%3d,g%3d,b%3d) F=%d."MEOR,
					cr,
					(r & 0xff),
					(g & 0xff),
					(b & 0xff),
					dwf );
				if( dwf > dwMax )
					dwMax = dwf;
			}
			sprtf( "Shown %d colors ... Max. freq = %d"MEOR,
				dwCnt,
				dwMax );

Shw_Done:

			DVGlobalUnlock(hg);

		}
		else
		{
			sprtf( "Get colours FAILED???"MEOR );
		}
	}
	else
	{
			sprtf("NO HGLOBAL or buffer of colours ???"MEOR );
	}
}


BOOL  ThdCreatArray( PDI lpDIBInfo, DWORD k, LPDWORD lpdw, DWORD crmx )
{
   BOOL     flg = FALSE;
   HGLOBAL  hg;
   PBYTE    lpb;
   DWORD    csize;
   LPPALEX  lppx;
	LPCOLR	lpcr;
   DWORD    dwi;
   DWORD    i, j;

   hg  = 0;
   lpb = 0;
	if( k )
	{
		csize = PALEXSIZE(k);

      sprtf( "Alloc %u bytes for the %d color array."MEOR,
         csize, k );

		//if( ( hg  = DVGlobalAlloc( GHND, csize ) ) &&
		if( ( hg  = DVGAlloc( "24-BitArray", GHND, csize ) ) &&
			 ( lpb = DVGlobalLock( hg )           ) )
		{
			j = 0;
			lppx = (LPPALEX)lpb;
			lpcr = &lppx->px_Colrs[0];
			for( i = 0; i < crmx; i++ )
			{
            if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
               goto Crt_Done;
				/* if there is a COUNT at this color */
				if( dwi = lpdw[i] )  // and rank
				{
					/* and we are still counting */
					if( j < k )
					{
						/* store offset as the COLOR */
						lpcr[j].cr_Color = (COLORREF)i;
						/* and its frequency         */
						lpcr[j].cr_Freq  = dwi;
						j++;
						/* and bump our progress */
						// ProgP( (int)(( j * 100 ) / k ) );
					}
					else
					{
						chkme( "Count error 1!" );
						break;
					}
				}
			}
			if( ( i == crmx ) &&
				 ( j == k    ) )
			{
				/* good count collected into PALEX/COLOR array */
				lppx->px_Size       = csize;
				lppx->px_Count      = k;
      		lpDIBInfo->di_hCOLR = hg;
				DVGlobalUnlock(hg);
				hg  = 0;
            lpb = 0;
            flg = TRUE;
			}
			else
			{
				chkme( "Count error 2!" );
				lppx->px_Count = 0;
				DVGlobalUnlock(hg);
            lpb = 0;
			}

			if( hg )
			{
				DVGlobalFree(hg);
				hg = 0;
			}
		}
		else
		{
         chkme( "NO MEMORY FOR THE COLOR ARRAY" );
		}
   }

Crt_Done:

   if( hg && lpb )
      DVGlobalUnlock(hg);
   if( hg )
      DVGlobalFree(hg);

   return flg;

}


BOOL  ThdCreat256Array( PDI lpDIBInfo, PBYTE lpBuf, DWORD crmx, DWORD k )
{
   BOOL  flg = FALSE;
   HGLOBAL     hg;
   PBYTE       lpb;
	LPPALEX	   lppxs;
	LPCOLR	   lpcrs;
   DWORD       i, j;
   DWORD       csize;
	LPPALEX	   lppx;
	LPCOLR	   lpcr;

	lpb   = lpBuf + crmx;
	lppxs = (LPPALEX)lpb;
	lpcrs = &lppxs->px_Colrs[0];
			if( k )
			{
				csize = PALEXSIZE(k);
//				csize = ( sizeof(PALEX) + ( k * sizeof(COLR) ) );
//				if( ( hg  = DVGlobalAlloc( GHND, csize ) ) &&
				if( ( hg  = DVGAlloc( "256Array", GHND, csize ) ) &&
					 ( lpb = DVGlobalLock( hg )           ) )
				{
					j = 0;
					lppx = (LPPALEX)lpb;
					lpcr = &lppx->px_Colrs[0];
					for( i = 0; i < k; i++ )
					{
						lpcr[j].cr_Color = lpcrs[i].cr_Color;
						lpcr[j].cr_Freq  = lpcrs[i].cr_Freq;
						j++;
					}
					if( ( i == k  ) &&
						 ( j == k  ) )
					{
						/* good count collected into PALEX/COLR array */
						lppx->px_Size       = csize;
						lppx->px_Count      = k;
            		lpDIBInfo->di_hCOLR = hg;
						DVGlobalUnlock(hg);
						hg = 0;
                  flg = TRUE;
					}
					else
					{
						lppx->px_Count = 0;
						DVGlobalUnlock(hg);
					}

					if( hg )
					{
						DVGlobalFree(hg);
						hg = 0;
					}
				}
				else
				{
					//DIBError( ERR_MEMORY );
               chkme( "Memory error" );
				}
			}

   return flg;
}

BOOL  ThdCreat16Array(  PDI lpDIBInfo, PBYTE lpBuf, DWORD crmx, DWORD k )
{
   BOOL  flg = FALSE;
   HGLOBAL     hg;
   PBYTE       lpb;
	LPPALEX	   lppxs;
	LPCOLR	   lpcrs;
   DWORD       i, j;
   DWORD       csize;
	LPPALEX	   lppx;
	LPCOLR	   lpcr;

	lpb   = lpBuf + crmx;
	lppxs = (LPPALEX)lpb;
	lpcrs = &lppxs->px_Colrs[0];

			if( k )
			{
				csize = PALEXSIZE(k);
//				csize = ( sizeof(PALEX) + ( k * sizeof(COLR) ) );
//				if( ( hg  = DVGlobalAlloc( GHND, csize ) ) &&
				if( ( hg  = DVGAlloc( "16-Array", GHND, csize ) ) &&
					 ( lpb = DVGlobalLock( hg )           ) )
				{
					j = 0;
					lppx = (LPPALEX)lpb;
					lpcr = &lppx->px_Colrs[0];
					for( i = 0; i < k; i++ )
					{
						lpcr[j].cr_Color = lpcrs[i].cr_Color;
						lpcr[j].cr_Freq  = lpcrs[i].cr_Freq;
						j++;
					}
					if( ( i == k  ) &&
						 ( j == k  ) )
					{
						/* good count collected into PALEX/COLR array */
						lppx->px_Size  = csize;
						lppx->px_Count = k;
            		lpDIBInfo->di_hCOLR = hg;
						DVGlobalUnlock(hg);
						hg = 0;
                  flg = TRUE;
					}
					else
					{
						lppx->px_Count = 0;
						DVGlobalUnlock(hg);
					}

					if( hg )
					{
						DVGlobalFree(hg);
						hg = 0;
					}
				}
				else
				{
					//DIBError( ERR_MEMORY );
               chkme( "Memory error" );
				}
			}

   return flg;

}


BOOL  ThdCreatMono(  PDI lpDIBInfo, PBYTE lpBuf, DWORD crmx, DWORD k )
{
   BOOL  flg = FALSE;
   HGLOBAL     hg;
   PBYTE       lpb;
	LPPALEX	   lppxs;
	LPCOLR	   lpcrs;
   DWORD       i, j;
   DWORD       csize;
	LPPALEX	   lppx;
	LPCOLR	   lpcr;

	lpb   = lpBuf + crmx;
	lppxs = (LPPALEX)lpb;
	lpcrs = &lppxs->px_Colrs[0];

			if( k )
			{
				csize = PALEXSIZE(k);
//				csize = ( sizeof(PALEX) + ( k * sizeof(COLR) ) );
//				if( ( hg  = DVGlobalAlloc( GHND, csize ) ) &&
				if( ( hg  = DVGAlloc( "2-Array", GHND, csize ) ) &&
					 ( lpb = DVGlobalLock( hg )           ) )
				{
					j = 0;
					lppx = (LPPALEX)lpb;
					lpcr = &lppx->px_Colrs[0];
					for( i = 0; i < k; i++ )
					{
						lpcr[j].cr_Color = lpcrs[i].cr_Color;
						lpcr[j].cr_Freq  = lpcrs[i].cr_Freq;
						j++;
					}
					if( ( i == k  ) &&
						 ( j == k  ) )
					{
						/* good count collected into PALEX/COLR array */
						lppx->px_Size  = csize;
						lppx->px_Count = k;
            		lpDIBInfo->di_hCOLR = hg;
						DVGlobalUnlock(hg);
						hg = 0;
                  flg = TRUE;
					}
					else
					{
						lppx->px_Count = 0;
						DVGlobalUnlock(hg);
					}

					if( hg )
					{
						DVGlobalFree(hg);
						hg = 0;
					}
				}
				else
				{
					//DIBError( ERR_MEMORY );
               chkme( "Memory error" );
				}
			}

   return flg;

}


// *** BIG MEMORY QUESTION ***
// ***************************
#ifdef   TRYONEMEM
#define  GETBIGMEM(a) \
   ( WaitAvailable( lpDIBInfo )                ) && \
   ( hBuf  = DVGAlloc( "24-BitWORK", GHND, a ) ) && \
   ( lpBuf = DVGlobalLock( hBuf )              )

#else // !#ifdef   TRYONEMEM
#ifdef   TRYVIRTMEM
            //LPVOID VirtualAlloc(
            // LPVOID lpAddress,        // region to reserve or commit
            // SIZE_T dwSize,           // size of region
            // DWORD flAllocationType,  // type of allocation
            // DWORD flProtect );          // type of access protection
#define  GETBIGMEM(a) \
   ( lpBuf = VirtualAlloc( NULL, a, MEM_COMMIT, PAGE_READWRITE ) ) && \
   ( hBuf  = (HGLOBAL)VirtualLock( lpBuf, a )                    )
#else // !#ifdef   TRYVIRTMEM
         // GlobalAlloc()
         //if( ( hBuf  = DVGlobalAlloc( GHND, bsize ) ) &&
#define  GETBIGMEM(a) \
   ( hBuf  = DVGAlloc( "24-BitWORK", GHND, a ) ) && \
   ( lpBuf = DVGlobalLock( hBuf )              )
#endif   // ifdef   TRYVIRTMEM
#endif   // #ifdef   TRYONEMEM y/n

#ifdef   TRYONEMEM
BOOL  WaitAvailable( PDI lpDIBInfo )
{
   while( bBigBuf )   // the static BIG handle
   {
      if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
         return FALSE;
      Sleep(200);
      if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
         return FALSE;
   }
   bBigBuf++;        // claim it
   return TRUE;      // and allocate it
}

#endif   // #ifdef   TRYONEMEM

TCHAR szThreadok[] = "Thread %d: That took %s seconds. (tm=%d)"MEOR;
TCHAR szThreadex[] = "Thread %d: Ran for %s seconds! (tm=%d)"MEOR;

//  lpDIBInfo->di_dwNumber, Dbl2Str(GetETime(i), 5 ), i );

// ***************************
// Service of the COUNT THREAD
// ***************************
DWORD WINAPI CountColorProc( LPVOID lpv ) // thread data
{
   DWORD    dwRet = 0xDEAD0000;
   LPTSTR   pform = szThreadex;  // change to OK, if naturual self exiy = DONE JOB
   HGLOBAL  hdi;
   PDI      lpDIBInfo;
   HANDLE   hDIB;
   int      i;
   LPSTR    lpdib;
   DWORD    dwSize;
   DWORD    dwBPP;
	BYTE		r, g, b;
   COLORREF cr, crmx;
   DWORD    bsize, dwMax;   // allocated memory
   DWORD    Width, Height, Wid;
   PBYTE    pbits;
   HGLOBAL  hBuf;
   PBYTE    lpBuf;
   HWND     hWnd;
   PBYTE    lpCOLR;  // pointer to color table (if any)
   DWORD    dinum = 0;

   hBuf  = 0;
   lpBuf = 0;
   hDIB  = 0;
   lpdib = 0;
   hWnd  = 0;

   if( ( hdi = (HGLOBAL)lpv                 ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hdi) ) )
   {
      dwRet |= lpDIBInfo->di_dwNumber; // add in this number
      dinum = lpDIBInfo->di_dwNumber; // add in this number
      i = SetBTime();
      sprtf( "Thread number %d beginning ..."MEOR, dinum );
      lpDIBInfo->di_dwDnCount &= ~(fg_DnThread);   // when thread count completes

#ifdef   WRTCLRFILE
      bsize = IsClrFile( lpDIBInfo ); 
      if(bsize)
      {
         // all done once it is read in
         hDIB = lpDIBInfo->di_hCLRFile;
         if( ( VFH(hDIB) ) &&
             ( hBuf  = DVGAlloc( "PALEX1a", GHND, bsize ) ) &&
             ( lpBuf = DVGlobalLock( hBuf )                 ) )
         {
            if( ( ReadFile(hDIB, lpBuf, bsize, &dwMax, NULL ) ) &&
                ( bsize == dwMax ) )
            {
               if( DVIsValidPalEx( (PPX)lpBuf ) )
               {
                  lpDIBInfo->di_hCOLR = hBuf;
						DVGlobalUnlock(hBuf);
                  lpBuf = 0;
                  hBuf  = 0;
                  lpDIBInfo->di_dwDnCount |=
                     ( fg_DnThread | fg_GotFile );   // when thread count completes
               }
            }
         }
         if( VFH(hDIB) )
            CloseHandle(hDIB);
         hDIB = 0;
         if( hBuf && lpBuf )
            DVGlobalUnlock(hBuf);
         if( hBuf )
            DVGlobalFree(hBuf);
         hBuf = 0;
         lpBuf = 0;
         lpDIBInfo->di_hCLRFile = 0;

         if( lpDIBInfo->di_dwDnCount & fg_DnThread )   // when thread count completes
         {
            goto CC_OK;
         }
      }

#endif   // #ifdef   WRTCLRFILE
      //( ( hDIB  = lpDIBInfo->hDIB      ) &&
      //  ( lpdib = DVGlobalLock( hDIB ) ) ) // LOCK DIB HANDLE
      hDIB  = lpDIBInfo->hDIB;
      if(hDIB)
         lpdib = DVGlobalLock(hDIB);   // LOCK DIB HANDLE
      else
         lpdib = 0;
      if(lpdib) // LOCK DIB HANDLE
      {
      	lpCOLR = lpdib + OffsetToColor(lpdib);
			pbits  = FindDIBBits( lpdib );
         dwBPP  = lpDIBInfo->di_dwDIBBits; // 1, 4, 8, 24, ...
         Wid    = lpDIBInfo->di_dwDIBWidth;   // This may not be final WIDTH
         // bytes of height = rows in DIB
         Height = lpDIBInfo->di_dwDIBHeight;
			Width  = WIDTHBYTES( Wid * dwBPP );  // this should be BYTE WIDTH

         lpDIBInfo->di_sIS.wBPP  = lpDIBInfo->di_dwDIBBits;
//         lpDIBInfo->di_sIS.cxDIB = lpDIBInfo->di_dwDIBWidth;
//         lpDIBInfo->di_sIS.cyDIB = lpDIBInfo->di_dwDIBHeight;
         dwMax  = lpDIBInfo->di_dwDIBWidth * lpDIBInfo->di_dwDIBHeight;
         dwSize = 
            (DWORD)( (WIDTHBYTES( lpDIBInfo->di_dwDIBWidth *
                        lpDIBInfo->di_dwDIBBits )) *
                        lpDIBInfo->di_dwDIBHeight );
			lpDIBInfo->di_sIS.iColors   = DIBNumColors( lpdib );
			lpDIBInfo->di_sIS.iCalcCols = CalcDIBColors( lpdib );
      }
      else
      {
         goto CC_EXIT;
      }

      if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
         goto CC_EXIT;

   	r = 255;
	   g = 255;
	   b = 255;
	   cr = RGB(r,g,b);
	   crmx = ((DWORD)cr + 1);
      switch( dwBPP )
      {
      case 24:    /* 3 bytes or 16 million colours */
   		//lpfm = "24-bit";
   		/* ok, allocate a DWORD count for every colour */
	   	/* 0x00ffffff =  16,777,215 + 1 = 16MB x 4 = 64MB contig */
		   bsize = ( crmx * 4 );
         sprtf( "24-Bit: Work %u * 4 = %u. [ie %u(%ux%u)=%u]"MEOR,
            crmx,
            bsize,
            lpDIBInfo->di_dwDIBWidth,
            (WIDTHBYTES( lpDIBInfo->di_dwDIBWidth * lpDIBInfo->di_dwDIBBits )),
            lpDIBInfo->di_dwDIBHeight,
            dwMax );

         if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
            goto CC_EXIT;

         if( GETBIGMEM( bsize ) )
         {
            // === GOT THIS BIG MEMORY ===
            // ****************************************************************
            // this is a 16 MB buffer
            lpDIBInfo->di_sIS.iUsedCols = ThdCount24RGB( Height, Wid, Width,
               pbits, (PDWORD)lpBuf, crmx, lpDIBInfo );
            if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
               goto CC_EXIT;

            if( lpDIBInfo->di_sIS.iUsedCols )
            {
               if( ThdCreatArray( lpDIBInfo,
                              lpDIBInfo->di_sIS.iUsedCols,
                              (LPDWORD)lpBuf,
                              crmx ) )
               {

                  if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                     goto CC_EXIT;
                  lpDIBInfo->di_dwDnCount |= fg_DnThread;   // when thread count completes
#ifdef   SHWDIAGCC
                  ShowSetArray( lpDIBInfo );
#endif   // ifdef SHWDIAGCC
               }

               if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                  goto CC_EXIT;
            }
            else
            {
               if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                  goto CC_EXIT;
               chkme( "WHAT! NO COLORS USED found by ThdCount24RGB()!" );
               goto CC_EXIT;
            }
         }
         else
         {
            if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
               goto CC_EXIT;

            gdwError = GetLastError();
            chkme( "MEMORY ALLOCATION OF %u bytes FAILED with Err=%#x!"MEOR,
               bsize, gdwError );

            goto CC_EXIT;
         }
         break;

      case 8:
   		//lpfm = "8-bit(256)";
   		crmx  = 256;
	   	bsize = ( crmx * 10 );	/* Colour byte/index, indexed, RGB & freq */
		   bsize = ( crmx + sizeof(PALEX) + ( crmx * sizeof(COLR) ) );
         sprtf( "8-Bit: Work %u +/* ?? = %u. [ie %u(%ux%u)=%u]"MEOR,
            crmx,
            bsize,
            lpDIBInfo->di_dwDIBWidth,
            (WIDTHBYTES( lpDIBInfo->di_dwDIBWidth * lpDIBInfo->di_dwDIBBits )),
            lpDIBInfo->di_dwDIBHeight,
            dwMax );

         if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
            goto CC_EXIT;

         // GlobalAlloc()
         // if( ( hBuf  = DVGlobalAlloc( GHND, bsize ) ) &&
         if( ( hBuf  = DVGAlloc( "8-BitWORK", GHND, bsize ) ) &&
             ( lpBuf = DVGlobalLock( hBuf )         ) )
         {
            lpDIBInfo->di_sIS.iUsedCols = ThdCount256( lpDIBInfo,
               Height, Width, Wid,
               pbits, lpBuf, crmx, lpCOLR );
            if( lpDIBInfo->di_sIS.iUsedCols )
            {
               if( ThdCreat256Array( lpDIBInfo, lpBuf, crmx, lpDIBInfo->di_sIS.iUsedCols ) )
               {
                  if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                     goto CC_EXIT;
                  lpDIBInfo->di_dwDnCount |= fg_DnThread;   // when thread count completes
               }
               if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                  goto CC_EXIT;
            }
            else
            {
               if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                  goto CC_EXIT;  // if a FORCED exit, then no chkme()!!!

               chkme( "WHAT! NO COLORS USED from ThdCount256()!" );
               goto CC_EXIT;
            }
         }
         else
         {
            gdwError = GetLastError();
            chkme( "MEMORY ALLOCATION OF %u bytes FAILED with Err=%#x!"MEOR,
               bsize, gdwError );
            goto CC_EXIT;
         }
         break;

      case 4:
   		//lpfm = "4-bit(16)";
   		//bsize = (dwMax / 2);
	   	crmx  = 16;
		   //bsize = crmx * 10;	/* Colour byte/index, indexed, RGB & freq */
		   bsize = ( crmx + sizeof(PALEX) + ( crmx * sizeof(COLR) ) );
         sprtf( "4-Bit: Work %u +/* ?? = %u. [ie %u(%ux%u)=%u]"MEOR,
            crmx,
            bsize,
            lpDIBInfo->di_dwDIBWidth,
            (WIDTHBYTES( lpDIBInfo->di_dwDIBWidth * lpDIBInfo->di_dwDIBBits )),
            lpDIBInfo->di_dwDIBHeight,
            dwMax );

         if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
            goto CC_EXIT;

         if( ( hBuf  = DVGAlloc( "4-BitWORK", GHND, bsize ) ) &&
             ( lpBuf = DVGlobalLock( hBuf )         ) )
         {

            if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
               goto CC_EXIT;

            lpDIBInfo->di_sIS.iUsedCols = ThdCount4BPP( lpDIBInfo,
               Height, Width, Wid,
               pbits, lpBuf, crmx, lpCOLR );

            if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
               goto CC_EXIT;

            if( lpDIBInfo->di_sIS.iUsedCols )
            {
               if( ThdCreat16Array( lpDIBInfo, lpBuf, crmx, lpDIBInfo->di_sIS.iUsedCols ) )
               {
                  if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                     goto CC_EXIT;
                  lpDIBInfo->di_dwDnCount |= fg_DnThread;   // when thread count completes
#ifdef   SHWDIAGCC
                  ShowSetArray( lpDIBInfo );
#endif   // ifdef SHWDIAGCC
               }

               if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                  goto CC_EXIT;
            }
            else
            {
               if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                  goto CC_EXIT;
               chkme( "WHAT! NO COLORS USED from ThdCreat16Array()!" );
               goto CC_EXIT;
            }
         }
         else
         {
            if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
               goto CC_EXIT;
            gdwError = GetLastError();
            chkme( "MEMORY ALLOCATION OF %u bytes FAILED with Err=%#x!"MEOR,
               bsize, gdwError );
            goto CC_EXIT;
         }
         break;

      case 1:
		   //lpfm = "1-bit(2)";
   		//bsize = (dwMax / 8);
	   	//rcol = 2;
         crmx = 2;   // monochrome has just TWO(2) colors
		   bsize = ( crmx + sizeof(PALEX) + ( crmx * sizeof(COLR) ) );
         sprtf( "1-Bit: Work %u +/* ?? = %u. [ie %u(%ux%u)=%u]"MEOR,
            crmx,
            bsize,
            lpDIBInfo->di_dwDIBWidth,
            (WIDTHBYTES( lpDIBInfo->di_dwDIBWidth * lpDIBInfo->di_dwDIBBits )),
            lpDIBInfo->di_dwDIBHeight,
            dwMax );
         if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
            goto CC_EXIT;
         if( ( hBuf  = DVGAlloc( "1-BitWORK", GHND, bsize ) ) &&
             ( lpBuf = DVGlobalLock( hBuf )         ) )
         {
            if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
               goto CC_EXIT;
            lpDIBInfo->di_sIS.iUsedCols = ThdCountMono( lpDIBInfo,
               Height, Width, Wid,
               pbits, lpBuf, crmx, lpCOLR );
            if( lpDIBInfo->di_sIS.iUsedCols )
            {
               if( ThdCreatMono( lpDIBInfo, lpBuf, crmx, lpDIBInfo->di_sIS.iUsedCols ) )
               {
                  if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                     goto CC_EXIT;
                  lpDIBInfo->di_dwDnCount |= fg_DnThread;   // when thread count completes
#ifdef   SHWDIAGCC
                  ShowSetArray( lpDIBInfo );
#endif   // ifdef SHWDIAGCC
               }

               if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
                  goto CC_EXIT;

            }

            if( bEndThrd )    // pdi->di_bThdExit = force thread to exit
               goto CC_EXIT;
         }
         break;

      default:
         chkme( "ERROR: Unknown Bits per Pixel of %d!"MEOR, dwBPP );
         goto CC_EXIT;
         break;

      }

#ifdef   WRTCLRFILE
CC_OK:
#endif   // #ifdef   WRTCLRFILE

      pform = szThreadok;  // change to OK, if naturual self exit = DONE JOB
      //sprtf( "Thread %d: That took %s seconds! (tm=%d)"MEOR,
      //   lpDIBInfo->di_dwNumber,
      //   Dbl2Str(GetETime(i), 5 ),
      //   i );
      lpDIBInfo->di_dwDnCount |= fg_Res32;
      hWnd = lpDIBInfo->di_hwnd;    // extract the child handle

      dwRet = (0xFEED0000 | dinum); // add in this number

CC_EXIT:

      sprtf( pform,
         dinum,
         Dbl2Str(GetETime(i), 5 ),
         i );

      if( hDIB && lpdib )
		   DVGlobalUnlock( hDIB );  // UNLOCK DIB HANDLE

// *** BIG BUFFER TROUBLE ***
#ifdef   TRYVIRTMEM
      if( lpBuf )
         VirtualFree( lpBuf, 0, MEM_RELEASE );
#else // !TRYVIRTMEM
      if( hBuf && lpBuf )
         DVGlobalUnlock(hBuf);
      if( hBuf )
         DVGlobalFree(hBuf);
#endif   // ifdef   TRYVIRTMEM y/n
// *** ****************** ***

      lpDIBInfo->di_dwCntThdId = 0; // kill the COUNT Thread ID
      DVGlobalUnlock(hdi);

      if( hWnd )
      {
         PostMessage( hWnd, MYWM_THREADDONE2, 0, 0 );
      }
   }

   // and FINALLY release HOLD on the BIG BUFFER
   if( bBigBuf )
      bBigBuf--;   // kill the static BIG handle

   return dwRet;
}

#endif   // ifdef   USEITHREAD


void DVPaintClip2( HDC hDC, PRECT prcClip, PRECT prcSize, PRECT prcFrame )
{
	char     szStr[80];
   INT      nLen, dwExt, dx, dy, x, y;
   HDC      hDCBits;
   HBITMAP  hBitmap, hOldmap;

	// Draw rectangular clip region
	PatBlt( hDC,
		prcClip->left,
		prcClip->top,
		prcClip->right - prcClip->left,
		1,
		DSTINVERT );

	PatBlt( hDC,
		prcClip->left,
		prcClip->bottom,
		1,
		-(prcClip->bottom - prcClip->top),
		DSTINVERT );

	PatBlt( hDC,
		prcClip->right - 1,
		prcClip->top,
		1,
		prcClip->bottom - prcClip->top,
		DSTINVERT );

	PatBlt( hDC,
		prcClip->right,
		prcClip->bottom - 1,
		-(prcClip->right - prcClip->left),
		1,
		DSTINVERT );

	// Format the dimensions string
	wsprintf( szStr,
		"%dx%d",
		(prcSize->right  - prcSize->left),
		(prcSize->bottom - prcSize->top) );

	// Output the text to the DC
	//SetTextColor( hDC, RGB(255, 255, 255) );
	//SetBkColor(   hDC, RGB(  0,   0,   0) );
   //DrawText( hDC, // handle to DC
   //   szStr,         // "???x???", // text to draw
   //   strlen(szStr), // text length
   //   prcFrame,      // formatting dimensions
   //   DT_SINGLELINE | DT_VCENTER | DT_CENTER ); // text-drawing options
	nLen = lstrlen( szStr );

	// and center it in the rectangle
	dwExt   = GetTextExtent( hDC, szStr, nLen );
	dx      = LOWORD (dwExt);
	dy      = HIWORD (dwExt);
//	x       = (prcClip->right  + prcClip->left - dx) / 2;
//	y       = (prcClip->bottom + prcClip->top  - dy) / 2;
	x       = (prcFrame->right  + prcFrame->left - dx) / 2;
	y       = (prcFrame->bottom + prcFrame->top  - dy) / 2;
   if( x < 0 )
      x = 0;
   if( y < 0 )
      y = 0;
   if( hDCBits = CreateCompatibleDC( hDC ) )
   {
      if( hBitmap = CreateBitmap( dx, dy, 1, 1, NULL ) )
	   {
         // Output the text to the memory DC
         SetTextColor( hDCBits, RGB(255, 255, 255) );    // white
         SetBkColor(   hDCBits, RGB(  0,   0,   0) );    // black
		   hOldmap = SelectObject( hDCBits, hBitmap );     // select in the BITMAP
		   ExtTextOut( hDCBits, 0, 0, 0, NULL, szStr, nLen, NULL ); // pour on the text
		   BitBlt( hDC, x, y, dx, dy, hDCBits, 0, 0, SRCINVERT );   // blit and INVERT
		   hOldmap = SelectObject( hDCBits, hOldmap );  // get our bitmap back
		   DeleteObject( hBitmap );   // and kill it
      }
   	DeleteDC( hDCBits ); // and toos the memory DC
	}

}

// eof - DvImage.c
