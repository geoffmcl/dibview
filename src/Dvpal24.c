
// =========================================================
// DvPal24.c
//
// Searching for ways to REDUCE a 24-bit BITMAP to
// a MAXIMUM of 256 colours.
//
// Main Service -
// HPALETTE	CreateDIBPal24( PDI lpDIBInfo, HANDLE hDIB, BOOL fNew )
//
//
// =========================================================
#include	"dv.h"
#include	"DvPal24.h"

//extern	WORD	wMaxCols;	// = DEF_MAX_COLS or per INI file

#ifdef	ADDPROGM
extern	void	PutProgM( void );		// Open progress windows
extern	void KillProgM( void );
extern	void SetProgM( DWORD, DWORD );
extern	void SetProgM1( void );
extern	void SetProgMInfo( LPSTR );
extern	void SetProgMInfo2( LPSTR );
extern	BOOL bUsrAbort;
#endif	// ADDPROGM

extern   DWORD	GetCOLRCnt( LPSTR lpb );

#ifdef	WRTPDIAG
extern	void	WrtPalDiag( LPLOGPALETTE lpPal, BOOL fAppd,
						   LPSTR lpi, LPDWORD lpdw );
#endif	// WRTPDIAG


#ifdef		ADDRNG1
// =====================================
// NOT PRESENTLY USED
#define		Rng1		  0,  51,   0
#define		Rng2		 52, 102,  63
#define		Rng3		103, 153, 128
#define		Rng4		154, 204, 192
#define		Rng5		205, 255, 255

COLRNG2	ColRng128[] = {
	// 5 Blue = 1 Gr = 2
	{ { Rng1 }, { Rng1 }, { Rng1 } },
	{ { Rng1 }, { Rng1 }, { Rng2 } },
	{ { Rng1 }, { Rng1 }, { Rng3 } },
	{ { Rng1 }, { Rng1 }, { Rng4 } },
	{ { Rng1 }, { Rng1 }, { Rng5 } },
	// 10         Gr = 2
	{ { Rng1 }, { Rng2 }, { Rng1 } },
	{ { Rng1 }, { Rng2 }, { Rng2 } },
	{ { Rng1 }, { Rng2 }, { Rng3 } },
	{ { Rng1 }, { Rng2 }, { Rng4 } },
	{ { Rng1 }, { Rng2 }, { Rng5 } },
	// 15         Gr = 3
	{ { Rng1 }, { Rng3 }, { Rng1 } },
	{ { Rng1 }, { Rng3 }, { Rng2 } },
	{ { Rng1 }, { Rng3 }, { Rng3 } },
	{ { Rng1 }, { Rng3 }, { Rng4 } },
	{ { Rng1 }, { Rng3 }, { Rng5 } },
	// 20         Gr = 4
	{ { Rng1 }, { Rng4 }, { Rng1 } },
	{ { Rng1 }, { Rng4 }, { Rng2 } },
	{ { Rng1 }, { Rng4 }, { Rng3 } },
	{ { Rng1 }, { Rng4 }, { Rng4 } },
	{ { Rng1 }, { Rng4 }, { Rng5 } },
	// 25         Gr = 5
	{ { Rng1 }, { Rng5 }, { Rng1 } },
	{ { Rng1 }, { Rng5 }, { Rng2 } },
	{ { Rng1 }, { Rng5 }, { Rng3 } },
	{ { Rng1 }, { Rng5 }, { Rng4 } },
	{ { Rng1 }, { Rng5 }, { Rng5 } },

	// 30 Bl = 2 Gr = 1
	{ { Rng2 }, { Rng1 }, { Rng1 } },
	{ { Rng2 }, { Rng1 }, { Rng2 } },
	{ { Rng2 }, { Rng1 }, { Rng3 } },
	{ { Rng2 }, { Rng1 }, { Rng4 } },
	{ { Rng2 }, { Rng1 }, { Rng5 } },
	// 35        Gr = 2
	{ { Rng2 }, { Rng2 }, { Rng1 } },
	{ { Rng2 }, { Rng2 }, { Rng2 } },
	{ { Rng2 }, { Rng2 }, { Rng3 } },
	{ { Rng2 }, { Rng2 }, { Rng4 } },
	{ { Rng2 }, { Rng2 }, { Rng5 } },
	// 40       Gr = 3
	{ { Rng2 }, { Rng3 }, { Rng1 } },
	{ { Rng2 }, { Rng3 }, { Rng2 } },
	{ { Rng2 }, { Rng3 }, { Rng3 } },
	{ { Rng2 }, { Rng3 }, { Rng4 } },
	{ { Rng2 }, { Rng3 }, { Rng5 } },
	// 45       Gr = 4
	{ { Rng2 }, { Rng4 }, { Rng1 } },
	{ { Rng2 }, { Rng4 }, { Rng2 } },
	{ { Rng2 }, { Rng4 }, { Rng3 } },
	{ { Rng2 }, { Rng4 }, { Rng4 } },
	{ { Rng2 }, { Rng4 }, { Rng5 } },
	// 50       Gr = 5
	{ { Rng2 }, { Rng5 }, { Rng1 } },
	{ { Rng2 }, { Rng5 }, { Rng2 } },
	{ { Rng2 }, { Rng5 }, { Rng3 } },
	{ { Rng2 }, { Rng5 }, { Rng4 } },
	{ { Rng2 }, { Rng5 }, { Rng5 } },

	// 55 Bl = 3 Gr = 1
	{ { Rng3 }, { Rng1 }, { Rng1 } },
	{ { Rng3 }, { Rng1 }, { Rng2 } },
	{ { Rng3 }, { Rng1 }, { Rng3 } },
	{ { Rng3 }, { Rng1 }, { Rng4 } },
	{ { Rng3 }, { Rng1 }, { Rng5 } },
	// 60        Gr = 2
	{ { Rng3 }, { Rng2 }, { Rng1 } },
	{ { Rng3 }, { Rng2 }, { Rng2 } },
	{ { Rng3 }, { Rng2 }, { Rng3 } },
	{ { Rng3 }, { Rng2 }, { Rng4 } },
	{ { Rng3 }, { Rng2 }, { Rng5 } },
	// 65        Gr = 3
	{ { Rng3 }, { Rng3 }, { Rng1 } },
	{ { Rng3 }, { Rng3 }, { Rng2 } },
	{ { Rng3 }, { Rng3 }, { Rng3 } },
	{ { Rng3 }, { Rng3 }, { Rng4 } },
	{ { Rng3 }, { Rng3 }, { Rng5 } },
	// 70        Gr = 4
	{ { Rng3 }, { Rng4 }, { Rng1 } },
	{ { Rng3 }, { Rng4 }, { Rng2 } },
	{ { Rng3 }, { Rng4 }, { Rng3 } },
	{ { Rng3 }, { Rng4 }, { Rng4 } },
	{ { Rng3 }, { Rng4 }, { Rng5 } },
	// 75       Gr = 5
	{ { Rng3 }, { Rng5 }, { Rng1 } },
	{ { Rng3 }, { Rng5 }, { Rng2 } },
	{ { Rng3 }, { Rng5 }, { Rng3 } },
	{ { Rng3 }, { Rng5 }, { Rng4 } },
	{ { Rng3 }, { Rng5 }, { Rng5 } },

	// 80
	{ { Rng4 }, { Rng1 }, { Rng1 } },
	{ { Rng4 }, { Rng1 }, { Rng2 } },
	{ { Rng4 }, { Rng1 }, { Rng3 } },
	{ { Rng4 }, { Rng1 }, { Rng4 } },
	{ { Rng4 }, { Rng1 }, { Rng5 } },
	// 85
	{ { Rng4 }, { Rng2 }, { Rng1 } },
	{ { Rng4 }, { Rng2 }, { Rng2 } },
	{ { Rng4 }, { Rng2 }, { Rng3 } },
	{ { Rng4 }, { Rng2 }, { Rng4 } },
	{ { Rng4 }, { Rng2 }, { Rng5 } },
	// 90
	{ { Rng4 }, { Rng3 }, { Rng1 } },
	{ { Rng4 }, { Rng3 }, { Rng2 } },
	{ { Rng4 }, { Rng3 }, { Rng3 } },
	{ { Rng4 }, { Rng3 }, { Rng4 } },
	{ { Rng4 }, { Rng3 }, { Rng5 } },
	// 95
	{ { Rng4 }, { Rng4 }, { Rng1 } },
	{ { Rng4 }, { Rng4 }, { Rng2 } },
	{ { Rng4 }, { Rng4 }, { Rng3 } },
	{ { Rng4 }, { Rng4 }, { Rng4 } },
	{ { Rng4 }, { Rng4 }, { Rng5 } },
	// 100
	{ { Rng4 }, { Rng5 }, { Rng1 } },
	{ { Rng4 }, { Rng5 }, { Rng2 } },
	{ { Rng4 }, { Rng5 }, { Rng3 } },
	{ { Rng4 }, { Rng5 }, { Rng4 } },
	{ { Rng4 }, { Rng5 }, { Rng5 } },

	// 105
	{ { Rng5 }, { Rng1 }, { Rng1 } },
	{ { Rng5 }, { Rng1 }, { Rng2 } },
	{ { Rng5 }, { Rng1 }, { Rng3 } },
	{ { Rng5 }, { Rng1 }, { Rng4 } },
	{ { Rng5 }, { Rng1 }, { Rng5 } },
	// 110
	{ { Rng5 }, { Rng2 }, { Rng1 } },
	{ { Rng5 }, { Rng2 }, { Rng2 } },
	{ { Rng5 }, { Rng2 }, { Rng3 } },
	{ { Rng5 }, { Rng2 }, { Rng4 } },
	{ { Rng5 }, { Rng2 }, { Rng5 } },
	// 115
	{ { Rng5 }, { Rng3 }, { Rng1 } },
	{ { Rng5 }, { Rng3 }, { Rng2 } },
	{ { Rng5 }, { Rng3 }, { Rng3 } },
	{ { Rng5 }, { Rng3 }, { Rng4 } },
	{ { Rng5 }, { Rng3 }, { Rng5 } },
	// 120
	{ { Rng5 }, { Rng4 }, { Rng1 } },
	{ { Rng5 }, { Rng4 }, { Rng2 } },
	{ { Rng5 }, { Rng4 }, { Rng3 } },
	{ { Rng5 }, { Rng4 }, { Rng4 } },
	{ { Rng5 }, { Rng4 }, { Rng5 } },
	// 125
	{ { Rng5 }, { Rng5 }, { Rng1 } },
	{ { Rng5 }, { Rng5 }, { Rng2 } },
	{ { Rng5 }, { Rng5 }, { Rng3 } },
	{ { Rng5 }, { Rng5 }, { Rng4 } },
	{ { Rng5 }, { Rng5 }, { Rng5 } },
	
};

