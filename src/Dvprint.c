

/*==============================================*/
/* DvPrint.c:  Typical printing process         */
/* =============================================*/
#include	"dv.h"
#include	"DvPrint.h"
//#include <stdio.h>      // for sscanf() function

extern   BOOL  CenterWindow( HWND hWnd, HWND hParent );
extern   void  InitDocStruct( DOCINFO MLPTR pdi, LPSTR lphdr );
extern   BOOL	IsYes( LPSTR lps );
extern   BOOL	IsNo( LPSTR lps );
INT_PTR CALLBACK PRINTABORTDLG(
  HWND   hDlg,  // handle to dialog box
  UINT   uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam ); // second message parameter
BOOL CALLBACK PRINTABORTPROC( HDC hdc,        // handle to DC
                              int iError );   // error value

extern   HANDLE CopyHDCToDIB( HDC hScrDC, LPRECT lpRect );
extern   BOOL  WriteBMPFile2( LPTSTR lpf, HGLOBAL hDIB, BOOL bShow );
// NEW
extern   PMWL	AddToFileList4( PRDIB prd );

#define  RW    RECTWIDTH
#define  RH    RECTHEIGHT
#define     ATOM_PRT     0x11b

#define   ADDPRTTXT     // some text output to PRINTER
#define   USEBPAGE
#define   USEDRES       // use RESOLUTION of device, NOT physical size

// Recall that one twip is exactly one-twentieth of a printer�s point,
// and there are 1440 twips to the inch or 567 to the centimeter.
#define     MCTPI       1440
#define     MCTPC       567
// ie 2,53968253968253968253968253968254 cm = 1 inch

#define  DEF_FRM        30
#define  USE_ARIAL
#define  MYASZ          12

typedef struct tagDLGITEMS {
   DWORD    di_dwType;
   UINT     di_uiID;
   HWND     di_hWnd;
   LPTSTR   di_pName;
   UINT     di_uiEffect;
   BOOL     di_bEnab;
}DLGITEMS, * PDLGITEMS;


#define  t_End       0  // end of list
#define  t_Button    1
#define  t_Combo     2
#define  t_Label     3
#define  t_Edit      4
#define  t_Scroll    5
#define  t_Frame     6
#define  t_Check     7

#define  MXPTS    20
#define  pt_BotLef   0
#define  pt_BotRit   2
#define  pt_RitTop   4
#define  pt_RitBot   6

#define  ptt_None    0
#define  ptt_Left    1
#define  ptt_Rite    2
#define  ptt_Up      3
#define  ptt_Down    4

typedef  struct   tagPTARR {
   POINT pa_Pt;
   INT   pa_Type;
}PTARR, * PPTARR;

typedef  struct   tagPTARR2 {
   POINT pt;      // text point
   PTARR p1;      // arrow pos. 1
   POINT p1_To;   // line to pos. 1
   PTARR p2;      // arrow pos. 2
   POINT p2_To;   // line to pos. 2
}PTARR2, * PPTARR2;

typedef  struct tagDOTXT {

   LPVOID   dt_pPrtStr; // point back to FULL stucture (if ever required)
   // which in turn hold a pointer to lpDIBInfo (if ever required);

   HDC      hdc;
   LPRECT   prcPrt;
   LPRECT   prcDPage;

   SIZE     szObj;      // object original size
   LPRECT   prcPrint;   // printed object size
   SIZE     szOff;      // offsets applied
   SIZE     szPercent;  // percentages applied
   SIZE     szSize;     // widht and height after percentage
   LPRECT   prcPPage;   // print page size

   PPTARR2  pp2;        // = &pps->ps_p2;
   HFONT    hfHorz;
   HFONT    hfVert;
   DWORD    dxSize;
   DWORD    dySize;
   DWORD    dxPx;
   DWORD    dyPx;
   double   dbXIns;
   double   dbYIns;
   DWORD    dwLeft;
   DWORD    dwRight;
   DWORD    dwTop;
   DWORD    dwBot;
}DOTXT, * PDOTXT;

typedef  struct   tagPRTSTR {

   PDI   ps_lpDIBInfo;    // pointer to WHOLE lpDIBInfo structure

   HWND  ps_hParent; // parent window handle

   HWND  ps_hMDI;    // current MDI child handle
   HWND  ps_hCBPrt;  // handle to printer name COMBO box
   HWND  ps_hOK;     // handle to OK button
   DWORD ps_dwPCnt;  // count of printers enumerated
   DWORD ps_dwCopies;   // copies to be printed

   DWORD ps_uiActID; // last active of IDC_WID, HIT, PCWID, PCHIT (if any)
   DWORD ps_uiFocus; // EDIT control that got focus

   DWORD ps_dwOWid;  // DIB pixel width
   DWORD ps_dwOHit;  // DIB pixel height
   RECT  ps_rcDIB;   // DIB output size/position

   // image positioning
   DWORD ps_dwTop;   // top of print - nPrtTop
   DWORD ps_dwLeft;  // left of print - PrtLeft

   // and sizing
   DWORD ps_dwPCWid; // percentage width - nPrtPCW
   DWORD ps_dwPCHit; // percentage height - nPrtPCW

   DWORD ps_dwWidth;    // pixel width of image - nPrtWid
   DWORD ps_dwHeight;   // pixel height of image

   // keep previous values
   DWORD ps_dwOldTop;   // top of print - nPrtTop
   DWORD ps_dwOldLeft;  // left of print - PrtLeft
   // and sizing
   DWORD ps_dwOPCWid; // percentage width - nPrtPCW
   DWORD ps_dwOPCHit; // percentage height - nPrtPCW
   DWORD ps_dwOWidth;    // pixel width of image - nPrtWid
   DWORD ps_dwOHeight;   // pixel height of image

   BOOL  ps_bChgPrt;    // do calculation again in Render_DIB()
   BOOL  ps_bNoteOn;    // when "helpful" note is displayed

   DWORD ps_dwMFWid;
   DWORD ps_dwMFHit;

   BOOL  ps_bDistort;   // allow distortion of image
   BOOL  ps_bFitPage;   // fit image into page

   BOOL  ps_bAddText;   // add dimension text strings
   BOOL  ps_bAddText2;  // add general text strings
   BOOL  ps_bAddText3;  // add border text strings

   RECT  ps_rcDlgFrm;   // actual DIALOG frame size
   RECT  ps_rcPFrame;   // paint frame
   RECT  ps_rcPPage;    // paint object size
   RECT  ps_rcObj;      // actual pixel object size
   RECT  ps_rcPObj;     // last rendered size
   RECT  ps_rcRend;     // last RENDERING of the image
   RECT  ps_rcPrt;      // and as object would PRINTED be on "real" PAGE

   RECT  ps_rcTmpRend;  // just gettting the size as a check
   RECT  ps_rcTmpPrt;   // that it all fits ok

   DWORD ps_dwSel;      // current selection
   DWORD ps_dwPrtOff;   // OFFSET into HGLOBAL allocation
   // to obtain a pointer to SELECTED print info 2 (PPRINTER_INFO_2)
   BOOL  ps_iPrtOK;     // indicates we could OPEN the printer if 2
   // means the SELECTION (ps_dwSel and ps_dwPrtOff) is VERY VALID
   // and a successful print may be possible, even if = 1
   HGLOBAL  ps_hDIB;    // all important DIB handle

   HANDLE            ps_hPrinter;   // handle of the OPEN printer
   PRINTER_DEFAULTS  ps_sPrtDefs;   // structure for OpenPrinter() function
   DWORD             ps_dwP2Size;   // requested size of first ENUM
   DWORD             ps_cbNeeded;   // required or copied size

   HGLOBAL           ps_hEnum;   // *** ALLOCATED MEMORY ***

   HFONT             ps_hFontA;   // *** CREATED FONT for Frame ***
   HFONT             ps_hFontV;   // *** CREATED VERT. FONT for Frame ***
   HFONT             ps_hPFontA;   // *** CREATED FONT for Printing ***
   HFONT             ps_hPFontV;   // *** CREATED FONT for V.Printing ***

   LOGFONT           ps_sLogFont;   // use to create fonts

   int               ps_iArray;     // count in array
   //POINT             ps_ptArray[MXPTS];   // point array
   PTARR             ps_ptArray[MXPTS];   // point array

   LONG              ps_xPerInch;   // = GetDeviceCaps( hdcTarget, LOGPIXELSX);
   LONG              ps_yPerInch;   // = GetDeviceCaps( hdcTarget, LOGPIXELSY);
   // like FORMATRANGE
   RECT              ps_rcArea;     // Area (in Device Units)
   RECT              ps_rcTPage;     // Entire area of rendering device. Units in twips. 
   RECT              ps_rcDPage;     // Entire area of rendering device. In device units
   RECT              ps_rcBPage;     // Area (in pixel) after BORDER removed

   LONG              ps_dxPage;
   LONG              ps_dyPage;
   LONG              ps_dxRes;   // = GetDeviceCaps(ppp->fr.hdcTarget, HORZRES);
   LONG              ps_dyRes;   // = GetDeviceCaps(ppp->fr.hdcTarget, VERTRES);
   INT               ps_dxSize;  // = GetDeviceCaps( hdcTarget, HORZSIZE  );   // in millimeters
   INT               ps_dySize;  //   = GetDeviceCaps( hdcTarget, VERTSIZE  );
   INT               ps_PhyWidth;   // PHYSICALWIDTH - the width in device units.
   // eg at 600 dpi on 8.5x11 in. paper has width of 5100 device units.
   INT               ps_PhyHeight;  // PHYSICALHEIGHT - the height in device units.
   // eg at 600 dpi on 8.5x11 in. paper has height value of 6600 device units.
   INT               ps_PhyOffX;    // PHYSICALOFFSETX -  distance from left in device units.
   // eg at 600 dpi on 8.5x11 in. paper, that cannot print on the leftmost 0.25-inch
   // has a horizontal physical offset of 150 device units. 
   INT               ps_PhyOffY;    // PHYSICALOFFSETY the distance from the top in device units.
   // eg at 600 dpi on 8.5x11 in. paper, that cannot print on the topmost
   // 0.5-inch of paper, has a vertical physical offset of 300 device units.  


   DOTXT       ps_sDT;  // DoTxt( structure )

   DOCINFO     ps_sDI;
   TCHAR       ps_szDefPrtr[CCHDEVICENAME];  // default printer name
   TCHAR       ps_szDocName[264];

   TCHAR       ps_szBotTxt[64];  // TCHAR szPrtHTxt[]  = "H_Text";         // nPrtHTxt
   TCHAR       ps_szRitTxt[64];  // TCHAR szPrtVTxt[]  = "V_Text";         // nPrtVTxt
   SIZE        ps_ssFA;    // string size szPrtHTS[] = "H_Text_Size"; // nPrtHTS
   SIZE        ps_ssF2;    // string size szPrtVTS[] = "V_Text_Size"; // nPrtVTS
   int         ps_iBotTxt;       // length of H text
   int         ps_iRitTxt;       // length of V text
   // position of text
   POINT       ps_ptBotTxt;   // bottom text
   POINT       ps_ptRitTxt;   // right text
   POINT       ps_ptTopTxt;   // top text
   POINT       ps_ptLefTxt;   // left text

   PTARR2      ps_p2;

   RDIB        ps_sRDIB;      // for openning a window

}PRTSTR, * PPS;

#define  pszDefPrtr  pps->ps_szDefPrtr

typedef	struct tagIDSTG   {
	UINT	   id_uiID;
   LPTSTR   id_psNm;
}IDSTG, * PIDSTG;

typedef enum tagPRTCHG {
 nPrtALL,
 nPrtDef,  
 nPrtxPI,  
 nPrtyPI,  
 nPrtxTPP, 
 nPrtyTPP, 
 nPrtHRes, 
 nPrtVRes, 
 nPrtHSiz, 
 nPrtVSiz, 
 nPrtHPhy, 
 nPrtVPhy, 
 nPrtPOffH,
 nPrtPOffV,
 nPrtFHit, 
 nPrtFWid, 
 nPrtArea, 
 nPrtDPg,  
 nPrtTPg,  
 nPrtBPg,
 nPrtPPg,
 nPrtFrm,
 nPrtPRc,
 nPrtDist, 
 nPrtFit,
 nPrtTxt,
 nPrtTxt2,
 nPrtTxt3,
 nPrtRend,
 nPrtPrt,
 nPrtTop,
 nPrtLeft,
   nPrtWid,
   nPrtHit,
   nPrtPCW,
   nPrtPCH,
   nPrtCop,
   nPrtHTxt,
   nPrtVTxt,
   nPrtHTS,
   nPrtVTS,
 nPrtMAX
}PRTCHG;

#define  SETITEM(a,b,c)\
   if( pps->a != b )\
   {\
      pps->a = b;\
      nPrtChg[c] = TRUE;\
   }

// local
VOID  ChkDefPrtr( VOID );
VOID  P2_RenderDIB( HWND hDlg, PPS pps );
VOID  GetPntPage( LPRECT prcPPage, LPRECT prcFrame, LPRECT prcPage, BOOL bCentre );
VOID  GetPntPage2( LPRECT prcPObj, LPRECT prcFrame, LPRECT prcObj, LPRECT prcPage );
BOOL  GetPrtRect( LPRECT prcPrt, LPRECT prcDPage, LPRECT prcPObj, LPRECT prcPPage );
BOOL  GetDevInfo( PPS pps, LPDEVNAMES pdn, PDEVMODE pdm );
INT   GetHdcInfo( PPS pps, HDC hdcTarget, LPTSTR lppn );
#ifndef  NDEBUG
VOID  ShowDBGSizes( PPS pps, LPTSTR lppn );
#endif   // #ifndef  NDEBUG
VOID  UpdateRendRect( PPS pps, LPRECT prcRend, LPRECT prcPrt );
BOOL  P2_InitText( PPS pps );
VOID  CopyDefPPS( PPS ppsn, PPS ppss );
VOID  PaintTxt( HDC hdc, PPS pps );
VOID  GetRenderSize( PPS pps );  // only puts values into ps_TmpRend and ps_TmpPrt
BOOL  CheckNewValue( PPS pps, UINT uid2, DWORD uiv );
VOID  SetNewValue( HWND hDlg, PPS pps, UINT uid2, UINT uiv );
VOID  ResetFrame( PPS pps );

// Macros
#define ChangePP(nPct)	if(ghDlgAbort) SendMessage( ghDlgAbort, MYWM_CHANGEPCT, (WPARAM)nPct, 0L )

BOOL  bOpenPW = FALSE;  // does NOT presently work as expected!!!

// block to hold changes in PRINT block
// copied from GLOBAL when print invoked,
// and copied back to global after print done
// ==========================================
PRTCHG   nPrtChg[nPrtMAX];
// ==========================================

// ***********************************************************************
// NEW PRINT DIALOG - LIKE MS PHOTO EDITOR (2000)
//IDD_PRINT DIALOGEX 0, 0, 351, 213
//STYLE DS_MODALFRAME | DS_3DLOOK | WS_POPUP | WS_CAPTION | WS_SYSMENU
//CAPTION "PRINT"
//FONT 8, "MS Sans Serif"
//BEGIN
DLGITEMS sPrtDlg[] = { 
//    DEFPUSHBUTTON   "OK",IDOK,229,192,50,14
   { t_Button, IDOK, 0, "OK", 0, FALSE },
//    PUSHBUTTON      "Cancel",IDCANCEL,294,192,50,14
//    LTEXT           "Printers:",IDC_STATIC,9,11,29,8
//    COMBOBOX        IDC_CBPRINTERS,41,7,235,53,CBS_DROPDOWN | 
//                    CBS_AUTOHSCROLL | WS_VSCROLL | WS_TABSTOP
   { t_Combo, IDC_CBPRINTERS, 0, "PRINTERS", 0, FALSE },
//    PUSHBUTTON      "P&roperties",IDC_BPROPERTIES,291,7,53,16,0,
//                    WS_EX_CLIENTEDGE
//    GROUPBOX        "Position",IDC_STATIC,175,37,169,55
//    LTEXT           "&Top:",IDC_LABTOP,181,51,34,8
   { t_Label, IDC_LABTOP, 0, "LTOP", 0, FALSE },
//    LTEXT           "&Left:",IDC_LABLEFT,181,70,34,8
   { t_Label, IDC_LABLEFT, 0, "LLEFT", 0, FALSE },
//    EDITTEXT        IDC_EDTOP,216,49,34,14,ES_AUTOHSCROLL
   { t_Edit, IDC_EDTOP, 0, "TOP", 0, TRUE },
//    SCROLLBAR       IDC_SBTOP,251,49,13,14,SBS_VERT
   { t_Scroll, IDC_SBTOP, 0, "SBTOP", IDC_EDTOP, TRUE },
//    EDITTEXT        IDC_EDLEFT,216,67,34,14,ES_AUTOHSCROLL
   { t_Edit, IDC_EDLEFT, 0, "LEFT", 0, TRUE },
//    SCROLLBAR       IDC_SBLEFT,251,67,13,14,SBS_VERT
   { t_Scroll, IDC_SBLEFT, 0, "SBLEFT", IDC_EDLEFT, TRUE },
//    CONTROL         "",IDC_FRMA4,"Static",SS_GRAYFRAME,7,41,159,165
   { t_Frame, IDC_FRMA4, 0, "FRAME", 0, FALSE },
//    PUSHBUTTON      "Ce&nter",IDC_BCENTER,287,59,45,14
   { t_Button, IDC_BCENTER, 0, "CENTER", 0, TRUE },
//    PUSHBUTTON      "&Zero",IDC_BZERO,287,68,45,14
   { t_Button, IDC_BZERO, 0, "ZERO", 0, TRUE },
//    LTEXT           "Px",IDC_LUTOP,269,54,12,8
   { t_Label, IDC_LUTOP, 0, "LUTOP", 0, FALSE },
//    LTEXT           "Px",IDC_LULEFT,269,70,12,8
   { t_Label, IDC_LULEFT, 0, "LULEFT", 0, FALSE },
//    GROUPBOX        "Size",IDC_STATIC,176,97,168,86
//    LTEXT           "&Width:",IDC_LABWID,181,110,34,8
   { t_Label, IDC_LABWID, 0, "LWID", 0, FALSE },
//    EDITTEXT        IDC_EDWID,218,106,34,14,ES_AUTOHSCROLL
   { t_Edit, IDC_EDWID, 0, "WIDTH", IDC_EDPCWID, TRUE },
//    LTEXT           "Hei&ght:",IDC_LABHIT,181,130,34,8
   { t_Label, IDC_LABHIT, 0, "LHIT", 0, FALSE },
//    CONTROL         "Allow //&Distortion",IDC_CHDIST,"Button",BS_AUTOCHECKBOX
//| WS_TABSTOP,181,150,64,10
   { t_Check, IDC_CHDIST, 0, "DISTORION", 0, FALSE },
//    CONTROL         "&Fit to Page",IDC_CHFIT,"Button",BS_AUTOCHECKBOX | 
//                    WS_TABSTOP,181,164,50,10
   { t_Check, IDC_CHFIT, 0, "FIT", 0, FALSE },
//    EDITTEXT        IDC_EDHIT,218,126,34,14,ES_AUTOHSCROLL
   { t_Edit, IDC_EDHIT, 0, "HEIGHT", IDC_EDPCWID, TRUE },
//    EDITTEXT        IDC_EDPCWID,283,105,25,14,ES_AUTOHSCROLL
   { t_Edit, IDC_EDPCWID, 0, "PCWIDTH", IDC_EDWID, TRUE },
//    EDITTEXT        IDC_EDPCHIT,283,127,25,14,ES_AUTOHSCROLL
   { t_Edit, IDC_EDPCHIT, 0, "PCHEIGHT", IDC_EDHIT, TRUE },
//    SCROLLBAR       IDC_SBWID,252,106,13,14,SBS_VERT
   { t_Scroll, IDC_SBWID, 0, "SBWID", IDC_EDWID, TRUE },
//    SCROLLBAR       IDC_SBHIT,252,126,13,14,SBS_VERT
   { t_Scroll, IDC_SBHIT, 0, "SBHIT", IDC_EDHIT, TRUE },
//    SCROLLBAR       IDC_SBWID2,308,105,13,14,SBS_VERT
   { t_Scroll, IDC_SBWID2, 0, "SBWID2", IDC_EDPCWID, TRUE },
//    SCROLLBAR       IDC_SBHIT2,309,126,13,14,SBS_VERT
   { t_Scroll, IDC_SBHIT2, 0, "SBHIT2", IDC_EDPCHIT, TRUE },
//    LTEXT           "%",IDC_STATIC,326,108,12,8
//    LTEXT           "%",IDC_STATIC,326,129,11,8
//    EDITTEXT        IDC_EDCOPIES,285,160,25,14,ES_AUTOHSCROLL
   { t_Edit, IDC_EDCOPIES, 0, "COPIES", 0, FALSE },
//    SCROLLBAR       IDC_SBCOPIES,310,160,13,14,SBS_VERT
   { t_Scroll, IDC_SBCOPIES, 0, "SBCOPIES", IDC_EDCOPIES, FALSE },
//    LTEXT           "Copies:",IDC_STATIC,251,163,24,8
//    LTEXT           "Px",IDC_LUWID,268,110,12,8
//    LTEXT           "Px",IDC_LUHIT,268,129,12,8
//    EDITTEXT        IDC_EDSELECTED,7,22,337,14,ES_AUTOHSCROLL | //ES_READONLY
//| NOT WS_BORDER
   { t_Edit, IDC_EDSELECTED, 0, "SELECTED", 0, FALSE },
//END
   { t_End, 0, 0, 0, 0, FALSE }
};

BOOL  GetDlgHandles( HWND hDlg )
{
   BOOL  bRet = FALSE;
   PDLGITEMS pdi = &sPrtDlg[0];

   while( pdi->di_dwType != t_End )
   {
      pdi->di_hWnd = GetDlgItem( hDlg, pdi->di_uiID );
      if( !pdi->di_hWnd )
         break;
      pdi++;
   }

   if( pdi->di_dwType == t_End )
      bRet = TRUE;

   return bRet;
}

HWND  GetDlgHand( UINT id )
{
   PDLGITEMS pdi = &sPrtDlg[0];
   while( pdi->di_dwType != t_End )
   {
      if( pdi->di_uiID == id )
         return( pdi->di_hWnd );
      pdi++;
   }
   return NULL;
}
HWND  GetHandPID( UINT id )
{
   return( GetDlgHand(id) );
}

UINT  GetItemPerHand( HWND hwnd )
{
   BOOL  bRet = FALSE;
   PDLGITEMS pdi = &sPrtDlg[0];
   while( pdi->di_dwType != t_End )
   {
      if( pdi->di_hWnd == hwnd )
         return( pdi->di_uiID );
      pdi++;
   }
   return 0;
}

LPTSTR   GetDBGNamePID( UINT id, BOOL bAlways )
{
   static TCHAR _sszNIL[] = "<not-in-list>";
   PDLGITEMS pdi = &sPrtDlg[0];
   while( pdi->di_dwType != t_End )
   {
      if( pdi->di_uiID == id )
         return( pdi->di_pName );
      pdi++;
   }

   if( bAlways )
      return( &_sszNIL[0] );
   else
      return NULL;
}

UINT  GetRelItemPID( UINT id )
{
   PDLGITEMS pdi = &sPrtDlg[0];
   while( pdi->di_dwType != t_End )
   {
      if( pdi->di_uiID == id )
         return( pdi->di_uiEffect );
      pdi++;
   }
   return 0;
}

//  Platform SDK: Windows GDI 
//EnumPrinters
//The EnumPrinters function enumerates available printers, print
//servers, domains, or print providers. 
//BOOL EnumPrinters(
//  DWORD Flags,         // printer object types
//  LPTSTR Name,         // name of printer object
//  DWORD Level,         // information level
//  LPBYTE pPrinterEnum, // printer information buffer
//  DWORD cbBuf,         // size of printer information buffer
//  LPDWORD pcbNeeded,   // bytes received or required
//  LPDWORD pcReturned   // number of printers enumerated
//);
//Parameters
//Flags 
//[in] Specifies the types of print objects that the function
//should enumerate. This value can be one or more of the following
//values. Value Meaning 
//PRINTER_ENUM_LOCAL The function ignores the Name parameter, and
//enumerates the locally installed printers. 
//Windows 95: The function will also enumerate network printers
//because they are handled by the local print provider.
//PRINTER_ENUM_NAME The function enumerates the printer identified
//by Name. This can be a server, a domain, or a print provider. If
//Name is NULL, the function enumerates available print providers.
//PRINTER_ENUM_SHARED The function enumerates printers that have
//the shared attribute. Cannot be used in isolation; use an OR
//operation to combine with another PRINTER_ENUM type. 
//PRINTER_ENUM_DEFAULT Windows 95: The function returns
//information about the default printer. 
//PRINTER_ENUM_CONNECTIONS Windows NT/ 2000: The function
//enumerates the list of printers to which the user has made
//previous connections. 
//PRINTER_ENUM_NETWORK Windows NT/ 2000: The function enumerates
//network printers in the computer's domain. This value is valid
//only if Level is 1. 
//PRINTER_ENUM_REMOTE Windows NT/ 2000: The function enumerates
//network printers and print servers in the computer's domain.
//This value is valid only if Level is 1. 
//If Level is 4, you can only use the PRINTER_ENUM_CONNECTIONS and
//PRINTER_ENUM_LOCAL constants. 
//Name 
//[in] If Level is 1, Flags contains PRINTER_ENUM_NAME, and Name
//is non-NULL, then Name is a pointer to a null-terminated string
//that specifies the name of the object to enumerate. This string
//can be the name of a server, a domain, or a print provider. 
//If Level is 1, Flags contains PRINTER_ENUM_NAME, and Name is
//NULL, then the function enumerates the available print
//providers. 
//If Level is 1, Flags contains PRINTER_ENUM_REMOTE, and Name is
//NULL, then the function enumerates the printers in the user's
//domain. 
//If Level is 2 or 5, Name is a pointer to a null-terminated
//string that specifies the name of a server whose printers are to
//be enumerated. If this string is NULL, then the function
//enumerates the printers installed on the local machine. 
//If Level is 4, Name should be NULL. The function always queries
//on the local machine. 
//When Name is NULL, it enumerates printers that are installed on
//the local machine. These printers include those that are
//physically attached to the local machine as well as remote printers to 
//which it has a network connection. 
//Level 
//[in] Specifies the type of data structures pointed to by
//pPrinterEnum. Valid values are 1, 2, 4, and 5, which correspond
//to the PRINTER_INFO_1, PRINTER_INFO_2, PRINTER_INFO_4, and
//PRINTER_INFO_5 data structures. 
//Windows 95: The value can be 1, 2, or 5. 
//Windows NT/Windows 2000: This value can be 1, 2, 4, or 5. 
//pPrinterEnum 
//[out] Pointer to a buffer that receives an array of
//PRINTER_INFO_1, PRINTER_INFO_2, PRINTER_INFO_4, or
//PRINTER_INFO_5 structures. Each structure contains data that
//describes an available print object. 
//If Level is 1, the array contains PRINTER_INFO_1 structures. If
//Level is 2, the array contains PRINTER_INFO_2 structures. If
//Level is 4, the array contains PRINTER_INFO_4 structures. If
//Level is 5, the array contains PRINTER_INFO_5 structures. 
//The buffer must be large enough to receive the array of data
//structures and any strings or other data to which the structure
//members point. If the buffer is too small, the pcbNeeded
//parameter returns the required buffer size. 
//Windows 95: The buffer cannot receive PRINTER_INFO_4 structures.
//It can receive any of the other types. 
//cbBuf 
//[in] Specifies the size, in bytes, of the buffer pointed to by
//pPrinterEnum. 
//pcbNeeded 
//[out] Pointer to a value that receives the number of bytes
//copied if the function succeeds or the number of bytes required
//if cbBuf is too small. 
//pcReturned 
//[out] Pointer to a value that receives the number of
//PRINTER_INFO_1, PRINTER_INFO_2, PRINTER_INFO_4, or
//PRINTER_INFO_5 structures that the function returns in the array
//to which pPrinterEnum points. 
//Return Values
//If the function succeeds, the return value is a nonzero value.
//If the function fails, the return value is zero. To get extended
//error information, call GetLastError. 
//Remarks
//If EnumPrinters returns a PRINTER_INFO_1 structure in which
//PRINTER_ENUM_CONTAINER is specified, this indicates that there
//is a hierarchy of printer objects. An application can enumerate
//the hierarchy by calling EnumPrinters again, setting Name to the
//value of the PRINTER_INFO_1 structure's pName member. 
//The EnumPrinters function does not retrieve security
//information. If PRINTER_INFO_2 structures are returned in the
//array pointed to by pPrinterEnum, their pSecurityDescriptor
//members will be set to NULL. 
//To get information about the default printer, call the
//GetProfileString function with the section name string set to
//"windows" and the key name string set to "device". The returned
//string contains the name of the default printer, the name of the
//printer DRV file, and the port to which the printer is attached.
//Windows 2000 and later: To get information about the default
//printer, call GetDefaultPrinter
//Windows NT/Windows 2000: The PRINTER_INFO_4 structure provides
//an easy and extremely fast way to retrieve the names of the
//printers installed on a local machine, as well as the remote
//connections that a user has established. When EnumPrinters is
//called with a PRINTER_INFO_4 data structure, that function queries the
//registry for the specified information, then returns
//immediately. This differs from the behavior of EnumPrinters when
//called with other levels of PRINTER_INFO_* data structures. In
//particular, when EnumPrinters is called with a level 2
//(PRINTER_INFO_2) data structure, it performs an OpenPrinter call
//on each remote connection. If a remote connection is down, or
//the remote server no longer exists, or the remote printer no
//longer exists, the function must wait for RPC to time out and
//consequently fail the OpenPrinter call. This can take a while.
//Passing a PRINTER_INFO_4 structure lets an application retrieve
//a bare minimium of required information; if more detailed
//information is desired, a subsequent EnumPrinter level 2 call
//can be made.
//Windows 95: To quickly enumerate local and network printers, use
//the PRINTER_INFO_5 structure. This causes EnumPrinters to query
//the registry rather than make remote calls, and is similar to
//using the PRINTER_INFO_4 structure on Windows NT as described in
//the preceding paragraph. 
//Examples
//The following table shows the EnumPrinters output for various
//Flags values when the Level parameter is set to 1. 
//In the Name parameter column of the table, you should substitute
//an appropriate name for Print Provider, Domain, and Machine. For
//example, for Print Provider, you could use the name of the
//Windows NT network print provider: "Windows NT Remote Printers",
//or the name of the Windows 95 local print provider: "Windows 95
//Local Print Provider". To get print provider names, call
//EnumPrinters with Name set to NULL. 
//Flags parameter Name parameter Result 
//PRINTER_ENUM_LOCAL The Name parameter is ignored. All local
//printers. 
//Windows 95: Also enumerates network printers because they are
//installed locally.
//PRINTER_ENUM_NAME "Print Provider" All domain names 
//PRINTER_ENUM_NAME Windows NT/ 2000: 
//"Print Provider!Domain" All printers and print servers in the
//computer's domain 
//PRINTER_ENUM_NAME Windows NT/ 2000: 
//"Print Provider!!\\Machine" All printers shared at \\Machine 
//PRINTER_ENUM_NAME Windows NT/ 2000: An empty string, "" 
//Windows 95: The name of the local machine or the local print
//provider.
// All local printers. 
//Windows 95: Also enumerates network printers because they are
//installed locally.
//PRINTER_ENUM_NAME NULL All print providers in the computer's
//domain 
//Windows NT/ 2000: 
//PRINTER_ENUM_CONNECTIONS  The Name parameter is ignored. All
//connected remote printers 
//Windows NT/ 2000: 
//PRINTER_ENUM_NETWORK  The Name parameter is ignored. All
//printers in the computer's domain 
//Windows NT/ 2000: 
//PRINTER_ENUM_REMOTE  An empty string, "" All printers and print
//servers in the computer's domain 
//Windows NT/ 2000: 
//PRINTER_ENUM_REMOTE  "Print Provider" Same as PRINTER_ENUM_NAME 
//Windows NT/ 2000: 
//PRINTER_ENUM_REMOTE  "Print Provider!Domain" All printers and
//print servers in computer's domain, regardless of Domain specified. 
//Requirements 
//  Windows NT/2000: Requires Windows NT 3.1 or later.
//  Windows 95/98: Requires Windows 95 or later.
//  Header: Declared in Winspool.h; include Windows.h.
//  Library: Use Winspool.lib.
//  Unicode: Implemented as Unicode and ANSI versions on Windows
//NT/2000.
//See Also
//Printing and Print Spooler Overview, Printing and Print Spooler
//Functions, AddPrinter, DeletePrinter, GetPrinter,
//GetProfileString, PRINTER_INFO_1, PRINTER_INFO_2,
//PRINTER_INFO_4, PRINTER_INFO_5, SetPrinter 
//Built on Wednesday, July 12, 2000Requirements 
//  Windows NT/2000: Requires Windows NT 3.1 or later.
//  Windows 95/98: Requires Windows 95 or later.
//  Header: Declared in Winspool.h; include Windows.h.
//  Library: Use Winspool.lib.
//  Unicode: Implemented as Unicode and ANSI versions on Windows
//NT/2000.
//See Also
//Printing and Print Spooler Overview, Printing and Print Spooler
//Functions, AddPrinter, DeletePrinter, GetPrinter,
//GetProfileString, PRINTER_INFO_1, PRINTER_INFO_2,
//PRINTER_INFO_4, PRINTER_INFO_5, SetPrinter
// typedef struct _PRINTER_INFO_1 { 
//  DWORD  Flags; 
//  LPTSTR pDescription; 
//  LPTSTR pName; 
//  LPTSTR pComment; 
//} PRINTER_INFO_1, *PPRINTER_INFO_1; 

