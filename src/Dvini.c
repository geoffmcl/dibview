
// =========================================================
//
// DvIni.c
//
// Read and Write INI file ...
//
// =========================================================
#include <sys/types.h>
#include <sys/stat.h>
#include "dv.h"	// All inclusive include ...
#include <tchar.h>
#include <direct.h> // for mkdir

#ifdef UNICODE
#define m_atoi (int)_wtof
#define m_sscanf  swscanf
#else
#define m_atoi atoi
#define m_sscanf  sscanf
#endif

#ifdef   NDEBUG
#define  DEF_VERB_LEVEL 1
#else
#define  DEF_VERB_LEVEL 9
#endif
#ifndef CHKMEM2
#define  CHKMEM2(a,b)   if(!a) { chkme( "C:ERROR: MEMORY FAILED! On %d bytes"MEOR, b ); exit(-1); }
#endif

//#define  dbgr_sprtf  sprtf
#define  dbgr_sprtf

// ===============================================================
// wscanf( lpData, lpFormat, lpList )
// A replacement of sscanf(...) which appears missing in WINDOWS
// Presently ONLY handles %d, %ld, %c ..
// ===============================================================
// WE ONLY HAVE ***ONE***
// See GwScanf.h
//extern	int _gwscanf( LPSTR lpd, LPSTR lpf, void MLPTR lpv );
//#define	dw_wscanf( a, b, c )	_gwscanf( a, b, c )

#undef	DIAGFILE3

// external
//extern	char	szSearchDir[MXFILNAM+2];
#ifdef	DIAGSAVEHOOK
extern	void	SRInitDiag( LPSTR );
#endif
extern	WORD	gwOutIndex;
extern	WORD	InIndex;
extern	BOOL	fChgOutInd;
extern	BOOL	fChgInInd;
extern   BOOL  gbHide;   // TRUE if MAIN window is to be hidden on capture
extern   BOOL  gbChgHide;
extern	BOOL	fReLoad;
extern	BOOL	fChgRL;
#ifdef	TIMEDPUT
extern	BOOL	fAddTime;
extern	DWORD	wBitsPSec;	/* Default to 8,000 bits per second */
extern	BOOL	fChgBS;
extern	BOOL	fChgAT;
#endif	/* TIMEDPUT */

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
extern	BOOL	fNoFErr;
extern	BOOL	fChgFE;
extern	void	WriteIniJPEG(LPSTR, BOOL);
extern	BOOL	ReadIniJPEG(LPSTR, LPLIBCONFIG);
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
//#ifdef   USELLIST
extern	BOOL	AddMenuNames(PLIST_ENTRY pHead);
//extern	DWORD	dwMaxFiles;	/* Remember the last ?? files - See Dib.h */
//extern	BOOL	fAutoLoad;
//extern	BOOL	fChgAL;
//extern	BOOL	fChgSC;
//extern	void	DVGetFPath( LPSTR, LPSTR, LPSTR, LPSTR, LPSTR );
extern	BOOL	fShwNoConv;
//#ifdef   USELLIST
extern   DWORD SetFileMRU( HWND hwnd, PLIST_ENTRY pHead );
extern   VOID  WritePrtIni( LPTSTR lpIni, BOOL fChgAll );
extern   VOID  ReadPrtIni( LPTSTR lpIni, BOOL fChgAll );

//extern	HWND	hFrameWnd;

// DvWarn.c
//extern	BOOL	bShowWarn;
//extern	BOOL	bChgWarn;

// GLOBAL
void	ReadIni( LPSTR );
void	WriteIni( HWND );
void	AddToHistList( LPSTR lpf );
PMWL  Add2LListTail( PLIST_ENTRY pHead, LPTSTR lpb ); // add to END OF LIST
PMWL  Add2LListHead( PLIST_ENTRY pHead, LPTSTR lpb ); // insert as FIRST OF LIST

//HGLOBAL	hFileList = 0;		// Global MEMORY handle for FILES
//WORD	wFilCnt = 0;	/* Count of files in File List ... */
DWORD	gdwfFlag;
BOOL	fInAutoLoad = FALSE;

RECT	TmpRec[2];

char gszAppData[260] = "\0";
char gszCacheData[260] = "\0";
// Default file name
char	szIniDef[] = "dibview.ini";   // was szIniName[]
// FIX20200602: DibView.ini should be in %APPDATA%\DibView/
// FIX20080316 - Use RUNTIME location, and above Default file = NO

char	szBlank[] = "";

//BOOL	fChgAll = FALSE;
//BOOL	fChgOpen = FALSE;
//BOOL	fChgSave = FALSE;

/* [DIBVIEW32] - WIN.INI Section */
char	szDB32[] = "DIBVIEW32";
//char szVers[] = "Version" ;	// See entry below
char	szIFile[] = "INIFile";

/* [Section Headers] */
char szVers[] = "Version" ;
char szWind[] = "Window" ;

/* [Version] Section */
char szVDate[] = "Date" ;
//char szVDDef[] = VER_DATE;
extern	char	szVerDate[];	// = VER_DATE;

/* [Window] Section */
#ifdef   USENEWWINSIZE
TCHAR szGOut[] = "OutSaved";
BOOL  gbGotWP = FALSE;
WINDOWPLACEMENT   g_sWP;
BOOL  bChgWP = FALSE;
// TCHAR szWin[] = L"Window";
TCHAR szShow[] = _T("ShowCmd");
TCHAR szMaxX[] = _T("MaxX");
TCHAR szMaxY[] = _T("MaxY");
TCHAR szMinX[] = _T("MinX");
TCHAR szMinY[] = _T("MinY");
TCHAR szLeft[] = _T("NormLeft");
TCHAR szTop[]  = _T("NormTop");
TCHAR szRite[] = _T("NormRight");
TCHAR szBot[]  = _T("NormBottom");

#else // !#ifdef   USENEWWINSIZE
char szSize[] = "Current Size";
char szMaxScn[] = "Max. Screen";
char szIsZmd[] = "Zoomed";
BOOL	fChgSiz = FALSE;
RECT	rIniSiz;
// GetWindowRect( GetDesktopWindow(), &rMaxSiz );
RECT	rMaxSiz;	// Max. SIZE of DESKTOP Window
BOOL	fChgMax = FALSE;
BOOL	bIsZoomed = FALSE;
BOOL	bChgZoomed = FALSE;

#endif // #ifdef   USENEWWINSIZE y/n

char szSz4[] = "%d,%d,%d,%d";
char szSz4l[] = "%ld,%ld,%ld,%ld";

//extern   BOOL  gbHide;   // TRUE if MAIN window is to be hidden on capture
//extern   BOOL  gbChgHide;
TCHAR szHide[] = "HideOnCapture";

/* [Directory] Section */
char	szDirs[] = "Directory";
char	szRDir[] = "File Open";
char	szWDir[] = "File Save";
char	szOInd[] = "Out Index";
char	szIInd[] = "In Index";
char	szSz1[]  = "%d";
char	szSz2[]  = "%lu";
//char	szSrch[] = "Search Path";
char	szRecur[]= "Recursive";

char	szYes[] = "Yes";
char	szOn[] = "On";
char	szNo[] = "No";
char	szOff[]= "Off";

/* [General] Section */
TCHAR szGens[]       = "General";
char	szBeTidy[]     = "Be Tidy";
char	szSaveI[]      = "Save INI";
TCHAR	szReLoad[]     = "ReloadOnSave";
char	szTimed[]      = "Timer";	/* On or OFF */
char	szRate[]       = "Rate(bps)";
char	szFErr[]       = "SysLoadMsg";
char	szMaxCols[]    = "MaxColors";
TCHAR	szAutoLoad[]   = "AutoLoadAllMRU";	// Auto reload all MRU files Yes/No
TCHAR szAutoRLLast[] = "AutoReLoadLast";  // Auto reload last loaded

TCHAR szMaxFiles[]   = "Max_File_Count";	   // gdwMaxFiles - Save Count
TCHAR szHisFiles[]   = "Max_History_Count";	// History Count
TCHAR szFndFiles[]   = "Max_Find_Count";	   // Find Count
TCHAR szMskFiles[]   = "Max_Mask_Count";     // Mask count

char	szOpenWindow[] = "OpenWindow";	//
TCHAR szNextPaste[]  = "NextPaste"; // gnPasteNum & gbChgPaste
char szVerbosity[] = "Verbosity";   // FIX20080316 with diag file always on, add
int   iVerbosity = DEF_VERB_LEVEL;
BOOL  bChgVerb = FALSE;

/* [Files] Section */
char	szFiles[] = "Files";
char	szFileN[] = "File%d";
// [History] Section - Keep the last ?? deleted
char	szHist[] = "History";
char	szHFileN[] = "HFile%d";

/* [Finds] Section */
char	szFinds[] = "Finds";
char	szFindN[] = "Find%d";

/* [Mask] Section */
char	szMasks[] = "Masks";
char	szMaskN[] = "Mask%d";

/* [AutoReload] Section */
TCHAR	szAutos[] = "AutoReload";
TCHAR	szAutoN[] = "Autos%d";

// [Options] Section
char szOptions[] = "Options";
char szStretch[] = "Stretch";

//#ifdef  WIN32
TCHAR	szPrtCenter[] = "PrintCenter";
//#else // !WIN32
#ifndef  WIN32
// ============================================================
//A Win32-based application should not call the
//NEXTBAND and BANDINFO escapes. Banding is no longer needed in
//Windows 95++.
char	szPrinterBand[] = "BandPrinter";
//#define	gfPrinterBand	WrkStr.w_bPrinterBand
//#define	gfChgPrinterBand WrkStr.w_bChgPrinterBand
char	szUse32API[] = "Use31API";
//#define	gfUse31PrintAPIs WrkStr.w_bUse31PrintAPIs
//#define	gfChgPrintAPIs	WrkStr.w_bChgPrintAPIs
// ============================================================
#endif   // !WIN32

char	szDispType[] = "DispType";
//#define	gwDispOption	WrkStr.w_wDispOption
//#define	gfChgDispOption	WrkStr.w_bChgDispOption
char	szPrintType[] = "PrintType";
//#define	gwPrintOption	WrkStr.w_wPrintOption
//#define	gfChgPrintOption WrkStr.w_bChgPrintOption
char	szXScale[] = "XScale";
//#define	gwXScale		WrkStr.w_wXScale
//#define	gfChgXScale		WrkStr.w_bChgXScale
char	szYScale[] = "YScale";
//#define	gwYScale		WrkStr.w_wYScale
//#define	gfChgYScale		WrkStr.w_bChgYScale
char	szMilSecs[] = "msTimer";
//#define	gdwMilSecs		WrkStr.w_dwMilSecs
//#define	gfChgMilSecs	WrkStr.w_bChgMilSecs
char	szSetDef[] = "SetDefaults";
//#define	gfSetDefault	WrkStr.w_bSetDefault
//#define	gfChgSetDefault	WrkStr.w_bChgSetDefault
char	szApplyAll[] = "ApplyAll";

//	{ &szOptions[0], szAspect, it_BoolT, (LPTSTR)&gbKeepAspect, &gbChgKeepAsp, 0, 0 },
TCHAR szAspect[] = "KeepAspectPV";
extern   BOOL gbKeepAspect, gbChgKeepAsp;

//#define	gfApplyAll		WrkStr.w_bApplyAll
//#define	gfChgApplyAll	WrkStr.w_bChgApplyAll
//#define	gfIsAGIF		WrkStr.w_bIsAGIF
//#define	gfChgIsAGIF		WrkStr.w_bChgIsAGIF
//#define	gfAnimate		WrkStr.w_bAnimate
//#define	gfChgAnimate	WrkStr.w_bChgAnimate


#ifdef	ADDLIBCFG
//==============================================

/* [Library Configuration] Section */
char	szLCfg[] = "Library Configuration";
char	szSafSiz[] = "Safety";	/* BOOL Yes or No - Def=Yes */
char	szShwWrn[] = "Show Warning";

extern	BOOL	fLSafety;
extern	BOOL	fChgSF;
extern	LIBCONFIG	DVLibConfig;
extern LPWGETLCONFIG	WGetLConfig; /* These are filled in, when the Library */
extern LPWSETLCONFIG	WSetLConfig;	/* has been successfully found and loaded */
extern	BOOL	GetJPEGLib( UINT caller );

// SEE Options2.c for
// [JPEG Decompression] Section
//char	szJDecomp[] = "JPEG Decompression";
// [JPEG Compression] Section
// ==========================
//char	szJComp[] = "JPEG Compression";

//==============================================
#endif	/* ADDLIBCFG */

#ifdef	ADDCOLRF
//	case WM_ERASEBKGND:
extern	DIBX	gDIBx;
extern	BOOL	fUdtgDIBx;
extern	BOOL	fChgUDIBx;
extern	BOOL	fChgGBack;
extern	BOOL	fChgGShad;
extern	BOOL	fChgGHigh;
extern	BOOL	fChgGText;
extern	void GDefFace( void );
extern	void GDefShad( void );
extern	void GDefHiLite( void );
extern	void GDefText( void );

char	szGifSect[] = "GIF Options";
char	szUdtGlob[] = "Global Update";

