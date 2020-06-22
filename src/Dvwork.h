
// DvWork.h
#ifndef	_DvWork_h
#define	_DvWork_h
// ensure DvGif.h included
#include "DvGif.h"

#define	MXFSB	   64
#define	MXFSBS	8

typedef	struct	{
	TCHAR	szBigBuf[2048];
	TCHAR szTmpBuf1[1024+4];
	TCHAR szTmpBuf2[1024+4];
	TCHAR szTmpBuf3[1024+4];
}ONEBUF;

// FIX20080720 - avoid multiple error dialogs
typedef struct tagLASTERR {
   TCHAR text[264];
   TCHAR title[264];
   UINT  type;
} LASTERR, * PLASTERR;

// GLOBAL ALLOCATED WORK STRUCTURE
typedef struct {

   BOOL        w_fDvInited;   // gfDvInited - All intialization has been DONE

	HINSTANCE	w_hDvInst;
	HWND		   w_hFrameWnd;	// Frame window
	HWND		   w_hMainWnd;
	HWND		   w_hWndMDIClient;	// MDI Client's window handle.
   HWND        w_hActMDIWnd;     // ghActMDIWnd
	HMENU		   w_hFrameMenu;		// Menu handle to frame menu.

   DWORD    w_dwDoTMsg;    // gdwDoTMsg

   BOOL     w_bInClose;       // had WM_CLOSE to FRAME - exitting app.
	int		w_nDIBsOpen;		// # of MDI childs currently open.
	char		w_szCWD[MAX_PATH+16];	// Current WORK directory

   GIFSTR   w_sGifStr;     // gsGif - global GIF structure

	int		w_iHPPI;	//  = GetDeviceCaps( hDC, LOGPIXELSX );
	int		w_iVPPI;	//  = GetDeviceCaps( hDC, LOGPIXELSY );

	char	   w_szTmpGif[MAX_PATH+16];
	HANDLE	   w_hGifFile;
	int		w_iTwipsPerInch;	//  = 1440;

	HGLOBAL	w_hDIBInfo;	// hDIBInfo;
	HGLOBAL	w_hDIBx;	// hDIBx;

   RECT     w_rcSize;   // =  grcSize - From frame WM_SIZE
	RECT	   w_rcClip;

	TCHAR    w_szRPTit[264];      // gszRPTit - title name - just file name part
	TCHAR    w_szRPNam[264];      // gszRPNam - full path name of file

	int		w_iFSBuf;
	char	   w_szFSBufs[ MXFSB * MXFSBS ];

	// Just some work pads
	TCHAR w_szDrive[_MAX_DRIVE+8];	// gszDrive - Drive
	TCHAR w_szDir[_MAX_DIR+8];		   // gszDir   - Directory
	TCHAR w_szFname[_MAX_FNAME+8];	// gszFname - Filename
	TCHAR	w_szExt[_MAX_EXT+8];		   // gszExt   - Extension
   TCHAR w_szNewName[264];          // gszNewName
   TCHAR w_szNewFile[264];          // gszNewFile

	TCHAR	w_szDIBFile[MAX_PATH+8];

	char	w_szDateTime[64];

	char	w_szDbgBmp[MAX_PATH+16];	// = { "TEMPD001.BMP" };

	char	w_szDefDiag[MAX_PATH];	// = "TEMPDIAG.TXT";

	DWORD	w_dwFilOff;
	DWORD	w_dwHistOff;
	DWORD	w_gdwFindOff;	// gdwFindOff

	DWORDLONG	w_dwlMenuFlag;
	BOOL	w_bAddSep;
	UINT	w_uiChecked;	// Item CHECKED on FILE MENU 

	DWORD	w_dwThreadBit;	// Next BIT for next thread
	DWORD	w_dwThreadSig;	// LIMIT fo 64 threads at any one time

   DWORD    w_dwLastMax;

	int		w_iVerbal;		// DIAG ouput verbosity
	BOOL	w_bChgVB;		// Change in verbosity

   DIBINFO  w_sDI;         // global DIBINFO structure

   DWORD    w_dwError;     // gdwError - system error value

   BOOL     w_bInSelWin;   // gbInSelWin

   DWORD w_dwDIBWidth;     // gdwDIBWidth -  last DIB Width
   DWORD w_dwDIBHeight;    // gdwDIBHeight - height of last DIB
   DWORD w_dwDIBBPP;       // gdwDIBBPP - Bits per pixel
   DWORD w_dwDIBColors;    // gdwDIBColors - last DIB color count

   BOOL  w_fInSearch;      // gfInSearch - We are processing a GROUP of files

   INT   w_iCurColor;      // giCurColor - current glazing color
   INT   w_iCurIndex;      // giCurIndex - current index - 0 to 255 cycle

   BOOL  w_bTmpCenter;     // gbTmpCenter - centre on print page
   HWND  w_hDlgAbort;      // ghDlgAbort - Handle to abort dialog box.
   BOOL  w_bAbort;         // gbAbort - user signals print abort

   OFSTRUCT       w_sOFS;  // gsOFS - General Open File Structure
   OPENFILENAME   w_OpnFN; // gOpnFN - Open file name structure
   WIN32_FIND_DATA   w_fd; // gfd - global FIND DATA structure
   PRINTDLG       w_sPD;   // gsPD
   TCHAR w_szDefPrtr[264]; // gszDefPrtr - name of DEFAULT printer

   TCHAR w_szFolder[264];  // gszFolder - IDM_OPEN - Selected file path name
   TCHAR w_szSelFile[264]; // gszSelFile - IDM_OPEN - Selected file path name
	TCHAR	w_gszTmp[1024];	// gszTmp
	TCHAR	w_gszMsg[1024];	// gszMsg
	TCHAR	w_gszTit[256];	// gszTit

   TCHAR w_szFile[264]; // gszFile - File name
   TCHAR w_szFileTitle[264];  // gszFileTitle  - File title buffer

   TCHAR    w_szDiag[1024];   // gszDiag - General diagnostic buffer
   TCHAR    w_szPText[264];   // gszPText - For PAL text preparation
   TCHAR    w_szSpl[264];     // gszSpl - colour from PALETTE (realised)
   TEXTMETRIC  w_sTM;         //  gsTM -  height, width of text at last check of HDC
   TCHAR    w_szStatus[264];  // = { 0 };

   TCHAR    w_szRunTime[264]; // gszRunTime - Runtime directory (for *.CLR files)
   TCHAR    w_szNxtClr[264];  // gszNxtClr - pad for building a ???.CLR file name

   TCHAR    w_szDirName[264]; // gszDirName - [MXDIR] - Directory name
   TCHAR    w_szIniFile[264]; // gszIniFile - [264] - INI file name
   TCHAR    w_szSchFil[264];  // gszSchFil - [MXFILNAM+8] - search file
   TCHAR    w_szSchMask[264]; // gszSchMask - [264] - find mask being used

   TCHAR    w_szTpBuf1[264];  // gszTpBuf1 - temp buffer 1
   TCHAR    w_szTpBuf2[264];  // gszTpBuf2 - temp buffer 2

   TCHAR    w_szChkme[1024];  // gszChkme
   TCHAR    w_szChkme2[1024];  // gszChkme2
   TCHAR    w_szSprtf[1024];  // gszSprtf
	ONEBUF	w_szOneBuf;			// Declare as ONE
   TCHAR w_szCGenBuf1[1024];  // gszCGenBuf1
   TCHAR w_szCGenBuf2[2048];  // gszCGenBuf2 - PPS pps = (PPS)&gszCGenBuf2[0];

   LASTERR w_lasterr;  // // FIX20080720 - last error dialog - avoid repeating

}WRKSTR, * PWS;