//  Platform SDK: Windows GDI 
//DEVMODE
//The DEVMODE data structure contains information about the device
//initialization and environment of a printer. 
//typedef struct _devicemode { 
//  BCHAR  dmDeviceName[CCHDEVICENAME]; 
//  WORD   dmSpecVersion; 
//  WORD   dmDriverVersion; 
//  WORD   dmSize; 
//  WORD   dmDriverExtra; 
//  DWORD  dmFields; 
//  union {
//    struct {
//      short dmOrientation;
//      short dmPaperSize;
//      short dmPaperLength;
//      short dmPaperWidth;
//    };
//    POINTL dmPosition;
//  };
//  short  dmScale; 
//  short  dmCopies; 
//  short  dmDefaultSource; 
//  short  dmPrintQuality; 
//  short  dmColor; 
//  short  dmDuplex; 
//  short  dmYResolution; 
//  short  dmTTOption; 
//  short  dmCollate; 
//  BCHAR  dmFormName[CCHFORMNAME]; 
//  WORD  dmLogPixels; 
//  DWORD  dmBitsPerPel; 
//  DWORD  dmPelsWidth; 
//  DWORD  dmPelsHeight; 
//  union {
//    DWORD  dmDisplayFlags; 
//    DWORD  dmNup;
//  }
//  DWORD  dmDisplayFrequency; 
//#if(WINVER >= 0x0400) 
//  DWORD  dmICMMethod;
//  DWORD  dmICMIntent;
//  DWORD  dmMediaType;
//  DWORD  dmDitherType;
//  DWORD  dmReserved1;
//  DWORD  dmReserved2;
//#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
//  DWORD  dmPanningWidth;
//  DWORD  dmPanningHeight;
//#endif
//#endif /* WINVER >= 0x0400 */
//} DEVMODE; 
//Members
//dmDeviceName 
//Specifies the the "friendly" name of the printer; for example,
//"PCL/HP LaserJet" in the case of PCL/HP LaserJet�. This string
//is unique among device drivers. Note that this name may be
//truncated to fit in the dmDeviceName array. 
//dmSpecVersion 
//Specifies the version number of the initialization data
//specification on which the structure is based. To ensure the correct 
//version is used for any operating system, use DM_SPECVERSION. 
//dmDriverVersion 
//Specifies the printer driver version number assigned by the
//printer driver developer. 
//dmSize 
//Specifies the size, in bytes, of the DEVMODE structure, not
//including any private driver-specific data that might follow the
//structure's public members. Set this member to sizeof(DEVMODE)
//to indicate the version of the DEVMODE structure being used. 
//dmDriverExtra 
//Contains the number of bytes of private driver-data that follow
//this structure. If a device driver does not use device-specific
//information, set this member to zero. 
//dmFields 
//Specifies whether certain members of the DEVMODE structure have
//been initialized. If a member is initialized, its corresponding
//bit is set, otherwise the bit is clear. A printer driver
//supports only those DEVMODE members that are appropriate for the
//printer technology. 
//The following values are defined, and are listed here with the
//corresponding structure members. Value Structure member 
//DM_ORIENTATION dmOrientation 
//DM_PAPERSIZE dmPaperSize 
//DM_PAPERLENGTH dmPaperLength 
//DM_PAPERWIDTH dmPaperWidth 
//DM_POSITION dmPosition 
//DM_SCALE dmScale 
//DM_COPIES dmCopies 
//DM_DEFAULTSOURCE dmDefaultSource 
//DM_PRINTQUALITY dmPrintQuality 
//DM_COLOR dmColor 
//DM_DUPLEX dmDuplex 
//DM_YRESOLUTION dmYResolution 
//DM_TTOPTION dmTTOption 
//DM_COLLATE dmCollate 
//DM_FORMNAME dmFormName 
//DM_LOGPIXELS dmLogPixels 
//DM_BITSPERPEL dmBitsPerPel 
//DM_PELSWIDTH dmPelsWidth 
//DM_PELSHEIGHT dmPelsHeight 
//DM_DISPLAYFLAGS dmDisplayFlags 
//DM_NUP dmNup 
//DM_DISPLAYFREQUENCY dmDisplayFrequency 
//DM_ICMMETHOD dmICMMethod 
//DM_ICMINTENT dmICMIntent 
//DM_MEDIATYPE dmMediaType 
//DM_DITHERTYPE dmDitherType 
//DM_PANNINGWIDTH Windows 2000: dmPanningWidth 
//DM_PANNINGHEIGHT Windows 2000: dmPanningHeight 
//dmOrientation 
//For printer devices only, selects the orientation of the paper.
//This member can be either DMORIENT_PORTRAIT (1) or
//DMORIENT_LANDSCAPE (2). 
//dmPaperSize 
//For printer devices only, selects the size of the paper to print
//on. This member can be set to zero if the length and width of
//the paper are both set by the dmPaperLength and dmPaperWidth
//members. Otherwise, the dmPaperSize member can be set to one of
//the following predefined values. Value Meaning 
//DMPAPER_LETTER Letter, 8 1/2- by 11-inches 
//DMPAPER_LEGAL Legal, 8 1/2- by 14-inches 
//DMPAPER_10X14 10- by 14-inch sheet 
//DMPAPER_11X17 11- by 17-inch sheet 
//DMPAPER_12X11 Windows 98, Windows NT 4.0, and later: 12- by
//11-inch sheet 
//DMPAPER_A3 A3 sheet, 297- by 420-millimeters 
//DMPAPER_A3_ROTATED  Windows 98, Windows NT 4.0, and later: A3
//rotated sheet, 420- by 297-millimeters 
//DMPAPER_A4 A4 sheet, 210- by 297-millimeters 
//DMPAPER_A4_ROTATED Windows 98, Windows NT 4.0 and later: A4
//rotated sheet, 297- by 210-millimeters 
//DMPAPER_A4SMALL A4 small sheet, 210- by 297-millimeters 
//DMPAPER_A5 A5 sheet, 148- by 210-millimeters 
//DMPAPER_A5_ROTATED Windows 98, Windows NT 4.0, and later: A5
//rotated sheet, 210- by 148-millimeters 
//DMPAPER_A6 Windows 98, Windows NT 4.0, and later: A6 sheet, 105-
//by 148-millimeters 
//DMPAPER_A6_ROTATED Windows 98, Windows NT 4.0, and later: A6
//rotated sheet, 148- by 105-millimeters 
//DMPAPER_B4 B4 sheet, 250- by 354-millimeters 
//DMPAPER_B4_JIS_ROTATED Windows 98, Windows NT 4.0, and later: B4
//(JIS) rotated sheet, 364- by 257-millimeters 
//DMPAPER_B5 B5 sheet, 182- by 257-millimeter paper 
//DMPAPER_B5_JIS_ROTATED Windows 98, Windows NT 4.0, and later: B5
//(JIS) rotated sheet, 257- by 182-millimeters 
//DMPAPER_B6_JIS Windows 98, Windows NT 4.0, and later: B6 (JIS)
//sheet, 128- by 182-millimeters 
//DMPAPER_B6_JIS_ROTATED Windows 98, Windows NT 4.0, and later: B6
//(JIS) rotated sheet, 182- by 128-millimeters 
//DMPAPER_CSHEET C Sheet, 17- by 22-inches 
//DMPAPER_DBL_JAPANESE_POSTCARD  Windows 98, Windows NT 4.0, and
//later: Double Japanese Postcard, 200- by 148-millimeters  
//DMPAPER_DBL_JAPANESE_POSTCARD_ROTATED  Windows 98, Windows NT
//4.0, and later: Double Japanese Postcard Rotated, 148- by
//200-millimeters  
//DMPAPER_DSHEET D Sheet, 22- by 34-inches 
//DMPAPER_ENV_9 #9 Envelope, 3 7/8- by 8 7/8-inches 
//DMPAPER_ENV_10 #10 Envelope, 4 1/8- by 9 1/2-inches 
//DMPAPER_ENV_11 #11 Envelope, 4 1/2- by 10 3/8-inches 
//DMPAPER_ENV_12 #12 Envelope, 4 3/4- by 11-inches 
//DMPAPER_ENV_14 #14 Envelope, 5- by 11 1/2-inches 
//DMPAPER_ENV_C5 C5 Envelope, 162- by 229-millimeters 
//DMPAPER_ENV_C3 C3 Envelope, 324- by 458-millimeters 
//DMPAPER_ENV_C4 C4 Envelope, 229- by 324-millimeters 
//DMPAPER_ENV_C6 C6 Envelope, 114- by 162-millimeters 
//DMPAPER_ENV_C65 C65 Envelope, 114- by 229-millimeters 
//DMPAPER_ENV_B4 B4 Envelope, 250- by 353-millimeters 
//DMPAPER_ENV_B5 B5 Envelope, 176- by 250-millimeters 
//DMPAPER_ENV_B6 B6 Envelope, 176- by 125-millimeters 
//DMPAPER_ENV_DL DL Envelope, 110- by 220-millimeters 
//DMPAPER_ENV_ITALY Italy Envelope, 110- by 230-millimeters 
//DMPAPER_ENV_MONARCH Monarch Envelope, 3 7/8- by 7 1/2-inches 
//DMPAPER_ENV_PERSONAL 6 3/4 Envelope, 3 5/8- by 6 1/2-inches 
//DMPAPER_ESHEET E Sheet, 34- by 44-inches 
//DMPAPER_EXECUTIVE Executive, 7 1/4- by 10 1/2-inches 
//DMPAPER_FANFOLD_US US Std Fanfold, 14 7/8- by 11-inches 
//DMPAPER_FANFOLD_STD_GERMAN German Std Fanfold, 8 1/2- by
//12-inches 
//DMPAPER_FANFOLD_LGL_GERMAN German Legal Fanfold, 8 �- by
//13-inches 
//DMPAPER_FOLIO Folio, 8 1/2- by 13-inch paper 
//DMPAPER_JAPANESE_POSTCARD_ROTATED  Windows 98, Windows NT 4.0,
//and later: Japanese Postcard Rotated, 148- by 100-millimeters  
//DMPAPER_JENV_CHOU3 Windows 98, Windows NT 4.0, and later:
//Japanese Envelope Chou #3 
//DMPAPER_JENV_CHOU3_ROTATED Windows 98, Windows NT 4.0, and
//later: Japanese Envelope Chou #3 Rotated 
//DMPAPER_JENV_CHOU4 Windows 98, Windows NT 4.0, and later:
//Japanese Envelope Chou #4 
//DMPAPER_JENV_CHOU4_ROTATED Windows 98, Windows NT 4.0, and
//later: Japanese Envelope Chou #4 Rotated 
//DMPAPER_JENV_KAKU2 Windows 98, Windows NT 4.0, and later:
//Japanese Envelope Kaku #2 
//DMPAPER_JENV_KAKU2_ROTATED Windows 98, Windows NT 4.0, and
//later: Japanese Envelope Kaku #2 Rotated 
//DMPAPER_JENV_KAKU3 Windows 98, Windows NT 4.0, and later:
//Japanese Envelope Kaku #3 
//DMPAPER_JENV_KAKU3_ROTATED Windows 98, Windows NT 4.0, and
//later: Japanese Envelope Kaku #3 Rotated 
//DMPAPER_JENV_YOU4 Windows 98, Windows NT 4.0, and later:
//Japanese Envelope You #4 
//DMPAPER_JENV_YOU4_ROTATED Windows 98, Windows NT 4.0, and later:
//Japanese Envelope You #4 Rotated 
//DMPAPER_LAST Windows 2000: DMPAPER_PENV_10_ROTATED 
//DMPAPER_LEDGER Ledger, 17- by 11-inches 
//DMPAPER_LETTER_ROTATED Letter Rotated 11 by 8 1/2 11 inches  
//DMPAPER_LETTERSMALL Letter Small, 8 1/2- by 11-inches 
//DMPAPER_NOTE Note, 8 1/2- by 11-inches 
//DMPAPER_P16K Windows 98, Windows NT 4.0, and later: PRC 16K,
//146- by 215-millimeters 
//DMPAPER_P16K_ROTATED Windows 98, Windows NT 4.0, and later: PRC
//16K Rotated, 215- by 146-millimeters 
//DMPAPER_P32K Windows 98, Windows NT 4.0, and later: PRC 32K, 97-
//by 151-millimeters 
//DMPAPER_P32K_ROTATED Windows 98, Windows NT 4.0, and later: PRC
//32K Rotated, 151- by 97-millimeters  
//DMPAPER_P32KBIG Windows 98, Windows NT 4.0, and later: PRC
//32K(Big) 97- by 151-millimeters 
//DMPAPER_P32KBIG_ROTATED Windows 98, Windows NT 4.0, and later:
//PRC 32K(Big) Rotated, 151- by 97-millimeters 
//DMPAPER_PENV_1 Windows 98, Windows NT 4.0, and later: PRC
//Envelope #1, 102- by 165-millimeters 
//DMPAPER_PENV_1_ROTATED Windows 98, Windows NT 4.0, and later:
//PRC Envelope #1 Rotated, 165- by 102-millimeters 
//DMPAPER_PENV_2 Windows 98, Windows NT 4.0, and later: PRC
//Envelope #2, 102- by 176-millimeters 
//DMPAPER_PENV_2_ROTATED Windows 98, Windows NT 4.0, and later:
//PRC Envelope #2 Rotated, 176- by 102-millimeters  
//DMPAPER_PENV_3 Windows 98, Windows NT 4.0, and later: PRC
//Envelope #3, 125- by 176-millimeters 
//DMPAPER_PENV_3_ROTATED Windows 98, Windows NT 4.0, and later:
//PRC Envelope #3 Rotated, 176- by 125-millimeters 
//DMPAPER_PENV_4 Windows 98, Windows NT 4.0, and later: PRC
//Envelope #4, 110- by 208-millimeters 
//DMPAPER_PENV_4_ROTATED Windows 98, Windows NT 4.0, and later:
//PRC Envelope #4 Rotated, 208- by 110-millimeters 
//DMPAPER_PENV_5 Windows 98, Windows NT 4.0, and later: PRC
//Envelope #5, 110- by 220-millimeters 
//DMPAPER_PENV_5_ROTATED Windows 98, Windows NT 4.0, and later:
//PRC Envelope #5 Rotated, 220- by 110-millimeters  
//DMPAPER_PENV_6 Windows 98, Windows NT 4.0, and later: PRC
//Envelope #6, 120- by 230-millimeters 
//DMPAPER_PENV_6_ROTATED Windows 98, Windows NT 4.0, and later:
//PRC Envelope #6 Rotated, 230- by 120-millimeters 
//DMPAPER_PENV_7 Windows 98, Windows NT 4.0, and later: PRC
//Envelope #7, 160- by 230-millimeters 
//DMPAPER_PENV_7_ROTATED Windows 98, Windows NT 4.0, and later:
//PRC Envelope #7 Rotated, 230- by 160-millimeters 
//DMPAPER_PENV_8 Windows 98, Windows NT 4.0, and later: PRC
//Envelope #8, 120- by 309-millimeters 
//DMPAPER_PENV_8_ROTATED Windows 98, Windows NT 4.0, and later:
//PRC Envelope #8 Rotated, 309- by 120-millimeters 
//DMPAPER_PENV_9 Windows 98, Windows NT 4.0, and later: PRC
//Envelope #9, 229- by 324-millimeters 
//DMPAPER_PENV_9_ROTATED Windows 98, Windows NT 4.0, and later:
//PRC Envelope #9 Rotated, 324- by 229-millimeters 
//DMPAPER_PENV_10 Windows 98, Windows NT 4.0, and later: PRC
//Envelope #10, 324- by 458-millimeters 
//DMPAPER_PENV_10_ROTATED Windows 98, Windows NT 4.0, and later:
//PRC Envelope #10 Rotated, 458- by 324-millimeters  
//DMPAPER_QUARTO Quarto, 215- by 275-millimeter paper 
//DMPAPER_STATEMENT Statement, 5 1/2- by 8 1/2-inches 
//DMPAPER_TABLOID Tabloid, 11- by 17-inches 
//dmPaperLength 
//For printer devices only, overrides the length of the paper
//specified by the dmPaperSize member, either for custom paper
//sizes or for devices such as dot-matrix printers that can print
//on a page of arbitrary length. These values, along with all
//other values in this structure that specify a physical length,
//are in tenths of a millimeter. 
//dmPaperWidth 
//For printer devices only, overrides the width of the paper
//specified by the dmPaperSize member. 
//dmPosition 
//Windows 98, Windows 2000: For display devices only, a POINTL
//structure that indicates the positional coordinates of the
//display device in reference to the desktop area. The primary
//display device is always located at coordinates (0,0). 
//dmScale 
//Specifies the factor by which the printed output is to be
//scaled. The apparent page size is scaled from the physical page
//size by a factor of dmScale/100. For example, a letter-sized
//page with a dmScale value of 50 would contain as much data as a
//page of 17- by 22-inches because the output text and graphics
//would be half their original height and width. 
//dmCopies 
//Selects the number of copies printed if the device supports
//multiple-page copies. 
//dmDefaultSource 
//Specifies the paper source. To retrieve a list of the available
//paper sources for a printer, use the DeviceCapabilities function
//with the DC_BINS flag. 
//This member can be one of the following values, or it can be a
//device-specific value greater than or equal to DMBIN_USER. 
//DMBIN_ONLYONE
//DMBIN_LOWER
//DMBIN_MIDDLE
//DMBIN_MANUAL
//DMBIN_ENVELOPE
//DMBIN_ENVMANUAL
//DMBIN_AUTO 
//DMBIN_TRACTOR
//DMBIN_SMALLFMT
//DMBIN_LARGEFMT 
//DMBIN_LARGECAPACITY
//DMBIN_CASSETTE
//DMBIN_FORMSOURCE 
//dmPrintQuality 
//Specifies the printer resolution. There are four predefined
//device-independent values: 
//DMRES_HIGH
//DMRES_MEDIUM
//DMRES_LOW
//DMRES_DRAFT 
//If a positive value is specified, it specifies the number of
//dots per inch (DPI) and is therefore device dependent. 
//dmColor 
//Switches between color and monochrome on color printers.
//Following are the possible values: 
//DMCOLOR_COLOR
//DMCOLOR_MONOCHROME 
//dmDuplex 
//Selects duplex or double-sided printing for printers capable of
//duplex printing. Following are the possible values. Value
//Meaning 
//DMDUP_SIMPLEX Normal (nonduplex) printing. 
//DMDUP_HORIZONTAL Short-edge binding, that is, the long edge of
//the page is horizontal.  
//DMDUP_VERTICAL Long-edge binding, that is, the long edge of the
//page is vertical.  
//dmYResolution 
//Specifies the y-resolution, in dots per inch, of the printer. If
//the printer initializes this member, the dmPrintQuality member
//specifies the x-resolution, in dots per inch, of the printer. 
//dmTTOption 
//Specifies how TrueType� fonts should be printed. This member can
//be one of the following values. Value Meaning 
//DMTT_BITMAP Prints TrueType fonts as graphics. This is the
//default action for dot-matrix printers. 
//DMTT_DOWNLOAD Downloads TrueType fonts as soft fonts. This is
//the default action for Hewlett-Packard printers that use Printer
//Control Language (PCL). 
//DMTT_DOWNLOAD_OUTLINE Window 95/98, Windows NT 4.0, and later:
//Downloads TrueType fonts as outline soft fonts. 
//DMTT_SUBDEV Substitutes device fonts for TrueType fonts. This is
//the default action for PostScript� printers. 
//dmUnusedPadding 
//Used to align the structure to a DWORD boundary. This should not
//be used or referenced. Its name and usage is reserved, and can
//change in future releases. 
//dmCollate 
//Specifies whether collation should be used when printing
//multiple copies. (This member is ignored unless the printer
//driver indicates support for collation by setting the dmFields
//member to DM_COLLATE.) This member can be be one of the
//following values. Value Meaning 
//DMCOLLATE_TRUE Collate when printing multiple copies. 
//DMCOLLATE_FALSE Do not collate when printing multiple copies. 
//Using DMCOLLATE_TRUE provides faster, more efficient output for
//collation, since the data is sent to the device driver just
//once, no matter how many copies are required. The printer is
//told to simply print the page again. 
//dmFormName 
//Windows NT/Windows 2000: Specifies the name of the form to use;
//for example, "Letter" or "Legal". A complete set of names can be retrieved 
//by using the EnumForms function. 
//Windows 95: Printer drivers do not use this member. 
//dmLogPixels 
//Specifies the number of pixels per logical inch. Printer drivers
//do not use this member. 
//dmBitsPerPel 
//Specifies the color resolution, in bits per pixel, of the
//display device (for example: 4 bits for 16 colors, 8 bits for
//256 colors, or 16 bits for 65,536 colors). Display drivers use
//this member, for example, in the ChangeDisplaySettings function.
//Printer drivers do not use this member. 
//dmPelsWidth 
//Specifies the width, in pixels, of the visible device surface.
//Display drivers use this member, for example, in the
//ChangeDisplaySettings function. Printer drivers do not use this
//member. 
//dmPelsHeight 
//Specifies the height, in pixels, of the visible device surface.
//Display drivers use this member, for example, in the
//ChangeDisplaySettings function. Printer drivers do not use this
//member. 
//dmDisplayFlags 
//Specifies the device's display mode. This member can be a
//combination of the following values. Value Meaning 
//DM_GRAYSCALE Specifies that the display is a noncolor device. If
//this flag is not set, color is assumed. 
//DM_INTERLACED Specifies that the display mode is interlaced. If
//the flag is not set, noninterlaced is assumed. 
//Display drivers use this member, for example, in the
//ChangeDisplaySettings function. Printer drivers do not use this
//member. 
//dmNup 
//Specifies where the NUP is done. It can be one of the following.
//Value Meaning 
//DMNUP_SYSTEM The print spooler does the NUP.  
//DMNUP_ONEUP The application does the NUP. 
//dmDisplayFrequency 
//Specifies the frequency, in hertz (cycles per second), of the
//display device in a particular mode. This value is also known as
//the display device's vertical refresh rate. Display drivers use
//this member. It is used, for example, in the
//ChangeDisplaySettings function. Printer drivers do not use this
//member. 
//When you call the EnumDisplaySettings function, the
//dmDisplayFrequency member may return with the value 0 or 1.
//These values represent the display hardware's default refresh
//rate. This default rate is typically set by switches on a
//display card or computer motherboard, or by a configuration
//program that does not use Win32 display functions such as
//ChangeDisplaySettings. 
//dmICMMethod 
//Windows 95 and later; Windows 2000: 
//Specifies how ICM is handled. For a non-ICM application, this
//member determines if ICM is enabled or disabled. For ICM
//applications, the system examines this member to determine how
//to handle ICM support. This member can be one of the following
//predefined values, or a driver-defined value greater than or
//equal to the value of DMICMMETHOD_USER. Value Meaning 
//DMICMMETHOD_NONE Specifies that ICM is disabled. 
//DMICMMETHOD_SYSTEM Specifies that ICM is handled by Windows. 
//DMICMMETHOD_DRIVER Specifies that ICM is handled by the device
//driver. 
//DMICMMETHOD_DEVICE Specifies that ICM is handled by the
//destination device. 
//The printer driver must provide a user interface for setting
//this member. Most printer drivers support only the
//DMICMMETHOD_SYSTEM or DMICMMETHOD_NONE value. Drivers for
//PostScript printers support all values. 
//dmICMIntent 
//Windows 95/98, Windows 2000: 
//Specifies which color matching method, or intent, should be used
//by default. This member is primarily for non-ICM applications.
//ICM applications can establish intents by using the ICM
//functions. This member can be one of the following predefined
//values, or a driver defined value greater than or equal to the
//value of DMICM_USER. Value Meaning 
//DMICM_ABS_COLORIMETRIC Color matching should optimize to match
//the exact color requested without white point mapping. This
//value is most appropriate for use with proofing. 
//DMICM_COLORMETRIC Color matching should optimize to match the
//exact color requested. This value is most appropriate for use
//with business logos or other images when an exact color match is
//desired. 
//DMICM_CONTRAST Color matching should optimize for color
//contrast. This value is the most appropriate choice for scanned
//or photographic images when dithering is desired. 
//DMICM_SATURATE Color matching should optimize for color
//saturation. This value is the most appropriate choice for
//business graphs when dithering is not desired. 
//dmMediaType 
//Windows 95/98, Windows 2000: 
//Specifies the type of media being printed on. The member can be
//one of the following predefined values, or a driver-defined
//value greater than or equal to the value of DMMEDIA_USER. Value
//Meaning 
//DMMEDIA_STANDARD Plain paper. 
//DMMEDIA_GLOSSY Glossy paper. 
//DMMEDIA_TRANSPARENCY Transparent film. 
//dmDitherType 
//Windows 95/98, Windows 2000: 
//Specifies how dithering is to be done. The member can be one of
//the following predefined values, or a driver-defined value
//greater than or equal to the value of DMDITHER_USER. Value
//Meaning 
//DMDITHER_NONE No dithering. 
//DMDITHER_COARSE Dithering with a coarse brush. 
//DMDITHER_FINE Dithering with a fine brush. 
//DMDITHER_LINEART Line art dithering, a special dithering method
//that produces well defined borders between black, white, and
//gray scalings. It is not suitable for images that include
//continuous graduations in intensisty and hue, such as scanned
//photographs. 
//DMDITHER_ERRORDIFFUSION Windows 95/98: Dithering in which an
//algorithm is used to spread, or diffuse, the error of
//approximating a specified color over adjacent pixels. In
//contrast, DMDITHER_COARSE, DMDITHER_FINE, and DMDITHER_LINEART
//use patterned halftoning to approximate a color.. 
//DMDITHER_GRAYSCALE Device does grayscaling. 
//dmReserved1 
//Windows 95/98, Windows 2000: Not used; must be zero. 
//dmReserved2 
//Windows 95/98, Windows 2000: Not used; must be zero. 
//dmPanningWidth 
//Windows NT/Windows 2000: This member must be zero. 
//Windows 95/98: This member is not supported. 
//dmPanningHeight 
//Windows NT/Windows 2000: This member must be zero. 
//Windows 95/98: This member is not supported. 
//Remarks
//A device driver's private data follows the public portion of the
//DEVMODE structure. The size of the public data can vary for
//different versions of the structure. The dmSize member specifies
//the number of bytes of public data, and the dmDriverExtra member
//specifies the number of bytes of private data.
//Requirements 
//  Windows NT/2000: Requires Windows NT 3.1 or later.
//  Windows 95/98: Requires Windows 95 or later.
//  Header: Declared in Wingdi.h; include Windows.h.
//  Unicode: Declared as Unicode and ANSI structures.
//See Also
//Printing and Print Spooler Overview, Printing and Print Spooler
//Structures, AdvancedDocumentProperties, ChangeDisplaySettings,
//CreateDC, CreateIC, DeviceCapabilities, DocumentProperties,
// EnumDisplaySettings, OpenPrinter 
//Built on Wednesday, July 12, 2000Requirements 
//  Windows NT/2000: Requires Windows NT 3.1 or later.
//  Windows 95/98: Requires Windows 95 or later.
//  Header: Declared in Wingdi.h; include Windows.h.
//  Unicode: Declared as Unicode and ANSI structures.
//See Also
//Printing and Print Spooler Overview, Printing and Print Spooler
//Structures, AdvancedDocumentProperties, ChangeDisplaySettings,
//CreateDC, CreateIC, DeviceCapabilities, DocumentProperties,
// EnumDisplaySettings, OpenPrinter
// NOTE ALSO MUST USE -
// HDC CreateIC(
//  LPCTSTR lpszDriver,       // driver name
//  LPCTSTR lpszDevice,       // device name
//  LPCTSTR lpszOutput,       // port or file name
//  CONST DEVMODE *lpdvmInit );  // optional initialization data
// Like in ewm project DEVMODE and DEVNAMES
//	pDevMode = (LPDEVMODE) GlobalLock(ppd->hDevMode);
//	pDevNames = (LPDEVNAMES) GlobalLock(ppd->hDevNames);
//	ppp->fr.hdcTarget = CreateIC((LPTSTR) pDevNames + pDevNames->wDriverOffset,
//								(LPTSTR) pDevNames + pDevNames->wDeviceOffset,
//								(LPTSTR) pDevNames + pDevNames->wOutputOffset,
//								pDevMode);
//	GlobalUnlock(ppp->predoc->mc_sPD.hDevNames);
//	GlobalUnlock(ppp->predoc->mc_sPD.hDevMode);
//	GlobalUnlock(ppd->hDevNames);
//	GlobalUnlock(ppd->hDevMode);

//	if(!ppp->fr.hdcTarget)
// {
      // NO INFORMATION DEVICE CONTEXT!!!
//		goto err;
//   }

	// does this matter?
//	SetMapMode(ppp->fr.hdcTarget, MM_TEXT);

//	if( Escape(ppp->fr.hdcTarget, GETPHYSPAGESIZE, 0, NULL, &pt) > 0 )
//	{
//		const LONG xPerInch = GetDeviceCaps(ppp->fr.hdcTarget, LOGPIXELSX);
//		const LONG yPerInch = GetDeviceCaps(ppp->fr.hdcTarget, LOGPIXELSY);

//		ppp->rc.left = ppp->rc.top = 0;
//		ppp->dxPage = (pt.x * 1440l) / xPerInch;
//		ppp->rc.right = (INT) ppp->dxPage;
//		ppp->fr.rcPage.left = 0;
//		ppp->fr.rcPage.right = ppp->rc.right;
		// leave 1.25" (1800 twips) margins if that will leave >= 1"
//		if(ppp->rc.right >= 1800 + 1440 + 1800)
//			ppp->rc.right -= (ppp->rc.left = 1800);
//		ppp->dyPage = (pt.y * 1440l) / yPerInch;
//		ppp->rc.bottom = (INT) ppp->dyPage;
//		ppp->fr.rcPage.top = 0;
//		ppp->fr.rcPage.bottom = ppp->rc.bottom;
		// leave 1" (1440 twips) margins if that will leave >= 1"
//		if(ppp->rc.bottom >= 1440 + 1440 + 1440)
//			ppp->rc.bottom -= (ppp->rc.top = 1440);
//	}
//	else
//	{
//		const LONG xPerInch = GetDeviceCaps(ppp->fr.hdcTarget, LOGPIXELSX);
//		const LONG yPerInch = GetDeviceCaps(ppp->fr.hdcTarget, LOGPIXELSY);
//		const LONG dxRes = GetDeviceCaps(ppp->fr.hdcTarget, HORZRES);
//		const LONG dyRes = GetDeviceCaps(ppp->fr.hdcTarget, VERTRES);
//
//		ppp->rc.left = ppp->rc.top = 0;
//		ppp->dxPage = (dxRes * 1440l) / xPerInch;
//		ppp->rc.right = (INT) ppp->dxPage;
//		ppp->dyPage = (dyRes * 1440l) / yPerInch;
//		ppp->rc.bottom = (INT) ppp->dyPage;
//		ppp->fr.rcPage = ppp->rc;
//	}
// to get the ACTUAL information about the SELECTED printer
// and then this must be used to "render" the image on the FRAME
// =============================================================

//Status 
//Specifies the printer status. This member can be any reasonable
//combination of the following values. Value Meaning 
IDSTG sPrtStatus[] = {
   { PRINTER_STATUS_BUSY, "busy" },
   { PRINTER_STATUS_DOOR_OPEN, "door open" },
   { PRINTER_STATUS_ERROR, "error" },
   { PRINTER_STATUS_INITIALIZING, "initializing" },
   { PRINTER_STATUS_IO_ACTIVE, "active" },
   { PRINTER_STATUS_MANUAL_FEED, "manual feed" },
   { PRINTER_STATUS_NO_TONER, "out of toner" },
   { PRINTER_STATUS_NOT_AVAILABLE, "not available" },
   { PRINTER_STATUS_OFFLINE, "offline" },
   { PRINTER_STATUS_OUT_OF_MEMORY, "out of memory" },
   { PRINTER_STATUS_OUTPUT_BIN_FULL, "bin full" },
   { PRINTER_STATUS_PAGE_PUNT, "cannot print" },   //  the current page.
//Windows 95: Indicates the page is being "punted" (that is, not
//printed) because it is too complex for the printer to print.
   { PRINTER_STATUS_PAPER_JAM, "Paper jam" },
   { PRINTER_STATUS_PAPER_OUT, "out of paper" },
   { PRINTER_STATUS_PAPER_PROBLEM, "paper problem" },
   { PRINTER_STATUS_PAUSED, "paused" },
   { PRINTER_STATUS_PENDING_DELETION, "pending deletion" }, //  The printer is deleting a ...
   { PRINTER_STATUS_POWER_SAVE, "in power save" },
   { PRINTER_STATUS_PRINTING, "printing" },
   { PRINTER_STATUS_PROCESSING, "processing" }, // The printer is processing a print job.
   { PRINTER_STATUS_SERVER_UNKNOWN, "unknown status" },
   { PRINTER_STATUS_TONER_LOW, "toner low" },
   { PRINTER_STATUS_USER_INTERVENTION, "need intervention" }, // The printer has an error that
//requires the user to do something. 
   { PRINTER_STATUS_WAITING, "waiting" },
   { PRINTER_STATUS_WARMING_UP, "warming up" },
   { 0,                          0 }
};

int   AddPStatus( LPTSTR lpb, UINT status )
{
   int      iRet = 0;
   UINT     i, ui;
   LPTSTR   lps;

   if( ui = status )
   {
      PIDSTG pids = &sPrtStatus[0];
      while( lps = pids->id_psNm )
      {
         if( pids->id_uiID & ui )
         {
            if( i = lstrlen(lps) )
            {
               if(iRet)
                  lstrcat(lpb, ", ");
               else
                  lstrcat(lpb, "S=");

               lstrcat(lpb,lps);

               iRet += i;
            }

            ui &= ~(pids->id_uiID);

            if( ui == 0 )

               break;
         }

         pids++;
      }

      if( ui )
      {
         if(iRet)
            lstrcat(lpb, ", ");
         else
            lstrcat(lpb, "S=");

         wsprintf(EndBuf(lpb), "+%#x?", ui );

      }
   }
   return iRet;
}

IDSTG sPrtPaper[] = {
   // paper selections
	{ DMPAPER_LETTER             ,"1  Letter 8 1/2 x 11 in"},
	{ DMPAPER_LETTERSMALL        ,"2  Letter Small 8 1/2 x 11 in"},
	{ DMPAPER_TABLOID            ,"3  Tabloid 11 x 17 in"},
	{ DMPAPER_LEDGER             ,"4  Ledger 17 x 11 in"},
	{ DMPAPER_LEGAL              ,"5  Legal 8 1/2 x 14 in"},
	{ DMPAPER_STATEMENT          ,"6  Statement 5 1/2 x 8 1/2 in"},
	{ DMPAPER_EXECUTIVE          ,"7  Executive 7 1/4 x 10 1/2 in"},
	{ DMPAPER_A3                 ,"8  A3 297 x 420 mm"},
	{ DMPAPER_A4                 ,"9  A4 210 x 297 mm"},
	{ DMPAPER_A4SMALL            ,"10  A4 Small 210 x 297 mm"},
	{ DMPAPER_A5                 ,"11  A5 148 x 210 mm"},
	{ DMPAPER_B4                 ,"12  B4 (JIS) 250 x 354"},
	{ DMPAPER_B5                 ,"13  B5 (JIS) 182 x 257 mm"},
	{ DMPAPER_FOLIO              ,"14  Folio 8 1/2 x 13 in"},
	{ DMPAPER_QUARTO             ,"15  Quarto 215 x 275 mm"},
	{ DMPAPER_10X14              ,"16  10x14 in"},
	{ DMPAPER_11X17              ,"17  11x17 in"},
	{ DMPAPER_NOTE               ,"18  Note 8 1/2 x 11 in"},
	{ DMPAPER_ENV_9              ,"19  Env. 9 3 7/8 x 8 7/8"},
	{ DMPAPER_ENV_10             ,"20  Env. 10 4 1/8 x 9 1/2"},
	{ DMPAPER_ENV_11             ,"21  Env. 11 4 1/2 x 10 3/8"},
	{ DMPAPER_ENV_12             ,"22  Env. 12 4 1/2 x 11"},
	{ DMPAPER_ENV_14             ,"23  Env. 14 5 x 11 1/2"},
	{ DMPAPER_CSHEET             ,"24  C size sheet"},
	{ DMPAPER_DSHEET             ,"25  D size sheet"},
	{ DMPAPER_ESHEET             ,"26  E size sheet"},
	{ DMPAPER_ENV_DL             ,"27  Env. DL 110 x 220mm"},
	{ DMPAPER_ENV_C5             ,"28  Env. C5 162 x 229 mm"},
	{ DMPAPER_ENV_C3             ,"29  Env. C3  324 x 458 mm"},
	{ DMPAPER_ENV_C4             ,"30  Env. C4  229 x 324 mm"},
	{ DMPAPER_ENV_C6             ,"31  Env. C6  114 x 162 mm"},
	{ DMPAPER_ENV_C65            ,"32  Env. C65 114 x 229 mm"},
	{ DMPAPER_ENV_B4             ,"33  Env. B4  250 x 353 mm"},
	{ DMPAPER_ENV_B5             ,"34  Env. B5  176 x 250 mm"},
	{ DMPAPER_ENV_B6             ,"35  Env. B6  176 x 125 mm"},
	{ DMPAPER_ENV_ITALY          ,"36  Env. 110 x 230 mm"},
	{ DMPAPER_ENV_MONARCH        ,"37  Env. Monarch 3.875 x 7.5 in"},
	{ DMPAPER_ENV_PERSONAL       ,"38  6 3/4 Env. 3 5/8 x 6 1/2 in"},
	{ DMPAPER_FANFOLD_US         ,"39  US Std Fanfold 14 7/8 x 11 in"},
	{ DMPAPER_FANFOLD_STD_GERMAN ,"40  Ger. Std Fanfold 8 1/2 x 12 in"},
	{ DMPAPER_FANFOLD_LGL_GERMAN ,"41  Ger. Legal Fanfold 8 1/2 x 13 in"},

#if(WINVER >= 0x0400)
	{ DMPAPER_ISO_B4             ,"42  B4 (ISO) 250 x 353 mm"},
	{ DMPAPER_JAPANESE_POSTCARD  ,"43  Jap. Postcard 100 x 148 mm"},
	{ DMPAPER_9X11               ,"44  9 x 11 in"},
	{ DMPAPER_10X11              ,"45  10 x 11 in"},
	{ DMPAPER_15X11              ,"46  15 x 11 in"},
	{ DMPAPER_ENV_INVITE         ,"47  Env. Invite 220 x 220 mm"},
	{ DMPAPER_RESERVED_48        ,"48  RESERVED--DO NOT USE"},
	{ DMPAPER_RESERVED_49        ,"49  RESERVED--DO NOT USE"},
	{ DMPAPER_LETTER_EXTRA       ,"50  Letter Extra 9 \275 x 12 in"},
	{ DMPAPER_LEGAL_EXTRA        ,"51  Legal Extra 9 \275 x 15 in"},
	{ DMPAPER_TABLOID_EXTRA      ,"52  Tabloid Extra 11.69 x 18 in"},
	{ DMPAPER_A4_EXTRA           ,"53  A4 Extra 9.27 x 12.69 in"},
	{ DMPAPER_LETTER_TRANSVERSE  ,"54  Letter Transverse 8 \275 x 11 in"},
	{ DMPAPER_A4_TRANSVERSE      ,"55  A4 Transverse 210 x 297 mm"},
	{ DMPAPER_LETTER_EXTRA_TRANSVERSE ,"56 Letter Extra Transverse 9\275 x 12 in"},
	{ DMPAPER_A_PLUS             ,"57  SuperA/SuperA/A4 227 x 356 mm"},
	{ DMPAPER_B_PLUS             ,"58  SuperB/SuperB/A3 305 x 487 mm"},
	{ DMPAPER_LETTER_PLUS        ,"59  Letter Plus 8.5 x 12.69 in"},
	{ DMPAPER_A4_PLUS            ,"60  A4 Plus 210 x 330 mm"},
	{ DMPAPER_A5_TRANSVERSE      ,"61  A5 Transverse 148 x 210 mm"},
	{ DMPAPER_B5_TRANSVERSE      ,"62  B5 (JIS) Transverse 182 x 257 mm"},
	{ DMPAPER_A3_EXTRA           ,"63  A3 Extra 322 x 445 mm"},
	{ DMPAPER_A5_EXTRA           ,"64  A5 Extra 174 x 235 mm"},
	{ DMPAPER_B5_EXTRA           ,"65  B5 (ISO) Extra 201 x 276 mm"},
	{ DMPAPER_A2                 ,"66  A2 420 x 594 mm"},
	{ DMPAPER_A3_TRANSVERSE      ,"67  A3 Transverse 297 x 420 mm"},
	{ DMPAPER_A3_EXTRA_TRANSVERSE,"68  A3 Extra Transverse 322 x 445 mm"},
#endif // WINVER >= 0x0400
                                    
#if ( WINVER >= 0x0500 )
   { DMPAPER_DBL_JAPANESE_POSTCARD, "69 Japanese Double Postcard 200 x 148 mm" },
   { DMPAPER_A6, "70 A6 105 x 148 mm" },
   { DMPAPER_JENV_KAKU2, "71 Japanese Envelope Kaku #2" },
   { DMPAPER_JENV_KAKU3, "72 Japanese Envelope Kaku #3" },
   { DMPAPER_JENV_CHOU3, "73 Japanese Envelope Chou #3" },
   { DMPAPER_JENV_CHOU4, "74 Japanese Envelope Chou #4" },
   { DMPAPER_LETTER_ROTATED, "75 Letter Rotated 11 x 8 1/2 11 in" },
   { DMPAPER_A3_ROTATED, "76 A3 Rotated 420 x 297 mm" },
   { DMPAPER_A4_ROTATED, "77 A4 Rotated 297 x 210 mm" },
   { DMPAPER_A5_ROTATED, "78 A5 Rotated 210 x 148 mm" },
   { DMPAPER_B4_JIS_ROTATED, "79 B4 (JIS) Rotated 364 x 257 mm" },
   { DMPAPER_B5_JIS_ROTATED, "80 B5 (JIS) Rotated 257 x 182 mm" },
   { DMPAPER_JAPANESE_POSTCARD_ROTATED, "81 Japanese Postcard Rotated 148 x 100 mm" },
   { DMPAPER_DBL_JAPANESE_POSTCARD_ROTATED, "82 Double Japanese Postcard Rotated 148 x 200 mm" },
   { DMPAPER_A6_ROTATED, "83 A6 Rotated 148 x 105 mm" },
   { DMPAPER_JENV_KAKU2_ROTATED, "84 Japanese Envelope Kaku #2 Rotated" },
   { DMPAPER_JENV_KAKU3_ROTATED, "85 Japanese Envelope Kaku #3 Rotated" },
   { DMPAPER_JENV_CHOU3_ROTATED, "86 Japanese Envelope Chou #3 Rotated" },
   { DMPAPER_JENV_CHOU4_ROTATED, "87 Japanese Envelope Chou #4 Rotated" },
   { DMPAPER_B6_JIS, "88 B6 (JIS) 128 x 182 mm" },
   { DMPAPER_B6_JIS_ROTATED, "89 B6 (JIS) Rotated 182 x 128 mm" },
   { DMPAPER_12X11, "90 12 x 11 in" },
   { DMPAPER_JENV_YOU4, "91 Japanese Envelope You #4" },
   { DMPAPER_JENV_YOU4_ROTATED, "92 Japanese Envelope You #4 Rotated" },
   { DMPAPER_P16K, "93 PRC 16K 146 x 215 mm" },
   { DMPAPER_P32K, "94 PRC 32K 97 x 151 mm" },
   { DMPAPER_P32KBIG, "95 PRC 32K(Big) 97 x 151 mm" },
   { DMPAPER_PENV_1, "96 PRC Envelope #1 102 x 165 mm" },
   { DMPAPER_PENV_2, "97 PRC Envelope #2 102 x 176 mm" },
   { DMPAPER_PENV_3, "98 PRC Envelope #3 125 x 176 mm" },
   { DMPAPER_PENV_4, "99 PRC Envelope #4 110 x 208 mm" },
   { DMPAPER_PENV_5, "100 PRC Envelope #5 110 x 220 mm" },
   { DMPAPER_PENV_6, "101 PRC Envelope #6 120 x 230 mm" },
   { DMPAPER_PENV_7, "102 PRC Envelope #7 160 x 230 mm" },
   { DMPAPER_PENV_8, "103 PRC Envelope #8 120 x 309 mm" },
   { DMPAPER_PENV_9, "104 PRC Envelope #9 229 x 324 mm" },
   { DMPAPER_PENV_10, "105 PRC Envelope #10 324 x 458 mm" },
   { DMPAPER_P16K_ROTATED, "106 PRC 16K Rotated" },
   { DMPAPER_P32K_ROTATED, "107 PRC 32K Rotated" },
   { DMPAPER_P32KBIG_ROTATED, "108 PRC 32K(Big) Rotated" },
   { DMPAPER_PENV_1_ROTATED, "109 PRC Envelope #1 Rotated 165 x 102 mm" },
   { DMPAPER_PENV_2_ROTATED, "110 PRC Envelope #2 Rotated 176 x 102 mm" },
   { DMPAPER_PENV_3_ROTATED, "111 PRC Envelope #3 Rotated 176 x 125 mm" },
   { DMPAPER_PENV_4_ROTATED, "112 PRC Envelope #4 Rotated 208 x 110 mm" },
   { DMPAPER_PENV_5_ROTATED, "113 PRC Envelope #5 Rotated 220 x 110 mm" },
   { DMPAPER_PENV_6_ROTATED, "114 PRC Envelope #6 Rotated 230 x 120 mm" },
   { DMPAPER_PENV_7_ROTATED, "115 PRC Envelope #7 Rotated 230 x 160 mm" },
   { DMPAPER_PENV_8_ROTATED, "116 PRC Envelope #8 Rotated 309 x 120 mm" },
   { DMPAPER_PENV_9_ROTATED, "117 PRC Envelope #9 Rotated 324 x 229 mm" },
   { DMPAPER_PENV_10_ROTATED, "118 PRC Envelope #10 Rotated 458 x 324 mm" },
#endif   // WINVER >= 0x0500
   { 0,                        0   }
};