#define		Rng61		  0,  42,   0
#define		Rng62		 43,  85,  51
#define		Rng63		 86, 127, 102
#define		Rng64		128, 170, 153
#define		Rng65		171, 212, 204
#define		Rng66		213, 255, 255

COLRNG2	ColRng2[] = {
	// 6 Blue = 1 Gr = 2
	{ { Rng61 }, { Rng61 }, { Rng61 } },
	{ { Rng61 }, { Rng61 }, { Rng62 } },
	{ { Rng61 }, { Rng61 }, { Rng63 } },
	{ { Rng61 }, { Rng61 }, { Rng64 } },
	{ { Rng61 }, { Rng61 }, { Rng65 } },
	{ { Rng61 }, { Rng61 }, { Rng66 } },
	// 12         Gr = 2
	{ { Rng61 }, { Rng62 }, { Rng61 } },
	{ { Rng61 }, { Rng62 }, { Rng62 } },
	{ { Rng61 }, { Rng62 }, { Rng63 } },
	{ { Rng61 }, { Rng62 }, { Rng64 } },
	{ { Rng61 }, { Rng62 }, { Rng65 } },
	{ { Rng61 }, { Rng62 }, { Rng66 } },
	// 18         Gr = 3
	{ { Rng61 }, { Rng63 }, { Rng61 } },
	{ { Rng61 }, { Rng63 }, { Rng62 } },
	{ { Rng61 }, { Rng63 }, { Rng63 } },
	{ { Rng61 }, { Rng63 }, { Rng64 } },
	{ { Rng61 }, { Rng63 }, { Rng65 } },
	{ { Rng61 }, { Rng63 }, { Rng66 } },
	// 24         Gr = 4
	{ { Rng61 }, { Rng64 }, { Rng61 } },
	{ { Rng61 }, { Rng64 }, { Rng62 } },
	{ { Rng61 }, { Rng64 }, { Rng63 } },
	{ { Rng61 }, { Rng64 }, { Rng64 } },
	{ { Rng61 }, { Rng64 }, { Rng65 } },
	{ { Rng61 }, { Rng64 }, { Rng66 } },
	// 30         Gr = 5
	{ { Rng61 }, { Rng65 }, { Rng61 } },
	{ { Rng61 }, { Rng65 }, { Rng62 } },
	{ { Rng61 }, { Rng65 }, { Rng63 } },
	{ { Rng61 }, { Rng65 }, { Rng64 } },
	{ { Rng61 }, { Rng65 }, { Rng65 } },
	{ { Rng61 }, { Rng65 }, { Rng66 } },
	// 36
	{ { Rng61 }, { Rng66 }, { Rng61 } },
	{ { Rng61 }, { Rng66 }, { Rng62 } },
	{ { Rng61 }, { Rng66 }, { Rng63 } },
	{ { Rng61 }, { Rng66 }, { Rng64 } },
	{ { Rng61 }, { Rng66 }, { Rng65 } },
	{ { Rng61 }, { Rng66 }, { Rng66 } },

	// 42 Bl = 2 Gr = 1
	{ { Rng62 }, { Rng61 }, { Rng61 } },
	{ { Rng62 }, { Rng61 }, { Rng62 } },
	{ { Rng62 }, { Rng61 }, { Rng63 } },
	{ { Rng62 }, { Rng61 }, { Rng64 } },
	{ { Rng62 }, { Rng61 }, { Rng65 } },
	{ { Rng62 }, { Rng61 }, { Rng66 } },
	// 48        Gr = 2
	{ { Rng62 }, { Rng62 }, { Rng61 } },
	{ { Rng62 }, { Rng62 }, { Rng62 } },
	{ { Rng62 }, { Rng62 }, { Rng63 } },
	{ { Rng62 }, { Rng62 }, { Rng64 } },
	{ { Rng62 }, { Rng62 }, { Rng65 } },
	{ { Rng62 }, { Rng62 }, { Rng66 } },
	// 54       Gr = 3
	{ { Rng62 }, { Rng63 }, { Rng61 } },
	{ { Rng62 }, { Rng63 }, { Rng62 } },
	{ { Rng62 }, { Rng63 }, { Rng63 } },
	{ { Rng62 }, { Rng63 }, { Rng64 } },
	{ { Rng62 }, { Rng63 }, { Rng65 } },
	{ { Rng62 }, { Rng63 }, { Rng66 } },
	// 60       Gr = 4
	{ { Rng62 }, { Rng64 }, { Rng61 } },
	{ { Rng62 }, { Rng64 }, { Rng62 } },
	{ { Rng62 }, { Rng64 }, { Rng63 } },
	{ { Rng62 }, { Rng64 }, { Rng64 } },
	{ { Rng62 }, { Rng64 }, { Rng65 } },
	{ { Rng62 }, { Rng64 }, { Rng66 } },
	// 66       Gr = 5
	{ { Rng62 }, { Rng65 }, { Rng61 } },
	{ { Rng62 }, { Rng65 }, { Rng62 } },
	{ { Rng62 }, { Rng65 }, { Rng63 } },
	{ { Rng62 }, { Rng65 }, { Rng64 } },
	{ { Rng62 }, { Rng65 }, { Rng65 } },
	{ { Rng62 }, { Rng65 }, { Rng66 } },
	// 72
	{ { Rng62 }, { Rng66 }, { Rng61 } },
	{ { Rng62 }, { Rng66 }, { Rng62 } },
	{ { Rng62 }, { Rng66 }, { Rng63 } },
	{ { Rng62 }, { Rng66 }, { Rng64 } },
	{ { Rng62 }, { Rng66 }, { Rng65 } },
	{ { Rng62 }, { Rng66 }, { Rng66 } },

	// 78 Bl = 3 Gr = 1
	{ { Rng63 }, { Rng61 }, { Rng61 } },
	{ { Rng63 }, { Rng61 }, { Rng62 } },
	{ { Rng63 }, { Rng61 }, { Rng63 } },
	{ { Rng63 }, { Rng61 }, { Rng64 } },
	{ { Rng63 }, { Rng61 }, { Rng65 } },
	{ { Rng63 }, { Rng61 }, { Rng66 } },
	// 84        Gr = 2
	{ { Rng63 }, { Rng62 }, { Rng61 } },
	{ { Rng63 }, { Rng62 }, { Rng62 } },
	{ { Rng63 }, { Rng62 }, { Rng63 } },
	{ { Rng63 }, { Rng62 }, { Rng64 } },
	{ { Rng63 }, { Rng62 }, { Rng65 } },
	{ { Rng63 }, { Rng62 }, { Rng66 } },
	// 90        Gr = 3
	{ { Rng63 }, { Rng63 }, { Rng61 } },
	{ { Rng63 }, { Rng63 }, { Rng62 } },
	{ { Rng63 }, { Rng63 }, { Rng63 } },
	{ { Rng63 }, { Rng63 }, { Rng64 } },
	{ { Rng63 }, { Rng63 }, { Rng65 } },
	{ { Rng63 }, { Rng63 }, { Rng66 } },
	// 96        Gr = 4
	{ { Rng63 }, { Rng64 }, { Rng61 } },
	{ { Rng63 }, { Rng64 }, { Rng62 } },
	{ { Rng63 }, { Rng64 }, { Rng63 } },
	{ { Rng63 }, { Rng64 }, { Rng64 } },
	{ { Rng63 }, { Rng64 }, { Rng65 } },
	{ { Rng63 }, { Rng64 }, { Rng66 } },
	// 102       Gr = 5
	{ { Rng63 }, { Rng65 }, { Rng61 } },
	{ { Rng63 }, { Rng65 }, { Rng62 } },
	{ { Rng63 }, { Rng65 }, { Rng63 } },
	{ { Rng63 }, { Rng65 }, { Rng64 } },
	{ { Rng63 }, { Rng65 }, { Rng65 } },
	{ { Rng63 }, { Rng65 }, { Rng66 } },
	// 108
	{ { Rng63 }, { Rng66 }, { Rng61 } },
	{ { Rng63 }, { Rng66 }, { Rng62 } },
	{ { Rng63 }, { Rng66 }, { Rng63 } },
	{ { Rng63 }, { Rng66 }, { Rng64 } },
	{ { Rng63 }, { Rng66 }, { Rng65 } },
	{ { Rng63 }, { Rng66 }, { Rng66 } },

	// 114
	{ { Rng64 }, { Rng61 }, { Rng61 } },
	{ { Rng64 }, { Rng61 }, { Rng62 } },
	{ { Rng64 }, { Rng61 }, { Rng63 } },
	{ { Rng64 }, { Rng61 }, { Rng64 } },
	{ { Rng64 }, { Rng61 }, { Rng65 } },
	{ { Rng64 }, { Rng61 }, { Rng66 } },
	// 120
	{ { Rng64 }, { Rng62 }, { Rng61 } },
	{ { Rng64 }, { Rng62 }, { Rng62 } },
	{ { Rng64 }, { Rng62 }, { Rng63 } },
	{ { Rng64 }, { Rng62 }, { Rng64 } },
	{ { Rng64 }, { Rng62 }, { Rng65 } },
	{ { Rng64 }, { Rng62 }, { Rng66 } },
	// 126
	{ { Rng64 }, { Rng63 }, { Rng61 } },
	{ { Rng64 }, { Rng63 }, { Rng62 } },
	{ { Rng64 }, { Rng63 }, { Rng63 } },
	{ { Rng64 }, { Rng63 }, { Rng64 } },
	{ { Rng64 }, { Rng63 }, { Rng65 } },
	{ { Rng64 }, { Rng63 }, { Rng66 } },
	// 132
	{ { Rng64 }, { Rng64 }, { Rng61 } },
	{ { Rng64 }, { Rng64 }, { Rng62 } },
	{ { Rng64 }, { Rng64 }, { Rng63 } },
	{ { Rng64 }, { Rng64 }, { Rng64 } },
	{ { Rng64 }, { Rng64 }, { Rng65 } },
	{ { Rng64 }, { Rng64 }, { Rng66 } },
	// 138
	{ { Rng64 }, { Rng65 }, { Rng61 } },
	{ { Rng64 }, { Rng65 }, { Rng62 } },
	{ { Rng64 }, { Rng65 }, { Rng63 } },
	{ { Rng64 }, { Rng65 }, { Rng64 } },
	{ { Rng64 }, { Rng65 }, { Rng65 } },
	{ { Rng64 }, { Rng65 }, { Rng66 } },
	// 144
	{ { Rng64 }, { Rng66 }, { Rng61 } },
	{ { Rng64 }, { Rng66 }, { Rng62 } },
	{ { Rng64 }, { Rng66 }, { Rng63 } },
	{ { Rng64 }, { Rng66 }, { Rng64 } },
	{ { Rng64 }, { Rng66 }, { Rng65 } },
	{ { Rng64 }, { Rng66 }, { Rng66 } },
	
	// 150
	{ { Rng65 }, { Rng61 }, { Rng61 } },
	{ { Rng65 }, { Rng61 }, { Rng62 } },
	{ { Rng65 }, { Rng61 }, { Rng63 } },
	{ { Rng65 }, { Rng61 }, { Rng64 } },
	{ { Rng65 }, { Rng61 }, { Rng65 } },
	{ { Rng65 }, { Rng61 }, { Rng66 } },
	// 156
	{ { Rng65 }, { Rng62 }, { Rng61 } },
	{ { Rng65 }, { Rng62 }, { Rng62 } },
	{ { Rng65 }, { Rng62 }, { Rng63 } },
	{ { Rng65 }, { Rng62 }, { Rng64 } },
	{ { Rng65 }, { Rng62 }, { Rng65 } },
	{ { Rng65 }, { Rng62 }, { Rng66 } },
	// 162
	{ { Rng65 }, { Rng63 }, { Rng61 } },
	{ { Rng65 }, { Rng63 }, { Rng62 } },
	{ { Rng65 }, { Rng63 }, { Rng63 } },
	{ { Rng65 }, { Rng63 }, { Rng64 } },
	{ { Rng65 }, { Rng63 }, { Rng65 } },
	{ { Rng65 }, { Rng63 }, { Rng66 } },
	// 168
	{ { Rng65 }, { Rng64 }, { Rng61 } },
	{ { Rng65 }, { Rng64 }, { Rng62 } },
	{ { Rng65 }, { Rng64 }, { Rng63 } },
	{ { Rng65 }, { Rng64 }, { Rng64 } },
	{ { Rng65 }, { Rng64 }, { Rng65 } },
	{ { Rng65 }, { Rng64 }, { Rng66 } },
	// 174
	{ { Rng65 }, { Rng65 }, { Rng61 } },
	{ { Rng65 }, { Rng65 }, { Rng62 } },
	{ { Rng65 }, { Rng65 }, { Rng63 } },
	{ { Rng65 }, { Rng65 }, { Rng64 } },
	{ { Rng65 }, { Rng65 }, { Rng65 } },
	{ { Rng65 }, { Rng65 }, { Rng66 } },
	// 180
	{ { Rng65 }, { Rng66 }, { Rng61 } },
	{ { Rng65 }, { Rng66 }, { Rng62 } },
	{ { Rng65 }, { Rng66 }, { Rng63 } },
	{ { Rng65 }, { Rng66 }, { Rng64 } },
	{ { Rng65 }, { Rng66 }, { Rng65 } },
	{ { Rng65 }, { Rng66 }, { Rng66 } },

	// 186
	{ { Rng66 }, { Rng61 }, { Rng61 } },
	{ { Rng66 }, { Rng61 }, { Rng62 } },
	{ { Rng66 }, { Rng61 }, { Rng63 } },
	{ { Rng66 }, { Rng61 }, { Rng64 } },
	{ { Rng66 }, { Rng61 }, { Rng65 } },
	{ { Rng66 }, { Rng61 }, { Rng66 } },
	// 192
	{ { Rng66 }, { Rng62 }, { Rng61 } },
	{ { Rng66 }, { Rng62 }, { Rng62 } },
	{ { Rng66 }, { Rng62 }, { Rng63 } },
	{ { Rng66 }, { Rng62 }, { Rng64 } },
	{ { Rng66 }, { Rng62 }, { Rng65 } },
	{ { Rng66 }, { Rng62 }, { Rng66 } },
	// 198
	{ { Rng66 }, { Rng63 }, { Rng61 } },
	{ { Rng66 }, { Rng63 }, { Rng62 } },
	{ { Rng66 }, { Rng63 }, { Rng63 } },
	{ { Rng66 }, { Rng63 }, { Rng64 } },
	{ { Rng66 }, { Rng63 }, { Rng65 } },
	{ { Rng66 }, { Rng63 }, { Rng66 } },
	// 204
	{ { Rng66 }, { Rng64 }, { Rng61 } },
	{ { Rng66 }, { Rng64 }, { Rng62 } },
	{ { Rng66 }, { Rng64 }, { Rng63 } },
	{ { Rng66 }, { Rng64 }, { Rng64 } },
	{ { Rng66 }, { Rng64 }, { Rng65 } },
	{ { Rng66 }, { Rng64 }, { Rng66 } },
	// 210
	{ { Rng66 }, { Rng65 }, { Rng61 } },
	{ { Rng66 }, { Rng65 }, { Rng62 } },
	{ { Rng66 }, { Rng65 }, { Rng63 } },
	{ { Rng66 }, { Rng65 }, { Rng64 } },
	{ { Rng66 }, { Rng65 }, { Rng65 } },
	{ { Rng66 }, { Rng65 }, { Rng66 } },
	// 216
	{ { Rng66 }, { Rng66 }, { Rng61 } },
	{ { Rng66 }, { Rng66 }, { Rng62 } },
	{ { Rng66 }, { Rng66 }, { Rng63 } },
	{ { Rng66 }, { Rng66 }, { Rng64 } },
	{ { Rng66 }, { Rng66 }, { Rng65 } },
	{ { Rng66 }, { Rng66 }, { Rng66 } },

};