char	szGBackgnd[] = "Background";
char	szGShadow[] = "Shadow";
char	szGHiLite[] = "HighLight";
char	szGText[] = "Text";
char	szSz3[] = "RGB(%3d,%3d,%3d)";

#endif	// ADDCOLRF

// Magnify sizes
TCHAR szMag[] = "Magnify";

extern TCHAR szMagOcx[];   // = "MagOutercx";
extern int gMagOCX;        // = 120;
extern BOOL gChgOCX;       // = FALSE;
extern TCHAR szMagOcy[];   // = "MagOutercy";
extern int gMagOCY;        // = 120;
extern BOOL gChgOCY;       // = FALSE;
extern TCHAR szMagIcx[];   // = "MagInnercx";
extern int gMagICX;        // = 40;
extern BOOL gChgICX;       // = FALSE;
extern TCHAR szMagIcy[];   // = "MagInnercy";
extern int gMagICY;        // = 40;
extern BOOL gChgICY;       // = FALSE;
extern TCHAR szMagTC[];    // = "MagTransColor";
extern COLORREF gMagTC;    // = RGB(255, 0, 254);
extern BOOL gChgTC;        // = FALSE;
extern TCHAR szMagState[];   // = "Magnify";
extern int   gi_MouseMagnify; // = 1;
extern BOOL  gChgMag;      // = FALSE;
extern TCHAR szMagOL[];    // = "MagOutlineColor";
extern COLORREF gMagOL;    // = RGB(255, 0, 0);
extern BOOL gChgOL;        // = FALSE;
extern TCHAR szMagAO[];    // = "MagAddOutline";
extern BOOL bAddOutline;   // = TRUE;
extern BOOL gChgAO;        // = FALSE;
// SWISH STUFF
extern TCHAR szPenW[];     // = "SwPenWidth";
extern DWORD gPenW;        // = SW_PEN_SZ;   // default SIZE
extern BOOL  gChgPen;      // = FALSE;
extern TCHAR szPenC[];     // = "SwPenColor";
extern COLORREF gPenC;     // = RGB(255,0,0);   // default COLOR
extern BOOL  gChgPC;       // = FALSE;
TCHAR szSwSwish[] = "SwTypeSwish";
extern BOOL gbSwish;       // = TRUE;
extern BOOL gbOval;        // = FALSE;
TCHAR szSwOval[] = "SwTypeOval";
extern BOOL gbChgSwi;      // = FALSE;
extern BOOL gbChgOva;      // = FALSE;

//extern	WORD	wMaxCols; 	/* = DEF_MAX_COLS */
//extern	BOOL	fChgMC;
TCHAR szCusCol[] = "Custom Colors";
extern COLORREF	CustColors[16];
TCHAR szColor[] = "Color";

#define  MX_CLIP_LIST   32
typedef struct tagCLIPLIST {
   LIST_ENTRY  list;
   RECT  clip;
}CLIPLIST, * PCLIPLIST;
TCHAR szCL[] = "Clip List";
LIST_ENTRY gClipList = { &gClipList, &gClipList };
BOOL bChgCL = FALSE;
TCHAR szClip[] = "Clip%d";
TCHAR szCLFm[] = "%d,%d,%d,%d";

VOID  FreeClipList(VOID)
{
   PLE   ph = &gClipList;
   KillLList(ph);
}

VOID  Add2ClipList( PRECT prc )
{
   PLE   ph,pn;
   PCLIPLIST   pcl;
   PRECT pclip;
   BOOL  fnd = FALSE;
   int   cnt;

   ph = &gClipList;
   Traverse_List(ph,pn)
   {
      pcl = (PCLIPLIST)pn;
      pclip = &pcl->clip;
      if( EqualRect( pclip, prc ) ) {
         // extract it, and place it FIRST
         fnd = TRUE;
         break; // already in LIST
      }
      //if(( RECTWIDTH(pclip) == RECTWIDTH(prc) )&&
      //   ( RECTHEIGHT(pclip) == RECTHEIGHT(prc) ))
      //{
      //   // set to NEW x,y, and likewise, extract, and put at head
      //   *pclip = *prc;
      //   bChgCL = TRUE;
      //   fnd = TRUE;
      //   break;
      //}
   }
   if(fnd)
   {
      if( !IsListHead(ph,pn) )
      {
         RemoveEntryList(pn);
         InsertHeadList(ph,pn);
         bChgCL = TRUE;
      }
      return;
   }
   // NEW ENTRY - Add to HEAD
   pn = (PLE)MALLOC(sizeof(CLIPLIST));
   CHKMEM2(pn, sizeof(CLIPLIST));
   pcl = (PCLIPLIST)pn;
   pclip = &pcl->clip;
   *pclip = *prc;
   InsertHeadList(ph,pn);
   bChgCL = TRUE;
   ListCount2(ph,&cnt);
   dbgr_sprtf( "RECTDBG: Added RECT %s to list, total %d ...\n", GetRectStg(pclip), cnt );

}

VOID  Remove_and_Add2ClipList( PRECT prc, PRECT rem )
{
   PLE   ph,pn;
   PCLIPLIST   pcl;
   PRECT pclip;
   BOOL  fnd = FALSE;
   ph = &gClipList;
   Traverse_List(ph,pn)
   {
      pcl = (PCLIPLIST)pn;
      pclip = &pcl->clip;
      if( EqualRect( pclip, rem ) ) {
         // extract it, and place it FIRST
         fnd = TRUE;
         break; // already in LIST
      }
      //if(( RECTWIDTH(pclip) == RECTWIDTH(rem) )&&
      //   ( RECTHEIGHT(pclip) == RECTHEIGHT(rem) ))
      //{
      //   // likewise, extract, and put at head
      //   fnd = TRUE;
      //   break;
      //}
   }
   if(fnd)
   {
      RemoveEntryList(pn);
      MFREE(pn);
      bChgCL = TRUE;
   }
   Add2ClipList( prc );
}

PRECT Get_Clip_Rect( UINT cmd )
{
   PLE   ph,pn;
   PCLIPLIST   pcl;
   PRECT pclip = NULL;
   BOOL  fnd = FALSE;
   UINT  cnt = 0;
   ph = &gClipList;
   Traverse_List(ph,pn)
   {
      if(cmd == cnt)
      {
         pcl = (PCLIPLIST)pn;
         pclip = &pcl->clip;
         break;
      }
      cnt++;
   }
   return pclip;
}

#define	MXSTKB		80

#define	GetStg( a, b )	\
	GetPrivateProfileString( a, b, (LPSTR)&szBlank[0], lpb, MXSTKB, lpIni )

//typedef struct	tagINILIST {	/* i */
//	LPSTR	i_Sect;
//	LPSTR	i_Item;
//	WORD	i_Type;
//	LPSTR	i_Deft;
//	LPINT	i_Chg;
//	LPVOID	i_Void;
//	DWORD	i_Res1;
//} INILIST;
//typedef INILIST MLPTR LPINILIST;

STGVAL	svDispOpt[] = {
	{ DISP_USE_DDBS, "DDB" },
	{ DISP_USE_DIBS, "DIB" },
	{ DISP_USE_SETDIBITS, "SETDI" },
	{ 0, 0 }
};

STGVAL	svPrintOpt[] = {
	{ IDRB_BESTFIT, "BestFit" },
	{ IDRB_STRETCH, "Stretch" },
	{ IDRB_SCALE,   "Scale" },
	{ 0, 0 }
};

// FIX20080316 - Use WINDOWPLACMENT sizing
BOOL  ChangedWP( WINDOWPLACEMENT * pw1, WINDOWPLACEMENT * pw2 )
{
   BOOL  bChg = FALSE;
   if( ( pw1->length != sizeof(WINDOWPLACEMENT) ) ||
       ( pw2->length != sizeof(WINDOWPLACEMENT) ) ||
       ( pw1->showCmd != pw2->showCmd ) ||
       ( pw1->ptMaxPosition.x != pw2->ptMaxPosition.x ) ||
       ( pw1->ptMaxPosition.y != pw2->ptMaxPosition.y ) ||
       ( !EqualRect( &pw1->rcNormalPosition, &pw2->rcNormalPosition ) ) )
   {
      bChg = TRUE;
   }
   return bChg;
}

static BOOL bDedbugWP = FALSE;
VOID UpdateWP( HWND hwnd )
{
   WINDOWPLACEMENT wp;
   wp.length = sizeof(WINDOWPLACEMENT);
   if( GetWindowPlacement(hwnd,&wp) )
   {
      if( ChangedWP( &wp, &g_sWP ) ) {
         if( bDedbugWP ) {
            sprtf( "UpdateWP: Set change on WP now %s, was %s (h=%#8X)"MEOR,
               Rect2Stg( &wp.rcNormalPosition ),
               Rect2Stg( &g_sWP.rcNormalPosition ),
               hwnd );
         }
         memcpy( &g_sWP, &wp, sizeof(WINDOWPLACEMENT) );
         g_sWP.length = sizeof(WINDOWPLACEMENT);
         bChgWP = TRUE;
      } else {
         //sprtf( "UpdateWP: No change for handle %#8X!"MEOR, hwnd );
      }
   } else {
      sprtf( "UpdateWP: GetWindowPlacement FAILED on handle %#8X!"MEOR, hwnd );
   }
}