int   AddPaperStg( LPTSTR lpb, UINT size )
{
   int      iRet = 0;
   LPTSTR   lps;
   UINT     ui;

   if( ui = size )
   {
      PIDSTG pids = &sPrtPaper[0];
      while( lps = pids->id_psNm )
      {
         if( pids->id_uiID == ui )
            break;
         pids++;
      }
      if( lps )
      {
         while( ( *lps ) && ( *lps >= '0' ) && ( *lps <= '9' ) )
            lps++;
         while( ( *lps ) && ( *lps == ' ' ) )
            lps++;
         if( *lps )
         {
            iRet = lstrlen(lps);
            wsprintf( EndBuf(lpb), "P=%s ", lps );
         }
         else
         {
            lps = 0;
         }
      }
      if( !lps )
      {
         wsprintf( EndBuf(lpb), "P=Unk%d ", ui );
      }
   }

   return iRet;

}

IDSTG sPrtBin[] = {
   { DMBIN_ONLYONE, "ONLYONE" },
   { DMBIN_LOWER, "LOWER" },
   { DMBIN_MIDDLE, "MIDDLE" },
   { DMBIN_MANUAL, "MANUAL" },
   { DMBIN_ENVELOPE, "ENV." },
   { DMBIN_ENVMANUAL, "ENVMAN" },
   { DMBIN_AUTO, "AUTO" },
   { DMBIN_TRACTOR, "TRACTOR" },
   { DMBIN_SMALLFMT, "SMLFMT" },
   { DMBIN_LARGEFMT, "LGEFMT" },
   { DMBIN_LARGECAPACITY, "LGECAP." },
   { DMBIN_CASSETTE, "CASS." },
   { DMBIN_FORMSOURCE, "FMSRC" },
   { 0,                0 }
};

INT   AddBinSrc( LPTSTR lpb, UINT val )
{
   INT   iRet = 0;
   PIDSTG pids = &sPrtBin[0];
   LPTSTR   lps;

   while( lps = pids->id_psNm )
   {
      if( pids->id_uiID == val )
         break;
      pids++;
   }
   if(lps)
   {
      iRet = lstrlen(lps);
      wsprintf(EndBuf(lpb), "B=%s ", lps );
   }
   else if( val >= DMBIN_USER )
   {
      wsprintf(EndBuf(lpb), "B=U%#x ", val );
   }
   else
   {
      wsprintf(EndBuf(lpb), "B=?%#x ", val );
   }

   return iRet;

}

INT   AddPrtIDStg( LPTSTR lpb, UINT val, PIDSTG pids, LPTSTR pFOk, LPTSTR pFNo )
{
   INT      iRet = 0;
   LPTSTR   lps;
   while( lps = pids->id_psNm )
   {
      if( pids->id_uiID == val )
         break;
      pids++;
   }
   if(lps)
   {
      iRet = lstrlen(lps);
      wsprintf(EndBuf(lpb), pFOk, lps );
   }
   else
   {
      wsprintf(EndBuf(lpb), pFNo, val );
   }
   return iRet;
}

IDSTG sPrtQual[] = {
   { DMRES_HIGH, "HIGH" },
   { DMRES_MEDIUM, "MEDIUM" },
   { DMRES_LOW, "LOW" },
   { DMRES_DRAFT, "DRAFT" },
   { 0,           0 }
};

INT   AddPrtQual( LPTSTR lpb, UINT val )
{
   PIDSTG   pids = &sPrtQual[0];
   INT      iRet;
   iRet = AddPrtIDStg( lpb, val, pids, "Q=%s ", "Q=%#x? " );
   return iRet;
}

// dmColor 
// Switches between color and monochrome on color printers.
// Following are the possible values:
IDSTG sPrtColor[] = {
   { DMCOLOR_COLOR, "COLOR" },
   { DMCOLOR_MONOCHROME, "MONO" },
   { 0,       0 }
};

static   TCHAR sszPort[] = TEXT("Or=Port ");
// else if( pdm->dmOrientation == DMORIENT_LANDSCAPE )
static   TCHAR sszLand[] = TEXT("Or=Land ");
// else
static   TCHAR sszOUnk[] = TEXT("Or=U%#x ");

INT   AddPrtColor( LPTSTR lpb, UINT val )
{
   INT      iRet;
   PIDSTG   pids = &sPrtColor[0];
   iRet = AddPrtIDStg( lpb, val, pids, "C=%s ", "C=%#x? " );
   return iRet;
}

// ENUMERATION OF PRINTERS using ALLOCATED MEMORY
// And edd the enumerated printer names and ports to the COMBO BOX list
// Set the SELECTION to the current DEFAULT PRINTER
// Hmmm - If the USER changes this SELECTION, DIBVIEW should
// REMEMBER which one they chose, and keep it is the later selection
// Maybe even write it to the INI file
//
// ==================================================================
//INT_PTR P2_GetSel(HWND  hDlg,  // handle to dialog box
//                  PPS   pps,
//                  PDI   lpDIBInfo )
INT_PTR P2_GetSel(HWND  hDlg,  // handle to dialog box
                  PPS   pps )
{
   INT_PTR  iRet = FALSE;
   LPTSTR   lpd  = &gszDiag[0];
   HDC      hdcI = 0;
   LRESULT  lRes = SendMessage( pps->ps_hCBPrt,  // handle to destination window
         CB_GETITEMDATA,         // message to send
         SendMessage( pps->ps_hCBPrt, CB_GETCURSEL, 0, 0 ),    // item index
         0 );     // not used; must be zero

   pps->ps_iPrtOK = 0;     // indicates we could OPEN the printer
   *lpd = 0;   // clear the selection information field
   //SetDlgItemText( hDlg, IDC_EDSELECTED, lpd );
   if( lRes != CB_ERR )
   {
      // assume lRes contains a zero based offset to the SELECTED
      // printers PRINTER_INFO_2
      HGLOBAL           hg;
      PPRINTER_INFO_2   ppi2, ppi2C;
      PDEVMODE          pdm;
      //LPDEVNAMES        pdn;
      LPTSTR            lpn, lppn, lpfm;
      LPPRINTER_DEFAULTS pDefault = &pps->ps_sPrtDefs;

      if( ( hg = pps->ps_hEnum ) &&
          ( lpn = DVGlobalLock(hg) ) )
      {
         lpfm = "p%d: ";
         ppi2  = (PPRINTER_INFO_2)lpn;
         ppi2C = (PPRINTER_INFO_2)((LPTSTR)lpn + pps->ps_dwP2Size); // to END of ENUM block
         ppi2 += lRes;  // get offset into structure
         if( lppn = ppi2->pPrinterName )  // if we HAVE a printer NAME
         {
            pps->ps_dwSel    = lRes;   // set selected
            pps->ps_dwPrtOff =  ppi2 - (PPRINTER_INFO_2)lpn; // and its DWORD offset
            pps->ps_iPrtOK  |= 1;     // indicates we have an offset to the printer

            ZeroMemory( pDefault, sizeof(PRINTER_DEFAULTS) );
            pDefault->DesiredAccess = PRINTER_ACCESS_USE;
            if( OpenPrinter( lppn, &pps->ps_hPrinter, pDefault ) )
            {
               // HOWTO: Update Printer Selection()
               // since for example the "need inter." bit is still there
               // on a NETWORK printer that was NOT originally available
               // but may have NOW become available
               if( GetPrinter( pps->ps_hPrinter, // (HANDLE) - handle to printer
                  2, // (DWORD) - information level
                  (LPBYTE)ppi2C, // printer information buffer
                  pps->ps_dwP2Size, // size of buffer
                  &pps->ps_cbNeeded ) )   // (LPDWORD) - bytes received or required
               {
                  ppi2 = ppi2C;  // used UPDATED info
                  lpfm = "P%d: ";   // and change to a CAPITAL "P" to show using update
                  pps->ps_dwPrtOff = ppi2 - (PPRINTER_INFO_2)lpn; // and its DWORD offset
                  pps->ps_iPrtOK  |= 2;     // indicates we could OPEN the printer
               }

               ClosePrinter( pps->ps_hPrinter );   // close the SELECTED printer
            }

            //wsprintf( lpd, "View per %s. ", ppi2->pPrinterName );
            wsprintf( lpd, lpfm, (lRes + 1) );
            // get the ALL IMPORTANT (much information) DEVMODE
            if( pdm = ppi2->pDevMode )
            {
               if( pdm->dmFields & DM_ORIENTATION )
               {
                  if( pdm->dmOrientation == DMORIENT_PORTRAIT )
                  {
                     lstrcat(lpd, sszPort ); // "OR=Port "
                  }
                  else if( pdm->dmOrientation == DMORIENT_LANDSCAPE )
                  {
                     lstrcat(lpd, sszLand ); // "OR=Land "
                  }
                  else
                  {
                     wsprintf(EndBuf(lpd), sszOUnk,   // "OR=U%#x"
                        pdm->dmOrientation );
                  }
               }
               else
               {
                  lstrcat(lpd, "Or=DefP " );
               }

               // paper SIZE
               if( pdm->dmFields & DM_PAPERSIZE )
               {
                  AddPaperStg( lpd, pdm->dmPaperSize );
               }

               // lenght / width OVER-RIDES in 1/10 of mm
               if( pdm->dmFields & DM_PAPERLENGTH )
                  wsprintf(EndBuf(lpd), "L=%d ", pdm->dmPaperLength );

               if( pdm->dmFields & DM_PAPERWIDTH  )
                  wsprintf(EndBuf(lpd), "W=%d ", pdm->dmPaperWidth ); 

               // any applied SCALE - factor dmScale / 100 ie 50 = 1/2 size
               if( pdm->dmFields & DM_SCALE )
                  wsprintf(EndBuf(lpd), "SC=%d ", pdm->dmScale );
               
               if( pdm->dmFields & DM_COPIES )
                  pps->ps_dwCopies = pdm->dmCopies;
               else
                  pps->ps_dwCopies = 1;

               if( pdm->dmFields & DM_DEFAULTSOURCE )
                  AddBinSrc( lpd, pdm->dmDefaultSource );

               if( pdm->dmFields & DM_PRINTQUALITY )
                  AddPrtQual( lpd, pdm->dmPrintQuality );

               if( pdm->dmFields & DM_COLOR )
                  AddPrtColor( lpd, pdm->dmColor );
               
            }
            else
            {
               lstrcat(lpd, "No DEVMODE ");
               pps->ps_dwCopies = 1;
            }

            if( ppi2->Status )
               AddPStatus( lpd, ppi2->Status );
            else
               lstrcat(lpd, "S=OK");

            //pdn = ppi2->pDriverName;
            //GetDevInfo( pps, LPDEVNAMES pdn!!!, pdm )
            if( hdcI = CreateIC( NULL, lppn, NULL, NULL ) )
            {
               iRet = GetHdcInfo( pps, hdcI, lppn );
               DeleteDC(hdcI);
               ResetFrame( pps );
            }
            else
            {
               sprtf( "WARNING: Unable to get HDC for printer [%s]!"MEOR,
                  lppn );
            }

            wsprintf(EndBuf(lpd),"%d",iRet);
            // printer information string is set
         }
         else
         {
            lstrcpy( lpd, "ERROR: No offset to PRINTER selection!" );
         }

         DVGlobalUnlock(hg);  // unlock allocated ENUM PRINTER structure
      }
      else
      {
            lstrcpy( lpd, "ERROR: Unable to get memory pointer to PRINTER selection!" );
      }

      //SetDlgItemInt( hDlg, IDC_EDCOPIES, pps->ps_dwCopies, FALSE );

      iRet = TRUE;   // return TRUE - ie we have a selection

   }
   else
   {
      wsprintf( lpd, "ERROR: Failed to get SELECTION from %#x."MEOR,
         pps->ps_hCBPrt );
      // return FALSE = NO SELECTION DUE TO SOME ERROR
   }

   // NOTE: P2_InitFont() MUST be called BEFORE this!
   P2_InitText( pps );  // initialise the TEXT display of the DIALOG
   // *****************************************
   // set a printer information string ========
   SetDlgItemText( hDlg, IDC_EDSELECTED, lpd );
   // =========================================
   sprtf( "GETSEL: %s"MEOR, lpd ); // just a DIAGNOSTIC output
   // *****************************************

   // and finally if there has been a CHANGE in the number of COPIES
   SetDlgItemInt( hDlg, IDC_EDCOPIES, pps->ps_dwCopies, FALSE );

   return iRet;

}

// ======================================================
// Called from notification from IDC_CBPRINTERS combo box
//   on SELECTION CHANGED, and
// called from SetPrintCombo() during P2_Init
//   to take care of the FIRST selection
//
// Get the NEW selection, and ENABLE / DISABLE the OK
//
// ======================================================
//BOOL P2_SetOK(HWND  hDlg,  // handle to dialog box
//                  PPS   pps,
//                  PDI   lpDIBInfo )
BOOL P2_SetOK(HWND  hDlg,  // handle to dialog box
                  PPS   pps )
{
   BOOL  bRet;

//   if( P2_GetSel( hDlg, pps, lpDIBInfo ) )
   if( P2_GetSel( hDlg, pps ) )
      bRet = TRUE;
   else
      bRet = FALSE;

   EnableWindow( pps->ps_hOK, bRet );

   return bRet;
}


//DWORD SetPrinterCombo( HWND hDlg, PDI lpDIBInfo )
DWORD SetPrinterCombo( HWND hDlg, PPS pps )
{
   DWORD    dwRet = 0;
   //if( lpDIBInfo )
   {
      DWORD    dwc, dwr, dwc2;
//      PPRINTER_INFO_1   ppi1;
      PPRINTER_INFO_2   ppi2 = 0;
      LPTSTR   lpd = &gszDiag[0];
      LPTSTR   lps;
      LRESULT  lRes;
      //PPS      pps = (PPS)&lpDIBInfo->di_szCGenBuf2[0];
      LPPRINTER_DEFAULTS pDefault = &pps->ps_sPrtDefs;
      DEVMODE * pdm;

      dwc2 = dwc = dwr = 0;
      EnumPrinters(
         PRINTER_ENUM_LOCAL,  // (DWORD) - printer object types
         NULL, // (LPTSTR) - name of printer object
         2,    // (DWORD) - information level
         &gszBigBuf[0], // (LPBYTE) - printer information buffer
         1, // (DWORD) - size of printer information buffer
         &dwr, // (LPDWORD) - bytes received or required
         &dwc );
      // get DOUBLE the required size
      if( ( dwr ) &&
          ( pps->ps_hEnum = DVGAlloc( "ENUMPRT", GHND, ( dwr * 2 ) ) ) &&
          ( ppi2 = (PPRINTER_INFO_2)DVGlobalLock( pps->ps_hEnum ) ) )
      {
         // got size and allocated buffer
         pps->ps_dwP2Size = dwr; // keep our SIZE for INFO 2
      }
      else
      {
         goto Got_Err2;
      }
      if( ( EnumPrinters(
         PRINTER_ENUM_LOCAL,  // (DWORD) - printer object types
         NULL, // (LPTSTR) - name of printer object
         2,    // (DWORD) - information level
         (LPBYTE)ppi2, // &gszBigBuf[0], // (LPBYTE) - printer information buffer
         dwr, // (DWORD) - size of printer information buffer
         &dwr, // (LPDWORD) - bytes received or required
         &dwc ) ) && // (LPDWORD) - number of printers enumerated
         ( dwc ) )
      {
         //ppi1 = (PPRINTER_INFO_1) &gszBigBuf[0];
         //ppi2 = (PPRINTER_INFO_2) &gszBigBuf[0];
//         ppi2 = (PPRINTER_INFO_2)((PPRINTER_INFO_1)ppi1 + dwc);
   		SendMessage( pps->ps_hCBPrt, CB_RESETCONTENT, 0, 0 );
         for( dwr = 0; dwr < dwc; dwr++ )
         {
            *lpd = 0;
            pdm = ppi2->pDevMode;
            if( ( lps = ppi2->pPrinterName ) && ( *lps ) )
            {
               ZeroMemory( pDefault, sizeof(PRINTER_DEFAULTS) );
               pDefault->DesiredAccess = PRINTER_ACCESS_USE;
               //lstrcpy( lpd, lps );
               wsprintf( lpd, "P%d: %s ", (dwr + 1), lps );
               if( OpenPrinter(
                  lps,  // (LPTSTR) - printer or server name
                  &pps->ps_hPrinter,   // (LPHANDLE) - printer or server handle
                  pDefault ) ) // printer defaults
               {
                  if( ppi2->pPortName )
                     wsprintf(EndBuf(lpd), " on port %s", ppi2->pPortName );

                  ClosePrinter( pps->ps_hPrinter );
               }
               else
               {
                  gdwError = GetLastError();
                  lstrcat( lpd, " (WARNING: May be NOT available!)" );
                  sprtf( "ERROR: HEY: Open Printer! Err=%#x", gdwError );
                  //*lpd = 0;
               }

               if( *lpd )
               {

                  lRes = SendMessage( pps->ps_hCBPrt, CB_ADDSTRING, 0, (LPARAM)lpd );
                  *lpd = 0;
					   if( ( lRes == CB_ERR      ) ||
						    ( lRes == CB_ERRSPACE ) )
					   {
						   goto Got_Err2;
					   }
					   else
					   {
						   SendMessage( pps->ps_hCBPrt, CB_SETITEMDATA, (WPARAM)lRes, (LPARAM)dwr );
                     dwc2++;
					   }

                  wsprintf( EndBuf(lpd), "Name=[%s]"MEOR, lps );

                  //if( strcmpi( &gszDefPrtr[0], lps ) == 0 )
                  if( strcmpi( &pps->ps_szDefPrtr[0], lps ) == 0 )
                  {
                     pps->ps_dwSel = lRes;   // select this one
                  }
               }
            }
            else
            {
               lstrcat(lpd, "Name is NULL"MEOR);
            }

            sprtf( "P%d: = %s", (dwr + 1), lpd );

            ppi2++;  // bump to the next structure
            
         }  // for the printer count

         // NOTE: On this message the text in the edit control of
         // the combo box changes to reflect the new selection,
		   SendMessage( pps->ps_hCBPrt, CB_SETCURSEL, pps->ps_dwSel, 0 ); // item index

         // and since it appears there is NO NOTIFY message from this
         // programic selection, we do it here!!!
         //P2_SetOK( hDlg, pps, lpDIBInfo );
         P2_SetOK( hDlg, pps );

         dwRet = dwc2;  // we can ONLY continue if this enumeration yielded strings

      }

      if( ( pps->ps_hEnum ) && ( ppi2 ) )
         DVGlobalUnlock( pps->ps_hEnum );

   }

Got_Err2:

   return dwRet;

}

VOID  EnabPerChecks( HWND hDlg, PPS pps )
{
   BOOL  flg;
   PDLGITEMS pdi = &sPrtDlg[0];

   //if( pps->ps_bDistort || pps->ps_bFitPage )
   if( pps->ps_bFitPage )
      flg = FALSE;
   else
      flg = TRUE;

   while( pdi->di_dwType != t_End )
   {
      if( pdi->di_bEnab )
      {
         EnableWindow( pdi->di_hWnd, flg );
      }
      pdi++;
   }

}

//#define  pt_BotLef   0
//#define  pt_BotRit   2
//#define  pt_RitTop   4
//#define  pt_RitBot   6
//   int               ps_iArray;     // count in array
//   POINT             ps_ptArray[MXPTS];   // point array
BOOL  P2_InitText( PPS pps )
{
   LPRECT   prcDlgFrm, prcPPage;
   RECT     rc;
   POINT    pt;
   PPOINT   ppt1, ppt2;
   PPTARR   ppta1, ppta2;
   HDC      hdc;
   LPTSTR   lpb = GetTmp2();	// &Buf[0];
   SIZE     sz;

   prcDlgFrm = &pps->ps_rcDlgFrm;
   prcPPage  = &pps->ps_rcPPage;

   if( ( pps->ps_hFontV && pps->ps_hFontA    ) &&
       ( hdc = GetDC(GetDlgHand(IDC_FRMA4)) ) )
   {
      HFONT hOld;
      // setup the TEXT and the size of the text
      // =======================================
      wsprintf( lpb, " %d Px ", pps->ps_dxRes ); // = GetDeviceCaps( hdcTarget, HORZRES   ); // in Pixels
      if( lstrcmpi( &pps->ps_szBotTxt[0], lpb ) )
      {
         lstrcpy( &pps->ps_szBotTxt[0], lpb );
         nPrtChg[nPrtHTxt] = TRUE;
      }
      pps->ps_iBotTxt = lstrlen( &pps->ps_szBotTxt[0] );

      hOld = SelectObject( hdc, pps->ps_hFontA );
      GetTextExtentPoint32( hdc,           // handle to DC
         &pps->ps_szBotTxt[0],  // text string
         pps->ps_iBotTxt,    // characters in string
         &sz );   // string size

      if( ( sz.cx != pps->ps_ssFA.cx ) ||  // string size
         ( sz.cy != pps->ps_ssFA.cy ) )
      {
         pps->ps_ssFA = sz;
         nPrtChg[nPrtHTS] = TRUE;
      }


      wsprintf( lpb, " %d Px ", pps->ps_dyRes ); // = GetDeviceCaps( hdcTarget, VERTRES   );
      if( lstrcmpi( &pps->ps_szRitTxt[0], lpb ) )
      {
         lstrcpy( &pps->ps_szRitTxt[0], lpb );
         nPrtChg[nPrtVTxt] = TRUE;
      }
      pps->ps_iRitTxt = lstrlen( &pps->ps_szRitTxt[0] );
      SelectObject( hdc, pps->ps_hFontV );
      GetTextExtentPoint32( hdc,           // handle to DC
         &pps->ps_szRitTxt[0],  // text string
         pps->ps_iRitTxt,    // characters in string
         &sz );   // get string size with this font

      if( ( sz.cx != pps->ps_ssF2.cx ) ||  // string size
         ( sz.cy != pps->ps_ssF2.cy ) )
      {
         pps->ps_ssF2 = sz;
         nPrtChg[nPrtVTS] = TRUE;
      }

      SelectObject( hdc, hOld );

      //pps->ps_dwMFWid = DEF_FRM;
      //pps->ps_dwMFHit = DEF_FRM;
      if( pps->ps_ssFA.cx == 0 )
         pps->ps_ssFA.cx = 53;
      if( pps->ps_ssFA.cy == 0 )
         pps->ps_ssFA.cy = 16;

      if( (DWORD)(pps->ps_ssFA.cy * 2) > pps->ps_dwMFHit )
      {
         pps->ps_dwMFHit = pps->ps_ssFA.cy * 2;
         nPrtChg[nPrtFHit] = TRUE;
      }

      if( (DWORD)(pps->ps_ssFA.cx * 2) > pps->ps_dwMFWid )
      {
         pps->ps_dwMFWid = pps->ps_ssFA.cx * 2;
         nPrtChg[nPrtFWid] = TRUE;
      }

      // now calculate the POSITION of the text
      // always just within the DIALOG FRAME
      // ======================================
      pps->ps_iArray = 0;

      // get the bottom text, lines and arrows
      rc.left   = 0;
      rc.right  = prcDlgFrm->right;
      rc.top    = prcPPage->bottom;
      rc.bottom = prcDlgFrm->bottom;

      pt.x = ( rc.left + rc.right ) / 2;
      if( pt.x > (pps->ps_ssFA.cx / 2) )
         pt.x -= (pps->ps_ssFA.cx / 2);
      else
         pt.x  = 0;

      pt.y = ( rc.top + rc.bottom ) / 2;
      if( pt.y > ( pps->ps_ssFA.cy / 2 ) )
         pt.y -= ( pps->ps_ssFA.cy / 2 );
      else
         pt.y  = 0;
      //TextOut( hdc, pt.x, pt.y, lpt, i );
      pps->ps_ptBotTxt = pt;

      ppta1 = &pps->ps_ptArray[pt_BotLef];
      ppta2 = ppta1 + 1;
      ppt1 = &ppta1->pa_Pt;
      ppt2 = &ppta2->pa_Pt;
      ppt1->x = prcPPage->left;
      ppt1->y = ( rc.top + rc.bottom ) / 2;
      ppt2->x = pt.x - 1;
      ppt2->y = ppt1->y;
      ppta1->pa_Type = ptt_Left;
      ppta2->pa_Type = ptt_None;
      pps->ps_iArray++;

      ppta1 = &pps->ps_ptArray[pt_BotRit];
      ppta2 = ppta1 + 1;
      ppt1 = &ppta1->pa_Pt;
      //ppt1 = &pps->ps_ptArray[pt_BotRit];
      ppt1->y = ppt2->y;
      ppt2 = &ppta2->pa_Pt;
      //ppt2 = ppt1 + 1;
      ppt1->x = pt.x + pps->ps_ssFA.cx + 1;
      ppt2->x = prcPPage->right;
      ppt2->y = ppt1->y;
      ppta2->pa_Type = ptt_Rite;
      ppta1->pa_Type = ptt_None;
      pps->ps_iArray++;

      // now the RIGHTHAND TEXT
     // get the right side text, lines and arrows
      rc.left   = prcPPage->right;
      rc.right  = prcDlgFrm->right;
      rc.top    = 0;
      rc.bottom = prcDlgFrm->bottom;

      pt.x = ( rc.left + rc.right ) / 2;
      if( (pt.x + (pps->ps_ssFA.cy / 2)) < rc.right )
         pt.x += (pps->ps_ssFA.cy / 2);
      else
         pt.x  = rc.right;

      pt.y = ( rc.top + rc.bottom ) / 2;
      if( pt.y > ( pps->ps_ssFA.cx / 2 ) )
         pt.y -= ( pps->ps_ssFA.cx / 2 );
      else
         pt.y  = 0;
      //TextOut( hdc, pt.x, pt.y, lpt, i );
      pps->ps_ptRitTxt = pt;

      ppta1 = &pps->ps_ptArray[pt_RitTop];
      ppta2 = ppta1 + 1;
      ppt1 = &ppta1->pa_Pt;
      ppt2 = &ppta2->pa_Pt;

      ppt1->x = ( rc.left + rc.right ) / 2;
      ppt1->y = prcPPage->top;
      ppt2->x = ppt1->x;   // x is unchange
      ppt2->y = pt.y - 1;  // to top of text
      ppta1->pa_Type = ptt_Up;
      ppta2->pa_Type = ptt_None;
      pps->ps_iArray++;

      ppta1 = &pps->ps_ptArray[pt_RitBot];
      ppta2 = ppta1 + 1;
      ppt1 = &ppta1->pa_Pt;
      ppt1->x = ppt2->x;   // keep X constant
      ppt1->y = ppt2->y + pps->ps_ssFA.cx + 2;
      ppt2 = &ppta2->pa_Pt;
      ppt2->x = ppt1->x;   // keep X
      ppt2->y = prcPPage->bottom;
      ppta2->pa_Type = ptt_Down;
      ppta1->pa_Type = ptt_None;
      pps->ps_iArray++;

      ReleaseDC( GetDlgHand(IDC_FRMA4), hdc );

   }

   return TRUE;
}

#ifndef  NDEBUG
// if( GetObject( pps->ps_hFontV, sizeof(LOGFONT), &slf2 ) )
VOID  ShwLogFont( PLOGFONT plf )
{
   sprtf( "Have FONT [%s] size %3d esc %4d orien %4d."MEOR,
                  &plf->lfFaceName[0],
                  plf->lfHeight,
                  plf->lfEscapement,
                  plf->lfOrientation );
}
#endif   // #ifndef  NDEBUG

BOOL  P2_InitFont( HWND hdlg, PPS pps )
{
   BOOL  bRet = FALSE;
   HFONT hFont;
   //LOGFONT  sLogFont;
#ifndef  NDEBUG
   LOGFONT  slf2;
#endif   // !NDEBUG
//#ifdef   USE_ARIAL
   PLOGFONT plf = &pps->ps_sLogFont;
   HDC      hdc;


   ZeroMemory( plf, sizeof(LOGFONT) );
   plf->lfHeight = -11;    // about 8 points
   if( hdc = GetDC( GetDlgHand(IDC_FRMA4) ) )
   {
      plf->lfHeight = -MulDiv( 8, GetDeviceCaps(hdc, LOGPIXELSY), 72 );
      ReleaseDC( GetDlgHand(IDC_FRMA4), hdc );
      lstrcpy( &plf->lfFaceName[0], "Arial" );
      plf->lfWeight = FW_BOLD;   // set BOLD font
      hFont = CreateFontIndirect( plf );
      if( hFont )
      {
         plf->lfEscapement = 2700;
         pps->ps_hFontV = CreateFontIndirect(plf);
         if( pps->ps_hFontV )
         {
            pps->ps_hFontA = hFont;

            //P2_InitText( pps ); - now done in SELECTION

            bRet = TRUE;
         }
      }
   }

   if( bRet )
   {
#ifndef  NDEBUG
            if( GetObject( pps->ps_hFontV, sizeof(LOGFONT), &slf2 ) )
            {
               ShwLogFont( &slf2 );
               //sprtf( "Have FONT [%s] size %d esc %d orien %d."MEOR,
               //   &slf2.lfFaceName[0],
               //   slf2.lfHeight,
               //   slf2.lfEscapement,
               //   slf2.lfOrientation,
               //   pps->ps_ssF2.cx,
               //   pps->ps_ssF2.cy );
            }

            if( GetObject( pps->ps_hFontA, sizeof(LOGFONT), &slf2 ) )
            {
               ShwLogFont( &slf2 );
            }
#endif   // !NDEBUG
   }
   else
   {
      chkme( "ERROR: YOWEE! FAILED TO GET FONTS!!!"MEOR );
   }

   return bRet;

}

VOID  ResetFrame( PPS pps )
{
   LPRECT   prcDlgFrm = &pps->ps_rcDlgFrm;
   LPRECT   prcDPage  = &pps->ps_rcDPage;  // device resolution (from printer HDC)
   LPRECT   prcFrame  = &pps->ps_rcPFrame;
   LPRECT   prcObj    = &pps->ps_rcObj;
   LPRECT   prcPPage  = &pps->ps_rcPPage;
   LPRECT   prcPObj   = &pps->ps_rcPObj;
   RECT     rcf, rcp;   // work rectangles

      // get ADJUSTED rectangles
      rcf = *prcDlgFrm;  // copy the DIALOG frame - ps_rcDlgFrm

      // render the PAGE into FRAME getting PAINT PAGE/FRAME
      GetPntPage( &rcp, &rcf, prcDPage, TRUE ); // this is OK
      //   ( (DWORD)(RW(prcDlgFrm) - RW(&rcp)) < pps->ps_dwMFHit ) )   // was DEF_FRM
      while( ( RW(&rcf) > 60 ) && ( RH(&rcf) > 100 ) &&
         ( (DWORD)(RH(prcDlgFrm) - RH(&rcp)) < pps->ps_dwMFHit ) )   // was DEF_FRM
      {
         // reduce the DIALOG FRAME until we have space for TEXT around it
         rcf.left++;
         rcf.top++;
         rcf.right--;
         rcf.bottom--;
         // and get a NEW page size
         GetPntPage( &rcp, &rcf, prcDPage, TRUE ); // this is OK
      }

      if( EqualRect( prcFrame, &rcf ) == 0 )
      {
         *prcFrame  = rcf;
         nPrtChg[nPrtPRc] = TRUE;
      }
      if( EqualRect( prcPPage, &rcp ) == 0 )
      {
         *prcPPage  = rcp;
         nPrtChg[nPrtPPg] = TRUE;
      }

      // render OBJECT into PAINT PAGE/FRAME per OBJECT to PAGE ratio
      GetPntPage2( prcPObj, prcPPage, prcObj, prcDPage );

}

INT_PTR P2_Init(
    HWND hDlg,  // handle to dialog box
    WPARAM wParam, // first message parameter
    LPARAM lParam ) // second message parameter
{
   INT_PTR  iRet = TRUE;
   HGLOBAL  hg;
   //PDI      lpDIBInfo;
   PDI      pdi;
   //LPRECT   prcPPage, prcPObj, prcFrame;
   LPRECT   prcObj, prcDlgFrm;
   LPTSTR   lpb;
   //LPRECT   prcBPage, prcDPage;
   //RECT     rcf, rcp;   // work rectangles
   RECT     rcf;   // work rectangle
   DWORD    dwi;

   SET_PROP( hDlg, ATOM_PRT, lParam );

   CenterWindow( hDlg, ghMainWnd );

   //    ( lpDIBInfo = (PDI)DVGlobalLock(hg) ) )
   if( ( hg  = (HGLOBAL)lParam       ) &&
       ( pdi = (PDI)DVGlobalLock(hg) ) )
   {
      //PPS      pps = (PPS)&lpDIBInfo->di_szCGenBuf2[0];
      //lpb = &lpDIBInfo->di_szCGenBuff[0]; // A general WORK buffer
      PPS      pps = (PPS)&pdi->di_szCGenBuf2[0];
      lpb = &pdi->di_szCGenBuff[0]; // A general WORK buffer

      // although the following may have been READ IN from INI
      // they will be RESET when filling the COMBO BOX with enumerated printers,
      // and at the end a programic SELECTION is made.
      prcObj    = &pps->ps_rcObj;
      //prcPPage  = &pps->ps_rcPPage;
      //prcPObj   = &pps->ps_rcPObj;
      //prcDPage  = &pps->ps_rcDPage;  // device resolution in pixels (from HDC)
      //prcBPage  = &pps->ps_rcBPage;    // get the PRINT page minus BORDERS

      //prcRend  = &pps->ps_rcRend;
      //prcFrame  = &pps->ps_rcPFrame;
      prcDlgFrm = &pps->ps_rcDlgFrm;

      prcObj->left   = prcObj->top = 0;
      prcObj->right  = pps->ps_dwOWid; // = lpDIBInfo->di_dwDIBWidth;
      prcObj->bottom = pps->ps_dwOHit; // = lpDIBInfo->di_dwDIBHeight;

      pps->ps_bChgPrt = TRUE;    // redo sizing calculation in ???

      if( !GetDlgHandles( hDlg ) )
         goto Got_Err;

      // get the COMBOBOX handle
      pps->ps_hCBPrt = GetDlgHand( IDC_CBPRINTERS );
      if( !pps->ps_hCBPrt )
         goto Got_Err;

      pps->ps_hOK = GetDlgHand( IDOK );
      if( !pps->ps_hOK )
         goto Got_Err;

      EnableWindow( pps->ps_hOK, FALSE );

      if( !GetClientRect( GetDlgHand(IDC_FRMA4), &rcf ) )   // pps->ps_rcDlgFrm
         goto Got_Err;
      if( EqualRect( prcDlgFrm, &rcf ) == 0 )
      {
         // set NEW dialog FRAME size
         *prcDlgFrm  = rcf;   // ps_rcDlgFrm
         nPrtChg[nPrtFrm] = TRUE;   // and set CHANGE for write INI value
      }

      // NOTE: Must create FONTS before setting COMBO
      if( !P2_InitFont( hDlg, pps ) )
         goto Got_Err;

      // NOTE: Set COMBO must be done BEFORE adjusting rectangles
      // ALSO: Setting the Printre COMBO also calls P2_SetOK()->P2_GetSel()
      // which uses the current ps_rcPFrame read from INI (if it existed)
      // in GetHdcInfo() ...
      //pps->ps_dwPCnt = SetPrinterCombo( hDlg, lpDIBInfo );
      pps->ps_dwPCnt = SetPrinterCombo( hDlg, pps );
      if( !pps->ps_dwPCnt )
         goto Got_Err;

      // set up the Object dimension - The DIB dimensions
      // ================================================
      wsprintf( lpb, "&Width: %d",  pps->ps_dwOWid ); // = lpDIBInfo->di_dwDIBWidth
      SetDlgItemText( hDlg, IDC_LABWID, lpb );
      wsprintf( lpb, "&Height: %d", pps->ps_dwOHit ); // = lpDIBInfo->di_dwDIBHeight
      SetDlgItemText( hDlg, IDC_LABHIT, lpb );
      // ================================================

      SetDlgItemInt( hDlg, IDC_EDTOP,   pps->ps_dwTop,    FALSE );
      SetDlgItemInt( hDlg, IDC_EDLEFT,  pps->ps_dwLeft,   FALSE );

      if( pps->ps_dwPCWid == 0 )
      {
         pps->ps_dwPCWid = 100;
         nPrtChg[nPrtPCW] = TRUE;
      }
      if( pps->ps_dwPCHit == 0 )
      {
         pps->ps_dwPCHit = 100;
         nPrtChg[nPrtPCH] = TRUE;
      }
      if( !pps->ps_bDistort )
      {
         if( pps->ps_dwPCHit != pps->ps_dwPCWid )  // default to WIDTH percentage
         {
            pps->ps_dwPCHit = pps->ps_dwPCWid;  // default to WIDTH percentage
            nPrtChg[nPrtPCH] = TRUE;
         }
      }
      // get width (from percentage given)
      //dwi = ( lpDIBInfo->di_dwDIBWidth * pps->ps_dwPCWid ) / 100;
      //dwi = ( lpDIBInfo->di_dwDIBHeight * pps->ps_dwPCHit) / 100;
      dwi = ( pps->ps_dwOWid * pps->ps_dwPCWid ) / 100;
      SETITEM( ps_dwWidth, dwi, nPrtWid );
     // and get height
      dwi = ( pps->ps_dwOHit * pps->ps_dwPCHit) / 100;
      SETITEM( ps_dwHeight, dwi, nPrtHit );

      SetDlgItemInt( hDlg, IDC_EDWID,   pps->ps_dwWidth,  FALSE );
      SetDlgItemInt( hDlg, IDC_EDHIT,   pps->ps_dwHeight, FALSE );
      SetDlgItemInt( hDlg, IDC_EDPCWID, pps->ps_dwPCWid,  FALSE );
      SetDlgItemInt( hDlg, IDC_EDPCHIT, pps->ps_dwPCHit,  FALSE );

      CheckDlgButton( hDlg, IDC_CHDIST,
         (pps->ps_bDistort ? BST_CHECKED : BST_UNCHECKED) );
      CheckDlgButton( hDlg, IDC_CHFIT,
         (pps->ps_bFitPage ? BST_CHECKED : BST_UNCHECKED) );

      // dimension text
      CheckDlgButton( hDlg, IDC_CHADDTEXT,
         (pps->ps_bAddText ? BST_CHECKED : BST_UNCHECKED) );
      // general text
      CheckDlgButton( hDlg, IDC_CHADDTEXT2,
         (pps->ps_bAddText2 ? BST_CHECKED : BST_UNCHECKED) );
      // border text
      CheckDlgButton( hDlg, IDC_CHADDTEXT3,
         (pps->ps_bAddText3 ? BST_CHECKED : BST_UNCHECKED) );

      EnabPerChecks( hDlg, pps );

      EnableWindow( GetDlgItem(hDlg, IDC_BAPPLY), FALSE );

   }
   else
   {

Got_Err:

      EndDialog(hDlg, -1);
   }

   if( hg )
      DVGlobalUnlock(hg);

   return iRet;
}