#define		nRngCnt		(sizeof( ColRng2 ) / sizeof( COLRNG2 ))
// =====================================
#endif	//		ADDRNG1

void	chknp( void )
{
int	i;
	i = 0;
}


static	char	szCntBuf[32];

#ifdef	ADDPROGM
#define	WSF_0001		9001
#define	WSF_0002		9002
#define	WSF_0003		9003
#define	WSF_0004		9004
typedef	struct	tagWSFMSG {
	int		wsf_Val;
	LPSTR	wsf_Ptr;
}WSFMSG;
typedef WSFMSG MLPTR LPWSFMSG;

char	szNoForm[] = "Unknown Form Value!!!";
WSFMSG	wsfMsg[] = {
	{ WSF_0001, "Extracting all the different colors from the 24-bit image bits up to a maximum of %u colours into memory lpPal!" },
	{ WSF_0003, "Writing the %u colors to TEMPPAL.TXT from memory lpPal..." },
	{ WSF_0004, "List of COLORS extracted, with frequency (from memory lpPal)...\r\n" },
	{ 0,        0 }
};

LPSTR	GetWForm( int	val )
{
	LPSTR		lpf;
	LPWSFMSG	lpw;
	int			nv;

	lpf = &szNoForm[0];
	lpw = &wsfMsg[0];
	while( nv = lpw->wsf_Val )
	{
		if( nv == val )
		{
			lpf = lpw->wsf_Ptr;
			break;
		}
		lpw++;
	}
	return lpf;
}