//	LPSTR i_Sect, LPSTR i_Item, WORD i_Type, LPSTR i_Deft, LPINT i_Chg
// ***NOTE***: First entry MUST be gfChgAll
INILIST	IniList[] = {
	{ &szVers[0], &szVDate[0], it_String, &szVerDate[0], &gfChgAll, 0, 0 },

	{ &szDirs[0], &szRDir[0],  it_Dir,    &gszRDirName[0], &gfChgOpen, 0, 0 },
	{ &szDirs[0], &szWDir[0],  it_Dir,    &gszWDirName[0], &gfChgSave, 0, 0 },
//	{ &szDirs[0], &szSrch[0],  it_Dir,    &gszSearchDir[0], &gfChgMask, 0, 0 },

	{ &szDirs[0], &szOInd[0],  it_Size1, (LPSTR)&gwOutIndex, &fChgOutInd, 0, 0 },
	{ &szDirs[0], &szIInd[0],  it_Size1, (LPSTR)&InIndex,  &fChgInInd, 0, 0 },
	{ &szDirs[0], &szRecur[0], it_BoolT,  (LPSTR)&gfDoIter, &gfChgIter, 0, 0 },

	{ &szGens[0], &szBeTidy[0],it_BoolT,  (LPSTR)&gfBe_Tidy, &gfChgBT, 0, 0 },
	{ &szGens[0], &szAutoLoad[0],it_BoolF,  (LPSTR)&gfAutoLoad, &gfChgAL, 0, 0 },
	{ &szGens[0], &szAutoRLLast[0],it_BoolT,  (LPSTR)&gfAutoRLLast, &gfChgRLL, 0, 0 },

   // MRU files, history, finds and masks
	{ &szGens[0], &szMaxFiles[0],it_Size2,  (LPSTR)&gdwMaxFiles, &gfChgSC, 0, 0 },
	{ &szGens[0], &szHisFiles[0],it_Size2,  (LPSTR)&gdwHisFiles, &gfChgMH, 0, 0 },
	{ &szGens[0], &szFndFiles[0],it_Size2,  (LPSTR)&gdwFndFiles, &gfChgMF, 0, 0 },
	{ &szGens[0], &szMskFiles[0],it_Size2,  (LPSTR)&gdwMskFiles, &gfChgMM, 0, 0 },

	{ &szGens[0], &szSaveI[0], it_BoolT,  (LPSTR)&gfSavINI, &gfChgSI, 0, 0 },
	{ &szGens[0], &szReLoad[0], it_BoolT,  (LPSTR)&fReLoad, &fChgRL, 0, 0 },
	{ &szGens[0], &szOpenWindow[0], it_BoolT,  (LPSTR)&gfOpenWindow, &gfChgOW, 0, 0 },
    // FIX20080316 with diag file always on, added this verbosity
   { szGens,     szVerbosity, it_Size2,   (LPSTR)&iVerbosity, &bChgVerb, 0, 0 },

#ifdef	ADDLIBCFG
	{ &szLCfg[0], &szSafSiz[0],it_BoolT,  (LPSTR)&fLSafety, &fChgSF, 0, 0 },
	{ &szLCfg[0], &szShwWrn[0],it_BoolT,  (LPSTR)&gfShowWarn, &gfChgShowWarn, 0, 0 },
#endif	/* ADDLIBCFG */
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
	{ &szGens[0], &szFErr[0],  it_BoolT,  (LPSTR)&fNoFErr, &fChgFE, 0, 0 },
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
#ifdef	TIMEDPUT
	{ &szGens[0], &szTimed[0], it_BoolF, (LPSTR)&fAddTime, &fChgAT, 0, 0 },
	{ &szGens[0], &szRate[0],  it_Size2, (LPSTR)&wBitsPSec, &fChgBS, 0, 0 },
#endif	/* TIMEDPUT */
	{ &szGens[0], &szMaxCols[0],  it_Size2, (LPSTR)&gdwMaxCols, &gfChgMC, 0, 0 },
	{ &szGens[0], &szNextPaste[0],  it_Size2, (LPSTR)&gnPasteNum, &gbChgPaste, 0, 0 },
	// FIX980430 - Generalized!!!
//#ifdef   USELLIST - NOT use the DWORD i_Res1 as a PDWORD to the MAXIMUM
// Of course this means szMaxFiles = gdwMaxFiles MUST be read BEFORE this!
	{ &szFiles[0], &szFileN[0], it_Files2, (LPSTR)&gsFileList, &gfChgFil,  &gdwFilCnt,
            (LPVOID) &gdwMaxFiles },
	{ &szHist[0], &szHFileN[0], it_Files2, (LPSTR)&gsHistList, &gfChgHist, &gdwHistCnt,
            (LPVOID) &gdwHisFiles },
	{ &szFinds[0], &szFindN[0], it_Files2, (LPSTR)&gsFindList, &giFindChg, &gdwFindCnt,
            (LPVOID) &gdwFndFiles },
	{ &szMasks[0], &szMaskN[0], it_Files2, (LPSTR)&gsMaskList, &gbChgMsk,  &gdwMaskCnt,
            (LPVOID) &gdwMskFiles },
	{ &szAutos[0], &szAutoN[0], it_Files2, (LPSTR)&gsAutoList, &gbChgARL,  &gdwAutoCnt,
            (LPVOID) &gdwAutFiles },

#ifdef	ADDCOLRF
	{ &szGifSect[0], &szUdtGlob[0], it_BoolT,(LPSTR)&fUdtgDIBx, &fChgUDIBx, 0, 0 },
	{ &szGifSect[0], &szGBackgnd[0],it_Colr,(LPSTR)&gDIBx.dx_Face,&fChgGBack, GDefFace, 0 },
	{ &szGifSect[0], &szGShadow[0],it_Colr,(LPSTR)&gDIBx.dx_Shadow,&fChgGShad, GDefShad, 0 },
	{ &szGifSect[0], &szGHiLite[0],it_Colr,(LPSTR)&gDIBx.dx_HiLite,&fChgGHigh, GDefHiLite, 0 },
	{ &szGifSect[0], &szGText[0],it_Colr,(LPSTR)&gDIBx.dx_Text,&fChgGText, GDefText, 0 },
//char	szGText[] = "Text";
//extern	BOOL	fChgGText;
#endif	// ADDCOLRF
	{ &szOptions[0], &szStretch[0], it_BoolF, (LPSTR)&gfStretch, &gfChgStretch, 0, 0 },
//#ifdef  WIN32
   // "PrintCenter";
   { &szOptions[0], &szPrtCenter[0], it_BoolT, (LPSTR)&gfPrtCenter, &gfChgPrtCent, 0, 0 },
//#else // !WIN32
#ifndef  WIN32
// ============================================================
	{ &szOptions[0], &szPrinterBand[0], it_BoolF, (LPSTR)&gfPrinterBand, &gfChgPrinterBand, 0, 0 },
	{ &szOptions[0], &szUse32API[0], it_BoolF, (LPSTR)&gfUse31PrintAPIs, &gfChgPrintAPIs, 0, 0 },
// ============================================================
#endif   // #ifndef  WIN32 y/n
	{ &szOptions[0], &szDispType[0], it_StgVal, (LPSTR)&gwDispOption, &gfChgDispOption, &svDispOpt[0], 0 },
	{ &szOptions[0], &szPrintType[0], it_StgVal, (LPSTR)&gwPrintOption, &gfChgPrintOption, &svPrintOpt[0], 0 },
	{ &szOptions[0], &szXScale[0], it_Size1, (LPSTR)&gwXScale, &gfChgXScale, 0, 0 },
	{ &szOptions[0], &szYScale[0], it_Size1, (LPSTR)&gwYScale, &gfChgYScale, 0, 0 },
	{ &szOptions[0], &szMilSecs[0], it_Size2, (LPSTR)&gdwMilSecs, &gfChgMilSecs, 0, 0 },
	{ &szOptions[0], &szSetDef[0], it_BoolT, (LPSTR)&gfSetDefault, &gfChgSetDefault, 0, 0 },
	{ &szOptions[0], &szApplyAll[0], it_BoolF, (LPSTR)&gfApplyAll, &gfChgApplyAll, 0, 0 },
	{ &szOptions[0], szAspect, it_BoolT, (LPTSTR)&gbKeepAspect, &gbChgKeepAsp, 0, 0 },

   { &szWind[0], &szHide[0],  it_BoolT, (LPSTR)&gbHide,   &gbChgHide, 0, 0 },
#ifdef   USENEWWINSIZE
   { szWind, szGOut, it_WinSize,         (PTSTR)&g_sWP,   &bChgWP, (PVOID)&gbGotWP, 0 },
#else // !#ifdef   USENEWWINSIZE
// LAST TWO are the MAXIMUM Screen size, and the current Window size
	{ &szWind[0], &szIsZmd[0], it_BoolF, (LPSTR)&bIsZoomed, &bChgZoomed, 0, 0 },
	{ &szWind[0], &szMaxScn[0], it_SizeW,  (LPSTR)&rMaxSiz, &fChgMax, 0, 0 },
	{ &szWind[0], &szSize[0],  it_SizeW,  (LPSTR)&rIniSiz, &fChgSiz, 0, 0 },
#endif // !#ifdef   USENEWWINSIZE

   { szMag,       szMagOcx,    it_Size1, (PTSTR)&gMagOCX, &gChgOCX, 0, 0 },
   { szMag,       szMagOcy,    it_Size1, (PTSTR)&gMagOCY, &gChgOCY, 0, 0 },
   { szMag,       szMagIcx,    it_Size1, (PTSTR)&gMagICX, &gChgICX, 0, 0 },
   { szMag,       szMagIcy,    it_Size1, (PTSTR)&gMagICY, &gChgICY, 0, 0 },
   { szMag,       szMagTC,     it_Colr,  (PTSTR)&gMagTC,  &gChgTC,  0, 0 },
   { szMag,    szMagState,     it_BoolF, (PTSTR)&gi_MouseMagnify, &gChgMag, 0, 0 },
   { szMag,       szMagOL,     it_Colr,  (PTSTR)&gMagOL,  &gChgOL,  0, 0 },
   { szMag,       szMagAO,     it_BoolT, (PTSTR)&bAddOutline, &gChgAO, 0, 0 },
   // SWISH STUFF
   { szMag,       szPenW,      it_Size1, (PTSTR)&gPenW,   &gChgPen, 0, 0 },
   { szMag,       szPenC,      it_Colr,  (PTSTR)&gPenC,   &gChgPC,  0, 0 },
   { szMag,    szSwSwish,      it_BoolT, (PTSTR)&gbSwish, &gbChgSwi,0, 0 },
   { szMag,     szSwOval,      it_BoolF, (PTSTR)&gbOval,  &gbChgOva,0, 0 },

   // [Clip List] 
   { szCL, szClip, it_ListClip, (PTSTR)&gClipList, &bChgCL, szCLFm, (LPVOID) MX_CLIP_LIST },

   // termination of table
	{ 0, 0, 0, 0, 0, 0, 0 }
};

BOOL  GotAChange( VOID )
{
   BOOL     bRet  = FALSE;
	PINILIST lpLst = &IniList[0];
   LPINT    lpb;

	while( lpLst->i_Type )
	{
      if( lpb = lpLst->i_Chg )
      {
         if( *lpb )
         {
            bRet = TRUE;
            break;
         }
      }

      lpLst++; // bump to next

   }
   return bRet;
}

BOOL	IsYes( LPSTR lps )
{
	char	buf[MXSTKB+1];
	int	i, j;
	BOOL	flg;
	char	c;

	flg = FALSE;
	if( lps && (i = lstrlen( lps ) ) )
	{
		if( i > MXSTKB )
			i = MXSTKB;
		for( j = 0; j < i; j++ )
		{
			c = lps[j];
			if( c <= ' ' || c == ';' )
				break;
			else
				buf[j] = c;
		}
		if( j )
		{
			buf[j] = 0;
			if( (lstrcmpi( &szYes[0], &buf[0] ) == 0) ||
				(lstrcmpi( &szOn[0], &buf[0] ) == 0) )
			{
				flg = TRUE;
			}
		}
	}
	return( flg );
}

BOOL	IsNo( LPSTR lps )
{
	char	buf[MXSTKB+1];
	int	i, j;
	BOOL	flg;
	char	c;

	flg = FALSE;
	if( lps && (i = lstrlen( lps ) ) )
	{
		if( i > MXSTKB )
			i = MXSTKB;
		for( j = 0; j < i; j++ )
		{
			c = lps[j];
			if( c <= ' ' || c == ';' )
				break;
			else
				buf[j] = c;
		}
		if( j )
		{
			buf[j] = 0;
			if( (lstrcmpi( &szNo[0], &buf[0] ) == 0) ||
				(lstrcmpi( &szOff[0], &buf[0] ) == 0) )
			{
				flg = TRUE;
			}
		}
	}
	return( flg );
}

#ifndef  USENEWWINSIZE
void	SetChgSize( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
//	RECT	rc;
	if( gfDvInited && !fChgSiz )	// Only AFTER initial window done
	{
//		GetWindowRect( hWnd, &rc );
//		rc.right = rc.right - rc.left;
//		rc.bottom = rc.bottom - rc.top;
//		if( (rc.left != rIniSiz.left) ||
//			(rc.top != rIniSiz.top) ||
//			(rc.right != rIniSiz.right) ||
//			(rc.bottom != rIniSiz.bottom) )
      if( ( grcSize.right  != rIniSiz.right  ) ||
          ( grcSize.bottom != rIniSiz.bottom ) )
      {
			fChgSiz = TRUE;	// Set to write the size out to INI file
		}
	}
}
#endif // #ifndef  USENEWWINSIZE

// ================================================================
//	The unbiquitous INI File ...
//
// Under the switch ONEINI, add TWO ENTRIES to WIN.INI -
// Namely Version and RUNTIME Directory (if a HDD - TBD)
// If WIN.INI yield NO VERSION, or a DIFFERENT VERSION
// then set CHANGE ALL, and ensure they are added for
// the NEXT run ...
//
// ================================================================
#define	MMIN_CX		100
#define MMIN_CY		30

void WriteWININI( LPSTR lpIni )
{
	LPSTR lps, lpi, lpd;
	gfChgAll = TRUE;
	lps = &szDB32[0];	// WIN.INI Section
	lpi = &szVers[0];	// WIN.INI Key/Item
	lpd = &szVerDate[0];	// Current VERSION DATE
	WriteProfileString( lps, // pointer to section name
		lpi,		// pointer to key name
		lpd );		// pointer to string to write
	lpi = &szIFile[0];	// WIN.INI Key/Item
	lpd = lpIni;		// Current INI Filename
	WriteProfileString( lps, // pointer to section name
		lpi,		// pointer to key name
		lpd );		// pointer to string to write
}

void	CheckINIVals( void )
{	// A chance to CHECK some INI values, like
	// 1. Change in RESOLUTION
   int   x;
#ifndef   USENEWWINSIZE
	RECT	rc;
	int	y, cx, cy;
	GetWindowRect( GetDesktopWindow(), &rc );
	if( ( rc.right  != rMaxSiz.right  ) ||
		( rc.bottom != rMaxSiz.bottom ) )
	{
		fChgMax = TRUE;
		if( rIniSiz.left != CW_USEDEFAULT )
		{
			x = rIniSiz.left;
			y = rIniSiz.top;
			cx = rIniSiz.right;
			cy = rIniSiz.bottom;
			if( (x + cx) > rc.right )
			{
				// Make SMALLER
				if( cx < rc.right )
				{
					rIniSiz.left = rc.right - cx;
					rIniSiz.right = rc.right - rIniSiz.left;
				}
				else
				{
					rIniSiz.left = 0;
					rIniSiz.right = rc.right;
				}
			}
			else
			{
				// Maybe ENLARGE


			}

			if( (y + cy) > rc.bottom )
			{
				// Make SMALLER
				if( cy < rc.bottom )
				{
					rIniSiz.top = rc.bottom - cy;
					rIniSiz.bottom = rc.bottom - rIniSiz.top;
				}
				else
				{
					rIniSiz.top = 0;
					rIniSiz.bottom = rc.bottom;
				}
			}
			else
			{
				// Maybe ENLARGE

			}
		}
	}
#endif   // ifndef   USENEWWINSIZE

	if( gdwMaxFiles == 0 )	// If this is ZERO
	{
		gdwMaxFiles = 1;
		gfChgSC = TRUE;		// Set CHANGED to write new value
	}
	if( gfChgAll )
	{
		if( !gfSavINI )
		{
			gfSavINI = TRUE;
			gfChgSI = TRUE;
		}
	}


   // check the FILE LIST - Only include those that EXIST now
// Of course this means szMaxFiles = gdwMaxFiles MUST be read BEFORE this!
//	{ &szFiles[0], &szFileN[0], it_Files2, (LPSTR)&gsFileList, &gfChgFil,  &gdwFilCnt,
//            (DWORD) &gdwMaxFiles },
   if( gdwFilCnt ) {
      static WIN32_FIND_DATA _s_wfd;
      PLE pn, pnx;
      PTSTR pf;
      PMWL pmwl;
      PLE ph = &gsFileList;    // get the MRU file list
      x = 0;
      Traverse_List( ph, pn )
      {
         pmwl = (PMWL)pn;
         // if( pmwl->wl_dwFlag & flg_IsLoaded )
         pf = pmwl->wl_szFile;
         if( !(IsValidFile( pf, &_s_wfd ) == IS_FILE) ) {
            // this file no longer exists - *** REMOVE IT ***
            pmwl->wl_dwFlag |= flg_NotValid; // mark it NOT valid
            // or, let us try ... ;=))
            pnx = pn->Blink;    // start from here for traverse
            RemoveEntryList(pn);
            AddToHistList( &pmwl->wl_szFile[0] );  // add this to HISTORY
            MFREE( pn );   // NOTE: pNext is NOW invalid
            if( gdwFilCnt )
               gdwFilCnt--;
            pn = pnx; // BACK one so that travers will continue to NEXT
            x++;
         }
      }
      if(x) {
         // and SET CHANGE
			gfChgFil = TRUE;
      }
   }

}