// ======================================================
// notification from IDC_CBPRINTERS
// Only act on SELECTION CHANGED
// ======================================================
INT_PTR P2_IDC_CBPRINTERS(     // a notify from COMBOBOX
    HWND hDlg,  // handle to dialog box
    WPARAM wParam, // first message parameter
    LPARAM lParam ) // second message parameter
{
   INT_PTR  iRet = TRUE;
   DWORD    code = HIWORD(wParam);
   HGLOBAL  hg;
   PDI      lpDIBInfo;

   if( ( code == LBN_SELCHANGE             ) &&
       ( hg = GET_PROP( hDlg, ATOM_PRT )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hg) ) )
   {
      PPS  pps = (PPS)&lpDIBInfo->di_szCGenBuf2[0];

      // get new selection
      //P2_SetOK( hDlg, pps, lpDIBInfo );
      P2_SetOK( hDlg, pps );

      // and update the image
      P2_RenderDIB( hDlg, pps );

      DVGlobalUnlock(hg);
   }

   return iRet;

}

INT_PTR  P2_IDC_PROPERTIES( HWND hDlg )
{
   INT_PTR  iRet = FALSE;
   HGLOBAL  hg;
   PDI      lpDIBInfo;

   if( ( hg = GET_PROP( hDlg, ATOM_PRT )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hg) ) )
   {
      PPS  pps = (PPS)&lpDIBInfo->di_szCGenBuf2[0];
      HGLOBAL           hg2;
      PPRINTER_INFO_2   ppi2;
      LPTSTR            lppn;
      LPPRINTER_DEFAULTS pDefault = &pps->ps_sPrtDefs;
      if( ( hg2 = pps->ps_hEnum      ) &&
          ( lppn = DVGlobalLock(hg2) ) )
      {
         ppi2  = (PPRINTER_INFO_2)lppn;
         ppi2 += pps->ps_dwSel;  // get offset into structure
         if( lppn = ppi2->pPrinterName )  // if we HAVE a printer NAME
         {
            ZeroMemory( pDefault, sizeof(PRINTER_DEFAULTS) );
            pDefault->DesiredAccess = PRINTER_ACCESS_USE;
            if( OpenPrinter( lppn, &pps->ps_hPrinter, pDefault ) )
            {
               if( PrinterProperties( hDlg,  // handle to parent window
                  pps->ps_hPrinter ) )   // handle to printer object
               {
                  iRet = TRUE;
               }
               ClosePrinter( pps->ps_hPrinter );
            }
         }
         DVGlobalUnlock(hg2);
      }
      DVGlobalUnlock(hg);
   }
   return iRet;
}

INT_PTR  P2_IDC_SBCOPIES( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
   INT_PTR  iRet = FALSE;
   HGLOBAL  hg;
   PDI      lpDIBInfo;

   if( ( hg = GET_PROP( hDlg, ATOM_PRT )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hg) ) )
   {
      PPS  pps = (PPS)&lpDIBInfo->di_szCGenBuf2[0];
      DWORD cmd = LOWORD(wParam);
      DWORD code = HIWORD(wParam);

      sprtf( "DLGNOTE: cmd=%#x code=%#x hlP=%#x."MEOR, cmd, code, lParam );

      DVGlobalUnlock(hg);
   }
   return iRet;
}

INT_PTR  ToggleCheck( HWND hDlg, PDI lpDIBInfo, PPS pps,
                     UINT uChItem, LPBOOL pBool, INT nChange )
{
   INT_PTR  iRet = FALSE;
   UINT     ui;

      BOOL  b  = *pBool;

      ui = IsDlgButtonChecked( hDlg,     // handle to dialog box
         uChItem );   // button identifier
      if( ui == BST_CHECKED )
      {
         if( !b )
         {
            b = TRUE;
            iRet = TRUE;
         }
      }
      else if( ui == BST_UNCHECKED )
      {
         if( b )
         {
            b = FALSE;
            iRet = TRUE;
         }
      }

      if( iRet )
      {
         sprtf( "DLGCHTOG: Item=%#x Now %s."MEOR,
            uChItem,
            ( b ? "True" : "False" ) );
         *pBool = b;    // set NEW value
         nPrtChg[nChange] = TRUE;
         //EnabPerChecks( hDlg, pps );
         P2_RenderDIB( hDlg, pps );
      }

   return iRet;
}


INT_PTR  P2_IDC_CHDIST( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
   INT_PTR  iRet = FALSE;
   HGLOBAL  hg;
   PDI      lpDIBInfo;
   DWORD    dwi;

   if( ( hg = GET_PROP( hDlg, ATOM_PRT )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hg) ) )
   {
      PPS   pps = (PPS)&lpDIBInfo->di_szCGenBuf2[0];
      iRet = ToggleCheck( hDlg, lpDIBInfo, pps,
         IDC_CHDIST, &pps->ps_bDistort, nPrtDist );
      if( iRet )
      {
         EnabPerChecks( hDlg, pps );   // enable or disable relevant block of controls
         if( ( !pps->ps_bDistort                  ) &&
             ( pps->ps_dwPCHit != pps->ps_dwPCWid ) )
         {
            // we MUST fix this "distortion"
            if( ( pps->ps_uiActID == IDC_EDHIT   ) ||
                ( pps->ps_uiActID == IDC_EDPCHIT ) ) // percentage HEIGHT was active
            {
                     // set the SAME percentage into WIDTH
                     if( pps->ps_dwPCWid != pps->ps_dwPCHit )
                     {
                        pps->ps_dwPCWid = pps->ps_dwPCHit;
                        nPrtChg[nPrtPCW] = TRUE;
                     }
                     SetDlgItemInt( hDlg, IDC_EDPCWID, pps->ps_dwPCWid, FALSE );
                     // and get NEW width
                     dwi = (lpDIBInfo->di_dwDIBWidth * pps->ps_dwPCWid) / 100;
                     SETITEM( ps_dwWidth, dwi, nPrtWid );
                     SetDlgItemInt( hDlg, IDC_EDWID, pps->ps_dwWidth, FALSE );
            }
            else
            {
               // case IDC_EDWID or IDC_EDPCWID: // percentage WIDTH was active
               // or was neither
               if( pps->ps_dwPCHit != pps->ps_dwPCWid )
               {
                  pps->ps_dwPCHit = pps->ps_dwPCWid;
                  nPrtChg[nPrtPCH] = TRUE;
               }
                     // set the SAME percentage into HEIGHT
                     SetDlgItemInt( hDlg, IDC_EDPCHIT, pps->ps_dwPCHit, FALSE );
                     // and get NEW height
                     dwi = (lpDIBInfo->di_dwDIBHeight * pps->ps_dwPCHit) / 100;
                     SETITEM( ps_dwHeight, dwi, nPrtHit );
                     SetDlgItemInt( hDlg, IDC_EDHIT, pps->ps_dwHeight, FALSE );
            }

         }

         // but update the image anyway
         P2_RenderDIB( hDlg, pps );

      }

      DVGlobalUnlock(hg);

   }
   return iRet;
}

INT_PTR  P2_IDC_CHFIT( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
   INT_PTR  iRet = FALSE;
   HGLOBAL  hg;
   PDI      lpDIBInfo;
   if( ( hg = GET_PROP( hDlg, ATOM_PRT )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hg) ) )
   {
      PPS  pps = (PPS)&lpDIBInfo->di_szCGenBuf2[0];
      iRet = ToggleCheck( hDlg, lpDIBInfo, pps,
         IDC_CHFIT, &pps->ps_bFitPage, nPrtFit );
      if( iRet )
         EnabPerChecks( hDlg, pps );
      DVGlobalUnlock(hg);
   }
   return iRet;
}


INT_PTR  P2_IDC_CHADDTEXT( HWND hDlg, UINT cmd )
{
   INT_PTR  iRet = FALSE;
   HGLOBAL  hg;
   PDI      lpDIBInfo;
   if( ( hg = GET_PROP( hDlg, ATOM_PRT )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hg) ) )
   {
      PPS  pps = (PPS)&lpDIBInfo->di_szCGenBuf2[0];
      switch(cmd)
      {
      case IDC_CHADDTEXT:
         iRet = ToggleCheck( hDlg, lpDIBInfo, pps,
            cmd, &pps->ps_bAddText, nPrtTxt );
         break;
      case IDC_CHADDTEXT2:
         iRet = ToggleCheck( hDlg, lpDIBInfo, pps,
            cmd, &pps->ps_bAddText2, nPrtTxt2 );
         break;
      case IDC_CHADDTEXT3:
         iRet = ToggleCheck( hDlg, lpDIBInfo, pps,
            cmd, &pps->ps_bAddText3, nPrtTxt3 );
         break;
      }
      DVGlobalUnlock(hg);
   }
   return iRet;
}


// center the image on the Paint Page
// ==================================
INT_PTR  P2_IDC_BCENTER( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
   INT_PTR  iRet = FALSE;
   HGLOBAL  hg;
   PDI      lpDIBInfo;
   //RECT     rcObj, rcPPage, rcPObj;
   INT      idX, idY;
   UINT     ui;
   RECT     rcCent, rc;
   LPRECT   prcObj, prcPPage, prcPObj;
   //LPRECT   prcRend;

   if( ( hg = GET_PROP( hDlg, ATOM_PRT )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hg) ) )
   {
      PPS  pps = (PPS)&lpDIBInfo->di_szCGenBuf2[0];

      ui  = 0;

      prcObj   = &pps->ps_rcObj;
      prcPPage = &pps->ps_rcPPage;
      prcPObj  = &pps->ps_rcPObj;
      //prcRend  = &pps->ps_rcRend;
      //prcObj->left   = prcObj->top = 0;
      //prcObj->right  = lpDIBInfo->di_dwDIBWidth;
      //prcObj->bottom = lpDIBInfo->di_dwDIBHeight;
      //rcObj.left   = rcObj.top = 0;
      //rcObj.right  = lpDIBInfo->di_dwDIBWidth;
      //rcObj.bottom = lpDIBInfo->di_dwDIBHeight;

      // render the PAGE into FRAME getting PAINT PAGE/FRAME
      //GetPntPage( prcPPage, &pps->ps_rcFrame, &pps->ps_rcPage, TRUE ); // this is OK

      // render OBJECT into PAINT PAGE/FRAME per OBJECT to PAGE ratio
      //GetPntPage2( prcPObj, prcPPage, prcObj, &pps->ps_rcPage );
      rc = *prcPObj; // copy the OBJECT size on this page
      //*prcRend = *prcPObj; // copy the OBJECT size on this page
      // but apply the percentages
      idX = idY = 0;
      if( pps->ps_dwPCWid != 100 )
      {
         idX = (int)((((double)RW(prcPObj) * (double)pps->ps_dwPCWid) / 100.0)) -
            RW(prcPObj);
      }
      if( pps->ps_dwPCHit != 100 )
      {
         idY = (int)((((double)RH(prcPObj) * (double)pps->ps_dwPCHit) / 100.0)) -
            RH(prcPObj);
      }
      //prcRend->right  += idX;
      //prcRend->bottom += idY;
      rc.right  += idX;
      rc.bottom += idY;

      // EXPAND and CENTRE the OBJECT into the PAINT PAGE
      //GetPntPage( &rc, &rcPPage, &rcObj, TRUE );
      // CENTRE the PAINT OBJECT
      idX = idY = 0;
      if( RW(&rc) < RW(prcPPage) )
      {
         idX = ( RW(prcPPage) - RW(&rc) ) / 2;
      }
      if( RH(&rc) < RH(prcPPage) )
      {
         idY = ( RH(prcPPage) - RH(&rc) ) / 2;
      }

      //rcCent = *prcRend;
      rcCent = rc;
      OffsetRect( &rcCent, idX, idY );

      sprtf( "BCENTER: Put %s, now %s. Use L=%d T=%d."MEOR
         "\tat %s on %s. [rc=%s]"MEOR,
         Rect2Stg(prcObj),
         Rect2Stg(prcPObj),
         idX, idY,
         Rect2Stg(&rcCent),
         Rect2Stg(prcPPage),
         Rect2Stg( &rc ) );

      //if( rcCent.left != (INT)pps->ps_dwLeft )
      //   pps->ps_dwLeft = rcCent.left;
      //if( RW(&rcCent) != (INT)pps->ps_dwLeft )
      //   pps->ps_dwLeft = RW(&rcCent);
      if( idX != (INT)pps->ps_dwLeft )
      {
         pps->ps_dwLeft = idX;
         SetDlgItemInt( hDlg, IDC_EDLEFT,  pps->ps_dwLeft,   FALSE );
         nPrtChg[nPrtLeft] = TRUE;
         ui++;
      }

      //if( rcCent.top != (INT)pps->ps_dwTop )
      //   pps->ps_dwTop = rcCent.top;
      //if( RH(&rcCent) != (INT)pps->ps_dwTop )
      //   pps->ps_dwTop = RH(&rcCent);
      if( idY != (INT)pps->ps_dwTop )
      {
         pps->ps_dwTop = idY;
         SetDlgItemInt( hDlg, IDC_EDTOP,   pps->ps_dwTop,    FALSE );
         nPrtChg[nPrtTop] = TRUE;
         ui++;
      }

      if( ui )
      {
         P2_RenderDIB( hDlg, pps );
      }

      DVGlobalUnlock(hg);
   }
   return iRet;
}

INT_PTR  P2_IDC_BZERO( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
   INT_PTR  iRet = FALSE;
   HGLOBAL  hg;
   PDI      lpDIBInfo;
   UINT     ui = 0;

   if( ( hg = GET_PROP( hDlg, ATOM_PRT )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hg) ) )
   {
      PPS  pps = (PPS)&lpDIBInfo->di_szCGenBuf2[0];
      if( pps->ps_dwLeft )
      {
         pps->ps_dwLeft = 0;
         SetDlgItemInt( hDlg, IDC_EDLEFT,  pps->ps_dwLeft,   FALSE );
         nPrtChg[nPrtLeft] = TRUE;
         ui++;
      }
      if( pps->ps_dwTop )
      {
         pps->ps_dwTop = 0;
         SetDlgItemInt( hDlg, IDC_EDTOP,   pps->ps_dwTop,    FALSE );
         nPrtChg[nPrtTop] = TRUE;
         ui++;
      }

      if( ui )
      {
         P2_RenderDIB( hDlg, pps );
      }

      DVGlobalUnlock(hg);
   }
   return iRet;
}

///*
// * Edit Control Notification Codes
// */
// #define EN_SETFOCUS         0x0100
// #define EN_KILLFOCUS        0x0200
// #define EN_CHANGE           0x0300
// #define EN_UPDATE           0x0400
// #define EN_ERRSPACE         0x0500
// #define EN_MAXTEXT          0x0501
// #define EN_HSCROLL          0x0601
// #define EN_VSCROLL          0x0602
#ifndef  NDEBUG
//WMSTR4 sEdNote[] = {
extern   LPTSTR   DBGGetEdNote( UINT uiNote );
#endif   // !NDEBUG

INT_PTR  P2_IDC_EDITTEXT( HWND hDlg, DWORD cmd, WPARAM wParam, LPARAM lParam )
{
   INT_PTR  iRet = FALSE;
   HGLOBAL  hg;
   PDI      lpDIBInfo;
   DWORD    id   = LOWORD(wParam);  // item ID (from RC or ...)
   DWORD    code = HIWORD(wParam);  // notification code
   HWND     hWnd = (HWND)lParam;    // handle
   UINT     uiv, uivold, uiv2;
   BOOL     bOk = FALSE;

#ifndef  NDEBUG
   if( ( code == EN_KILLFOCUS ) ||
       ( code == EN_SETFOCUS )  )
   {
         LPTSTR   lpn = GetDBGNamePID( id, TRUE );
         sprtf( "EDITNOTE: From %s code = %s (%#x) hand = %#x."MEOR,
            lpn,
            DBGGetEdNote(code),
            code,
            hWnd );
   }
#endif   // !NDEBUG
   // NOTE: Only be interested in if the EDIT control LOST focus
   // Also NOTE the ps_bFitPage should be FALSE;
   // =========================================
   if( ( code == EN_SETFOCUS               ) &&
       ( hg = GET_PROP( hDlg, ATOM_PRT )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hg) ) )
   {
      PPS  pps  = (PPS)&lpDIBInfo->di_szCGenBuf2[0];
      switch(cmd)
      {
      case IDC_EDTOP:
      case IDC_EDLEFT:
      case IDC_EDWID:
      case IDC_EDPCWID:
      case IDC_EDHIT:
      case IDC_EDPCHIT:
         if( !pps->ps_bNoteOn )   // when "helpful" note is displayed
         {
            SetDlgItemText( hDlg, IDC_EDNOTE,
               "NOTE: If you change the value in the EDIT control"MEOR
               "only when you leave this edit control and"MEOR
               "the value entered is within range"MEOR
               "will the image be updated accordingly."MEOR
               "Also the Apply button indicates whether"MEOR
               "the current value is valid!" );
            pps->ps_bNoteOn = TRUE;    // when "helpful" note is displayed
         }
         pps->ps_dwOldTop = pps->ps_dwTop;   // top of print - nPrtTop
         pps->ps_dwOldLeft= pps->ps_dwLeft;  // left of print - PrtLeft
         // and sizing
         pps->ps_dwOPCWid = pps->ps_dwPCWid; // percentage width - nPrtPCW
         pps->ps_dwOPCHit = pps->ps_dwPCHit; // percentage height - nPrtPCW
         pps->ps_dwOWidth = pps->ps_dwWidth;    // pixel width of image - nPrtWid
         pps->ps_dwOHeight= pps->ps_dwHeight;   // pixel height of image
         pps->ps_uiFocus  = cmd; // EDIT control that got focus

         uiv = GetDlgItemInt( hDlg, cmd, &bOk, FALSE );
         if( ( bOk ) && ( CheckNewValue( pps, cmd, uiv ) ) )
            EnableWindow( GetDlgItem(hDlg, IDC_BAPPLY), TRUE );
         else
            EnableWindow( GetDlgItem(hDlg, IDC_BAPPLY), FALSE );

         break;
      default:
         if( pps->ps_bNoteOn )   // when "helpful" note is displayed
         {
            SetDlgItemText( hDlg, IDC_EDNOTE, "" );
            pps->ps_bNoteOn = FALSE;    // when "helpful" note is displayed
         }
         break;

      }
      DVGlobalUnlock(hg);
   }
   else if( ( code == EN_KILLFOCUS         ) &&
       ( hg = GET_PROP( hDlg, ATOM_PRT )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hg) ) )
   {
      PPS  pps  = (PPS)&lpDIBInfo->di_szCGenBuf2[0];
      switch(cmd)
      {
      case IDC_EDTOP:
         uivold = pps->ps_dwOldTop;
         break;
      case IDC_EDLEFT:
         uivold = pps->ps_dwOldLeft;
         break;
      case IDC_EDWID:
         uivold = pps->ps_dwOWid;
         break;
      case IDC_EDPCWID:
         uivold = pps->ps_dwOPCWid;
         break;
      case IDC_EDHIT:
         uivold = pps->ps_dwOHeight;
         break;
      case IDC_EDPCHIT:
         uivold = pps->ps_dwOPCHit;
         break;
      }

      switch(cmd)
      {
      case IDC_EDTOP:
      case IDC_EDLEFT:
      case IDC_EDWID:
      case IDC_EDPCWID:
      case IDC_EDHIT:
      case IDC_EDPCHIT:
         if( pps->ps_bNoteOn )   // when "helpful" note is displayed
         {
            SetDlgItemText( hDlg, IDC_EDNOTE, "" );
            pps->ps_bNoteOn = FALSE;    // when "helpful" note is displayed
         }
         uiv = GetDlgItemInt( hDlg, cmd, &bOk, FALSE );
         if( ( bOk ) &&
             ( pps->ps_uiFocus == cmd ) )
         {
            bOk = FALSE;
            if( CheckNewValue( pps, cmd, uiv ) )
            {
               bOk = TRUE;
               SetNewValue( hDlg, pps, cmd, uiv );
            }
            else if( CheckNewValue( pps, cmd, uivold ) )
            {
               if( uiv > uivold )
               {
                  uiv2 = uivold; // start at uivold value
                  while( CheckNewValue( pps, cmd, uiv2 ) )
                  {
                     uiv2++;  // and head upwards to largest possible
                     if( uiv2 > uiv )
                        break;
                  }
                  uiv2--;
                  if( CheckNewValue( pps, cmd, uiv2 ) )
                  {
                     bOk = TRUE;
                     SetNewValue( hDlg, pps, cmd, uiv2 );
                  }
                  else
                  {
                     SetNewValue( hDlg, pps, cmd, uivold );
                  }
               }
               else
               {
                  uiv2 = uivold; // start at uivold value
                  while( CheckNewValue( pps, cmd, uiv2 ) )
                  {
                     uiv2--;  // and go down to smallest possible
                     if( uiv2 == 0 )
                        break;
                  }
                  uiv2++;
                  if( CheckNewValue( pps, cmd, uiv2 ) )
                  {
                     bOk = TRUE;
                     SetNewValue( hDlg, pps, cmd, uiv2 );
                  }
                  else
                  {
                     SetNewValue( hDlg, pps, cmd, uivold );
                  }
               }
            }
            else
            {
               chkme( "HEY: The OLD %d value and NEW of %d FAILED!!!",
                  uivold,
                  uiv );
               // but set it anyway
               SetNewValue( hDlg, pps, cmd, uivold );
            }
         }
         else if( bOk )
         {
            sprtf( "HEY THERE! Losing FOCUS from %s but %s had it!!!"MEOR,
               GetDBGNamePID( cmd, TRUE ),
               GetDBGNamePID( pps->ps_uiFocus, TRUE ) );
         }
         break;
      default:
         if( pps->ps_bNoteOn )   // when "helpful" note is displayed
         {
            SetDlgItemText( hDlg, IDC_EDNOTE, "" );
            pps->ps_bNoteOn = FALSE;    // when "helpful" note is displayed
         }
         break;
      }

      EnableWindow( GetDlgItem(hDlg, IDC_BAPPLY), FALSE );

      DVGlobalUnlock(hg);
   }
   else if( ( code == EN_UPDATE            ) &&
       ( hg = GET_PROP( hDlg, ATOM_PRT )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hg) ) )
   {
      PPS  pps  = (PPS)&lpDIBInfo->di_szCGenBuf2[0];
      switch(cmd)
      {
      case IDC_EDTOP:
      case IDC_EDLEFT:
      case IDC_EDWID:
      case IDC_EDPCWID:
      case IDC_EDHIT:
      case IDC_EDPCHIT:
         uiv = GetDlgItemInt( hDlg, cmd, &bOk, FALSE );
         if( bOk )
         {
            if( CheckNewValue( pps, cmd, uiv ) )
            {
               EnableWindow( GetDlgItem(hDlg, IDC_BAPPLY), TRUE );
            }
            else
            {
               EnableWindow( GetDlgItem(hDlg, IDC_BAPPLY), FALSE );
            }
         }
         break;
      }
      DVGlobalUnlock(hg);
   }
   return iRet;
}

INT_PTR P2_Command(
    HWND hDlg,  // handle to dialog box
    WPARAM wParam, // first message parameter
    LPARAM lParam ) // second message parameter
{
   INT_PTR iRet = TRUE;
   DWORD    cmd = LOWORD(wParam);
   switch(cmd)
   {

   case IDOK:
      EndDialog(hDlg,IDOK);   // User clicked on PRINT
      break;

   case IDCANCEL:
      EndDialog(hDlg,0);  // User clicked on CANCEL
      break;

   case IDC_CBPRINTERS:
      P2_IDC_CBPRINTERS( hDlg, wParam, lParam ); // a notify from COMBOBOX
      break;

   case IDC_BPROPERTIES:
      iRet = P2_IDC_PROPERTIES( hDlg );   // run the PRINTER properties tab control
      break;

   case IDC_SBCOPIES:
      P2_IDC_SBCOPIES( hDlg, wParam, lParam );  // a notify from COPIES EDIT control
      break;

   case IDC_CHDIST:
      P2_IDC_CHDIST( hDlg, wParam, lParam ); // a notify from DISTORT check button
      break;

   case IDC_CHFIT:
      P2_IDC_CHFIT( hDlg, wParam, lParam );  // a notify from FIT-TO-PAGE check button
      break;

   case IDC_CHADDTEXT:
   case IDC_CHADDTEXT2:
   case IDC_CHADDTEXT3:
      P2_IDC_CHADDTEXT( hDlg, cmd );  // a notify from ADD TEXT check button
      break;

   case IDC_BCENTER:
      P2_IDC_BCENTER( hDlg, wParam, lParam );   // a notify from CENTER button
      break;

   case IDC_BZERO:
      P2_IDC_BZERO( hDlg, wParam, lParam );
      break;

   case IDC_BAPPLY:
      // NOTE: This action alone will have shifted the focus, so the
      // edit control values have already been updated!!! So ...
      EnableWindow( GetDlgItem(hDlg, IDC_BAPPLY), FALSE );
      break;

   case IDC_EDTOP:
   case IDC_EDLEFT:
   case IDC_EDWID:
   case IDC_EDPCWID:
   case IDC_EDHIT:
   case IDC_EDPCHIT:
   case IDC_EDCOPIES:
      P2_IDC_EDITTEXT( hDlg, cmd, wParam, lParam );   // notify from EDIT controls
      break;

   }
   return iRet;
}

// for a frame Xf x Yf
// and an obj  Xp x Yp
// find the   Xn x Yn
// will fit entirely within Xf x Yf without distorting the obj Xp x Yp
// if tan @ = Yp / Xp
// then Yp = Xp * tan @
// since any Yn = Xn * tan @
// thus Ynew = ( Xnew * Yp ) / Xp
// or   Xn = Yn / ( Yp / Xp )
// or   Xnew = ( Ynew * Xp ) / Yp
// =================
// try using the Yf value
// then Xt = ( Yf * Xp ) / Yp
// thus Yt = ( Xt * Yp ) / Xp
// conversly try using the Xf value
// then Yt = ( Xf * Yp ) / Xp
// thus Xt = ( Yt * Xp ) / Yp
// which of these two RESULT is best
// here I want the PAGE to be "fitted" to the FRAME
// the RESULT in PPage
//#define  RW    RECTWIDTH
//#define  RH    RECTHEIGHT

VOID  GetPntPage( LPRECT prcPPage, LPRECT prcFrame, LPRECT prcPage, BOOL bCentre )
{
   INT   Yt1, Yt2;
   INT   Xt1, Xt2;
   RECT  rc1, rc2;
   INT   idX, idY;

   // calc 1
   Xt1 = ( RH(prcFrame) * RW(prcPage) ) / RH(prcPage);
   Yt1 = ( Xt1          * RH(prcPage) ) / RW(prcPage);
   // calc 2
   Yt2 = ( RW(prcFrame) * RH(prcPage) ) / RW(prcPage);
   Xt2 = ( Yt2          * RW(prcPage) ) / RH(prcPage);

   rc1.top    = prcFrame->top;
   rc1.left   = prcFrame->left;
   rc1.right  = rc1.left + Xt1;
   rc1.bottom = rc1.top  + Yt1;

   //rc2.top = rc2.left = 0;
   rc2.top    = prcFrame->top;
   rc2.left   = prcFrame->left;
   rc2.right  = rc2.left + Xt2;
   rc2.bottom = rc2.top  + Yt2;

   //sprtf( "Fi1 %s in %s gives %s."MEOR,
   //   Rect2Stg(prcPage),
   //   Rect2Stg(prcFrame),
   //   Rect2Stg(&rc1) );

   //sprtf( "Fi2 %s in %s gives %s."MEOR,
   //   Rect2Stg(prcPage),
   //   Rect2Stg(prcFrame),
   //   Rect2Stg(&rc2) );

   if( ( Xt1 <= RW(prcFrame) ) && ( Yt1 <= RH(prcFrame) ) )
   {
      *prcPPage = rc1;
   }
   else
   {
      *prcPPage = rc2;
   }

   if( bCentre )
   {
         if( RW(prcPPage) < RW(prcFrame) )
         {
            idX = ( RW(prcFrame) - RW(prcPPage) ) / 2;
            prcPPage->left   += idX;   // shift to right
            prcPPage->right  += idX;
         }

         if( RH(prcPPage) < RH(prcFrame) )
         {
            idY = ( RH(prcFrame) - RH(prcPPage) ) / 2;
            prcPPage->top    += idY;   // shift down
            prcPPage->bottom += idY;
         }
   }

   sprtf( "FiR %s in %s gives %s."MEOR,
      Rect2Stg(prcPage),
      Rect2Stg(prcFrame),
      Rect2Stg(prcPPage) );

}

// get the object PAINT rectangle as part of the FRAME
// in the ratio of an OBJECT rectangle as part of a PAGE rectangle
// ===============================================================
VOID  GetPntPage2( LPRECT prcPObj, LPRECT prcPPage, LPRECT prcObj, LPRECT prcPage )
{
   int   iCond = 0;

   prcPObj->left = prcPObj->right = prcPObj->top = prcPObj->bottom = 0;  // zero dest. rectange
   // deal with the EASY stuff first
   //if( ( RW(prcObj) <= RW(prcPage) ) &&
   //    ( RH(prcObj) <= RH(prcPage) ) )
   {
      // object is less in width and height,
      // so it is only a ratio question
      //prcPObj->right  = prcFrame->right * prcObj->right / prcPage->right;
      //prcPObj->bottom = pcrFrame->bottom * prcObj->bottom / prcPage->bottom;
      // or more generalised
      if( RW(prcPage) > 0 )
         prcPObj->right  = (RW(prcPPage) * RW(prcObj)) / RW(prcPage);

      if( RH(prcPage) > 0 )
         prcPObj->bottom = (RH(prcPPage) * RH(prcObj)) / RH(prcPage);

      //sprtf( "Obj rect %s from %s"MEOR
      //   "\tas %s in %s."MEOR,
      //   Rect2Stg(prcPObj),
      //   Rect2Stg(prcPPage),
      //   Rect2Stg(prcObj),
      //   Rect2Stg(prcPage) );

      OffsetRect( prcPObj, prcPPage->left, prcPPage->top );

      //sprtf( "After Offset: Obj rect %s from %s"MEOR
      //   "\tas %s in %s."MEOR,
      //   Rect2Stg(prcPObj),
      //   Rect2Stg(prcPPage),
      //   Rect2Stg(prcObj),
      //   Rect2Stg(prcPage) );

      // condition the rectangle
      if( prcPObj->left < prcPPage->left )
      {
         prcPObj->left = prcPPage->left;
         iCond |= 0x1;
      }

      if( prcPObj->left > prcPPage->right )
      {
         prcPObj->left = prcPPage->right;
         iCond |= 0x2;
      }

      if( prcPObj->right > prcPPage->right )
      {
         prcPObj->right = prcPPage->right;
         iCond |= 0x4;
      }

      if( prcPObj->top < prcPPage->top )
      {
         prcPObj->top = prcPPage->top;
         iCond |= 0x8;
      }

      if( prcPObj->top > prcPPage->bottom )
      {
         prcPObj->top = prcPPage->bottom;
         iCond |= 0x10;
      }

      if( prcPObj->bottom > prcPPage->bottom )
      {
         prcPObj->bottom = prcPPage->bottom;
         iCond |= 0x20;
      }

      sprtf( "PObj: %s in %s"MEOR
         "\tas %s to %s. (c=%#x)"MEOR,
         Rect2Stg(prcPObj),
         Rect2Stg(prcPPage),
         Rect2Stg(prcObj),
         Rect2Stg(prcPage),
         iCond );

   }
}

BOOL  GetPrtRect( LPRECT prcPrt, LPRECT prcDPage, LPRECT prcPObj, LPRECT prcPPage )
{
   BOOL  bRet = FALSE;

   prcPrt->left = prcPrt->top = prcPrt->right = prcPrt->bottom = 0;
   if( ( RW(prcPPage) > 0 ) && ( RH(prcPPage) > 0 ) )
   {
      int   iOx, iOy;
      prcPrt->right  = (int)(((double)RW(prcPObj) * (double)RW(prcDPage)) / 
         (double)RW(prcPPage));

      prcPrt->bottom = (int)(((double)RH(prcPObj) * (double)RH(prcDPage)) /
         (double)RH(prcPPage));

      sprtf( "Prt1: W=%d H=%d on %s from %s:%s"MEOR,
         prcPrt->right,
         prcPrt->bottom,
         Rect2Stg(prcDPage),
         Rect2Stg(prcPObj),
         Rect2Stg(prcPPage) );


      iOx = prcPObj->left - prcPPage->left;
      iOx = (int)(( (double)iOx * (double)RW(prcDPage) ) /
         (double)RW(prcPPage));
      iOy = prcPObj->top  - prcPPage->top;
      iOy = (int)(( (double)iOy * (double)RH(prcDPage) ) /
         (double)RH(prcPPage));

      OffsetRect( prcPrt, iOx, iOy );

      sprtf( "Prt2: Applying (%d,%d) give print rect %s."MEOR,
         iOx, iOy,
         Rect2Stg(prcPrt) );

      bRet = TRUE;

   }

   return bRet;

}

// ==================
//lfEscapement This is an angle in tenths of a degree, measured
//from the horizontal in a counterclockwise direction. It
//specifies how the successive characters of a string are placed
//when you write text. Here are some examples: 
//Value Placement of Characters 
//0 Run from left to right (default) 
//900 Go up 
//1800 Run from right to left 
//2700 Go down 
//In Windows 98, this value sets both the escapement and
//orientation of TrueType text. In Windows NT, this value also
//normally sets both the escapement and orientation of TrueType
//text, except when you call SetGraphicsMode with the GM_ADVANCED
//argument, in which case it works as documented. 
//
//lfOrientation This is an angle in tenths of a degree, measured
//from the horizontal in a counterclockwise direction. It affects
//the appearance of each individual character. Here are some
//examples: 
//Value Character Appearance 
//0 Normal (default) 
//900 Tipped 90 degrees to the right 
//1800 Upside down 
//2700 Tipped 90 degrees to the left 
//This field has no effect except with a TrueType font under
//Windows NT with the graphics mode set to GM_ADVANCED, in which
//case it works as documented. 
// maybe via WM_GETFONT WM_SETFONT
// FontInitialize
// D:\GTools\Tools\ewm\ewm.c(3440):
//   GetObject (GetStockObject (SYSTEM_FIXED_FONT), sizeof (LOGFONT),
//                (LPSTR) &gsLogFont);
//   ghFixedFont = CreateFontIndirect(&gsLogFont);
// see VOID  P2_InitFont( HWND hDlg, PPS pps )
//    SIZE              ps_ssFA;    // string size
//   SIZE              ps_ssF2;    // string size

// ============================================
// VOID  PntPtType( HDC hdc, PPTARR ppta )
//
// painting of a simple arrow head
// it can be type
// left, like |<
// where pt1 |      pt4
//           |    /
//           |-pt3
//           |    \
//       pt2 |      pt5
// right, like >|
// where the above points are inverted
// top, like _
//           ^
// where pt1 -- pt3 -- pt2
//               |
//            /     \
//       pt4           pt5
// or down \/
//         --
// where the top points are inverted
// ============================================
VOID  PntPtType( HDC hdc, PPTARR ppta )
{
   BOOL     bAct = FALSE;
   POINT    pt1, pt2, pt3, pt4, pt5;
   PPOINT   ppt = &ppta->pa_Pt;
   int      iX, iY;

   iX = iY = 5;
   pt3.x = ppt->x;
   pt3.y = ppt->y;
   switch( ppta->pa_Type )
   {
   case ptt_Left:
      iX = 10;
      pt1.x = pt3.x;
      pt1.y = pt3.y - iY;
      pt2.x = pt3.x;
      pt2.y = pt3.y + iY;

      pt4.x = pt3.x + iX;
      pt4.y = pt1.y;
      pt5.x = pt4.x;
      pt5.y = pt2.y;
      bAct = TRUE;
      break;
   case ptt_Rite:
      iX = 10;
      pt1.x = pt3.x;
      pt1.y = pt3.y - iY;
      pt2.x = pt3.x;
      pt2.y = pt3.y + iY;

      pt4.x = pt3.x - iX;
      pt4.y = pt1.y;
      pt5.x = pt4.x;
      pt5.y = pt2.y;
      bAct = TRUE;
      break;

   case ptt_Up:
      iY = 10;
      pt1.x = pt3.x - iX;
      pt1.y = pt3.y;
      pt2.x = pt3.x + iX;
      pt2.y = pt3.y;

      pt4.x = pt1.x;
      pt4.y = pt3.y + iY;
      pt5.x = pt2.x;
      pt5.y = pt4.y;
      bAct = TRUE;
      break;

   case ptt_Down:
      iY = 10;
      pt1.x = pt3.x - iX;
      pt1.y = pt3.y;
      pt2.x = pt3.x + iX;
      pt2.y = pt3.y;

      pt4.x = pt1.x;
      pt4.y = pt3.y - iY;
      pt5.x = pt2.x;
      pt5.y = pt4.y;
      bAct = TRUE;
      break;
   }

   if( bAct )
   {
      MoveToEx( hdc,  // handle to device context
         pt1.x,             // x-coordinate of new current position
         pt1.y,             // y-coordinate of new current position
         NULL );           // old current position
      LineTo( hdc, pt2.x, pt2.y );

      MoveToEx( hdc,  // handle to device context
         pt3.x,             // x-coordinate of new current position
         pt3.y,             // y-coordinate of new current position
         NULL );           // old current position
      LineTo( hdc, pt4.x, pt4.y );

      MoveToEx( hdc,  // handle to device context
         pt3.x,             // x-coordinate of new current position
         pt3.y,             // y-coordinate of new current position
         NULL );           // old current position
      LineTo( hdc, pt5.x, pt5.y );

   }
}