#endif	// ADDPROGM

// ====================================================
//BOOL	SortPalRGB( LPLOGPALETTE lpSrc, LPDWORD lpSF,
//				    LPLOGPALETTE lpDst, LPDWORD lpDF )
//
// Sort the logical palette from the SOURCE to the
//	Destination according to RGB value
//
// ====================================================
BOOL	SortPalRGB( LPLOGPALETTE lpSrc, LPDWORD lpSF,
				    LPLOGPALETTE lpDst, LPDWORD lpDF )
{
	BOOL	flg;
	WORD	wCCols, ccc, cc2, MPos;
	DWORD	MSiz, TSiz;
	LPSTR	lptmp;
	BYTE	c[4];

	flg = TRUE;
	if( lpSrc && lpSF && lpDst && lpDF &&
		( wCCols = lpSrc->palNumEntries ) )
	{
		flg = FALSE;
		lptmp = GetTmp1();
		lpDst->palVersion    = PALVERSION;
		lpDst->palNumEntries = wCCols;
		// THIS IS A SIMPLE COLOR SORT PER RGB VALUE
		// =========================================
#ifdef	ADDPROGM
		wsprintf( lptmp, "Sorting %u colors per RGB value from memory lpPal to lpPal2...", wCCols );
		SetProgMInfo( lptmp );
		SetProgM( 0, (DWORD)wCCols );	// 0%
#endif
		ccc = 0;
		for( cc2 = 0; cc2 < wCCols; cc2++ )
		{
			MSiz = RGB( 255, 255, 255 );
			for( ccc = 0; ccc < wCCols; ccc++ )
			{
				if( lpSrc->palPalEntry[ccc].peFlags )
				{
					c[0] = lpSrc->palPalEntry[ccc].peBlue;
					c[1] = lpSrc->palPalEntry[ccc].peGreen;
					c[2] = lpSrc->palPalEntry[ccc].peRed;
					TSiz = RGB( c[0], c[1], c[2] );
					if( TSiz <= MSiz )
					{
						MSiz = TSiz;
						MPos = ccc;
					}
				}	// If it has a count
			}
			// Get this MINIMUM Entry from memory lpPal
			c[0] = lpSrc->palPalEntry[MPos].peBlue;
			c[1] = lpSrc->palPalEntry[MPos].peGreen;
			c[2] = lpSrc->palPalEntry[MPos].peRed;
			c[3] = lpSrc->palPalEntry[MPos].peFlags; // Get freq.
			// Ensure entry NOT found again
			lpSrc->palPalEntry[MPos].peFlags = 0;
			// Add this entry to memory lpPal2
			lpDst->palPalEntry[cc2].peBlue = c[0];
			lpDst->palPalEntry[cc2].peGreen = c[1];
			lpDst->palPalEntry[cc2].peRed = c[2];
			lpDst->palPalEntry[cc2].peFlags = c[3];
			lpDF[cc2] = lpSF[MPos];	// Transfer the COUNT
#ifdef	ADDPROGM
			SetProgM( (DWORD)cc2, (DWORD)wCCols );	// 0 - 100%
			if( bUsrAbort )
				flg = TRUE;
#endif
			if( flg )
				break;
		}	/* For each of the FOUND colors ... */
#ifdef	WRTPDIAG
		if( !flg )
		{
#ifdef	ADDPROGM
			wsprintf( lptmp, "Writing the SORTED %u colors to TEMPPAL.TXT from memory lpPal2...",
				wCCols );
			SetProgMInfo( lptmp );
#endif
			WrtPalDiag( lpDst, TRUE,
				(LPSTR)"Sorted List (per simple RGB value) from memory lpPal2...\r\n",
				lpDF );
		}
#endif	// WRTPDIAG
#ifdef	ADDPROGM
		SetProgM( (DWORD)wCCols, (DWORD)wCCols );	// 100%
		if( bUsrAbort )
			flg = TRUE;
#endif
	}
	return flg;
}