// FIX20080316 - read INI from path of RUNTIME,
// with a special exception when running in the DEBUG folder.
// ==========================================================
BOOL  Chk4Debug( PTSTR lpd )
{
   static char m_szTmpBuf[260];
   BOOL     bret = FALSE;

   PTSTR ptmp = m_szTmpBuf;
   PTSTR   p;
   DWORD  dwi;

   strcpy(ptmp, lpd);
   dwi = (DWORD)strlen(ptmp);
   if(dwi)
   {
      dwi--;
      if(ptmp[dwi] == '\\')
      {
         ptmp[dwi] = 0;
         p = strrchr(ptmp, '\\');
         if(p)
         {
            p++;
            if( strcmpi(p, "DEBUG") == 0 )
            {
               *p = 0;
               strcpy(lpd,ptmp);    // use this
               bret = TRUE;
            }
         }
      }
   }
   return bret;
}

// FIX20080316 - read INI from path of RUNTIME,
void  GetModulePath( PTSTR lpb )
{
   LPTSTR   p;
   GetModuleFileName( NULL, lpb, 256 );
   p = strrchr( lpb, '\\' );
   if( p )
      p++;
   else
      p = lpb;
   *p = 0;
#ifndef  NDEBUG
   Chk4Debug( lpb );
#endif   // !NDEBUG

}

#ifndef PATH_SEP
#ifdef WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif
#endif

#ifdef _WIN32
#define M_IS_DIR _S_IFDIR
#else // !_MSC_VER
#define M_IS_DIR S_IFDIR
#endif

#define MDT_NONE 0
#define	MDT_FILE 1
#define	MDT_DIR 2


static struct stat buf;

int is_file_or_directory(const char* path)
{
	if (!path)
		return MDT_NONE;
	if (stat(path, &buf) == 0)
	{
		if (buf.st_mode & M_IS_DIR)
			return MDT_DIR;
		else
			return MDT_FILE;
	}
	return MDT_NONE;
}

size_t get_last_file_size() { return buf.st_size; }


int create_dir(const char* pd)
{
	int iret = 1;
	int res;
	if (is_file_or_directory(pd) != MDT_DIR) {
		size_t i, j, len = strlen(pd);
		char ps, ch, pc = 0;
		char tmp[260];
		j = 0;
		iret = 0;
		tmp[0] = 0;
#ifdef _WIN32
		ps = '\\';
#else
		ps = '/'
#endif

		for (i = 0; i < len; i++) {
			ch = pd[i];
			if ((ch == '\\') || (ch == '/')) {
				ch = ps;
				if ((pc == 0) || (pc == ':')) {
					tmp[j++] = ch;
					tmp[j] = 0;
				}
				else {
					tmp[j] = 0;
					if (is_file_or_directory(tmp) != MDT_DIR) {
						res = mkdir(tmp);
						if (res != 0) {
							return 0; // FAILED
						}
						if (is_file_or_directory(tmp) != MDT_DIR) {
							return 0; // FAILED
						}
					}
					tmp[j++] = ch;
					tmp[j] = 0;
				}
			}
			else {
				tmp[j++] = ch;
				tmp[j] = 0;
			}
			pc = ch;
		} // for lenght of path
		if ((pc == '\\') || (pc == '/')) {
			iret = 1; // success
		}
		else {
			if (j && pc) {
				tmp[j] = 0;
				if (is_file_or_directory(tmp) == MDT_DIR) {
					iret = 1; // success
				}
				else {
					res = mkdir(tmp);
					if (res != 0) {
						return 0; // FAILED
					}
					if (is_file_or_directory(tmp) != MDT_DIR) {
						return 0; // FAILED
					}
					iret = 1; // success
				}
			}
		}
	}
	return iret;
}

void setup_default_cache(PTSTR lpini)
{
	if (!gszCacheData[0]) {
		size_t len = strlen(lpini);
		char ch;
		if (len) {
			strcpy(gszCacheData, lpini);
			ch = lpini[len - 1];
			if ((ch != '\\') && (ch != '/'))
				strcat(gszCacheData, PATH_SEP);
			strcat(gszCacheData, "Cache");
			if (!create_dir(gszCacheData)) {
				gszCacheData[0] = 0;
			}
			else {
				strcat(gszCacheData, PATH_SEP);
			}
		}
	}
}

void GetAppData(PTSTR lpini)
{
	char* pd;
	if (!gszAppData[0]) {
		//pd = getenv("PROGRAMDATA"); // UGH - do not have permissions -  how to GET permissions
		//if (!pd) {
		//	pd = getenv("ALLUSERSPROFILE");
		//}
		pd = getenv("APPDATA");
		if (!pd) {
			pd = getenv("LOCALAPPDATA");
		}
		if (pd) {
			strcpy(gszAppData, pd);
			strcat(gszAppData, PATH_SEP);
			strcat(gszAppData, "DibView");
			if (!create_dir(gszAppData)) {
				gszAppData[0] = 0;
			}
			else {
				strcat(gszAppData, PATH_SEP);
			}
		}
	}

	if (gszAppData[0]) {
		strcpy(lpini, gszAppData);
	}
	else {
		GetModulePath(lpini);    // does GetModuleFileName( NULL, lpini, 256 );
	}

	////////////////////////////////////
	// setup the default DATA cache
	setup_default_cache(lpini);
	///////////////////////////////////

}


// FIX20200602: DibView.ini should be in %APPDATA%\DibView/
// NO - FIX20080316 - read INI from path of RUNTIME,
void GetINIFile( PTSTR lpini )
{
	GetAppData(lpini);
	strcat(lpini, szIniDef);
}

// FIX20080316 - change window sizing
BOOL  ValidShowCmd( UINT ui )
{
   BOOL  bRet = FALSE;
   if( ( ui == SW_HIDE ) ||   //Hides the window and activates another window. 
       ( ui == SW_MAXIMIZE ) ||  //Maximizes the specified window. 
       ( ui == SW_MINIMIZE ) ||  //Minimizes the specified window and activates the next top-level window in the Z order. 
       ( ui == SW_RESTORE ) ||   //Activates and displays the window. If the window is minimized or maximized, the system restores it to its original size and position. An application should specify this flag when restoring a minimized window. 
       ( ui == SW_SHOW ) || //Activates the window and displays it in its current size and position.  
       ( ui == SW_SHOWMAXIMIZED ) || //Activates the window and displays it as a maximized window. 
       ( ui == SW_SHOWMINIMIZED ) || //Activates the window and displays it as a minimized window. 
       ( ui == SW_SHOWMINNOACTIVE ) ||  //Displays the window as a minimized window. 
       ( ui == SW_SHOWNA ) || //Displays the window in its current size and position. 
       ( ui == SW_SHOWNOACTIVATE ) ||  //Displays a window in its most recent size and position. 
       ( ui == SW_SHOWNORMAL ) )
       bRet = TRUE;
   return bRet;
}

///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : GotWP
// Return type: BOOL 
// Arguments  : LPTSTR pSect
//            : LPTSTR pDef
//            : LPTSTR lpb
//            : LPTSTR lpini
// Description: Read in a special BLOCK of window palcement item from
//              the INI file. This is in its own [section].
///////////////////////////////////////////////////////////////////////////////
#define	GetStg2( a, b )	\
	GetPrivateProfileString( a, b, &szBlank[0], lpb, 256, lpini )

BOOL  GotWP( PTSTR pSect, PTSTR pDef, PTSTR lpb, PTSTR lpini )
{
   BOOL  bRet = FALSE;
   WINDOWPLACEMENT   wp;
   WINDOWPLACEMENT * pwp = (WINDOWPLACEMENT *)pDef;
   if( !pwp )
      return FALSE;

   *lpb = 0;
   GetStg2( pSect, szShow ); // = "ShowCmd";
   if( *lpb == 0 )
      return FALSE;
   wp.showCmd = m_atoi(lpb);
   if( !ValidShowCmd( wp.showCmd ) )
      return FALSE;

   *lpb = 0;
   GetStg2( pSect, szMaxX );
   if( *lpb == 0 )
      return FALSE;
   wp.ptMaxPosition.x = m_atoi(lpb);
   *lpb = 0;
   GetStg2( pSect, szMaxY );
   if( *lpb == 0 )
      return FALSE;
   wp.ptMaxPosition.y = m_atoi(lpb);

   *lpb = 0;
   GetStg2( pSect, szMinX );
   if( *lpb == 0 )
      return FALSE;
   wp.ptMinPosition.x = m_atoi(lpb);
   *lpb = 0;
   GetStg2( pSect, szMinY );
   if( *lpb == 0 )
      return FALSE;
   wp.ptMinPosition.y = m_atoi(lpb);

   *lpb = 0;
   GetStg2( pSect, szLeft );   // = "NormLeft";
   if( *lpb == 0 )
      return FALSE;
   wp.rcNormalPosition.left = m_atoi(lpb);
   *lpb = 0;
   GetStg2( pSect, szTop ); // = "NormTop";
   if( *lpb == 0 )
      return FALSE;
   wp.rcNormalPosition.top = m_atoi(lpb);
   *lpb = 0;
   GetStg2( pSect, szRite );   // = "NormRight";
   if( *lpb == 0 )
      return FALSE;
   wp.rcNormalPosition.right = m_atoi(lpb);
   *lpb = 0;
   GetStg2( pSect, szBot ); //  = "NormBottom";
   if( *lpb == 0 )
      return FALSE;
   wp.rcNormalPosition.bottom = m_atoi(lpb);

   wp.flags = 0;
   wp.length = sizeof(WINDOWPLACEMENT);

   memcpy( pwp, &wp, sizeof(WINDOWPLACEMENT) );
   bRet = TRUE;
   return bRet;
}

//TCHAR szCusCol[] = "Custom Colors";
//extern COLORREF	CustColors[16];
//TCHAR szColor[] = "Color";

VOID ReadCustomColors( PTSTR lpini )
{
	PTSTR lps = &szGens[0];
   PTSTR pSect = szCusCol;
	PTSTR lpb = GetTmp1();	// Get buffer from DvData.c (was &Buf[0];)
   PTSTR lpfm = &szSz3[0];
   short  ints[4];
   UINT  ui;
   COLORREF * pcr;

   for( ui = 0; ui < 16; ui++ )
   {
      pcr = &CustColors[ui];
      sprintf(lps, "%s%d", szColor, (ui + 1));
      GetStg2( pSect, lps );
      if(*lpb)
      {
         ints[0] = -1;
         ints[1] = -1;
         ints[2] = -1;
         ints[3] = -1;
         if( dv_wscanf( lpb, lpfm, (PTSTR)&ints[0] ) == 3 )
         {
            if((ints[0] >= 0) && (ints[0] <= 255) && 
               (ints[1] >= 0) && (ints[1] <= 255) &&
               (ints[2] >= 0) && (ints[2] <= 255) )
            {
               *pcr = RGB(ints[0], ints[1], ints[2]);
            }
         }
      }
   }
}

VOID WriteCustomColors( PTSTR lpIni )
{
   PTSTR lps = szCusCol;
	PTSTR lpout = GetTmp1();	// Get buffer from DvData.c (was &Buf[0];)
	PTSTR lpi = &szGens[0];
   UINT  ui;
   COLORREF * pcr;
   for( ui = 0; ui < 16; ui++ )
   {
      pcr = &CustColors[ui];
      sprintf(lpi, "%s%d", szColor, (ui + 1));
      sprintf(lpout, "RGB(%3d,%3d,%3d)",
         GetRValue( *pcr ),
         GetGValue( *pcr ),
         GetBValue( *pcr ) );
      WritePrivateProfileString(
						lps,		// Section
						lpi,		// Res.Word
						lpout,		// String to write
						lpIni );	// File Name
   }
}

