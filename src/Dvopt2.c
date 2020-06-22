
/* =========================================================================

	File:  DVOPT2.C

	Purpose:  Contains the handlers for the "JPEG Options" Dialog box

	Comments:

   History:
	Date     Reason
	 5 March 1996	Created                           Geoff R. McLane
 * ======================================================================== */
#include	"dv.h"	/* Single, all inclusing include ... except 'specials' */
//#include "wjpeglib.h"
//#include	"WJPEG32\WJPEGLIB.h"
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
///////////////////////////////////////////////////////////////////

#define	USEDCFG		/* NEW Config - Using LIBRARY GET & SET ... */
#undef	ADDDIAGB

// external
extern	char	szYes[];	// = "Yes";
extern	char	szOn[];		// = "On";
extern	char	szNo[];		// = "No";
extern	char	szOff[];	//= "Off";
extern	char	szBlank[];	//= ""
extern	BOOL	IsYes( LPSTR );
extern	BOOL	IsNo( LPSTR );
extern	char	szSz1[];	//  = "%d";
extern	char	szSz2[];	//  = "%lu";

//#define	ChkDlg( a, b )	CheckDlgButton( hDlg, a, b )
//#define	IsChk( a )		IsDlgButtonChecked( hDlg, a )
//	{ &fSetQual, &fSQChg, IDO_CQUAL, QP_Default, &fQualNum, QP_DNumb, IDO_CQPERC  },
//typedef	struct tagOPTLIST	{
//	WORD	op_Type;
//	LPINT	op_Flag;
//	LPINT	op_Chg;
//	WORD	op_ID;
//	BOOL	op_Def;
//	LPWORD	op_Num;
//	WORD	op_DNum;
//	WORD	op_NID;
//	LPINT	op_Chgn;
//	LPSTR	opi_Sect;
//	LPSTR	opi_Item;
//	WORD	opi_Type;
//	LPSTR	opi_Deft;
//}OPTLIST;
//typedef	OPTLIST MLPTR LPOPTLIST;

//typedef struct tagLIBCONFIG {
//	/* Decompress - READ JPEG Configuration */
//	WORD dcf_struct_size;	/* Just SIZE of stucture ... */
//	WORD dcf_out_color; /* colorspace of output - Def=CS_RGB */
//	BOOL dcf_quantize_colors; /* T if output is a colormapped format - Def=F */
//	BOOL dcf_two_pass_quantize;	/* use two-pass color quantization? - Def=T */
//	BOOL dcf_use_dithering;		/* want color dithering? - Def=TRUE */
//	int  dcf_desired_colors;	/* max number of colors to use - Def=256 */
//	BOOL dcf_do_block_smoothing; /* T = apply cross-block smoothing - Def=F */
//	BOOL dcf_do_pixel_smoothing; /* T = apply post-upsampling smoothing - Def=F */
//	/* Compress - WRITE JPEG Configuration */
//	WORD ccf_jpeg_color;	/* Ouput color space. Def = CS_YCbCr */
//	WORD ccf_num_components;	/* Number of color components 1 for mono Def=3 */
//	WORD ccf_h_samp_factor;	/* Def is  2x2 subsamples of chrominance */
//	WORD ccf_v_samp_factor;	/* Both set to 1x1 for monochrome */
//	BOOL ccf_optimize_coding;	/* Huffman optimisation Def=FALSE */
//	WORD ccf_restart_in_rows;	/* Restart row count. Def=0 */
//	WORD ccf_restart_intevals;	/* Restart Huff MCU rows. Def=0 */
//	WORD ccf_smoothing_factor;	/* Def=0 Value 0 - 100 allowed */
//	WORD ccf_out_quality;	/* Output quality. Range 1 - 100. Def=75 */
//	/* Just a reserved area */
//	BYTE cf_reserved[32];	/* Reserved block - all ZERO */
//} LIBCONFIG;
//typedef LIBCONFIG MLPTR LPLIBCONFIG;

/* Internal DEFAULT Values are as follows - From WJPEGLIB.H */
LIBCONFIG	DLibConfig = {
	DefLibConfig };
/* PAD Used to Change Library CONFIGURATION */
LIBCONFIG	DVLibConfig = {
	DefLibConfig };

#ifdef	USEDCFG
#if	( defined( USEDCFG ) && defined( CVLIBBUG ) )
extern BOOL	GetJPEGLib( UINT caller );
extern void	CloseJPEGLib( void );

extern LPWGETLCONFIG	WGetLConfig; /* These are filled in, when the Library */
extern LPWSETLCONFIG	WSetLConfig;	/* has been successfully found and loaded */
#endif	/* If CVLIBBUG */

void	UnloadJLib( void );	/* If CVLIBBUG is ON then this does NOTHING! */
void	UnloadJLib2( void );	// Done in WRITE INI

/* In that case unloading the library is ONLY done at DESTROY ... */

/* Compression Parameter. ie BMP -> JPEG */
/* ===================================== */
#define	OG_Default	FALSE
BOOL	fOutGray = OG_Default;	/* If T MUST change ccf_num_components=1 */
	/* WORD ccf_h_samp_factor = 1 Def is  2x2 subsamples of chrominance */
	/*	WORD ccf_v_samp_factor = 1	Both set to 1x1 for monochrome */
BOOL	fOGChg = FALSE;

#define	UH_Default	FALSE
// BOOL	fUseHuff = UH_Default;
#define	fUseHuff		DVLibConfig.ccf_optimize_coding	/* Huffman opt Def=FALSE */
BOOL	fUHChg = FALSE;

#define	QP_Default	FALSE
#define	QP_DNumb		75
BOOL	fSetQual = QP_Default;
// WORD	fQualNum = QP_DNumb;
#define	fQualNum		DVLibConfig.ccf_out_quality /* Output qual. 1-100 Def=75 */
BOOL	fSQChg	= FALSE;

#define	CR_Default	FALSE
#define	CR_DNumb		0
BOOL	fCReset = CR_Default;
//WORD	fCRNum = CR_DNumb;
#define	fCRNum	DVLibConfig.ccf_restart_in_rows	/* Restart row count. Def=0 */
BOOL	fCRChg = FALSE;

#define	CB_Default	FALSE
#define	CB_DNumb		0
BOOL	fCBReset = CB_Default;
//WORD	fCBNum = CB_DNumb;
#define	fCBNum	DVLibConfig.ccf_restart_intevals /* Restart block Def=0 */
BOOL	fCBChg = FALSE;