VOID  PutScaleText( HDC hdc, PPS pps )
{
   LPRECT   prcDlgFrm;
   LPRECT   prcPPage;
   HFONT    hOld;
   HPEN     hOldPen;
   LPPOINT  ppt, ppt1, ppt2;
   PPTARR   ppta1, ppta2;

   prcDlgFrm = &pps->ps_rcDlgFrm;
   prcPPage  = &pps->ps_rcPPage;

   hOldPen = SelectObject( hdc, GetStockObject(WHITE_PEN) );
   // OutText(
   if( ( (RH(prcDlgFrm) - RH(prcPPage)) > 10    ) &&
       ( pps->ps_hFontA                         ) &&
       ( pps->ps_szBotTxt[0] && pps->ps_iBotTxt ) )
   {
      //rc.left   = 0;
      //rc.right  = prcDlgFrm->right;
      //rc.top    = prcPPage->bottom;
      //rc.bottom = prcDlgFrm->bottom;
      //wsprintf( lpt, "%d Px", pps->ps_dxRes ); // = GetDeviceCaps( hdcTarget, HORZRES   ); // in Pixels
      // pps->ps_dyRes, // = GetDeviceCaps( hdcTarget, VERTRES   );
      //i = lstrlen(lpt);

      hOld = SelectObject( hdc, pps->ps_hFontA );
      SetTextColor(hdc, RGB(255,255,255) );
      SetBkColor(hdc, RGB( 65, 65, 65 )  );

      //pt.x = ( rc.left + rc.right ) / 2;
      //if( pt.x > (pps->ps_ssFA.cx / 2) )
      //   pt.x -= (pps->ps_ssFA.cx / 2);
      //else
      //   pt.x  = 0;

      //pt.y = ( rc.top + rc.bottom ) / 2;
      //if( pt.y > ( pps->ps_ssFA.cy / 2 ) )
      //   pt.y -= ( pps->ps_ssFA.cy / 2 );
      //else
      //   pt.y  = 0;
      //DrawText( hdc, // handle to DC
      //   lpt,  // text to draw
      //   i,    // text length
      //   &rc, // formatting dimensions
      //   DT_SINGLELINE | DT_VCENTER | DT_CENTER ); // text-drawing options
      ppt = &pps->ps_ptBotTxt;
      TextOut( hdc,
         ppt->x,
         ppt->y,
         &pps->ps_szBotTxt[0],
         pps->ps_iBotTxt );

      hOld = SelectObject( hdc, hOld );

      //ppt1 = &pps->ps_ptArray[pt_BotLef];
      //ppt2 = ppt1 + 1;
      ppta1 = &pps->ps_ptArray[pt_BotLef];
      ppta2 = ppta1 + 1;
      ppt1 = &ppta1->pa_Pt;
      ppt2 = &ppta2->pa_Pt;
      MoveToEx( hdc,  // handle to device context
         ppt1->x,             // x-coordinate of new current position
         ppt1->y,             // y-coordinate of new current position
         NULL );           // old current position
      LineTo( hdc, ppt2->x, ppt2->y );
      if( ppta1->pa_Type != ptt_None )
         PntPtType( hdc, ppta1 );
      if( ppta2->pa_Type != ptt_None )
         PntPtType( hdc, ppta2 );

      //ppt1 = &pps->ps_ptArray[pt_BotRit];
      //ppt2 = ppt1 + 1;
      ppta1 = &pps->ps_ptArray[pt_BotRit];
      ppta2 = ppta1 + 1;
      ppt1 = &ppta1->pa_Pt;
      ppt2 = &ppta2->pa_Pt;
      MoveToEx( hdc,  // handle to device context
         ppt1->x,             // x-coordinate of new current position
         ppt1->y,             // y-coordinate of new current position
         NULL );           // old current position
      LineTo( hdc, ppt2->x, ppt2->y );
      if( ppta1->pa_Type != ptt_None )
         PntPtType( hdc, ppta1 );
      if( ppta2->pa_Type != ptt_None )
         PntPtType( hdc, ppta2 );

   }
   else
   {
      if( (RH(prcDlgFrm) - RH(prcPPage)) > 10 )
      {
         if( pps->ps_hFontA )
         {
            sprtf( "WARNING: No text since TEXT not initialised!"MEOR );
         }
         else
         {
            sprtf( "WARNING: No text since ps_hFontA not initialised!"MEOR );
         }
      }
      else
      {
         sprtf( "WARNING: No text since HT < 10 [%s & %s]!"MEOR,
            Rect2Stg(prcDlgFrm),
            Rect2Stg(prcPPage) );
      }
   }


   if( ( (RW(prcDlgFrm) - RW(prcPPage)) > 10    ) &&
       ( pps->ps_hFontV                         ) &&
       ( pps->ps_szRitTxt[0] && pps->ps_iRitTxt ) )
   {
      //rc.left   = prcPPage->right;
      //rc.right  = prcDlgFrm->right;
      //rc.top    = 0;
      //rc.bottom = prcDlgFrm->bottom;

      //wsprintf( lpt, "%d Px", pps->ps_dyRes ); // = GetDeviceCaps( hdcTarget, VERTRES) in pixels
      //i = lstrlen(lpt);

      hOld = SelectObject( hdc, pps->ps_hFontV );
      SetTextColor(hdc, RGB(255,255,255) );
      SetBkColor(hdc, RGB( 65, 65, 65 )  );
      //SetBkMode( hdc, TRANSPARENT );

      //pt.x = ( rc.left + rc.right ) / 2;
      //if( (pt.x + (pps->ps_ssFA.cy / 2)) < rc.right )
      //   pt.x += (pps->ps_ssFA.cy / 2);
      //else
      //   pt.x  = rc.right;

      //pt.y = ( rc.top + rc.bottom ) / 2;
      //if( pt.y > ( pps->ps_ssFA.cx / 2 ) )
      //   pt.y -= ( pps->ps_ssFA.cx / 2 );
      //else
      //   pt.y  = 0;
      //DrawText( hdc, // handle to DC
      //   lpt,  // text to draw
      //   i,    // text length
      //   &rc, // formatting dimensions
      //   DT_SINGLELINE | DT_VCENTER | DT_CENTER ); // text-drawing options
      ppt = &pps->ps_ptRitTxt;
      TextOut( hdc,
         ppt->x,
         ppt->y,
         &pps->ps_szRitTxt[0],
         pps->ps_iRitTxt );

      hOld = SelectObject( hdc, hOld );
      //SetBkMode( hdc, OPAQUE );

      //ppt1 = &pps->ps_ptArray[pt_RitTop];
      //ppt2 = ppt1 + 1;
      ppta1 = &pps->ps_ptArray[pt_RitTop];
      ppta2 = ppta1 + 1;
      ppt1 = &ppta1->pa_Pt;
      ppt2 = &ppta2->pa_Pt;
      MoveToEx( hdc,  // handle to device context
         ppt1->x,             // x-coordinate of new current position
         ppt1->y,             // y-coordinate of new current position
         NULL );           // old current position
      LineTo( hdc, ppt2->x, ppt2->y );
      if( ppta1->pa_Type != ptt_None )
         PntPtType( hdc, ppta1 );
      if( ppta2->pa_Type != ptt_None )
         PntPtType( hdc, ppta2 );

      //ppt1 = &pps->ps_ptArray[pt_RitBot];
      //ppt2 = ppt1 + 1;
      ppta1 = &pps->ps_ptArray[pt_RitBot];
      ppta2 = ppta1 + 1;
      ppt1 = &ppta1->pa_Pt;
      ppt2 = &ppta2->pa_Pt;
      MoveToEx( hdc,  // handle to device context
         ppt1->x,             // x-coordinate of new current position
         ppt1->y,             // y-coordinate of new current position
         NULL );           // old current position
      LineTo( hdc, ppt2->x, ppt2->y );
      if( ppta1->pa_Type != ptt_None )
         PntPtType( hdc, ppta1 );
      if( ppta2->pa_Type != ptt_None )
         PntPtType( hdc, ppta2 );

   }
   else
   {
      if( (RW(prcDlgFrm) - RW(prcPPage)) > 10 )
      {
         if( pps->ps_hFontV )
         {
            sprtf( "WARNING: No text since TEXT not initialised!"MEOR );
         }
         else
         {
            sprtf( "WARNING: No text since ps_hFontV not initialised!"MEOR );
         }
      }
      else
      {
         sprtf( "WARNING: No text since WD < 10 [%s & %s]!"MEOR,
            Rect2Stg(prcDlgFrm),
            Rect2Stg(prcPPage) );
      }
   }

   SelectObject( hdc, hOldPen );

}


VOID  GetRenderSize( PPS pps )
{
   LPRECT   prcPPage, prcObj, prcPObj, prcRend;
   LPRECT   prcDPage, prcBPage, prcPFrame;
   LPRECT   prcPrt;

   //prcDPage  = &pps->ps_rcTPage;  // in twips
   prcDPage  = &pps->ps_rcDPage;  // in device units
   prcBPage  = &pps->ps_rcBPage;  // in device units minus BORDER

   {
      prcObj    = &pps->ps_rcObj;
      prcPPage  = &pps->ps_rcPPage;
      prcPObj   = &pps->ps_rcPObj;
      prcPFrame = &pps->ps_rcPFrame;

      prcRend  = &pps->ps_rcTmpRend;
      prcPrt   = &pps->ps_rcTmpPrt;

      if( pps->ps_bChgPrt )
      {
         GetPntPage( prcPPage, prcPFrame, prcDPage, TRUE ); // this is OK
         GetPntPage2( prcPObj, prcPPage, prcObj, prcDPage );
         pps->ps_bChgPrt = FALSE;
      }

      //sprtf( "REND: Obj to page %s (D=%s F=%s) F=%s (L=%d,T=%d)"MEOR,
      //   Rect2Stg( prcDPage ),    // &pps->ps_rcPage ),
      //   ( pps->ps_bDistort ? "T" : "F"),
      //   ( pps->ps_bFitPage ? "T" : "F"),
      //   Rect2Stg( prcPFrame ),
      //   pps->ps_dwLeft,
      //   pps->ps_dwTop );

      {
         if( pps->ps_bDistort && pps->ps_bFitPage )
         {
            //rc = rcPPage;  // just spread it to ALL the PAINT PAGE
            *prcRend = *prcPPage;  // just spread it to ALL the PAINT PAGE
         }
         else if( pps->ps_bFitPage )
         {
            // EXPAND and CENTRE the OBJECT into the PAINT PAGE
            GetPntPage( prcRend, prcPPage, prcObj, TRUE );
         }
         else
         {
            int   iWid, iHit;
            // paint natural
            //rc = rcPObj;
            *prcRend = *prcPObj;
            // but apply the percentages
            iWid = iHit = 0;
            if( pps->ps_dwPCWid != 100 )
            {
               iWid = (int)((((double)RW(prcPObj) * (double)pps->ps_dwPCWid) / 100.0)) -
                  RW(prcPObj);
            }
            if( pps->ps_dwPCHit != 100 )
            {
               iHit = (int)((((double)RH(prcPObj) * (double)pps->ps_dwPCHit) / 100.0)) -
                  RH(prcPObj);
            }

            prcRend->right  += iWid;
            prcRend->bottom += iHit;

            // and apply the offsets like
            OffsetRect( prcRend, pps->ps_dwLeft, pps->ps_dwTop );

         }

         GetPrtRect( prcPrt, prcBPage, prcRend, prcPPage );

         //sprtf( "Stretch: Obj %s as %s on pg %s."MEOR
         //   "\tgives %s on %s finally"MEOR,
         //   Rect2Stg( prcObj ),
         //   Rect2Stg( prcRend ),
         //   Rect2Stg( prcPPage ),
         //   Rect2Stg( prcPrt ),
         //   Rect2Stg( prcBPage ) );
      }
   }
}

            //SetDIBitsToDevice( hdc,	// hDC
            //   rc.left,          // DestX
			   //   rc.top,		      // DestY
			   //   RECTWIDTH(&rc),   // nDestWidth
			   //   RECTHEIGHT(&rc),  // nDestHeight
			   //   0,	               // SrcX
			   //   0,	               // SrcY
			   //   0,					   // nStartScan
			   //   gdwDIBHeight,	   // nNumScans
			   //   FindDIBBits(lpDIB),		// lpBits
            //   (LPBITMAPINFO)lpDIB, 	// lpBitsInfo
            //   DIB_RGB_COLORS );	// wUsage

VOID  P2_RenderDIB( HWND hDlg, PPS pps )
{
   HWND     hWnd;
   HDC      hdc, hdc2;
   LPRECT   prcPPage, prcObj, prcPObj, prcRend;
   LPRECT   prcDPage, prcBPage, prcPFrame;
   LPRECT   prcPrt;
   RECT     rcr, rcp;   // rendering and printing WORK rectangles

   //prcDPage  = &pps->ps_rcTPage;  // in twips
   prcDPage  = &pps->ps_rcDPage;  // in device units
   prcBPage  = &pps->ps_rcBPage;  // in device units minus BORDER
   // GetDC vs GetWindowDC
   // ( GetClientRect( hWnd, &pps->ps_rcFrame ) ) &&
   if( ( hWnd = GetDlgHand( IDC_FRMA4 )  ) &&
       ( hdc2 = GetDC( hWnd )             ) )
   {
      int      ir;
      HGLOBAL  hDIB;
      LPSTR    lpDIB;
      HBITMAP  hbm, hbmo;

      prcObj    = &pps->ps_rcObj;
      prcPPage  = &pps->ps_rcPPage;
      prcPObj   = &pps->ps_rcPObj;
      prcPFrame = &pps->ps_rcPFrame;

      //prcRend  = &pps->ps_rcRend;
      prcRend  = &rcr;
      //prcPrt   = &pps->ps_rcPrt;
      prcPrt   = &rcp;
      //prcObj->left   = prcObj->top = 0;
      //prcObj->right  = lpDIBInfo->di_dwDIBWidth;
      //prcObj->bottom = lpDIBInfo->di_dwDIBHeight;
      //sprtf( "Object %dx%d into paint page frame %s as %s"MEOR,
      //   lpDIBInfo->di_dwDIBWidth,
      //   lpDIBInfo->di_dwDIBHeight,
      //   Rect2Stg( &rcPPage ),
      //   Rect2Stg( &rcPObj ) );
      if( pps->ps_bChgPrt )
      {
         // WE NEED TO RE-DO THE CALCULATIONS
         // render the PAGE into FRAME getting PAINT PAGE/FRAME
         //GetPntPage( &rcPPage, &pps->ps_rcFrame, &pps->ps_rcPage, TRUE ); // this is OK
         //GetPntPage( prcPPage, &pps->ps_rcFrame, &pps->ps_rcPage, TRUE ); // this is OK
         GetPntPage( prcPPage, prcPFrame, prcDPage, TRUE ); // this is OK

         // render OBJECT into PAINT PAGE/FRAME per OBJECT to PAGE ratio
         //GetPntPage2( &rcPObj, &rcPPage, &rcObj, &pps->ps_rcPage );
         //GetPntPage2( prcPObj, prcPPage, prcObj, &pps->ps_rcPage );
         GetPntPage2( prcPObj, prcPPage, prcObj, prcDPage );

         pps->ps_bChgPrt = FALSE;

      }

      sprtf( "REND: Obj %dx%d to page %s (D=%s F=%s) F=%s (L=%d,T=%d)"MEOR,
         pps->ps_dwOWid,    // lpDIBInfo->di_dwDIBWidth,
         pps->ps_dwOHit,    // lpDIBInfo->di_dwDIBHeight,
         Rect2Stg( prcDPage ),    // &pps->ps_rcPage ),
         ( pps->ps_bDistort ? "T" : "F"),
         ( pps->ps_bFitPage ? "T" : "F"),
         Rect2Stg( prcPFrame ),
         pps->ps_dwLeft,
         pps->ps_dwTop );

      hbm = hbmo = 0;
      if( hdc = CreateCompatibleDC(hdc2) )
      {
         if( hbm = CreateCompatibleBitmap( hdc2, pps->ps_rcDlgFrm.right, pps->ps_rcDlgFrm.bottom ) )
         {
            hbmo = SelectObject( hdc, hbm );
         }
         else
         {
            DeleteDC(hdc);
            hdc = hdc2;
         }
      }
      else
      {
         hdc = hdc2;
      }

      //FillRect( hdc, &rcFrame, GetStockObject(WHITE_BRUSH) );

      // fill the WHOLE FRAME with BLACK
      //FillRect( hdc, &pps->ps_rcFrame, GetStockObject(BLACK_BRUSH) );
      FillRect( hdc, &pps->ps_rcDlgFrm, GetStockObject(BLACK_BRUSH) );

      // fill the PAINT PAGE with GRAY
      //FillRect( hdc, prcPPage, GetStockObject(WHITE_BRUSH) );
      // NOTE: When filling the specified rectangle, FillRect
      // does not include the rectangle's right and bottom sides.
      // GDI fills a rectangle up to, but not including,
      // the right column and bottom row, regardless of the current mapping mode.
      // so
      *prcRend = *prcPPage;   // copy the rectangle
      prcRend->right  += 1;   // add one to right
      prcRend->bottom += 1;   // and bottom
      FillRect( hdc, prcRend, GetStockObject(WHITE_BRUSH) );

      // fill the PAGE AREA with WHITE
      //FillRect( hdc, prcPPage, GetStockObject(WHITE_BRUSH) );

      SetStretchBltMode( hdc, COLORONCOLOR );

      if( ( hDIB  = pps->ps_hDIB       ) &&
          ( lpDIB = DVGlobalLock(hDIB) ) )
      {
         //gdwDIBWidth  = DIBWidth(lpDIB);
         //gdwDIBHeight = DIBHeight(lpDIB);
         //gdwDIBBPP    = DIBBitCount(lpDIB);
         if( pps->ps_bDistort && pps->ps_bFitPage )
         {
            //rc = rcPPage;  // just spread it to ALL the PAINT PAGE
            *prcRend = *prcPPage;  // just spread it to ALL the PAINT PAGE
         }
         else if( pps->ps_bFitPage )
         {
            // EXPAND and CENTRE the OBJECT into the PAINT PAGE
            //GetPntPage( &rc, &rcPPage, &rcObj, TRUE );
            GetPntPage( prcRend, prcPPage, prcObj, TRUE );
         }
         else
         {
            int   iWid, iHit;
            // paint natural
            //rc = rcPObj;
            *prcRend = *prcPObj;
            // but apply the percentages
            iWid = iHit = 0;
            //iWid = RW(prcPObj);
            //iHit = RH(prcPObj);
            //if( pps->ps_bDistort )
            //{
            if( pps->ps_dwPCWid != 100 )
            {
               iWid = (int)((((double)RW(prcPObj) * (double)pps->ps_dwPCWid) / 100.0)) -
                  RW(prcPObj);
            }
            if( pps->ps_dwPCHit != 100 )
            {
               iHit = (int)((((double)RH(prcPObj) * (double)pps->ps_dwPCHit) / 100.0)) -
                  RH(prcPObj);
            }
            //}

            prcRend->right  += iWid;
            prcRend->bottom += iHit;

            // and apply the offsets like
            OffsetRect( prcRend, pps->ps_dwLeft, pps->ps_dwTop );

         }

         ir = StretchDIBits( hdc,	// hDC
                             prcRend->left,  // rc.left,	// DestX
                             prcRend->top,   // rc.top,   // DestY
               			     RW(prcRend), // RW(&rc),  // nDestWidth
			                    RH(prcRend), // RH(&rc),  // nDestHeight
                             pps->ps_rcDIB.left,   // = 0 - SrcX
                             pps->ps_rcDIB.top,    // = 0 - SrcY
                             pps->ps_rcDIB.right,  // SrcWidth
                             pps->ps_rcDIB.bottom, // SrcHeight
                             FindDIBBits(lpDIB),  
                             (LPBITMAPINFO)lpDIB, // lpBitsInfo
                             DIB_RGB_COLORS,		// wUsage
                             SRCCOPY );

         GetPrtRect( prcPrt, prcBPage, prcRend, prcPPage );

         sprtf( "Stretch: Obj %s as %s on pg %s."MEOR
            "\tgives %s on %s finally [%s]"MEOR,
            Rect2Stg( &pps->ps_rcDIB ),   // or prcObj ???
            Rect2Stg( prcRend ),
            Rect2Stg( prcPPage ),
            Rect2Stg( prcPrt ),
            Rect2Stg( prcDPage ),
            Rect2Stg( prcBPage ) );

         UpdateRendRect( pps, prcRend, prcPrt );

         DVGlobalUnlock(hDIB);

      }

      PutScaleText( hdc, pps );
      // dimensions           general              border text
      if( pps->ps_bAddText || pps->ps_bAddText2 || pps->ps_bAddText3 )
      {
         PaintTxt( hdc, pps );
      }

      if( hdc != hdc2 )
      {
         BitBlt( hdc2,	// Destination
            0, 0,
            pps->ps_rcDlgFrm.right, // Size
            pps->ps_rcDlgFrm.bottom,
            hdc,				// Source
            0, 0, // from location
            SRCCOPY );			// COPY operation
         if( hbmo )
            SelectObject( hdc, hbmo );
         DeleteObject(hbm);
         DeleteDC(hdc);
      }

      ReleaseDC( hWnd, hdc2 );
   }
   else
   {
      sprtf( "DLGPNT: Obj %dx%d to page %s FAILED!!!"MEOR,
         pps->ps_dwOWid,    // lpDIBInfo->di_dwDIBWidth,
         pps->ps_dwOHit,    // lpDIBInfo->di_dwDIBHeight,
         Rect2Stg( prcDPage ) );
   }
}


INT_PTR  P2_Paint( HWND hDlg )
{
   INT_PTR  iRet = TRUE;
   PAINTSTRUCT ps;
   HGLOBAL     hg;
   PDI         lpDIBInfo;

   BeginPaint(hDlg, &ps);
   if( ( hg = GET_PROP( hDlg, ATOM_PRT )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hg) ) )
   {
      PPS  pps = (PPS)&lpDIBInfo->di_szCGenBuf2[0];
      P2_RenderDIB( hDlg, pps );
      DVGlobalUnlock(hg);
   }

   EndPaint(hDlg, &ps);

   return iRet;
}

IDSTG sPrtScroll[] = {
   { SB_BOTTOM, "BOTTOM" },   // Scrolls to the lower right. 
   { SB_ENDSCROLL, "END" },   // Ends scroll.
   { SB_LINEDOWN, "LINEDOWN" },  // Scrolls one line down.
   { SB_LINEUP, "LINEUP" },   // Scrolls one line up.
   { SB_PAGEDOWN, "PAGEDOWN" },  // Scrolls one page down.
   { SB_PAGEUP, "PAGEUP" },   // Scrolls one page up. 
   { SB_THUMBPOSITION, "THUMBPOS" },   // The user has dragged the scroll box (thumb) and released the mouse button. The high-order word indicates the position of the scroll box at the end of the drag operation. 
   { SB_THUMBTRACK, "THUMBTRK" },   // The user is dragging the scroll box. This message is sent repeatedly until the user releases the mouse button. The high-order word indicates the position that the scroll box has been dragged to. 
   { SB_TOP, "TOP" },   // Scrolls to the upper left.
   { 0, 0 }
};

LPTSTR   GetScrStg( UINT val )
{
   LPTSTR   lps;
   PIDSTG   pids = &sPrtScroll[0];
   while( lps = pids->id_psNm )
   {
      if( pids->id_uiID == val )
         break;
      pids++;
   }
   return lps;

}

BOOL  NotInRect( LPRECT prcTmp, LPRECT prcPg )
{
   BOOL  bRet = FALSE;

   if( prcTmp->left < prcPg->left )
       return TRUE;
    if( prcTmp->right > prcPg->right )
       return TRUE;
    if( prcTmp->top < prcPg->top )
       return TRUE;
    if( prcTmp->bottom > prcPg->bottom )
       return TRUE;
    if( RW(prcTmp) > RW(prcPg ) )
       return TRUE;
    if( RH(prcTmp) > RH(prcPg ) )
       return TRUE;
    return bRet;  // else all appears OK
}

BOOL  CheckNewValue( PPS pps, UINT uid2, DWORD uiv )
{
   BOOL  bRet = FALSE;  // assume it is NOT ok
   DWORD    dwi, dwi2, dwi3, dwi4;
   LPRECT   prcTmp = &pps->ps_rcTmpRend;    // temporary size
   LPRECT   prcPg  = &pps->ps_rcPPage;    // paint object size
   UINT     uiv2, uiv3;

   switch( uid2 )
   {
   case IDC_EDTOP:
      //if( pps->ps_dwTop != uiv )
      {
         dwi = pps->ps_dwTop;    // get previous
         pps->ps_dwTop = uiv;    // set the NEW
         GetRenderSize( pps );  // only puts values into ps_TmpRend and ps_TmpPrt
         pps->ps_dwTop = dwi;    // restore OLD value
         if( NotInRect( prcTmp, prcPg ) )
            return FALSE;

         bRet = TRUE;   // let new value stand
      }
      break;

   case IDC_EDLEFT:
      //if( pps->ps_dwLeft != uiv )
      {
         dwi = pps->ps_dwTop;    // get previous
         pps->ps_dwLeft = uiv;   // set new
         GetRenderSize( pps );  // only puts values into ps_TmpRend and ps_TmpPrt
         pps->ps_dwTop = dwi;    // restore OLD value
         if( NotInRect( prcTmp, prcPg ) )
            return FALSE;
         bRet = TRUE;   // let new value stand
      }
      break;

   case IDC_EDWID:
      //if( pps->ps_dwWidth != uiv )
      {
         dwi  = pps->ps_dwWidth; // get old
         dwi2 = pps->ps_dwPCWid; // and PC
         dwi3 = pps->ps_dwPCHit;
         dwi4 = pps->ps_dwHeight;
         pps->ps_dwWidth = uiv;  // set NEW
         //uiv2 = ( uiv * 100 ) / lpDIBInfo->di_dwDIBWidth;
         uiv2 = ( uiv * 100 ) / pps->ps_dwOWid; // = lpDIBInfo->di_dwDIBWidth;
         if( pps->ps_dwPCWid != uiv2 )
            pps->ps_dwPCWid = uiv2; // set NEW here also
         if( !pps->ps_bDistort )
         {
            // set the SAME percentage into HEIGHT
            if( pps->ps_dwPCHit != uiv2 )
               pps->ps_dwPCHit = uiv2;
            // and get NEW height
            //uiv3 = (lpDIBInfo->di_dwDIBHeight * pps->ps_dwPCHit) / 100;
            uiv3 = (pps->ps_dwOHit * pps->ps_dwPCHit) / 100;
            if( pps->ps_dwHeight != uiv3 )
               pps->ps_dwHeight = uiv3;
         }
         GetRenderSize( pps );  // only puts values into ps_TmpRend and ps_TmpPrt
         pps->ps_dwWidth  = dwi; // put em back
         pps->ps_dwPCWid  = dwi2; // and PC
         pps->ps_dwPCHit  = dwi3;
         pps->ps_dwHeight = dwi4;
         if( NotInRect( prcTmp, prcPg ) )
            return FALSE;
         bRet = TRUE;   // let new value stand
      }
      break;

   case IDC_EDHIT:
      //if( pps->ps_dwHeight != uiv )
      {
         dwi  = pps->ps_dwWidth; // get old
         dwi2 = pps->ps_dwPCWid; // and PC
         dwi3 = pps->ps_dwPCHit;
         dwi4 = pps->ps_dwHeight;
         uiv2 = ( uiv * 100 ) / pps->ps_dwOHit; // = lpDIBInfo->di_dwDIBHeight;
         if( pps->ps_dwPCHit != uiv2 )
            pps->ps_dwPCHit = uiv2;
         if( !pps->ps_bDistort )
         {
            // set the SAME percentage into WIDTH
            if( pps->ps_dwPCWid != uiv2 )
               pps->ps_dwPCWid = uiv2;
            // and get NEW width
            //uiv3 = (lpDIBInfo->di_dwDIBWidth * pps->ps_dwPCWid) / 100;
            uiv3 = (pps->ps_dwOWid * pps->ps_dwPCWid) / 100;
            if( pps->ps_dwWidth != uiv3 )
               pps->ps_dwWidth = uiv3;
         }
         GetRenderSize( pps );  // only puts values into ps_TmpRend and ps_TmpPrt
         pps->ps_dwWidth  = dwi; // put em back
         pps->ps_dwPCWid  = dwi2; // and PC
         pps->ps_dwPCHit  = dwi3;
         pps->ps_dwHeight = dwi4;
         if( NotInRect( prcTmp, prcPg ) )
            return FALSE;
         bRet = TRUE;   // let new value stand
      }
      break;

   case IDC_EDPCWID: // CHANGE in percentage WIDTH
      //if( pps->ps_dwPCWid != uiv )
      {
         dwi  = pps->ps_dwWidth; // get old
         dwi2 = pps->ps_dwPCWid; // and PC
         dwi3 = pps->ps_dwPCHit;
         dwi4 = pps->ps_dwHeight;
         pps->ps_dwPCWid = uiv;
         //uiv2 = lpDIBInfo->di_dwDIBWidth * uiv / 100;
         uiv2 = pps->ps_dwOWid * uiv / 100;
         if( pps->ps_dwWidth != uiv2 )
            pps->ps_dwWidth = uiv2;
         if( !pps->ps_bDistort )
         {
            // set the SAME percentage into HEIGHT
            if( pps->ps_dwPCHit != uiv )
               pps->ps_dwPCHit = uiv;
            // and get NEW height
            //uiv3 = (lpDIBInfo->di_dwDIBHeight * pps->ps_dwPCHit) / 100;
            uiv3 = (pps->ps_dwOHit * pps->ps_dwPCHit) / 100;
            if( pps->ps_dwHeight != uiv3 )
               pps->ps_dwHeight = uiv3;
         }
         GetRenderSize( pps );  // only puts values into ps_TmpRend and ps_TmpPrt
         pps->ps_dwWidth  = dwi; // put em back
         pps->ps_dwPCWid  = dwi2; // and PC
         pps->ps_dwPCHit  = dwi3;
         pps->ps_dwHeight = dwi4;
         if( NotInRect( prcTmp, prcPg ) )
            return FALSE;
         bRet = TRUE;   // let new value stand
      }
      break;

   case IDC_EDPCHIT:
      //if( pps->ps_dwPCHit != uiv )
      {
         dwi  = pps->ps_dwWidth; // get old
         dwi2 = pps->ps_dwPCWid; // and PC
         dwi3 = pps->ps_dwPCHit;
         dwi4 = pps->ps_dwHeight;
         pps->ps_dwPCHit = uiv;
         //uiv2 = lpDIBInfo->di_dwDIBHeight * uiv / 100;
         uiv2 = pps->ps_dwOHit * uiv / 100;
         if( pps->ps_dwHeight != uiv2 )
            pps->ps_dwHeight = uiv2;
         if( !pps->ps_bDistort )
         {
            // set the SAME percentage into WIDTH
            if( pps->ps_dwPCWid != uiv )
               pps->ps_dwPCWid = uiv;
            // and get NEW width
            //uiv3 = (lpDIBInfo->di_dwDIBWidth * pps->ps_dwPCWid) / 100;
            uiv3 = (pps->ps_dwOWid * pps->ps_dwPCWid) / 100;
            if( pps->ps_dwWidth != uiv3 )
               pps->ps_dwWidth = uiv3;
         }
         GetRenderSize( pps );  // only puts values into ps_TmpRend and ps_TmpPrt
         pps->ps_dwWidth  = dwi; // put em back
         pps->ps_dwPCWid  = dwi2; // and PC
         pps->ps_dwPCHit  = dwi3;
         pps->ps_dwHeight = dwi4;
         if( NotInRect( prcTmp, prcPg ) )
            return FALSE;
         bRet = TRUE;   // let new value stand
      }
      break;
   }

   return bRet;
}

VOID  SetNewValue( HWND hDlg, PPS pps, UINT uid2, UINT uiv )
{
   BOOL  bTran = FALSE;
   UINT  uiv2, uiv3;

   // set the NEW value in the associated EDIT control
   SetDlgItemInt( hDlg, uid2, uiv, FALSE );

   switch( uid2 )
   {

   case IDC_EDTOP:
      if( pps->ps_dwTop != uiv )
      {
         pps->ps_dwTop = uiv;
         nPrtChg[nPrtTop] = TRUE;
      }
      bTran = TRUE;
      break;

   case IDC_EDLEFT:
      //    EDITTEXT        IDC_EDLEFT,216,67,34,14,ES_AUTOHSCROLL
      // { t_Edit, IDC_EDLEFT, 0, "LEFT", 0, TRUE },
      if( pps->ps_dwLeft != uiv )
      {
         pps->ps_dwLeft = uiv;
         nPrtChg[nPrtLeft] = TRUE;
      }
      bTran = TRUE;
      break;

   case IDC_EDWID:
      //    EDITTEXT        IDC_EDWID,218,106,34,14,ES_AUTOHSCROLL
      // { t_Edit, IDC_EDWID, 0, "WIDTH", IDC_EDPCWID, TRUE },
      pps->ps_uiActID = uid2;
      SETITEM( ps_dwWidth, uiv, nPrtWid );
      //uiv2 = lpDIBInfo->di_dwDIBWidth * uiv / 100;
      uiv2 = ( uiv * 100 ) / pps->ps_dwOWid;    // = lpDIBInfo->di_dwDIBWidth;
      //if( pps->ps_dwPCWid != uiv2 )
      {
         if( pps->ps_dwPCWid != uiv2 )
         {
            pps->ps_dwPCWid = uiv2;
            nPrtChg[nPrtPCW] = TRUE;
         }
         SetDlgItemInt( hDlg, IDC_EDPCWID, pps->ps_dwPCWid, FALSE );
         if( !pps->ps_bDistort )
         {
            // set the SAME percentage into HEIGHT
            if( pps->ps_dwPCHit != uiv2 )
            {
               pps->ps_dwPCHit = uiv2;
               nPrtChg[nPrtPCH] = TRUE;
            }

            SetDlgItemInt( hDlg, IDC_EDPCHIT, pps->ps_dwPCHit, FALSE );
            // and get NEW height
            //uiv3 = (lpDIBInfo->di_dwDIBHeight * pps->ps_dwPCHit) / 100;
            uiv3 = (pps->ps_dwOHit * pps->ps_dwPCHit) / 100;
            SETITEM( ps_dwHeight, uiv3, nPrtHit );
            SetDlgItemInt( hDlg, IDC_EDHIT, pps->ps_dwHeight, FALSE );
         }
      }
      bTran = TRUE;
      break;

   case IDC_EDHIT:
      //    EDITTEXT        IDC_EDHIT,218,126,34,14,ES_AUTOHSCROLL
      // { t_Edit, IDC_EDHIT, 0, "HEIGHT", IDC_EDPCWID, TRUE },
      pps->ps_uiActID = uid2;
      SETITEM(ps_dwHeight,uiv,nPrtHit);
      //uiv2 = lpDIBInfo->di_dwDIBHeight * 100 / uiv;
      uiv2 = ( uiv * 100 ) / pps->ps_dwOHit; // = lpDIBInfo->di_dwDIBHeight;
      //if( pps->ps_dwPCHit != uiv2 )
      {
         if( pps->ps_dwPCHit != uiv2 )
         {
            pps->ps_dwPCHit = uiv2;
            nPrtChg[nPrtPCH] = TRUE;
         }

         SetDlgItemInt( hDlg, IDC_EDPCHIT, pps->ps_dwPCHit, FALSE );
         if( !pps->ps_bDistort )
         {
            // set the SAME percentage into WIDTH
            if( pps->ps_dwPCWid != uiv2 )
            {
               pps->ps_dwPCWid = uiv2;
               nPrtChg[nPrtPCW] = TRUE;
            }
            SetDlgItemInt( hDlg, IDC_EDPCWID, pps->ps_dwPCWid, FALSE );
            // and get NEW width
            //uiv3 = (lpDIBInfo->di_dwDIBWidth * pps->ps_dwPCWid) / 100;
            uiv3 = (pps->ps_dwOWid * pps->ps_dwPCWid) / 100;
            SETITEM( ps_dwWidth, uiv3, nPrtWid );
            SetDlgItemInt( hDlg, IDC_EDWID, pps->ps_dwWidth, FALSE );
         }
      }
      bTran = TRUE;
      break;

   case IDC_EDPCWID: // CHANGE in percentage WIDTH
      //    EDITTEXT        IDC_EDPCWID,283,105,25,14,ES_AUTOHSCROLL
      //{ t_Edit, IDC_EDPCWID, 0, "PCWIDTH", IDC_EDWID, TRUE },
      pps->ps_uiActID = uid2;
      if( pps->ps_dwPCWid != uiv )
      {
         pps->ps_dwPCWid = uiv;
         nPrtChg[nPrtPCW] = TRUE;
      }
      //uiv2 = lpDIBInfo->di_dwDIBWidth * uiv / 100;
      uiv2 = pps->ps_dwOWid * uiv / 100;
      //if( pps->ps_dwWidth != uiv2 )
      {
         SETITEM( ps_dwWidth, uiv2, nPrtWid );
         SetDlgItemInt( hDlg, IDC_EDWID, pps->ps_dwWidth, FALSE );

         if( !pps->ps_bDistort )
         {
            // set the SAME percentage into HEIGHT
            if( pps->ps_dwPCHit != uiv )
            {
               pps->ps_dwPCHit = uiv;
               nPrtChg[nPrtPCH] = TRUE;
            }

            SetDlgItemInt( hDlg, IDC_EDPCHIT, pps->ps_dwPCHit, FALSE );
            // and get NEW height
            //uiv3 = (lpDIBInfo->di_dwDIBHeight * pps->ps_dwPCHit) / 100;
            uiv3 = (pps->ps_dwOHit * pps->ps_dwPCHit) / 100;
            SETITEM( ps_dwHeight, uiv3, nPrtHit );
            SetDlgItemInt( hDlg, IDC_EDHIT, pps->ps_dwHeight, FALSE );
         }
      }
      bTran = TRUE;
      break;

   case IDC_EDPCHIT:
      //    EDITTEXT        IDC_EDPCHIT,283,127,25,14,ES_AUTOHSCROLL
      //   { t_Edit, IDC_EDPCHIT, 0, "PCHEIGHT", IDC_EDHIT, TRUE },
      pps->ps_uiActID = uid2;
      if( pps->ps_dwPCHit != uiv )
      {
         pps->ps_dwPCHit = uiv;
         nPrtChg[nPrtPCH] = TRUE;
      }

      //uiv2 = lpDIBInfo->di_dwDIBHeight * uiv / 100;
      uiv2 = pps->ps_dwOHit * uiv / 100;
      //if( pps->ps_dwHeight != uiv2 )
      {
         SETITEM( ps_dwHeight, uiv2, nPrtHit );
         SetDlgItemInt( hDlg, IDC_EDHIT, pps->ps_dwHeight, FALSE );
         if( !pps->ps_bDistort )
         {
            // set the SAME percentage into WIDTH
            if( pps->ps_dwPCWid != uiv )
            {
               pps->ps_dwPCWid = uiv;
               nPrtChg[nPrtPCW] = TRUE;
            }
            SetDlgItemInt( hDlg, IDC_EDPCWID, pps->ps_dwPCWid, FALSE );
            // and get NEW width
            //uiv3 = (lpDIBInfo->di_dwDIBWidth * pps->ps_dwPCWid) / 100;
            uiv3 = (pps->ps_dwOWid * pps->ps_dwPCWid) / 100;
            SETITEM( ps_dwWidth, uiv3, nPrtWid );
            SetDlgItemInt( hDlg, IDC_EDWID, pps->ps_dwWidth, FALSE );
         }
      }
      bTran = TRUE;
      break;

   }

   if( bTran )
   {
      // change the rendered image
      P2_RenderDIB( hDlg, pps );
   }

}