// ===========================================================
//
// void ReadIni( LPSTR lpDefDir )
//
// The ALL IMPORTANT load previous preferences
//
// TBD: If an entry is NOT found in WIN.INI
//	then the Version and Location of the INI file
//	are added to this file.
//	Thereafter only ONE INI file is used by DibView!
//	But IF the Version Changes, this is also a chance
//	to correct/change the location of the INI file.
//	But we should READ from the OLD version, but WRITE
//	out to only the NEW version.
//	I don't think this is happening yet???
//
// FIX20200602: DibView.ini should be in %APPDATA%\DibView/
// NO! FIX20080316 - Use an INI file in the SANE PATH as the runtime
// ===========================================================
void	ReadIni( LPSTR lpDD )
{
	LPRECT	   lprc;
	LPSTR	      lpIni, lpNew;
	DWORD	      i, j;
	PINILIST 	lpLst;
//	char	Buf[MXSTKB+2]; - now using GetTmp()
	DWORD	      Typ;
	LPSTR	      lps, lpi, lpb, lpd, lpfm;
	LPWORD	   lpw;
	LPDWORD	   lpdw;
	LPINT	      lpInt;
#ifdef	ADDLIBCFG
	LPLIBCONFIG	lpCfg;
	BOOL	      fSaf, fLOk;
#endif	// ADDLIBCFG
	DWORD	      dw;
   LPSTR		   lpfl;
	DWORD		   dwoff, dwcnt;
   PLIST_ENTRY pHead, pNext;
   PTSTR       ptmp = GetStgBuf();

	lpNew = 0;
	gszIniFile[0] = 0;	// Kill any INI
	lpIni = &gszIniFile[0];	// INI file buffer
#ifdef	ADDLIBCFG
	lpCfg = &DVLibConfig;
#endif	// ADDLIBCFG

	// Establish an INI [PATH]\FileName.Ini
	// ====================================
	// This is in the CURRENT RUNTIME Directory
	// Use cwd() or GetModuleName()???
   // FIX20080316 - NO NO BACK TO GetModuleName()
   // since the INI file should ALWAYS be in the RUNTIME folder
	// FIX20200602: DibView.ini should be in %APPDATA%\DibView/ - ie lpDD shoudl be NULL
	if( lpDD && (i = lstrlen( lpDD )) )
	{
		lstrcpy( lpIni, lpDD );
		if( lpDD[i-1] != '\\' )
			lstrcat( lpIni, "\\" );
   	lstrcat( lpIni, &szIniDef[0] );
   } else {
      GetINIFile( lpIni ); // FIX20080316
   }

	// ====================================

	lpb = GetTmp1();	// Get buffer from DvData.c (was &Buf[0];)
	lpLst = &IniList[0];

#ifdef	ADDCOLRF
//	case WM_ERASEBKGND:
	dv_fmemset( &gDIBx, 0, sizeof( DIBX	) );
	fUdtgDIBx = TRUE;
	fChgUDIBx = FALSE;
#endif	// ADDCOLRF

#ifdef	ADDWININI   // FIX20080316 - REMOVE
	// This is some entries in WIN.INI
	// normally in the C:\WINDOWS directory.
	// =====================================
	lps = &szDB32[0];	// Header [DIBVIEW32]
	lpi = &szVers[0];	// Section "Version="
	lpd = &szVerDate[0];	// Current ie DEFAULT
	if( GetProfileString( lps, // address of section name
		lpi,	// address of key name
		(LPSTR)&szBlank[0],	// address of default string
		lpb,	// address of destination buffer
		MXSTKB ) ) // size of destination buffer
	{
		if( lstrcmpi( lpb, lpd ) == 0 )
		{
			// OK, we ARE EXACTLY the same VERSION
			// Get the INI file name.
			// ===================================
			lpi = &szIFile[0];
			lpd = lpIni;
			if( GetProfileString( lps, // address of section name
				lpi,	// address of key name
				(LPSTR)&szBlank[0],	// address of default string
				lpb,	// address of destination buffer
				MXSTKB ) ) // size of destination buffer
			{
				if( lstrcmpi( lpb, lpd ) != 0 )
				{
					// We are the SAME Version, but
					// we are NOT the same INI FILE
					if( CheckIfFileExists( lpb ) )
					{
						// If the OLD INI file
						// exists, then USE IT.
						lpNew = GetTmp2(); // get buffer
						lstrcpy( lpNew, lpIni ); // keep NEW
						lstrcpy( lpIni, lpb ); // use OLD
					}
					else
					{
						WriteWININI( lpIni );
					}
				}
				// else they are the SAME FILE
				// ie SAME VERSION and FILE
			}
			else
			{
				// OOPS, SAME VERIONS
				// BUT NO INI FILE!!!
				// This is bizarre -
				// Should NOT happen
				WriteWININI( lpIni );
			}
		}
		else
		{
			// DIFFERENT VERSION
			// =================
			// SHOULD try LOADING from
			// the PREVIOUS INI File,
			// then SET new one, if location
			// has changed.
			// We could just WriteWININI( lpIni );
			// but first get the OLD INI file name.
			// ====================================
			lpi = &szIFile[0];
			lpd = lpIni;
			if( GetProfileString( lps, // address of section name
				lpi,	// address of key name
				(LPSTR)&szBlank[0],	// address of default string
				lpb,	// address of destination buffer
				MXSTKB ) ) // size of destination buffer
			{
				if( strcmpi( lpb, lpd ) != 0 )
				{
					// We are DIFFERENT Version,
					// and
					// DIFFERENT INI FILE
					if( CheckIfFileExists( lpb ) )
					{
						// If the OLD INI file
						// exists, then USE IT.
						lpNew = GetTmp3(); // get buffer
						lstrcpy( lpNew, lpIni ); // keep NEW
						lstrcpy( lpIni, lpb ); // use OLD
					}
					else
					{
						WriteWININI( lpIni );
					}
				}
				else
				{
					// STRANGE
					// DIFFERENT VERSION but
					// SAME FILE!!!
					lpNew = GetTmp1(); // get buffer
					lstrcpy( lpNew, lpIni ); // keep NEW
					lstrcpy( lpIni, lpb ); // use OLD
				}
			}
			else
			{
				// DIFFERENT VERSION
				// BUT NO PREVIOUS INI FILE!!!
				// This is bizarre - 
				// Should NOT happen
				WriteWININI( lpIni );
			}
		}
	}
	else
	{
		// NO VERSION AT ALL
		// =================
		// This could be assumed as the
		//      *** FIRST RUN!!! ***
		// =============================
		WriteWININI( lpIni );
	}
#endif	// ADDWININI

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
// NOTE: This MUST be done BEFORE
// the first attempt a LoadLibrary
// ===============================
	Typ   = it_BoolT;	// Default is *NO* ... That is NO SHOW Sys Message
	lps   = &szGens[0];
	lpi   = &szFErr[0];
	lpInt = &fNoFErr;
	GetStg( lps, lpi );
	if( lpb[0] )
	{
		if( IsYes( lpb ) )
			*lpInt = TRUE;
		else if( IsNo( lpb ) )
			*lpInt = FALSE;
		else
			goto	Set_Def1;
	}
	else
	{
Set_Def1:
			*lpInt = FALSE;	
			fChgFE = TRUE;
	}

#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

// ==========================================
#ifdef	ADDLIBCFG
	fLOk = FALSE;
	// (STATIC or DYNAMIC)
	if( GetJPEGLib(7) &&
		WGetLConfig ) // filled in, when Library loaded
	{
		if( ((*WGetLConfig) ( lpCfg )) == 0 )
		{
			fSaf = lpCfg->cf_Safety_Check;	// Get Libraries item
			fLOk = TRUE;
		}
	}
#endif	// ADDLIBCFG


	// RUN THROUGH THE LIST OF ITEMS
	// =============================
	while( Typ = lpLst->i_Type )
	{
		lpd = lpLst->i_Deft;
		lprc = (LPRECT)lpd;
		if( (lps = lpLst->i_Sect) && (lpi = lpLst->i_Item) )
		{
			GetStg( lps, lpi );
			switch( Typ )
			{
			case it_String:
				{
					if( lstrcmpi( lpb, lpd ) != 0 )
					{
						*lpLst->i_Chg = TRUE;
					}
				}
				break;

			case it_Size4:
				{
					LPRECT	lptr;
#ifdef	WIN32
					lpfm = &szSz4l[0];
#else
					lpfm = &szSz4[0];
#endif	// WIN#@ y/n
					lptr = &TmpRec[0];
					// Set 4 aritrary values
					lptr->left   = CW_USEDEFAULT; // Default horizontal pos.
					lptr->top    = CW_USEDEFAULT; // Default vertical position.
					lptr->right  = CW_USEDEFAULT; // Default width.
					lptr->bottom = CW_USEDEFAULT; // Default height.
					if( lpb[0] &&
						(dv_wscanf( lpb, lpfm, (LPSTR)lptr )) == 4 )
					{
						if( ( lptr->left == CW_USEDEFAULT ) &&
							( lptr->top == CW_USEDEFAULT ) &&
							( lptr->right == CW_USEDEFAULT ) &&
							( lptr->bottom == CW_USEDEFAULT ) )
						{
							// Yikes, NOTHING CHANGED!!!
							*lpLst->i_Chg = TRUE;
						}
						else
						{
							lprc->left   = lptr->left; // horizontal pos.
							lprc->top    = lptr->top; // vertical position.
							lprc->right  = lptr->right; // width.
							lprc->bottom = lptr->bottom; // height.
						}
					}
					else	// ~4 Use DEFAULTS
					{
						*lpLst->i_Chg = TRUE;
					}
				}
				break;

			case it_Size1:
				{
					if( lpd )
					{ // If we have a POINTER, to a WORD in this case
						if( (dv_wscanf( lpb, &szSz1[0], ( PINT8 )lpd )) !=
							1 )
						{
							lpw = (LPWORD) lpd;
//							*lpw = 0;	// Default should already be SET
							*lpLst->i_Chg = TRUE;
						}
					}
				}
				break;

			case it_Size2:
				{
					if( lpd )
					{ // If we have a POINTER, to a DWORD in this case
						lpdw = (LPDWORD) lpd;
						dw = 0;
						if( dv_wscanf( lpb, &szSz2[0], ( PINT8) &dw ) == 1 )
						{
							if( *lpdw != dw )
							{
								*lpdw = dw;
							}
						}
						else
						{
//							*lpdw = 0;	// LEAVE DEFAULT INTERNAL VALUE
							*lpLst->i_Chg = TRUE;
						}
					}
				}
				break;

			case it_Dir:
				{
					if( lpb[0] )	// If it Yielded an entry
					{
						lstrcpy( lpd, lpb );	// Use the GIVEN Directory
					}
					else
					{
						*lpLst->i_Chg = TRUE;	// Mark as CHANGED
					}
				}
				break;

			case it_BoolF:
			case it_BoolT:
				{
					if( lpd && lpb[0] )
					{
						lpInt = (LPINT) lpd;
						if( IsYes( lpb ) )
							*lpInt = TRUE;
						else if( IsNo( lpb ) )
							*lpInt = FALSE;
						else
						{
							// NOT "yes" / "on" or "no" / "off"
							if( lpd )
							{
								// Set the DEFAULT
								lpInt = (LPINT) lpd;
								if( Typ == it_BoolF )
									*lpInt = FALSE;	// Default is FALSE
								else
									*lpInt = TRUE;		// Default is TRUE
							}
							// And SET changed
							*lpLst->i_Chg = TRUE;	// Mark as CHANGED
						}
					}
					else
					{
						if( lpd )
						{
							lpInt = (LPINT) lpd;
							if( Typ == it_BoolF )
								*lpInt = FALSE;	// Default is FALSE
							else
								*lpInt = TRUE;		// Default is TRUE
						}
						*lpLst->i_Chg = TRUE;	// Mark as CHANGED
					}
				}
				break;

			case it_Files:
				{
					LPHANDLE	lphdl;
					HGLOBAL		hg;
					//LPSTR		lpfl;
					//DWORD		dwoff, dwcnt;

					dwcnt = 0;
					dwoff = 0;
					if( lphdl = (LPHANDLE)lpd )
					{
						hg = *lphdl;
						/* INIT ghFileList, ghFindList, etc */
						if( hg == 0 )
						{
							hg = DVGlobalAlloc( GHND, (MXFILNAM * gdwMaxFiles) );
							*lphdl = hg;
						}
						/* ================================ */

						if( ( hg ) &&
							( lpfl = DVGlobalLock( hg ) ) )
						{
							lpdw = (LPDWORD)lpLst->i_Void;
							for( j = 0; j < gdwMaxFiles; j++ )
							{
								// Always do FULL LIST, in case of gaps
								wsprintf( lpb, lpi, (j+1) );
								GetStg( lps, lpb );
								if( i = lstrlen( lpb ) ) // If we GOT a FILE NAME
								{
									lstrcpy( (lpfl + dwoff), lpb );
									dwcnt++;
									dwoff += i + 1;
									lpfl[dwoff] = 0;
								}
							}
							DVGlobalUnlock( hg );
							lpfl = 0;
							if( lpdw )
								*lpdw = dwcnt;
							else
								*lpLst->i_Chg = TRUE;
						}
						else
						{
							*lpLst->i_Chg = TRUE;
						}
					}
					else
					{
						*lpLst->i_Chg = TRUE;
					}
				}	// case it_Files
				break;

			case it_Colr:
				{
					void (*lpvoid) (void);
					lpfm = &szSz3[0];
					if( lpb[0] &&
						(dv_wscanf( lpb, lpfm, lpb )) == 3 )
					{
						lpw = (LPWORD)lpb;
						if( ( lpw[0] < 256 ) &&
							( lpw[1] < 256 ) &&
							( lpw[2] < 256 ) )
						{
							// OK, appears VALID Color Reference
							*(COLORREF MLPTR)lpd = RGB( (BYTE)lpw[0],
								(BYTE)lpw[1],
								(BYTE)lpw[2] );
						}
						else
						{
							if( lpvoid = lpLst->i_Void )
								(*lpvoid)();
							*lpLst->i_Chg = TRUE;
						}
					}
					else	// Use DEFAULTS
					{
						if( lpvoid = lpLst->i_Void )
							(*lpvoid)();
						*lpLst->i_Chg = TRUE;
					}
				}	// case it_Colr
				break;

			case it_SizeW:
				{
#ifdef   USENEWWINSIZE
               PBOOL pb = (PBOOL)lpLst->i_Chg;  // extract POINTER to change flag
               if( ( IsYes(lpb) ) &&
                   ( GotWP( lps, lpLst->i_Deft, lpb, lpIni ) ) )
               {
                  // only if SAVED is yes, AND then success
                  pb = (PBOOL) lpLst->i_Void;
                  *pb = TRUE; // set that we have a (valid!) WINDOWPLACEMENT
               }
               else
                  *pb = TRUE;
#else // !#ifdef   USENEWWINSIZE
					// Set a Window Rectangle (with sanity check)
#ifdef	WIN32
					lpfm = &szSz4l[0];
#else
					lpfm = &szSz4[0];
#endif	// WIN#@ y/n
					if( lprc &&
						lpb[0] &&
						(dv_wscanf( lpb, lpfm, lpd )) == 4 )
					{
						if( !(( lprc->left < lprc->right ) &&
							( lprc->top < lprc->bottom ) &&
							( lprc->right > MMIN_CX ) &&
							( lprc->bottom > MMIN_CY )) )
						{
							lprc->left   = CW_USEDEFAULT; // Default horizontal pos.
							lprc->top    = CW_USEDEFAULT; // Default vertical position.
							lprc->right  = CW_USEDEFAULT; // Default width.
							lprc->bottom = CW_USEDEFAULT; // Default height.
							*lpLst->i_Chg = TRUE;
						}
					}
					else	// ~4 Use DEFAULTS
					{
						if( lprc == &rIniSiz )
						{
							lprc->left   = CW_USEDEFAULT; // Default horizontal pos.
							lprc->top    = CW_USEDEFAULT; // Default vertical position.
							lprc->right  = CW_USEDEFAULT; // Default width.
							lprc->bottom = CW_USEDEFAULT; // Default height.
						}
						*lpLst->i_Chg = TRUE;
					}
#endif // !#ifdef   USENEWWINSIZE y/n
				}
				break;

			case it_StgVal:
				{
					LPSTGVAL	lpsv;
					if( lpd && lpb[0] )
					{
						lpw = (LPWORD)lpd;
						if( lpsv = (LPSTGVAL)lpLst->i_Void )
						{
							while( lpsv->sv_Stg )
							{
								if( lstrcmpi( lpb, lpsv->sv_Stg ) == 0 )
									break;
								lpsv++;
							}
							if( lpsv->sv_Stg )
							{
								*lpw = (WORD)lpsv->sv_Val;
							}
							else
							{
								lpsv = (LPSTGVAL)lpLst->i_Void;
								*lpw = (WORD)lpsv->sv_Val;
								*lpLst->i_Chg = TRUE;
							}
						}
						else
						{
							*lpLst->i_Chg = TRUE;
						}
					}
					else
					{
						if( lpd )
						{
							lpw = (LPWORD)lpd;
							if( lpsv = (LPSTGVAL)lpLst->i_Void )
								*lpw = (WORD)lpsv->sv_Val;
						}
						*lpLst->i_Chg = TRUE;
					}
				}
				break;
			case it_Files2:
				{
					//PLIST_ENTRY phd;
					dw = 0;
               // this is the POINTER to the LIST HEADER
					if( pHead = (PLIST_ENTRY)lpd )
					{
                  if( lpdw = (PDWORD)lpLst->i_Res1 )
                     dwcnt = *lpdw;
                  else
                     dwcnt = DEF_MXFILCNT;
                  if( dwcnt == 0 )
                     dwcnt = 1;

                  // this is a pointer to the COUNT in the LIST
							lpdw = (LPDWORD)lpLst->i_Void;
							//for( j = 0; j < gdwMaxFiles; j++ )
							for( j = 0; j < dwcnt; j++ )
							{
								// Always do FULL LIST, in case of gaps
								wsprintf( lpb, lpi, (j+1) );
								GetStg( lps, lpb );
								if( i = lstrlen( lpb ) ) // If we GOT a FILE NAME
								{
                           // append to the TAIL of the list
                           // that puts File 1 first, File 2 2nd, etc ...
                           if( Add2LListTail( pHead, lpb ) )
                           {
                              dw++;
                           }
                           //else
                           //{
                                 // ONLY FAILURE IS MEMORY ALLOCATION!!!
                           //}
                        }
                     }

                     if( lpdw )
                        *lpdw = dw;    // set the COUNT
                     // NOTE: With LINKED LIST implementation
                     // PERHAPS we do NOT have to keep count!!!
                     // it was always a SANITY check only!!!!!
               }
            }
            break;
         case it_ListClip:
            // { szCL, szClip, it_ListClip, (PTSTR)&gClipList, &bChgCL, szCLFm, MX_CLIP_LIST },
				pHead = (PLIST_ENTRY)lpd;
            lpdw = (PDWORD)&lpLst->i_Res1;
            lpfm = lpLst->i_Void;
				lpw = (LPWORD)lpb;
            lpi = lpLst->i_Item;
            ptmp = GetStgBuf();
            dwcnt = 0;
            if(pHead)
            {
               if(lpdw)
                  dwcnt = *lpdw;
               if( dwcnt == 0 )
                  dwcnt = MX_CLIP_LIST;
               // Always do FULL LIST, in case of gaps
               lprc = (LPRECT)GetStgBuf();
               for( j = 0; j < dwcnt; j++ )
					{
                  wsprintf( ptmp, lpi, (j+1) );
                  GetStg( lps, ptmp );
					i = (DWORD)strlen(lpb); // If we GOT a RECT STRING
                  // if( i && ( dv_wscanf( lpb, lpfm, ptmp ) == 4 )) {
                  if( i && ( sscanf( lpb, lpfm, &lprc->left, &lprc->top, &lprc->right, &lprc->bottom ) == 4 )) {
                     PRECT prect;
                     pNext = (PLE)MALLOC(sizeof(CLIPLIST));
                     prect = &((PCLIPLIST)pNext)->clip;
                     *prect = *lprc;
                     InsertTailList( pHead, pNext );
                     dbgr_sprtf( "RECTDBG:it_ListClip:INI: Added RECT %s to list...(%d)\n", Rect2Stg(lprc), (j+1) );
                  } else {
   						*lpLst->i_Chg = TRUE;
                     if(i)
                        sprtf( "WARNING:it_ListClip: Failed to get 4 values from %s! CHECKME!!\n",lpb);
                  }
               }
            }
            break;

				// other TYPES

			}	// switch (Typ)
		}
		lpLst++;
	}
#ifdef	ADDCOLRF
	if( !fChgGBack &&
		!fChgGShad &&
		!fChgGHigh &&
		!fChgGText )
	{
		gDIBx.dx_Valid = TRUE;
	}
#endif	// ADDCOLRF
#ifdef	ADDLIBCFG
	if( fLOk )
	{
		if( ReadIniJPEG( lpIni, lpCfg ) ||
			( fSaf != fLSafety ) )
		{
			if( GetJPEGLib(8) && WSetLConfig ) // filled in, when Library loaded
			{
				lpCfg->cf_Safety_Check = fLSafety;
				(*WSetLConfig) ( lpCfg );	// Set Library accordingly
			}
		}
	}
#endif	// ADDLIBCFG
	CheckINIVals();
	if( lpNew )
	{
		if( lstrcmpi( lpIni, lpNew ) )
			gfChgAll = TRUE;
		lstrcpy( lpIni, lpNew ); // establish NEW
		WriteWININI( lpIni );
	}

   ReadPrtIni( lpIni, gfChgAll );
   ReadCustomColors( lpIni );
}

