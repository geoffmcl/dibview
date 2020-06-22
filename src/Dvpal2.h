
/* DvPal2.h - some shared PAL items */
#ifndef	_DvPal2_H
#define	_DvPal2_H

#define	DEFDRWPOL		FALSE	/* init BOOL gfDrwPol (Split) */

#ifdef	FRMENU
/* menu is Fichier Edition Affichage ... */

#define		FP_FILE		0
#define		FP_EDIT		1
#define		FP_VIEW		2

#else	/* !FRMENU */
/* org. english menu for palette window */

#define		PP_FILE		0
#define		PP_SQUARE	1
#define		PP_ORDER	2

#endif	/* FRMENU y/n */

// Values for pi_iNormal member - see DvPal.h
#define		pif_Intensity		0x00000001
#define		pif_Rainbow			0x00000002
#define		pif_Numeric			0x00000004
#define     pif_Gray          0x00000008
#define     pif_Freq          0x00000010

#define		pif_Flags	( pif_Intensity | pif_Rainbow | pif_Numeric | pif_Gray | pif_Freq )
#define		pif_Clear	~( pif_Flags )

#define		_P		PPI lppi

// Useful magic numbers.
//#define PAL_SIZE_DEF       IDM_PAL_MEDIUM    // Default palette square size.
#define PAL_SIZE_DEF       IDM_PAL_SMALL    // Default palette square size.
#define ID_INFO_CHILD      1                 // Palette information window ID
#define	MXENTRIES			4

#define		CR_RED		RGB(255,  0,  0)
#define		CR_ORANGE	RGB(255,128,  0)
#define		CR_YELLOW	RGB(255,255,  0)
#define		CR_YEL2GR	RGB(128,255,  0)
#define		CR_GREEN	RGB(  0,255,  0)
#define		CR_GR2BL	RGB(  0,255,128)
#define		CR_GR2BL2	RGB(  0,255,255)
#define		CR_GR2BL3	RGB(  0,128,255)
#define		CR_BLUE		RGB(  0,  0,255)
#define		CR_BL2RD	RGB(128,  0,255)
#define		CR_BL2RD2	RGB(255,  0,255)
#define		CR_BL2RD3	RGB(255,  0,128)
#define		MCYCSET			12		/* defined RGB components */

typedef struct tagCLRSET {
	COLORREF	cs_Colr;
	LPSTR		cs_RGB;
	LPSTR		cs_Name;
}CLRSET, * PCS;

extern	CLRSET	crClrSet[];	// rainbow order - 12

//extern	char	gszDiag[];	// General diagnostic buffer
extern	void	   chkit( char * );	/* ERRANT something */
extern	LPSTR	   SetgszStatus( DWORD dwCnt );
extern	DWORD		GetCOLRCnt( LPSTR lpb );
extern	COLORREF	GetCOLR( LPSTR lpb, DWORD index );
extern	DWORD		GetFREQ( LPSTR lpb, DWORD index );
extern	DWORD		GetFREQMax( LPSTR lpb );
extern	void		DoSORT( PPI lpPalInfo );

// Image information (if required)
extern	int GetUCCnt( LPDIBINFO lpDIBInfo,
					 LPSTR lpb, int Cols, DWORD Size,
					 int Wid, int Height,
					 DWORD wBPP, LPSTR lpDIB );
extern	DWORD	GetTextExtent( HDC hDC, LPSTR szStr, int nLen );

//int nEntriesPerInch[4] = {15,               // Tiny mode  squares/inch
//                          10,               // Small mode squares/inch
//                           6,               // Medium mode squares/inch
//                           3};              // Large mode squares/inch
extern	int nEntriesPerInch[];	// was MXENTRIES=4
//					10,	// Tiny mode  squares/inch
//					6,		// Small mode squares/inch
//					4,		// Medium mode squares/inch
//					2 };	// Large mode squares/inch

extern	BOOL	fShwBT;
extern	UINT	guiRMCnt;
extern	BOOL	gfDrwPol;
//extern	char	gszPText[];	/* pad for paint of text (abt 260) */
extern	int		giChgDrwP;
// 	case IDM_PAL_HISTO:
extern	int		giDrwHisto;
extern	int		giChgHisto;
extern	int		gfAddPenC;
extern	int		gfAddPenC2;
extern	int		gfEraseBkGnd;
extern	int		gcxPixs;	// = GetDeviceCaps( hDC, HORZRES ); //Width, in pixels, of the screen.
extern	int		gcyPixs;	// = GetDeviceCaps( hDC, VERTRES ); //Height, in raster lines, of the screen.
//extern	char	gszSpl[];	// colour from PALETTE (realised) abt 256 bytes
//extern	TEXTMETRIC  gsTM;	// height, width of text at last check of HDC

//		lstrcpy( lpt, "Logical / Physical Colors!" );
//		if( gfDrwPol )
//				"%d Ranked / Palette Colors!",
//				dwCnt );	// number of Colors in bitmap
//				"%d Colors per BMP order!",
//extern	char	gszStatus[];	// was 256

//void PalRowsAndColumns  (HWND hWnd, 
extern	void PalRowsAndColumns( PPI lpPalInfo, 
                          int nSquareSize, 
                        LPINT lpnCols, 
                        LPINT lpnRows,
                        LPINT lpcxSquare,
                        LPINT lpcySquare);

extern	void UnHighlightSquare   (HDC hDC, 
                         DWORD wEntry, 
                          int cxSquare, 
                          int cySquare,
                          int nCols,
                          int nScroll);
extern	void	HiSquare( HWND hWnd, HDC hDC,
				 PPI lpPalInfo, LPSTR lpb );

extern	BOOL	GotFlags( _P );
extern	BOOL	IsFlagIntensity( _P );
extern	BOOL	IsFlagRainbow( _P );
extern	BOOL	IsFlagNumeric( _P );
extern   BOOL	IsFlagGray( PPI lpPalInfo );
extern   BOOL	IsFlagFreq( PPI lpPalInfo );


#endif	/* _DvPal2_H */
/* eof - DvPal2.h */