// on WM_VSCROLL
// wParam 
// The low-order word specifies a scroll bar value that
//    indicates the user's scrolling request, like SB_LINEDOWN, SB_LINEUP, etc
// The high-order word specifies the current position of the scroll box
//    if SB_THUMB???
// lParam 
// If the message is sent by a scroll bar, this parameter is the
//    handle to the scroll bar control. 
// If an application processes this message, it should return zero.
//
INT_PTR  P2_WM_VSCROLL( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
   INT_PTR  iRet = TRUE;
   HGLOBAL  hg;
   PDI      pdi;
   if( ( hg = GET_PROP( hDlg, ATOM_PRT ) ) &&
       ( pdi = (PDI)DVGlobalLock(hg)     ) )
   {
      PPS  pps = (PPS)&pdi->di_szCGenBuf2[0];
      DWORD    req  = LOWORD(wParam);
      DWORD    code = HIWORD(wParam);
      UINT     uid, uiv;
      UINT     uid2;
      LPTSTR   lps, lpn, lpn2;
      BOOL     bTran;

      uid = GetItemPerHand( (HWND)lParam );  // get SCROLL ID per handle, and then
      uid2 = GetRelItemPID( uid );  // get EDIT associated with this scroll

      lpn = GetDBGNamePID( uid, FALSE );
      lpn2 = GetDBGNamePID( uid2, FALSE );
      lps = GetScrStg( req );
      if( lps )
      {
         if( lpn && lpn2 )
         {
            //sprtf( "WM_VSCROLL: %s h=%#x. That is %s effecting %s"MEOR,
            //   lps, lParam, lpn, lpn2 );
            uiv = GetDlgItemInt( hDlg, uid2, &bTran, FALSE );
            if( bTran )
            {
               // got number from associated EDIT control
               bTran = FALSE;
               // ONLY process line up and down
               switch(req)
               {
               case SB_LINEDOWN:
                  // down = reduce value
                  if( ( uid2 == IDC_EDTOP  ) ||
                      ( uid2 == IDC_EDLEFT ) )
                  {
                     // these can go to ZERO
                     if( uiv )
                     {
                        bTran = TRUE;
                        uiv--;
                     }
                  }
                  else  // if( uid == IDC_SBCOPIES ), etc
                  {  // then not less than 1
                     if( uiv > 1 )
                     {
                        bTran = TRUE;
                        uiv--;
                     }
                  }
                  iRet = FALSE;  // processed
                  break;

               case SB_LINEUP:
                  // up = increase value
                  uiv++;
                  bTran = TRUE;
                  iRet = FALSE;  // processed
                  break;
               }

               if( bTran )
               {
                  bTran = FALSE;
                  // BEFORE THE NEW VALUE IS SET IN STONE IT MUST BE RANGE
                  // CHECKED - THE OBJECT CAN NOT MOVE OUTSIDE ITS RECTANGE
                  if( CheckNewValue( pps, uid2, uiv ) )
                  {
                     bTran = TRUE;
                     SetNewValue( hDlg, pps, uid2, uiv );
                  }
               }
            }
         }
         else
         {
            sprtf( "WM_VSCROLL: %s h=%#x."MEOR, lps, lParam );
         }
      }
      else
      {
         sprtf( "WM_VSCROLL: req=%#x? code=%#x hlP=%#x."MEOR, req, code, lParam );
      }

      DVGlobalUnlock(hg);
   }

   return iRet;

}

INT_PTR  P2_WM_NOTIFY( HWND hDlg, WPARAM wParam, LPARAM lParam )
{
   INT_PTR  iRet = FALSE;
   HGLOBAL  hg;
   PDI      lpDIBInfo;

   if( ( hg = GET_PROP( hDlg, ATOM_PRT )   ) &&
       ( lpDIBInfo = (PDI)DVGlobalLock(hg) ) )
   {
      PPS  pps = (PPS)&lpDIBInfo->di_szCGenBuf2[0];
      DWORD cmd = LOWORD(wParam);
      DWORD code = HIWORD(wParam);

      sprtf( "DLG-WM_NOTIFY: cmd=%#x code=%#x hlP=%#x."MEOR, cmd, code, lParam );

      DVGlobalUnlock(hg);
   }
   return iRet;
}

INT_PTR CALLBACK PRINT2DLGPROC(
    HWND hDlg,  // handle to dialog box
    UINT uMsg,     // message
    WPARAM wParam, // first message parameter
    LPARAM lParam ) // second message parameter
{
	INT_PTR	flg = FALSE;
	switch (uMsg)
	{

	case WM_INITDIALOG:
		flg = P2_Init( hDlg, wParam, lParam );
		break;

	case WM_CLOSE:	/* 0x0010 */
      EndDialog( hDlg, FALSE ) ;
      flg = TRUE;
		break;

	case WM_COMMAND:
      flg = P2_Command( hDlg, wParam, lParam );
		break;

	case WM_NOTIFY:
      flg = P2_WM_NOTIFY( hDlg, wParam, lParam );
		break;

   case WM_DESTROY:
      REMOVE_PROP( hDlg, ATOM_PRT );
      flg = TRUE;
      break;

   case WM_PAINT:
      flg = P2_Paint( hDlg );
      break;

   //case WM_HSCROLL:   // noe in this app.
   case WM_VSCROLL:
      flg = P2_WM_VSCROLL( hDlg, wParam, lParam );
      break;


	}	/* case of uMsg */

	return( flg );

} /* end of OPENMRUDLGPROC() */

//typedef  struct   tagPTARR {
//   POINT pa_Pt;
//   INT   pa_Type;
//}PTARR, * PPTARR;
//#define  ptt_Left    1
//#define  ptt_Rite    2
//#define  ptt_Up      3
//#define  ptt_Down    4
//VOID  PntPtType( HDC hdc, PPTARR ppta )
#define  TO(a,b)  if( TextOut( hdc, a, b, lpt, iLen ) )\
                     iTot += iLen
#define  DOP2(a)\
   TO( (a->pt.x), (a->pt.y) );\
   MoveToEx( hdc, a->p1.pa_Pt.x, a->p1.pa_Pt.y, &cpt );\
   LineTo( hdc, a->p1_To.x, a->p1_To.y );\
   PntPtType( hdc, &a->p1 );\
   MoveToEx( hdc, a->p2.pa_Pt.x, a->p2.pa_Pt.y, &cpt );\
   LineTo( hdc, a->p2_To.x, a->p2_To.y );\
   PntPtType( hdc, &a->p2 )

//typedef  struct tagDOTXT {
//   HDC      hdc;
//   LPRECT   prcPrt;
//   LPRECT   prcDPage;

//   SIZE     szObj;      // object original size
//   LPRECT   prcPrint;   // printed object size
//   SIZE     szOff;      // offsets applied
//   SIZE     szPercent;  // percentages applied
//   SIZE     szSize;     // widht and height after percentage
//   LPRECT   prcPPage;   // print page size

//   PPTARR2  pp2;        // = &pps->ps_p2;
//   HFONT    hfHorz;
//   HFONT    hfVert;
//   DWORD    dxSize;
//   DWORD    dySize;
//   DWORD    dxPx;
//   DWORD    dyPx;
//   double   dbXIns;
//   double   dbYIns;
//   DWORD    dwLeft;
//   DWORD    dwRight;
//   DWORD    dwTop;
//   DWORD    dwBot;
//}DOTXT, * PDOTXT;

VOID  DoTxt( PPS pps, PDOTXT pdt )
{
   RECT     rc, rc2;   // work rects
   LPTSTR   lpt = &gszDiag[0];
   int      iLen;
   SIZE     sz;
   POINT    pt, cpt, ptl;
   HPEN     hOldPen;
   HFONT    hOldFont = 0;
   INT      iTot = 0;
   HDC      hdc;
   LPRECT   prcPrt, prcPrint;
   LPRECT   prcDPage;
   PPTARR2  pp2;        // = &pps->ps_p2;

   if( !( hdc = pdt->hdc ) ||
       !( prcPrt = pdt->prcPrt ) ||
       !( prcDPage = pdt->prcDPage ) ||
       !( pp2 = pdt->pp2 ) ||
       !( prcPrint = pdt->prcPrint ) )
   {
      return;
   }
   // rectangle(s) are -
   //prcPrt   = &pps->ps_rcPrt;
   //prcDPage = &pps->ps_rcDPage;

   SetTextColor(hdc, RGB(0,0,0));      // use WHITE
   SetBkColor(hdc, RGB(255,255,255));  // on BLACK
   hOldPen = SelectObject( hdc, GetStockObject(BLACK_PEN) );

   if( !pps->ps_bAddText2 )   // add general text?
      goto Done_Text;         // NO!

   wsprintf( lpt, "Printed as %s.", Rect2Stg(pdt->prcPrint) );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );
   //ptl.y = ((2 * sz.cy) - (sz.cy / 2)); // one line unit for NEW Y position
   ptl.y = sz.cy + 4; // one line unit for NEW Y position
   pt.y  = prcDPage->top;   // begin position

   rc = *prcDPage;
   if( (prcDPage->top + ptl.y) < prcPrt->top )
   {
      rc.bottom = prcPrt->top;
      cpt.y     = prcPrt->top;   // set end of text as TOP of image
      // if add border text
      if( ( pps->ps_bAddText3 ) &&
         ( pdt->hfHorz ) &&
         ( RH( &rc ) > ( 8 * ptl.y ) ) )
      {
         // move general text down a bit
         rc.top += (5 * ptl.y);
      }
   }
   else if( (prcDPage->bottom - ptl.y) > prcPrt->bottom )
   {
      rc.top    = prcPrt->bottom;
      cpt.y     = prcDPage->bottom; // set end of text as BOTTOM of view
   }
   else
   {
     sprtf( "No room for text with %s on %s."MEOR,
        Rect2Stg( prcPrt ),
        Rect2Stg( prcDPage ) );
      goto Done_Text;
   }

   rc2 = rc;   // copy the rectange
   rc2.bottom = rc.top + ptl.y;
   // TextOut()
   if( DrawText( hdc, // handle to DC
      lpt,  // text to draw
      iLen,    // text length
      &rc2, // formatting dimensions
      DT_SINGLELINE | DT_VCENTER | DT_CENTER ) ) // text-drawing options
   {
      sprtf( "Added text [%s] at %s."MEOR,
         lpt,
         Rect2Stg( &rc2 ) );
   }
   else
   {
      sprtf( "ERROR: Added text [%s] FAILED!"MEOR,
         lpt );
      goto Done_Text;
   }

   rc2.top    += ptl.y;
   rc2.bottom += ptl.y;
   if( rc2.bottom < cpt.y )
   {
      wsprintf( lpt, "Printed on %s.", Rect2Stg(pdt->prcPPage) );
      iLen = lstrlen(lpt);
      DrawText( hdc, // handle to DC
         lpt,  // text to draw
         iLen,    // text length
         &rc2, // formatting dimensions
         DT_SINGLELINE | DT_VCENTER | DT_CENTER ); // text-drawing options
   }
   else
   {
      goto Done_Text;
   }

   rc2.top    += ptl.y;
   rc2.bottom += ptl.y;
   if( rc2.bottom < cpt.y )
   {
      // room for more text
      wsprintf( lpt, "Offset %dx, %dy, Percent %dx%d.",
         pdt->szOff.cx,
         pdt->szOff.cy,
         pdt->szPercent.cx,
         pdt->szPercent.cy );
      iLen = lstrlen(lpt);
      DrawText( hdc, // handle to DC
         lpt,  // text to draw
         iLen,    // text length
         &rc2, // formatting dimensions
         DT_SINGLELINE | DT_VCENTER | DT_CENTER ); // text-drawing options
   }
   else
   {
      goto Done_Text;
   }

Done_Text:

   // put a border at the PRINTER border - prcDPage = resolution in pixels
   // FrameRect( hdc, prcDPage, GetStockObject(BLACK_BRUSH) );
   // this will be placed like
   //FrameRect( hdc, prcBPage, GetStockObject(BLACK_BRUSH) );
   // *********************************************
   // select HORIZONTAL font
   // ======================
   if( pdt->hfHorz )
      hOldFont = SelectObject( hdc, pdt->hfHorz );
   else
      goto Dn_Horz;

   // add border text
   if( !pps->ps_bAddText3 )
      goto Add_HPos;

   // text showing horizontal size in mm
   wsprintf( lpt, " %d mm ", pdt->dxSize );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );
   ptl.y = ((2 * sz.cy) - (sz.cy / 2)); // one line unit for NEW Y position
   pt.y = prcDPage->top;   // begin position
   if( (pt.y + (3 * sz.cy)) < prcPrt->top )
   {
      // setup text/line postion
      pt.x  = ( prcDPage->right - sz.cx ) / 2;
      pt.y += ptl.y;
      // set text postition
      pp2->pt.x       = pt.x;
      pp2->pt.y       = pt.y - (sz.cy / 2);
      // setup line-arrow positions in structure
      pp2->p1.pa_Pt.x = prcDPage->left; // left arrow position
      pp2->p1.pa_Pt.y = pt.y;
      pp2->p1.pa_Type = ptt_Left;
      pp2->p1_To.x    = pt.x;          // line to position
      pp2->p1_To.y    = pt.y;
      pp2->p2.pa_Pt.x = prcDPage->right; // right arrow position
      pp2->p2.pa_Pt.y = pt.y;
      pp2->p2.pa_Type = ptt_Rite;
      pp2->p2_To.x    = pt.x + sz.cx;     // line to position
      pp2->p2_To.y    = pt.y;
      DOP2(pp2);           // out the text, lines and arrows
   }
   else
   {
      pt.y  = prcDPage->bottom;   // begin position
      goto Try_BUp1;
   }

   wsprintf( lpt, " %d pixels ", pdt->dxPx );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );
   if( (pt.y + (3 * sz.cy)) < prcPrt->top )
   {
      // we can fit another line
      pt.x  = (prcDPage->right - sz.cx) / 2;
      pt.y += ptl.y; // bump to NEW Y position
      // set text postition
      pp2->pt.x       = pt.x;
      pp2->pt.y       = pt.y - (sz.cy / 2);
      // setup line-arrow positions in structure
      pp2->p1.pa_Pt.x = prcDPage->left; // left arrow position
      pp2->p1.pa_Pt.y = pt.y;
      pp2->p1.pa_Type = ptt_Left;
      pp2->p1_To.x    = pt.x;          // line to position
      pp2->p1_To.y    = pt.y;
      pp2->p2.pa_Pt.x = prcDPage->right; // right arrow position
      pp2->p2.pa_Pt.y = pt.y;
      pp2->p2.pa_Type = ptt_Rite;
      pp2->p2_To.x    = pt.x + sz.cx;     // line to position
      pp2->p2_To.y    = pt.y;
      DOP2(pp2);           // out the text, lines and arrows
   }
   else
   {
      pt.y  = prcDPage->bottom;   // begin position
      goto Try_BUp2;
   }

   // db = (double)pps->ps_dxRes / (double)pps->ps_xPerInch;
   wsprintf( lpt, " %s inches ", Dbl2Str( pdt->dbXIns, 3 ) );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );
   if( (pt.y + (3 * sz.cy)) < prcPrt->top )
   {
      // we can fit another line
      pt.x  = (prcDPage->right - sz.cx) / 2;
      pt.y += ptl.y; // bump to NEW Y position
      pp2->pt.x       = pt.x;
      pp2->pt.y       = pt.y - (sz.cy / 2);
      // setup line-arrow positions in structure
      pp2->p1.pa_Pt.x = prcDPage->left; // left arrow position
      pp2->p1.pa_Pt.y = pt.y;
      pp2->p1.pa_Type = ptt_Left;
      pp2->p1_To.x    = pt.x;          // line to position
      pp2->p1_To.y    = pt.y;
      pp2->p2.pa_Pt.x = prcDPage->right; // right arrow position
      pp2->p2.pa_Pt.y = pt.y;
      pp2->p2.pa_Type = ptt_Rite;
      pp2->p2_To.x    = pt.x + sz.cx;     // line to position
      pp2->p2_To.y    = pt.y;
      DOP2(pp2);           // out the text, lines and arrows
   }
   else
   {
      pt.y  = prcDPage->bottom;   // begin position
      goto Try_BUp3;
   }

   goto Add_HPos;

Try_BUp1:

   ptl.y = ((2 * sz.cy) - (sz.cy / 2)); // one line unit for NEW Y position
   if( (pt.y - ( 3 * sz.cy )) > prcPrt->bottom )
   {
      // setup text/line postion
      pt.x  = ( prcDPage->right - sz.cx ) / 2;
      pt.y -= ptl.y;
      // set text postition
      pp2->pt.x       = pt.x;
      pp2->pt.y       = pt.y - (sz.cy / 2);
      // setup line-arrow positions in structure
      pp2->p1.pa_Pt.x = prcDPage->left; // left arrow position
      pp2->p1.pa_Pt.y = pt.y;
      pp2->p1.pa_Type = ptt_Left;
      pp2->p1_To.x    = pt.x;          // line to position
      pp2->p1_To.y    = pt.y;
      pp2->p2.pa_Pt.x = prcDPage->right; // right arrow position
      pp2->p2.pa_Pt.y = pt.y;
      pp2->p2.pa_Type = ptt_Rite;
      pp2->p2_To.x    = pt.x + sz.cx;     // line to position
      pp2->p2_To.y    = pt.y;
      DOP2(pp2);           // out the text, lines and arrows

   }
   else
   {
      goto Add_HPos;
   }

   wsprintf( lpt, " %d pixels ", pdt->dxPx );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );

Try_BUp2:

   if( (pt.y - ( 3 * sz.cy )) > prcPrt->bottom )
   {
      // setup text/line postion
      pt.x  = ( prcDPage->right - sz.cx ) / 2;
      pt.y -= ptl.y;
      // set text postition
      pp2->pt.x       = pt.x;
      pp2->pt.y       = pt.y - (sz.cy / 2);
      // setup line-arrow positions in structure
      pp2->p1.pa_Pt.x = prcDPage->left; // left arrow position
      pp2->p1.pa_Pt.y = pt.y;
      pp2->p1.pa_Type = ptt_Left;
      pp2->p1_To.x    = pt.x;          // line to position
      pp2->p1_To.y    = pt.y;
      pp2->p2.pa_Pt.x = prcDPage->right; // right arrow position
      pp2->p2.pa_Pt.y = pt.y;
      pp2->p2.pa_Type = ptt_Rite;
      pp2->p2_To.x    = pt.x + sz.cx;     // line to position
      pp2->p2_To.y    = pt.y;
      DOP2(pp2);           // out the text, lines and arrows

   }
   else
   {
      goto Add_HPos;
   }

   wsprintf( lpt, " %s inches ", Dbl2Str( pdt->dbXIns, 3 ) );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );

Try_BUp3:

   if( (pt.y - ( 3 * sz.cy )) > prcPrt->bottom )
   {
      // setup text/line postion
      pt.x  = ( prcDPage->right - sz.cx ) / 2;
      pt.y -= ptl.y;
      // set text postition
      pp2->pt.x       = pt.x;
      pp2->pt.y       = pt.y - (sz.cy / 2);
      // setup line-arrow positions in structure
      pp2->p1.pa_Pt.x = prcDPage->left; // left arrow position
      pp2->p1.pa_Pt.y = pt.y;
      pp2->p1.pa_Type = ptt_Left;
      pp2->p1_To.x    = pt.x;          // line to position
      pp2->p1_To.y    = pt.y;
      pp2->p2.pa_Pt.x = prcDPage->right; // right arrow position
      pp2->p2.pa_Pt.y = pt.y;
      pp2->p2.pa_Type = ptt_Rite;
      pp2->p2_To.x    = pt.x + sz.cx;     // line to position
      pp2->p2_To.y    = pt.y;
      DOP2(pp2);           // out the text, lines and arrows

   }

Add_HPos:

   if( !pps->ps_bAddText )
      goto Dn_Horz;

   // ************************************************
   // text showing pixels from left of border of paper
   //sz2.cx = prcPrt->left;
   wsprintf( lpt, " %d px ", pdt->dwLeft );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );
   if( ( prcPrt->left - prcDPage->left ) > sz.cx )
   {
      // will fit to left of image, so
      pt.x = prcDPage->left + ((prcPrt->left - prcDPage->left - sz.cx) / 2);
      pt.y = (prcPrt->top + (RH(prcPrt) / 2));
      // set text postition
      pp2->pt.x       = pt.x;
      pp2->pt.y       = pt.y - (sz.cy / 2);
      if( ( prcPrt->left - prcDPage->left - sz.cx ) > MYASZ )
      {
         // setup line-arrow positions in structure
         pp2->p1.pa_Pt.x = prcDPage->left; // left arrow position
         pp2->p1.pa_Pt.y = pt.y;
         pp2->p1.pa_Type = ptt_Left;
         pp2->p1_To.x    = pt.x;          // line to position
         pp2->p1_To.y    = pt.y;
         pp2->p2.pa_Pt.x = prcPrt->left; // right arrow position
         pp2->p2.pa_Pt.y = pt.y;
         pp2->p2.pa_Type = ptt_Rite;
         pp2->p2_To.x    = pt.x + sz.cx;     // line to position
         pp2->p2_To.y    = pt.y;
         DOP2(pp2);           // out the text, lines and arrows
      }
      else
      {
         TO( (pp2->pt.x), (pp2->pt.y) );  // else just the text - no line/arrows
      }
   }

   // text showing pixels to right border of paper
   //sz2.cx = (prcDPage->right - prcPrt->right);
   wsprintf( lpt, " %d px ", pdt->dwRight );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );
   if( (prcDPage->right - prcPrt->right) > sz.cx )
   {
      // will fit to RIGHT of image
      pt.x = prcPrt->right + ( ( prcDPage->right - prcPrt->right - sz.cx ) / 2 );
      pt.y = (prcPrt->top + (RH(prcPrt) / 2));
      // set text postition
      pp2->pt.x       = pt.x;
      pp2->pt.y       = pt.y - (sz.cy / 2);
      //if( ( prcDPage->right - pt.x ) > 6 )   // if room for LINES / ARROWS
      if( ( prcDPage->right - prcPrt->right - sz.cx ) > MYASZ )
      {
         // setup line-arrow positions in structure
         pp2->p1.pa_Pt.x = prcPrt->right; // left arrow position
         pp2->p1.pa_Pt.y = pt.y;
         pp2->p1.pa_Type = ptt_Left;
         pp2->p1_To.x    = pt.x;          // line to position
         pp2->p1_To.y    = pt.y;
         pp2->p2.pa_Pt.x = prcDPage->right; // right arrow position
         pp2->p2.pa_Pt.y = pt.y;
         pp2->p2.pa_Type = ptt_Rite;
         pp2->p2_To.x    = pt.x + sz.cx;     // line to position
         pp2->p2_To.y    = pt.y;
         DOP2(pp2);           // out the text, lines and arrows
      }
      else
      {
         TO( (pp2->pt.x), (pp2->pt.y) );  // else just the text - no line/arrows
      }
   }

   wsprintf( lpt, " %d px ", RW(prcPrint) );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );
   if( RW(prcPrt) > sz.cx )  // if sufficient WIDTH
   {
      if( (prcPrt->top - sz.cy) > prcDPage->top )  // if room at top
      {
         // begin at left plus half of width
         pt.x = prcPrt->left + (( RW(prcPrt) - sz.cx ) / 2 );
         pt.y = prcPrt->top  - (sz.cy / 2);
         // set text postition
         pp2->pt.x       = pt.x;
         pp2->pt.y       = pt.y - (sz.cy / 2);
         if( (RW(prcPrt) - sz.cx) > MYASZ )
         {
            // setup line-arrow positions
            pp2->p1.pa_Pt.x = prcPrt->left; // left arrow position
            pp2->p1.pa_Pt.y = pt.y;
            pp2->p1.pa_Type = ptt_Left;
            pp2->p1_To.x    = pt.x;          // line to position
            pp2->p1_To.y    = pt.y;
            pp2->p2.pa_Pt.x = prcPrt->right; // right arrow position
            pp2->p2.pa_Pt.y = pt.y;
            pp2->p2.pa_Type = ptt_Rite;
            pp2->p2_To.x    = pt.x + sz.cx;     // line to position
            pp2->p2_To.y    = pt.y;
            DOP2(pp2);           // out the text, lines and arrows
         }
         else
         {
            TO( (pp2->pt.x), (pp2->pt.y) );  // else just the text - no line/arrows
         }
      }
      else if( (prcPrt->bottom + sz.cy) < prcDPage->bottom )   // or at bottom
      {
         // begin at left plus half of width
         pt.x = prcPrt->left + ( ( RW(prcPrt) - sz.cx ) / 2 );
         pt.y = prcPrt->bottom + (sz.cy / 2);
         // set text postition
         pp2->pt.x       = pt.x;
         pp2->pt.y       = pt.y - (sz.cy / 2);
         if( (RW(prcPrt) - sz.cx) > MYASZ )
         {
            // setup line-arrow positions
            pp2->p1.pa_Pt.x = prcPrt->left; // left arrow position
            pp2->p1.pa_Pt.y = pt.y;
            pp2->p1.pa_Type = ptt_Left;
            pp2->p1_To.x    = pt.x;          // line to position
            pp2->p1_To.y    = pt.y;
            pp2->p2.pa_Pt.x = prcPrt->right; // right arrow position
            pp2->p2.pa_Pt.y = pt.y;
            pp2->p2.pa_Type = ptt_Rite;
            pp2->p2_To.x    = pt.x + sz.cx;     // line to position
            pp2->p2_To.y    = pt.y;
            DOP2(pp2);           // out the text, lines and arrows
         }
         else
         {
            TO( (pp2->pt.x), (pp2->pt.y) );  // else just the text - no line/arrows
         }
      }
   }

   // ************************************************

Dn_Horz:

   // ==================================
   // SWITCH TO VERTICAL FONT
   // ***********************
   if( pdt->hfVert )
   {
      if( hOldFont )
         SelectObject( hdc, pdt->hfVert );
      else
         hOldFont = SelectObject( hdc, pdt->hfVert );
   }
   else
      goto Dn_Vert;

   // add border text
   if( !pps->ps_bAddText3 )
      goto Add_VPos;

   // text showing vertical size in mm
   wsprintf( lpt, " %d mm ", pdt->dySize );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );
   ptl.x = ((2 * sz.cy) - (sz.cy / 2)); // get BUMP to next column LEFT
   pt.x  = prcDPage->right; // NOTE: Text at 90 degrees so use cy on x
   if( pt.x - (3 * sz.cy) > prcPrt->right )
   {
      pt.x -= ptl.x; // NOTE: Text at 90 degrees so use cy on x
      pt.y  = ((prcDPage->bottom - sz.cx) / 2); // and cx on y positon
      // set text postition
      pp2->pt.x       = pt.x + (sz.cy / 2);
      pp2->pt.y       = pt.y;
      // setup line-arrow positions in structure
      pp2->p1.pa_Pt.x = pt.x; // top arrow position
      pp2->p1.pa_Pt.y = prcDPage->top;
      pp2->p1.pa_Type = ptt_Up;
      pp2->p1_To.x    = pt.x;          // line to position
      pp2->p1_To.y    = pt.y;
      pp2->p2.pa_Pt.x = pt.x; // bottom arrow position
      pp2->p2.pa_Pt.y = prcDPage->bottom;
      pp2->p2.pa_Type = ptt_Down;
      pp2->p2_To.x    = pt.x;     // line to position
      pp2->p2_To.y    = pt.y + sz.cx;
      DOP2(pp2);           // out the text, lines and arrows
   }
   else
   {
      pt.x  = prcDPage->left; // NOTE: Text at 90 degrees so use cy on x
      goto Try_BLf1;
   }

   wsprintf( lpt, " %d pixels ", pdt->dyPx );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );
   // can we fit more vertical text to the right?
   if( pt.x - (3 * sz.cy) > prcPrt->right )
   {
      pt.x -= ptl.x; // BUMP to next column LEFT
      // NOTE: Text at 90 degrees so use cy on x
      pt.y = ((prcDPage->bottom - sz.cx) / 2); // and cx on y positon
      // set text postition
      pp2->pt.x       = pt.x + (sz.cy / 2);
      pp2->pt.y       = pt.y;
      // setup line-arrow positions in structure
      pp2->p1.pa_Pt.x = pt.x;    // top arrow position
      pp2->p1.pa_Pt.y = prcDPage->top;
      pp2->p1.pa_Type = ptt_Up;
      pp2->p1_To.x    = pt.x;    // line to position
      pp2->p1_To.y    = pt.y;
      pp2->p2.pa_Pt.x = pt.x;    // bottom arrow position
      pp2->p2.pa_Pt.y = prcDPage->bottom;
      pp2->p2.pa_Type = ptt_Down;
      pp2->p2_To.x    = pt.x;    // line to position
      pp2->p2_To.y    = pt.y + sz.cx;
      DOP2(pp2);                 // out the text, lines and arrows
   }
   else
   {
      pt.x  = prcDPage->left; // NOTE: Text at 90 degrees so use cy on x
      goto Try_BLf2;
   }

   //db = (double)pps->ps_dyRes / (double)pps->ps_yPerInch;
   wsprintf( lpt, " %s inches ", Dbl2Str( pdt->dbYIns, 3 ) );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );
   if( pt.x - (3 * sz.cy) > prcPrt->right )
   {
      // and we can add a THIRD column to right of image
      pt.x -= ptl.x; // BUMP to next column LEFT
      // NOTE: Text at 90 degrees so use cy on x
      pt.y = ((prcDPage->bottom - sz.cx) / 2); // and cx on y positon
      // set text postition
      pp2->pt.x       = pt.x + (sz.cy / 2);
      pp2->pt.y       = pt.y;
      // setup line-arrow positions in structure
      pp2->p1.pa_Pt.x = pt.x;    // top arrow position
      pp2->p1.pa_Pt.y = prcDPage->top;
      pp2->p1.pa_Type = ptt_Up;
      pp2->p1_To.x    = pt.x;    // line to position
      pp2->p1_To.y    = pt.y;
      pp2->p2.pa_Pt.x = pt.x;    // bottom arrow position
      pp2->p2.pa_Pt.y = prcDPage->bottom;
      pp2->p2.pa_Type = ptt_Down;
      pp2->p2_To.x    = pt.x;    // line to position
      pp2->p2_To.y    = pt.y + sz.cx;
      DOP2(pp2);                 // out the text, lines and arrows
   }
   else
   {
      pt.x  = prcDPage->left; // NOTE: Text at 90 degrees so use cy on x
      goto Try_BLf3;
   }

   goto Add_VPos;

Try_BLf1:

   if( ( pt.x + ( 3 * sz.cy ) ) < prcPrt->left )
   {
      pt.x += ptl.x; // NOTE: Text at 90 degrees so use cy on x
      pt.y  = ((prcDPage->bottom - sz.cx) / 2); // and cx on y positon
      // set text postition
      pp2->pt.x       = pt.x + (sz.cy / 2);
      pp2->pt.y       = pt.y;
      // setup line-arrow positions in structure
      pp2->p1.pa_Pt.x = pt.x; // top arrow position
      pp2->p1.pa_Pt.y = prcDPage->top;
      pp2->p1.pa_Type = ptt_Up;
      pp2->p1_To.x    = pt.x;          // line to position
      pp2->p1_To.y    = pt.y;
      pp2->p2.pa_Pt.x = pt.x; // bottom arrow position
      pp2->p2.pa_Pt.y = prcDPage->bottom;
      pp2->p2.pa_Type = ptt_Down;
      pp2->p2_To.x    = pt.x;     // line to position
      pp2->p2_To.y    = pt.y + sz.cx;
      DOP2(pp2);           // out the text, lines and arrows
   }
   else
   {
      goto Add_VPos;
   }

   wsprintf( lpt, " %d pixels ", pdt->dyPx );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );

Try_BLf2:

   if( ( pt.x + ( 3 * sz.cy ) ) < prcPrt->left )
   {
      pt.x += ptl.x; // NOTE: Text at 90 degrees so use cy on x
      pt.y  = ((prcDPage->bottom - sz.cx) / 2); // and cx on y positon
      // set text postition
      pp2->pt.x       = pt.x + (sz.cy / 2);
      pp2->pt.y       = pt.y;
      // setup line-arrow positions in structure
      pp2->p1.pa_Pt.x = pt.x; // top arrow position
      pp2->p1.pa_Pt.y = prcDPage->top;
      pp2->p1.pa_Type = ptt_Up;
      pp2->p1_To.x    = pt.x;          // line to position
      pp2->p1_To.y    = pt.y;
      pp2->p2.pa_Pt.x = pt.x; // bottom arrow position
      pp2->p2.pa_Pt.y = prcDPage->bottom;
      pp2->p2.pa_Type = ptt_Down;
      pp2->p2_To.x    = pt.x;     // line to position
      pp2->p2_To.y    = pt.y + sz.cx;
      DOP2(pp2);           // out the text, lines and arrows
   }
   else
   {
      goto Add_VPos;
   }

   //db = (double)pps->ps_dyRes / (double)pps->ps_yPerInch;
   wsprintf( lpt, " %s inches ", Dbl2Str( pdt->dbYIns, 3 ) );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );

Try_BLf3:

   if( ( pt.x + ( 3 * sz.cy ) ) < prcPrt->left )
   {
      pt.x += ptl.x; // NOTE: Text at 90 degrees so use cy on x
      pt.y  = ((prcDPage->bottom - sz.cx) / 2); // and cx on y positon
      // set text postition
      pp2->pt.x       = pt.x + (sz.cy / 2);
      pp2->pt.y       = pt.y;
      // setup line-arrow positions in structure
      pp2->p1.pa_Pt.x = pt.x; // top arrow position
      pp2->p1.pa_Pt.y = prcDPage->top;
      pp2->p1.pa_Type = ptt_Up;
      pp2->p1_To.x    = pt.x;          // line to position
      pp2->p1_To.y    = pt.y;
      pp2->p2.pa_Pt.x = pt.x; // bottom arrow position
      pp2->p2.pa_Pt.y = prcDPage->bottom;
      pp2->p2.pa_Type = ptt_Down;
      pp2->p2_To.x    = pt.x;     // line to position
      pp2->p2_To.y    = pt.y + sz.cx;
      DOP2(pp2);           // out the text, lines and arrows
   }

Add_VPos:

   // add dimension text
   if( !pps->ps_bAddText )
      goto Dn_Vert;

   wsprintf( lpt, " %d px ", pdt->dwTop );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );
   if( (prcPrt->top - prcDPage->top) > sz.cx ) // NOTE: size orientation is 2700 sz.cx is VERTICAL size of text
   {
      // will fit above image, so
      // x begin at left + half of wid - half of text len
      pt.x = (prcPrt->left + (RW(prcPrt) / 2));
      pt.y = (prcDPage->top  + ((prcPrt->top - prcDPage->top - sz.cx) / 2) );
      // set text postition
      pp2->pt.x       = pt.x + (sz.cy / 2);
      pp2->pt.y       = pt.y;
      //if( (pt.y - prcDPage->top) > MYASZ )
      if( ( prcPrt->top - prcDPage->top -sz.cx ) > MYASZ )
      {
         // setup line-arrow positions in structure
         pp2->p1.pa_Pt.x = pt.x;    // top arrow position
         pp2->p1.pa_Pt.y = prcDPage->top; // is at top
         pp2->p1.pa_Type = ptt_Up;  // an up arrow
         pp2->p1_To.x    = pt.x;    // line to position
         pp2->p1_To.y    = pt.y;    // top of text
         pp2->p2.pa_Pt.x = pt.x;    // bottom arrow position
         pp2->p2.pa_Pt.y = prcPrt->top;   // is at bottom
         pp2->p2.pa_Type = ptt_Down;   // a down arrow
         pp2->p2_To.x    = pt.x;    // line to position
         pp2->p2_To.y    = pt.y + sz.cx;  // bottom of text
         DOP2(pp2);                 // out the text, lines and arrows
      }
      else
      {
         TO( (pp2->pt.x), (pp2->pt.y) );
      }
   }

   wsprintf( lpt, " %d px ", pdt->dwBot );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );
   if( (prcDPage->bottom - prcPrt->bottom) > sz.cx ) // NOTE: size orientation is 2700 sz.cx is VERTICAL size of text
   {
      // will fit BELOW the image
      // x begin at left + half of wid ( later - half of text height )
      pt.x = (prcPrt->left + (RW(prcPrt) / 2));
      // y begin at bottom + half available - half text length (2700 orien.)
      pt.y = (prcPrt->bottom + ((prcDPage->bottom - prcPrt->bottom) / 2) - (sz.cx / 2) );
      // set text postition
      pp2->pt.x       = pt.x + (sz.cy / 2);
      pp2->pt.y       = pt.y;
      //if( ( prcDPage->bottom - pt.y ) > 6 )
      if( ( prcDPage->bottom - prcPrt->bottom - sz.cx ) > MYASZ ) // NOTE: size orientation is 2700 sz.cx is VERTICAL size of text
      {
         // setup line-arrow positions in structure
         pp2->p1.pa_Pt.x = pt.x;    // top arrow position
         pp2->p1.pa_Pt.y = prcPrt->bottom;
         pp2->p1.pa_Type = ptt_Up;
         pp2->p1_To.x    = pt.x;    // line to position
         pp2->p1_To.y    = pt.y;
         pp2->p2.pa_Pt.x = pt.x;    // bottom arrow position
         pp2->p2.pa_Pt.y = prcDPage->bottom;
         pp2->p2.pa_Type = ptt_Down;
         pp2->p2_To.x    = pt.x;    // line to position
         pp2->p2_To.y    = pt.y + sz.cx;
         DOP2(pp2);                 // out the text, lines and arrows
      }
      else
      {
         TO( (pp2->pt.x), (pp2->pt.y) );
      }
   }

   wsprintf( lpt, " %d px ", RH(prcPrint) );
   iLen = lstrlen(lpt);
   GetTextExtentPoint32( hdc, lpt, iLen, &sz );
   if( RH(prcPrt) > sz.cx )  // if sufficient HEIGHT (NOTE: cx used since text is 90 deg.)
   {
      if( (prcPrt->right + sz.cy) < prcDPage->right )  // if room on right of image
      {
         pt.x = prcPrt->right + (sz.cy / 2);
         pt.y = prcPrt->top   + ( (RH(prcPrt) - sz.cx) / 2 );
         // set text postition
         pp2->pt.x       = pt.x + (sz.cy / 2);
         pp2->pt.y       = pt.y;
         if( ( RH(prcPrt) - sz.cx ) > MYASZ )
         {
            // set line-arrows
            pp2->p1.pa_Pt.x = pt.x;    // top arrow position
            pp2->p1.pa_Pt.y = prcPrt->top; // is at top
            pp2->p1.pa_Type = ptt_Up;  // an up arrow
            pp2->p1_To.x    = pt.x;    // line to position
            pp2->p1_To.y    = pt.y;    // top of text
            pp2->p2.pa_Pt.x = pt.x;    // bottom arrow position
            pp2->p2.pa_Pt.y = prcPrt->bottom;   // is at bottom
            pp2->p2.pa_Type = ptt_Down;   // a down arrow
            pp2->p2_To.x    = pt.x;    // line to position
            pp2->p2_To.y    = pt.y + sz.cx;  // bottom of text
            DOP2(pp2);                 // out the text, lines and arrows
         }
         else
         {
            // just text only
            TO( (pp2->pt.x), (pp2->pt.y) );
         }
      }
      else if( (prcPrt->left - sz.cy) > prcDPage->left ) // or on left of image
      {
         pt.x = prcPrt->left  - (sz.cy / 2);
         pt.y = prcPrt->top   + ( (RH(prcPrt) - sz.cx) / 2 );
         // set text postition
         pp2->pt.x       = pt.x + (sz.cy / 2);
         pp2->pt.y       = pt.y;
         if( ( RH(prcPrt) - sz.cx ) > MYASZ )
         {
            // set line-arrows
            pp2->p1.pa_Pt.x = pt.x;    // top arrow position
            pp2->p1.pa_Pt.y = prcPrt->top; // is at top
            pp2->p1.pa_Type = ptt_Up;  // an up arrow
            pp2->p1_To.x    = pt.x;    // line to position
            pp2->p1_To.y    = pt.y;    // top of text
            pp2->p2.pa_Pt.x = pt.x;    // bottom arrow position
            pp2->p2.pa_Pt.y = prcPrt->bottom;   // is at bottom
            pp2->p2.pa_Type = ptt_Down;   // a down arrow
            pp2->p2_To.x    = pt.x;    // line to position
            pp2->p2_To.y    = pt.y + sz.cx;  // bottom of text
            DOP2(pp2);                 // out the text, lines and arrows
         }
         else
         {
            // just text only
            TO( (pp2->pt.x), (pp2->pt.y) );
         }
      }
   }