// general options item -
//	{ &szGens[0], &szAutoRLLast[0],it_BoolT,  (LPSTR)&gfAutoRLLast, &gfChgRLL, 0, 0 },
// and the AUTOS LIST
// 	{ &szAutos[0], &szAutoN[0], it_Files2, (LPSTR)&gsAutoList, &gbChgARL,  &gdwAutoCnt,
//            (DWORD) &gdwAutFiles },
// #define  flg_IsLoaded   0x00000004     // currently LOADED in a child window
//         pmwl->wl_dwFlag & flg_IsLoaded

VOID  SetAutoList( VOID )
{
   PLIST_ENTRY pHead, pNext, pList;
   PMWL  pmwl;

   pHead = &gsAutoList;
   if( IsListEmpty( pHead ) )
   {
      gbChgARL = FALSE;
   }
   else
   {
      gbChgARL = TRUE;
      FreeLList( pHead, pNext );
   }

   gdwAutoCnt = 0;   // no count left

   if( gfAutoRLLast )
   {
      pList = &gsFileList;    // get the MRU file list
      Traverse_List( pList, pNext )
      {
         pmwl = (PMWL)pNext;
         if( pmwl->wl_dwFlag & flg_IsLoaded )
         {
            if( Add2LListTail( pHead, &pmwl->wl_szFile[0] ) )
            {
               gdwAutoCnt++;
               if( gdwAutoCnt >= gdwAutFiles )
                  break;
            }
         }
      }
   }

   if( gdwAutoCnt )
      gbChgARL = TRUE;

}

// FIX20080316 - Use WINDOWPLACEMENT for window SIZING
#define  WI( a, b )\
   {  wsprintf(lpb, _T("%d"), b );\
      WritePrivateProfileString(lps, a, lpb, lpIni ); }

