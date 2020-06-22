

// DvPal24.h
#ifndef	_DvPal24_h
#define	_DvPal24_h

#define		WRTPDIAG
#undef		DIAGDT

typedef	struct	tagBYTE3 {
	BYTE	cMin;
	BYTE	cMax;
	BYTE	cVal;
}BYTE3;

typedef	struct	tagCOLRNG2 {
	BYTE3	bl;
	BYTE3	gr;
	BYTE3	re;
}COLRNG2;

typedef	COLRNG2 MLPTR LPCOLRNG2;


typedef	struct	tagHPTBL {
	int		iCnt;
	HGLOBAL	hTbl;
}HPTBL;
typedef HPTBL MLPTR LPHPTBL;

extern	BOOL	MakeTable( LPHPTBL lpHTbl, int Percent );

#endif	// _DvPal24_h
// eof - DvPal24.h