//extern	WRKSTR	WrkStr;
//#define		W		WrkStr
extern   PWS      pWS;
#define  W     (*pWS)

typedef  struct tagFIXEDWORK {

   // FIXED DATA STRUCTURE

	BOOL	   fw_bChgAll;    // gfChgAll - Change in ALL INI settings
   BOOL     fw_fChgOpen;   // gfChgOpen - change in READ/OPEN Folder
   BOOL     fw_fChgSave;   // gfChgSave - change in WRITE/SAVE Folder

   BOOL     fw_bBe_Tidy;   // gfBe_Tidy - clean up at end
   BOOL     fw_bChgBT;     // gfChgBT   - change in clean-up flag

   BOOL     fw_bAutoLoad;  // gfAutoLoad - reload full MRU list
   BOOL     fw_bChgAL;     // gfChgAL    - change in gfAutoLoad

   BOOL     fw_fAutoRLLast;   // gfAutoRLLast - auto reload last loaded
   BOOL     fw_fChgRLL;       // gfChgRLL - change in gfAutoRLLast

	DWORD	   fw_dwMaxFiles; // gdwMaxFiles - max. count of MRU list
	BOOL	   fw_bChgSC;		// gfChgSC - Change in MAX FILE count

	DWORD	   fw_dwHisFiles; // gdwHisFiles - max. count of HISTORY list
	BOOL	   fw_bChgMH;		// gfChgMH - Change in MAX HISTORY count
	DWORD	   fw_dwFndFiles; // gdwFndFiles - max. count of HISTORY list
	BOOL	   fw_bChgMF;		// gfChgMF - Change in MAX FIND count
	DWORD	   fw_dwMskFiles; // gdwMskFiles - max. count of HISTORY list
	BOOL	   fw_bChgMM;		// gfChgMM - Change in MAX MASK count

   BOOL     fw_bSavINI;    // gfSavINI - Save INI file
   BOOL     fw_bChgSI;     // gfChgSI  - change in SAVE INI file

	BOOL	fw_bStretch;		// = OPT_DEF_STRETCH;
//#ifdef  WIN32
   BOOL  fw_bPrtCenter;    // gfPrtCenter - center print on page
   BOOL  fw_bChgPrtCent;   // gfChgPrtCent - change in above
//#else // !WIN32
#ifndef  WIN32
	BOOL	fw_bPrinterBand;	// = OPT_DEF_BANDING;
	BOOL	fw_bUse31PrintAPIs;	// = OPT_DEF_USE31PRNAPIS;
	BOOL	fw_bChgPrinterBand;
	BOOL	fw_bChgPrintAPIs;
#endif   // WIN32 y/n

	DWORD	fw_wDispOption;	// = OPT_DEF_DISP;
	DWORD	fw_wPrintOption;	// = OPT_DEF_PRNSIZE;
	DWORD	fw_wXScale;		// = OPT_DEF_PRNSCALEX;
	DWORD	fw_wYScale;		// = OPT_DEF_PRNSCALEY;
	DWORD	fw_dwMilSecs;	// = OPT_DEF_MILSECS;
	BOOL	fw_bSetDefault;	// Set DEFAULT to these
	BOOL	fw_bApplyAll;	// Apply to ALL current
	BOOL	fw_bIsAGIF;		// Is an animation GIF ...
	BOOL	fw_bAnimate;		// Run animation (if NETSCAPE GIF) ...

	BOOL	fw_bChgStretch;
	BOOL	fw_bChgDispOption;
	BOOL	fw_bChgPrintOption;
	BOOL	fw_bChgXScale;
	BOOL	fw_bChgYScale;
	BOOL	fw_bChgMilSecs;
	BOOL	fw_bChgSetDefault;
	BOOL	fw_bChgApplyAll;
	BOOL	fw_bChgIsAGIF;
	BOOL	fw_bChgAnimate;

   BOOL  fw_bOpenWindow;   // gfOpenWindow
   BOOL  fw_bChgOW;        // gfChgOW

   BOOL  fw_bShowWarn;     // gfShowWarn
   BOOL  fw_bChgShowWarn;  // gfChgShowWarn

   DWORD fw_nPasteNum;  // gnPasteNum
   BOOL  fw_bChgPaste;  // gbChgPaste

   DWORD fw_dwFilCnt;   // gdwFilCnt
   DWORD fw_dwHistCnt;  // gdwHistCnt
   DWORD fw_gdwFindCnt; // gdwFindCnt
   DWORD fw_gdwMaskCnt; // gdwMaskCnt

   BOOL  fw_bChgFil;    // gfChgFil
   BOOL  fw_bChgHist;   // gfChgHist
   BOOL  fw_giFindChg;  // giFindChg
   BOOL  fw_bChgMsk;    // gfChgMsk
   BOOL  fw_fChgMask;   // gszChgMask - Change in Search folder (gszSearchDir)

	//{ &szAutos[0], &szAutoN[0], it_Files2, (LPSTR)&gsAutoList, &gbChgARL,  &gdwAutoCnt,
   //         (DWORD) &gdwAutFiles },
   DWORD fw_dwAutoCnt;  // gdwAutoCnt
   DWORD fw_dwAutFiles; // gdwAutFiles
   BOOL  fw_bChgARL;    // gbChgARL

   HFILE fw_hDiag;      // ghDiag

   DWORD fw_dwMaxCols;  // gdwMaxCols    = DEF_MAX_COLS;
   BOOL  fw_fChgMC;     // gfChgMC       = FALSE;

   BOOL  fw_fDoIter;    // gfDoIter - iterate into folders (DEF_ITER = TRUE)
   BOOL  fw_fChgIter;   // gfChgIter - change in iterate into folder option

   LIST_ENTRY  fw_sWL1;     // gsFileList - file list
   LIST_ENTRY  fw_sWL2;     // gsHistList - history list
   LIST_ENTRY  fw_sWL3;     // gsFindList - find list
   LIST_ENTRY  fw_sWL4;     // gsMaskList - mask list
   LIST_ENTRY  fw_sWL5;     // gsAutoList - auto reload list
   LE    fw_sLoadedList;   // gsLoadedList - loaded files, after WM_CREATE

   TCHAR    fw_szRDirName[264];  // gszRDirName - [MXDIR] - READ/Open Directory name
   TCHAR    fw_szWDirName[264];  // gszWDirName - [MXDIR] - WRITE/Save Directory name
   TCHAR    fw_szSearchDir[264]; // gszSearchDir - Search for files folder

}FIXEDWORK;