// ========================================================
// void	WriteIni( HWND hWnd )
//
// ========================================================
void	WriteIni( HWND hWnd )
{
	LPRECT	lprc;
	LPSTR	   lpIni;
	WORD	   i, j;
	PINILIST	lpLst;
	DWORD	   Typ;
	LPSTR	   lps, lpi, lpb, lpd, lpout;
	BOOL	   flg, aflg;
	LPWORD	lpw;
	LPDWORD	lpdw;
	LPINT	   lpInt;
   DWORD    dwi, dwc;
   PLIST_ENTRY pHead, pNext;
   LPTSTR   lprw;

   SetAutoList();

#ifndef   USENEWWINSIZE
	if( gfChgAll || fChgSiz )
	{
		GetWindowRect( hWnd, &rIniSiz );	// Set the FINAL window size/position
		// Is Window Maximised?
		bIsZoomed = IsZoomed( hWnd );
	}

	if( gfChgAll || fChgMax )
		GetWindowRect( GetDesktopWindow(), &rMaxSiz );
#endif // ifndef   USENEWWINSIZE

	lpIni = &gszIniFile[0];
	lpb = GetTmp2();	// &Buf[0];
	lpLst = &IniList[0];

	aflg = *lpLst->i_Chg;	// Get CHANGE ALL
	*lpLst->i_Chg = FALSE;	// and clear change all

	// ===========================================================
	if( !aflg &&
		!gfSavINI )	// If NOT change ALL, and NOT SAVE INI
	{
      // NOT All and NOT Save INI parameters - What a shame
		if( gfChgSI )	// But if it CHANGED on this run
		{
			lps = &szGens[0];
			lpi = &szSaveI[0];
			if( gfSavINI )
				lstrcpy( lpb, &szYes[0] );
			else
				lstrcpy( lpb, &szNo[0] );
			WritePrivateProfileString(
				lps,		// Section
				lpi,		// Res.Word
				lpb,		// String to write
				lpIni );	// File Name
		}
		gfChgSI = FALSE;
		goto DnWhile;
	}

	// ===========================================================
	if( aflg )
	{
		lpLst = &IniList[0];
		lps = 0;
		while( Typ = lpLst->i_Type )
		{
			if( (lpi = lpLst->i_Sect) && (lpi != lps) )
			{
				lps = lpi;

            // CLEAR ALL EXISTING ENTRIES FOR EACH SECTION
				WritePrivateProfileString(
					lps,		// Section
					NULL,		// Res.Word
					NULL,		// String to write
					lpIni );	// File Name
            // ===========================================
			}
			lpLst++; // bump to NEXT list entry
		}
	}

	// Now process the TABLE
	// =====================
	lpLst = &IniList[0];
	while( Typ = lpLst->i_Type )
	{
		lpd = lpLst->i_Deft;
		lprc = (LPRECT)lpd;
		if( ( lps = lpLst->i_Sect ) &&
			 ( lpi = lpLst->i_Item ) )
		{
			flg = *lpLst->i_Chg; // get CHANGE
			if( aflg || flg )    // if _ALL_ or changed
			{
				i = 0;
				switch( Typ )
				{
				case it_String:
				case it_Dir:
					{
						lpout = lpd;
						i = lstrlen( lpout );
					}
					break;

				case it_Size4:
					{
						// Assumed to be a RECT structure
						wsprintf( lpb, &szSz4[0], 
							lprc->left,	// Screen coordinates of top left,
							lprc->top,	// and Top, and
							lprc->right,	// Width and
							lprc->bottom );	// Height
						lpout = lpb;
						i = lstrlen( lpout );
					}
					break;

				case it_Size1:
					{
						if( lpd )
						{
							lpw = (LPWORD) lpd;
							wsprintf( lpb, &szSz1[0],
								*lpw );
							lpout = lpb;
							i = lstrlen( lpout );
						}
					}
					break;

				case it_Size2:
					{
						if( lpd )
						{
							lpdw = (LPDWORD) lpd;
							wsprintf( lpb, &szSz2[0],
								*lpdw );
							lpout = lpb;
							i = lstrlen( lpout );
						}
					}
					break;

				case it_BoolF:
				case it_BoolT:
					{
						if( lpd )
						{
							lpInt = (LPINT) lpd;
							if( *lpInt )
								lstrcpy( lpb, &szYes[0] );
							else
								lstrcpy( lpb, &szNo[0] );
							lpout = lpb;
							i = lstrlen( lpout );
						}
					}
					break;

				case it_Files:
					{
						LPHANDLE 	lphdl;
						HGLOBAL		hg;
						//DWORD		   dwc, dwoff;
						LPSTR		   lpfl;

						// First CLEAR SECTION
						// ===================
						WritePrivateProfileString(
							lps,		// Section
							NULL,		// Res.Word
							NULL,		// String to write
							lpIni );	// File Name

						dwi = 0;
						if( ( lpdw = lpLst->i_Void ) &&
							 ( lphdl = (LPHANDLE)lpd ) &&
							 ( dwc = *lpdw ) &&
							 ( hg = *lphdl ) &&
							 ( lpfl = DVGlobalLock( hg ) ) )
						{
							dwi = 0;
							lprw = GetTmp3();
							for( j = 0; j < dwc; j++ )
							{
								wsprintf( lprw, lpi, (j+1) );
								lpout = (lpfl + dwi);
								if( i = lstrlen( lpout ) )
								{
									WritePrivateProfileString(
										lps,		// Section
										lprw,		// Res.Word
										lpout,		// String to write
										lpIni );	// File Name
									dwi += i + 1;
								}
								else
									break;
							}
							DVGlobalUnlock( hg );
						}
						i = 0;	// All writing done
					}
					break;

				case it_Colr:
					{
						if( lpd )
						{
							wsprintf( lpb, &szSz3[0],
								(GetRValue(*(COLORREF MLPTR)lpd) & 0xff),
								(GetGValue(*(COLORREF MLPTR)lpd) & 0xff),
								(GetBValue(*(COLORREF MLPTR)lpd) & 0xff) );
							lpout = lpb;
							i = lstrlen( lpout );
						}
					}
					break;

				case it_SizeW:
					{
#ifdef   USENEWWINSIZE
                  // FIX20080316 - Use WINDOWPLACEMENT for window SIZING
                  PBOOL pb = (PBOOL)lpLst->i_Void;
                  WINDOWPLACEMENT * pwp = (WINDOWPLACEMENT *)lpLst->i_Deft;   // pDef
                  if( ( pwp->length == sizeof(WINDOWPLACEMENT) ) &&
                      ( ValidShowCmd( pwp->showCmd ) ) )
                  {
                     WI( szShow, pwp->showCmd );
                     WI( szMaxX, pwp->ptMaxPosition.x );
                     WI( szMaxY, pwp->ptMaxPosition.y );
                     WI( szMinX, pwp->ptMinPosition.x );
                     WI( szMinY, pwp->ptMinPosition.y );
                     WI( szLeft, pwp->rcNormalPosition.left );
                     WI( szTop,  pwp->rcNormalPosition.top  );
                     WI( szRite, pwp->rcNormalPosition.right);
                     WI( szBot,  pwp->rcNormalPosition.bottom);
                     lstrcpy(lpb, _T("Yes"));
                  }
                  else
                  {
                     lstrcpy(lpb, _T("No"));
                  }
#else // !#ifdef   USENEWWINSIZE
                  // GetWindowRect( hWnd, (LPRECT) lpd );
						// Convert window POSITION to
						// Left/Upper plus Width/Height
						lprc->right = lprc->right - lprc->left;
						lprc->bottom= lprc->bottom - lprc->top;
						wsprintf( lpb, &szSz4[0], 
							lprc->left,	// Screen coordinates of top left,
							lprc->top,	// and Top, and
							lprc->right,	// Width and
							lprc->bottom );	// Height
#endif // #ifdef   USENEWWINSIZE
						lpout = lpb;   // set OUPUT pointer
						i = lstrlen( lpout );   // and a LENGTH, to WRITE item
					}
					break;

					// ================================
				case it_StgVal:
					{
						LPSTGVAL	lpsv;
						if( lpd )
						{
							lpw = (LPWORD)lpd;
							if( lpsv = (LPSTGVAL)lpLst->i_Void )
							{
								while( lpsv->sv_Stg )
								{
									if( *lpw == (WORD) lpsv->sv_Val )
										break;
									lpsv++;
								}
								if( lpsv->sv_Stg )
								{
									lpout = lpsv->sv_Stg;
								}
								else
								{
									lpsv = (LPSTGVAL)lpLst->i_Void;
									lpout = lpsv->sv_Stg;
								}
								i = lstrlen( lpout );
							}
						}
						*lpLst->i_Chg = FALSE;
					}
					break;
					// ================================

            case it_Files2:
               {
                  //LPTSTR   lprw;
                  //DWORD    dwi, dwc;
						//PLIST_ENTRY pHead, pNext;
                  PMWL     pmwl;
						if( ( pHead = (PLIST_ENTRY)lpd ) &&
                      ( lpdw  = lpLst->i_Void    ) &&
                      ( dwc   = *lpdw            ) )
                  {
		   				// First CLEAR SECTION
	   					// ===================
   						WritePrivateProfileString(
							lps,		// Section
							NULL,		// Res.Word
							NULL,		// String to write
							lpIni );	// File Name
							lprw = GetTmp1();
                     dwi = 0;
                     Traverse_List(pHead,pNext)
                     {
								wsprintf( lprw, lpi, (dwi+1) );
                        pmwl = (PMWL)pNext;
								lpout = &pmwl->wl_szFile[0];
                        if( !( pmwl->wl_dwFlag & flg_DELETE ) &&
                             ( i = lstrlen( lpout )         ) )
								{
									WritePrivateProfileString(
										lps,		// Section
										lprw,		// Res.Word
										lpout,		// String to write
										lpIni );	// File Name
									dwi++;   // bump to NEXT number
								}
                     }
                  }
                  i = 0;
               }
               break;
            case it_ListClip:
               // { szCL, szClip, it_ListClip, (PTSTR)&gClipList, &bChgCL, szCLFm, MX_CLIP_LIST },
				   pHead = (PLIST_ENTRY)lpd;
               lpdw = (PDWORD)&lpLst->i_Res1;
               //lpfm = lpLst->i_Void;
				   lpw = (LPWORD)lpb;
					lpout = lpb;   // set OUPUT pointer
               dwc = 0;
					lprw = GetStgBuf();
               i = 0;
               if(pHead)
               {
                  PTSTR lpfm = lpLst->i_Void;
                  if(lpdw)
                     dwc = *lpdw;
                  if( dwc == 0 )
                     dwc = MX_CLIP_LIST;
	   				// First CLEAR SECTION
  						WritePrivateProfileString(
							lps,		// Section
							NULL,		// Res.Word
							NULL,		// String to write
							lpIni );	// File Name
                  j = 0;
                  Traverse_List(pHead,pNext)
					   {
                     PCLIPLIST pcl = (PCLIPLIST)pNext;
                     wsprintf( lprw, lpi, (j+1) );
                     wsprintf( lpout, lpfm, pcl->clip.left, pcl->clip.top,
                        pcl->clip.right, pcl->clip.bottom );
							WritePrivateProfileString(
										lps,		// Section
										lprw,		// Res.Word
										lpout,		// String to write
										lpIni );	// File Name
                     dbgr_sprtf( "RECTDBG:it_ListClip:OUTINI: Added RECT %s to INI ...(%d)\n", Rect2Stg(&pcl->clip), (j+1) );
                     j++;
                     if(j >= dwc)
                        break;
                  }
               }
               i = 0;
               break;

				}	// Switch per TYPE, and get output string (len in i )
				// ==========================================
				if( i )	// If an output string generated
				{
					WritePrivateProfileString(
						lps,		// Section
						lpi,		// Res.Word
						lpout,		// String to write
						lpIni );	// File Name
				}
				*lpLst->i_Chg = FALSE;
			}
		}
		lpLst++;
	}	// while items in list

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
	WriteIniJPEG( lpIni, aflg );
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

DnWhile:

	gdwFindCnt = 0;
   WritePrtIni( lpIni, aflg );   // NOTE: pass the ALL flag to the PRINTER section
   WriteCustomColors( lpIni );
	return;
}

BOOL	fInAdd2Fil = FALSE;

//#ifdef  USELLIST

void	AddToHistList( LPTSTR lpf );

PLE   GetLListEntry( PLE pHead, LPTSTR lpf )
{
   PLE      pNext, pRet;
   PMWL     pmwl;
   LPTSTR   lps;

   pRet = 0;
   Traverse_List( pHead, pNext )
   {
      pmwl = (PMWL)pNext;
      lps  = &pmwl->wl_szFile[0];
      if( lstrcmpi( lpf, lps ) == 0 )
      {
         pRet = pNext;
         break;
      }
   }

   return pRet;

}

BOOL  AddToLList( LPTSTR lpf, PLIST_ENTRY pHead, PDWORD pdwCnt, PBOOL pbChg, BOOL bMenu )
{
   BOOL     flg = FALSE;
   PLE      pNext;
   PMWL     pmwl, pmwln;
   LPTSTR   lps;
   DWORD    dwc = 0;

   pmwln = 0;
   Traverse_List( pHead, pNext )
   {
      pmwl = (PMWL)pNext;
      lps  = &pmwl->wl_szFile[0];
      if( lstrcmpi( lpf, lps ) == 0 )
      {
         RemoveEntryList( pNext );
         MFREE( pNext );
         if( pdwCnt )
         {
            dwc = *pdwCnt;
            if(dwc)
            {
               dwc--;
               *pdwCnt = dwc;
            }
         }
         if( pbChg )
            *pbChg = TRUE;
         break;   // ending traverse, so no problem with pNext now being INVALID!
      }
      dwc++;
   }

   if( bMenu )
      pmwln = Add2LListHead( pHead, lpf );
   else
      pmwln = Add2LListTail( pHead, lpf );
   if( pmwln )
   {
      flg = TRUE;
      if( pdwCnt )
      {
         DWORD dwc = *pdwCnt;
         dwc++;
         *pdwCnt = dwc;
      }
      if( pbChg )
         *pbChg = TRUE;

      // ===============================================
		if( bMenu )		// ONLY for FILE LIST
         AddMenuNames( pHead );	// Add FILENAMES to MENU ITEM
      // ===============================================
      flg = TRUE;
   }
   return flg;
}

PMWL  AddToLList4( PRDIB prd, PLIST_ENTRY pHead, PDWORD pdwCnt, PBOOL pbChg, BOOL bMenu )
{
   BOOL     flg = FALSE;
   PLE      pNext;
   PMWL     pmwl, pmwln;
   LPTSTR   lps;
   LPTSTR   lpf;
   DWORD    dwc = 0;

   pmwln = 0;
   if( lpf = prd->rd_pPath )
   {
      Traverse_List( pHead, pNext )
      {
         pmwl = (PMWL)pNext;
         lps  = &pmwl->wl_szFile[0];
         if( lstrcmpi( lpf, lps ) == 0 )
         {
            RemoveEntryList( pNext );
            MFREE( pNext );
            if( pdwCnt )
            {
               dwc = *pdwCnt;
               if(dwc)
               {
                  dwc--;
                  *pdwCnt = dwc;
               }
            }
            if( pbChg )
               *pbChg = TRUE;
            break;   // exit traverse so no problem that pNext is now INVALID
         }
         dwc++;
      }

      if( bMenu )
         pmwln = Add2LListHead( pHead, lpf );
      else
         pmwln = Add2LListTail( pHead, lpf );
      if( pmwln )
      {
         if( pmwln->wl_hMDI = prd->rd_hMDI )
         {
            pmwln->wl_dwFlag |= (flg_IsValid | flg_IsLoaded | flg_MDIOpen) ;
         }
         if( pdwCnt )
         {
            DWORD dwc = *pdwCnt;
            dwc++;
            *pdwCnt = dwc;
         }
         if( pbChg )
            *pbChg = TRUE;

         // ===============================================
		   if( bMenu )		// ONLY for FILE LIST
            AddMenuNames( pHead );	// Add FILENAMES to MENU ITEM
         // ===============================================
      }
   }
   return pmwln;
}


void	AddToHistList( LPTSTR lpf )
{
	AddToLList( lpf, &gsHistList, &gdwHistCnt, &gfChgHist, FALSE );
}