// ====================================================
//BOOL	SortPalFREQ( LPLOGPALETTE lpSrc, LPDWORD lpSF,
//				    LPLOGPALETTE lpDst, LPDWORD lpDF )
//
// Sort the logical palette from the SOURCE to the
//	Destination according to RGB value
//
// ====================================================
BOOL	SortPalFREQ( LPLOGPALETTE lpSrc, LPDWORD lpSF,
				    LPLOGPALETTE lpDst, LPDWORD lpDF )
{
	BOOL	flg;
	WORD	wCCols, ccc, cc2;
	LPSTR	lptmp;
	int		iLen;
	DWORD	TSiz, CSiz, lH, lW;
	BYTE	c[4];

	flg = TRUE;
	if( lpSrc && lpSF && lpDst && lpDF &&
		( wCCols = lpSrc->palNumEntries ) )
	{
		flg = FALSE;
		lpDst->palVersion    = PALVERSION;
		lpDst->palNumEntries = wCCols;
		lptmp = GetTmp2();
#ifdef	ADDPROGM
		wsprintf( lptmp, "Sorting %u colors PER FREQUENCY from memory lpPal to lpPal2... ", wCCols );
		iLen = lstrlen( lptmp );
		SetProgMInfo( lptmp );
		SetProgM( 0, (DWORD)wCCols );	// 0%
#endif
		TSiz = 0;
		for( cc2 = 0; cc2 < wCCols; cc2++ )
		{
			if( (CSiz = lpSF[cc2]) > TSiz )
				TSiz = CSiz;
		}
		ccc = 0;	// and begin of list
		while( TSiz )	// Assumes LOWEST freq. == 1
		{
			lH = 0;	// Start with ZERO = EXIT loop!
#ifdef	ADDPROGM
			wsprintf( (lptmp + iLen),
				"Seeking %u ...", TSiz );
			SetProgMInfo( lptmp );
#endif
			for( cc2 = 0; cc2 < wCCols; cc2++ )
			{
				lW = lpSF[cc2];
				if( lW == TSiz )
				{
					// This frequency
					c[0] = lpSrc->palPalEntry[cc2].peBlue;
					c[1] = lpSrc->palPalEntry[cc2].peGreen;
					c[2] = lpSrc->palPalEntry[cc2].peRed;
					c[3] = lpSrc->palPalEntry[cc2].peFlags;
					lpDst->palPalEntry[ccc].peBlue = c[0];
					lpDst->palPalEntry[ccc].peGreen = c[1];
					lpDst->palPalEntry[ccc].peRed = c[2];
					lpDst->palPalEntry[ccc].peFlags = c[3];
					lpDF[ccc] = TSiz;
					ccc++;	// and bump to next
				}
				else if( lW < TSiz )
				{
					// This is a LESSER Freq.
					if( lW > lH )
					{
						// But seeking NEXT largest
						lH = lW;
					}
				}
#ifdef	ADDPROGM
				if( bUsrAbort )
					flg = TRUE;
#endif
				if( flg )
					break;
			}	// Look for THIS frequency
#ifdef	ADDPROGM
			SetProgM( (DWORD)ccc, (DWORD)wCCols );	// 0 - 99%
			if( bUsrAbort )
				flg = TRUE;
#endif
			if( flg )
				break;
			TSiz = lH;	// Next highest frequency
		}	// while( TSiz )
		if( !flg )
		{
#ifdef	WRTPDIAG
#ifdef	ADDPROGM
			wsprintf( lptmp, "Writing %u colors SORTED PER FREQUENCY to TEMPPAL.TXT from memory lpPal...",
				wCCols );
			SetProgMInfo( lptmp );
#endif
			WrtPalDiag( lpDst, TRUE,
				(LPSTR)"Sorted List per FREQUENCY (from memory lpPal)...\r\n",
				lpDF );
#endif	// WRTPDIAG
#ifdef	ADDPROGM
			SetProgM( (DWORD)wCCols, (DWORD)wCCols );	// 100%
			if( bUsrAbort )
				flg = TRUE;
#endif
		}
	}
	return flg;
}