Dn_Vert:

// Fin_Text:

   if( hOldPen )
      SelectObject( hdc, hOldPen );

   if( hOldFont )
      SelectObject( hdc, hOldFont );

}

VOID  SetCommonTxt( PPS pps, PDOTXT pdt )
{
   pdt->pp2      = &pps->ps_p2;
//   SIZE     szObj;      // object original size
   pdt->szObj.cx = pps->ps_dwOWid;  // width of DIB
   pdt->szObj.cy = pps->ps_dwOHit;  // height of DIB
//   LPRECT   prcPrint;   // printed object size
   pdt->prcPrint = &pps->ps_rcPrt;
//   SIZE     szOff;      // offsets applied
   pdt->szOff.cx = pps->ps_dwLeft;
   pdt->szOff.cy = pps->ps_dwTop;
//   SIZE     szPercent;  // percentages applied
   pdt->szPercent.cx = pps->ps_dwPCWid;
   pdt->szPercent.cy = pps->ps_dwPCHit;
//   SIZE     szSize;     // width and height after percentage
   pdt->szSize.cx = pps->ps_dwWidth;
   pdt->szSize.cy = pps->ps_dwHeight;
//   LPRECT   prcPPage;   // print page size
   pdt->prcPPage  = &pps->ps_rcDPage;  // device page size

   pdt->dxSize   = pps->ps_dxSize;
   pdt->dxPx     = pps->ps_rcDPage.right;
   pdt->dbXIns   = (double)pps->ps_dxRes / (double)pps->ps_xPerInch;
   pdt->dwLeft   = pps->ps_rcPrt.left;
   pdt->dwRight  = pps->ps_rcDPage.right - pps->ps_rcPrt.right;
   pdt->dySize   = pps->ps_dySize;
   pdt->dyPx     = pps->ps_rcDPage.bottom;
   pdt->dbYIns   = (double)pps->ps_dyRes / (double)pps->ps_yPerInch;
   pdt->dwTop    = pps->ps_rcPrt.top;
   pdt->dwBot    = pps->ps_rcDPage.bottom - pps->ps_rcPrt.bottom;

}

VOID  SetPrtDoTxt( HDC hdc, PPS pps, PDOTXT pdt )
{
   pdt->dt_pPrtStr = pps;  // back pointer to main PRTSTR stucture (if ever required)
   pdt->hdc        = hdc;
   SetCommonTxt( pps, pdt );

   // set output rectangle(s)
   pdt->prcPrt   = &pps->ps_rcPrt;     // image pos/size
   pdt->prcDPage = &pps->ps_rcDPage;   // full frame pos/size
   // set output fonts
   pdt->hfHorz   = pps->ps_hPFontA;
   pdt->hfVert   = pps->ps_hPFontV;

}

//   RECT  ps_rcPPage;    // paint object size
//   RECT  ps_rcObj;      // actual pixel object size
//   RECT  ps_rcPObj;     // last rendered size
//   RECT  ps_rcRend;     // last RENDERING of the image
VOID  SetPaintDoTxt( HDC hdc, PPS pps, PDOTXT pdt )
{
   pdt->dt_pPrtStr = pps;  // back pointer to main PRTSTR stucture (if ever required)
   pdt->hdc        = hdc;
   SetCommonTxt( pps, pdt );

   // set output rectangle(s)
   pdt->prcPrt   = &pps->ps_rcRend;    // image pos/size
   pdt->prcDPage = &pps->ps_rcPPage;   // full frame pos/size
   // set output fonts
   pdt->hfHorz   = pps->ps_hFontA;
   pdt->hfVert   = pps->ps_hFontV;

}


VOID  PrintTxt( HDC hdc, PPS pps )
{
   PDOTXT   pdt = &pps->ps_sDT;
   SetPrtDoTxt( hdc, pps, pdt );
   if( pdt->hfHorz && pps->ps_bAddText3 )
   {
      // put a border at the PRINTER border = prcDPage = resolution in pixels
      FrameRect( hdc, &pps->ps_rcDPage, GetStockObject(BLACK_BRUSH) );
      // ********************************************************************
   }
   DoTxt( pps, pdt );
}

VOID  PaintTxt( HDC hdc, PPS pps )
{
   PDOTXT   pdt = &pps->ps_sDT;
   SetPaintDoTxt( hdc, pps, pdt );
   DoTxt( pps, pdt );
}

BOOL  PrintIt( PPS pps )
{
   BOOL  bRet = FALSE;
   HGLOBAL  hg;
   LPTSTR   lpn;
   LPRECT   prcPrt, prcDPage, prcBPage;

   // set rectangle(s)
   prcPrt   = &pps->ps_rcPrt;
   prcDPage = &pps->ps_rcDPage;
   prcBPage = &pps->ps_rcBPage;

   if( ( pps->ps_iPrtOK         ) &&
       ( hg = pps->ps_hEnum     ) &&
       ( lpn = DVGlobalLock(hg) ) )
   {
      PPRINTER_INFO_2   ppi2;
      HDC               hdc;
      PDEVMODE          pdm;
      LPTSTR            lppn, lpdn;
      // get to the SELECTED printer =========================
      ppi2 = (PPRINTER_INFO_2)((LPTSTR)lpn + pps->ps_dwPrtOff);
      lppn = ppi2->pPrinterName;  // if we HAVE a printer NAME
      // =====================================================
      lpdn = ppi2->pDriverName;
      pdm  = ppi2->pDevMode;
      if( !lppn || *lppn == 0 )
      {
         ppi2 = (PPRINTER_INFO_2)lpn;
         ppi2 += pps->ps_dwSel;
         lppn = ppi2->pPrinterName;  // if we HAVE a printer NAME
      }
      if( ( lppn && *lppn             ) &&
          ( hdc = CreateDC( NULL,   // driver name
                           lppn,    // device name
                           NULL,    // not used; should be NULL
                           NULL )     ) ) // optional printer data
      {
         HGLOBAL  hDIB;
         LPTSTR   lpDIB;
         if( ( hDIB = pps->ps_hDIB ) &&
             ( lpDIB = DVGlobalLock(hDIB) ) )   // LOCK DIB
         {
            int      ir;

            // Init the DOCINFO
            //InitDocStruct( &pps->ps_sDI, "DvPrt");
            InitDocStruct( &pps->ps_sDI, &pps->ps_szDocName[0] );

            gbAbort = FALSE;  // ensure NOT set

				ghDlgAbort = CreateDialogParam( ghDvInst,
					"PRINTDLG", // Name of Print dialog from .RC
               GetFocus(),
               PRINTABORTDLG,
               (LPARAM) &pps->ps_sDI );
            if( ghDlgAbort )
            {
               SetAbortProc( hdc, PRINTABORTPROC );
            }

            // start the document
            if( StartDoc( hdc, &pps->ps_sDI ) <= 0 )
            {
               gdwError = GetLastError();
               ir = GDI_ERROR;
               goto Done_Doc;
            }

            // Print one page
            if( StartPage( hdc ) <= 0 )
            {
               gdwError = GetLastError();
               ir = GDI_ERROR;
               goto Done_Doc;
            }

            // output to PRINTER HDC
            ir = StretchDIBits( hdc,                        // DestDC
                                prcPrt->left,               // DestX
                                prcPrt->top,                // DestY
                                RW(prcPrt),                 // DestWidth
                                RH(prcPrt),                 // DestHeight
                                pps->ps_rcDIB.left,   // = 0 - SrcX
                                pps->ps_rcDIB.top,    // = 0 - SrcY
                                pps->ps_rcDIB.right,  // SrcWidth
                                pps->ps_rcDIB.bottom, // SrcHeight
                                FindDIBBits(lpDIB),   // lpBits
                                (LPBITMAPINFO) lpDIB,    // lpBitInfo
                                DIB_RGB_COLORS,          // wUsage
                                SRCCOPY );               // dwROP

            if( pps->ps_bAddText || pps->ps_bAddText2 || pps->ps_bAddText3 )
            {
               PrintTxt( hdc, pps );
            }

            ChangePP( 100 );

            // indicate end of page
            EndPage( hdc );

            // Indicate end of document
            EndDoc( hdc );


Done_Doc:

            if( ghDlgAbort )
               DestroyWindow( ghDlgAbort );
            ghDlgAbort = 0;

            DVGlobalUnlock(hDIB);   // UNLOCK DIB

            if( ir != GDI_ERROR )
            {
               sprtf( "PRINTED: Obj %s as %s"MEOR
                  "\ton %s. Scans=%d"MEOR,
                  Rect2Stg( &pps->ps_rcDIB ),
                  Rect2Stg( prcPrt ),
                  Rect2Stg( &pps->ps_rcDPage ),
                  ir );
               bRet = TRUE;
            }
            else
            {
               sprtf( "ERROR: Print FAILED! %s on %s."MEOR,
                  Rect2Stg( prcPrt ),
                  Rect2Stg( &pps->ps_rcDPage ) );
            }
         }
         else
         {
               sprtf( "ERROR: Print FAILED of %s. NO hDIB or lpDIB MEMORY!"MEOR,
                  Rect2Stg( prcPrt ) );
         }

         if( bRet )
         {
            // AWK!!! THIS DOES NOT WORK PROPERLY!!! WHY????????
            // *************************************************
            if( bOpenPW )
            {
               PRDIB prd = &pps->ps_sRDIB;
               DWORD dwi = 1;

               //sprtf( "Doing CopyHDCToDIB() with hdc=%#x area=%s."MEOR,
               //   hdc, Rect2Stg( &pps->ps_rcArea ) );
               //if( prd->rd_hDIB = CopyHDCToDIB( hdc, &pps->ps_rcArea ) )   // !prcDPage
               sprtf( "Doing CopyHDCToDIB() with hdc=%#x area=%s."MEOR,
                  hdc, Rect2Stg( prcDPage ) );
               if( prd->rd_hDIB = CopyHDCToDIB( hdc, prcDPage ) )   // ??? or area ???
               {
                  prd->rd_pTitle = gszRPTit;     // set TITLE file name
                  prd->rd_pPath  = gszRPNam;     // but keep the FULL NAME
                  DVNextRDName2( prd, "TEMPP%03d.BMP", &dwi, 0 );   // = "TEMPP%03d.BMP"
                  prd->rd_Caller = 123;
                  //rd.rd_Caller  = df_CLIPBRD;
                  if( OpenDIBWindow2( prd ) )
                  {
                     if( WriteBMPFile2( gszRPNam, prd->rd_hDIB, TRUE ) )
                     {
#ifdef	CHGADDTO
                        //AddToFileList4( szTitle );
                        ADD2LIST( prd );
#endif	// CHGADDTO
                     }
                  }
               }
            }
         }

         DeleteDC(hdc); // remove the created HDC

      }
      else
      {
               sprtf( "ERROR: Print FAILED of %s. NO HDC!"MEOR,
                  Rect2Stg( prcPrt ) );
      }

      DVGlobalUnlock(hg);
   }
   else
   {
               sprtf( "ERROR: Print FAILED of %s. NO ENUM MEMORY!"MEOR,
                  Rect2Stg( prcPrt ) );
   }

   if( !bRet )
   {
      chkme( "PRINT request FAILED" );
   }
   return bRet;
}

void Do_IDM_PRINT( HWND hWnd )
{
   HWND     hMDI;
   HGLOBAL  hDIBInfo;
   PDI      lpDIBInfo;
   INT_PTR  ui = 0;
   DWORD    dwi;

   if( ( hMDI = GetCurrentMDIWnd() ) &&
       ( hDIBInfo = GetWindowExtra(hMDI, WW_DIB_HINFO) ) &&
       ( lpDIBInfo   = (LPDIBINFO) DVGlobalLock (hDIBInfo) ) )
   {
      PPS      pps  = (PPS)&lpDIBInfo->di_szCGenBuf2[0];
      PPS      ppss = (PPS)&gszCGenBuf2[0];
      ZeroMemory(pps, sizeof(PRTSTR));

      pps->ps_lpDIBInfo    = lpDIBInfo;   // if ever required

      pps->ps_hMDI         = hMDI;
      pps->ps_hParent      = hWnd;
      pps->ps_hDIB         = lpDIBInfo->hDIB;    // all important DIB handle
      pps->ps_dwOWid       = lpDIBInfo->di_dwDIBWidth;   // DIB pixel width
      pps->ps_dwOHit       = lpDIBInfo->di_dwDIBHeight;  // DIB pixel height
      pps->ps_rcDIB.top    = 0;
      pps->ps_rcDIB.left   = 0;
      pps->ps_rcDIB.right  = pps->ps_dwOWid;
      pps->ps_rcDIB.bottom = pps->ps_dwOHit;

      CopyDefPPS( pps, ppss );

      dwi = lpDIBInfo->di_dwDIBWidth; // pixel width of image
      SETITEM( ps_dwWidth, dwi, nPrtWid );
      dwi = lpDIBInfo->di_dwDIBHeight;   // pixel height of image
      SETITEM( ps_dwHeight, dwi, nPrtHit );

      if( pps->ps_dwPCWid == 0 )
      {
         pps->ps_dwPCWid  = 100;    // percentage width
         nPrtChg[nPrtPCW] = TRUE;
      }
      if( pps->ps_dwPCWid == 0 )
      {
         pps->ps_dwPCHit  = 100;    // percentage height
         nPrtChg[nPrtPCH] = TRUE;
      }

      //pps->ps_dwTop    = 0;   // top of print
      //pps->ps_dwLeft   = 0;  // left of print
      //pps->ps_bDistort = FALSE;  // allow distortion of image
      //pps->ps_bFitPage = FALSE;  // fit image into page
      if( pps->ps_dwCopies == 0 )
         pps->ps_dwCopies = 1;

      if( pps->ps_dwMFWid == 0 )
      {
         pps->ps_dwMFWid = DEF_FRM;
         nPrtChg[nPrtFWid] = TRUE;
      }
      if( pps->ps_dwMFHit == 0 )
      {
         pps->ps_dwMFHit = DEF_FRM;
         nPrtChg[nPrtFHit] = TRUE;
      }
      if( !pps->ps_hDIB )
         goto DoneDlg;

      lstrcpy( &pps->ps_szDocName[0], &lpDIBInfo->di_szDibFile[0] );

	   ui = DialogBoxParam( ghDvInst,
			MAKEINTRESOURCE(IDD_PRINT),
			hWnd,
			PRINT2DLGPROC,
         (LPARAM) hDIBInfo );

DoneDlg:

      if( ui == IDOK )
      {
         if( pps->ps_dwCopies == 0 )
            pps->ps_dwCopies = 1;

         // PRINT IT *** TBD ***
         // ********************
         PrintIt( pps );
         // ********************
         // ====================
         // AND KEEP RESULTS
         CopyDefPPS( ppss, pps );   // copy items back

      }

      if( pps->ps_hEnum )
         DVGlobalFree( pps->ps_hEnum );
      pps->ps_hEnum = 0;

      if( pps->ps_hFontV )
         DeleteObject( pps->ps_hFontV );
      pps->ps_hFontV = 0;

      if( pps->ps_hFontA )
         DeleteObject( pps->ps_hFontA );
      pps->ps_hFontA = 0;

      if( pps->ps_hPFontV )
         DeleteObject( pps->ps_hPFontV );
      pps->ps_hPFontV = 0;

      if( pps->ps_hPFontA )
         DeleteObject( pps->ps_hPFontA );
      pps->ps_hPFontA = 0;

      DVGlobalUnlock(hDIBInfo);
   }


   if( ghDlgAbort )
   {
      sprtf( "Doing DestroyWindow( ghDlgAbort [%#x] )."MEOR, ghDlgAbort );
      DestroyWindow( ghDlgAbort );
   }
   ghDlgAbort = 0;

}


//  Platform SDK: Windows GDI 
//GetDeviceCaps
//The GetDeviceCaps function retrieves device-specific information
//for the specified device. 
//int GetDeviceCaps(
//  HDC hdc,     // handle to DC
//  int nIndex   // index of capability
//);
//Parameters
//hdc 
//[in] Handle to the DC. 
//nIndex 
//[in] Specifies the item to return. This parameter can be one of
//the following values. Index Meaning 
//DRIVERVERSION The device driver version. 
//TECHNOLOGY Device technology. It can be any one of the following
//values. 
//  DT_PLOTTER Vector plotter 
//  DT_RASDISPLAY Raster display 
//  DT_RASPRINTER Raster printer 
//  DT_RASCAMERA Raster camera 
//  DT_CHARSTREAM Character stream 
//  DT_METAFILE Metafile 
//  DT_DISPFILE Display file 
//  If the hdc parameter is a handle to the DC of an enhanced
//metafile, the device technology is that of the referenced device
//as specified to the CreateEnhMetaFile function. To determine
//whether it is an enhanced metafile DC, use the GetObjectType
//function. 
//HORZSIZE Width, in millimeters, of the physical screen. 
//VERTSIZE Height, in millimeters, of the physical screen. 
//HORZRES Width, in pixels, of the screen. 
//VERTRES Height, in raster lines, of the screen. 
//LOGPIXELSX Number of pixels per logical inch along the screen
//width. In a system with multiple display monitors, this value is
//the same for all monitors. 
//LOGPIXELSY Number of pixels per logical inch along the screen
//height. In a system with multiple display monitors, this value
//is the same for all monitors. 
//BITSPIXEL Number of adjacent color bits for each pixel. 
//PLANES Number of color planes. 
//NUMBRUSHES Number of device-specific brushes. 
//NUMPENS Number of device-specific pens. 
//NUMFONTS Number of device-specific fonts. 
//NUMCOLORS Number of entries in the device's color table, if the
//device has a color depth of no more than 8 bits per pixel. For
//devices with greater color depths, - 1 is returned. 
//ASPECTX Relative width of a device pixel used for line drawing. 
//ASPECTY Relative height of a device pixel used for line drawing.
//ASPECTXY Diagonal width of the device pixel used for line
//drawing. 
//PDEVICESIZE Reserved. 
//CLIPCAPS Flag that indicates the clipping capabilities of the
//device. If the device can clip to a rectangle, it is 1.
//Otherwise, it is 0. 
//SIZEPALETTE Number of entries in the system palette. This index
//is valid only if the device driver sets the RC_PALETTE bit in
//the RASTERCAPS index and is available only if the driver is
//compatible with 16-bit Windows. 
//NUMRESERVED Number of reserved entries in the system palette.
//This index is valid only if the device driver sets the
//RC_PALETTE bit in the RASTERCAPS index and is available only if the driver
//is compatible with 16-bit Windows. 
//COLORRES Actual color resolution of the device, in bits per
//pixel. This index is valid only if the device driver sets the
//RC_PALETTE bit in the RASTERCAPS index and is available only if
//the driver is compatible with 16-bit Windows. 
//PHYSICALWIDTH For printing devices: the width of the physical
//page, in device units. For example, a printer set to print at
//600 dpi on 8.5-x11-inch paper has a physical width value of 5100
//device units. Note that the physical page is almost always
//greater than the printable area of the page, and never smaller. 
//PHYSICALHEIGHT For printing devices: the height of the physical
//page, in device units. For example, a printer set to print at
//600 dpi on 8.5-by-11-inch paper has a physical height value of
//6600 device units. Note that the physical page is almost always
//greater than the printable area of the page, and never smaller. 
//PHYSICALOFFSETX For printing devices: the distance from the left
//edge of the physical page to the left edge of the printable
//area, in device units. For example, a printer set to print at
//600 dpi on 8.5-by-11-inch paper, that cannot print on the
//leftmost 0.25-inch of paper, has a horizontal physical offset of
//150 device units.  
//PHYSICALOFFSETY For printing devices: the distance from the top
//edge of the physical page to the top edge of the printable area,
//in device units. For example, a printer set to print at 600 dpi
//on 8.5-by-11-inch paper, that cannot print on the topmost
//0.5-inch of paper, has a vertical physical offset of 300 device
//units.  
//VREFRESH Windows NT/2000: For display devices: the current
//vertical refresh rate of the device, in cycles per second (Hz). 
//A vertical refresh rate value of 0 or 1 represents the display
//hardware's default refresh rate. This default rate is typically
//set by switches on a display card or computer motherboard, or by
//a configuration program that does not use Win32 display
//functions such as ChangeDisplaySettings. 
//SCALINGFACTORX Scaling factor for the x-axis of the printer. 
//SCALINGFACTORY Scaling factor for the y-axis of the printer.  
//BLTALIGNMENT Windows NT/2000: Preferred horizontal drawing
//alignment, expressed as a multiple of pixels. For best drawing
//performance, windows should be horizontally aligned to a
//multiple of this value. A value of zero indicates that the
//device is accelerated, and any alignment may be used. 
//SHADEBLENDCAPS Windows 98, Windows 2000: Value that indicates
//the shading and blending capabilities of the device.  
//  SB_CONST_ALPHA Handles the SourceConstantAlpha member of the
//BLENDFUNCTION structure, which is referenced by the
//blendFunction parameter of the AlphaBlend function. 
//  SB_GRAD_RECT Capable of doing GradientFill rectangles. 
//  SB_GRAD_TRI Capable of doing GradientFill triangles. 
//  SB_NONE Device does not support any of these capabilities. 
//  SB_PIXEL_ALPHA Capable of handling per-pixel alpha in
//AlphaBlend.  
//  SB_PREMULT_ALPHA Capable of handling premultiplied alpha in
//AlphaBlend.  
//RASTERCAPS Value that indicates the raster capabilities of the
//device, as shown in the following table. 
//  RC_BANDING Requires banding support. 
//  RC_BITBLT Capable of transferring bitmaps. 
//  RC_BITMAP64 Capable of supporting bitmaps larger than 64 KB. 
//  RC_DI_BITMAP Capable of supporting the SetDIBits and GetDIBits
//functions. 
//  RC_DIBTODEV Capable of supporting the SetDIBitsToDevice
//function. 
//  RC_FLOODFILL Capable of performing flood fills. 
//  RC_GDI20_OUTPUT Capable of supporting features of 16-bit
//Windows 2.0. 
//  RC_PALETTE Specifies a palette-based device. 
//  RC_SCALING Capable of scaling. 
//  RC_STRETCHBLT Capable of performing the StretchBlt function. 
//  RC_STRETCHDIB Capable of performing the StretchDIBits
//function. 
//CURVECAPS Value that indicates the curve capabilities of the
//device, as shown in the following table: 
//  CC_NONE Device does not support curves. 
//  CC_CHORD Device can draw chord arcs. 
//  CC_CIRCLES Device can draw circles. 
//  CC_ELLIPSES Device can draw ellipses. 
//  CC_INTERIORS Device can draw interiors. 
//  CC_PIE Device can draw pie wedges. 
//  CC_ROUNDRECT Device can draw rounded rectangles. 
//  CC_STYLED Device can draw styled borders. 
//  CC_WIDE Device can draw wide borders. 
//  CC_WIDESTYLED Device can draw borders that are wide and
//styled. 
//LINECAPS Value that indicates the line capabilities of the
//device, as shown in the following table: 
//  LC_NONE Device does not support lines. 
//  LC_INTERIORS Device can draw interiors. 
//  LC_MARKER Device can draw a marker. 
//  LC_POLYLINE Device can draw a polyline. 
//  LC_POLYMARKER Device can draw multiple markers. 
//  LC_STYLED Device can draw styled lines. 
//  LC_WIDE Device can draw wide lines. 
//  LC_WIDESTYLED Device can draw lines that are wide and styled. 
//POLYGONALCAPS Value that indicates the polygon capabilities of
//the device, as shown in the following table. 
//  PC_NONE Device does not support polygons. 
//  PC_INTERIORS Device can draw interiors. 
//  PC_POLYGON Device can draw alternate-fill polygons. 
//  PC_RECTANGLE Device can draw rectangles. 
//  PC_SCANLINE Device can draw a single scanline. 
//  PC_STYLED Device can draw styled borders. 
//  PC_WIDE Device can draw wide borders. 
//  PC_WIDESTYLED Device can draw borders that are wide and
//styled. 
//  PC_WINDPOLYGON Device can draw winding-fill polygons. 
//TEXTCAPS Value that indicates the text capabilities of the
//device, as shown in the following table. 
//  TC_OP_CHARACTER Device is capable of character output
//precision. 
//  TC_OP_STROKE Device is capable of stroke output precision. 
//  TC_CP_STROKE Device is capable of stroke clip precision. 
//  TC_CR_90 Device is capable of 90-degree character rotation. 
//  TC_CR_ANY Device is capable of any character rotation. 
//  TC_SF_X_YINDEP Device can scale independently in the x- and
//y-directions. 
//  TC_SA_DOUBLE Device is capable of doubled character for
//scaling. 
//  TC_SA_INTEGER Device uses integer multiples only for character
//scaling. 
//  TC_SA_CONTIN Device uses any multiples for exact character
//scaling. 
//  TC_EA_DOUBLE Device can draw double-weight characters. 
//  TC_IA_ABLE Device can italicize. 
//  TC_UA_ABLE Device can underline. 
//  TC_SO_ABLE Device can draw strikeouts. 
//  TC_RA_ABLE Device can draw raster fonts. 
//  TC_VA_ABLE Device can draw vector fonts. 
//  TC_RESERVED Reserved; must be zero. 
//  TC_SCROLLBLT Device cannot scroll using a bit-block transfer.
//Note that this meaning may be the opposite of what you expect. 
//COLORMGMTCAPS Windows 2000: Value that indicates the color
//management capabilities of the device. 
//  CM_CMYK_COLOR Device can accept CMYK color space ICC color
//profile. 
//  CM_DEVICE_ICM Device can perform ICM on either the device
//driver or the device itself. 
//  CM_GAMMA_RAMP Device supports GetDeviceGammaRamp and
//SetDeviceGammaRamp 
//  CM_NONE Device does not support ICM. 
//Return Values
//The return value specifies the value of the desired item. 
//When nIndex is BITSPIXEL and the device has 15bpp or 16bpp, the
//return value is 16.
//Remarks
//GetDeviceCaps provides the following six indexes in place of
//printer escapes. 
//Index Printer escape replaced 
//PHYSICALWIDTH GETPHYSPAGESIZE 
//PHYSICALHEIGHT GETPHYSPAGESIZE 
//PHYSICALOFFSETX GETPRINTINGOFFSET 
//PHYSICALOFFSETY GETPHYSICALOFFSET 
//SCALINGFACTORX GETSCALINGFACTOR 
//SCALINGFACTORY GETSCALINGFACTOR 
//Requirements 
//  Windows NT/2000: Requires Windows NT 3.1 or later.
//  Windows 95/98: Requires Windows 95 or later.
//  Header: Declared in Wingdi.h; include Windows.h.
//  Library: Use Gdi32.lib.
//See Also
//Device Contexts Overview, Device Context Functions,
//CreateEnhMetaFile, CreateIC, DeviceCapabilities, GetDIBits,
//GetObjectType, SetDIBits, SetDIBitsToDevice, StretchBlt,
//StretchDIBits 
//Built on Wednesday, July 12, 2000Requirements 
//  Windows NT/2000: Requires Windows NT 3.1 or later.
//  Windows 95/98: Requires Windows 95 or later.
//  Header: Declared in Wingdi.h; include Windows.h.
//  Library: Use Gdi32.lib.
//See Also
//Device Contexts Overview, Device Context Functions,
//CreateEnhMetaFile, CreateIC, DeviceCapabilities, GetDIBits,
//GetObjectType, SetDIBits, SetDIBitsToDevice, StretchBlt,
//StretchDIBits
// from ewm project
//PRINTDLG gsPD;
VOID  GetGlobPrinter( VOID )
{
   PRINTDLG *  ppd = &gsPD;
   PPS         pps = (PPS)&gszCGenBuf2[0];
   LPRECT      prcDPage = &pps->ps_rcDPage;
   LPRECT      prcTPage = &pps->ps_rcTPage;

//#ifndef MAC
//typedef struct tagPD { 
//  DWORD           lStructSize; 
//  HWND            hwndOwner; 
//  HGLOBAL         hDevMode; 
//  HGLOBAL         hDevNames; 
//  HDC             hDC; 
//  DWORD           Flags; 
//  WORD            nFromPage; 
//  WORD            nToPage; 
//  WORD            nMinPage; 
//  WORD            nMaxPage; 
//  WORD            nCopies; 
//  HINSTANCE       hInstance; 
//  LPARAM          lCustData; 
//  LPPRINTHOOKPROC lpfnPrintHook; 
//  LPSETUPHOOKPROC lpfnSetupHook; 
//  LPCTSTR         lpPrintTemplateName; 
//  LPCTSTR         lpSetupTemplateName; 
//  HGLOBAL         hPrintTemplate; 
//  HGLOBAL         hSetupTemplate; 
//} PRINTDLG, *LPPRINTDLG;
   ZeroMemory(ppd, sizeof(PRINTDLG));
   ppd->lStructSize = sizeof(PRINTDLG);
//	ppd->hwndOwner   = hwndChild;
	ppd->hwndOwner   = ghMainWnd;
	ppd->Flags       = PD_RETURNDEFAULT;
   if( !PrintDlg(ppd) && CommDlgExtendedError())
   {
      sprtf( "Getting default printer failed, thus setting PRINTDLG (pd member) 0!"MEOR );
		ZeroMemory(ppd, sizeof(PRINTDLG));
   }
   else
   {
      HGLOBAL  hDevMode; // contains
//typedef struct _devicemode { 
//  BCHAR  dmDeviceName[CCHDEVICENAME]; 
//  WORD   dmSpecVersion; 
//  WORD   dmDriverVersion; 
//  WORD   dmSize; 
//  WORD   dmDriverExtra; 
//  DWORD  dmFields; 
//  union {
//    struct {
//      short dmOrientation;
//      short dmPaperSize;
//      short dmPaperLength;
//      short dmPaperWidth;
//    };
//    POINTL dmPosition;
//  };
//  short  dmScale; 
//  short  dmCopies; 
//  short  dmDefaultSource; 
//  short  dmPrintQuality; 
//  short  dmColor; 
//  short  dmDuplex; 
//  short  dmYResolution; 
//  short  dmTTOption; 
//  short  dmCollate; 
//  BCHAR  dmFormName[CCHFORMNAME]; 
//  WORD  dmLogPixels; 
//  DWORD  dmBitsPerPel; 
//  DWORD  dmPelsWidth; 
//  DWORD  dmPelsHeight; 
//  union {
//    DWORD  dmDisplayFlags; 
//    DWORD  dmNup;
//  }
//  DWORD  dmDisplayFrequency; 
//#if(WINVER >= 0x0400) 
//  DWORD  dmICMMethod;
//  DWORD  dmICMIntent;
//  DWORD  dmMediaType;
//  DWORD  dmDitherType;
//  DWORD  dmReserved1;
//  DWORD  dmReserved2;
//#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
//  DWORD  dmPanningWidth;
//  DWORD  dmPanningHeight;
//#endif
//#endif /* WINVER >= 0x0400 */
//} DEVMODE;
   HGLOBAL         hDevNames; // contains
// typedef struct tagDEVNAMES {
//  WORD wDriverOffset; 
//  WORD wDeviceOffset; 
//  WORD wOutputOffset; 
//  WORD wDefault; 
// Driver, device, and port name strings follow wDefault. 
//} DEVNAMES, *LPDEVNAMES; 
      if( hDevMode = ppd->hDevMode )
      {

         PDEVMODE    pdm;
         LPDEVNAMES  pdn;
         if( pdm = (DEVMODE *)GlobalLock(hDevMode) )
         {
            if( pdm->dmDeviceName[0] )
            {
               sprtf( "Default printer is [%s]."MEOR, &pdm->dmDeviceName[0] );
               lstrcpy( &gszDefPrtr[0], &pdm->dmDeviceName[0] );
               ChkDefPrtr();

            }
            else
            {
               sprtf( "Default printer name is BLANK! (in mem %#x h %#x)!"MEOR,
                  pdm, hDevMode );
            }

            if( ( hDevNames = ppd->hDevNames ) &&
                ( pdn = (LPDEVNAMES)GlobalLock(hDevNames) ) )
            {
               GetDevInfo( pps, pdn, pdm );  // calls GetHdcInfo()
               GlobalUnlock(hDevNames);
            }
            else
            {
               sprtf( MEOR"WARNING: hDevNames FAILED to LOCK!!!"MEOR );
            }
            GlobalUnlock(hDevMode);
         }
         else
         {
            sprtf( "hDevMode failed to yield memory! (h %#x)"MEOR, hDevMode );
         }
      }
      else
      {
         sprtf( "Default printer returned with ZERO hDevMode!" );
      }

   }  // DEFAULT printer from PrintDlg() API n/y

//#ifndef  NDEBUG
//   ShowDBGSizes( pps, &gszDefPrtr[0] );
//#endif   // #ifndef  NDEBUG

//#endif   // !MAC

}

TCHAR szPrt[]     = "Printer";    // [Section] header
// items in this section
TCHAR szPrtDef[]  = "Default";    // nPrtDef - items in printer section
// some numbers only
TCHAR szPrtxPI[]   = "XPixelPerInch";     // nPrtxPI
TCHAR szPrtyPI[]   = "YPixelPerInch";     // nPrtyPI
TCHAR szPrtHRes[]  = "HorizRes";          // nPrtHRes
TCHAR szPrtVRes[]  = "VertRes";           // nPrtVRes
TCHAR szPrtHSiz[]  = "HorizSizemm";       // nPrtHSiz
TCHAR szPrtVSiz[]  = "VertiSizemm";       // nPrtVSiz
TCHAR szPrtHPhy[]  = "HPhySizeDU";        // nPrtHPhy - ps_PhyWidth, szPrtHPhy PHYSICALWIDTH - the width in device units.
TCHAR szPrtVPhy[]  = "VPhySizeDU";        // nPrtVPhy
TCHAR szPrtPOffH[] = "HPhyOffset";        // nPrtPOffH - ps_PhyOffX
TCHAR szPrtPOffV[] = "VPhyOffset";        // nPrtPOffV - ps_PhyOffY
// calculated TWIPS
TCHAR szPrtxTPP[]  = "XTwipsPerPage";     // nPrtxTPP = ps_dxPage
TCHAR szPrtyTPP[]  = "YTwipsPerPage";     // nPrtyTPP = ps_dyPage

// some RECTANGLES
TCHAR szPrtArea[]  = "Area";        //    ps_rcArea,  nPrtArea (RECT) Area DU
TCHAR szPrtDPg[]   = "PageInDU";    //    ps_rcDPage, nPrtDPg
TCHAR szPrtTPg[]   = "PageInTwips"; //    ps_rcTPage, nPrtTPg
TCHAR szPrtBPg[]   = "PageBorder";  //    ps_rcBPage, nPrtBPg
//   RECT  ps_rcRend;     // last RENDERING of the image
//   RECT  ps_rcPrt;      // and as object would PRINTED be on "real" PAGE
TCHAR szPrtRend[]  = "RenderedAs";  // ps_rcRend, nPrtRend
TCHAR szPrtPrt[]   = "PrintedAs";   // ps_rcPrt,  nPrtPrt
TCHAR szPrtPPage[] = "DlgPaintPage";   // ps_rcPPage, nPrtPPg - dialog paint page size
TCHAR szPrtDlgFrm[]= "DlgFrameRect";   // ps_rcDlgFrm, nPrtFrm
TCHAR szPrtDlgPnt[]= "DlgPaintRect";   // ps_rcPFrame, nPrtPRc

TCHAR szPrtTop[]   = "PrtTop";   // ps_dwTop, nPrtTop
TCHAR szPrtLeft[]  = "PrtLeft";  // ps_dwLeft, nPrtLeft
TCHAR szPrtWid[]   = "PrtWidth"; // ps_dwWid,  nPrtWid
TCHAR szPrtHit[]   = "PrtHeight";   // ps_dwHit, nPrtHit
TCHAR szPrtXPc[]   = "XPercent";    // ps_dwPCWid, nPrtPCW
TCHAR szPrtYPc[]   = "YPercent";    // ps_dwPCHit, nPrtPCH

TCHAR szPrtCop[]   = "Copies";

// some BOOLEAN items
TCHAR szPrtDist[]  = "AllowDistortion";
TCHAR szPrtFit[]   = "FitToPage";
TCHAR szPrtTxt[]   = "AddDimensionText";
TCHAR szPrtTxt2[]  = "AddGeneralText";
TCHAR szPrtTxt3[]  = "AddBorderText";

//pps->ps_dwMFHit 
TCHAR szPrtFHit[]  = "TextHeightx2";   // nPrtFHit
TCHAR szPrtFWid[]  = "TextWidthx2";    // nPrtChg[nPrtFWid]

TCHAR szPrtHTxt[]  = "H_Text";         // nPrtHTxt
TCHAR szPrtVTxt[]  = "V_Text";         // nPrtVTxt
TCHAR szPrtHTS[]   = "H_Text_Size";    // nPrtHTS
TCHAR szPrtVTS[]   = "V_Text_Size";    // nPrtVTS

#define  CPPS(a)     ppsn->a = ppss->a
#define  CPPSS(a)     lstrcpy(ppsn->a, ppss->a);

VOID  CopyDefPPS( PPS ppsn, PPS ppss )
{

   CPPSS(ps_szDefPrtr); // copy default and later SELECTED printer name

   CPPS(ps_dwTop);   // top of print
   CPPS(ps_dwLeft);  // left of print
   CPPS(ps_dwPCWid); // percentage width
   CPPS(ps_dwPCHit); // percentage height
   CPPS(ps_dwWidth); //  = lpDIBInfo->di_dwDIBWidth; // pixel width of image
   CPPS(ps_dwHeight);   // = lpDIBInfo->di_dwDIBHeight;   // pixel height of image

   CPPS(ps_bDistort);   // = FALSE;  // allow distortion of image
   CPPS(ps_bFitPage);   // = FALSE;  // fit image into page
   CPPS(ps_bAddText);   // = TRUE;  // add dimension text
   CPPS(ps_bAddText2);   // = FALSE;  // add general text
   CPPS(ps_bAddText3);   // = FALSE;  // add border text

   CPPS(ps_dwCopies);   // copies to be printed


   CPPS(ps_xPerInch);   // = GetDeviceCaps( hdcTarget, LOGPIXELSX);
   CPPS(ps_yPerInch);   // = GetDeviceCaps( hdcTarget, LOGPIXELSY);
   // like FORMATRANGE
   CPPS(ps_rcArea);     // (RECT) Area (in Device Units)
   CPPS(ps_rcTPage);     // Entire area of rendering device. Units in twips. 
   CPPS(ps_rcDPage);     // Entire area of rendering device. In Device Units
   CPPS(ps_rcBPage);     // Printer area (minus borders) in pixels
   CPPS(ps_rcRend);  //  szPrtRenc, nPrtRend );
   CPPS(ps_rcPrt);   //   szPrtPrt,  nPrtPrt  );
   CPPS(ps_rcPPage); // dialog paint page size
   CPPS(ps_rcDlgFrm);
   CPPS(ps_rcPFrame);

   CPPS(ps_dxPage);
   CPPS(ps_dyPage);
   CPPS(ps_dxRes);   // = GetDeviceCaps(ppp->fr.hdcTarget, HORZRES);
   CPPS(ps_dyRes);   // = GetDeviceCaps(ppp->fr.hdcTarget, VERTRES);
   CPPS(ps_dxSize);  // = GetDeviceCaps( hdcTarget, HORZSIZE  );   // in millimeters
   CPPS(ps_dySize);  //   = GetDeviceCaps( hdcTarget, VERTSIZE  );
   CPPS(ps_PhyWidth);   // PHYSICALWIDTH - the width in device units.
//   // eg at 600 dpi on 8.5x11 in. paper has width of 5100 device units.
   CPPS(ps_PhyHeight);  // PHYSICALHEIGHT - the height in device units.
//   // eg at 600 dpi on 8.5x11 in. paper has height value of 6600 device units.
   CPPS(ps_PhyOffX);    // PHYSICALOFFSETX -  distance from left in device units.
//   // eg at 600 dpi on 8.5x11 in. paper, that cannot print on the leftmost 0.25-inch
//   // has a horizontal physical offset of 150 device units. 
   CPPS(ps_PhyOffY);    // PHYSICALOFFSETY the distance from the top in device units.

   CPPS(ps_dwMFHit); // szPrtFHit = "TextHeightx2";
   CPPS(ps_dwMFWid); // szPrtFHit = "TextWidthx2";

   //TCHAR       ps_szBotTxt[64];  // TCHAR szPrtHTxt[]  = "H_Text";         // nPrtHTxt
   //TCHAR       ps_szRitTxt[64];  // TCHAR szPrtVTxt[]  = "V_Text";         // nPrtVTxt
   //SIZE        ps_ssFA;    // string size szPrtHTS[] = "H_Text_Size"; // nPrtHTS
   //SIZE        ps_ssF2;    // string size szPrtVTS[] = "V_Text_Size"; // nPrtVTS
   CPPSS(ps_szBotTxt);
   CPPSS(ps_szRitTxt);
   CPPS(ps_ssFA);
   CPPS(ps_ssF2);

}