extern   FIXEDWORK   sFixedWork;    // definition of data - see DvData.c
#define  FW          sFixedWork

// macro definitions of FIXED work members
#define	gfChgAll		FW.fw_bChgAll    // INI
#define  gszRDirName FW.fw_szRDirName // [MXDIR] - READ/Open Directory name
#define  gszWDirName FW.fw_szWDirName // [MXDIR] - WRITE/Save Directory name
#define  gfChgOpen   FW.fw_fChgOpen  // change in READ/OPEN Folder
#define  gfChgSave   FW.fw_fChgSave  // change in WRITE/SAVE Folder

#define	gfBe_Tidy		FW.fw_bBe_Tidy  // clean up at end
#define	gfChgBT			FW.fw_bChgBT    // change in clean-up flag

#define  gfAutoLoad     FW.fw_bAutoLoad // reload full MRU list
#define  gfChgAL        FW.fw_bChgAL    // change in gfAutoLoad

#define  gfAutoRLLast   FW.fw_fAutoRLLast // auto reload last loaded
#define  gfChgRLL       FW.fw_fChgRLL     // change in gfAutoRLLast

#define	gdwMaxFiles		FW.fw_dwMaxFiles   // max. files in MRU list
#define	gfChgSC			FW.fw_bChgSC       // change in Max. MRU file length