// create a BIG 24-bit palette
// it can have THOUSANDS of colors!!!
// ==============================================
HPALETTE	CreateDPal24( HANDLE hDIB, BOOL fNew )
{
	HPALETTE	      NewPal;
	LPBITMAPINFOHEADER lpbmih;
	LPRGBTRIPLE	   lpRGB;
	DWORD	         lWid, lHgt, lW, lH, lNWid;
	PINT8	         hp;
	int		      i;
	BYTE	         c[16];
	HGLOBAL	      hLogPal;
	LPLOGPALETTE	lpPal;
	HGLOBAL	      hLogPal2;
	LPLOGPALETTE	lpPal2;
	WORD		      wNCols, wCCols, ccc, cc2, cc3;
	BOOL		      flg;
	DWORD		      ddSiz, MCnt;
	LPSTR		      lpcnt, lptmp;
	HGLOBAL		   hDW, hDW2;
	LPDWORD		   lpDW, lpDW2;
	LPCOLRNG2	   lpCR;
	HPTBL		      hpTbl;

	Hourglass( TRUE );	// SetCursor
//	if( MakeTable( &hpTbl, 1 ) )
//	{
//		if( hpTbl.hTbl )
//			DVGlobalFree( hpTbl.hTbl );
//		hpTbl.hTbl = 0;
//	}
	lptmp = GetTmp3();
#ifdef	ADDPROGM
	PutProgM();		// Open progress windows
#endif
	NewPal = 0;
	lpcnt = &szCntBuf[0];
//	wNCols = 256;
	wNCols = (WORD)gdwMaxCols;	// Maximum colours
	wCCols = 0;
	hLogPal = hLogPal2 = 0;
	lpPal = lpPal2 = 0;
	hDW = hDW2 = 0;
	lpDW = lpDW2 = 0;
	MCnt = 0;
	flg = FALSE;
	ddSiz = sizeof( LOGPALETTE ) + ( sizeof( PALETTEENTRY ) * wNCols );
	if(( lpbmih = (LPBITMAPINFOHEADER) DVGlobalLock( hDIB ) ) &&   // LOCK DIB HANDLE
		( lpbmih->biSize == sizeof(BITMAPINFOHEADER) ) &&
		( lpbmih->biBitCount == 24 ) &&
		( lpbmih->biCompression == BI_RGB ) &&
		( (void *)( lpRGB = (LPRGBTRIPLE)FindDIBBits((LPSTR)lpbmih) ) ==
			(void *)((LPSTR)lpbmih + sizeof(BITMAPINFOHEADER) ) ) &&
		( lWid = lpbmih->biWidth ) &&
		( lHgt = lpbmih->biHeight) )
	{
		if( (lWid*3) % 4 )
			lNWid = (((lWid*3) / 4) + 1) * 4;
		else
			lNWid = lWid * 3;
		lNWid -= lWid * 3;
		ddSiz = lWid * lHgt;
		ddSiz = sizeof( LOGPALETTE ) + ( sizeof( PALETTEENTRY ) * wNCols );
      if( ( hLogPal = DVGlobalAlloc( GHND, ddSiz ) ) &&
		    ( hDW = DVGlobalAlloc( GHND, (sizeof(DWORD) * wNCols) ) ) &&
		    ( lpPal = (LPLOGPALETTE) DVGlobalLock( hLogPal ) ) &&
		    ( lpDW = (LPDWORD) DVGlobalLock( hDW ) ) )
      {
#ifdef	ADDPROGM
		   wsprintf( lptmp, GetWForm( WSF_0001 ),
			   wNCols );
		   SetProgMInfo( lptmp );
		   SetProgM( 0, 100 );
#endif
		   lpPal->palVersion    = PALVERSION;
		   lpPal->palNumEntries = wNCols;	// This is MAX.
		   hp = ( PINT8 )lpRGB;
		   for( lH = 0; lH < lHgt; lH++ )
		   {
			   for( lW = 0; lW < lWid; lW++ )
			   {
				   for( i = 0; i < 3; i++ )
				   {
					   c[i] = *hp++;
				   }
/* ================================================ */
				   ccc = 0;
				   if( wCCols == 0 )
				   {
Add_New:
					   // ADD a NEW Color to LIST
					   lpPal->palPalEntry[ccc].peRed   = c[0];
					   lpPal->palPalEntry[ccc].peGreen = c[1];
					   lpPal->palPalEntry[ccc].peBlue  = c[2];
					   lpPal->palPalEntry[ccc].peFlags = 1;
					   lpDW[ccc] = 1;
					   wCCols++;	// BUMP Color COUNTER
#ifdef	ADDPROGM
					   // Put up the NEW count ...
					   wsprintf( lpcnt,
						   "Count: %u colors.",
						   wCCols );
					   SetProgMInfo2( lpcnt );
#endif	// ADDPROGM
				   }
				   else if( wCCols < wNCols )	/* If we can ADD a new color ... */
				   {
					   for( ccc = 0;  ccc < wCCols;  ccc++ )
					   {
						   if( (lpPal->palPalEntry[ccc].peRed == c[0] ) &&
							    (lpPal->palPalEntry[ccc].peGreen == c[1] ) &&
							    (lpPal->palPalEntry[ccc].peBlue == c[2] ) )
                     {
							   if( lpPal->palPalEntry[ccc].peFlags < 255 )
								   lpPal->palPalEntry[ccc].peFlags++;
							   lpDW[ccc]++;	// BUMP the COUNT
							   if( lpDW[ccc] > MCnt )
								   MCnt = lpDW[ccc];	// Get largest COUNT
							   break;
						   }
					   }
					   if( ccc == wCCols )	/* If NO match found ... */
					   {
						   goto Add_New;
					   }
				   }
				   else	/* WOW - We must find BEST MATCH ... or INCREASE Buffer */
				   {
					   ccc = (WORD)-1;
					   flg = TRUE;	/* But for now just exit with an ERROR message */
				   }
#ifdef	ADDPROGM
				   SetProgM( lH, lHgt );	// 1 to 99% done		
				   if( bUsrAbort )
					   flg = TRUE;
#endif
/* ================================================ */
				   if( flg )
					   break;
			   }	/* For WIDTH */
			   for( lW = 0; lW < lNWid; lW++ )
			   {
				   hp++;	// Use up any alignement bytes
			   }
			   if( flg )
				   break;
		   }	/* For HEIGHT ... */

		   // WOW, done EXTRACTING ALL THE COLORS.
		   // Actually reading the 24-bit BITMAP and putting
		   // it into a LOGICAL palette form.
		   if( flg || (wCCols == 0 ) )
		   {
			   flg = TRUE;
			   if( hLogPal && lpPal )
				   DVGlobalUnlock( hLogPal );
			   if( hLogPal )
				   DVGlobalFree( hLogPal );
			   if( hDW && lpDW )
				   DVGlobalUnlock( hDW );
			   lpDW = 0;
			   if( hDW )
				   DVGlobalFree( hDW );
			   hDW = 0;
			   lpPal = 0;
			   hLogPal = 0;
		   }
		   else
		   {
            // This is SUCCESS ...
			   // We have CREATED a logical PALETTE ... NOW???
			   lpPal->palNumEntries = wCCols;
#ifdef	WRTPDIAG
#ifdef	ADDPROGM
			   wsprintf( lptmp,
				   GetWForm( WSF_0003 ),
				   wNCols );
			   SetProgMInfo( lptmp );
#endif
			   WrtPalDiag( lpPal, FALSE,
				   (LPSTR)GetWForm( WSF_0004 ),
				   lpDW );
#endif	// WRTPDIAG
#ifdef	ADDPROGM
			   SetProgM( lHgt, lHgt );
#endif
			   if( fNew )
			   {
				   lpPal->palNumEntries = wCCols;
				   DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE (for fNew return)
				   DVGlobalUnlock( hLogPal );
				   if( hDW && lpDW )
					   DVGlobalUnlock( hDW );
				   lpDW = 0;
				   if( hDW )
					   DVGlobalFree( hDW );
				   hDW = 0;
				   Hourglass( FALSE );	/* SetCursor ... */
#ifdef	ADDPROGM
				   KillProgM();
#endif
				   return((HPALETTE) hLogPal );	/* Return this array of colors ... */
			   }
			   lpPal->palNumEntries = wCCols;	/* Set ACTUAL colors extracted ... */
			   ddSiz = sizeof( LOGPALETTE ) + ( sizeof( PALETTEENTRY ) * wCCols );
			   if( ( hLogPal2 = DVGlobalAlloc( GHND, ddSiz ) ) &&
				    ( hDW2 = DVGlobalAlloc( GHND, (sizeof(DWORD) * wCCols) ) ) &&
				    ( lpPal2 = (LPLOGPALETTE) DVGlobalLock( hLogPal2 ) ) &&
				    ( lpDW2 = (LPDWORD) DVGlobalLock( hDW2 ) ) )
            {
				   // SORT per RGB from SOURCE to DESTINATION
				   // =======================================
//				flg = SortPalRGB( lpPal, lpDW, lpPal2, lpDW2 );
//				if( !flg )	// If still flying
//				{
					// SORT per FREQ. from SOURCE  to  DESTINATION
					// ===========================================
//					flg = SortPalFREQ( lpPal2, lpDW2, lpPal, lpDW );
//				}
				   flg = SortPalFREQ( lpPal, lpDW, lpPal2, lpDW2 );
/* ======== ALL THIS FOR THIS ================== */
				   chknp();
				   if( !flg )	/* If NO ABORT attempted ... */
				   {
#ifdef	ADDPROGM
					   wsprintf( lptmp, "Attempting to CreatePalette using the SORTED %u colors in a logical palette...",
						   wCCols );
					   SetProgMInfo( lptmp );
#endif
					   for( cc2 = 0; cc2 < wCCols; cc2++ )
						   lpPal2->palPalEntry[cc2].peFlags = 0;
					   NewPal = CreatePalette( lpPal2 );
					   if( NewPal == 0 )
					   {
#ifdef	ADDPROGM
						   wsprintf( lptmp,
							   "FAILED in an attempt to CreatePalette using the SORTED %u colors in a logical palette (in memory lpPal2)...",
							   wCCols );
						   SetProgMInfo( lptmp );
#endif
						   if( lpPal2->palNumEntries > 256 )
						   {
							   // ONLY chance is to REDUCE colors to
							   // JUST 256 or LESS!!!
//==============================================================
							   int		iii, iii2;
							   BOOL	g256;

							   g256 = FALSE;
							   for( iii = 1; iii < 100; iii++ )
							   {
#ifdef	ADDPROGM
								   if( iii == 1 )
								   {
									   wsprintf( lptmp,
										   "Generating the first %u%% table of ranges...",
										   iii );
									   SetProgMInfo( lptmp );
								   }
								   else
								   {
									   wsprintf( lptmp,
										   "Previous Table yielded more than 256 colours. Now generating a %u%% table of ranges...",
									   	iii );
									   SetProgMInfo( lptmp );
								   }
								   SetProgM( 0, 100 );
#endif
								   if( MakeTable( &hpTbl, iii ) )
								   {
									   if( hpTbl.hTbl && 
										   (iii2 = hpTbl.iCnt) &&
										   (lpCR = (LPCOLRNG2) DVGlobalLock( hpTbl.hTbl ) ) )
									   {
//*******************************************
#ifdef	ADDPROGM
										   if( iii == 1 )
										   {
											   wsprintf( lptmp,
												   "Using the %u%% table with %u entries...",
												   iii,
												   iii2 );
										   }
										   else
										   {
											   wsprintf( lptmp,
												   "Previous Table yielded more than 256 colours. Now using the %u%% table with %u entries...",
												   iii,
												   iii2 );
										   }
										   SetProgMInfo( lptmp );
										   SetProgM( 1, 100 );	// 1%
#endif
								         // Set ALL as available
							            for( cc2 = 0; cc2 < wCCols; cc2++ )
								            lpPal2->palPalEntry[cc2].peFlags = 1;
							            ccc = 0;	// Start with NONE
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
				if( wCCols < iii2 )
				{
					for( cc2 = 0; cc2 < wCCols; cc2++ )
					{
						if( lpPal2->palPalEntry[cc2].peFlags )
						{
							c[0] = lpPal2->palPalEntry[cc2].peBlue;
							c[1] = lpPal2->palPalEntry[cc2].peGreen;
							c[2] = lpPal2->palPalEntry[cc2].peRed;
							lH = 0;	// Start a COUNTER
							for( i = 0; i < iii2; i++ )
							{
								// SEEK the RANGE for this COLOR
								c[4] = lpCR[i].bl.cMin;
								c[5] = lpCR[i].bl.cMax;
								c[6] = lpCR[i].bl.cVal;

								c[7] = lpCR[i].gr.cMin;
								c[8] = lpCR[i].gr.cMax;
								c[9] = lpCR[i].gr.cVal;

								c[10] = lpCR[i].re.cMin;
								c[11] = lpCR[i].re.cMax;
								c[12] = lpCR[i].re.cVal;
								if( ( ( c[0] >= c[4] ) && ( c[0] <= c[5] ) ) &&
									( ( c[1] >= c[7] ) && ( c[1] <= c[8] ) ) &&
									( ( c[2] >= c[10] ) && ( c[2] <= c[11] ) ) )
								{
									// Kill this entry
									lpPal2->palPalEntry[cc2].peFlags = 0;
									lpPal->palPalEntry[ccc].peBlue  = c[6];
									lpPal->palPalEntry[ccc].peGreen = c[9];
									lpPal->palPalEntry[ccc].peRed   = c[12];
									lpPal->palPalEntry[ccc].peFlags = 0;
									lpDW[ccc] = lpDW2[cc2];
									lH++;	// Got RANGE
									for( cc3 = cc2+1; cc3 < wCCols; cc3++ )
									{
										// Seek MORE in this RANGE
										if( lpPal2->palPalEntry[cc3].peFlags )
										{
											c[0] = lpPal2->palPalEntry[cc3].peBlue;
											c[1] = lpPal2->palPalEntry[cc3].peGreen;
											c[2] = lpPal2->palPalEntry[cc3].peRed;
											if( ( ( c[0] >= c[4] ) && ( c[0] <= c[5] ) ) &&
												( ( c[1] >= c[7] ) && ( c[1] <= c[8] ) ) &&
												( ( c[2] >= c[10] ) && ( c[2] <= c[11] ) ) )
											{
												// Kill this entry
												lpPal2->palPalEntry[cc3].peFlags = 0;
												lpDW[ccc] += lpDW2[cc3];
												lH++;	// Another
											}
										}
									}	// for all the balance
									// We have found ALL in this range
									ccc++;	// Bump the FOUND count
									break;	// break from table
								}	// is IN range
							}	// for ALL table tanges
							if( lH == 0 )
							{
								// WE HAVE NO RANGE FOR THIS COLOUR!!!
								lW = 0;	// THIS IS AN ERROR!!!
								c[0] = lpPal2->palPalEntry[cc2].peBlue;
								c[1] = lpPal2->palPalEntry[cc2].peGreen;
								c[2] = lpPal2->palPalEntry[cc2].peRed;
								lpPal2->palPalEntry[cc2].peFlags = 0;
								lpPal->palPalEntry[ccc].peBlue  = c[0];
								lpPal->palPalEntry[ccc].peGreen = c[1];
								lpPal->palPalEntry[ccc].peRed   = c[2];
								lpPal->palPalEntry[ccc].peFlags = 0;
								lpDW[ccc] = lpDW2[cc2];
								ccc++;
							}
						}	// if NOT already matched
						if( ccc > 256 )
							break;
#ifdef	ADDPROGM
						SetProgM( (DWORD)cc2, (DWORD)wCCols );	// 75 - 100%
						if( bUsrAbort )
							flg = TRUE;
#endif
						if( flg )
						{
							break;
						}
					}	// for EACH color FOUND (cc2 < wCCols)
				}
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
				else
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
				{
							for( i = 0; i < iii2; i++ )
							{
								// Extract the RANGES
								c[4] = lpCR[i].bl.cMin;
								c[5] = lpCR[i].bl.cMax;
								c[6] = lpCR[i].bl.cVal;

								c[7] = lpCR[i].gr.cMin;
								c[8] = lpCR[i].gr.cMax;
								c[9] = lpCR[i].gr.cVal;

								c[10] = lpCR[i].re.cMin;
								c[11] = lpCR[i].re.cMax;
								c[12] = lpCR[i].re.cVal;
//								wsprintf( lptmp,
//									"Seeking B(%u-%u)=%u G(%u-%u)=%u R(%u-%u)=%u!",
//									( c[4] & 0xff ),
//									( c[5] & 0xff ),
//									( c[6] & 0xff ),
//									( c[7] & 0xff ),
//									( c[8] & 0xff ),
//									( c[9] & 0xff ),
//									( c[10] & 0xff ),
//									( c[11] & 0xff ),
//									( c[12] & 0xff ) );
								lH = 0;
								for( cc2 = 0; cc2 < wCCols; cc2++ )
								{
									if( lpPal2->palPalEntry[cc2].peFlags )
									{
										c[0] = lpPal2->palPalEntry[cc2].peBlue;
										c[1] = lpPal2->palPalEntry[cc2].peGreen;
										c[2] = lpPal2->palPalEntry[cc2].peRed;
										if( ( ( c[0] >= c[4] ) && ( c[0] <= c[5] ) ) &&
											( ( c[1] >= c[7] ) && ( c[1] <= c[8] ) ) &&
											( ( c[2] >= c[10] ) && ( c[2] <= c[11] ) ) )
										{
											// Kill this entry
											lpPal2->palPalEntry[cc2].peFlags = 0;
											if( lH == 0 )
											{
												lpPal->palPalEntry[ccc].peBlue  = c[6];
												lpPal->palPalEntry[ccc].peGreen = c[9];
												lpPal->palPalEntry[ccc].peRed   = c[12];
												lpPal->palPalEntry[ccc].peFlags = 0;
												lpDW[ccc] = 0;
											}
											lpDW[ccc] += lpDW2[cc2];
											lH++;
										}
									}
#ifdef	ADDPROGM
									if( bUsrAbort )
										flg = TRUE;
#endif
									if( flg )
										break;
								}	// Search for THIS RANGE
#ifdef	ADDPROGM
								SetProgM( i, iii2 );	// 0 - 100%
								if( bUsrAbort )
									flg = TRUE;
#endif
								if( flg )
								{
									break;
								}
								else
								{
									if( lH )
									{
										ccc++;	// To NEXT
										lH = 0;
										if( ccc > 256 )
											break;
									}
								}
							}	// For 0 to iii2
				}
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//***********************************************
										   DVGlobalUnlock( hpTbl.hTbl );
									   }
									   if( hpTbl.hTbl )
										   DVGlobalFree( hpTbl.hTbl );
									   hpTbl.hTbl = 0;
								   }
								   if( ccc &&
									   (ccc <= 256) )
								   {
									   g256 = TRUE;
									   break;
								   }
							   }	// for EACH percentage
//==============================================================
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef	ADDRNG1
// =====================================

							if( !g256 )
							{
#ifdef	ADDPROGM
							wsprintf( lptmp,
								"Attempting to put Colors into %u ranges...",
								nRngCnt );
							SetProgMInfo( lptmp );
							SetProgM( 0, 100 );
#endif
							lpCR = &ColRng2[0];
							for( cc2 = 0; cc2 < wCCols; cc2++ )
								lpPal2->palPalEntry[cc2].peFlags = 1;
							ccc = 0;
							for( i = 0; i < nRngCnt; i++ )
							{
								// Extract the RANGES
								c[4] = lpCR[i].bl.cMin;
								c[5] = lpCR[i].bl.cMax;
								c[6] = lpCR[i].bl.cVal;

								c[7] = lpCR[i].gr.cMin;
								c[8] = lpCR[i].gr.cMax;
								c[9] = lpCR[i].gr.cVal;

								c[10] = lpCR[i].re.cMin;
								c[11] = lpCR[i].re.cMax;
								c[12] = lpCR[i].re.cVal;
//								wsprintf( lptmp,
//									"Seeking B(%u-%u)=%u G(%u-%u)=%u R(%u-%u)=%u!",
//									( c[4] & 0xff ),
//									( c[5] & 0xff ),
//									( c[6] & 0xff ),
//									( c[7] & 0xff ),
//									( c[8] & 0xff ),
//									( c[9] & 0xff ),
//									( c[10] & 0xff ),
//									( c[11] & 0xff ),
//									( c[12] & 0xff ) );
								lH = 0;
								for( cc2 = 0; cc2 < wCCols; cc2++ )
								{
									if( lpPal2->palPalEntry[cc2].peFlags )
									{
										c[0] = lpPal2->palPalEntry[cc2].peBlue;
										c[1] = lpPal2->palPalEntry[cc2].peGreen;
										c[2] = lpPal2->palPalEntry[cc2].peRed;
										if( ( ( c[0] >= c[4] ) && ( c[0] <= c[5] ) ) &&
											( ( c[1] >= c[7] ) && ( c[1] <= c[8] ) ) &&
											( ( c[2] >= c[10] ) && ( c[2] <= c[11] ) ) )
										{
											// Kill this entry
											lpPal2->palPalEntry[cc2].peFlags = 0;
											if( lH == 0 )
											{
												lpPal->palPalEntry[ccc].peBlue  = c[6];
												lpPal->palPalEntry[ccc].peGreen = c[9];
												lpPal->palPalEntry[ccc].peRed   = c[12];
												lpPal->palPalEntry[ccc].peFlags = 0;
												lpDW[ccc] = 0;
											}
											lpDW[ccc] += lpDW2[cc2];
											lH++;
										}
									}
#ifdef	ADDPROGM
									if( bUsrAbort )
										flg = TRUE;
#endif
									if( flg )
										break;
								}	// Search for THIS RANGE
#ifdef	ADDPROGM
								SetProgM( i, nRngCnt );	// 0 - 100%
								if( bUsrAbort )
									flg = TRUE;
#endif
								if( flg )
								{
									break;
								}
								else
								{
									if( lH )
									{
										ccc++;	// To NEXT
										lH = 0;
									}
								}
                     }	// For 0 to nRngCnt-1
							}	// !g256
// =====================================
#endif	//		ADDRNG1

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
							   if( !flg && ccc )
							   {
								   lpPal->palVersion    = PALVERSION;
								   lpPal->palNumEntries = ccc;
#ifdef	WRTPDIAG
#ifdef	ADDPROGM
								   wsprintf( lptmp, "Writing %u colors to TEMPPAL.TXT from memory lpPal...",
									   ccc );
								   SetProgMInfo( lptmp );
#endif
								   wsprintf( lptmp, "Reduced to %u color ranges in memory lpPal!\r\n",
									   ccc );
								   WrtPalDiag( lpPal, TRUE,
									   lptmp,
									   lpDW );
#endif	// WRTPDIAG
								   NewPal = CreatePalette( lpPal );
							   }
//							lpPal->palNumEntries = 256;
//							NewPal = CreatePalette( lpPal );
//							lpPal2->palNumEntries = 256;
//							NewPal = CreatePalette( lpPal2 );
						   }	// More than 256 colours
						   if( NewPal == 0 )
						   {
							   chknp();
						   }
					   }
				   }
/* ============================================= */
			   }
			   else
			   {
				   flg = TRUE;
			   }
			   if( hLogPal && lpPal )
				   DVGlobalUnlock( hLogPal );
			   if( hLogPal )
				   DVGlobalFree( hLogPal );
			   lpPal = 0;
			   hLogPal = 0;
		   }
      } /* if we GOT the memory ... */
	}	/* if we get many things right ... */

	if( hDIB && lpbmih )
		DVGlobalUnlock( hDIB ); // UNLOCK DIB HANDLE
	if( hLogPal && lpPal )
		DVGlobalUnlock( hLogPal );
	if( hLogPal )
		DVGlobalFree( hLogPal );
	if( hLogPal2 && lpPal2 )
		DVGlobalUnlock( hLogPal2 );
	if( hLogPal2 )
		DVGlobalFree( hLogPal2 );
	if( hDW && lpDW )
		DVGlobalUnlock( hDW );
	lpDW = 0;
	if( hDW )
		DVGlobalFree( hDW );
	hDW = 0;
	if( hDW2 && lpDW2 )
		DVGlobalUnlock( hDW2 );
	lpDW2 = 0;
	if( hDW2 )
		DVGlobalFree( hDW2 );
	hDW2 = 0;
	Hourglass( FALSE );	/* SetCursor ... */
#ifdef	ADDPROGM
	KillProgM();
#endif
   return( NewPal );

}