/* Smoothing factor. Def=0 */
#define	CS_Default	FALSE
#define	CS_DNumb		75
BOOL	fCSmooth = CS_Default;
BOOL	fCSChg = FALSE;
//WORD	fCSNum = CS_DNumb;
#define	fCSNum	DVLibConfig.ccf_smoothing_factor /* Def=0 0-100 allowed */

/* DECOMPRESS Parameters. ie JPG -> BMP */
/* ==================================== */
#define	RC_Default	FALSE
#define	RC_DNumb		256
//BOOL	fRedCols = RC_Default;
#define	fRedCols		DVLibConfig.dcf_quantize_colors
BOOL	fRCChg = FALSE;
//int	fRCNum = RC_DNumb;
#define	fRCNum	DVLibConfig.dcf_desired_colors
#define	AC_Default	FALSE
//BOOL	fAppCblk = AC_Default;
#define	fAppCblk	DVLibConfig.dcf_do_block_smoothing
BOOL	fACChg = FALSE;
#define	IG_Default	FALSE
BOOL	fInGray = IG_Default;
BOOL	fIGChg = FALSE;
#define	UD_Default	FALSE
BOOL	fUseDith = UD_Default;
BOOL	fUDChg = FALSE;
#define	UQ_Default	FALSE
BOOL	fUseQuant = UQ_Default;
BOOL	fUQChg = FALSE;

/* General Parameters - moved to Option3.c */
//#define	BT_Default	TRUE
//BOOL	fBe_Tidy = BT_Default;
//BOOL	fBTChg = FALSE;

#else	/* !USEDCFG = Old method - Nothing to do with library ... */
//JPEGOPTIONS DIALOG 37, 34, 252, 234
//STYLE DS_MODALFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU
//CAPTION "JPEG Options"
//FONT 8, "MS Sans Serif"
//BEGIN
//    GROUPBOX        "Write JPEG (wcmaind.c)", -1, 4, 6, 239, 80
//    CONTROL         "Output JPEG Gray Scale Only.", IDO_CGRAY, "Button", 
#define	OG_Default	FALSE
BOOL	fOutGray = OG_Default;
BOOL	fOGChg = FALSE;
//                    BS_AUTOCHECKBOX | WS_TABSTOP, 11, 18, 230, 10
//    CONTROL         "Use highest Huffman compression.", IDO_CHUFF, "Button", 
//                    BS_AUTOCHECKBOX | WS_TABSTOP, 12, 31, 223, 10
#define	UH_Default	FALSE
BOOL	fUseHuff = UH_Default;
BOOL	fUHChg = FALSE;
//    CONTROL         "Set JPEG quality. Default is 75. Enter percentage ->", 
//                    IDO_CQUAL, "Button", BS_AUTOCHECKBOX | WS_TABSTOP, 12, 44, 186, 
//                    10
//    EDITTEXT        IDO_CQPERC, 202, 43, 32, 12, ES_AUTOHSCROLL
#define	QP_Default	FALSE
#define	QP_DNumb		75
BOOL	fSetQual = QP_Default;
WORD	fQualNum = QP_DNumb;
BOOL	fSQChg	= FALSE;
//    CONTROL         "Set restart interval in rows.                       Use ->", 
//                    IDO_CREST, "Button", BS_AUTOCHECKBOX | WS_TABSTOP, 12, 58, 181, 
//                    10
//    EDITTEXT        IDO_CRNUM, 202, 56, 32, 12, ES_AUTOHSCROLL
#define	CR_Default	FALSE
#define	CR_DNumb		2
BOOL	fCReset = CR_Default;
WORD	fCRNum = CR_DNumb;
BOOL	fCRChg = FALSE;
//    CONTROL         "Smooth dithered input (1-100)                   Use ->", 
//                    IDO_CSMTH, "Button", BS_AUTOCHECKBOX | WS_TABSTOP, 12, 71, 185, 
//                    10
//    EDITTEXT        IDO_CSNUM, 202, 69, 32, 12, ES_AUTOHSCROLL
//
#define	CS_Default	FALSE
#define	CS_DNumb		75
BOOL	fCSmooth = CS_Default;
BOOL	fCSChg = FALSE;
WORD	fCSNum = CS_DNumb;
//    GROUPBOX        "Read JPEG (wdmaind.c)", -1, 5, 89, 237, 82
//    CONTROL         "Reduce image colors.                              Use ->", 
//                    IDO_DRIC, "Button", BS_AUTOCHECKBOX | WS_TABSTOP, 12, 100, 
//                    182, 10
//    EDITTEXT        IDO_DRNUM, 202, 99, 32, 12, ES_AUTOHSCROLL
#define	RC_Default	FALSE
#define	RC_DNumb		256
BOOL	fRedCols = RC_Default;
BOOL	fRCChg = FALSE;
int		fRCNum = RC_DNumb;
//    CONTROL         "Apply cross block smoothing.", IDO_DCBS, "Button", 
//                    BS_AUTOCHECKBOX | WS_TABSTOP, 12, 113, 185, 10
#define	AC_Default	FALSE
BOOL	fAppCblk = AC_Default;
BOOL	fACChg = FALSE;
//    CONTROL         "Force Grayscale output.", IDO_DGRAY, "Button", 
//                    BS_AUTOCHECKBOX | WS_TABSTOP, 12, 127, 186, 10
#define	IG_Default	FALSE
BOOL	fInGray = IG_Default;
BOOL	fIGChg = FALSE;
//    CONTROL         "Do not use dithering in quantization.", IDO_DNODIT, "Button", 
//                    BS_AUTOCHECKBOX | WS_TABSTOP, 11, 140, 199, 10
#define	UD_Default	FALSE
BOOL	fUseDith = UD_Default;
BOOL	fUDChg = FALSE;
//    CONTROL         "Use 1-pass quantization. (Fast, low quality.)", IDO_D1PASS, 
//                    "Button", BS_AUTOCHECKBOX | WS_TABSTOP, 12, 154, 201, 10
#define	UQ_Default	FALSE
BOOL	fUseQuant = UQ_Default;
BOOL	fUQChg = FALSE;
//    GROUPBOX        "General Options", -1, 6, 174, 235, 32
//    CONTROL         "Be Tidy. Remove intermediate files", IDO_GTIDY, "Button", 
//                    BS_AUTOCHECKBOX | WS_TABSTOP, 11, 186, 221, 10
#define	BT_Default	TRUE
//BOOL	fBe_Tidy = BT_Default;
BOOL	fBTChg = FALSE;
//    DEFPUSHBUTTON   "OK", IDOK, 22, 212, 40, 14
//    PUSHBUTTON      "Defaults", IDDEFAULT, 100, 212, 40, 14
//    PUSHBUTTON      "Cancel", IDCANCEL, 183, 212, 40, 14
//END
#endif	/* USEDCFG y/n (no is OLD non-library method */