#define	gdwHisFiles		FW.fw_dwHisFiles   // max. files in HISTORY list
#define	gfChgMH			FW.fw_bChgMH       // change in Max. history count
#define	gdwFndFiles		FW.fw_dwFndFiles   // max. files in FIND list
#define	gfChgMF			FW.fw_bChgMF       // change in Max. find count
#define	gdwMskFiles		FW.fw_dwMskFiles   // max. files in MASK list
#define	gfChgMM			FW.fw_bChgMM       // change in Max. MASK count

#define	gfSavINI		   FW.fw_bSavINI  // Save INI file
#define	gfChgSI			FW.fw_bChgSI   // change in SAVE INI file

#define	gfStretch		FW.fw_bStretch

//#ifdef  WIN32
#define	gfPrtCenter 	FW.fw_bPrtCenter  // center DIB on PRINT page
#define  gfChgPrtCent   FW.fw_bChgPrtCent // change in above
//#else // !WIN32
#ifndef  WIN32
// !!!!!!!!!!!!!!!!!!
#define	gfPrinterBand	FW.fw_bPrinterBand
#define	gfUse31PrintAPIs FW.fw_bUse31PrintAPIs
#define	gfChgPrinterBand FW.fw_bChgPrinterBand
#define	gfChgPrintAPIs	FW.fw_bChgPrintAPIs
// !!!!!!!!!!!!!!!!!!
#endif   // !WIN32