#define  RDPPSS2(a,b,c,d)\
   if( !GetPrivateProfileString( lps, b, (LPSTR)"", pps->a, d, lpIni ) )\
      nPrtChg[c] = TRUE;

#define  RDPPS2(a,b,c)\
   if( ( GetPrivateProfileString( lps, b, (LPSTR)"", lpb, 256, lpIni ) ) &&\
       ( *lpb ) && ( dv_wscanf( lpb, "%ld", (PINT8)&dwi ) == 1 ) ) \
       pps->a = dwi;\
   else \
       nPrtChg[c] = TRUE;


// NOTE WELL: sscanf() appears to yield a PROTECTION FAULT!!!!!!!!!!
#define  RDPPSR2(a,b, c)\
   if( ( GetPrivateProfileString( lps, b, (LPSTR)"", lpb, 256, lpIni ) ) &&\
       ( *lpb ) && ( dv_wscanf( lpb, "%ld,%ld,%ld,%ld", (PINT8)&rc ) == 4 ) )\
   {   pps->a.left  = rc.left;\
      pps->a.top    = rc.top;\
      pps->a.right  = rc.right;\
      pps->a.bottom = rc.bottom;\
   }\
   else \
      nPrtChg[c] = TRUE;

#define  RDPPSSZ2(a,b, c)\
   if( ( GetPrivateProfileString( lps, b, (LPSTR)"", lpb, 256, lpIni ) ) &&\
       ( *lpb ) && ( dv_wscanf( lpb, "%ld,%ld", (PINT8)&sz ) == 2 ) )\
   {   pps->a.cx  = sz.cx;\
      pps->a.cy   = sz.cy;\
   }\
   else \
      nPrtChg[c] = TRUE;

#define  RDPPSB2F(a,b,c)\
if( GetPrivateProfileString( lps, b, (LPSTR)"", lpb, 256, lpIni ) )\
{\
   if( IsYes(lpb) )\
   {\
      pps->a = TRUE;\
   }else if( IsNo(lpb) ){\
      pps->a = FALSE;\
   }else{\
      pps->a = FALSE;\
      nPrtChg[c] = TRUE;\
   }\
}else{\
   pps->a = FALSE;\
   nPrtChg[c] = TRUE;\
}

#define  RDPPSB2T(a,b,c)\
if( GetPrivateProfileString( lps, b, (LPSTR)"", lpb, 256, lpIni ) )\
{\
   if( IsYes(lpb) )\
   {\
      pps->a = TRUE;\
   }else if( IsNo(lpb) ){\
      pps->a = FALSE;\
   }else{\
      pps->a = TRUE;\
      nPrtChg[c] = TRUE;\
   }\
}else{\
   pps->a = TRUE;\
   nPrtChg[c] = TRUE;\
}

VOID  ReadPrtIni( LPTSTR lpIni, BOOL fChgAll )
{
   PPS pps = (PPS)&gszCGenBuf2[0];
   LPTSTR   lps = &szPrt[0];  //"Printer";  // [Section] header
   LPTSTR   lpb = GetTmp2();	// &Buf[0];
   DWORD    dwi;
   RECT     rc;
   SIZE     sz;

   ZeroMemory( &nPrtChg[0], sizeof(nPrtChg) );
   nPrtChg[nPrtALL] = fChgAll;   // set/reset change all flag

   //GetPrivateProfileString( lps, &szPrtDef[0], (LPSTR)"", &gszDefPrtr[0], 256, lpIni );
   //GetPrivateProfileString( lps, &szPrtDef[0], (LPSTR)"", &pszDefPrtr[0], 32, lpIni );
   RDPPSS2(ps_szDefPrtr, szPrtDef, nPrtDef, 32);

   // TCHAR szPrtTop[]   = "PrtTop";   // ps_dwTop, nPrtTop
   // TCHAR szPrtLeft[]  = "PrtLeft";  // ps_dwLeft, nPrtLeft
   RDPPS2(ps_dwTop,   szPrtTop,  nPrtTop );
   RDPPS2(ps_dwLeft,  szPrtLeft, nPrtLeft);
   RDPPS2(ps_dwPCWid, szPrtXPc,  nPrtPCW ); // percentage width
   RDPPS2(ps_dwPCHit, szPrtYPc,  nPrtPCH ); // percentage height
   RDPPS2(ps_dwWidth,  szPrtWid,  nPrtWid ); // percentage width
   RDPPS2(ps_dwHeight, szPrtHit,  nPrtHit ); // percentage height
   RDPPS2(ps_dwCopies, szPrtCop,  nPrtCop ); // copies to be printed

   RDPPS2(ps_xPerInch, szPrtxPI, nPrtxPI);   // = GetDeviceCaps( hdcTarget, LOGPIXELSX);
   RDPPS2(ps_yPerInch, szPrtyPI, nPrtyPI);   // = GetDeviceCaps( hdcTarget, LOGPIXELSY);

   // like FORMATRANGE
//   CPPS(ps_rcTPage);     // Entire area of rendering device. Units in twips. 
//   CPPS(ps_rcDPage);     // Entire area of rendering device. In Device Units
   // like FORMATRANGE
   RDPPSR2(ps_rcArea,  szPrtArea, nPrtArea);     // (RECT) Area (in Device Units)
   RDPPSR2(ps_rcTPage, szPrtTPg,  nPrtTPg );     // Entire area of rendering device. Units in twips. 
   RDPPSR2(ps_rcDPage, szPrtDPg,  nPrtDPg );     // Entire area of rendering device. In Device Units
   RDPPSR2(ps_rcBPage, szPrtBPg,  nPrtBPg );   // print area in pixels (minus border)
   //TCHAR szPrtRend[]  = "RenderedAs";  // ps_rcRend, nPrtRend
   //TCHAR szPrtPrt[]   = "PrintedAs";   // ps_rcPrt,  nPrtPrt
   RDPPSR2(ps_rcRend,  szPrtRend, nPrtRend );
   RDPPSR2(ps_rcPrt,   szPrtPrt,  nPrtPrt  );
   //TCHAR szPrtPPage[] = "DlgPaintPage";   // ps_rcPPage, nPrtPPg - dialog paint page size
   //TCHAR szPrtDlgFrm[]= "DlgFrameRect";   // ps_rcDlgFrm, nPrtFrm
   RDPPSR2(ps_rcPPage,  szPrtPPage,  nPrtPPg );
   RDPPSR2(ps_rcDlgFrm, szPrtDlgFrm, nPrtFrm );
   RDPPSR2(ps_rcPFrame, szPrtDlgPnt, nPrtPRc );

   RDPPS2(ps_dxPage, szPrtxTPP, nPrtxTPP );
   RDPPS2(ps_dyPage, szPrtyTPP, nPrtyTPP );
   RDPPS2(ps_dxRes,  szPrtHRes, nPrtHRes );   // = GetDeviceCaps(ppp->fr.hdcTarget, HORZRES);
   RDPPS2(ps_dyRes,  szPrtVRes, nPrtVRes );   // = GetDeviceCaps(ppp->fr.hdcTarget, VERTRES);
   RDPPS2(ps_dxSize, szPrtHSiz, nPrtHSiz );  // = GetDeviceCaps( hdcTarget, HORZSIZE  );   // in millimeters
   RDPPS2(ps_dySize, szPrtVSiz, nPrtVSiz );  //   = GetDeviceCaps( hdcTarget, VERTSIZE  );

   RDPPS2(ps_PhyWidth, szPrtHPhy, nPrtHPhy);   // PHYSICALWIDTH - the width in device units.
//   // eg at 600 dpi on 8.5x11 in. paper has width of 5100 device units.
   RDPPS2(ps_PhyHeight, szPrtVPhy, nPrtVPhy);  // PHYSICALHEIGHT - the height in device units.
//   // eg at 600 dpi on 8.5x11 in. paper has height value of 6600 device units.
   RDPPS2(ps_PhyOffX,   szPrtPOffH, nPrtPOffH);    // PHYSICALOFFSETX -  distance from left in device units.
//   // eg at 600 dpi on 8.5x11 in. paper, that cannot print on the leftmost 0.25-inch
//   // has a horizontal physical offset of 150 device units. 
   RDPPS2(ps_PhyOffY,   szPrtPOffV, nPrtPOffV);    // PHYSICALOFFSETY the distance from the top in device units.

   RDPPSB2F(ps_bDistort, szPrtDist, nPrtDist );    // allow distortion
   RDPPSB2F(ps_bFitPage, szPrtFit,  nPrtFit  );   // fit image into page
   RDPPSB2T(ps_bAddText,  szPrtTxt,   nPrtTxt  );   // add dimension text
   RDPPSB2F(ps_bAddText2, szPrtTxt2,  nPrtTxt2 );   // general text
   RDPPSB2F(ps_bAddText3, szPrtTxt3,  nPrtTxt3 );   // border text

   RDPPS2(ps_dwMFHit,   szPrtFHit, nPrtFHit); // "TextHeightx2";
   RDPPS2(ps_dwMFWid,   szPrtFWid, nPrtFWid); // "TextWidthx2";

   //TCHAR       ps_szBotTxt[64];  // TCHAR szPrtHTxt[]  = "H_Text";         // nPrtHTxt
   //TCHAR       ps_szRitTxt[64];  // TCHAR szPrtVTxt[]  = "V_Text";         // nPrtVTxt
   //SIZE        ps_ssFA;    // string size szPrtHTS[] = "H_Text_Size"; // nPrtHTS
   //SIZE        ps_ssF2;    // string size szPrtVTS[] = "V_Text_Size"; // nPrtVTS
   RDPPSS2(ps_szBotTxt, szPrtHTxt, nPrtHTxt, 64);
   RDPPSS2(ps_szRitTxt, szPrtVTxt, nPrtVTxt, 64);
   RDPPSSZ2(ps_ssFA, szPrtHTS, nPrtHTS);
   RDPPSSZ2(ps_ssF2, szPrtVTS, nPrtVTS);

}

#define  WRTPPS2(a,b,c)\
   if( ( nPrtChg[nPrtALL] ) || ( nPrtChg[c] ) ) \
   {\
      wsprintf(lpb,"%d", pps->a);\
      WritePrivateProfileString( lps, b, lpb, lpIni );\
      nPrtChg[c] = 0;\
   }

#define  WRTPPSS2(a,b,c)\
   if( ( nPrtChg[nPrtALL] ) || ( nPrtChg[c] ) ) \
   {\
      wsprintf(lpb,"%s", pps->a);\
      WritePrivateProfileString( lps, b, lpb, lpIni );\
      nPrtChg[c] = 0;\
   }

#define  WRTPPSR2(a,b,c)\
   if( ( nPrtChg[nPrtALL] ) || ( nPrtChg[c] ) ) \
   {\
      wsprintf(lpb,"%ld,%ld,%ld,%ld", pps->a.left, pps->a.top, pps->a.right, pps->a.bottom);\
	   WritePrivateProfileString( lps, b, lpb, lpIni );\
      nPrtChg[c] = 0;\
   }

#define  WRTPPSSZ2(a,b,c)\
   if( ( nPrtChg[nPrtALL] ) || ( nPrtChg[c] ) ) \
   {\
      wsprintf(lpb,"%ld,%ld", pps->a.cx, pps->a.cy);\
	   WritePrivateProfileString( lps, b, lpb, lpIni );\
      nPrtChg[c] = 0;\
   }

#define  WRTPPSB2(a,b,c)\
   if( ( nPrtChg[nPrtALL] ) || ( nPrtChg[c] ) ) \
   {\
      wsprintf(lpb,"%s", (pps->a ? "Yes" : "No") );\
	   WritePrivateProfileString( lps, b, lpb, lpIni );\
   }


VOID  WritePrtIni( LPTSTR lpIni, BOOL fChgAll )
{
   PPS pps = (PPS)&gszCGenBuf2[0];
   //LPTSTR   lpIni = &gszIniFile[0];
   LPTSTR   lps = &szPrt[0];  //"Printer";  // [Section] header
   LPTSTR   lpb = GetTmp2();	// &Buf[0];

   if( fChgAll )
   {
      nPrtChg[nPrtALL] = TRUE;   // set the ALL flag
      // clear ALL the items
      WritePrivateProfileString( lps,		// Section
         NULL,    // Res.Words
         NULL,    // String to write
         lpIni );	// File Name
   }

   // lstrcpy( &gszDefPrtr[0], &pdm->dmDeviceName[0] );
   WRTPPSS2(ps_szDefPrtr, szPrtDef, nPrtDef);

   // TCHAR szPrtTop[]   = "PrtTop";   // ps_dwTop, nPrtTop
   // TCHAR szPrtLeft[]  = "PrtLeft";  // ps_dwLeft, nPrtLeft
   WRTPPS2(ps_dwTop,   szPrtTop,  nPrtTop );
   WRTPPS2(ps_dwLeft,  szPrtLeft, nPrtLeft);
   WRTPPS2(ps_dwPCWid, szPrtXPc,  nPrtPCW ); // percentage width
   WRTPPS2(ps_dwPCHit, szPrtYPc,  nPrtPCH ); // percentage height
   WRTPPS2(ps_dwWidth,  szPrtWid,  nPrtWid ); // percentage width
   WRTPPS2(ps_dwHeight, szPrtHit,  nPrtHit ); // percentage height
   WRTPPS2(ps_dwCopies, szPrtCop,  nPrtCop ); // copies to be printed

   WRTPPS2(ps_xPerInch, szPrtxPI, nPrtxPI);   // = GetDeviceCaps( hdcTarget, LOGPIXELSX);
   WRTPPS2(ps_yPerInch, szPrtyPI, nPrtyPI);   // = GetDeviceCaps( hdcTarget, LOGPIXELSY);

   // like FORMATRANGE
   WRTPPSR2(ps_rcArea,  szPrtArea, nPrtArea );     // (RECT) Area (in Device Units)
   WRTPPSR2(ps_rcTPage, szPrtTPg,  nPrtTPg  );     // Entire area of rendering device. Units in twips. 
   WRTPPSR2(ps_rcDPage, szPrtDPg,  nPrtDPg  );     // Entire area of rendering device. In Device Units
   WRTPPSR2(ps_rcBPage, szPrtBPg,  nPrtBPg  );     // Print area less border (in pixels)
   WRTPPSR2(ps_rcRend,  szPrtRend, nPrtRend );
   WRTPPSR2(ps_rcPrt,   szPrtPrt,  nPrtPrt  );
   //TCHAR szPrtPPage[] = "DlgPaintPage";   // ps_rcPPage, nPrtPPg - dialog paint page size
   //TCHAR szPrtDlgFrm[]= "DlgFrameRect";   // ps_rcDlgFrm, nPrtFrm
   WRTPPSR2(ps_rcPPage,  szPrtPPage,  nPrtPPg );
   WRTPPSR2(ps_rcDlgFrm, szPrtDlgFrm, nPrtFrm );
   WRTPPSR2(ps_rcPFrame, szPrtDlgPnt, nPrtPRc );

   WRTPPS2(ps_dxPage, szPrtxTPP, nPrtxTPP   );
   WRTPPS2(ps_dyPage, szPrtyTPP, nPrtyTPP   );
   WRTPPS2(ps_dxRes,  szPrtHRes, nPrtHRes   );   // = GetDeviceCaps(ppp->fr.hdcTarget, HORZRES);
   WRTPPS2(ps_dyRes,  szPrtVRes, nPrtVRes   );   // = GetDeviceCaps(ppp->fr.hdcTarget, VERTRES);
   WRTPPS2(ps_dxSize, szPrtHSiz, nPrtHSiz   );  // = GetDeviceCaps( hdcTarget, HORZSIZE  );   // in millimeters
   WRTPPS2(ps_dySize, szPrtVSiz, nPrtVSiz   );  //   = GetDeviceCaps( hdcTarget, VERTSIZE  );

   WRTPPS2(ps_PhyWidth, szPrtHPhy, nPrtHPhy );   // PHYSICALWIDTH - the width in device units.
//   // eg at 600 dpi on 8.5x11 in. paper has width of 5100 device units.
   WRTPPS2(ps_PhyHeight, szPrtVPhy, nPrtVPhy);  // PHYSICALHEIGHT - the height in device units.
//   // eg at 600 dpi on 8.5x11 in. paper has height value of 6600 device units.
   WRTPPS2(ps_PhyOffX,   szPrtPOffH, nPrtPOffH);    // PHYSICALOFFSETX -  distance from left in device units.
//   // eg at 600 dpi on 8.5x11 in. paper, that cannot print on the leftmost 0.25-inch
//   // has a horizontal physical offset of 150 device units. 
   WRTPPS2(ps_PhyOffY,   szPrtPOffV, nPrtPOffV);    // PHYSICALOFFSETY the distance from the top in device units.

   // write some binary values
   WRTPPSB2(ps_bDistort, szPrtDist, nPrtDist);    // allow distortion
   WRTPPSB2(ps_bFitPage, szPrtFit,  nPrtFit );   // fit image into page
   WRTPPSB2(ps_bAddText,  szPrtTxt,   nPrtTxt  );   // add dimension text
   WRTPPSB2(ps_bAddText2, szPrtTxt2,  nPrtTxt2 );   // general text
   WRTPPSB2(ps_bAddText3, szPrtTxt3,  nPrtTxt3 );   // border text

   WRTPPS2(ps_dwMFHit,   szPrtFHit, nPrtFHit); // "TextHeightx2";
   WRTPPS2(ps_dwMFWid,   szPrtFWid, nPrtFWid); // "TextWidthx2";

   //TCHAR       ps_szBotTxt[64];  // TCHAR szPrtHTxt[]  = "H_Text";         // nPrtHTxt
   //TCHAR       ps_szRitTxt[64];  // TCHAR szPrtVTxt[]  = "V_Text";         // nPrtVTxt
   //SIZE        ps_ssFA;    // string size szPrtHTS[] = "H_Text_Size"; // nPrtHTS
   //SIZE        ps_ssF2;    // string size szPrtVTS[] = "V_Text_Size"; // nPrtVTS
   WRTPPSS2(ps_szBotTxt, szPrtHTxt, nPrtHTxt);
   WRTPPSS2(ps_szRitTxt, szPrtVTxt, nPrtVTxt);
   WRTPPSSZ2(ps_ssFA, szPrtHTS, nPrtHTS);
   WRTPPSSZ2(ps_ssF2, szPrtVTS, nPrtVTS);

   nPrtChg[nPrtALL] = FALSE;   // reset the ALL flag

}

VOID  ChkDefPrtr( VOID )
{
   PPS pps = (PPS)&gszCGenBuf2[0];
   if( pszDefPrtr[0] == 0 )
   {
      lstrcpy( &pszDefPrtr[0], &gszDefPrtr[0] );
      nPrtChg[nPrtDef] = TRUE;
   }
}

//  pps->ps_xPerInch = GetDeviceCaps( hdcTarget, LOGPIXELSX); // pixels per inch 
#define  GDC(a,b,c)\
   idc = GetDeviceCaps( hdcTarget, b);\
   if( pps->a != idc )\
   {\
      pps->a = idc;\
      nPrtChg[c] = TRUE;\
      iChg++;\
   }

int  GetHdcInfo( PPS pps, HDC hdcTarget, LPTSTR lppn )
{
   LPRECT      prcDPage = &pps->ps_rcDPage;
   LPRECT      prcTPage = &pps->ps_rcTPage;
   LPRECT      prcBPage = &pps->ps_rcBPage;
   PLOGFONT    plf      = &pps->ps_sLogFont;
   POINT       pt;
   int         idc, idx, idy;
   int         iChg = 0;
   RECT        rc;   // work rectangle

   if( pps && hdcTarget )
   {
      //	SetMapMode(hdcTarget, MM_TEXT);  // is this required?
      //pps->ps_xPerInch = GetDeviceCaps( hdcTarget, LOGPIXELSX); // pixels per inch 
      //pps->ps_yPerInch = GetDeviceCaps( hdcTarget, LOGPIXELSY);
      // TCHAR szPrtxPI[]   = "XPixelPerInch";
      // TCHAR szPrtyPI[]   = "YPixelPerInch";
      GDC(ps_xPerInch, LOGPIXELSX, nPrtxPI);
      GDC(ps_yPerInch, LOGPIXELSY, nPrtyPI);

      // **************************************************************************
      //pps->ps_dxRes    = GetDeviceCaps( hdcTarget, HORZRES   ); // in Pixels
      //pps->ps_dyRes    = GetDeviceCaps( hdcTarget, VERTRES   );
      // TCHAR szPrtHRes[]  = "HorizRes";
      // TCHAR szPrtVRes[]  = "VertRes";
      GDC(ps_dxRes, HORZRES, nPrtHRes); // in Pixels
      GDC(ps_dyRes, VERTRES, nPrtVRes);
      // **************************************************************************

      //pps->ps_dxSize   = GetDeviceCaps( hdcTarget, HORZSIZE  ); // in millimeters
      //pps->ps_dySize   = GetDeviceCaps( hdcTarget, VERTSIZE  );
      // TCHAR szPrtHSiz[]  = "HorizSizemm";
      // TCHAR szPrtVSiz[]  = "VertiSizemm";
      GDC(ps_dxSize, HORZSIZE, nPrtHSiz); // in millimeters
      GDC(ps_dySize, VERTSIZE, nPrtVSiz);

      //pps->ps_PhyWidth = GetDeviceCaps( hdcTarget, PHYSICALWIDTH);
      // eg at 600 dpi on 8.5x11 in. paper has width of 5100 device units.
      //pps->ps_PhyHeight= GetDeviceCaps( hdcTarget, PHYSICALHEIGHT);
      // eg at 600 dpi on 8.5x11 in. paper has height value of 6600 device units.
      // TCHAR szPrtHPhy[]  = "HPhySizeDU";        // nPrtHPhy - ps_PhyWidth, szPrtHPhy PHYSICALWIDTH - the width in device units.
      // TCHAR szPrtVPhy[]  = "VPhySizeDU";        // nPrtVPhy
      GDC(ps_PhyWidth,  PHYSICALWIDTH,  nPrtHPhy);
      GDC(ps_PhyHeight, PHYSICALHEIGHT, nPrtVPhy);

      //pps->ps_PhyOffX  = GetDeviceCaps( hdcTarget, PHYSICALOFFSETX);
      // eg 600 dpi on 8.5x11 in. paper, leftmost 0.25-inch = 150 device units.
      //pps->ps_PhyOffY  = GetDeviceCaps( hdcTarget, PHYSICALOFFSETY);
      // TCHAR szPrtPOffH[] = "HPhyOffset";        // nPrtPOffH - ps_PhyOffX
      // TCHAR szPrtPOffV[] = "VPhyOffset";
      GDC(ps_PhyOffX, PHYSICALOFFSETX, nPrtPOffH);
      GDC(ps_PhyOffY, PHYSICALOFFSETY, nPrtPOffV);

      // RDPPSR(ps_rcDPage, szPrtDPg );     // Entire area of print device. In DU or Px
      prcDPage->left   = prcDPage->top = 0;
#ifdef   USEDRES  // use device resolution (in pxels), NOT physical size in DU
      if( prcDPage->right  != pps->ps_dxRes )  // = GetDeviceCaps( hdcTarget, HORZRES);
      {
         prcDPage->right  = pps->ps_dxRes;
         nPrtChg[nPrtDPg] = TRUE;
         iChg++;
      }
      if( prcDPage->bottom != pps->ps_dyRes ) //= GetDeviceCaps( hdcTarget, VERTRES);
      {
         prcDPage->bottom = pps->ps_dyRes;
         nPrtChg[nPrtDPg] = TRUE;
         iChg++;
      }
#else // !USEDRES
      if( prcDPage->right  != pps->ps_PhyWidth )  // = GetDeviceCaps( hdcTarget, PHYSICALWIDTH);
      {
         prcDPage->right  = pps->ps_PhyWidth;
         nPrtChg[nPrtDPg] = TRUE;
         iChg++;
      }
      if( prcDPage->bottom != pps->ps_PhyHeight ) //= GetDeviceCaps( hdcTarget, PHYSICALHEIGHT);
      {
         prcDPage->bottom = pps->ps_PhyHeight;
         nPrtChg[nPrtDPg] = TRUE;
         iChg++;
      }
#endif   // USEDRES y/n

      //GDC(ps_dxRes, HORZRES, nPrtHRes); // in Pixels
      //GDC(ps_dyRes, VERTRES, nPrtVRes);
      rc.left = rc.top = 0;
      idx = pps->ps_PhyWidth  - pps->ps_dxRes;
      idy = pps->ps_PhyHeight - pps->ps_dyRes;
      if( idx > 1 )
         rc.left = idx / 2;
      if( idy > 1 )
         rc.top = idy / 2;
      rc.right  = pps->ps_dxRes - rc.left;
      rc.bottom = pps->ps_dyRes - rc.top;
      //rc.right  = rc.left + pps->ps_dxRes;
      //rc.bottom = rc.top  + pps->ps_dyRes;
      //rc.right  = pps->ps_dxRes;
      //rc.bottom = pps->ps_dyRes;
      if( EqualRect( prcBPage, &rc ) == 0 )
      {
         *prcBPage        = rc;   // copy the PRINT rectangle minus BORDERS
         nPrtChg[nPrtBPg] = TRUE;
         iChg++;
      }

      // int Escape(    // this is SENT to the DEVICE
      //  HDC hdc,           // handle to DC
      //  int nEscape,       // escape function
      //  int cbInput,       // size of input structure
      //  LPCSTR lpvInData,  // input structure
      //  LPVOID lpvOutData ); // output structure
      // If the function succeeds, the return value is greater than zero,
      //    except with the QUERYESCSUPPORT 
      if( Escape( hdcTarget, GETPHYSPAGESIZE, 0, NULL, &pt ) > 0 )
      {
         //pps->ps_rcArea.left = pps->rcArea.top = 0;
         //		ppp->dxPage = (pt.x * 1440l) / xPerInch;
         //pps->ps_dxPage = (pt.x * MCTPI ) / pps->ps_xPerInch;
         //pps->ps_dyPage = (pt.y * MCTPI ) / pps->ps_yPerInch;
         // // calculated TWIPS
         // TCHAR szPrtxTPP[]  = "XTwipsPerPage";     // nPrtxTPP = ps_dxPage
         // TCHAR szPrtyTPP[]  = "YTwipsPerPage";     // nPrtyTPP = ps_dyPage
         //idx = (pt.x * MCTPI ) / pps->ps_xPerInch;
         //idy = (pt.y * MCTPI ) / pps->ps_yPerInch;
         // leave 1.25" (1800 twips) margins if that will leave >= 1"
         //if( pps->ps_rcArea.right >= 1800 + MCTPI + 1800 )
         //   pps->ps_rcArea.right -= (pps->ps_rcArea.left = 1800);
         // leave 1" (1440 twips) margins if that will leave >= 1"
         //if( pps->ps_rcArea.bottom >= 1440 + MCTPI + 1440 )
         //   pps->ps_rcArea.bottom -= (pps->ps_rcArea.top = 1440);

      }
      else
      {
         //ppp->rc.left = ppp->rc.top = 0;
         //ppp->dxPage = (dxRes * 1440l) / xPerInch;
         //pps->ps_dxPage = (pps->ps_dxRes * MCTPI) / pps->ps_xPerInch;
         //pps->ps_dyPage = (pps->ps_dyRes * MCTPI) / pps->ps_yPerInch;
         pt.x = pps->ps_dxRes;
         pt.y = pps->ps_dyRes;
         //idx = (pps->ps_dxRes * MCTPI) / pps->ps_xPerInch;
         //idy = (pps->ps_dyRes * MCTPI) / pps->ps_yPerInch;
      }

      // TCHAR szPrtArea[]  = "Area"; // ps_rcArea,  nPrtArea (RECT) Area in DU
      rc.left   = rc.top = 0;
      rc.right  = pps->ps_PhyWidth;
      rc.bottom = pps->ps_PhyHeight;
      if( EqualRect( &pps->ps_rcArea, &rc ) == 0 )
      {
         pps->ps_rcArea    = rc;
         nPrtChg[nPrtArea] = TRUE;
         iChg++;
      }

      idx = (int)(((double)pt.x * (double)MCTPI ) / (double)pps->ps_xPerInch);
      idy = (int)(((double)pt.y * (double)MCTPI ) / (double)pps->ps_yPerInch);
      if( pps->ps_dxPage != idx )
      {
         pps->ps_dxPage    = idx;   // = XTwipsPerPage
         nPrtChg[nPrtxTPP] = TRUE;
         iChg++;
      }
      if( pps->ps_dyPage != idy )
      {
         pps->ps_dyPage    = idy;   // = ((pt.y|ps_dyRes) * MCTPI ) / pps->ps_yPerInch;
         nPrtChg[nPrtyTPP] = TRUE;
         iChg++;
      }


      // TCHAR szPrtTPg[]   = "PageInTwips"; //    ps_rcTPage, szPrtTPg, nPrtTPg
      // EqualRect(pr1, pr2) - If the two rectangles are not identical,
      // the return value is zero.
      rc.left   = rc.top = 0;
      rc.right  = idx;
      rc.bottom = idy;
      //if( EqualRect( prcTPage, &pps->ps_rcArea ) == 0 )
      if( EqualRect( prcTPage, &rc ) == 0 )
      {
         //*prcTPage        = pps->ps_rcArea;   // copy the Area to the TWIPS rectangle
         *prcTPage = rc;
         nPrtChg[nPrtTPg] = TRUE;
         iChg++;
      }

      if( pps->ps_hPFontA )
         DeleteObject( pps->ps_hPFontA );
      pps->ps_hPFontA = 0;
      if( pps->ps_hPFontV )
         DeleteObject( pps->ps_hPFontV );
      pps->ps_hPFontV = 0;

      {

         ZeroMemory( plf, sizeof(LOGFONT) );
         plf->lfHeight = -MulDiv( 12, GetDeviceCaps(hdcTarget, LOGPIXELSY), 72 );
         lstrcpy( &plf->lfFaceName[0], "Courier New" );
         plf->lfWeight = FW_BOLD;   // set BOLD font
         pps->ps_hPFontA = CreateFontIndirect( plf );
         plf->lfEscapement = 2700;
         pps->ps_hPFontV = CreateFontIndirect( plf );

      }

      if( iChg )
      {
         pps->ps_bChgPrt = TRUE;    // do calculation again in Render_DIB()
         GetPntPage( &rc, &pps->ps_rcPFrame, prcDPage, TRUE ); // this appears OK
         if( EqualRect( &pps->ps_rcPPage, &rc ) == 0 )
         {
            pps->ps_rcPPage = rc; // update PAINT PAGE of DIALOG
         }
      }

   }

#ifndef  NDEBUG
   ShowDBGSizes( pps, lppn );
#endif   // #ifndef  NDEBUG

   return iChg;
}

BOOL  GetDevInfo( PPS pps, LPDEVNAMES pdn, PDEVMODE pdm )
{
   BOOL        bRet = FALSE;
   HDC         hdcTarget;
   LPTSTR      lpdn;

   lpdn = (LPTSTR)pdn;
   if( hdcTarget = CreateIC( lpdn + pdn->wDriverOffset,  // driver name
                  lpdn + pdn->wDeviceOffset,             // device name
                  lpdn + pdn->wOutputOffset,             // port or file name
                  pdm ) )     // optional initialization data
   {
      GetHdcInfo( pps, hdcTarget, (lpdn + pdn->wDeviceOffset) );
      DeleteDC( hdcTarget );  // handle to DC
      bRet = TRUE;
   }
   else
   {
      sprtf( MEOR"WARNING: Failed to get HDC for target printer!!!"MEOR );
   }

   return bRet;
}


#ifndef  NDEBUG
// ONLY FOR DEBUG INFORMATION
VOID  ShowDBGSizes( PPS pps, LPTSTR lppn )
{
   double   dbw, dbh;
   int      iChg;

   dbw = (double)pps->ps_dxSize / 25.4;
   dbh = (double)pps->ps_dySize / 25.4;

   if( lppn && *lppn )
      sprtf( "Details: From HDC of printer [%s]."MEOR, lppn );

   iChg = 0;
   //RDPPS(ps_dxSize, szPrtHSiz);  // = GetDeviceCaps( hdcTarget, HORZSIZE  );   // in millimeters
   //RDPPS(ps_dySize, szPrtVSiz);  //   = GetDeviceCaps( hdcTarget, VERTSIZE  );
   if( nPrtChg[nPrtHSiz] )
      iChg++;
   if( nPrtChg[nPrtVSiz] )
      iChg++;
   //TCHAR szPrtHPhy[]  = "HPhySizeDU";        // nPrtHPhy - ps_PhyWidth, szPrtHPhy PHYSICALWIDTH - the width in device units.
   //TCHAR szPrtVPhy[]  = "VPhySizeDU";        // nPrtVPhy
   if( nPrtChg[nPrtHPhy] )
      iChg++;
   if( nPrtChg[nPrtVPhy] )
      iChg++;

   sprtf( "Printer is %dx%d mm. %dx%d in dev.units. (Apx %sx%s ins.)[C=%d]"MEOR,
      pps->ps_dxSize,   // in millimeters
      pps->ps_dySize, // GetDeviceCaps( hdcTarget, VERTSIZE  );
      pps->ps_PhyWidth, // = GetDeviceCaps( hdcTarget, PHYSICALWIDTH);
      pps->ps_PhyHeight,   //= GetDeviceCaps( hdcTarget, PHYSICALHEIGHT);
      Dbl2Str( dbw, 3 ),
      Dbl2Str( dbh, 3 ),
      iChg );

   iChg = 0;
   // TCHAR szPrtDPg[]   = "PageInDU";    //    ps_rcDPage, szPrtDPg, nPrtDPg
   // TCHAR szPrtTPg[]   = "PageInTwips"; //    ps_rcTPage, szPrtTPg, nPrtTPg
   if( nPrtChg[nPrtDPg] )
      iChg++;
   if( nPrtChg[nPrtTPg] )
      iChg++;
   if( nPrtChg[nPrtBPg] )
      iChg++;

   // NOTE: Rect2Stg( &pps->ps_rcArea ) - THIS IS THE SAME AS ps_rcTPage
   sprtf( "Areas: DPage %s (DU). BPage %s (Px)"MEOR
      "\tTPage %s = Area %s (TWIPS) [C=%d]"MEOR,
      Rect2Stg( &pps->ps_rcDPage ),
      Rect2Stg( &pps->ps_rcBPage ),
      Rect2Stg( &pps->ps_rcTPage ),
      Rect2Stg( &pps->ps_rcArea  ),
      iChg );

   dbw = (double)pps->ps_dxRes / (double)pps->ps_xPerInch;
   dbh = (double)pps->ps_dyRes / (double)pps->ps_yPerInch;
   iChg = 0;
   // RDPPS(ps_dxRes,  szPrtHRes);   // = GetDeviceCaps(ppp->fr.hdcTarget, HORZRES);
   // RDPPS(ps_dyRes,  szPrtVRes);   // = GetDeviceCaps(ppp->fr.hdcTarget, VERTRES);
   if( nPrtChg[nPrtHRes] )
      iChg++;
   if( nPrtChg[nPrtVRes] )
      iChg++;
   // RDPPS2(ps_xPerInch, szPrtxPI, nPrtxPI);   // = GetDeviceCaps( hdcTarget, LOGPIXELSX);
   // RDPPS2(ps_yPerInch, szPrtyPI, nPrtyPI);   // = GetDeviceCaps( hdcTarget, LOGPIXELSY);
   if( nPrtChg[nPrtxPI] )
      iChg++;
   if( nPrtChg[nPrtyPI] )
      iChg++;
   // TCHAR szPrtPOffH[] = "HPhyOffset";        // nPrtPOffH - ps_PhyOffX
   // TCHAR szPrtPOffV[] = "VPhyOffset";        // nPrtPOffV - ps_PhyOffY
   if( nPrtChg[nPrtPOffH] )
      iChg++;
   if( nPrtChg[nPrtPOffV] )
      iChg++;

   sprtf( "Res.of %dx%d px, at %dx%d px/inch. (Off=%dx%d) (Apx %sx%s ins.) [C=%d]"MEOR,
      pps->ps_dxRes, // = GetDeviceCaps( hdcTarget, HORZRES   ); // in Pixels
      pps->ps_dyRes, // = GetDeviceCaps( hdcTarget, VERTRES   );
      pps->ps_xPerInch, // = GetDeviceCaps( hdcTarget, LOGPIXELSX); // pixels per inch
      pps->ps_yPerInch, // = GetDeviceCaps( hdcTarget, LOGPIXELSY);
      pps->ps_PhyOffX,  //  = GetDeviceCaps( hdcTarget, PHYSICALOFFSETX);
      pps->ps_PhyOffY,  // = GetDeviceCaps( hdcTarget, PHYSICALOFFSETY);
      Dbl2Str( dbw, 3 ),
      Dbl2Str( dbh, 3 ),
      iChg );

   {
      static BOOL bDnSSize = FALSE;
      if( !bDnSSize )
      {
         sprtf( "PRTSTR (PPS) take %d bytes of the gszCGenBuf2 of %d (Bal=%d)."MEOR,
            sizeof(PRTSTR), sizeof( W.w_szCGenBuf2 ),
            (sizeof( W.w_szCGenBuf2 ) - sizeof(PRTSTR)) );
         bDnSSize = TRUE;
      }
   }

}

#endif   // #ifndef  NDEBUG

VOID     UpdateRendRect( PPS pps, LPRECT prcRend, LPRECT prcPrt )
{
         if( EqualRect( &pps->ps_rcRend, prcRend ) == 0 )
         {
            //TCHAR szPrtRend[]  = "RenderedAs";  // ps_rcRend, nPrtRend
            pps->ps_rcRend = *prcRend;
            nPrtChg[nPrtRend] = TRUE;
         }

         if( EqualRect( &pps->ps_rcPrt, prcPrt ) == 0 )
         {
            //TCHAR szPrtRend[]  = "PrintedAs";  // ps_rcPrt, nPrtPrt
            pps->ps_rcPrt = *prcPrt;
            nPrtChg[nPrtPrt] = TRUE;
         }
}

// EOF - DvPrint.c