///* kept in lpDIBInfo->di_hCOLR */
///* =========================== */
//typedef struct tagCOLR {
//	COLORREF	cr_Color;
//	DWORD		cr_Freq;
//}COLR;
//typedef COLR MLPTR LPCOLR;
//typedef struct tagPALEX {
//	/* ======================================================= */
//	DWORD	px_Size;		/* SIZE of this instance of struct */
//	DWORD	px_Count;		/* COUNT of color listed in array  */
//	COLR	px_Colrs[1];	/* array of COLOR and FREQUENCY    */
//	/* ======================================================= */
//}PALEX;
//typedef PALEX MLPTR LPPALEX;
HPALETTE	CreateDIBPal24( PDI lpDIBInfo, HANDLE hDIB, BOOL fNew )
{
   HPALETTE    hPal = 0;
   HGLOBAL     hg;
   LPSTR       lpb;

   if( !lpDIBInfo )
   {
      return( CreateDPal24( hDIB, fNew ) );
   }

   // else we should HAVE a color count
   if( hg = lpDIBInfo->di_hCOLR )
   {
      if( lpb = DVGlobalLock(hg) )
      {
         DWORD          dwCnt, ddSiz;
         LPPALEX        ppx;
         LPCOLR         pcr;
         ppx = (LPPALEX)lpb;
         if( ( dwCnt = GetCOLRCnt(lpb) ) &&
             ( dwCnt == ppx->px_Count  ) )
         {
            HGLOBAL        hLogPal;
            LPLOGPALETTE   lpPal;
            PALETTEENTRY * ppe;
            DWORD          dwi;

            hLogPal = 0;
            lpPal = 0;
      		ddSiz = sizeof( LOGPALETTE ) + ( sizeof( PALETTEENTRY ) * dwCnt );
            if( ( hLogPal = DVGlobalAlloc( GHND, ddSiz ) ) &&
		          ( lpPal = (LPLOGPALETTE) DVGlobalLock( hLogPal ) ) )
            {
      		   lpPal->palVersion    = PALVERSION;
		         lpPal->palNumEntries = (WORD)dwCnt;	// This is MAX.
               ppe = &lpPal->palPalEntry[0];
               ppx = (LPPALEX)lpb;
               pcr = &ppx->px_Colrs[0];
               for( dwi = 0; dwi < dwCnt; dwi++ )
               {
                  //*ppe = *pcr->cr_Color;
                  ppe->peRed   = GetRValue( pcr->cr_Color );
                  ppe->peGreen = GetGValue( pcr->cr_Color );
                  ppe->peBlue  = GetBValue( pcr->cr_Color );
                  ppe->peFlags = 0;
                  ppe++;   // bump to next entry
                  pcr++;   // and next color
               }

               hPal = CreatePalette( lpPal );

            }
            if( hLogPal && lpPal )
               DVGlobalUnlock(hLogPal);
            if( hLogPal )
               DVGlobalFree(hLogPal);
         }

         DVGlobalUnlock(hg);

         if( !hPal )
            hPal = CreateDPal24( hDIB, fNew );

      }
   }

   return hPal;
}

// eof - DvPal24.c