#define	gwDispOption	FW.fw_wDispOption
#define	gwPrintOption	FW.fw_wPrintOption
#define	gwXScale		   FW.fw_wXScale
#define	gwYScale		   FW.fw_wYScale
#define	gdwMilSecs		FW.fw_dwMilSecs
#define	gfSetDefault	FW.fw_bSetDefault
#define	gfApplyAll		FW.fw_bApplyAll
#define	gfIsAGIF		   FW.fw_bIsAGIF
#define	gfAnimate		FW.fw_bAnimate

#define	gfChgStretch	FW.fw_bChgStretch
#define	gfChgDispOption	FW.fw_bChgDispOption
#define	gfChgPrintOption FW.fw_bChgPrintOption
#define	gfChgXScale		FW.fw_bChgXScale
#define	gfChgYScale		FW.fw_bChgYScale
#define	gfChgMilSecs	FW.fw_bChgMilSecs
#define	gfChgSetDefault	FW.fw_bChgSetDefault
#define	gfChgApplyAll	FW.fw_bChgApplyAll
#define	gfChgIsAGIF		FW.fw_bChgIsAGIF
#define	gfChgAnimate	FW.fw_bChgAnimate

#define	gsFileList		FW.fw_sWL1    // file list
#define	gsHistList		FW.fw_sWL2    // history list
#define	gsFindList		FW.fw_sWL3    // find list
#define	gsMaskList		FW.fw_sWL4    // mask list
#define  gsAutoList     FW.fw_sWL5    // auto reload list
#define  gsLoadedList   FW.fw_sLoadedList // loaded files, after WM_CREATE

#define	gfOpenWindow	FW.fw_bOpenWindow
#define	gfChgOW			FW.fw_bChgOW

#define	gfShowWarn		FW.fw_bShowWarn
#define	gfChgShowWarn	FW.fw_bChgShowWarn

#define	gnPasteNum		FW.fw_nPasteNum
#define  gbChgPaste     FW.fw_bChgPaste

#define	gdwFilCnt		FW.fw_dwFilCnt
#define	gdwHistCnt		FW.fw_dwHistCnt
#define	gdwFindCnt		FW.fw_gdwFindCnt
#define	gdwMaskCnt		FW.fw_gdwMaskCnt

#define	gfChgFil		   FW.fw_bChgFil
#define	gfChgHist		FW.fw_bChgHist
#define	giFindChg		FW.fw_giFindChg
#define  gbChgMsk       FW.fw_bChgMsk     // Change in MASK count

//{ &szAutos[0], &szAutoN[0], it_Files2, (LPSTR)&gsAutoList, &gbChgARL,  &gdwAutoCnt,
//         (DWORD) &gdwAutFiles },
#define  gdwAutoCnt     FW.fw_dwAutoCnt   // count in auto reload list
#define  gdwAutFiles    FW.fw_dwAutFiles  // max. count for auto reload list
#define  gbChgARL       FW.fw_bChgARL     // change is gsAutoList