void	AddToFileList( LPTSTR lpf )
{
	if( fInAdd2Fil )
		return;
	fInAdd2Fil = TRUE;
	AddToLList( lpf, &gsFileList, &gdwFilCnt, &gfChgFil, TRUE );
	fInAdd2Fil = FALSE;
}

PMWL	AddToFileList4( PRDIB prd )
{
   PMWL  pmwl = 0;
	if( fInAdd2Fil )
		return 0;
	fInAdd2Fil = TRUE;
	pmwl = AddToLList4( prd, &gsFileList, &gdwFilCnt, &gfChgFil, TRUE );
	fInAdd2Fil = FALSE;
   return pmwl;
}


HWND	ChkFileOpen2( LPSTR lpf, PENUSTR lpes )
{
	HWND  hMDI;
	int		i;
	LPSTR	lps;

	hMDI = 0;
	if( lpf && lpes &&
		( i = lstrlen( lpf ) ) )
	{
		lpes->es_code = ENU_FINDFILES;
		lpes->es_hwnd = 0;
		lps = &lpes->es_string[0];
		strcpy( lps, lpf );
		EnumAllKids( lpes );
		if( lpes->es_hwnd )
		{
			// Got the CHILD with this FILE!!!
			hMDI = lpes->es_hwnd;
		}
	}
	return( hMDI );
}

HWND	ChkFileOpen( LPSTR lpf )
{
	HWND     hMDI;
	ENUSTR	es;

	hMDI = 0;
	if( lpf && *lpf )
	{
		hMDI = ChkFileOpen2( lpf, &es );
	}
	return( hMDI );
}

HWND	ChkFileOpen4( PMWL pmwl )
{
	HWND     hMDI = 0;
   HWND     h;
	ENUSTR	es;
   if( ( h = pmwl->wl_hMDI ) &&
       ( IsWindow( h )     ) )
   {
      hMDI = h;  // success
   }
   else
   {
      LPTSTR   lpf = &pmwl->wl_szFile[0];
      if( lpf && *lpf )
	   {
		   hMDI = ChkFileOpen2( lpf, &es );
	   }
   }
	return( hMDI );
}

BOOL	SetRDirName( LPSTR lpdf )
{
	BOOL	flg;
	char	buf[MAX_PATH+4];
	char	szDr[_MAX_DRIVE+4];
	char	szDi[_MAX_DIR+4];
	LPSTR	lpt;
	int		i, j;

	flg = FALSE;
	if( lpdf && lpdf[0] )
	{
		lpt = &buf[0];
		DVGetFPath( lpdf, szDr, szDi, NULL, NULL );
		lstrcpy( lpt, (LPSTR)szDr );
		lstrcat( lpt, (LPSTR)szDi );
		// Possible REMOVAL of final "\" char if present
		// which it should be, but NOT present on
		// the szRDirName ... and vice vers!!!
		if( ( i = lstrlen( lpt ) ) &&
			( j = lstrlen( &gszRDirName[0] ) ) )
		{
			i--;
			j--;
			if( lpt[i] == '\\' )
			{
				if( gszRDirName[0] != '\\' )
				{
					lpt[i] = 0;
				}
			}
			else
			{
				if( gszRDirName[0] == '\\' )
				{
					lstrcat( lpt, "\\" );
				}
			}
		}
		if( lstrcmpi( lpt, &gszRDirName[0] ) )
		{
			lstrcpy( &gszRDirName[0], lpt );
			gfChgOpen = TRUE;
			flg = TRUE;
		}
	}
	return( flg );
}

void	SetReadPath( LPSTR lpdf )
{
	SetRDirName( lpdf );
}

void	chknod( void )
{
	int i;
	i = 0;
}

// ============================================================
//
// DoAutoLoad( PLE pHead )    // PLIST_ENTRY
//
// Purpose: To re-load each of the PREVIOUS loaded files. If
//	it is an ANIMATED GIF, then add the Animation
//
// If a FILE does NOT exist in the LIST, then it will be DELETED
// from the LIST, and a NEW LIST created.
//
// This happens is during the FIRST Timer1 message, or at any
// time gfAutoLoad is SET and fDnAutoLoad is CLEARED
//
// ============================================================
//#ifdef  USELLIST

DWORD	DoAutoLoad( PLE pHead )
{
   DWORD    dwcnt = 0;
	PRDIB		prd;
   //PLE      pHead;
   PLE      pNext;
   BOOL     pncf;
   LPTSTR   lpf;
   PMWL     pmwl;
   DWORD    dwc1, dwc2, dwc3;
   PDI      pdi;
   HWND     hMDI;

	if( fInAutoLoad )
		return dwcnt;

	fInAutoLoad = TRUE;
	prd = (PRDIB)MALLOC( sizeof(RDIB) );
   if(!prd)
      chkme( "C:ERROR: No memory!!!"MEOR );
	NULPRDIB( prd );

   prd->rd_pTitle = gszRPTit;
   prd->rd_pPath  = gszRPNam;
   //pHead = &gsFileList;
	Hourglass( TRUE );
   pncf = fShwNoConv;
	fShwNoConv = TRUE;	// NO Show a FAILED Conversion

   // do the whole list
   // *****************
   dwc1 = dwc2 = dwc3 = 0;    // keep count of load attempts, successes and already loaded
   Traverse_List( pHead, pNext )
   {
      dwc1++;  // a load attempt
      pmwl = (PMWL)pNext;
      pmwl->wl_dwFlag |= flg_InAuto;   // being processed by AUTO LOAD
      lpf  = &pmwl->wl_szFile[0];
      strcpy( gszRPTit, lpf );
      DVGetFullName2( gszRPNam, gszRPTit );
		if( CheckIfFileExists( gszRPNam ) )
		{
         pmwl->wl_dwFlag |= flg_IsValid;  // have checked validity, and is valid
         if( ( gnDIBsOpen              ) &&	// If already HAVE CHILDREN
             ( hMDI = ChkFileOpen( gszRPNam ) ) )   // and is already open
         {
            // skip second open
            pmwl->wl_dwFlag |= flg_IsLoaded; // currently LOADED in a child window
            if( pdi = GETDI( hMDI ) )
            {
               if( pdi->di_psMWL != pmwl )
               {
                  chkme( "WARNING: pdi di_psMWL member is %#x. Setting %#x!"MEOR,
                     pdi->di_psMWL,
                     pmwl );
                   pdi->di_psMWL = pmwl;
               }
               RELDI( hMDI );
            }
            dwc3++;
         }
         else
         {
            HANDLE   hDIB;
            if( hDIB = GetDIB( gszRPNam, gd_AL ) )
            {
               prd->rd_hDIB   = hDIB;
					prd->rd_Caller = df_AUTOLOAD;
					hMDI = OpenDIBWindow2( prd );
               if( hMDI )
               {
                  pmwl->wl_hMDI    = hMDI;
                  pmwl->wl_dwFlag |= ( flg_MDIOpen | flg_IsLoaded );  // is in an OPEN MDI window
                  if( pdi = GETDI( hMDI ) )
                  {
                     if( ( pdi->di_psMWL         ) &&
                         ( pdi->di_psMWL != pmwl ) )
                     {
                        chkme( "WARNING: pdi di_psMWL member is %#x. Setting %#x!"MEOR,
                        pdi->di_psMWL,
                        pmwl );
                     }
                     pdi->di_psMWL = pmwl;
                     RELDI( hMDI );
                  }
                  dwc2++;  // count successful loads
               }
               else
               {
                  pmwl->wl_dwFlag |= flg_MDIFail;  // tried to open a MDI, but FAILED
               }
            }
            else
            {
               // failed to get handle to DIB (device independant bitmap)
               pmwl->wl_dwFlag |= flg_NoLoad;   // load attempted, but FAILED
            }
         }
      }
      else
      {
         // does NOT exist
         pmwl->wl_dwFlag |= flg_NotValid; // checked validity and NOT valid file
      }

      pmwl->wl_dwFlag &= ~( flg_InAuto );   // NOT being processed by AUTO LOAD
   }

   // === did we load anything ===
   if( dwcnt = dwc2 )  // if any got NEW loaded
   {
      SetFileMRU( ghFrameWnd, pHead );    // fix the MRU list to reflect the LOADED
   }
   if( ( dwc1 - (dwc2 + dwc3) ) )
   {
      sprtf( "WARNING: %d MRU failed to load!"MEOR,
         ( dwc1 - (dwc2 + dwc3) ) );
   }

   MFREE(prd);

   fShwNoConv = pncf;
	Hourglass( FALSE );
	fInAutoLoad = FALSE;

   return dwcnt;
}

//#endif   // !USELLIST

PMWL  Add2LListTail( PLIST_ENTRY pHead, LPTSTR lpb )
{
   PMWL  pmwl = 0;
   DWORD dwi;
   if( ( pHead && lpb      ) &&
       ( dwi = (DWORD)strlen(lpb) ) )
   {
      pmwl = MALLOC(sizeof(MWL)); 
      if(pmwl) 
      {
         ZeroMemory( pmwl, sizeof(MWL) );
         pmwl->wl_dwLen = dwi;
         strcpy( &pmwl->wl_szFile[0], lpb );
         InsertTailList( pHead, (PLIST_ENTRY) pmwl );
      }
   }
   return pmwl;
}

PMWL  Add2LListTail4( PLIST_ENTRY pHead, LPTSTR lpb, PBYTE pb, DWORD dwsz )
{
   PMWL  pmwl = 0;
   DWORD dwi;
   DWORD dwlen;
   PMWL  pmwl2;
   if( ( pHead && lpb      ) &&
       ( dwi = (DWORD)strlen(lpb) ) )
   {
      dwlen = sizeof(MWL);
      if( pb && dwsz )
         dwlen += dwsz;
      if( pmwl = MALLOC(dwlen) )
      {
         ZeroMemory( pmwl, sizeof(MWL) );
         pmwl->wl_dwLen = dwi;
         strcpy( &pmwl->wl_szFile[0], lpb );
         pmwl2 = pmwl + 1;
         memcpy( pmwl2, pb, dwsz );
         InsertTailList( pHead, (PLIST_ENTRY) pmwl );
      }
   }
   return pmwl;
}


PMWL  Add2LListHead( PLIST_ENTRY pHead, LPTSTR lpb )
{
   PMWL  pmwl = 0;
   DWORD dwi;
   if( ( pHead && lpb      ) &&
       ( dwi = (DWORD)strlen(lpb) ) )
   {
      if( pmwl = MALLOC(sizeof(MWL)) )
      {
         ZeroMemory( pmwl, sizeof(MWL) );
         pmwl->wl_dwLen = dwi;
         strcpy( &pmwl->wl_szFile[0], lpb );
         InsertHeadList( pHead, (PLIST_ENTRY) pmwl );
      }
   }
   return pmwl;
}


BOOL	ChkMFRFile( LPTSTR lpf )
{
	BOOL	   flg;
	DWORD	   wc1;
	WARNSTR	ws;
	UINT	   ui;
   PLE      pHead, pNext;
	LPSTR	   lpb, lptmp;
   PMWL     pmwl;

   flg = FALSE;
	if( ( lpf ) &&
		( *lpf ) &&
		( !CheckIfFileExists( lpf ) ) &&
		( wc1 = gdwFilCnt ) )
	{
		NULWARN(ws);
		lptmp = gszTmp;
      DVGetFileTitle( lpf, lptmp );
      pHead = &gsFileList;       // pointer to FILE LINKED LIST
      Traverse_List( pHead, pNext )
		{
         pmwl = (PMWL)pNext;
         lpb  = &pmwl->wl_szFile[0];
			if( lstrcmpi( lpf, lpb ) == 0 )
         {
            strcpy( gszRPTit, lpf );
            flg = TRUE;
         }
         else if ( lstrcmpi( lptmp, lpb ) == 0 )
         {
            strcpy( gszRPTit, lptmp );
            flg = TRUE;
         }
         if( flg )
			{
            // we have found the file in our list
				//flg = TRUE;
            //strcpy( gszRPTit, lpf );
            DVGetFullName2( gszRPNam, gszRPTit );
				wsprintf( lptmp,
					"The file [%s],"MEOR
					"which is in the Recent File List,"MEOR
               "path [%s]"MEOR
					"can now NOT be located for the load!"MEOR
					"Do you wish it DELETED from the file list?",
					gszRPTit,
               gszRPNam );
				ws.lpText     = lptmp;
				ws.bJustOK    = FALSE;
				ws.bCheck     = FALSE;
				ws.bChgCheck  = FALSE;
				ws.bAddCheck  = FALSE;
            ws.bAddBrowse = TRUE;
            ws.pszFile    = gszRPTit;
            ws.pszPath    = gszRPNam;
				ui = ShowWarning( GetFocus(), &ws );
				if( ui == IDOK )
				{
					ui = IDYES;
				}
				if( ui == IDYES )
				{
					// REDUCE file count
               RemoveEntryList( pNext );  // single entry removal
               // so no problems when pNext becomes INVALID
               AddToHistList( lpb );
               MFREE( pNext );
               if( gdwFilCnt )
                  gdwFilCnt--;
					// and SET CHANGE
					gfChgFil = TRUE;
               SetFileMRU( ghFrameWnd, pHead );
				}
				break;   // end this travers of list
			}
		}
	}
	return flg;
}

	
// eof - End DvIni.c