//BOOL MEXPORTS OPTION2DLG(HWND, unsigned, WORD, LONG);
INT_PTR CALLBACK OPTION2DLG(
  HWND hDlg,  // handle to dialog box
  UINT message,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
);
BOOL	Option2Term( HWND );
BOOL	Option2Init( HWND, LPARAM );
BOOL	Option2Def( HWND );

/* [JPEG Decompression] Section */
char	szJDecomp[] = "JPEG Decompression";
	/* DeCompress - READ JPEG Configuration */
//	WORD dcf_out_color; /* colorspace of output - Def=CS_RGB */
// &szJDecomp[0], &szDecColOut[0], it_Table, (LPSTR)&DVLibConfig.dcf_out_color },
char szDecColOut[] = "Decompress Color";
//	BOOL dcf_quantize_colors; /* T if output is a colormapped format - Def=F */
// &szJDecomp[0], &szDecQuant[0], it_BoolF, (LPSTR)&DVLibConfig.dcf_quantize_colors },
char szDecQuant[] = "Quantize Colors";
//	BOOL dcf_two_pass_quantize;	/* use two-pass color quantization? - Def=T */
// &szJDecomp[0], &szDec2Pass[0], it_BoolT, (LPSTR)&DVLibConfig.dcf_two_pass_quantize },
char szDec2Pass[] = "Two Pass";
//	BOOL dcf_use_dithering;		/* want color dithering? - Def=TRUE */
// &szJDecomp[0], &szDecDith[0], it_BoolT, (LPSTR)&DVLibConfig.dcf_use_dithering },
char szDecDith[] = "Use Dithering";

//	int  dcf_desired_colors;	/* max number of colors to use - Def=256 */
// &szJDecomp[0], &szDecCols[0], it_Size2, (LPSTR)&DVLibConfig.dcf_desired_colors },
char szDecCols[] = "Desired Colors";
//	BOOL dcf_do_block_smoothing; /* T = apply cross-block smoothing - Def=F */
// &szJDecomp[0], &szDecBSmooth[0], it_BoolF, (LPSTR)&DVLibConfig.dcf_do_block_smoothing },
char szDecBSmooth[] = "Block Smoothing";
//	BOOL dcf_do_pixel_smoothing; /* T = apply post-upsampling smoothing - Def=F */
// &szJDecomp[0], &szDecPSmooth[0], it_BoolF, (LPSTR)&DVLibConfig.dcf_do_pixel_smoothing },
char szDecPSmooth[] = "Pixel Smoothing";

/* [JPEG Compression] Section */
/* ========================== */
char	szJComp[] = "JPEG Compression";
	/* Compress - WRITE JPEG Configuration */

//	WORD ccf_jpeg_color;	/* Ouput color space. Def = CS_YCbCr */
// &szJComp[0], &szComColor[0], it_Table, (LPSTR)&DVLibConfig.ccf_jpeg_color },
char	szComColor[] = "Compression Color";

//	WORD ccf_num_components;	/* Number of color components 1 for mono Def=3 */
// &szJComp[0], &szComNumComp[0], it_Size1, (LPSTR)&DVLibConfig.ccf_num_components },
char	szComNumComp[] = "Components";
//	WORD ccf_h_samp_factor;	/* Def is  2x2 subsamples of chrominance */
// &szJComp[0], &szComHSampFac[0], it_Size1, (LPSTR)&DVLibConfig.ccf_h_samp_factor },
char szComHSampFac[] = "Hoz. Sample Factor";
//	WORD ccf_v_samp_factor;	/* Both set to 1x1 for monochrome */
// &szJComp[0], &szComVSampFac[0], it_Size1, (LPSTR)&DVLibConfig.ccf_v_samp_factor },
char szComVSampFac[] = "Vert. Sample Factor";

//	BOOL ccf_optimize_coding;	/* Huffman optimisation Def=FALSE */
// &szJComp[0], &szComOptHuff[0], it_BoolF, (LPSTR)&DVLibConfig.ccf_optimize_coding },
char szComOptHuff[] = "Optimize Huffman";

//	WORD ccf_restart_in_rows;	/* Restart row count. Def=0 */
// &szJComp[0], &szComRestRow[0], it_Size1, (LPSTR)&DVLibConfig.ccf_restart_in_rows },
char szComRestRow[] = "Restart Rows";

//	WORD ccf_restart_intevals;	/* Restart Huff MCU rows. Def=0 */
// &szJComp[0], &szComRestInt[0], it_Size1, (LPSTR)&DVLibConfig.ccf_restart_intevals },
char szComRestInt[] = "Restart Interval";

//	WORD ccf_smoothing_factor;	/* Def=0 Value 0 - 100 allowed */
// &szJComp[0], &szComSmooth[0], it_Size1, (LPSTR)&DVLibConfig.ccf_smoothing_factor },
char szComSmooth[] = "Smooth Factor";

//	WORD ccf_out_quality;	/* Output quality. Range 1 - 100. Def=75 */
// &szJComp[0], &szComQual[0], it_Size1, (LPSTR)&DVLibConfig.ccf_out_quality },
char szComQual[] = "Out Quality";


char	szOption2Dlg[] = "JPEGOPTIONS";
typedef	struct tagCCOLTAB {
	WORD	ct_Type;
	char *	ct_Stg;
}CCOLTAB;
typedef CCOLTAB FAR * PCCOLTAB;

CCOLTAB	CColTab[] = {
	{ CS_CMYK,      "CS_CMYK" },
	{ CS_YIQ,       "CS_YIQ" },
	{ CS_YCbCr,     "CS_YCbCr" },
	{ CS_RGB,       "CS_RGB" },
	{ CS_GRAYSCALE, "CS_GRAYSCALE" },
	{ CS_UNKNOWN,   "CS_UNKNOWN" } };

//typedef	struct tagOPTLIST	{
//	WORD	op_Type;
//	LPINT	op_Flag;
//	LPINT	op_Chg;
//	WORD	op_ID;
//	BOOL	op_Def;
//	LPWORD	op_Num;
//	WORD	op_DNum;
//	WORD	op_NID;
//	LPINT	op_Chgn;
//	LPSTR	opi_Sect;
//	LPSTR	opi_Item;
//	WORD	opi_Type;
//	LPSTR	opi_Deft;
//}OPTLIST;
//typedef	OPTLIST MLPTR LPOPTLIST;
//  dw op_Type,ptr op_Flag,ptr op_Chg,dw op_ID,flg op_Def,ptr op_Num,dw op_DNum,dw op_NID,ptr op_Chgn 
#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

