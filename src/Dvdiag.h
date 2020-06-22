
// DvDiag.h

#ifndef	_DvDiag_h
#define	_DvDiag_h

extern	void	DiagOpen( void );
extern	void	DiagString( LPSTR );
extern	void	DiagClose( void );

#ifdef	DIAGFILE2
#define	DO( a )			DiagString( a );
#else	// !DIAGFILE2
#define	DO( a )
#endif	// DIAGFILE2 y/n

#ifdef   NDEBUG
#define  sprtf
#define  chkdi
#else // !NDEBUG
extern   VOID  _cdecl sprtf( LPTSTR lpf, ... );
extern   VOID  chkdi( LPTSTR lpm, INT iln );
#endif   // NDEBUG y/n

extern   VOID  _cdecl chkme( LPTSTR lpf, ... );

#endif	// _DvDiag_h
// eof - DvDiag.h