#define  gszSearchDir   FW.fw_szSearchDir // [264] Search for files folder
#define  gfChgMask      FW.fw_fChgMask    // Change in Search folder (gszSearchDir)

#define	ghDiag			FW.fw_hDiag

#define  gdwMaxCols     FW.fw_dwMaxCols   // = DEF_MAX_COLS
#define  gfChgMC        FW.fw_fChgMC      // = FALSE;

#define  gfDoIter       FW.fw_fDoIter  // iterate into folders (DEF_ITER = TRUE)
#define  gfChgIter      FW.fw_fChgIter // change in iterate into folder option

// DEFINE (ALLOCATED) GLOBAL Objects
// *********************************
#define  gfDvInited     W.w_fDvInited  // All intialization has been DONE
#define	ghDvInst		   W.w_hDvInst    // instance handle
#define	ghFrameWnd		W.w_hFrameWnd  // frame window
#define	ghMainWnd		W.w_hMainWnd
#define	ghWndMDIClient	W.w_hWndMDIClient // MDI client window - "MDICLIENT" class
#define	ghFrameMenu		W.w_hFrameMenu
#define	gnDIBsOpen		W.w_nDIBsOpen
#define	gszCWD			W.w_szCWD
#define  gbInClose      W.w_bInClose   // had WM_CLOSE to FRAME - exiting app.

#define  gszIniFile     W.w_szIniFile  // [264] - INI file name

#define  gsGif          W.w_sGifStr  // global GIF structure

#define	giHPPI			W.w_iHPPI
#define	giVPPI			W.w_iVPPI

#define	gszTmpGif		W.w_szTmpGif
#define	ghGifFile		W.w_hGifFile
#define	giTwipsPerInch	W.w_iTwipsPerInch

#define	ghDIBInfo		W.w_hDIBInfo
#define	ghDIBx			W.w_hDIBx

#define  grcSize        W.w_rcSize  // From frame WM_SIZE
#define	grcClip			W.w_rcClip

#define	gszRPTit		   W.w_szRPTit
#define	gszRPNam		   W.w_szRPNam

#define	giFSBuf			W.w_iFSBuf
#define	gszFSBufs		W.w_szFSBufs

// Just some work pads
#define	gszDrive		   W.w_szDrive
#define	gszDir			W.w_szDir
#define	gszFname		   W.w_szFname
#define	gszExt			W.w_szExt
#define  gszNewName     W.w_szNewName
#define  gszNewFile     W.w_szNewFile

#define	gszDIBFile		W.w_szDIBFile

// a combined buffer set
#define	gszOneBuf		W.w_szOneBuf
// individual buffers within set
#define  gszBigBuf      gszOneBuf.szBigBuf   // [2048];
#define  gszTmpBuf1     gszOneBuf.szTmpBuf1  // [1024+4];
#define  gszTmpBuf2     gszOneBuf.szTmpBuf2  // [1024+4];
#define  gszTmpBuf3     gszOneBuf.szTmpBuf3  // [1024+4];

#define  gszDiag        W.w_szDiag  // [1024];	// General diagnostic buffer

#define	gszDateTime		W.w_szDateTime

#define	gszDbgBmp		W.w_szDbgBmp
#define	gszDefDiag		W.w_szDefDiag

#define	gdwFilOff		W.w_dwFilOff
#define	gdwHistOff		W.w_dwHistOff
#define	gdwFindOff		W.w_gdwFindOff

#define	gdwLastMax		W.w_dwLastMax

#define	gdwlMenuFlag	W.w_dwlMenuFlag
#define	gfAddSep		   W.w_bAddSep
#define	guiChecked		W.w_uiChecked

// LIMIT fo 32 threads at any one time
#define	gdwThreadBit	W.w_dwThreadBit
#define	gdwThreadSig	W.w_dwThreadSig

// DIAG ouput verbosity
#define	giVerbal		   W.w_iVerbal
#define	gfChgVB			W.w_bChgVB