OPTLIST	OptList[] = {

	/* Compression BMP -> JPEG */
	{ it_General, &fOutGray, &fOGChg, IDO_CGRAY, OG_Default, 0, 0, 0, 0, &szJComp[0], &szComColor[0], it_Table, (LPSTR)&DVLibConfig.ccf_jpeg_color },
	{ it_General, &fUseHuff, &fUHChg, IDO_CHUFF, UH_Default, 0, 0, 0, 0, &szJComp[0], &szComOptHuff[0], it_BoolF, (LPSTR)&DVLibConfig.ccf_optimize_coding },
	{ it_General, &fSetQual, &fSQChg, IDO_CQUAL, QP_Default, &fQualNum, QP_DNumb, IDO_CQPERC, 0, &szJComp[0], &szComQual[0], it_Size1, (LPSTR)&DVLibConfig.ccf_out_quality },
	{ it_General, &fCReset, &fCRChg, IDO_CREST, CR_Default, &fCRNum, CR_DNumb, IDO_CRNUM, 0, &szJComp[0], &szComRestRow[0], it_Size1, (LPSTR)&DVLibConfig.ccf_restart_in_rows },
	{ it_General, &fCBReset, &fCBChg, IDO_CBBLK, CB_Default, &fCBNum, CB_DNumb, IDO_CBNUM, 0, &szJComp[0], &szComRestInt[0], it_Size1, (LPSTR)&DVLibConfig.ccf_restart_intevals },
	{ it_General, &fCSmooth, &fCSChg, IDO_CSMTH, CS_Default, &fCSNum, CS_DNumb, IDO_CSNUM, 0, &szJComp[0], &szComSmooth[0], it_Size1, (LPSTR)&DVLibConfig.ccf_smoothing_factor },

	/* Decompression JPEG -> BMP */
	{ it_General, &fRedCols, &fRCChg, IDO_DRIC, RC_Default,(PWORD)&fRCNum,RC_DNumb,IDO_DRNUM, 0, &szJDecomp[0], &szDecQuant[0], it_BoolF, (LPSTR)&DVLibConfig.dcf_quantize_colors },
	{ it_General, &fAppCblk, &fACChg, IDO_DCBS, AC_Default, 0, 0, 0, 0, &szJDecomp[0], &szDecBSmooth[0], it_BoolF, (LPSTR)&DVLibConfig.dcf_do_block_smoothing },
	{ it_General, &fInGray, &fIGChg, IDO_DGRAY, IG_Default, 0, 0, 0, 0, &szJDecomp[0], &szDecColOut[0], it_Table, (LPSTR)&DVLibConfig.dcf_out_color },
	{ it_General, &fUseDith, &fUDChg, IDO_DNODIT, UD_Default, 0,0,0, 0, &szJDecomp[0], &szDecDith[0], it_BoolT, (LPSTR)&DVLibConfig.dcf_use_dithering },
	{ it_General, &fUseQuant, &fUQChg, IDO_D1PASS, UQ_Default, 0,0,0,0, &szJDecomp[0], &szDec2Pass[0], it_BoolT, (LPSTR)&DVLibConfig.dcf_two_pass_quantize },

	// Compression to be completed
	{ it_General,         0, &fOGChg,         0,          0,       0,        0,         0, 0, &szJComp[0], &szComNumComp[0], it_Size1, (LPSTR)&DVLibConfig.ccf_num_components },
	{ it_General,         0, &fOGChg,         0,          0,       0,        0,         0, 0, &szJComp[0], &szComHSampFac[0], it_Size1, (LPSTR)&DVLibConfig.ccf_h_samp_factor },
	{ it_General,         0, &fOGChg,         0,          0,       0,        0,         0, 0, &szJComp[0], &szComVSampFac[0], it_Size1, (LPSTR)&DVLibConfig.ccf_v_samp_factor },

	// Additional Decompression items
	{ it_General,         0, &fRCChg,        0,          0,             0,       0,        0, 0, &szJDecomp[0], &szDecCols[0], it_Size2, (LPSTR)&DVLibConfig.dcf_desired_colors },
	{ it_General,         0,       0,        0,          0,             0,       0,        0, 0, &szJDecomp[0], &szDecPSmooth[0], it_BoolF, (LPSTR)&DVLibConfig.dcf_do_pixel_smoothing },

	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

/* ================================================================
 *
 * Function:   ShowOption2
 * Purpose:    Brings up option2 dialog box
 * Parms:      hWnd    == Handle to options dialog box's parent.
 * History:
 *   Date      Reason
 *  5 March 96 Created
 *
 * ================================================================ */

void ShowOption2( HWND hWnd )
{
#ifndef	WIN32
	FARPROC lpProcOpt;
#endif	/* !WIN32 */
#ifdef	ADDDIAGB
	MessageBox( hWnd, "Begin of Options Dialog!", NULL, MB_OK );
#endif
#ifdef	WIN32
   DialogBoxParam( ghDvInst,
                  szOption2Dlg, // char sz[] = "JPEGOPTIONS";
                  hWnd,
                  OPTION2DLG,	// lpProcOpt,
                  (LPARAM)NULL);
#else	/* !WIN32 */
	lpProcOpt = MakeProcInstance( OPTION2DLG, ghDvInst );
   DialogBoxParam( ghDvInst,
                  szOption2Dlg,
                  hWnd,
                  lpProcOpt,
                  (LPARAM)NULL);
   FreeProcInstance( lpProcOpt );
#endif	/* WIn32 y.n */
#ifdef	ADDDIAGB
	MessageBox( hWnd, "End of Options Dialog!", NULL, MB_OK );
#endif	
}

/* =====================================================================
 * OPTION2DLG( ... )
 * ===================================================================== */
//BOOL MEXPORTS OPTION2DLG (HWND hDlg, unsigned message, 
//	WORD wParam, LONG lParam)
INT_PTR CALLBACK OPTION2DLG( // char	szOption2Dlg[] = "JPEGOPTIONS";
  HWND hDlg,  // handle to dialog box
  UINT message,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
   DWORD cmd = LOWORD(wParam);
	switch( message )
	{
		case WM_INITDIALOG:
			return Option2Init( hDlg, lParam );

		case WM_COMMAND:
         {
			   switch( cmd ) {
            case IDDEFAULT:
               Option2Def( hDlg );
				   break;
            case IDOK:
            case IDD_OK:
               if( !Option2Term( hDlg ) )	/* Get values, and fall through CANCEL */
                  break;	/* If NO TRUE, then stay in Dialog to try fix */
            case IDCANCEL:
            case IDD_CANCEL:
   #if defined(USEDCFG) && defined(ADD_JPEG_SUPPORT)
               UnloadJLib();	/* Reduce instance count of library */
   #endif
				   EndDialog(hDlg, TRUE);
				   return( TRUE );
			   }
         }
         break;
	}
   return( FALSE );
}

//	LPINT	op_Flag;
//	LPINT	op_Chg;
//	WORD	op_ID;
//	BOOL	op_Def;
//	PWORD	op_Num;
//	WORD	op_DNum;
//	WORD	op_NID;

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

BOOL	Option2Def( HWND hDlg )
{
BOOL	flg;
LPOPTLIST	lpo;
	lpo = &OptList[0];
	PutDefOpt( hDlg, lpo );
	flg = TRUE;
return( flg );
}

#ifdef	USEDCFG
#ifndef	CVLIBBUG
/* Load WJPEG.DLL instance data */
#ifdef	LDYNALIB
HINSTANCE	hLibI = 0;
// = DEF_JPEG_LIB;   // 32-bit = WJPEG32.DLL
// extern	char szJpegLib[];	/* Ascii string being NAME of Library */
char	szWGetLConfig[] = "WGETLCONFIG";	/* Names of 2 headers used */
char	szWSetLConfig[] = "WSETLCONFIG";	/* by this module ... */
LPWGETLCONFIG	WGetLConfig = 0;	/* These are filled in, when the Library */
LPWSETLCONFIG	WSetLConfig = 0;	/* has been successfully found and loaded */
#endif	// LDYNALIB
#endif	/* !CVLIBBUG */

/* =====================================================================
 * LoadJLib( ... )
 * If previously loaded, return TRUE for OK, else
 * try to LOAD Library into memory, and resolve the two used headers.
 * RETURN = TRUE if OK, else
 *          FALSE = Load failed for some reason ...
 * ===================================================================== */
BOOL	LoadJLib( HWND hDlg )
{
#ifdef	LDYNALIB
	BOOL	flg = FALSE;
#ifdef	CVLIBBUG
	// FULLY RE-OPEN - Loading all the OFFSETS again
	// =============
	flg = GetJPEGLib(12);
#else	/* !CVLIBBUG */
//	if( (hLibI > HINSTANCE_ERROR) &&
//		WGetLConfig &&
//		WSetLConfig )
//	{
//		flg = TRUE;	/* Already previously loaded ... */
//	}
//	else if( (hLibI = LoadLibrary( &szJpegLib[0] )) > HINSTANCE_ERROR )
//	{
//		if( WGetLConfig = (LPWGETLCONFIG) GetProcAddress( hLibI, &szWGetLConfig[0] ) )
//		{
//			if( WSetLConfig = (LPWSETLCONFIG) GetProcAddress( hLibI, &szWSetLConfig[0] ) )
//			{
//				flg = TRUE;
//			}
//		}
//	}
#endif	/* CVLIBBUG y/n */
#else	// !LDYNALIB
	BOOL flg = TRUE;
#endif	// LDYNALIB y/n
	return( flg );
}

/* ===================================================================
 *	UnloadJLib() - Reduce instance count of Library 
 * =================================================================== */
void	UnloadJLib( void )
{
#ifdef	LDYNALIB
#ifndef	CVLIBBUG		/* If this is ON,then ONLY released at END ... */
	if( hLibI > MYHERROR )  // was HINSTANCE_ERROR
		FreeLibrary( hLibI );
	hLibI = 0;
	WGetDConfig = 0;
	WSetDConfig = 0;
#endif	/* CVLIBBUG */
#endif	// LDYNALIB
}

void	UnloadJLib2( void )
{
#ifdef	LDYNALIB
#ifdef	CVLIBBUG
	CloseJPEGLib();		// FULL-CLOSE = Unload.
#else	// !CVLIBBUG
//	if( hLibI > HINSTANCE_ERROR )
//		FreeLibrary( hLibI );
//	hLibI = 0;
//	WGetDConfig = 0;
//	WSetDConfig = 0;
#endif	// CVLIBBUG y/n
#endif	// LDYNALIB
}

#endif	/* USEDCFG */

/* =====================================================================
 * Option2Init( ... )
 * If USEDCFG to enable Dialog initialisation we must LOAD the WJPEG
 * Library (DLL) and resolve the two headers used. An ERROR MESSAGE is
 * shown if this FAILS, and NO Options dialog will be displayed.
 * ===================================================================== */
BOOL	Option2Init( HWND hDlg, LPARAM lParam )
{
BOOL	flg;
LPOPTLIST	lpo;
#ifdef	USEDCFG
LPLIBCONFIG	lpCfg;
#endif
	flg = TRUE;	/* Default to all OK ... */
#ifdef	USEDCFG
	lpCfg = &DVLibConfig;
	if( LoadJLib( hDlg ) )
	{
		if( (*WGetLConfig) ( lpCfg ) )
		{
			UnloadJLib();
			if( !fShownNOLIB )
			{
				UINT	ui;
				fShownNOLIB = TRUE;
				//DIBError( ERR_NO_LIB );
				ui = Err_No_Lib();
			}
			flg = FALSE;
		}
		else
		{	/* We have the libraries CONFIG information ... */
//	WORD dcf_out_color; /* colorspace of output */
//	BOOL dcf_quantize_colors; /* T if output is a colormapped format */
//	BOOL dcf_two_pass_quantize;	/* use two-pass color quantization? */
//	BOOL dcf_use_dithering;		/* want color dithering? */
//	int  dcf_desired_colors;	/* max number of colors to use */
//	BOOL dcf_do_block_smoothing; /* T = apply cross-block smoothing */
//	BOOL dcf_do_pixel_smoothing; /* T = apply post-upsampling smoothing */
			if( lpCfg->dcf_out_color == CS_GRAYSCALE )
				fInGray = TRUE;
			else
				fInGray = FALSE;
			if( lpCfg->dcf_use_dithering )
				fUseDith = FALSE;	/* Do NOT inhibit dithering ... */
			else
				fUseDith = TRUE;	/* Inhibit dithering ... */
			if( lpCfg->dcf_two_pass_quantize )
				fUseQuant = FALSE;
			else
				fUseQuant = TRUE;
		}
	}
	else
	{
		if( !fShownNOLIB )
		{
			UINT	ui;
			fShownNOLIB = TRUE;
			//DIBError( ERR_NO_LIB );
			ui = Err_No_Lib();
		}
		flg = FALSE;
	}
	if( flg )
	{
#endif	/* USEDCFG */
		lpo = &OptList[0];	/* OK, now put up all the parameters ... */
		PutCurOpt( hDlg, lpo );
#ifdef	USEDCFG
	}	/* If library loaded ok */
#endif
return( flg );
}

BOOL	Option2Term( HWND hDlg )
{
BOOL	flg, achg;
LPOPTLIST	lpo;
#ifdef	USEDCFG
LPLIBCONFIG	lpCfg;
#endif
	flg = TRUE;		/* TRUE to fall into EXIT DIALOG ... */
	achg = FALSE;
	lpo = &OptList[0];
	achg = GetCurOpt( hDlg, lpo );
#ifdef	USEDCFG
	if( achg )	/* If some change made ... */
	{
		lpCfg = &DVLibConfig;
		if( fInGray )
			lpCfg->dcf_out_color = CS_GRAYSCALE;
		else
			lpCfg->dcf_out_color = CS_RGB;
		if( fUseDith )
			lpCfg->dcf_use_dithering = FALSE;
		else
			lpCfg->dcf_use_dithering = TRUE;
		if( fUseQuant )
			lpCfg->dcf_two_pass_quantize = FALSE;
		else
			lpCfg->dcf_two_pass_quantize = TRUE;
		if( fOutGray )	/* If only a gray scale output ... */
		{
			if( !( (lpCfg->ccf_num_components == 1) && /* Num comps 1=mono Def=3 */
				(lpCfg->ccf_h_samp_factor == 1) &&	/* Def 2x2 chrominance */
				(lpCfg->ccf_v_samp_factor == 1) ) )	/* Both 1x1 for mono */
			{
				fOGChg = TRUE;
				lpCfg->ccf_num_components = 1; /* Num comps 1=mono Def=3 */
				lpCfg->ccf_h_samp_factor = 1;	/* Def 2x2 chrominance */
				lpCfg->ccf_v_samp_factor = 1;	/* Both 1x1 for mono */
			}
		}
		else
		{
			if( !( (lpCfg->ccf_num_components == 3) && /* Num comps 1=mono Def=3 */
				(lpCfg->ccf_h_samp_factor == 2) &&	/* Def 2x2 chrominance */
				(lpCfg->ccf_v_samp_factor == 2) ) )	/* Both 1x1 for mono */
			{
				fOGChg = TRUE;
				lpCfg->ccf_num_components = 3; /* Num comps 1=mono Def=3 */
				lpCfg->ccf_h_samp_factor = 2;	/* Def 2x2 chrominance */
				lpCfg->ccf_v_samp_factor = 2;	/* Both 1x1 for mono */
			}
		}
		if( LoadJLib( hDlg ) )
		{
			if( (*WSetLConfig) ( lpCfg ) )
			{
				DIBError( ERR_NOLCONF );
				flg = FALSE;
			}
			else
			{
				DIBInfo( INF_LIBSET );
				achg = FALSE;
			}
		}
		else
		{
			if( !fShownNOLIB )
			{
				UINT	ui;
				fShownNOLIB = TRUE;
				//DIBError( ERR_NO_LIB );
				ui = Err_No_Lib();
			}
		}
	}
#endif
return( flg );
}
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF

BOOL	fWrtAll = TRUE;
//typedef	struct tagOPTLIST	{
//	WORD	op_Type;
//	LPINT	op_Flag;
//	LPINT	op_Chg;
//	WORD	op_ID;
//	BOOL	op_Def;
//	LPWORD	op_Num;
//	WORD	op_DNum;
//	WORD	op_NID;
//	LPINT	op_Chgn;
//	LPSTR	opi_Sect;
//	LPSTR	opi_Item;
//	WORD	opi_Type;
//	LPSTR	opi_Deft;
//}OPTLIST;
//typedef	OPTLIST MLPTR LPOPTLIST;

#ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
void	WriteIniJPEG( LPSTR lpIni, BOOL fCA )
{
	LPOPTLIST	lpo;
	LPSTR		lpd, lps, lpi, lpb, lpout;
	BOOL		flg, flgn;
	LPRECT		lprc;
	int			i;
	WORD		typ;
	char		buf[MAX_PATH+2];
	PCCOLTAB	pct;
	LPINT		pi, pi2, pin;
	PWORD		pw;
#ifdef	USEDCFG
	LPLIBCONFIG	lpCfg;
#endif	// USEDCFG

	if( !( lpIni && lpIni[0] ) )
		return;
#ifdef	USEDCFG
	lpCfg = &DVLibConfig;
	if( LoadJLib( (HWND)0 ) )
	{
		if( (*WGetLConfig) ( lpCfg ) )
		{
			UnloadJLib();
			return;
		}
	}
	else
	{
		return;
	}
#endif	// USEDCFG
	if( fCA || fWrtAll )	// if changing ALL
	{
		lpo = &OptList[0];
		lps = 0;
		while( lpo->op_Type )
		{
			if( lpo->opi_Sect )
			{
				if( lpo->opi_Sect != lps )
				{
					lps = lpo->opi_Sect;
					WritePrivateProfileString(
						lps,			/* Section */
						NULL,		/* Res.Word */
						NULL,		/* String to write */
						lpIni );		/* File Name */
				}
			}
			lpo++;
		}
	}
	lpb = &buf[0];
	lpo = &OptList[0];
	if( lpIni && lpIni[0] )
	{
		while( lpo->op_Type )
		{
			typ = lpo->opi_Type;
			lpd = lpo->opi_Deft;
			lprc = (LPRECT)lpd;
			if( (lps = lpo->opi_Sect) && (lpi = lpo->opi_Item) )
			{
				if( pi = lpo->op_Chg )
					flg = *pi;
				else
					flg = FALSE;
				if( pin = lpo->op_Chgn )
					flgn = *pin;
				else
					flgn = FALSE;
				if( fCA || flg || flgn || fWrtAll )
				{
					i = 0;
					lpout = 0;
					switch( typ )
					{
					case it_Table:
						if( pw = (PWORD)lpd )
							typ = *pw;
						else
							typ = CS_UNKNOWN;
						pct = &CColTab[0];
						while( pct->ct_Type != CS_UNKNOWN )
						{
							if( pct->ct_Type == typ )
								break;
							pct++;
						}
						if( pct->ct_Stg && (i = lstrlen( pct->ct_Stg )) )
						{
							lpout = pct->ct_Stg;
						}
						break;
					case it_BoolF:
					case it_BoolT:
						if( pi2 = (LPINT)lpd )
						{
							if( *pi2 )
								lpout = &szYes[0];
							else
								lpout = &szNo[0];
							i = lstrlen( lpout );
						}
						break;
					case it_Size1:	// A WORD Number output
						if( pw = (PWORD)lpd )
						{
							wsprintf( lpb, "%d", *pw );
							lpout = lpb;
							i = lstrlen( lpb );
						}
						break;
					case it_Size2:	// An int Number output
						if( pi2 = (LPINT)lpd )
						{
							i = *pi2;
							wsprintf( lpb, "%d", i );
							lpout = lpb;
							i = lstrlen( lpb );
						}
						break;
					}
					if( i && lpout ) // If an output string
					{
						WritePrivateProfileString(
							lps,			/* Section */
							lpi,		/* Res.Word */
							lpout,		/* String to write */
							lpIni );		/* File Name */
					}
				}	// Only if CHANGED (or change ALL)
			}	// If an INI file SECTION and ITEM (or KEY)
			lpo++;	// Down the OptList[] ...
		}	// While there is an op_Type ...
		lpo = &OptList[0];	// Restart to CLEAR CHANGES
		while( lpo->op_Type )
		{
			if( pi = lpo->op_Chg )	// Changed OPTION
				*pi = FALSE;
			if( pi2 = lpo->op_Chgn ) // Changed NUMBER with OPTION
				*pi2 = FALSE;
			lpo++;
		}
	}	// If an INI file given
#ifdef	USEDCFG
	UnloadJLib2();
#endif	// USEDCFG
}

BOOL	ReadIniJPEG( LPSTR lpIni, LPLIBCONFIG lpCfg )
{
	BOOL	chg = FALSE;
	LPOPTLIST	lpo;
	LPSTR		lpd, lps, lpi, lpb;
	WORD		typ;
	DWORD		dw;
	LPINT		pi1, pi2, pii;
	char		buf[MAX_PATH+2];
	PCCOLTAB	pct;
	PWORD		pw;

	if( !( lpIni && lpIni[0] ) )
		return( chg );
	lpb = &buf[0];
	lpo = &OptList[0];
	while( lpo->op_Type )
	{
		typ = lpo->opi_Type;
		lpd = lpo->opi_Deft;
		if( lpd && (lps = lpo->opi_Sect) && (lpi = lpo->opi_Item) )
		{
			pi1 = lpo->op_Chg;
			pi2 = lpo->op_Chgn;
			dw = GetPrivateProfileString( lps, // points to section name
				lpi, // points to key name
				&szBlank[0], // points to default string
				lpb,	// points to destination buffer
				MAX_PATH, // size of destination buffer
				lpIni ); // points to initialization filename
			if( dw && (dw < (MAX_PATH-2)) )
			{
				switch( typ )
				{
				case it_BoolT:
					pii = (LPINT)lpd;
					if( IsYes( lpb ) )
					{
						if( !(*pii) )
						{
							*pii = TRUE;
							chg = TRUE;
						}
					}
					else if( IsNo( lpb ) )
					{
						if( *pii )
						{
							*pii = FALSE;
							chg = TRUE;
						}
					}
					else
					{
						if( pi1 )
							*pi1 = TRUE;
						if( !(*pii) )
						{
							*pii = TRUE;
							chg = TRUE;
						}
					}
					break;
				case it_BoolF:
					pii = (LPINT)lpd;
					if( IsYes( lpb ) )
					{
						if( !(*pii) )
						{
							*pii = TRUE;
							chg = TRUE;
						}
					}
					else if( IsNo( lpb ) )
					{
						if( *pii )
						{
							*pii = FALSE;
							chg = TRUE;
						}
					}
					else
					{
						if( pi1 )
							*pi1 = TRUE;
						if( *pii )
						{
							*pii = FALSE;
							chg = TRUE;
						}
					}
					break;
				case it_Table:
					pct = &CColTab[0];
					pw = (PWORD)lpd;
					while( pct->ct_Type != CS_UNKNOWN )
					{
						if( lps = pct->ct_Stg )
						{
							if( lstrcmpi( lpb, lps ) == 0 )
							{
								if( *pw != pct->ct_Type )
								{
									*pw = pct->ct_Type;
									chg = TRUE;
								}
								break;
							}
						}
						pct++;
					}
					if( pct->ct_Type == CS_UNKNOWN )
					{
						if( pi1 )
							*pi1 = TRUE;
						else if( pi2 )
							*pi2 = TRUE;
					}
					break;
				case it_Size1:	// Word SIZE
					pw = (PWORD)lpd;
					typ = 0;
					if( dv_wscanf( lpb, &szSz1[0], ( PINT8 )&typ ) == 1 )
					{
						if( *pw != typ )
						{
							*pw = typ;
							chg = TRUE;
						}
					}
					else
					{
						if( pi2 )
							*pi2 = TRUE;
						else if( pi1 )
							*pi1 = TRUE;
					}
					break;
				case it_Size2:	// int SIZE
					pii = (LPINT)lpd;
					dw = 0;
					if( dv_wscanf( lpb, &szSz2[0], ( PINT8 )&dw ) == 1 )
					{
						if( *pii != (int)dw )
						{
							*pii = (int)dw;
							chg = TRUE;
						}
					}
					else
					{
						if( pi2 )
							*pi2 = TRUE;
						else if( pi1 )
							*pi1 = TRUE;
					}
					break;
				}
			}
			else
			{
				if( pi1 )
					*pi1 = TRUE;
				if( pi2 )
					*pi2 = TRUE;
			}
		}
		lpo++;
	}
	return( chg );
}

#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF
///////////////////////////////////////////////////////////////////////////
#endif // #ifdef ADD_JPEG_SUPPORT // 20200620: turn this off for now, inc GIF


// ********************************************************
void	putddnum(LPSTR lpd, DWORD num)
{
	DWORD	kcnt, kdrem;
	WORD	krem;
	if (num < 1000)
	{
		wsprintf(lpd, "%lu", num);
	}
	else if (num < 100000)
	{
		kcnt = num / 1000;
		krem = LOWORD(num % 1000);
		wsprintf(lpd, "%lu.%0uK", kcnt, krem);
	}
	else
	{
		kcnt = num / 100000;
		kdrem = num % 100000;
		wsprintf(lpd, "%lu.%0luM", kcnt, kdrem);
	}
}

DWORD	getddnum(LPSTR lpb)
{
	BOOL	flg;
	WORD	i, j;
	DWORD	num;
	DWORD	num1, num2;
	char	c;

	num = num1 = num2 = 0;
	flg = FALSE;
	if (lpb && (i = lstrlen(lpb)))
	{
		for (j = 0; j < i; j++)
		{
			c = lpb[j];
			if ((c >= '0') && (c <= '9'))
			{
				if (flg)
					num2 = (num2 * 10) + (c - '0');
				else
					num1 = (num1 * 10) + (c - '0');
			}
			else if (c == '.')
			{
				flg = TRUE;
			}
			else
			{	// First NON-NUMERIC
				break;	// = End of number
			}
		}
		num = num1;	// Simple case
		if ((c == 'K') || (c == 'k'))
		{
			num = (num1 * 1000) + num2;
		}
		else if ((c == 'M') || (c == 'm'))
		{
			num = (num1 * 100000) + num2;
		}
		else if (flg)
		{
			num = (num1 * 1000) + num2;
		}
	}
	return(num);
}

void	PutCurOpt(HWND hDlg, LPOPTLIST lpob)
{
	LPOPTLIST	lpo;
	BOOL	flg;
	WORD	id1, num, typ, id2;
	char	ddbuf[20];
	DWORD	ddnum;
	LPDWORD	lpdw;
	LPSTR	lpb;
	if (lpo = lpob)
	{
		lpb = &ddbuf[0];
		id2 = lpo->op_NID;
		while ((id1 = lpo->op_ID) || id2)
		{
			typ = lpo->op_Type;	/* Get TYPE */
			if (lpo->op_Flag)	// If there IS a pointer to an OPTION FLAG
				flg = *lpo->op_Flag;
			else
				flg = FALSE;
			if (id1)
				ChkDlg(id1, flg);
			if (id2)	/* If a (associated or not) TEXT BOX ... */
			{
				switch (typ)
				{
				case it_General:
				case it_Size1:
					if (lpo->op_Num)
					{
						num = *lpo->op_Num;
						SetDlgItemInt(hDlg, id2, num, FALSE);
					}
					break;
				case it_Size2:
					if (lpdw = (LPDWORD)lpo->op_Num)
					{
						ddnum = *lpdw;
						putddnum(lpb, ddnum);
						SetDlgItemText(hDlg, id2, lpb);
					}
					break;
				}
			}
			lpo++;
			id2 = lpo->op_NID;
		}
	}
}

BOOL GetCurOpt(HWND hDlg, LPOPTLIST lpob)
{
	LPOPTLIST	lpo;
	BOOL	flg, achg;
	WORD	id1, num, typ, id2;
	LPINT	ptr, ptr2;
	UINT	ui;
	char	ddbuf[20];
	DWORD	ddnum;
	LPDWORD	lpdw;
	LPSTR	lpb;

	achg = FALSE;
	if (lpo = lpob)
	{
		lpb = &ddbuf[0];
		id2 = lpo->op_NID;	// Get associated NUMBER ID (if ANY!!!)
		while ((id1 = lpo->op_ID) || id2)
		{
			typ = lpo->op_Type;	/* Get TYPE */
			if (id1)	// If an Option Item
			{
				ui = IsChk(id1);
				if (ptr = lpo->op_Flag)
				{
					if (ui == 0)
					{
						if (*ptr)
						{
							*ptr = FALSE;
							if (lpo->op_Chg)
								*lpo->op_Chg = TRUE;
							achg = TRUE;
						}
					}
					else
					{
						if (!(*ptr))
						{
							*ptr = TRUE;
							if (lpo->op_Chg)
								*lpo->op_Chg = TRUE;
							achg = TRUE;
						}
					}
				}
			}
			if (id2)
			{
				switch (typ)
				{
				case it_General:
				case it_Size1:
				{
					num = GetDlgItemInt(hDlg, id2, &flg, FALSE);
					if (flg)	/* If OK ... */
					{
						if (lpo->op_Num)
						{
							if (*lpo->op_Num != num)
							{
								*lpo->op_Num = num;
								if (lpo->op_Chg)
									*lpo->op_Chg = TRUE;
								achg = TRUE;
								if (ptr2 = lpo->op_Chgn)
								{
									*ptr2 = TRUE;
								}
							}
						}
					}
					break;
				}
				case it_Size2:
				{
					num = GetDlgItemText(hDlg, id2, lpb, sizeof(ddbuf));
					ddnum = getddnum(lpb);
					if (lpdw = (LPDWORD)lpo->op_Num)
					{
						if (*lpdw != ddnum)
						{
							*lpdw = ddnum;
							if (lpo->op_Chg)
								*lpo->op_Chg = TRUE;
							if (ptr2 = lpo->op_Chgn)
							{
								*ptr2 = TRUE;
							}
							achg = TRUE;
						}
					}
					break;
				}
				}
			}
			lpo++;
			id2 = lpo->op_NID;
		}
	}
	return(achg);
}

void	PutDefOpt(HWND hDlg, LPOPTLIST lpob)
{
	BOOL	flg;
	WORD	id1, num, typ, id2;
	LPOPTLIST	lpo;
	char	ddbuf[20];
	DWORD	ddnum;
	LPSTR	lpb;
	lpb = &ddbuf[0];
	if (lpo = lpob)
	{
		id2 = lpo->op_NID;
		while ((id1 = lpo->op_ID) || id2)
		{
			typ = lpo->op_Type;	/* Get the TYPE */
			flg = lpo->op_Def;	/* Extract DEFAULT flag ... */
			if (id1)
				ChkDlg(id1, flg);	/* And set control to DEFAULT ... */
			if (id2)	/* If there is an associated NUMBER ... */
			{
				switch (typ)
				{
				case it_General:
				case it_Size1:
					num = lpo->op_DNum;	/* Extract DEFAULT number ... */
					SetDlgItemInt(hDlg, id2, num, FALSE);	/* And set the CONTROL */
					break;
				case it_Size2:
					ddnum = (DWORD)lpo->op_DNum;
					putddnum(lpb, ddnum);
					SetDlgItemText(hDlg, id2, lpb);
					break;
				}
			}
			lpo++;
			id2 = lpo->op_NID;
		}
	}
}

// eof - DvOpt2.c
