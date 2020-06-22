
#ifndef	_DvUtil2_H
#define	_DvUtil2_H
// Module: DvUtil2.h
// Services exposed by DvUtil2.c
#define  VFH(h)   ( h && ( h != INVALID_HANDLE_VALUE ) )

extern   VOID  DumpRGBQUAD( LPTSTR lps, DWORD cc );   // diag. output of BMP colour table

// new - Oct 2002 - transferred from ewm project
extern   LPTSTR   GetElapsedStg( VOID );  // return pointer to static single instance
#define  ELAPSTG  GetElapsedStg()
extern   double   GetElapTime( VOID );
#define  ELAPTIME GetElapTime()

extern	VOID	InitTimers( VOID );
extern	INT		SetBTime( VOID );
extern	double	GetETime( INT i );
extern   LPTSTR   Dbl2Str( double factor, int prec );

// NEW - Nov 2000
extern   LPTSTR	GetFDTStg( FILETIME * pft );
extern   LPTSTR   GetI64Stg( PLARGE_INTEGER pli );

// Added 07DEC2000
extern   LPTSTR	Rect2Stg( LPRECT lpr );
extern   LPTSTR	RectL2Stg( PRECTL lpr );
// added 08DEC2000
#define  IS_FOLDER      1
#define  IS_FILE        2
extern   DWORD  IsValidFile( LPTSTR lpf, PWIN32_FIND_DATA pfd );

#ifdef   WRTCLRFILE
extern   VOID  GetCLRFile( LPTSTR lpt, PDI lpDIBInfo );

typedef  struct tagCLRHDR {
   char  ch_cSig[8];    // signature = "PALEX",0x1a,0
   DWORD ch_dwWidth;    // width of DIB
   DWORD ch_dwHeight;   // height of DIB
   DWORD ch_dwBPP;      // Bits per pixel
   DWORD ch_dwSize;     // size of the following PALEX structure block
}CLRHDR, * PCLRHDR;

#endif   // #ifdef   WRTCLRFILE

extern   HANDLE   DVCreateFile( LPTSTR lpf );   // returns VALID handle or ZERO if fail
extern   HANDLE   DVOpenFile2( LPTSTR lpf );    // returns VALID handle or ZERO if fail
extern   DWORD    IsClrFile( PDI lpDIBInfo );   // returns PALEX size (& handle), else ZERO

extern   LPTSTR   GetStgBuf( VOID );
extern char * GetRectStg( PRECT prc ); // like &m_Clip
extern char * GetPointStg( PPOINT pp );

#endif	// ifndef	_DvUtil2_H
// eof - DvUtil2.h