#define  gsOFS          W.w_sOFS    // (OFSTRUCT) General Open File Structure
#define  gOpnFN         W.w_OpnFN      // Open file name structure
#define  gszFolder      W.w_szFolder   // IDM_OPEN - Selected file path name
#define  gfd            W.w_fd         // global find data structure
#define  gszSelFile     W.w_szSelFile  // selected file

#define	gszTmp			W.w_gszTmp
#define	gszMsg			W.w_gszMsg
#define	gszTit			W.w_gszTit

#define  gdwError       W.w_dwError    // system error value

#define  gszFile        W.w_szFile     // [264] - File name
#define  gszFileTitle   W.w_szFileTitle   // 264 - File title buffer

// some temporary / last DIB values
#define  gdwDIBWidth    W.w_dwDIBWidth // last DIB Width
#define  gdwDIBHeight   W.w_dwDIBHeight   // height of last DIB
#define  gdwDIBBPP      W.w_dwDIBBPP   // Bits per pixel
#define  gdwDIBColors   W.w_dwDIBColors   // last DIB color count

#define  gfInSearch     W.w_fInSearch  /* We are processing a GROUP of files */

#define		VERBAL		( giVerbal > 0 )
#define		VERBAL2		( giVerbal > 1 )
#define		VERBAL3		( giVerbal > 2 )
#define		VERBAL4		( giVerbal > 3 )
#define		VERBAL5		( giVerbal > 4 )
#define		VERBAL6		( giVerbal > 5 )
#define		VERBAL7		( giVerbal > 6 )
#define		VERBAL8		( giVerbal > 7 )
#define		VERBAL9		( giVerbal > 8 )

#define  gszPText W.w_szPText    // [264] THCAR pad for PAL text preparation
#define  gszSpl   W.w_szSpl      // [264] -  colour from PALETTE (realised)
#define  gsTM     W.w_sTM        // height, width of text at last check of HDC
#define  gszStatus   W.w_szStatus   // [264] = { 0 };

#define  gszRunTime  W.w_szRunTime  // Runtime directory (for *.CLR files)
#define  gszNxtClr   W.w_szNxtClr   // pad for building a ???.CLR file name

#define  gszDirName  W.w_szDirName  // [MXDIR] - Directory name
#define  gszSchFil   W.w_szSchFil   // [MXFILNAM+8] - search file
#define  gszSchMask  W.w_szSchMask  // [264] - find mask being used
#define  gszTpBuf1   W.w_szTpBuf1   // [264] - temp buffer 1
#define  gszTpBuf2   W.w_szTpBuf2   // [264] - temp buffer 2

#define  gszChkme    W.w_szChkme    // [1024]
#define  gszChkme2   W.w_szChkme2   // [1024]
#define  gszSprtf    W.w_szSprtf    // [1024]
#define  gsPD        W.w_sPD        // PRINTDLG gsPD;
#define  gszDefPrtr  W.w_szDefPrtr  // [264] - name of DEFAULT printer
#define  gszCGenBuf1 W.w_szCGenBuf1 // [1024] - global general buffer 1
#define  gszCGenBuf2 W.w_szCGenBuf2 // [1024] - PPS pps = (PPS)&gszCGenBuf2[0];

#define  giCurColor  W.w_iCurColor  // current glazing color
#define  giCurIndex  W.w_iCurIndex  // current index - 0 to 255 cycle

#define  gbTmpCenter W.w_bTmpCenter // centre on print page
#define  ghDlgAbort  W.w_hDlgAbort  // Handle to abort dialog box.
#define  gbAbort     W.w_bAbort     // user signals print abort

#define gbInSelWin W.w_bInSelWin // g BOOL = ADDSELWIN2  = global 'sel' proc

#define gdwDoTMsg W.w_dwDoTMsg // g DWORD
#define g_lasterr W.w_lasterr  // // FIX20080720 - last error dialog - avoid repeating

#define ghActMDIWnd W.w_hActMDIWnd  // ghActMDIWnd

#endif	// _DvWork_h
// eof - DvWork.h
